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
	M_TRANSFORM_UPPERCASE      = 'UPCS',
	M_TRANSFORM_LOWERCASE      = 'lwcs',
	M_TRANSFORM_CAPITALIZE     = 'CPSZ',
	M_TRANSFORM_TITLE_CASE     = 'TTCS',
	M_TRANSFORM_RANDOM_CASE    = 'RNDC',
	M_TRANSFORM_ALTERNATING_CASE = 'ALTC',
	M_TRANSFORM_TOGGLE_CASE    = 'TGGL',
	M_REMOVE_LINE_BREAKS       = 'RMNL',
	M_INSERT_LINE_BREAKS       = 'INSN',
	M_BREAK_LINES_ON_DELIMITER = 'BRKD',
	M_TRIM_LINES               = 'TRML',
	M_TRIM_EMPTY_LINES         = 'TREL',
	M_TRANSFORM_ENCODE_URL     = 'ENUR',
	M_TRANSFORM_DECODE_URL     = 'DCUR',
	M_TRANSFORM_ENCODE_BASE64  = 'EN64',
	M_TRANSFORM_DECODE_BASE64  = 'DC64',
	M_TRANSFORM_ROT13          = 'RT13',
	M_TRANSFORM_WIP            = 'TRWP',
	M_TRANSFORM_PREFIX_SUFFIX  = 'PXSF',
	M_TRANSFORM_REPLACE        = 'RPLC',
	B_TEXT_CHANGED             = 'TXCH',
	B_CURSOR_MOVED             = 'CSMV',
	M_INSERT_EXAMPLE_TEXT      = 'EXMP',
	M_UPDATE_STATUSBAR         = 'STBR',
	M_FILE_NEW                 = 'FLNW',
	M_FILE_OPEN                = 'FLOP',
	M_FILE_SAVE                = 'FLSV',
	M_FILE_SAVE_AS             = 'SVAS',
	M_TOGGLE_WORD_WRAP         = 'WRWP',
	M_SHOW_SETTINGS            = 'Shws'
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
			BWindow* fSettingsWindow;

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
