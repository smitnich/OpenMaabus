#include <SDL/SDL.h>
#include "input.h"

input_t getInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.sym == SDLK_ESCAPE)
				return INPUT_QUIT;
		}
		else if (event.type == SDL_QUIT)
		{
			return INPUT_QUIT;
		}
	}
	return INPUT_NONE;
}