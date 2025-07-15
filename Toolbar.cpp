/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Toolbar.h"
#include "Constants.h"
#include <Bitmap.h>
#include <Catalog.h>
#include <File.h>
#include <IconUtils.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <Resources.h>
#include <SpaceLayoutItem.h>
#include <View.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Toolbar"


BToolBar*
CreateToolbar(BHandler* target)
{
	BToolBar* toolbar = new BToolBar(B_HORIZONTAL);

	toolbar->AddAction(new BMessage(M_FILE_NEW), target, ResourceToBitmap("NEW_ICON"),
		B_TRANSLATE("New"), "", false);
	toolbar->AddAction(new BMessage(M_FILE_OPEN), target, ResourceToBitmap("OPEN_ICON"),
		B_TRANSLATE("Open file" B_UTF8_ELLIPSIS), "", false);
	toolbar->AddAction(new BMessage(M_FILE_SAVE), target, ResourceToBitmap("SAVE_ICON"),
		B_TRANSLATE("Save file"), "", false);

	toolbar->GroupLayout()->AddItem(
		BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));
	toolbar->AddSeparator();
	toolbar->GroupLayout()->AddItem(
		BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));

	toolbar->AddAction(new BMessage(B_UNDO), target, ResourceToBitmap("UNDO_ICON"),
		B_TRANSLATE("Undo"), "", false);
	toolbar->AddAction(new BMessage(B_REDO), target, ResourceToBitmap("REDO_ICON"),
		B_TRANSLATE("Redo"), "", false);

	toolbar->GroupLayout()->AddItem(
		BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));
	toolbar->AddSeparator();
	toolbar->GroupLayout()->AddItem(
		BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));

	toolbar->AddAction(new BMessage(B_CUT), target, ResourceToBitmap("CUT_ICON"),
		B_TRANSLATE("Cut"), "", true);

	toolbar->AddAction(new BMessage(B_COPY), target, ResourceToBitmap("COPY_ICON"),
		B_TRANSLATE("Copy"), "", true);

	toolbar->AddAction(new BMessage(B_PASTE), target, ResourceToBitmap("PASTE_ICON"),
		B_TRANSLATE("Paste"), "", true);

	toolbar->GroupLayout()->AddItem(
		BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));
	toolbar->AddSeparator();
	toolbar->GroupLayout()->AddItem(
		BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));

	toolbar->AddAction(new BMessage(M_TOGGLE_WORD_WRAP), target, ResourceToBitmap("LINE_WRAP_ICON"),
		B_TRANSLATE("Word wrap"), "", false);

	toolbar->GroupLayout()->AddItem(
		BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));
	toolbar->AddSeparator();
	toolbar->GroupLayout()->AddItem(
		BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));

	toolbar->AddAction(new BMessage(M_TRANSFORM_UPPERCASE), target,
		ResourceToBitmap("UPPERCASE_ICON"), B_TRANSLATE("UPPERCASE"), "", false);
	toolbar->AddAction(new BMessage(M_TRANSFORM_LOWERCASE), target,
		ResourceToBitmap("LOWERCASE_ICON"), B_TRANSLATE("lowercase"), "", false);
	toolbar->AddAction(new BMessage(M_TRANSFORM_TITLE_CASE), target,
		ResourceToBitmap("TITLECASE_ICON"), B_TRANSLATE("Title Case"), "", false);
	toolbar->AddAction(new BMessage(M_TRANSFORM_CAPITALIZE), target,
		ResourceToBitmap("CAPITALIZE_ICON"), B_TRANSLATE("Capitalize"), "", false);

	toolbar->GroupLayout()->AddItem(
		BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));
	toolbar->AddSeparator();
	toolbar->GroupLayout()->AddItem(
		BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));

	toolbar->AddAction(new BMessage(M_TRANSFORM_ENCODE_URL), target,
		ResourceToBitmap("URL_ENCODE_ICON"), B_TRANSLATE("URL encode"), "", false);
	toolbar->AddAction(new BMessage(M_TRANSFORM_DECODE_URL), target,
		ResourceToBitmap("URL_DECODE_ICON"), B_TRANSLATE("URL decode"), "", false);
	toolbar->AddAction(new BMessage(M_TRANSFORM_ROT13), target, ResourceToBitmap("PUZZLE_ICON"),
		B_TRANSLATE("ROT13 encode/decode"), "", false);

	toolbar->AddGlue();

	toolbar->AddAction(new BMessage(M_SHOW_STATS), target, ResourceToBitmap("STATS_ICON"),
		B_TRANSLATE("Show stats" B_UTF8_ELLIPSIS), "", false);
	toolbar->AddSeparator();
	toolbar->AddAction(new BMessage(M_SHOW_SETTINGS), target, ResourceToBitmap("SETTINGS_ICON"),
		B_TRANSLATE("Settings" B_UTF8_ELLIPSIS), "", false);
	toolbar->AddAction(new BMessage(M_SHOW_HELP), target, ResourceToBitmap("HELP_ICON"),
		B_TRANSLATE("Help" B_UTF8_ELLIPSIS), "", false);

	return toolbar;
}


BBitmap*
ResourceToBitmap(const char* resName)
{
	image_info info;
	int32 cookie = 0;
	while (get_next_image_info(0, &cookie, &info) == B_OK) {
		BFile file(info.name, B_READ_ONLY);
		if (file.InitCheck() == B_OK) {
			BResources res(&file);
			size_t size;
			const void* data = res.LoadResource(B_VECTOR_ICON_TYPE, resName, &size);
			if (data) {
				BBitmap* icon = new BBitmap(BRect(0, 0, 23, 23), B_RGBA32);
				if (BIconUtils::GetVectorIcon((const uint8*)data, size, icon) == B_OK)
					return icon;
				delete icon;
			}
		}
		break; // stop after the first image
	}
	return nullptr;
}
