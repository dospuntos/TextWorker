#include "MainWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Bitmap.h>
#include <Button.h>
#include <Catalog.h>
#include <Clipboard.h>
#include <File.h>
#include <FindDirectory.h>
#include <Font.h>
#include <IconUtils.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuItem.h>
#include <NodeInfo.h>
#include <Path.h>
#include <Resources.h>
#include <ScrollView.h>
#include <TranslationUtils.h>
#include <TranslatorRoster.h>
#include <View.h>
#include <Url.h>
#include <cctype>
#include <Screen.h>

#include "Constants.h"
#include "SettingsWindow.h"
#include "TextUtils.h"
#include "Toolbar.h"
#include "IconMenuItem.h"

static const char* kSettingsFile = "TextWorker_settings";

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainView"


MainWindow::MainWindow(void)
	:
	BWindow(BRect(100, 100, 900, 800), kApplicationName, B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE)
{
	BMenuBar* menuBar = _BuildMenu();

	fTextView = new UndoableTextView("TextView");
	fTextView->MakeEditable(true);

	fScrollView = new BScrollView("TextViewScroll", fTextView, B_WILL_DRAW | B_FRAME_EVENTS, true,
		true, B_PLAIN_BORDER);

	fToolbar = CreateToolbar(this);
	fSidebar = new Sidebar();

	fStatusBar = new BStringView("StatusBar", "");
	fStatusBar->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_VERTICAL_CENTER));

	fSettingsWindow = nullptr;

	// Layout
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(menuBar, 0)
		.Add(fToolbar, 0)
		.SetInsets(2)
		.AddGroup(B_HORIZONTAL, 0)
		.Add(fSidebar, 0)
		.Add(fScrollView, 1)
		.SetInsets(5, 5, 5, 5)
		.End()
		.Add(fStatusBar, 0);

	BMessage settings;
	_LoadSettings(settings);
	_RestoreValues(settings);

	BMessenger messenger(this);
	fOpenPanel = new BFilePanel(B_OPEN_PANEL, &messenger, NULL, B_FILE_NODE, false);
	fSavePanel = new BFilePanel(B_SAVE_PANEL, &messenger, NULL, B_FILE_NODE, false);

	BRect frame;
	if (settings.FindRect("main_window_rect", &frame) == B_OK) {
		MoveTo(frame.LeftTop());
		ResizeTo(frame.Width(), frame.Height());
	}
	MoveOnScreen();

	// Use MessageRunner as temporary solution for status bar
	BMessage* updateMessage = new BMessage(M_UPDATE_STATUSBAR);
	fStatusUpdater = new BMessageRunner(this, updateMessage, 100000);

	if (!fSaveTextOnExit && fInsertClipboard) {
		// Get data from clipboard
		BMessage* clipboard;
		if (be_clipboard->Lock()) {
			clipboard = be_clipboard->Data();
			if (clipboard != nullptr) {
				const char* text = nullptr;
				ssize_t textLen;
				if (clipboard->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen)
						== B_OK
					&& text != nullptr) {
					// Only insert if the text is not empty
					if (textLen > 0)
						fTextView->SetText(text, textLen);
				}
			}
			be_clipboard->Unlock();
		}
	}
}


MainWindow::~MainWindow()
{
	_SaveSettings();
	delete fOpenPanel;
	delete fSavePanel;
}


