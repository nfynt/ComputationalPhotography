#include "pch.h"

#include <iostream>
#include <string>
#include "XMLParser.h"
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "CameraParams.h"
#include <fstream>
#include <filesystem>

using namespace std;
using namespace cv;



int Calib()
{
	string dataPath = "D:/UCL_coursework/ComputationalPhotography/ProjectFiles/Data/";
	string chessImgDir = "C:/Users/shubh/Pictures/OpenCVImg/Data/Note8Camera_chess";
	string cameraParamFile = "camera_note8.xml";
	XMLParser xml;

	//ifstream f((dataPath + cameraParamFile).c_str());
	//if (f.good())
	if (filesystem::exists(dataPath + cameraParamFile))
	{
		//file exists
		cout << "Camera parameters already exists" << endl;
		CameraParams params;
		xml.DeserializeCameraParams(dataPath + cameraParamFile, params);

		cout << endl << params.intrinsic.at<uchar>(0, 0) << "\n" << params.intrinsic.at<uchar>(1, 1);

		return 0;
	}

	//file doesn't exists create one

	int numBoards = 0;
	//internal
	int numCornersHor = 6;
	int numCornersVer = 9;

	int numSquares = numCornersHor * numCornersVer;
	Size board_sz = Size(numCornersHor, numCornersVer);

	vector<vector<Point3f>> object_points;
	vector<vector<Point2f>> image_points;

	vector<Point2f> corners;
	int successes = 0;

	Mat image;
	Mat gray_image;

	cv::namedWindow("Chess", WINDOW_NORMAL);
	cv::resizeWindow("Chess", 600, 600);

	for (const auto & entry : filesystem::directory_iterator(chessImgDir))
	{
		string file = entry.path().string();
		if (file.compare(file.length() - 4, 4, ".jpg") == 0)
		{
			image = imread(file);
		}
		else {
			std::cout << entry.path() << "is not valid .jpg" << endl;
			break;
		}
		vector<Point3f> obj;
		for (int j = 0; j < numSquares; j++)
			obj.push_back(Point3f(j / numCornersHor, j%numCornersHor, 0.0f));

		cvtColor(image, gray_image, COLOR_BGR2GRAY);
		bool found = findChessboardCorners(image, board_sz, corners, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FILTER_QUADS);

		std::cout << endl << successes << " ";
		if (found)
		{
			cornerSubPix(gray_image, corners, Size(11, 11), Size(-1, -1), TermCriteria(TermCriteria::EPS | TermCriteria::MAX_ITER, 30, 0.1));
			drawChessboardCorners(gray_image, board_sz, corners, found);

			image_points.push_back(corners);
			object_points.push_back(obj);

			printf("Snap stored!");

			successes++;
		}

		cv::imshow("Chess", gray_image);
		cv::waitKey(10);
		//}
		//cout << endl;
	}//end directory iterator

	CameraParams params;

	cv::calibrateCamera(object_points, image_points, image.size(), params.intrinsic, params.distCoeffs, params.rvecs, params.tvecs);

	//cout << endl << params.intrinsic.ptr<float>(0)[0] << "\n" << params.intrinsic.ptr<float>(1)[1];
	xml.SerializeCameraParams(dataPath + cameraParamFile, params);

	return 0;
}