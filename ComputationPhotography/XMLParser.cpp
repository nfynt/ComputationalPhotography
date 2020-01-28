#include "pch.h"
#include "XMLParser.h"



//These write and read functions must be defined for the serialization in FileStorage to work
void write(cv::FileStorage& fs, const std::string&, const MyData& x)
{
	x.write(fs);
}
void read(const cv::FileNode& node, MyData& x, const MyData& default_value = MyData()) {
	if (node.empty())
		x = default_value;
	else
		x.read(node);
}

// This function will print our custom class to the console
ostream& operator<<(ostream& out, const MyData& m)
{
	out << "{ id = " << m.id << ", ";
	out << "X = " << m.X << ", ";
	out << "A = " << m.A << "}";
	return out;
}

bool XMLParser::SerializeMyData(std::string filePath, MyData data)
{
	//write
	cv::Mat R = cv::Mat_<uchar>::eye(3, 3),
		T = cv::Mat_<double>::zeros(3, 1);
	
	cv::FileStorage fs(filePath, cv::FileStorage::WRITE);

	fs << "iterationNr" << 100;
	fs << "strings" << "[";                              // text - string sequence
	fs << "image1.jpg" << "Awesomeness" << "baboon.jpg";
	fs << "]";                                           // close sequence

	fs << "Mapping";                              // text - mapping
	fs << "{" << "One" << 1;
	fs << "Two" << 2 << "}";

	fs << "R" << R;                                      // cv::Mat
	fs << "T" << T;

	fs << "MyData" << data;                                // your own data structures

	fs.release();                                       // explicit close
	//std::cout << "Writing done!\n";
	return true;
}

bool XMLParser::SerializeCameraParams(std::string filePath, CameraParams params)
{
	cv::FileStorage fs(filePath, cv::FileStorage::WRITE);

	fs << "intrinsic" << params.intrinsic;
	fs << "distCoeffs" << params.distCoeffs;
	fs << "rvecs" << params.rvecs;
	fs << "tvecs" << params.tvecs;

	fs.release();

	return true;
}

bool XMLParser::DeserializeMyData(cv::String filePath, MyData& data)
{
	//Read
	//cout << endl << "Reading: " << endl;

	cv::FileStorage fs;
	fs.open(filePath, cv::FileStorage::READ);

	int itNr;
	//fs["iterationNr"] >> itNr;
	itNr = (int)fs["iterationNr"];
	cout << itNr;
	if (!fs.isOpened())
	{
		cerr << "Failed to open " << filePath << endl;
		return false;
	}

	cv::FileNode n = fs["strings"];                         // Read string sequence - Get node
	if (n.type() != cv::FileNode::SEQ)
	{
		cerr << "strings is not a sequence! FAIL" << endl;
		return false;
	}

	cv::FileNodeIterator it = n.begin(), it_end = n.end(); // Go through the node
	for (; it != it_end; ++it)
		cout << (string)*it << endl;


	n = fs["Mapping"];                                // Read mappings from a sequence
	cout << "Two  " << (int)(n["Two"]) << "; ";
	cout << "One  " << (int)(n["One"]) << endl << endl;


	//MyData m;
	cv::Mat R, T;

	fs["R"] >> R;                                      // Read cv::Mat
	fs["T"] >> T;
	fs["MyData"] >> data;                                 // Read your own structure_

	cout << endl
		<< "R = " << R << endl;
	cout << "T = " << T << endl << endl;
	cout << "MyData = " << endl << data << endl << endl;

	//Show default behavior for non existing nodes
	//cout << "Attempt to read NonExisting (should initialize the data structure with its default).";
	//fs["NonExisting"] >> data;
	//cout << endl << "NonExisting = " << endl << data << endl;
	return true;
}

bool XMLParser::DeserializeCameraParams(cv::String filePath, CameraParams& params)
{
	cv::FileStorage fs;
	fs.open(filePath, cv::FileStorage::READ);

	if (!fs.isOpened())
	{
		cerr << "Failed to open " << filePath << endl;
		return false;
	}

	fs["intrinsic"] >> params.intrinsic;
	fs["distCoeffs"] >> params.distCoeffs;
	fs["rvecs"] >> params.rvecs;
	fs["tvecs"] >> params.tvecs;

	cout << endl << params.intrinsic.at<double>(0, 0) << "\n" << params.intrinsic.at<float>(1, 1);

	return true;
}

XMLParser::XMLParser()
{
}


XMLParser::~XMLParser()
{
}
