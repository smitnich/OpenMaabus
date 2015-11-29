typedef int input_t;

enum
{
	INPUT_NONE,
	INPUT_QUIT,
	INPUT_FORWARD,
	INPUT_RIGHT,
	INPUT_LEFT,
	INPUT_REVERSE
};

input_t getInput();