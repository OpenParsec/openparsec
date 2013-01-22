/*
 * PARSEC - Mouse Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:27 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   2000
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
#ifdef NULL // XXX: NOT USED BY SDL!
// C library includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// svgalib headers
#include <vga.h>
#include <vgamouse.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "inp_defs.h"
#include "vid_defs.h"

// local module header
#include "il_mouse.h"

// proprietary module headers



// init mouse handling code ---------------------------------------------------
//
void INPs_MouseInitHandler()
{

/*
	mouse_init("/dev/psaux", MOUSE_IMPS2, MOUSE_DEFAULTSAMPLERATE);
	mouse_setxrange( 0, 639 );
	mouse_setyrange( 0, 479 );
	mouse_setwrap( MOUSE_NOWRAP );
*/
	MSGOUT( "Mouse code initialized." );
}


// de-init mouse handling code ------------------------------------------------
//
void INPs_MouseKillHandler()
{
	// not necessary
}


// set mouse coordinates independent of current screen resolution -------------
//
int INPs_MouseSetState( mousestate_s *state )
{
	ASSERT( state != NULL );

	return FALSE;
}


// get mouse coordinates independent of current screen resolution -------------
//
int INPs_MouseGetState( mousestate_s *state )
{
	ASSERT( state != NULL );

	return FALSE;
/*
	mouse_update();

	int xpos = mouse_getx();
	int ypos = mouse_gety();
	
	MSGOUT( "x: %d   y: %d", xpos, ypos );
	
	state->xpos = ( (float) xpos ) / ( 640.0f );
	state->ypos = ( (float) ypos ) / ( 480.0f );

	int buttons = mouse_getbutton();
	
	if ( buttons & MOUSE_LEFTBUTTON )
		state->buttons[ MOUSE_BUTTON_LEFT ] = MOUSE_BUTTON_PRESSED;
	else
		state->buttons[ MOUSE_BUTTON_LEFT ] = MOUSE_BUTTON_RELEASED;

	if ( buttons & MOUSE_MIDDLEBUTTON )
		state->buttons[ MOUSE_BUTTON_MIDDLE ] = MOUSE_BUTTON_PRESSED;
	else
		state->buttons[ MOUSE_BUTTON_MIDDLE ] = MOUSE_BUTTON_RELEASED;

	if ( buttons & MOUSE_RIGHTBUTTON )
		state->buttons[ MOUSE_BUTTON_RIGHT ] = MOUSE_BUTTON_PRESSED;
	else
		state->buttons[ MOUSE_BUTTON_RIGHT ] = MOUSE_BUTTON_RELEASED;
		
	return TRUE;
*/	
}


#endif
