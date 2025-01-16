#include <iostream>
#include "AppCore/vk_core.h"


int main() {


	Core::Application app;

	app.Init();
	app.RunApp();
	app.CleanUp();

	return 0;
	
}