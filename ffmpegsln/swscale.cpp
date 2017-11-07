#include "stdafx.h"
#include "swscale.h"
using namespace sws;
/*
libswscale里面实现了各种图像像素格式的转换。

libswscale使用起来很方便，最主要的函数只有3个：
（1） sws_getContext()：使用参数初始化SwsContext结构体。
（2） sws_scale()：转换一帧图像。
（3） sws_freeContext()：释放SwsContext结构体。
其中sws_getContext()也可以用另一个接口函数sws_getCachedContext()取代。

初始化方法

初始化SwsContext除了调用sws_getContext()之外还有另一种方法，更加灵活，可以配置更多的参数。该方法调用的函数如下所示。
（1） sws_alloc_context()：为SwsContext结构体分配内存。
（2） av_opt_set_XXX()：通过av_opt_set_int()，av_opt_set()…等等一系列方法设置SwsContext结构体的值。在这里需要注意，SwsContext结构体的定义看不到，所以不能对其中的成员变量直接进行赋值，必须通过av_opt_set()这类的API才能对其进行赋值。
（3） sws_init_context()：初始化SwsContext结构体。
这种复杂的方法可以配置一些sws_getContext()配置不了的参数。比如说设置图像的YUV像素的取值范围是JPEG标准（Y、U、V取值范围都是0-255）还是MPEG标准（Y取值范围是16-235，U、V的取值范围是16-240）。

通过av_pix_fmt_desc_get()可以获得指定像素格式的AVPixFmtDescriptor结构体。
通过AVPixFmtDescriptor结构体可以获得不同像素格式的一些信息。例如av_get_bits_per_pixel()，通过该函数可以获得指定像素格式每个像素占用的比特数（Bit Per Pixel）。

图像拉伸
SWS_BICUBIC性能比较好；SWS_FAST_BILINEAR在性能和速度之间有一个比好好的平衡。
*/

swscale::swscale()
{
}


swscale::~swscale()
{
}

