/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Toolbar.h"
#include "MainWindow.h"
#include <Bitmap.h>
#include <File.h>
#include <IconUtils.h>
#include <Message.h>
#include <Resources.h>
#include <SpaceLayoutItem.h>
#include <View.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>

BToolBar* CreateToolbar(BHandler* target)
{
	BToolBar* toolbar = new BToolBar(B_HORIZONTAL);

	toolbar->AddAction(new BMessage(M_FILE_NEW), target, ResourceToBitmap("NEW_ICON"),
		"New file (Alt-N)", "", false);
	toolbar->AddAction(new BMessage(M_FILE_OPEN), target, ResourceToBitmap("OPEN_ICON"),
		"Open file (Alt-O)", "", false);
	toolbar->AddAction(new BMessage(M_FILE_SAVE), target, ResourceToBitmap("SAVE_ICON"),
		"Save file (Alt-S)", "", false);

	toolbar->GroupLayout()->AddItem(BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));
	toolbar->AddSeparator();
	toolbar->GroupLayout()->AddItem(BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));

	toolbar->AddAction(new BMessage(M_TOGGLE_WORD_WRAP), target, ResourceToBitmap("LINE_WRAP_ICON"),
		"Word wrap", "", true);

	toolbar->GroupLayout()->AddItem(BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));
	toolbar->AddSeparator();
	toolbar->GroupLayout()->AddItem(BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));

	toolbar->AddAction(new BMessage(M_TRANSFORM_UPPERCASE), target,
		ResourceToBitmap("UPPERCASE_ICON"), "UPPERCASE", "", false);
	toolbar->AddAction(new BMessage(M_TRANSFORM_LOWERCASE), target,
		ResourceToBitmap("LOWERCASE_ICON"), "lowercase", "", false);
	toolbar->AddAction(new BMessage(M_TRANSFORM_TITLE_CASE), target,
		ResourceToBitmap("TITLECASE_ICON"), "Title Case", "", false);
	toolbar->AddAction(new BMessage(M_TRANSFORM_CAPITALIZE), target,
		ResourceToBitmap("CAPITALIZE_ICON"), "Capitalize", "", false);
	toolbar->AddAction(new BMessage(M_TRANSFORM_TOGGLE_CASE), target,
		ResourceToBitmap("TOGGLE_ICON"), "Toggle case", "", false);

	toolbar->GroupLayout()->AddItem(BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));
	toolbar->AddSeparator();
	toolbar->GroupLayout()->AddItem(BSpaceLayoutItem::CreateHorizontalStrut(B_USE_HALF_ITEM_SPACING));

	toolbar->AddAction(new BMessage(M_TRANSFORM_ENCODE_URL), target,
		ResourceToBitmap("URL_ENCODE_ICON"), "URL encode", "", false);
	toolbar->AddAction(new BMessage(M_TRANSFORM_DECODE_URL), target,
		ResourceToBitmap("URL_DECODE_ICON"), "URL decode", "", false);
	toolbar->AddAction(new BMessage(M_TRANSFORM_ROT13), target,
		ResourceToBitmap("PUZZLE_ICON"), "ROT13 encode/decode", "", false);

	toolbar->AddGlue();

	toolbar->AddAction(new BMessage(M_TRANSFORM_WIP), target,
		ResourceToBitmap("SETTINGS_ICON"), "Settings" B_UTF8_ELLIPSIS, "", false);
	toolbar->AddAction(new BMessage(M_TRANSFORM_WIP), target,
		ResourceToBitmap("HELP_ICON"), "Help" B_UTF8_ELLIPSIS, "", false);

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