#ifndef APP_H
#define APP_H

#include "MainWindow.h"
#include <Application.h>

class App : public BApplication
{
public:
	App(void);
	void RefsReceived(BMessage* message);

	virtual void	AboutRequested();
	MainWindow* fMainWindow;
};

#endif // APP_H