int swscale::yuv2rgb(const std::string& yuvpath, const std::string& rgbpath)
{
	FILE *src_file;
	fopen_s(&src_file, yuvpath.c_str(), "rb");

	const int src_w = 1920;
	const int src_h = 1080;
	AVPixelFormat src_pixfmt = AV_PIX_FMT_YUV420P;

	/*
	* 该函数可以获得指定像素格式每个像素占用的比特数（Bit Per Pixel）,
	* av_pix_fmt_desc_get()可以获得指定像素格式的AVPixFmtDescriptor结构体。
	*/
	int src_bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(src_pixfmt));

	FILE* dst_file;
	fopen_s(&dst_file, rgbpath.c_str(), "wb");
	const int dst_w = 1280, dst_h = 720;
	AVPixelFormat dst_pixfmt = AV_PIX_FMT_RGB24;
	int dst_bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(dst_pixfmt));

	uint8_t *src_data[4];
	/*
	* linesize 分别代表各个数据的长度
	*/
	int src_linesize[4];

	uint8_t *dst_data[4];
	int dst_linesize[4];

	int rescale_method = SWS_BICUBIC;
	struct SwsContext *img_convert_ctx;
	uint8_t *temp_buffer = (uint8_t *)malloc(src_w*src_h*src_bpp / 8);

	int frame_idx = 0;
	int ret = 0;
	/*
	* 分配数据空间,用于读取时保存一帧的空间大小
	*/
	ret = av_image_alloc(src_data, src_linesize, src_w, src_h, src_pixfmt, 1);
	if (ret < 0)
	{
		std::cout << "could not allocate source image" << std::endl;
		return -1;
	}

	/*
	* 分配数据空间,用于写入时保存一帧的空间大小
	*/
	ret = av_image_alloc(dst_data, dst_linesize, dst_w, dst_h, dst_pixfmt, 1);
	if (ret < 0)
	{
		std::cout << "" << std::endl;
		return -1;
	}

	/*
	* 为SwsContext结构体分配内存
	*/
	img_convert_ctx = sws_alloc_context();

	/*
	* 打印所有的AVOption信息到控制台
	*/
	av_opt_show2(img_convert_ctx, stdout, AV_OPT_FLAG_VIDEO_PARAM, 0);
	//Set Value
	av_opt_set_int(img_convert_ctx, "sws_flags", SWS_BICUBIC | SWS_PRINT_INFO, 0);
	av_opt_set_int(img_convert_ctx, "srcw", src_w, 0);
	av_opt_set_int(img_convert_ctx, "srch", src_h, 0);
	av_opt_set_int(img_convert_ctx, "src_format", src_pixfmt, 0);
	//'0' for MPEG (Y:0-235);'1' for JPEG (Y:0-255)
	av_opt_set_int(img_convert_ctx, "src_range", 1, 0);
	av_opt_set_int(img_convert_ctx, "dstw", dst_w, 0);
	av_opt_set_int(img_convert_ctx, "dsth", dst_h, 0);
	av_opt_set_int(img_convert_ctx, "dst_format", dst_pixfmt, 0);
	av_opt_set_int(img_convert_ctx, "dst_range", 1, 0);

	/*
	* 对SwsContext中的各种变量进行赋值
	*/
	sws_init_context(img_convert_ctx, NULL, NULL);

	while (1)
	{
		/*
		* 开始读取每一帧数据,并置于temp_buffer中
		*/
		if (fread(temp_buffer, 1, src_w*src_h*src_bpp / 8, src_file) != src_w*src_h*src_bpp / 8)
		{
			break;
		}
		/*
		* 按照格式不同将数据拷贝到已经分配好内存的src_data中
		*/
		switch (src_pixfmt)
		{
			case AV_PIX_FMT_GRAY8:
			{
				memcpy(src_data[0], temp_buffer, src_w*src_h);
				break;
			}
			case AV_PIX_FMT_YUV420P:
			{
				memcpy(src_data[0], temp_buffer, src_w*src_h);                    //Y
				memcpy(src_data[1], temp_buffer + src_w*src_h, src_w*src_h / 4);      //U
				memcpy(src_data[2], temp_buffer + src_w*src_h * 5 / 4, src_w*src_h / 4);  //V
			}
			break;
			case AV_PIX_FMT_YUV422P:
			{
				memcpy(src_data[0], temp_buffer, src_w*src_h);                    //Y
				memcpy(src_data[1], temp_buffer + src_w*src_h, src_w*src_h / 2);      //U
				memcpy(src_data[2], temp_buffer + src_w*src_h * 3 / 2, src_w*src_h / 2);  //V
			}
			break;
			case AV_PIX_FMT_YUV444P:
			{
				memcpy(src_data[0], temp_buffer, src_w*src_h);                    //Y
				memcpy(src_data[1], temp_buffer + src_w*src_h, src_w*src_h);        //U
				memcpy(src_data[2], temp_buffer + src_w*src_h * 2, src_w*src_h);      //V
			}
			break;
			case AV_PIX_FMT_YUYV422:
			{
				memcpy(src_data[0], temp_buffer, src_w*src_h * 2);                  //Packed
			}
			break;
			case AV_PIX_FMT_RGB24:
			{
				memcpy(src_data[0], temp_buffer, src_w*src_h * 3);                  //Packed
			}
			break;
			default:
				break;
		}
		/*
		* 由于输入输出的像素不一样
		* 需要转换一下
		*/
		sws_scale(img_convert_ctx, src_data, src_linesize, 0, src_h, dst_data, dst_linesize);
		std::cout << "process frame : " << frame_idx << std::endl;
		frame_idx++;
		/*
		* 转换后写入文件
		*/
		switch (dst_pixfmt){
			case AV_PIX_FMT_GRAY8:
			{
				fwrite(dst_data[0], 1, dst_w*dst_h, dst_file);
				break;
			}
			case AV_PIX_FMT_YUV420P:
			{
				fwrite(dst_data[0], 1, dst_w*dst_h, dst_file);                 //Y
				fwrite(dst_data[1], 1, dst_w*dst_h / 4, dst_file);               //U
				fwrite(dst_data[2], 1, dst_w*dst_h / 4, dst_file);               //V
				break;
			}
			case AV_PIX_FMT_YUV422P:
			{
				fwrite(dst_data[0], 1, dst_w*dst_h, dst_file);                 //Y
				fwrite(dst_data[1], 1, dst_w*dst_h / 2, dst_file);               //U
				fwrite(dst_data[2], 1, dst_w*dst_h / 2, dst_file);               //V
				break;
			}
			case AV_PIX_FMT_YUV444P:
			{
				fwrite(dst_data[0], 1, dst_w*dst_h, dst_file);                 //Y
				fwrite(dst_data[1], 1, dst_w*dst_h, dst_file);                 //U
				fwrite(dst_data[2], 1, dst_w*dst_h, dst_file);                 //V
				break;
			}
			case AV_PIX_FMT_YUYV422:
			{
				fwrite(dst_data[0], 1, dst_w*dst_h * 2, dst_file);               //Packed
				break;
			}
			case AV_PIX_FMT_RGB24:
			{
				fwrite(dst_data[0], 1, dst_w*dst_h * 3, dst_file);               //Packed
				break;
			}
			default:
			{
				printf("Not Support Output Pixel Format.\n");
				break;
			}
		}

	}

	sws_freeContext(img_convert_ctx);
	free(temp_buffer);
	fclose(dst_file);
	av_freep(&src_data[0]);
	av_freep(&dst_data[0]);

	std::cout << "process finish" << std::endl;

	return 0;

}

