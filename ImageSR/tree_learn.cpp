#include "stdafx.h"
#include "tree.h"
#include "utils_image.h"
#include "utils_logger.h"
#include "training_data.h"
#include "tree_learn_helper.h"

using namespace imgsr;
using namespace imgsr::utils;
using namespace imgsr::treelearnhelper;

void DTree::CreateRoot()
{
    root = std::make_unique<DTNode>();
}

void DTree::Learn(
    const Ptr<ImageReader> & low_reader,
    const Ptr<ImageReader> & high_reader, 
    DTreeTraingingStatus* status)
{
    if (low_reader->Empty()) return;

    Ptr<TrainingData> total_samples = TrainingData::Create(settings);
    total_samples->PushBackImages(low_reader, high_reader);

    Learn(total_samples, status);
}

void DTree::Learn(
    const Ptr<TrainingData> & total_samples, 
    DTreeTraingingStatus* status)
{
    using std::endl;

    if (total_samples->Num() == 0) return;
    // recording status


    DTNode* node = new DTNode(total_samples);
    root.reset(node);
    // iterate all unprocessed node, starting from the first
    queue<DTNode*> unprocessed;
    unprocessed.push(node);

    if (status)
    {
        status->n_samples = total_samples->Num();
        status->n_leaf = 0;
        status->n_nonleaf = 0;
        status->n_curr_node = 0;
    }

    while (!unprocessed.empty())
    {
        DTNode* node = unprocessed.front();
        unprocessed.pop();

        // calculate regression model for this node

        CalculationResult node_calc_res = DoComplexCalculate(
            node->GetSamples()->RowMatX(),
            node->GetSamples()->RowMatY(),
            settings.lamda);

        size_t numPairs = node->GetSamples()->Num();

        if (numPairs < 2 * settings.min_n_patches)
        {
            node->BecomeLeafNode(node_calc_res.c);
            if(status)
                ++(status->n_leaf);
        }
        else
        {
            BinaryTestResult bin_res =
                GenerateTestWithMaxErrorReduction(node_calc_res.fitting_error, *node->GetSamples(), rand(), status);
            if (bin_res.error_reduction > 0)
            {
                node->BecomeNonLeafNode(bin_res.left, bin_res.right, bin_res.test);
                unprocessed.push(node->GetLeft());
                unprocessed.push(node->GetRight());

                if(status) ++(status->n_nonleaf);
            }
            else
            {
                // this node is a leaf node
                node->BecomeLeafNode(node_calc_res.c);

                if(status) ++(status->n_leaf);
            }
        }
        node->GetSamples()->ClearAndRelease();
        node = nullptr;
        if(status)  ++status->n_curr_node += 1;
    }
}

void HDTrees::Learn(
    const Ptr<ImageReader> & low_reader, 
    const Ptr<ImageReader> & high_reader, 
    HDTreesTrainingStatus* status)
{
    assert(low_reader->Size() == high_reader->Size());
    assert(!low_reader->Empty());

    using std::endl;

    trees.clear();
    trees.reserve(settings.layers);

    __int64 n_imgs = low_reader->Size();
    __int64 n_per_layer = n_imgs / settings.layers;

    vector<Mat> buf_low;
    vector<Mat> buf_high;
    buf_low.reserve(n_per_layer * 2);
    buf_high.reserve(n_per_layer * 2);

    for (int layer = 0; layer < settings.layers; ++layer)
    {
        if(status) status->curr_layer = layer;
        //MyLogger::debug << "Processing layer " << layer << " ..." << endl;

        int start = layer * n_per_layer;
        int end = start + n_per_layer;
        if (layer == settings.layers - 1) end = n_imgs;
        buf_low.clear();
        buf_high.clear();

        for (__int64 img_ind = start; img_ind < end; ++img_ind)
        {
            Mat low = low_reader->Get(img_ind);
            Mat high = high_reader->Get(img_ind);

            high = image::ResizeImage(high, high.size(), settings.patch_size, settings.overlap);
            low = image::ResizeImage(low, high.size(), settings.patch_size, settings.overlap);

            for (const auto & tree : trees)
            {
                low = tree.PredictImage(low, low.size());
            }

            buf_low.push_back(low);
            buf_high.push_back(high);
        }

        Ptr<MemoryImageReader> low_imgs = MemoryImageReader::Create();
        low_imgs->Set(buf_low);
        Ptr<MemoryImageReader> high_imgs = MemoryImageReader::Create();
        high_imgs->Set(buf_high);

        trees.push_back(DTree(settings));

        MyLogger::debug << "Layer " << layer << " training begins" << endl
            << "data size: " << low_imgs->Size() << endl;

        trees.back().Learn(low_imgs, high_imgs);

        MyLogger::debug << "Layer " << layer << " completed" << endl;
    }
}