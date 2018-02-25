#include "stdafx.h"
#include "training_data.h"
#include "utils_math.h"
#include "utils_image.h"
#include "utils_logger.h"

using namespace imgsr;
using namespace utils;

void TrainingData::Split(const BinaryTest & test, TrainingData* left_ptr, TrainingData* right_ptr) const
{
    size_t n_total = Num();
    size_t n_left = GetLeftPatchesNumber(test);
    size_t n_right = n_total - n_left;

    if (left_ptr) left_ptr->Resize(n_left);
    if (right_ptr) right_ptr->Resize(n_right);

    int ind_left = 0;
    int ind_right = 0;
    for (auto i = 0; i < n_total; ++i)
    {
        if (test.IsOnLeft(X(i)))
        {
            if (left_ptr)
            {
                left_ptr->X(ind_left) = X(i);
                left_ptr->Y(ind_left) = Y(i);
            }
            ++ind_left;
        }
        else
        {
            if (right_ptr)
            {
                right_ptr->X(ind_right) = X(i);
                right_ptr->Y(ind_right) = Y(i);
            }
            ++ind_right;
        }
    }

    assert(!left_ptr || left_ptr->Num() == ind_left);
    assert(!right_ptr || right_ptr->Num() == ind_right);
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
    assert(pat_low.type() == CV_32F && pat_high.type() == CV_32F);
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

void TrainingData::HandlePreparedImage(const Mat & in_low, const Mat & in_high)
{
    assert(in_high.type() == CV_8U && in_low.type() == CV_8U);
    assert(in_low.size() == in_high.size());

    Mat edge = image::GetEdgeMap(in_low, settings.canny_edge_threshold);

    Mat low = image::GrayImage2FloatGrayMap(in_low);
    Mat high = image::GrayImage2FloatGrayMap(in_high);

    auto pairs = image::GetPatchPairs(low, high, edge, settings.patch_size, settings.overlap);
    size_t n_pairs = pairs.low.size();

    for (size_t i = 0; i < n_pairs; ++i)
    {
        PushBackPatch(pairs.low[i], pairs.high[i]);
    }

    /*image::ForeachPatch(low, settings.patch_size, settings.overlap,
        [&edge, &high, this](
            const cv::Rect& rect, const Mat& pat_low)
    {
        const Mat pat_edge = edge(rect);
        const Mat pat_high = high(rect);
        if (cv::countNonZero(pat_edge) > 0)
        {
            this->PushBackPatch(pat_low, pat_high);
        }
    });*/
}

void TrainingData::PushBackImage(const Mat & in_low, const Mat & in_high)
{
    assert(!in_low.empty() && !in_high.empty());

    if (in_low.empty() || in_high.empty()) return;

    Mat high = image::ResizeImage(in_high, in_high.size(), settings.patch_size, settings.overlap);
    Mat low = image::ResizeImage(in_low, high.size(), settings.patch_size, settings.overlap);

    if(high.type() == CV_8UC3)
        high = image::SplitYCrcb(high).y;

    if (low.type() == CV_8UC3)
        low = image::SplitYCrcb(low).y;

    HandlePreparedImage(low, high);
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

void TrainingData::Append(const vector<TrainingData>& data_to_append)
{
    size_t n_total = 0;
    for (const auto & data : data_to_append)
    {
        n_total += data.Num();
    }
    Reserve(n_total);

    for (const auto & data : data_to_append)
    {
        assert(data.settings == settings);
        Append(data);
    }
}

void TrainingData::PushBackImages(const Ptr<ImageReader> & low_imgs,
    const Ptr<ImageReader> & high_imgs, int n_threads)
{
    assert(low_imgs->Size() == high_imgs->Size());
    if (low_imgs->Empty()) return;
    if (n_threads < 1) n_threads = 1;

    size_t n_imgs = low_imgs->Size();

    for (int i = 0; i < n_imgs; ++i)
    {
        PushBackImage(low_imgs->Get(i), high_imgs->Get(i));
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