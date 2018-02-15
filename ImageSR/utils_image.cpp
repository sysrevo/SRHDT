#include "stdafx.h"
#include "utils.h"

using namespace cv;
using namespace imgsr::utils;

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

    Scalar s = sum(tmp);            // sum elements per channel

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
    cv::resize(img, res, size, 0, 0, INTER_CUBIC);
    return res;
}

Mat image::GetEdgeMap(const Mat & img, double threshold)
{
    Mat out;
    cv::Canny(img, out, threshold, threshold * 3, 3);
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

Mat image::GetGrayImage(const Mat & img)
{
    Mat gray_img;
    if (img.type() == CV_8UC3)
        cv::cvtColor(img, gray_img, COLOR_BGR2GRAY);
    else if (img.type() == CV_8UC4)
        cv::cvtColor(img, gray_img, COLOR_BGRA2GRAY);
    else if (img.type() == CV_8U)
        gray_img = Mat(img);
    return gray_img;
}

Mat image::GrayImage2FloatGrayMap(const Mat & gray_img)
{
    assert(gray_img.type() == CV_8U);
    Mat f_gray;
    gray_img.convertTo(f_gray, CV_32F);
    f_gray /= kScaleFactor;
    return f_gray;
}

Mat image::FloatGrayMap2GrayImage(const Mat & f_gray)
{
    assert(f_gray.type() == CV_32F);
    Mat res = Mat(f_gray);
    res *= kScaleFactor;
    res.convertTo(res, CV_8U);
    return res;
}
