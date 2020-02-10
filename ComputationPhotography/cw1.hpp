#pragma once


#include <iostream>
#include <string>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <cmath>

using namespace std;
using namespace cv;

namespace cw1 {
	void InitHomomorphicParams(int height, int width);
	cv::Mat homomorphic(const cv::Mat &src, bool isGray=true);
	cv::Mat butterworth(const cv::Mat &img, int d0, int n, int high, int low);
	//void ApplyHomomorphicFilter(Mat& srcImg);
	void CorrectBlotches(Mat prevFrame, Mat currFrame, Mat& outImg, Mat& diff);
	void ApplyHistogramEqualization(Mat& srcImg, bool colorImage = false, bool adaptive = false);
	bool TestCLAHE(string srcPath, string savePath);
	void RemoveLineArtifacts(Mat& srcImg);
	bool ConvertFramesToVideo(string framesPath, string videoPath, int fps=30);
	void ApplyGammaCorrection(Mat& srcImg, float gamma = 2.2);
	void GetStabilizedFrame(Mat currImg, Mat prevImg, Mat& outStabilizedImg, Mat& colImg, bool test);
}

void cw1::CorrectBlotches(Mat prevFrame, Mat currFrame, Mat& outImg, Mat& diff)
{
	Mat img1, img2;
	prevFrame.copyTo(img1);
	currFrame.copyTo(img2);
	//medianBlur(img1, img1, 5);
	//medianBlur(img2, img2, 5);
	GaussianBlur(img1, img1, Size(5, 5), 1.2);
	GaussianBlur(img2, img2, Size(5, 5), 1.2);
	//Mat diff = img2 - img1;

	//imshow("Diff", diff);
	threshold(diff, diff, 60, 255, THRESH_BINARY);

	//imshow("Diff_Th", diff);
	Mat motion;// type-CV_32FC2
	//pyr_scale, pyr_level,winsize,iteration,poly_n,poly_sigma
	calcOpticalFlowFarneback(img1, img2, motion, 0.5, 3, 15, 3, 5, 1.2, 0);
	
	//Viz
	Mat motionC[2], mag, ang;
	split(motion, &motionC[0]);
	cartToPolar(motionC[0], motionC[1], mag, ang, true);
	normalize(mag, mag, 0, 1, NORM_MINMAX);
	Mat _hsv[3], hsv, hsv8, bgr;
	_hsv[0] = ang;
	_hsv[1] = Mat::ones(ang.size(), CV_32F);
	_hsv[2] = mag;
	merge(_hsv, 3, hsv);
	hsv.convertTo(hsv8, CV_8U, 255.0);
	cvtColor(hsv8, bgr, COLOR_HSV2BGR);
	cvtColor(bgr, motion, COLOR_BGR2GRAY);
	
	threshold(motion, motion, 10, 255, THRESH_BINARY);

	dilate(motion, motion, Mat::ones(3, 3, CV_8UC1));

	//imshow("Motion", motion);

	diff = diff - motion;
	//imwrite(dataPath + "footage_033_mask_o.png", diff);

	//erode(diff, diff, Mat::ones(3, 3, CV_8UC1));
	dilate(diff, diff, Mat::ones(17, 17, CV_8UC1));

	//imshow("Blotches", diff);

	Mat paint;
	inpaint(outImg, diff, paint, 20, INPAINT_NS);
	
	paint.copyTo(outImg);
}

int width, height;
int dft_M = 0;
int dft_N = 0;
int yh, yl, c, d0; //gamma high, gamma low, order n, radius d0
Mat comp;

void cw1::InitHomomorphicParams(int h, int w)
{
	width = w; height = h;
	dft_M = getOptimalDFTSize(height);
	dft_N = getOptimalDFTSize(width);
	yh = 80;
	yl = 30;
	c = 30*0.1;
	d0 = 6;
}

bool FftShift(const Mat& src, Mat& dst)
{
	if (src.empty()) return true;

	const uint h = src.rows, w = src.cols;        // height and width of src-image
	const uint qh = h >> 1, qw = w >> 1;              // height and width of the quadrants

	Mat qTL(src, Rect(0, 0, qw, qh));   // define the quadrants in respect to
	Mat qTR(src, Rect(w - qw, 0, qw, qh));   // the outer dimensions of the matrix.
	Mat qBL(src, Rect(0, h - qh, qw, qh));   // thus, with odd sizes, the center
	Mat qBR(src, Rect(w - qw, h - qh, qw, qh));   // line(s) get(s) omitted.

	Mat tmp;
	hconcat(qBR, qBL, dst);                   // build destination matrix with switched
	hconcat(qTR, qTL, tmp);                   // quadrants 0 & 2 and 1 & 3 from source
	vconcat(dst, tmp, dst);

	return false;
}

