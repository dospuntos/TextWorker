/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "MainWindow.h"
#include "Toolbar.h"
#include <Button.h>
#include <Bitmap.h>
#include <TranslationUtils.h>
#include <Application.h>
#include <Resources.h>

enum {
	MSG_UPPERCASE = 'upcs',
	MSG_LOWERCASE,
	MSG_TITLECASE
};

Toolbar::Toolbar()
	: BGroupView(B_HORIZONTAL, 5)
{
	BResources* resources = be_app->AppResources();

	size_t dataSize;
	const void* data = resources->LoadResource('VICN', "uppercase_icon", &dataSize);
	BBitmap* uppercaseIcon = nullptr;
	if (data != nullptr) {
		BMemoryIO stream(data, dataSize);
		uppercaseIcon = BTranslationUtils::GetBitmap(&stream);
	}

	BButton* uppercaseButton = new BButton(
		BRect(0, 0, 24, 24), "UppercaseButton", "", new BMessage(M_TRANSFORM_UPPERCASE));
	if (uppercaseIcon != nullptr)
		uppercaseButton->SetIcon(uppercaseIcon);
	AddChild(uppercaseButton);

	BButton* lowercaseButton = new BButton("Lowercase", new BMessage(M_TRANSFORM_LOWERCASE));
	AddChild(lowercaseButton);

	BButton* titlecaseButton = new BButton("Title Case", new BMessage(M_TRANSFORM_TITLE_CASE));
	AddChild(titlecaseButton);

	SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_TOP));
}