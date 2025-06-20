/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Sidebar.h"
#include "Constants.h"

#include <Alert.h>
#include <Box.h>
#include <Button.h>
#include <Catalog.h>
#include <GridLayoutBuilder.h>
#include <GroupView.h>
#include <LayoutBuilder.h>
#include <RadioButton.h>
#include <StringView.h>
#include <TabView.h>
#include <TextControl.h>
#include <Window.h>
#include <cstdio>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Sidebar"

Sidebar::Sidebar()
	:
	BTabView("Sidebar")
{
	// === Create tabs ===
	_BuildLineTab();
	_BuildPrefixTab();
	_BuildSortTab();
}


void
Sidebar::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case M_MODE_REMOVE_ALL:
			fWordWrapCheck->SetEnabled(false);
			fBreakInput->SetEnabled(false);
			fKeepDelimiterCheck->SetEnabled(false);
			fBreakOnChars->SetEnabled(false);
			this->Invalidate();
			fBreakMode = BREAK_REMOVE_ALL;
			break;
		case M_MODE_BREAK_ON:
			fWordWrapCheck->SetEnabled(false);
			fBreakInput->SetEnabled(true);
			fBreakInput->MakeFocus(true);
			fKeepDelimiterCheck->SetEnabled(true);
			fBreakOnChars->SetEnabled(false);
			this->Invalidate();
			fBreakMode = BREAK_ON;
			break;
		case M_MODE_REPLACE_LINE_BREAKS:
			fWordWrapCheck->SetEnabled(false);
			fBreakInput->SetEnabled(true);
			fBreakInput->MakeFocus(true);
			fKeepDelimiterCheck->SetEnabled(false);
			fBreakOnChars->SetEnabled(false);
			this->Invalidate();
			fBreakMode = BREAK_REPLACE;
			break;
		case M_MODE_BREAK_AFTER_CHARS:
			fWordWrapCheck->SetEnabled(true);
			fBreakInput->SetEnabled(false);
			fKeepDelimiterCheck->SetEnabled(false);
			fBreakOnChars->SetEnabled(true);
			fBreakOnChars->MakeFocus(true);
			fBreakMode = BREAK_AFTER_CHARS;
			this->Invalidate();
			break;
		default:
			BView::MessageReceived(msg);
			break;
	}
}


