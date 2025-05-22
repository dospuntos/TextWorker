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
	replaceLineBreaksInput = new BTextControl("ReplaceLineBreaksInput", nullptr, "", nullptr);
	prefixInput = new BTextControl("PrefixInput", "Prefix:", "", nullptr);
	suffixInput = new BTextControl("SuffixInput", "Suffix:", "", nullptr);
	maxWidthInput = new BTextControl("MaxWidthInput", nullptr, "", nullptr);
	splitOnWordsCheckbox = new BCheckBox("SplitOnWordsCheckbox", "Split on words", nullptr);
	lineBreakDelimiterInput = new BTextControl("LineBreakDelimiter", nullptr, "", nullptr);

	replaceSearchString = new BTextControl("ReplaceSearchString", nullptr, "", nullptr);
	replaceWithString = new BTextControl("ReplaceWithString", nullptr, "", nullptr);
	replaceCaseSensitiveCheckbox = new BCheckBox("ReplaceCaseSensitiveCheckbox", "Case sensitive", nullptr);
	replaceFullWordsCheckbox = new BCheckBox("ReplaceFullWordsCheckbox", "Full words", nullptr);

	float maxLabelWidth = 0;

	// List of BTextControls
	BTextControl* fields[] = {
		replaceLineBreaksInput, prefixInput, suffixInput, maxWidthInput, lineBreakDelimiterInput,
		replaceSearchString, replaceWithString,
		nullptr
	};

	// Step 1–2: Find max label width
	for (int i = 0; fields[i]; ++i) {
		float width = be_plain_font->StringWidth(fields[i]->Label());
		maxLabelWidth = MAX(width, maxLabelWidth);
	}

	maxLabelWidth += 20;

	// === Search/Replace Box ===
	BBox* searchReplaceBox = new BBox("SearchReplaceBox");
	searchReplaceBox->SetLabel("Search and replace");

	// Controls
	BButton* searchReplaceBtn
		= new BButton("SearchReplaceBtn", "Replace", new BMessage(M_TRANSFORM_REPLACE));
	searchReplaceBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	// Set consistent minimum widths for text fields
	replaceSearchString->SetExplicitMinSize(BSize(150, B_SIZE_UNSET));
	replaceWithString->SetExplicitMinSize(BSize(150, B_SIZE_UNSET));

	// Grid layout
	BGridLayoutBuilder grid1(B_USE_SMALL_SPACING, B_USE_SMALL_SPACING);
	// clang-format off
	grid1.Add(new BStringView(NULL, "Find:"),         0, 0)
		.Add(replaceSearchString,                     1, 0)

		.Add(new BStringView(NULL, "Replace with:"),  0, 1)
		.Add(replaceWithString,                       1, 1)

		.Add(replaceCaseSensitiveCheckbox,            0, 2)
		.Add(replaceFullWordsCheckbox,                1, 2)

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

	// Views and controls
	replaceRadio = new BRadioButton("ReplaceRadio", "Replace with:", new BMessage(M_MODE_REPLACE));
	breakRadio = new BRadioButton("BreakRadio", "Break on:", new BMessage(M_MODE_BREAK));
	charRadio = new BRadioButton("CharRadio", "Characters:", new BMessage(M_MODE_CHARACTERS));
	replaceRadio->SetValue(B_CONTROL_ON);
	BCheckBox* splitWords = new BCheckBox("SplitWords", "Split on words", NULL);

	replaceLineBreaksInput->SetExplicitMinSize(BSize(150, B_SIZE_UNSET));
	lineBreakDelimiterInput->SetExplicitMinSize(BSize(150, B_SIZE_UNSET));
	maxWidthInput->SetExplicitMinSize(BSize(150, B_SIZE_UNSET));
	BButton* applyBtn = new BButton("ApplyBtn", "Apply", new BMessage(M_REMOVE_LINE_BREAKS));
	applyBtn->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	// Layout grid
	BGridLayoutBuilder grid(B_USE_SMALL_SPACING, B_USE_SMALL_SPACING);
	// clang-format off
	grid.Add(replaceRadio,              0, 0)
		.Add(replaceLineBreaksInput,    1, 0)

		.Add(breakRadio,                0, 1)
		.Add(lineBreakDelimiterInput,   1, 1)

		.Add(charRadio,                 0, 2)
		.Add(maxWidthInput,             1, 2)
		.Add(splitWords,                1, 3)

		.Add(applyBtn,                  0, 4, 2);
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

	// === Prefix/suffix Box ===
	BBox* prefixSuffixBox = new BBox("PrefixSuffixBox");
	prefixSuffixBox->SetLabel("Prefix/suffix each line");
	BGroupView* prefixSuffixGroup = new BGroupView(B_VERTICAL, 5);
	prefixSuffixBox->AddChild(prefixSuffixGroup);

	// clang-format off
	BLayoutBuilder::Group<>(prefixSuffixGroup)
		.Add(prefixInput)
		.Add(suffixInput)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(new BButton("prefixSuffixBtn", "Apply", new BMessage(M_TRANSFORM_PREFIX_SUFFIX)))
		.End()
		.SetInsets(10, 12, 10, 10);
	// clang-format on
	// === Prefix/suffix Tab ===
	BLayoutBuilder::Group<>(prefixSuffixView, B_VERTICAL, 10)
		.Add(prefixSuffixBox)
		.AddGlue()
		.SetInsets(10, 12, 10, 10);

	BTab* prefixSuffixTab = new BTab();
	AddTab(prefixSuffixView, prefixSuffixTab);
	prefixSuffixTab->SetLabel("Prefix/suffix");
}


void
Sidebar::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case M_MODE_REPLACE:
		case M_MODE_BREAK:
			splitOnWordsCheckbox->Hide();
			break;
		case M_MODE_CHARACTERS:
			splitOnWordsCheckbox->Show();
			break;
		default:
			BView::MessageReceived(msg);
			break;
	}
}
