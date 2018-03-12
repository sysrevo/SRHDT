#include <thread>
#include <chrono>
#include "Test.h"
#include "../Utils/utils.h"
#include "../ImageSR/tree.h"
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
}

ImagesPair GetHighAndCreateLow(const string & dir_path, int max_num = 0,
    ImageReader::HandleFunc func_high = nullptr)
{
    auto high_imgs = FileImageReader::Create(func_high);

    vector<string> files = utils::filesys::GetFilesInDir(dir_path);
    if (max_num > 0 && files.size() > max_num) files.resize(max_num);
    high_imgs->Set(files);

    auto low_imgs = HandlerImageReader::Create(GetLowRes);
    low_imgs->SetInput(high_imgs);

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
    Mat img = cv::imread("D:\\test\\SRHDT\\SRHDT\\2xSet5-Bicubic\\baby[1-Original].bmp");
    Mat edge = utils::image::GetEdgeMap(img, 20);
    cv::imwrite("D:\\test\\SRHDT\\SRHDT\\2xSet5-Bicubic\\edge.png", edge);

    Settings settings;
    settings.layers = 4;
    settings.fuse_option = Settings::Rotate;
    settings.min_n_patches = 1000;

    hdtrees = make_shared<HDTrees>(settings);

    set5 = GetHighAndCreateLow(path_set5);
    set14 = GetHighAndCreateLow(path_set14);
    bsd100 = GetHighAndCreateLow(path_bsd100);
    training = GetHighAndCreateLow(path_training, 200);
}

void Learn()
{
    // learning
    const HDTrees::LearnStatus* status;
    bool is_finish = false;
    std::thread t([&is_finish, &status]()
    {
        hdtrees->Learn(*training.low, *training.high);
        status = &hdtrees->GetLearnStatus();
        is_finish = true;
    });

    int last_layer = -1;

    while (!is_finish)
    {
        if (last_layer == status->layer)
            cout << "\r";
        else
        {
            // layer changed
            cout << endl;
            last_layer = status->layer;
        }

        // wipe things out
        char tmp[64];
        memset(tmp, ' ', sizeof(tmp));
        tmp[63] = 0;
        cout << tmp << "\r";
        
        // output status
        cout << "curr_layer: " << status->layer
            << ", samples" << status->tree->n_samples
            << ", non-leaves:" << status->tree->n_nonleaf
            << ", leaves:" << status->tree->n_leaf
            << ", n_test:" << status->tree->n_curr_test;
        cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    cout << endl;
    t.join();

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
    imgs_low->SetInput({ bsd100.low, set5.low, set14.low });
    Ptr<HandlerImageReader> imgs_high = HandlerImageReader::Create();
    imgs_high->SetInput({ bsd100.high, set5.high, set14.high });

    for (int i = 0; i < imgs_low->Size(); ++i)
    {
        Mat high = imgs_high->Get(i);
        Mat low = imgs_low->Get(i);

        //Mat out = MonitorProcessTest(low, high.size());
        Mat out = hdtrees->PredictImage(low, high.size());

        Mat h0;
        cv::resize(low, h0, high.size(), 0, 0, cv::INTER_CUBIC);
        cout << "PSNR: " << image::GetPSNR(out, high) - image::GetPSNR(h0, high) << endl;
        cv::imshow("low", low);
        cv::imshow("out", out);
        cv::imshow("high", high);
        cv::waitKey(0);
    }
}