#include "Decoder.h"
#include <stdio.h>
#include <iostream>

Decoder::Decoder(){
	context = NULL;
	dec_ctx = NULL;
}

Decoder::~Decoder(){

}

int Decoder::Open(const std::string url){
	context = NULL;
    int ret = avformat_open_input(&context, url.c_str(), 0, 0);
    if(ret != 0){
        std::cout << "open url failed" << std::endl;
        return -1;
    }

    ret = avformat_find_stream_info(context, 0);
    if(ret != 0){
        std::cout << "find stream info failed" << std::endl;
        return -1;
    }

    AVStream* st = context->streams[0];
    AVCodec* dec = avcodec_find_decoder(st->codecpar->codec_id);
    if(!dec){
        std::cout << "can not find codec" << std::endl;
        return -1;
    }
    dec_ctx = avcodec_alloc_context3(dec);
    if(!dec_ctx){
        std::cout << "alloc decode context failed" << std::endl;
        return -1;
    }
    ret = avcodec_parameters_to_context(dec_ctx, st->codecpar);
    if(ret < 0){
        std::cout << "avcodec_parameters_to_context failed" << std::endl;
        return -1;
    }
    ret = avcodec_open2(dec_ctx, dec, NULL);
    if(ret < 0){
        std::cout << "open codec failed" << std::endl;
        return -1;
    }

	return 0;
}

int Decoder::GetFrame(AVFrame* frame){
	if(frame == NULL){
		return -1;
	}

	int ret = 0;
	AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
    
    int got_frame = 0;

    while(1){
    	ret = av_read_frame(context, &pkt);
        if(ret < 0)
        {
            std::cout << "read frame failed" << std::endl;
            break;
        }
        else
        {
            if(pkt.stream_index != 0)
                continue;

            ret = avcodec_decode_video2(dec_ctx, frame, &got_frame, &pkt);
            if(ret < 0)
            {
                std::cout << "decode failed" << std::endl;
                continue;
            }

        }
        if(!got_frame)
        {
            std::cout << "not decode image" << std::endl;
            continue;
        }else{
            break;
        }
    }
	return 0;
}



