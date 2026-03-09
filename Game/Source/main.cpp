#include "Application.h"

int main(int argc, char* argv[])
{
	Apex::Application app(argv[0]);

	app.Init();
	app.LoadScene();
	app.Run();

	return 0;
}