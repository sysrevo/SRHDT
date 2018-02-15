#include "stdafx.h"
#include "CppUnitTest.h"
#include "../ImageSR/utils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace imgsr;
using namespace imgsr::utils;

namespace UnitTest1 
{
    TEST_CLASS(MathTest)
    {
    public:
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
}