/*
 * PARSEC - Framerate Plot
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:25 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   1999
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

// drawing subsystem
#include "d_misc.h"

// local module header
#include "h_frmplt.h"

// proprietary module headers
#include "con_aux.h"
#include "e_callbk.h"



// ----------------------------------------------------------------------------
//
#define MAX_HORIZ_RESOLUTION			2880


// ----------------------------------------------------------------------------
//
static refframe_t	Frames[ MAX_HORIZ_RESOLUTION ];
static int			nCurrentIndex = 0;


// draw the framerate plot ----------------------------------------------------
//
int FramePlot_Draw( void* param )
{
	if ( AUX_ENABLE_REFFRAME_TIMEPLOT ) {
		Frames[ nCurrentIndex ] = CurScreenRefFrames;

		for( int i = 0; i < Screen_Width; i++ ) {

			int x = nCurrentIndex - i;
			if ( x < 0 )
				x = Screen_Width + x;

			int nHeight =  FLOAT2INT( INT2FLOAT( Screen_Height ) *  ( INT2FLOAT( Frames[ i ] ) / INT2FLOAT( FRAME_MEASURE_TIMEBASE ) ) );

			if ( nHeight > 0 ) {
				D_DrawVertBar( x, Screen_Height - 1 - nHeight, 0xffffff, nHeight );
			}
		}

		nCurrentIndex++;
		nCurrentIndex %= Screen_Width;
	}

	return TRUE;
}


// used by G_BOOT::BootFinish() -----------------------------------------------
//
PUBLIC time_t limit_timeval_old;
PUBLIC time_t limit_timeval_cur;


// register frame plot callbacks ----------------------------------------------
//
PRIVATE
void FRAMEPLOT_RegisterCallbacks()
{
	// specify callback type and flags
	int callbacktype = CBTYPE_DRAW_OVERLAYS | CBFLAG_PERSISTENT;

	// register the drawing callback for drawing frameplot
	CALLBACK_RegisterCallback( callbacktype, FramePlot_Draw, (void*) NULL );

	// init current time used by G_BOOT::BootFinish()
	limit_timeval_old = time( &limit_timeval_cur );
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( H_FRMPLT )
{
	for ( int i = 0; i < MAX_HORIZ_RESOLUTION; i++ )
		Frames[ i ] = 0;

	nCurrentIndex = 0;

	FRAMEPLOT_RegisterCallbacks();
}



