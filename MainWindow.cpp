#include "MainWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Bitmap.h>
#include <Button.h>
#include <Catalog.h>
#include <Clipboard.h>
#include <ControlLook.h>
#include <File.h>
#include <FindDirectory.h>
#include <Font.h>
#include <IconUtils.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuItem.h>
#include <NodeInfo.h>
#include <Path.h>
#include <RecentItems.h>
#include <Resources.h>
#include <Screen.h>
#include <ScrollView.h>
#include <TranslationUtils.h>
#include <TranslatorRoster.h>
#include <Url.h>
#include <View.h>
#include <string>

#include "Constants.h"
#include "IconMenuItem.h"
#include "SettingsWindow.h"
#include "TextUtils.h"
#include "Toolbar.h"

static const char* kSettingsFile = "TextWorker_settings";
static const char* kIssueTracker = "https://github.com/dospuntos/TextWorker/issues/";


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainView"


MainWindow::MainWindow(void)
	:
	BWindow(BRect(100, 100, 900, 800), kApplicationName, B_DOCUMENT_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE)
{
	BMenuBar* menuBar = _BuildMenu();

	fTextView = new UndoableTextView("TextView");
	fTextView->MakeEditable(true);
	const float spacing = be_control_look->DefaultItemSpacing() / 2;
	fTextView->SetInsets(spacing, spacing, spacing, spacing);

	fScrollView = new BScrollView("TextViewScroll", fTextView, B_WILL_DRAW | B_FRAME_EVENTS, true,
		true, B_FANCY_BORDER);

	fToolbar = CreateToolbar(this);
	fToolbar->SetActionPressed(M_TOGGLE_WORD_WRAP, fTextView->DoesWordWrap());
	fSidebar = new Sidebar();

	fStatusBar = new BStringView("StatusBar", "");
	fStatusBar->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_VERTICAL_CENTER));
	fMessageBar = new BStringView("MessageBar", "");
	fMessageBar->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_VERTICAL_CENTER));

	fSettingsWindow = nullptr;

	// Layout
	// clang-format off
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(menuBar, 0)
		.Add(fToolbar, 0)
		.SetInsets(0, 0, 0, 2)
		.AddGroup(B_HORIZONTAL, -2)
			.Add(fSidebar, 0)
			.Add(fScrollView, 1)
			.SetInsets(-2, B_USE_HALF_ITEM_INSETS, -2, B_USE_HALF_ITEM_INSETS)
		.End()
		.AddGroup(B_HORIZONTAL, 0)
			.Add(fStatusBar, 0)
			.Add(fMessageBar, 0)
			.SetInsets(B_USE_HALF_ITEM_INSETS, 0, be_control_look->GetScrollBarWidth() + spacing, 0)
		.End();
	// clang-format on

	_UpdateWindowTitle();
	BMessage settings;
	_LoadSettings(settings);
	_RestoreValues(settings);

	// Set min size of main window
	BSize toolbarSize = fToolbar->PreferredSize();
	BSize sidebarSize = fSidebar->PreferredSize();
	const float minWidth = toolbarSize.width + 20;
	const float minHeight = sidebarSize.height + 100;
	SetSizeLimits(minWidth, B_SIZE_UNLIMITED, minHeight, B_SIZE_UNLIMITED);

	BMessenger messenger(this);
	fOpenPanel = new BFilePanel(B_OPEN_PANEL, &messenger, NULL, B_FILE_NODE, false);
	fSavePanel = new BFilePanel(B_SAVE_PANEL, &messenger, NULL, B_FILE_NODE, false);

	BRect frame;
	if (settings.FindRect("main_window_rect", &frame) == B_OK) {
		MoveTo(frame.LeftTop());
		ResizeTo(frame.Width(), frame.Height());
	}
	MoveOnScreen();

	if (!fSaveTextOnExit && fInsertClipboard) {
		BString clipboardText;
		if (_GetClipboardText(clipboardText))
			fTextView->SetText(clipboardText);
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
		{
			fTextView->SetText("");
			fFilePath = "";
			fLastSavedText = "";
			_UpdateWindowTitle();
			break;
		}
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
			_UpdateStatusBar();
			_UpdateToolbarState();
			_UpdateWindowTitle();
			break;
		case M_SHOW_SETTINGS:
		{
			if (!fSettingsWindow) {
				fSettingsWindow
					= new SettingsWindow(fSaveTextOnExit, fSaveFieldsOnExit, fInsertClipboard,
						fClearSettingsAfterUse, fFontSize, fFontFamily);
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
			_UpdateStatusMessage("Settings updated");
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
				RemoveLineBreaks(fTextView, "");
			} else if (fSidebar->getBreakMode() == BREAK_ON) {
				BreakLinesOnDelimiter(fTextView, fSidebar->getBreakModeInput(),
					fSidebar->getKeepDelimiterValue());
			} else if (fSidebar->getBreakMode() == BREAK_REPLACE) {
				RemoveLineBreaks(fTextView, fSidebar->getBreakModeInput());
			} else if (fSidebar->getBreakMode() == BREAK_AFTER_CHARS) {
				InsertLineBreaks(fTextView, fSidebar->getBreakOnCharsSpinner(),
					fSidebar->getSplitOnWords());
			}
			if (fClearSettingsAfterUse)
				fSidebar->setBreakModeInput("");
			break;
		case M_TRIM_LINES:
			TrimWhitespace(fTextView);
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
			if (msg->what == M_INDENT_LINES) {
				IndentLines(fTextView, fSidebar->getTabsRadio(), fSidebar->getIndentSpinner());
			} else {
				UnindentLines(fTextView, fSidebar->getTabsRadio(), fSidebar->getIndentSpinner());
			}
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
			fToolbar->SetActionPressed(M_TOGGLE_WORD_WRAP, fTextView->DoesWordWrap());
			break;
		case B_ABOUT_REQUESTED:
			be_app->AboutRequested();
			break;
		case M_SHOW_HELP:
		{
			BString helpText
				= B_TRANSLATE("TextWorker Help\n\n"
							  "Use the tabs on the left to apply transformations such as:\n"
							  "- Search and replace\n"
							  "- Add/remove prefixes and suffixes\n"
							  "- Sort or clean up lines\n"
							  "- Adjust indentation\n\n"
							  "You can open text files via \"File → Open\" or by dragging them "
							  "into the window.\n\n"
							  "Undo changes with \"Edit → Undo\" or Alt+Z.\n\n"
							  "Use \"Settings\" to configure font and save options.");
			(new BAlert("Help", helpText.String(), B_TRANSLATE("OK")))->Go();
			break;
		}
		case M_SHOW_STATS:
			ShowTextStats(fTextView);
			break;
		case M_SHOW_STATUS:
		{
			BString text;
			if (msg->FindString("text", &text) == B_OK)
				_UpdateStatusMessage(text);
			break;
		}
		case M_CLEAR_STATUS:
			if (fMessageBar)
				fMessageBar->SetText("");
			break;
		case M_REPORT_A_BUG:
		{
			std::string uri = "https://github.com/dospuntos/TextWorker/issues/";
			BUrl url = uri.c_str();
			url.OpenWithPreferredApplication();
			break;
		}
		default:
			BWindow::MessageReceived(msg);
			break;
	}
	_UpdateStatusBar();
	_UpdateToolbarState();
	_UpdateWindowTitle();
}


