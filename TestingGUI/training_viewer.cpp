
#include <qstring.h>
#include <qpixmap.h>
#include <qtimer.h>

#include "training_viewer.h"

#include "../ImageSR/tree.h"

using std::vector;
using std::string;
using namespace imgsr;

TrainingViewer::TrainingViewer(QWidget *parent)
{
    ui.setupUi(this);
}

void TrainingViewer::Train(const ImageReader& low, const ImageReader& high, HDTrees* trees)
{
    is_finish = false;

    thd_learn = std::make_shared<std::thread>([trees, &low, &high, this]()
    {
        status = &trees->GetLearnStatus();
        trees->Learn(low, high);

        is_finish = true;
    });

    thd_update = std::make_shared<std::thread>([this]()
    {
        while (!is_finish)
        {
            if(status) UpdateStatus(*status);
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    });

    thd_update->detach();
    thd_learn->detach();
}

void TrainingViewer::UpdateStatus(const imgsr::HDTrees::LearnStatus & status)
{
    ui.label_val_curr_layer->setText(QString::number(status.layer));
    auto tree_status = status.tree;
    if (tree_status)
    {
        ui.label_val_n_leaves->setText(QString::number(tree_status->n_leaf));
        ui.label_val_n_samples->setText(QString::number(tree_status->n_samples));
        ui.label_val_n_test->setText(QString::number(tree_status->n_curr_test));
    }
}

void TrainingViewer::closeEvent(QCloseEvent * event)
{
    if (thd_learn)
    {
        
    }
}
