/*
 * PARSEC - Supporting Functions (SDL)
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:37 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1999
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
#include "config.h"

// C library
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
#include "aud_defs.h"
#include "sys_defs.h"
#include "vid_defs.h"

// drawing subsystem
#include "d_iter.h"

// local module header
#include "vsdl_suppo.h"
#include "vsdl_ogl.h"


// opengl headers
#include "r_gl.h"


// draw colored and alpha-blended rectangle that covers the entire screen -----
//
PRIVATE
void VSDL_ScreenRectangle( colrgba_s color, word itertype )
{

	IterRectangle2 itrect;

	itrect.Vtxs[ 0 ].X = RASTV_0;
	itrect.Vtxs[ 0 ].Y = RASTV_0;
	itrect.Vtxs[ 0 ].Z = RASTV_1;
	itrect.Vtxs[ 0 ].R = color.R;
	itrect.Vtxs[ 0 ].G = color.G;
	itrect.Vtxs[ 0 ].B = color.B;
	itrect.Vtxs[ 0 ].A = color.A;

	itrect.Vtxs[ 1 ].X = INT_TO_RASTV( Screen_Width );
	itrect.Vtxs[ 1 ].Y = RASTV_0;
	itrect.Vtxs[ 1 ].Z = RASTV_1;
	itrect.Vtxs[ 1 ].R = itrect.Vtxs[ 0 ].R;
	itrect.Vtxs[ 1 ].G = itrect.Vtxs[ 0 ].G;
	itrect.Vtxs[ 1 ].B = itrect.Vtxs[ 0 ].B;
	itrect.Vtxs[ 1 ].A = itrect.Vtxs[ 0 ].A;

	itrect.Vtxs[ 2 ].X = INT_TO_RASTV( Screen_Width );
	itrect.Vtxs[ 2 ].Y = INT_TO_RASTV( Screen_Height );
	itrect.Vtxs[ 2 ].Z = RASTV_1;
	itrect.Vtxs[ 2 ].R = itrect.Vtxs[ 0 ].R;
	itrect.Vtxs[ 2 ].G = itrect.Vtxs[ 0 ].G;
	itrect.Vtxs[ 2 ].B = itrect.Vtxs[ 0 ].B;
	itrect.Vtxs[ 2 ].A = itrect.Vtxs[ 0 ].A;

	itrect.Vtxs[ 3 ].X = RASTV_0;
	itrect.Vtxs[ 3 ].Y = INT_TO_RASTV( Screen_Height );
	itrect.Vtxs[ 3 ].Z = RASTV_1;
	itrect.Vtxs[ 3 ].R = itrect.Vtxs[ 0 ].R;
	itrect.Vtxs[ 3 ].G = itrect.Vtxs[ 0 ].G;
	itrect.Vtxs[ 3 ].B = itrect.Vtxs[ 0 ].B;
	itrect.Vtxs[ 3 ].A = itrect.Vtxs[ 0 ].A;

	itrect.flags	 = ITERFLAG_NONE;
	itrect.itertype  = itertype;
	itrect.raststate = rast_default;
	itrect.rastmask  = rast_nomask;
	itrect.texmap	 = NULL;

	D_DrawIterRectangle2( &itrect );
}


// fade in screen (palette) from black ----------------------------------------
//
int VIDs_FadeScreenFromBlack( int palno )
{
	return 1;
}


// fade screen (palette) to black ---------------------------------------------
//
int VIDs_FadeScreenToBlack( int palno, int fademusic )
{
	if ( fademusic )
		AUDs_MaxMusicVolume();

	// use black rectangle
	colrgba_s color = { 0, 0, 0, 0 };

	float subval = 0;
	while ( subval <= 255 ) {

		if ( fademusic )
			AUDs_FadeMusicVolume();
		
		SYSs_Yield();

		// fade alpha of rectangle
		color.A = (byte) subval;
		VSDL_ScreenRectangle( color, iter_rgba | iter_alphablend );
		VIDs_CommitRenderBuffer();

		subval += 4; // FIXME: doesn't take framerate into account
	}

	return 1;
}


// fade screen (palette) to black (palette referenced via pointer) ------------
//
int VIDs_FadePaletteToBlack( char *palette, int fademusic )
{
	// since no actual palette is involved here,
	// just use preceding function.
	VIDs_FadeScreenToBlack( 0, fademusic );

	return 1;
}


// fade in screen (palette) from white ----------------------------------------
//
int VIDs_FadeScreenFromWhite( int palno )
{
	return 1;
}


// set entire screen to single color ------------------------------------------
//
int VIDs_SetScreenToColor( colrgba_s col )
{
	// draw alpha-blended rectangle
	VSDL_ScreenRectangle( col, iter_rgba | iter_alphablend );

	return 1;
}


// set screen to dazzling white (whiteness depending on strength) -------------
//
int VIDs_SetScreenDazzle( float strength )
{
	// determine alpha of rectangle
	byte fadeval = (byte) strength;

	// use white rectangle
	colrgba_s color = { fadeval, fadeval, fadeval, 255 };

	// draw additive-blended rectangle (should be multiplicative, but Parsec doesn't do HDR rendering)
	VSDL_ScreenRectangle( color, iter_rgba | iter_additiveblend );

	return 1;
}


// set color index zero to specified color ------------------------------------
//
int VIDs_SetColIndexZero( int color )
{
	// only check for blue
	if ( color == COL_BACKGROUND_BLUE ) {

		// use translucent blue rectangle
		colrgba_s color_rgba = { 0, 0, 255, 100 };

		// draw alpha-blended rectangle
		VSDL_ScreenRectangle( color_rgba, iter_rgba | iter_alphablend );
	}

	return 1;
}


// set gamma correction value -------------------------------------------------
//
int VIDs_SetGammaCorrection( float gamma )
{
	// disabled because it causes visual glitches, isn't well-supported, and leaks gamma changes onto the desktop
	// SDL_SetWindowBrightness(curwindow, gamma);

	return TRUE;
}

