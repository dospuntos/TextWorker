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
#include <map>
#include <set>
#include <sstream>
#include <unicode/brkiter.h>
#include <unicode/coll.h>
#include <unicode/locid.h>
#include <unicode/unistr.h>
#include <vector>


int32 startSelection, endSelection; // For cursor position
int32 selStart, selEnd; // For selected text


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


BString
GetRelevantTextFromTextView(BTextView* textView, bool applyToSelectionOnly, bool isLineBased)
{
	selStart = 0;
	selEnd = 0;

	if (textView == nullptr)
		return BString("");

	int32 textLength = textView->TextLength();
	if (textLength == 0)
		return BString("");

	if (!applyToSelectionOnly) {
		selStart = 0;
		selEnd = textLength;
	} else {
		textView->GetSelection(&selStart, &selEnd);

		if (isLineBased) {
			const char* fullText = textView->Text();

			// Extend selStartOut to beginning of line
			while (selStart > 0 && fullText[selStart - 1] != '\n')
				selStart--;

			// Extend selEndOut to end of line (but don’t go past final \n)
			while (selEnd < textLength && fullText[selEnd] != '\n')
				selEnd++;
			// Don't add one unless we're not already at a linebreak
			if (selEnd < textLength && fullText[selEnd] == '\n')
				selEnd++;
		}
	}

	char* buffer = new char[selEnd - selStart + 1];
	textView->GetText(selStart, selEnd, buffer);
	buffer[selEnd - selStart] = '\0';

	BString result(buffer);
	delete[] buffer;

	SaveCursorPosition(textView);
	return result;
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
ConvertToUppercase(BTextView* textView, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, false);

	icu::UnicodeString unicodeText = icu::UnicodeString::fromUTF8(text.String());
	unicodeText.toUpper();

	// Convert back to UTF-8
	std::string utf8Text;
	unicodeText.toUTF8String(utf8Text);

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, utf8Text.c_str(), text.Length());
	RestoreCursorPosition(textView);
}


void
ConvertToLowercase(BTextView* textView, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, false);

	icu::UnicodeString unicodeText = icu::UnicodeString::fromUTF8(text.String());
	unicodeText.toLower();

	// Convert back to UTF-8
	std::string utf8Text;
	unicodeText.toUTF8String(utf8Text);

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, utf8Text.c_str(), text.Length());
	RestoreCursorPosition(textView);
}


void
ConvertToTitlecase(BTextView* textView, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, false);

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

	std::string utf8Text;
	unicodeText.toUTF8String(utf8Text);

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, utf8Text.c_str(), text.Length());
	RestoreCursorPosition(textView);
}


void
Capitalize(BTextView* textView, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, false);

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

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, text.String(), text.Length());
	RestoreCursorPosition(textView);
}


void
ConvertToRandomCase(BTextView* textView, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, false);

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

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, text.String(), text.Length());
	RestoreCursorPosition(textView);
}


void
ConvertToAlternatingCase(BTextView* textView, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, false);

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

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, text.String(), text.Length());
	RestoreCursorPosition(textView);
}


void
ToggleCase(BTextView* textView, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, false);

	for (int32 i = 0; i < text.Length(); ++i) {
		char currentChar = text.ByteAt(i);
		if (std::isupper(currentChar))
			currentChar = std::tolower(currentChar);
		else if (std::islower(currentChar))
			currentChar = std::toupper(currentChar);
		text.SetByteAt(i, currentChar);
	}

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, text.String(), text.Length());
	RestoreCursorPosition(textView);
}


void
RemoveLineBreaks(BTextView* textView, BString replacement, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, true);

	text.ReplaceAll("\n", replacement);

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, text.String(), text.Length());
	RestoreCursorPosition(textView);
}


// Note: The ROT-13 algorithm is symmetrical, the same function will encode and decode the text.
void
ConvertToROT13(BTextView* textView, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, false);

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

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, text.String(), text.Length());
	RestoreCursorPosition(textView);
}


void
URLEncode(BTextView* textView, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, false);

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

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, encoded.String(), encoded.Length());
	RestoreCursorPosition(textView);
}


void
URLDecode(BTextView* textView, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, false);

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

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, decoded.String(), decoded.Length());
	RestoreCursorPosition(textView);
}


void
AddStringsToEachLine(BTextView* textView, const BString& startString, const BString& endString,
	bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, true);

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

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());
	RestoreCursorPosition(textView);
}


void
RemoveStringsFromEachLine(BTextView* textView, const BString& prefix, const BString& suffix,
	bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, true);

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

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());
	RestoreCursorPosition(textView);
}


void
InsertLineBreaks(BTextView* textView, int32 maxLength, bool breakOnWords, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, true);

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

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());
	RestoreCursorPosition(textView);
}


void
BreakLinesOnDelimiter(BTextView* textView, const BString& delimiter, bool keepDelimiter,
	bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, true);

	BString updatedText;

	int32 start = 0;
	int32 delimiterPosition;

	while ((delimiterPosition = text.FindFirst(delimiter, start)) >= 0) {
		if (keepDelimiter) {
			// Include the delimiter in the line
			updatedText.Append(text.String() + start,
				delimiterPosition - start + delimiter.Length());
		} else {
			// Exclude the delimiter from the line
			updatedText.Append(text.String() + start, delimiterPosition - start);
		}
		updatedText.Append("\n");
		start = delimiterPosition + delimiter.Length();
	}

	if (start < text.Length())
		updatedText.Append(text.String() + start, text.Length() - start);

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());
	RestoreCursorPosition(textView);
}


