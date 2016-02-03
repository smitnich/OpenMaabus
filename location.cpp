
//1 North to East
//4 North to West
//5 West to East
//6 
//7 East to West
//9 East to North
//10 North to West
//12 West to North
//R#-A turn towards north exit
//R#-B turn away from north exit
//R1 = North
//R2 = East
//R3 = South
//R4 = East
#include <string>
#include "location.h"
#include "input.h"
#include <list>

#define BUFFER_SIZE 128

extern std::list<std::string> queuedVideos;

void playVideo(const char *fileName);

int lookupTable[NUM_DIRS][NUM_DIRS] =
{
	{ -1, 8, 1, 4 }, // North to *
	{ 6, -1, 10, 11 }, // South to *
	{ 9, 2, -1, 7 }, // East to *
	{ 12, 3, 5, -1 }, // West to *
};

extern std::string rootPath;

Location::Location(const char *name)
{
	currentPos = 0;
	reversed = false;
	nodeName = std::string(name);
}
void Location::handleInput(int input)
{
	int toMove = 0;
	switch (input)
	{
	case INPUT_REVERSE:
		reverse();
		return;
	case INPUT_FORWARD:
		switch (currentPos) {
		case DIR_NORTH:
			toMove = DIR_SOUTH;
			break;
		case DIR_EAST:
			toMove = DIR_WEST;
			break;
		case DIR_WEST:
			toMove = DIR_EAST;
			break;
		case DIR_SOUTH:
			toMove = DIR_NORTH;
			break;
		default:
			return;
		}
		break;
	case INPUT_RIGHT:
		if (reversed)
			return;
		switch (currentPos)
		{
		case DIR_NORTH:
			toMove = DIR_WEST;
			break;
		case DIR_EAST:
			toMove = DIR_NORTH;
			break;
		case DIR_WEST:
			toMove = DIR_SOUTH;
			break;
		case DIR_SOUTH:
			toMove = DIR_EAST;
			break;
		default:
			return;
		}
		break;
	case INPUT_LEFT:
		if (reversed)
			return;
		switch (currentPos)
		{
		case DIR_NORTH:
			toMove = DIR_EAST;
			break;
		case DIR_EAST:
			toMove = DIR_SOUTH;
			break;
		case DIR_WEST:
			toMove = DIR_NORTH;
			break;
		case DIR_SOUTH:
			toMove = DIR_WEST;
			break;
		default:
			return;
		}
	}
	move(currentPos, toMove);
}
void Location::reverse()
{
	char buffer[1024] = { 0 };
	int lookup[] = { 0, 1, 3, 2, 4 };
	sprintf_s(buffer, "%sCD1/%s/%sR%d%s.mav", rootPath.data(), nodeName.data(), nodeName.data(), lookup[currentPos], !reversed ? "A" : "B");
	queuedVideos.push_back(std::string(buffer));
	reversed = !reversed;
}
void Location::move(int start, int end)
{
	char buffer[1024] = { 0 };
	int val = lookupTable[start - 1][end - 1];
	sprintf_s(buffer, "%sCD1/%s/%s%d.mav", rootPath.data(), nodeName.data(), nodeName.data(), lookupTable[start - 1][end - 1]);
	queuedVideos.push_back(std::string(buffer));
	currentPos = end;
	reversed = true;
}