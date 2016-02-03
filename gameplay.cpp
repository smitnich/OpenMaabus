enum AnalysisSpeed
{
	AS_SPEED_SLOW,
	AS_SPEED_MED,
	AS_SPEED_FAST
};

// The amount of kilometers traveled
int kmTraveled = 0;
// The speed that analysis text scrolls at
int analysisSpeed = AS_SPEED_SLOW;

int missilesLeft = 4;
int lasersLeft = 0;
int toxinLeft = 3;

int half_hours_availible = 3;

int secondsLeft = 60 * 60 + 60*23 + 45;
