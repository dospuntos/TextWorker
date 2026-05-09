/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include <TextView.h>

struct HtmlEntity {
	const char* name;
	uint32 codepoint;
};

static const HtmlEntity kEntities[] = {
	// Most common
		{ "amp",   '&'  },
		{ "lt",    '<'  },
		{ "gt",    '>'  },
		{ "quot",  '"'  },
		{ "apos",  '\'' },
		// Extended
		{ "nbsp",   160 },
		{ "iexcl",  161 },
		{ "cent",   162 },
		{ "pound",  163 },
		{ "curren", 164 },
		{ "yen",    165 },
		{ "brvbar", 166 },
		{ "sect",   167 },
		{ "uml",    168 },
		{ "copy",   169 },
		{ "ordf",   170 },
		{ "laquo",  171 },
		{ "not",    172 },
		{ "shy",    173 },
		{ "reg",    174 },
		{ "macr",   175 },
		{ "deg",    176 },
		{ "plusmn", 177 },
		{ "sup2",   178 },
		{ "sup3",   179 },
		{ "acute",  180 },
		{ "micro",  181 },
		{ "para",   182 },
		{ "middot", 183 },
		{ "cedil",  184 },
		{ "sup1",   185 },
		{ "ordm",   186 },
		{ "raquo",  187 },
		{ "frac14", 188 },
		{ "frac12", 189 },
		{ "frac34", 190 },
		{ "iquest", 191 },
		{ "Agrave", 192 }, { "Aacute", 193 }, { "Acirc",  194 },
		{ "Atilde", 195 }, { "Auml",   196 }, { "Aring",  197 },
		{ "AElig",  198 }, { "Ccedil", 199 }, { "Egrave", 200 },
		{ "Eacute", 201 }, { "Ecirc",  202 }, { "Euml",   203 },
		{ "Igrave", 204 }, { "Iacute", 205 }, { "Icirc",  206 },
		{ "Iuml",   207 }, { "ETH",    208 }, { "Ntilde", 209 },
		{ "Ograve", 210 }, { "Oacute", 211 }, { "Ocirc",  212 },
		{ "Otilde", 213 }, { "Ouml",   214 }, { "times",  215 },
		{ "Oslash", 216 }, { "Ugrave", 217 }, { "Uacute", 218 },
		{ "Ucirc",  219 }, { "Uuml",   220 }, { "Yacute", 221 },
		{ "THORN",  222 }, { "szlig",  223 },
		{ "agrave", 224 }, { "aacute", 225 }, { "acirc",  226 },
		{ "atilde", 227 }, { "auml",   228 }, { "aring",  229 },
		{ "aelig",  230 }, { "ccedil", 231 }, { "egrave", 232 },
		{ "eacute", 233 }, { "ecirc",  234 }, { "euml",   235 },
		{ "igrave", 236 }, { "iacute", 237 }, { "icirc",  238 },
		{ "iuml",   239 }, { "eth",    240 }, { "ntilde", 241 },
		{ "ograve", 242 }, { "oacute", 243 }, { "ocirc",  244 },
		{ "otilde", 245 }, { "ouml",   246 }, { "divide", 247 },
		{ "oslash", 248 }, { "ugrave", 249 }, { "uacute", 250 },
		{ "ucirc",  251 }, { "uuml",   252 }, { "yacute", 253 },
		{ "thorn",  254 }, { "yuml",   255 },
};

static const int32 kEntityCount =
	(int32)(sizeof(kEntities) / sizeof(kEntities[0]));

BString GetText(BTextView* textView, bool isLineBased);
void SaveCursorPosition(BTextView* textView);
void RestoreCursorPosition(BTextView* textView);
void RestoreCursorPosition(BTextView* textView, int32 textLength);
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
void BreakLinesOnDelimiter(BTextView* textView, const BString& delimiter,
	bool keepDelimiter = true);
void TrimWhitespace(BTextView* textView);
void TrimEmptyLines(BTextView* textView);
void RemoveDuplicateLines(BTextView* textView, bool caseSensitive = true);
void ReplaceAll(BTextView* textView, BString find, BString replaceWith, bool caseSensitive,
	bool fullWordsOnly);

void URLEncode(BTextView* textView);
void URLDecode(BTextView* textView);
void Base64(BTextView* textView);
void EncodeHTMLEntities(BTextView* textView, bool encodeByName);
void DecodeHTMLEntities(BTextView* textView);
void ConvertToROT13(BTextView* textView);
void AddStringsToEachLine(BTextView* textView, const BString& startString,
	const BString& endString);
void RemoveStringsFromEachLine(BTextView* textView, const BString& startString,
	const BString& endString);
void IndentLines(BTextView* textView, bool useTabs = true, int32 count = 1);
void UnindentLines(BTextView* textView, bool useTabs = true, int32 count = 1);

bool IsProbablyText(BFile& file);
void ShowTextStats(BTextView* textView);

void SortLines(BTextView* textView, bool ascending = true, bool caseSensitive = true);
void SortLinesByLength(BTextView* textView, bool ascending = true, bool caseSensitive = true);
void SendStatusMessage(const BString& text);
int32 _CountCharChanges(const BString& original, const BString& transformed);
int32 CountLines(const BString& text);
int32 CountWords(const BString& text);
int32 CountSentences(const BString& text);

#endif // TEXT_UTILS_H
