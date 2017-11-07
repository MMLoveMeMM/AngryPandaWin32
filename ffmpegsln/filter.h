#pragma once
#include<iostream>
extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libavfilter/buffersink.h"
}

#define ENABLE_YUV_FILE 1 
using namespace std;
namespace fter
{
class filter
{
public:
	filter();
	~filter();
public:
	static int yuvfilter(const std::string& yuvpath, const std::string& filterpath);

};
}


