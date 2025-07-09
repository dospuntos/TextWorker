/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "SettingsWindow.h"
#include "Constants.h"
#include <Alert.h>
#include <Application.h>
#include <Catalog.h>
#include <Font.h>
#include <LayoutBuilder.h>
#include <MenuItem.h>
#include <StringView.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Settings"


SettingsWindow::SettingsWindow(bool saveText, bool saveSettings, bool clipboard, bool clearSettings, int32 fontSize, BString fontFamily)
	:
	BWindow(BRect(200, 200, 500, 400), B_TRANSLATE("Settings"), B_TITLED_WINDOW,
		B_NOT_RESIZABLE | B_NOT_MINIMIZABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS
			| B_CLOSE_ON_ESCAPE),
	fFontFamily(fontFamily)
{
	fSaveTextCheck = new BCheckBox("SaveText", B_TRANSLATE("Save text on exit"), new BMessage(M_APPLY_SETTINGS));
	fSaveTextCheck->SetValue(saveText ? B_CONTROL_ON : B_CONTROL_OFF);
	fSaveFieldsCheck
		= new BCheckBox("SaveSettings", B_TRANSLATE("Save field values on exit"), new BMessage(M_APPLY_SETTINGS));
	fSaveFieldsCheck->SetValue(saveSettings ? B_CONTROL_ON : B_CONTROL_OFF);
	fInsertClipboard = new BCheckBox("Clipboard", B_TRANSLATE("Paste clipboard contents on open"),
		new BMessage(M_APPLY_SETTINGS));
	fInsertClipboard->SetValue(clipboard ? B_CONTROL_ON : B_CONTROL_OFF);
	fClearSettingsAfterUse = new BCheckBox("ClearSettings", B_TRANSLATE("Clear field values after use"),
		new BMessage(M_APPLY_SETTINGS));
	fClearSettingsAfterUse->SetValue(clearSettings ? B_CONTROL_ON : B_CONTROL_OFF);

	BPopUpMenu* fontMenu = new BPopUpMenu("FontFamily");
	PopulateFontMenu(fontMenu);
	fFontFamilyField = new BMenuField("Font", B_TRANSLATE("Font family:"), fontMenu);

	fFontSizeSlider
		= new BSlider("FontSize", "", new BMessage(M_APPLY_SETTINGS), 6, 72, B_HORIZONTAL);
	fFontSizeSlider->SetLimitLabels("6", "72");
	fFontSizeSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fFontSizeSlider->SetHashMarkCount(10);
	fFontSizeSlider->SetValue(fontSize);
	fFontSizeSlider->SetModificationMessage(new BMessage(M_APPLY_SETTINGS));
	BString sliderLabel = B_TRANSLATE("Font size:");
	sliderLabel << " " << fontSize << "pt";
	fFontSizeSlider->SetLabel(sliderLabel.String());

	fApplyButton = new BButton("Close", B_TRANSLATE("Close"), new BMessage(M_CLOSE_SETTINGS));

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(fInsertClipboard)
		.Add(fSaveTextCheck)
		.Add(fSaveFieldsCheck)
		.Add(fClearSettingsAfterUse)
		.Add(fFontFamilyField)
		.Add(fFontSizeSlider)
		.AddGlue()
		.Add(fApplyButton)
		.SetInsets(B_USE_WINDOW_INSETS);

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
			applyMsg.AddBool("saveSettings", fSaveFieldsCheck->Value() == B_CONTROL_ON);
			applyMsg.AddBool("insertClipboard", fInsertClipboard->Value() == B_CONTROL_ON);
			applyMsg.AddBool("clearSettings", fClearSettingsAfterUse->Value() == B_CONTROL_ON);
			applyMsg.AddInt32("fontSize", fFontSizeSlider->Value());
			applyMsg.AddString("fontFamily", fontFamily);

			be_app->WindowAt(0)->PostMessage(&applyMsg);

			BString sliderLabel = B_TRANSLATE("Font size:");
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


bool
SettingsWindow::QuitRequested()
{
	if (!IsHidden())
		Hide();
	return false;
}