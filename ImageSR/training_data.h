#pragma once
#include "common.h"
#include "image_reader.h"
#include "settings.h"
#include "binary_test.h"

namespace imgsr
{
    class TrainingData
    {
    public:
        static Ptr<TrainingData> Create(const Settings & sets);

        TrainingData(const Settings & sets);

        void Split(const BinaryTest& test, 
            TrainingData* left_out_ptr, TrainingData* right_out_ptr) const;

        inline void Split(const BinaryTest & test, 
            Ptr<TrainingData> left_out, Ptr<TrainingData> right_out) const
        {
            Split(test, left_out.get(), right_out.get());
        }

        size_t GetLeftPatchesNumber(const BinaryTest& test) const;

        void Resize(size_t num);
        void Clear();

	    void SetImages(const vector<Mat>& lows, const vector<Mat>& highs);

        inline size_t Num() const { return data_x.rows(); }

        inline int LengthPatchVector() const { return len_vec; }

	    inline auto X(size_t row) { return MatX().row(row); }
	    inline auto Y(size_t row) { return MatY().row(row); }

        inline auto X(size_t row) const { return MatX().row(row); }
	    inline auto Y(size_t row) const { return MatY().row(row); }

	    inline EMat& MatX() { return data_x; }
	    inline EMat& MatY() { return data_y; }

        inline const EMat& MatX() const { return data_x; }
	    inline const EMat& MatY() const { return data_y; }

        const Settings settings;
        const int len_vec;
    private:
	    EMat data_x;
	    EMat data_y;
    };
}