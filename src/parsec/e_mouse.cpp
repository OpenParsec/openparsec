/*
 * PARSEC - Mouse Support Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:22 $
 *
 * Orginally written by:
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1999-2000
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999-2000
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
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

// local module header
#include "e_mouse.h"

// proprietary module headers
#include "con_aux.h"
#include "e_callbk.h"
#include "e_draw.h"
#include "e_supp.h"



// calculate scaled area coordinates from reference definition ----------------
//
void MouseCalcScaledAreas( mouse_area_s *src, mouse_area_scaled_s *dst, int numareas )
{
	ASSERT( src != NULL );
	ASSERT( dst != NULL );

	for ( int aid = 0; aid < numareas; aid++ ) {

		dst[ aid ].xleft   = ( src[ aid ].xpos                     ) * Screen_Width  / 640;
		dst[ aid ].xright  = ( src[ aid ].xpos + src[ aid ].width  ) * Screen_Width  / 640;
		dst[ aid ].ytop    = ( src[ aid ].ypos                     ) * Screen_Height / 480;
		dst[ aid ].ybottom = ( src[ aid ].ypos + src[ aid ].height ) * Screen_Height / 480;
	}
}


// mouse cursor info ----------------------------------------------------------
//
#define	CURSOR_TEXWIDTH		64
#define CURSOR_TEXHEIGHT	64

#define CURSOR_SCREENWIDTH	48
#define CURSOR_SCREENHEIGHT	48

static char cursor_texname[] = "mouse";


// draw the mouse cursor ------------------------------------------------------
//
int DrawMouseCursor()
{
	if ( ( !InFloatingMenu || AUX_DISABLE_FLOATING_MENU_DRAWING ) && !InStarMap )
		return FALSE;

	if ( !AUX_ENABLE_MOUSE_FOR_MENU_SCREENS )
		return FALSE;

	mousestate_s mousestate;
	int mouseavailable = INPs_MouseGetState( &mousestate );

	// check if mouse is there and the custom cursor is enabled
	if ( !mouseavailable || !mousestate.drawcursor ) {
		return FALSE;
	}

	// acquire mouse cursor texture map once if already loaded
	static TextureMap *texmap = NULL;
	if ( texmap == NULL ) {
		texmap = FetchTextureMap( cursor_texname );
		if ( texmap == NULL ) {
			MSGOUT( "texture '%s' was not found", cursor_texname );
			return FALSE;
		}
	}

	texscreenrect_s rect;
	rect.itertype = iter_texrgba | iter_alphablend;
	rect.alpha    = 255;
	rect.x        = (int) ( mousestate.xpos * Screen_Width );
	rect.y        = (int) ( mousestate.ypos * Screen_Height );
	rect.w        = CURSOR_TEXWIDTH;
	rect.h        = CURSOR_TEXHEIGHT;
	rect.scaled_w = CURSOR_SCREENWIDTH;
	rect.scaled_h = CURSOR_SCREENHEIGHT;
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = texmap;

	DRAW_TexturedScreenRect( &rect, NULL );

	return TRUE;
}



