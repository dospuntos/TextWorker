/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

#include <Window.h>
#include <CheckBox.h>
#include <PopUpMenu.h>
#include <MenuField.h>
#include <Slider.h>
#include <Button.h>
#include <Box.h>
#include <String.h>

class SettingsWindow : public BWindow {
public:
	SettingsWindow(BRect frame);
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

