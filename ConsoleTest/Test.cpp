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

const vector<string> valid_type =
{
	".png", ".bmp", ".jpg"
};

inline bool IsFileImage(const string& file)
{
	for (const auto& ending : valid_type)
	{
		if (file.length() >= ending.length())
		{
			if (0 == file.compare(file.length() - ending.length(), ending.length(), ending))
			{
				return true;
			}
		}
	}
	return false;
}

TestCase GetHighAndCreateLow(const string & dir_path, int max_num = 0,
    ImageReader::HandleFunc func_high = nullptr, int times = 2)
{

    auto high_imgs = FileIR::Create(func_high);

    vector<string> files = utils::math::Select(
		utils::filesys::GetFilesInDir(dir_path), IsFileImage);
    if (max_num > 0 && files.size() > max_num) files.resize(max_num);
	high_imgs->paths = files;

	auto low_imgs = WrappedIR::Create([times](Mat* img)
	{
		Size size = img->size();
		*img = image::ResizeImage(*img, size / times);
	});
    low_imgs->source = high_imgs;

    TestCase res;
    res.low = low_imgs;
    res.high = high_imgs;
    return res;
}

TestCase GetHighAndLow(const string& lr_dir, const string& hr_dir, 
	ImageReader::HandleFunc low_func = nullptr, ImageReader::HandleFunc high_func = nullptr)
{
	auto low_imgs = FileIR::Create(low_func);
	auto high_imgs = FileIR::Create(high_func);

	vector<string> low_files = utils::math::Select(
		utils::filesys::GetFilesInDir(lr_dir), IsFileImage);

	vector<string> high_files = utils::math::Select(
		utils::filesys::GetFilesInDir(hr_dir), IsFileImage);

	low_imgs->paths = low_files;
	high_imgs->paths = high_files;

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
    settings.min_n_patches = 1000;
	settings.patch_size = 6;
	settings.overlap = 4;
	settings.canny_edge_threshold = 30;

    hdtrees = make_shared<HDTrees>(settings);

	// ---------------------- nice --------------------------
	{
		const auto& path_lr = "D:\\test\\images\\nice\\set5\\lr";
		const auto& path_hr = "D:\\test\\images\\nice\\set5\\hr";
		TestCase test_case = GetHighAndLow(path_lr, path_hr);
		test_cases["nice_set5"] = test_case;
	}
	{
		const auto& path_lr = "D:\\test\\images\\nice\\set14\\lr";
		const auto& path_hr = "D:\\test\\images\\nice\\set14\\hr";
		TestCase test_case = GetHighAndLow(path_lr, path_hr);
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
	// ----------------------- manga ---------------------------
	{
		const auto& path_hr = "D:\\test\\images\\manga";
		TestCase manga = GetHighAndCreateLow(path_hr, 0, [](Mat* img_ptr)
		{
			Mat& img = *img_ptr;
			img = image::ResizeImage(img, img.size() / 4);
		});
		manga.json_path = "D:\\test\\manga.json";
		test_cases["manga"] = manga;
	}
	// ------------ normal ------------------
	{
		const auto& path_hr = "D:\\test\\images\\bsd100";
		TestCase test_case = GetHighAndCreateLow(path_hr);
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
		TestCase test_case = GetHighAndCreateLow(path_hr);
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
		TestCase test_case = GetHighAndCreateLow(path_hr);
		test_case.json_path = "D:\\test\\set14.json";
		test_cases["set14"] = test_case;
	}
	// ------------ normal ------------------
	{
		const auto& path_hr = "D:\\test\\images\\simple";
		TestCase test_case = GetHighAndCreateLow(path_hr);
		test_case.json_path = "D:\\test\\simple.json";
		test_cases["simple"] = test_case;
	}
	{
		const auto& path_hr = "D:\\test\\images\\simple";

		auto high = FileIR::Create();
		high->paths = utils::filesys::GetFilesInDir(path_hr);
		auto low = FileIR::Create([](Mat* img_ptr)
		{
			Mat& img = *img_ptr;
			cv::blur(img, img, cv::Size(5, 5));
		});
		low->paths = utils::filesys::GetFilesInDir(path_hr);

		TestCase test_case;
		test_case.high = high;
		test_case.low = low;

		test_case.json_path = "D:\\test\\simple.json";
		test_cases["simple_blur"] = test_case;
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

	auto start = std::chrono::system_clock::now();

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

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	cout << "Elapsed time: " << elapsed_seconds.count() << endl;

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

	double aver_psnr_bicubic = 0;
	double aver_psnr_predict = 0;
	int img_count = 0;

	for (const auto& case_name : case_names)
	{
		const auto& test_imgs = test_cases[case_name];
		const Ptr<ImageReader>& imgs_low = test_imgs.low;
		const Ptr<ImageReader>& imgs_high = test_imgs.high;

		for (int i = 0; i < imgs_low->Size(); ++i)
		{
			Mat high = imgs_high->Get(i);
			Mat low = imgs_low->Get(i);


			//Mat out = MonitorProcessTest(low, high.size());
			Mat out = hdtrees->PredictImage(low, high.size());

			high = image::ResizeImageToFitPatchIfNeeded(high, high.size(), hdtrees->settings.patch_size, hdtrees->settings.overlap);
			low = image::ResizeImageToFitPatchIfNeeded(low, high.size(), hdtrees->settings.patch_size, hdtrees->settings.overlap);
			double psnr_predict = image::GetPSNR(out, high);
			double psnr_bicubic = image::GetPSNR(low, high);
			aver_psnr_bicubic += psnr_bicubic;
			aver_psnr_predict += psnr_predict;
			cout << "PSNR: " << psnr_bicubic << "; Predict: " << psnr_predict << endl;
			cout << "PSNR difference: " << psnr_predict - psnr_bicubic << endl;
			cv::imshow("low", low);
			cv::imshow("out", out);
			cv::imshow("high", high);
			cv::waitKey(0);

			img_count += 1;
		}
	}

	aver_psnr_bicubic /= img_count;
	aver_psnr_predict /= img_count;

	cout << "Bicubic PSNR: " << aver_psnr_bicubic << endl;
	cout << "Predict PSNR: " << aver_psnr_predict << endl;
}