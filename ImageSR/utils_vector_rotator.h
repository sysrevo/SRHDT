#pragma once
#include "utils.h"

namespace imgsr
{
    namespace utils
    {
        struct VectorRotator
        {
        public:

            VectorRotator(int pat_size_);

            inline int GetPos(int m, int rotate_times) const
            {
                if (rotate_times < 0) rotate_times = 0;
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
                        res(r, c) = in(fr, fc);
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
    }
}
