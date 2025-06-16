/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Sidebar.h"
#include "Constants.h"

#include <Alert.h>
#include <Box.h>
#include <Button.h>
#include <GridLayoutBuilder.h>
#include <GroupView.h>
#include <LayoutBuilder.h>
#include <RadioButton.h>
#include <StringView.h>
#include <TabView.h>
#include <TextControl.h>
#include <Window.h>


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
	searchReplaceBox->SetLabel("Search and replace");

	fSearchInput = new BTextControl("SearchString", nullptr, "", nullptr);
	fReplaceInput = new BTextControl("ReplaceString", nullptr, "", nullptr);
	fCaseCheck = new BCheckBox("ReplaceCaseSensitiveCheckbox", "Case sensitive", nullptr);
	fWholeWordCheck = new BCheckBox("ReplaceFullWordsCheckbox", "Full words", nullptr);

	BButton* searchReplaceBtn
		= new BButton("SearchReplaceBtn", "Replace", new BMessage(M_TRANSFORM_REPLACE));
	searchReplaceBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	// Set consistent minimum widths for text fields
	fSearchInput->SetExplicitMinSize(BSize(150, B_SIZE_UNSET));
	fReplaceInput->SetExplicitMinSize(BSize(150, B_SIZE_UNSET));

	// Grid layout
	BGridLayoutBuilder grid1(B_USE_SMALL_SPACING, B_USE_SMALL_SPACING);
	// clang-format off
	grid1.Add(new BStringView(NULL, "Find:"),		0, 0, 1)
		.Add(fSearchInput,							1, 0, 1)
		.Add(new BStringView(NULL, "Replace:"),		0, 1, 1)
		.Add(fReplaceInput,                       	1, 1, 1)
		.Add(fCaseCheck,            				0, 2, 1)
		.Add(fWholeWordCheck,                		1, 2, 1)
		.Add(searchReplaceBtn,                      0, 3, 2);
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
	breakBox->SetLabel("Line break options");

	fBreakMenu = new BMenu("linebreaks");
	fBreakMenu->AddItem(new BMenuItem("Remove all", new BMessage(M_MODE_REMOVE_ALL)));
	fBreakMenu->AddItem(
		new BMenuItem("Replace with" B_UTF8_ELLIPSIS, new BMessage(M_MODE_REPLACE_LINE_BREAKS)));
	fBreakMenu->AddItem(new BMenuItem("Break on" B_UTF8_ELLIPSIS, new BMessage(M_MODE_BREAK_ON)));
	fBreakMenu->AddItem(new BMenuItem("Break after X characters" B_UTF8_ELLIPSIS,
		new BMessage(M_MODE_BREAK_AFTER_CHARS)));
	fBreakMenu->SetLabelFromMarked(true);
	fBreakMenu->SetRadioMode(true);
	fBreakMenu->ItemAt(0L)->SetMarked(true);

	BMenuField* breakField = new BMenuField("breakField", "", fBreakMenu);

	fBreakInput = new BTextControl("MaxWidthInput", nullptr, "", nullptr);
	fBreakInput->SetEnabled(false);
	fBreakInput->SetExplicitMinSize(BSize(100, B_SIZE_UNSET));

	fBreakOnChars = new BSpinner("BreakOn", "Chars:", nullptr);
	fBreakOnChars->SetMinValue(1);
	fBreakOnChars->SetEnabled(false);

	fWordWrapCheck = new BCheckBox("SplitWords", "Split on words", NULL);
	fWordWrapCheck->SetEnabled(false);

	fKeepDelimiterCheck = new BCheckBox("KeepDelimiter", "Keep delimiter", NULL);
	fKeepDelimiterCheck->SetEnabled(false);

	BButton* applyBtn = new BButton("ApplyBtn", "Apply", new BMessage(M_REMOVE_LINE_BREAKS));
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
	cleanupBox->SetLabel("Cleanup");
	BGroupView* cleanGroup = new BGroupView(B_VERTICAL, 5);
	cleanupBox->AddChild(cleanGroup);

	// Layout for Cleanup Box
	BButton* trimLinesBtn
		= new BButton("TrimLinesBtn", "Trim whitespace", new BMessage(M_TRIM_LINES));
	trimLinesBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BButton* trimEmptyLinesBtn
		= new BButton("TrimEmptyLinesBtn", "Remove empty lines", new BMessage(M_TRIM_EMPTY_LINES));
	trimEmptyLinesBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BButton* removeDuplicates = new BButton("RemoveDuplicatesBtn", "Remove duplicate lines",
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
	lineOperationsTab->SetLabel("Lines");
}


