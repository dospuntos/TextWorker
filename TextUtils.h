/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include <TextView.h>

void SaveCursorPosition(BTextView* textView);
void RestoreCursorPosition(BTextView* textView);
void ConvertToUppercase(BTextView* textView);
void ConvertToLowercase(BTextView* textView);
void ConvertToTitlecase(BTextView* textView);
void ConvertToAlternatingCase(BTextView* textView);
void ConvertToRandomCase(BTextView* textView);
void Capitalize(BTextView* textView);
void ToggleCase(BTextView* textView);

void RemoveLineBreaks(BTextView* textView, BString replacement = "");
void InsertLineBreaks(BTextView* textView, int32 maxWidth, bool breakOnWords = false);
BString ProcessLineWithBreaks(const BString& line, int32 maxLength, bool KeepWordsIntact);
void BreakLinesOnDelimiter(BTextView* textView, const BString& delimiter);
void TrimLines(BTextView* textView);
void TrimEmptyLines(BTextView* textView);
void RemoveDuplicateLines(BTextView* textView, bool caseSensitive = true);
void ReplaceAll(BTextView* textView, BString find, BString replaceWith, bool caseSensitive,
	bool fullWordsOnly);

void URLEncode(BTextView* textView);
void URLDecode(BTextView* textView);
void ConvertToROT13(BTextView* textView);
void AddStringsToEachLine(BTextView* textView, const BString& startString,
	const BString& endString);
void RemoveStringsFromEachLine(BTextView* textView, const BString& startString,
	const BString& endString);
void IndentLines(BTextView* textView, bool useTabs = true, int32 count = 1);
void UnindentLines(BTextView* textView, bool useTabs = true, int32 count = 1);

bool IsProbablyText(BFile& file);

void SortLines(BTextView* textView, bool ascending = true, bool caseSensitive = true);
void SortLinesByLength(BTextView* textView, bool ascending = true, bool caseSensitive = true);

#endif // TEXT_UTILS_H
