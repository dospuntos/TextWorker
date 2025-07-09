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
	BBox* searchReplaceBox = new BBox("SearchReplaceBox");
	searchReplaceBox->SetLabel(B_TRANSLATE("Search and replace"));

	fSearchInput = new BTextControl("SearchString",
		B_TRANSLATE_COMMENT("Search:", "As short as possible"), "", nullptr);
	fReplaceInput = new BTextControl("ReplaceString",
		B_TRANSLATE_COMMENT("Replace:", "As short as possible"), "", nullptr);
	fCaseCheck = new BCheckBox("ReplaceCaseSensitiveCheckbox",
		B_TRANSLATE_COMMENT("Case sensitive", "As short as possible"), nullptr);
	fWholeWordCheck = new BCheckBox("ReplaceFullWordsCheckbox",
		B_TRANSLATE_COMMENT("Full words", "As short as possible"), nullptr);

	BButton* searchReplaceBtn = new BButton("SearchReplaceBtn", B_TRANSLATE("Replace"),
		new BMessage(M_TRANSFORM_REPLACE));
	searchReplaceBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BView* searchView = new BView("searchreplace", B_WILL_DRAW);
	// clang-format off
	BLayoutBuilder::Group<>(searchView, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_HALF_ITEM_INSETS, B_USE_ITEM_INSETS,
			B_USE_HALF_ITEM_INSETS, B_USE_HALF_ITEM_INSETS)
		.AddGrid(B_USE_HALF_ITEM_SPACING, B_USE_HALF_ITEM_SPACING)
			.Add(fSearchInput->CreateLabelLayoutItem(),			0, 0, 1)
			.Add(fSearchInput->CreateTextViewLayoutItem(),		1, 0, 1)
			.Add(fReplaceInput->CreateLabelLayoutItem(),		0, 1, 1)
			.Add(fReplaceInput->CreateTextViewLayoutItem(),		1, 1, 1)
			.End()
		.AddGroup(B_HORIZONTAL, B_USE_SMALL_SPACING)
			.Add(fCaseCheck)
			.Add(fWholeWordCheck)
			.End()
		.Add(searchReplaceBtn)
		.End();
	// clang-format on

	searchReplaceBox->AddChild(searchView);


	// === Line Breaks Options Box ===
	BBox* breakBox = new BBox("LineBreakOptionsBox");
	breakBox->SetLabel(B_TRANSLATE("Line break options"));

	fBreakMenu = new BMenu("linebreaks");
	fBreakMenu->AddItem(new BMenuItem(B_TRANSLATE("Remove all"), new BMessage(M_MODE_REMOVE_ALL)));
	fBreakMenu->AddItem(new BMenuItem(B_TRANSLATE("Replace with" B_UTF8_ELLIPSIS),
		new BMessage(M_MODE_REPLACE_LINE_BREAKS)));
	fBreakMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Break on" B_UTF8_ELLIPSIS), new BMessage(M_MODE_BREAK_ON)));
	fBreakMenu->AddItem(new BMenuItem(B_TRANSLATE("Break after X characters" B_UTF8_ELLIPSIS),
		new BMessage(M_MODE_BREAK_AFTER_CHARS)));
	fBreakMenu->SetLabelFromMarked(true);
	fBreakMenu->SetRadioMode(true);
	fBreakMenu->ItemAt(0L)->SetMarked(true);

	BMenuField* breakField = new BMenuField("breakField", "", fBreakMenu);

	fBreakInput = new BTextControl("MaxWidthInput", nullptr, "", nullptr);
	fBreakInput->SetEnabled(false);
	fBreakInput->SetExplicitMinSize(BSize(100, B_SIZE_UNSET));

	fBreakOnChars
		= new BSpinner("BreakOn", B_TRANSLATE_COMMENT("Chars:", "As short as possible"), nullptr);
	fBreakOnChars->SetMinValue(1);
	fBreakOnChars->SetEnabled(false);

	fWordWrapCheck = new BCheckBox("SplitWords",
		B_TRANSLATE_COMMENT("Split on words", "As short as possible"), NULL);
	fWordWrapCheck->SetEnabled(false);

	fKeepDelimiterCheck = new BCheckBox("KeepDelimiter",
		B_TRANSLATE_COMMENT("Keep delimiter", "As short as possible"), NULL);
	fKeepDelimiterCheck->SetEnabled(false);

	BButton* applyBtn
		= new BButton("ApplyBtn", B_TRANSLATE("Apply"), new BMessage(M_REMOVE_LINE_BREAKS));
	applyBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BView* breakView = new BView("searchreplace", B_WILL_DRAW);
	// clang-format off
	BLayoutBuilder::Group<>(breakView, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_HALF_ITEM_INSETS, B_USE_ITEM_INSETS,
			B_USE_HALF_ITEM_INSETS, B_USE_HALF_ITEM_INSETS)
		.Add(breakField)
		.AddGroup(B_HORIZONTAL)
			.Add(fBreakInput)
			.Add(fBreakOnChars)
			.End()
		.AddGroup(B_HORIZONTAL)
			.Add(fWordWrapCheck)
			.Add(fKeepDelimiterCheck)
			.End()
		.Add(applyBtn)
		.End();
	// clang-format on

	breakBox->AddChild(breakView);


	// === Cleanup Box ===
	BBox* cleanupBox = new BBox("CleanupBox");
	cleanupBox->SetLabel(B_TRANSLATE("Cleanup"));

	BButton* trimLinesBtn = new BButton("TrimLinesBtn",
		B_TRANSLATE_COMMENT("Trim whitespace", "As short as possible"), new BMessage(M_TRIM_LINES));
	trimLinesBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BButton* trimEmptyLinesBtn = new BButton("TrimEmptyLinesBtn",
		B_TRANSLATE_COMMENT("Remove empty lines", "As short as possible"),
		new BMessage(M_TRIM_EMPTY_LINES));
	trimEmptyLinesBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BButton* removeDuplicates = new BButton("RemoveDuplicatesBtn",
		B_TRANSLATE_COMMENT("Remove duplicate lines", "As short as possible"),
		new BMessage(M_REMOVE_DUPLICATES));
	removeDuplicates->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BView* cleanView = new BView("cleanView", B_WILL_DRAW);
	// clang-format off
	BLayoutBuilder::Group<>(cleanView, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(B_USE_HALF_ITEM_INSETS, B_USE_ITEM_INSETS,
			B_USE_HALF_ITEM_INSETS, B_USE_HALF_ITEM_INSETS)
		.Add(trimLinesBtn)
		.Add(trimEmptyLinesBtn)
		.Add(removeDuplicates)
		.End();
	// clang-format on

	cleanupBox->AddChild(cleanView);


	// === Line Operations Tab ===
	BView* lineOperationsView = new BView("lineView", B_WILL_DRAW);
	BLayoutBuilder::Group<>(lineOperationsView, B_VERTICAL, B_USE_SMALL_SPACING)
		.SetInsets(B_USE_ITEM_INSETS)
		.Add(searchReplaceBox)
		.Add(breakBox)
		.Add(cleanupBox)
		.AddGlue()
		.End();

	BTab* lineOperationsTab = new BTab();
	AddTab(lineOperationsView, lineOperationsTab);
	lineOperationsTab->SetLabel(B_TRANSLATE_COMMENT("Lines", "As short as possible"));
}


