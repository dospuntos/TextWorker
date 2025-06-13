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
	// Tab: Line operations
	BGroupView* lineOperationsView = new BGroupView(B_VERTICAL, 5);

	// === Widgets ===
	fLineBreakInput = new BTextControl("ReplaceLineBreaksInput", nullptr, "", nullptr);
	fPrefixInput = new BTextControl("PrefixInput", "Prefix:", "", nullptr);
	fSuffixInput = new BTextControl("SuffixInput", "Suffix:", "", nullptr);
	fRemovePrefixInput = new BTextControl("RPrefixInput", "Prefix:", "", nullptr);
	fRemoveSuffixInput = new BTextControl("RSuffixInput", "Suffix:", "", nullptr);
	fBreakInput = new BTextControl("MaxWidthInput", nullptr, "", nullptr);
	fWordWrapCheck = new BCheckBox("SplitOnWordsCheckbox", "Split on words", nullptr);
	fDelimiterInput = new BTextControl("LineBreakDelimiter", nullptr, "", nullptr);

	fSearchInput = new BTextControl("ReplaceSearchString", nullptr, "", nullptr);
	fReplaceInput = new BTextControl("ReplaceWithString", nullptr, "", nullptr);
	fCaseCheck = new BCheckBox("ReplaceCaseSensitiveCheckbox", "Case sensitive", nullptr);
	fWholeWordCheck = new BCheckBox("ReplaceFullWordsCheckbox", "Full words", nullptr);

	float maxLabelWidth = 0;

	BTextControl* fields[] = {fLineBreakInput, fPrefixInput, fSuffixInput, fBreakInput,
		fDelimiterInput, fSearchInput, fReplaceInput, nullptr};

	for (int i = 0; fields[i]; ++i) {
		float width = be_plain_font->StringWidth(fields[i]->Label());
		maxLabelWidth = MAX(width, maxLabelWidth);
	}

	maxLabelWidth += 10;

	// === Search/Replace Box ===
	BBox* searchReplaceBox = new BBox("SearchReplaceBox");
	searchReplaceBox->SetLabel("Search and replace");

	// Controls
	BButton* searchReplaceBtn
		= new BButton("SearchReplaceBtn", "Replace", new BMessage(M_TRANSFORM_REPLACE));
	searchReplaceBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	// Set consistent minimum widths for text fields
	fSearchInput->SetExplicitMinSize(BSize(150, B_SIZE_UNSET));
	fReplaceInput->SetExplicitMinSize(BSize(150, B_SIZE_UNSET));

	// Grid layout
	BGridLayoutBuilder grid1(B_USE_SMALL_SPACING, B_USE_SMALL_SPACING);
	// clang-format off
	grid1.Add(new BStringView(NULL, "Find:"),			0, 0)
		.Add(fSearchInput,								1, 0)
		.Add(new BStringView(NULL, "Replace with:"),	0, 1)
		.Add(fReplaceInput,                       		1, 1)

		.Add(fCaseCheck,            					0, 2)
		.Add(fWholeWordCheck,                			1, 2)

		.Add(searchReplaceBtn,                        0, 3, 2);
	// clang-format on
	grid1.GridLayout()->SetMinColumnWidth(0, maxLabelWidth);
	// Wrap in a group and add to the box
	BGroupView* searchGroup = new BGroupView(B_VERTICAL, 0);
	searchReplaceBox->AddChild(searchGroup);
	searchGroup->GroupLayout()->AddView(grid1.View());
	searchGroup->GroupLayout()->SetInsets(10, 12, 10, 10);

	// === Line Breaks Options Box ===
	// Create the container box
	BBox* breakBox = new BBox("LineBreakOptionsBox");
	breakBox->SetLabel("Line break options");

	fBreakMenu = new BMenu("linebreaks");
	fBreakMenu->AddItem(new BMenuItem("Remove all", new BMessage(M_MODE_REMOVE_ALL)));
	fBreakMenu->AddItem(
		new BMenuItem("Replace with" B_UTF8_ELLIPSIS, new BMessage(M_MODE_REPLACE_LINE_BREAKS)));
	fBreakMenu->AddItem(new BMenuItem("Break on" B_UTF8_ELLIPSIS, new BMessage(M_MODE_BREAK_ON)));
	fBreakMenu->AddItem(
		new BMenuItem("Break after X characters" B_UTF8_ELLIPSIS, new BMessage(M_MODE_BREAK_AFTER_CHARS)));

	fBreakMenu->SetLabelFromMarked(true);
	fBreakMenu->SetRadioMode(true);
	fBreakMenu->ItemAt(0L)->SetMarked(true);
	fBreakMenu->SetTargetForItems(this);

	BMenuField* breakField = new BMenuField("breakField", "", fBreakMenu);

	fWordWrapCheck = new BCheckBox("SplitWords", "Split on words", NULL);
	fWordWrapCheck->SetEnabled(false);
	fBreakInput->SetExplicitMinSize(BSize(100, B_SIZE_UNSET));
	BButton* applyBtn = new BButton("ApplyBtn", "Apply", new BMessage(M_REMOVE_LINE_BREAKS));
	applyBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	// Layout grid
	BGridLayoutBuilder grid(B_USE_SMALL_SPACING, B_USE_SMALL_SPACING);
	// clang-format off
	grid.Add(breakField,		0, 0)
		.Add(fBreakInput,		0, 1)
		.Add(fWordWrapCheck,    0, 2)
		.Add(applyBtn,          0, 3);
	// clang-format on
	grid.GridLayout()->SetMinColumnWidth(0, maxLabelWidth);

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
	BButton* trimEmptyLinesBtn
		= new BButton("TrimEmptyLinesBtn", "Remove empty lines", new BMessage(M_TRIM_EMPTY_LINES));
	BButton* removeDuplicates = new BButton("RemoveDuplicatesBtn", "Remove duplicates",
		new BMessage(M_REMOVE_DUPLICATES));
	trimLinesBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	trimEmptyLinesBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
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

	// === Prefix/Suffix Tab ===
	// Tab: Line operations
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

	// Inputs
	fIndentCountInput = new BTextControl("IndentCount", "Indent size:", "4", nullptr);
	fUseTabsCheckbox = new BCheckBox("UseTabs", "Use tabs instead of spaces", nullptr);

	// Buttons
	BButton* indentButton = new BButton("IndentBtn", "Indent", new BMessage(M_INDENT_LINES));
	BButton* unindentButton
		= new BButton("UnindentBtn", "Unindent", new BMessage(M_UNINDENT_LINES));

	// Layout
	BLayoutBuilder::Group<>(indentGroup)
		.Add(fIndentCountInput)
		.Add(fUseTabsCheckbox)
		.AddGroup(B_HORIZONTAL)
		.Add(indentButton)
		.AddGlue()
		.Add(unindentButton)
		.End()
		.SetInsets(10, 12, 10, 10);

	// === Prefix/suffix Tab ===
	BLayoutBuilder::Group<>(prefixSuffixView, B_VERTICAL, 10)
		.Add(prefixSuffixBox)
		.Add(indentBox)
		.AddGlue()
		.SetInsets(10, 12, 10, 10);

	BTab* prefixSuffixTab = new BTab();
	AddTab(prefixSuffixView, prefixSuffixTab);
	prefixSuffixTab->SetLabel("Prefix/suffix");

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
	BRadioButton* alphabeticalRadio = new BRadioButton("radio_sort_alpha", "Alphabetical", nullptr);
	BRadioButton* lengthRadio = new BRadioButton("radio_sort_length", "By line length", nullptr);
	alphabeticalRadio->SetValue(B_CONTROL_ON);

	BLayoutBuilder::Group<>(sortTypeGroup)
		.Add(new BStringView(nullptr, "Sort type:"))
		.Add(alphabeticalRadio)
		.Add(lengthRadio);

	// --- Sort order group ---
	BGroupView* sortOrderGroup = new BGroupView(B_VERTICAL, 5);
	BRadioButton* ascendingRadio = new BRadioButton("radio_sort_asc", "Ascending", nullptr);
	BRadioButton* descendingRadio = new BRadioButton("radio_sort_desc", "Descending", nullptr);
	ascendingRadio->SetValue(B_CONTROL_ON); // default

	BLayoutBuilder::Group<>(sortOrderGroup)
		.Add(new BStringView(nullptr, "Sort order:"))
		.Add(ascendingRadio)
		.Add(descendingRadio);

	// --- Case sensitivity checkbox ---
	BCheckBox* caseCheck = new BCheckBox("checkbox_case", "Case sensitive", nullptr);
	caseCheck->SetValue(B_CONTROL_OFF);

	// --- Sort button ---
	BButton* sortButton = new BButton("sortBtn", "Sort", new BMessage(M_SORT_LINES));

	// Layout the full sortGroup box
	BLayoutBuilder::Group<>(sortGroup)
		.Add(sortTypeGroup)
		.Add(sortOrderGroup)
		.Add(caseCheck)
		.Add(sortButton)
		.SetInsets(10, 12, 10, 10);


	// Pre-select default options
	alphabeticalRadio->SetValue(B_CONTROL_ON);
	ascendingRadio->SetValue(B_CONTROL_ON);
	caseCheck->SetValue(B_CONTROL_OFF);

	// === Sort Tab ===
	BLayoutBuilder::Group<>(sortView, B_VERTICAL, 10)
		.Add(sortBox)
		.AddGlue()
		.SetInsets(10, 12, 10, 10);

	BTab* sortTab = new BTab();
	AddTab(sortView, sortTab);
	sortTab->SetLabel("Sort lines");
}


void
Sidebar::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case M_MODE_REMOVE_ALL:
			fWordWrapCheck->SetEnabled(false);
			fBreakInput->SetEnabled(false);
			this->Invalidate();
			fBreakMode = BREAK_REMOVE_ALL;
			break;
		case M_MODE_BREAK_ON:
			fWordWrapCheck->SetEnabled(false);
			fBreakInput->SetEnabled(true);
			this->Invalidate();
			fBreakMode = BREAK_ON;
			break;
		case M_MODE_REPLACE_LINE_BREAKS:
			fWordWrapCheck->SetEnabled(false);
			fBreakInput->SetEnabled(true);
			fBreakMode = BREAK_REPLACE;
			this->Invalidate();
			fBreakMode = BREAK_REPLACE;
			break;
		case M_MODE_BREAK_AFTER_CHARS:
			fWordWrapCheck->SetEnabled(true);
			fBreakInput->SetEnabled(true);
			fBreakMode = BREAK_AFTER_CHARS;
			this->Invalidate();
			break;
		default:
			BView::MessageReceived(msg);
			break;
	}
}
