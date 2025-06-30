/*
 * Copyright 2025, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CONSTANTS_H
#define CONSTANTS_H

extern const char* kApplicationSignature;
extern const char* kApplicationName;

enum {
	// Text Transformations
	M_TRANSFORM_UPPERCASE              = 'upcs',
	M_TRANSFORM_LOWERCASE              = 'lwcs',
	M_TRANSFORM_CAPITALIZE             = 'cpts',
	M_TRANSFORM_TITLE_CASE             = 'tlcs',
	M_TRANSFORM_RANDOM_CASE            = 'rdcs',
	M_TRANSFORM_ALTERNATING_CASE       = 'altc',
	M_TRANSFORM_TOGGLE_CASE            = 'tgcs',
	M_TRANSFORM_ROT13                  = 'rt13',
	M_TRANSFORM_ENCODE_URL             = 'eurl',
	M_TRANSFORM_DECODE_URL             = 'durl',
	M_TRANSFORM_ENCODE_BASE64          = 'b64e',
	M_TRANSFORM_DECODE_BASE64          = 'b64d',
	M_TRANSFORM_PREFIX_SUFFIX          = 'psfx',
	M_TRANSFORM_REMOVE_PREFIX_SUFFIX   = 'rmpf',
	M_TRANSFORM_REPLACE                = 'repl',
	M_TRANSFORM_WIP                    = 'twip',

	// Line/Block Operations
	M_REMOVE_LINE_BREAKS               = 'rmlb',
	M_INSERT_LINE_BREAKS               = 'inlb',
	M_BREAK_LINES_ON_DELIMITER         = 'brdl',
	M_TRIM_LINES                       = 'trml',
	M_TRIM_EMPTY_LINES                 = 'trel',
	M_SORT_LINES                       = 'stln',
	M_REMOVE_DUPLICATES                = 'rmdp',
	M_INDENT_LINES                     = 'inln',
	M_UNINDENT_LINES                   = 'unln',

	// Line Break Mode Settings
	M_SET_LINEBREAK_MODE               = 'mset',
	M_MODE_REMOVE_ALL                  = 'mdra',
	M_MODE_REPLACE_LINE_BREAKS         = 'mdrl',
	M_MODE_BREAK_ON                    = 'mdbr',
	M_MODE_BREAK_AFTER_CHARS           = 'mdba',

	// File and Document Handling
	M_FILE_NEW                         = 'flnw',
	M_FILE_OPEN                        = 'flop',
	M_FILE_SAVE                        = 'flsv',
	M_FILE_SAVE_AS                     = 'flsa',
	M_INSERT_EXAMPLE_TEXT              = 'expl',

	// Settings
	M_SHOW_SETTINGS                    = 'stng',
	M_APPLY_SETTINGS                   = 'sapl',
	M_CLOSE_SETTINGS                   = 'scls',
	M_SETTINGS_SAVETEXT                = 'stxt',
	M_SETTINGS_SAVESETTINGS            = 'sset',
	M_SETTINGS_CLIPBOARD               = 'sclp',
	M_TOGGLE_WORD_WRAP                 = 'tgww',
	M_TOGGLE_APPLY_TO_SELECTION        = 'tgsl',

	// UI / Status
	M_UPDATE_STATUSBAR                 = 'stbr',
	M_SHOW_STATUS                      = 'stms',
	M_CLEAR_STATUS                     = 'clrs',

	//  Text View Events
	B_TEXT_CHANGED                     = 'txch',
	B_CURSOR_MOVED                     = 'curm',

	// Help & Feedback
	M_REPORT_A_BUG                     = 'rbug',
	M_SHOW_HELP                        = 'shlp',
	M_SHOW_STATS                       = 'stat'
};

enum BreakMode { BREAK_REMOVE_ALL = 0, BREAK_ON, BREAK_REPLACE, BREAK_AFTER_CHARS };

#endif // CONSTANTS_H
