/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <private/shared/ToolBar.h>

class BHandler;

BToolBar* CreateToolbar(BHandler* target);
BBitmap* ResourceToBitmap(const char* resName);
#endif // TOOLBAR_H
