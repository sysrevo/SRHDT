#include "stdafx.h"
#include "utils.h"

using namespace imgsr::utils;
using namespace imgsr;

vector<int> math::Range(int start, int end)
{
    vector<int> res(end - start);
    for (int i = 0; i < res.size(); ++i) res[i] = i + start;
    return res;
}