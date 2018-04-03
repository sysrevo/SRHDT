#include "stdafx.h"
#include "tree.h"
#include "../Utils/utils.h"
#include "training_data.h"
#include <chrono>

#ifdef USE_CUDA
#include "../UtilsCudaHelper/cuda_calculator.h"
#endif

using namespace imgsr;
using namespace imgsr::utils;

// -------------------------------------------------------------------------------
//								Helper Functions
// -------------------------------------------------------------------------------

inline bool IsNumAvailable(size_t numLeft, size_t numRight, double k)
{
	__int64 tmp = (__int64)(std::max(numLeft, numRight) * k);
	__int64 tmp2 = (__int64)std::min(numLeft, numRight);
	return tmp <= tmp2;
}

// using regularized linear regression
// C = (XT * X + lamda)-1 * XT * Y
template<class Input1, class Input2>
inline EMat GetRegressionModel(const Input1 & x, const Input2 & y, double lamda)
{
	EMat xt = x.transpose();
	EMat tmp_lamda = lamda * ERowMat::Identity(x.cols(), x.cols());

	EMat c;
	if (x.rows() * x.cols() <= 360000)
	{
		c = ((xt * x + tmp_lamda).inverse()) * xt * y;
	}
	else
	{
		CudaMat cuda_xt(xt);
		EMat part1;
		{ // xt * x
			CudaMat cuda_x(x);
			CudaMat res;
			CudaCalculator::Mul(cuda_xt, cuda_x, &res);
			part1 = res.GetMat();
		}
		EMat part2;
		{ // xt * y
			CudaMat cuda_y(y);
			CudaMat res;
			CudaCalculator::Mul(cuda_xt, cuda_y, &res);
			part2 = res.GetMat();
		}
		c = (part1 + tmp_lamda).inverse() * part2;
	}
	return c;
}

// return the error reduction R at node j
inline Real GetErrorReduction(Real ej, size_t nl, Real el, size_t nr, Real er)
{
	auto nj = nl + nr;
	auto tmp_left = el * nl / nj;
	auto tmp_right = er * nr / nj;
	return ej - (tmp_left + tmp_right);
}

inline Real GetFittingError(const EMat& c, const EMat& x, const EMat& y)
{
	EMat yr;
#ifdef USE_CUDA
	if (x.rows() * x.cols() <= 360000)
	{
		yr = x * c;
	}
	else
	{
		yr = CudaCalculator::Mul(x, c);
	}
#else
    yr = x * c;
#endif
	EMat delta = y - yr;
	auto tmp = delta.squaredNorm();
	return delta.squaredNorm() / y.rows();
}

struct CalculationResult
{
	EMat model;
	Real fitting_error;
	size_t num_samples;
};

template<class Input1, class Input2>
CalculationResult DoComplexCalculation(const Input1 & x, const Input2 & y, Real lamda)
{
	assert(x.rows() == y.rows());

	CalculationResult res;
	res.num_samples = x.rows();
	{
		auto xt = x.transpose();
		auto tmp_lamda = lamda * EMat::Identity(x.cols(), x.cols());
		res.model = ((xt * x + tmp_lamda).inverse()) * xt * y;
	}
	{
		EMat yr = x * res.model;
		EMat delta = y - yr;
		res.fitting_error = delta.squaredNorm() / y.rows();
	}
	return res;
}

#ifdef USE_CUDA
template<class Input1, class Input2>
CalculationResult DoComplexCalculationGpu(const Input1 & x, const Input2 & y, Real lamda)
{
	assert(x.rows() == y.rows());

	CalculationResult res;
	res.num_samples = x.rows();

	CudaMat cuda_x(x);
	CudaMat cuda_y(y);
	{
		CudaMat cuda_xt(x.transpose());
		EMat part1;
		{ // xt * x
			CudaMat res;
			CudaCalculator::Mul(cuda_xt, cuda_x, &res);
			part1 = res.GetMat();
		}
		EMat part2;
		{ // xt * y
			CudaMat res;
			CudaCalculator::Mul(cuda_xt, cuda_y, &res);
			part2 = res.GetMat();
		}
		EMat tmp_lamda = lamda * ERowMat::Identity(x.cols(), x.cols());
		res.model = (part1 + tmp_lamda).inverse() * part2;
	}
	{
		CudaMat cuda_model(res.model);
		CudaMat cuda_yr;
		CudaCalculator::Mul(cuda_x, cuda_model, &cuda_yr);

		CudaMat cuda_delta;
		CudaCalculator::Add(cuda_y, cuda_yr, &cuda_delta, 1, -1);

		EMat delta = cuda_delta.GetMat();
		res.fitting_error = delta.squaredNorm() / y.rows();
	}
	return res;
}
#endif