void
Sidebar::_BuildLineTab()
{
	// === Search/Replace Box ===
	// ==========================
	BBox* searchReplaceBox = new BBox("SearchReplaceBox");
	searchReplaceBox->SetLabel(B_TRANSLATE("Search and replace"));

	fSearchInput = new BTextControl("SearchString", nullptr, "", nullptr);
	fReplaceInput = new BTextControl("ReplaceString", nullptr, "", nullptr);
	fCaseCheck = new BCheckBox("ReplaceCaseSensitiveCheckbox", "Case sensitive", nullptr);
	fWholeWordCheck = new BCheckBox("ReplaceFullWordsCheckbox", "Full words", nullptr);

	BButton* searchReplaceBtn
		= new BButton("SearchReplaceBtn", B_TRANSLATE("Replace"), new BMessage(M_TRANSFORM_REPLACE));
	searchReplaceBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	// Set consistent minimum widths for text fields
	fSearchInput->SetExplicitMinSize(BSize(150, B_SIZE_UNSET));
	fReplaceInput->SetExplicitMinSize(BSize(150, B_SIZE_UNSET));

	// Grid layout
	BGridLayoutBuilder grid1(B_USE_SMALL_SPACING, B_USE_SMALL_SPACING);
	// clang-format off
	grid1.Add(new BStringView(NULL, B_TRANSLATE("Search:")),		0, 0, 1)
		.Add(fSearchInput,										1, 0, 1)
		.Add(new BStringView(NULL, B_TRANSLATE("Replace:")),	0, 1, 1)
		.Add(fReplaceInput,                    				   	1, 1, 1)
		.Add(fCaseCheck,            							0, 2, 1)
		.Add(fWholeWordCheck,                					1, 2, 1)
		.Add(searchReplaceBtn,                      			0, 3, 2);
	// clang-format on

	grid1.GridLayout()->SetMinColumnWidth(0, fMaxLabelWidth);

	// Wrap in a group and add to the box
	BGroupView* searchGroup = new BGroupView(B_VERTICAL, 0);
	searchReplaceBox->AddChild(searchGroup);
	searchGroup->GroupLayout()->AddView(grid1.View());
	searchGroup->GroupLayout()->SetInsets(10, 12, 10, 10);

	// === Line Breaks Options Box ===
	// ===============================
	BGroupView* lineOperationsView = new BGroupView(B_VERTICAL, 5);

	// Create the container box
	BBox* breakBox = new BBox("LineBreakOptionsBox");
	breakBox->SetLabel(B_TRANSLATE("Line break options"));

	fBreakMenu = new BMenu("linebreaks");
	fBreakMenu->AddItem(new BMenuItem(B_TRANSLATE("Remove all"), new BMessage(M_MODE_REMOVE_ALL)));
	fBreakMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Replace with" B_UTF8_ELLIPSIS), new BMessage(M_MODE_REPLACE_LINE_BREAKS)));
	fBreakMenu->AddItem(new BMenuItem(B_TRANSLATE("Break on" B_UTF8_ELLIPSIS), new BMessage(M_MODE_BREAK_ON)));
	fBreakMenu->AddItem(new BMenuItem(B_TRANSLATE("Break after X characters" B_UTF8_ELLIPSIS),
		new BMessage(M_MODE_BREAK_AFTER_CHARS)));
	fBreakMenu->SetLabelFromMarked(true);
	fBreakMenu->SetRadioMode(true);
	fBreakMenu->ItemAt(0L)->SetMarked(true);

	BMenuField* breakField = new BMenuField("breakField", "", fBreakMenu);

	fBreakInput = new BTextControl("MaxWidthInput", nullptr, "", nullptr);
	fBreakInput->SetEnabled(false);
	fBreakInput->SetExplicitMinSize(BSize(100, B_SIZE_UNSET));

	fBreakOnChars = new BSpinner("BreakOn", B_TRANSLATE("Chars:"), nullptr);
	fBreakOnChars->SetMinValue(1);
	fBreakOnChars->SetEnabled(false);

	fWordWrapCheck = new BCheckBox("SplitWords", B_TRANSLATE("Split on words"), NULL);
	fWordWrapCheck->SetEnabled(false);

	fKeepDelimiterCheck = new BCheckBox("KeepDelimiter", B_TRANSLATE("Keep delimiter"), NULL);
	fKeepDelimiterCheck->SetEnabled(false);

	BButton* applyBtn = new BButton("ApplyBtn", B_TRANSLATE("Apply"), new BMessage(M_REMOVE_LINE_BREAKS));
	applyBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	// Layout grid
	BGridLayoutBuilder grid(B_USE_SMALL_SPACING, B_USE_SMALL_SPACING);
	// clang-format off
	grid.Add(breakField,			0, 0, 2)
		.Add(fBreakInput,			0, 1, 1)
		.Add(fBreakOnChars,			1, 1, 1)
		.Add(fWordWrapCheck,    	0, 2, 1)
		.Add(fKeepDelimiterCheck, 	1, 2, 1)
		.Add(applyBtn,          	0, 3, 2);
	// clang-format on
	// grid.GridLayout()->SetMinColumnWidth(0, fMaxLabelWidth);

	BGroupView* breakGroup = new BGroupView(B_VERTICAL, 0);
	breakBox->AddChild(breakGroup);
	breakGroup->GroupLayout()->AddView(grid.View());
	breakGroup->GroupLayout()->SetInsets(10, 12, 10, 10);

	// === Cleanup Box ===
	BBox* cleanupBox = new BBox("CleanupBox");
	cleanupBox->SetLabel(B_TRANSLATE("Cleanup"));
	BGroupView* cleanGroup = new BGroupView(B_VERTICAL, 5);
	cleanupBox->AddChild(cleanGroup);

	// Layout for Cleanup Box
	BButton* trimLinesBtn
		= new BButton("TrimLinesBtn", B_TRANSLATE("Trim whitespace"), new BMessage(M_TRIM_LINES));
	trimLinesBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BButton* trimEmptyLinesBtn
		= new BButton("TrimEmptyLinesBtn", B_TRANSLATE("Remove empty lines"), new BMessage(M_TRIM_EMPTY_LINES));
	trimEmptyLinesBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BButton* removeDuplicates = new BButton("RemoveDuplicatesBtn", B_TRANSLATE("Remove duplicate lines"),
		new BMessage(M_REMOVE_DUPLICATES));
	removeDuplicates->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BLayoutBuilder::Group<>(cleanGroup)
		.Add(trimLinesBtn)
		.Add(trimEmptyLinesBtn)
		.Add(removeDuplicates)
		.SetInsets(10, 12, 10, 10);

	// === Line Operations Tab ===
	BLayoutBuilder::Group<>(lineOperationsView, B_VERTICAL, 10)
		.Add(searchReplaceBox)
		.Add(breakBox)
		.Add(cleanupBox)
		.AddGlue()
		.SetInsets(10, 12, 10, 10);

	BTab* lineOperationsTab = new BTab();
	AddTab(lineOperationsView, lineOperationsTab);
	lineOperationsTab->SetLabel(B_TRANSLATE("Lines"));
}