void
MainWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case B_UNDO:
			if (fTextView->CanUndo())
				fTextView->Undo();
			break;
		case B_REDO:
			if (fTextView->CanRedo())
				fTextView->Redo();
			break;
		case B_CUT:
		case B_COPY:
		case B_PASTE:
		case B_SELECT_ALL:
		{
			BView* focusView = CurrentFocus();
			if (focusView) {
				BMessenger messenger(focusView);
				messenger.SendMessage(msg);
				return;
			}
		}
		case M_FILE_NEW:
			fTextView->SetText("");
			fFilePath = "";
			break;
		case B_SIMPLE_DATA:
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			if (msg->FindRef("refs", &ref) != B_OK)
				break;
			OpenFile(ref);
		} break;
		case B_SAVE_REQUESTED:
		{
			entry_ref dir;
			BString name;
			if (msg->FindRef("directory", &dir) == B_OK && msg->FindString("name", &name) == B_OK) {
				BPath path(&dir);
				path.Append(name);
				SaveFile(path.Path());
			}
			break;
		}
		case M_FILE_OPEN:
			fOpenPanel->Show();
			break;
		case M_FILE_SAVE:
			if (fFilePath.CountChars() < 1)
				fSavePanel->Show();
			else
				SaveFile(fFilePath.String());
			break;
		case M_FILE_SAVE_AS:
			fSavePanel->Show();
			break;
		case M_UPDATE_STATUSBAR:
			UpdateStatusBar();
			break;
		case M_SHOW_SETTINGS:
		{
			if (!fSettingsWindow) {
				fSettingsWindow = new SettingsWindow(fSaveTextOnExit, fSaveFieldsOnExit,
					fInsertClipboard, fClearSettingsAfterUse, fFontSize, fFontFamily);
				fSettingsWindow->CenterIn(Frame());
				fSettingsWindow->Show();
			} else {
				if (fSettingsWindow->IsHidden())
					fSettingsWindow->Show();
				fSettingsWindow->Activate();
			}
			break;
		}
		case M_APPLY_SETTINGS:
		{
			bool flag;
			int32 number;
			BString text;
			if (msg->FindBool("saveText", &flag) == B_OK)
				fSaveTextOnExit = flag;

			if (msg->FindBool("saveSettings", &flag) == B_OK)
				fSaveFieldsOnExit = flag;

			if (msg->FindBool("insertClipboard", &flag) == B_OK)
				fInsertClipboard = flag;

			if (msg->FindBool("clearSettings", &flag) == B_OK)
				fClearSettingsAfterUse = flag;

			if (msg->FindInt32("fontSize", &number) == B_OK)
				fFontSize = number;

			if (msg->FindString("fontFamily", &text) == B_OK)
				fFontFamily = text;

			// Apply font to the textView
			BFont newFont(be_fixed_font); // Default fallback
			if (!fFontFamily.IsEmpty() || fFontFamily != "System default")
				newFont.SetFamilyAndStyle(fFontFamily.String(), NULL);
			newFont.SetSize(fFontSize);
			fTextView->SetFontAndColor(&newFont);
			break;
		}
		case M_TRANSFORM_UPPERCASE:
			ConvertToUppercase(fTextView);
			break;
		case M_TRANSFORM_LOWERCASE:
			ConvertToLowercase(fTextView);
			break;
		case M_TRANSFORM_CAPITALIZE:
			Capitalize(fTextView);
			break;
		case M_TRANSFORM_TITLE_CASE:
			ConvertToTitlecase(fTextView);
			break;
		case M_TRANSFORM_RANDOM_CASE:
			ConvertToRandomCase(fTextView);
			break;
		case M_TRANSFORM_ALTERNATING_CASE:
			ConvertToAlternatingCase(fTextView);
			break;
		case M_TRANSFORM_TOGGLE_CASE:
			ToggleCase(fTextView);
			break;
		case M_REMOVE_LINE_BREAKS:
			if (fSidebar->getBreakMode() == BREAK_REMOVE_ALL) {
				RemoveLineBreaks(fTextView);
			} else if (fSidebar->getBreakMode() == BREAK_ON) {
				BreakLinesOnDelimiter(fTextView, fSidebar->getBreakModeInput(), fSidebar->getKeepDelimiterValue());
			} else if (fSidebar->getBreakMode() == BREAK_REPLACE) {
				RemoveLineBreaks(fTextView, fSidebar->getBreakModeInput());
			} else if (fSidebar->getBreakMode() == BREAK_AFTER_CHARS) {
				InsertLineBreaks(fTextView, fSidebar->getBreakOnCharsSpinner(),
					fSidebar->getSplitOnWords());
			}
			if (fClearSettingsAfterUse) {
				fSidebar->setBreakModeInput("");
			}
			break;
		case M_TRIM_LINES:
			TrimLines(fTextView);
			break;
		case M_TRANSFORM_REPLACE:
			ReplaceAll(fTextView, fSidebar->getSearchText(), fSidebar->getReplaceText(),
				fSidebar->getReplaceCaseSensitive(), fSidebar->getReplaceFullWords());
			if (fClearSettingsAfterUse) {
				fSidebar->setSearchText("");
				fSidebar->setReplaceText("");
				fSidebar->setReplaceCaseSensitive(false);
				fSidebar->setReplaceFullWords(false);
			}
			break;
		case M_TRIM_EMPTY_LINES:
			TrimEmptyLines(fTextView);
			break;
		case M_TRANSFORM_PREFIX_SUFFIX:
			AddStringsToEachLine(fTextView, fSidebar->getPrefixText(), fSidebar->getSuffixText());
			if (fClearSettingsAfterUse) {
				fSidebar->setPrefixText("");
				fSidebar->setSuffixText("");
			}
			break;
		case M_TRANSFORM_REMOVE_PREFIX_SUFFIX:
			RemoveStringsFromEachLine(fTextView, fSidebar->getPrefixText(),
				fSidebar->getSuffixText());
			if (fClearSettingsAfterUse) {
				fSidebar->setPrefixText("");
				fSidebar->setSuffixText("");
			}
			break;
		case M_TRANSFORM_ROT13:
			ConvertToROT13(fTextView);
			break;
		case M_TRANSFORM_ENCODE_URL:
			URLEncode(fTextView);
			break;
		case M_TRANSFORM_DECODE_URL:
			URLDecode(fTextView);
			break;
		case M_SORT_LINES:
		{
			bool sortAlphabetically = fSidebar->getAlphaSortRadio();
			bool sortAscending = fSidebar->getSortAsc();
			bool caseSensitive = fSidebar->getCaseSortCheck();

			if (sortAlphabetically)
				SortLines(fTextView, sortAscending, caseSensitive);
			else
				SortLinesByLength(fTextView, sortAscending, caseSensitive);
			break;
		}
		case M_INDENT_LINES:
		case M_UNINDENT_LINES:
		{
			if (msg->what == M_INDENT_LINES)
				IndentLines(fTextView, fSidebar->getTabsRadio(), fSidebar->getIndentSpinner());
			else
				UnindentLines(fTextView, fSidebar->getTabsRadio(), fSidebar->getIndentSpinner());
			break;
		}
		case M_MODE_REMOVE_ALL:
		case M_MODE_BREAK_ON:
		case M_MODE_REPLACE_LINE_BREAKS:
		case M_MODE_BREAK_AFTER_CHARS:
			fSidebar->MessageReceived(msg);
			break;
		case M_REMOVE_DUPLICATES:
			RemoveDuplicateLines(fTextView);
			break;
		case M_INSERT_EXAMPLE_TEXT:
			fTextView->SetText(B_TRANSLATE("Haiku is an open-source operating system.\n"
							  "It is fast, simple and elegant.\n"
							  "Developers love its clean architecture.\n"
							  "Users enjoy its intuitive interface.\n"
							  "Start exploring the power of Haiku today."));
			break;
		case M_TRANSFORM_WIP:
			(new BAlert("Not implemented",
				 B_TRANSLATE("Sorry, this functionality has not been implemented "
				 "yet, but it is planned for the near future."),
				 B_TRANSLATE("OK")))
				->Go();
			break;
		case M_TOGGLE_WORD_WRAP:
			fTextView->SetWordWrap(!fTextView->DoesWordWrap());
			break;
		case B_ABOUT_REQUESTED:
			be_app->AboutRequested();
			break;
		case M_SHOW_HELP:
			{
			BString helpText = B_TRANSLATE("TextWorker Help\n\n"
				"Use the tabs on the left to apply transformations such as:\n"
				"- Search and replace\n"
				"- Add/remove prefixes and suffixes\n"
				"- Sort or clean up lines\n"
				"- Adjust indentation\n\n"
				"You can open text files via \"File → Open\" or by dragging them into the window.\n\n"
				"Undo changes with \"Edit → Undo\" or Ctrl+Z.\n"
				"Zoom text using Ctrl + Mouse Wheel.\n\n"
				"Use \"Settings\" to configure font and save options.");
			(new BAlert("Help", helpText.String(), B_TRANSLATE("OK")))->Go();
			break;
			}
		case M_REPORT_A_BUG:
			BUrl("https://github.com/dospuntos/TextWorker/issues/", true).OpenWithPreferredApplication();
			break;
		default:
			BWindow::MessageReceived(msg);
			break;
	}
	fSidebar->InvalidateLayout();
	fSidebar->Invalidate();
}


