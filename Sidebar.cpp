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
	prefixInput = new BTextControl("PrependInput", "Prepend:", "", nullptr);
	suffixInput = new BTextControl("AppendInput", "Append:", "", nullptr);
	maxWidthInput = new BTextControl("MaxWidth", "Characters:", "", nullptr);
	splitOnWordsCheckbox = new BCheckBox("SplitOnWordsCheckbox", "Split on words", nullptr);
	lineBreakDelimiterInput = new BTextControl("LineBreakDelimiter", "Break on:", "", nullptr);

	replaceSearchString = new BTextControl("ReplaceSearchString", "Find", "", nullptr);
	replaceWithString = new BTextControl("ReplaceWithString", "Replace", "", nullptr);
	replaceCaseSensitiveCheckbox = new BCheckBox("ReplaceCaseSensitiveCheckbox", "Case sensitive", nullptr);
	replaceFullWordsCheckbox = new BCheckBox("ReplaceFullWordsCheckbox", "Full words", nullptr);

	// === Remove Line Breaks Box ===
	BBox* removeBreaksBox = new BBox("RemoveLineBreaksBox");
	removeBreaksBox->SetLabel("Remove line breaks");
	BGroupView* removeGroup = new BGroupView(B_HORIZONTAL, 5);
	removeBreaksBox->AddChild(removeGroup);

	BLayoutBuilder::Group<>(removeGroup)
		.Add(replaceLineBreaksInput)
		.Add(new BButton("RemoveLineBreaksBtn", "Apply", new BMessage(M_REMOVE_LINE_BREAKS)))
		.SetInsets(10, 12, 10, 10);

	// === Add Line Breaks Box ===
	BBox* addBreaksBox = new BBox("AddLineBreaksBox");
	addBreaksBox->SetLabel("Add line breaks");
	BGroupView* addGroup = new BGroupView(B_VERTICAL, 5);
	addBreaksBox->AddChild(addGroup);

	// Layout for Add Line Breaks Box
	BLayoutBuilder::Group<>(addGroup)
		.AddGroup(B_HORIZONTAL) // maxWidthInput and Apply button on the same line
			.Add(maxWidthInput)
			.Add(new BButton("AddLineBreaksBtn", "Apply", new BMessage(M_INSERT_LINE_BREAKS)))
		.End()
		.Add(splitOnWordsCheckbox) // Split on words checkbox on a separate line
		.AddGroup(B_HORIZONTAL) // lineBreakDelimiterInput and Apply button on the same line
			.Add(lineBreakDelimiterInput)
			.Add(new BButton("BreakOnDelimiterBtn", "Apply", new BMessage(M_BREAK_LINES_ON_DELIMITER)))
		.End()
		.Add(new BButton("TrimLinesBtn", "Trim whitespace", new BMessage(M_TRIM_LINES)))
		.Add(new BButton("TrimEmptyLinesBtn", "Remove empty lines", new BMessage(M_TRIM_EMPTY_LINES)))
		.SetInsets(10, 12, 10, 10);

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
		.Add(new BButton("SearchReplaceBtn", "Replace", new BMessage(M_TRANSFORM_REPLACE)))
		.SetInsets(10, 12, 10, 10);

	// === Line Operations Tab ===
	BLayoutBuilder::Group<>(lineOperationsView, B_VERTICAL, 10)
		.Add(removeBreaksBox)
		.Add(addBreaksBox)
		.Add(searchReplaceBox)
		.AddGlue()
		.SetInsets(10, 10, 10, 10);

	BTab* lineOperationsTab = new BTab();
	AddTab(lineOperationsView, lineOperationsTab);
	lineOperationsTab->SetLabel("Lines");

	// === Prefix/Suffix Tab ===
	BGroupView* prefixSuffixView = new BGroupView(B_VERTICAL, 5);
	BLayoutBuilder::Group<>(prefixSuffixView, B_VERTICAL, 5)
		.Add(new BStringView("PrependLabel", "Prefix/suffix each line"))
		.Add(prefixInput)
		.Add(suffixInput)
		.Add(new BButton("prefixSuffixBtn", "Apply", new BMessage(M_TRANSFORM_PREFIX_SUFFIX)))
		.AddGlue()
		.SetInsets(10, 10, 10, 10);

	BTab* prefixSuffixTab = new BTab();
	AddTab(prefixSuffixView, prefixSuffixTab);
	prefixSuffixTab->SetLabel("Prefix/suffix");
}