void
Sidebar::_BuildPrefixTab()
{
	fPrefixInput = new BTextControl("PrefixInput", B_TRANSLATE("Prefix:"), "", nullptr);
	fSuffixInput = new BTextControl("SuffixInput", B_TRANSLATE("Suffix:"), "", nullptr);

	BGroupView* prefixSuffixView = new BGroupView(B_VERTICAL, 5);

	// === Add prefix/suffix Box ===
	BBox* prefixSuffixBox = new BBox("PrefixSuffixBox");
	prefixSuffixBox->SetLabel(B_TRANSLATE("Prefix/suffix each line"));
	BGroupView* addGroup = new BGroupView(B_VERTICAL, 5);
	prefixSuffixBox->AddChild(addGroup);

	// clang-format off
	BLayoutBuilder::Group<>(addGroup)
		.Add(fPrefixInput)
		.Add(fSuffixInput)
		.AddGroup(B_HORIZONTAL)
			.Add(new BButton("addPrefixSuffixBtn", B_TRANSLATE("Add"), new BMessage(M_TRANSFORM_PREFIX_SUFFIX)))
			.AddGlue()
			.Add(new BButton("removePrefixSuffixBtn", B_TRANSLATE("Remove"), new BMessage(M_TRANSFORM_REMOVE_PREFIX_SUFFIX)))
		.End()
		.SetInsets(10, 12, 10, 10);
	// clang-format on

	// === Indent/Unindent Box ===
	BBox* indentBox = new BBox("IndentBox");
	indentBox->SetLabel(B_TRANSLATE("Indent/unindent lines"));

	BGroupView* indentGroup = new BGroupView(B_VERTICAL, 5);
	indentBox->AddChild(indentGroup);

	// Spinner for indent size
	fIndentSizeSpinner = new BSpinner("IndentSize", B_TRANSLATE("Indent size:"), nullptr);
	fIndentSizeSpinner->SetValue(4);
	fIndentSizeSpinner->SetRange(1, 16);

	// Radio buttons for tab/space selection
	fTabsRadio = new BRadioButton("TabsRadio", B_TRANSLATE("Tabs"), nullptr);
	fSpacesRadio = new BRadioButton("SpacesRadio", B_TRANSLATE("Spaces"), nullptr);
	fSpacesRadio->SetValue(B_CONTROL_ON);

	// Group the radio buttons horizontally
	BGroupView* indentModeGroup = new BGroupView(B_HORIZONTAL, 10);
	indentModeGroup->AddChild(fTabsRadio);
	indentModeGroup->AddChild(fSpacesRadio);

	// Buttons
	BButton* indentButton = new BButton("IndentBtn", B_TRANSLATE("Indent"), new BMessage(M_INDENT_LINES));
	BButton* unindentButton
		= new BButton("UnindentBtn", B_TRANSLATE("Unindent"), new BMessage(M_UNINDENT_LINES));

	// clang-format off
	BLayoutBuilder::Group<>(indentGroup)
		.Add(fIndentSizeSpinner)
		.Add(indentModeGroup)
		.AddGroup(B_HORIZONTAL)
			.Add(indentButton)
			.AddGlue()
			.Add(unindentButton)
		.End()
		.SetInsets(10, 12, 10, 10);
	// clang-format on

	// === Prefix/suffix Tab ===
	BLayoutBuilder::Group<>(prefixSuffixView, B_VERTICAL, 10)
		.Add(prefixSuffixBox)
		.Add(indentBox)
		.AddGlue()
		.SetInsets(10, 12, 10, 10);

	BTab* prefixSuffixTab = new BTab();
	AddTab(prefixSuffixView, prefixSuffixTab);
	prefixSuffixTab->SetLabel(B_TRANSLATE("Prefix/suffix"));
}


