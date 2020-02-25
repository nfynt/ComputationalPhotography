#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <vector>

using namespace std;
using namespace cv;

namespace image_pyr {
	void UpSampleImage(Mat& in, Mat& out);
	void ApplyHistogramEqualization(Mat& srcImg, bool colorImage=false, bool adaptive=false);
	void DeblurImage(Mat& in, Mat& out);
}

void image_pyr::UpSampleImage(Mat& in, Mat& out)
{
	pyrUp(in, out);
}

//Histogram Equalization
void image_pyr::ApplyHistogramEqualization(Mat& srcImg, bool colorImage, bool adaptive)
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
		cvtColor(ycrcb, ycrcb, COLOR_YCrCb2BGR);
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

void calcPSF(Mat& outputImg, Size filterSize, int R)
{
	Mat h(filterSize, CV_32F, Scalar(0));
	Point point(filterSize.width / 2, filterSize.height / 2);
	circle(h, point, R, 255, -1, 8);
	Scalar summa = sum(h);
	outputImg = h / summa[0];
}


void fftshift(const Mat& inputImg, Mat& outputImg)
{
	outputImg = inputImg.clone();
	int cx = outputImg.cols / 2;
	int cy = outputImg.rows / 2;
	Mat q0(outputImg, Rect(0, 0, cx, cy));
	Mat q1(outputImg, Rect(cx, 0, cx, cy));
	Mat q2(outputImg, Rect(0, cy, cx, cy));
	Mat q3(outputImg, Rect(cx, cy, cx, cy));
	Mat tmp;
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);
	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);
}

void filter2DFreq(const Mat& inputImg, Mat& outputImg, const Mat& H)
{
	Mat planes[2] = { Mat_<float>(inputImg.clone()), Mat::zeros(inputImg.size(), CV_32F) };
	Mat complexI;
	merge(planes, 2, complexI);
	dft(complexI, complexI, DFT_SCALE);
	Mat planesH[2] = { Mat_<float>(H.clone()), Mat::zeros(H.size(), CV_32F) };
	Mat complexH;
	merge(planesH, 2, complexH);
	Mat complexIH;
	mulSpectrums(complexI, complexH, complexIH, 0);
	idft(complexIH, complexIH);
	split(complexIH, planes);
	outputImg = planes[0];
}

void calcWnrFilter(const Mat& input_h_PSF, Mat& output_G, double nsr)
{
	Mat h_PSF_shifted;
	fftshift(input_h_PSF, h_PSF_shifted);
	Mat planes[2] = { Mat_<float>(h_PSF_shifted.clone()), Mat::zeros(h_PSF_shifted.size(), CV_32F) };
	Mat complexI;
	merge(planes, 2, complexI);
	dft(complexI, complexI);
	split(complexI, planes);
	Mat denom;
	pow(abs(planes[0]), 2, denom);
	denom += nsr;
	divide(planes[0], denom, output_G);
}

void image_pyr::DeblurImage(Mat& in, Mat& out)
{
	int R = 51;
	double snr = 5200;
	// it needs to process even image only
	Rect roi = Rect(0, 0, in.cols & -2, in.rows & -2);
	//Hw calculation (start)
	Mat Hw, h;
	calcPSF(h, roi.size(), R);
	calcWnrFilter(h, Hw, 1.0 / double(snr));
	//Hw calculation (stop)
	// filtering (start)
	filter2DFreq(in(roi), out, Hw);
	// filtering (stop)
	out.convertTo(out, CV_8U);
	normalize(out, out, 0, 255, NORM_MINMAX);
}