bool
MainWindow::QuitRequested(void)
{
	if (fSettingsWindow && fSettingsWindow->LockLooper()) {
		fSettingsWindow->Quit();
		fSettingsWindow = nullptr;
	}
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


BMenuBar*
MainWindow::_BuildMenu()
{
	BMenuBar* menuBar = new BMenuBar("menubar");
	BMenu* menu;
	BMenu* subMenu;
	BMenuItem* item;

	fUndoItem = new BMenuItem(B_TRANSLATE("Undo"), new BMessage(B_UNDO), 'Z');
	fRedoItem = new BMenuItem(B_TRANSLATE("Redo"), new BMessage(B_REDO), 'Y');
	fCutItem = new BMenuItem(B_TRANSLATE("Cut"), new BMessage(B_CUT), 'X');
	fCopyItem = new BMenuItem(B_TRANSLATE("Copy"), new BMessage(B_COPY), 'C');
	fPasteItem = new BMenuItem(B_TRANSLATE("Paste"), new BMessage(B_PASTE), 'V');
	fSelectAllItem = new BMenuItem(B_TRANSLATE("Select all"), new BMessage(B_SELECT_ALL), 'A');

	// App menu
	menu = new BMenu(B_TRANSLATE(""));
	menu->AddItem(
		new BMenuItem(B_TRANSLATE("About" B_UTF8_ELLIPSIS), new BMessage(B_ABOUT_REQUESTED)));
	menu->AddItem(
		new BMenuItem(B_TRANSLATE("Help" B_UTF8_ELLIPSIS), new BMessage(M_SHOW_HELP), 'H'));
	menu->AddItem(
		new BMenuItem(B_TRANSLATE("Report a bug" B_UTF8_ELLIPSIS), new BMessage(M_REPORT_A_BUG)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(B_TRANSLATE("Settings" B_UTF8_ELLIPSIS),
		new BMessage(M_SHOW_SETTINGS), ',', B_COMMAND_KEY));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(B_TRANSLATE("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q'));

	IconMenuItem* iconMenu;
	iconMenu = new IconMenuItem(menu, NULL, kApplicationSignature, B_MINI_ICON);
	menuBar->AddItem(menu);

	// 'File' menu
	menu = new BMenu(B_TRANSLATE("File"));

	menu->AddItem(new BMenuItem(B_TRANSLATE("New"), new BMessage(M_FILE_NEW), 'N'));
	menu->AddItem(
		new BMenuItem(B_TRANSLATE("Open" B_UTF8_ELLIPSIS), new BMessage(M_FILE_OPEN), 'O'));
	menu->AddSeparatorItem();

	menu->AddItem(new BMenuItem(B_TRANSLATE("Save"), new BMessage(M_FILE_SAVE), 'S'));
	menu->AddItem(new BMenuItem(B_TRANSLATE("Save as" B_UTF8_ELLIPSIS),
		new BMessage(M_FILE_SAVE_AS), 'S', B_SHIFT_KEY));

	menuBar->AddItem(menu);

	// 'Edit' menu
	menu = new BMenu(B_TRANSLATE("Edit"));

	menu->AddItem(fUndoItem);
	menu->AddItem(fRedoItem);
	menu->AddSeparatorItem();
	menu->AddItem(fCutItem);
	menu->AddItem(fCopyItem);
	menu->AddItem(fPasteItem);
	menu->AddSeparatorItem();
	menu->AddItem(fSelectAllItem);
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(B_TRANSLATE("Insert example text"),
		new BMessage(M_INSERT_EXAMPLE_TEXT), 'E'));

	menuBar->AddItem(menu);

	// === TEXT CASE MENU ===
	BMenu* textCaseMenu = new BMenu(B_TRANSLATE("Text case"));

	BMenuItem* uppercaseItem
		= new BMenuItem(B_TRANSLATE("UPPERCASE"), new BMessage(M_TRANSFORM_UPPERCASE));
	uppercaseItem->SetShortcut('u', B_COMMAND_KEY | B_OPTION_KEY);
	textCaseMenu->AddItem(uppercaseItem);

	BMenuItem* lowercaseItem
		= new BMenuItem(B_TRANSLATE("lowercase"), new BMessage(M_TRANSFORM_LOWERCASE));
	lowercaseItem->SetShortcut('l', B_COMMAND_KEY | B_OPTION_KEY);
	textCaseMenu->AddItem(lowercaseItem);

	BMenuItem* capitalizeItem
		= new BMenuItem(B_TRANSLATE("Capitalize"), new BMessage(M_TRANSFORM_CAPITALIZE));
	capitalizeItem->SetShortcut('c', B_COMMAND_KEY | B_OPTION_KEY);
	textCaseMenu->AddItem(capitalizeItem);

	BMenuItem* titleCaseItem
		= new BMenuItem(B_TRANSLATE("Title Case"), new BMessage(M_TRANSFORM_TITLE_CASE));
	titleCaseItem->SetShortcut('t', B_COMMAND_KEY | B_OPTION_KEY);
	textCaseMenu->AddItem(titleCaseItem);

	BMenuItem* randomCaseItem
		= new BMenuItem(B_TRANSLATE("RaNDoM caSE"), new BMessage(M_TRANSFORM_RANDOM_CASE));
	randomCaseItem->SetShortcut('r', B_COMMAND_KEY | B_OPTION_KEY);
	textCaseMenu->AddItem(randomCaseItem);

	BMenuItem* alternatingCaseItem = new BMenuItem(B_TRANSLATE("AlTeRnAtInG cAsE"),
		new BMessage(M_TRANSFORM_ALTERNATING_CASE));
	alternatingCaseItem->SetShortcut('a', B_COMMAND_KEY | B_OPTION_KEY);
	textCaseMenu->AddItem(alternatingCaseItem);

	BMenuItem* toggleCaseItem
		= new BMenuItem(B_TRANSLATE("Toggle case"), new BMessage(M_TRANSFORM_TOGGLE_CASE));
	toggleCaseItem->SetShortcut('t', B_COMMAND_KEY | B_OPTION_KEY | B_SHIFT_KEY);
	textCaseMenu->AddItem(toggleCaseItem);

	menuBar->AddItem(textCaseMenu);

	// ====== Transform Menu Structure ======
	BMenu* transformMenu = new BMenu(B_TRANSLATE("Transform"));

	transformMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Search and replace"), new BMessage(M_TRANSFORM_REPLACE)));

	// === ENCODE/DECODE MENU ===
	BMenu* encodeMenu = new BMenu(B_TRANSLATE("Encode/Decode"));

	BMenuItem* urlEncodeItem
		= new BMenuItem(B_TRANSLATE("URL encode"), new BMessage(M_TRANSFORM_ENCODE_URL));
	encodeMenu->AddItem(urlEncodeItem);

	BMenuItem* urlDecodeItem
		= new BMenuItem(B_TRANSLATE("URL decode"), new BMessage(M_TRANSFORM_DECODE_URL));
	encodeMenu->AddItem(urlDecodeItem);

	BMenuItem* rot13Item
		= new BMenuItem(B_TRANSLATE("ROT-13 encode/decode"), new BMessage(M_TRANSFORM_ROT13));
	rot13Item->SetShortcut('m', B_COMMAND_KEY | B_OPTION_KEY);
	encodeMenu->AddItem(rot13Item);

	transformMenu->AddItem(encodeMenu);

	// === LINE TOOLS MENU ===
	BMenu* lineMenu = new BMenu(B_TRANSLATE("Line Tools"));

	BMenuItem* removeLineBreaksItem
		= new BMenuItem(B_TRANSLATE("Remove line breaks"), new BMessage(M_REMOVE_LINE_BREAKS));
	removeLineBreaksItem->SetShortcut('b', B_COMMAND_KEY | B_OPTION_KEY);
	lineMenu->AddItem(removeLineBreaksItem);

	BMenuItem* insertLineBreaksItem = new BMenuItem(B_TRANSLATE("Insert line breaks (width)"),
		new BMessage(M_INSERT_LINE_BREAKS));
	insertLineBreaksItem->SetShortcut('i', B_COMMAND_KEY | B_OPTION_KEY);
	lineMenu->AddItem(insertLineBreaksItem);

	BMenuItem* breakOnDelimiterItem = new BMenuItem(B_TRANSLATE("Break lines on delimiter"),
		new BMessage(M_BREAK_LINES_ON_DELIMITER));
	breakOnDelimiterItem->SetShortcut('d', B_COMMAND_KEY | B_OPTION_KEY);
	lineMenu->AddItem(breakOnDelimiterItem);

	BMenuItem* trimItem = new BMenuItem(B_TRANSLATE("Trim whitespace"), new BMessage(M_TRIM_LINES));
	trimItem->SetShortcut('w', B_COMMAND_KEY | B_OPTION_KEY);
	lineMenu->AddItem(trimItem);

	BMenuItem* removeEmptyLinesItem
		= new BMenuItem(B_TRANSLATE("Remove empty lines"), new BMessage(M_TRIM_EMPTY_LINES));
	removeEmptyLinesItem->SetShortcut('e', B_COMMAND_KEY | B_OPTION_KEY);
	lineMenu->AddItem(removeEmptyLinesItem);

	transformMenu->AddItem(lineMenu);

	// Add the whole Transform menu to the menu bar
	menuBar->AddItem(transformMenu);

	return menuBar;
}


status_t
MainWindow::_LoadSettings(BMessage& settings)
{
	BPath path;
	status_t status;
	status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (status != B_OK)
		return status;

	status = path.Append(kSettingsFile);
	if (status != B_OK)
		return status;

	BFile file;
	status = file.SetTo(path.Path(), B_READ_ONLY);
	if (status != B_OK)
		return status;

	return settings.Unflatten(&file);
}


status_t
MainWindow::_SaveSettings()
{
	BPath path;
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (status != B_OK)
		return status;

	status = path.Append(kSettingsFile);
	if (status != B_OK)
		return status;

	BFile file;
	status = file.SetTo(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	if (status != B_OK)
		return status;

	BMessage settings;
	status = settings.AddRect("main_window_rect", Frame());
	settings.AddBool("saveTextOnExit", fSaveTextOnExit);
	settings.AddBool("saveFieldsOnExit", fSaveFieldsOnExit);
	settings.AddBool("insertClipboard", fInsertClipboard);
	settings.AddBool("clearAfterUse", fClearSettingsAfterUse);
	settings.AddInt32("fontSize", fFontSize);
	settings.AddString("fontFamily", fFontFamily);

	// Save textView
	if (fTextView && fSaveTextOnExit)
		settings.AddString("textViewContent", fTextView->Text());

	// Save sidebar text inputs
	if (fSidebar && fSaveFieldsOnExit) {
		settings.AddInt32("activeTab", fSidebar->Selection());
		// Search/replace
		settings.AddString("replaceSearchString", fSidebar->getSearchText());
		settings.AddString("replaceWithString", fSidebar->getReplaceText());
		settings.AddBool("replaceCaseSensitive", fSidebar->getReplaceCaseSensitive());
		settings.AddBool("replaceFullWords", fSidebar->getReplaceFullWords());

		// Line breaks
		settings.AddInt32("breakMode", fSidebar->getBreakMode());
		settings.AddString("breakModeInput", fSidebar->getBreakModeInput());
		settings.AddInt32("breakOnChars", fSidebar->getBreakOnCharsSpinner());
		settings.AddBool("splitOnWords", fSidebar->getSplitOnWords());
		settings.AddBool("keepDelimiter", fSidebar->getKeepDelimiterValue());

		// Prefix/suffix
		settings.AddString("prefixInput", fSidebar->getPrefixText());
		settings.AddString("suffixInput", fSidebar->getSuffixText());

		// Indent/unindent
		settings.AddInt32("indentSize", fSidebar->getIndentSpinner());
		settings.AddBool("tabsRadio", fSidebar->getTabsRadio());

		// Sort lines
		settings.AddBool("sortType", fSidebar->getAlphaSortRadio());
		settings.AddBool("sortAsc", fSidebar->getSortAsc());
		settings.AddBool("sortCase", fSidebar->getCaseSortCheck());
	}

	if (status == B_OK)
		status = settings.Flatten(&file);

	return status;
}


void
MainWindow::_RestoreValues(BMessage& settings)
{
	// Set defaults
	fFontSize = be_fixed_font->Size();
	fFontFamily = "System default";
	fSaveTextOnExit = false;
	fSaveFieldsOnExit = false;
	fInsertClipboard = true;
	fClearSettingsAfterUse = false;

	BString text;
	int32 number;
	bool flag;

	// General settings
	if (settings.FindBool("saveTextOnExit", &flag) == B_OK)
		fSaveTextOnExit = flag;

	if (settings.FindBool("saveFieldsOnExit", &flag) == B_OK)
		fSaveFieldsOnExit = flag;

	if (settings.FindBool("insertClipboard", &flag) == B_OK)
		fInsertClipboard = flag;

	if (settings.FindBool("clearAfterUse", &flag) == B_OK)
		fClearSettingsAfterUse = flag;

	if (settings.FindInt32("fontSize", &number) == B_OK)
		fFontSize = number;

	if (settings.FindString("fontFamily", &text) == B_OK)
		fFontFamily = text;

	// Apply the restored font settings to textView
	BFont font;
	if (fFontFamily == "System default")
		font = *be_fixed_font;
	else
		font.SetFamilyAndStyle(fFontFamily.String(), NULL);

	font.SetSize(fFontSize);
	fTextView->SetFontAndColor(&font);

	if (settings.FindString("textViewContent", &text) == B_OK && fSaveTextOnExit)
		fTextView->SetText(text);

	if (fSidebar && fSaveFieldsOnExit) {
		if (settings.FindInt32("activeTab", &number) == B_OK)
			fSidebar->Select(number);

		// Search/replace
		if (settings.FindString("replaceSearchString", &text) == B_OK)
			fSidebar->setSearchText(text);
		if (settings.FindString("replaceWithString", &text) == B_OK)
			fSidebar->setReplaceText(text);
		if (settings.FindBool("replaceCaseSensitive", &flag) == B_OK)
			fSidebar->setReplaceCaseSensitive(flag);
		if (settings.FindBool("replaceFullWords", &flag) == B_OK)
			fSidebar->setReplaceFullWords(flag);

		// Line breaks
		if (settings.FindInt32("breakMode", &number) == B_OK)
			fSidebar->setBreakMode(number);
		if (settings.FindString("breakModeInput", &text) == B_OK)
			fSidebar->setBreakModeInput(text);
		if (settings.FindInt32("breakOnChars", &number) == B_OK)
			fSidebar->setBreakOnCharsSpinner(number);
		if (settings.FindBool("splitOnWords", &flag) == B_OK)
			fSidebar->setSplitOnWords(flag);
		if (settings.FindBool("keepDelimiter", &flag) == B_OK)
			fSidebar->setKeepDelimiterValue(flag);
		// Todo: enable/disable corresponding fields for restored breakMode

		// Prefix/suffix
		if (settings.FindString("prefixInput", &text) == B_OK)
			fSidebar->setPrefixText(text);
		if (settings.FindString("suffixInput", &text) == B_OK)
			fSidebar->setSuffixText(text);

		// Indent/unindent
		if (settings.FindInt32("indentSize", &number) == B_OK)
			fSidebar->setIndentSpinner(number);
		if (settings.FindBool("tabsRadio", &flag) == B_OK)
			fSidebar->setTabsRadio(flag);

		// Sort lines
		if (settings.FindBool("sortType", &flag) == B_OK)
			fSidebar->setAlphaSortRadio(flag);
		if (settings.FindBool("sortAsc", &flag) == B_OK)
			fSidebar->setSortAsc(flag);
		if (settings.FindBool("sortCase", &flag) == B_OK)
			fSidebar->setCaseSortCheck(flag);
	}
}


void
MainWindow::UpdateStatusBar()
{
	// Calculate Row/column based on cursor position
	int32 start, end;
	fTextView->GetSelection(&start, &end);
	BString textBuffer = fTextView->Text();

	int32 row = 1;
	for (int32 i = 0; i < start; ++i) {
		if (textBuffer[i] == '\n')
			row++;
	}
	int col = start - textBuffer.FindLast('\n', start);

	// Calculate character count and word count
	textBuffer.ReplaceAll('\n', ' ');
	textBuffer.ReplaceAll('\t', ' ');
	int32 charCount = fTextView->TextLength();
	int32 wordCount = 0;
	bool inWord = false;
	for (int32 i = 0; i < charCount; ++i) {
		if (std::isspace(textBuffer[i])) {
			inWord = false;
		} else if (!inWord) {
			inWord = true;
			wordCount++;
		}
	}

	// Update the status bar text
	BString statusText;
	statusText.SetToFormat(B_TRANSLATE_COMMENT("%d:%d | Chars: %d | Words: %d",
							   "Statusbar text - only change Chars and Words"),
		row, col, charCount, wordCount);
	fStatusBar->SetText(statusText.String());
}


void
MainWindow::OpenFile(const entry_ref& ref)
{
	BEntry entry(&ref, true);
	entry_ref realRef;
	entry.GetRef(&realRef);

	BFile file(&realRef, B_READ_ONLY);
	if (file.InitCheck() != B_OK)
		return;

	// Check MIME type
	BNode node(&realRef);
	BNodeInfo nodeInfo(&node);
	char mimeType[B_MIME_TYPE_LENGTH];
	bool isText = false;

	if (nodeInfo.GetType(mimeType) == B_OK && strncmp(mimeType, "text/", 5) == 0)
		isText = true;

	if (!isText) {
		file.Seek(0, SEEK_SET);
		const off_t kMaxCheckSize = 1024;
		off_t fileSize;
		if (file.GetSize(&fileSize) == B_OK) {
			uint8 buffer[kMaxCheckSize];
			ssize_t bytesRead = file.ReadAt(0, buffer, std::min(fileSize, kMaxCheckSize));
			if (bytesRead > 0) {

				// Count non-printable characters
				int nonPrintable = 0;
				for (ssize_t i = 0; i < bytesRead; ++i) {
					char c = buffer[i];
					if ((c < 32 || c > 126) && c != '\n' && c != '\r' && c != '\t') {
						if (++nonPrintable > 10)
							break;
					}
				}
				if (nonPrintable <= 10)
					isText = true;
			}
		}
	}


	if (!isText) {
		(new BAlert("Error", B_TRANSLATE("The selected file is not a recognized text file."),
			 B_TRANSLATE("OK")))
			->Go();
		return;
	}

	fTextView->SetText("");
	if (BTranslationUtils::GetStyledText(&file, fTextView) == B_OK) {
		BPath path(&realRef);
		fFilePath = path.Path();
		BString windowTitle(kApplicationName);
		windowTitle << ": " << path.Leaf();
		SetTitle(windowTitle.String());
	}
}


void
MainWindow::SaveFile(const char* path)
{
	BFile file;
	if (file.SetTo(path, B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE) != B_OK)
		return;

	if (BTranslationUtils::PutStyledText(fTextView, &file) == B_OK) {
		fFilePath = path;

		BPath p(path);
		BString windowTitle(kApplicationName);
		windowTitle << ": " << p.Leaf();
		SetTitle(windowTitle.String());

		BNodeInfo nodeInfo(&file);
		nodeInfo.SetType("text/plain");
	}
}


BBitmap*
MainWindow::ResourceToBitmap(const char* resName)
{
	image_info info;
	int32 cookie = 0;
	while (get_next_image_info(0, &cookie, &info) == B_OK) {
		BFile file(info.name, B_READ_ONLY);
		if (file.InitCheck() == B_OK) {
			BResources res(&file);
			size_t size;
			const void* data = res.LoadResource(B_VECTOR_ICON_TYPE, resName, &size);
			if (data) {
				BBitmap* icon = new BBitmap(BRect(0, 0, 23, 23), B_RGBA32);
				if (BIconUtils::GetVectorIcon((const uint8*)data, size, icon) == B_OK)
					return icon;
				delete icon;
			}
		}
		break; // stop after the first image
	}
	return nullptr;
}


void
MainWindow::MenusBeginning()
{
	BView* focus = CurrentFocus();
	BTextView* textView = nullptr;

	if (auto control = dynamic_cast<BTextControl*>(focus))
		textView = control->TextView();
	else
		textView = dynamic_cast<BTextView*>(focus);

	// Check if the view supports editing and selection
	bool hasTextView = (textView != nullptr);
	bool hasSelection = false;
	if (hasTextView) {
		int32 start = 0, end = 0;
		textView->GetSelection(&start, &end);
		hasSelection = (start != end);
	}

	// Enable/disable menu items based on state
	fCutItem->SetEnabled(hasTextView && hasSelection && textView->IsEditable());
	fCopyItem->SetEnabled(hasTextView && hasSelection);
	fPasteItem->SetEnabled(hasTextView && textView->IsEditable());
	fSelectAllItem->SetEnabled(hasTextView);
	fUndoItem->SetEnabled(fTextView->CanUndo());
	fRedoItem->SetEnabled(fTextView->CanRedo());
}
