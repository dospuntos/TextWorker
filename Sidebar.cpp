/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "Sidebar.h"
#include "MainWindow.h"
#include <TabView.h>
#include <GroupView.h>
#include <StringView.h>
#include <LayoutBuilder.h>
#include <TextControl.h>
#include <Button.h>

Sidebar::Sidebar()
	: BTabView("Sidebar")
{
	// Tab: Line operations
	BGroupView* lineOperationsView = new BGroupView(B_VERTICAL, 5);

	replaceLineBreaksInput = new BTextControl("ReplaceLineBreaksInput", "Replacement:", "", nullptr);
	prependInput = new BTextControl("PrependInput", "Prepend:", "", nullptr);
	appendInput = new BTextControl("AppendInput", "Append:", "", nullptr);
	maxWidthInput = new BTextControl("MaxWidth", "Characters:", "", nullptr);
	splitOnWordsCheckbox = new BCheckBox("SplitOnWordsCheckbox", "Split on words", nullptr);
	lineBreakDelimiterInput = new BTextControl("LineBreakDelimiter", "Break on:", "", nullptr);

	// Add settings
	BLayoutBuilder::Group<>(lineOperationsView, B_VERTICAL, 5)
		.Add(new BStringView("RemoveLineBreaks", "Remove Line Breaks"))
		.Add(replaceLineBreaksInput)
		.Add(new BButton("RemoveLineBreaksButton", "Apply", new BMessage(M_REMOVE_LINE_BREAKS)))
		.Add(new BStringView("AddLineBreaksLabel", "Add Line Breaks After X Characters"))
		.Add(maxWidthInput)
		.Add(splitOnWordsCheckbox)
		.Add(new BButton("AddLineBreaksButton", "Apply", new BMessage(M_INSERT_LINE_BREAKS)))
		.Add(new BStringView("BreakOnDelimiter", "Break on delimiter:"))
		.Add(lineBreakDelimiterInput)
		.Add(new BButton("BreakOnDelimiterButton", "Apply", new BMessage(M_BREAK_LINES_ON_DELIMITER)))
		.Add(new BButton("TrimLinesButton", "Trim", new BMessage(M_TRIM_LINES)))
		.Add(new BButton("TrimEmptyLinesButton", "Remove empty lines", new BMessage(M_TRIM_EMPTY_LINES)))
		.AddGlue()
		.SetInsets(10, 10, 10, 10);

	// Add line operations tab
	BTab* lineOperationsTab = new BTab();
	AddTab(lineOperationsView, lineOperationsTab);
	lineOperationsTab->SetLabel("Lines");

	// Tab: Append/prepend
	BGroupView* appendPrependView = new BGroupView(B_VERTICAL, 5);

	BLayoutBuilder::Group<>(appendPrependView, B_VERTICAL, 5)
		.Add(new BStringView("PrependLabel", "Prepend/append to each line"))
		.Add(prependInput)
		.Add(appendInput)
		.Add(new BButton("appendPrependButton", "Apply", new BMessage(M_TRANSFORM_PREPEND_APPEND)))
		.AddGlue()
		.SetInsets(10, 10, 10, 10);

	// Add line operations tab
	BTab* appendPrependTab = new BTab();
	AddTab(appendPrependView, appendPrependTab);
	appendPrependTab->SetLabel("Add");
}
