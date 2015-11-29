class Location
{
public:
	int currentPos;
	// Reversed is considered to be facing away from the center
	// of the area
	bool reversed;
	Location(const char* name);
	std::string nodeName;
	void handleInput(int input);
	void move(int start, int end);
	void reverse();
};
enum dirs
{
	DIR_NORTH = 1,
	DIR_SOUTH,
	DIR_EAST,
	DIR_WEST,
	NUM_DIRS = DIR_WEST
};