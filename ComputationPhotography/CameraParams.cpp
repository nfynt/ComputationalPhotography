#include "pch.h"
#include "CameraParams.h"


CameraParams::CameraParams()
{
	intrinsic = Mat(3, 3, CV_32FC1);

	//setting default focal length to 1
	intrinsic.ptr<float>(0)[0] = 1;
	intrinsic.ptr<float>(1)[1] = 1;
}

CameraParams::~CameraParams()
{
}
