#pragma once
#include "binary_test.h"
#include "training_data.h"

namespace imgsr
{
    class DTNode
    {
    public:
        DTNode(Ptr<const TrainingData> samples = nullptr);

        void BecomeLeafNode(const EMat & c);
        void BecomeNonLeafNode(Ptr<const TrainingData> left_samples, Ptr<const TrainingData> right_samples, const BinaryTest& test);

        const DTNode* ReachLeafNode(const ERowVec & x) const;

        int GetNumLeafNodes() const;
        int GetNumNodes() const;

        inline DTNode* GetLeft() const { return left.get(); }
        inline DTNode* GetRight() const { return right.get(); }
        void CreateLeft(Ptr<const TrainingData> samples = nullptr);
        void CreateRight(Ptr<const TrainingData> samples = nullptr);

        void ClearSamples();
        const TrainingData* GetSamples() const { return samples.get(); }

        bool is_leaf;
        BinaryTest test;
        EMat model;
    private:
        Ptr<const TrainingData> samples;
        UPtr<DTNode> left = nullptr;
        UPtr<DTNode> right = nullptr;
    };
}