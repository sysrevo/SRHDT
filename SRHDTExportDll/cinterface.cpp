// SRHDTExportDll.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "srhdt.h"

#include <opencv2/opencv.hpp>
#include "../ImageSR/tree.h"
#include "../ImageSR/tree_serializer.h"

#define BYTE unsigned char

imgsr::Settings settings;

using imgsr::Ptr;
using imgsr::HDTrees;
using cv::Mat;

Ptr<HDTrees> trees = nullptr;

void srhdt::Init(const char* data_path)
{
	if (data_path == nullptr) return;

    trees = HDTrees::Create(settings);
    imgsr::RapidJsonSerializer json;
    json.ReadTrees(data_path, trees.get());
}

void srhdt::Release()
{
    trees = nullptr;
}

void srhdt::PredictImage(BYTE* input_img, int channel,
	int input_height, int input_width, int output_height, int output_width,
	BYTE* output_img)
{
	if (!trees)
	{
		return;
	}
    int type = channel == 3 ? CV_8UC3 : CV_8UC1;
    Mat input(input_height, input_width, type, input_img);

    Mat out = trees->PredictImage(input, cv::Size(output_width, output_height));

    Mat out_img(out.size(), type, output_img);

    out.copyTo(out_img);
}