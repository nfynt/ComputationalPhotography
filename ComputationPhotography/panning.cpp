// ComputationPhotography.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>]
#include <string>
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/videoio.hpp"
#include "MyData.h"
#include "XMLParser.h"
#include "ExifReader.h"

using namespace std;
using namespace cv;

string dataPath = "D:/UCL_coursework/ComputationalPhotography/ProjectFiles/Data/";
Vec2i speed;
XMLParser xml;
ExifReader exifRead;

//Mouse function binding for opencv windows
void MouseFunc(int eve, int mouseX, int mouseY, int flags, void* params)
{
	switch (eve)
	{
	case EVENT_LBUTTONDOWN:
		speed = Vec2i(mouseX / 100, mouseY / 100);
		break;
	}
}


int Pan()
{
	int fWidth, fHeight;
	fWidth = fHeight = 400;
	Rect frameRect = Rect(0, 0, fWidth, fHeight);

	//Video params
	int fps = 15;
	//in seconds
	int vidDur = 30;
	int frameCnt = 0;
	VideoWriter video(dataPath + "pan.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(fWidth, fHeight));

	// parse image EXIF and XMP metadata
	TinyEXIF::EXIFInfo imageEXIF = exifRead.GetImageInfo(dataPath + "image1.jpg");
	if (imageEXIF.Fields)
		std::cout
		<< "Image Description " << imageEXIF.ImageDescription << "\n"
		<< "Image Resolution " << imageEXIF.ImageWidth << "x" << imageEXIF.ImageHeight << " pixels\n"
		<< "Camera Model " << imageEXIF.Make << " - " << imageEXIF.Model << "\n"
		<< "Focal Length " << imageEXIF.FocalLength << " mm" << std::endl;

	Mat img = imread(dataPath + "image1.jpg");
	Mat frame;
	speed = Vec2i(1, 0);
	namedWindow("Test", WINDOW_NORMAL);
	resizeWindow("Test", Size(400, 400));
	int params = 5;
	setMouseCallback("Test", MouseFunc, &params);

	int imgWidth = img.size().width;
	int imgHeight = img.size().height;
	cout << "\nImg size:" << imgWidth << "x" << imgHeight << endl;

	while (true)
	{
		if (frameRect.x + speed.val[0] + fWidth < imgWidth)
			frameRect.x += speed.val[0];
		else
			frameRect.x = 0;
		if (frameRect.y + speed.val[1] + fHeight < imgHeight)
			frameRect.y += speed.val[1];
		else
			frameRect.y = 0;

		Mat roi = img(frameRect);
		roi.copyTo(frame);
		//line(frame, cv::Point(0, 0), cv::Point(300, 300), Scalar(255), 4);
		putText(frame, imageEXIF.Model, cv::Point(10, 300), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);
		imshow("Test", frame);

		video.write(frame);

		frameCnt++;
		int key = waitKey(5);
		//char c = (char)waitKey(5); c == 27;//esc
		if (key > 0 || frameCnt > fps*vidDur)
		{
			cout << "\nPressed: " << key << endl;
			break;
		}
	}

	video.release();

	return 0;
}
