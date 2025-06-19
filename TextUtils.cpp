/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "TextUtils.h"
#include "UndoableTextView.h"
#include <Alert.h>
#include <File.h>
#include <LayoutBuilder.h>
#include <String.h>
#include <TextControl.h>
#include <algorithm>
#include <cctype>
#include <set>
#include <sstream>
#include <unicode/brkiter.h>
#include <unicode/coll.h>
#include <unicode/locid.h>
#include <unicode/unistr.h>
#include <vector>

int32 startSelection, endSelection; // For cursor position


BString
GetTextFromTextView(BTextView* textView)
{
	// Check if invalid or empty
	if (textView == nullptr)
		return BString("");
	int32 textLength = textView->TextLength();
	if (textLength == 0)
		return BString("");

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
	if (text.IsEmpty())
		return; // Nothing to convert

	icu::UnicodeString unicodeText = icu::UnicodeString::fromUTF8(text.String());
	unicodeText.toUpper();

	// Convert back to UTF-8
	std::string utf8Text;
	unicodeText.toUTF8String(utf8Text);

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(utf8Text.c_str());
	RestoreCursorPosition(textView);
}


void
ConvertToLowercase(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty())
		return; // Nothing to convert

	icu::UnicodeString unicodeText = icu::UnicodeString::fromUTF8(text.String());
	unicodeText.toLower();

	// Convert back to UTF-8
	std::string utf8Text;
	unicodeText.toUTF8String(utf8Text);

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(utf8Text.c_str());
	RestoreCursorPosition(textView);
}


void
ConvertToTitlecase(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty())
		return;

	icu::UnicodeString unicodeText = icu::UnicodeString::fromUTF8(text.String());
	unicodeText.toLower(); // normalize first

	bool capitalizeNext = true;

	for (int32_t i = 0; i < unicodeText.length(); ++i) {
		UChar32 c = unicodeText.char32At(i);

		if (u_isUWhiteSpace(c) || u_ispunct(c)) {
			capitalizeNext = true;
			continue;
		}

		if (capitalizeNext) {
			UChar32 upperC = u_toupper(c);
			unicodeText.replace(i, U16_LENGTH(c), upperC);
			capitalizeNext = false;
		}
	}

	std::string result;
	unicodeText.toUTF8String(result);

	if (result.empty()) {
		printf("Manual titlecase result is empty!\n");
		return;
	}

	BString bResult(result.c_str());
	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(bResult.String());
	RestoreCursorPosition(textView);
}


void
Capitalize(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty())
		return; // Nothing to convert

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
	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(text.String());
	RestoreCursorPosition(textView);
}


void
ConvertToRandomCase(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty())
		return;

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

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(text.String());
	RestoreCursorPosition(textView);
}


void
ConvertToAlternatingCase(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty())
		return;

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

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(text.String());
	RestoreCursorPosition(textView);
}


void
ToggleCase(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty())
		return;

	for (int32 i = 0; i < text.Length(); ++i) {
		char currentChar = text.ByteAt(i);
		if (std::isupper(currentChar))
			currentChar = std::tolower(currentChar);
		else if (std::islower(currentChar))
			currentChar = std::toupper(currentChar);
		text.SetByteAt(i, currentChar);
	}

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(text.String());
	RestoreCursorPosition(textView);
}


void
RemoveLineBreaks(BTextView* textView, BString replacement)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty())
		return;

	text.ReplaceAll("\n", replacement);

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(text.String());
	RestoreCursorPosition(textView);
}


// Note: The ROT-13 algorithm is symmetrical, the same function will encode and decode the text.
void
ConvertToROT13(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty())
		return;

	for (int32 i = 0; i < text.Length(); ++i) {
		char currentChar = text.ByteAt(i);

		if (std::isalpha(currentChar)) {
			if (std::islower(currentChar))
				currentChar = 'a' + (currentChar - 'a' + 13) % 26;
			else
				currentChar = 'A' + (currentChar - 'A' + 13) % 26;
		}
		text.SetByteAt(i, currentChar);
	}

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(text.String());
	RestoreCursorPosition(textView);
}


