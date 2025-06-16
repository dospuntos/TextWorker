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

	// Search and replace
	BString getSearchText() const;
	void setSearchText(const BString& text);
	BString getReplaceText() const;
	void setReplaceText(const BString& text);
	bool getReplaceCaseSensitive() const;
	void setReplaceCaseSensitive(bool enabled);
	bool getReplaceFullWords() const;
	void setReplaceFullWords(bool enabled);

	// Line breaks
	int8 getBreakMode() const;
	void setBreakMode(int breakMode);
	BString getBreakModeInput();
	void setBreakModeInput(const BString& text);
	int getBreakOnCharsSpinner();
	void setBreakOnCharsSpinner(int value);
	bool getSplitOnWords() const;
	void setSplitOnWords(bool enabled);
	bool getKeepDelimiterValue();
	void setKeepDelimiterValue(bool value);

	// Prefix/suffix
	BString getPrefixText() const;
	void setPrefixText(const BString& text);
	BString getSuffixText() const;
	void setSuffixText(const BString& text);

	// Indent/unindent
	int32 getIndentSpinner() const;
	void setIndentSpinner(int value);
	bool getTabsRadio() const;
	void setTabsRadio(bool value) const;

	// Sort lines
	bool getAlphaSortRadio() const;
	void setAlphaSortRadio(bool enabled) const;
	bool getSortAsc() const;
	void setSortAsc(bool enabled) const;
	bool getCaseSortCheck() const;
	void setCaseSortCheck(bool enabled);

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
