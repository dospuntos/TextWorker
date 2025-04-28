/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "TextUtils.h"
#include <String.h>
#include <TextControl.h>
#include <cctype>

int32 startSelection, endSelection; // For cursor position

BString
GetTextFromTextView(BTextView* textView)
{
	// Check if invalid or empty
	if (textView == nullptr) return BString("");
	int32 textLength = textView->TextLength();
	if (textLength == 0) return BString("");

	// Get text from BTextView
	char* buffer = new char[textLength + 1];
	textView->GetText(0, textView->TextLength(), buffer);

	BString text(buffer);

	delete[] buffer;

	SaveCursorPosition(textView);
	return text;
}


void
SaveCursorPosition(BTextView* textView)
{
	textView->GetSelection(&startSelection, &endSelection);
}


void
RestoreCursorPosition(BTextView* textView)
{
	textView->Select(startSelection, endSelection);
}


void
ConvertToUppercase(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty()) return; // Nothing to convert

	text.ToUpper();
	textView->SetText(text.String());

	textView->Select(startSelection, endSelection);
}


void
ConvertToLowercase(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty()) return; // Nothing to convert

	text.ToLower();
	textView->SetText(text.String());
	RestoreCursorPosition(textView);
}


void
ConvertToTitlecase(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty()) return; // Nothing to convert

	text.CapitalizeEachWord();
	textView->SetText(text.String());
	RestoreCursorPosition(textView);
}


void
Capitalize(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty()) return; // Nothing to convert

	bool capitalizeNext = true; // Keep track whether to capitalize next character

	for (int32 i = 0; i < text.Length(); ++i) {
		char currentChar = text.ByteAt(i);

		if (capitalizeNext && std::isalpha(currentChar)) {
			currentChar = std::toupper(currentChar);
			text.SetByteAt(i, currentChar);
			capitalizeNext = false;
		} else if (currentChar == '.' || currentChar == '!' || currentChar == '?') {
			capitalizeNext = true;
		}
	}
	textView->SetText(text.String());
	RestoreCursorPosition(textView);
}


void
ConvertToRandomCase(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty()) return;

	srand(time(nullptr)); // Seed random number generator

	for (int32 i = 0; i < text.Length(); ++i) {
		char currentChar = text.ByteAt(i);
		if (std::isalpha(currentChar)) {
			if (rand() % 2 == 0)
				currentChar = std::toupper(currentChar);
			else
				currentChar = std::tolower(currentChar);

			text.SetByteAt(i, currentChar);
		}

	}

	textView->SetText(text.String());
	RestoreCursorPosition(textView);
}


void
ConvertToAlternatingCase(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty()) return;

	bool uppercase = !(std::isupper(text.ByteAt(0)));
	for (int32 i = 0; i < text.Length(); ++i) {
		char currentChar = text.ByteAt(i);
		if (std::isalpha(currentChar)) {
			if (uppercase)
				currentChar = std::toupper(currentChar);
			else
				currentChar = std::tolower(currentChar);

			uppercase = !uppercase;
			text.SetByteAt(i, currentChar);
		}
	}

	textView->SetText(text.String());
	RestoreCursorPosition(textView);
}


void
RemoveLineBreaks(BTextView* textView, BString replacement)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty()) return;

	text.ReplaceAll("\n", replacement); // Replace line breaks ('\n') with value

	textView->SetText(text.String());
	RestoreCursorPosition(textView);
}


// Note: The ROT-13 algorithm is symmetrical, the same function will encode and decode the text.
void
ConvertToROT13(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty()) return;

	for (int32 i = 0; i < text.Length(); ++i) {
		char currentChar = text.ByteAt(i);

		if (std::isalpha(currentChar)) {
			if (std::islower(currentChar)) {
				currentChar = 'a' + (currentChar - 'a' + 13) % 26;
			} else {
				currentChar = 'A' + (currentChar - 'A' + 13) % 26;
			}
		}
		text.SetByteAt(i, currentChar);
	}

	textView->SetText(text.String());
	RestoreCursorPosition(textView);
}


