#pragma once

namespace imgsr
{
    struct DTreeTraingingStatus
    {
        size_t n_samples = 0;
        int n_nonleaf = 0;
        int n_leaf = 0;
        int n_curr_node = 0;
        int n_test_gen = 0;
    };

    struct HDTreesTrainingStatus
    {
        int curr_layer = 0;
        DTreeTraingingStatus tree;
    };

    struct DTreePredictStatus
    {
        size_t n_pats = 0;
        int n_curr_pats = 0;
    };

    struct HDTreesPredictStatus
    {
        int layer = 0;
        DTreePredictStatus tree_status;
    };
}