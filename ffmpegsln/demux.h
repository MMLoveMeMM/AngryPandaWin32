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
}
using namespace std;
namespace mux
{

class demux
{
public:
	demux();
	~demux();
public:
	static int simpledemuxmp4(const std::string& inpath, const std::string& vepath, const std::string& aupath);
	static int demuxmp4(const std::string& inpath, const std::string& vepath, const std::string& aupath);
	static int yuv2jpeg(const std::string& yuvpath, const std::string& jpegpath);
	
};

}