bool
MainWindow::QuitRequested(void)
{
	if (fSettingsWindow && fSettingsWindow->LockLooper()) {
		fSettingsWindow->Quit();
		fSettingsWindow = nullptr;
	}
	if (!fSaveTextOnExit && IsDocumentModified() == true)
		this->PostMessage(M_FILE_SAVE_AS);
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

	fSaveMenuItem = new BMenuItem(B_TRANSLATE("Save"), new BMessage(M_FILE_SAVE), 'S');
	fUndoItem = new BMenuItem(B_TRANSLATE("Undo"), new BMessage(B_UNDO), 'Z');
	fRedoItem = new BMenuItem(B_TRANSLATE("Redo"), new BMessage(B_REDO), 'Y');
	fCutItem = new BMenuItem(B_TRANSLATE("Cut"), new BMessage(B_CUT), 'X');
	fCopyItem = new BMenuItem(B_TRANSLATE("Copy"), new BMessage(B_COPY), 'C');
	fPasteItem = new BMenuItem(B_TRANSLATE("Paste"), new BMessage(B_PASTE), 'V');
	fSelectAllItem = new BMenuItem(B_TRANSLATE("Select all"), new BMessage(B_SELECT_ALL), 'A');

	// App menu
	menu = new BMenu("");
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

	IconMenuItem* iconMenu = new IconMenuItem(menu, NULL, kApplicationSignature, B_MINI_ICON);
	menuBar->AddItem(iconMenu);

	// 'File' menu
	menu = new BMenu(B_TRANSLATE("File"));

	menu->AddItem(new BMenuItem(B_TRANSLATE("New"), new BMessage(M_FILE_NEW), 'N'));
	BMenuItem* openItem
		= new BMenuItem(BRecentFilesList::NewFileListMenu(B_TRANSLATE("Open" B_UTF8_ELLIPSIS), NULL,
							NULL, be_app, 9, true, NULL, kApplicationSignature),
			new BMessage(M_FILE_OPEN));
	openItem->SetShortcut('O', 0);
	menu->AddItem(openItem);

	menu->AddSeparatorItem();

	menu->AddItem(fSaveMenuItem);
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

	// 'Text case' menu
	BMenu* textCaseMenu = new BMenu(B_TRANSLATE("Text case"));

	textCaseMenu->AddItem(new BMenuItem(B_TRANSLATE("UPPERCASE"),
		new BMessage(M_TRANSFORM_UPPERCASE), 'U', B_COMMAND_KEY));
	textCaseMenu->AddItem(new BMenuItem(B_TRANSLATE("lowercase"),
		new BMessage(M_TRANSFORM_LOWERCASE), 'L', B_COMMAND_KEY));
	textCaseMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Capitalize"), new BMessage(M_TRANSFORM_CAPITALIZE)));
	textCaseMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Title Case"), new BMessage(M_TRANSFORM_TITLE_CASE)));
	textCaseMenu->AddItem(
		new BMenuItem(B_TRANSLATE("RaNDoM caSE"), new BMessage(M_TRANSFORM_RANDOM_CASE)));
	textCaseMenu->AddItem(
		new BMenuItem(B_TRANSLATE("AlTeRnAtInG cAsE"), new BMessage(M_TRANSFORM_ALTERNATING_CASE)));
	textCaseMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Toggle case"), new BMessage(M_TRANSFORM_TOGGLE_CASE)));

	menuBar->AddItem(textCaseMenu);

	// 'Transform' menu
	BMenu* transformMenu = new BMenu(B_TRANSLATE("Transform"));

	transformMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Search/replace"), new BMessage(M_TRANSFORM_REPLACE)), 'H');
	transformMenu->AddItem(new BMenuItem(B_TRANSLATE("Add prefix/suffix"),
		new BMessage(M_TRANSFORM_PREFIX_SUFFIX), 'P'));
	transformMenu->AddItem(new BMenuItem(B_TRANSLATE("Remove prefix/suffix"),
		new BMessage(M_TRANSFORM_REMOVE_PREFIX_SUFFIX), 'P', B_SHIFT_KEY));
	transformMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Indent lines"), new BMessage(M_INDENT_LINES), 'I'));
	transformMenu->AddItem(new BMenuItem(B_TRANSLATE("Unindent lines"),
		new BMessage(M_UNINDENT_LINES), 'I', B_SHIFT_KEY));

	transformMenu->AddSeparatorItem();

	// 'Encode/decode' submenu
	BMenu* encodeMenu = new BMenu(B_TRANSLATE("Encode/decode"));
	encodeMenu->AddItem(
		new BMenuItem(B_TRANSLATE("URL encode"), new BMessage(M_TRANSFORM_ENCODE_URL)));
	encodeMenu->AddItem(
		new BMenuItem(B_TRANSLATE("URL decode"), new BMessage(M_TRANSFORM_DECODE_URL)));
	encodeMenu->AddItem(
		new BMenuItem(B_TRANSLATE("ROT-13 encode/decode"), new BMessage(M_TRANSFORM_ROT13)));
	transformMenu->AddItem(encodeMenu);

	// 'Line tools' submenu
	BMenu* lineMenu = new BMenu(B_TRANSLATE("Line tools"));
	lineMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Remove line breaks"), new BMessage(M_REMOVE_LINE_BREAKS), 'B'));
	lineMenu->AddItem(new BMenuItem(B_TRANSLATE("Replace line breaks"),
		new BMessage(M_MODE_REPLACE_LINE_BREAKS), 'F'));
	lineMenu->AddItem(new BMenuItem(B_TRANSLATE("Insert line breaks (width)"),
		new BMessage(M_INSERT_LINE_BREAKS), 'I'));
	lineMenu->AddItem(new BMenuItem(B_TRANSLATE("Break lines on delimiter"),
		new BMessage(M_BREAK_LINES_ON_DELIMITER), 'D'));
	transformMenu->AddItem(lineMenu);

	// 'Cleanup' submenu
	BMenu* cleanMenu = new BMenu(B_TRANSLATE("Cleanup"));
	cleanMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Trim whitespace"), new BMessage(M_TRIM_LINES), 'G'));
	cleanMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Remove empty lines"), new BMessage(M_TRIM_EMPTY_LINES), 'T'));
	cleanMenu->AddItem(new BMenuItem(B_TRANSLATE("Remove duplicate lines"),
		new BMessage(M_REMOVE_DUPLICATES), 'R'));
	transformMenu->AddItem(cleanMenu);

	transformMenu->AddSeparatorItem();

	transformMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Sort lines"), new BMessage(M_SORT_LINES), 'J'));

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
	settings.AddBool("wrapLines", fTextView->DoesWordWrap());

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

	if (settings.FindBool("wrapLines", &flag) == B_OK)
		fTextView->SetWordWrap(flag);

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
	else
		fTextView->SetText("");

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
	_UpdateStatusBar();
	_UpdateToolbarState();
	_UpdateWindowTitle();
}


