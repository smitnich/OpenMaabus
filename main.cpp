#include <iostream>
#include <SDL.h>
#include <string>

#include "input.h"
#include "openAudio.h"
#include "location.h"
#include "openVideo.h"
#include "drawDisplay.h"
#include "mapFile.h"
#include <list>

using namespace std;
SDL_Surface *screen = NULL;
SDL_Renderer *renderer;
SDL_Window *window;
std::string rootPath = "D:/source/openmaabus/OpenMaabus/Maabus/";

std::list<std::string> queuedVideos;
void openEXE(const char *fileName);

bool playQueuedVideo()
{
	if (queuedVideos.size() == 0)
	{
		return false;
	}
	std::string queuedVideo = queuedVideos.front();
	queuedVideos.pop_front();
	closeAudio();
	closeVideo();
	openVideo(queuedVideo.data(), screen);
	return true;
}

void loadInitialVideos()
{
	queuedVideos.push_back(rootPath + "INTRO1.mav");
	queuedVideos.push_back(rootPath + "INTRO.mav");
	queuedVideos.push_back(rootPath + "INTRO2.mav");
	queuedVideos.push_back(rootPath + "INTRO3.mav");
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
	loadInitialVideos();
	loadImages(screen);
	openEXE((rootPath + "INSTALL/Maabus.exe").data());
	readMapFile((rootPath + "INSTALL/MAP.dat").data());
	Location location("J2-");
	location.currentPos = DIR_EAST;
	openVideo((rootPath + "INTRO/INTRO.mav").data(), screen);
	playQueuedVideo();
	while (!doExit)
	{
		drawScreen(screen);
		if (isVideoOpen && !renderFrame(screen))
		{
			playQueuedVideo();
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
		case INPUT_SKIP:
			playQueuedVideo();
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