#include <SDL.h>
#include "input.h"

input_t getInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				return INPUT_QUIT;
			case SDLK_UP:
				return INPUT_FORWARD;
			case SDLK_DOWN:
				return INPUT_REVERSE;
			case SDLK_RIGHT:
				return INPUT_RIGHT;
			case SDLK_LEFT:
				return INPUT_LEFT;
			}
		}
		else if (event.type == SDL_QUIT)
		{
			return INPUT_QUIT;
		}
	}
	return INPUT_NONE;
}