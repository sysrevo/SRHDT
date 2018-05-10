#include "stdafx.h"
#include "tree.h"
#include <chrono>
#include "training_data.h"
#include "image_reader.h"
#include "../Utils/utils.h"

#include "tree_serializer.h"

using namespace imgsr;
using namespace imgsr::utils;

// -------------------------------------------------------------------------------
//								Helper Functions
// -------------------------------------------------------------------------------

inline bool IsNumAvailable(size_t nleft, size_t nright, double k)
{
	__int64 tmp = (__int64)(std::max(nleft, nright) * k);
	__int64 tmp2 = (__int64)std::min(nleft, nright);
	return tmp <= tmp2;
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
	EMat yr = x * c;
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
		size_t n_left = samples.CountLeftPatches(test);
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

void DTree::Learn(Ptr<const ImageReader> lows, Ptr<const ImageReader> highs, LearnStatus* status)
{
    auto now = std::chrono::system_clock::now();
    srand(0);

    Ptr<TrainingData> total_samples = TrainingData::Create(settings);
    total_samples->SetImages(lows, highs);

    if (total_samples->Num() == 0) return;

    root = UPtr<DTNode>(new DTNode(total_samples));
    // iterate all unprocessed node, starting from the first
    queue<DTNode*> unprocessed;
    unprocessed.push(root.get());

    if(status) *status = LearnStatus();

    while (!unprocessed.empty())
    {
        DTNode* node = unprocessed.front();
        unprocessed.pop();

        const size_t n_samples = node->GetSamples()->Num();

        if (status) status->n_samples = n_samples;

        // calculate regression model for this node

        CalculationResult node_calc_res = CalculateModelAndFittingError(
            node->GetSamples()->MatX(),
            node->GetSamples()->MatY(),
            settings.lamda);

        BinaryTestResult bin_res;

        bool is_leaf = true;
        if (n_samples < 2 * settings.min_n_patches)
            is_leaf = true;
        else
        {
            bin_res =
                GenerateTestWithMaxErrorReduction(node_calc_res.fitting_error, *node->GetSamples(), rand(), status);
            if (bin_res.error_reduction > 0)
                is_leaf = false;
            else
                is_leaf = true;
        }

        if (is_leaf)
        {
            node->BecomeLeafNode(node_calc_res.model);
            if (status) status->n_leaf += 1;
        }
        else
        {
            node->BecomeNonLeafNode(
                std::move(bin_res.left),
                std::move(bin_res.right),
                bin_res.test);
            unprocessed.push(node->GetLeft());
            unprocessed.push(node->GetRight());
            if (status) status->n_nonleaf += 1;
        }
        node->ClearSamples();
    }
}

void HDTrees::Learn(Ptr<const ImageReader> low_reader, Ptr<const ImageReader> high_reader, LearnStatus* status)
{
    assert(low_reader->Size() == high_reader->Size());
    assert(!low_reader->Empty());

    // initialize trees 
    trees.clear();
    trees.reserve(settings.layers);
    for (int layer = 0; layer < settings.layers; layer += 1)
        trees.push_back(DTree(settings));

    size_t n_imgs = low_reader->Size();
    size_t n_per_layer = n_imgs / settings.layers;

    // buffer for low resolution images and high resolution images
    Ptr<MemIR> low_imgs = MemIR::Create();
    Ptr<MemIR> high_imgs = MemIR::Create();
    low_imgs->images.reserve(n_per_layer * 2);
    high_imgs->images.reserve(n_per_layer * 2);

    for (int layer = 0; layer < settings.layers; ++layer)
    {
        if (status) status->layer = layer;

        // calculate start and end location
        size_t start = layer * n_per_layer;
        size_t end = start + n_per_layer;
        if (layer == settings.layers - 1) end = n_imgs;

        // clean up the buffer for later usage
        low_imgs->images.clear();
        high_imgs->images.clear();

        for (size_t img_ind = start; img_ind < end; ++img_ind)
        {
            Mat low = low_reader->Get(img_ind);
            Mat high = high_reader->Get(img_ind);

            for (int prev_layer = 0; prev_layer < layer; prev_layer += 1)
            {
                low = trees[prev_layer].PredictImage(low, high.size());
            }

            low_imgs->images.push_back(low);
            high_imgs->images.push_back(high);
        }
        DTree::LearnStatus* tree_status = status ? &(status->tree_status) : nullptr;
        trees[layer].Learn(low_imgs, high_imgs, tree_status);
    }
}