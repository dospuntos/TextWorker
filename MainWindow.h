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
#include <private/shared/ToolBar.h>
#include <Window.h>

class MainWindow : public BWindow {
public:
	MainWindow(void);
	virtual ~MainWindow();

	void MessageReceived(BMessage* msg);
	BBitmap* ResourceToBitmap(const char* resName);

	bool QuitRequested(void);

	void OpenFile(const entry_ref& ref);
	void SaveFile(const char* path);
	void MenusBeginning();

private:
	BMenuBar* _BuildMenu();
	BToolBar* fToolbar;
	Sidebar* fSidebar;
	void UpdateStatusBar();

	bool fSaveTextOnExit;
	bool fSaveSettingsOnExit;
	bool fInsertClipboard;
	int32 fFontSize;
	BString fFontFamily;
	status_t _LoadSettings(BMessage& settings);
	status_t _SaveSettings();
	void _RestoreValues(BMessage& settings);

	BTextView* fTextView;
	BScrollView* fScrollView;
	BStringView* fStatusBar;
	BMessageRunner* fStatusUpdater;

	BMenuItem* fSaveMenuItem;
	BFilePanel* fOpenPanel;
	BFilePanel* fSavePanel;
	BString fFilePath;
	BWindow* fSettingsWindow;

	BMenuItem* fUndoItem;
	BMenuItem* fRedoItem;
	BMenuItem* fCutItem;
	BMenuItem* fCopyItem;
	BMenuItem* fPasteItem;
	BMenuItem* fSelectAllItem;
};

#endif
