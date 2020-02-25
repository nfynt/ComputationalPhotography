#include "pch.h"

#include <iostream>
#include <string>
#include "XMLParser.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <fstream>
#include <filesystem>
#include <deque>

#include "cw1.hpp"

using namespace std;
using namespace cv;

XMLParser xml;

int man()
{
	string dataPath = "D:/UCL_coursework/ComputationalPhotography/ProjectFiles/Data/CW1/";
	//cw1::ConvertFramesToVideo(dataPath + "footage/",dataPath + "movie.avi");
	//cw1::TestCLAHE(dataPath + "footage_105.png", dataPath + "footage_105_clahe.jpg");

	//median blur test
	//Mat img = imread(dataPath + "footage/footage_036.png", IMREAD_GRAYSCALE);
	//Mat img = imread(dataPath + "test.jpg");
	//cw1::RemoveLineArtifacts(img);
	//img=cw1::homomorphic(img);
	//GaussianBlur(img, img, Size(11, 11), 20);
	//imwrite(dataPath + "footage_036_blur.png", img);

	//return 0;

	cw1::InitHomomorphicParams(360, 476);
	VideoCapture cap(dataPath + "source.avi");
	Ptr<BackgroundSubtractor> pBackSubtractor;

	//Record params
	bool record = true;
	string savePath = dataPath + "processed.mp4";
	VideoWriter videoWrite(savePath, VideoWriter::fourcc('x', '2', '6', '4'), 30, Size(476, 360));

	pBackSubtractor = createBackgroundSubtractorMOG2(10, 400, false);

	Mat frame, fgMask;
	int frameCnt = 0;
	int changeCnt = 0;
	float changeThreshold = 20000;

	//frames queue
	int frameQueueSize = 5;
	int frameAvgSize = 3;	//must be less than or equal to queue size
	deque<Mat> frameQueue;

	while (true)
	{
		cap >> frame;
		if (frame.empty()) break;
		frameCnt++;
		//frame = imread("C:/Users/shubh/Documents/Python/OpenCVPython/Images/lenna2.png");
		//Convert to grayscale
		Mat frame_gray;
		cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

		Mat frame_gray_copy;
		frame_gray.copyTo(frame_gray_copy);

		frameQueue.push_back(frame_gray_copy);

		if (frameCnt > frameQueueSize) {
			frameQueue.pop_front();
		}
		//cout << frameQueue.size() << endl;

	//Task2: Global flicker correction
		if (frameCnt > frameAvgSize)
		{
			//Frames averaging
			Mat acc(frame_gray.size(), CV_32F, Scalar());
			for (int i = 0; i < frameAvgSize; i++)
			{
				accumulate(frameQueue[frameQueue.size() - i - 1], acc);
			}
			acc.convertTo(frame_gray, CV_8U, 1.0 / frameAvgSize);

			//Histogram qualization
			//cw1::ApplyHistogramEqualization(frame_gray,false,true);

			//Mat m = cw1::homomorphic(frame_gray);
			//imshow("Homomorphic filter (DST)", m);
		}

		//Output for global flicker sample
		//if (frameCnt == 46) 
			//imwrite(dataPath + "footage_046_corr.png", frame_gray);


		//Task1: Scene Change detection algo
		pBackSubtractor->apply(frame, fgMask);

		if (countNonZero(fgMask) > changeThreshold)
		{
			changeCnt++;
			cout << "Scene change" << frameCnt << ";  change-" << countNonZero(fgMask) << endl;

			//putText(frame, "Scene: " + to_string(changeCnt), Point(10, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 1);
			//imwrite(dataPath + "footage_change" + to_string(changeCnt) + ".png", frame);
		}
		putText(frame, "Scene: " + to_string(changeCnt), Point(10, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 1);
		//putText(frame_gray, "Scene: " + to_string(changeCnt), Point(10, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 1);

		//Task3: Blotch detection and correction
		//Do the correction for scene 1 & 2 only
		if (frameCnt > frameQueueSize && changeCnt < 3)
		{
			//Mat acc(frame_gray.size(), CV_32F, Scalar());
			//for (int i = 0; i < 3; i++)
			//{
			//	accumulate(frameQueue[frameQueue.size() - i - 1], acc);
			//}
			Mat diff;
			//acc.convertTo(diff, CV_8U, 1.0 / 3);
			diff = frameQueue[frameQueueSize - 1] - frameQueue[frameQueueSize - 2];
			//cw1::CorrectBlotches(frameQueue[frameQueueSize - 1], frameQueue[frameQueueSize - 2], frame_gray, diff);
		}

		//Output for blotch detection and inpainting
		//if (frameCnt == 34)
			//imwrite(dataPath + "footage_034_inpaint.png", frame_gray);


		//Task4: Use vertical line correction only for 3rd scene
		if (changeCnt == 3) {
			cw1::RemoveLineArtifacts(frame_gray);
			//imshow("Median blur", m);
		}

		//output line artefact correction
		//if (frameCnt == 556)
			//imwrite(dataPath + "footage_line_" + to_string(frameCnt) + ".png", frame_gray);

		//Task5: Camera shake correction
		if (frameCnt > frameQueueSize && changeCnt < 3) {
			cw1::GetStabilizedFrame(frame_gray, frameQueue[frameQueueSize - 2], frame_gray, frame, false);
		}

		putText(frame_gray, "Scene: " + to_string(changeCnt), Point(10, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 1);

		imshow("Processed video", frame_gray);
		imshow("Original", frame);

		if (record)
		{
			cvtColor(frame_gray, frame_gray, COLOR_GRAY2BGR);
			videoWrite.write(frame_gray);
		}

		char c = (char)waitKey(20);
		if (c == 27)
		{
			cout << "\nEscape key pressed" << endl;
			break;
		}
	}

	videoWrite.release();
	cap.release();
	return 0;
}

