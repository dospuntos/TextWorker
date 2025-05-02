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

	// Getters
	BString GetReplaceLineBreaksValue() const { return BString(replaceLineBreaksInput->Text()); }
	BString GetPrependInput() const { return BString(prefixInput->Text()); }
	BString GetAppendInput() const { return BString(suffixInput->Text()); }
	int32 GetLineBreaksMaxWidth() const { return atoi(maxWidthInput->Text()); }
	bool GetBreakOnWords() const
	{
		return splitOnWordsCheckbox ? splitOnWordsCheckbox->Value() == B_CONTROL_ON : false;
	}
	BString GetLineBreakDelimiter() const { return BString(lineBreakDelimiterInput->Text()); }
	BString GetReplaceSearchString() const { return BString(replaceSearchString->Text()); }
	BString GetReplaceWithString() const { return BString(replaceWithString->Text()); }
	bool GetReplaceCaseSensitive() const
	{
		return replaceCaseSensitiveCheckbox ? replaceCaseSensitiveCheckbox->Value() == B_CONTROL_ON : false;
	}
	bool GetReplaceFullWords() const
	{
		return replaceFullWordsCheckbox ? replaceFullWordsCheckbox->Value() == B_CONTROL_ON : false;
	}

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
