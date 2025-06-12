/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "SettingsWindow.h"
#include "Constants.h"
#include <Alert.h>
#include <Application.h>
#include <Font.h>
#include <LayoutBuilder.h>
#include <MenuItem.h>
#include <StringView.h>


SettingsWindow::SettingsWindow(bool saveText, bool saveSettings, bool clipboard, int32 fontSize,
	BString fontFamily)
	:
	BWindow(BRect(200, 200, 500, 400), "Settings", B_FLOATING_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL,
		B_NOT_RESIZABLE | B_NOT_MINIMIZABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS
			| B_CLOSE_ON_ESCAPE),
	fFontFamily(fontFamily)
{
	fSaveTextCheck = new BCheckBox("SaveText", "Save text on exit", new BMessage(M_APPLY_SETTINGS));
	fSaveTextCheck->SetValue(saveText ? B_CONTROL_ON : B_CONTROL_OFF);
	fSaveSettingsCheck
		= new BCheckBox("SaveSettings", "Save settings on exit", new BMessage(M_APPLY_SETTINGS));
	fSaveSettingsCheck->SetValue(saveSettings ? B_CONTROL_ON : B_CONTROL_OFF);
	fInsertClipboard = new BCheckBox("Clipboard", "Paste clipboard contents on open",
		new BMessage(M_APPLY_SETTINGS));
	fInsertClipboard->SetValue(clipboard ? B_CONTROL_ON : B_CONTROL_OFF);

	BPopUpMenu* fontMenu = new BPopUpMenu("FontFamily");
	PopulateFontMenu(fontMenu);
	fFontFamilyField = new BMenuField("Font", "Font family:", fontMenu);

	fFontSizeSlider
		= new BSlider("FontSize", "", new BMessage(M_APPLY_SETTINGS), 8, 48, B_HORIZONTAL);
	fFontSizeSlider->SetLimitLabels("8", "48");
	fFontSizeSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fFontSizeSlider->SetHashMarkCount(10);
	fFontSizeSlider->SetValue(fontSize);
	fFontSizeSlider->SetModificationMessage(new BMessage(M_APPLY_SETTINGS));
	BString sliderLabel = "Font size:";
	sliderLabel << " " << fontSize << "pt";
	fFontSizeSlider->SetLabel(sliderLabel.String());

	fApplyButton = new BButton("Close", "Close", new BMessage(M_CLOSE_SETTINGS));

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(fSaveTextCheck)
		.Add(fSaveSettingsCheck)
		.Add(fInsertClipboard)
		.Add(fFontFamilyField)
		.Add(fFontSizeSlider)
		.AddGlue()
		.Add(fApplyButton)
		.SetInsets(10, 10, 10, 10);

	ResizeToPreferred();
}


void
SettingsWindow::PopulateFontMenu(BPopUpMenu* menu)
{
	// Add a default system font option
	BMenuItem* defaultItem = new BMenuItem("System default", new BMessage(M_APPLY_SETTINGS));
	menu->AddItem(defaultItem);
	menu->AddSeparatorItem(); // Optional visual separator
	defaultItem->SetMarked(true);

	font_family family;
	int32 count = count_font_families();
	for (int32 i = 0; i < count; ++i) {
		if (get_font_family(i, &family) == B_OK) {
			BMenuItem* item = new BMenuItem(family, new BMessage(M_APPLY_SETTINGS));
			menu->AddItem(item);
			if (family == fFontFamily)
				item->SetMarked(true);
		}
	}
}


void
SettingsWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case M_APPLY_SETTINGS:
		{
			BMenuItem* selected = fFontFamilyField->Menu()->FindMarked();
			BString fontFamily = selected->Label();

			BMessage applyMsg(M_APPLY_SETTINGS);
			applyMsg.AddBool("saveText", fSaveTextCheck->Value() == B_CONTROL_ON);
			applyMsg.AddBool("saveSettings", fSaveSettingsCheck->Value() == B_CONTROL_ON);
			applyMsg.AddBool("insertClipboard", fInsertClipboard->Value() == B_CONTROL_ON);
			applyMsg.AddInt32("fontSize", fFontSizeSlider->Value());
			applyMsg.AddString("fontFamily", fontFamily);

			be_app->WindowAt(0)->PostMessage(&applyMsg);

			BString sliderLabel = "Font size:";
			sliderLabel << " " << fFontSizeSlider->Value() << "pt";
			fFontSizeSlider->SetLabel(sliderLabel.String());
			break;
		}
		case M_CLOSE_SETTINGS:
			Hide();
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}
}
 