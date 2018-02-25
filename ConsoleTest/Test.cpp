#include "Test.h"
#include "../ImageSR/tree.h"
#include "../ImageSR/utils_logger.h"
#include "../ImageSR/utils_math.h"
#include "../ImageSR/utils_filesys.h"
#include "../ImageSR/utils_image.h"
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
using namespace utils;

struct ImagesPair
{
    Ptr<ImageReader> low;
    Ptr<ImageReader> high;
};

void GetLowRes(Mat* img)
{
    Size size = img->size();
    cv::resize(*img, *img, size / 2, 0, 0, cv::INTER_LINEAR);
    cv::resize(*img, *img, size    , 0, 0, cv::INTER_CUBIC);
}

ImagesPair GetHighAndCreateLow(const string & dir_path, int max_num = 0,
    ImageReader::HandleFunc func_high = nullptr)
{
    auto high_imgs = FileImageReader::Create(func_high);

    vector<string> files = filesys::GetFilesInDir(dir_path);
    if (max_num > 0 && files.size() > max_num) files.resize(max_num);
    high_imgs->Set(files);

    auto low_imgs = HandlerImageReader::Create(GetLowRes);
    low_imgs->SetInput(high_imgs);

    ImagesPair res;
    res.low = low_imgs;
    res.high = high_imgs;
    return res;
}

ImagesPair GetHighAndLow(const string & dir_path,
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
    auto high_imgs = FileImageReader::Create(func_high);
    high_imgs->Set(math::Select(
        filesys::GetFilesInDir(dir_path), filter_high_res));

    auto low_imgs = FileImageReader::Create(func_low);
    low_imgs->Set(math::Select(
        filesys::GetFilesInDir(dir_path), filter_low_res));

    ImagesPair res;
    res.low = low_imgs;
    res.high = high_imgs;
    return res;
}

void ClipOnePixel(Mat* img_ptr)
{
    Mat & img = *img_ptr;
    img = Mat(img, cv::Rect(0, 0, img.cols - 1, img.rows - 1));
}

const string path_set5 = "D:\\test\\images\\set5";
const string path_set14 = "D:\\test\\images\\set14";
const string path_bsd100 = "D:\\test\\images\\bsd100";
const string path_training = "D:\\test\\images\\training_images";
//const string path_bsd100 = "D:\\test\\SRHDT\\SRHDT\\2xBSD100-Bicubic\\hr";

const string json_path = "D:\\test\\jsontest_hdt.json";

Ptr<HDTrees> hdtrees = nullptr;
ImagesPair set5, set14, bsd100;
ImagesPair training;

void Init()
{
    Settings settings;
    settings.layers = 4;
    settings.fuse_option = Settings::Rotate;
    settings.min_n_patches = 1200;

    hdtrees = make_shared<HDTrees>(settings);

    set5 = GetHighAndCreateLow(path_set5);
    set14 = GetHighAndCreateLow(path_set14);
    bsd100 = GetHighAndCreateLow(path_bsd100);
    training = GetHighAndCreateLow(path_training, 200);
}

void Learn()
{
    // learning
    hdtrees->Learn(training.low, training.high);

    RapidJsonSerializer json;
    json.Serialize(*hdtrees, json_path);
}

void Test()
{
    RapidJsonSerializer json;
    {
        json.Deserialize(json_path, hdtrees.get());
        cout << "hdt trees loaded" << endl;
    }


    Ptr<HandlerImageReader> imgs_low = HandlerImageReader::Create();
    imgs_low->SetInput({ set5.low, set14.low });
    Ptr<HandlerImageReader> imgs_high = HandlerImageReader::Create();
    imgs_high->SetInput({ set5.high, set14.high });

    for (int i = 0; i < imgs_low->Size(); ++i)
    {
        Mat high = imgs_high->Get(i);
        Mat low = imgs_low->Get(i);

        Mat out = hdtrees->PredictImage(low, high.size());

        cout << "PSNR: " << image::GetPSNR(out, high) - image::GetPSNR(low, high) << endl;
        cv::imshow("low", low);
        cv::imshow("out", out);
        cv::imshow("high", high);
        cv::waitKey(0);
    }
}