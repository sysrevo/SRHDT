#include "testinggui.h"
#include "image_viewer.h"
#include "training_viewer.h"
#include "qt_file_image_reader.h"
#include "../ImageSR/tree.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TestingGUI w;
    w.show();

    TrainingViewer tv;
    tv.show();

    auto high = QFileImageReader::Create("D:\\test\\images\\training_images", 100);
    auto low = imgsr::HandlerImageReader::Create([](cv::Mat* img) 
    {
        cv::resize(*img, *img, img->size() / 2, 0, 0);
    });
    low->SetInput(high);
    
    imgsr::Settings settings;
    imgsr::HDTrees trees(settings);
    tv.Train(*low, *high, &trees);

    return a.exec();
}
