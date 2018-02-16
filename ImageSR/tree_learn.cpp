#include "stdafx.h"
#include "tree.h"
#include "utils.h"
#include "utils_logger.h"
#include "training_data.h"
#include "tree_learn_helper.h"

using namespace imgsr;
using namespace imgsr::utils;
using namespace imgsr::treelearnhelper;

void DTree::Learn(
    const Ptr<ImageReader> & low_reader, 
    const Ptr<ImageReader> & high_reader)
{
    if (low_reader->Empty()) return;

    Ptr<TrainingData> total_samples = TrainingData::Create(settings);
    total_samples->PushBackImages(low_reader, high_reader);

    Learn(std::move(total_samples));
}

void DTree::Learn(const Ptr<TrainingData> & total_samples)
{
    using std::endl;
    if (total_samples->Num() == 0) return;
    MyLogger::debug << "Tree learning... num:" << total_samples->Num() << endl;

    DTNode* node = new DTNode(std::move(total_samples));
    root.reset(node);
    // iterate all unprocessed node, starting from the first
    queue<DTNode*> unprocessed;
    unprocessed.push(node);
    int count = 0;
    while (!unprocessed.empty())
    {
        MyLogger::debug << "Processing node " << count++ << " ..... ";
        DTNode* node = unprocessed.front();
        unprocessed.pop();

        // calculate regression model for this node

        CalculationResult res = DoComplexCalculate(
            node->samples->RowMatX(),
            node->samples->RowMatY(),
            settings.lamda);

        size_t numPairs = node->samples->Num();

        if (numPairs < 2 * settings.min_n_patches)
        {
            node->BecomeLeafNode(res.c);
        }
        else
        {
            MyLogger::debug << " generating tests... ";
            BinaryTestResult result =
                GenerateTestWithMaxErrorReduction(res.fitting_error, *node->samples, rand());
            if (result.error_reduction > 0)
            {
                Ptr<TrainingData> left = TrainingData::Create(settings);
                Ptr<TrainingData> right = TrainingData::Create(settings);
                node->samples->Split(result.test, left.get(), right.get());

                node->BecomeNonLeafNode(std::move(left), std::move(right), result.test);
                unprocessed.push(node->left.get());
                unprocessed.push(node->right.get());
            }
            else
            {
                node->BecomeLeafNode(res.c);
            }
        }
        node->samples->ClearAndRelease();
        node = nullptr;
        MyLogger::debug << " complete " << endl;
    }
}

void DTree::PrintBrief(std::ostream & os) const
{
    root->PrintBrief(os);
}

void HDTrees::Learn(const Ptr<ImageReader> & low_reader, const Ptr<ImageReader> & high_reader)
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
        MyLogger::debug << "Processing layer " << layer << " ..." << endl;

        int start = layer * n_per_layer;
        int end = start + n_per_layer;
        if (layer == settings.layers - 1) end = n_imgs;
        buf_low.clear();
        buf_high.clear();

        for (__int64 img_ind = start; img_ind < end; ++img_ind)
        {
            Mat low = low_reader->Get(img_ind);
            Mat high = high_reader->Get(img_ind);

            high = image::GetGrayImage(high);
            low = image::GetGrayImage(low);

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
        trees.back().Learn(low_imgs, high_imgs);

        MyLogger::debug << "Layer " << layer << " completed" << endl;
    }
}