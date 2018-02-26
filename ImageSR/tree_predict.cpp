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
    
    // get all patches with edge
    // 0 - patches for gray image
    // 1 - patches for output gray image
    // 2 - patches for count map
    auto pats = image::GetPatches(vector<Mat>({ gray, gray_res, count_map }), edge, settings.patch_size, settings.overlap);
    size_t n_pats = pats[0].size();

    vector<Mat>& pats_in = pats[0];
    vector<Mat>& pats_out = pats[1];
    vector<Mat>& pats_count = pats[2];

    // init buffer for result
    vector<Mat> res_buf;
    res_buf.resize(n_pats);
    for (auto & pat : res_buf)
        pat = Mat(pats_in[0].size(), pats_in[0].type(), cv::Scalar(0));

    // calculate predicted result for each patch
    #pragma omp parallel for
    for (int i = 0; i < n_pats; ++i)
    {
        res_buf[i] = PredictPatch(pats_in[i]);
    }

    // try to merge result from predicted patches
    {
        // count map is for getting an average value 
        // from all possible predicted color
        for (auto & count_pat : pats_count)
            count_pat += 1;

        // clear the edge area in the output image
        // and add all result together
        for (auto & pat_out : pats_out)
            pat_out = cv::Scalar(0);

        for (int i = 0; i < n_pats; ++i)
        {
            pats_out[i] += res_buf[i];
        }
    }
    
    for (int r = 0; r < count_map.rows; ++r)
    {
        float* ptr_count = count_map.ptr<float>(r);
        float* ptr_gray = gray.ptr<float>(r);
        for (int c = 0; c < count_map.cols; ++c)
        {
            if (ptr_count[c] != 0)
                ptr_gray[c] /= ptr_count[c];
        }
    }

    imgs.y = image::FloatGrayMap2GrayImage(gray);
    Mat res = image::Merge(imgs);
    return res;
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

Mat HDTrees::PredictImage(const Mat & img, cv::Size size) const
{
    Mat out_img = img;
    for (const auto & tree : trees)
    {
        out_img = tree.PredictImage(out_img, size);
    }
    return out_img;
}