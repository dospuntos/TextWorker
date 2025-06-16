/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Constants.h"
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "App"

const char* kApplicationSignature = "application/x-vnd.jpw-TextWorker";
const char* kApplicationName = B_TRANSLATE_SYSTEM_NAME("TextWorker");

const int kDefaultFontSize = 12;
const char* kDefaultFontFamily = "be_fixed_font";
