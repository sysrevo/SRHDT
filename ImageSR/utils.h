#pragma once
#include "stdafx.h"
#include "utils_cuda_helper.h"

namespace imgsr
{
    namespace utils
    {
        namespace math
        {
            template<class T>
            inline int Square(T x) { return x * x; }

            inline double Clamp(double val, double min_val, double max_val)
            {
                if (min_val > max_val) std::swap(min_val, max_val);
                return std::min(max_val, std::max(val, min_val));
            }

            // using regularized linear regression
            // C = (XT * X + lamda)-1 * XT * Y
            template<class Input1, class Input2>
            inline EMat GetRegressionModel(const Input1 & x, const Input2 & y, double lamda)
            {
                EMat xt = x.transpose();
                EMat tmp_lamda = lamda * ERowMat::Identity(x.cols(), x.cols());
                EMat xt_x;
                EMat xt_y;
                xt_x = xt * x;
                xt_y = xt * y;
                EMat c = ((xt_x + tmp_lamda).inverse()) * xt_y;
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

            inline bool EndsWith(const string & value, const string & pattern)
            {
                if (pattern.size() > value.size()) return false;
                return std::equal(pattern.rbegin(), pattern.rend(), value.rbegin());
            }

            // return range from [start, start+1, start+2, ... , end-2, end-1]
            vector<int> Range(int start, int end);

            /// <summary>
            /// Split an array of strings into n arrays whose sizes are input.size()/num. The size of input must be larger than num.The number of result arrays must be larger than 0.
            /// If the input array's size % num != 0, then the size of the last array is larger than the others.
            /// </summary>
            /// <param name="src">The array to split. Must be larger than num.</param>
            /// <param name="num">The number of result array. Must be larger than 0.</param>
            /// <returns>Num arrays that come from src.</returns>
            template<class T>
            vector<vector<T>> SplitEqually(const vector<T>& input, int num)
            {
                __int64 step = (__int64)input.size() / num;

                vector<vector<T>> res;
                res.reserve(num);

                if (num <= 0) return res;
                if (input.size() < num) return res;

                for (int i = 0; i < num; ++i)
                {
                    if (i == num - 1)
                    {
                        res.push_back(vector<T>(
                            input.begin() + i * step, input.end()));
                    }
                    else
                    {
                        res.push_back(vector<T>(
                            input.begin() + i * step, input.begin() + (i + 1) * step));
                    }
                }
                return res;
            }

            template<class T, class FilterFunc>
            vector<T> Select(const vector<T> & in, FilterFunc func)
            {
                vector<T> res;
                std::copy_if(in.begin(), in.end(), back_inserter(res), func);
                return res;
            }
        } // Math

        namespace image
        {
            Mat ResizeImage(const Mat & img, cv::Size expected_size, int patch_size, int overlap);
            Mat GetEdgeMap(const Mat& img, double threshold);
            Mat GetLowResImage(const Mat & img, float ratio = 0.5f);
            Mat GetGrayImage(const Mat & img);
            Mat GrayImage2FloatGrayMap(const Mat & gray_img);
            Mat FloatGrayMap2GrayImage(const Mat & f_gray);
            // ==================================================
            //                  Image Patch Vectorization
            // ==================================================
            const double kScaleFactor = 255;

            template<class VecType>
            void DevectorizePatch(const VecType & vec, Mat * pat_out)
            {
                if (pat_out == nullptr) return;
                int counter = 0;
                Mat & img = *pat_out;
                for (auto it_pix = img.begin<float>(); it_pix != img.end<float>(); ++it_pix)
                {
                    *it_pix = vec[counter];
                    ++counter;
                }
            }

            template<class VecType>
            void VectorizePatch(const Mat & img, VecType* vec_out)
            {
                if (vec_out == nullptr) return;
                assert(vec_out->size() == img.rows * img.cols);
                assert(img.type() == CV_32F);

                int counter = 0;
                for (auto it_pix = img.begin<float>(); it_pix != img.end<float>(); ++it_pix)
                {
                    (*vec_out)[counter] = *it_pix;
                    ++counter;
                }
            }

            inline Mat DevectorizePatch(const ERowVec & vec, int patch_size)
            {
                Mat res(cv::Size(patch_size, patch_size), CV_32F);
                DevectorizePatch(vec, &res);
                return res;
            }

            inline ERowVec VectorizePatch(const Mat & img)
            {
                ERowVec res = ERowVec::Zero(img.cols * img.rows);
                VectorizePatch(img, &res);
                return res;
            }

            // ========================================================
            //                      Image Check
            // ========================================================

            double GetPSNR(const Mat & img1, const Mat & img2);

            cv::Scalar GetSSIM(const Mat & a, const Mat & b);

            // ========================================================
            //                      Patch Iteration
            // ========================================================

            /// <summary>
            /// For each patch in an image.
            /// </summary>
            /// <param name="img"></param>
            /// <param name="patch_size"></param>
            /// <param name="overlap"></param>
            /// <param name="func"></param>
            /// <param name="n_threads"></param>
            template<class PatchHandleFunc>
            void ForeachPatch(const Mat & img, int patch_size, int overlap, PatchHandleFunc func)
            {
                if (img.empty()) return;
                assert(patch_size > 0);
                int jump = patch_size - overlap;
                assert(jump > 0);

                for (int y = 0; y + patch_size <= img.rows; y += jump)
                {
                    assert(y + patch_size <= img.rows);
                    for (int x = 0; x + patch_size <= img.cols; x += jump)
                    {
                        assert(x + patch_size <= img.cols);
                        const cv::Rect rect(x, y, patch_size, patch_size);
                        func(rect, img(rect));
                    }
                }
            }

            template<class PatchHandleFunc>
            void ForeachPatchParallel(const Mat & img, int patch_size, int overlap, PatchHandleFunc func, int n_threads = 1)
            {
                if (img.empty()) return;
                assert(patch_size > 0);
                int jump = patch_size - overlap;
                assert(jump > 0);

                int y_limit = img.rows - patch_size;
                int x_limit = img.cols - patch_size;

                #pragma omp parallel for num_threads(n_threads)
                for (int y = 0; y <= y_limit; y += jump)
                {
                    int tid = omp_get_thread_num();
                    assert(y + patch_size <= img.rows);
                    for (int x = 0; x <= x_limit; x += jump)
                    {
                        assert(x + patch_size <= img.cols);
                        const cv::Rect rect(x, y, patch_size, patch_size);
                        func(rect, img(rect), tid);
                    }
                }
            }
        } // image

        template<class T>
        struct DoubleBuffers
        {
            T front;
            T back;

            void Swap()
            {
                std::swap(front, back);
            }
        };

        namespace filesys
        {
            vector<string> GetFilesInDir(const string & directory_path);
            __int64 GetFileSize(std::ifstream & ifs);
            vector<char> ReadFileBytes(std::ifstream & ifs);
            string ReadFileString(std::ifstream & ifs);
        } // filesys
    } // utils
} // imagesr