void
Sidebar::_BuildPrefixTab()
{
	fPrefixInput = new BTextControl("PrefixInput", "Prefix:", "", nullptr);
	fSuffixInput = new BTextControl("SuffixInput", "Suffix:", "", nullptr);
	fRemovePrefixInput = new BTextControl("RPrefixInput", "Prefix:", "", nullptr);
	fRemoveSuffixInput = new BTextControl("RSuffixInput", "Suffix:", "", nullptr);

	BGroupView* prefixSuffixView = new BGroupView(B_VERTICAL, 5);

	// === Add prefix/suffix Box ===
	BBox* prefixSuffixBox = new BBox("PrefixSuffixBox");
	prefixSuffixBox->SetLabel("Prefix/suffix each line");
	BGroupView* addGroup = new BGroupView(B_VERTICAL, 5);
	prefixSuffixBox->AddChild(addGroup);

	// clang-format off
	BLayoutBuilder::Group<>(addGroup)
		.Add(fPrefixInput)
		.Add(fSuffixInput)
		.AddGroup(B_HORIZONTAL)
			.Add(new BButton("addPrefixSuffixBtn", "Add", new BMessage(M_TRANSFORM_PREFIX_SUFFIX)))
			.AddGlue()
			.Add(new BButton("removePrefixSuffixBtn", "Remove", new BMessage(M_TRANSFORM_REMOVE_PREFIX_SUFFIX)))
		.End()
		.SetInsets(10, 12, 10, 10);
	// clang-format on

	// === Indent/Unindent Box ===
	BBox* indentBox = new BBox("IndentBox");
	indentBox->SetLabel("Indent/unindent lines");

	BGroupView* indentGroup = new BGroupView(B_VERTICAL, 5);
	indentBox->AddChild(indentGroup);

	// Spinner for indent size
	fIndentSizeSpinner = new BSpinner("IndentSize", "Indent size:", new BMessage());
	fIndentSizeSpinner->SetValue(4);
	fIndentSizeSpinner->SetRange(1, 16);

	// Radio buttons for tab/space selection
	fTabsRadio = new BRadioButton("TabsRadio", "Tabs", nullptr);
	fSpacesRadio = new BRadioButton("SpacesRadio", "Spaces", nullptr);
	fSpacesRadio->SetValue(B_CONTROL_ON);

	// Group the radio buttons horizontally
	BGroupView* indentModeGroup = new BGroupView(B_HORIZONTAL, 10);
	indentModeGroup->AddChild(fTabsRadio);
	indentModeGroup->AddChild(fSpacesRadio);

	// Buttons
	BButton* indentButton = new BButton("IndentBtn", "Indent", new BMessage(M_INDENT_LINES));
	BButton* unindentButton
		= new BButton("UnindentBtn", "Unindent", new BMessage(M_UNINDENT_LINES));

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
	prefixSuffixTab->SetLabel("Prefix/suffix");
}


void
Sidebar::_BuildSortTab()
{
	// === Sort lines Tab ===
	// Tab: Line operations
	BGroupView* sortView = new BGroupView(B_VERTICAL, 5);

	// === Sort Box ===
	BBox* sortBox = new BBox("SortBox");
	sortBox->SetLabel("Sort lines");
	BGroupView* sortGroup = new BGroupView(B_VERTICAL, 10);
	sortBox->AddChild(sortGroup);

	// --- Sorting method group ---
	BGroupView* sortTypeGroup = new BGroupView(B_VERTICAL, 5);
	fAlphaSortRadio = new BRadioButton("radio_sort_alpha", "Alphabetical", nullptr);
	fLengthSortRadio = new BRadioButton("radio_sort_length", "By line length", nullptr);

	BLayoutBuilder::Group<>(sortTypeGroup)
		.Add(new BStringView(nullptr, "Sort type:"))
		.Add(fAlphaSortRadio)
		.Add(fLengthSortRadio);

	// --- Sort order group ---
	BGroupView* sortOrderGroup = new BGroupView(B_VERTICAL, 5);
	fSortAsc = new BRadioButton("radio_sort_asc", "Ascending", nullptr);
	fSortDesc = new BRadioButton("radio_sort_desc", "Descending", nullptr);

	BLayoutBuilder::Group<>(sortOrderGroup)
		.Add(new BStringView(nullptr, "Sort order:"))
		.Add(fSortAsc)
		.Add(fSortDesc);

	fCaseSortCheck = new BCheckBox("caseSortCheck", "Case sensitive", nullptr);

	BButton* sortButton = new BButton("sortBtn", "Sort", new BMessage(M_SORT_LINES));
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
	sortTab->SetLabel("Sort lines");
}
