#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Application.h>//
#include <StringView.h>
#include <TextControl.h>
#include <TextView.h>
#include <Window.h>
#include <MenuBar.h>
#include <FilePanel.h>
#include <MessageRunner.h>
#include <Bitmap.h>
#include "Sidebar.h"
#include <private/shared/ToolBar.h>
#include <SupportDefs.h>

enum {
	M_TRANSFORM_UPPERCASE = 'UPCS',
	M_TRANSFORM_LOWERCASE,
	M_TRANSFORM_CAPITALIZE,
	M_TRANSFORM_TITLE_CASE,
	M_TRANSFORM_RANDOM_CASE,
	M_TRANSFORM_ALTERNATING_CASE,
	M_REMOVE_LINE_BREAKS,
	M_INSERT_LINE_BREAKS,
	M_BREAK_LINES_ON_DELIMITER,
	M_TRIM_LINES,
	M_TRIM_EMPTY_LINES,
	M_TRANSFORM_ROT13,
	M_TRANSFORM_WIP,
	M_TRANSFORM_PREPEND_APPEND,
	B_TEXT_CHANGED,
	B_CURSOR_MOVED,
	M_INSERT_EXAMPLE_TEXT,
	M_UPDATE_STATUSBAR,
	M_FILE_NEW,
	M_FILE_OPEN,
	M_FILE_SAVE,
	M_FILE_SAVE_AS
};

class MainWindow : public BWindow
{
public:
						MainWindow(void);
			virtual		~MainWindow();

			void		MessageReceived(BMessage *msg);
			BBitmap		*ResVectorToBitmap(const char *resName);

			bool		QuitRequested(void);

			void 		OpenFile(const entry_ref &ref);
			void		SaveFile(const char *path);

private:
			BMenuBar*	_BuildMenu();
			BToolBar*	toolbar;
			Sidebar* 	sidebar;
			void UpdateStatusBar();

			status_t	_LoadSettings(BMessage& settings);
			status_t	_SaveSettings();

			BTextView*  textView;
			BStringView* statusBar;
			BMessageRunner* statusUpdater;

			BMenuItem*		fSaveMenuItem;
			BFilePanel*		fOpenPanel;
			BFilePanel*		fSavePanel;
			BString			fFilePath;
};

#endif
