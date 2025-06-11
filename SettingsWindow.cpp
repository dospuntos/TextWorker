/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "SettingsWindow.h"
#include <Alert.h>
#include <Application.h>
#include <Font.h>
#include <LayoutBuilder.h>
#include <MenuItem.h>
#include <StringView.h>

enum { M_APPLY_SETTINGS = 'aply', M_SETTINGS_SAVETEXT = 'stxt', M_SETTINGS_SAVESETTINGS = 'sset' };


SettingsWindow::SettingsWindow(bool saveText, bool saveSettings)
	:
	BWindow(BRect(200, 200, 500, 400), "Settings", B_FLOATING_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL,
		B_NOT_RESIZABLE | B_NOT_MINIMIZABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS
			| B_CLOSE_ON_ESCAPE)
{
	fSaveTextCheck
		= new BCheckBox("SaveText", "Save text on exit", new BMessage(M_SETTINGS_SAVETEXT));
	fSaveTextCheck->SetValue(saveText ? B_CONTROL_ON : B_CONTROL_OFF);
	fSaveSettingsCheck = new BCheckBox("SaveSettings", "Save settings on exit",
		new BMessage(M_SETTINGS_SAVESETTINGS));
	fSaveSettingsCheck->SetValue(saveSettings ? B_CONTROL_ON : B_CONTROL_OFF);

	BPopUpMenu* fontMenu = new BPopUpMenu("FontFamily");
	PopulateFontMenu(fontMenu);
	fFontFamilyField = new BMenuField("Font", "Font family:", fontMenu);

	fFontSizeSlider = new BSlider("FontSize", "Font size:", nullptr, 8, 48, B_HORIZONTAL);
	fFontSizeSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fFontSizeSlider->SetHashMarkCount(10);
	fFontSizeSlider->SetValue(12);

	fApplyButton = new BButton("Close", "Close", new BMessage(M_APPLY_SETTINGS));

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(fSaveTextCheck)
		.Add(fSaveSettingsCheck)
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
	font_family family;
	int32 count = count_font_families();
	for (int32 i = 0; i < count; ++i) {
		if (get_font_family(i, &family) == B_OK)
			menu->AddItem(new BMenuItem(family, nullptr));
	}
}


void
SettingsWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case M_SETTINGS_SAVETEXT:
		{
			BMessage applyMsg(M_SETTINGS_SAVETEXT);
			applyMsg.AddBool("saveText", fSaveTextCheck->Value() == B_CONTROL_ON);
			be_app->WindowAt(0)->PostMessage(&applyMsg);
			break;
		}
		case M_SETTINGS_SAVESETTINGS:
		{
			BMessage applyMsg(M_SETTINGS_SAVESETTINGS);
			applyMsg.AddBool("saveSettings", fSaveSettingsCheck->Value() == B_CONTROL_ON);
			be_app->WindowAt(0)->PostMessage(&applyMsg);
			break;
		}
		case M_APPLY_SETTINGS:
			ApplySettings();
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
SettingsWindow::ApplySettings()
{
	// TODO: apply or store the settings
	Hide(); // or Close() depending on your app
}
