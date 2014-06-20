/*
 * PARSEC - Planet Custom Object
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:36 $
 *
 * Orginally written by:
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

// C library
#include <math.h>
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
#include "aud_defs.h"
#include "net_defs.h"
#include "sys_defs.h"
#include "vid_defs.h"

// drawing subsystem
#include "d_font.h"
#include "d_iter.h"
#include "d_misc.h"

// rendering subsystem
#include "r_sfx.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "g_planet.h"

// proprietary module headers
#include "con_info.h"
#include "e_callbk.h"
#include "e_color.h"
#include "e_supp.h"
#include "g_supp.h"
#include "h_supp.h"
#include "net_serv.h"
#include "obj_clas.h"
#include "obj_creg.h"
#include "obj_ctrl.h"
#include "obj_cust.h"
#include "obj_xtra.h"
#include "part_api.h"


// flags
//#define ORBIT_ANIMATION
//#define SHIP_ORBIT_ANIMATION


// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// offset definitions into the Planet -----------------------------------------
//
#define OFS_ROTSPEED		offsetof( Planet, RotSpeed )
#define OFS_ORBITSPEED		offsetof( Planet, OrbitSpeed )
#define OFS_ORBITRADIUS		offsetof( Planet, OrbitRadius )
#define OFS_NAME			offsetof( Planet, Name )
#define OFS_HASRING			offsetof( Planet, HasRing )
#define OFS_RINGOUTERRADIUS	offsetof( Planet, RingOuterRadius )
#define OFS_RINGINNERRADIUS	offsetof( Planet, RingInnerRadius )
#define OFS_RINGTEXNAME		offsetof( Planet, RingTexName )


// list of console-accessible properties --------------------------------------
//
PRIVATE
proplist_s Planet_PropList[] = {


	{ "rotspeed",		 OFS_ROTSPEED,	 		0,			0xffff,			  PROPTYPE_INT,	   NULL	},
	{ "orbitspeed",		 OFS_ORBITSPEED,		0,			0xffff,			  PROPTYPE_INT,	   NULL	},
	{ "orbitradius",	 OFS_ORBITRADIUS, 		0x10000,	0x4000000,		  PROPTYPE_FLOAT,  NULL	},
	{ "name",			 OFS_NAME,		 		0,			MAX_PLANET_NAME,  PROPTYPE_STRING, NULL	},
	{ "hasring",		 OFS_HASRING, 	 		0x0,		0x1,		 	  PROPTYPE_INT,    NULL	},
	{ "ringouterradius", OFS_RINGOUTERRADIUS, 	0x10000,	0x4000000,	 	  PROPTYPE_FLOAT,  NULL	},
	{ "ringinnerradius", OFS_RINGINNERRADIUS, 	0x10000,	0x4000000,	 	  PROPTYPE_FLOAT,  NULL	},
	{ "ringtexname",	 OFS_RINGTEXNAME, 		0,			MAX_RING_TEXNAME, PROPTYPE_STRING, NULL	},

	{ NULL,				0,					0,			0,				  0,			   NULL	},
};


// type fields init function for planet ---------------------------------------
//
PRIVATE
void PlanetInitType( CustomObject *base )
{
	ASSERT( base != NULL );

	Planet *planet = (Planet *) base;

	planet->RotSpeed		= 0x0001;
	planet->CurOrbitPos		= 0;
	planet->OrbitSpeed		= 0x0001;
	planet->OrbitRadius		= 3000000;
	planet->OrbitParent 	= NULL;
	planet->HasRing		 	= FALSE;
	planet->RingOuterRadius = FLOAT_TO_GEOMV( 600.0f );
	planet->RingInnerRadius = FLOAT_TO_GEOMV( 300.0f );

	strncpy( planet->RingTexName, PLANET_RING_TEXTURE, MAX_RING_TEXNAME );
	planet->RingTexName[ MAX_RING_TEXNAME ] = 0;

	planet->RingTexture		= NULL;

	planet->NumOrbitShips	= 0;
}


static float  orbit_depth = 1.0f;


// draw planet rings ----------------------------------------------------------
//
void PlanetDraw_Rings( Planet *planet )
{
	// create vertex array
	IterArray3 *itarray = (IterArray3 *) ALLOCMEM(
		(size_t)&((IterArray3*)0)->Vtxs[ PLANET_RING_SEGMENTS * 2 ] );
	if ( itarray == NULL )
		OUTOFMEM( 0 );

	itarray->NumVerts	= PLANET_RING_SEGMENTS * 2;
	itarray->arrayinfo	= ITERARRAY_USE_COLOR |
						  ITERARRAY_USE_TEXTURE | ITERARRAY_GLOBAL_TEXTURE;
	itarray->flags		= ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_DIV_UVW |
						  ITERFLAG_Z_TO_DEPTH;
	itarray->itertype	= iter_texrgba | iter_alphablend;
	itarray->raststate	= rast_zcompare/*rast_zbuffer */| rast_texclamp |
						  rast_chromakeyoff;
	itarray->rastmask	= rast_nomask;
	itarray->texmap		= planet->RingTexture;

	bams_t angleoffs 	= BAMS_DEG360 / PLANET_RING_SEGMENTS;
	int seg = 0;
	for ( seg = 0; seg < PLANET_RING_SEGMENTS; seg++ ) {

		int vid = seg * 2;

		bams_t angle = seg * angleoffs;

		sincosval_s sincos;

		GetSinCos( angle, &sincos );

		byte r = 110;
		byte g = 80;
		byte b = 180;
		byte a = 180;

		// fill vertex array
		itarray->Vtxs[ vid ].X = GEOMV_MUL( planet->BoundingSphere + planet->RingOuterRadius, sincos.cosval );
		itarray->Vtxs[ vid ].Y = GEOMV_0;
		itarray->Vtxs[ vid ].Z = GEOMV_MUL( planet->BoundingSphere + planet->RingOuterRadius, sincos.sinval );
		itarray->Vtxs[ vid ].W = GEOMV_1;
		itarray->Vtxs[ vid ].U = 0;
		itarray->Vtxs[ vid ].V = 0;
		itarray->Vtxs[ vid ].R = r;
		itarray->Vtxs[ vid ].G = g;
		itarray->Vtxs[ vid ].B = b;
		itarray->Vtxs[ vid ].A = a;

		itarray->Vtxs[ vid + 1 ].X = GEOMV_MUL( planet->BoundingSphere + planet->RingInnerRadius, sincos.cosval );
		itarray->Vtxs[ vid + 1 ].Y = GEOMV_0;
		itarray->Vtxs[ vid + 1 ].Z = GEOMV_MUL( planet->BoundingSphere + planet->RingInnerRadius, sincos.sinval );
		itarray->Vtxs[ vid + 1 ].W = GEOMV_1;
		itarray->Vtxs[ vid + 1 ].U = 0;
		itarray->Vtxs[ vid + 1 ].V = PLANET_RING_TEX_HEIGHT;
		itarray->Vtxs[ vid + 1 ].R = r;
		itarray->Vtxs[ vid + 1 ].G = g;
		itarray->Vtxs[ vid + 1 ].B = b;
		itarray->Vtxs[ vid + 1 ].A = a;
	}

	size_t numtriindxs = PLANET_RING_SEGMENTS * 6;

	uint16 *vindxs = (uint16 *) ALLOCMEM( numtriindxs * sizeof( uint16 ) );
	if ( vindxs == NULL )
		OUTOFMEM( 0 );

	int dstindx = 0;
	int srcindx = 0;

	// store all strip indexes but last
	for ( seg = 0; seg < PLANET_RING_SEGMENTS - 1; seg++ ) {

		vindxs[ dstindx + 0 ] = srcindx;
		vindxs[ dstindx + 1 ] = srcindx + 1;
		vindxs[ dstindx + 2 ] = srcindx + 3;
		dstindx += 3;

		vindxs[ dstindx + 0 ] = srcindx;
		vindxs[ dstindx + 1 ] = srcindx + 2;
		vindxs[ dstindx + 2 ] = srcindx + 3;
		dstindx += 3;

		srcindx += 2;
	}

	// store last strip index
	vindxs[ dstindx + 0 ] = srcindx;
	vindxs[ dstindx + 1 ] = srcindx + 1;
	vindxs[ dstindx + 2 ] = 1;
	dstindx += 3;

	vindxs[ dstindx + 0 ] = srcindx;
	vindxs[ dstindx + 1 ] = 0;
	vindxs[ dstindx + 2 ] = 1;
	dstindx += 3;

	// calculate transformation matrix
	MtxMtxMUL( ViewCamera, planet->ObjPosition, DestXmatrx );

	// setup transformation matrix
	D_LoadIterMatrix( DestXmatrx );

	// lock array
	D_LockIterArray3( itarray, 0, itarray->NumVerts );

	// draw indexed triangles in a single call (no far-plane clipping!)
	D_DrawIterArrayIndexed(
		ITERARRAY_MODE_TRIANGLES, numtriindxs, vindxs, 0x3d );

	// unlock array
	D_UnlockIterArray();

	// restore identity transformation
	D_LoadIterMatrix( NULL );

	// free vertex index array
	FREEMEM( vindxs );
	
	// free vertex array
	FREEMEM( itarray );
}


