/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include <TextView.h>

BString GetRelevantTextFromTextView(BTextView* textView, bool applyToSelectionOnly,
	bool isLineBased);
void SaveCursorPosition(BTextView* textView);
void RestoreCursorPosition(BTextView* textView);
void ConvertToUppercase(BTextView* textView, bool applyToSelection);
void ConvertToLowercase(BTextView* textView, bool applyToSelection);
void ConvertToTitlecase(BTextView* textView, bool applyToSelection);
void ConvertToAlternatingCase(BTextView* textView, bool applyToSelection);
void ConvertToRandomCase(BTextView* textView, bool applyToSelection);
void Capitalize(BTextView* textView, bool applyToSelection);
void ToggleCase(BTextView* textView, bool applyToSelection);

void RemoveLineBreaks(BTextView* textView, BString replacement = "", bool applyToSelection = false);
void InsertLineBreaks(BTextView* textView, int32 maxWidth, bool breakOnWords = false,
	bool applyToSelection = false);
BString ProcessLineWithBreaks(const BString& line, int32 maxLength, bool KeepWordsIntact);
void BreakLinesOnDelimiter(BTextView* textView, const BString& delimiter, bool keepDelimiter = true,
	bool applyToSelection = false);
void TrimWhitespace(BTextView* textView, bool applyToSelection);
void TrimEmptyLines(BTextView* textView, bool applyToSelection);
void RemoveDuplicateLines(BTextView* textView, bool caseSensitive = true,
	bool applyToSelection = false);
void ReplaceAll(BTextView* textView, BString find, BString replaceWith, bool caseSensitive,
	bool fullWordsOnly, bool applyToSelection = false);

void URLEncode(BTextView* textView, bool applyToSelection);
void URLDecode(BTextView* textView, bool applyToSelection);
void ConvertToROT13(BTextView* textView, bool applyToSelection);
void AddStringsToEachLine(BTextView* textView, const BString& startString, const BString& endString,
	bool applyToSelection);
void RemoveStringsFromEachLine(BTextView* textView, const BString& startString,
	const BString& endString, bool applyToSelection = false);
void IndentLines(BTextView* textView, bool useTabs = true, int32 count = 1,
	bool applyToSelection = false);
void UnindentLines(BTextView* textView, bool useTabs = true, int32 count = 1,
	bool applyToSelection = false);

bool IsProbablyText(BFile& file);
void ShowTextStats(BTextView* textView, bool applyToSelection);

void SortLines(BTextView* textView, bool ascending = true, bool caseSensitive = true,
	bool applyToSelection = false);
void SortLinesByLength(BTextView* textView, bool ascending = true, bool caseSensitive = true,
	bool applyToSelection = false);
void SendStatusMessage(const BString& text);
int32 _CountCharChanges(const BString& original, const BString& transformed);
int32 CountLines(const BString& text);
int32 CountWords(const BString& text);
int32 CountSentences(const BString& text);

#endif // TEXT_UTILS_H
