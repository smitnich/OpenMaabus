#include <iostream>
#include <SDL.h>
#include <list>
#include <SDL_ttf.h>

#include "gameplay.h"
#include "displayClasses.h"

SDL_Surface *backdrop = NULL;
SDL_Rect fullScreen = { 0, 0, 640, 480 };
#include "videoState.h"
#include "main.h"

// WEAPON_ACTIVE = Weapon has ammo and is usable and is selected
// WEAPON_AVAILABLE = Weapon has ammo and is usable but is not selected
// WEAPON_UNAVILABLE = Weapon has no ammo or is not usable for this encounter
// These are used for the GUI display and not actual logic; these must 
// be updated when weapon availability changes

int missileState = WEAPON_ACTIVE;
int laserState = WEAPON_AVAILABLE;
int toxinState = WEAPON_AVAILABLE;

// Green when weapons are availible during an encounter, red if we aren't in
// an encounter, or that weapon is not availible
int missileAvailible = true;
int laserAvailible = false;
int toxinAvailible = false;

// Green when the target in an attack has been locked onto
int targetLocked = false;

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

// Used during an attack before the target is locked onto
int targetProgress = 5;

// Buttons in the center
int manualButton = 0;
int lastAnalysis = 0;
int specimenStorage = 0;
int selfDestruct = 0;
// Only active when self destruct is counting down
int selfDestructCountdown = 0;

// The three possible speeds of analysis text scrolling
int slowAnalysisSpeed = 0;
int mediumAnalysisSpeed = 0;
int fastAnalysisSpeed = 1;

// Whether or not the analysis light is currently lit up
int lightOn = 0;

// Availibility of moves (reversal is always availible)
int forwardMovePossible = true;
int leftMovePossible = true;
int rightMovePossible = true;

int uplinkActive = 0;

int kmDigits[3] = { 0, 0, 0 };


// Color used for transparency in source images
SDL_Color transparent = { 0, 0xFF, 0xFF };

std::list<ImageHandler *> allImages;


#define TRANSPARENT 0x00FFFF

void updateKM()
{
	kmDigits[0] = kmTraveled / 100;
	kmDigits[1] = (kmTraveled / 10) % 10;
	kmDigits[2] = kmTraveled % 10;
}


#define COLON 10
int timeDigits[] = { 0, COLON, 0, 0, COLON, 0, 0 };

void updateTime()
{
	int tmp = secondsLeft;
	timeDigits[0] = tmp / (60 * 60);
	timeDigits[1] = COLON;
	tmp %= (60 * 60);
	timeDigits[2] = (tmp / 60) / 10;
	timeDigits[3] = (tmp / 60) % 10;
	tmp %= 60;
	timeDigits[4] = COLON;
	timeDigits[5] = tmp / 10;
	timeDigits[6] = tmp % 10;

}

static SDL_Surface *analysisSurface = NULL;
static int scrollProgress = 0;
static TTF_Font *analysisFont = NULL;
static SDL_Color textColor = { 0, 255, 0 };
static SDL_Color black = { 0, 0, 0 };

void initAnalysis()
{
	TTF_Init();
	analysisFont = TTF_OpenFont((rootPath + "INSTALL/SCROLL.FON").data(),8);
	if (!analysisFont)
	{
		fprintf(stderr, "Unable to load font: %s", SDL_GetError());
		exit(0);
	}
	analysisSurface = TTF_RenderText(analysisFont,"",textColor,black);
}


void updateAnalysis(std::string text)
{
	scrollProgress = 0;
	if (analysisSurface)
		SDL_FreeSurface(analysisSurface);
	analysisSurface = TTF_RenderText(analysisFont, text.data(),
		textColor, black);
}

void addImageHandler(const std::string path, int x, int y)
{
	std::string fullPath = rootPath + path;
	SDL_Surface *tmp = SDL_LoadBMP(fullPath.data());
	allImages.push_back(
		new ImageHandler(tmp, x, y));

}
extern SDL_Renderer *renderer;
extern SDL_Window *window; 

SDL_Surface *loadImage(std::string path)
{
	SDL_RendererInfo info;
	SDL_Surface *firstSurf = SDL_LoadBMP((rootPath + path).data());
	// Optimize the image
	SDL_Surface *newSurf = SDL_ConvertSurfaceFormat(firstSurf, SDL_GetWindowPixelFormat(window), 0);
	// And free the old image
	SDL_FreeSurface(firstSurf);
	// Set the transparency color
	if (SDL_SetColorKey(newSurf, SDL_TRUE,
		TRANSPARENT))
	{
		fprintf(stderr, "Warning: colorkey will not be used, reason: %s\n", SDL_GetError());
	}
	return newSurf;

}

