/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include <TextView.h>

void ConvertToUppercase(BTextView* textView);
void ConvertToLowercase(BTextView* textView);
void ConvertToTitlecase(BTextView* textView);
void ConvertToAlternatingCase(BTextView* textView);
void Capitalize(BTextView* textView);

void RemoveLineBreaks(BTextView* textView);

void ConvertToROT13(BTextView* textView);


#endif // TEXT_UTILS_H