// planet effect drawing callback ---------------------------------------------
//
int PlanetDraw( void *param )
{
	ASSERT( param != NULL );
	Planet *planet = (Planet *) param;

	// draw sphere
	R_DrawPlanet( planet );

	//TODO:
	// move all drawing together with a single interface.

	if ( planet->HasRing ) {
		//PlanetDraw_Rings( planet );
	}

	if ( orbit_depth == 1.0f ) {
		return FALSE;
	}

	colrgba_s glowcol;

	int val = (int)(255 * 4 - ( orbit_depth * 255 ) * 4);

	glowcol.R = ( val > 255 ) ? 255 : val;
	glowcol.G = 0;
	glowcol.B = 0;
	glowcol.A = glowcol.R;

	VIDs_SetScreenToColor( glowcol );

	orbit_depth = 1.0f;

	return TRUE;
}


// planet constructor (class instantiation) -----------------------------------
//
PRIVATE
void PlanetInstantiate( CustomObject *base )
{
	ASSERT( base != NULL );
	Planet *planet = (Planet *) base;

	planet->RingTexture = FetchTextureMap( planet->RingTexName );
	if ( planet->RingTexture == NULL ) {
		MSGOUT( "texture '%s' was not found.", planet->RingTexName );
	}
}


// callback type and flags ----------------------------------------------------
//
//static int callback_type = CBTYPE_DRAW_CUSTOM_ITER | CBFLAG_REMOVE;
static int callback_type = CBTYPE_DRAW_OBJECTS | CBFLAG_REMOVE;


