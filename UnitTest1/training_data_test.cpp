#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Utils/utils.h"
#include "../ImageSR/settings.h"
#include "../ImageSR/training_data.h"

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

		TEST_METHOD(GetLeftNumTest)
		{
			Settings settings;
			TrainingData data(settings);
		}
    };
}