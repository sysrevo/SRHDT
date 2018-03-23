#pragma once
#include "../Utils/utils_math.h"

namespace imgsr
{
namespace utils
{	
	class CudaMat
	{
	public:
		CudaMat();
		CudaMat(const EMat& mat);
		CudaMat(int rows, int cols);
		~CudaMat();

		void Create(int rows, int cols);
		void Release();

		void CopyFrom(const EMat& mat);
		void CopyTo(EMat* mat_out)const;
		EMat GetMat() const;

		inline bool IsEmpty() const { return ptr == nullptr; }

		Real* ptr = nullptr;
		__int64 rows = 0;
		__int64 cols = 0;
	};

	class CudaCalculator
	{
	public:
		static string GetDeviceName();
		static void Mul(const CudaMat& a, const CudaMat& b, CudaMat* c);
		static EMat Mul(const EMat& a, const EMat& b);

		static void Add(const CudaMat& a, const CudaMat& b, CudaMat* c, Real alpha = 1, Real beta = 1);
	};
} // utils
} // imgsr
