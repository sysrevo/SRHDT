#include <thread>
#include <chrono>
#include <unordered_map>
#include <numeric>
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
    Settings settings;
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
    ImageReader::Handler func_high = nullptr, int times = 2)
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
	ImageReader::Handler low_func = nullptr, ImageReader::Handler high_func = nullptr)
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
        test_case.settings = settings;
		test_cases["nice_set5"] = test_case;
	}
	{
		const auto& path_lr = "D:\\test\\images\\nice\\set14\\lr";
		const auto& path_hr = "D:\\test\\images\\nice\\set14\\hr";
		TestCase test_case = GetHighAndLow(path_lr, path_hr);
        test_case.settings = settings;
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
        test_case.settings = settings;
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
        manga.settings = settings;
		test_cases["manga"] = manga;
	}

    // ----------------------- pexels ---------------------------
    {
        const auto& path_hr = "D:\\test\\images\\pexels400";
        TestCase pexels = GetHighAndCreateLow(path_hr, 400);
        pexels.json_path = "D:\\test\\pexels400.json";
        pexels.settings = settings;
        test_cases["pexels400"] = pexels;
    }
    {
        const auto& path_hr = "D:\\test\\images\\pexels400";
        TestCase pexels = GetHighAndCreateLow(path_hr, 400, nullptr, 3);
        pexels.json_path = "D:\\test\\pexels400_3.json";
        pexels.settings = settings;
        test_cases["pexels400_3"] = pexels;
    }
    {
        const auto& path_hr = "D:\\test\\images\\pexels200";
        TestCase pexels = GetHighAndCreateLow(path_hr, 200);
        pexels.json_path = "D:\\test\\pexels.json";
        pexels.settings = settings;
        pexels.settings.fuse_option = Settings::None;
        test_cases["pexels_nf"] = pexels;
    }

	// ------------ normal ------------------
	{
		const auto& path_hr = "D:\\test\\images\\bsd100";
		TestCase bsd100 = GetHighAndCreateLow(path_hr);
		bsd100.json_path = "D:\\test\\bsd100.json";
        bsd100.settings = settings;
		test_cases["bsd100"] = bsd100;
	}
    {
        const auto& path_hr = "D:\\test\\images\\bsd100";
        TestCase bsd100 = GetHighAndCreateLow(path_hr);
        bsd100.json_path = "D:\\test\\bsd100_1.json";
        bsd100.settings = settings;
        bsd100.settings.layers = 1;
        test_cases["bsd100_1"] = bsd100;
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
        test_case.settings = settings;
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
        test_case.settings = settings;
		test_cases["set14"] = test_case;
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
    hdtrees->settings = test_case.settings;
	const Ptr<ImageReader>& lows = test_case.low;
	const Ptr<ImageReader>& highs = test_case.high;
	const string& json_path = test_case.json_path;

    // learning
    HDTrees::LearnStatus status;
    bool is_finish = false;
    std::thread t([&is_finish, &status, lows, highs]()
    {
        hdtrees->Learn(lows, highs, &status);
        is_finish = true;
    });

    int last_layer = -1;

	auto start = std::chrono::system_clock::now();

    while (!is_finish)
    {
        if (last_layer == status.layer)
            cout << "\r";
        else
        {
            // layer changed
            cout << endl;
            last_layer = status.layer;
        }

        // wipe things out
        char tmp[80];
        memset(tmp, ' ', sizeof(tmp));
        tmp[sizeof(tmp) - 1] = 0;
        cout << tmp << "\r";
        
        // output status
		cout << "curr_layer: " << status.layer;
		cout
			<< ", samples" << status.tree_status.n_samples
			<< ", non-leaves:" << status.tree_status.n_nonleaf
			<< ", leaves:" << status.tree_status.n_leaf
			<< ", n_test:" << status.tree_status.n_curr_test;
        cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    cout << endl;
    t.join();

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	cout << "Elapsed time: " << elapsed_seconds.count() << endl;

    RapidJsonSerializer json;
    json.WriteTrees(*hdtrees, json_path);
}

class Analyzer
{
public:
    void Analyze(const Mat& low, const Mat& high, long long time)
    {
        double psnr = image::GetPSNR(low, high);
        double ssim = image::GetSSIM(low, high);
        psnrs.push_back(psnr);
        ssims.push_back(ssim);
        times.push_back(time);
    }

    void Append(const Analyzer& other)
    {
        psnrs.insert(psnrs.end(), other.psnrs.begin(), other.psnrs.end());
        ssims.insert(ssims.end(), other.ssims.begin(), other.ssims.end());
    }

    // return the last psnr value
    // , 0 if no image has been analyzed
    double GetLastPSNR() const
    {
        if (psnrs.empty()) return 0;
        return psnrs.back();
    }

    // return the last ssim value
    // , 0 if no image has been analyzed
    double GetLastSSIM() const
    {
        if (ssims.empty()) return 0;
        return ssims.back();
    }

    long long GetLastTime() const
    {
        if (times.empty()) return 0;
        return times.back();
    }

    // return the mean psnr value
    // , 0 if no image has been analyzed
    double GetMeanPSNR() const
    {
        if (psnrs.empty()) return 0;
        double sum = std::accumulate(psnrs.begin(), psnrs.end(), 0.0);
        return sum / psnrs.size();
    }

    // return the mean ssim value
    // , 0 if no image has been analyzed
    double GetMeanSSIM() const
    {
        if (ssims.empty()) return 0;
        double sum = std::accumulate(ssims.begin(), ssims.end(), 0.0);
        return sum / ssims.size();
    }

    double GetMeanTime() const
    {
        if (times.empty()) return 0;
        double sum = std::accumulate(times.begin(), times.end(), 0.0);
        return sum / times.size();
    }

    size_t Count() const
    {
        return psnrs.size();
    }
private:
    vector<double> ssims;
    vector<double> psnrs;
    vector<double> times;
};

const bool print_each = true;
const bool print_mean = true;

void Test::Test(const string& test_name, const std::vector<std::string>& case_names)
{
    using std::chrono::steady_clock;
    using std::chrono::duration_cast;
	const auto& test_case = test_cases[test_name];
	const string& json_path = test_case.json_path;

    RapidJsonSerializer json;
    {
        json.ReadTrees(json_path, hdtrees.get());
        cout << "hdt trees loaded" << endl;
    }
    hdtrees->settings = test_case.settings;
    for (auto& tree : hdtrees->trees)
        tree.settings = hdtrees->settings;

    cout << "number of nodes: " << hdtrees->GetNumNodes() << endl;
    cout << "number of leaf nodes: " << hdtrees->GetNumLeafNodes() << endl;

	for (const auto& case_name : case_names)
	{
		const auto& test_imgs = test_cases[case_name];
		const Ptr<ImageReader>& imgs_low = test_imgs.low;
		const Ptr<ImageReader>& imgs_high = test_imgs.high;

        cout << case_name << endl;

        Analyzer analyzer_bicubic;
        Analyzer analyzer_predict;

		for (int i = 0; i < imgs_low->Size(); i += 1)
		{
			Mat high = imgs_high->Get(i);
			Mat low = imgs_low->Get(i);
            // resize the image to the desired size
			high = image::ResizeImageToFitPatchIfNeeded(high, high.size(), hdtrees->settings.patch_size, hdtrees->settings.overlap);

            auto start = steady_clock::now();
			Mat bicubic = image::ResizeImageToFitPatchIfNeeded(low, high.size(), hdtrees->settings.patch_size, hdtrees->settings.overlap);
            auto end = steady_clock::now();
            auto dif = duration_cast<std::chrono::milliseconds>(end - start).count();
            analyzer_bicubic.Analyze(bicubic, high, dif);

            start = steady_clock::now();
            Mat out = hdtrees->PredictImage(low, high.size());
            end = steady_clock::now();
            dif = duration_cast<std::chrono::milliseconds>(end - start).count();
            analyzer_predict.Analyze(out, high, dif);
            if (print_each)
            {
                cout << "Bicubic PSNR: " << analyzer_bicubic.GetLastPSNR() << endl;
                cout << "Bicubic SSIM: " << analyzer_bicubic.GetLastSSIM() << endl;
                cout << "Bicubic Time(ms): " << analyzer_bicubic.GetLastTime() << endl;

                cout << "Predict PSNR: " << analyzer_predict.GetLastPSNR() << endl;
                cout << "Predict SSIM: " << analyzer_predict.GetLastSSIM() << endl;
                cout << "Predict Time(ms): " << analyzer_predict.GetLastTime() << endl;
                cout << endl;
            }
		}
        if (print_mean)
        {
            cout << "Mean Bicubic PSNR: " << analyzer_bicubic.GetMeanPSNR() << endl;
            cout << "Mean Bicubic SSIM: " << analyzer_bicubic.GetMeanSSIM() << endl;
            cout << "Mean Bicubic Time(ms): " << analyzer_bicubic.GetMeanTime() << endl;

            cout << "Mean Predict PSNR: " << analyzer_predict.GetMeanPSNR() << endl;
            cout << "Mean Predict SSIM: " << analyzer_predict.GetMeanSSIM() << endl;
            cout << "Mean Predict Time(ms): " << analyzer_predict.GetMeanTime() << endl;
            cout << endl;
        }
	}
}