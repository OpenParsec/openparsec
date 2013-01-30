/*
 * PARSEC - CURSES Input Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:48 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001-2002
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
#include <stdio.h>
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"

// subsystem headers
#include "inp_defs_sv.h"

// local module header
#include "inp_main_sv.h"

// proprietary module headers
#include "con_main_sv.h"

// keyboard mapping -----------------------------------------------------------
//
#include "inp_keybmap_sv.h"

// Keyboard buffer ------------------------------------------------------------
//
PRIVATE dword INPS_KeybBuffer[ 2 + KEYB_BUFF_SIZ ];	

// global pointer to the keyboard buffer ( for console input ) ----------------
//
keybbuffer_s* KeybBuffer;


// init the input subsystem ---------------------------------------------------
//
PUBLIC
void INP_Init()
{
	memset( INPS_KeybBuffer, 0, sizeof( INPS_KeybBuffer ) );
	KeybBuffer = (keybbuffer_s*) &INPS_KeybBuffer;
}

// kill the input subsystem ---------------------------------------------------
//
PUBLIC
void INP_Kill()
{
}

// stuff key in internal keyboard buffer using repeatcount --------------------
//
PRIVATE
void _INP_HandleBufferedKey( int key, int nRepcount )
{
	dword *pos;
	
	for ( int nCount = 0; nCount < nRepcount; nCount++ ) {
		dword wpos = KeybBuffer->WritePos + 1;
		wpos = wpos % (KEYB_BUFF_SIZ);
		
		pos = &KeybBuffer->Data + wpos;
		
		// prevent overwrite
		if ( *pos == 0 ) {
			
			// map into KeybMap
			ASSERT( (dword)key < sizeof( INP_KeybMap ) );
			dword dwKey = INP_KeybMap[ key ];
			
			// write into keyboard buffer
			if ( dwKey != CKC_NIL ) {
				*(dword*)(&KeybBuffer->Data + KeybBuffer->WritePos) = dwKey;
				KeybBuffer->WritePos = wpos;
			}
		}
	}
}


// handle the input from CURSES -----------------------------------------------
//
PUBLIC
void INP_HandleInput()
{
	ASSERT( g_curses_in_win != NULL );
	int c = wgetch( g_curses_in_win );

	if ( c != ERR ) {
		_INP_HandleBufferedKey( c, 1 );		
	}
}



