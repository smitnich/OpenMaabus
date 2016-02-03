// Map file format:
// First line - Number of locations within (78)
// Followed by locations in this format:
// Name (ie j2)
// Followed by 1-4 of the following blocks, one for each accessible
// direction
//	7 digits, unknown purpose
//	Analysis text upon arriving, or t for nothing
//	Map position x, y
//	Headers for connection mav file names
//	connection mav file names
//	(Equivalent file names when a key is present)
//	If death on a timer, 0 fileName, timer in milliseconds
//	Count of analysis hotspots
//		Analysis Hotspot format
//		X, Y start and X, Y end of bounding box, followed by 5 numbers, unknown purpose
//		One or two lines of analysis text
//

#include <fstream>
#include <string>
#include "main.h"
#include "mapFile.h"
using namespace std;

#define ANALYSIS_TEXT_LENGTH

struct Analysis
{
	int xStart, yStart, xEnd, yEnd;
	int unknown[5];
	char text[2][64];
};

Analysis *readAnalysis(fstream *input)
{
	Analysis *a = new Analysis();
	*input >> a->xStart >> a->yStart >> a->xEnd >> a->yEnd;
	for (int i = 0; i < 5; i++)
		*input >> a->unknown[i];
	input->ignore(32, '\n');
	input->getline(a->text[0],128,'\n');
	char tmp = input->peek();
	if (tmp < '0' && tmp > '9')
		input->getline(a->text[1],128,'\n');
	return a;
}

enum directions
{
	GO_LEFT = 0,
	GO_FORWARD,
	GO_RIGHT,
	REVERSE_A,
	REVERSE_B,
	LEAVE_AREA
};

enum events
{
	EVENT_ATTACK = 1,
	EVENT_TIMED_DEATH = 4
};

void parseLocation(fstream *input)
{
	int headers[7];
	char id[3];
	char arrivalText[64] = { 0 };
	int unknownNum[10];
	int coord[2];
	int directionAvailible;
	int distances[7];
	char movePaths[2][7][10] = { 0 };
	int numAnalysis;
	Analysis **analysis;
	int moreUnknown[11];
	*input >> id;
	for (int i = 0; i < 7; i++)
		*input >> headers[i];
	if (input->peek() != 't')
	{
		*input >> arrivalText; 
	}
	input->ignore(16,'t');
	*input >> unknownNum[0] >> coord[0] >> coord[1];
	*input >> directionAvailible;
	for (int i = 0; i < 7; i++)
		*input >> distances[i];
	for (int i = 0; i < 7; i++)
		*input >> movePaths[0][i];
	// Check for a second row of headers; mainly used when multiple
	// videos exist for the same movement due to presence of items
	if (input->peek() != ' ')
	{
		for (int i = 0; i < 7; i++)
			*input >> movePaths[1][i];
	}
	input->ignore(32, '\n');
	*input >> numAnalysis;

	analysis = new Analysis*[numAnalysis];

	for (int i = 0; i < numAnalysis; i++)
		analysis[i] = readAnalysis(input);
	for (int i = 0; i < 11; i++)
	{
		*input >> moreUnknown[i];
	}
	input->ignore(32, '\n');

}

void readMapFile(const char *fileName)
{
	return;
	fstream finput = fstream(fileName);
	int numItems;
	finput >> numItems;
	parseLocation(&finput);
}