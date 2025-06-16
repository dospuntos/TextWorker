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

	// Prefix/suffix
	BString PrefixText() const { return fPrefixInput->Text(); }
	void SetPrefixText(const BString& text) { fPrefixInput->SetText(text); }
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

	// Search and replace
	BString ReplaceSearchText() const { return fSearchInput->Text(); }
	void SetReplaceSearchText(const BString& text) { fSearchInput->SetText(text); }
	BString ReplaceWithText() const { return fReplaceInput->Text(); }
	void SetReplaceWithText(const BString& text) { fReplaceInput->SetText(text); }
	bool ReplaceCaseSensitive() const { return fCaseCheck->Value() == B_CONTROL_ON; }
	void SetReplaceCaseSensitive(bool enabled)
	{
		fCaseCheck->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF);
	}

	// Line break mode
	int8 BreakMode() const { return fBreakMode; }
	void setBreakMode(int breakMode) { fBreakMenu->ItemAt(breakMode)->SetMarked(true); }
	BString getBreakModeInput() { return fBreakInput->Text(); }
	void setBreakModeInput(const BString& text) { fBreakInput->SetText(text); }
	int getBreakOnCharsSpinner() { return fBreakOnChars->Value(); }
	void setBreakOnCharsSpinner(const int value) {fBreakOnChars->SetValue(value); }
	bool getKeepDelimiterValue() {return fKeepDelimiterCheck->Value() == B_CONTROL_ON; }
	void setKeepDelimiterValue(const bool value) { fKeepDelimiterCheck->SetValue(value); }

	// Prefix/suffix
	bool ReplaceFullWordsOnly() const { return fWholeWordCheck->Value() == B_CONTROL_ON; }
	void SetReplaceFullWordsOnly(bool enabled)
	{
		fWholeWordCheck->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF);
	}

	// Indent/unindent
	int32 getIndentSpinner() const { return fIndentSizeSpinner->Value(); }
	void setIndentSpinner(const int value) { fIndentSizeSpinner->SetValue(value); }
	bool getTabsRadio() const { return fTabsRadio->Value() == B_CONTROL_ON; }
	void setTabsRadio(bool value) const { fTabsRadio->SetValue(value); }

	// Sort lines
	bool getAlphaSortRadio() const
	{
		return fAlphaSortRadio && fAlphaSortRadio->Value() == B_CONTROL_ON;
	}
	void setAlphaSortRadio(bool enabled) const
	{
		fAlphaSortRadio->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF);
		fLengthSortRadio->SetValue(enabled ? B_CONTROL_OFF : B_CONTROL_ON);
	}
	bool getSortAsc() const { return fSortAsc && fSortAsc->Value() == B_CONTROL_ON; }
	void setSortAsc(bool enabled) const
	{
		fSortAsc->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF);
		fSortDesc->SetValue(enabled ? B_CONTROL_OFF : B_CONTROL_ON);
	}
	bool getCaseSortCheck() const
	{
		return fCaseSortCheck && fCaseSortCheck->Value() == B_CONTROL_ON;
	}
	void setCaseSortCheck(bool enabled) {fCaseSortCheck->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF); }


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
	BRadioButton* fAlphaSortRadio;
	BRadioButton* fLengthSortRadio;
	BRadioButton* fSortAsc;
	BRadioButton* fSortDesc;
	BCheckBox* fCaseSortCheck;

	BMenu* fBreakMenu;
	enum BreakMode fBreakMode;
};

#endif // SIDEBAR_H
