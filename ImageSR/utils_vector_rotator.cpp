#include "stdafx.h"
#include "utils_vector_rotator.h"

using namespace imgsr;
using namespace imgsr::utils;

VectorRotator::VectorRotator(int pat_size_)
{
    pat_size = pat_size_;
    len_vec = math::Square(pat_size);

    table.resize(len_vec);
    for (int i = 0; i < table.size(); ++i)
    {
        auto & tmp_table = table[i];
        tmp_table[0] = i;
        for (int j = 1; j < 4; ++j)
        {
            tmp_table[j] = math::GetRotatedPos(tmp_table[j - 1], len_vec, pat_size);
        }
    }
}