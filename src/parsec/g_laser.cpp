/*
 * PARSEC - Laser Beam Weapon
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:36 $
 *
 * Orginally written by:
 *   Copyright (c) Michael Woegerbauer <maiki@parsec.org> 1999
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999
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

// drawing subsystem
#include "d_iter.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "g_laser.h"

// proprietary module headers
#include "con_info.h"
#include "e_callbk.h"
#include "e_supp.h"
#include "h_supp.h"
#include "obj_ctrl.h"
#include "obj_cust.h"
#include "obj_expl.h"



// preset laserbeam type properties values ------------------------------------
//
#define LASERBEAM_LIFETIME				300
#define LASERBEAM_WIDTH					3.0f
#define LASERBEAM_DELTA_X				0.0f
#define LASERBEAM_DELTA_Y				3.0f
#define LASERBEAM_DELTA_Z				5.0f
#define LASERBEAM_RANGE					10000.0f
#define LASERBEAM_ENERGY_CONSUMPTION	1
#define LASERBEAM_HITPOINTS_PER_FRAME	1
#define LASERBEAM_RED					49
#define LASERBEAM_GREEN					105
#define LASERBEAM_BLUE					245
#define LASERBEAM_ALPHA					255
#define LASERBEAM_TEXNAME				"laserb1.3df"
#define MAX_TEX_NAME					128
#define MIN_LASERBEAM_ENERGY			10


// message strings ------------------------------------------------------------
//
static char out_of_range_str[]			= "target out of range";
static char laserbeam_downed_ship_str[] = "laserbeam downed ship";
static char no_target_selected_str[]	= "no target selected";
static char low_energy_str[]			= "low energy";

extern int headless_bot;
// laserbeam custom type structure --------------------------------------------
//
struct LaserBeam : CustomObject {

	dword		OwnerObjno;		// needed for animation calculation
	dword		TargetObjno;
	char		texname[ MAX_TEX_NAME + 1 ];
	TextureMap *texmap;
//	int			lifetime;
	int			kill;
	int			energy_consumption;
	int			hitpoints_per_frame;
	geomv_t		width;			// width of laserbeam
	geomv_t		half_width;		// half of width of laserbeam
	geomv_t		delta_x;		// relative to owner object
	geomv_t		delta_y;
	geomv_t		delta_z;
	float		range;
	int			red;
	int			green;
	int			blue;
	int			alpha;
};

//#define OFS_LIFETIME				offsetof( LaserBeam, lifetime )
#define OFS_WIDTH					offsetof( LaserBeam, width )
#define OFS_TEXNAME					offsetof( LaserBeam, texname )
#define OFS_DELTA_X					offsetof( LaserBeam, delta_x )
#define OFS_DELTA_Y					offsetof( LaserBeam, delta_y )
#define OFS_DELTA_Z					offsetof( LaserBeam, delta_z )
#define OFS_RANGE					offsetof( LaserBeam, range )
#define OFS_ENERGY					offsetof( LaserBeam, energy_consumption )
#define OFS_HITPOINTS_PER_FRAME		offsetof( LaserBeam, hitpoints_per_frame )
#define OFS_RED						offsetof( LaserBeam, red )
#define OFS_GREEN					offsetof( LaserBeam, green )
#define OFS_BLUE					offsetof( LaserBeam, blue )
#define OFS_ALPHA					offsetof( LaserBeam, alpha )


// list of console-accessible properties --------------------------------------
//
PRIVATE
proplist_s LaserBeam_PropList[] = {

//	{ "lifetime",	OFS_LIFETIME,	0,	0xffff,		PROPTYPE_INT	},
	{ "width",		OFS_WIDTH,	 -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "texname",	OFS_TEXNAME,	0,		MAX_TEX_NAME,   PROPTYPE_STRING	},
	{ "delta.x",	OFS_DELTA_X, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "delta.y",	OFS_DELTA_Y, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "delta.z",	OFS_DELTA_Z, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "range",		OFS_RANGE,		0x0,	0x300000,		PROPTYPE_FLOAT	},
	{ "energyconsumption",	OFS_ENERGY,		0,	0xffff,		PROPTYPE_INT	},
	{ "hitpoints",	OFS_HITPOINTS_PER_FRAME,	0,	0xffff,	PROPTYPE_INT	},
	{ "red",		OFS_RED,		0,	0xff,		PROPTYPE_INT	},
	{ "green",		OFS_GREEN,		0,	0xff,		PROPTYPE_INT	},
	{ "blue",		OFS_BLUE,		0,	0xff,		PROPTYPE_INT	},
	{ "alpha",		OFS_ALPHA,		0,	0xff,		PROPTYPE_INT	},

	{ NULL,			0,			0,		0,				0				}
};


// assigned type id for laserbeam type ----------------------------------------
//
static dword laserbeam_type_id;


// type template for laserbeam ------------------------------------------------
//
static LaserBeam *laserbeam_type_template = NULL;


// macro to set the properties of a itervertex --------------------------------
//
#define SET_ITER_VTX( P_ITER_VTX, P_IN_VTX, P_OUT_VTX, P_VTX_U, P_VTX_V, VTX_RED, VTX_GREEN, VTX_BLUE, VTX_ALPHA ) \
		MtxVctMUL( SetIterXmatrx, (P_IN_VTX), (P_OUT_VTX) ); \
		(P_ITER_VTX)->X = (P_OUT_VTX)->X; \
		(P_ITER_VTX)->Y = (P_OUT_VTX)->Y; \
		(P_ITER_VTX)->Z = (P_OUT_VTX)->Z; \
		(P_ITER_VTX)->W = GEOMV_1; \
		(P_ITER_VTX)->U = *(P_VTX_U); \
		(P_ITER_VTX)->V = *(P_VTX_V); \
		(P_ITER_VTX)->R = VTX_RED; \
		(P_ITER_VTX)->G = VTX_GREEN; \
		(P_ITER_VTX)->B = VTX_BLUE; \
		(P_ITER_VTX)->A = VTX_ALPHA;


// draw laserbeam -------------------------------------------------------------
//
PRIVATE
int LaserBeam_Draw( void* param )
{
	if(headless_bot)
		return true;
	ASSERT( param != NULL );
	LaserBeam *laserbeam = (LaserBeam *) param;

	Vertex3   temp;
	Xmatrx    SetIterXmatrx;
	Vertex3   Pos, ViewPosOwner, ViewPosTarget, ViewPosOwnerScreen, ViewPosTargetScreen;
	geomv_t   u[ 4 ], v[ 4 ];
	Vertex3	  objvtxs[ 4 ];
	Vector3	  Diff;

	byte red, green, blue, alpha;

	if ( laserbeam->texmap == NULL ) {
//		MSGOUT( "texture '%s' was not found", laserbeam->texname );
		return FALSE;
	}

	GenObject *ownerpo = NULL;
	if ( laserbeam->OwnerObjno == 0 )
		ownerpo = MyShip;
	else
		ownerpo = FetchHostObject( laserbeam->OwnerObjno );
	
	if ( ownerpo == NULL )
		return FALSE;

	GenObject *targetpo = FetchHostObject( laserbeam->TargetObjno );
	if ( targetpo == NULL )
		return FALSE;

	// set vertex color
	red   = laserbeam->red;
	green = laserbeam->green;
	blue  = laserbeam->blue;
	alpha = laserbeam->alpha;

	// set texture coordinates
	u[ 0 ] = GEOMV_0;
	v[ 0 ] = GEOMV_0;

	u[ 1 ] = GEOMV_0;
	v[ 1 ] = INT_TO_GEOMV( 1L << laserbeam->texmap->Height );

	u[ 2 ] = INT_TO_GEOMV( 1L << laserbeam->texmap->Width );
	v[ 2 ] = INT_TO_GEOMV( 1L << laserbeam->texmap->Height );

	u[ 3 ] = INT_TO_GEOMV( 1L << laserbeam->texmap->Width );
	v[ 3 ] = GEOMV_0;

	IterRectangle3 itrect;

	itrect.flags	 = ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_DIV_UVW | ITERFLAG_Z_TO_DEPTH;
	itrect.itertype  = iter_texrgba | iter_specularadd;
	itrect.raststate = rast_zcompare | rast_texwrap | rast_chromakeyoff;
	itrect.rastmask  = rast_nomask;
	itrect.texmap	 = laserbeam->texmap;

	// position of owner in worldspace
	MtxMtxMUL( ViewCamera, ownerpo->ObjPosition, DestXmatrx );
	Pos.X = laserbeam->delta_x;
	Pos.Y = laserbeam->delta_y;
	Pos.Z = laserbeam->delta_z;
	MtxVctMUL( DestXmatrx, &Pos, &ViewPosOwner );

	// position of target in worldspace
	FetchTVector( targetpo->ObjPosition, &Pos );
	MtxVctMUL( ViewCamera, &Pos, &ViewPosTarget );

	//FIXME:
	// -------------------------------------------------------
	// integer screen coordinates and geomv_t's are intermixed
	// freely here. PROJECT_TO_SCREEN() also only initializes
	// fields X and Y (of an SPoint!!), but it is used for
	// a Vector3 and Z is used also! Visual C issues a warning
	// for the access of the uninitialized Z field.
	// has this ever worked???
	// -------------------------------------------------------

	// get difference vector in screenspace
	PROJECT_TO_SCREEN( ViewPosOwner, ViewPosOwnerScreen );
	PROJECT_TO_SCREEN( ViewPosTarget, ViewPosTargetScreen );
	Diff.X = ViewPosTargetScreen.X - ViewPosOwnerScreen.X;
	Diff.Y = ViewPosTargetScreen.Y - ViewPosOwnerScreen.Y;
	Diff.Z = GEOMV_0; //ViewPosTargetScreen.Z - ViewPosOwnerScreen.Z;

	// norm the x/y part of difference vector
	float x = GEOMV_TO_FLOAT( Diff.X );
	float y = GEOMV_TO_FLOAT( Diff.Y );

	float norm = sqrt( x*x + y*y );

	Diff.X = FLOAT_TO_GEOMV( x / norm );
	Diff.Y = FLOAT_TO_GEOMV( y / norm );

	// set rectangle vertices in viewspace
	objvtxs[ 0 ].X = ViewPosOwner.X + GEOMV_MUL( Diff.Y, laserbeam->half_width );
	objvtxs[ 0 ].Y = ViewPosOwner.Y - GEOMV_MUL( Diff.X, laserbeam->half_width );
	objvtxs[ 0 ].Z = ViewPosOwner.Z;
	objvtxs[ 1 ].X = ViewPosTarget.X + GEOMV_MUL( Diff.Y, laserbeam->half_width );
	objvtxs[ 1 ].Y = ViewPosTarget.Y - GEOMV_MUL( Diff.X, laserbeam->half_width );
	objvtxs[ 1 ].Z = ViewPosTarget.Z;
	objvtxs[ 2 ].X = ViewPosTarget.X - GEOMV_MUL( Diff.Y, laserbeam->half_width );
	objvtxs[ 2 ].Y = ViewPosTarget.Y + GEOMV_MUL( Diff.X, laserbeam->half_width );
	objvtxs[ 2 ].Z = ViewPosTarget.Z;
	objvtxs[ 3 ].X = ViewPosOwner.X - GEOMV_MUL( Diff.Y, laserbeam->half_width );
	objvtxs[ 3 ].Y = ViewPosOwner.Y + GEOMV_MUL( Diff.X, laserbeam->half_width );
	objvtxs[ 3 ].Z = ViewPosOwner.Z;

	// vertices already in viewspace
	MakeIdMatrx( SetIterXmatrx );

	SET_ITER_VTX( &itrect.Vtxs[ 0 ], &objvtxs[ 0 ], &temp, &u[ 0 ], &v[ 0 ], red, green, blue, alpha );
	SET_ITER_VTX( &itrect.Vtxs[ 1 ], &objvtxs[ 1 ], &temp, &u[ 1 ], &v[ 1 ], red, green, blue, alpha );
	SET_ITER_VTX( &itrect.Vtxs[ 2 ], &objvtxs[ 2 ], &temp, &u[ 2 ], &v[ 2 ], red, green, blue, alpha );
	SET_ITER_VTX( &itrect.Vtxs[ 3 ], &objvtxs[ 3 ], &temp, &u[ 3 ], &v[ 3 ], red, green, blue, alpha );

	// clip and draw polygon
	D_DrawIterRectangle3( &itrect, 0x3f );

	return TRUE;
}


// callback type and flags ----------------------------------------------------
//
static int callback_type = CBTYPE_DRAW_CUSTOM_ITER | CBFLAG_REMOVE;


// laserbeam animation callback -----------------------------------------------
//
PRIVATE
int LaserBeamAnimate( CustomObject *base )
{
	ASSERT( base != NULL );
	LaserBeam *laserbeam = (LaserBeam *) base;

	if ( laserbeam->kill == 1 ) {
		// stop the laser beam sound
		AUD_StopLaserBeam();

		return FALSE;
	}

	ShipObject *ownerpo  = (ShipObject *) FetchHostObject( laserbeam->OwnerObjno );
	if ( laserbeam->OwnerObjno == 0 )
		ownerpo = MyShip;
	ShipObject *targetpo = (ShipObject *) FetchHostObject( laserbeam->TargetObjno );

	// check if target destroyed
	if ( targetpo == NULL ) {
		// stop the laser beam sound
		AUD_StopLaserBeam();

		ownerpo = MyShip;
		ownerpo->WeaponsActive &= ~WPMASK_LASER_BEAM;
		return FALSE;
	}

	Vertex3 OwnerPos, TargetPos, Diff;
	geomv_t range;
	int		cur_energy_consumption;

	// calculate range
	FetchTVector( ownerpo->ObjPosition, &OwnerPos );
	FetchTVector( targetpo->ObjPosition, &TargetPos );

	Diff.X = TargetPos.X - OwnerPos.X;
	Diff.Y = TargetPos.Y - OwnerPos.Y;
	Diff.Z = TargetPos.Z - OwnerPos.Z;

	range = GEOMV_TO_FLOAT( GEOMV_MUL( Diff.X, Diff.X ) + GEOMV_MUL( Diff.Y, Diff.Y ) + GEOMV_MUL( Diff.Z, Diff.Z ) );

	// check range
	if ( range > ( laserbeam->range * laserbeam->range ) ) {
		// stop the laser beam sound
		AUD_StopLaserBeam();

		ownerpo->WeaponsActive &= ~WPMASK_LASER_BEAM;
		ShowMessage( out_of_range_str );
		return FALSE;
	}

	// check energy consumption
	cur_energy_consumption = ( CurScreenRefFrames * laserbeam->energy_consumption ) / 10;
	if ( ownerpo->CurEnergy < MIN_LASERBEAM_ENERGY + cur_energy_consumption ) {
		// stop the laser beam sound
		AUD_StopLaserBeam();

		ownerpo->WeaponsActive &= ~WPMASK_LASER_BEAM;
		ShowMessage( low_energy_str );
		AUD_LowEnergy();
		return FALSE;
	}
	else {
		ownerpo->CurEnergy -= cur_energy_consumption;
	}

	// damage if megashield not active
	if ( targetpo->MegaShieldAbsorption <= 0 ) {
		if ( targetpo == MyShip ) {
			targetpo->CurDamage += ( CurScreenRefFrames * laserbeam->hitpoints_per_frame ) / 10;

		} else if ( !NetConnected && ( targetpo->ExplosionCount == 0 ) ) {

			targetpo->CurDamage += ( CurScreenRefFrames * laserbeam->hitpoints_per_frame ) / 10;
			if ( targetpo->CurDamage > targetpo->MaxDamage ) {
				// schedule explosion
				LetShipExplode( targetpo );

				// needed for explosions caused by particles
//				shippo->DelayExplosion = 0;

				ownerpo->WeaponsActive &= ~WPMASK_LASER_BEAM;
				ShowMessage( laserbeam_downed_ship_str );
				AUD_ShipDestroyed( targetpo );

				// stop the laser beam sound
				AUD_StopLaserBeam();

				return FALSE;
			}
		}
	}

	// register the drawing callback for drawing the laserbeam
	CALLBACK_RegisterCallback( callback_type, LaserBeam_Draw, (void *) base );

	return TRUE;
}


// ----------------------------------------------------------------------------
//
PUBLIC
int KillLaserBeam( dword laserbeamobjno )
{
	LaserBeam *laserbeam = (LaserBeam *) FetchObject( laserbeamobjno );
	if ( laserbeam == NULL)
		return FALSE;

	laserbeam->kill = 1;

	return TRUE;
}


// ----------------------------------------------------------------------------
//
PUBLIC
int CreateLaserBeam( GenObject *ownerpo, dword targetobjno, dword *laserbeamobjno )
{
	ASSERT( ownerpo != NULL );

	Vertex3 OwnerPos, TargetPos, Diff;
	float range, cur_range;

	if ( targetobjno == TARGETID_NO_TARGET ) {
		ShowMessage( no_target_selected_str );
		return FALSE;
	}
	GenObject *targetpo = FetchHostObject( targetobjno );
	if ( targetpo == NULL ) {
		ShowMessage( no_target_selected_str );
		return FALSE;
	}

	// calculate range
	FetchTVector( ownerpo->ObjPosition, &OwnerPos );
	FetchTVector( targetpo->ObjPosition, &TargetPos );

	Diff.X = TargetPos.X - OwnerPos.X;
	Diff.Y = TargetPos.Y - OwnerPos.Y;
	Diff.Z = TargetPos.Z - OwnerPos.Z;

	cur_range = GEOMV_TO_FLOAT( GEOMV_MUL( Diff.X, Diff.X ) + GEOMV_MUL( Diff.Y, Diff.Y ) + GEOMV_MUL( Diff.Z, Diff.Z ) );

	if ( laserbeam_type_template != NULL )
		range = ( laserbeam_type_template->range * laserbeam_type_template->range );
	else
		range = LASERBEAM_RANGE * LASERBEAM_RANGE;

	// check range
	if ( cur_range > range ) {
		ShowMessage( out_of_range_str );
		return FALSE;
	}

	LaserBeam *laserbeam   = (LaserBeam *) CreateVirtualObject( laserbeam_type_id );
	if ( laserbeam == NULL ) {
		return FALSE;
	}
	laserbeam->OwnerObjno  = ownerpo->HostObjNumber;
	laserbeam->TargetObjno = targetobjno;

	*laserbeamobjno = laserbeam->ObjectNumber;

	// start the laser beam sound
	AUD_StartLaserBeam();

	return TRUE;
}


// init type fields with default values ---------------------------------------
//
PRIVATE
void LaserBeamInitDefaults( LaserBeam *laserbeam )
{
	ASSERT( laserbeam != NULL );

//	laserbeam->lifetime		= LASERBEAM_LIFETIME;
//	laserbeam->kill			= 0;
	laserbeam->width		= FLOAT_TO_GEOMV( LASERBEAM_WIDTH );

	strncpy( laserbeam->texname, LASERBEAM_TEXNAME, MAX_TEX_NAME );
	laserbeam->texname[ MAX_TEX_NAME ] = 0;

//	laserbeam->half_width	= GEOMV_MUL( LASERBEAM_WIDTH, FLOAT_TO_GEOMV( 0.5 ) );
	laserbeam->delta_x		= FLOAT_TO_GEOMV( LASERBEAM_DELTA_X );
	laserbeam->delta_y		= FLOAT_TO_GEOMV( LASERBEAM_DELTA_Y );
	laserbeam->delta_z		= FLOAT_TO_GEOMV( LASERBEAM_DELTA_Z );
	laserbeam->range		= LASERBEAM_RANGE;
	laserbeam->hitpoints_per_frame	= LASERBEAM_HITPOINTS_PER_FRAME;
	laserbeam->energy_consumption	= LASERBEAM_ENERGY_CONSUMPTION;
	laserbeam->red			= LASERBEAM_RED;
	laserbeam->green		= LASERBEAM_GREEN;
	laserbeam->blue			= LASERBEAM_BLUE;
	laserbeam->alpha		= LASERBEAM_ALPHA;
}


// type fields init function for laserbeam trail ---------------------------------
//
PRIVATE
void LaserBeamInitType( CustomObject *base )
{
	ASSERT( base != NULL );
	LaserBeam *laserbeam = (LaserBeam *) base;

	// init either from template or default values
	if ( !OBJ_InitFromCustomTypeTemplate( laserbeam, laserbeam_type_template ) ) {
		LaserBeamInitDefaults( laserbeam );
	}
}


// laserbeam constructor (class instantiation) --------------------------------
//
PRIVATE
void LaserBeamInstantiate( CustomObject *base )
{
	ASSERT( base != NULL );
	LaserBeam *laserbeam = (LaserBeam *) base;

	// get pointer to texture map
	laserbeam->texmap = FetchTextureMap( laserbeam->texname );
	if ( laserbeam->texmap == NULL ) {
		MSGOUT( "texture '%s' was not found", laserbeam->texname );
		return;
	}
	laserbeam->half_width	= GEOMV_MUL( laserbeam->width, FLOAT_TO_GEOMV( 0.5 ) );
	laserbeam->kill			= 0;
}


// laserbeam destructor (instance destruction) --------------------------------
//
PRIVATE
void LaserBeamDestroy( CustomObject *base )
{
	ASSERT( base != NULL );
	
	// ensure pending callbacks are destroyed to avoid
	// calling them with invalid pointers
	int numremoved = CALLBACK_DestroyCallback( callback_type, (void *) base );
	if(!headless_bot)
		ASSERT( numremoved <= 1 );
}


// register object type for laser beam ----------------------------------------
//
PRIVATE
void LaserBeamRegisterCustomType()
{
	custom_type_info_s info;
	memset( &info, 0, sizeof( info ) );

	// always try to allocate template
	laserbeam_type_template = (LaserBeam *) ALLOCMEM( sizeof( LaserBeam ) );
	if ( laserbeam_type_template != NULL ) {
		memset( laserbeam_type_template, 0, sizeof( LaserBeam ) );
		LaserBeamInitDefaults( laserbeam_type_template );
	}

	info.type_name			= "laserbeam";
	info.type_id			= 0x00000000;
	info.type_size			= sizeof( LaserBeam );
	info.type_template		= laserbeam_type_template;
	info.type_flags			= CUSTOM_TYPE_DEFAULT;
	info.callback_init		= LaserBeamInitType;
	info.callback_instant	= LaserBeamInstantiate;
	info.callback_destroy	= LaserBeamDestroy;
	info.callback_animate	= LaserBeamAnimate;
	info.callback_collide	= NULL;
	info.callback_notify	= NULL;
	info.callback_persist   = NULL;

	laserbeam_type_id = OBJ_RegisterCustomType( &info );
	CON_RegisterCustomType( info.type_id, LaserBeam_PropList );
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( G_LASER )
{
	// register type
	LaserBeamRegisterCustomType();
}



