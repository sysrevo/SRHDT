#pragma once
#include <QtWidgets/QWidget>
#include <opencv2/opencv.hpp>
#include <thread>
#include <memory>

#include "ui_training_viewer.h"
#include "../ImageSR/image_reader.h"
#include "../ImageSR/tree.h"

class TrainingViewer : public QWidget
{
    Q_OBJECT

public:
    TrainingViewer(QWidget *parent = Q_NULLPTR);
    void Train(const imgsr::ImageReader& low, const imgsr::ImageReader& high, imgsr::HDTrees* trees);
private:
    void UpdateStatus(const imgsr::HDTrees::LearnStatus& status);
    void closeEvent(QCloseEvent* event) override;

    Ui::TrainingViewer ui;
    const imgsr::HDTrees::LearnStatus* status;
    bool is_finish = false;

    std::shared_ptr<std::thread> thd_update;
    std::shared_ptr<std::thread> thd_learn;
};
