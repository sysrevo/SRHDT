#pragma once
#include "stdafx.h"
#include "image_reader.h"
#include "settings.h"
#include "binary_test.h"

namespace imgsr
{
    class TrainingData
    {
    public:
        inline static Ptr<TrainingData> Create(const Settings & sets)
        {
            return make_shared<TrainingData>(sets);
        }

        inline TrainingData(const Settings & sets) :
            settings(sets), len_vec(sets.patch_size * sets.patch_size) {}

        void Split(const BinaryTest& test, TrainingData* left_out_ptr, TrainingData* right_out_ptr) const;
        inline void Split(const BinaryTest & test, Ptr<TrainingData> left_out, Ptr<TrainingData> right_out) const
        {
            Split(test, left_out.get(), right_out.get());
        }

        size_t GetLeftPatchesNumber(const BinaryTest& test) const;


        void Reserve(size_t patch_num);
        void Resize(size_t num);
        void ShrinkToFit();
        void Clear();
        void ClearAndRelease();
        void Append(const TrainingData & data);

        void PushBackPatch(const Mat & low_patch, const Mat & high_patch);
        void PushBackImage(const Mat & in_low, const Mat & in_high);
        void PushBackImages(const ImageReader& low_imgs_reader, const ImageReader& high_imgs_reader, int n_threads = 1);

        inline size_t Num() const {
            return data_x.size() / len_vec;
        }

        inline int LengthPatchVector() const {
            return len_vec;
        }

#define RETURN_VEC(name, i)\
	return EVec::Map(data_##name.data() + i * len_vec, len_vec);

        inline auto X(size_t i) { RETURN_VEC(x, i); }
        inline auto X(size_t i) const { RETURN_VEC(x, i); }

        inline auto Y(size_t i) { RETURN_VEC(y, i); }
        inline auto Y(size_t i) const { RETURN_VEC(y, i); }

        inline auto BackX() { assert(Num() > 0); return X(Num() - 1); }
        inline auto BackY() { assert(Num() > 0); return Y(Num() - 1); }

#undef RETURN_VEC

#define RETURN_MAT(name) return EMat::Map(data_##name.data(), len_vec, Num());

        inline auto MatX() { RETURN_MAT(x); }
        inline auto MatX() const { RETURN_MAT(x); }

        inline auto MatY() { RETURN_MAT(y); }
        inline auto MatY() const { RETURN_MAT(y); }

#undef RETURN_MAT

#define RETURN_ROW_MAT(name) return ERowMat::Map(data_##name.data(), Num(), len_vec);

        inline auto RowMatX() { RETURN_ROW_MAT(x); }
        inline auto RowMatX() const { RETURN_ROW_MAT(x); }

        inline auto RowMatY() { RETURN_ROW_MAT(y); }
        inline auto RowMatY() const { RETURN_ROW_MAT(y); }

#undef RETURN_ROW_MAT


        const Settings settings;
        const int len_vec;
    private:
        vector<Real> data_x;
        vector<Real> data_y;
    };
}