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
