#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "MyData.h"
#include "CameraParams.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

class XMLParser
{
public:
	XMLParser();

	bool SerializeMyData(std::string filePath, MyData data);
	bool SerializeCameraParams(std::string filePath, CameraParams params);
	bool DeserializeMyData(cv::String filePath, MyData& data);
	bool DeserializeCameraParams(cv::String filePath, CameraParams& params);

	//static void write(cv::FileStorage& fs, const std::string&, const MyData& x);
	//static void read(const cv::FileNode& node, MyData& x, const MyData& default_value = MyData());
	//static ostream& operator<<(ostream& out, const MyData& m);

	~XMLParser();
};

