#include <SDL.h>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

AVFormatContext *pFormatCtx = NULL;
AVCodecContext  *pCodecCtxOrig = NULL;

SDL_Texture     *bmp;
AVPacket        packet;
int videoStream, audioStream;
AVCodecContext  *pCodecCtx = NULL;
AVFrame         *pFrame = NULL;
int             frameFinished;
struct SwsContext *sws_ctx = NULL;
SDL_Event       event;
SDL_mutex *screenMutex;
bool isVideoOpen = false;

AVRational frameRate;
#include "openAudio.h"
#include "main.h"
unsigned int startTicks = 0;
void resetTicks();

void initVideo()
{
	// Register all formats and codecs
	av_register_all();
}


int openVideo(const char *name, SDL_Surface *outputTo)
{
	int             i;
	AVCodec         *pCodec = NULL;
	float           aspect_ratio;

	SDL_Surface     *screen = outputTo;



	// Open video file
	if (avformat_open_input(&pFormatCtx, name, NULL, NULL) != 0)
		return -1; // Couldn't open file

	// Retrieve stream information
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
		return -1; // Couldn't find stream information

	// Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, name, 0);

	// Find the first video stream
	videoStream = -1;
	audioStream = -1;
	for (i = 0; i<pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO &&
			videoStream < 0) {
			videoStream = i;
		}
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO &&
			audioStream < 0) {
			audioStream = i;
		}
	}
	if (videoStream == -1)
		return -1; // Didn't find a video stream

	initAudio(audioStream, pFormatCtx);

	// Get a pointer to the codec context for the video stream
	pCodecCtxOrig = pFormatCtx->streams[videoStream]->codec;
	// Find the decoder for the video stream
	pCodec = avcodec_find_decoder(pCodecCtxOrig->codec_id);
	if (pCodec == NULL) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1; // Codec not found
	}

	// Copy context
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
		fprintf(stderr, "Couldn't copy codec context");
		return -1; // Error copying codec context
	}

	// Open codec
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
		return -1; // Could not open codec

	// Allocate video frame
	pFrame = av_frame_alloc();

	if (!renderer) {
		fprintf(stderr, "SDL: could not set video mode - exiting\n");
		exit(1);
	}

	// Allocate a place to put our YUV image on that screen
	bmp = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_YV12,
		SDL_TEXTUREACCESS_STREAMING,
		pCodecCtx->width,
		pCodecCtx->height
		);

	// initialize SWS context for software scaling
	sws_ctx = sws_getContext(pCodecCtx->width,
		pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width,
		pCodecCtx->height,
		AV_PIX_FMT_YUV420P,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
		);

	resetTicks();
	frameRate = av_guess_frame_rate(pFormatCtx, pFormatCtx->streams[videoStream], NULL);
	isVideoOpen = true;
	return 0;
}

void playVideo(const char *fileName)
{
	openVideo(fileName, getScreen());
}

int getDelay(AVPacket *packet)
{
	unsigned int pts = 0;
	if (packet->dts != AV_NOPTS_VALUE) {
		pts = av_frame_get_best_effort_timestamp(pFrame);
	}
	else {
		pts = 0;
	}
	double fr = av_q2d(frameRate);
	return pts *= (AV_TIME_BASE/fr)/1000;
}

void setTicks()
{
	startTicks = SDL_GetTicks();
}
void resetTicks()
{
	startTicks = 0;
}

double timestampToMilliseconds(int64_t ts)
{
	return ts*1000/av_q2d(frameRate);
}

bool pastTimestamp(int64_t ts)
{
	int64_t result = SDL_GetTicks() - startTicks;
	return (SDL_GetTicks()-startTicks > timestampToMilliseconds(ts));
}

static SDL_Texture *texture = NULL;

bool loadAudioOnly()
{
	while (av_read_frame(pFormatCtx, &packet) < 0) {
		if (packet.stream_index == audioStream) {
			packet_queue_put(&audioq, &packet);
		}
	}
	return true;
}

bool renderFrame(SDL_Surface *screen)
{
	if (isVideoOpen == false)
		return false;
	SDL_Rect posRect = { 165, 61, 315, 194 };
	static bool frameReady = false;
	SDL_Rect        rect;
	if (frameReady == false) {
		if (startTicks == 0)
			setTicks();
		if (av_read_frame(pFormatCtx, &packet) < 0)
			return false;
		// Is this a packet from the video stream?
		if (packet.stream_index == videoStream) {
			// Decode video frame
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
			SDL_PollEvent(&event);
			switch (event.type) {
			case SDL_QUIT:
				SDL_Quit();
				exit(0);
				break;
			default:
				break;
			}
			frameReady = true;
			return true;
		}
		else if (packet.stream_index == audioStream) {
			packet_queue_put(&audioq, &packet);
			return true;
		}
	}
	Uint8 *yPlane, *uPlane, *vPlane;
	size_t yPlaneSz, uvPlaneSz;
	int uvPitch = pCodecCtx->width / 2;

	// Allocate a place to put our YUV image on that screen
	if (texture == NULL)
	{
		texture = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_YV12,
			SDL_TEXTUREACCESS_STREAMING,
			pCodecCtx->width,
			pCodecCtx->height
			);
	}
	// set up YV12 pixel array (12 bits per pixel)
	yPlaneSz = pCodecCtx->width * pCodecCtx->height;
	uvPlaneSz = pCodecCtx->width * pCodecCtx->height / 4;
	yPlane = (Uint8*)malloc(yPlaneSz);
	uPlane = (Uint8*)malloc(uvPlaneSz);
	vPlane = (Uint8*)malloc(uvPlaneSz);
	if (!yPlane || !uPlane || !vPlane) {
		fprintf(stderr, "Could not allocate pixel buffers - exiting\n");
		exit(1);
	}

	int64_t ts = av_frame_get_best_effort_timestamp(pFrame);
	if (pastTimestamp(ts))
	{
		// Did we get a video frame?
		if (frameFinished) {
			AVFrame pict;
			pict.data[0] = yPlane;
			pict.data[1] = uPlane;
			pict.data[2] = vPlane;
			pict.linesize[0] = pCodecCtx->width;
			pict.linesize[1] = uvPitch;
			pict.linesize[2] = uvPitch;

			// Convert the image into YUV format that SDL uses
			sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
				pFrame->linesize, 0, pCodecCtx->height, pict.data,
				pict.linesize);

			SDL_UpdateYUVTexture(
				texture,
				NULL,
				yPlane,
				pCodecCtx->width,
				uPlane,
				uvPitch,
				vPlane,
				uvPitch
				);
		}
		// Free the packet that was allocated by av_read_frame
		av_packet_unref(&packet);
		frameReady = false;
	}
	if (SDL_RenderCopy(renderer, texture, NULL, &posRect) == -1)
	{
		fprintf(stderr, "Failed to render from texture: %s", SDL_GetError());
	}
	SDL_RenderPresent(renderer);
	free(yPlane);
	free(uPlane);
	free(vPlane);
	//SDL_DestroyTexture(texture);
	return true;
}

extern SDL_AudioDeviceID dev;
void closeVideo()
{
	if (!isVideoOpen)
		return;
	// Close the codec
	avcodec_close(pCodecCtx);
	avcodec_close(pCodecCtxOrig);

	// Close the video file
	avformat_close_input(&pFormatCtx);
	// Free the YUV frame
	av_frame_free(&pFrame);
	SDL_CloseAudioDevice(dev);
	isVideoOpen = false;
}