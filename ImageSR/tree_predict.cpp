#include "stdafx.h"
#include "tree.h"
#include "utils.h"

using namespace imgsr;
using namespace imgsr::utils;

Mat DTree::PredictImage(Mat low, cv::Size expected_size) const
{
    using namespace cv;

    // the input image must be converted to 8bit gray image for edge extraction
    // then it is converted into 32bit gray image of float to perform calculation
    if (low.type() != CV_8U)
        low = image::GetGrayImage(low);

    low = image::ResizeImage(low, low.size(), settings.patch_size, settings.overlap);
    Mat edge = image::GetEdgeMap(low, settings.canny_edge_threshold);
    low = image::GrayImage2FloatGrayMap(low);
    
    int n_threads = 4;
    vector<Mat> tbuf_res(n_threads);
    vector<Mat> tbuf_count(n_threads);

    for (Mat & img : tbuf_res)
        img = Mat(low.size(), low.type(), Scalar(0));

    for (Mat & img : tbuf_count)
        img = Mat(low.size(), CV_8U, Scalar(0));

    int rotate_times = settings.GetRotateTimes();

    image::ForeachPatchParallel(low, settings.patch_size, settings.overlap,
    [&edge, &tbuf_res, &tbuf_count, this, rotate_times](const cv::Rect& rect, const Mat& pat_in, int tid)
    {
        Mat pat_res;
        Mat pat_edge = edge(rect);
        if (cv::countNonZero(pat_edge) > 0)
        {
            //predict this patch
            pat_res = this->PredictPatch(pat_in);
        }
        else
        {
            pat_res = Mat(pat_in);
        }
        tbuf_res[tid](rect) += pat_res;
        tbuf_count[tid](rect) += 1;
    }, n_threads);

    Mat result(low.size(), low.type(), Scalar(0));
    for (const Mat & img : tbuf_res)
        result += img;

    Mat count_map(result.size(), CV_8U, Scalar(0));
    for (const Mat & img : tbuf_count)
        count_map += img;

    // divide to get the average color of each pixel
    // some overlapped pixel will have 9 values
    count_map.convertTo(count_map, CV_32F);
    result /= count_map;

    result = image::FloatGrayMap2GrayImage(result);
    return result;
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
                vec = math::RotateVector(vec, settings.patch_size, i + 1);
                auto leaf = root->ReachLeafNode(vec);
                EMat tmp_model = math::RotateModel(leaf->c, settings.patch_size, i + 1);
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