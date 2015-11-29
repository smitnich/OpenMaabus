#include <iostream>
#include <SDL/SDL.h>
#include <string>

#include "input.h"
#include "openAudio.h"
#include "location.h"

extern bool isVideoOpen;
void initVideo();
int openVideo(const char *name, SDL_Surface *outputTo);
bool renderFrame(SDL_Surface *screen);
void closeVideo();
void closeAudio();
using namespace std;
void drawScreen(SDL_Surface *screen);
void loadImages(SDL_Surface *screen);
SDL_Surface *screen = NULL;
int openAmbience(const char *name);
bool renderSample();

std::string rootPath = "D:/source/openmaabus/OpenMaabus/Maabus/";

std::string queuedVideo;

void playQueuedVideo()
{
	if (queuedVideo.size() == 0)
	{
		return;
	}
	closeAudio();
	closeVideo();
	openVideo(queuedVideo.data(), screen);
	queuedVideo.erase();
}

int main(int argc, char *argv[])
{
	bool doExit = false;
	SDL_Init(SDL_INIT_EVERYTHING);
	screen = SDL_SetVideoMode(640, 480, 24, 0);
	if (!screen) {
		fprintf(stderr, "SDL: could not set video mode - exiting\n");
		exit(1);
	}
	initVideo();
	openVideo((rootPath + "CD1/J1-/J1-J2.mav").data(), screen);
	loadImages(screen);
	drawScreen(screen);
	SDL_Flip(screen);
	Location location("J2-");
	location.currentPos = DIR_EAST;
	while (!doExit)
	{
		playQueuedVideo();
		if (isVideoOpen && !renderFrame(screen))
		{
			closeVideo();
			closeAudio();
			openAmbience((rootPath + "CD1/J2-/AMB.wav").data());
		}
		int input = getInput();
		switch (input) {
		case INPUT_QUIT:
		{
			doExit = true;
			break;
		}
		case INPUT_FORWARD:
		case INPUT_REVERSE:
		case INPUT_RIGHT:
		case INPUT_LEFT:
			location.handleInput(input);
			break;
		}
	}
	closeVideo();
	return 0;
}
SDL_Surface *getScreen()
{
	return screen;
}