void
Sidebar::_BuildPrefixTab()
{
	// === Prefix/Suffix Box ===
	BBox* prefixSuffixBox = new BBox("PrefixSuffixBox");
	prefixSuffixBox->SetLabel(B_TRANSLATE("Prefix/suffix each line"));

	fPrefixInput = new BTextControl("PrefixInput",
		B_TRANSLATE_COMMENT("Prefix:", "As short as possible"), "", nullptr);
	fSuffixInput = new BTextControl("SuffixInput",
		B_TRANSLATE_COMMENT("Suffix:", "As short as possible"), "", nullptr);

	BButton* addBtn = new BButton("addPrefixSuffixBtn", B_TRANSLATE_COMMENT(
		"Add", "As short as possible"), new BMessage(M_TRANSFORM_PREFIX_SUFFIX));

	BButton* removeBtn = new BButton("removePrefixSuffixBtn", B_TRANSLATE_COMMENT(
		"Remove", "As short as possible"), new BMessage(M_TRANSFORM_REMOVE_PREFIX_SUFFIX));

	BView* presuffixView = new BView("prefixsuffix", B_WILL_DRAW);

	// clang-format off
	BLayoutBuilder::Group<>(presuffixView, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_HALF_ITEM_INSETS, B_USE_ITEM_INSETS,
			B_USE_HALF_ITEM_INSETS, B_USE_HALF_ITEM_INSETS)
		.AddGrid(B_USE_HALF_ITEM_SPACING, B_USE_HALF_ITEM_SPACING)
			.Add(fPrefixInput->CreateLabelLayoutItem(),			0, 0, 1)
			.Add(fPrefixInput->CreateTextViewLayoutItem(),		1, 0, 1)
			.Add(fSuffixInput->CreateLabelLayoutItem(),			0, 1, 1)
			.Add(fSuffixInput->CreateTextViewLayoutItem(),		1, 1, 1)
			.End()
		.AddGroup(B_HORIZONTAL)
			.Add(addBtn)
			.AddGlue()
			.Add(removeBtn)
			.End();
	// clang-format on

	prefixSuffixBox->AddChild(presuffixView);


	// === Indent/Unindent Box ===
	BBox* indentBox = new BBox("IndentBox");
	indentBox->SetLabel(B_TRANSLATE("Indent/unindent lines"));

	// Spinner for indent size
	fIndentSizeSpinner = new BSpinner("IndentSize", B_TRANSLATE("Indent size:"), nullptr);
	fIndentSizeSpinner->SetValue(4);
	fIndentSizeSpinner->SetRange(1, 16);

	// Radio buttons for tab/space selection
	fTabsRadio = new BRadioButton("TabsRadio", B_TRANSLATE("Tabs"), nullptr);
	fSpacesRadio = new BRadioButton("SpacesRadio", B_TRANSLATE("Spaces"), nullptr);
	fSpacesRadio->SetValue(B_CONTROL_ON);

	// Buttons
	BButton* indentButton = new BButton("IndentBtn",
		B_TRANSLATE_COMMENT("Indent", "As short as possible"), new BMessage(M_INDENT_LINES));
	BButton* unindentButton = new BButton("UnindentBtn",
		B_TRANSLATE_COMMENT("Unindent", "As short as possible"), new BMessage(M_UNINDENT_LINES));

	BView* indentView = new BView("indent", B_WILL_DRAW);
	// clang-format off
	BLayoutBuilder::Group<>(indentView, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_HALF_ITEM_INSETS, B_USE_ITEM_INSETS,
			B_USE_HALF_ITEM_INSETS, B_USE_HALF_ITEM_INSETS)
		.Add(fIndentSizeSpinner)
		.AddGroup(B_HORIZONTAL)
			.Add(fTabsRadio)
			.Add(fSpacesRadio)
			.End()
		.AddGroup(B_HORIZONTAL)
			.Add(indentButton)
			.AddGlue()
			.Add(unindentButton)
		.End();
	// clang-format on

	indentBox->AddChild(indentView);

	// === Prefix/suffix Tab ===
	BView* prefixSuffixView = new BView("presuffixView", B_WILL_DRAW);
	BLayoutBuilder::Group<>(prefixSuffixView, B_VERTICAL, B_USE_SMALL_SPACING)
		.SetInsets(B_USE_ITEM_INSETS)
		.Add(prefixSuffixBox)
		.Add(indentBox)
		.AddGlue()
		.End();

	BTab* prefixSuffixTab = new BTab();
	AddTab(prefixSuffixView, prefixSuffixTab);
	prefixSuffixTab->SetLabel(B_TRANSLATE_COMMENT("Prefix/suffix", "As short as possible"));
}


