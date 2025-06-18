/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef COMMAND_H
#define COMMAND_H


#include <string>
#include <SupportDefs.h>
#include <TextView.h>
#include <memory>

class TextBuffer;

class Command {

public:
	virtual ~Command() {}
	virtual void Execute(BTextView* view) = 0;
	virtual void Undo(BTextView* view) = 0;

private:
	std::string fOldText;
	std::string fNewText;
};

#endif // COMMAND_H
