#pragma once
#include <stdio.h>
#include<iostream>
extern "C"
{
#include "libavformat/avformat.h"
}
using namespace std;
namespace fmt
{
class format
{
public:
	format();
	~format();
public:
	static int mp42avi(const std::string& mp4path, const std::string& avipath);
};
}


