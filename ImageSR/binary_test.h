#pragma once
#include "common.h"

namespace imgsr
{
    /// <summary>
    /// Binary Test is stored in each non-leaf node and used to tell where
    /// the incomming vectorized patch should go.
    /// All the parameters within this class should be randomly generated.
    /// </summary>
    struct BinaryTest
    {
        // two positions on the vectorized patch with a range [0, patch_size]
        int p1 = 0;
        int p2 = 0;
        // threshold value with a range of [0, 1]
        double r = 0;

        template<class InputPatchType>
        /// <summary>
        /// Tell if the input patch should go to left
        /// </summary>
        /// <param name="patch">Vectorized patch, should be the size of settings.patch_size.</param>
        /// <returns></returns>
        bool IsOnLeft(const InputPatchType& patch) const
        {
            Real x1 = patch[p1];
            Real x2 = patch[p2];
            if (x1 < x2 + r) return false;
            return true;
        }
    };
}