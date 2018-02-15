#include "stdafx.h"
#include "node.h"

using namespace imgsr;

void DTNode::BecomeLeafNode(const EMat & c)
{
    this->is_leaf = true;
    this->c = c;
}

void DTNode::BecomeNonLeafNode(UPtr<TrainingData> && left_samples, UPtr<TrainingData> && right_samples, const BinaryTest & test)
{
    this->is_leaf = false;
    this->left = UPtr<DTNode>(new DTNode(std::move(left_samples)));
    this->right = UPtr<DTNode>(new DTNode(std::move(right_samples)));
    this->test = test;
}

void DTNode::PrintBrief(std::ostream & os, int stack) const
{
    using namespace std;
    os << "|";
    for (int i = 0; i < stack; ++i) os << "-";
    if (is_leaf)
    {
        os << "[Leaf] c" << endl;
    }
    else
    {
        os << "[Nonleaf] test: p1: " << test.p1 << "; p2: " << test.p2 << "; r: " << test.r << endl;
        left->PrintBrief(os, stack + 1);
        right->PrintBrief(os, stack + 1);
    }
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
