#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Sidebar.h"
#include "UndoableTextView.h"
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
	void _UpdateStatusBar();
	void _UpdateStatusMessage(BString message);
	void _UpdateToolbarState();
	bool _ClipboardHasText() const;
	bool _GetClipboardText(BString& outText) const;

	bool fSaveTextOnExit;
	bool fSaveFieldsOnExit;
	bool fInsertClipboard;
	bool fClearSettingsAfterUse;
	int32 fFontSize;
	BString fFontFamily;
	bool fApplyToSelection;
	status_t _LoadSettings(BMessage& settings);
	status_t _SaveSettings();
	void _RestoreValues(BMessage& settings);

	UndoableTextView* fTextView;
	BScrollView* fScrollView;
	BStringView* fStatusBar;
	BStringView* fMessageBar;
	BMessageRunner* fStatusUpdater;
	BMessageRunner* fStatusClearRunner = nullptr;

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

	BString fLastSavedText;
	bool IsDocumentModified() const;
};

#endif
