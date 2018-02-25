#include "stdafx.h"
#include "CppUnitTest.h"
#include "../ImageSR/utils_image.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace imgsr;
using namespace utils;

TEST_CLASS(ImageTest)
{
public:
    TEST_METHOD(TestForeachPatch)
    {
        Mat mat(cv::Size(8, 4), CV_8U, cv::Scalar(0));
        int count = 0;
        for (auto it = mat.begin<uchar>(); it != mat.end<uchar>(); ++it)
        {
            *it = count;
            ++count;
        }

        auto test = [&mat](const vector<vector<int>> & res, int patch_size, int overlap)
        {
            int count = 0;
            image::ForeachPatch(mat, patch_size, overlap, [&res, &count](const cv::Rect & rect, const Mat & patch)
            {
                if (count < res.size())
                {
                    int res_count = 0;
                    for (auto it = patch.begin<uchar>(); it != patch.end<uchar>(); ++it)
                    {
                        Assert::AreEqual(int(*it), res[count][res_count]);
                        ++res_count;
                    }
                    ++count;
                }
            });
        };

        std::vector<vector<int>>
            res = { { 0, 1, 8, 9 },{ 2, 3, 10, 11 },{ 4, 5, 12, 13 },{ 6, 7, 14, 15 } };
        test(res, 2, 0);

        res = { { 0, 1, 8, 9 },{ 1, 2, 9, 10 },{ 2, 3, 10, 11 },{ 3, 4, 11, 12 },{ 4, 5, 12, 13 },
        { 5, 6, 13, 14 },{ 6, 7, 14, 15 },{ 8, 9, 16, 17 },{ 9, 10, 17, 18 } };
        test(res, 2, 1);
    }

    TEST_METHOD(TestImage)
    {
        using namespace cv;
        Mat img = cv::imread("..\\..\\UnitTest1\\test_256_4.png", CV_LOAD_IMAGE_UNCHANGED);
        img = image::SplitYCrcb(img).y;
        Assert::AreEqual(img.type(), CV_8U);

        image::ForeachPatch(img, 6, 4, [&img](const Rect & rect, Mat & patch)
        {
            char buf[256] = { 0 };
            sprintf_s(buf, "Patch left_top(%d, %d) is ok.\n", rect.x, rect.y);
            Logger::WriteMessage(buf);
        });
    }

    TEST_METHOD(TestVectorizedPatch)
    {
        using namespace cv;
        Mat img = cv::imread("..\\..\\UnitTest1\\test_256_4.png", CV_LOAD_IMAGE_UNCHANGED);
        img = image::ResizeImage(img, img.size(), 6, 4);
        img = image::SplitYCrcb(img).y;
        img = image::GrayImage2FloatGrayMap(img);

        image::ForeachPatch(img, 6, 4, [&img](const Rect & rect, Mat & patch)
        {
            ERowVec vec = image::VectorizePatch(patch);
            ERowVec vec2 = ERowVec::Zero(patch.cols * patch.rows);
            image::VectorizePatch(patch, &vec2);

            ERowVec expected = ERowVec::Zero(patch.cols * patch.rows);
            {
                int count = 0;
                for (auto it = patch.begin<float>(); it != patch.end<float>(); ++it)
                {
                    expected[count] = *it;
                    ++count;
                }
            }
            Assert::IsTrue(expected == vec);
            Assert::IsTrue(expected == vec2);
        });
    }

    TEST_METHOD(TestUnvectorized)
    {
        using namespace cv;
        Mat img = cv::imread("..\\..\\UnitTest1\\test_256_4.png", CV_LOAD_IMAGE_UNCHANGED);
        img = image::ResizeImage(img, img.size(), 6, 4);
        img = image::SplitYCrcb(img).y;
        img = image::GrayImage2FloatGrayMap(img);

        image::ForeachPatch(img, 6, 4, [&img](const Rect & rect, Mat & patch)
        {
            ERowVec vec = image::VectorizePatch(patch);

            Mat expected(Size(patch.cols, patch.rows), CV_32F);
            image::DevectorizePatch(vec, &expected);
            expected = patch - expected;
            int count = cv::countNonZero(expected);
            Assert::AreEqual(count, 0);
        });
    }
};