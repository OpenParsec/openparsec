/*
 * PARSEC - Subsystem Independent Miscellaneous Drawing Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:21 $
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
#include "vid_defs.h"

// drawing subsystem
#include "d_iter.h"
#include "d_misc.h"

// mathematics header
#include "utl_math.h"

// proprietary module headers
#include "e_color.h"
#include "ro_api.h"
#include "ro_supp.h"


// draw a line in world space -------------------------------------------------
//
void D_LineWorld( const Vector3* start, const Vector3* end, const colrgba_s* color )
{
	IterLine3 itline;
	itline.NumVerts  = 2;
	
	itline.flags	 = ITERFLAG_LS_ANTIALIASED | ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_TO_DEPTH | ITERFLAG_NONDESTRUCTIVE;
	
	itline.itertype  = iter_rgba;
	itline.raststate = rast_zbuffer;
	itline.rastmask  = rast_nomask;
	
	itline.Vtxs[ 0 ].W	   = GEOMV_1;
	itline.Vtxs[ 0 ].R 	   = color->R;
	itline.Vtxs[ 0 ].G 	   = color->G;
	itline.Vtxs[ 0 ].B 	   = color->B;
	itline.Vtxs[ 0 ].A 	   = color->A;
	itline.Vtxs[ 0 ].flags = ITERVTXFLAG_NONE;
	
	itline.Vtxs[ 1 ].W	   = GEOMV_1;
	itline.Vtxs[ 1 ].R 	   = color->R;
	itline.Vtxs[ 1 ].G 	   = color->G;
	itline.Vtxs[ 1 ].B 	   = color->B;
	itline.Vtxs[ 1 ].A 	   = color->A;
	itline.Vtxs[ 1 ].flags = ITERVTXFLAG_NONE;

	MtxVctMUL( ViewCamera, start, (Vertex3*)&itline.Vtxs[ 0 ] );
	MtxVctMUL( ViewCamera, end,   (Vertex3*)&itline.Vtxs[ 1 ] );
	
	// setup transformation matrix
	D_LoadIterMatrix( NULL );

	D_DrawIterLine3( &itline, 0x00 );
}

// draw a frame of reference as a axis cross ----------------------------------
//
void D_FrameOfReference( const Xmatrx frame, const Vector3* transl )
{
	ASSERT( frame != NULL );

	Vector3 orig;
	Vector3 axis[ 3 ];
	FetchXVector( frame, &axis[ 0 ] );
	FetchYVector( frame, &axis[ 1 ] );
	FetchZVector( frame, &axis[ 2 ] );
	FetchTVector( frame, &orig );

	// scale axis
	VECMULS( &axis[ 0 ], &axis[ 0 ], INT_TO_GEOMV( 30 ) );
	VECMULS( &axis[ 1 ], &axis[ 1 ], INT_TO_GEOMV( 30 ) );
	VECMULS( &axis[ 2 ], &axis[ 2 ], INT_TO_GEOMV( 30 ) );

	VECADD( &axis[ 0 ], &axis[ 0 ], &orig );
	VECADD( &axis[ 1 ], &axis[ 1 ], &orig );
	VECADD( &axis[ 2 ], &axis[ 2 ], &orig );
	
	if ( transl != NULL ) {
		VECADD( &axis[ 0 ], &axis[ 0 ], transl );
		VECADD( &axis[ 1 ], &axis[ 1 ], transl );
		VECADD( &axis[ 2 ], &axis[ 2 ], transl );
		VECADD( &orig, &orig, transl );
	}

	colrgba_s color;
	SETRGBA( &color, 255, 0, 0, 255 );
	D_LineWorld( &orig, &axis[ 0 ], &color );
	SETRGBA( &color, 0, 255, 0, 255 );
	D_LineWorld( &orig, &axis[ 1 ], &color );
	SETRGBA( &color, 0, 0, 255, 255 );
	D_LineWorld( &orig, &axis[ 2 ], &color );
}

