#include "App.h"
#include "Constants.h"
#include <Catalog.h>
#include <Path.h>

#include <AboutWindow.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainView"

App::App(void)
	:
	BApplication(kApplicationSignature)
{
	fMainWindow = new MainWindow();
	fMainWindow->Show();
}


void
App::AboutRequested()
{
	const char* authors[] = {
		"Johan Wagenheim",
		NULL
	};

	const char* mentions = B_TRANSLATE("ZuMi (Toolbar icons)");
	const char* mentions2 = B_TRANSLATE("Humdinger (advice and review)");
	const char* thanks[] = {
		"Davidmp (Catalan translation)",
		"Humdinger (German translation)",
		mentions,
		mentions2,
		NULL
	};

	BAboutWindow* about = new BAboutWindow(kApplicationName, kApplicationSignature);
	about->AddDescription(B_TRANSLATE("Clean and transform text easily with this lightweight tool."));
	about->AddCopyright(2025, "Johan Wagenheim");
	about->AddText(B_TRANSLATE("Distributed under the terms of the MIT License."));
	about->AddAuthors(authors);
	about->AddSpecialThanks(thanks);
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


void
App::RefsReceived(BMessage* msg)
{
	if (fMainWindow)
		fMainWindow->PostMessage(msg);
}