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

int main()
{
    Settings settings;
    settings.layers = 4;
    settings.fuse_option = Settings::Rotate;
    settings.min_n_patches = 1500;

    HDTrees test_hdtrees(settings);

    vector<string> dir_paths =
    {
        //"D:\\test\\SRHDT\\SRHDT\\2xSet5-Bicubic",
        //"D:\\test\\SRHDT\\SRHDT\\2xSet14-Bicubic",
        "D:\\test\\SRHDT\\SRHDT\\2xBSD100-Bicubic",
    };

    vector<string> high_paths;
    vector<string> low_paths;

    for (const auto dir_path : dir_paths)
    {
        auto files = filesys::GetFilesInDir(dir_path);
        for (const auto file : files)
            if (math::EndsWith(file, "Original].bmp"))
                high_paths.push_back(file);
    }

    for (const auto p : dir_paths)
    {
        auto files = filesys::GetFilesInDir(p);
        for (const auto file : files)
            if (math::EndsWith(file, "Bicubic].bmp"))
                low_paths.push_back(file);
    }

    auto clip_one_pixel = [](Mat * img_ptr)
    {
        Mat & img = *img_ptr;
        img = Mat(img, cv::Rect(0, 0, img.cols - 1, img.rows - 1));
    };

    auto high_reader = make_shared<FileImageReader>(clip_one_pixel);
    high_reader->Set(high_paths);
    auto low_reader = make_shared<FileImageReader>();
    low_reader->Set(low_paths);

    string json_path = "D:\\test\\jsontest_hdt.json";

    RapidJsonSerializer json;
    if (learn)
    {
        // learning
        test_hdtrees.Learn(low_reader, high_reader);

        ofstream os(json_path);
        json.Serialize(test_hdtrees, os);
    }
    else
    {
        {
            ifstream ifs(json_path);
            string buf = utils::filesys::ReadFileString(ifs);
            cout << "file " << json_path << " loaded" << endl;

            istringstream iss(buf);
            json.Deserialize(iss, &test_hdtrees);
            cout << "hdt trees loaded" << endl;
        }

        for (int i = 0; i < high_paths.size(); ++i)
        {
            Mat high = high_reader->Get(i);
            Mat low = low_reader->Get(i);

            high = image::GetGrayImage(high);
            low = image::GetGrayImage(low);

            Mat out = test_hdtrees.PredictImage(low, high.size());

            cout << high_paths[i] << endl;
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