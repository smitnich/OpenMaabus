#include <iostream>
#include <SDL/SDL.h>
#include <list>

SDL_Surface *backdrop = NULL;
SDL_Rect fullScreen = { 0, 0, 640, 480 };
extern SDL_Surface *screen;
extern std::string rootPath;

enum {
	WEAPON_ACTIVE,
	WEAPON_AVAILABLE,
	WEAPON_UNAVAILABLE
};

int missileState = WEAPON_ACTIVE;
int laserState = WEAPON_AVAILABLE;
int toxinState = WEAPON_AVAILABLE;

class ImageHandler
{
public:
	SDL_Rect pos;
	SDL_Surface *image;
	ImageHandler(SDL_Surface *surface, int x, int y)
	{
		image = surface;
		pos = { x, y, surface->w, surface->h };
	}
	virtual void draw()
	{
		SDL_BlitSurface(image, NULL, screen, &pos);
	}
};

std::list<ImageHandler *> allImages;

class WeaponDisplay : public ImageHandler
{
public:
	int *weaponState;
	WeaponDisplay(SDL_Surface *surface, int x, int y, int *ws)
		: ImageHandler(surface, x, y)
	{
		weaponState = ws;
	}
	void draw() override
	{
		SDL_Rect toDraw = { 0, 0, image->w, image->h/3 };
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
};

void addImageHandler(const std::string path, int x, int y)
{
	std::string fullPath = rootPath + path;
	SDL_Surface *tmp = SDL_LoadBMP(fullPath.data());
	allImages.push_back(
		new ImageHandler(tmp, x, y));

}

void loadImages(SDL_Surface *screen)
{
	// This must be drawn first, goes behind everything else
	addImageHandler("Install/Screen/Final2.dib", 0, 0);
	// The satellite map of the island
	addImageHandler("Install/Screen/SatIslan.dib", 462, 314);
	allImages.push_back(new WeaponDisplay(SDL_LoadBMP((rootPath + "Install/Screen/MIS.dib").data()),31,51,&missileState));
	allImages.push_back(new WeaponDisplay(SDL_LoadBMP((rootPath + "Install/Screen/LASER.dib").data()), 31, 122, &laserState));
	allImages.push_back(new WeaponDisplay(SDL_LoadBMP((rootPath + "Install/Screen/TOXIN.dib").data()), 31, 191, &toxinState));

}

void drawScreen(SDL_Surface *screen)
{
	SDL_FillRect(screen, &fullScreen, SDL_MapRGB(screen->format, 0, 0, 0));
	for (ImageHandler *ih : allImages)
	{
		ih->draw();
	}
}