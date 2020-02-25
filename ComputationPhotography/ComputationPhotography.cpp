#include "pch.h"

#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include <cmath>
#include "ImagePyr.hpp"

using namespace std;
using namespace cv;

//pendulum0000.jpeg
string dataPath = "D:/UCL_coursework/ComputationalPhotography/ProjectFiles/Data/lab5/";


int main()
{

	Mat distMatrix = Mat::zeros(Size(66, 66), CV_32F);
	int ind = 0;
	vector<Mat> frames;
	string filePath = dataPath + "pendulum/";
	for (const auto & entry : filesystem::directory_iterator(filePath))
	{
		string file = entry.path().string();
		if (file.compare(file.length() - 5, 5, ".jpeg") != 0) {
			cout << "Invalid file: " + file << endl;
		}
		cout << "reading... "<<file << endl;
		ind++;
		Mat frame = imread(file);
		frames.push_back(frame);

		Mat m;

		for (int i = 0; i < ind; i++)
		{
			m = frames[ind - 1] - frames[i];
			
			//m.convertTo(m, CV_32FC3);

			double sum = 0;

			for (int j = 0; j < 240; j++)
			{
				for (int k = 0; k < 240; k++)
				{
					Vec3d val = m.at<Vec3b>(j, k);
					sum += val.val[0] * val.val[0] + val.val[1] * val.val[1] + val.val[2] * val.val[2];
				}
			}

			sum = sqrt(sum);

			sum /= 240 * 240 * 3;


			cout << ind << " " << sum << endl;

			//m = norm(frames[ind - 1], frames[i]);

			distMatrix.at<float>(ind - 1, i) = distMatrix.at<float>(i, ind-1) = sum;

			imshow("diff", distMatrix);
		}

	}

	distMatrix.convertTo(distMatrix, CV_8U);
	imwrite(dataPath + "distMAtrix.jpeg", distMatrix);

	return 0;
}