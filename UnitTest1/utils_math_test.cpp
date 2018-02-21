#include "stdafx.h"
#include "CppUnitTest.h"
#include "../ImageSR/utils_image.h"
#include "../ImageSR/utils_vector_rotator.h"
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
            {{1, 2}, {1}}, 
            {{0, 6}, {0, 1, 2, 3, 4, 5}},
            {{4, 6}, {4, 5}}
        };
        for (const auto & test : tests)
        {
            auto res = math::Range(test.first[0], test.first[1]);
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
        Mat pat(cv::Size(6, 6), CV_32F);
        int count = 0;
        for (auto it = pat.begin<float>(); it != pat.end<float>(); ++it)
        {
            *it = count;
            ++count;
        }

        utils::VectorRotator rotator(6);

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