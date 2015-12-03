#include <SDL.h>
#include <SDL_ttf.h>
#include "displayClasses.h"

extern SDL_Surface *screen;

ImageHandler::ImageHandler(SDL_Surface *surface, int x, int y)
{
	image = surface;
	if (image)
		pos = { x, y, surface->w, surface->h };
	else
		pos = { x, y, 0, 0 };
}
ImageHandler::~ImageHandler()
{
	if (image)
		SDL_FreeSurface(image);
}
void ImageHandler::draw()
{
	SDL_BlitSurface(image, NULL, screen, &pos);
}

RectangleHandler::RectangleHandler(Uint32 _onColor, Uint32 _offColor, SDL_Rect _pos, int *_isOn)
	: ImageHandler(NULL, 0, 0), onColor(_onColor), offColor(_offColor), pos(_pos), isOn(_isOn)
{
}
void RectangleHandler::draw()
{
	// Hack for FillRect not supporting transparency: make sure we don't draw pure black
	if (!*isOn && offColor == 0)
		return;
	SDL_FillRect(screen, &pos, *isOn ? onColor : offColor);
}
RectangleHandler::~RectangleHandler()
{
}



ArrowDisplay::ArrowDisplay(SDL_Surface *surface, SDL_Rect _srcRect, SDL_Rect _destRect, int *_isOn) : ImageHandler(NULL, 0, 0),
		image(surface), srcRect(_srcRect), pos(_destRect), isOn(_isOn) {}
void ArrowDisplay::draw()
{
	if (*isOn)
		SDL_BlitSurface(image, &srcRect, screen, &pos);
}

WeaponDisplay::WeaponDisplay(SDL_Surface *surface, int x, int y, int *ws)
		: ImageHandler(surface, x, y)
{
	weaponState = ws;
}
void WeaponDisplay::draw()
{
	SDL_Rect toDraw = { 0, 0, image->w, image->h / 3 };
	switch (*weaponState)
	{
	case WEAPON_AVAILABLE:
		break;
	case WEAPON_ACTIVE:
		toDraw.y = 20;
		break;
	case WEAPON_UNAVAILABLE:
	default:
		toDraw.y = 40;
	}
	SDL_BlitSurface(image, &toDraw, screen, &pos);
}

BasicDisplay::BasicDisplay(SDL_Surface *surface, int x, int y, int *state, int numImages) :
		ImageHandler(surface, x, y)
{
	condition = state;
	imageHeight = surface->h / numImages;
}
void BasicDisplay::draw()
{
	SDL_Rect toDraw = { 0, *condition*imageHeight, image->w, imageHeight };
	SDL_BlitSurface(image, &toDraw, screen, &pos);
}

AnalysisDisplay::AnalysisDisplay(SDL_Surface **surface, int x, int y) : ImageHandler(NULL, x, y)
{
	toDraw = surface;
}
void AnalysisDisplay::draw()
{
	SDL_BlitSurface(*toDraw, NULL, screen, &pos);
}