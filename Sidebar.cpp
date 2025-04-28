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

	// Add settings
	BLayoutBuilder::Group<>(lineOperationsView, B_VERTICAL, 5)
		.Add(new BStringView("RemoveLineBreaks", "Remove Line Breaks"))
		.Add(new BTextControl("ReplaceLineBreaksInput", "Replacement:", "", nullptr))
		.Add(new BButton("RemoveLineBreaksButton", "Apply", new BMessage(M_REMOVE_LINE_BREAKS)))
		.Add(new BStringView("AddLineBreaksLabel", "Add Line Breaks After X Characters"))
		.Add(new BTextControl("AddLineBreaksInput", "Characters:", "", nullptr))
		.Add(new BButton("AddLineBreaksButton", "Apply", new BMessage(M_TRANSFORM_WIP)))
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
		.Add(new BTextControl("PrependInput", "Prepend:", "", nullptr))
		.Add(new BTextControl("AppendInput", "Append:", "", nullptr))
		.Add(new BButton("appendPrependButton", "Apply", new BMessage(M_TRANSFORM_WIP)))
		.AddGlue()
		.SetInsets(10, 10, 10, 10);

	// Add line operations tab
	BTab* appendPrependTab = new BTab();
	AddTab(appendPrependView, appendPrependTab);
	appendPrependTab->SetLabel("Add");
}

