/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <TabView.h>
#include <GroupView.h>
#include <TextControl.h>
#include <CheckBox.h>


class Sidebar : public BTabView {
public:
	Sidebar();

	//Getters
	BString GetReplaceLineBreaksValue() const { return BString(replaceLineBreaksInput->Text()); };
	BString GetPrependInput() const { return BString(prependInput->Text()); }
	BString GetAppendInput() const { return BString(appendInput->Text()); }
	int32 GetLineBreaksMaxWidth() const { return atoi(maxWidthInput->Text()); }
	bool GetBreakOnWords() const { return splitOnWordsCheckbox ? splitOnWordsCheckbox->Value() == B_CONTROL_ON : false; }
	BString GetLineBreakDelimiter() const { return BString(lineBreakDelimiterInput->Text()); }

private:
	BTextControl* replaceLineBreaksInput;
	BTextControl* prependInput;
	BTextControl* appendInput;
	BTextControl* maxWidthInput;
	BCheckBox* splitOnWordsCheckbox;
	BTextControl* lineBreakDelimiterInput;
};


#endif // SIDEBAR_H
