#pragma once
#include "stdafx.h"

namespace imgsr
{
    struct BinaryTest
    {
        // two positions on the vectorized LR patch, which is patch.x
        int p1;
        int p2;
        double r; // threshold value

        template<class InputPatchType>
        bool IsOnLeft(const InputPatchType& patch) const
        {
            Real x1 = patch[p1];
            Real x2 = patch[p2];
            if (x1 < x2 + r) return false;
            return true;
        }

        BinaryTest() { memset(this, 0, sizeof(*this)); }
    };
}