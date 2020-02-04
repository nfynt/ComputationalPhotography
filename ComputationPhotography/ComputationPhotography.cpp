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

string dataPath = "D:/UCL_coursework/ComputationalPhotography/ProjectFiles/Data/CW1/";
XMLParser xml;

int main()
{
	//cw1::ConvertFramesToVideo(dataPath + "footage/",dataPath + "movie.avi");
	//cw1::TestCLAHE(dataPath + "footage_105.png", dataPath + "footage_105_clahe.jpg");

	//median blur test
	/*Mat img = imread(dataPath + "footage/footage_556.png", IMREAD_GRAYSCALE);
	cw1::RemoveLineArtifacts(img);
	imwrite(dataPath + "footage_556_line.png", img);
	return 0;*/

	VideoCapture cap(dataPath + "source.avi");
	Ptr<BackgroundSubtractor> pBackSubtractor;

	pBackSubtractor = createBackgroundSubtractorMOG2(10, 400, false);

	Mat frame, fgMask;
	int frameCnt = 0;
	int changeCnt = 0;
	float changeThreshold = 20000;
	
	
	//frames queue
	int frameQueueSize = 3;
	deque<Mat> frameQueue;

	while (true)
	{
		cap >> frame;
		if (frame.empty()) break;
		frameCnt++;

		//Convert to grayscale
		Mat frame_gray;
		cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

		//Global flicker correction
		frameQueue.push_back(frame_gray);
		//Frames averaging
		if (frameCnt > frameQueueSize) {
			frameQueue.pop_front();
			//cout << frameQueue.size() << endl;
			Mat acc(frame_gray.size(),CV_32F,Scalar());
			//for (frameIt=frameQueue.begin(); frameIt!=frameQueue.end(); ++frameIt)
			for(int i=0;i<frameQueueSize;i++)
			{
				accumulate(frameQueue[i], acc);
			}
			acc.convertTo(frame_gray, CV_8U, 1.0 / frameQueueSize);
		}

		//Scene Change detection algo
		pBackSubtractor->apply(frame, fgMask);

		if (countNonZero(fgMask) > changeThreshold)
		{
			changeCnt++;
			cout << "Scene change" << frameCnt << ";  change-" << countNonZero(fgMask) << endl;
		}

		putText(frame, "Scene: " + to_string(changeCnt), Point(10, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 1);

		//Use vertical line correction only for 3rd scene
		if (changeCnt == 3) {
			cw1::RemoveLineArtifacts(frame_gray);
			//imshow("Median blur", m);
		}
		imshow("Processed video", frame_gray);
		imshow("Video", frame);
		//imshow("Flicker correction", frame_gray);
		char c = (char)waitKey(25);
		if (c == 27)
		{
			cout << "\nEscape key pressed" << endl;
			break;
		}
	}
	cap.release();
	return 0;
}

