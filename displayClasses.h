#include <SDL.h>
#include <SDL_ttf.h>
extern SDL_Surface *screen;

class ImageHandler
{
public:
	SDL_Rect pos;
	SDL_Surface *image;
	ImageHandler(SDL_Surface *surface, int x, int y);
	~ImageHandler();
	virtual void draw();
};

class RectangleHandler : public ImageHandler
{
public:
	Uint32 onColor, offColor;
	SDL_Rect pos;
	int *isOn;
	RectangleHandler(Uint32 _onColor, Uint32 _offColor, SDL_Rect _pos, int *_isOn);
	void draw();
	~RectangleHandler();
};


class ArrowDisplay : public ImageHandler
{
public:
	int *isOn;
	SDL_Rect srcRect, pos;
	SDL_Surface *image;
	ArrowDisplay(SDL_Surface *surface, SDL_Rect _srcRect, SDL_Rect _destRect, int *_isOn);
	void draw();
};

class WeaponDisplay : public ImageHandler
{
public:
	int *weaponState;
	WeaponDisplay(SDL_Surface *surface, int x, int y, int *ws);
	void draw();
};

class BasicDisplay : public ImageHandler
{
public:
	int *condition;
	int imageHeight;
	BasicDisplay(SDL_Surface *surface, int x, int y, int *state, int numImages);
	void draw();
};

class AnalysisDisplay : public ImageHandler
{
public:
	int scrollPosition;
	bool active;
	SDL_Surface **toDraw;
	AnalysisDisplay(SDL_Surface **surface, int x, int y);
	void draw();
};

// Availability of weapons

enum {
	WEAPON_ACTIVE,
	WEAPON_AVAILABLE,
	WEAPON_UNAVAILABLE
};