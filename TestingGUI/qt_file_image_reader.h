#pragma once
#include "../ImageSR/image_reader.h"
#include <opencv2/opencv.hpp>
#include <qstring.h>

class QFileImageReader : public imgsr::ImageReader
{
public:
    // 通过 ImageReader 继承
    virtual bool Empty() const override;
    virtual size_t Size() const override;
    virtual cv::Mat Read(int ind) const override;

    static imgsr::Ptr<QFileImageReader> Create(const QString& dir, int max_size = 0);

private:
    std::vector<std::string> paths;
};