void
TrimWhitespace(BTextView* textView, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, true);

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

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());
	RestoreCursorPosition(textView);
}


void
TrimEmptyLines(BTextView* textView, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, true);

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

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());
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
	bool fullWordsOnly, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, false);

	if (find.IsEmpty())
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

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, text.String(), text.Length());
	RestoreCursorPosition(textView);
}


void
SortLines(BTextView* textView, bool ascending, bool caseSensitive, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, true);

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

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());
	RestoreCursorPosition(textView);
}


void
SortLinesByLength(BTextView* textView, bool ascending, bool caseSensitive, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, true);

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

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());
	RestoreCursorPosition(textView);
}


void
RemoveDuplicateLines(BTextView* textView, bool caseSensitive, bool applyToSelection)
{
	BString text = GetRelevantTextFromTextView(textView, applyToSelection, true);

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
	BString updatedText;
	for (size_t i = 0; i < uniqueLines.size(); ++i) {
		updatedText << uniqueLines[i];
		if (i != uniqueLines.size() - 1)
			updatedText << '\n';
	}

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());
	RestoreCursorPosition(textView);
}


void
IndentLines(BTextView* textView, bool useTabs, int32 count, bool applyToSelection)
{
	if (count <= 0)
		return;

	BString text = GetRelevantTextFromTextView(textView, applyToSelection, true);

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

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());
	RestoreCursorPosition(textView);
}


void
UnindentLines(BTextView* textView, bool useTabs, int32 count, bool applyToSelection)
{
	if (count <= 0)
		return;

	BString text = GetRelevantTextFromTextView(textView, applyToSelection, true);

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

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());
	RestoreCursorPosition(textView);
}


void
ShowTextStats(BTextView* textView)
{
	BString text = GetTextFromTextView(textView);
	if (text.IsEmpty()) {
		(new BAlert("stats", "The text is empty.", "OK"))->Go();
		return;
	}

	icu::UnicodeString unicodeText = icu::UnicodeString::fromUTF8(text.String());
	BString statsMsg;

	int32 charCount = unicodeText.countChar32();
	int32 lineCount = 0;
	int32 maxLineLength = 0;
	int32 totalWordLength = 0;
	int32 wordCount = 0;
	int32 sentenceCount = 0;

	std::map<std::string, int> wordFrequency;

	// Max line length
	int32 start = 0, end;
	while ((end = text.FindFirst('\n', start)) >= 0) {
		int32 lineLen = end - start;
		if (lineLen > maxLineLength)
			maxLineLength = lineLen;
		start = end + 1;
	}
	if (start < text.Length()) {
		int32 lineLen = text.Length() - start;
		if (lineLen > maxLineLength)
			maxLineLength = lineLen;
	}

	// ICU word iterator
	UErrorCode status = U_ZERO_ERROR;
	std::unique_ptr<icu::BreakIterator> wordIter(
		icu::BreakIterator::createWordInstance(icu::Locale::getDefault(), status));
	wordIter->setText(unicodeText);

	int32_t startWord = wordIter->first();
	for (int32_t endWord = wordIter->next(); endWord != icu::BreakIterator::DONE;
		startWord = endWord, endWord = wordIter->next()) {
		icu::UnicodeString word = unicodeText.tempSubStringBetween(startWord, endWord);
		if (word.trim().isEmpty())
			continue;
		if (word.char32At(0) >= 0x30 && u_isalnum(word.char32At(0))) {
			wordCount++;
			totalWordLength += word.countChar32();
			word.toLower();
			std::string utf8;
			word.toUTF8String(utf8);
			if (utf8.length() > 2)
				wordFrequency[utf8]++;
		}
	}

	// ICU sentence iterator
	std::unique_ptr<icu::BreakIterator> sentIter(
		icu::BreakIterator::createSentenceInstance(icu::Locale::getDefault(), status));
	sentIter->setText(unicodeText);

	for (int32_t boundary = sentIter->first(); boundary != icu::BreakIterator::DONE;
		boundary = sentIter->next()) {
		sentenceCount++;
	}

	// Most common words
	std::vector<std::pair<BString, int>> sortedWords;
	for (const auto& entry : wordFrequency)
		sortedWords.emplace_back(BString(entry.first.c_str()), entry.second);
	std::sort(sortedWords.begin(), sortedWords.end(),
		[](const auto& a, const auto& b) { return b.second > a.second; });

	statsMsg.SetToFormat("STATISTICS FOR CURRENT TEXT\n\n"
						 "Characters: %d\n"
						 "Words: %d\n"
						 "Lines: %d\n"
						 "Sentences: %d\n"
						 "Longest line: %d chars\n"
						 "Average word length: %.2f\n\n"
						 "Most used words:\n",
		charCount, wordCount, lineCount, sentenceCount, maxLineLength,
		wordCount > 0 ? (float)totalWordLength / wordCount : 0.0);

	int shown = 0;
	for (const auto& [word, freq] : sortedWords) {
		statsMsg << "  " << word << ": " << freq << '\n';
		if (++shown == 5)
			break;
	}
	(new BAlert("Stats", statsMsg.String(), "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_IDEA_ALERT))
		->Go();
}
