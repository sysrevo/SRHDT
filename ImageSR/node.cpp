#include "stdafx.h"
#include "node.h"

using namespace imgsr;

DTNode::DTNode(Ptr<const TrainingData> samples)
{
    this->samples = samples;
}

void DTNode::BecomeLeafNode(const EMat & c)
{
    this->is_leaf = true;
    this->model = c;
}

void DTNode::BecomeNonLeafNode(Ptr<const TrainingData> left_samples, Ptr<const TrainingData> right_samples, const BinaryTest& test)
{
    this->is_leaf = false;
    this->left = UPtr<DTNode>(new DTNode(left_samples));
    this->right = UPtr<DTNode>(new DTNode(right_samples));
    this->test = test;
}

const DTNode * DTNode::ReachLeafNode(const ERowVec & x) const
{
    if (is_leaf) return this;

    // this node is non leaf node
    assert(left != nullptr);
    assert(right != nullptr);

    if (test.IsOnLeft(x))
    {
        return left->ReachLeafNode(x);
    }
    else
    {
        return right->ReachLeafNode(x);
    }
}

int DTNode::GetNumLeafNodes() const
{
    if (is_leaf)
        return 1;
    else
        return left->GetNumLeafNodes() + right->GetNumLeafNodes();
}

int DTNode::GetNumNodes() const
{
    if (is_leaf) 
        return 1;
    else 
        return 1 + left->GetNumNodes() + right->GetNumNodes();
}

void DTNode::CreateLeft(Ptr<const TrainingData> samples)
{
    left = nullptr;
    left = std::make_unique<DTNode>(samples);
}

void DTNode::CreateRight(Ptr<const TrainingData> samples)
{
    right = nullptr;
    right = std::make_unique<DTNode>(samples);
}

void DTNode::ClearSamples()
{
    samples = nullptr;
}