void
MainWindow::_UpdateStatusBar()
{
	// Calculate Row/column based on cursor position
	int32 start, end;
	fTextView->GetSelection(&start, &end);
	BString textBuffer = fTextView->Text();

	int32 lineCount = CountLines(textBuffer);
	int32 wordCount = CountWords(textBuffer);
	int32 charCount = textBuffer.Length();

	int32 row = 1;
	for (int32 i = 0; i < start; i++) {
		if (textBuffer[i] == '\n')
			row++;
	}
	int col = start - textBuffer.FindLast('\n', start);

	// Update the status bar text
	BString statusText;
	statusText.SetToFormat(B_TRANSLATE_COMMENT("%d:%d | Chars: %d | Words: %d | Lines: %d",
							   "Statusbar text - only change Chars, Words and Lines"),
		row, col, charCount, wordCount, lineCount);
	if (IsDocumentModified())
		statusText << " | " << B_TRANSLATE("Modified");
	fStatusBar->SetText(statusText.String());
}


void
MainWindow::_UpdateStatusMessage(BString message)
{
	if (fStatusBar) {
		fMessageBar->SetText(message.String());

		// Set up 5-second timer to clear message
		BMessage clearMsg(M_CLEAR_STATUS);
		BMessageRunner::StartSending(this, &clearMsg, 5000000, 1);
	}
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
	}
	fLastSavedText = fTextView->Text();
	_UpdateWindowTitle();
}