void
AddStringsToEachLine(BTextView* textView, const BString& startString, const BString&endString)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty()) return;

	BString updatedText;

	int32 start = 0;
	int32 end;

	// Process each line
	while ((end = text.FindFirst('\n', start)) >= 0) {
		BString line(text.String() + start, end - start);
		updatedText << startString << line << endString << '\n';
		start = end + 1; // Move past the line break
	}

	// Last line (if it doesn't end with '\n')
	if (start < text.Length()) {
		BString line(text.String() + start, text.Length() - start);
		updatedText << startString << line << endString;
	}

	textView->SetText(updatedText.String());
	RestoreCursorPosition(textView);
}



void
InsertLineBreaks(BTextView* textView, int32 maxLength, bool KeepWordsIntact)
{
	BString text(GetTextFromTextView(textView));
	if (text.IsEmpty()) return;

	BString updatedText;

	int32 start = 0;
	while (start < text.Length()) {
		int32 segmentEnd = start + maxLength;

		if (segmentEnd >= text.Length())  {
			updatedText.Append(text.String() + start, text.Length() - start);
			break;
		}

		if (KeepWordsIntact) {
			int32 nearestSpace = text.FindLast(' ', segmentEnd);
			if (nearestSpace >= start) {
				segmentEnd = nearestSpace;
			}
		}

		// Append the segment and add line break
		updatedText.Append(text.String() + start, segmentEnd - start);
		updatedText.Append("\n");

		if (!KeepWordsIntact && segmentEnd == start + maxLength) {
			start = segmentEnd;
		} else {
			// Move past the space or to the next segment
			start = segmentEnd + 1;
		}
	}

	textView->SetText(updatedText.String());
	RestoreCursorPosition(textView);
}


BString
ProcessLineWithBreaks(const BString& line, int32 maxLength, bool KeepWordsIntact)
{
	BString processedLine;
	int32 start = 0;
	int32 currentPos;

	while (start < line.Length()) {
		if ((start + maxLength) >= line.Length()) {
			processedLine.Append(line.String() + start, line.Length() - start);
			break;
		}

	if (KeepWordsIntact) {
		currentPos = line.FindLast(' ', start + maxLength);
		if (currentPos < start) {
			currentPos = start + maxLength;
		}
	} else {
		currentPos = start + maxLength;
	}

	processedLine.Append(line.String() + start, currentPos - start);
	processedLine << '\n';
	start = currentPos + (KeepWordsIntact ? 1 : 0);
	}
	return processedLine;
}


void
BreakLinesOnDelimiter(BTextView* textView, const BString& delimiter)
{
	BString text(GetTextFromTextView(textView));
	if (text.IsEmpty()) return;

	BString updatedText;

	int32 start = 0;
	int32 delimiterPosition;

	while ((delimiterPosition = text.FindFirst(delimiter, start)) >= 0) {
		updatedText.Append(text.String() + start, delimiterPosition - start + delimiter.Length());
		updatedText.Append("\n");
		start = delimiterPosition + delimiter.Length();
	}

	if (start < text.Length()) {
		updatedText.Append(text.String() + start, text.Length() - start);
	}

	textView->SetText(updatedText.String());
	RestoreCursorPosition(textView);
}


void
TrimLines(BTextView* textView)
{
	BString text(GetTextFromTextView(textView));
	if (text.IsEmpty()) return;

	BString updatedText;
	int32 start = 0;
	int32 end;

	while ((end = text.FindFirst('\n', start)) >= 0) {
		BString line(text.String() + start, end - start);
		line.Trim();
		updatedText << line << "\n";
		start = end + 1;
	}

	if (start < text.Length()) {
		BString line(text.String() + start, text.Length() - start);
		line.Trim();
		updatedText << line << "\n";
	}

	textView->SetText(updatedText.String());
	RestoreCursorPosition(textView);
}


void
TrimEmptyLines(BTextView* textView)
{
	BString text(GetTextFromTextView(textView));
	if (text.IsEmpty()) return;

	int32 start = 0;
	int32 end;
	BString updatedText;

	while ((end = text.FindFirst('\n', start)) >= 0) {
		BString line(text.String() + start, end - start);

		if (line.Length() > 0)
			updatedText << line << '\n';

		start = end + 1;
	}

	// Handle last line if no '\n'
	if (start < text.Length()) {
		BString lastLine(text.String() + start, text.Length() - start);
		if (lastLine.Length() > 0)
			updatedText.Append(lastLine);
	}

	textView->SetText(updatedText.String());
	RestoreCursorPosition(textView);
}