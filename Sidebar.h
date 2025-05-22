/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <CheckBox.h>
#include <GroupView.h>
#include <TabView.h>
#include <TextControl.h>


class Sidebar : public BTabView {
public:
	Sidebar();
	void MessageReceived(BMessage* msg);

	// Replace Line Breaks
	BString ReplaceLineBreaksText() const { return replaceLineBreaksInput->Text(); }
	void SetReplaceLineBreaksText(const BString& text) { replaceLineBreaksInput->SetText(text); }

	// Prefix
	BString PrefixText() const { return prefixInput->Text(); }
	void SetPrefixText(const BString& text) { prefixInput->SetText(text); }

	// Suffix
	BString SuffixText() const { return suffixInput->Text(); }
	void SetSuffixText(const BString& text) { suffixInput->SetText(text); }

	// Max Width
	int32 MaxWidthText() const { return atoi(maxWidthInput->Text()); }
	void SetMaxWidthText(const int32 value) { BString text; text << value; maxWidthInput->SetText(text.String()); }

	// Split on Words
	bool SplitOnWordsEnabled() const { return splitOnWordsCheckbox->Value() == B_CONTROL_ON; }
	void SetSplitOnWordsEnabled(bool enabled) { splitOnWordsCheckbox->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF); }

	// Line Break Delimiter
	BString LineBreakDelimiterText() const { return lineBreakDelimiterInput->Text(); }
	void SetLineBreakDelimiterText(const BString& text) { lineBreakDelimiterInput->SetText(text); }

	// Replace Search String
	BString ReplaceSearchText() const { return replaceSearchString->Text(); }
	void SetReplaceSearchText(const BString& text) { replaceSearchString->SetText(text); }

	// Replace With String
	BString ReplaceWithText() const { return replaceWithString->Text(); }
	void SetReplaceWithText(const BString& text) { replaceWithString->SetText(text); }

	// Case Sensitivity
	bool ReplaceCaseSensitive() const { return replaceCaseSensitiveCheckbox->Value() == B_CONTROL_ON; }
	void SetReplaceCaseSensitive(bool enabled) { replaceCaseSensitiveCheckbox->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF); }

	// Full Word Match
	bool ReplaceFullWordsOnly() const { return replaceFullWordsCheckbox->Value() == B_CONTROL_ON; }
	void SetReplaceFullWordsOnly(bool enabled) { replaceFullWordsCheckbox->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF); }


private:
	BTextControl* replaceLineBreaksInput;
	BTextControl* prefixInput;
	BTextControl* suffixInput;
	BTextControl* maxWidthInput;
	BCheckBox* splitOnWordsCheckbox;
	BTextControl* lineBreakDelimiterInput;
	BTextControl* replaceSearchString;
	BTextControl* replaceWithString;
	BCheckBox* replaceCaseSensitiveCheckbox;
	BCheckBox* replaceFullWordsCheckbox;
};

#endif // SIDEBAR_H