void
MainWindow::SaveFile(const char* path)
{
	BFile file;
	if (file.SetTo(path, B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE) != B_OK)
		return;

	if (BTranslationUtils::WriteStyledEditFile(fTextView, &file) == B_OK) {
		fFilePath = path;

		BNodeInfo nodeInfo(&file);
		nodeInfo.SetType("text/plain");
	}
	fLastSavedText = fTextView->Text();
	_UpdateWindowTitle();
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
	if (fSaveMenuItem)
		fSaveMenuItem->SetEnabled(IsDocumentModified());
	fCutItem->SetEnabled(hasTextView && hasSelection && textView->IsEditable());
	fCopyItem->SetEnabled(hasTextView && hasSelection);
	fPasteItem->SetEnabled(hasTextView && textView->IsEditable());
	fSelectAllItem->SetEnabled(hasTextView);
	fUndoItem->SetEnabled(fTextView->CanUndo());
	fRedoItem->SetEnabled(fTextView->CanRedo());
}


void
MainWindow::_UpdateToolbarState()
{
	if (!fToolbar || !fTextView)
		return;

	bool hasSelection = false;
	int32 start = 0, end = 0;
	fTextView->GetSelection(&start, &end);
	hasSelection = (start != end);

	fToolbar->SetActionEnabled(B_UNDO, fTextView->CanUndo());
	fToolbar->SetActionEnabled(B_REDO, fTextView->CanRedo());

	fToolbar->SetActionEnabled(M_FILE_SAVE, IsDocumentModified());

	fToolbar->SetActionEnabled(B_PASTE, _ClipboardHasText());
	fToolbar->SetActionEnabled(B_COPY, hasSelection);
	fToolbar->SetActionEnabled(B_CUT, hasSelection);

	fToolbar->SetActionPressed(M_TOGGLE_WORD_WRAP, fTextView->DoesWordWrap());
}


void
MainWindow::_UpdateWindowTitle()
{
	BString windowTitle = kApplicationName;
	windowTitle.Append(": ");
	windowTitle.Append(fFilePath.IsEmpty() ? B_TRANSLATE("Untitled") : fFilePath.String());
	if (IsDocumentModified()) {
		if (!windowTitle.EndsWith("*"))
			windowTitle.Append("*");
	} else {
		if (windowTitle.EndsWith("*"))
			windowTitle.RemoveLast("*");
	}
	SetTitle(windowTitle);
}



bool
MainWindow::_ClipboardHasText() const
{
	BClipboard* clipboard = be_clipboard;
	if (!clipboard)
		return false;

	BMessage* data = nullptr;
	clipboard->Lock();
	data = clipboard->Data();
	bool hasText = data && data->HasData("text/plain", B_MIME_TYPE);
	clipboard->Unlock();

	return hasText;
}


bool
MainWindow::_GetClipboardText(BString& outText) const
{
	if (!be_clipboard || !be_clipboard->Lock())
		return false;

	BMessage* data = be_clipboard->Data();
	const char* text = nullptr;
	ssize_t textLen = 0;

	bool success = data
		&& data->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen) == B_OK
		&& textLen > 0;

	if (success)
		outText.SetTo(text, textLen);

	be_clipboard->Unlock();
	return success;
}


bool
MainWindow::IsDocumentModified() const
{
	if (!fTextView)
		return false;

	BString currentText = fTextView->Text();
	return currentText != fLastSavedText;
}
