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
#include <unicode/ucsdet.h>
#include <unicode/ucnv.h>
#include <Window.h>
#include <private/shared/ToolBar.h>

enum LineEnding {
    LINE_ENDING_LF,    // Unix/Haiku \n
    LINE_ENDING_CRLF,  // Windows \r\n
    LINE_ENDING_CR,    // Old Mac \r
    LINE_ENDING_MIXED
};

enum TextEncoding {
    ENCODING_UTF8,
    ENCODING_UTF16_LE,
    ENCODING_UTF16_BE,
    ENCODING_LATIN1,
    ENCODING_WINDOWS1252,
    ENCODING_UNKNOWN
};

class MainWindow : public BWindow {
public:
	MainWindow(void);
	virtual ~MainWindow();

	void MessageReceived(BMessage* msg);
	BBitmap* ResourceToBitmap(const char* resName);

	bool QuitRequested(void);

	void OpenFile(const entry_ref& ref);
	status_t SaveFile(const char* path);
	void MenusBeginning();
	void _ApplySettings(const BMessage& msg);
	void DispatchMessage(BMessage* message, BHandler* target);

private:
	bool fPendingQuitAfterSave = false;
	BMenuBar* _BuildMenu();
	BToolBar* fToolbar;
	Sidebar* fSidebar;
	void _UpdateStatusBar();
	void _UpdateStatusMessage(BString message);
	void _UpdateToolbarState();
	void _UpdateWindowTitle();
	bool _ClipboardHasText() const;
	bool _GetClipboardText(BString& outText) const;

	bool fSaveTextOnExit;
	bool fSaveFieldsOnExit;
	bool fInsertClipboard;
	bool fClearSettingsAfterUse;
	int32 fFontSize;
	bool fCloseOnEsc;
	bool fAskToSave;
	BString fFontFamily;
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
	BMenuItem* fLineEndingLfItem;
	BMenuItem* fLineEndingCrlfItem;
	BMenuItem* fLineEndingCrItem;

	BString fLastSavedText;
	bool IsDocumentModified() const;
	bool _CheckSaveAndContinue(BMessage* pendingMessage);
	void _OnSaveComplete();
	BMessage* fPendingMessage;

	LineEnding fLineEnding;
	LineEnding _DetectLineEnding(BFile& file);
	BString _LineEndingLabel() const;
	BString _NormalizeLineEndings(const char* raw, ssize_t length);
	BString _RestoreLineEndings(const BString& text) const;

	TextEncoding fEncoding;
	bool fHasBOM;
	TextEncoding    _DetectEncoding(const char* raw, ssize_t length, size_t& outDataStart);
	BString         _DecodeToUTF8(const char* raw, ssize_t length, TextEncoding encoding);
	BString         _EncodeFromUTF8(const BString& text, TextEncoding encoding);
	BString		    _EncodingToICUName(TextEncoding encoding) const;
	BString		    _EncodingLabel() const;
};

#endif
