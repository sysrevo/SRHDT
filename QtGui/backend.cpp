#include "backend.h"

#include <thread>
#include <memory>
#include <chrono>
#include <string>
#include <vector>

#include <QString>
#include <QDebug>
#include <QTimer>
#include <QFile>

#include "../ImageSR/tree.h"
#include "../ImageSR/image_reader.h"
#include "../ImageSR/tree_serializer.h"
#include "../Utils/utils.h"

using imgsr::HDTrees;
using imgsr::RapidJsonSerializer;
using imgsr::ImageReader;

using std::thread;
using std::unique_ptr;
using std::vector;
using cv::Mat;

class MyPixmapProvider : public QQuickImageProvider
{
public:
    MyPixmapProvider()
        : QQuickImageProvider(QQuickImageProvider::Pixmap)
    {}

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
    {
        if (currImg.isNull())
        {
            QSize newSize = requestedSize;
            newSize.setWidth(requestedSize.width() < 0 ? nullImg.width() : requestedSize.width());
            newSize.setHeight(requestedSize.height() < 0 ? nullImg.height() : requestedSize.height());
            
            if (newSize.width() == 0) newSize.setWidth(100);
            if (newSize.height() == 0) newSize.setHeight(100);

            if (nullImg.size() != newSize)
            {
                nullImg = QPixmap(newSize);
                nullImg.fill(QColor(255, 255, 255));
            }
            if (size) *size = nullImg.size();
            return nullImg;
        }
        if (size) *size = currImg.size();
        return currImg;
    }

    void setImage(const Mat& img)
    {
        Mat rgbImg;
        cv::cvtColor(img, rgbImg, cv::COLOR_BGR2RGB);
        currImg = QPixmap::fromImage(QImage((uchar*)rgbImg.data, rgbImg.cols, rgbImg.rows, rgbImg.step, QImage::Format_RGB888));
    }
private:
    QPixmap nullImg;
    QPixmap currImg;
};

struct BackEndData
{
    HDTrees::LearnStatus status;

    HDTrees trees;
    unique_ptr<thread> thd_observe = nullptr;
    unique_ptr<thread> thd_calc = nullptr;
    imgsr::Settings settings;

    Mat curr_image;
    MyPixmapProvider provider;
};

BackEnd::BackEnd(QObject *parent) : QObject(parent)
{
    m_timer = new QTimer();
    m_timer->setInterval(333);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateData()));
    data = new BackEndData();
}

BackEnd::~BackEnd()
{
    if (data) delete data;
    data = nullptr;
}

inline std::string ToStdPath(const QString& url)
{
    std::string path = QString(url).replace("file:///", "").toStdString();
    return path;
}

inline Mat loadImage(const QString& url)
{
    QFile file(QString(url).replace("file:///", ""));
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray bytes = file.readAll();
        vector<uchar> data(bytes.data(), bytes.data() + bytes.count());
        return cv::imdecode(Mat(data, 1), CV_LOAD_IMAGE_COLOR);
    }
    return Mat();
}

inline void saveImage(const QString& url, const Mat& img)
{
    QFile file(QString(url).replace("file:///", ""));
    if (file.open(QIODevice::WriteOnly))
    {
        vector<uchar> data;
        cv::imencode(".png", img, data);
        QByteArray bytes((char*)data.data(), data.size());
        file.write(bytes);
    }
}

void BackEnd::read(const QString& file_url)
{
    std::string path = ToStdPath(file_url);
    if (path.length() != 0)
    {
        RapidJsonSerializer json;
        data->trees = HDTrees();
        json.ReadTrees(path, &(data->trees));
        emit dataRead();
    }
}

void BackEnd::write(const QString& file_url)
{
    std::string path = ToStdPath(file_url);
    if (path.length() != 0)
    {
        RapidJsonSerializer json;
        json.WriteTrees(data->trees, path);
        emit dataWritten();
    }
}

void BackEnd::train(const QString& imgs_dir_url, int patch_size, int overlap, 
    double k, double lamda, int min_patches, int n_tests)
{
    using std::thread;

    std::string dir_path = ToStdPath(imgs_dir_url);
    auto imgs_paths = imgsr::utils::filesys::GetFilesInDir(dir_path);
    auto imgs = imgsr::FileIR::Create();
    imgs->paths = imgs_paths;

    auto lows = imgsr::WrappedIR::Create([](cv::Mat* img)
    {
        auto size = img->size();
        *img = imgsr::utils::image::ResizeImage(*img, size / 2);
        *img = imgsr::utils::image::ResizeImage(*img, size);
    });
    lows->source = imgs;

    auto& settings = data->settings;
    settings.patch_size = patch_size;
    settings.overlap = overlap;
    settings.k = k;
    settings.lamda = lamda;
    settings.min_n_patches = min_patches;
    settings.n_test = n_tests;

    running = true;
    data->thd_observe = unique_ptr<thread>(new thread([this]()
    {
        while(this->running)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            this->updateData();
        }
    }));

    data->thd_calc = unique_ptr<thread>(new thread([this, imgs, lows]()
    {
        data->trees = HDTrees(data->settings);
        data->trees.Learn(lows, imgs, &(data->status));
        running = false;
        data->thd_observe->join();
        emit completed();
    }));
}

void BackEnd::predictBicubic(const QString & img_url)
{
    Mat img = loadImage(img_url);
    cv::Size size = img.size();
    img = imgsr::utils::image::ResizeImage(img, size / 2);
    img = imgsr::utils::image::ResizeImage(img, size);
    data->curr_image = img;
    data->provider.setImage(data->curr_image);
    emit imagePredicted();
}

void BackEnd::predict(const QString & img_url)
{
    Mat img = loadImage(img_url);
    cv::Size size = img.size();
    img = imgsr::utils::image::ResizeImage(img, size / 2);
    data->curr_image = data->trees.PredictImage(img, size);
    data->provider.setImage(data->curr_image);
    emit imagePredicted();
}

void BackEnd::savePredicted(const QString & img_url)
{
    if (data->curr_image.empty()) return;
    saveImage(img_url, data->curr_image);
    emit imageSaved();
}

int BackEnd::numSamples() const
{
    return n_samples;
}

int BackEnd::numNonLeaves() const
{
    return n_nonleaves;
}

int BackEnd::numLeaves() const
{
    return n_leaves;
}

int BackEnd::numNodes() const
{
    return n_nodes;
}

int BackEnd::layer() const
{
    return curr_layer;
}

int BackEnd::numTests() const 
{
    return (int)data->status.tree_status.n_curr_test;
}

float BackEnd::percentCurrentTest() const
{
    int n_test = 0;
    if (data) n_test = data->settings.n_test;
    if (n_test <= 0) return 0;
    float res = float(data->status.tree_status.n_curr_test) / n_test;
    return res;
}

QQuickImageProvider * BackEnd::getProvider() const
{
    return &(data->provider);
}

void BackEnd::updateData()
{
    curr_layer = data->status.layer;
    const auto& tree_status = data->status.tree_status;
    n_samples = (int)tree_status.n_samples;
    n_nonleaves = (int)tree_status.n_nonleaf;
    n_leaves = (int)tree_status.n_leaf;
    n_nodes = (int)n_nonleaves + n_leaves;

    emit dataChanged();
}
