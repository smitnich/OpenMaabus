#include <iostream>
#include <SDL.h>
#include <string>

#include "input.h"
#include "openAudio.h"
#include "location.h"
#include "openVideo.h"
#include "drawDisplay.h"

using namespace std;
SDL_Surface *screen = NULL;
SDL_Renderer *renderer;
SDL_Window *window;
std::string rootPath = "D:/source/openmaabus/OpenMaabus/Maabus/";

std::string queuedVideo;
void openEXE(const char *fileName);

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
	window = SDL_CreateWindow("Maabus",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640, 480,
		0);
	screen = SDL_CreateRGBSurface(0, 640, 480, 24, 0xFF, 0xFF, 0xFF, 0xFF );
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

	if (!renderer) {
		fprintf(stderr, "SDL: could not set video mode - exiting\n");
		exit(1);
	}
	initVideo();
	openVideo((rootPath + "CD1/J1-/J1-J2.mav").data(), screen);
	loadImages(screen); 
	openEXE((rootPath + "INSTALL/Maabus.exe").data());
	Location location("J2-");
	location.currentPos = DIR_EAST;
	while (!doExit)
	{
		drawScreen(screen);
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