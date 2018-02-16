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

    int vec_len = pat_in.rows * pat_in.cols;
    EMat model_buf = EMat::Zero(vec_len, vec_len);

    int rotate_times = settings.fuse_option == Settings::FuseModelOption::Rotate ? 4 : 1;
    {
        Size size = Size(settings.patch_size, settings.patch_size);
        DoubleBuffers<Mat> pat_buf;
        pat_buf.front = Mat(size, pat_in.type());
        pat_buf.back = Mat(size, pat_in.type());

        ERowVec vec_buf = ERowVec(vec_len);

        for (int i = 0; i < rotate_times; ++i)
        {
            if (i == 0)
                pat_in.copyTo(pat_buf.front);
            else
            {
                cv::rotate(pat_buf.front, pat_buf.back, rotate_times);
            }
            image::VectorizePatch(pat_buf.front, &vec_buf);
            auto leaf = root->ReachLeafNode(vec_buf);
            model_buf += leaf->c;
        }
        model_buf /= rotate_times;
    }
    ERowVec x = image::VectorizePatch(pat_in);
    ERowVec res = x * model_buf;
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