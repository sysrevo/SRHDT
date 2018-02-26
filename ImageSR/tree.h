#pragma once

#include "node.h"

namespace imgsr
{
    class TreeJsonException : std::exception
    {
    public:
        TreeJsonException(const char* msg_) :msg(msg_) {}
        const char* what() const { return msg.data(); }
    private:
        string msg;
    };

    class DTree
    {
    public:
        Settings settings;

        inline DTNode* GetRoot() const { return root.get(); }

        void CreateRoot();

        inline DTree(const Settings & settings_): 
            settings(settings_) {}

        /// <summary>
        /// Learn input patches. Please modified this->settings to match the requirements before calling this function.
        /// Beware that the number of patches input must be larger than
        /// the length of a vectorized patch (patchSize * patchSize) and that length must equal what's in the tree's settings,
        /// otherwise shit happens (part of the regression model will not be invertable, or the size of multiplied matrics is incorrect)
        /// </summary>
        /// <param name="low_reader"></param>
        /// <param name="high_reader"></param>
        void Learn(const Ptr<ImageReader> & low_reader, const Ptr<ImageReader> & high_reader);

        /// <summary>
        /// Predict an image. Expected width and height must be at least the width and height of image 'low'.
        /// </summary>
        /// <param name="in_low">The input image. Please check if this image is not empty and only with type CV_8U first.</param>
        /// <param name="size">The width of high-res output image.</param>
        /// <returns>The high-res image predicted by this decision tree</returns>
        Mat PredictImage(const Mat & in_low, cv::Size size) const;

        /// <summary>
        /// Predict an input patch. Please modified this->settings to match requirements before calling this function.
        /// Beware that the size of this two patch must be the same and match settings.patchSize.
        /// </summary>
        /// <param name="in_patch">Input patch for the prediction. Cols and rows must be the same as settings.patchSize.</param>
        Mat PredictPatch(const Mat & in_patch) const;

        void PrintBrief(std::ostream & os) const;

        inline int GetNumNodes() const
        {
            return root ? root->GetNumNodes() : 0;
        }

        inline int GetNumLeafNodes() const 
        {
            return root ? root->GetNumLeafNodes() : 0;
        }

    private:
        void Learn(const Ptr<TrainingData> & total_samples);

        UPtr<DTNode> root = nullptr;
    };

    class HDTrees
    {
    public:
        vector<DTree> trees;
        Settings settings;

        inline HDTrees(const Settings & settings_):
            settings(settings_) {}

        void Learn(const Ptr<ImageReader> & low_reader, const Ptr<ImageReader> & high_reader);

        Mat PredictImage(const Mat & img, cv::Size expected_size) const;
    };
}
