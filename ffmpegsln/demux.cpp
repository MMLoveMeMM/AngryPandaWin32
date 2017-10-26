#include "stdafx.h"
#include "demux.h"
#include<iostream>
using namespace std;
using namespace mux;

demux::demux()
{
}


demux::~demux()
{
}
int demux::yuv2rgb(const std::string& yuvpath, const std::string& rgbpath)
{
	return 0;
}
int demux::yuv2jpeg(const std::string& yuvpath, const std::string& jpegpath)
{
	AVFormatContext* pFormatCtx;
	AVOutputFormat* fmt;
	AVStream* video_st;
	AVCodecContext* pCodecCtx;
	AVCodec* pCodec;

	uint8_t* picture_buf;
	AVFrame* picture;
	AVPacket pkt;
	int y_size;
	int got_picture = 0;
	int size;

	int ret = 0;

	FILE *in_file = NULL;
	int in_w = 480, in_h = 272;
	const char* out_file = jpegpath.c_str();

	fopen_s(&in_file, yuvpath.c_str(), "rb");

	av_register_all();

	pFormatCtx = avformat_alloc_context();
	fmt = av_guess_format("mjpeg", NULL, NULL);
	pFormatCtx->oformat = fmt;
	if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_WRITE) < 0)
	{
		std::cout << "could not open output file ." << std::endl;
		return -1;
	}

	video_st = avformat_new_stream(pFormatCtx, 0);
	if (video_st == NULL)
	{
		return -1;
	}
	pCodecCtx = video_st->codec;
	pCodecCtx->codec_id = fmt->video_codec;
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;

	pCodecCtx->width = in_w;
	pCodecCtx->height = in_h;

	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = 25;

	av_dump_format(pFormatCtx, 0, jpegpath.c_str(), 1);

	pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
	if (!pCodec)
	{
		std::cout << "codec not found ." << std::endl;
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		std::cout << "could not open codec ." << std::endl;
		return -1;
	}

	picture = av_frame_alloc();
	size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
	picture_buf = (uint8_t*)av_malloc(size);
	if (!picture_buf)
	{
		return -1;
	}

	avpicture_fill((AVPicture*)picture, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width,
		pCodecCtx->height);

	avformat_write_header(pFormatCtx, NULL);

	y_size = pCodecCtx->width*pCodecCtx->height;
	av_new_packet(&pkt,y_size*3);

	if (fread(picture_buf, 1, y_size * 3 / 2, in_file) <= 0)
	{
		std::cout << "could not read input file ." << std::endl;
		return -1;
	}
	picture->data[0] = picture_buf;
	picture->data[1] = picture_buf + y_size;
	picture->data[2] = picture_buf + y_size * 5 / 4;

	ret = avcodec_encode_video2(pCodecCtx, &pkt, picture, &got_picture);
	if (ret < 0)
	{
		std::cout << "encode error . " << std::endl;
		return -1;
	}

	if (got_picture == 1)
	{
		pkt.stream_index = video_st->index;
		ret = av_write_frame(pFormatCtx, &pkt);
	}

	av_free_packet(&pkt);
	av_write_trailer(pFormatCtx);

	std::cout << "encode successfully ." << std::endl;

	if (video_st)
	{
		avcodec_close(video_st->codec);
		av_free(picture);
		av_free(picture_buf);
	}
	avio_close(pFormatCtx->pb);
	avformat_free_context(pFormatCtx);

	fclose(in_file);

	return 0;

}

