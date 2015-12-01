#include "videoState.h"
#include "openAudio.h"
#include <string>

int quit = 0;
PacketQueue audioq;

AVCodecContext  *aCodecCtx = NULL;
AVCodec         *aCodec = NULL;
AVCodecContext  *aCodecCtxOrig = NULL;
SDL_AudioSpec   wanted_spec, spec;
SDL_Thread *audioThread;

static Uint8 audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
static Uint32 audio_buf_size = 0;
static unsigned int audio_buf_index = 0;

bool pastTimestamp(int64_t ts);

volatile bool stopRequested = false;
bool ambiencePlaying = false;
bool audioOpen = false;
void closeAmbience();

void free_packet_queue(PacketQueue *q);

SDL_AudioDeviceID dev = -1;

void closeAudio()
{
	if (ambiencePlaying)
		closeAmbience();
	memset(audio_buf, 0, (MAX_AUDIO_FRAME_SIZE * 3) / 2);
}

double get_audio_clock(VideoState *is) {
	double pts;
	int hw_buf_size, bytes_per_sec, n;

	pts = is->audio_clock; /* maintained in the audio thread */
	hw_buf_size = is->audio_buf_size - is->audio_buf_index;
	bytes_per_sec = 0;
	n = is->audio_ctx->channels * 2;
	if (is->audio_st) {
		bytes_per_sec = is->audio_ctx->sample_rate * n;
	}
	if (bytes_per_sec) {
		pts -= (double)hw_buf_size / bytes_per_sec;
	}
	return pts;
}

int initAudio(int audioStream, AVFormatContext *pFormatCtx)
{
	aCodecCtxOrig = pFormatCtx->streams[audioStream]->codec;
	aCodec = avcodec_find_decoder(aCodecCtxOrig->codec_id);
	if (!aCodec) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1;
	}

	aCodecCtxOrig = pFormatCtx->streams[audioStream]->codec;
	aCodec = avcodec_find_decoder(aCodecCtxOrig->codec_id);
	if (!aCodec) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1;
	}

	// Copy context
	aCodecCtx = avcodec_alloc_context3(aCodec);
	if (avcodec_copy_context(aCodecCtx, aCodecCtxOrig) != 0) {
		fprintf(stderr, "Couldn't copy codec context");
		return -1; // Error copying codec context
	}
	// Set audio settings from codec info
	wanted_spec.freq = aCodecCtx->sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = aCodecCtx->channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
	wanted_spec.callback = audio_callback;
	wanted_spec.userdata = aCodecCtx;
	dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &spec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	if (dev < 0) {
		fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
		return -1;
	}
	audioOpen = true;
	if (avcodec_open2(aCodecCtx, aCodec, NULL))
	{
		fprintf(stderr, "Unable to open codec");
	}

	// audio_st = pFormatCtx->streams[index]
	packet_queue_init(&audioq);
	SDL_PauseAudioDevice(dev, 0);
	return 0;
}

void packet_queue_init(PacketQueue *q) {
	free_packet_queue(q);
	memset(q, 0, sizeof(PacketQueue));
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}
int packet_queue_put(PacketQueue *q, AVPacket *pkt) {

	AVPacketList *pkt1;
	if (av_packet_ref(pkt,pkt) < 0) {
		return -1;
	}
	pkt1 = (AVPacketList*) av_malloc(sizeof(AVPacketList));
	if (!pkt1)
		return -1;
	pkt1->pkt = *pkt;
	pkt1->next = NULL;


	SDL_LockMutex(q->mutex);

	if (!q->last_pkt)
		q->first_pkt = pkt1;
	else
		q->last_pkt->next = pkt1;
	q->last_pkt = pkt1;
	q->nb_packets++;
	q->size += pkt1->pkt.size;
	SDL_CondSignal(q->cond);

	SDL_UnlockMutex(q->mutex);
	return 0;
}
void free_packet_queue(PacketQueue *q)
{
	AVPacketList *pkt1;
	if (q == NULL)
		return;
	pkt1 = q->first_pkt;
	while (pkt1 != NULL)
	{
		q->first_pkt = pkt1->next;
		av_free(pkt1);
		pkt1 = q->first_pkt;
	}
}

static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
	AVPacketList *pkt1;
	int ret;

	SDL_LockMutex(q->mutex);

	for (;;) {

		if (quit) {
			ret = -1;
			break;
		}

		pkt1 = q->first_pkt;
		if (pkt1) {
			q->first_pkt = pkt1->next;
			if (!q->first_pkt)
				q->last_pkt = NULL;
			q->nb_packets--;
			q->size -= pkt1->pkt.size;
			*pkt = pkt1->pkt;
			av_free(pkt1);
			ret = 1;
			break;
		}
		else if (!block) {
			ret = 0;
			break;
		}
		else {
			SDL_CondWait(q->cond, q->mutex);
		}
	}
	SDL_UnlockMutex(q->mutex);
	return ret;
}
extern AVFormatContext *pFormatCtx;
void audio_callback(void *userdata, Uint8 *stream, int len) {

	AVCodecContext *aCodecCtx = (AVCodecContext *)userdata;
	int len1, audio_size;
	while (len > 0) {
		if (audio_buf_index >= audio_buf_size) {
			/* We have already sent all our data; get more */
			audio_size = audio_decode_frame(aCodecCtx, audio_buf, sizeof(audio_buf));
			if (audio_size < 0) {
				/* If error, output silence */
				audio_buf_size = 1024; // arbitrary?
				memset(audio_buf, 0, audio_buf_size);
			}
			else {
				audio_buf_size = audio_size;
			}
			audio_buf_index = 0;
		}
		len1 = audio_buf_size - audio_buf_index;
		if (len1 > len)
			len1 = len;
		memcpy(stream, (uint8_t *)audio_buf + audio_buf_index, len1);
		len -= len1;
		stream += len1;
		audio_buf_index += len1;
	}
}
extern AVFormatContext *pFormatCtx;
extern AVPacket        packet;
extern int audioStream;

bool renderSample()
{
	if (av_read_frame(pFormatCtx, &packet) < 0)
		return false;
	// Is this a packet from the video stream?
	if (packet.stream_index == audioStream) {
		packet_queue_put(&audioq, &packet);
		return true;
	}
	return false;
}

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size) {

	static AVPacket pkt;
	static uint8_t *audio_pkt_data = NULL;
	static int audio_pkt_size = 0;
	static AVFrame frame;

	int len1, data_size = 0;

	for (;;) {
		while (audio_pkt_size > 0) {
			int got_frame = 0;
			len1 = avcodec_decode_audio4(aCodecCtx, &frame, &got_frame, &pkt);
			if (len1 < 0) {
				/* if error, skip frame */
				audio_pkt_size = 0;
				break;
			}
			audio_pkt_data += len1;
			audio_pkt_size -= len1;
			data_size = 0;
			if (got_frame) {
				data_size = av_samples_get_buffer_size(NULL,
					aCodecCtx->channels,
					frame.nb_samples,
					aCodecCtx->sample_fmt,
					1);
				memcpy(audio_buf, frame.data[0], data_size);
			}
			if (data_size <= 0) {
				/* No data yet, get more frames */
				continue;
			}
			/* We have data, return it and come back for more later */
			return data_size;
		}
		if (pkt.data)
			av_packet_unref(&pkt);

		if (quit) {
			return -1;
		}

		if (packet_queue_get(&audioq, &pkt, 1) < 0) {
			return -1;
		}
		audio_pkt_data = pkt.data;
		audio_pkt_size = pkt.size;
	}
}