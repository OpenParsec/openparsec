/*
 * PARSEC - Say/Talk Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:23 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-1998
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */ 

// C library
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "net_defs.h"

// local module header
#include "con_say.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux.h"
#include "con_main.h"


// textmacros (up to ten textmacros can be assigned and sent) -----------------
//
char text_macros[][ MAX_TEXTMACRO_LEN + 1 ] = {

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	""
};

// generic string paste area
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];

// string constants
static char text_not_sent[] 	= "text could not be sent.";
static char say_hint[]			= "use say? where ? is [0..9].";
static char set_hint[]			= "use set? where ? is [0..9].";
static char undef_macro[]		= "macro is undefined.";
static char you_said[]			= "you said: ";


// exec say macro (send preset string to all remote players) ------------------
//
PRIVATE
int ExecSayMacro( char *cstr )
{
	ASSERT( cstr != NULL );
	ASSERT( isdigit( *cstr ) );

	// display syntax if chars behind "say{digit}"
	if ( strtok( cstr + 1, " " ) != NULL ) {
		CON_AddLine( say_hint );
		return FALSE;
	}

	// retrieve macro text
	cstr = text_macros[ *cstr - '0' ];
	if ( *cstr == 0 ) {
		CON_AddLine( undef_macro );
		return FALSE;
	}

	// try to send text
	if ( !NetConnected || !NET_RmEvSendText( cstr ) ) {
		CON_AddLine( text_not_sent );
		return !NetConnected;
	}

	if ( con_in_talk_mode ) {

		// show text in talk area
		ShowSentText( LocalPlayerName, cstr, TALK_PUBLIC );
		CON_DisableLineFeed();

	} else {

		// show text normally
		ASSERT( ( strlen( you_said ) + strlen( cstr ) ) <= PASTE_STR_LEN );
		strcpy( paste_str, you_said );
		strcat( paste_str, cstr );
		CON_AddLine( paste_str );
	}

	return TRUE;
}


// exec say command (send arbitrary string to all remote players) -------------
//
int Cmd_SayText( char *cstr )
{
	//NOTE:
	// if the text is defined by a macro (SAY0, SAY1, ...)
	// it will be displayed in the console of the local
	// player so he will know what he said.
	// ("you said: ...")
	//
	// if the text is typed in directly, then it will
	// not be displayed since it is part of the command
	// (which is still visible) anyway.
	//
	// in talk mode everything will be displayed in the
	// talk area, however.

	ASSERT( cstr != NULL );

	// if digit immediately after command try macro
	if ( isdigit( *cstr ) ) {
		ExecSayMacro( cstr );
		return TRUE;
	}

	//NOTE:
	// in talkmode say without an argument is
	// allowed and an empty line will be sent.

	const char *scan = GetStringBehindCommand( cstr, con_in_talk_mode );

	// command invalid or argument missing?
	if ( scan == NULL )
		return TRUE;

	// try to send text
	if ( !NetConnected || !NET_RmEvSendText( scan ) ) {
		CON_AddLine( text_not_sent );
		return TRUE;
	}

	// show text in talk area
	if ( con_in_talk_mode ) {
		ShowSentText( LocalPlayerName, scan, TALK_PUBLIC );
		CON_DisableLineFeed();
	}

	return TRUE;
}


// exec set macro (set text macro) --------------------------------------------
//
int Cmd_SetTextMacro( char *cstr )
{
	ASSERT( cstr != NULL );

	// recognize only if digit behind "set"
	if ( !isdigit( *cstr ) )
		return FALSE;

	int indx   = *cstr - '0';
	const char *scan = GetStringBehindCommand( cstr + 1, FALSE );

	if ( scan == NULL )
		return TRUE;

	strncpy( text_macros[ indx ], scan, MAX_TEXTMACRO_LEN );
	text_macros[ indx ][ MAX_TEXTMACRO_LEN ] = 0;

	return TRUE;
}


// show text sent by remote player in console ---------------------------------
//
void ShowSentText( const char *name, const char *text, int mode )
{
	ASSERT( name != NULL );
	ASSERT( text != NULL );
	ASSERT( ( mode >= 0 ) && ( mode < NUM_TALK_MODES ) );

	//NOTE:
	// in talkmode the output of say strings cannot
	// be disabled.

	//NOTE:
	// if mode == TALK_STATUS, the name parameter of ShowSentText()
	// is ignored, but it should not be NULL, because of the ASSERT.

	if ( !AUX_DISABLE_CONSOLE_SAY_OUTPUT || con_in_talk_mode ) {

		// save current cursor pos and input line
		int curs_pos = cursor_x;
		strcpy( paste_str, con_lines[ con_bottom ] );

		if ( con_in_talk_mode ) {

			// print sent text in line where separator currently is
			int pline = ( con_talk_line - 1 ) & NUM_CONSOLE_LINES_MASK;

			const char *br_l = "";
			const char *br_r = br_l;

			// determine caller brackets
			if ( mode == TALK_PUBLIC ) {
				br_l = "<";
				br_r = "> ";
			} else if ( mode == TALK_PRIVATE ) {
				br_l = "*";
				br_r = "* ";
			} else {
				name = br_l;
			}

			// print name of talker and sent text
			strcpy( con_lines[ pline ], br_l );
			strcat( con_lines[ pline ], name );
			strcat( con_lines[ pline ], br_r );
			strcat( con_lines[ pline ], text );

			// scroll down lines to preserve
			if ( con_talk_line < con_bottom ) {

				int numl = con_bottom - con_talk_line;
				memmove( con_lines[ con_talk_line + 1 ],
						 con_lines[ con_talk_line ],
						 numl * ( MAX_CONSOLE_LINE_LENGTH + 1 ) );

			} else if ( con_talk_line > con_bottom ) {

				// with wraparound handling

				int numl = NUM_CONSOLE_LINES - con_talk_line - 1;
				memmove( con_lines[ 1 ], con_lines[ 0 ],
						 con_bottom * ( MAX_CONSOLE_LINE_LENGTH + 1 ) );
				memcpy( con_lines[ 0 ], con_lines[ NUM_CONSOLE_LINES - 1 ],
						MAX_CONSOLE_LINE_LENGTH + 1 );
				if ( numl > 0 )
					memmove( con_lines[ con_talk_line + 1 ],
							 con_lines[ con_talk_line ],
							 numl * ( MAX_CONSOLE_LINE_LENGTH + 1 ) );
			}

			// new separator
			CreateSeparatorLine( con_talk_line );
			CON_EnableLineFeed();

			// this actually increases here
			con_talk_line = ( con_talk_line + 1 ) & NUM_CONSOLE_LINES_MASK;

		} else {

			// output two lines (name and text)
			EraseConLine( con_bottom );
			strcpy( con_lines[ con_bottom ], name );
			strcat( con_lines[ con_bottom ], " says:" );
			CON_AddLine( text );
		}

		// restore cursor pos and input line
		CON_AddLine( paste_str );
		cursor_x = curs_pos;
	}
}



