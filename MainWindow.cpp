#include "MainWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Bitmap.h>
#include <Button.h>
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
#include <cctype>

#include "Constants.h"
#include "SettingsWindow.h"
#include "TextUtils.h"
#include "Toolbar.h"

static const char* kSettingsFile = "TextWorker_settings";


MainWindow::MainWindow(void)
	:
	BWindow(BRect(100, 100, 900, 800), kApplicationName, B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE)
{
	BMenuBar* menuBar = _BuildMenu();

	textView = new BTextView("TextView");
	textView->MakeEditable(true);

	scrollView = new BScrollView("TextViewScroll", textView, B_WILL_DRAW | B_FRAME_EVENTS, true,
		true, B_PLAIN_BORDER);

	BFont font(be_fixed_font);
	textView->SetFontAndColor(&font);

	// Toolbar
	toolbar = CreateToolbar(this);

	// Sidebar
	sidebar = new Sidebar();

	// Status bar
	statusBar = new BStringView("StatusBar", "");
	statusBar->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_VERTICAL_CENTER));

	// Layout
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(menuBar, 0)
		.Add(toolbar, 0)
		.SetInsets(2)
		.AddGroup(B_HORIZONTAL, 0)
		.Add(sidebar, 0)
		.Add(scrollView, 1)
		.SetInsets(5, 5, 5, 5)
		.End()
		.Add(statusBar, 0);

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
	statusUpdater = new BMessageRunner(this, updateMessage, 100000);

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
					if (textLen > 0) {
						textView->SetText(text,
							textLen); // Replace default text with clipboard contents
					}
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
		case M_FILE_NEW:
			textView->SetText("");
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
			fSettingsWindow = new SettingsWindow(fSaveTextOnExit, fSaveSettingsOnExit, fInsertClipboard);
			fSettingsWindow->CenterIn(Frame());
			fSettingsWindow->Show();
			break;
		}
		case M_SETTINGS_SAVETEXT:
		{
			if (msg->FindBool("saveText", &fSaveTextOnExit) != B_OK)
				(new BAlert("Error", "Error saving setting", "OK"))->Go();
			break;
		}
		case M_SETTINGS_SAVESETTINGS:
		{
			if (msg->FindBool("saveSettings", &fSaveSettingsOnExit) != B_OK)
				(new BAlert("Error", "Error saving setting", "OK"))->Go();
			break;
		}
		case M_SETTINGS_CLIPBOARD:
		{
			if (msg->FindBool("insertClipboard", &fInsertClipboard) != B_OK)
				(new BAlert("Error", "Error saving setting", "OK"))->Go();
			break;
		}
		case M_TRANSFORM_UPPERCASE:
			ConvertToUppercase(textView);
			break;
		case M_TRANSFORM_LOWERCASE:
			ConvertToLowercase(textView);
			break;
		case M_TRANSFORM_CAPITALIZE:
			Capitalize(textView);
			break;
		case M_TRANSFORM_TITLE_CASE:
			ConvertToTitlecase(textView);
			break;
		case M_TRANSFORM_RANDOM_CASE:
			ConvertToRandomCase(textView);
			break;
		case M_TRANSFORM_ALTERNATING_CASE:
			ConvertToAlternatingCase(textView);
			break;
		case M_TRANSFORM_TOGGLE_CASE:
			ToggleCase(textView);
			break;
		case M_REMOVE_LINE_BREAKS:
			if (sidebar->BreakMode() == BREAK_REMOVE_ALL) {
				RemoveLineBreaks(textView);
			} else if (sidebar->BreakMode() == BREAK_ON) {
				BreakLinesOnDelimiter(textView, sidebar->getBreakModeInput());
			} else if (sidebar->BreakMode() == BREAK_REPLACE) {
				RemoveLineBreaks(textView, sidebar->getBreakModeInput());
			} else if (sidebar->BreakMode() == BREAK_AFTER_CHARS) {
				InsertLineBreaks(textView, atoi(sidebar->getBreakModeInput()),
					sidebar->SplitOnWordsEnabled());
			}
			break;
		case M_TRIM_LINES:
			TrimLines(textView);
			break;
		case M_TRANSFORM_REPLACE:
			ReplaceAll(textView, sidebar->ReplaceSearchText(), sidebar->ReplaceWithText(),
				sidebar->ReplaceCaseSensitive(), sidebar->ReplaceFullWordsOnly());
			sidebar->SetReplaceSearchText("");
			sidebar->SetReplaceWithText("");
			break;
		case M_TRIM_EMPTY_LINES:
			TrimEmptyLines(textView);
			break;
		case M_TRANSFORM_PREFIX_SUFFIX:
			AddStringsToEachLine(textView, sidebar->PrefixText(), sidebar->SuffixText());
			sidebar->SetPrefixText("");
			sidebar->SetSuffixText("");
			break;
		case M_TRANSFORM_ROT13:
			ConvertToROT13(textView);
			break;
		case M_TRANSFORM_ENCODE_URL:
			URLEncode(textView);
			break;
		case M_TRANSFORM_DECODE_URL:
			URLDecode(textView);
			break;
		case M_MODE_REMOVE_ALL:
		case M_MODE_BREAK_ON:
		case M_MODE_REPLACE_LINE_BREAKS:
		case M_MODE_BREAK_AFTER_CHARS:
			sidebar->MessageReceived(msg);
			break;
		case M_INSERT_EXAMPLE_TEXT:
			textView->SetText("Haiku is an open-source operating system.\n"
							  "It is fast, simple and elegant.\n"
							  "Developers love its clean architecture.\n"
							  "Users enjoy its intuitive interface.\n"
							  "Start exploring the power of Haiku today.");
			break;
		case M_TRANSFORM_WIP:
			(new BAlert("Not implemented",
				 "Sorry, this functionality has not been implemented "
				 "yet, but it is planned for the near future.",
				 "OK"))
				->Go();
			break;
		case M_TOGGLE_WORD_WRAP:
			textView->SetWordWrap(!textView->DoesWordWrap());
			break;
		case B_ABOUT_REQUESTED:
			be_app->AboutRequested();
			break;
		default:
			BWindow::MessageReceived(msg);
			break;
	}
	sidebar->InvalidateLayout();
	sidebar->Invalidate();
}


