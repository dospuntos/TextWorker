/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "CommandManager.h"

void CommandManager::ExecuteCommand(std::unique_ptr<Command> command, BTextView* view) {
	command->Execute(view);
	fUndoStack.push(std::move(command));
	while (!fRedoStack.empty()) fRedoStack.pop(); //Clear redo stack on new action
}


void CommandManager::Undo(BTextView* view) {
	if (!CanUndo()) return;
	auto command = std::move(fUndoStack.top());
	fUndoStack.pop();
	command->Undo(view);
	fRedoStack.push(std::move(command));
}


void CommandManager::Redo(BTextView* view) {
	if (!CanRedo()) return;
	auto command = std::move(fRedoStack.top());
	fRedoStack.pop();
	command->Execute(view);
	fUndoStack.push(std::move(command));
}


bool CommandManager::CanUndo() const {
	return !fUndoStack.empty();
}


bool CommandManager::CanRedo() const {
	return !fRedoStack.empty();
}