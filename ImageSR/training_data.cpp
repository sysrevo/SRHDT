#include "stdafx.h"
#include "training_data.h"
#include "utils_math.h"
#include "utils_image.h"
#include "utils_logger.h"

using namespace imgsr;
using namespace utils;

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
    size_t n_right = n_total - n_left;;

    // Clear things up in case of messing up
    // Resize the data to avoid dynamic allocation
    if (out_left)
        out_left->Resize(n_left);

    if (out_right) 
        out_right->Resize(n_right); 

    // Split the training data into left and right
    int ind_left = 0;
    int ind_right = 0;
    for (auto i = 0; i < n_total; ++i)
    {
        const bool on_left = on_left_res[i];
        if (on_left)
        {
            if (out_left)
            {
                out_left->X(ind_left) = X(i);
                out_left->Y(ind_left) = Y(i);
            }
            ++ind_left;
        }
        else
        {
            if (out_right)
            {
                out_right->X(ind_right) = X(i);
                out_right->Y(ind_right) = Y(i);
            }
            ++ind_right;
        }
    }

    assert(!out_left || out_left->Num() == ind_left);
    assert(!out_right || out_right->Num() == ind_right);
    assert(ind_left + ind_right == Num());
}

size_t TrainingData::GetLeftPatchesNumber(const BinaryTest & test) const
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
    data_x.resize(n_patches * len_vec);
    data_y.resize(n_patches * len_vec);
}

void TrainingData::Reserve(size_t n_patches)
{
    data_x.reserve(n_patches * len_vec);
    data_y.reserve(n_patches * len_vec);
}

void TrainingData::PushBackPatch(const Mat & pat_low, const Mat & pat_high)
{
    assert(pat_low.size() == pat_high.size());
    assert(pat_low.type() == image::kFloatImageType && pat_high.type() == image::kFloatImageType);
    assert(pat_low.cols * pat_low.rows == len_vec);
    size_t n_curr = Num();
    if (data_x.capacity() == data_x.size())
    {
        Reserve(n_curr * 3 / 2);
    }
    Resize(n_curr + 1);
    image::VectorizePatch(pat_low, &X(n_curr));
    image::VectorizePatch(pat_high, &Y(n_curr));
}

void TrainingData::PushBackImage(const Mat & in_low, const Mat & in_high)
{
    assert(!in_low.empty() && !in_high.empty());

    if (in_low.empty() || in_high.empty()) return;

    // resize the image to be able to extract overlapped patches
    Mat high = image::ResizeImage(in_high, in_high.size(), settings.patch_size, settings.overlap);
    Mat low = image::ResizeImage(in_low, high.size(), settings.patch_size, settings.overlap);

    // Now extract the luminance value as the intensity value
    // for the following process
    // Now the type of high and low should be CV_8U of range [0, 255]
    high = image::SplitYCrcb(high).y;
    low = image::SplitYCrcb(low).y;

    assert(high.type() == CV_8U);
    assert(low.type() == high.type());
    assert(low.size() == high.size());

    // Now extract the edge map for edge patch identification 
    // We only need overlapped patches with edge pixel(s)
    // since bicubic(or any) interpolation is pretty good at non-edge area
    Mat edge = image::GetEdgeMap(high, settings.canny_edge_threshold);

    // Now convert the image to float map of range [0, 1], which is 
    // mapped from the original CV_8U map

    // Range [0, 1] is needed for EVERY processes involving Vectorization
    // If you use [0, 255], the following super complicated math things 
    // will produce a HUGE result value and the error caused by float(or double) is too high
    low = image::MatUchar2Float(low);
    high = image::MatUchar2Float(high);

    // Now extract patches and push them to training data
    auto pairs = image::GetPatchesMulti(vector<Mat>({ low, high }), edge, settings.patch_size, settings.overlap);
    auto& low_pats = pairs[0];
    auto& high_pats = pairs[1];
    size_t n_pats = low_pats.size();

    for (size_t i = 0; i < n_pats; ++i)
    {
        PushBackPatch(low_pats[i], high_pats[i]);
    }
}

void TrainingData::Append(const TrainingData & data)
{
    assert(data.len_vec == len_vec);
    assert(data.data_x.size() == data.data_y.size());
    assert(data.data_x.size() % len_vec == 0);
    assert(data.settings == settings);

    data_x.insert(data_x.end(), data.data_x.begin(), data.data_x.end());
    data_y.insert(data_y.end(), data.data_y.begin(), data.data_y.end());
}

void TrainingData::PushBackImages(const ImageReader& low_imgs_reader, const ImageReader& high_imgs_reader, int n_threads)
{
    assert(low_imgs_reader.Size() == high_imgs_reader.Size());
    if (low_imgs_reader.Empty()) return;
    if (n_threads < 1) n_threads = 1;

    size_t n_imgs = low_imgs_reader.Size();

    for (int i = 0; i < n_imgs; ++i)
    {
        PushBackImage(low_imgs_reader.Get(i), high_imgs_reader.Get(i));
    }
}

void TrainingData::ShrinkToFit()
{
    data_x.shrink_to_fit();
    data_y.shrink_to_fit();
}

void TrainingData::Clear()
{
    data_x.clear();
    data_y.clear();
}

void TrainingData::ClearAndRelease()
{
    vector<Real>().swap(data_x);
    vector<Real>().swap(data_y);
}