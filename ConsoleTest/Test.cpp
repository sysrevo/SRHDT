#include <thread>
#include <chrono>
#include <unordered_map>
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
using std::unordered_map;

using namespace imgsr;
using namespace utils;

struct TestCase
{
	string json_path;
    Ptr<ImageReader> low;
    Ptr<ImageReader> high;
};

TestCase GetHighAndCreateLow(const string & dir_path, int max_num = 0,
    ImageReader::HandleFunc func_high = nullptr)
{
    auto high_imgs = FileImageReader::Create(func_high);

    vector<string> files = utils::filesys::GetFilesInDir(dir_path);
    if (max_num > 0 && files.size() > max_num) files.resize(max_num);
    high_imgs->Set(files);

	auto low_imgs = HandlerImageReader::Create([](Mat* img)
	{
		Size size = img->size();
		cv::resize(*img, *img, size / 2, 0, 0, cv::INTER_AREA);
	});
    low_imgs->SetInput(high_imgs);

    TestCase res;
    res.low = low_imgs;
    res.high = high_imgs;
    return res;
}

TestCase GetHighAndLow(const string& lr_dir, const string& hr_dir, 
	ImageReader::HandleFunc low_func = nullptr, ImageReader::HandleFunc high_func = nullptr)
{
	auto low_imgs = FileImageReader::Create(low_func);
	auto high_imgs = FileImageReader::Create(high_func);

	low_imgs->Set(filesys::GetFilesInDir(lr_dir));
	high_imgs->Set(filesys::GetFilesInDir(hr_dir));

	TestCase res;
	res.low = low_imgs;
	res.high = high_imgs;
	return res;
}
unordered_map<string, TestCase> test_cases;
Ptr<HDTrees> hdtrees = nullptr;

void Test::Init()
{
    Settings settings;
    settings.layers = 4;
    settings.fuse_option = Settings::Rotate;
    settings.min_n_patches = 1024;
	settings.patch_size = 6;
	settings.overlap = 4;

    hdtrees = make_shared<HDTrees>(settings);

	{
		const auto& path_lr = "D:\\test\\images\\nice\\set5\\lr";
		const auto& path_hr = "D:\\test\\images\\nice\\set5\\hr";
		TestCase test_case = GetHighAndLow(path_lr, path_hr);
		test_case.json_path = "D:\\test\\nice_set5.json";
		test_cases["nice_set5"] = test_case;
	}

	{
		const auto& path_lr = "D:\\test\\images\\nice\\set14\\lr";
		const auto& path_hr = "D:\\test\\images\\nice\\set14\\hr";
		TestCase test_case = GetHighAndLow(path_lr, path_hr);
		test_case.json_path = "D:\\test\\nice_set14.json";
		test_cases["nice_set14"] = test_case;
	}

	{
		const auto& path_lr = "D:\\test\\images\\nice\\bsd100\\lr";
		const auto& path_hr = "D:\\test\\images\\nice\\bsd100\\hr";
		auto clip_one_pixel = [](Mat* img_ptr)
		{
			Mat & img = *img_ptr;
			img = Mat(img, cv::Rect(0, 0, img.cols - 1, img.rows - 1));
		};
		TestCase test_case = GetHighAndLow(path_lr, path_hr, nullptr, clip_one_pixel);
		test_case.json_path = "D:\\test\\nice_bsd100.json";
		test_cases["nice_bsd100"] = test_case;
	}

	{
		const auto& path_hr = "D:\\test\\images\\manga";
		TestCase manga = GetHighAndCreateLow(path_hr, 0, [](Mat* img_ptr)
		{
			Mat& img = *img_ptr;
			cv::resize(img, img, img.size() / 4, 0, 0, cv::INTER_AREA);
		});
		manga.json_path = "D:\\test\\manga.json";
		test_cases["manga"] = manga;
	}

	{
		const auto& path_hr = "D:\\test\\images\\bsd100";
		auto clip_one_pixel = [](Mat* img_ptr)
		{
			Mat & img = *img_ptr;
			img = Mat(img, cv::Rect(0, 0, img.cols - 1, img.rows - 1));
		};
		TestCase test_case = GetHighAndCreateLow(path_hr, 50);
		test_case.json_path = "D:\\test\\bsd100.json";
		test_cases["bsd100"] = test_case;
	}

	{
		const auto& path_hr = "D:\\test\\images\\set5";
		auto clip_one_pixel = [](Mat* img_ptr)
		{
			Mat & img = *img_ptr;
			img = Mat(img, cv::Rect(0, 0, img.cols - 1, img.rows - 1));
		};
		TestCase test_case = GetHighAndCreateLow(path_hr, 50);
		test_case.json_path = "D:\\test\\set5.json";
		test_cases["set5"] = test_case;
	}

	{
		const auto& path_hr = "D:\\test\\images\\set14";
		auto clip_one_pixel = [](Mat* img_ptr)
		{
			Mat & img = *img_ptr;
			img = Mat(img, cv::Rect(0, 0, img.cols - 1, img.rows - 1));
		};
		TestCase test_case = GetHighAndCreateLow(path_hr, 50);
		test_case.json_path = "D:\\test\\set14.json";
		test_cases["set14"] = test_case;
	}

	{
		const auto& path_lr = "D:\\test\\images\\nice\\set5_3\\lr";
		const auto& path_hr = "D:\\test\\images\\nice\\set5_3\\hr";
		TestCase test_case = GetHighAndLow(path_lr, path_hr);
		test_case.json_path = "D:\\test\\nice_set5_3.json";
		test_cases["nice_set5_3"] = test_case;
	}

	{
		const auto& path_lr = "D:\\test\\images\\nice\\set14_3\\lr";
		const auto& path_hr = "D:\\test\\images\\nice\\set14_3\\hr";
		TestCase test_case = GetHighAndLow(path_lr, path_hr);
		test_case.json_path = "D:\\test\\nice_set14_3.json";
		test_cases["nice_set14_3"] = test_case;
	}

	{
		const auto& path_lr = "D:\\test\\images\\nice\\bsd100_3\\lr";
		const auto& path_hr = "D:\\test\\images\\nice\\bsd100_3\\hr";
		auto clip_one_pixel = [](Mat* img_ptr)
		{
			Mat & img = *img_ptr;
			img = Mat(img, cv::Rect(0, 0, img.cols - 1, img.rows - 1));
		};
		TestCase test_case = GetHighAndLow(path_lr, path_hr, nullptr, clip_one_pixel);
		test_case.json_path = "D:\\test\\nice_bsd100_3.json";
		test_cases["nice_bsd100_3"] = test_case;
	}
}