void
URLEncode(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty())
		return;

	BString encoded;
	for (int32 i = 0; i < text.Length(); ++i) {
		char currentChar = text.ByteAt(i);

		// Check if the character is URL-safe (alphanumeric or special characters)
		if (std::isalnum(currentChar) || currentChar == '-' || currentChar == '_'
			|| currentChar == '.' || currentChar == '~') {
			encoded += currentChar;
		} else {
			// Encode the non-safe characters
			encoded += '%';
			std::stringstream ss;
			ss << std::uppercase << std::hex
			   << (int)(unsigned char)currentChar; // Convert char to hex
			std::string hexStr = ss.str();

			// Ensure the hex string is two characters long
			if (hexStr.length() == 1)
				encoded += '0'; // Add leading zero if needed
			encoded += BString(hexStr.c_str()); // Convert std::string to BString and append
		}
	}

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(encoded.String());
	RestoreCursorPosition(textView);
}


void
URLDecode(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty())
		return;

	BString decoded;
	for (int32 i = 0; i < text.Length(); ++i) {
		char currentChar = text.ByteAt(i);

		if (currentChar == '%') {
			// Check if there are enough characters for a valid hex code
			if (i + 2 < text.Length()) {
				char hex[3] = {text.ByteAt(i + 1), text.ByteAt(i + 2), '\0'};
				int decodedChar = 0;
				std::stringstream ss;
				ss << std::hex << hex;
				ss >> decodedChar;

				// Append the decoded character
				decoded += static_cast<char>(decodedChar);
				i += 2; // Skip the next two characters (hex code)
			}
		} else {
			// Regular character, append to decoded string
			decoded += currentChar;
		}
	}

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(decoded.String());
	RestoreCursorPosition(textView);
}


void
AddStringsToEachLine(BTextView* textView, const BString& startString, const BString& endString)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty())
		return;

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

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(updatedText.String());
	RestoreCursorPosition(textView);
}


void
RemoveStringsFromEachLine(BTextView* textView, const BString& prefix, const BString& suffix)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty())
		return;

	BString updatedText;
	int32 start = 0;
	int32 end;

	while ((end = text.FindFirst('\n', start)) >= 0) {
		BString line(text.String() + start, end - start);

		// Remove prefix if present
		if (!prefix.IsEmpty() && line.StartsWith(prefix))
			line.Remove(0, prefix.Length());

		// Remove suffix if present
		if (!suffix.IsEmpty() && line.EndsWith(suffix))
			line.Truncate(line.Length() - suffix.Length());

		updatedText << line << '\n';
		start = end + 1;
	}

	// Handle last line if it doesn't end with \n
	if (start < text.Length()) {
		BString line(text.String() + start, text.Length() - start);

		if (!prefix.IsEmpty() && line.StartsWith(prefix))
			line.Remove(0, prefix.Length());

		if (!suffix.IsEmpty() && line.EndsWith(suffix))
			line.Truncate(line.Length() - suffix.Length());

		updatedText << line;
	}

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(updatedText.String());
	RestoreCursorPosition(textView);
}


void
InsertLineBreaks(BTextView* textView, int32 maxLength, bool breakOnWords)
{
	BString text(GetTextFromTextView(textView));
	if (text.IsEmpty() || maxLength <= 0)
		return;

	BString updatedText;

	int32 start = 0;
	while (start < text.Length()) {
		int32 segmentEnd = start + maxLength;

		if (segmentEnd >= text.Length()) {
			updatedText.Append(text.String() + start, text.Length() - start);
			break;
		}

		if (breakOnWords) {
			int32 nearestSpace = text.FindLast(' ', segmentEnd);
			if (nearestSpace >= start)
				segmentEnd = nearestSpace;
		}

		// Append the segment and add line break
		updatedText.Append(text.String() + start, segmentEnd - start);
		updatedText.Append("\n");

		if (!breakOnWords && segmentEnd == start + maxLength) {
			start = segmentEnd;
		} else {
			// Move past the space or to the next segment
			start = segmentEnd + 1;
		}
	}

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(updatedText.String());
	RestoreCursorPosition(textView);
}


void
BreakLinesOnDelimiter(BTextView* textView, const BString& delimiter, bool keepDelimiter)
{
	BString text(GetTextFromTextView(textView));
	if (text.IsEmpty() || delimiter.Length() <= 0)
		return;

	BString updatedText;

	int32 start = 0;
	int32 delimiterPosition;

	while ((delimiterPosition = text.FindFirst(delimiter, start)) >= 0) {
		if (keepDelimiter) {
			// Include the delimiter in the line
			updatedText.Append(text.String() + start, delimiterPosition - start + delimiter.Length());
		} else {
			// Exclude the delimiter from the line
			updatedText.Append(text.String() + start, delimiterPosition - start);
		}
		updatedText.Append("\n");
		start = delimiterPosition + delimiter.Length();
	}

	if (start < text.Length()) {
		updatedText.Append(text.String() + start, text.Length() - start);
	}

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(updatedText.String());
	RestoreCursorPosition(textView);
}



