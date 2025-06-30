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
	SettingsWindow(bool saveText, bool saveSettings, bool clipboard, bool clearSettings,
		bool applyToSelection, int32 fontSize, BString fontFamily);
	virtual void MessageReceived(BMessage* message);
	void SetApplyToSelectionValue(bool value);
	bool QuitRequested();

private:
	BCheckBox* fSaveTextCheck;
	BCheckBox* fSaveFieldsCheck;
	BCheckBox* fInsertClipboard;
	BCheckBox* fClearSettingsAfterUse;
	BCheckBox* fApplyToSelectionOnly;
	BMenuField* fFontFamilyField;
	BString fFontFamily;
	BSlider* fFontSizeSlider;
	BButton* fApplyButton;

	void PopulateFontMenu(BPopUpMenu* menu);
};

#endif // SETTINGS_WINDOW_H
