#include "MainWindow.h"

#include <Alert.h>
#include <Application.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <ScrollView.h>
#include <MenuItem.h>
#include <Button.h>
#include <View.h>
#include <cctype>

#include "TextUtils.h"
#include "Toolbar.h"
#include "Sidebar.h"

static const char* kSettingsFile = "TextWorker_settings";


MainWindow::MainWindow(void)
	:	BWindow(BRect(100,100,500,400),"TextWorker",B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE)
{
	BMenuBar* menuBar = _BuildMenu();

	textView = new BTextView("TextView");
	textView->MakeEditable(true);
	textView->SetText("Paste your text here...");

	// Scroll view for textView - not working
	//BScrollView* scrollView = new BScrollView(
		//"ScrollView", textView, B_FOLLOW_ALL, 1, true, true);

	// Toolbar
	Toolbar* toolbar = new Toolbar();

	// Sidebar
	Sidebar* sidebar = new Sidebar();

	// Status bar
	statusBar = new BStringView("StatusBar", "Row: 0, Col: 0");
	statusBar->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_VERTICAL_CENTER));

	// Layout
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(menuBar, 0)
		.Add(toolbar, 0)
		.SetInsets(2)
		.AddGroup(B_HORIZONTAL, 0)
			.Add(sidebar, 0)
			.Add(textView, 1)
			.SetInsets(5, 5, 5, 5)
		.End()
		.Add(statusBar, 0);

	BMessage settings;
	_LoadSettings(settings);

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
}


void
MainWindow::MessageReceived(BMessage *msg)
{
	UpdateStatusBar();
	switch (msg->what)
	{
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
		case M_TRANSFORM_ALTERNATING_CASE:
			ConvertToAlternatingCase(textView);
			break;
		case M_REMOVE_LINE_BREAKS:
			RemoveLineBreaks(textView);
			break;
		case M_TRANSFORM_ROT13:
			ConvertToROT13(textView);
			break;
		case M_TRANSFORM_WIP:
			(new BAlert("Not implemented", "Sorry, this functionality has not been implemented "
						"yet, but it is planned for the near future.", "OK"))->Go();
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

	item = new BMenuItem("Open" B_UTF8_ELLIPSIS, new BMessage(M_TRANSFORM_WIP), 'O');
	menu->AddItem(item);

	menu->AddSeparatorItem();

	item = new BMenuItem("About" B_UTF8_ELLIPSIS, new BMessage(B_ABOUT_REQUESTED));
	item->SetTarget(be_app);
	menu->AddItem(item);

	item = new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q');
	menu->AddItem(item);

	menuBar->AddItem(menu);

	// 'Transform' menu
	menu = new BMenu("Transform");

	// ' Case' submenu
	subMenu = new BMenu("Text case");

	item = new BMenuItem("UPPERCASE", new BMessage(M_TRANSFORM_UPPERCASE), 'U');
	subMenu->AddItem(item);

	item = new BMenuItem("lowercase", new BMessage(M_TRANSFORM_LOWERCASE), 'L');
	subMenu->AddItem(item);

	item = new BMenuItem("Capitalize", new BMessage(M_TRANSFORM_CAPITALIZE), 'C');
	subMenu->AddItem(item);

	item = new BMenuItem("Title Case", new BMessage(M_TRANSFORM_TITLE_CASE), 'T');
	subMenu->AddItem(item);

	item = new BMenuItem("RaNDoM caSE", new BMessage(M_TRANSFORM_RANDOM_CASE), 'R');
	subMenu->AddItem(item);

	item = new BMenuItem("AlTeRnAtInG cAsE", new BMessage(M_TRANSFORM_ALTERNATING_CASE));
	subMenu->AddItem(item);

	menu->AddItem(subMenu);

	// 'Encode/Decode' submenu
	subMenu = new BMenu("Encode/decode");

	item = new BMenuItem("ROT-13 encode", new BMessage(M_TRANSFORM_ROT13));
	subMenu->AddItem(item);

	item = new BMenuItem("ROT-13 decode", new BMessage(M_TRANSFORM_ROT13));
	subMenu->AddItem(item);

	menu->AddItem(subMenu);

	// 'Line break' submenu
	subMenu = new BMenu("Line breaks");

	item = new BMenuItem("Remove line breaks", new BMessage(M_REMOVE_LINE_BREAKS));
	subMenu->AddItem(item);

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