int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index)
{
	int ret;
	int got_frame;
	AVPacket enc_pkt;

	if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities & CODEC_CAP_DELAY))
	{
		return 0;
	}

	while (1)
	{
		enc_pkt.data = NULL;
		enc_pkt.size = 0;
		av_init_packet(&enc_pkt);
		ret = avcodec_encode_audio2(fmt_ctx->streams[stream_index]->codec, &enc_pkt, NULL, &got_frame);
		av_frame_free(NULL);
		if (ret < 0)
		{
			break;
		}
		if (!got_frame)
		{
			ret = 0;
			break;
		}

		std::cout << "flush encoder : successed to encode 1 frame size : " << enc_pkt.size << std::endl;

		ret = av_write_frame(fmt_ctx, &enc_pkt);
		if (ret < 0)
		{
			break;
		}

	}

	return ret;
}

/*
* <1> : 先定格式
*/
int swscale::yuv2h264(const std::string& yuvpath, const std::string& h264paht)
{

	AVFormatContext *fmt_ctx;
	AVOutputFormat *fmt;
	AVStream *video_st;
	AVCodecContext* codec_ctx;
	AVCodec *codec;
	AVPacket pkt;
	uint8_t *picture_buf;
	AVFrame *pFrame;
	int picture_size;
	int y_size;
	int framecnt = 0;

	FILE *in_file;
	fopen_s(&in_file, yuvpath.c_str(), "rb");
	int in_w = 480, in_h = 272;
	int framenum = 100;

	/*
	* 注册所有的模块
	*/
	av_register_all();
	/*
	* 分配/获取格式上下文
	*/
	fmt_ctx = avformat_alloc_context();

	/*
	* 根据输出文件名推测输出的文件格式,并且返回
	*/
	fmt = av_guess_format(NULL, h264paht.c_str(), NULL);

	fmt_ctx->oformat = fmt;

	/*
	* 函数调用成功之后创建的AVIOContext结构体
	*/
	if (avio_open(&fmt_ctx->pb, h264paht.c_str(), AVIO_FLAG_WRITE) < 0)
	{
		std::cout << "failed to open output file" << std::endl;
		return -1;
	}

	/*
	* 创建一个流通道,这里是视频流通道
	*/
	video_st = avformat_new_stream(fmt_ctx, 0);

	if (video_st == NULL)
	{
		return -1;
	}

	codec_ctx = video_st->codec;
	codec_ctx->codec_id = fmt->video_codec;
	codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
	codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	codec_ctx->width = in_w;
	codec_ctx->height = in_h;
	codec_ctx->bit_rate = 40 * 1000;
	codec_ctx->gop_size = 250;

	codec_ctx->time_base.num = 1;
	codec_ctx->time_base.den = 25;

	codec_ctx->qmin = 10;
	codec_ctx->qmax = 15;

	codec_ctx->max_b_frames = 3;

	AVDictionary *param = 0;
	if (codec_ctx->codec_id == AV_CODEC_ID_H264)
	{
		av_dict_set(&param,"preset","slow",0);
		av_dict_set(&param, "tune", "zerolatency", 0);
	}

	if (codec_ctx->codec_id == AV_CODEC_ID_H265)
	{
		av_dict_set(&param, "preset", "ultrafast", 0);
		av_dict_set(&param, "tune", "zero-latency", 0);
	}

	/*
	* 打印所有的格式信息
	*/
	av_dump_format(fmt_ctx, 0, h264paht.c_str(), 1);

	/*
	* 根据上面的配置,试图获取编码器
	*/
	codec = avcodec_find_encoder(codec_ctx->codec_id);
	if (!codec)
	{
		std::cout << "can not find encodec" << std::endl;
		return -1;
	}
	/*
	* 打开编码器
	* 如果打开成功,就赋值给编码上下文
	*/
	if (avcodec_open2(codec_ctx, codec, &param) < 0)
	{
		std::cout << "fail to open encodec!" << std::endl;
		return -1;
	}
	/*
	* 初始化帧缓存
	*/
	pFrame = av_frame_alloc();
	/*
	* 根据格式和大小获取一帧的大小
	*/
	picture_size = avpicture_get_size(codec_ctx->pix_fmt, codec_ctx->width, codec_ctx->height);
	/*
	* 分配内存大小
	*/
	picture_buf = (uint8_t *)av_malloc(picture_size);
	/*
	* 本质上是为已经分配的空间的结构体AVPicture挂上一段用于保存数据的空间，这个结构体中有一个指针数组data[4]，挂在这个数组里.
	*/
	avpicture_fill((AVPicture*)pFrame, picture_buf, codec_ctx->pix_fmt, codec_ctx->width, codec_ctx->height);

	/*
	* 写视频文件头
	*/
	avformat_write_header(fmt_ctx, NULL);
	/*
	* 初始化用于保存数据的Packet包而已
	*/
	av_new_packet(&pkt, picture_size);

	y_size = codec_ctx->width*codec_ctx->height;

	for (int i = 0; i < framenum; i++)
	{
		/*
		* 从文件中读取数据
		*/
		if (fread(picture_buf, 1, y_size * 3 / 2, in_file) <= 0)
		{
			std::cout << "failed to read raw data !" << std::endl;
			return -1;
		}
		else if (feof(in_file))
		{
			break;
		}

		pFrame->data[0] = picture_buf;
		pFrame->data[1] = picture_buf + y_size;
		pFrame->data[2] = picture_buf + y_size * 5 / 4;
		/*
		* 时间戳转换参考 :
		* http://blog.csdn.net/h514434485/article/details/77619872
		* http://blog.csdn.net/neustar1/article/details/38235113
		* http://www.cnblogs.com/yinxiangpei/p/3890462.html
		* http://www.cnblogs.com/yinxiangpei/articles/3892982.html
		*/
		pFrame->pts = i*(video_st->time_base.den) / ((video_st->time_base.num) * 25);
		int got_picture = 0;
		/*
		* 用于编码一帧视频数据,并保存到pkt中
		*/
		int ret = avcodec_encode_video2(codec_ctx, &pkt, pFrame, &got_picture);
		if (ret < 0)
		{
			std::cout << "failed to encode !" << std::endl;
			return -1;
		}

		if (got_picture == 1)
		{
			std::cout << "success to encode frame : " << framecnt << "\t size : " << pkt.size << std::endl;
			framecnt++;
			pkt.stream_index = video_st->index;
			/*
			* 输出一帧视音频数据
			*/
			ret = av_write_frame(fmt_ctx, &pkt);
			av_free_packet(&pkt);
		}

	}

	/*
	* flush还存在缓存中的数据帧
	*/
	int ret = flush_encoder(fmt_ctx,0);
	if (video_st)
	{
		avcodec_close(video_st->codec);
		av_free(pFrame);
		av_free(picture_buf);
	}

	avio_close(fmt_ctx->pb);
	avformat_free_context(fmt_ctx);

	fclose(in_file);

	return 0;

}