#pragma once

#include "TinyEXIF.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;
using namespace TinyEXIF;

class ExifReader
{
public:
	//Only JPEG files are supported
	EXIFInfo GetImageInfo(string filePath);
	ExifReader();
	~ExifReader();
};

