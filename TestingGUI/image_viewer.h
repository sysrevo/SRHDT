#pragma once
#include <QtWidgets/QWidget>
#include <qpixmap.h>
#include <opencv2/opencv.hpp>
#include "ui_image_viewer.h"

class ImageViewer : public QWidget
{
    Q_OBJECT

public:
    ImageViewer(QWidget *parent = Q_NULLPTR);
    void SetImage(const QPixmap & pixmap);
    void SetImage(const cv::Mat& img);
private:
    Ui::ImageViewerWidget ui;
};
