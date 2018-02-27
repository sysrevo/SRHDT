#pragma once
#include "stdafx.h"

namespace imgsr
{
    namespace utils
    {
        namespace image
        {
            Mat ResizeImage(const Mat & img, cv::Size expected_size, int patch_size, int overlap);
            Mat GetEdgeMap(const Mat& img, double threshold);

            Mat GetLowResImage(const Mat & img, float ratio = 0.5f);

            struct YCrCbImage
            {
                Mat y, cr, cb;
            };
            YCrCbImage SplitYCrcb(const Mat & img);

            Mat Merge(const YCrCbImage & img);

            const int kFloatImageType = CV_64F;
            const int kGrayImageType = CV_8U;

            typedef double FloatMapValue;

            Mat GrayImage2FloatGrayMap(const Mat & gray_img);
            Mat FloatGrayMap2GrayImage(const Mat & f_gray);

            vector<Mat> GetPatches(const Mat & img, const Mat & edge, int pat_size, int overlap);

            vector<vector<Mat>> GetPatchesMulti(const vector<Mat> & imgs, const Mat & edge, int pat_size, int overlap);

            // ==================================================
            //                  Image Patch Vectorization
            // ==================================================
            const double kScaleFactor = 255;

            template<class VecType>
            void DevectorizePatch(const VecType & vec, Mat * pat_out)
            {
                if (pat_out == nullptr) return;
                int vec_pos = 0;
                Mat & img = *pat_out;

                for (int r = 0; r < img.rows; ++r)
                {
                    auto* p = img.ptr<FloatMapValue>(r);
                    for (int c = 0; c < img.cols; ++c)
                    {
                        p[c] = vec[vec_pos];
                        ++vec_pos;
                    }
                }
            }

            template<class VecType>
            void VectorizePatch(const Mat & img, VecType* vec_out)
            {
                if (vec_out == nullptr) return;
                assert(vec_out->size() == img.rows * img.cols);
                assert(img.type() == image::kFloatImageType);

                int vec_pos = 0;
                for (int r = 0; r < img.rows; ++r)
                {
                    auto* p = img.ptr<FloatMapValue>(r);
                    for (int c = 0; c < img.cols; ++c)
                    {
                        (*vec_out)[vec_pos] = p[c];
                        ++vec_pos;
                    }
                }
            }

            inline Mat DevectorizePatch(const ERowVec & vec, int patch_size)
            {
                Mat res(cv::Size(patch_size, patch_size), image::kFloatImageType);
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
    }
}