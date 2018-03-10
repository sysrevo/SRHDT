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