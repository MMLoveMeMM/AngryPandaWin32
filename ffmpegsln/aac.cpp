#include "stdafx.h"
#include "aac.h"

using namespace aacn;

aac::aac()
{
}


aac::~aac()
{
}

int aac::pcm2aacdetail(const std::string& pcmfile, const std::string& aacfile)
{

	AVFrame *frame;
	AVCodec *codec = NULL;
	AVPacket packet;
	AVCodecContext *codecContext;
	int readSize = 0;
	int ret = 0, getPacket;
	FILE *in_file, *out_file;
	int frameCount = 0;

	av_register_all();

	//1.������Ҫ��һ֡һ֡�����ݣ�������ҪAVFrame�ṹ
	//������һ֡���ݱ�����AVFrame�С�
	frame = av_frame_alloc();
	frame->format = AV_SAMPLE_FMT_S16;
	frame->nb_samples = 1024;
	frame->sample_rate = 11025;
	frame->channels = 2;
	fopen_s(&in_file, pcmfile.c_str(), "rb");
	frame->data[0] = (uint8_t *)av_malloc(1024 * 4);

	//2.�����������ݱ�����AVPacket�У���ˣ����ǻ���ҪAVPacket�ṹ��
	//��ʼ��packet
	memset(&packet, 0, sizeof(AVPacket));
	av_init_packet(&packet);

	//3.�����������ݣ�������Ҫ���룬�����Ҫ������
	//����ĺ����ҵ�h.264���͵ı�����
	/* find the mpeg1 video encoder */
	codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (!codec)
	{
		std::cout << "can not find codec !" << std::endl;
		return -1;
	}

	//���˱����������ǻ���Ҫ�������������Ļ������������Ʊ���Ĺ���
	codecContext = avcodec_alloc_context3(codec);
	if (!codecContext)
	{
		std::cout << "can not open codec !" << std::endl;
		return -1;
	}

	codecContext->sample_rate = frame->sample_rate;
	codecContext->channels = frame->channels;
	codecContext->sample_fmt = AV_SAMPLE_FMT_S16;// frame->format;

	codecContext->channel_layout = AV_CH_LAYOUT_STEREO;

	//׼�����˱������ͱ����������Ļ��������ڿ��Դ򿪱�������
	//���ݱ����������Ĵ򿪱�����
	if (avcodec_open2(codecContext, codec, NULL) < 0)
	{
		std::cout << "can not find open codec !" << std::endl;
		return -1;
	}

	fopen_s(&out_file,aacfile.c_str(), "w+");
	//���濪ʼ����
	while (1)
	{
		//��һ֡���ݳ���
		readSize = fread(frame->data[0], 1, 1024 * 4, in_file);
		if (readSize == 0)
		{
			std::cout << "end of file" << std::endl;
			frameCount++;
			break;
		}

		av_init_packet(&packet);
		frame->pts = frameCount;
		ret = avcodec_encode_audio2(codecContext, &packet, frame, &getPacket);
		if (ret < 0)
		{
			std::cout << "error encoding frame !" << std::endl;
			return -1;
		}

		if (getPacket)
		{
			frameCount++;
			std::cout << "write frame : " << frameCount << " packet size : " << packet.size << std::endl;
			fwrite(packet.data, 1, packet.size, out_file);
			av_packet_unref(&packet);
		}

	}

	for (getPacket = 1; getPacket; frameCount++)
	{
		frame->pts = frameCount;
		//�����������ʣ�������
		ret = avcodec_encode_audio2(codecContext, &packet, NULL, &getPacket);
		if (ret < 0)
		{
			std::cout << "error encoding frame !" << std::endl;
			goto OUT;
		}
		if (getPacket)
		{
			fwrite(packet.data, 1, packet.size, out_file);
			av_packet_unref(&packet);
		}
	}


OUT:
	fclose(in_file);
	fclose(out_file);
	av_frame_free(&frame);
	avcodec_close(codecContext);
	av_free(codecContext);
	return 0;

}

