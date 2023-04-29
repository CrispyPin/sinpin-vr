#include "app.h"
#include <signal.h>

#define UPDATE_RATE 120

bool should_exit = false;

void interrupted(int _sig)
{
	should_exit = true;
}

int main()
{
	signal(SIGINT, interrupted);

	auto app = App();

	while (!should_exit)
	{
		usleep(1000000 / UPDATE_RATE);
		app.Update(1.0 / UPDATE_RATE);
	}
	printf("\nShutting down\n");
	return 0;
}
