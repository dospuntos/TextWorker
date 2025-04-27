#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <TextView.h>
#include <Window.h>
#include <MenuBar.h>

enum {
	M_TRANSFORM_UPPERCASE = 'UPCS',
	M_TRANSFORM_LOWERCASE,
	M_TRANSFORM_CAPITALIZE,
	M_TRANSFORM_TITLE_CASE,
	M_TRANSFORM_RANDOM_CASE,
	M_TRANSFORM_ALTERNATING_CASE,
	M_REMOVE_LINE_BREAKS,
	M_TRANSFORM_ROT13,
	M_TRANSFORM_WIP
};

class MainWindow : public BWindow
{
public:
						MainWindow(void);
			void		MessageReceived(BMessage *msg);
			bool		QuitRequested(void);

private:
			BMenuBar*	_BuildMenu();
			BTextView*  textView;


};

#endif
