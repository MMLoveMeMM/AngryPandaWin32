// ffmpegsln.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<stdio.h>
#include <iostream>
#include "demux.h"
#include "filter.h"
#include "swscale.h"
#include "format.h"
using namespace std;
using namespace mux;
using namespace fter;
using namespace sws;
using namespace fmt;

extern "C"
{
#include "libavformat/avformat.h"
}

int _tmain(int argc, _TCHAR* argv[])
{

	
	std::string in_filename = "hello.flv";
	std::string out_filename_v = "hello_v.h264";
	std::string out_filename_a = "hello_a.mp3";

	//demux::simpledemuxmp4(in_filename, out_filename_v, out_filename_a);
	
	//demux::demuxmp4(in_filename, out_filename_v, out_filename_a);

	// 转出第一帧
	//demux::yuv2jpeg("ds_480x272.yuv", "445566.jpg");

	//filter::yuvfilter("ds_480x272.yuv","filter_ds_480x272.yuv");

	//swscale::yuv2rgb("ds_480x272.yuv", "ds_480x272.rgb");

	format::mp42avi("input.mp4", "input.avi");

	getchar();
	return 0;
}

