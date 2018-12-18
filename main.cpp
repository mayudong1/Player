#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <thread>

extern "C"
{
#include <libavformat/avformat.h>	
#include <libavutil/mem.h>
}

#define STRIPCOUNT 10

int ReadThread(AVFormatContext* context)
{
	AVPacket pkt;
	int count = 0;
	while(1)
	{
		int ret = av_read_frame(context, &pkt);
		if(ret < 0)
			return -1;
		printf("pts = %lld\n", pkt.pts);
		usleep(500000);
	}
}

static void ConvertNalu(unsigned char* data, int length){
  if(data[0] == 0 && data[1] == 0 && data[2] == 0 && data[3] == 1)
    return;
  int index = 0;
  unsigned char* p = data;
  while(index < length)
  {
    int len = (p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
    if(len <= 0)
      return;
    // printf("-----len = %d, nalu type = %d\n", len, p[4]&0x1f);
    p[0] = 0;
    p[1] = 0;
    p[2] = 0;
    p[3] = 1; 
    p += (4+len);
    index += (4+len);
  }
  
}

int main()
{
	int center_index = 5;
	AVFormatContext* context = NULL;
	int ret = avformat_open_input(&context, "http://127.0.0.1:8000/dash/henry5k_clip_base.mp4", 0, 0);
	if(ret != 0)
	{
		printf("open failed\n");
		return -1;
	}
	ret = avformat_find_stream_info(context, 0);
	if(ret != 0)
	{
		printf("find stream info failed\n");
		return -1;
	}

	int count = 0;

	FILE* pFile = fopen("out.h264", "wb");
	if(pFile == NULL)
		return -1;
	unsigned char* extradata = context->streams[0]->codecpar->extradata;
	int size = context->streams[0]->codecpar->extradata_size;
////////////////////

#define AV_RB16(x)                          \
((((const uint8_t*)(x))[0] <<  8) |        \
((const uint8_t*)(x)) [1])

	uint8_t *sps= NULL, *pps = NULL;
    uint32_t sps_size = 0, pps_size = 0;
	sps_size = AV_RB16(extradata + 6);
    sps  = (uint8_t*)av_malloc(sps_size * sizeof(char));
    if(sps == NULL)
    {
        ret = -1;
        return -1;
    }
    memcpy(sps, extradata + 6 + 2, sps_size);
    
    pps_size = AV_RB16(extradata + 6 + 2 + sps_size + 1);
    pps = (uint8_t*)av_malloc(pps_size * sizeof(char));
    if(pps == NULL)
    {
        ret = -1;
        return -1;
    }
    memcpy(pps, extradata + 6 + 2 + sps_size + 1 + 2, pps_size);
///////////////////////

    char split[4] = {0, 0, 0, 1};
    fwrite(split, 4, 1, pFile);
    fwrite(sps, sps_size, 1, pFile);
    fwrite(split, 4, 1, pFile);
    fwrite(pps, pps_size, 1, pFile);

	while(1)
	{	
		AVPacket pkt;
		int ret = av_read_frame(context, &pkt);
		if(ret < 0)
			return -1;
		if(pkt.stream_index != 0)
			continue;

		// printf("pts = %lld, count = %d, size = %d\n", pkt.pts, count, pkt.size);
		ConvertNalu(pkt.data, pkt.size);
		printf("%d: %02x %02x %02x %02x %02x %02x\n", count,
			pkt.data[4], 
			pkt.data[5], pkt.data[6], pkt.data[7], pkt.data[8], pkt.data[9]);
		fwrite(pkt.data, pkt.size, 1, pFile);

		count++;
		usleep(100000);
	}
	fclose(pFile);

	return 0;
}
