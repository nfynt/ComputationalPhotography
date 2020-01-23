// ComputationPhotography.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "TinyEXIF.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "MyData.h"
#include "XMLParser.h"

using namespace std;
using namespace cv;

string dataPath = "D:/UCL_coursework/ComputationalPhotography/Data/";
Vec2i speed;
XMLParser xml;

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


int main()
{
	int fWidth, fHeight;
	fWidth = fHeight = 400;
	Rect frameRect = Rect(0, 0, fWidth,fHeight);

	//Video params
	int fps = 15;
	//in seconds
	int vidDur = 30;

	Mat img = imread(dataPath+"image1.jpg");
	Mat frame;

	//Exif data
	ifstream file(dataPath + "image1.jpg", ifstream::in | ifstream::binary);
	file.seekg(0, std::ios::end);
	std::streampos length = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<uint8_t> data(length);
	file.read((char*)data.data(), length);

	// parse image EXIF and XMP metadata
	TinyEXIF::EXIFInfo imageEXIF(data.data(), length);
	if (imageEXIF.Fields)
		std::cout
		<< "Image Description " << imageEXIF.ImageDescription << "\n"
		<< "Image Resolution " << imageEXIF.ImageWidth << "x" << imageEXIF.ImageHeight << " pixels\n"
		<< "Camera Model " << imageEXIF.Make << " - " << imageEXIF.Model << "\n"
		<< "Focal Length " << imageEXIF.FocalLength << " mm" << std::endl;
	
	//Write and parse XML Data
	MyData mdat(5);
	xml.SerializeMyData(dataPath + "test.xml", mdat);

	MyData ndat;
	xml.DeserializeMyData(dataPath + "test.xml", ndat);
	cout <<endl<< ndat.A << endl;

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

		//cout << frameRect << endl;

		frame = img(frameRect);
		imshow("Test", frame);
		int key = waitKey(5);
		if (key > 0)
		{
			cout <<"\nPressed: "<< key << endl;
			break;
		}
	}

	return 0;
}
