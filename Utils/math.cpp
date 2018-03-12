#include "utils_math.h"

using namespace imgsr::utils;
using namespace imgsr;

int math::GetRotatedPos(int m, int len_vec, int patch_size, int times)
{
	assert(len_vec == math::Square(patch_size));
	if (times < 0) times = 0;
	times = times % 4;
	for (int i = 0; i < times; ++i)
	{
		m = GetRotatedPos(m, len_vec, patch_size);
	}
	return m;
}

math::VectorRotator::VectorRotator(int pat_size_)
{
	pat_size = pat_size_;
	len_vec = math::Square(pat_size);

	table.resize(len_vec);
	for (int i = 0; i < table.size(); ++i)
	{
		auto & tmp_table = table[i];
		tmp_table[0] = i;
		for (int j = 1; j < 4; ++j)
		{
			tmp_table[j] = math::GetRotatedPos(tmp_table[j - 1], len_vec, pat_size);
		}
	}
}