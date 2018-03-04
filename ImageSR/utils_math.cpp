#include "stdafx.h"
#include "utils_math.h"

using namespace imgsr::utils;
using namespace imgsr;

int math::GetRotatedPos(int m, int len_vec, int patch_size, int times)
{
    assert(len_vec == math::Square(patch_size));
    if (times < 0) times = 0;
    times = times % 4;
    for (int i = 0; i < times; ++i)
    {
        m = GetRotatedPos(m, len_vec, patch_size);
    }
    return m;
}