void
Sidebar::_BuildSortTab()
{
	// === Sort Box ===
	BBox* sortBox = new BBox("SortBox");
	sortBox->SetLabel(B_TRANSLATE("Sort lines"));

	// --- Sorting method group ---
	fAlphaSortRadio = new BRadioButton("radio_sort_alpha", B_TRANSLATE("Alphabetical"), nullptr);
	fLengthSortRadio
		= new BRadioButton("radio_sort_length", B_TRANSLATE("By line length"), nullptr);

	BView* sortTypeGroup = new BView("sortTypeGroup", B_WILL_DRAW);
	BLayoutBuilder::Group<>(sortTypeGroup, B_VERTICAL, 0)
		.Add(fAlphaSortRadio)
		.Add(fLengthSortRadio);

	// --- Sort order group ---
	fSortAsc = new BRadioButton("ascRadio", B_TRANSLATE("Ascending"), nullptr);
	fSortDesc = new BRadioButton("descRadio", B_TRANSLATE("Descending"), nullptr);
	fCaseSortCheck = new BCheckBox("caseSortCheck", B_TRANSLATE("Case sensitive"), nullptr);

	BView* sortOrderGroup = new BView("sortOrderGroup", B_WILL_DRAW);
	BLayoutBuilder::Group<>(sortOrderGroup, B_VERTICAL, 0)
		.Add(fSortAsc)
		.Add(fSortDesc)
		.AddStrut(B_USE_HALF_ITEM_SPACING)
		.Add(fCaseSortCheck);

	BButton* sortButton = new BButton("sortBtn", B_TRANSLATE("Sort"), new BMessage(M_SORT_LINES));
	sortButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	// Layout the full Sort Box
	BView* sortView = new BView("sortView", B_WILL_DRAW);
	// clang-format off
	BLayoutBuilder::Group<>(sortView, B_VERTICAL)
		.SetInsets(B_USE_HALF_ITEM_INSETS, B_USE_ITEM_INSETS,
			B_USE_HALF_ITEM_INSETS, B_USE_HALF_ITEM_INSETS)
		.Add(new BStringView(nullptr, B_TRANSLATE("Sort type:")))
		.AddGroup(B_HORIZONTAL)
			.AddStrut(B_USE_HALF_ITEM_SPACING)
			.Add(sortTypeGroup)
			.AddGlue()
			.End()
		.Add(new BStringView(nullptr, B_TRANSLATE("Sort order:")))
		.AddGroup(B_HORIZONTAL)
			.AddStrut(B_USE_HALF_ITEM_SPACING)
			.Add(sortOrderGroup)
			.AddGlue()
			.End()
		.Add(sortButton)
		.End();
	// clang-format on

	sortBox->AddChild(sortView);

	fAlphaSortRadio->SetValue(B_CONTROL_ON);
	fSortAsc->SetValue(B_CONTROL_ON);
	fCaseSortCheck->SetValue(B_CONTROL_OFF);

	// === Sort Tab ===
	BView* sortingView = new BView("sortview", B_WILL_DRAW);
	// clang-format off
	BLayoutBuilder::Group<>(sortingView, B_VERTICAL, B_USE_SMALL_SPACING)
		.SetInsets(B_USE_ITEM_INSETS)
		.Add(sortBox)
		.AddGlue()
		.End();
	// clang-format on

	BTab* sortTab = new BTab();
	AddTab(sortingView, sortTab);
	sortTab->SetLabel(B_TRANSLATE_COMMENT("Sort lines", "As short as possible"));
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
