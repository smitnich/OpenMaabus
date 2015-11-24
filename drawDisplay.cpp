#include <iostream>
#include <SDL/SDL.h>
#include <list>

SDL_Surface *backdrop = NULL;
SDL_Rect fullScreen = { 0, 0, 640, 480 };
extern SDL_Surface *screen;
extern std::string rootPath;

// Availability of weapons

enum {
	WEAPON_ACTIVE,
	WEAPON_AVAILABLE,
	WEAPON_UNAVAILABLE
};

// WEAPON_ACTIVE = Weapon has ammo and is usable and is selected
// WEAPON_AVAILABLE = Weapon has ammo and is usable but is not selected
// WEAPON_UNAVILABLE = Weapon has no ammo or is not usable for this encounter
// These are used for the GUI display and not actual logic; these must 
// be updated when weapon availability changes

int missileState = WEAPON_ACTIVE;
int laserState = WEAPON_AVAILABLE;
int toxinState = WEAPON_AVAILABLE;

int missilesLeft = 4;
int lasersLeft   = 0;
int toxinLeft    = 3;
// The weapon LED displays have 19 portions despite only 3 being used;
// this sets the length using the actual amount of available ammo
int laserDisplayPortion = lasersLeft * 6;
int toxinDisplayPortion = toxinLeft * 6;

// Used for determining the length of the bar to be shown in displays

#define MAX_LENGTH 24

int radLevel = 24;
int lifeLevel = 5;
int magnetLevel = 5;
int unidLevel = 5;
// How close an attacking monster is to you; only active during attacks
int pohLevel = 24;

// Whether or not the Position button is pressed
int positionPressed = 0;
// Whether or not the Analysis button is pressed
int analysPressed = 0;

// Display of whether the target is locked during an attack
int targetLocked = 5;

// Buttons in the center
int manualButton = 0;
int lastAnalysis = 0;
int specimenStorage = 0;
int selfDestruct = 0;
// Only active when self destruct is counting down
int selfDestructCountdown = 0;

// The amount of kilometers traveled
int kmTraveled;

enum AnalysisSpeed
{
	AS_SPEED_SLOW,
	AS_SPEED_MED,
	AS_SPEED_FAST
};

int analysisSpeed = AS_SPEED_SLOW;

// The three possible speeds of analysis text scrolling
int slowAnalysisSpeed = 0;
int mediumAnalysisSpeed = 0;
int fastAnalysisSpeed = 1;

int uplinkActive = 0;

// Color used for transparency in source images
SDL_Color transparent = { 0, 0xFF, 0xFF };


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
	~ImageHandler()
	{
		if (image)
			SDL_FreeSurface(image);
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

class BasicDisplay : public ImageHandler
{
public:
	int *condition;
	int imageHeight;
	BasicDisplay(SDL_Surface *surface, int x, int y, int *state, int numImages) :
		ImageHandler(surface, x, y)
	{
		condition = state;
		imageHeight = surface->h/numImages;
	}
	void draw() override
	{
		SDL_Rect toDraw = { 0, *condition*imageHeight, image->w, imageHeight };
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

SDL_Surface *loadImage(std::string path)
{
	SDL_Surface *firstSurf = SDL_LoadBMP((rootPath + path).data());
	// Optimize the image
	SDL_Surface *newSurf = SDL_DisplayFormat(firstSurf);
	// And free the old image
	SDL_FreeSurface(firstSurf);
	// Set the transparency color
	if (SDL_SetColorKey(newSurf, SDL_SRCCOLORKEY,
		SDL_MapRGB(screen->format, transparent.r, transparent.g, transparent.b)))
	{
		fprintf(stderr, "Warning: colorkey will not be used, reason: %s\n", SDL_GetError());
	}
	return newSurf;

}

void loadImages(SDL_Surface *screen)
{
	// This must be drawn first, goes behind everything else
	addImageHandler("Install/Screen/Final2.dib", 0, 0);
	// The satellite map of the island
	addImageHandler("Install/Screen/SatIslan.dib", 462, 314);

	// Weapon availibility
	allImages.push_back(new WeaponDisplay(loadImage("Install/Screen/MIS.dib"),31,51,&missileState));
	allImages.push_back(new WeaponDisplay(loadImage("Install/Screen/LASER.dib"), 31, 122, &laserState));
	allImages.push_back(new WeaponDisplay(loadImage("Install/Screen/TOXIN.dib"), 31, 191, &toxinState));
	
	// Weapon shots left
	// Missiles use a numerical display instead of a bar
	allImages.push_back(new BasicDisplay(loadImage("Install/Screen/SMALLCD.dib"), 98, 86, &missilesLeft, 11));
	allImages.push_back(new BasicDisplay(loadImage("Install/Screen/LED.dib"), 35, 165, &laserDisplayPortion, 19));
	allImages.push_back(new BasicDisplay(loadImage("Install/Screen/LED.dib"), 35, 234, &toxinDisplayPortion, 19));

	SDL_Surface *led2 = loadImage("Install/Screen/LED2.dib");
	// Sensor Bars
	allImages.push_back(new BasicDisplay(led2, 531, 56,  &radLevel, 25));
	allImages.push_back(new BasicDisplay(led2, 531, 112, &lifeLevel, 25));
	allImages.push_back(new BasicDisplay(led2, 531, 171, &magnetLevel, 25));
	allImages.push_back(new BasicDisplay(led2, 531, 224, &unidLevel, 25));
	allImages.push_back(new BasicDisplay(led2, 243, 19,  &pohLevel, 25));

	// Center display buttons
	allImages.push_back(new BasicDisplay(loadImage("Install/Screen/CENTER1.dib"), 251, 296, &manualButton, 2));
	allImages.push_back(new BasicDisplay(loadImage("Install/Screen/CENTER2.dib"), 306, 296, &specimenStorage, 2));
	allImages.push_back(new BasicDisplay(loadImage("Install/Screen/CENTER3.dib"), 251, 331, &manualButton, 2));
	allImages.push_back(new BasicDisplay(loadImage("Install/Screen/CENTER4.dib"), 306, 331, &specimenStorage, 1));

	// Analysis speed options
	SDL_Surface *speedButton = loadImage("Install/Screen/SPEED.dib");
	allImages.push_back(new BasicDisplay(speedButton, 248, 394, &slowAnalysisSpeed, 2));
	allImages.push_back(new BasicDisplay(speedButton, 248 + (speedButton->w+3), 394, &mediumAnalysisSpeed, 2));
	allImages.push_back(new BasicDisplay(speedButton, 248 + (speedButton->w+3) * 2, 394, &fastAnalysisSpeed, 2));

	// Misc parts of the display
	allImages.push_back(new BasicDisplay(loadImage("Install/Screen/SATT.dib"), 441, 443, &positionPressed, 2));
	allImages.push_back(new BasicDisplay(loadImage("Install/Screen/ANALYS.dib"), 19, 444, &analysPressed, 2));
	allImages.push_back(new BasicDisplay(loadImage("Install/Screen/TARGET.dib"), 408, 19, &targetLocked, 6));
	allImages.push_back(new BasicDisplay(loadImage("Install/Screen/UPLINK.dib"), 280, 266, &uplinkActive, 2));
}

void drawScreen(SDL_Surface *screen)
{
	SDL_FillRect(screen, &fullScreen, SDL_MapRGB(screen->format, 0, 0, 0));
	for (ImageHandler *ih : allImages)
	{
		ih->draw();
	}
}