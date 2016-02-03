void readMapFile(const char *fileName);

struct MapData
{
	char name[3];
	char arrivalText[32];
	int coords[2];

};
struct FacingData
{
	MapData *moveTo;
	bool canHaveKey;
	bool keyPresent;
};
