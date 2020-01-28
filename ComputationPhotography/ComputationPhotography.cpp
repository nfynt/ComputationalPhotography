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

using namespace std;
using namespace cv;

string dataPath = "D:/UCL_coursework/ComputationalPhotography/ProjectFiles/Data/CW1/";
XMLParser xml;




int main()
{
	//ConvertFramesToVideo(dataPath + "footage/",dataPath + "movie.avi");


	VideoCapture cap(dataPath + "source.avi");
	Ptr<BackgroundSubtractor> pBackSubtractor;

	//pBackSubtractor = createBackgroundSubtractorKNN(10, 400, false);
	pBackSubtractor = createBackgroundSubtractorMOG2(10, 400, false);

	Mat frame, fgMask;
	int frameCnt = 0;
	int changeCnt = 0;
	float changeThreshold = 20000;

	while (true)
	{
		cap >> frame;
		if (frame.empty()) break;
		frameCnt++;


		//Scene Change detection algo
		pBackSubtractor->apply(frame, fgMask);

		if (countNonZero(fgMask) > changeThreshold)
		{
			changeCnt++;
			cout << "Scene change" << frameCnt << ";  change-" << countNonZero(fgMask) << endl;
		}

		putText(frame, "Scene: " + to_string(changeCnt), Point(10, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 1);

		imshow("Video", frame);

		char c = (char)waitKey(25);
		if (c == 27)
			break;
	}
	cap.release();
	return 0;
}

bool ConvertFramesToVideo(string framesPath, string videoPath)
{
	int fps = 30;
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
}