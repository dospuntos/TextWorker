/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "Sidebar.h"
#include "MainWindow.h"
#include <Box.h>
#include <Button.h>
#include <GroupView.h>
#include <LayoutBuilder.h>
#include <RadioButton.h>
#include <StringView.h>
#include <TabView.h>
#include <TextControl.h>


Sidebar::Sidebar()
	:
	BTabView("Sidebar")
{
	// Tab: Line operations
	BGroupView* lineOperationsView = new BGroupView(B_VERTICAL, 5);

	// === Widgets ===
	replaceLineBreaksInput = new BTextControl("ReplaceLineBreaksInput", "Replacement:", "", nullptr);
	prefixInput = new BTextControl("PrefixInput", "Prefix:", "", nullptr);
	suffixInput = new BTextControl("SuffixInput", "Suffix:", "", nullptr);
	maxWidthInput = new BTextControl("MaxWidth", "Characters:", "", nullptr);
	splitOnWordsCheckbox = new BCheckBox("SplitOnWordsCheckbox", "Split on words", nullptr);
	lineBreakDelimiterInput = new BTextControl("LineBreakDelimiter", "Break on:", "", nullptr);

	replaceSearchString = new BTextControl("ReplaceSearchString", "Find:", "", nullptr);
	replaceWithString = new BTextControl("ReplaceWithString", "Replace:", "", nullptr);
	replaceCaseSensitiveCheckbox = new BCheckBox("ReplaceCaseSensitiveCheckbox", "Case sensitive", nullptr);
	replaceFullWordsCheckbox = new BCheckBox("ReplaceFullWordsCheckbox", "Full words", nullptr);

	// === Search/Replace Box ===
	BBox* searchReplaceBox = new BBox("SearchReplaceBox");
	searchReplaceBox->SetLabel("Search and replace");
	BGroupView* searchGroup = new BGroupView(B_VERTICAL, 5);
	searchReplaceBox->AddChild(searchGroup);

	BLayoutBuilder::Group<>(searchGroup)
		.Add(replaceSearchString)
		.Add(replaceWithString)
		.AddGroup(B_HORIZONTAL)
			.Add(replaceCaseSensitiveCheckbox)
			.Add(replaceFullWordsCheckbox)
		.End()
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(new BButton("SearchReplaceBtn", "Replace", new BMessage(M_TRANSFORM_REPLACE)))
		.End()
		.SetInsets(10, 12, 10, 10);

	// === Line Breaks Options Box ===
	BBox* lineBreaksBox = new BBox("LineBreaksBox");
	lineBreaksBox->SetLabel("Line break options");

	BGroupView* lineBreaksGroup = new BGroupView(B_VERTICAL, 10);
	lineBreaksBox->AddChild(lineBreaksGroup);

	// Input fields with labels hidden
	replaceLineBreaksInput->SetLabel(nullptr);
	lineBreakDelimiterInput->SetLabel(nullptr);
	maxWidthInput->SetLabel(nullptr);

	// Radio buttons
	BRadioButton* replaceMode = new BRadioButton("ReplaceMode", "Replace with:", nullptr);
	BRadioButton* breakOnMode = new BRadioButton("BreakOnMode", "Break on:", nullptr);
	BRadioButton* charactersMode = new BRadioButton("CharactersMode", "Characters:", nullptr);
	/*
	// Group them in one container to enforce exclusivity
	BGroupView* radioGroup = new BGroupView(B_VERTICAL, 5);

	// Add each radio row (radio button + field) as a horizontal group
	BGroupView* replaceRow = new BGroupView(B_HORIZONTAL, 5);
	replaceRow->AddChild(replaceMode);
	replaceRow->AddChild(replaceLineBreaksInput);

	BGroupView* breakRow = new BGroupView(B_HORIZONTAL, 5);
	breakRow->AddChild(breakOnMode);
	breakRow->AddChild(lineBreakDelimiterInput);

	BGroupView* charRow = new BGroupView(B_HORIZONTAL, 5);
	charRow->AddChild(charactersMode);
	charRow->AddChild(maxWidthInput);

	// Add rows to the radio group
	radioGroup->AddChild(replaceMode);
	radioGroup->AddChild(breakOnMode);
	radioGroup->AddChild(charactersMode);
	*/
	// Apply button
	BButton* applyBtn = new BButton("ApplyLineBreaksBtn", "Apply", new BMessage(M_INSERT_LINE_BREAKS));

	// Build final layout
	BLayoutBuilder::Group<>(lineBreaksGroup)
		.AddGroup(B_HORIZONTAL)
			.Add(replaceMode)
			.Add(breakOnMode)
			.Add(charactersMode)
		.End()
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(applyBtn)
		.End()
		.SetInsets(10, 12, 10, 10);

	// Set initial selection
	replaceMode->SetValue(B_CONTROL_ON);


	// === Cleanup Box ===
	BBox* cleanupBox = new BBox("CleanupBox");
	cleanupBox->SetLabel("Cleanup");
	BGroupView* cleanGroup = new BGroupView(B_VERTICAL, 5);
	cleanupBox->AddChild(cleanGroup);

	// Layout for Cleanup Box
	BButton* trimLinesBtn = new BButton("TrimLinesBtn", "Trim whitespace", new BMessage(M_TRIM_LINES));
	BButton* trimEmptyLinesBtn = new BButton("TrimEmptyLinesBtn", "Remove empty lines", new BMessage(M_TRIM_EMPTY_LINES));

	// Make both buttons expand to fill width
	trimLinesBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	trimEmptyLinesBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BLayoutBuilder::Group<>(cleanGroup)
		.Add(trimLinesBtn)
		.Add(trimEmptyLinesBtn)
		.SetInsets(10, 12, 10, 10);

	// === Line Operations Tab ===
	BLayoutBuilder::Group<>(lineOperationsView, B_VERTICAL, 10)
		.Add(searchReplaceBox)
		.Add(lineBreaksBox)
		.Add(cleanupBox)
		.AddGlue()
		.SetInsets(10, 12, 10, 10);

	BTab* lineOperationsTab = new BTab();
	AddTab(lineOperationsView, lineOperationsTab);
	lineOperationsTab->SetLabel("Lines");

	// === Prefix/Suffix Tab ===
	// Tab: Line operations
	BGroupView* prefixSuffixView = new BGroupView(B_VERTICAL, 5);

	// === Prefix/suffix Box ===
	BBox* prefixSuffixBox = new BBox("PrefixSuffixBox");
	prefixSuffixBox->SetLabel("Prefix/suffix each line");
	BGroupView* prefixSuffixGroup = new BGroupView(B_VERTICAL, 5);
	prefixSuffixBox->AddChild(prefixSuffixGroup);

	BLayoutBuilder::Group<>(prefixSuffixGroup)
		.Add(prefixInput)
		.Add(suffixInput)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(new BButton("prefixSuffixBtn", "Apply", new BMessage(M_TRANSFORM_PREFIX_SUFFIX)))
		.End()
		.SetInsets(10, 12, 10, 10);

	// === Prefix/suffix Tab ===
	BLayoutBuilder::Group<>(prefixSuffixView, B_VERTICAL, 10)
		.Add(prefixSuffixBox)
		.AddGlue()
		.SetInsets(10, 12, 10, 10);

	BTab* prefixSuffixTab = new BTab();
	AddTab(prefixSuffixView, prefixSuffixTab);
	prefixSuffixTab->SetLabel("Prefix/suffix");
}
