#include <SDL/SDL.h>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

extern AVFormatContext *pFormatCtx;
extern AVCodecContext  *pCodecCtxOrig;
extern int videoStream, audioStream;
int initAudio(int audioStream, AVFormatContext *pFormatCtx);
void resetTicks();

int openAmbience(const char *name)
{
	int             i;
	AVCodec         *pCodec = NULL;
	float           aspect_ratio;

	// Open video file
	if (avformat_open_input(&pFormatCtx, name, NULL, NULL) != 0)
		return -1; // Couldn't open file

	// Retrieve stream information
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
		return -1; // Couldn't find stream information

	// Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, name, 0);

	// Find the first video stream
	audioStream = -1;
	for (i = 0; i<pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO &&
			audioStream < 0) {
			audioStream = i;
		}
	}
	if (videoStream == -1)
		return -1; // Didn't find a video stream

	initAudio(audioStream, pFormatCtx);
	return 0;
}

