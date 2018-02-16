#include "stdafx.h"
#include "../ImageSR/tree.h"
#include "../ImageSR/utils_logger.h"
#include "../ImageSR/utils.h"
#include "../ImageSR/image_reader.h"
#include "../ImageSR/tree_serializer.h"

using cv::Mat;
using std::ofstream;
using std::ifstream;
using std::istringstream;
using std::stringstream;
using std::cout;
using std::endl;
using std::fstream;

using namespace imgsr;
using namespace imgsr::utils;

const bool learn = false;

struct ImagesPair
{
    Ptr<FileImageReader> low;
    Ptr<FileImageReader> high;
};

ImagesPair GetImages(const string & dir_path, 
    ImageReader::HandleFunc func_high = nullptr, 
    ImageReader::HandleFunc func_low = nullptr)
{
    auto filter_high_res = [](const string & p)
    {
        return math::EndsWith(p, "Original].bmp");
    };

    auto filter_low_res = [](const string & p)
    {
        return math::EndsWith(p, "Bicubic].bmp");
    };
    Ptr<FileImageReader> high_imgs = FileImageReader::Create(func_high);
    high_imgs->Set(math::Select(
        filesys::GetFilesInDir(dir_path), filter_high_res));

    Ptr<FileImageReader> low_imgs = FileImageReader::Create(func_low);
    low_imgs->Set(math::Select(
        filesys::GetFilesInDir(dir_path), filter_low_res));

    ImagesPair res;
    res.low = low_imgs;
    res.high = high_imgs;
    return res;
}

int main()
{
    Settings settings;
    settings.layers = 4;
    settings.fuse_option = Settings::Rotate;
    settings.min_n_patches = 1500;

    HDTrees test_hdtrees(settings);

    const string path_set5 = "D:\\test\\SRHDT\\SRHDT\\2xSet5-Bicubic";
    const string path_set14 = "D:\\test\\SRHDT\\SRHDT\\2xSet14-Bicubic";
    const string path_bsd100 = "D:\\test\\SRHDT\\SRHDT\\2xBSD100-Bicubic";

    auto clip_one_pixel = [](Mat * img_ptr)
    {
        Mat & img = *img_ptr;
        img = Mat(img, cv::Rect(0, 0, img.cols - 1, img.rows - 1));
    };

    ImagesPair set5 = GetImages(path_set5);
    ImagesPair set14 = GetImages(path_set14);
    ImagesPair bsd100 = GetImages(path_bsd100, clip_one_pixel);

    Ptr<HandlerImageReader> imgs_low = HandlerImageReader::Create();
    imgs_low->SetInput({ set5.low, set14.low, bsd100.low });
    Ptr<HandlerImageReader> imgs_high = HandlerImageReader::Create();
    imgs_high->SetInput({ set5.high, set14.high, bsd100.high });

    string json_path = "D:\\test\\jsontest_hdt.json";

    RapidJsonSerializer json;
    if (learn)
    {
        // learning
        test_hdtrees.Learn(bsd100.low, bsd100.high);

        ofstream os(json_path);
        json.Serialize(test_hdtrees, os);
    }
    else
    {
        {
            ifstream ifs(json_path);
            string buf = utils::filesys::ReadFileString(ifs);
            cout << "file " << json_path << " loaded" << endl;
            ifs.close();
            ifs.clear();

            istringstream iss(buf);
            json.Deserialize(iss, &test_hdtrees);
            cout << "hdt trees loaded" << endl;
        }

        for (int i = 0; i < imgs_low->Size(); ++i)
        {
            Mat high = imgs_high->Get(i);
            Mat low = imgs_low->Get(i);

            high = image::GetGrayImage(high);
            low = image::GetGrayImage(low);

            Mat out = test_hdtrees.PredictImage(low, high.size());

            cout << "Bicubic PSNR: " << image::GetPSNR(low, high) << endl;
            cout << "Predict PSNR: " << image::GetPSNR(out, high) << endl;
            cv::imshow("low", low);
            cv::imshow("out", out);
            cv::imshow("high", high);
            cv::waitKey(0);
        }
    }
    return 0;
}