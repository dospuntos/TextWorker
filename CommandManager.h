/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H

#include "Command.h"
#include <stack>

class CommandManager {
public:
	void ExecuteCommand(std::unique_ptr<Command> command, BTextView* view) {
		if (!view) return;
		command->Execute(view);
		fUndoStack.push(std::move(command));
		while (!fRedoStack.empty()) fRedoStack.pop(); // Clear redo stack
	}
	void Undo(BTextView* view) {
		if (!CanUndo() || !view) return;
		auto command = std::move(fUndoStack.top());
		fUndoStack.pop();
		command->Undo(view);
		fRedoStack.push(std::move(command));
	}
	void Redo(BTextView* view) {
		if (!CanRedo() || !view) return;
		auto command = std::move(fRedoStack.top());
		fRedoStack.pop();
		command->Execute(view);
		fUndoStack.push(std::move(command));
	}

	bool CanUndo() const { return !fUndoStack.empty(); }
	bool CanRedo() const { return !fRedoStack.empty(); }

private:
	std::stack<std::unique_ptr<Command>> fUndoStack;
	std::stack<std::unique_ptr<Command>> fRedoStack;
};


#endif // COMMAND_MANAGER_H