template<class Input1, class Input2>
CalculationResult CalculateModelAndFittingError(const Input1 & x, const Input2 & y, Real lamda)
{
	return DoComplexCalculation(x, y, lamda);
}

struct BinaryTestResult
{
	Real error_reduction = 0;
	BinaryTest test;
	Ptr<TrainingData> left = nullptr;
	Ptr<TrainingData> right = nullptr;
};

template<class HandleFunc>
BinaryTestResult ForeachRandomBinaryTest(
	int rand_seed, const Settings & sets, HandleFunc test_func, DTree::LearnStatus* status, int n_threads = 4)
{
	srand(rand_seed);
	const int len_vec = sets.GetVectorLength();
	const int n_tests = sets.n_test;
	// [0, 1, 2, ..., len_vec - 2, len_vec - 1]
	vector<int> rand_pos1 = math::RangeVector(0, len_vec);
	vector<int> rand_pos2 = rand_pos1;

	// [0, 1, 2, ..., 254, 255] for random r, which means each of a 8bit color
	// and then map into [0, 1]
	array<double, 255> rand_r;
	math::Range(rand_r.begin(), rand_r.end(), 0);
	for (auto& r : rand_r) r /= image::kScaleFactor;

	std::random_shuffle(rand_pos1.begin(), rand_pos1.end());
	std::random_shuffle(rand_pos2.begin(), rand_pos2.end());
	std::random_shuffle(rand_r.begin(), rand_r.end());

	int n_curr_tests = 0;
	BinaryTestResult res;

	auto buf_left = TrainingData::Create(sets);
	auto buf_right = TrainingData::Create(sets);


	for (int i = 0; i < rand_pos1.size(); ++i)
	{
		int p1 = rand_pos1[i];
		for (int p2 : rand_pos2)
		{
			for (double r : rand_r)
			{
				BinaryTest test;
				test.p1 = p1;
				test.p2 = p2;
				test.r = r;

				double error_reduction = test_func(
					test, buf_left.get(), buf_right.get());
				if (error_reduction != 0)
				{
					++n_curr_tests;
					if (status) status->n_curr_test = n_curr_tests;
					if (error_reduction > res.error_reduction)
					{
						res.test = test;
						res.error_reduction = error_reduction;
						res.left = buf_left;
						res.right = buf_right;
					}
				}
				if (n_curr_tests >= n_tests) break;
			}
			if (n_curr_tests >= n_tests) break;
		}
		if (n_curr_tests >= n_tests) break;
	}

	return res;
}

BinaryTestResult GenerateTestWithMaxErrorReduction(
	Real err, const TrainingData & samples, int seed, DTree::LearnStatus* status)
{
	auto func = [&samples, err](const BinaryTest & test,
		TrainingData* buf_left, TrainingData* buf_right)
	{
		size_t n_left = samples.GetLeftPatchesNumber(test);
		size_t n_right = samples.Num() - n_left;
		bool success = IsNumAvailable(n_left, n_right, samples.settings.k);
		BinaryTestResult res;
		if (success)
		{
			samples.Split(test, buf_left, buf_right);

			// do some very complex calculation
			const auto& left_x = buf_left->MatX();
			const auto& left_y = buf_left->MatY();
			const auto& right_x = buf_right->MatX();
			const auto& right_y = buf_right->MatY();

			//MyLogger::debug << "start doing complex calculation..." << endl;
			CalculationResult res_left = CalculateModelAndFittingError(left_x, left_y, samples.settings.lamda);
			CalculationResult res_right = CalculateModelAndFittingError(right_x, right_y, samples.settings.lamda);

			// get error reduction and compare to find the max error reduction
			Real error_reduction = GetErrorReduction(err,
				res_left.num_samples, res_left.fitting_error,
				res_right.num_samples, res_right.fitting_error);

			return error_reduction;
		} // end of if success
		return Real(0);
	};

	BinaryTestResult res = ForeachRandomBinaryTest(
		seed, samples.settings, func, status);
	return res;
}

