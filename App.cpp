#include "App.h"
#include "Constants.h"
#include "MainWindow.h"

#include <AboutWindow.h>


App::App(void)
	:
	BApplication(kApplicationSignature)
{
	MainWindow* mainwin = new MainWindow();
	mainwin->Show();
}


void
App::AboutRequested()
{
	BAboutWindow* about = new BAboutWindow(kApplicationName, kApplicationSignature);
	about->AddDescription("A small program to manipulate text in a multitude of ways.");
	about->AddCopyright(2025, "Johan Wagenheim");
	about->Show();
}


int
main(void)
{
	App* app = new App();
	app->Run();
	delete app;
	return 0;
}
