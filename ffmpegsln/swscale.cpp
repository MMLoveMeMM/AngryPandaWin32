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
	/*
	* linesize �ֱ����������ݵĳ���
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
* <1> : �ȶ���ʽ
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
	* ע�����е�ģ��
	*/
	av_register_all();
	/*
	* ����/��ȡ��ʽ������
	*/
	fmt_ctx = avformat_alloc_context();

	/*
	* ��������ļ����Ʋ�������ļ���ʽ,���ҷ���
	*/
	fmt = av_guess_format(NULL, h264paht.c_str(), NULL);

	fmt_ctx->oformat = fmt;

	/*
	* �������óɹ�֮�󴴽���AVIOContext�ṹ��
	*/
	if (avio_open(&fmt_ctx->pb, h264paht.c_str(), AVIO_FLAG_WRITE) < 0)
	{
		std::cout << "failed to open output file" << std::endl;
		return -1;
	}

	/*
	* ����һ����ͨ��,��������Ƶ��ͨ��
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
	* ��ӡ���еĸ�ʽ��Ϣ
	*/
	av_dump_format(fmt_ctx, 0, h264paht.c_str(), 1);

	/*
	* �������������,��ͼ��ȡ������
	*/
	codec = avcodec_find_encoder(codec_ctx->codec_id);
	if (!codec)
	{
		std::cout << "can not find encodec" << std::endl;
		return -1;
	}
	/*
	* �򿪱�����
	* ����򿪳ɹ�,�͸�ֵ������������
	*/
	if (avcodec_open2(codec_ctx, codec, &param) < 0)
	{
		std::cout << "fail to open encodec!" << std::endl;
		return -1;
	}
	/*
	* ��ʼ��֡����
	*/
	pFrame = av_frame_alloc();
	/*
	* ���ݸ�ʽ�ʹ�С��ȡһ֡�Ĵ�С
	*/
	picture_size = avpicture_get_size(codec_ctx->pix_fmt, codec_ctx->width, codec_ctx->height);
	/*
	* �����ڴ��С
	*/
	picture_buf = (uint8_t *)av_malloc(picture_size);
	/*
	* ��������Ϊ�Ѿ�����Ŀռ�Ľṹ��AVPicture����һ�����ڱ������ݵĿռ䣬����ṹ������һ��ָ������data[4]���������������.
	*/
	avpicture_fill((AVPicture*)pFrame, picture_buf, codec_ctx->pix_fmt, codec_ctx->width, codec_ctx->height);

	/*
	* д��Ƶ�ļ�ͷ
	*/
	avformat_write_header(fmt_ctx, NULL);
	/*
	* ��ʼ�����ڱ������ݵ�Packet������
	*/
	av_new_packet(&pkt, picture_size);

	y_size = codec_ctx->width*codec_ctx->height;

	for (int i = 0; i < framenum; i++)
	{
		/*
		* ���ļ��ж�ȡ����
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
		* ʱ���ת���ο� :
		* http://blog.csdn.net/h514434485/article/details/77619872
		* http://blog.csdn.net/neustar1/article/details/38235113
		* http://www.cnblogs.com/yinxiangpei/p/3890462.html
		* http://www.cnblogs.com/yinxiangpei/articles/3892982.html
		*/
		pFrame->pts = i*(video_st->time_base.den) / ((video_st->time_base.num) * 25);
		int got_picture = 0;
		/*
		* ���ڱ���һ֡��Ƶ����,�����浽pkt��
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
			* ���һ֡����Ƶ����
			*/
			ret = av_write_frame(fmt_ctx, &pkt);
			av_free_packet(&pkt);
		}

	}

	/*
	* flush�����ڻ����е�����֡
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