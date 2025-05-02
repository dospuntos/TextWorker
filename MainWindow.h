#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Sidebar.h"
#include <Application.h>
#include <Bitmap.h>
#include <FilePanel.h>
#include <MenuBar.h>
#include <MessageRunner.h>
#include <StringView.h>
#include <SupportDefs.h>
#include <TextControl.h>
#include <TextView.h>
#include <Window.h>
#include <private/shared/ToolBar.h>

enum {
	M_TRANSFORM_UPPERCASE = 'UPCS',
	M_TRANSFORM_LOWERCASE,
	M_TRANSFORM_CAPITALIZE,
	M_TRANSFORM_TITLE_CASE,
	M_TRANSFORM_RANDOM_CASE,
	M_TRANSFORM_ALTERNATING_CASE,
	M_TRANSFORM_TOGGLE_CASE,
	M_REMOVE_LINE_BREAKS,
	M_INSERT_LINE_BREAKS,
	M_BREAK_LINES_ON_DELIMITER,
	M_TRIM_LINES,
	M_TRIM_EMPTY_LINES,
	M_TRANSFORM_ENCODE_URL,
	M_TRANSFORM_DECODE_URL,
	M_TRANSFORM_ENCODE_BASE64,
	M_TRANSFORM_DECODE_BASE64,
	M_TRANSFORM_ROT13,
	M_TRANSFORM_WIP,
	M_TRANSFORM_PREFIX_SUFFIX,
	M_TRANSFORM_REPLACE,
	B_TEXT_CHANGED,
	B_CURSOR_MOVED,
	M_INSERT_EXAMPLE_TEXT,
	M_UPDATE_STATUSBAR,
	M_FILE_NEW,
	M_FILE_OPEN,
	M_FILE_SAVE,
	M_FILE_SAVE_AS,
	M_TOGGLE_WORD_WRAP
};

class MainWindow : public BWindow
{
public:
						MainWindow(void);
			virtual		~MainWindow();

			void		MessageReceived(BMessage *msg);
			BBitmap* ResourceToBitmap(const char* resName);

			bool		QuitRequested(void);

			void OpenFile(const entry_ref& ref);
			void SaveFile(const char* path);

		private:
			BMenuBar*	_BuildMenu();
			BToolBar* toolbar;
			Sidebar* sidebar;
			void UpdateStatusBar();

			status_t	_LoadSettings(BMessage& settings);
			status_t	_SaveSettings();
			void		_RestoreValues(BMessage& settings);

			BTextView*  textView;
			BScrollView* scrollView;
			BStringView* statusBar;
			BMessageRunner* statusUpdater;

			BMenuItem* fSaveMenuItem;
			BFilePanel* fOpenPanel;
			BFilePanel* fSavePanel;
			BString fFilePath;
};

#endif
