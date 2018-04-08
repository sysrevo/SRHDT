#include "backend.h"

#include <thread>
#include <memory>
#include <chrono>

#include <QString>
#include <QDebug>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include "../ImageSR/tree.h"
#include "../ImageSR/image_reader.h"
#include "../ImageSR/tree_serializer.h"
#include "../Utils/utils_image.h"

using imgsr::HDTrees;
using imgsr::RapidJsonSerializer;
using imgsr::ImgReader;

using std::thread;
using std::unique_ptr;

struct BackEndData
{
    HDTrees trees;
    HDTrees::LearnStatus status;

    unique_ptr<thread> thd_observe;
    unique_ptr<thread> thd_calc;
};

BackEnd::BackEnd(QObject *parent) : QObject(parent)
{
    m_timer = new QTimer();
    m_timer->setInterval(333);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(UpdateData()));
}

BackEnd::~BackEnd()
{
    if (data) delete data;
    data = nullptr;
}

void BackEnd::Read(const QString& path)
{
    if (path.length() != 0)
    {
        RapidJsonSerializer json;
        json.Deserialize(path.toStdString(), &(data->trees));
    }
}

void BackEnd::Write(const QString& path)
{
    if (path.length() != 0)
    {
        RapidJsonSerializer json;
        json.Serialize(data->trees, path.toStdString());
    }
}

void BackEnd::Train(const QString& imgsDirPath)
{
    auto imgs = imgsr::FileIR::Create();
    qDebug() << "train " << imgsDirPath;

    using std::thread;
    running = true;
    data->thd_observe = unique_ptr<thread>(new thread([this]()
    {
        while(this->running)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            this->UpdateData();
        }
    }));

    data->thd_calc = unique_ptr<thread>(new thread([this]()
    {
    }));
}

int BackEnd::NumSamples() const
{
    return n_samples;
}

int BackEnd::NumNonLeaves() const
{
    return n_nonleaves;
}

int BackEnd::NumLeaves() const
{
    return n_leaves;
}

int BackEnd::NumNodes() const
{
    return n_nodes;
}

int BackEnd::Layer() const
{
    return layer;
}

void BackEnd::UpdateData()
{
    layer = data->status.layer;
    const auto& tree_status = data->status.tree_status;
    n_samples = tree_status.n_samples;
    n_nonleaves = tree_status.n_nonleaf;
    n_leaves = tree_status.n_leaf;
    n_nodes = n_nonleaves + n_leaves;

    //emit DataChanged;
}
