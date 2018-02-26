#pragma once
#include "stdafx.h"
#include "tree.h"
#include "utils_math.h"

namespace imgsr
{
    namespace treelearnhelper
    {
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
            EMat c = ((xt * x + tmp_lamda).inverse()) * xt * y;
            return c;
        }

        // return the fitting error at node E
        template<class Input1, class Input2>
        inline Real GetFittingError(const Input1 & y, const Input2 & yr, size_t numSamples)
        {
            EMat delta = y - yr;
            auto tmp = delta.squaredNorm();
            return delta.squaredNorm() / numSamples;
        }

        // return the error reduction R at node j
        inline Real GetErrorReduction(Real ej, size_t nl, Real el, size_t nr, Real er)
        {
            auto nj = nl + nr;
            auto tmp_left = el * nl / nj;
            auto tmp_right = er * nr / nj;
            return ej - (tmp_left + tmp_right);
        }

        struct CalculationResult
        {
            EMat c;
            Real fitting_error;
            size_t num_samples;
        };

        template<class InputType>
        CalculationResult DoComplexCalculate(const InputType & x, const InputType & y, Real lamda)
        {
            assert(x.rows() == y.rows());

            CalculationResult res;
            res.num_samples = x.rows();
            res.c = GetRegressionModel(x, y, lamda);
            EMat yr = x * res.c;
            res.fitting_error = GetFittingError(y, yr, res.num_samples);
            return res;
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
            int rand_seed, const Settings & sets, HandleFunc test_func, DTreeTraingingStatus* status, int n_threads = 4)
        {
            srand(rand_seed);
            int len_vec = sets.GetVectorLength();
            int n_tests = sets.n_test;
            // [0, 1, 2, ..., len_vec - 2, len_vec - 1]
            vector<int> rand_pos = math::Range(0, len_vec);
            // [0, 1, 2, ..., 254, 255] for random r, which means each of a 8bit color

            array<double, 255> rand_r;
            {
                auto it = rand_r.begin();
                for (int i_r : math::Range(0, len_vec))
                {
                    *it = i_r;
                    *it /= image::kScaleFactor;
                    ++it;
                }
            }

            std::random_shuffle(rand_pos.begin(), rand_pos.end());
            std::random_shuffle(rand_r.begin(), rand_r.end());

            int n_test_gen = 0;
            BinaryTestResult res;

            vector<Ptr<TrainingData>> bufs_left(n_threads);
            vector<Ptr<TrainingData>> bufs_right(n_threads);
            for (auto & buf : bufs_left)
                buf = TrainingData::Create(sets);

            for (auto & buf : bufs_right)
                buf = TrainingData::Create(sets);

            #pragma omp parallel for num_threads(n_threads)
            for (int i = 0; i < rand_pos.size(); ++i)
            {
                int p1 = rand_pos[i];
                if (n_test_gen >= n_tests) break;
                for (int p2 : rand_pos)
                {
                    if (n_test_gen >= n_tests) break;
                    for (double r : rand_r)
                    {
                        if (n_test_gen >= n_tests) break;
                        BinaryTest test;
                        test.p1 = p1;
                        test.p2 = p2;
                        test.r = r;

                        int tid = omp_get_thread_num();
                        auto & buf_left = bufs_left[tid];
                        auto & buf_right = bufs_right[tid];

                        double error_reduction = test_func(
                            test, buf_left, buf_right);
                        if (error_reduction != 0)
                        {
                            #pragma omp critical
                            {
                                ++n_test_gen;
                                if (status) status->n_test_gen = n_test_gen;
                                if (error_reduction > res.error_reduction)
                                {
                                    res.test = test;
                                    res.error_reduction = error_reduction;
                                    res.left = buf_left;
                                    res.right = buf_right;
                                }
                            }
                        }
                    }
                }
            }

            return res;
        }

        BinaryTestResult GenerateTestWithMaxErrorReduction(
            Real err, const TrainingData & samples, int seed, DTreeTraingingStatus* status)
        {
            auto func = [&samples, err](const BinaryTest & test,
                const Ptr<TrainingData> & buf_left, const Ptr<TrainingData> & buf_right)
            {
                size_t n_left = samples.GetLeftPatchesNumber(test);
                size_t n_right = samples.Num() - n_left;
                bool success = IsNumAvailable(n_left, n_right, samples.settings.k);
                BinaryTestResult res;
                if (success)
                {
                    buf_left->Clear();
                    buf_right->Clear();
                    samples.Split(test, buf_left, buf_right);

                    // do some very complex calculation
                    auto left_x = buf_left->RowMatX();
                    auto left_y = buf_left->RowMatY();
                    auto right_x = buf_right->RowMatX();
                    auto right_y = buf_right->RowMatY();

                    //MyLogger::debug << "start doing complex calculation..." << endl;
                    CalculationResult res_left = DoComplexCalculate(left_x, left_y, samples.settings.lamda);
                    CalculationResult res_right = DoComplexCalculate(right_x, right_y, samples.settings.lamda);

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
    } // tree_learn_helper
}// imgsr