/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "TextUtils.h"
#include <String.h>
#include <cctype>

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

	return text;
}

void
ConvertToUppercase(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);

	if (text.IsEmpty()) return; // Nothing to convert

	text.ToUpper();
	textView->SetText(text.String());
}


void
ConvertToLowercase(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);

	if (text.IsEmpty()) return; // Nothing to convert

	text.ToLower();
	textView->SetText(text.String());
}


void
ConvertToTitlecase(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);

	if (text.IsEmpty()) return; // Nothing to convert

	text.CapitalizeEachWord();
	textView->SetText(text.String());
}


void
Capitalize(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);

	if (text.IsEmpty()) return; // Nothing to convert

	text.Capitalize();
	textView->SetText(text.String());
}

/*
void
ConvertToRandomCase(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);

	if (text.IsEmpty()) return;

	for (int32 i = 0; i < text.Length(); ++i) {
		char currentChar = text[i];
		bool uppercase = rand() % 2;
		if (std::isalpha(currentChar)) {
			if (uppercase) {
				text[i] = std::toupper(currentChar);
			}
			else text[i] = std::tolower(currentChar);
		}
	}

	textView->SetText(text.String());
} */


void
ConvertToAlternatingCase(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);

	if (text.IsEmpty()) return;

	bool uppercase = true;
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
}


void
RemoveLineBreaks(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty()) return;

	text.ReplaceAll("\n", " "); // Replace line breaks ('\n') with space

	textView->SetText(text.String());
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
}