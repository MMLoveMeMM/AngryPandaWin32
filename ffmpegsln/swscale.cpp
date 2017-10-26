#include "stdafx.h"
#include "swscale.h"
using namespace sws;
/*
libswscale����ʵ���˸���ͼ�����ظ�ʽ��ת����

libswscaleʹ�������ܷ��㣬����Ҫ�ĺ���ֻ��3����
��1�� sws_getContext()��ʹ�ò�����ʼ��SwsContext�ṹ�塣
��2�� sws_scale()��ת��һ֡ͼ��
��3�� sws_freeContext()���ͷ�SwsContext�ṹ�塣
����sws_getContext()Ҳ��������һ���ӿں���sws_getCachedContext()ȡ����

��ʼ������

��ʼ��SwsContext���˵���sws_getContext()֮�⻹����һ�ַ������������������ø���Ĳ������÷������õĺ���������ʾ��
��1�� sws_alloc_context()��ΪSwsContext�ṹ������ڴ档
��2�� av_opt_set_XXX()��ͨ��av_opt_set_int()��av_opt_set()���ȵ�һϵ�з�������SwsContext�ṹ���ֵ����������Ҫע�⣬SwsContext�ṹ��Ķ��忴���������Բ��ܶ����еĳ�Ա����ֱ�ӽ��и�ֵ������ͨ��av_opt_set()�����API���ܶ�����и�ֵ��
��3�� sws_init_context()����ʼ��SwsContext�ṹ�塣
���ָ��ӵķ�����������һЩsws_getContext()���ò��˵Ĳ���������˵����ͼ���YUV���ص�ȡֵ��Χ��JPEG��׼��Y��U��Vȡֵ��Χ����0-255������MPEG��׼��Yȡֵ��Χ��16-235��U��V��ȡֵ��Χ��16-240����

ͨ��av_pix_fmt_desc_get()���Ի��ָ�����ظ�ʽ��AVPixFmtDescriptor�ṹ�塣
ͨ��AVPixFmtDescriptor�ṹ����Ի�ò�ͬ���ظ�ʽ��һЩ��Ϣ������av_get_bits_per_pixel()��ͨ���ú������Ի��ָ�����ظ�ʽÿ������ռ�õı�������Bit Per Pixel����

ͼ������
SWS_BICUBIC���ܱȽϺã�SWS_FAST_BILINEAR�����ܺ��ٶ�֮����һ���Ⱥúõ�ƽ�⡣
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
	* �ú������Ի��ָ�����ظ�ʽÿ������ռ�õı�������Bit Per Pixel��,
	* av_pix_fmt_desc_get()���Ի��ָ�����ظ�ʽ��AVPixFmtDescriptor�ṹ�塣
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
	* �������ݿռ�,���ڶ�ȡʱ����һ֡�Ŀռ��С
	*/
	ret = av_image_alloc(src_data, src_linesize, src_w, src_h, src_pixfmt, 1);
	if (ret < 0)
	{
		std::cout << "could not allocate source image" << std::endl;
		return -1;
	}

	/*
	* �������ݿռ�,����д��ʱ����һ֡�Ŀռ��С
	*/
	ret = av_image_alloc(dst_data, dst_linesize, dst_w, dst_h, dst_pixfmt, 1);
	if (ret < 0)
	{
		std::cout << "" << std::endl;
		return -1;
	}

	/*
	* ΪSwsContext�ṹ������ڴ�
	*/
	img_convert_ctx = sws_alloc_context();

	/*
	* ��ӡ���е�AVOption��Ϣ������̨
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
	* ��SwsContext�еĸ��ֱ������и�ֵ
	*/
	sws_init_context(img_convert_ctx, NULL, NULL);

	while (1)
	{
		/*
		* ��ʼ��ȡÿһ֡����,������temp_buffer��
		*/
		if (fread(temp_buffer, 1, src_w*src_h*src_bpp / 8, src_file) != src_w*src_h*src_bpp / 8)
		{
			break;
		}
		/*
		* ���ո�ʽ��ͬ�����ݿ������Ѿ�������ڴ��src_data��
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
		* ����������������ز�һ��
		* ��Ҫת��һ��
		*/
		sws_scale(img_convert_ctx, src_data, src_linesize, 0, src_h, dst_data, dst_linesize);
		std::cout << "process frame : " << frame_idx << std::endl;
		frame_idx++;
		/*
		* ת����д���ļ�
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
