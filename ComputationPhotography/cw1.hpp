#pragma once


#include <iostream>
#include <string>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

namespace cw1 {
	void ApplyHistogramEqualization(Mat& srcImg, bool colorImage = false, bool adaptive = false);
	bool TestCLAHE(string srcPath, string savePath);
	void RemoveLineArtifacts(Mat& srcImg);
	bool ConvertFramesToVideo(string framesPath, string videoPath, int fps=30);
	void ApplyGammaCorrection(Mat& srcImg, float gamma = 2.2);
}

//Contrast Limited Adaptive Histogram Equalization
bool cw1::TestCLAHE(string srcPath, string savePath) {
	Mat img = imread(srcPath, IMREAD_GRAYSCALE);
	Ptr<CLAHE> clahe = createCLAHE();
	clahe->setClipLimit(2);

	clahe->apply(img, img);
	imwrite(savePath, img);
	return true;
}

//Histogram Equalization
void cw1::ApplyHistogramEqualization(Mat& srcImg, bool colorImage, bool adaptive)
{
	if (colorImage)
	{
		Mat ycrcb;
		if (srcImg.channels() == 3) {
			cvtColor(srcImg, ycrcb, COLOR_BGR2YCrCb);
		}
		else {
			cout << "Invalid image channels!" << endl;
			return;
		}
		vector<Mat> channels;
		split(ycrcb, channels);

		if (adaptive) {
			Ptr<CLAHE> clahe = createCLAHE();
			clahe->setClipLimit(1);
			clahe->apply(channels[0], channels[0]);
		}
		else {
			equalizeHist(channels[0], channels[0]);
		}
		cvtColor(ycrcb, ycrcb, COLOR_YUV2BGR);
		ycrcb.copyTo(srcImg);
	}
	else {
		if (adaptive) {
			Ptr<CLAHE> clahe = createCLAHE();
			clahe->setClipLimit(1);
			clahe->apply(srcImg, srcImg);
		}
		else {
			equalizeHist(srcImg, srcImg);
		}
	}
}

void cw1::RemoveLineArtifacts(Mat& srcImg)
{
	long cols = srcImg.rows*srcImg.cols;
	//create 1D array
	Mat img(1, cols, CV_32F);
	int r, c;
	Scalar s;
	//Store pixel values in row major to remove vertical line artifacts
	for (long i = 0; i < cols; i++)
	{
		r = i / srcImg.cols;
		c = i - r * srcImg.cols;
		//cout << i << "=" << r<<"x"<< c << endl;
		s = srcImg.at<uchar>(r, c);
		img.at<float>(0, i) = s.val[0];
	}

	for (int i = 0; i < 5; i++)
		medianBlur(img, img, 5);

	for (long i = 0; i < cols; i++)
	{
		r = i / srcImg.cols;
		c = i - r * srcImg.cols;
		srcImg.at<uchar>(r,c) = img.at<float>(0,i);
	}
	
}

//Convert png frames to mp4 video
bool cw1::ConvertFramesToVideo(string framesPath, string videoPath, int fps)
{
	int frameCnt = 0;
	int fWidth = 476;
	int fHeight = 360;

	VideoWriter video(videoPath, VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(fWidth, fHeight));

	for (const auto & entry : filesystem::directory_iterator(framesPath))
	{
		string file = entry.path().string();
		if (file.compare(file.length() - 4, 4, ".png") != 0) {
			cout << "Invalid file: " + file << endl;
			continue;
		}
		//0: grayscale; :color
		Mat frame = imread(file);
		video.write(frame);

		imshow("Video", frame);

		frameCnt++;
		cout << frameCnt << " " << frame.size << " " << file << endl;
		//waitKey(0);
	}
	video.release();
	return true;
}

//Apply gamma correction
void cw1::ApplyGammaCorrection(Mat& srcImg, float gamma)
{
	//Gamma correction LUT
	Mat lookUpTable(1, 256, CV_8U);
	uchar* p = lookUpTable.ptr();
	for (int i = 0; i < 256; ++i)
		p[i] = saturate_cast<uchar>(pow(i / 255.0, gamma) * 255.0);

	//Mat res = srcImg.clone();
	LUT(srcImg, lookUpTable, srcImg);
}

