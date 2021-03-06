﻿#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Utils/utils.h"
#include "../UtilsCudaHelper/cuda_calculator.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace imgsr;
using namespace imgsr::utils;



TEST_CLASS(MathTest)
{
public:
    TEST_METHOD(TestSquare)
    {
        vector<int> int_tests = { 1, 2, 4, 5, 6, 0, -1, -10 };
        vector<float> float_tests = { 1.1f, 2.2f, 4.4f, 5, 6, 0, -1.1f, -10 };
        vector<double> double_tests = { 1.1, 2.2, 4.4, 5, 6, 0, -1.1, -10 };
        for (const auto & x : int_tests)
            Assert::AreEqual(math::Square(x), x * x);
        for (const auto & x : float_tests)
            Assert::AreEqual(math::Square(x), x * x);
        for (const auto & x : double_tests)
            Assert::AreEqual(math::Square(x), x * x);
    }

    TEST_METHOD(TestRange)
    {
        vector<std::pair<array<int, 2>, vector<int>>> tests =
        {
            { { 1, 0}, {}}, 
            { { 1, 2 },{ 1, 2 } },
            { { 0, 6 },{ 0, 1, 2, 3, 4, 5 } },
            { { 4, 3 },{ 4, 5, 6 } }
        };

        for (const auto & test : tests)
        {
            vector<int> res;
            res.resize(test.first[1]);
            math::Range(res.begin(), res.end(), test.first[0]);

            Assert::IsTrue(res == test.second);
        }
    }

    TEST_METHOD(TestRangeVector)
    {
        vector<std::pair<array<int, 2>, vector<int>>> tests = 
        {
            { { 1, 0 },{} },
            { { 1, 2 },{ 1, 2 } },
            { { 0, 6 },{ 0, 1, 2, 3, 4, 5 } },
            { { 4, 3 },{ 4, 5, 6 } }
        };
        for (const auto & test : tests)
        {
            auto res = math::RangeVector(test.first[0], test.first[1]);
            Assert::IsTrue(res == test.second);
        }
    }

    TEST_METHOD(TestSplitOdd)
    {
        vector<string> src = { "a", "b", "c", "d", "e" };
        vector<vector<string>> expected = { { "a" },{ "b" },{ "c" },{ "d", "e" } };
        const auto res = math::SplitEqually(src, 4);
        Logger::WriteMessage((GetString(res) + " =? " + GetString(expected)).c_str());
        Assert::IsTrue(res == expected, L"Split for odd failed");
    }

    TEST_METHOD(TestSplitEven)
    {
        vector<string> src = { "a", "b", "c", "d" };
        vector<vector<string>> expected = { { "a" },{ "b" },{ "c" },{ "d" } };
        const auto res = math::SplitEqually(src, 4);
        Logger::WriteMessage((GetString(res) + " =? " + GetString(expected)).c_str());
        Assert::IsTrue(res == expected, L"Split for odd failed");
    }

    TEST_METHOD(TestRotateVec)
    {
        Mat pat(cv::Size(6, 6), image::kFloatImageType);
        int count = 0;
        for (auto it = pat.begin<double>(); it != pat.end<double>(); ++it)
        {
            *it = count;
            ++count;
        }

        utils::math::VectorRotator rotator(6);

        ERowVec original_pat_vec = image::VectorizePatch(pat);
        for (int i = 0; i < 3; ++i)
        {
            // get rotated vector
            ERowVec vec = rotator.RotateVector(original_pat_vec, i + 1);

            // rotate pat and get vectorized patch
            cv::rotate(pat, pat, cv::ROTATE_90_COUNTERCLOCKWISE);

            Mat res_pat = image::DevectorizePatch(vec, 6);
            Assert::IsTrue(cv::countNonZero(res_pat - pat) == 0);
        }
    }

private:
    static string GetString(const vector<vector<string>> & strs)
    {
        string output = "";
        for (const auto & t : strs)
        {
            for (const auto & a : t)
            {
                output += a;
                output += ", ";
            }
            output += ";";
        }
        return output;
    }
};

TEST_CLASS(CudaTest)
{
	TEST_METHOD(TestCudaMul)
	{
		Logger::WriteMessage(CudaCalculator::GetDeviceName().c_str());

		EMat a = EMat::Random(512, 256);
		EMat b = EMat::Random(256, 128);

		EMat exp = a * b;
		CudaMat cuda_a(a);
		CudaMat cuda_b(b);
		CudaMat cuda_c;
		CudaCalculator::Mul(cuda_a, cuda_b, &cuda_c);
		EMat res = cuda_c.GetMat();

		Assert::IsTrue(exp.isApprox(res));
	}

	TEST_METHOD(TestCudaMul2)
	{
		Logger::WriteMessage(CudaCalculator::GetDeviceName().c_str());

		EMat a = EMat::Random(512, 256);
		EMat b = EMat::Random(256, 128);

		EMat exp = a * b;
		EMat res = CudaCalculator::Mul(a, b);

		Assert::IsTrue(exp.isApprox(res));
	}

	TEST_METHOD(TestCudaAdd)
	{
		vector<double> params = { 1, -1, 0, 2 };
		for (double alpha : params)
		{
			for (double beta : params)
			{
				EMat a = EMat::Random(512, 256);
				EMat b = EMat::Random(512, 256);

				CudaMat cuda_a(a);
				CudaMat cuda_b(b);
				CudaMat cuda_c;
				CudaCalculator::Add(cuda_a, cuda_b, &cuda_c, alpha, beta);
				EMat res = cuda_c.GetMat();

				EMat exp = alpha * a + beta * b;

				Assert::IsTrue(exp.isApprox(res));
			}
		}
	}
};