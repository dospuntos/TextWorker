#include "MainWindow.h"

#include <Alert.h>
#include <Application.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuItem.h>
#include <View.h>

#include "TextUtils.h"


MainWindow::MainWindow(void)
	:	BWindow(BRect(100,100,500,400),"TextWorker",B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE)
{
	BRect r(Bounds());
	r.bottom = 20;
	BMenuBar* menuBar = _BuildMenu();
	BRect textRect = BRect(10, 30, Bounds().Width() - 10, Bounds().Height() - 10);
	textView = new BTextView(textRect, "TextView", textRect.InsetByCopy(2, 2),
							B_FOLLOW_ALL, B_WILL_DRAW);
	textView->MakeEditable(true);
	textView->SetText("Paste your text here" B_UTF8_ELLIPSIS);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(menuBar)
		.Add(textView)
		.End();
}


void
MainWindow::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
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
			(new BAlert("Not implemented", "Sorry, this functionality has not been implemented"
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

	item = new BMenuItem("AlTeRnAtInG cAsE", new BMessage(M_TRANSFORM_ALTERNATING_CASE), 'A');
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
