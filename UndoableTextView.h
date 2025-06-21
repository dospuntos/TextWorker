/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef UNDOABLE_TEXT_VIEW_H
#define UNDOABLE_TEXT_VIEW_H

#include <MessageRunner.h>
#include <TextView.h>
#include <stack>

struct TextSnapshot {
	BString text;
	int32 selectionStart;
	int32 selectionEnd;
};

class UndoableTextView : public BTextView {
public:
	UndoableTextView(const char* name);
	~UndoableTextView();
	void MessageReceived(BMessage* msg) override;

	void InsertText(const char* text, int32 length, int32 offset,
		const text_run_array* runs) override;
	void DeleteText(int32 start, int32 finish) override;
	void KeyDown(const char* bytes, int32 numBytes) override;
	void MouseDown(BPoint point) override;

	void AllAttached() override;
	void SetColorsFromTheme();

	void SetTextWithUndo(const BString& newText);
	void Undo();
	void Redo();

	bool CanUndo() const { return !fUndoStack.empty(); }
	bool CanRedo() const { return !fRedoStack.empty(); }

private:
	void PushUndoSnapshot();
	void StartCoalesceTimer();
	void StopCoalesceTimer();

	static const bigtime_t kCoalesceDelay = 1500000; // 1.5 seconds
	static const size_t kMaxUndoSteps = 100;

	std::stack<TextSnapshot> fUndoStack;
	std::stack<TextSnapshot> fRedoStack;

	bool fCoalescing;
	bool fRecording;

	BMessageRunner* fTimer;
};


#endif // UNDOABLE_TEXT_VIEW_H
