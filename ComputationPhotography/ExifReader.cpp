#include "pch.h"
#include "ExifReader.h"


EXIFInfo ExifReader::GetImageInfo(string filePath)
{
	TinyEXIF::EXIFInfo info;
	try {
		//Exif data
		cout << "\nReading exif file: " + filePath + "\n";
		ifstream file(filePath, ifstream::in | ifstream::binary);
		file.seekg(0, std::ios::end);
		std::streampos length = file.tellg();
		file.seekg(0, std::ios::beg);
		std::vector<uint8_t> data(length);
		file.read((char*)data.data(), length);

		info =EXIFInfo(data.data(), length);
	}
	catch (exception ex) {
		std::cout << "Exception: " << ex.what() << endl;
	}


	return info;
}

ExifReader::ExifReader()
{
}


ExifReader::~ExifReader()
{
}
