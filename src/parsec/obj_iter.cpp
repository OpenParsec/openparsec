/*
 * PARSEC - Attached Iter Polygons
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:35 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999-2001
 *   Copyright (c) Michael Woegerbauer <maiki@parsec.org> 1999-2000
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
#include "od_class.h"

// global externals
#include "globals.h"

// drawing subsystem
#include "d_iter.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "obj_iter.h"

// proprietary module headers
#include "obj_clas.h"
#include "obj_creg.h"
#include "obj_game.h"
#include "obj_name.h"
#include "part_api.h"
#include "part_def.h"



// macro to ease dynamic vertex and triangle definition -----------------------
//
#define SET_VTX( n, X1, Y1, Z1, R1, G1, B1, A1 ) \
	itprim.Vtxs[ n ].X = FLOAT_TO_GEOMV( (X1) + (OBJITEROFS_X) ); \
	itprim.Vtxs[ n ].Y = FLOAT_TO_GEOMV( (Y1) + (OBJITEROFS_Y) ); \
	itprim.Vtxs[ n ].Z = FLOAT_TO_GEOMV( (Z1) + (OBJITEROFS_Z) ); \
	itprim.Vtxs[ n ].R = R1; \
	itprim.Vtxs[ n ].G = G1; \
	itprim.Vtxs[ n ].B = B1; \
	itprim.Vtxs[ n ].A = A1;


// limit length of thrust to maximum speed in case of active afterburner ------
//
#define GET_SPEED() \
	( ( shippo->CurSpeed == AFTERBURNER_SPEED ) ? \
			shippo->MaxSpeed : shippo->CurSpeed )


// yellow thrust polygons if afterburner active -------------------------------
//
#define CHECK_AFTERBURNER_COLOR() \
	if ( shippo->CurSpeed == AFTERBURNER_SPEED ) { \
		apex_r ^= apex_b; \
		apex_b ^= apex_r; \
		apex_r ^= apex_b; \
		base_r ^= base_b; \
		base_b ^= base_r; \
		base_r ^= base_b; \
	}


// fixed offset to hard-coded coordinate specification ------------------------
//
#define OBJITEROFS_X	0.0
#define OBJITEROFS_Y	0.0
#define OBJITEROFS_Z	0.0


// attach dynamic iter polygons to firebird -----------------------------------
//
PRIVATE
void AttachIterPolysFirebird( GenObject *obj, int inorder )
{
	ASSERT( obj != NULL );
	ASSERT( OBJECT_TYPE_SHIP( obj ) );

	// only immediately after the object is drawn
	if ( !inorder ) {
		return;
	}

	ShipObject *shippo = (ShipObject *) obj;

	// setup transformation matrix
	D_LoadIterMatrix( shippo->CurrentXmatrx );

	IterTriangle3 itprim;
	itprim.flags	 = ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_TO_DEPTH;
	itprim.itertype	 = iter_rgba | iter_additiveblend;
	itprim.raststate = rast_nozwrite;
	itprim.rastmask	 = rast_mask_zcompare;

	int		rand8 = RAND() % 20;
	float pbase = ( (float)GET_SPEED() / 2440.0 );
	float Param1 = -8.8 - pbase + ( RAND() % ( (int)pbase + 1 ) ) / 10.0;
	float Param2 = -8.8 - pbase + ( RAND() % ( (int)pbase + 1 ) ) / 10.0;
	float Param3 = -8.8 - pbase + ( RAND() % ( (int)pbase + 1 ) ) / 10.0;
	float Param4 = -8.8 - pbase + ( RAND() % ( (int)pbase + 1 ) ) / 10.0;

	byte apex_r = 10  + rand8;
	byte apex_g = 20  + rand8;
	byte apex_b = 205 + rand8;
	byte apex_a = 10  + rand8;

	byte base_r = 140 + rand8;
	byte base_g = 140 + rand8;
	byte base_b = 180 + rand8;
	byte base_a = 96 + rand8;

	// change color of thrust polygons if afterburner active
	CHECK_AFTERBURNER_COLOR();

	// first --------------------------------------------
	SET_VTX( 0, 10.05, -0.3,  -9.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,   11, -2.5, Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  7.6, -5.0,   -6.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, 23.1, -0.3,   -6.9, base_r, base_g, base_b, base_a );
	SET_VTX( 1,   11, -2.5, Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  7.6, -5.0,   -6.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, 23.1, -0.3,   -6.9, base_r, base_g, base_b, base_a );
	SET_VTX( 1,   11, -2.5, Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, 10.05, -0.3,  -9.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	// second --------------------------------------------
	SET_VTX( 0, 10.05, 0.3,   -9.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,   11,  2.5, Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  7.6,  5.0,   -6.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, 23.1, 0.3,   -6.9, base_r, base_g, base_b, base_a );
	SET_VTX( 1,   11, 2.5, Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  7.6, 5.0,   -6.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  23.1, 0.3,   -6.9, base_r, base_g, base_b, base_a );
	SET_VTX( 1,    11, 2.5, Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, 10.05, 0.3,   -9.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	// third ------
	SET_VTX( 0, -7.9, -0.3,   -8.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -8.0, -3.1, Param3, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -5.4, -5.0,   -6.9, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  -21, -0.3,   -6.9, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -8.0, -3.1, Param3, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -5.4, -5.0,   -6.9, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  -21, -0.3,   -6.9, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -8.0, -3.1, Param3, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -7.9, -0.3,   -8.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	// forth ------
	SET_VTX( 0, -7.9, 0.3,   -8.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -8.0, 3.1, Param4, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -5.4, 5.0,   -6.9, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  -21, 0.3,   -6.9, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -8.0, 3.1, Param4, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -5.4, 5.0,   -6.9, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  -21, 0.3,   -6.9, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -8.0, 3.1, Param4, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -7.9, 0.3,   -8.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	// restore identity transformation
	D_LoadIterMatrix( NULL );
}


// fixed offset to hard-coded coordinate specification ------------------------
//
#undef  OBJITEROFS_X
#undef  OBJITEROFS_Y
#undef  OBJITEROFS_Z
#define OBJITEROFS_X	0.0
#define OBJITEROFS_Y	0.0
#define OBJITEROFS_Z	0.0


// attach dynamic iter polygons to bluespire ----------------------------------
//
PRIVATE
void AttachIterPolysBluespire( GenObject *obj, int inorder )
{
	ASSERT( obj != NULL );
	ASSERT( OBJECT_TYPE_SHIP( obj ) );

	// only immediately after the object is drawn
	if ( !inorder ) {
		return;
	}

	ShipObject *shippo = (ShipObject *) obj;

	// setup transformation matrix
	D_LoadIterMatrix( shippo->CurrentXmatrx );

	IterTriangle3 itprim;
	itprim.flags	 = ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_TO_DEPTH;
	itprim.itertype	 = iter_rgba | iter_additiveblend;
	itprim.raststate = rast_nozwrite;
	itprim.rastmask	 = rast_mask_zcompare;

	int		rand8 = RAND() % 20;
	float pbase = ( (float)GET_SPEED() / 2440.0 );
	float Param1 = -14 - pbase + ( RAND() % ( (int)pbase + 1 ) ) / 10.0;
	float Param3 = -14 - pbase + ( RAND() % ( (int)pbase + 1 ) ) / 10.0;
	float Param2 = -15 - pbase + ( RAND() % ( (int)pbase + 1 ) ) / 10.0;

	byte apex_r = 20  + rand8;
	byte apex_g = 60  + rand8;
	byte apex_b = 210 + rand8;
	byte apex_a = 0   + rand8 / 2;

	byte base_r = 180 + rand8;
	byte base_g = 180 + rand8;
	byte base_b = 180 + rand8;
	byte base_a = 96 + rand8;

	// change color of thrust polygons if afterburner active
	CHECK_AFTERBURNER_COLOR();

	SET_VTX( 0, 3.4, -0.35, -12.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, 2.5,  -1.3,Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, 3.4, -2.25, -12.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  3.4, -0.35, -12.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  2.5,  -1.3,Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -3.5, -0.35, -12.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -2.6,  -1.3,Param3, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 1,  2.5,  -1.3,Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -3.5, -0.35, -12.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -3.5, -0.35, -12.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -2.6,  -1.3,Param3, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -3.5, -2.25, -12.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -2.6,  -1.3,Param3, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 1,  2.5,  -1.3,Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -3.5, -2.25, -12.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  3.4, -2.25, -12.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  2.5,  -1.3,Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -3.5, -2.25, -12.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	// right --------------------------------------------
	SET_VTX( 0, 17.7,  0.8, -13.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, 16.2,  1.9,Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, 17.7,  3.0, -13.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, 15.7,  0.0, -13.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, 16.2,  1.9,Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, 17.7,  0.8, -13.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, 15.7,  0.0, -13.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, 16.2,  1.9,Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, 14.2,  1.9, -13.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, 14.2,  1.9, -13.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, 16.2,  1.9,Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, 15.7,  3.8, -13.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, 17.7,  3.0, -13.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, 16.2,  1.9,Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, 15.7,  3.8, -13.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	// left --------------------------------------------
	SET_VTX( 0, -17.2,  0.8, -13.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -15.7,  1.9,Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -17.2,  3.0, -13.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -15.2,  0.0, -13.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -15.7,  1.9,Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -17.2,  0.8, -13.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -15.2,  0.0, -13.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -15.7,  1.9,Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -13.7,  1.9, -13.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -13.7,  1.9, -13.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -15.7,  1.9,Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -15.2,  3.8, -13.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -17.2,  3.0, -13.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -15.7,  1.9,Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -15.2,  3.8, -13.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	// restore identity transformation
	D_LoadIterMatrix( NULL );
}


// fixed offset to hard-coded coordinate specification ------------------------
//
#undef  OBJITEROFS_X
#undef  OBJITEROFS_Y
#undef  OBJITEROFS_Z
#define OBJITEROFS_X	0.0
#define OBJITEROFS_Y	0.0
#define OBJITEROFS_Z	0.0


// attach dynamic iter polygons to cormoran -----------------------------------
//
PRIVATE
void AttachIterPolysCormoran( GenObject *obj, int inorder )
{
	ASSERT( obj != NULL );
	ASSERT( OBJECT_TYPE_SHIP( obj ) );

	// only immediately after the object is drawn
	if ( !inorder ) {
		return;
	}

	ShipObject *shippo = (ShipObject *) obj;

	// setup transformation matrix
	D_LoadIterMatrix( shippo->CurrentXmatrx );

	IterTriangle3 itprim;
	itprim.flags	 = ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_TO_DEPTH;
	itprim.itertype	 = iter_rgba | iter_additiveblend /* | iter_specularadd*/;
	itprim.raststate = rast_nozwrite;
	itprim.rastmask	 = rast_mask_zcompare;

	int		rand8 = RAND() % 20;
	float pbase = ( (float)GET_SPEED() / 2440.0 );
	float Param = -6 - pbase + ( RAND() % ( (int)pbase + 1 ) ) / 10.0;

	byte apex_r = 20  + rand8;
	byte apex_g = 150 + rand8;
	byte apex_b = 235 + rand8;
	byte apex_a = 0   + rand8 / 2;

	byte base_r = 220 + rand8;
	byte base_g = 220 + rand8;
	byte base_b = 235 + rand8;
	byte base_a = 80 + rand8;

	// change color of thrust polygons if afterburner active
	CHECK_AFTERBURNER_COLOR();

	SET_VTX( 0,  1.0, -1.4,  -5.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  0.0, -3.0, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -1.0, -1.4,  -5.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  1.9, -3.0,  -5.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  0.0, -3.0, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  1.0, -1.4,  -5.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  1.0, -4.7,  -5.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  0.0, -3.0, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  1.9, -3.0,  -5.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -1.0, -4.7,  -5.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  0.0, -3.0, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  1.0, -4.7,  -5.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -1.9, -3.0,  -5.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  0.0, -3.0, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -1.0, -4.7,  -5.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -1.0, -1.4,  -5.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  0.0, -3.0, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -1.9, -3.0,  -5.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	// restore identity transformation
	D_LoadIterMatrix( NULL );
}