bool
MainWindow::QuitRequested(void)
{
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

	// 'File' menu
	menu = new BMenu("File");

	menu->AddItem(new BMenuItem("New" B_UTF8_ELLIPSIS, new BMessage(M_FILE_NEW), 'N'));
	menu->AddItem(new BMenuItem("Open" B_UTF8_ELLIPSIS, new BMessage(M_FILE_OPEN), 'O'));
	menu->AddSeparatorItem();

	menu->AddItem(new BMenuItem("Save", new BMessage(M_FILE_SAVE), 'S'));
	menu->AddItem(new BMenuItem("Save as...", new BMessage(M_FILE_SAVE_AS)));
	menu->AddSeparatorItem();

	menu->AddItem(new BMenuItem("About" B_UTF8_ELLIPSIS, new BMessage(B_ABOUT_REQUESTED)));
	menu->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q'));

	menuBar->AddItem(menu);

	// 'Edit' menu
	menu = new BMenu("Edit");

	menu->AddItem(new BMenuItem("Undo", new BMessage(B_UNDO), 'Z'));
	menu->AddItem(new BMenuItem("Redo", new BMessage(B_REDO), 'Y'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Cut", new BMessage(B_CUT), 'X'));
	menu->AddItem(new BMenuItem("Copy", new BMessage(B_COPY), 'C'));
	menu->AddItem(new BMenuItem("Paste", new BMessage(B_PASTE), 'V'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Select all", new BMessage(B_SELECT_ALL), 'A'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Insert example text", new BMessage(M_INSERT_EXAMPLE_TEXT), 'E'));

	menuBar->AddItem(menu);

	// ====== Transform Menu Structure ======
	BMenu* transformMenu = new BMenu("Transform");

	// === TEXT CASE MENU ===
	BMenu* textCaseMenu = new BMenu("Text Case");

	BMenuItem* uppercaseItem = new BMenuItem("UPPERCASE", new BMessage(M_TRANSFORM_UPPERCASE));
	uppercaseItem->SetShortcut('u', B_COMMAND_KEY | B_OPTION_KEY);
	textCaseMenu->AddItem(uppercaseItem);

	BMenuItem* lowercaseItem = new BMenuItem("lowercase", new BMessage(M_TRANSFORM_LOWERCASE));
	lowercaseItem->SetShortcut('l', B_COMMAND_KEY | B_OPTION_KEY);
	textCaseMenu->AddItem(lowercaseItem);

	BMenuItem* capitalizeItem = new BMenuItem("Capitalize", new BMessage(M_TRANSFORM_CAPITALIZE));
	capitalizeItem->SetShortcut('c', B_COMMAND_KEY | B_OPTION_KEY);
	textCaseMenu->AddItem(capitalizeItem);

	BMenuItem* titleCaseItem = new BMenuItem("Title Case", new BMessage(M_TRANSFORM_TITLE_CASE));
	titleCaseItem->SetShortcut('t', B_COMMAND_KEY | B_OPTION_KEY);
	textCaseMenu->AddItem(titleCaseItem);

	BMenuItem* randomCaseItem = new BMenuItem("RaNDoM caSE", new BMessage(M_TRANSFORM_RANDOM_CASE));
	randomCaseItem->SetShortcut('r', B_COMMAND_KEY | B_OPTION_KEY);
	textCaseMenu->AddItem(randomCaseItem);

	BMenuItem* alternatingCaseItem
		= new BMenuItem("AlTeRnAtInG cAsE", new BMessage(M_TRANSFORM_ALTERNATING_CASE));
	alternatingCaseItem->SetShortcut('a', B_COMMAND_KEY | B_OPTION_KEY);
	textCaseMenu->AddItem(alternatingCaseItem);

	BMenuItem* toggleCaseItem = new BMenuItem("Toggle case", new BMessage(M_TRANSFORM_TOGGLE_CASE));
	toggleCaseItem->SetShortcut('t', B_COMMAND_KEY | B_OPTION_KEY | B_SHIFT_KEY);
	textCaseMenu->AddItem(toggleCaseItem);

	transformMenu->AddItem(textCaseMenu);

	// === ENCODE/DECODE MENU ===
	BMenu* encodeMenu = new BMenu("Encode/Decode");

	BMenuItem* urlEncodeItem = new BMenuItem("URL encode", new BMessage(M_TRANSFORM_ENCODE_URL));
	encodeMenu->AddItem(urlEncodeItem);

	BMenuItem* urlDecodeItem = new BMenuItem("URL decode", new BMessage(M_TRANSFORM_DECODE_URL));
	encodeMenu->AddItem(urlDecodeItem);

	BMenuItem* rot13Item = new BMenuItem("ROT-13 encode/decode", new BMessage(M_TRANSFORM_ROT13));
	rot13Item->SetShortcut('m', B_COMMAND_KEY | B_OPTION_KEY);
	encodeMenu->AddItem(rot13Item);

	transformMenu->AddItem(encodeMenu);

	// === LINE TOOLS MENU ===
	BMenu* lineMenu = new BMenu("Line Tools");

	BMenuItem* removeLineBreaksItem
		= new BMenuItem("Remove line breaks", new BMessage(M_REMOVE_LINE_BREAKS));
	removeLineBreaksItem->SetShortcut('b', B_COMMAND_KEY | B_OPTION_KEY);
	lineMenu->AddItem(removeLineBreaksItem);

	BMenuItem* insertLineBreaksItem
		= new BMenuItem("Insert line breaks (width)", new BMessage(M_INSERT_LINE_BREAKS));
	insertLineBreaksItem->SetShortcut('i', B_COMMAND_KEY | B_OPTION_KEY);
	lineMenu->AddItem(insertLineBreaksItem);

	BMenuItem* breakOnDelimiterItem
		= new BMenuItem("Break lines on delimiter", new BMessage(M_BREAK_LINES_ON_DELIMITER));
	breakOnDelimiterItem->SetShortcut('d', B_COMMAND_KEY | B_OPTION_KEY);
	lineMenu->AddItem(breakOnDelimiterItem);

	BMenuItem* trimItem = new BMenuItem("Trim whitespace", new BMessage(M_TRIM_LINES));
	trimItem->SetShortcut('w', B_COMMAND_KEY | B_OPTION_KEY);
	lineMenu->AddItem(trimItem);

	BMenuItem* removeEmptyLinesItem
		= new BMenuItem("Remove empty lines", new BMessage(M_TRIM_EMPTY_LINES));
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
	settings.AddBool("saveSettingsOnExit", fSaveSettingsOnExit);
	settings.AddBool("insertClipboard", fInsertClipboard);

	// Save settings and textView (optional setting in the future)
	if (textView && fSaveTextOnExit)
		settings.AddString("textViewContent", textView->Text());

	// Save sidebar text inputs
	if (sidebar && fSaveSettingsOnExit) {
		settings.AddInt32("breakMode", sidebar->BreakMode());
		settings.AddString("breakModeInput", sidebar->getBreakModeInput());
		settings.AddString("prefixInput", sidebar->PrefixText());
		settings.AddString("suffixInput", sidebar->SuffixText());

		settings.AddBool("splitOnWords", sidebar->SplitOnWordsEnabled());

		settings.AddString("replaceSearchString", sidebar->ReplaceSearchText());
		settings.AddString("replaceWithString", sidebar->ReplaceWithText());
		settings.AddBool("replaceCaseSensitive", sidebar->ReplaceCaseSensitive());
		settings.AddBool("replaceFullWords", sidebar->ReplaceFullWordsOnly());
	}

	if (status == B_OK)
		status = settings.Flatten(&file);

	return status;
}


void
MainWindow::_RestoreValues(BMessage& settings)
{
	BString text;
	int32 number;
	bool flag;

	// General settings
	if (settings.FindBool("saveTextOnExit", &flag) == B_OK)
		fSaveTextOnExit = flag;

	if (settings.FindBool("saveSettingsOnExit", &flag) == B_OK)
		fSaveSettingsOnExit = flag;

	if (settings.FindBool("insertClipboard", &flag) == B_OK)
		fInsertClipboard = flag;

	if (settings.FindString("textViewContent", &text) == B_OK && fSaveTextOnExit)
		textView->SetText(text);

	if (sidebar && fSaveSettingsOnExit) {
		if (settings.FindInt32("breakMode", &number) == B_OK)
			sidebar->setBreakMode(number);

		if (settings.FindString("breakModeInput", &text) == B_OK)
			sidebar->setBreakModeInput(text);

		if (settings.FindString("prefixInput", &text) == B_OK)
			sidebar->SetPrefixText(text);

		if (settings.FindString("suffixInput", &text) == B_OK)
			sidebar->SetSuffixText(text);

		if (settings.FindBool("splitOnWords", &flag) == B_OK)
			sidebar->SetSplitOnWordsEnabled(flag);

		if (settings.FindString("replaceSearchString", &text) == B_OK)
			sidebar->SetReplaceSearchText(text);

		if (settings.FindString("replaceWithString", &text) == B_OK)
			sidebar->SetReplaceWithText(text);

		if (settings.FindBool("replaceCaseSensitive", &flag) == B_OK)
			sidebar->SetReplaceCaseSensitive(flag);

		if (settings.FindBool("replaceFullWords", &flag) == B_OK)
			sidebar->SetReplaceFullWordsOnly(flag);
	}
}


void
MainWindow::UpdateStatusBar()
{
	// Calculate Row/column based on cursor position
	int32 start, end;
	textView->GetSelection(&start, &end);
	BString textBuffer = textView->Text();

	int32 row = 1;
	for (int32 i = 0; i < start; ++i) {
		if (textBuffer[i] == '\n')
			row++;
	}
	int col = start - textBuffer.FindLast('\n', start);

	// Calculate character count and word count
	textBuffer.ReplaceAll('\n', ' ');
	textBuffer.ReplaceAll('\t', ' ');
	int32 charCount = textView->TextLength();
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
	statusText.SetToFormat("%d:%d | Chars: %d | Words: %d", row, col, charCount, wordCount);
	statusBar->SetText(statusText.String());
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
		(new BAlert("Error", "The selected file is not a recognized text file.", "OK"))->Go();
		return;
	}

	textView->SetText("");
	if (BTranslationUtils::GetStyledText(&file, textView) == B_OK) {
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

	if (BTranslationUtils::PutStyledText(textView, &file) == B_OK) {
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