//Homomorphic filter using DFT. ref- https://medium.com/@elvisdias/introduction-to-fourier-transform-with-opencv-922a79cddf36
//extra - https://blogs.mathworks.com/steve/2013/06/25/homomorphic-filtering-part-1/
cv::Mat cw1::homomorphic(const cv::Mat &src, bool isGray)
{
	Mat padded;
	src.convertTo(padded, CV_32F);

	cv::copyMakeBorder(padded, padded, dft_M - height, 0, dft_N - width, BORDER_CONSTANT, 0);
	//so we never get log 0
	cv::log(padded + 1, padded);


	dft(padded / 255, comp, DFT_COMPLEX_OUTPUT);
	
	FftShift(comp, comp);

	//DFT result
	//Mat compV[2];
	//split(comp, &compV[0]);	//real and comlex part
	//Mat img; //dft result image
	//magnitude(compV[0], compV[1],img);
	//cv::log(img, img);
	//img = 20 * img;
	//img.convertTo(img, CV_8U);
	//imshow("DFT", img);

	//homomorphic
	Mat du = cv::Mat::zeros(comp.size(), CV_32FC2);

	//H(u, v)
	for (int u = 0; u < dft_M; u++)
		for (int v = 0; v < dft_N; v++)
			du.at<float>(u, v) = sqrt((u - dft_M / 2.0)*(u - dft_M / 2.0) + (v - dft_N / 2.0)*(v - dft_N / 2.0));

	Mat du2;
	cv::multiply(du, du, du2);
	du2 /= (d0*d0);
	Mat re;
	cv::exp(-c * du2, re);
	Mat H = (yh - yl) * (1 - re) + yl;

	//S(u, v)
	Mat filtered;
	mulSpectrums(comp, H,filtered, 0);
	
	//inverse DFT(does the shift back first)
	FftShift(filtered, filtered);
	idft(filtered, filtered);
	
	//normalization to be representable
	Mat filterV[2];
	split(filtered, &filterV[0]);
	magnitude(filterV[0], filterV[1], filtered);
	normalize(filtered, filtered, 0, 1, NORM_MINMAX);

	//g(x, y) = exp(s(x, y))
	cv::exp(filtered, filtered);
	normalize(filtered, filtered, 0, 1, NORM_MINMAX);
	
	//filtered.convertTo(filtered, CV_8U);CV_32F

	return filtered;
	//return img;
}


cv::Mat cw1::butterworth(const cv::Mat &img, int d0, int n, int high, int low)
{
	cv::Mat single(img.rows, img.cols, CV_32F);
	int cx = img.rows / 2;
	int cy = img.cols / 2;
	float upper = high * 0.01;
	float lower = low * 0.01;

	for (int i = 0; i < img.rows; i++)
	{
		for (int j = 0; j < img.cols; j++)
		{
			double radius = sqrt(pow(i - cx, 2) + pow(j - cy, 2));
			single.at<float>(i, j) = ((upper - lower) * (1 / pow(d0 / radius, 2 * n))) + lower;
		}
	}
	return single;
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

//Remove vertical line artifacts with 1D median filter
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

	for (int i = 1; i <= 5; i+=2)
		medianBlur(img, img, i);

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

	VideoWriter video(videoPath, VideoWriter::fourcc('x', '2', '6', '4'), fps, Size(fWidth, fHeight));

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


//Digital video stabilization
//motion estimation -> motion smoothing -> image composition
//ref: https://www.learnopencv.com/video-stabilization-using-point-feature-matching-in-opencv/
void cw1::GetStabilizedFrame(Mat currImg, Mat prevImg, Mat& outStabilizedImg, Mat& colImg, bool test)
{
	vector<Point2f> features1, features2;
	vector<Point2f> goodFeatures1, goodFeatures2;
	vector<uchar> status;
	vector<float> err;

	Mat stabilized;

	//Find features in curr and prev Image
	goodFeaturesToTrack(prevImg, features1, 200, 0.1, 30);
	calcOpticalFlowPyrLK(prevImg, currImg, features1, features2, status, err);

	for (int i = 0; i < status.size(); i++)
	{
		if (status[i]) {
			goodFeatures1.push_back(features1[i]);
			goodFeatures2.push_back(features2[i]);
		}
	}
	
	//Test feature points
	if (test) {
		Mat fImg;
		colImg.copyTo(fImg);
		for (int i = 0; i < goodFeatures1.size(); i++)
			circle(fImg, goodFeatures1[i], 5, Scalar(0, 0, 255));
		for (int i = 0; i < goodFeatures2.size(); i++)
		{
			Point2f pt = goodFeatures2[i];
			rectangle(fImg, Rect(pt.x - 4, pt.y - 4, 8, 8), Scalar(0, 200, 200));
		}
		//imshow("Features", fImg);
		imwrite("D:/UCL_coursework/ComputationalPhotography/ProjectFiles/Data/CW1/features_map.png", fImg);
	}
	
	//scale, angle and translation
	Mat affine = estimateAffine2D(goodFeatures1, goodFeatures2);

	//In case no transform was found
	if (affine.data == NULL) {
		return;
	}

	/*TransformParam tp;
	tp.dx = affine.at<double>(0, 2);
	tp.dy = affine.at<double>(1, 2);
	tp.da = atan2(affine.at<double>(1, 0), affine.at<double>(0, 0));*/

	//warpAffine(currImg, stabilized, affine, currImg.size());
	
	//apply the transform to current frame
	warpAffine(outStabilizedImg, outStabilizedImg, affine, currImg.size(), WARP_INVERSE_MAP, BORDER_DEFAULT);
	
}

struct TransformParam
{
	TransformParam() {}
	TransformParam(double _dx, double _dy, double _da)
	{
		dx = _dx;
		dy = _dy;
		da = _da;
	}

	double dx;
	double dy;
	double da; // angle

	void getTransform(Mat &amp, Mat Tr)
	{
		
	}
};