#pragma once
#include "image_reader.h"

namespace imgsr
{
    class ImgSR
    {
    public:
        virtual Mat PredictImage(
            const Mat& img, cv::Size size) const = 0;

        inline Mat PredictImageRatio(
            const Mat& img, double ratio) const
        {
            cv::Size size = img.size();
            size.width *= ratio;
            size.height *= ratio;
            return PredictImage(img, size);
        }
    };

    class LearningBasedImgSR : public ImgSR
    {
    public:
        virtual void Learn(
            const ImageReader& low_imgs,
            const ImageReader& high_imgs) = 0;
    };
}