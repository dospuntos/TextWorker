/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <Slider.h>
#include <String.h>
#include <Window.h>

class SettingsWindow : public BWindow {
public:
	SettingsWindow(bool saveText, bool saveSettings);
	virtual void MessageReceived(BMessage* message);

private:
	BCheckBox* fSaveTextCheck;
	BCheckBox* fSaveSettingsCheck;
	BMenuField* fFontFamilyField;
	BSlider* fFontSizeSlider;
	BButton* fApplyButton;

	void ApplySettings();
	void PopulateFontMenu(BPopUpMenu* menu);
};

#endif // SETTINGS_WINDOW_H