int demux::simpledemuxmp4(const std::string& inpath, const std::string& vepath, const std::string& aupath)
{
	AVFormatContext *ifmt_ctx = NULL;
	AVPacket pkt;
	int ret, i;
	int videoindex = -1, audioindex = -1;

	/*
	* 注册ffmpeg所有编解码转换器
	*/
	av_register_all();
	/*
	* 打开需要处理的文件
	* 并且将文件格式信息保存到第一个参数
	*/
	if ((ret = avformat_open_input(&ifmt_ctx, inpath.c_str(), 0, 0)))
	{
		std::cout << "could not open input file ." << std::endl;
		return -1;
	}

	/*
	* 该函数可以读取一部分视音频数据并且获得一些相关的信息
	*/
	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0)
	{
		std::cout << "failed to retrieve input stream information" << std::endl;
		return -1;
	}

	videoindex = -1;
	for (i = 0; i < ifmt_ctx->nb_streams; i++)
	{
		if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
		}
		else if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioindex = i;
		}
	}

	std::cout << "input video .." << std::endl;
	/*
	* Print detailed information about the input or output format, such as
	* duration, bitrate, streams, container, programs, metadata, side data,
	* codec and time base
	*/
	av_dump_format(ifmt_ctx, 0, inpath.c_str(), 0);


	FILE *fp_audio;
	fopen_s(&fp_audio, aupath.c_str(), "wb+");
	FILE *fp_video;
	fopen_s(&fp_video, vepath.c_str(), "wb+");

	AVBitStreamFilterContext* h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");

	while (av_read_frame(ifmt_ctx, &pkt) >= 0)
	{
		if (pkt.stream_index == videoindex)
		{
			av_bitstream_filter_filter(h264bsfc, ifmt_ctx->streams[videoindex]->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
		}
		else if (pkt.stream_index == audioindex)
		{
			std::cout << "write audio packet size : " << pkt.size << "pts : " << pkt.pts << std::endl;
			fwrite(pkt.data, 1, pkt.size, fp_audio);
		}
		av_free_packet(&pkt);
	}

	av_bitstream_filter_close(h264bsfc);

	fclose(fp_audio);
	fclose(fp_video);

	avformat_close_input(&ifmt_ctx);

	if (ret < 0 && ret != AVERROR_EOF)
	{
		std::cout << "error occurred ." << std::endl;
		return -1;
	}
	std::cout << "finish ..." << std::endl;

}
int demux::demuxmp4(const std::string& inpath, const std::string& vepath, const std::string& aupath)
{

	int audioindex = -1;
	int videoindex = -1;
	int isacccodec = -1;

	av_register_all();

	FILE *vf = NULL;
	FILE *af = NULL;
	fopen_s(&vf, vepath.c_str(), "wb");
	fopen_s(&af, aupath.c_str(), "wb");

	if (!vf || !af)
	{
		std::cout << "open write file error" << std::endl;
		return -1;
	}

	AVFormatContext *fmtctx = NULL;
	AVPacket audiopack;
	if (avformat_open_input(&fmtctx, inpath.c_str(), NULL, NULL) < 0)
	{
		std::cout << "open fmtctx error" << std::endl;
		return -1;
	}

	if (avformat_find_stream_info(fmtctx, NULL) < 0)
	{
		std::cout << "find stream info error" << std::endl;
		return -1;
	}

	int streamnum = fmtctx->nb_streams;
	std::cout << "stream num is : " << streamnum << std::endl;

	for (int i = 0; i < streamnum; i++)
	{
		if (fmtctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audioindex == -1)
		{
			audioindex = i;
		}
		else if (fmtctx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO && videoindex==-1)
		{
			videoindex = i;
		}
	}

	std::cout << "audioindex is " << audioindex << std::endl;
	std::cout << "videoindex is " << videoindex << std::endl;

	AVCodecContext *codecctx = fmtctx->streams[videoindex]->codec;
	AVCodec *decode = avcodec_find_decoder(codecctx->codec_id);

	AVCodecContext *audioCodecCtx = fmtctx->streams[audioindex]->codec;
	AVCodec *audiodecode = avcodec_find_decoder(audioCodecCtx->codec_id);
	if (audiodecode->id == AV_CODEC_ID_AAC)
	{
		isacccodec = 1;
	}

	if (avcodec_open2(codecctx, decode, NULL) < 0)
	{
		return -1;
	}
	if (avcodec_open2(audioCodecCtx, audiodecode, NULL) < 0)
	{
		return -1;
	}
	std::cout << "extra data size is " << audioCodecCtx->extradata_size << std::endl;

	AVBitStreamFilterContext* bsfc = av_bitstream_filter_init("h264_mp4toannexb");
	AVBitStreamFilterContext* aacbsfc = av_bitstream_filter_init("aac_adtstoasc");
	if (!bsfc || !aacbsfc)
	{
		return -1;
	}

	AVFrame picture;
	while (!(av_read_frame(fmtctx, &audiopack)))
	{
		if (audiopack.stream_index == 1)
		{
			if (isacccodec == 1)
			{
				char bits[7] = { 0 };
				int sample_index = 0, channel = 0;
				char temp = 0;
				int length = 7 + audiopack.size;
				sample_index = (audioCodecCtx->extradata[0] & 0x07) << 1;
				temp = (audioCodecCtx->extradata[1] & 0x80);
				switch (audioCodecCtx->sample_rate)
				{
					case 44100:
					{
						sample_index = 0x7;
					}
					break;
					default:
					{
						sample_index = sample_index + (temp >> 7);
					}
					break;
				}
				channel = ((audioCodecCtx->extradata[1] - temp) & 0xff) >> 3;
				bits[0] = 0xff;
				bits[1] = 0xf1;
				bits[2] = 0x40 | (sample_index << 2) | (channel >> 2);
				bits[3] = ((channel & 0x3) << 6) | (length >> 11);
				bits[4] = (length >> 3) & 0xff;
				bits[5] = ((length << 5) & 0xff) | 0x1f;
				bits[6] = 0xfc;

				fwrite(bits, 1, 7, af);
				
			}
			fwrite(audiopack.data, 1, audiopack.size, af);
			std::cout << "audio pts is " << audiopack.pts*av_q2d(fmtctx->streams[audioindex]->time_base);
		}
		else if (audiopack.stream_index == videoindex)
		{
			AVPacket pkt = audiopack;

			int a = av_bitstream_filter_filter(bsfc, codecctx, NULL, &pkt.data, &pkt.size, audiopack.data, audiopack.size, audiopack.flags &AV_PKT_FLAG_KEY);

			if (a > 0)
			{
				audiopack = pkt;
			}
			fwrite(audiopack.data, 1, audiopack.size, vf);
			int gotfininshed = 0;
			if (avcodec_decode_video2(codecctx, &picture, &gotfininshed, &audiopack) < 0)
			{
				std::cout << "decode video error" << std::endl;
			}
			if (picture.key_frame)
			{
				std::cout << "key frame" << std::endl;
			}
			else
			{
				std::cout << "frame num is " << picture.pict_type << std::endl;
			}

		}

		av_free_packet(&audiopack);

	}

	fclose(af);
	fclose(vf);

	return 0;
}
