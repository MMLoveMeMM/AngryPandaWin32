#pragma once
#include<iostream>
extern "C"
{
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
}
using namespace std;
namespace sws
{
class swscale
{
public:
	swscale();
	~swscale();
public:
	static int yuv2rgb(const std::string& yuvpath, const std::string& rgbpath);
};
}