void
TrimLines(BTextView* textView)
{
	BString text(GetTextFromTextView(textView));
	if (text.IsEmpty())
		return;

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

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(updatedText.String());
	RestoreCursorPosition(textView);
}


void
TrimEmptyLines(BTextView* textView)
{
	BString text(GetTextFromTextView(textView));
	if (text.IsEmpty())
		return;

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

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(updatedText.String());
	RestoreCursorPosition(textView);
}


bool
IsFullWord(const BString& text, int32 pos, int32 length)
{
	bool startOk = (pos == 0) || !isalnum(text.ByteAt(pos - 1));
	bool endOk = (pos + length >= text.Length() || !isalnum(text.ByteAt(pos + length)));
	return startOk && endOk;
}


void
ReplaceAll(BTextView* textView, BString find, BString replaceWith, bool caseSensitive,
	bool fullWordsOnly)
{
	BString text(GetTextFromTextView(textView));
	if (text.IsEmpty() || find.IsEmpty())
		return;

	int32 pos = 0;
	int32 findLength = find.Length();

	while (true) {
		pos = caseSensitive ? text.FindFirst(find.String(), pos)
							: text.IFindFirst(find.String(), pos);

		if (pos < 0)
			break;

		if (fullWordsOnly && !IsFullWord(text, pos, findLength)) {
			pos += findLength;
			continue;
		}

		text.Remove(pos, findLength);
		text.Insert(replaceWith, pos);
		pos += replaceWith.Length();
	}

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(text.String());
	RestoreCursorPosition(textView);
}


void
SortLines(BTextView* textView, bool ascending, bool caseSensitive)
{
	BString text(GetTextFromTextView(textView));
	if (text.IsEmpty())
		return;

	// Split text into lines
	std::vector<BString> lines;
	int32_t start = 0;
	while (true) {
		int32_t end = text.FindFirst('\n', start);
		BString line;
		if (end >= 0) {
			text.CopyInto(line, start, end - start);
			start = end + 1;
		} else {
			text.CopyInto(line, start, text.Length() - start);
			lines.push_back(line);
			break;
		}
		lines.push_back(line);
	}

	// Create ICU Collator
	UErrorCode status = U_ZERO_ERROR;
	std::unique_ptr<icu::Collator> collator(
		icu::Collator::createInstance(icu::Locale::getDefault(), status));

	if (U_FAILURE(status) || !collator) {
		printf("Failed to create ICU Collator: %s\n", u_errorName(status));
		return;
	}

	collator->setStrength(
		caseSensitive ? icu::Collator::TERTIARY // case-sensitive, accent-sensitive
					  : icu::Collator::SECONDARY // case-insensitive, accent-sensitive
	);

	// Sort using ICU
	std::sort(lines.begin(), lines.end(), [&](const BString& a, const BString& b) {
		icu::UnicodeString ua = icu::UnicodeString::fromUTF8(a.String());
		icu::UnicodeString ub = icu::UnicodeString::fromUTF8(b.String());
		UErrorCode cmpStatus = U_ZERO_ERROR;
		UCollationResult result = collator->compare(ua, ub, cmpStatus);
		if (U_FAILURE(cmpStatus))
			return ascending; // fallback: don't swap

		return ascending ? result == UCOL_LESS : result == UCOL_GREATER;
	});

	// Reconstruct the sorted text
	BString updatedText;
	for (size_t i = 0; i < lines.size(); ++i) {
		updatedText << lines[i];
		if (i != lines.size() - 1)
			updatedText << '\n';
	}

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(updatedText.String());
	RestoreCursorPosition(textView);
}


void
SortLinesByLength(BTextView* textView, bool ascending, bool caseSensitive)
{
	BString text(GetTextFromTextView(textView));
	if (text.IsEmpty())
		return;

	// Split text into lines
	std::vector<BString> lines;
	int32_t start = 0;
	while (true) {
		int32_t end = text.FindFirst('\n', start);
		BString line;
		if (end >= 0) {
			text.CopyInto(line, start, end - start);
			start = end + 1;
		} else {
			text.CopyInto(line, start, text.Length() - start);
			lines.push_back(line);
			break;
		}
		lines.push_back(line);
	}

	// Sort by length, with optional case-aware tiebreaker
	std::sort(lines.begin(), lines.end(), [&](const BString& a, const BString& b) {
		int32_t lenA = a.Length();
		int32_t lenB = b.Length();

		if (lenA != lenB)
			return ascending ? (lenA < lenB) : (lenA > lenB);

		// Tie-breaker: case-sensitive or insensitive compare
		icu::UnicodeString ua = icu::UnicodeString::fromUTF8(a.String());
		icu::UnicodeString ub = icu::UnicodeString::fromUTF8(b.String());

		if (!caseSensitive) {
			ua.toLower();
			ub.toLower();
		}

		int cmp = ua.compare(ub);
		return ascending ? (cmp < 0) : (cmp > 0);
	});

	// Reconstruct sorted text
	BString updatedText;
	for (size_t i = 0; i < lines.size(); ++i) {
		updatedText << lines[i];
		if (i != lines.size() - 1)
			updatedText << '\n';
	}

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(updatedText.String());
	RestoreCursorPosition(textView);
}


