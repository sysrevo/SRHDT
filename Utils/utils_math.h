#pragma once
#include "common.h"

namespace imgsr
{
namespace utils
{
namespace math
{
template<class T>
inline T Square(T x) { return x * x; }

inline double Clamp(double val, double min_val, double max_val)
{
	if (min_val > max_val) std::swap(min_val, max_val);
	return std::min(max_val, std::max(val, min_val));
}

inline bool EndsWith(const string & value, const string & pattern)
{
	if (pattern.size() > value.size()) return false;
	return std::equal(pattern.rbegin(), pattern.rend(), value.rbegin());
}

template<class Iterator>
void Range(Iterator beg, Iterator end, int start = 0)
{
	int count = 0;
	for (Iterator it = beg; it != end; ++it)
	{
		*it = start + count;
		++count;
	}
}

template<class T = int>
vector<T> RangeVector(int start, int size)
{
	vector<T> res(size);
	Range(res.begin(), res.end(), start);
	return res;
}

/// <summary>
/// Split an array of strings into n arrays whose sizes are input.size()/num. The size of input must be larger than num.The number of result arrays must be larger than 0.
/// If the input array's size % num != 0, then the size of the last array is larger than the others.
/// </summary>
/// <param name="src">The array to split. Must be larger than num.</param>
/// <param name="num">The number of result array. Must be larger than 0.</param>
/// <returns>Num arrays that come from src.</returns>
template<class T>
vector<vector<T>> SplitEqually(const vector<T>& input, int num)
{
	__int64 step = (__int64)input.size() / num;

	vector<vector<T>> res;
	res.reserve(num);

	if (num <= 0) return res;
	if (input.size() < num) return res;

	for (int i = 0; i < num; ++i)
	{
		if (i == num - 1)
		{
			res.push_back(vector<T>(
				input.begin() + i * step, input.end()));
		}
		else
		{
			res.push_back(vector<T>(
				input.begin() + i * step, input.begin() + (i + 1) * step));
		}
	}
	return res;
}

template<class T, class FilterFunc>
vector<T> Select(const vector<T> & in, FilterFunc func)
{
	vector<T> res;
	std::copy_if(in.begin(), in.end(), back_inserter(res), func);
	return res;
}

inline int GetRotatedPos(int m, int len_vec, int pat_size)
{
	assert(math::Square(pat_size) == len_vec);
	return len_vec - pat_size * (m % pat_size + 1) + m / pat_size;
}

int GetRotatedPos(int m, int len_vec, int pat_size, int times);

struct VectorRotator
{
public:

	VectorRotator(int pat_size_);

	/// <summary>
	/// Look up table 
	/// </summary>
	/// <param name="m">position on the vector with correct length</param>
	/// <param name="rotate_times">rotate 90 degrees counterclockwise, can be positive and negative</param>
	/// <returns>
	/// the transformed position of the vector from the patch which is rotated 90 degrees 
	/// counter clockwise.
	/// </returns>
	inline int GetPos(int m, int rotate_times) const
	{
		if (rotate_times < 0) rotate_times = -rotate_times + 2;
		rotate_times %= 4;
		return table[m][rotate_times];
	}

	template<class InVecType, class OutVecType>
	void RotateVector(const InVecType & vec, int times, OutVecType* out)
	{
		auto & res = *out;
		int len = res.cols() == 1 ? res.rows() : res.cols();
		assert(len_vec == len);
		for (int i = 0; i < len_vec; ++i)
		{
			res[GetPos(i, times)] = vec[i];
		}
	}

	template<class InputVector>
	ERowVec RotateVector(const InputVector & vec, int times)
	{
		assert(vec.size() == len_vec);
		ERowVec out = ERowVec::Zero(len_vec);
		RotateVector(vec, times, &out);
		return out;
	}

	template<class InputType, class OutputType>
	void RotateModel(const InputType & in, int times, OutputType* out)
	{
		if (out == nullptr) return;
		assert(in.cols() == in.rows());
		auto & res = *out;
		for (int c = 0; c < len_vec; ++c)
		{
			int fc = GetPos(c, times);
			for (int r = 0; r < len_vec; ++r)
			{
				int fr = GetPos(r, times);
				res(fr, fc) = in(r, c);
			}
		}
	}

	template<class InputType>
	inline EMat RotateModel(const InputType & in, int times)
	{
		InputType res = InputType::Zero(in.cols(), in.rows());
		RotateModel(in, times, &res);
		return res;
	}

private:
	int len_vec;
	int pat_size;
	vector<array<int, 4>> table;
};

} // math
} // utils
} // imgsr