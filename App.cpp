#include "App.h"
#include "Constants.h"
#include <Catalog.h>
#include "MainWindow.h"

#include <AboutWindow.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainView"

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
	about->AddDescription("Clean and transform text easily with this lightweight tool.");
	about->AddCopyright(2025, "Johan Wagenheim");
	about->AddDescription("Toolbar icons by ZuMi\nThanks to Humdinger for advice and review");
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
