#pragma once
#include "common.h"

namespace imgsr
{
    struct Settings
    {
    public:
        enum FuseModelOption
        {
            None = 0, Rotate
        };

        /// <summary>
        /// The size of each patch.
        /// The length of a vectorized patch is the square of patch size.
        /// Beware that this must be the same between tree and patches when processing.
        /// </summary>
        int patch_size = 6;

        /// <summary>
        /// The number of overlapped images.
        /// This must be less than patch_size to make sense.
        /// </summary>
        int overlap = 4;

        /// <summary>
        /// Threshold value for canny edge detector. Only patch that contains edge pixel will
        /// be predicted or learned. Mysterious stuff.
        /// </summary>
        double canny_edge_threshold = 20;

        /// <summary>
        /// The factor used in determining whether the split of a binary test is valid.
        /// </summary>
        double k = 0.7f;

        /// <summary>
        /// The parameter used in regulated linear regression. Mysterous stuff.
        /// </summary>
        double lamda = 0.01f;

        /// <summary>
        /// Known as Nmin. Minimal number of patches that a leaf node can have.
        /// Used to limit the complexity of tree.
        /// </summary>
        size_t min_n_patches = 1800;

        /// <summary>
        /// Knows as Q. Number of test generated to find the binary test with the max error reduction.
        /// Larger number will cause inefficiency.
        /// If larger than (patchSize)*(patchSize)*(patchSize)*(patchSize)*255, it will fail in math level.
        /// </summary>
        int n_test = 36;

        int layers = 4;

        FuseModelOption fuse_option = FuseModelOption::Rotate;

        inline int GetVectorLength() const
        {
            return patch_size * patch_size;
        }

        inline int GetRotateTimes() const
        {
            return fuse_option == Settings::FuseModelOption::Rotate ? 4 : 1;
        }

        inline bool operator==(const Settings & other) const
        {
            return memcmp(this, &other, sizeof(Settings)) == 0;
        }

        inline bool operator!=(const Settings & other) const
        {
            return !operator==(other);
        }
    };
}