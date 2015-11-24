#include <iostream>
#include <SDL/SDL.h>
#include <string>

int initVideo(const char *name, SDL_Surface *outputTo);
bool renderFrame(SDL_Surface *screen);
void closeVideo();
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
	initVideo((rootPath + "CD1/Intro.mav").data(), screen);
	//initVideo("../Maabus/CD1/CL1/CL1-R3A.mav", screen);
	loadImages(screen);
	drawScreen(screen);
	SDL_Flip(screen);
	while (renderFrame(screen));
	closeVideo();
	return 0;
}