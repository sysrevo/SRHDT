#include "image_viewer.h"

ImageViewer::ImageViewer(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

void ImageViewer::SetImage(const QPixmap & pixmap)
{
    ui.img->setPixmap(pixmap);
}

QPixmap cvMat2QImage(const cv::Mat & img)
{
    QImage::Format format;
    switch (img.type())
    {
    case CV_8UC3:
        format = QImage::Format_RGB888;
        break;

    case CV_8UC4:
        format = QImage::Format_RGBA8888;
        break;
    }
    // the default format of opencv is bgr
    // so convert must be made
    cv::Mat rgb_img;
    cv::cvtColor(img, rgb_img, cv::COLOR_BGR2RGB);
    return QPixmap::fromImage(QImage((unsigned char*)rgb_img.data, rgb_img.cols, rgb_img.rows, format));
}

void ImageViewer::SetImage(const cv::Mat & img)
{
    QPixmap pixmap = cvMat2QImage(img);
    SetImage(pixmap);
}
