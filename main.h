#include <string>

struct SDL_Surface;
struct SDL_Renderer;

extern std::string rootPath;
extern SDL_Surface *screen;
extern SDL_Surface *getScreen();
extern SDL_Renderer *renderer;