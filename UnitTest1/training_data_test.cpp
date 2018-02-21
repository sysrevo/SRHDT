#include "stdafx.h"
#include "CppUnitTest.h"

#include "../ImageSR/training_data.h"
#include "../ImageSR/utils_image.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace imgsr;
using namespace utils;

namespace UnitTest1
{
    TEST_CLASS(TrainingDataTest)
    {
    public:
        TEST_METHOD(TestInitSettings)
        {
            Settings settings;
            TrainingData data(settings);
            Assert::IsTrue(settings == data.settings);
        }

        TEST_METHOD(TestNumPairs)
        {
            Settings settings;
            settings.patch_size = 2;
            TrainingData data(settings);
            Assert::IsTrue(data.Num() == 0);

            Size size(settings.patch_size, settings.patch_size);
            Mat patch_x(size, CV_32F, cv::Scalar(1));
            Mat patch_y(size, CV_32F, cv::Scalar(1));

            int num = 4;
            for (int i = 0; i < num; ++i)
            {
                data.PushBackPatch(patch_x, patch_y);
            }
            Assert::AreEqual(data.Num(), size_t(num));
        }

        TEST_METHOD(TestPushPatch)
        {
            Settings settings;
            settings.patch_size = 2;
            TrainingData data(settings);

            Size size(settings.patch_size, settings.patch_size);
            Mat patch_x(size, CV_32F, cv::Scalar(1));
            Mat patch_y(size, CV_32F, cv::Scalar(1));

            data.PushBackPatch(patch_x, patch_y);

            EVec exp_vec_x = image::VectorizePatch(patch_x);
            EVec exp_vec_y = image::VectorizePatch(patch_y);

            EVec vec_x = data.X(0);
            EVec vec_y = data.Y(0);

            Assert::IsTrue(exp_vec_x == vec_x);
            Assert::IsTrue(exp_vec_y == vec_y);
            Assert::AreEqual(data.Num(), (size_t)1);
        }

        TEST_METHOD(TestPushData) {
            Settings settings;
            settings.patch_size = 2;
            TrainingData data(settings);
            TrainingData data2(settings);

            Size size(settings.patch_size, settings.patch_size);
            Mat patch_x(size, CV_32F, cv::Scalar(1));
            Mat patch_y(size, CV_32F, cv::Scalar(2));

            int n = 10;
            for (int i = 0; i < n; ++i)
            {
                data.PushBackPatch(patch_x, patch_x);
                data2.PushBackPatch(patch_y, patch_y);
            }

            TrainingData data3(settings);
            data3.Append(data);
            data3.Append(data2);
            for (int i = 0; i < n; ++i)
            {
                Assert::IsTrue(data3.X(i) == data.X(i));
                Assert::IsTrue(data3.X(i + n) == data2.X(i));
            }
        }

        TEST_METHOD(TestResize) {
            Settings settings;
            settings.patch_size = 2;
            TrainingData data(settings);

            Assert::AreEqual(data.Num(), size_t(0));

            size_t num = 10;
            data.Resize(num);

            Assert::AreEqual(data.Num(), size_t(num));

            for (size_t i = 0; i < num; ++i)
            {
                data.X(i) = data.Y(i);
            }
        }
    };
}