std::vector<std::string> Test::GetTestCaseNames()
{
	vector<string> names;
	names.reserve(test_cases.size());

	for (const auto& item : test_cases)
	{
		names.push_back(item.first);
	}

	return names;
}

void Test::Learn(const string& name)
{
	const auto& test_case = test_cases[name];
	const Ptr<ImageReader>& lows = test_case.low;
	const Ptr<ImageReader>& highs = test_case.high;
	const string& json_path = test_case.json_path;

    // learning
    const HDTrees::LearnStatus* status = nullptr;
    bool is_finish = false;
	status = &hdtrees->GetLearnStatus();
    std::thread t([&is_finish, &status, lows, highs]()
    {
        hdtrees->Learn(*lows, *highs);
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
        char tmp[80];
        memset(tmp, ' ', sizeof(tmp));
        tmp[sizeof(tmp) - 1] = 0;
        cout << tmp << "\r";
        
        // output status
		cout << "curr_layer: " << status->layer;
		if (status->tree)
		{
			cout
				<< ", samples" << status->tree->n_samples
				<< ", non-leaves:" << status->tree->n_nonleaf
				<< ", leaves:" << status->tree->n_leaf
				<< ", n_test:" << status->tree->n_curr_test;
		}
        cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    cout << endl;
    t.join();

    RapidJsonSerializer json;
    json.Serialize(*hdtrees, json_path);
}

void Test::Test(const string& test_name, const std::vector<std::string>& case_names)
{
	const auto& test_case = test_cases[test_name];
	const string& json_path = test_case.json_path;

    RapidJsonSerializer json;
    {
        json.Deserialize(json_path, hdtrees.get());
        cout << "hdt trees loaded" << endl;
    }

	for (const auto& case_name : case_names)
	{
		const auto& test_imgs = test_cases[case_name];
		const Ptr<ImageReader>& imgs_low = test_imgs.low;
		const Ptr<ImageReader>& imgs_high = test_imgs.high;

		for (int i = 0; i < imgs_low->Size(); ++i)
		{
			Mat high = imgs_high->Get(i);
			Mat low = imgs_low->Get(i);

			high = image::ResizeImage(high, high.size(), hdtrees->settings.patch_size, hdtrees->settings.overlap);
			low = image::ResizeImage(low, low.size(), hdtrees->settings.patch_size, hdtrees->settings.overlap);

			//Mat out = MonitorProcessTest(low, high.size());
			Mat out = hdtrees->PredictImage(low, high.size());

			Mat h0;
			cv::resize(low, h0, high.size(), 0, 0, cv::INTER_CUBIC);
			cout << "PSNR: " << image::GetPSNR(out, high) - image::GetPSNR(h0, high) << endl;
			cv::imshow("low", h0);
			cv::imshow("out", out);
			cv::imshow("high", high);
			cv::waitKey(0);
		}
	}

    
}