struct SDL_Surface;

extern bool isVideoOpen;
void initVideo();
int openVideo(const char *name, SDL_Surface *outputTo);
bool renderFrame(SDL_Surface *screen);
void closeVideo();
void resetTicks();
bool loadAllFrames();
bool loadAudioOnly();

extern AVFormatContext *pFormatCtx;
extern AVCodecContext  *pCodecCtxOrig;
extern int videoStream, audioStream;