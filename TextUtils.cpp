/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "TextUtils.h"
#include "Constants.h"
#include <Alert.h>
#include <Application.h>
#include <Catalog.h>
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

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Text utilities"

int32 startSelection, endSelection; // For cursor position
int32 selStart, selEnd; // For selected text
bool appliedToSelection = true;


BString
GetRelevantTextFromTextView(BTextView* textView, bool isLineBased)
{
	selStart = 0;
	selEnd = 0;

	if (textView == nullptr)
		return BString("");

	int32 textLength = textView->TextLength();
	if (textLength == 0)
		return BString("");


	textView->GetSelection(&selStart, &selEnd);

	if (selStart == selEnd) {
		selStart = 0;
		selEnd = textLength;
		appliedToSelection = false;
	} else if (isLineBased) {
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
ConvertToUppercase(BTextView* textView)
{
	BString text = GetRelevantTextFromTextView(textView, false);
	BString original = text;

	icu::UnicodeString unicodeText = icu::UnicodeString::fromUTF8(text.String());
	unicodeText.toUpper();

	// Convert back to UTF-8
	std::string utf8Text;
	unicodeText.toUTF8String(utf8Text);
	text = utf8Text.c_str();

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, text, text.Length());

	BString status;
	int32 changedCount = _CountCharChanges(original, text);
	if (appliedToSelection) {
		status.SetToFormat(B_TRANSLATE("%i characters changed to uppercase in selection"),
			changedCount);
	} else {
		status.SetToFormat(B_TRANSLATE("%i characters changed to uppercase in entire text"),
			changedCount);
	}
	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


void
ConvertToLowercase(BTextView* textView)
{
	BString text = GetRelevantTextFromTextView(textView, false);
	BString original = text;

	icu::UnicodeString unicodeText = icu::UnicodeString::fromUTF8(text.String());
	unicodeText.toLower();

	// Convert back to UTF-8
	std::string utf8Text;
	unicodeText.toUTF8String(utf8Text);
	text = utf8Text.c_str();

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, text, text.Length());
	BString status;
	int32 changedCount = _CountCharChanges(original, text);
	if (appliedToSelection) {
		status.SetToFormat(B_TRANSLATE("%i characters changed to lowercase in selection"),
			changedCount);
	} else {
		status.SetToFormat(B_TRANSLATE("%i characters changed to lowercase in entire text"),
			changedCount);
	}
	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


void
ConvertToTitlecase(BTextView* textView)
{
	BString text = GetRelevantTextFromTextView(textView, false);
	BString original = text;

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
	text = utf8Text.c_str();

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, text, text.Length());
	BString status;
	int32 changedCount = _CountCharChanges(original, text);
	if (appliedToSelection)
		status.SetToFormat(B_TRANSLATE("%i characters changed in selection"), changedCount);
	else
		status.SetToFormat(B_TRANSLATE("%i characters changed in entire text"), changedCount);
	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


void
Capitalize(BTextView* textView)
{
	BString rawText = GetRelevantTextFromTextView(textView, false);
	BString original = rawText;

	icu::UnicodeString utext = icu::UnicodeString::fromUTF8(rawText.String());
	utext.toLower(); // lowercase everything first

	bool capitalizeNext = true;
	for (int32 i = 0; i < utext.length(); ++i) {
		UChar32 c = utext.char32At(i);

		if (capitalizeNext && u_isalpha(c)) {
			UChar32 upper = u_totitle(c);
			utext.replace(i, U16_LENGTH(c), upper);
			capitalizeNext = false;
		} else if (c == '.' || c == '!' || c == '?') {
			capitalizeNext = true;
		} else if (!u_isspace(c)) {
			capitalizeNext = false;
		}
	}

	// Convert result back to UTF-8
	std::string utf8Result;
	utext.toUTF8String(utf8Result);
	BString text(utf8Result.c_str());

	textView->Select(selStart, selEnd);
	textView->Delete();
	textView->Insert(text.String());

	BString status;
	int32 changedCount = _CountCharChanges(original, text);
	if (appliedToSelection)
		status.SetToFormat(B_TRANSLATE("%i characters changed in selection"), changedCount);
	else
		status.SetToFormat(B_TRANSLATE("%i characters changed in entire text"), changedCount);
	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


void
ConvertToRandomCase(BTextView* textView)
{
	BString text = GetRelevantTextFromTextView(textView, false);
	BString original = text;

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
	BString status;
	int32 changedCount = _CountCharChanges(original, text);
	if (appliedToSelection)
		status.SetToFormat(B_TRANSLATE("%i characters changed in selection"), changedCount);
	else
		status.SetToFormat(B_TRANSLATE("%i characters changed in entire text"), changedCount);
	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


void
ConvertToAlternatingCase(BTextView* textView)
{
	BString text = GetRelevantTextFromTextView(textView, false);
	BString original = text;

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
	BString status;
	int32 changedCount = _CountCharChanges(original, text);
	if (appliedToSelection)
		status.SetToFormat(B_TRANSLATE("%i characters changed in selection"), changedCount);
	else
		status.SetToFormat(B_TRANSLATE("%i characters changed in entire text"), changedCount);
	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


void
ToggleCase(BTextView* textView)
{
	BString text = GetRelevantTextFromTextView(textView, false);
	BString original = text;

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
	BString status;
	int32 changedCount = _CountCharChanges(original, text);
	if (appliedToSelection)
		status.SetToFormat(B_TRANSLATE("%i characters changed in selection"), changedCount);
	else
		status.SetToFormat(B_TRANSLATE("%i characters changed in entire text"), changedCount);
	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


void
RemoveLineBreaks(BTextView* textView, BString replacement)
{
	BString text = GetRelevantTextFromTextView(textView, true);

	int32 count = 0;
	for (int32 i = 0; i < text.Length(); i++) {
		if (text[i] == '\n')
			count++;
	}
	text.ReplaceAll("\n", replacement);

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, text.String(), text.Length());

	BString status;
	if (replacement.IsEmpty()) {
		if (appliedToSelection)
			status.SetToFormat(B_TRANSLATE("%i line breaks removed in selection"), count);
		else
			status.SetToFormat(B_TRANSLATE("%i line breaks removed in entire text"), count);
	} else {
		if (appliedToSelection)
			status.SetToFormat(B_TRANSLATE("%i line breaks replaced in selection"), count);
		else
			status.SetToFormat(B_TRANSLATE("%i line breaks replaced in entire text"), count);
	}
	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


// Note: The ROT-13 algorithm is symmetrical, the same function will encode and decode the text.
void
ConvertToROT13(BTextView* textView)
{
	BString text = GetRelevantTextFromTextView(textView, false);
	int32 count = 0;

	for (int32 i = 0; i < text.Length(); ++i) {
		char currentChar = text.ByteAt(i);

		if (std::isalpha(currentChar)) {
			if (std::islower(currentChar))
				currentChar = 'a' + (currentChar - 'a' + 13) % 26;
			else
				currentChar = 'A' + (currentChar - 'A' + 13) % 26;
			count++;
		}
		text.SetByteAt(i, currentChar);
	}

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, text.String(), text.Length());
	BString status;
	if (appliedToSelection)
		status.SetToFormat(B_TRANSLATE("ROT13 applied to %i characters in selection"), count);
	else
		status.SetToFormat(B_TRANSLATE("ROT13 applied to %i characters in entire text"), count);
	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


void
URLEncode(BTextView* textView)
{
	BString text = GetRelevantTextFromTextView(textView, false);

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
	BString status;
	if (appliedToSelection)
		status.Append(B_TRANSLATE("Selected text URL-encoded"));
	else
		status.Append(B_TRANSLATE("Entire text URL-encoded"));
	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


void
URLDecode(BTextView* textView)
{
	BString text = GetRelevantTextFromTextView(textView, false);

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
	BString status;
	if (appliedToSelection)
		status.Append(B_TRANSLATE("Selected text URL-decoded"));
	else
		status.Append(B_TRANSLATE("Entire text URL-decoded"));
	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


void
AddStringsToEachLine(BTextView* textView, const BString& startString, const BString& endString)
{
	BString text = GetRelevantTextFromTextView(textView, true);
	int32 lineCount = 0;
	BString updatedText;

	int32 start = 0;
	int32 end;

	// Process each line
	while ((end = text.FindFirst('\n', start)) >= 0) {
		BString line(text.String() + start, end - start);
		updatedText << startString << line << endString << '\n';
		start = end + 1; // Move past the line break
		lineCount++;
	}

	// Last line (if it doesn't end with '\n')
	if (start < text.Length()) {
		BString line(text.String() + start, text.Length() - start);
		updatedText << startString << line << endString;
		lineCount++;
	}

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());
	BString status;
	if (appliedToSelection) {
		status.SetToFormat(B_TRANSLATE("Prefix/suffix added to %i lines in selection"), lineCount);
	} else {
		status.SetToFormat(B_TRANSLATE("Prefix/suffix added to %i lines in entire text"),
			lineCount);
	}
	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


void
RemoveStringsFromEachLine(BTextView* textView, const BString& prefix, const BString& suffix)
{
	BString text = GetRelevantTextFromTextView(textView, true);

	BString updatedText;
	int32 start = 0;
	int32 end;
	int32 lineCount = 0;

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
		lineCount++;
	}

	// Handle last line if it doesn't end with \n
	if (start < text.Length()) {
		BString line(text.String() + start, text.Length() - start);

		if (!prefix.IsEmpty() && line.StartsWith(prefix))
			line.Remove(0, prefix.Length());

		if (!suffix.IsEmpty() && line.EndsWith(suffix))
			line.Truncate(line.Length() - suffix.Length());

		updatedText << line;
		lineCount++;
	}

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());
	BString status;
	if (appliedToSelection) {
		status.SetToFormat(B_TRANSLATE("Prefix/suffix removed from %i lines in selection"),
			lineCount);
	} else {
		status.SetToFormat(B_TRANSLATE("Prefix/suffix removed from %i lines in entire text"),
			lineCount);
	}
	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


void
InsertLineBreaks(BTextView* textView, int32 maxLength, bool breakOnWords)
{
	bool appliedToSelection = false;
	BString text = GetRelevantTextFromTextView(textView, true);

	BString updatedText;

	int32 lineStart = 0;
	while (lineStart < text.Length()) {
		// Find the end of the current line
		int32 lineEnd = text.FindFirst('\n', lineStart);
		bool isLastLine = false;

		if (lineEnd == B_ERROR) {
			lineEnd = text.Length();
			isLastLine = true;
		}

		BString line;
		text.CopyInto(line, lineStart, lineEnd - lineStart);

		// Process line if needed
		int32 pos = 0;
		while (pos < line.Length()) {
			int32 segmentEnd = pos + maxLength;
			if (segmentEnd >= line.Length()) {
				updatedText.Append(line.String() + pos, line.Length() - pos);
				break;
			}

			if (breakOnWords) {
				int32 nearestSpace = line.FindLast(' ', segmentEnd);
				if (nearestSpace >= pos) {
					segmentEnd = nearestSpace;
				} else {
					// No space found: fallback to hard break at maxLength
					segmentEnd = pos + maxLength;
				}
			}

			// Append the segment and break
			updatedText.Append(line.String() + pos, segmentEnd - pos);
			updatedText.Append("\n");

			// Advance past this segment
			if (segmentEnd < line.Length() && line[segmentEnd] == ' ')
				pos = segmentEnd + 1; // skip space
			else
				pos = segmentEnd;
		}

		// If line was already short and unbroken, add newline
		if (line.Length() <= maxLength)
			updatedText.Append("\n");

		lineStart = lineEnd + 1;
	}

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());

	BString status;
	BString breakType
		= breakOnWords ? B_TRANSLATE("breaking on words") : B_TRANSLATE("breaking anywhere");

	if (appliedToSelection) {
		status.SetToFormat(B_TRANSLATE("Line breaks inserted in selection (max length: %d, %s)"),
			maxLength, breakType.String());
	} else {
		status.SetToFormat(B_TRANSLATE("Line breaks inserted in entire text (max length: %d, %s)"),
			maxLength, breakType.String());
	}
	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


void
BreakLinesOnDelimiter(BTextView* textView, const BString& delimiter, bool keepDelimiter)
{
	BString text = GetRelevantTextFromTextView(textView, true);

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
	BString status;
	BString keepStr = keepDelimiter ? B_TRANSLATE("kept") : B_TRANSLATE("removed");

	if (appliedToSelection) {
		status.SetToFormat(B_TRANSLATE("Lines broken on delimiter \"%s\" (%s) in selection"),
			delimiter.String(), keepStr.String());
	} else {
		status.SetToFormat(B_TRANSLATE("Lines broken on delimiter \"%s\" (%s) in entire text"),
			delimiter.String(), keepStr.String());
	}

	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


void
TrimWhitespace(BTextView* textView)
{
	BString text = GetRelevantTextFromTextView(textView, true);

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
	BString status;
	if (appliedToSelection)
		status.SetToFormat(B_TRANSLATE("Whitespace trimmed from lines in selection"));
	else
		status.SetToFormat(B_TRANSLATE("Whitespace trimmed from lines in entire text"));
	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


void
TrimEmptyLines(BTextView* textView)
{
	BString text = GetRelevantTextFromTextView(textView, true);

	int32 start = 0;
	int32 end;
	int32 removedLineCount = 0;

	BString updatedText;

	while ((end = text.FindFirst('\n', start)) >= 0) {
		BString line(text.String() + start, end - start);

		if (line.Length() > 0)
			updatedText << line << '\n';
		else
			removedLineCount++;

		start = end + 1;
	}

	// Handle last line if no '\n'
	if (start < text.Length()) {
		BString lastLine(text.String() + start, text.Length() - start);
		if (lastLine.Length() > 0)
			updatedText.Append(lastLine);
		else
			removedLineCount++;
	}

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());
	BString status;
	if (appliedToSelection) {
		status.SetToFormat(B_TRANSLATE("%d empty lines removed from selection"), removedLineCount);
	} else {
		status.SetToFormat(B_TRANSLATE("%d empty lines removed from entire text"),
			removedLineCount);
	}
	SendStatusMessage(status);
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
	BString text = GetRelevantTextFromTextView(textView, false);
	int32 replacementCount = 0;

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
		replacementCount++;
	}

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, text.String(), text.Length());
	BString status;

	if (appliedToSelection) {
		status.SetToFormat(B_TRANSLATE("%d occurrences of \"%s\" replaced in selection"),
			replacementCount, find.String());
	} else {
		status.SetToFormat(B_TRANSLATE("%d occurrences of \"%s\" replaced in entire text"),
			replacementCount, find.String());
	}

	SendStatusMessage(status);
	RestoreCursorPosition(textView);
}


void
SortLines(BTextView* textView, bool ascending, bool caseSensitive)
{
	BString text = GetRelevantTextFromTextView(textView, true);

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
	BString order = ascending ? B_TRANSLATE("ascending") : B_TRANSLATE("descending");

	BString statusMsg;
	if (appliedToSelection) {
		statusMsg.SetToFormat(
			B_TRANSLATE("%zu lines sorted alphabetically in %s order in selection"), lines.size(),
			order.String());
	} else {
		statusMsg.SetToFormat(
			B_TRANSLATE("%zu lines sorted alphabetically in %s order in entire text"), lines.size(),
			order.String());
	}
	SendStatusMessage(statusMsg);
	RestoreCursorPosition(textView);
}


void
SortLinesByLength(BTextView* textView, bool ascending, bool caseSensitive)
{
	BString text = GetRelevantTextFromTextView(textView, true);

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
	BString order = ascending ? B_TRANSLATE("ascending") : B_TRANSLATE("descending");

	BString statusMsg;
	if (appliedToSelection) {
		statusMsg.SetToFormat(
			B_TRANSLATE("%zu lines sorted by line length in %s order in selection"), lines.size(),
			order.String());
	} else {
		statusMsg.SetToFormat(
			B_TRANSLATE("%zu lines sorted by line length in %s order in entire text"), lines.size(),
			order.String());
	}
	SendStatusMessage(statusMsg);
	RestoreCursorPosition(textView);
}


void
RemoveDuplicateLines(BTextView* textView, bool caseSensitive)
{
	BString text = GetRelevantTextFromTextView(textView, true);

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

	int32 linesRemoved = lines.size() - uniqueLines.size();

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());
	BString statusMsg;
	if (appliedToSelection) {
		statusMsg.SetToFormat(B_TRANSLATE("%i duplicated lines removed from selection"),
			linesRemoved);
	} else {
		statusMsg.SetToFormat(B_TRANSLATE("%i duplicated lines removed from entire text"),
			linesRemoved);
	}
	SendStatusMessage(statusMsg);
	RestoreCursorPosition(textView);
}


void
IndentLines(BTextView* textView, bool useTabs, int32 count)
{
	if (count <= 0)
		return;

	BString text = GetRelevantTextFromTextView(textView, true);

	BString updatedText;
	BString indent;
	int32 lineCount = 0;

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
		lineCount++;
	}

	// Handle last line if no newline at the end
	if (start < text.Length()) {
		BString line(text.String() + start, text.Length() - start);
		updatedText << indent << line;
		lineCount++;
	}

	textView->Delete(selStart, selEnd);
	textView->Insert(selStart, updatedText.String(), updatedText.Length());
	BString statusMsg;
	BString indentationType = useTabs ? B_TRANSLATE("tabs") : B_TRANSLATE("spaces");
	if (appliedToSelection) {
		statusMsg.SetToFormat(B_TRANSLATE("%i selected lines indented by %i %s"), lineCount, count,
			indentationType.String());
	} else {
		statusMsg.SetToFormat(B_TRANSLATE("%i lines indented by %i %s"), lineCount, count,
			indentationType.String());
	}
	SendStatusMessage(statusMsg);
	RestoreCursorPosition(textView);
}


void
UnindentLines(BTextView* textView, bool useTabs, int32 count)
{
	if (count <= 0)
		return;

	BString text = GetRelevantTextFromTextView(textView, true);

	BString updatedText;
	BString indent;
	int32 lineCount = 0;

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
			lineCount++;
		} else {
			// Try to remove as much as possible
			int32 i = 0;
			while (i < count && line.Length() > 0) {
				if ((useTabs && line.ByteAt(0) == '\t') || (!useTabs && line.ByteAt(0) == ' ')) {
					line.Remove(0, 1);
					lineCount++;
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
			lineCount++;
		} else {
			int32 i = 0;
			while (i < count && line.Length() > 0) {
				if ((useTabs && line.ByteAt(0) == '\t') || (!useTabs && line.ByteAt(0) == ' ')) {
					line.Remove(0, 1);
					lineCount++;
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
	BString statusMsg;
	BString indentationType = useTabs ? B_TRANSLATE("tabs") : B_TRANSLATE("spaces");
	if (appliedToSelection) {
		statusMsg.SetToFormat(B_TRANSLATE("%i selected lines unindented by %i %s"), lineCount,
			count, indentationType.String());
	} else {
		statusMsg.SetToFormat(B_TRANSLATE("%i lines unindented by %i %s"), lineCount, count,
			indentationType.String());
	}
	SendStatusMessage(statusMsg);
	RestoreCursorPosition(textView);
}


void
ShowTextStats(BTextView* textView)
{
	BString text = GetRelevantTextFromTextView(textView, false);
	if (text.IsEmpty()) {
		SendStatusMessage(B_TRANSLATE("No text selected"));
		return;
	}

	icu::UnicodeString unicodeText = icu::UnicodeString::fromUTF8(text.String());
	BString statsMsg;

	int32 charCount = unicodeText.countChar32();
	int32 lineCount = CountLines(text);
	int32 maxLineLength = 0;
	int32 totalWordLength = 0;
	int32 wordCount = CountWords(text);
	int32 sentenceCount = CountSentences(text);

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
			totalWordLength += word.countChar32();
			word.toLower();
			std::string utf8;
			word.toUTF8String(utf8);
			if (utf8.length() > 2)
				wordFrequency[utf8]++;
		}
	}

	// Most common words
	std::vector<std::pair<BString, int>> sortedWords;
	for (const auto& entry : wordFrequency)
		sortedWords.emplace_back(BString(entry.first.c_str()), entry.second);
	std::sort(sortedWords.begin(), sortedWords.end(), [](const auto& a, const auto& b) {
		if (a.second != b.second)
			return a.second > b.second; // Highest frequency first
		return a.first.ICompare(b.first) < 0; // Alphabetical for tie-breaking
	});


	statsMsg.SetToFormat(B_TRANSLATE("STATISTICS FOR CURRENT TEXT\n\n"
									 "Characters: %d\n"
									 "Words: %d\n"
									 "Lines: %d\n"
									 "Sentences: %d\n"
									 "Longest line: %d chars\n"
									 "Average word length: %.2f\n\n"
									 "Most used words:\n"),
		charCount, wordCount, lineCount, sentenceCount, maxLineLength,
		wordCount > 0 ? (float)totalWordLength / wordCount : 0.0);

	int shown = 0;
	for (const auto& [word, freq] : sortedWords) {
		statsMsg << "  " << word << ": " << freq << '\n';
		if (++shown == 10)
			break;
	}
	(new BAlert("Stats", statsMsg.String(), B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL,
		 B_IDEA_ALERT))
		->Go();
}


void
SendStatusMessage(const BString& text)
{
	BWindow* window = be_app->WindowAt(0);
	if (window) {
		BMessage msg(M_SHOW_STATUS);
		msg.AddString("text", text);

		BMessenger messenger(window);
		messenger.SendMessage(&msg);
	}
}


int32
_CountCharChanges(const BString& original, const BString& transformed)
{
	int32 count = 0;
	int32 len = MIN(original.Length(), transformed.Length());

	for (int32 i = 0; i < len; i++) {
		if (original[i] != transformed[i])
			count++;
	}

	return count;
}


int32
CountLines(const BString& text)
{
	if (text.Length() == 0)
		return 0;

	int32 lineCount = 0;
	for (int32 i = 0; i < text.Length(); i++) {
		if (text[i] == '\n')
			lineCount++;
	}

	// Add one more if the text doesn't end in a newline
	if (text[text.Length() - 1] != '\n')
		lineCount++;

	return lineCount;
}


int32
CountWords(const BString& text)
{
	UErrorCode status = U_ZERO_ERROR;
	icu::UnicodeString utext(text.String());

	std::unique_ptr<icu::BreakIterator> bi(
		icu::BreakIterator::createWordInstance(icu::Locale::getDefault(), status));

	if (U_FAILURE(status) || !bi)
		return 0;

	bi->setText(utext);

	int32_t count = 0;
	for (int32_t start = bi->first(), end = bi->next(); end != icu::BreakIterator::DONE;
		start = end, end = bi->next()) {

		// Check if the boundary is a word (letters or numbers)
		icu::UnicodeString word = utext.tempSubStringBetween(start, end);
		if (word.trim().length() > 0
			&& u_getIntPropertyValue(word.char32At(0), UCHAR_GENERAL_CATEGORY)
				!= U_SPACE_SEPARATOR) {
			count++;
		}
	}

	return count;
}


int32
CountSentences(const BString& text)
{
	if (text.IsEmpty())
		return 0;

	UErrorCode status = U_ZERO_ERROR;
	icu::UnicodeString unicodeText(text.String());

	std::unique_ptr<icu::BreakIterator> sentIter(
		icu::BreakIterator::createSentenceInstance(icu::Locale::getDefault(), status));

	if (U_FAILURE(status) || !sentIter)
		return 0;

	sentIter->setText(unicodeText);

	int32 count = 0;
	for (int32_t start = sentIter->first(), end = sentIter->next(); end != icu::BreakIterator::DONE;
		start = end, end = sentIter->next()) {

		icu::UnicodeString segment = unicodeText.tempSubStringBetween(start, end);
		segment.trim();
		if (!segment.isEmpty())
			count++;
	}

	return count;
}
