#include "pch.h"
#include "MyData.h"


MyData::MyData(int n) : A(n), X(CV_PI), id("mydata1234") // explicit to avoid implicit conversion
{}

void MyData::write(cv::FileStorage& fs) const                        //Write serialization for this class
{
	fs << "{" << "A" << A << "X" << X << "id" << id << "}";
}

void MyData::read(const cv::FileNode& node)                          //Read serialization for this class
{
	A = (int)node["A"];
	X = (double)node["X"];
	id = (string)node["id"];
}

MyData::MyData() : A(0), X(0), id()
{}


MyData::~MyData()
{}
