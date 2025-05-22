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
