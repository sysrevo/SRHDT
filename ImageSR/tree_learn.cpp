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
    const ImageReader& low_reader, const ImageReader& high_reader)
{
    if (low_reader.Empty()) return;

    Ptr<TrainingData> total_samples = TrainingData::Create(settings);
    total_samples->PushBackImages(low_reader, high_reader);

    Learn(total_samples);
}

void DTree::Learn(
    const Ptr<TrainingData> & total_samples)
{
    using std::endl;

    if (total_samples->Num() == 0) return;
    // recording status

    root = UPtr<DTNode>(new DTNode(total_samples));
    // iterate all unprocessed node, starting from the first
    queue<DTNode*> unprocessed;
    unprocessed.push(root.get());

    learn_stat = LearnStatus();

    while (!unprocessed.empty())
    {
        DTNode* node = unprocessed.front();
        unprocessed.pop();

        const size_t n_samples = node->GetSamples()->Num();

        learn_stat.n_samples = n_samples;

        // calculate regression model for this node

        CalculationResult node_calc_res = DoComplexCalculate(
            node->GetSamples()->RowMatX(),
            node->GetSamples()->RowMatY(),
            settings.lamda);

        if (n_samples < 2 * settings.min_n_patches)
        {
            node->BecomeLeafNode(node_calc_res.c);
            learn_stat.n_leaf += 1;
        }
        else
        {
            BinaryTestResult bin_res =
                GenerateTestWithMaxErrorReduction(node_calc_res.fitting_error, *node->GetSamples(), rand(), &learn_stat);
            if (bin_res.error_reduction > 0)
            {
                node->BecomeNonLeafNode(bin_res.left, bin_res.right, bin_res.test);
                unprocessed.push(node->GetLeft());
                unprocessed.push(node->GetRight());

                learn_stat.n_nonleaf += 1;
            }
            else
            {
                // this node is a leaf node
                node->BecomeLeafNode(node_calc_res.c);

                learn_stat.n_leaf += 1;
            }
        }
        node->GetSamples()->ClearAndRelease();
        node = nullptr;
    }
}

void HDTrees::Learn(
    const ImageReader& low_reader, const ImageReader& high_reader)
{
    assert(low_reader.Size() == high_reader.Size());
    assert(!low_reader.Empty());

    using std::endl;

    trees.clear();
    trees.reserve(settings.layers);

    __int64 n_imgs = low_reader.Size();
    __int64 n_per_layer = n_imgs / settings.layers;

    vector<Mat> buf_low;
    vector<Mat> buf_high;
    buf_low.reserve(n_per_layer * 2);
    buf_high.reserve(n_per_layer * 2);

    for (int layer = 0; layer < settings.layers; ++layer)
    {
        stat_learn.layer = layer;

        int start = layer * n_per_layer;
        int end = start + n_per_layer;
        if (layer == settings.layers - 1) end = n_imgs;
        buf_low.clear();
        buf_high.clear();

        for (__int64 img_ind = start; img_ind < end; ++img_ind)
        {
            Mat low = low_reader.Get(img_ind);
            Mat high = high_reader.Get(img_ind);

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

        stat_learn.tree = &trees.back().GetLearnStatus();
        trees.back().Learn(*low_imgs, *high_imgs);
        stat_learn.tree = nullptr;
    }
}