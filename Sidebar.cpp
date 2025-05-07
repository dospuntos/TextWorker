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

	// === Remove Line Breaks Box ===
	BBox* removeBreaksBox = new BBox("RemoveLineBreaksBox");
	removeBreaksBox->SetLabel("Remove line breaks");

	BGroupView* removeGroup = new BGroupView(B_VERTICAL, 5);
	removeBreaksBox->AddChild(removeGroup);

	BButton* removeBtn = new BButton("RemoveLineBreaksBtn", "Apply", new BMessage(M_REMOVE_LINE_BREAKS));

	BLayoutBuilder::Group<>(removeGroup)
		.Add(replaceLineBreaksInput)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(removeBtn)
		.End()
		.SetInsets(10, 12, 10, 10);

	// === Add Line Breaks Box ===
	BBox* addBreaksBox = new BBox("AddLineBreaksBox");
	addBreaksBox->SetLabel("Add line breaks");
	BGroupView* addGroup = new BGroupView(B_VERTICAL, 5);
	addBreaksBox->AddChild(addGroup);

	// Layout for Add Line Breaks Box
	BLayoutBuilder::Group<>(addGroup)
		.AddGroup(B_HORIZONTAL)
			.Add(maxWidthInput)
			.Add(new BButton("AddLineBreaksBtn", "Apply", new BMessage(M_INSERT_LINE_BREAKS)))
		.End()
		.Add(splitOnWordsCheckbox)
		.AddGroup(B_HORIZONTAL)
			.Add(lineBreakDelimiterInput)
			.Add(new BButton("BreakOnDelimiterBtn", "Apply", new BMessage(M_BREAK_LINES_ON_DELIMITER)))
		.End()
		.SetInsets(10, 12, 10, 10);

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
		.Add(removeBreaksBox)
		.Add(addBreaksBox)
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
