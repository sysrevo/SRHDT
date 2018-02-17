#include "stdafx.h"
#include "CppUnitTest.h"
#include <chrono>
#include "../ImageSR/settings.h"
#include "../ImageSR/binary_test.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace imgsr;

using Clock = std::chrono::high_resolution_clock;

namespace UnitTest1 {
    TEST_CLASS(BinaryTestTest) 
    {
    public:
        TEST_METHOD(TestBinaryTestIsOnLeft) 
        {
            BinaryTest test;

            EVec vec(6);
            vec << 1, 2, 3, 4, 5, 6;

            test.p1 = 0;
            test.p2 = 0;
            test.r = 0;
            Assert::AreEqual(!test.IsOnLeft(vec), vec[test.p1] < vec[test.p2] + test.r);

            test.p1 = 2;
            test.p2 = 2;
            test.r = 3;
            Assert::AreEqual(!test.IsOnLeft(vec), vec[test.p1] < vec[test.p2] + test.r);

            test.p1 = 5;
            test.p2 = 5;
            test.r = 0;
            Assert::AreEqual(!test.IsOnLeft(vec), vec[test.p1] < vec[test.p2] + test.r);
        }


        TEST_METHOD(TestSettingsEqual)
        {
            Settings a;
            Settings b = a;
            Assert::IsTrue(a == b);
            Assert::IsFalse(a != b);

            b.k += 1;
            Assert::IsTrue(a != b);
            Assert::IsFalse(a == b);

            b.k = a.k;
            b.lamda = a.lamda + 1;
            Assert::IsTrue(a != b);
            Assert::IsFalse(a == b);
        }

        TEST_METHOD(TestSettingsGetVectorLength)
        {
            Settings a;
            Assert::AreEqual(a.GetVectorLength(), a.patch_size * a.patch_size);
        }
    };
}