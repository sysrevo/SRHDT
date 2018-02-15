#pragma once

#include "binary_test.h"
#include "training_data.h"

namespace imgsr
{
    class DTNode
    {
    public:
        bool is_leaf;
        UPtr<TrainingData> samples;
        UPtr<DTNode> left = nullptr;
        UPtr<DTNode> right = nullptr;
        BinaryTest test;
        EMat c;

        inline DTNode(UPtr<TrainingData> && samples_ptr = nullptr)
            : samples(std::move(samples_ptr)) { }

        void BecomeLeafNode(const EMat & c);
        void BecomeNonLeafNode(UPtr<TrainingData> && left_samples, UPtr<TrainingData> && right_samples, const BinaryTest& test);

        const DTNode* ReachLeafNode(const ERowVec & x) const;
        void PrintBrief(std::ostream & os, int stack = 0) const;
        int GetNumLeafNodes() const;
        int GetNumNodes() const;
    };
}