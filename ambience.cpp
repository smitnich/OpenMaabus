#include <SDL.h>
#include <SDL_mixer.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#include "openAudio.h"

bool ambiencePlaying = false;

Mix_Chunk *currentAmbience = NULL;

int openAmbience(const char *name)
{
	SDL_CloseAudio();
	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
	{
		return false;
	}
	currentAmbience = Mix_LoadWAV(name);
	if (!currentAmbience)
		return false;
	Mix_PlayChannel(-1, currentAmbience, -1);
	ambiencePlaying = true;
	return true;
}
void closeAmbience()
{
	Mix_CloseAudio();
	currentAmbience = NULL;
	ambiencePlaying = false;
	Mix_FreeChunk(currentAmbience);
}