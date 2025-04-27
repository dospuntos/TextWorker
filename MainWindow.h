#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <StringView.h>
#include <TextView.h>
#include <Window.h>
#include <MenuBar.h>
#include <MessageRunner.h>

enum {
	M_TRANSFORM_UPPERCASE = 'UPCS',
	M_TRANSFORM_LOWERCASE,
	M_TRANSFORM_CAPITALIZE,
	M_TRANSFORM_TITLE_CASE,
	M_TRANSFORM_RANDOM_CASE,
	M_TRANSFORM_ALTERNATING_CASE,
	M_REMOVE_LINE_BREAKS,
	M_TRANSFORM_ROT13,
	M_TRANSFORM_WIP,
	B_TEXT_CHANGED,
	B_CURSOR_MOVED,
	M_UPDATE_STATUSBAR
};

class MainWindow : public BWindow
{
public:
						MainWindow(void);
			virtual		~MainWindow();

			void		MessageReceived(BMessage *msg);
			bool		QuitRequested(void);

private:
			BMenuBar*	_BuildMenu();
			void UpdateStatusBar();

			status_t	_LoadSettings(BMessage& settings);
			status_t	_SaveSettings();

			BTextView*  textView;
			BStringView* statusBar;
			BMessageRunner* statusUpdater;
};

#endif
