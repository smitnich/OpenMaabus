#include <iostream>

struct help_text
{
	char missile[280];
	char missilesLeft[106];
	char laser[274];
	char laserLeft[111];
	char toxin[278];
	char toxinLeft[95];
	char computer[169];
	char analyser[251];
	char text[108];
	char scrolling[276];
	char specimen[176];
	char lastAnalysis[276];
	char selfDestruct[233];
	char distTravelled[145];
	char uplink[150];
	char timeLeft[102];
	char mainView[467];
	char poh[268];
	char targetLock[255];
	char radiation[214];
	char lifeForm[209];
	char magnets[217];
	char unid[228];
	char status[441];
	char position[355];
	char missilesAvailible[144];
	char laserAvailible[153];
	char toxinAvailible[133];


	char remainder[1024];
};

help_text allHelp;

void openEXE(const char *fileName)
{
	// We need to fetch some hard-coded strings from the executable file
	// NOTE: This may not work for all versions of the game
	const size_t text_offset = 0x00027AA9;
	const size_t end_offset = 0x0002928A;
	char buffer[end_offset - text_offset] = { 0 };
	FILE *exe;
	help_text test = { 0 };
	errno_t err = fopen_s(&exe, fileName, "rb");
	int max_seek = 2048;
	size_t it = text_offset;
	while (it > 0)
	{
		if (it < max_seek)
		{
			fread(buffer, sizeof(char), it, exe);
			break;
		}
		else
			fread(buffer, sizeof(char), max_seek, exe);
		it -= max_seek;
	}
	fread(buffer, sizeof(char), end_offset - text_offset, exe);
	memcpy(&test, buffer, end_offset - text_offset);
	int next = strlen(test.remainder)+1;
	printf("%d - %s", next, test.remainder);
	fclose(exe);
}