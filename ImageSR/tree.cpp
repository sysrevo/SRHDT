#include "stdafx.h"
#include "tree.h"

using namespace imgsr;

DTree::DTree(const Settings & settings_) :
    settings(settings_) {}

HDTrees::HDTrees(const Settings& settings_)
    :settings(settings_) {}

Ptr<HDTrees> HDTrees::Create(const Settings & settings)
{
    return std::make_shared<HDTrees>(settings);
}

size_t DTree::GetNumNodes() const
{
    return root ? root->GetNumNodes() : 0;
}

size_t DTree::GetNumLeafNodes() const
{
    return root ? root->GetNumLeafNodes() : 0;
}

size_t HDTrees::GetNumNodes() const
{
    size_t num = 0;
    for (const auto& tree : trees)
    {
        num += tree.GetNumNodes();
    }
    return num;
}

size_t HDTrees::GetNumLeafNodes() const
{
    size_t num = 0;
    for (const auto& tree : trees)
    {
        num += tree.GetNumLeafNodes();
    }
    return num;
}