// -------------------------------------------------------------------------------
//				Implementation of DTree and HDTrees' learning process
// -------------------------------------------------------------------------------

void DTree::CreateRoot()
{
    root = std::make_unique<DTNode>();
}

void DTree::Learn(
    const ImageReader& low_reader, const ImageReader& high_reader)
{
    if (low_reader.Empty()) return;

    Ptr<TrainingData> total_samples = TrainingData::Create(settings);

	const size_t n_imgs = low_reader.Size();

	vector<Mat> lows(low_reader.Size());
	for (int i = 0; i < n_imgs; ++i)
		lows[i] = low_reader.Get(i);

	vector<Mat> highs(high_reader.Size());
	for (int i = 0; i < n_imgs; ++i)
		highs[i] = high_reader.Get(i);

    total_samples->SetImages(lows, highs);

    Learn(total_samples);
}

void DTree::Learn(
    const Ptr<TrainingData> & total_samples)
{
	auto now = std::chrono::system_clock::now();
	srand(std::chrono::system_clock::to_time_t(now));

    if (total_samples->Num() == 0) return;
    // recording status

    root = UPtr<DTNode>(new DTNode(total_samples));
    // iterate all unprocessed node, starting from the first
    queue<DTNode*> unprocessed;
    unprocessed.push(root.get());

    learn_stat = LearnStatus();

    while (!unprocessed.empty())
    {
        DTNode* node = unprocessed.front();
        unprocessed.pop();

        const size_t n_samples = node->GetSamples()->Num();

        learn_stat.n_samples = n_samples;

        // calculate regression model for this node

        CalculationResult node_calc_res = CalculateModelAndFittingError(
            node->GetSamples()->MatX(),
            node->GetSamples()->MatY(),
            settings.lamda);

        if (n_samples < 2 * settings.min_n_patches)
        {
            node->BecomeLeafNode(node_calc_res.model);
            learn_stat.n_leaf += 1;
        }
        else
        {
            BinaryTestResult bin_res =
                GenerateTestWithMaxErrorReduction(node_calc_res.fitting_error, *node->GetSamples(), rand(), &learn_stat);
            if (bin_res.error_reduction > 0)
            {
                node->BecomeNonLeafNode(bin_res.left, bin_res.right, bin_res.test);
                unprocessed.push(node->GetLeft());
                unprocessed.push(node->GetRight());

                learn_stat.n_nonleaf += 1;
            }
            else
            {
                // this node is a leaf node
                node->BecomeLeafNode(node_calc_res.model);

                learn_stat.n_leaf += 1;
            }
        }
		node->GetSamples()->Clear();
        node = nullptr;
    }
}

void HDTrees::Learn(
    const ImageReader& low_reader, const ImageReader& high_reader)
{
	assert(low_reader);
	assert(high_reader);
    assert(low_reader.Size() == high_reader.Size());
    assert(!low_reader.Empty());

    using std::endl;

    trees.clear();
    trees.reserve(settings.layers);

    __int64 n_imgs = low_reader.Size();
    __int64 n_per_layer = n_imgs / settings.layers;

    vector<Mat> buf_low;
    vector<Mat> buf_high;
    buf_low.reserve(n_per_layer * 2);
    buf_high.reserve(n_per_layer * 2);

    for (int layer = 0; layer < settings.layers; ++layer)
    {
        stat_learn.layer = layer;

        int start = layer * n_per_layer;
        int end = start + n_per_layer;
        if (layer == settings.layers - 1) end = n_imgs;
        buf_low.clear();
        buf_high.clear();

        for (__int64 img_ind = start; img_ind < end; ++img_ind)
        {
            Mat low = low_reader.Get(img_ind);
            Mat high = high_reader.Get(img_ind);

			low = PredictImage(low, high.size());

            buf_low.push_back(low);
            buf_high.push_back(high);
        }

        Ptr<MemIR> low_imgs = MemIR::Create();
        low_imgs->images = buf_low;
        Ptr<MemIR> high_imgs = MemIR::Create();
        high_imgs->images = buf_high;

        trees.push_back(DTree(settings));

        stat_learn.tree = &trees.back().GetLearnStatus();
        trees.back().Learn(*low_imgs, *high_imgs);
        stat_learn.tree = nullptr;
    }
}