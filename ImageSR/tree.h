#pragma once
#include "node.h"

namespace imgsr
{
    class DTree
    {
    public:
        struct LearnStatus
        {
            size_t n_samples = 0;
            int n_nonleaf = 0;
            int n_leaf = 0;
            int n_curr_test = 0;
        };

        inline DTNode* GetRoot() const { return root.get(); }

        void CreateRoot();

        DTree(const Settings & settings_);

        void Learn(Ptr<const ImgReader> lows, Ptr<const ImgReader> highs, LearnStatus* status = nullptr);

        /// <summary>
        /// Predict an image. Expected width and height must be at least the width and height of image 'low'.
        /// </summary>
        /// <param name="in_low">The input low resolution image. Please check if this image is not empty and only with type CV_8U first.</param>
        /// <param name="size">The width of high-res output image.</param>
        /// <returns>The high resolution image predicted by this decision tree</returns>
        Mat PredictImage(const Mat & in_low, cv::Size size) const;

        /// <summary>
        /// Predict an input patch. Please modified this->settings to match requirements before calling this function.
        /// Beware that the size of this two patch must be the same and match settings.patchSize.
        /// </summary>
        /// <param name="in_patch">Input patch for the prediction. Cols and rows must be the same as settings.patchSize.</param>
        Mat PredictPatch(const Mat & in_patch) const;

        inline int GetNumNodes() const
        {
            return root ? root->GetNumNodes() : 0;
        }

        inline int GetNumLeafNodes() const 
        {
            return root ? root->GetNumLeafNodes() : 0;
        }

        Settings settings;
    private:
        UPtr<DTNode> root = nullptr;
    };

    class HDTrees
    {
    public:
        struct LearnStatus
        {
            int layer = 0;
            DTree::LearnStatus tree_status;
        };

        HDTrees(const Settings & settings = Settings());

        void Learn(Ptr<const ImgReader> low_reader, Ptr<const ImgReader> high_reader, LearnStatus* status = nullptr);

        Mat PredictImage(const Mat & in_low, cv::Size size) const;

        static Ptr<HDTrees> Create(const Settings& settings);

        vector<DTree> trees;
        Settings settings;
    };
}
