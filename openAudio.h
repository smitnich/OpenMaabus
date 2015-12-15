#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)

#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0

#define FF_REFRESH_EVENT (SDL_USEREVENT)
#define FF_QUIT_EVENT (SDL_USEREVENT + 1)

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000
#define VIDEO_PICTURE_QUEUE_SIZE 1

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#include <SDL.h>
#include <SDL_thread.h>
#include "videoState.h"

double get_audio_clock(VideoState *is);
int initAudio(int audioStream, AVFormatContext *pFormatCtx);
void packet_queue_init(PacketQueue *q);
void audio_callback(void *userdata, Uint8 *stream, int len);
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block);
int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size);
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block);
int packet_queue_put(PacketQueue *q, AVPacket *pkt);
void closeAudio();
int openAmbience(const char *name);
void playAmbience(const char *path);
static bool pastTimestamp(int64_t ts);

extern bool ambiencePlaying;
extern PacketQueue audioq;