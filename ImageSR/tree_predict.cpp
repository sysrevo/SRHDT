#include "stdafx.h"
#include "tree.h"
#include "../Utils/utils.h"

using namespace imgsr;
using namespace imgsr::utils;
using utils::math::VectorRotator;

VectorRotator rotator(1);

Mat DTree::PredictImage(const Mat & in_img, Size size) const
{
    rotator = VectorRotator(settings.patch_size);
    using namespace cv;

    Mat img = image::ResizeImageToFitPatchIfNeeded(in_img, size, settings.patch_size, settings.overlap);

    image::YCrCbImage channels = image::SplitYCrcb(img);
    Mat edge = image::GetEdgeMap(channels.y, settings.canny_edge_threshold);

    Mat gray = image::MatUchar2Float(channels.y);

    Mat count_map = Mat(gray.size(), image::kFloatImageType, cv::Scalar(0));
    
    // get all patches with edge
    // 0 - patches for gray image
    // 1 - patches for output gray image
    // 2 - patches for count map
    auto patches = image::GetPatchesMulti(vector<Mat>({ gray, count_map }), edge, settings.patch_size, settings.overlap);
    
    vector<Mat>& patches_gray = patches[0];
    vector<Mat>& patches_count = patches[1]; 

    size_t n_pats = patches_gray.size();

    // init buffer for result
    vector<Mat> result_buf;
    result_buf.resize(n_pats);
    for (auto & pat : result_buf)
        pat = Mat(patches_gray[0].size(), patches_gray[0].type(), cv::Scalar(0));

    // calculate predicted result for each patch
    #pragma omp parallel for num_threads(4)
    for (int i = 0; i < n_pats; ++i)
    {
        result_buf[i] = PredictPatch(patches_gray[i]);
    }

    // try to merge result from predicted patches
    {
        // count map is for getting an average value 
        // from all possible predicted color
        for (auto & count_pat : patches_count)
            count_pat += 1;

        // clear the edge area in the output image
        // and add all result together
        for (auto & pat : patches_gray)
            pat = cv::Scalar(0);

        for (int i = 0; i < n_pats; ++i)
            patches_gray[i] += result_buf[i];
    }
    
    for (int r = 0; r < count_map.rows; ++r)
    {
        auto* ptr_count = count_map.ptr<image::FloatMapValue>(r);
        auto* ptr_gray = gray.ptr<image::FloatMapValue>(r);
        for (int c = 0; c < count_map.cols; ++c)
        {
            if (ptr_count[c] != 0)
                ptr_gray[c] /= ptr_count[c];
        }
    }

    gray = image::MatFloat2Uchar(gray);
	channels.y = gray;
    img = image::Merge(channels);
    return img;
}

Mat DTree::PredictPatch(const Mat & pat_in) const
{
    assert(pat_in.cols == settings.patch_size && pat_in.rows == pat_in.cols);

    int len_vec = pat_in.rows * pat_in.cols;
    int n_models = 0;

	ERowVec x = image::VectorizePatch(pat_in);
    EMat model = EMat::Zero(len_vec, len_vec);
    {
        Size size = Size(settings.patch_size, settings.patch_size);
        {
            auto leaf = root->ReachLeafNode(x);
            model += leaf->model;
            n_models += 1;
        }

        if (settings.fuse_option == Settings::Rotate)
        {
            const int n_rotates = 3;
            for (int i = 0; i < n_rotates; ++i)
            {
				ERowVec vec = rotator.RotateVector(x, i + 1);
                auto leaf = root->ReachLeafNode(vec);
                EMat tmp_model = rotator.RotateModel(leaf->model, i + 1);
                model += tmp_model;
            }
            n_models += n_rotates;
        }
    }
    ERowVec res = x * model / n_models;
    return image::DevectorizePatch(res, settings.patch_size);
}

Mat HDTrees::PredictImage(const Mat & img, Size size) const
{
    Mat out_img = img;
    for (int layer = 0; layer < trees.size(); ++layer)
    {
        out_img = trees[layer].PredictImage(out_img, size);
    }
    return out_img;
}