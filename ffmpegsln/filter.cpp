#include "stdafx.h"
#include "filter.h"
using namespace fter;

filter::filter()
{
}


filter::~filter()
{
}

/*
加特效步骤 :
avfilter_register_all()：注册所有AVFilter。
avfilter_graph_alloc()：为FilterGraph分配内存。
avfilter_graph_create_filter()：创建并向FilterGraph中添加一个Filter。
avfilter_graph_parse_ptr()：将一串通过字符串描述的Graph添加到FilterGraph中。
avfilter_graph_config()：检查FilterGraph的配置。
av_buffersrc_add_frame()：向FilterGraph中加入一个AVFrame。
av_buffersink_get_frame()：从FilterGraph中取出一个AVFrame。
*/
int filter::yuvfilter(const std::string& yuvpath, const std::string& filterpath)
{

	int ret;
	AVFrame *frame_in;
	AVFrame *frame_out;
	unsigned char *frame_buffer_in;
	unsigned char *frame_buffer_out;

	AVFilterContext *buffersink_ctx;
	AVFilterContext *buffersrc_ctx;
	AVFilterGraph *filter_graph;
	static int video_stream_index = -1;

	FILE *fp_in;
	fopen_s(&fp_in, yuvpath.c_str(), "rb+");
	if (fp_in == NULL)
	{
		std::cout << "error open input file" << std::endl;
		return -1;
	}

	int in_width = 1920;
	int in_height = 1080;

	FILE *fp_out;
	fopen_s(&fp_out, filterpath.c_str(), "wb+");
	if (fp_out == NULL)
	{
		std::cout << "error open output file" << std::endl;
	}

	//const char *filter_descr = "lutyuv='u=128:v=128'";
	const char *filter_descr = "boxblur";
	//const char *filter_descr = "hflip";
	//const char *filter_descr = "hue='h=60:s=-3'";
	//const char *filter_descr = "crop=2/3*in_w:2/3*in_h";
	//const char *filter_descr = "drawbox=x=100:y=100:w=100:h=100:color=pink@0.5";
	//const char *filter_descr = "drawtext=fontfile=simhei.ttf:fontcolor=red:fontsize=50:text='Shuo.Wang'";

	avfilter_register_all();//注册所有AVFilter。

	char args[512];
	AVFilter *buffersrc = avfilter_get_by_name("buffer");
	AVFilter *buffersink = avfilter_get_by_name("ffbuffersink");
	AVFilterInOut *outputs = avfilter_inout_alloc();
	AVFilterInOut *inputs = avfilter_inout_alloc();
	enum PixelFormat pix_fmts[] = { PIX_FMT_YUV420P, PIX_FMT_NONE };
	AVBufferSinkParams *buffersink_params;

	filter_graph = avfilter_graph_alloc();//为FilterGraph分配内存。

	/* buffer video source: the decoded frames from the decoder will be inserted here. */
	_snprintf_s(args, sizeof(args),
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		in_width, in_height, PIX_FMT_YUV420P,
		1, 25, 1, 1);

	//创建并向FilterGraph中添加一个Filter。
	ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
		args, NULL, filter_graph);
	if (ret < 0) {
		printf("Cannot create buffer source\n");
		return ret;
	}

	/* buffer video sink: to terminate the filter chain. */
	buffersink_params = av_buffersink_params_alloc();
	buffersink_params->pixel_fmts = pix_fmts;
	ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
		NULL, buffersink_params, filter_graph);
	av_free(buffersink_params);
	if (ret < 0) {
		printf("Cannot create buffer sink\n");
		return ret;
	}

	outputs->name = av_strdup("in");
	outputs->filter_ctx = buffersrc_ctx;
	outputs->pad_idx = 0;
	outputs->next = NULL;

	inputs->name = av_strdup("out");
	inputs->filter_ctx = buffersink_ctx;
	inputs->pad_idx = 0;
	inputs->next = NULL;

	if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_descr,
		&inputs, &outputs, NULL)) < 0)
	{
		return ret;
	}

	if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
	{
		return ret;
	}

	frame_in = av_frame_alloc();
	frame_buffer_in = (unsigned char *)av_malloc(av_image_get_buffer_size(PIX_FMT_YUV420P, in_width, in_height, 1));
	av_image_fill_arrays(frame_in->data, frame_in->linesize,
		frame_buffer_in, PIX_FMT_YUV420P, in_width, in_height, 1);

	frame_out = av_frame_alloc();
	frame_buffer_out = (unsigned char *)av_malloc(av_image_get_buffer_size(PIX_FMT_YUV420P, in_width, in_height, 1));

	av_image_fill_arrays(frame_out->data, frame_out->linesize, frame_buffer_out,
		PIX_FMT_YUV420P, in_width, in_height, 1);

	frame_in->width = in_width;
	frame_in->height = in_height;
	frame_in->format = PIX_FMT_YUV420P;

	while (1)
	{

		if (fread(frame_buffer_in, 1, in_width*in_height * 3 / 2, fp_in) != in_width*in_height * 3 / 2)
		{
			break;
		}

		//input Y,U,V
		frame_in->data[0] = frame_buffer_in;
		frame_in->data[1] = frame_buffer_in + in_width*in_height;
		frame_in->data[2] = frame_buffer_in + in_width*in_height * 5 / 4;

		if (av_buffersrc_add_frame(buffersrc_ctx, frame_in) < 0)
		{
			std::cout << "error while add frame" << std::endl;
			break;
		}

		ret = av_buffersink_get_frame(buffersink_ctx, frame_out);
		if (ret < 0)
		{
			break;
		}

		//output U,V
		if (frame_out->format == PIX_FMT_YUV420P)
		{
			for (int i = 0; i < frame_out->height; i++){
				fwrite(frame_out->data[0] + frame_out->linesize[0] * i, 1, frame_out->width, fp_out);
			}
			for (int i = 0; i < frame_out->height / 2; i++){
				fwrite(frame_out->data[1] + frame_out->linesize[1] * i, 1, frame_out->width / 2, fp_out);
			}
			for (int i = 0; i < frame_out->height / 2; i++){
				fwrite(frame_out->data[2] + frame_out->linesize[2] * i, 1, frame_out->width / 2, fp_out);
			}
		}

		std::cout << "process a frame" << std::endl;

	}

	fclose(fp_in);
	fclose(fp_out);

	av_frame_free(&frame_in);
	av_frame_free(&frame_out);
	avfilter_graph_free(&filter_graph);

	std::cout << "process finish" << std::endl;

	return 0;

}