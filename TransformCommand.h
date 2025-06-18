/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TRANSFORM_COMMAND_H
#define TRANSFORM_COMMAND_H

#include "Command.h"
#include "TextUtils.h"
#include <SupportDefs.h>
#include <functional>

class TransformCommand : public Command {
public:
	TransformCommand(std::function<void()> func, const BString& snapshot = "")
		: fFunc(func), fOldText(snapshot) {}

	void Execute(BTextView* view) override {
		if (!view) return;
		if (fOldText.IsEmpty())
			fOldText = view->Text();
		fFunc();
	}

	void Undo(BTextView* view) override {
		if (!view) return;
		view->SetText(fOldText);
		RestoreCursorPosition(view);
	}

private:
	std::function<void()> fFunc;
	BString fOldText;
};

#endif // TRANSFORM_COMMAND_H