// planet animation callback --------------------------------------------------
//
PRIVATE
int PlanetAnimate( CustomObject *base )
{
	ASSERT( base != NULL );
	Planet *planet = (Planet *) base;

	planet->CullMask = 0x00;		// no far plane clipping
	


	// simply rotate around Z
	ObjRotZ( planet->ObjPosition, planet->RotSpeed * CurScreenRefFrames );
	return true;

	// Oribit code to orbit around an object.
	// get orbit center object, center is (0,0,0) if object pointer is null
	Vector3 orbitcenter;

	if ( planet->OrbitParent == NULL ) {


		return true;
		/*
		orbitcenter.X = GEOMV_0;
		orbitcenter.Y = GEOMV_0;
		orbitcenter.Z = GEOMV_0;
		*/
	} else {

		orbitcenter.X = planet->ObjPosition[ 0 ][ 3 ];
		orbitcenter.Y = planet->ObjPosition[ 1 ][ 3 ];
		orbitcenter.Z = planet->ObjPosition[ 2 ][ 3 ];
	}

	planet->CurOrbitPos += ( planet->OrbitSpeed * CurScreenRefFrames );

	sincosval_s sincos;

	GetSinCos( planet->CurOrbitPos, &sincos );

	planet->ObjPosition[ 0 ][ 3 ] = orbitcenter.X + GEOMV_MUL( sincos.sinval, planet->OrbitRadius );
	planet->ObjPosition[ 1 ][ 3 ] = orbitcenter.Y + GEOMV_MUL( sincos.cosval, planet->OrbitRadius );
	planet->ObjPosition[ 2 ][ 3 ] = orbitcenter.Z;



	return TRUE;
}