// fixed offset to hard-coded coordinate specification ------------------------
//
#undef  OBJITEROFS_X
#undef  OBJITEROFS_Y
#undef  OBJITEROFS_Z
#define OBJITEROFS_X	0.0
#define OBJITEROFS_Y	0.0
#define OBJITEROFS_Z	0.0


// attach dynamic iter polygons to stingray -----------------------------------
//
PRIVATE
void AttachIterPolysStingray( GenObject *obj, int inorder )
{
	ASSERT( obj != NULL );
	ASSERT( OBJECT_TYPE_SHIP( obj ) );

	// only immediately after the object is drawn
	if ( !inorder ) {
		return;
	}

	ShipObject *shippo = (ShipObject *) obj;

	// setup transformation matrix
	D_LoadIterMatrix( shippo->CurrentXmatrx );

	IterTriangle3 itprim;
	itprim.flags	 = ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_TO_DEPTH;
	itprim.itertype	 = iter_rgba | iter_additiveblend;
	itprim.raststate = rast_nozwrite;
	itprim.rastmask	 = rast_mask_zcompare;

	int		rand8  = RAND() % 20;
	float pbase  = ( (float)GET_SPEED() / 2440.0 );
	float Param1 = -26.0 - pbase + ( RAND() % ( (int)pbase + 1 ) ) / 10.0;
	float Param2 = -26.0 - pbase + ( RAND() % ( (int)pbase + 1 ) ) / 10.0;
	float Param3 = -10.0 - pbase + ( RAND() % ( (int)pbase + 1 ) ) / 10.0;
	float Param4 = -11.0 - pbase + ( RAND() % ( (int)pbase + 1 ) ) / 10.0;

	byte apex_r = 10  + rand8;
	byte apex_g = 20  + rand8;
	byte apex_b = 205 + rand8;
	byte apex_a = 10  + rand8;

	byte base_r = 140 + rand8;
	byte base_g = 140 + rand8;
	byte base_b = 180 + rand8;
	byte base_a = 96 + rand8;

	// change color of thrust polygons if afterburner active
	CHECK_AFTERBURNER_COLOR();

	// first ------------------------------------------------------------
	SET_VTX( 0, 15.0,  1.7,  -25.2, base_r, base_g, base_b, base_a );
	SET_VTX( 1, 15.5,  0.0, Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, 15.0, -1.7,  -25.2, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, 15.0, -1.7,  -25.2, base_r, base_g, base_b, base_a );
	SET_VTX( 1, 15.5,  0.0, Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, 17.9,  0.0,  -23.3, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, 17.9,  0.0,  -23.3, base_r, base_g, base_b, base_a );
	SET_VTX( 1, 15.5,  0.0, Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, 15.0,  1.7,  -25.2, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	// second -----------------------------------------------------------
	SET_VTX( 0, -15.0,  1.7,  -25.2, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -15.5,  0.0, Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -15.0, -1.7,  -25.2, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -15.0, -1.7,  -25.2, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -15.5,  0.0, Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -17.9,  0.0,  -23.3, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -17.9,  0.0,  -23.3, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -15.5,  0.0, Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -15.0,  1.7,  -25.2, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	// third ------------------------------------------------------------
	SET_VTX( 0, -9.4,  2.4,   -8.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -6.5,  0.0, Param3, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -9.4, -2.5,   -8.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -9.4, -2.5,   -8.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -6.5,  0.0, Param3, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  0.0, -3.1,   -8.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  0.0, -3.1,   -8.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -6.5,  0.0, Param3, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  0.0,  0.0, Param4, apex_r, apex_g, apex_b, apex_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  0.0,  0.0, Param4, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 1,  6.5,  0.0, Param3, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  0.0, -3.1,   -8.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  0.0, -3.1,   -8.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  6.5,  0.0, Param3, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  9.3, -2.5,   -8.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  9.3, -2.5,   -8.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  6.5,  0.0, Param3, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  9.3,  2.4,   -8.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  9.3,  2.4,   -8.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  6.5,  0.0, Param3, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  0.0,  3.0,   -8.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 2,  0.0,  3.0,   -8.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  6.5,  0.0, Param3, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 0,  0.0,  0.0, Param4, apex_r, apex_g, apex_b, apex_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 2,  0.0,  0.0, Param4, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 1, -6.5,  0.0, Param3, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 0,  0.0,  3.0,   -8.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  0.0,  3.0,   -8.8, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -6.5,  0.0, Param3, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -9.4,  2.4,   -8.8, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	// restore identity transformation
	D_LoadIterMatrix( NULL );
}


// fixed offset to hard-coded coordinate specification ------------------------
//
#undef  OBJITEROFS_X
#undef  OBJITEROFS_Y
#undef  OBJITEROFS_Z
#define OBJITEROFS_X	0.0
#define OBJITEROFS_Y	0.0
#define OBJITEROFS_Z	2.2


// attach dynamic iter polygons to claymore -----------------------------------
//
PRIVATE
void AttachIterPolysClaymore( GenObject *obj, int inorder )
{
	ASSERT( obj != NULL );
	ASSERT( OBJECT_TYPE_SHIP( obj ) );

	// only immediately after the object is drawn
	if ( !inorder ) {
		return;
	}

	ShipObject *shippo = (ShipObject *) obj;

	// setup transformation matrix
	D_LoadIterMatrix( shippo->CurrentXmatrx );

	IterTriangle3 itprim;
	itprim.flags	 = ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_TO_DEPTH;
	itprim.itertype	 = iter_rgba | iter_additiveblend;
	itprim.raststate = rast_nozwrite;
	itprim.rastmask	 = rast_mask_zcompare;

	int		rand8 = RAND() % 20;
	float pbase = ( (float)GET_SPEED() / 2440.0 );
	float Param1 = -30.0 - pbase + ( RAND() % ( (int)pbase + 1 ) ) / 10.0;
	float Param2 = -30.0 - pbase + ( RAND() % ( (int)pbase + 1 ) ) / 10.0;

	byte apex_r = 10  + rand8;
	byte apex_g = 20  + rand8;
	byte apex_b = 205 + rand8;
	byte apex_a = 10  + rand8;

	byte base_r = 140 + rand8;
	byte base_g = 140 + rand8;
	byte base_b = 180 + rand8;
	byte base_a = 96 + rand8;

	// change color of thrust polygons if afterburner active
	CHECK_AFTERBURNER_COLOR();

	// left --------------------------------------------------------------
	SET_VTX( 0, -11.0, -3.0,  -29.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -11.0,  0.0, Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  -9.0, -2.0,  -29.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  -9.0, -2.0,  -29.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -11.0,  0.0, Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  -8.0,  0.0,  -29.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  -8.0,  0.0,  -29.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -11.0,  0.0, Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  -9.0,  2.0,  -29.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  -9.0,  2.0,  -29.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -11.0,  0.0, Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -11.0,  3.0,  -29.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -11.0,  3.0,  -29.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -11.0,  0.0, Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -13.0,  2.0,  -29.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -13.0,  2.0,  -29.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -11.0,  0.0, Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -14.0,  0.0,  -29.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -14.0,  0.0,  -29.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -11.0,  0.0, Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -13.0, -2.0,  -29.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0, -13.0, -2.0,  -29.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -11.0,  0.0, Param1, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -11.0, -3.0,  -29.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	// right -------------------------------------------------------------
	SET_VTX( 0,  11.0, -3.0,  -29.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  11.0,  0.0, Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,   9.0, -2.0,  -29.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,   9.0, -2.0,  -29.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  11.0,  0.0, Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,   8.0,  0.0,  -29.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,   8.0,  0.0,  -29.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  11.0,  0.0, Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,   9.0,  2.0,  -29.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,   9.0,  2.0,  -29.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  11.0,  0.0, Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  11.0,  3.0,  -29.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  11.0,  3.0,  -29.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  11.0,  0.0, Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  13.0,  2.0,  -29.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  13.0,  2.0,  -29.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  11.0,  0.0, Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  14.0,  0.0,  -29.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  14.0,  0.0,  -29.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  11.0,  0.0, Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  13.0, -2.0,  -29.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	SET_VTX( 0,  13.0, -2.0,  -29.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  11.0,  0.0, Param2, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  11.0, -3.0,  -29.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterTriangle3( &itprim, 0x3f );

	// restore identity transformation
	D_LoadIterMatrix( NULL );
}


// fixed offset to hard-coded coordinate specification ------------------------
//
#undef  OBJITEROFS_X
#undef  OBJITEROFS_Y
#undef  OBJITEROFS_Z
#define OBJITEROFS_X	0.0
#define OBJITEROFS_Y	0.0
#define OBJITEROFS_Z	2.2


// remember the subobject transformation for post-rendering -------------------
//
static Xmatrx inorder_xmatrx;


// attach dynamic iter polygons to hurricane ----------------------------------
//
PRIVATE
void AttachIterPolysHurricane( GenObject *obj, int inorder )
{
	ASSERT( obj != NULL );
	ASSERT( OBJECT_TYPE_SHIP( obj ) );

	// only attach to base object
	if ( obj->ActiveVtxAnims > 0 ) {
		ASSERT( obj->VtxAnimStates != NULL );
		dword *baseinfo = (dword *) &obj->VtxAnimStates[ obj->NumVtxAnims ];
		if ( baseinfo[ 0 ] != obj->NumVtxAnims ) {
			return;
		}
	}

	// remember transformation for post-rendering the iter polys
	if ( inorder ) {
		memcpy( inorder_xmatrx, obj->CurrentXmatrx, sizeof( Xmatrx ) );
		return;
	}

	ShipObject *shippo = (ShipObject *) obj;

	// setup transformation matrix
	D_LoadIterMatrix( inorder_xmatrx );

	IterRectangle3 itprim;
	itprim.flags	 = ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_TO_DEPTH;
	itprim.itertype  = iter_rgba | iter_additiveblend;
	itprim.raststate = rast_nozwrite;
	itprim.rastmask	 = rast_mask_zcompare;

	int		rand8 = RAND() % 20;
	float pbase = ( (float)GET_SPEED() / 2440.0 );
	float Param = -30.0 - pbase + ( RAND() % ( (int)pbase + 1 ) ) / 10.0;

	byte apex_r = 10  + rand8;
	byte apex_g = 20  + rand8;
	byte apex_b = 205 + rand8;
	byte apex_a = 10  + rand8;

	byte base_r = 140 + rand8;
	byte base_g = 140 + rand8;
	byte base_b = 180 + rand8;
	byte base_a = 96 + rand8;

	// change color of thrust polygons if afterburner active
	CHECK_AFTERBURNER_COLOR();

	// upper thrust outlet -----------------------------------------------
	SET_VTX( 0, -6.5, -5.5, -30.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -6.5, -5.5, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -6.5, -3.5, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 3, -6.5, -3.5, -30.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterRectangle3( &itprim, 0x3f );

	SET_VTX( 0, -6.5, -5.5, -30.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -6.5, -5.5, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  6.5, -5.5, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 3,  6.5, -5.5, -30.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterRectangle3( &itprim, 0x3f );

	SET_VTX( 0,  6.5, -5.5, -30.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  6.5, -5.5, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  6.5, -3.5, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 3,  6.5, -3.5, -30.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterRectangle3( &itprim, 0x3f );

	SET_VTX( 0, -6.5, -3.5, -30.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -6.5, -3.5, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  6.5, -3.5, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 3,  6.5, -3.5, -30.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterRectangle3( &itprim, 0x3f );

	// lower thrust outlet -----------------------------------------------
	SET_VTX( 0, -6.5, 5.5, -30.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -6.5, 5.5, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2, -6.5, 3.5, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 3, -6.5, 3.5, -30.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterRectangle3( &itprim, 0x3f );

	SET_VTX( 0, -6.5, 5.5, -30.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -6.5, 5.5, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  6.5, 5.5, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 3,  6.5, 5.5, -30.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterRectangle3( &itprim, 0x3f );

	SET_VTX( 0,  6.5, 5.5, -30.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1,  6.5, 5.5, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  6.5, 3.5, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 3,  6.5, 3.5, -30.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterRectangle3( &itprim, 0x3f );

	SET_VTX( 0, -6.5, 3.5, -30.0, base_r, base_g, base_b, base_a );
	SET_VTX( 1, -6.5, 3.5, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 2,  6.5, 3.5, Param, apex_r, apex_g, apex_b, apex_a );
	SET_VTX( 3,  6.5, 3.5, -30.0, base_r, base_g, base_b, base_a );

	// clip and draw polygon
	D_DrawIterRectangle3( &itprim, 0x3f );

	// restore identity transformation
	D_LoadIterMatrix( NULL );
}


// fixed offset to hard-coded coordinate specification ------------------------
//
#undef  OBJITEROFS_X
#undef  OBJITEROFS_Y
#undef  OBJITEROFS_Z
#define OBJITEROFS_X	0.0
#define OBJITEROFS_Y	0.0
#define OBJITEROFS_Z	0.0


// iter polygon to object attachment functions --------------------------------
//
PRIVATE
attach_iterpolys_fpt functab_attach_iterpolys[ MAX_DISTINCT_OBJCLASSES ];


// attach dynamically generated iter polygons ---------------------------------
//
void OBJ_AttachIterPolygons( GenObject *obj, int inorder )
{
	ASSERT( obj != NULL  );
	ASSERT( obj->ObjectClass < (dword)NumObjClasses );

	//NOTE:
	// this function is called externally when an object is being rendered
	// at the correct time to render dynamically attached polygons.
	// e.g., by RO_OBJ::RO_RenderDynamicPolys().

	// call attachment function if registered
	if ( functab_attach_iterpolys[ obj->ObjectClass ] != NULL ) {

		D_BiasIterDepth( 1 );
		(*functab_attach_iterpolys[ obj->ObjectClass ])( obj, inorder );
		D_BiasIterDepth( 0 );
	}
}


// a single registered vertex set (one to many polys as a float list) ---------
//
struct attach_iterpolys_set_s {

	short		apexid;			// for clustering primitives to a single apex
	short		apexvtx;		// vertex index that is apex (in a quad second must follow)
	short		apexcomp;		// component to modulate with random value (usually z == 2)
	short		numprimvtxs;	// number of vertices per primitive

	int			numvtxs;		// number of vertices in list
	float*	vtxs;			// vertex list (set)
};


// table of all particles to attach to a specific object class ----------------
//
struct attach_iterpolys_tab_s {

	attach_iterpolys_set_s*	sets;
	short					numsets;
	short					maxsets;
};


// all registered iter polys for all object classes ---------------------------
//
PRIVATE
attach_iterpolys_tab_s attach_iterpolys_tab[ MAX_DISTINCT_OBJCLASSES ];


// maximum number of shared apexes --------------------------------------------
//
#define MAX_SHARED_APEXES		10


// attach previously registered iter polygons ---------------------------------
//
PRIVATE
void AttachIterPolysRegistered( GenObject *obj, int inorder )
{
	ASSERT( obj != NULL );
	ASSERT( OBJECT_TYPE_SHIP( obj ) );
	ASSERT( obj->ObjectClass < (dword)NumObjClasses );

	// only immediately after the object is drawn
	if ( !inorder ) {
		return;
	}

	dword objclass = obj->ObjectClass;
	ShipObject *shippo = (ShipObject *) obj;

	attach_iterpolys_set_s *ipset = attach_iterpolys_tab[ objclass ].sets;
	if ( ipset == NULL ) {
		return;
	}

	// setup transformation matrix
	D_LoadIterMatrix( shippo->CurrentXmatrx );

	IterRectangle3 _itprim;
	IterPolygon3   *itprim = (IterPolygon3 *) &_itprim;

	itprim->flags	  = ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_TO_DEPTH;
	itprim->itertype  = iter_rgba | iter_additiveblend;
	itprim->raststate = rast_nozwrite;
	itprim->rastmask  = rast_mask_zcompare;

	int		rand8 = RAND() % 20;
	float pbase = ( (float)GET_SPEED() / 2440.0 );

	float apexrands[ MAX_SHARED_APEXES ];
	for ( int crand = 0; crand < MAX_SHARED_APEXES; crand++ ) {
		apexrands[ crand ] = -pbase + ( RAND() % ( (int)pbase + 1 ) ) / 10.0;
	}

	byte apex_r = 10  + rand8;
	byte apex_g = 20  + rand8;
	byte apex_b = 205 + rand8;
	byte apex_a = 10  + rand8;

	byte base_r = 140 + rand8;
	byte base_g = 140 + rand8;
	byte base_b = 180 + rand8;
	byte base_a = 96 + rand8;

	// change color of thrust polygons if afterburner active
	CHECK_AFTERBURNER_COLOR();

	// draw all iter polys from all vertex sets
	for ( int acnt = attach_iterpolys_tab[ objclass ].numsets; acnt > 0; acnt--, ipset++ ) {

		ASSERT( ipset->apexvtx == 1 );
		ASSERT( ipset->apexcomp == 2 );

		//TODO:
		// use ipset->apexvtx and ipset->apexcomp.

		if ( ipset->numprimvtxs == 3 ) {

			float *vcpos = ipset->vtxs;
			for ( int numvtxs = ipset->numvtxs; numvtxs > 0; numvtxs-=3, vcpos+=9 ) {

				itprim->Vtxs[ 0 ].X = FLOAT_TO_GEOMV( vcpos[ 0 ] );
				itprim->Vtxs[ 0 ].Y = FLOAT_TO_GEOMV( vcpos[ 1 ] );
				itprim->Vtxs[ 0 ].Z = FLOAT_TO_GEOMV( vcpos[ 2 ] );
				itprim->Vtxs[ 0 ].R = base_r;
				itprim->Vtxs[ 0 ].G = base_g;
				itprim->Vtxs[ 0 ].B = base_b;
				itprim->Vtxs[ 0 ].A = base_a;

				itprim->Vtxs[ 1 ].X = FLOAT_TO_GEOMV( vcpos[ 3 ] );
				itprim->Vtxs[ 1 ].Y = FLOAT_TO_GEOMV( vcpos[ 4 ] );
				itprim->Vtxs[ 1 ].Z = FLOAT_TO_GEOMV( vcpos[ 5 ] + apexrands[ ipset->apexid ] );
				itprim->Vtxs[ 1 ].R = apex_r;
				itprim->Vtxs[ 1 ].G = apex_g;
				itprim->Vtxs[ 1 ].B = apex_b;
				itprim->Vtxs[ 1 ].A = apex_a;

				itprim->Vtxs[ 2 ].X = FLOAT_TO_GEOMV( vcpos[ 6 ] );
				itprim->Vtxs[ 2 ].Y = FLOAT_TO_GEOMV( vcpos[ 7 ] );
				itprim->Vtxs[ 2 ].Z = FLOAT_TO_GEOMV( vcpos[ 8 ] );
				itprim->Vtxs[ 2 ].R = base_r;
				itprim->Vtxs[ 2 ].G = base_g;
				itprim->Vtxs[ 2 ].B = base_b;
				itprim->Vtxs[ 2 ].A = base_a;

				// clip and draw polygon
				D_DrawIterTriangle3( (IterTriangle3 *)itprim, 0x3f );
			}

		} else if ( ipset->numprimvtxs == 4 ) {

			float *vcpos = ipset->vtxs;
			for ( int numvtxs = ipset->numvtxs; numvtxs > 0; numvtxs-=4, vcpos+=12 ) {

				itprim->Vtxs[ 0 ].X = FLOAT_TO_GEOMV( vcpos[ 0 ] );
				itprim->Vtxs[ 0 ].Y = FLOAT_TO_GEOMV( vcpos[ 1 ] );
				itprim->Vtxs[ 0 ].Z = FLOAT_TO_GEOMV( vcpos[ 2 ] );
				itprim->Vtxs[ 0 ].R = base_r;
				itprim->Vtxs[ 0 ].G = base_g;
				itprim->Vtxs[ 0 ].B = base_b;
				itprim->Vtxs[ 0 ].A = base_a;

				itprim->Vtxs[ 1 ].X = FLOAT_TO_GEOMV( vcpos[ 3 ] );
				itprim->Vtxs[ 1 ].Y = FLOAT_TO_GEOMV( vcpos[ 4 ] );
				itprim->Vtxs[ 1 ].Z = FLOAT_TO_GEOMV( vcpos[ 5 ] + apexrands[ ipset->apexid ] );
				itprim->Vtxs[ 1 ].R = apex_r;
				itprim->Vtxs[ 1 ].G = apex_g;
				itprim->Vtxs[ 1 ].B = apex_b;
				itprim->Vtxs[ 1 ].A = apex_a;

				itprim->Vtxs[ 2 ].X = FLOAT_TO_GEOMV( vcpos[ 6 ] );
				itprim->Vtxs[ 2 ].Y = FLOAT_TO_GEOMV( vcpos[ 7 ] );
				itprim->Vtxs[ 2 ].Z = FLOAT_TO_GEOMV( vcpos[ 8 ] + apexrands[ ipset->apexid ] );
				itprim->Vtxs[ 2 ].R = apex_r;
				itprim->Vtxs[ 2 ].G = apex_g;
				itprim->Vtxs[ 2 ].B = apex_b;
				itprim->Vtxs[ 2 ].A = apex_a;

				itprim->Vtxs[ 3 ].X = FLOAT_TO_GEOMV( vcpos[  9 ] );
				itprim->Vtxs[ 3 ].Y = FLOAT_TO_GEOMV( vcpos[ 10 ] );
				itprim->Vtxs[ 3 ].Z = FLOAT_TO_GEOMV( vcpos[ 11 ] );
				itprim->Vtxs[ 3 ].R = base_r;
				itprim->Vtxs[ 3 ].G = base_g;
				itprim->Vtxs[ 3 ].B = base_b;
				itprim->Vtxs[ 3 ].A = base_a;

				// clip and draw polygon
				D_DrawIterRectangle3( (IterRectangle3 *)itprim, 0x3f );
			}

		} else if ( ipset->numprimvtxs == 5 ) {

			//TODO:
			ASSERT( 0 );
		}
	}
}


// reset registered iter polys for specified class ----------------------------
//
void OBJ_ResetRegisteredClassIterPolys( dword objclass )
{
	ASSERT( objclass < (dword)NumObjClasses );

	functab_attach_iterpolys[ objclass ] = NULL;

	if ( attach_iterpolys_tab[ objclass ].sets != NULL ) {

		// free all allocated vertex lists (corresponding to sets)
		for ( int cset = 0; cset < attach_iterpolys_tab[ objclass ].numsets; cset++ ) {
			ASSERT( attach_iterpolys_tab[ objclass ].sets[ cset ].vtxs != NULL );
			FREEMEM( attach_iterpolys_tab[ objclass ].sets[ cset ].vtxs );
		}

		// free vertex sets
		FREEMEM( attach_iterpolys_tab[ objclass ].sets );
		attach_iterpolys_tab[ objclass ].sets = NULL;
	}
}


// maximum number of vertex sets that can be registered per class -------------
//
#define MAX_ATTACH_VERTEX_SETS	100


// register iter polys to attach to class (one call register a vertex set) ----
//
int OBJ_RegisterClassIterPolys( dword objclass, int apexid, int apexvtx, int apexcomp, int numprimvtxs, int numvtxs, float *vtxlist )
{
	ASSERT( objclass < (dword)NumObjClasses );
	ASSERT( vtxlist != NULL );

	// register general callback for this class if not already done
	if ( functab_attach_iterpolys[ objclass ] == NULL  ) {
		functab_attach_iterpolys[ objclass ] = AttachIterPolysRegistered;
	}

	// create new table of iter polys (vertex set) to attach if not yet there
	if ( attach_iterpolys_tab[ objclass ].sets == NULL ) {
		attach_iterpolys_tab[ objclass ].sets = (attach_iterpolys_set_s *)
			ALLOCMEM( MAX_ATTACH_VERTEX_SETS * sizeof( attach_iterpolys_set_s ) );
		if ( attach_iterpolys_tab[ objclass ].sets == NULL )
			OUTOFMEM( "no mem for iter poly vertex sets." );
		attach_iterpolys_tab[ objclass ].numsets = 0;
		attach_iterpolys_tab[ objclass ].maxsets = MAX_ATTACH_VERTEX_SETS;
	}

	// guard against overflow
	if ( attach_iterpolys_tab[ objclass ].numsets == attach_iterpolys_tab[ objclass ].maxsets ) {
		return FALSE;
	}

	// alloc vertex list in the exact size needed (source may be larger)
	float *_vtxlist = (float *) ALLOCMEM( numvtxs * 3 * sizeof( float ) );
	if ( _vtxlist == NULL )
		OUTOFMEM( 0 );
	memcpy( _vtxlist, vtxlist, numvtxs * 3 * sizeof( float ) );
	vtxlist = _vtxlist;

	// register this vertex set
	attach_iterpolys_set_s *ipset = &attach_iterpolys_tab[ objclass ].sets[ attach_iterpolys_tab[ objclass ].numsets ];

	ipset->apexid		= apexid;
	ipset->apexvtx		= apexvtx;
	ipset->apexcomp		= apexcomp;
	ipset->numprimvtxs	= numprimvtxs;
	ipset->numvtxs		= numvtxs;
	ipset->vtxs			= vtxlist;

	attach_iterpolys_tab[ objclass ].numsets++;

	return TRUE;
}


// register attachment function -----------------------------------------------
//
PRIVATE
int OBJ_RegisterAttachIterPolys( const char *classname, attach_iterpolys_fpt atfunc )
{
	ASSERT( classname != NULL );

	// try to resolve name to id
	dword classid = OBJ_FetchObjectClassId( classname );
	if ( classid == CLASS_ID_INVALID ) {
		return FALSE;
	}

	// double registration not allowed
	if ( functab_attach_iterpolys[ classid ] != NULL ) {
		return FALSE;
	}

	// set callback
	functab_attach_iterpolys[ classid ] = atfunc;

	return TRUE;
}


// register attachment functions for all accessible classes -------------------
//
PRIVATE
void RegisterAttachIterPolysCallback( int numparams, int *params )
{
	//NOTE:
	// must be idempotent.

	// ships
	OBJ_RegisterAttachIterPolys(
		OBJCLASSNAME_SHIP_FIREBIRD,  AttachIterPolysFirebird );

	OBJ_RegisterAttachIterPolys(
		OBJCLASSNAME_SHIP_BLUESPIRE, AttachIterPolysBluespire );

	OBJ_RegisterAttachIterPolys(
		OBJCLASSNAME_SHIP_CORMORAN,  AttachIterPolysCormoran );

	OBJ_RegisterAttachIterPolys(
		OBJCLASSNAME_SHIP_STINGRAY,  AttachIterPolysStingray );

	OBJ_RegisterAttachIterPolys(
		OBJCLASSNAME_SHIP_CLAYMORE,	 AttachIterPolysClaymore );

	OBJ_RegisterAttachIterPolys(
		OBJCLASSNAME_SHIP_HURRICANE, AttachIterPolysHurricane );
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( OBJ_ITER )
{
	// redundant
	for ( int cid = 0; cid < MAX_DISTINCT_OBJCLASSES; cid++ ) {
		functab_attach_iterpolys[ cid ] = NULL;
	}

	// register on creg
	OBJ_RegisterClassRegistration( RegisterAttachIterPolysCallback );
}



