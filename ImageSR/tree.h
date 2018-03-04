#pragma once
#include "status_struct.h"
#include "node.h"
#include "imgsr_system.h"

namespace imgsr
{
    class DTree : public LearningBasedImgSR
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

        inline DTree(const Settings & settings_): 
            settings(settings_) {}

        /// <summary>
        /// Learn input patches. Please modified this->settings to match the requirements before calling this function.
        /// Beware that the number of patches input must be larger than
        /// the length of a vectorized patch (patchSize * patchSize) and that length must equal what's in the tree's settings,
        /// otherwise shit happens (part of the regression model will not be invertable, or the size of multiplied matrics is incorrect)
        /// </summary>
        /// <param name="low_imgs"></param>
        /// <param name="high_imgs"></param>
        virtual void Learn(
            const ImageReader& low_reader, const ImageReader& high_reader) override;

        void Learn(const Ptr<TrainingData> & total_samples);

        /// <summary>
        /// Predict an image. Expected width and height must be at least the width and height of image 'low'.
        /// </summary>
        /// <param name="in_low">The input image. Please check if this image is not empty and only with type CV_8U first.</param>
        /// <param name="size">The width of high-res output image.</param>
        /// <returns>The high-res image predicted by this decision tree</returns>
        virtual Mat PredictImage(const Mat & in_low, cv::Size size) const override;

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

        inline const LearnStatus& GetLearnStatus() const
        {
            return learn_stat;
        }

        Settings settings;
    private:
        UPtr<DTNode> root = nullptr;
        LearnStatus learn_stat;
    };

    class HDTrees
    {
    public:
        struct LearnStatus
        {
            int layer = 0;
            const DTree::LearnStatus* tree = nullptr;
        };

        inline HDTrees(const Settings & settings_):
            settings(settings_) {}

        void Learn(const ImageReader& low_reader, const ImageReader& high_reader);

        Mat PredictImage(const Mat & img, cv::Size expected_size) const;

        inline const LearnStatus& GetLearnStatus() const
        {
            return stat_learn;
        }

        vector<DTree> trees;
        Settings settings;
    private:
        LearnStatus stat_learn;
    };
}
