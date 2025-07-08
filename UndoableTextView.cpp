/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "UndoableTextView.h"
#include "Constants.h"
#include <Application.h>
#include <Message.h>
#include <Messenger.h>
#include <Window.h>

enum { M_COALESCE_TIMEOUT = '_cut' };


UndoableTextView::UndoableTextView(const char* name)
	:
	BTextView(name, B_WILL_DRAW | B_SCROLL_VIEW_AWARE),
	fCoalescing(false),
	fRecording(true)
{
	rgb_color viewColor = ui_color(B_DOCUMENT_BACKGROUND_COLOR);
	rgb_color textColor = ui_color(B_DOCUMENT_TEXT_COLOR);

	SetViewColor(viewColor);
	SetLowColor(viewColor);
	SetHighColor(textColor);
	SetFontAndColor(nullptr, B_FONT_ALL, &textColor);
}


UndoableTextView::~UndoableTextView()
{
}


void
UndoableTextView::InsertText(const char* text, int32 length, int32 offset,
	const text_run_array* runs)
{
	if (fRecording && !fCoalescing)
		PushUndoSnapshot();

	StartCoalesceTimer();
	BTextView::InsertText(text, length, offset, runs);
}


void
UndoableTextView::DeleteText(int32 start, int32 finish)
{
	if (fRecording && !fCoalescing)
		PushUndoSnapshot();

	StartCoalesceTimer();
	BTextView::DeleteText(start, finish);
}


void
UndoableTextView::MouseDown(BPoint point)
{
	BTextView::MouseDown(point);
	Window()->PostMessage(M_UPDATE_STATUSBAR);
}


void
UndoableTextView::SetTextWithUndo(const BString& newText)
{
	fRecording = false;
	PushUndoSnapshot();
	SetText(newText.String());
	Select(0, 0);
	StopCoalesceTimer();
	fRecording = true;
}


void
UndoableTextView::Undo()
{
	if (fUndoStack.empty())
		return;

	TextSnapshot current;
	current.text = Text();
	GetSelection(&current.selectionStart, &current.selectionEnd);
	fRedoStack.push(current);

	const TextSnapshot& snapshot = fUndoStack.top();
	fRecording = false;
	SetText(snapshot.text.String());
	Select(snapshot.selectionStart, snapshot.selectionEnd);
	fUndoStack.pop();
	fRecording = true;
}


void
UndoableTextView::Redo()
{
	if (fRedoStack.empty())
		return;

	TextSnapshot current;
	current.text = Text();
	GetSelection(&current.selectionStart, &current.selectionEnd);
	fUndoStack.push(current);

	const TextSnapshot& snapshot = fRedoStack.top();
	fRecording = false;
	SetText(snapshot.text.String());
	Select(snapshot.selectionStart, snapshot.selectionEnd);
	fRedoStack.pop();
	fRecording = true;
}


void
UndoableTextView::PushUndoSnapshot()
{
	TextSnapshot snapshot;
	snapshot.text = Text();

	int32 selStart, selEnd;
	GetSelection(&selStart, &selEnd);
	snapshot.selectionStart = selStart;
	snapshot.selectionEnd = selEnd;

	// Avoid duplicate snapshots
	if (!fUndoStack.empty() && fUndoStack.top().text == snapshot.text)
		return;

	fUndoStack.push(snapshot);

	// Trim stack
	if (fUndoStack.size() > kMaxUndoSteps)
		fUndoStack.pop();

	while (!fRedoStack.empty())
		fRedoStack.pop();
}


void
UndoableTextView::StartCoalesceTimer()
{
	StopCoalesceTimer();
	fCoalescing = true;

	BMessage timeoutMsg(M_COALESCE_TIMEOUT);
	BMessageRunner::StartSending(this, &timeoutMsg, kCoalesceDelay, 1);
}


void
UndoableTextView::StopCoalesceTimer()
{
	fCoalescing = false;
}


void
UndoableTextView::KeyDown(const char* bytes, int32 numBytes)
{
	BTextView::KeyDown(bytes, numBytes);
	Window()->PostMessage(M_UPDATE_STATUSBAR);
}


void
UndoableTextView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case M_COALESCE_TIMEOUT:
			StopCoalesceTimer();
			break;
		case B_COLORS_UPDATED:
			SetColorsFromTheme();
			break;
		default:
			BTextView::MessageReceived(msg);
			break;
	}
}


void
UndoableTextView::AllAttached()
{
	BTextView::AllAttached();
	SetColorsFromTheme();
}


void
UndoableTextView::SetColorsFromTheme()
{
	rgb_color viewColor = ui_color(B_DOCUMENT_BACKGROUND_COLOR);
	rgb_color textColor = ui_color(B_DOCUMENT_TEXT_COLOR);

	SetViewColor(viewColor);
	SetLowColor(viewColor);
	SetHighColor(textColor);
	SetFontAndColor(nullptr, B_FONT_ALL, &textColor);
}
