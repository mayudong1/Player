#ifndef _DECODER_H_
#define _DECODER_H_

#define __STDC_CONSTANT_MACROS

extern "C"
{
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}

#include <string>


class Decoder{
public:
	Decoder();
	~Decoder();

	int Open(const std::string url);
	int GetFrame(AVFrame* frame);

private:
	AVFormatContext* context;
	AVCodecContext* dec_ctx;
};

#endif