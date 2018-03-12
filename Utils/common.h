#pragma once
#include <cstdio>
#include <tchar.h>
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <exception>

#include <Eigen/Core>
#include <Eigen/Dense>

#include <opencv2/opencv.hpp>
#include <opencv2/core/eigen.hpp>

namespace imgsr
{
	using std::vector;
	using std::string;
	using std::wstring;
	using std::array;
	using std::unordered_map;
	using std::unordered_set;
	using std::queue;
	using std::pair;
	using cv::Mat;
	using cv::Size;

	template<class T>
	using UPtr = std::unique_ptr<T>;

	template<class T>
	using Ptr = std::shared_ptr<T>;

	using std::make_shared;
	using std::make_unique;

	//typedef double Real;
	using Real = double;
	using EVec = Eigen::Matrix<Real, Eigen::Dynamic, 1>;
	using ERowVec = Eigen::Matrix<Real, 1, Eigen::Dynamic>;
	//typedef Eigen::Matrix<Real, Eigen::Dynamic, 1> EVec;
	//typedef Eigen::Matrix<Real, 1, Eigen::Dynamic> ERowVec;
	typedef Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> EMat;
	typedef Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> ERowMat;
}