void
Sidebar::_BuildSortTab()
{
	// === Sort lines Tab ===
	// Tab: Line operations
	BGroupView* sortView = new BGroupView(B_VERTICAL, 5);

	// === Sort Box ===
	BBox* sortBox = new BBox("SortBox");
	sortBox->SetLabel(B_TRANSLATE("Sort lines"));
	BGroupView* sortGroup = new BGroupView(B_VERTICAL, 10);
	sortBox->AddChild(sortGroup);

	// --- Sorting method group ---
	BGroupView* sortTypeGroup = new BGroupView(B_VERTICAL, 5);
	fAlphaSortRadio = new BRadioButton("radio_sort_alpha", B_TRANSLATE("Alphabetical"), nullptr);
	fLengthSortRadio = new BRadioButton("radio_sort_length", B_TRANSLATE("By line length"), nullptr);

	BLayoutBuilder::Group<>(sortTypeGroup)
		.Add(new BStringView(nullptr, B_TRANSLATE("Sort type:")))
		.Add(fAlphaSortRadio)
		.Add(fLengthSortRadio);

	// --- Sort order group ---
	BGroupView* sortOrderGroup = new BGroupView(B_VERTICAL, 5);
	fSortAsc = new BRadioButton("ascRadio", B_TRANSLATE("Ascending"), nullptr);
	fSortDesc = new BRadioButton("descRadio", B_TRANSLATE("Descending"), nullptr);

	BLayoutBuilder::Group<>(sortOrderGroup)
		.Add(new BStringView(nullptr, B_TRANSLATE("Sort order:")))
		.Add(fSortAsc)
		.Add(fSortDesc);

	fCaseSortCheck = new BCheckBox("caseSortCheck", B_TRANSLATE("Case sensitive"), nullptr);

	BButton* sortButton = new BButton("sortBtn", B_TRANSLATE("Sort"), new BMessage(M_SORT_LINES));
	sortButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	// Layout the full sortGroup box
	BLayoutBuilder::Group<>(sortGroup)
		.Add(sortTypeGroup)
		.Add(sortOrderGroup)
		.Add(fCaseSortCheck)
		.Add(sortButton)
		.SetInsets(10, 12, 10, 10);

	fAlphaSortRadio->SetValue(B_CONTROL_ON);
	fSortAsc->SetValue(B_CONTROL_ON);
	fCaseSortCheck->SetValue(B_CONTROL_OFF);

	// === Sort Tab ===
	BLayoutBuilder::Group<>(sortView, B_VERTICAL, 10)
		.Add(sortBox)
		.AddGlue()
		.SetInsets(10, 12, 10, 10);

	BTab* sortTab = new BTab();
	AddTab(sortView, sortTab);
	sortTab->SetLabel(B_TRANSLATE("Sort lines"));
}


// Search and replace
BString
Sidebar::getSearchText() const
{
	return fSearchInput->Text();
}


