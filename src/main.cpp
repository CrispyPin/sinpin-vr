#include "app.h"
#include <signal.h>

#define FRAMERATE 30

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
		app.Update();
		usleep(1000000 / FRAMERATE);
	}
	printf("\nShutting down\n");
	return 0;
}
