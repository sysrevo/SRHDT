#pragma once

#include "binary_test.h"
#include "training_data.h"

namespace imgsr
{
    class DTNode
    {
    public:
        bool is_leaf;
        BinaryTest test;
        EMat c;

        inline DTNode(const Ptr<TrainingData> & samples_ptr = nullptr)
            : samples(samples_ptr) { }

        void BecomeLeafNode(const EMat & c);
        void BecomeNonLeafNode(const Ptr<TrainingData> & left_samples, const Ptr<TrainingData> & right_samples, const BinaryTest& test);

        const DTNode* ReachLeafNode(const ERowVec & x) const;
        void PrintBrief(std::ostream & os, int stack = 0) const;
        int GetNumLeafNodes() const;
        int GetNumNodes() const;

        inline DTNode* GetLeft() const { return left.get(); }
        inline DTNode* GetRight() const { return right.get(); }
        void CreateLeft(const Ptr<TrainingData> & samples = nullptr);
        void CreateRight(const Ptr<TrainingData> & samples = nullptr);
        void ClearSamples();
        TrainingData* GetSamples() const { return samples.get(); }
    private:
        Ptr<TrainingData> samples;
        UPtr<DTNode> left = nullptr;
        UPtr<DTNode> right = nullptr;
    };
}