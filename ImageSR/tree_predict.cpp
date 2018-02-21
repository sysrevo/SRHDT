#include "stdafx.h"
#include "tree.h"
#include "utils_image.h"
#include "utils_vector_rotator.h"

using namespace imgsr;
using namespace imgsr::utils;

utils::VectorRotator rotator(1);

Mat DTree::PredictImage(const Mat & in_low, cv::Size size) const
{
    rotator = VectorRotator(settings.patch_size);
    using namespace cv;

    Mat low = image::ResizeImage(in_low, size, settings.patch_size, settings.overlap);

    image::YCrCbImage imgs = image::SplitYCrcb(low);
    Mat edge = image::GetEdgeMap(imgs.y, settings.canny_edge_threshold);

    Mat gray = image::GrayImage2FloatGrayMap(imgs.y);
    Mat gray_res = Mat(gray);

    Mat count_map = Mat(gray.size(), CV_32F, cv::Scalar(0));
    auto pats = image::GetPatches(vector<Mat>({ gray, gray_res, count_map }), edge, settings.patch_size, settings.overlap);
    {
        vector<Mat>& gray_pats = pats[0];
        vector<Mat>& res_pats = pats[1];
        vector<Mat>& count_pats = pats[2];
        vector<Mat> res_buf;

        size_t n_pats = gray_pats.size();
        res_buf.resize(n_pats);

        for (auto & pat : res_buf) 
            pat = Mat(gray_pats[0].size(), gray_pats[0].type(), cv::Scalar(0));

        #pragma omp parallel for
        for (int i = 0; i < n_pats; ++i)
        {
            res_buf[i] = PredictPatch(gray_pats[i]);
        }

        for (auto & count_pat : count_pats)
            count_pat += 1;

        for (auto & res_pat : res_pats)
            res_pat = cv::Scalar(0);

        for (int i = 0; i < n_pats; ++i)
        {
            res_pats[i] += res_buf[i];
        }
    }
    
    for (int r = 0; r < count_map.rows; ++r)
    {
        float* ptr = count_map.ptr<float>(r);
        for (int c = 0; c < count_map.cols; ++c)
            if (ptr[c] == 0) ptr[c] = 1;
    }

    gray /= count_map;

    imgs.y = image::FloatGrayMap2GrayImage(gray);
    Mat res = image::Merge(imgs);
    return res;
    
    //int n_threads = 4;
    //vector<Mat> tbuf_res(n_threads);
    //vector<Mat> tbuf_count(n_threads);

    //for (Mat & img : tbuf_res)
    //    img = Mat(low.size(), low.type(), Scalar(0));

    //for (Mat & img : tbuf_count)
    //    img = Mat(low.size(), CV_8U, Scalar(0));

    //rotator = VectorRotator(settings.patch_size);

    //image::ForeachPatchParallel(low, settings.patch_size, settings.overlap,
    //[&edge, &tbuf_res, &tbuf_count, this](const cv::Rect& rect, const Mat& pat_in, int tid)
    //{
    //    Mat pat_res;
    //    Mat pat_edge = edge(rect);
    //    if (cv::countNonZero(pat_edge) > 0)
    //    {
    //        //predict this patch
    //        pat_res = this->PredictPatch(pat_in);
    //    }
    //    else
    //    {
    //        pat_res = Mat(pat_in);
    //    }
    //    tbuf_res[tid](rect) += pat_res;
    //    tbuf_count[tid](rect) += 1;
    //}, n_threads);

    //Mat result(low.size(), low.type(), Scalar(0));
    //for (const Mat & img : tbuf_res)
    //    result += img;

    //Mat count_map(result.size(), CV_8U, Scalar(0));
    //for (const Mat & img : tbuf_count)
    //    count_map += img;

    //// divide to get the average color of each pixel
    //// some overlapped pixel will have 9 values
    //count_map.convertTo(count_map, CV_32F);
    //result /= count_map;

    //result = image::FloatGrayMap2GrayImage(result);
    //return result;
}

Mat DTree::PredictPatch(const Mat & pat_in) const
{
    assert(pat_in.cols == settings.patch_size && pat_in.rows == pat_in.cols);

    int len_vec = pat_in.rows * pat_in.cols;
    int n_models = 0;

    EMat model = EMat::Zero(len_vec, len_vec);
    {
        Size size = Size(settings.patch_size, settings.patch_size);
        ERowVec pat_vec = image::VectorizePatch(pat_in);
        {
            auto leaf = root->ReachLeafNode(pat_vec);
            model += leaf->c;
            n_models += 1;
        }

        if (settings.fuse_option == Settings::Rotate)
        {
            const int n_rotates = 3;
            ERowVec vec = pat_vec;
            for (int i = 0; i < n_rotates; ++i)
            {
                vec = rotator.RotateVector(vec, 1);
                auto leaf = root->ReachLeafNode(vec);
                EMat tmp_model = rotator.RotateModel(leaf->c, i + 1);
                model += tmp_model;
            }
            n_models += n_rotates;
        }
    }
    ERowVec x = image::VectorizePatch(pat_in);
    ERowVec res = x * model / n_models;
    return image::DevectorizePatch(res, settings.patch_size);;
}

Mat HDTrees::PredictImage(Mat img, cv::Size expected_size) const
{
    for (const auto & tree : trees)
    {
        Mat tmp;
        tmp = tree.PredictImage(img, expected_size);
        img = tmp;
    }

    return img;
}