void
RemoveDuplicateLines(BTextView* textView, bool caseSensitive)
{
	BString text(GetTextFromTextView(textView));
	if (text.IsEmpty())
		return;

	// Split text into lines
	std::vector<BString> lines;
	int32_t start = 0;
	while (true) {
		int32_t end = text.FindFirst('\n', start);
		BString line;
		if (end >= 0) {
			text.CopyInto(line, start, end - start);
			start = end + 1;
		} else {
			text.CopyInto(line, start, text.Length() - start);
			lines.push_back(line);
			break;
		}
		lines.push_back(line);
	}

	// Store seen lines using ICU UnicodeString for proper comparison
	std::set<icu::UnicodeString> seen;
	std::vector<BString> uniqueLines;

	for (const BString& line : lines) {
		icu::UnicodeString uLine = icu::UnicodeString::fromUTF8(line.String());

		if (!caseSensitive)
			uLine.toLower();

		if (seen.find(uLine) == seen.end()) {
			seen.insert(uLine);
			uniqueLines.push_back(line);
		}
	}

	// Reconstruct unique lines into a single string
	BString result;
	for (size_t i = 0; i < uniqueLines.size(); ++i) {
		result << uniqueLines[i];
		if (i != uniqueLines.size() - 1)
			result << '\n';
	}

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(result.String());
	RestoreCursorPosition(textView);
}


void
IndentLines(BTextView* textView, bool useTabs, int32 count)
{
	if (count <= 0)
		return;

	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty())
		return;

	BString updatedText;
	BString indent;

	// Create the indentation string
	if (useTabs) {
		for (int32 i = 0; i < count; i++)
			indent << '\t';
	} else {
		for (int32 i = 0; i < count; i++)
			indent << ' ';
	}

	int32 start = 0;
	int32 end;

	while ((end = text.FindFirst('\n', start)) >= 0) {
		BString line(text.String() + start, end - start);
		updatedText << indent << line << '\n';
		start = end + 1;
	}

	// Handle last line if no newline at the end
	if (start < text.Length()) {
		BString line(text.String() + start, text.Length() - start);
		updatedText << indent << line;
	}

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(updatedText.String());
	RestoreCursorPosition(textView);
}


void
UnindentLines(BTextView* textView, bool useTabs, int32 count)
{
	if (count <= 0)
		return;

	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty())
		return;

	BString updatedText;
	BString indent;

	// Create the indentation string
	if (useTabs) {
		for (int32 i = 0; i < count; i++)
			indent << '\t';
	} else {
		for (int32 i = 0; i < count; i++)
			indent << ' ';
	}

	int32 start = 0;
	int32 end;

	while ((end = text.FindFirst('\n', start)) >= 0) {
		BString line(text.String() + start, end - start);

		if (line.StartsWith(indent)) {
			line.Remove(0, indent.Length());
		} else {
			// Try to remove as much as possible
			int32 i = 0;
			while (i < count && line.Length() > 0) {
				if ((useTabs && line.ByteAt(0) == '\t') || (!useTabs && line.ByteAt(0) == ' ')) {
					line.Remove(0, 1);
					i++;
				} else {
					break;
				}
			}
		}

		updatedText << line << '\n';
		start = end + 1;
	}

	// Handle last line
	if (start < text.Length()) {
		BString line(text.String() + start, text.Length() - start);
		if (line.StartsWith(indent)) {
			line.Remove(0, indent.Length());
		} else {
			int32 i = 0;
			while (i < count && line.Length() > 0) {
				if ((useTabs && line.ByteAt(0) == '\t') || (!useTabs && line.ByteAt(0) == ' ')) {
					line.Remove(0, 1);
					i++;
				} else {
					break;
				}
			}
		}
		updatedText << line;
	}

	static_cast<UndoableTextView*>(textView)->SetTextWithUndo(updatedText.String());
	RestoreCursorPosition(textView);
}