void
Sidebar::setSearchText(const BString& text)
{
	fSearchInput->SetText(text);
}


BString
Sidebar::getReplaceText() const
{
	return fReplaceInput->Text();
}


void
Sidebar::setReplaceText(const BString& text)
{
	fReplaceInput->SetText(text);
}


bool
Sidebar::getReplaceCaseSensitive() const
{
	return fCaseCheck->Value() == B_CONTROL_ON;
}


void
Sidebar::setReplaceCaseSensitive(bool enabled)
{
	fCaseCheck->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF);
}


bool
Sidebar::getReplaceFullWords() const
{
	return fWholeWordCheck->Value() == B_CONTROL_ON;
}


void
Sidebar::setReplaceFullWords(bool enabled)
{
	fWholeWordCheck->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF);
}


// Line break mode
int8
Sidebar::getBreakMode() const
{
	return fBreakMode;
}


void
Sidebar::setBreakMode(int breakMode)
{
	fBreakMenu->ItemAt(breakMode)->SetMarked(true);
}


BString
Sidebar::getBreakModeInput()
{
	return fBreakInput->Text();
}


void
Sidebar::setBreakModeInput(const BString& text)
{
	fBreakInput->SetText(text);
}


int
Sidebar::getBreakOnCharsSpinner()
{
	return fBreakOnChars->Value();
}


void
Sidebar::setBreakOnCharsSpinner(const int value)
{
	fBreakOnChars->SetValue(value);
}


bool
Sidebar::getSplitOnWords() const
{
	return fWordWrapCheck->Value() == B_CONTROL_ON;
}


void
Sidebar::setSplitOnWords(bool enabled)
{
	fWordWrapCheck->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF);
}


bool
Sidebar::getKeepDelimiterValue()
{
	return fKeepDelimiterCheck->Value() == B_CONTROL_ON;
}


void
Sidebar::setKeepDelimiterValue(const bool value)
{
	fKeepDelimiterCheck->SetValue(value ? B_CONTROL_ON : B_CONTROL_OFF);
}


// Prefix/suffix
BString
Sidebar::getPrefixText() const
{
	return fPrefixInput->Text();
}


void
Sidebar::setPrefixText(const BString& text)
{
	fPrefixInput->SetText(text);
}


BString
Sidebar::getSuffixText() const
{
	return fSuffixInput->Text();
}


void
Sidebar::setSuffixText(const BString& text)
{
	fSuffixInput->SetText(text);
}


// Indent/unindent
int32
Sidebar::getIndentSpinner() const
{
	return fIndentSizeSpinner->Value();
}


void
Sidebar::setIndentSpinner(const int value)
{
	fIndentSizeSpinner->SetValue(value);
}


bool
Sidebar::getTabsRadio() const
{
	return fTabsRadio->Value() == B_CONTROL_ON;
}


void
Sidebar::setTabsRadio(bool enabled) const
{
	fTabsRadio->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF);
	fSpacesRadio->SetValue(enabled ? B_CONTROL_OFF : B_CONTROL_ON);
}

// Sort lines
bool
Sidebar::getAlphaSortRadio() const
{
	return fAlphaSortRadio && fAlphaSortRadio->Value() == B_CONTROL_ON;
}


void
Sidebar::setAlphaSortRadio(bool enabled) const
{
	fAlphaSortRadio->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF);
	fLengthSortRadio->SetValue(enabled ? B_CONTROL_OFF : B_CONTROL_ON);
}


bool
Sidebar::getSortAsc() const
{
	return fSortAsc && fSortAsc->Value() == B_CONTROL_ON;
}


void
Sidebar::setSortAsc(bool enabled) const
{
	fSortAsc->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF);
	fSortDesc->SetValue(enabled ? B_CONTROL_OFF : B_CONTROL_ON);
}


bool
Sidebar::getCaseSortCheck() const
{
	return fCaseSortCheck && fCaseSortCheck->Value() == B_CONTROL_ON;
}


void
Sidebar::setCaseSortCheck(bool enabled)
{
	fCaseSortCheck->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF);
}