// planet destructor (instance destruction) -----------------------------------
//
PRIVATE
void PlanetDestroy( CustomObject *base )
{
	ASSERT( base != NULL );
	Planet *planet = (Planet *) base;

	// ensure pending callbacks are destroyed to avoid
	// calling them with invalid pointers
	int numremoved = CALLBACK_DestroyCallback( callback_type, (void *) base );
	ASSERT( numremoved <= 1 );
}


// planet collision callback --------------------------------------------------
//
PRIVATE
int PlanetCollide( CustomObject *base )
{
	ASSERT( base != NULL );
	Planet *planet = (Planet *) base;

#if 0

	Vector3	diff;
	diff.X = MyShip->ObjPosition[ 0 ][ 3 ] - planet->ObjPosition[ 0 ][ 3 ];
	diff.Y = MyShip->ObjPosition[ 1 ][ 3 ] - planet->ObjPosition[ 1 ][ 3 ];
	diff.Z = MyShip->ObjPosition[ 2 ][ 3 ] - planet->ObjPosition[ 2 ][ 3 ];

	geomv_t orbit_dist = VctLenX( &diff );

	if ( orbit_dist <= ( planet->BoundingSphere + MyShip->BoundingSphere ) ) {

		//FIXME: this must be triggered by the server

		// trigger explosion
		KillDurationWeapons( MyShip );
		OBJ_CreateShipExtras( MyShip );

		AUD_PlayerKilled();
		ShipDowned = TRUE;

		return TRUE;
	}

	geomv_t maxorbitdistance = GEOMV_MUL( INT_TO_GEOMV( 10 ), MyShip->BoundingSphere );

	if ( orbit_dist <= ( planet->BoundingSphere + maxorbitdistance ) ) {

		if ( MyShip->Orbit != planet ) {

			ShowMessage( "entering orbit" );

			MyShip->Orbit = (GenObject *) planet;
			planet->NumOrbitShips++;
		}

		// orbit_depth == 1.0 if ship is in outermost orbit distance
		orbit_depth = GEOMV_TO_FLOAT( GEOMV_DIV( orbit_dist, planet->BoundingSphere + maxorbitdistance ) );

#ifdef SHIP_ORBIT_ANIMATION

		// move ship along orbit if speed is zero
		if ( MyShip->CurSpeed == 0 ) {

			float orbit_rot = asin( GEOMV_TO_FLOAT( GEOMV_DIV( diff.Z, orbit_dist ) ) );

			orbit_rot += 10 * CurScreenRefFrames;

			MSGOUT( "rot: %f", orbit_rot );

			sincosval_s sincos;

			GetSinCos( orbit_rot, &sincos );

			MSGOUT( "sin: %f, cos: %f", sincos.sinval, sincos.cosval );

			Xmatrx curobjpos;
			CalcOrthoInverse( ShipViewCamera, curobjpos );

			Vector3 offs, toffs;
			offs.X = ( planet->ObjPosition[ 0 ][ 3 ] + GEOMV_MUL( sincos.cosval, orbit_dist ) ) - curobjpos[ 0 ][ 3 ];
			offs.Y = GEOMV_0;
			offs.Z = ( planet->ObjPosition[ 2 ][ 3 ] + GEOMV_MUL( sincos.sinval, orbit_dist ) ) - curobjpos[ 2 ][ 3 ];

			// apply transformation
			MtxVctMULt( ShipViewCamera, &offs, &toffs );

			ShipViewCamera[ 0 ][ 3 ] -= INT_TO_GEOMV( GEOMV_TO_INT( toffs.X ) );
			ShipViewCamera[ 1 ][ 3 ] -= INT_TO_GEOMV( GEOMV_TO_INT( toffs.Y ) );
			ShipViewCamera[ 2 ][ 3 ] -= INT_TO_GEOMV( GEOMV_TO_INT( toffs.Z ) );

			PseudoStarMovement[ 0 ][ 3 ] -= INT_TO_GEOMV( GEOMV_TO_INT( toffs.X ) );
			PseudoStarMovement[ 1 ][ 3 ] -= INT_TO_GEOMV( GEOMV_TO_INT( toffs.Y ) );
			PseudoStarMovement[ 2 ][ 3 ] -= INT_TO_GEOMV( GEOMV_TO_INT( toffs.Z ) );

/*
			static bams_t orbit_rot = 0;

			sincosval_s sincos;

			GetSinCos( orbit_rot, &sincos );

			Xmatrx curobjpos;
			CalcOrthoInverse( ShipViewCamera, curobjpos );

			Vector3 offs, toffs;
			offs.X = ( planet->ObjPosition[ 0 ][ 3 ] + GEOMV_MUL( sincos.cosval, orbit_dist ) ) - curobjpos[ 0 ][ 3 ];
			offs.Y = GEOMV_0;
			offs.Z = ( planet->ObjPosition[ 2 ][ 3 ] + GEOMV_MUL( sincos.sinval, orbit_dist ) ) - curobjpos[ 2 ][ 3 ];

			// apply transformation
			MtxVctMULt( ShipViewCamera, &offs, &toffs );

			ShipViewCamera[ 0 ][ 3 ] -= INT_TO_GEOMV( GEOMV_TO_INT( toffs.X ) );
			ShipViewCamera[ 1 ][ 3 ] -= INT_TO_GEOMV( GEOMV_TO_INT( toffs.Y ) );
			ShipViewCamera[ 2 ][ 3 ] -= INT_TO_GEOMV( GEOMV_TO_INT( toffs.Z ) );

			PseudoStarMovement[ 0 ][ 3 ] -= INT_TO_GEOMV( GEOMV_TO_INT( toffs.X ) );
			PseudoStarMovement[ 1 ][ 3 ] -= INT_TO_GEOMV( GEOMV_TO_INT( toffs.Y ) );
			PseudoStarMovement[ 2 ][ 3 ] -= INT_TO_GEOMV( GEOMV_TO_INT( toffs.Z ) );

//			orbit_rot += CurScreenRefFrames;
*/

		}

#endif	// SHIP_ORBIT_ANIMATION

	} else {

		if ( MyShip->Orbit == planet ) {

			ShowMessage( "leaving orbit" );

			MyShip->Orbit = NULL;
			planet->NumOrbitShips--;
		}
	}

#endif

	// register the drawing callback for the red overlay
	CALLBACK_RegisterCallback( callback_type, PlanetDraw, (void *) base );

	return TRUE;
}


// register object type for planet --------------------------------------------
//
PRIVATE
void PlanetRegisterCustomType()
{
	custom_type_info_s info;
	memset( &info, 0, sizeof( info ) );

	info.type_name			= "planet";
	info.type_id			= 0x00000000;
	info.type_size			= sizeof( Planet );
	info.type_template		= NULL;
	info.type_flags			= CUSTOM_TYPE_DEFAULT;
	info.callback_init		= PlanetInitType;
	info.callback_instant	= PlanetInstantiate;
	info.callback_destroy	= PlanetDestroy;
	info.callback_animate	= PlanetAnimate;
	info.callback_collide	= PlanetCollide;
	info.callback_notify	= NULL;
	info.callback_persist   = NULL;

	OBJ_RegisterCustomType( &info );
	CON_RegisterCustomType( info.type_id, Planet_PropList );
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( G_PLANET )
{
	// register type
	PlanetRegisterCustomType();
}


