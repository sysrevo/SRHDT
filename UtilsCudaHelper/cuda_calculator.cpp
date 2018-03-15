#include "cuda_calculator.h"
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <exception>

using namespace imgsr;
using namespace utils;

CudaMat::CudaMat()
{
}

CudaMat::CudaMat(const EMat& mat)
{
	CopyFrom(mat);
}

CudaMat::CudaMat(int rows, int cols)
{
	Create(rows, cols);
}

CudaMat::~CudaMat()
{
	Release();
}

void CudaMat::CopyFrom(const EMat& mat)
{
	Create(mat.rows(), mat.cols());
	__int64 size = rows * cols;

	auto status = 
		cublasSetVector(size, sizeof(Real), mat.data(), 1, ptr, 1);

	if (status != cudaSuccess)
		throw std::runtime_error("Cuda cublasSetVector failed!");
}

void CudaMat::CopyTo(EMat* mat_out) const
{
	auto& mat = *mat_out;
	assert(mat.rows() == rows);
	assert(mat.cols() == cols);

	__int64 size = rows * cols;
	cublasGetVector(size, sizeof(Real), ptr, 1, mat.data(), 1);
}

EMat CudaMat::GetMat() const
{
	EMat res = EMat::Zero(rows, cols);
	CopyTo(&res);
	return res;
}

void CudaMat::Create(int rows, int cols)
{
	Release();
	const int size = rows * cols * sizeof(Real);
	auto status = cudaMalloc((void **)&ptr, size);

	if (status != cudaSuccess)
		throw std::runtime_error("Cuda memory allocation failed!");

	this->rows = rows;
	this->cols = cols;
}

void CudaMat::Release()
{
	if (ptr)
	{
		auto status = cudaFree(ptr);
		ptr = nullptr;
	}
	rows = 0;
	cols = 0;
}

struct CublasHandle
{
	cublasHandle_t handle;

	CublasHandle()
	{
		cublasStatus_t status = cublasCreate(&handle);
		if (status != CUBLAS_STATUS_SUCCESS)
			throw std::runtime_error("Cublas handle initialization failed!");
	}

	~CublasHandle()
	{
		cublasStatus_t status = cublasDestroy(handle);
		if (status != CUBLAS_STATUS_SUCCESS)
		{
			// ignore it
		}
	}
};

string CudaCalculator::GetDeviceName()
{
	int device;
	cudaGetDevice(&device);
	struct cudaDeviceProp props;
	cudaGetDeviceProperties(&props, device);
	return string(props.name);
}

// c = a * b
void CudaCalculator::Mul(const CudaMat & a, const CudaMat & b, CudaMat* c_ptr)
{
	if (c_ptr == nullptr) return;
	CudaMat& c = *c_ptr;
	assert(a.cols == b.rows);
	
	if (c.IsEmpty())
	{
		c.Create(a.rows, b.cols);
	}
	assert(c.rows == a.rows);
	assert(c.cols == b.cols);

	CublasHandle handle;

	// c = alpha * op(a) * op(b) + beta * c
	double alpha = 1;
	double beta = 0;

	cublasDgemm(handle.handle, 
		CUBLAS_OP_N, CUBLAS_OP_N, // operation of matrix a and matrix b
		a.rows, b.cols, a.cols,
		&alpha, 
		a.ptr, a.rows, // matrix a and leading dimension 
		b.ptr, b.rows, // matrix b and leading dimension
		&beta, 
		c.ptr, c.rows); // matrix c and leading dimension
}

EMat CudaCalculator::Mul(const EMat & a, const EMat & b)
{
	CudaMat cuda_a(a);
	CudaMat cuda_b(b);
	CudaMat cuda_c(a.rows(), b.cols());
	Mul(cuda_a, cuda_b, &cuda_c);
	return cuda_c.GetMat();
}

void CudaCalculator::Sub(const CudaMat & a, const CudaMat & b, CudaMat * c_ptr)
{
	if (c_ptr == nullptr) return;
	assert(a.cols == b.cols);
	assert(a.rows == b.rows);

	CudaMat& c = *c_ptr;
	if (c.IsEmpty())
	{
		c.Create(a.rows, a.cols);
	}

	assert(a.rows == c.rows);
	assert(a.cols == c.cols);

	CublasHandle handle;
}
