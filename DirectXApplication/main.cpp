#include "application.h"

int wmain(int argc, wchar_t** argv, wchar_t** evnp)
{
	Application app = Application(960, 640);
	app.run();

	return 0;
}