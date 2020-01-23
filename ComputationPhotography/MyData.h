#pragma once

#include <string>
#include "opencv2/core/core.hpp"

using namespace std;

class MyData
{
public:
	MyData();
	MyData(int);
	void  write(cv::FileStorage& fs) const;
	void read(const cv::FileNode& node);
	~MyData();   
	// Data Members
	int A;
	double X;
	string id;
};

