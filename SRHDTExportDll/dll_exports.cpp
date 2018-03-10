// SRHDTExportDll.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "dll_exports.h"
#include <opencv2/opencv.hpp>
#include "../ImageSR/tree.h"
#include "../ImageSR/tree_serializer.h"

imgsr::Settings settings;

using imgsr::Ptr;
using imgsr::HDTrees;
using cv::Mat;

Ptr<HDTrees> trees = nullptr;

void hello::Init()
{
    trees = HDTrees::Create(settings);
    imgsr::RapidJsonSerializer json;
    json.Deserialize("D:\\test\\jsontest_hdt.json", trees.get());
}

void hello::Release()
{
    trees = nullptr;
}
void hello::PredictImage(unsigned char* img_data, unsigned char* img_data_out, int height, int width, int channel)
{
    int type = channel == 3 ? CV_8UC3 : CV_8UC1;
    Mat img(height, width, type, img_data);

    Mat out = trees->PredictImage(img, img.size() * 2);

    Mat out_img(out.size(), type, img_data_out);

    out.copyTo(out_img);
}