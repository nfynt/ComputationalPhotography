#pragma once

#include <string>
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"

using namespace std;
using namespace cv;

class CameraParams
{
public:

	Mat intrinsic = Mat(3, 3, CV_32FC1);
	Mat distCoeffs;
	vector<Mat> rvecs;
	vector<Mat> tvecs;

	CameraParams();
	~CameraParams();
};

