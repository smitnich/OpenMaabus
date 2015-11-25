#include <iostream>
#include <SDL/SDL.h>
#include <string>

#include "input.h"
#include "openAudio.h"

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

std::string rootPath = "D:/source/openmaabus/OpenMaabus/Maabus/";

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_EVERYTHING);
	screen = SDL_SetVideoMode(640, 480, 24, 0);
	if (!screen) {
		fprintf(stderr, "SDL: could not set video mode - exiting\n");
		exit(1);
	}
	initVideo();
	openVideo((rootPath + "CD1/J1-/J1-BEACH.mav").data(), screen);
	//initVideo("../Maabus/CD1/CL1/CL1-R3A.mav", screen);
	loadImages(screen);
	drawScreen(screen);
	SDL_Flip(screen);
	while (true)
	{
		if (isVideoOpen && !renderFrame(screen))
		{
			closeVideo();
			closeAudio();
			//openAmbience((rootPath + "CD1/J1-/AMB.wav").data());
		}
		if (getInput() == INPUT_QUIT)
		{
			break;
		}
	}
	closeVideo();
	return 0;
}