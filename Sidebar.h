/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "Constants.h"
#include <CheckBox.h>
#include <GroupView.h>
#include <Menu.h>
#include <MenuItem.h>
#include <RadioButton.h>
#include <TabView.h>
#include <TextControl.h>
#include <private/interface/Spinner.h>

enum { M_SORT_ALPHA = 'salp', M_SORT_LENGTH, M_SORT_ASCENDING, M_SORT_DESCENDING, M_SORT_CASE };

class Sidebar : public BTabView {
public:
	Sidebar();
	void MessageReceived(BMessage* msg);

	// Prefix
	BString PrefixText() const { return fPrefixInput->Text(); }
	void SetPrefixText(const BString& text) { fPrefixInput->SetText(text); }

	// Suffix
	BString SuffixText() const { return fSuffixInput->Text(); }
	void SetSuffixText(const BString& text) { fSuffixInput->SetText(text); }

	void SetMaxWidthText(const int32 value)
	{
		BString text;
		text << value;
		fBreakInput->SetText(text.String());
	}

	// Split on Words
	bool SplitOnWordsEnabled() const { return fWordWrapCheck->Value() == B_CONTROL_ON; }
	void SetSplitOnWordsEnabled(bool enabled)
	{
		fWordWrapCheck->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF);
	}

	// Line break mode
	int8 BreakMode() const { return fBreakMode; }
	void setBreakMode(int breakMode) { fBreakMenu->ItemAt(breakMode)->SetMarked(true); }
	BString getBreakModeInput() { return fBreakInput->Text(); }
	void setBreakModeInput(const BString& text) { fBreakInput->SetText(text); }
	bool getKeepDelimiterValue() {return fKeepDelimiterCheck->Value() == B_CONTROL_ON; }

	// Replace Search String
	BString ReplaceSearchText() const { return fSearchInput->Text(); }
	void SetReplaceSearchText(const BString& text) { fSearchInput->SetText(text); }

	// Replace With String
	BString ReplaceWithText() const { return fReplaceInput->Text(); }
	void SetReplaceWithText(const BString& text) { fReplaceInput->SetText(text); }

	// Case Sensitivity
	bool ReplaceCaseSensitive() const { return fCaseCheck->Value() == B_CONTROL_ON; }
	void SetReplaceCaseSensitive(bool enabled)
	{
		fCaseCheck->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF);
	}

	// Full Word Match
	bool ReplaceFullWordsOnly() const { return fWholeWordCheck->Value() == B_CONTROL_ON; }
	void SetReplaceFullWordsOnly(bool enabled)
	{
		fWholeWordCheck->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF);
	}


private:
	void _BuildLineTab();
	void _BuildPrefixTab();
	void _BuildSortTab();
	float fMaxLabelWidth;
	BTextControl* fPrefixInput;
	BTextControl* fSuffixInput;
	BTextControl* fRemovePrefixInput;
	BTextControl* fRemoveSuffixInput;
	BTextControl* fBreakInput;
	BSpinner* fBreakOnChars;
	BSpinner* fIndentSizeSpinner;
	BRadioButton* fTabsRadio;
	BRadioButton* fSpacesRadio;
	BCheckBox* fWordWrapCheck;
	BCheckBox* fKeepDelimiterCheck;
	BTextControl* fDelimiterInput;
	BTextControl* fSearchInput;
	BTextControl* fReplaceInput;
	BCheckBox* fCaseCheck;
	BCheckBox* fWholeWordCheck;

	BMenu* fBreakMenu;
	enum BreakMode fBreakMode;
};

#endif // SIDEBAR_H