int aac::flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index){
	int ret;
	int got_frame;
	AVPacket enc_pkt;
	if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
		CODEC_CAP_DELAY))
		return 0;
	while (1) {
		enc_pkt.data = NULL;
		enc_pkt.size = 0;
		av_init_packet(&enc_pkt);
		ret = avcodec_encode_audio2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,
			NULL, &got_frame);
		av_frame_free(NULL);
		if (ret < 0)
			break;
		if (!got_frame){
			ret = 0;
			break;
		}
		std::cout<<"Flush Encoder: Succeed to encode 1 frame! size: "<< enc_pkt.size<<std::endl;
		/* mux encoded frame */
		ret = av_write_frame(fmt_ctx, &enc_pkt);
		if (ret < 0)
			break;
	}
	return ret;
}

int aac::pcm2aac(const std::string& pcmfile, const std::string& aacfile)
{

	AVFormatContext* pfmtCtx;
	AVOutputFormat* fmt;
	AVStream* audio_st;
	AVCodecContext* pCodecCtx;
	AVCodec* pCodec;

	uint8_t* frame_buf;
	AVFrame* pFrame;
	AVPacket pkt;

	int got_frame = 0;
	int ret = 0;
	int size = 0;
	
	FILE *in_file = NULL;
	int framenum = 1000;
	const char* out_file = aacfile.c_str();
	int i;

	fopen_s(&in_file, pcmfile.c_str(), "rb");

	av_register_all();

	pfmtCtx = avformat_alloc_context();
	fmt = av_guess_format(NULL, out_file, NULL);
	pfmtCtx->oformat = fmt;

	if (avio_open(&pfmtCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0){
		std::cout << "failed to open output file !" << std::endl;
		return -1;
	}

	audio_st = avformat_new_stream(pfmtCtx, 0);
	if (audio_st == NULL){
		return -1;
	}

	pCodecCtx = audio_st->codec;
	pCodecCtx->codec_id = fmt->audio_codec;
	pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
	pCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
	pCodecCtx->sample_rate = 44100;
	pCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
	pCodecCtx->channels = av_get_channel_layout_nb_channels(pCodecCtx->channel_layout);
	pCodecCtx->bit_rate = 64000;

	av_dump_format(pfmtCtx, 0, out_file, 1);

	pCodec = avcodec_find_encoder(pCodecCtx->codec_id);

	if (!pCodec){
		std::cout << "can not find encoder !" << std::endl;
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0){
		std::cout << "failed to open encoder!" << std::endl;
		return -1;
	}

	pFrame = av_frame_alloc();
	pFrame->nb_samples = pCodecCtx->frame_size;
	pFrame->format = pCodecCtx->sample_fmt;

	size = av_samples_get_buffer_size(NULL, pCodecCtx->channels, pCodecCtx->frame_size, pCodecCtx->sample_fmt, 1);
	frame_buf = (uint8_t*)av_malloc(size);
	avcodec_fill_audio_frame(pFrame, pCodecCtx->channels, pCodecCtx->sample_fmt, (const uint8_t*)frame_buf,size,1);

	avformat_write_header(pfmtCtx, NULL);
	av_new_packet(&pkt, size);

	for (i = 0; i < framenum; i++)
	{
	
		if (fread(frame_buf, 1, size, in_file) < 0)
		{
			std::cout << "failed to read raw data !" << std::endl;
			return -1;
		}
		else if (feof(in_file))
		{
			break;
		}

		pFrame->data[0] = frame_buf;

		pFrame->pts = i * 100;
		got_frame = 0;
		
		ret = avcodec_encode_audio2(pCodecCtx, &pkt, pFrame, &got_frame);
		if (ret < 0)
		{
			std::cout << "failed to encode !" << std::endl;
			return -1;
		}
		if (got_frame == 1)
		{
			std::cout << "success to encode 1 frame size : " << pkt.size << std::endl;
			pkt.stream_index = audio_st->index;
			ret = av_write_frame(pfmtCtx, &pkt);
			av_free_packet(&pkt);
		}

	}

	ret = flush_encoder(pfmtCtx, 0);
	if (ret < 0)
	{
		std::cout << "flushing encoder failed !" << std::endl;
		return -1;
	}

	av_write_trailer(pfmtCtx);

	if (audio_st)
	{
		avcodec_close(audio_st->codec);
		av_free(pFrame);
		av_free(frame_buf);
	}

	avio_close(pfmtCtx->pb);
	avformat_free_context(pfmtCtx);

	fclose(in_file);

	return 0;
}
