#include "MainWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Bitmap.h>
#include <Button.h>
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
#include <TranslatorRoster.h>
#include <TranslationUtils.h>
#include <View.h>
#include <cctype>

#include "Constants.h"
#include "TextUtils.h"

static const char* kSettingsFile = "TextWorker_settings";


MainWindow::MainWindow(void)
	:
	BWindow(BRect(100, 100, 900, 800), kApplicationName, B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE)
{
	BMenuBar* menuBar = _BuildMenu();

	textView = new BTextView("TextView");
	textView->MakeEditable(true);
	textView->SetText("Paste your text here...");
	textView->SetWordWrap(false);

	scrollView = new BScrollView(
		"TextViewScroll", textView, B_WILL_DRAW | B_FRAME_EVENTS, true, true, B_PLAIN_BORDER);

	BFont font(be_fixed_font);
	textView->SetFontAndColor(&font);

	// Toolbar
	toolbar = new BToolBar(B_HORIZONTAL);

	toolbar->AddAction(new BMessage(M_FILE_OPEN), this, ResVectorToBitmap("OPEN_ICON"),
		"Open file (Alt-O)", "", false);
	toolbar->AddAction(new BMessage(M_FILE_SAVE), this, ResVectorToBitmap("SAVE_ICON"),
		"Save file (Alt-S)", "", false);
	toolbar->GroupLayout()->AddItem(
		BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));
	toolbar->AddSeparator();
	toolbar->GroupLayout()->AddItem(
		BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));
	toolbar->AddAction(new BMessage(M_TOGGLE_WORD_WRAP), this, ResVectorToBitmap("LINE_WRAP_ICON"),
		"Word wrap", "", true);
	toolbar->GroupLayout()->AddItem(
		BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));
	toolbar->AddSeparator();
	toolbar->GroupLayout()->AddItem(
		BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));
	toolbar->AddAction(new BMessage(M_TRANSFORM_UPPERCASE), this,
		ResVectorToBitmap("UPPERCASE_ICON"), "UPPERCASE (Alt-U)", "", false);
	toolbar->AddAction(new BMessage(M_TRANSFORM_LOWERCASE), this,
		ResVectorToBitmap("LOWERCASE_ICON"), "lowercase (Alt-L)", "", false);

	toolbar->AddGlue();

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
}


MainWindow::~MainWindow()
{
	_SaveSettings();
	delete fOpenPanel;
	delete fSavePanel;
}


void
MainWindow::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
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
		case M_REMOVE_LINE_BREAKS:
			RemoveLineBreaks(textView, sidebar->GetReplaceLineBreaksValue());
			break;
		case M_INSERT_LINE_BREAKS:
			InsertLineBreaks(textView, sidebar->GetLineBreaksMaxWidth(),
				sidebar->GetBreakOnWords());
			break;
		case M_BREAK_LINES_ON_DELIMITER:
			BreakLinesOnDelimiter(textView, sidebar->GetLineBreakDelimiter());
			break;
		case M_TRIM_LINES:
			TrimLines(textView);
			break;
		case M_TRANSFORM_REPLACE:
			ReplaceAll(textView, sidebar->GetReplaceSearchString(), sidebar->GetReplaceWithString(),
				sidebar->GetReplaceCaseSensitive(), sidebar->GetReplaceFullWords());
			break;
		case M_TRIM_EMPTY_LINES:
			TrimEmptyLines(textView);
		case M_TRANSFORM_PREFIX_SUFFIX:
			AddStringsToEachLine(textView, sidebar->GetPrependInput(), sidebar->GetAppendInput());
			break;
		case M_TRANSFORM_ROT13:
			ConvertToROT13(textView);
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
		default:
			BWindow::MessageReceived(msg);
			break;
	}
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

	// 'Transform' menu
	menu = new BMenu("Transform");

	// ' Case' submenu
	subMenu = new BMenu("Text case");

	subMenu->AddItem(new BMenuItem("UPPERCASE", new BMessage(M_TRANSFORM_UPPERCASE)));
	subMenu->AddItem(new BMenuItem("lowercase", new BMessage(M_TRANSFORM_LOWERCASE)));
	subMenu->AddItem(new BMenuItem("Capitalize", new BMessage(M_TRANSFORM_CAPITALIZE)));
	subMenu->AddItem(new BMenuItem("Title Case", new BMessage(M_TRANSFORM_TITLE_CASE)));
	subMenu->AddItem(new BMenuItem("RaNDoM caSE", new BMessage(M_TRANSFORM_RANDOM_CASE)));
	subMenu->AddItem(new BMenuItem("AlTeRnAtInG cAsE", new BMessage(M_TRANSFORM_ALTERNATING_CASE)));

	menu->AddItem(subMenu);

	// 'Encode/Decode' submenu
	subMenu = new BMenu("Encode/decode");

	subMenu->AddItem(new BMenuItem("ROT-13 encode/decode", new BMessage(M_TRANSFORM_ROT13)));

	menu->AddItem(subMenu);

	// 'Line break' submenu
	subMenu = new BMenu("Line breaks");

	subMenu->AddItem(new BMenuItem("Remove line breaks", new BMessage(M_REMOVE_LINE_BREAKS)));

	menu->AddItem(subMenu);

	menuBar->AddItem(menu);

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

	if (status == B_OK)
		status = settings.Flatten(&file);

	return status;
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
		if (textBuffer[i] == '\n') {
			row++;
		}
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


BBitmap*
MainWindow::ResVectorToBitmap(const char* resName)
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
						if (++nonPrintable > 10) break;
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
