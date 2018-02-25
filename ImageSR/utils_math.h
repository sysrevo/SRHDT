#pragma once
#include "stdafx.h"

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

            // return range from [start, start+1, start+2, ... , end-2, end-1]
            vector<int> Range(int start, int end);

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

        } // Math
    }
}