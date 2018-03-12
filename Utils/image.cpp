#include "utils_image.h"

using namespace imgsr;
using namespace utils;

//EVec Image::PatchToVector(const Mat & patch)
//{
//    EVec vec = EVec::Zero(patch.rows * patch.cols);
//    PatchToVector(patch, &vec);
//    return vec;
//}

double image::GetPSNR(const Mat & img1, const Mat & img2)
{
	assert(img1.size() == img2.size());
	Mat tmp;
	absdiff(img1, img2, tmp);       // |img1 - img2|
	tmp.convertTo(tmp, CV_64F);     // cannot make a square on 8 bits
	tmp = tmp.mul(tmp);             // |img1 - img1|^2

	cv::Scalar s = sum(tmp);            // sum elements per channel

	double sse = s.val[0] + s.val[1] + s.val[2]; // sum channels

	if (sse <= 1e-10) // for small values return zero
		return 0;
	else
	{
		double mse = sse / (double)(img1.channels() * img1.total());
		double psnr = 10.0 * log10((255 * 255) / mse);
		return psnr;
	}
}

cv::Scalar image::GetSSIM(const Mat & i1, const Mat & i2)
{
	const double C1 = 6.5025, C2 = 58.5225;
	/***************************** INITS **********************************/
	int d = CV_64F;

	Mat tmp1, tmp2;
	i1.convertTo(tmp1, d);           // cannot calculate on one byte large values
	i2.convertTo(tmp2, d);

	Mat I2_2 = tmp2.mul(tmp2);        // I2^2
	Mat I1_2 = tmp1.mul(tmp1);        // I1^2
	Mat I1_I2 = tmp1.mul(tmp2);        // I1 * I2

									   /***********************PRELIMINARY COMPUTING ******************************/

	Mat mu1, mu2;   //
	GaussianBlur(tmp1, mu1, Size(11, 11), 1.5);
	GaussianBlur(tmp2, mu2, Size(11, 11), 1.5);

	Mat mu1_2 = mu1.mul(mu1);
	Mat mu2_2 = mu2.mul(mu2);
	Mat mu1_mu2 = mu1.mul(mu2);

	Mat sigma1_2, sigma2_2, sigma12;

	GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
	sigma1_2 -= mu1_2;

	GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
	sigma2_2 -= mu2_2;

	GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
	sigma12 -= mu1_mu2;

	///////////////////////////////// FORMULA ////////////////////////////////
	Mat t1, t2, t3;

	t1 = 2 * mu1_mu2 + C1;
	t2 = 2 * sigma12 + C2;
	t3 = t1.mul(t2);              // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))

	t1 = mu1_2 + mu2_2 + C1;
	t2 = sigma1_2 + sigma2_2 + C2;
	t1 = t1.mul(t2);               // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))

	Mat ssim_map;
	divide(t3, t1, ssim_map);      // ssim_map =  t3./t1;

	cv::Scalar mssim = mean(ssim_map); // mssim = average of ssim map
	return mssim;
}

inline cv::Size GetCorrectSize(cv::Size size, int patch_size, int jump)
{
	auto func = [patch_size, jump](int n)
	{
		return patch_size + int(std::ceil(double(n - patch_size) / jump) * jump);
	};
	return cv::Size(func(size.width), func(size.height));
}

Mat image::ResizeImage(const Mat & img, cv::Size expected_size, int patch_size, int overlap)
{
	const int jump = patch_size - overlap;

	Mat res;
	Size size = GetCorrectSize(expected_size, patch_size, jump);
	cv::resize(img, res, size, 0, 0, cv::INTER_CUBIC);
	return res;
}

Mat image::GetEdgeMap(const Mat & img, double threshold)
{
	Mat out;
	cv::Canny(img, out, threshold, threshold * 3);
	return out;
}

inline int GetCorrectSize(int size, int patch_size)
{
	int num_patches = int(std::ceil(double(size) / patch_size));
	return num_patches * patch_size;
}

Mat image::GetLowResImage(const Mat & img, float ratio)
{
	Mat out;
	Size size = img.size();
	size.width = (int)(size.width * ratio);
	size.height = (int)(size.height * ratio);

	cv::resize(img, out, size, 0, 0, cv::INTER_CUBIC);
	cv::resize(out, out, img.size(), 0, 0, cv::INTER_CUBIC);
	return out;
}

image::YCrCbImage image::SplitYCrcb(const Mat & img)
{
	YCrCbImage res;
	Mat buf;
	if (img.channels() == 3)
		cv::cvtColor(img, buf, CV_BGR2YCrCb);
	cv::split(buf, (Mat*)&res);
	return res;
}

Mat image::Merge(const YCrCbImage & img)
{
	Mat res;
	cv::merge((Mat*)&img, 3, res);
	cv::cvtColor(res, res, CV_YCrCb2BGR);
	return res;
}

Mat image::MatUchar2Float(const Mat & img)
{
	assert(img.type() == CV_8U);
	Mat f_img;
	img.convertTo(f_img, image::kFloatImageType);
	f_img /= kScaleFactor;
	return f_img;
}

Mat image::MatFloat2Uchar(const Mat & img)
{
	assert(img.type() == image::kFloatImageType);
	Mat u8_img = Mat(img);
	u8_img *= kScaleFactor;
	u8_img.convertTo(u8_img, CV_8U);
	return u8_img;
}

vector<Mat> image::GetPatches(const Mat & img, const Mat & edge, int pat_size, int overlap)
{
	assert(img.type() == image::kFloatImageType);
	assert(edge.type() == CV_8U);
	assert(img.size() == edge.size());

	vector<Mat> res;

	image::ForeachPatch(img, pat_size, overlap, [&edge, &res](
		const cv::Rect& rect, const Mat& pat)
	{
		if (cv::countNonZero(edge(rect)) > 0)
			res.push_back(pat);
	});

	return res;
}

vector<vector<Mat>> image::GetPatchesMulti(const vector<Mat>& imgs, const Mat & edge, int pat_size, int overlap)
{
	for (const auto & img : imgs)
	{
		assert(img.size() == edge.size());
		assert(pat_size > overlap);
	}

	vector<vector<Mat>> res;
	size_t n_imgs = imgs.size();
	res.resize(n_imgs);

	image::ForeachPatch(edge, pat_size, overlap, [&imgs, n_imgs, &res](
		const cv::Rect& rect, const Mat& pat_edge)
	{
		if (cv::countNonZero(pat_edge) > 0)
		{
			for (int i = 0; i < n_imgs; ++i)
				res[i].push_back(imgs[i](rect));
		}
	});

	return res;
}
