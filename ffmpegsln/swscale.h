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
#include "libswscale/swscale.h"
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
	static int yuv2h264(const std::string& yuvpath, const std::string& h264paht);
};
}


