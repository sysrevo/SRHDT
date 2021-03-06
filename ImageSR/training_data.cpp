﻿#include "stdafx.h"
#include "training_data.h"
#include "../Utils/utils.h"

using namespace imgsr;
using namespace utils;

Ptr<TrainingData> TrainingData::Create(const Settings & sets)
{
    return make_shared<TrainingData>(sets);
}

TrainingData::TrainingData(const Settings & sets) :
    settings(sets), len_vec(sets.patch_size * sets.patch_size) {}

void TrainingData::Split(const BinaryTest & test,
    TrainingData* out_left, TrainingData* out_right) const
{
    // Number of things
    // Putting n_ in front of every number of something is just my style
    const size_t n_total = Num();

    // Store the result on a temp buffer for later usage
    vector<bool> on_left_res(n_total);
    for (int i = 0; i < n_total; ++i)
        on_left_res[i] = test.IsOnLeft(X(i));

    // Count the number of left and right samples
    // The number of left and right samples is needed to avoid dynamic allocation 
    // of memory when the generating results
    // And obviously, it is either left or right
    size_t n_left = 0;
    for (bool on_left : on_left_res)
        if (on_left) n_left += 1;
    size_t n_right = n_total - n_left;

    vector<int> left_indexes;
    left_indexes.reserve(n_left);

    vector<int> right_indexes;
    right_indexes.reserve(n_right);

    for (int i = 0; i < n_total; i += 1)
    {
        const bool on_left = on_left_res[i];
        if (on_left)
            left_indexes.push_back(i);
        else
            right_indexes.push_back(i);
    }

    // Clear things up in case of messing up
    // Resize the data to avoid dynamic allocation
    if (out_left)
    {
        out_left->Resize(n_left);
        #pragma omp parallel for
        for (int i = 0; i < n_left; i += 1)
        {
            out_left->X(i) = X(left_indexes[i]);
            out_left->Y(i) = Y(left_indexes[i]);
        }
    }

    if (out_right)
    {
        out_right->Resize(n_right);
        #pragma omp parallel for
        for (int i = 0; i < n_right; i += 1)
        {
            out_right->X(i) = X(right_indexes[i]);
            out_right->Y(i) = Y(right_indexes[i]);
        }
    }


    // Split the training data into left and right
    //int ind_left = 0;
    //int ind_right = 0;
    //for (auto i = 0; i < n_total; ++i)
    //{
    //    const bool on_left = on_left_res[i];
    //    if (on_left)
    //    {
    //        if (out_left)
    //        {
    //            out_left->X(ind_left) = X(i);
    //            out_left->Y(ind_left) = Y(i);
    //        }
    //        ++ind_left;
    //    }
    //    else
    //    {
    //        if (out_right)
    //        {
    //            out_right->X(ind_right) = X(i);
    //            out_right->Y(ind_right) = Y(i);
    //        }
    //        ++ind_right;
    //    }
    //}

    assert(!out_left || out_left->Num() == ind_left);
    assert(!out_right || out_right->Num() == ind_right);
    assert(ind_left + ind_right == Num());
}

size_t TrainingData::CountLeftPatches(const BinaryTest & test) const
{
    size_t n_left = 0;
    size_t n_total = Num();
    for (size_t i = 0; i < n_total; ++i)
    {
        n_left += test.IsOnLeft(X(i)) ? 1 : 0;
    }
    return n_left;
}

void TrainingData::Resize(size_t n_patches)
{
    data_x.resize(n_patches, len_vec);
    data_y.resize(n_patches, len_vec);
	//data_x = EMat::Zero(n_patches, len_vec);
	//data_y = EMat::Zero(n_patches, len_vec);
}

void TrainingData::SetImages(Ptr<const ImageReader> images, int factor)
{
    Ptr<ImageReader> lows = WrappedIR::Create([](Mat* img)
    {
        *img = utils::image::ResizeImage(*img, img->size() / 2);
    });
    SetImages(lows, images);
}

void TrainingData::SetImages(Ptr<const ImageReader> input_lows, Ptr<const ImageReader> input_highs)
{
	const size_t n_imgs = input_lows->Size();
	assert(input_highs->Size() == n_imgs);
	assert(input_lows->Size() == n_imgs);

	vector<vector<Mat>> patches_low_imgs(n_imgs);
	vector<vector<Mat>> patches_high_imgs(n_imgs);

	#pragma omp parallel for
	for (int i = 0; i < n_imgs; ++i)
	{
		Mat low = input_lows->Get(i);
		Mat high = input_highs->Get(i);

		high = image::ResizeImageToFitPatchIfNeeded(high, high.size(), settings.patch_size, settings.overlap);
		low = image::ResizeImageToFitPatchIfNeeded(low, high.size(), settings.patch_size, settings.overlap);

		low = image::SplitYCrcb(low).y;
		high = image::SplitYCrcb(high).y;

		Mat edge = image::GetEdgeMap(low, settings.canny_edge_threshold);

		low = image::MatUchar2Float(low);
		high = image::MatUchar2Float(high);

		auto patches = image::GetPatchesMulti(
			vector<Mat>({ low, high }), edge, settings.patch_size, settings.overlap);

		patches_low_imgs[i] = patches[0];
		patches_high_imgs[i] = patches[1];
	}

	__int64 n_patches = 0;
	vector<__int64> start_index_imgs(n_imgs);
	for (int i = 0; i < n_imgs; ++i)
	{
		const auto& patches = patches_low_imgs[i];
		start_index_imgs[i] = n_patches;
		n_patches += patches.size();
	}

	Resize(n_patches);

	#pragma omp parallel for
	for (int i = 0; i < n_imgs; ++i)
	{
		const vector<Mat>& patches_low = patches_low_imgs[i];
		const vector<Mat>& patches_high = patches_high_imgs[i];
		__int64 start_index = start_index_imgs[i];
		for (int j = 0; j < patches_low.size(); ++j)
		{
			const Mat& patch_low = patches_low[j];
			const Mat& patch_high = patches_high[j];

			image::VectorizePatch(patch_low, &X(j + start_index));
			image::VectorizePatch(patch_high, &Y(j + start_index));
		}
	}
}

void TrainingData::Clear()
{
	data_x = ERowMat::Zero(0, 0);
	data_y = ERowMat::Zero(0, 0);
}