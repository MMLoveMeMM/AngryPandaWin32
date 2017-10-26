// ffmpegsln.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<stdio.h>
#include <iostream>
#include "demux.h"
using namespace std;
using namespace mux;

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
	demux::yuv2jpeg("ds_480x272.yuv", "445566.jpg");

	getchar();
	return 0;
}