void loadImages(SDL_Surface *screen)
{
	initAnalysis();
	// This must be drawn first, goes behind everything else
	addImageHandler("Install/Screen/Final2.dib", 0, 0);
	// The satellite map of the island
	addImageHandler("Install/Screen/SatIslan.dib", 462, 314);

	// Arrows display green if a direction can be moved in
	SDL_Surface *arrows = loadImage("Install/Screen/DIRIND.dib");
	allImages.push_back(new ArrowDisplay(arrows, { 0, 8, 8, 8  }, { 153, 141, 8, 8 }, &leftMovePossible));
	allImages.push_back(new ArrowDisplay(arrows, { 0, 24, 8, 8 }, { 320, 48, 8, 8  }, &forwardMovePossible));
	allImages.push_back(new ArrowDisplay(arrows, { 0, 40, 8, 8 }, { 483, 149, 8, 8 }, &rightMovePossible));

	// Rectangles displaying red if a weapon is unavailible during an attack, or green otherwise
	SDL_Rect wpnRect[] = { { 99, 56, 35, 4 }, {99, 128, 16, 4}, {99, 197, 35, 4} };
	allImages.push_back(new RectangleHandler(0x00FF00, 0xFF0000, wpnRect[0], &missileAvailible));
	allImages.push_back(new RectangleHandler(0x00FF00, 0xFF0000, wpnRect[1], &laserAvailible));
	allImages.push_back(new RectangleHandler(0x00FF00, 0xFF0000, wpnRect[2], &toxinAvailible));

	// Target lock display: first object is for the growing red rectangle prior to lock on,
	// second is for green overlay once lock on is achieved
	allImages.push_back(new BasicDisplay(loadImage("Install/Screen/TARGET.dib"), 408, 19, &targetProgress, 6));

	SDL_Rect lockedOnRect = { 407, 16, 22, 25 };
	allImages.push_back(new RectangleHandler(0x00FF00, 0x00, lockedOnRect, &targetLocked));

	// Weapon availibility
	allImages.push_back(new WeaponDisplay(loadImage("Install/Screen/MIS.dib"),31,51,&missileState));
	allImages.push_back(new WeaponDisplay(loadImage("Install/Screen/LASER.dib"), 31, 122, &laserState));
	allImages.push_back(new WeaponDisplay(loadImage("Install/Screen/TOXIN.dib"), 31, 191, &toxinState));
	
	SDL_Surface *textSheet = loadImage("Install/Screen/SMALLCD.dib");
	SDL_SetColorKey(textSheet, SDL_TRUE, TRANSPARENT);

	// Weapon shots left
	// Missiles use a numerical display instead of a bar
	allImages.push_back(new BasicDisplay(textSheet, 98, 86, &missilesLeft, 11));
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
	// Scrolling analysis display
	allImages.push_back(new AnalysisDisplay(&analysisSurface, 219, 415));

	// Kilometers traveled display
	for (int i = 0; i < 3; i++)
	{
		allImages.push_back(new BasicDisplay(textSheet, 238+10*i, 270, &kmDigits[i], 11));
	}

	int xPos = 346;
	for (int i = 0; i < 7; i++)
	{
		allImages.push_back(new BasicDisplay(textSheet, xPos, 270, &timeDigits[i], 11));
		if (timeDigits[i+1] == COLON)
			xPos += 8;
		else 
			xPos += 10;
	}

	// Misc parts of the display
	allImages.push_back(new BasicDisplay(loadImage("Install/Screen/SATT.dib"), 441, 443, &positionPressed, 2));
	allImages.push_back(new BasicDisplay(loadImage("Install/Screen/ANALYS.dib"), 19, 444, &analysPressed, 2));
	allImages.push_back(new BasicDisplay(loadImage("Install/Screen/UPLINK.dib"), 280, 266, &uplinkActive, 2));
	allImages.push_back(new BasicDisplay(loadImage("Install/Screen/LIGHT.dib"), 313, 367, &lightOn, 2));

	// Temp test
	kmTraveled = 5;
	updateKM();
	updateTime();
	updateAnalysis("Testing");
}

void drawScreen(SDL_Surface *screen)
{
	SDL_RenderClear(renderer);
	SDL_RenderDrawRect(renderer, NULL);
	for (ImageHandler *ih : allImages)
	{
		ih->draw();
	}
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, screen);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_DestroyTexture(texture);
}