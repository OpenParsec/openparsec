/*
 * PARSEC - Electromagnetic Impulse Weapon Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:36 $
 *
 * Orginally written by:
 *   Copyright (c) Michael Woegerbauer <maiki@parsec.org> 2000-2001
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   2000
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

#include "net_defs.h"

#ifndef PARSEC_SERVER
// drawing subsystem
#include "d_iter.h"
#endif

// mathematics header
#include "utl_math.h"

// model header
//#include "utl_model.h"

// local module header
#include "g_emp.h"
#include "con_arg.h"


#ifndef PARSEC_SERVER
// proprietary module headers
#include "aud_defs.h"

#include "con_com.h"
#include "con_info.h"
#include "con_main.h"
#include "e_callbk.h"
#include "e_record.h"
#include "e_supp.h"
#include "h_supp.h"
#include "obj_expl.h"
#include "obj_ctrl.h"
#include "obj_game.h"
#else
#include "con_info_sv.h"
#include "con_main_sv.h"
#include "con_com_sv.h"
#endif

#include "obj_cust.h"

extern int headless_bot;

// assigned type id for emp type ----------------------------------------------
//
dword emp_type_id[ EMP_UPGRADES ];


// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// string constants -----------------------------------------------------------
//
static char emp_inval_lifetime_spec[]	= "lifetime invalid";
static char emp_inval_maxwidth_spec[]	= "maxwidth invalid";
static char emp_inval_lambda_spec[]		= "lambda invalid";
static char emp_inval_fadeout_spec[]	= "fadeout invalid";
static char emp_inval_waves_spec[]		= "waves invalid";
static char emp_inval_delay_spec[]		= "delay invalid";
static char emp_inval_energy_spec[]		= "energy invalid";
static char no_emp_str[]				= "no emp device";
static char low_energy_str[]			= "low energy";


// list of console-accessible properties --------------------------------------
//

// emp standard
PRIVATE
proplist_s Emp_PropList[] = {

	{ "texname",	OFS_TEXNAME,	0,	EMP_MAX_TEX_NAME,  	PROPTYPE_STRING	},
	{ "lod",		OFS_LOD,		2,			0xff,		PROPTYPE_INT	},
	{ "lat",		OFS_LAT,   0x0aaa,			0xffff,		PROPTYPE_INT	},
	{ "rot",		OFS_ROT,		0,			0xffff,		PROPTYPE_INT	},
	{ "red",		OFS_RED,		0,			0xff,		PROPTYPE_INT	},
	{ "green",		OFS_GREEN,		0,			0xff,		PROPTYPE_INT	},
	{ "blue",		OFS_BLUE,		0,			0xff,		PROPTYPE_INT	},
	{ "alpha",		OFS_ALPHA,		0,			0xff,		PROPTYPE_INT	},
	{ "damage",		OFS_DAMAGE,		0,			0xffff,		PROPTYPE_INT	},

	{ NULL,			0,			0,			0,			0				}
};


// emp upgrade level 1
PRIVATE
proplist_s EmpUp1_PropList[] = {

	{ "texname",	OFS_TEXNAME,	0,	EMP_MAX_TEX_NAME,  	PROPTYPE_STRING	},
	{ "lod",		OFS_LOD,		2,			0xff,		PROPTYPE_INT	},
	{ "lat",		OFS_LAT,   0x0aaa,			0xffff,		PROPTYPE_INT	},
	{ "rot",		OFS_ROT,		0,			0xffff,		PROPTYPE_INT	},
	{ "red",		OFS_RED,		0,			0xff,		PROPTYPE_INT	},
	{ "green",		OFS_GREEN,		0,			0xff,		PROPTYPE_INT	},
	{ "blue",		OFS_BLUE,		0,			0xff,		PROPTYPE_INT	},
	{ "alpha",		OFS_ALPHA,		0,			0xff,		PROPTYPE_INT	},
	{ "damage",		OFS_DAMAGE,		0,			0xffff,		PROPTYPE_INT	},

	{ NULL,			0,			0,			0,			0				}
};


// emp upgrade level 2
PRIVATE
proplist_s EmpUp2_PropList[] = {

	{ "texname",	OFS_TEXNAME,	0,	EMP_MAX_TEX_NAME,  	PROPTYPE_STRING	},
	{ "lod",		OFS_LOD,		2,			0xff,		PROPTYPE_INT	},
	{ "lat",		OFS_LAT,   0x0aaa,			0xffff,		PROPTYPE_INT	},
	{ "rot",		OFS_ROT,		0,			0xffff,		PROPTYPE_INT	},
	{ "red",		OFS_RED,		0,			0xff,		PROPTYPE_INT	},
	{ "green",		OFS_GREEN,		0,			0xff,		PROPTYPE_INT	},
	{ "blue",		OFS_BLUE,		0,			0xff,		PROPTYPE_INT	},
	{ "alpha",		OFS_ALPHA,		0,			0xff,		PROPTYPE_INT	},
	{ "damage",		OFS_DAMAGE,		0,			0xffff,		PROPTYPE_INT	},

	{ NULL,			0,			0,			0,			0				}
};




// draw emp -------------------------------------------------------------------
//
PRIVATE
int EmpDraw( void* param )
{
#ifndef PARSEC_SERVER
	if(headless_bot)
		return true;
	ASSERT( param != NULL );
	Emp *emp = (Emp *) param;

	ASSERT( emp->texmap != NULL );
	ASSERT( emp->alive < emp_lifetime[ emp->upgradelevel ] + emp->delay );

	// setup transformation matrix (emps are defined
	// in world-space, so transform is world->view)
	D_LoadIterMatrix( ViewCamera );

	// set vertex color
	byte red   = emp->red;
	byte green = emp->green;
	byte blue  = emp->blue;
	byte alpha = emp->alpha;

    int remaining = ( emp_lifetime[ emp->upgradelevel ] + emp->delay - emp->alive );
    if ( remaining <= emp_fadeout[ emp->upgradelevel ] ) {
        alpha = emp->alpha - ( ( emp->alpha * ( emp_fadeout[ emp->upgradelevel ] - remaining ) )
			/ emp_fadeout[ emp->upgradelevel ] );
    }

	COLOR_MUL( red,   red,   alpha );
	COLOR_MUL( green, green, alpha );
	COLOR_MUL( blue,  blue,  alpha );

	// ( emp->lod * 2 ) segments
	// plus the first segment extra
	// two vertices per segment
	int numverts = ( emp->lod * 2 + 1 ) * 2;

	IterTriStrip3 *itstrip = (IterTriStrip3 *)
		ALLOCMEM( (size_t)&((IterTriStrip3*)0)->Vtxs[ numverts ] );

	itstrip->flags		= ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_DIV_UVW | ITERFLAG_Z_TO_DEPTH;
	itstrip->NumVerts	= numverts;
	itstrip->itertype	= iter_texrgba | iter_specularadd;
	itstrip->raststate	= rast_zcompare | rast_texwrap | rast_chromakeyoff;
	itstrip->rastmask	= rast_nomask;
	itstrip->texmap		= emp->texmap;

	geomv_t vtx_u;
	geomv_t vtx_v;
	geomv_t tex_w_delt = FLOAT_TO_GEOMV( (float)( 1L << emp->texmap->Width ) / (float)emp->lod );
	geomv_t tex_h_delt = FLOAT_TO_GEOMV( (float)( 1L << emp->texmap->Height ) / (float)emp->lod );

	int latsegs = emp->lod;
	int lonsegs = emp->lod * 2 + 1;

	// fill in the strip vertices
	vtx_v = GEOMV_0;
	for ( int lat = 0; lat < latsegs; lat++ ) {

		vtx_u = GEOMV_0;
		for ( int lon = 0; lon < lonsegs; lon++ ) {
			// fadeout bottom edge
			if ( lat == 0 ) {
				SET_ITER_VTX( &itstrip->Vtxs[ lon * 2 ],
					&emp->WorldVtxs[ lat * lonsegs + lon ],
					vtx_u, vtx_v, 0, 0, 0, alpha );
			}
			else {
				SET_ITER_VTX( &itstrip->Vtxs[ lon * 2 ],
					&emp->WorldVtxs[ lat * lonsegs + lon ],
					vtx_u, vtx_v, red, green, blue, alpha );

			}

			// fadeout top edge
			if ( lat == ( latsegs - 1 ) ) {
				SET_ITER_VTX( &itstrip->Vtxs[ ( lon * 2 ) + 1 ],
					&emp->WorldVtxs[ ( lat + 1 ) * lonsegs + lon ],
					vtx_u, vtx_v + tex_h_delt, 0, 0, 0, alpha );
			}
			else {
				SET_ITER_VTX( &itstrip->Vtxs[ ( lon * 2 ) + 1 ],
					&emp->WorldVtxs[ ( lat + 1 ) * lonsegs + lon ],
					vtx_u, vtx_v + tex_h_delt, red, green, blue, alpha );
			}
			vtx_u += tex_w_delt;
		}

		// draw entire strip
		D_DrawIterTriStrip3( itstrip, 0x3f );

		vtx_v += tex_h_delt;
	}

	FREEMEM( itstrip );
	itstrip = NULL;

	// restore identity transformation
	D_LoadIterMatrix( NULL );
#endif
	return TRUE;
}

#ifndef PARSEC_SERVER
// callback type and flags ----------------------------------------------------
//
static int callback_type = CBTYPE_DRAW_CUSTOM_ITER | CBFLAG_REMOVE;
#else
static int callback_type = 0;
#endif


// ----------------------------------------------------------------------------
// SINGLE EMP WAVE FUNCTIONS
// ----------------------------------------------------------------------------


// single emp collision detection ---------------------------------------------
//
PRIVATE
int EmpShipCollision( Emp *emp, ShipObject* shippo )
{
	ASSERT( shippo != NULL );

	geomv_t empX = emp->WorldXmatrx[ 0 ][ 3 ];
	geomv_t empY = emp->WorldXmatrx[ 1 ][ 3 ];
	geomv_t empZ = emp->WorldXmatrx[ 2 ][ 3 ];

	geomv_t bsphere  = emp->BoundingSphere;
	geomv_t bsphere2 = GEOMV_MUL( bsphere, bsphere );

	geomv_t shipX = shippo->ObjPosition[ 0 ][ 3 ];
	geomv_t shipY = shippo->ObjPosition[ 1 ][ 3 ];
	geomv_t shipZ = shippo->ObjPosition[ 2 ][ 3 ];

	if ( shipX < ( empX - bsphere ) ) return FALSE;
	if ( shipX > ( empX + bsphere ) ) return FALSE;
	if ( shipY < ( empY - bsphere ) ) return FALSE;
	if ( shipY > ( empY + bsphere ) ) return FALSE;
	if ( shipZ < ( empZ - bsphere ) ) return FALSE;
	if ( shipZ > ( empZ + bsphere ) ) return FALSE;

	// do actual bounding sphere collision test
	Vector3 vecdist;
	vecdist.X = empX - shipX;
	vecdist.Y = empY - shipY;
	vecdist.Z = empZ - shipZ;

	geomv_t dist2 = DOT_PRODUCT( &vecdist, &vecdist );
	return( dist2 < bsphere2 );
}


// emp collision detection ----------------------------------------------------
//
PRIVATE
int CheckEmpCollision( CustomObject *base )
{
	ASSERT( base != NULL );
	Emp *emp = (Emp *) base;

#ifndef PARSEC_SERVER
	// check local ship
	if ( ( emp->Owner != OWNER_LOCAL_PLAYER ) && NetJoined &&
			EmpShipCollision( emp, MyShip ) ) {

 		OBJ_EventShipImpact( MyShip, TRUE );
		OBJ_ShipEmpDamage( MyShip, emp->Owner, emp->damage );
	}


	// check shiplist
	ShipObject *walkships = FetchFirstShip();
	for ( ; walkships; walkships = (ShipObject*) walkships->NextObj ) {
		// prevent collision with owner of emp
		if ( NetConnected && ( walkships->HostObjNumber == ShipHostObjId( emp->Owner ) ) )
			continue;

		if ( !EmpShipCollision( emp, walkships ) )
			continue;
		OBJ_EventShipImpact( walkships, TRUE );
		OBJ_ShipEmpDamage( walkships, emp->Owner, emp->damage );

    }
#endif
	return TRUE;
}


// emp animation callback -----------------------------------------------------
//
PRIVATE
int EmpAnimate( CustomObject *base )
{


	ASSERT( base != NULL );
	Emp *emp = (Emp *) base;
#ifndef PARSEC_SERVER
	// remove emp if no texture found
	if ( emp->texmap == NULL ) {
		// returning FALSE deletes the object
		return FALSE;
	}
	emp->alive += CurScreenRefFrames;
#else
	emp->alive += 10; // approx to the client... //TheSimulator->GetThisFrameRefFrames();
#endif

	// check emp expired
	if ( emp->alive >= ( emp_lifetime[ emp->upgradelevel ] + emp->delay ) ) {
		// returning FALSE deletes the object
		MSGOUT("alive is %d, lifetime is %d, delay is %d,  lifetime - delay is %d, delete object",
				emp->alive,
				emp_lifetime[emp->upgradelevel],
				emp->delay,
				( emp_lifetime[ emp->upgradelevel ] + emp->delay ));
		return FALSE;
	}

	// check emp delay
	if ( emp->alive < emp->delay ) {
		MSGOUT("alive is %d, delay is %d, no show, return", emp->alive, emp->delay);
		// do not show yet
		return TRUE;
	}
#ifndef PARSEC_SERVER
	// register the drawing callback for drawing the emp
	CALLBACK_RegisterCallback( callback_type, EmpDraw, (void*) base );
#endif

	ASSERT( ( emp->alive - emp->delay ) < emp_lifetime[ emp->upgradelevel ] );

	geomv_t sc = GEOMV_MUL( emp_max_width[ emp->upgradelevel ],
			FLOAT_TO_GEOMV( emp_expansion_tab[ emp->upgradelevel ][ emp->alive - emp->delay ] ) );

#ifdef PARSEC_SERVER

	GenObject *ownerpo = TheWorld->FetchObject(emp->OwnerHostObjno);
	if ( ownerpo != NULL ) {
			memcpy( emp->WorldXmatrx, ownerpo->ObjPosition, sizeof( Xmatrx ) );
	}
#else // PARSEC_CLIENT

	// FIXME: known bug!!!
	// FetchHostObject does not fetch anything if ( OwnerHostObjno == 0 ) and in
	// ObjectCamera-Mode

	// try to get ownerpo in first place, since it could have been destroyd intermediatly
	GenObject *ownerpo = FetchHostObject( emp->OwnerHostObjno );

	if ( ownerpo != NULL ) {
		memcpy( emp->WorldXmatrx, ownerpo->ObjPosition, sizeof( Xmatrx ) );
	} else if ( emp->ownerpo == MyShip ) {
		memcpy( emp->WorldXmatrx, MyShip->ObjPosition, sizeof( Xmatrx ) );
	} else {
		// owner-ship destroyed, emp does not change position any more
	}
#endif
	// animate emp object
	Xmatrx curmatrx;
	memcpy( curmatrx, emp->WorldXmatrx, sizeof( Xmatrx ) );

	// rotate emp
	ObjRotY( curmatrx, emp->rot * ( emp->alive - emp->delay ) );

	// scale emp
	Vertex3 instvtx;
	for ( int curvtx = 0; curvtx < emp->vtxsnr; curvtx++ ) {

		instvtx.X = GEOMV_MUL( sc, emp->ObjVtxs[ curvtx ].X );
		instvtx.Y = GEOMV_MUL( sc, emp->ObjVtxs[ curvtx ].Y );
		instvtx.Z = GEOMV_MUL( sc, emp->ObjVtxs[ curvtx ].Z );

		// transform vtxs into worldspace
		MtxVctMUL( curmatrx, &instvtx, &emp->WorldVtxs[ curvtx ] );
	}

	emp->BoundingSphere = sc;

 	return TRUE;
}


// create single emp wave object ----------------------------------------------
//
PRIVATE
void CreateEmp( GenObject *ownerpo, int delay, int alive, int upgradelevel, int nClientID )
{
	ASSERT( ownerpo != NULL );

#ifndef PARSEC_SERVER
	// create emp object
	Emp *emp = (Emp *) CreateVirtualObject( emp_type_id[ upgradelevel ] );
#else
	// FIXME: last arg is nClientID, need to figure out a way to pass that in or something.
	Emp *emp = (Emp *) TheWorld->CreateVirtualObject( emp_type_id[ upgradelevel ], nClientID );

#endif

	ASSERT( emp != NULL );

	if ( emp == NULL ) return;

#ifndef PARSEC_SERVER
	// get pointer to texture map
	emp->texmap = FetchTextureMap( emp->texname );
	if ( emp->texmap == NULL ) {
		MSGOUT( "emp texture '%s' invalid.", emp->texname );
		// emp will be deleted by animation callback
		return;
	}
#endif
	// needed, since FetchHostObject( emp->OwnerHostObjno ) does not return MyShip
	emp->ownerpo	= ownerpo;

#ifndef PARSEC_SERVER
	// does GetObjectOwner return ownernumber of local ship ?
	if ( ownerpo == MyShip ) {
		emp->OwnerHostObjno	= ( LocalPlayerId << 16 );
		emp->Owner = OWNER_LOCAL_PLAYER;
	}
	else {
		emp->OwnerHostObjno	= ownerpo->HostObjNumber;
		emp->Owner = GetObjectOwner( ownerpo );
	}
#else
	emp->OwnerHostObjno	= ownerpo->HostObjNumber;
	emp->Owner = nClientID; //GetObjectOwner( ownerpo );
#endif
	memcpy( emp->WorldXmatrx, ownerpo->ObjPosition, sizeof( Xmatrx ) );

	// emp->lod = lod;

	// emp->lod latitude segments, first needs two rings of vertices
	int numlatsegs = emp->lod + 1;

	// ( emp->lod * 2 ) longitude segments, first counts double
	int numlonsegs = emp->lod * 2 + 1;

	emp->alive	= alive;
	emp->delay	= delay;
	emp->vtxsnr = numlonsegs * numlatsegs;

	// allocate emp object structure memory
	emp->ObjVtxs = (Vertex3 *) ALLOCMEM( sizeof( Vertex3 ) * emp->vtxsnr * 2 );
	if ( emp->ObjVtxs == NULL ) {
		OUTOFMEM( "no mem for emp." );
	}
	emp->WorldVtxs = &emp->ObjVtxs[ emp->vtxsnr ];

	// build emp object structure
	for ( int latseg = 0; latseg < numlatsegs; latseg++ ) {

		bams_t lat = emp->lat - ( ( emp->lat * 2 * latseg ) / ( numlatsegs - 1 ) );

		sincosval_s latirp;
		GetSinCos( lat, &latirp );

		for ( int lonseg = 0; lonseg < numlonsegs; lonseg++ ) {

			bams_t lon = ( BAMS_DEG360 * lonseg ) / ( numlonsegs - 1 );

			sincosval_s longrp;
			GetSinCos( lon, &longrp );

			int curidx = latseg * numlonsegs + lonseg;
			emp->ObjVtxs[ curidx ].X = GEOMV_MUL( longrp.cosval, latirp.cosval );
			emp->ObjVtxs[ curidx ].Y = latirp.sinval;
			emp->ObjVtxs[ curidx ].Z = GEOMV_MUL( longrp.sinval, latirp.cosval );
		}
	}

//	AUD_Emp( shippo );
}


// template for default values of fields; may be altered from the console -----
//
static Emp *emp_type_template[ EMP_UPGRADES ] = {
	NULL, NULL, NULL,
};


// init type fields with default values ---------------------------------------
//
PRIVATE
void EmpInitDefaults( Emp *emp, int upgradelevel )
{
	ASSERT( emp != NULL );
	ASSERT( ( upgradelevel < EMP_UPGRADES ) && ( upgradelevel >= 0 ) );

	emp->upgradelevel = upgradelevel;

	switch ( upgradelevel ) {
		case 0:
			strncpy( emp->texname, EMP_TEXNAME, EMP_MAX_TEX_NAME );
			emp->texname[ EMP_MAX_TEX_NAME ] = 0;
			emp->lat		= EMP_LAT;
			emp->lod		= EMP_LOD;
			emp->rot		= EMP_ROT;
			emp->red		= EMP_RED;
			emp->green		= EMP_GREEN;
			emp->blue		= EMP_BLUE;
			emp->alpha		= EMP_ALPHA;
			emp->damage		= EMP_DAMAGE;
			break;
		case 1:
			strncpy( emp->texname, EMP_UP1_TEXNAME, EMP_MAX_TEX_NAME );
			emp->texname[ EMP_MAX_TEX_NAME ] = 0;
			emp->lat		= EMP_UP1_LAT;
			emp->lod		= EMP_UP1_LOD;
			emp->rot		= EMP_UP1_ROT;
			emp->red		= EMP_UP1_RED;
			emp->green		= EMP_UP1_GREEN;
			emp->blue		= EMP_UP1_BLUE;
			emp->alpha		= EMP_UP1_ALPHA;
			emp->damage		= EMP_UP1_DAMAGE;
			break;
		case 2:
			strncpy( emp->texname, EMP_UP2_TEXNAME, EMP_MAX_TEX_NAME );
			emp->texname[ EMP_MAX_TEX_NAME ] = 0;
			emp->lat		= EMP_UP2_LAT;
			emp->lod		= EMP_UP2_LOD;
			emp->rot		= EMP_UP2_ROT;
			emp->red		= EMP_UP2_RED;
			emp->green		= EMP_UP2_GREEN;
			emp->blue		= EMP_UP2_BLUE;
			emp->alpha		= EMP_UP2_ALPHA;
			emp->damage		= EMP_UP2_DAMAGE;
			break;
		default:
			ASSERT( 0 );
	}

	emp->ObjVtxs	= NULL;
	emp->WorldVtxs	= NULL;
	emp->BoundingSphere = GEOMV_0;
}


// type fields init function for emp ------------------------------------------
//
PRIVATE
void EmpInitType( CustomObject *base )
{
	ASSERT( base != NULL );
	Emp *emp = (Emp *) base;

	// init either from template or default values
	if ( !OBJ_InitFromCustomTypeTemplate( emp, emp_type_template[ 0 ] ) ) {
		EmpInitDefaults( emp, 0 );
	}
}


PRIVATE
void EmpInitTypeUp1( CustomObject *base )
{
	ASSERT( base != NULL );
	Emp *emp = (Emp *) base;

	// init either from template or default values
	if ( !OBJ_InitFromCustomTypeTemplate( emp, emp_type_template[ 1 ] ) ) {
		EmpInitDefaults( emp, 1 );
	}
}


PRIVATE
void EmpInitTypeUp2( CustomObject *base )
{
	ASSERT( base != NULL );
	Emp *emp = (Emp *) base;

	// init either from template or default values
	if ( !OBJ_InitFromCustomTypeTemplate( emp, emp_type_template[ 2 ] ) ) {
		EmpInitDefaults( emp, 2 );
	}
}


// emp constructor (class instantiation) --------------------------------------
//
PRIVATE
void EmpInstantiate( CustomObject *base )
{
	ASSERT( base != NULL );
	Emp *emp = (Emp *) base;

	// dynamic mem allocated in CreateEmp()
	// (LOD is set there)
}


// emp destructor (instance destruction) --------------------------------------
//
PRIVATE
void EmpDestroy( CustomObject *base )
{
	ASSERT( base != NULL );
	Emp *emp = (Emp *) base;

#ifndef PARSEC_SERVER
	// ensure pending callbacks are destroyed to avoid
	// calling them with invalid pointers
	int numremoved = CALLBACK_DestroyCallback( callback_type, (void *) base );
	if(!headless_bot)
		ASSERT( numremoved <= 1 );
#endif

	// free object structure memory
	FREEMEM( emp->ObjVtxs );
	emp->ObjVtxs = NULL;
}



#ifndef PARSEC_SERVER
// ----------------------------------------------------------------------------
// EMP DEVICE BEHAVIOUR
// ----------------------------------------------------------------------------


// maintain emp of local ship -------------------------------------------------
//
void WFX_CreateEmpWaves( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	shippo->EmpRefframesDelta += CurScreenRefFrames;

	int curwave;

	int curupgrade = 0;
	if ( shippo->Specials & SPMASK_EMP_UPGRADE_2 ) {
		curupgrade = 2;
	} else if ( shippo->Specials & SPMASK_EMP_UPGRADE_1 ) {
		curupgrade = 1;
	}

	// try standard emp if low energy
	if ( shippo->CurEnergy < emp_energy[ curupgrade ] ) {
		curupgrade = 0;
	}

	int numwaves = shippo->EmpRefframesDelta / emp_delay[ curupgrade ];
	int curalive = shippo->EmpRefframesDelta - emp_delay[ curupgrade ];
	dword energy_consumption = emp_energy[ curupgrade ];

	// create waves (oldest first)
	for ( curwave = ( numwaves - 1 ); curwave >= 0; curwave-- ) {

		// check if enough energy to create single wave
		if ( (dword)shippo->CurEnergy >= energy_consumption ) {

			shippo->CurEnergy -= energy_consumption;
			CreateEmp( shippo, 0, curalive, curupgrade );
			curalive -= emp_delay[ curupgrade ];
		} else {

			break;
		}
	}

	shippo->EmpRefframesDelta %= emp_delay[ curupgrade ];
}


// activate emp of local ship -------------------------------------------------
//
int WFX_ActivateEmp( ShipObject *shippo )
{
	ASSERT( shippo != NULL );
	ASSERT( shippo == MyShip );
	ASSERT( ( shippo->WeaponsActive & WPMASK_DEVICE_EMP ) == 0 );

	// check if enough space in RE_List
	if ( !NET_RmEvAllowed( RE_WEAPONSTATE ) ) {
		return FALSE;
	}

	// check if emp available
	if ( !OBJ_DeviceAvailable( shippo, WPMASK_DEVICE_EMP ) ) {
		ShowMessage( no_emp_str );
		return FALSE;
	}

	// create first emp-wave
	int curupgrade = 0;
	if ( shippo->Specials & SPMASK_EMP_UPGRADE_2 ) {
		curupgrade = 2;
	} else if ( shippo->Specials & SPMASK_EMP_UPGRADE_1 ) {
		curupgrade = 1;
	}

	shippo->EmpRefframesDelta = 0;

	// check if enough energy to shoot emp
	if ( shippo->CurEnergy >= emp_energy[ curupgrade ]  ) {
		shippo->CurEnergy -= emp_energy[ curupgrade ];
		CreateEmp( shippo, 0, 0, curupgrade );
	}
	// try standard emp
	else if ( shippo->CurEnergy >= emp_energy[ 0 ] ) {
		shippo->CurEnergy -= emp_energy[ 0 ];
		CreateEmp( shippo, 0, 0, 0 );
	}
	else {
		ShowMessage( low_energy_str );
		AUD_LowEnergy();
		return FALSE;
	}

	// set active flag
	shippo->WeaponsActive |= WPMASK_DEVICE_EMP;

	// send remote event to switch emp on
	NET_RmEvWeaponState( WPMASK_DEVICE_EMP, WPSTATE_ON, shippo->CurEnergy, shippo->Specials );

	// record activation event if recording active
	Record_EmpActivation();

	// play sound
//	AUD_Emp( shippo );

	return TRUE;
}


// deactivate emp of specified ship -------------------------------------------
//
void WFX_DeactivateEmp( ShipObject *shippo )
{
	ASSERT( shippo != NULL );
	ASSERT( shippo->WeaponsActive & WPMASK_DEVICE_EMP );

	// local ship is special case
	if ( shippo == MyShip ) {

		// check if enough space in RE_List
		if ( !NET_RmEvAllowed( RE_WEAPONSTATE ) ) {
			return;
		}

		// send remote event to switch emp off
		NET_RmEvWeaponState( WPMASK_DEVICE_EMP, WPSTATE_OFF, shippo->CurEnergy, shippo->Specials );

		// record deactivation event if recording active
		Record_EmpDeactivation();
	}

	// reset activation flag
	shippo->WeaponsActive &= ~WPMASK_DEVICE_EMP;
}


// remote player activated emp ------------------------------------------------
//
void WFX_RemoteActivateEmp( int playerid )
{
	// fetch pointer to remote player's ship
	ShipObject *shippo = NET_FetchOwnersShip( playerid );
	ASSERT( shippo != NULL );
	ASSERT( shippo != MyShip );

	// make sure emp is active
	if ( ( shippo->WeaponsActive & WPMASK_DEVICE_EMP) == 0 ) {

		// avoid additional stuff that would be done
		// by WFX_ActivateEmp()

		// create first emp-wave
		int curupgrade = 0;
		if ( shippo->Specials & SPMASK_EMP_UPGRADE_2 ) {
			curupgrade = 2;
		} else if ( shippo->Specials & SPMASK_EMP_UPGRADE_1 ) {
			curupgrade = 1;
		}

		shippo->EmpRefframesDelta = 0;

		// check if enough energy to shoot emp
		if ( shippo->CurEnergy >= emp_energy[ curupgrade ]  ) {
			shippo->CurEnergy -= emp_energy[ curupgrade ];
			CreateEmp( shippo, 0, 0, curupgrade );
		}
		// try standard emp
		else if ( shippo->CurEnergy >= emp_energy[ 0 ] ) {
			shippo->CurEnergy -= emp_energy[ 0 ];
			CreateEmp( shippo, 0, 0, 0 );
		}
		else {
			return;
		}

		shippo->WeaponsActive |= WPMASK_DEVICE_EMP;
	}
}


// remote player deactivated emp ----------------------------------------------
//
void WFX_RemoteDeactivateEmp( int playerid )
{
	// fetch pointer to remote player's ship
	ShipObject *shippo = NET_FetchOwnersShip( playerid );
	ASSERT( shippo != NULL );
	ASSERT( shippo != MyShip );

	// make sure emp is inactive
	if ( shippo->WeaponsActive & WPMASK_DEVICE_EMP) {

		// reset activation flag
		shippo->WeaponsActive &= ~WPMASK_DEVICE_EMP;
	}
}


// create emp blast -----------------------------------------------------------
//
void WFX_EmpBlast( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	// check if enough space in RE_List
	if ( !NET_RmEvAllowed( RE_CREATEEMP ) )
		return;

	if ( ( MyShip->Weapons & WPMASK_DEVICE_EMP ) == 0 ) {
		ShowMessage( no_emp_str );
		return;
	}

	int curdelay = 0;

	int curupgrade = 0;
	if ( shippo->Specials & SPMASK_EMP_UPGRADE_2 ) {
		curupgrade = 2;
	} else if ( shippo->Specials & SPMASK_EMP_UPGRADE_1 ) {
		curupgrade = 1;
	}
	int used_upgrade = curupgrade;

	int energy_consumption = emp_energy[ curupgrade ] * emp_waves[ curupgrade ];

	// check if enough energy to shoot emp
	if ( shippo->CurEnergy >= energy_consumption  ) {

		shippo->CurEnergy -= energy_consumption;

		for ( int i = 0; i < emp_waves[ curupgrade ]; i++ ) {
			CreateEmp( shippo, curdelay, 0, curupgrade );
			curdelay += emp_delay[ curupgrade ];
		}
	}
	// try standard emp
	else if ( shippo->CurEnergy >= ( emp_energy[ 0 ] * emp_waves[ 0 ] ) ) {

		shippo->CurEnergy -= ( emp_energy[ 0 ] * emp_waves[ 0 ] );

		for ( int i = 0; i < emp_waves[ 0 ]; i++ ) {
			CreateEmp( shippo, curdelay, 0, 0 );
			curdelay += emp_delay[ 0 ];
		}
		used_upgrade = 0;
	}
	else {
		ShowMessage( low_energy_str );
		AUD_LowEnergy();
		return;
	}

	// insert remote event
	NET_RmEvCreateEmp( used_upgrade );

	// record create event if recording active
	Record_EmpCreation();

	// play sound effect
	AUD_EmpBlast( shippo, curupgrade );
}


// create emp blast -----------------------------------------------------------
//
void WFX_RemoteEmpBlast( ShipObject *shippo, int curupgrade )
{
	ASSERT( shippo != NULL );

	int curdelay = 0;

	for ( int i = 0; i < emp_waves[ curupgrade ]; i++ ) {
		CreateEmp( shippo, curdelay, 0, curupgrade );
		curdelay += emp_delay[ curupgrade ];
	}

	// play sound effect
	AUD_EmpBlast( shippo, curupgrade );
}

#endif


// ----------------------------------------------------------------------------
// EMP REGISTRATION AND CONSOLE COMMANDS
// ----------------------------------------------------------------------------


// register object types for emp ----------------------------------------------
//
PRIVATE
void EmpRegisterCustomTypes()
{
	custom_type_info_s info;
	memset( &info, 0, sizeof( info ) );

	// always try to allocate templates
	for ( int i = 0; i < EMP_UPGRADES; i++ ) {
		emp_type_template[ i ] = (Emp *) ALLOCMEM( sizeof( Emp ) );
		if ( emp_type_template[ i ] != NULL ) {
			memset( emp_type_template[ i ], 0, sizeof( Emp ) );
			EmpInitDefaults( emp_type_template[ i ], i );
		}
	}

	info.type_name			= "emp";
	info.type_id			= 0x00000000;
	info.type_size			= sizeof( Emp );
	info.type_template		= emp_type_template[ 0 ];
	info.type_flags			= CUSTOM_TYPE_DEFAULT;
	info.callback_init		= EmpInitType;
	info.callback_instant	= EmpInstantiate;
	info.callback_destroy	= EmpDestroy;
	info.callback_animate	= EmpAnimate;
	info.callback_collide	= CheckEmpCollision;
	info.callback_notify	= NULL;
	info.callback_persist   = NULL;

	emp_type_id[ 0 ] = OBJ_RegisterCustomType( &info );
	CON_RegisterCustomType( info.type_id, Emp_PropList );

	memset( &info, 0, sizeof( info ) );

	info.type_name			= "empup1";
	info.type_id			= 0x00000000;
	info.type_size			= sizeof( Emp );
	info.type_template		= emp_type_template[ 1 ];
	info.type_flags			= CUSTOM_TYPE_DEFAULT;
	info.callback_init		= EmpInitTypeUp1;
	info.callback_instant	= EmpInstantiate;
	info.callback_destroy	= EmpDestroy;
	info.callback_animate	= EmpAnimate;
	info.callback_collide	= CheckEmpCollision;
	info.callback_notify	= NULL;
	info.callback_persist   = NULL;

	emp_type_id[ 1 ] = OBJ_RegisterCustomType( &info );
	CON_RegisterCustomType( info.type_id, EmpUp1_PropList );

	memset( &info, 0, sizeof( info ) );

	info.type_name			= "empup2";
	info.type_id			= 0x00000000;
	info.type_size			= sizeof( Emp );
	info.type_template		= emp_type_template[ 2 ];
	info.type_flags			= CUSTOM_TYPE_DEFAULT;
	info.callback_init		= EmpInitTypeUp2;
	info.callback_instant	= EmpInstantiate;
	info.callback_destroy	= EmpDestroy;
	info.callback_animate	= EmpAnimate;
	info.callback_collide	= CheckEmpCollision;
	info.callback_notify	= NULL;
	info.callback_persist   = NULL;

	emp_type_id[ 2 ] = OBJ_RegisterCustomType( &info );
	CON_RegisterCustomType( info.type_id, EmpUp2_PropList );
}


// precalculate expansion table for given parameters --------------------------
//
PRIVATE
void InitExpansionTableEmp( float **expansion_tab, int lifetime, float lambda )
{
	ASSERT( ( lifetime >= EMP_MIN_LIFETIME ) && ( lifetime <= EMP_MAX_LIFETIME ) );
	ASSERT( lambda > 0.0 );

	if ( *expansion_tab != NULL ) {
		FREEMEM( *expansion_tab );
		*expansion_tab = NULL;
	}

	*expansion_tab = (float *) ALLOCMEM( ( lifetime ) * sizeof( float ) );
	if ( *expansion_tab == NULL )
		OUTOFMEM( 0 );

	// precalc full expansion curve in time resolution
	// exactly as needed for emp expansion
	float ref_factor = 1.0 / ( 1.0 - exp( -lambda ) );

	for ( int i = 0; i < lifetime; i++ ) {
		(*expansion_tab)[ i ] = ( 1.0 - exp( -lambda * ( (float) i / (float) lifetime) ) ) * ref_factor;
	}
}


// key table for emp command --------------------------------------------
//
key_value_s emp_key_value[] = {

	{ "lifetime",	NULL,	KEYVALFLAG_NONE				},
	{ "maxwidth",	NULL,	KEYVALFLAG_NONE				},
	{ "lambda",		NULL,	KEYVALFLAG_NONE				},
    { "fadeout",    NULL,   KEYVALFLAG_NONE             },
	{ "waves",    	NULL,   KEYVALFLAG_NONE             },
	{ "delay",    	NULL,   KEYVALFLAG_NONE             },
	{ "energy",    	NULL,   KEYVALFLAG_NONE             },

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_EMP_LIFETIME,
	KEY_EMP_MAXWIDTH,
	KEY_EMP_LAMBDA,
    KEY_EMP_FADEOUT,
	KEY_EMP_WAVES,
	KEY_EMP_DELAY,
	KEY_EMP_ENERGY
};


// customize emp --------------------------------------------------------------
//
PRIVATE
int EMP_CONF( char *paramstr, int upgradelevel )
{
	//NOTE:
	//CONCOM:
	// emp_conf_command	::= 'emp.conf' [<lifetime_spec>] [<maxwidth_spec>]
	//						[<lambda_spec>] [<fadeout_spec>] [<waves_spec>]
	//						[<delay_spec>] [<energy_spec>]
	// lifetime_spec	::= 'lifetime' <int>
	// maxwidth_spec	::= 'maxwidth' <float>
	// lambda_spec		::= 'lambda' <float>
	// fadeout_spec		::= 'fadeout' <int>
	// waves_spec		::= 'waves' <int>
	// delay_spec		::= 'delay' <int>
	// energy_spec		::= 'energy' <int>

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( emp_key_value, paramstr ) )
		return TRUE;

	int		alldefaults	= TRUE;
	int		lifetime	= emp_lifetime[ upgradelevel ];
	float max_width	= GEOMV_TO_FLOAT( emp_max_width[ upgradelevel ] );
	float lambda		= emp_lambda[ upgradelevel ];
	int		fadeout     = emp_fadeout[ upgradelevel ];
	int		waves		= emp_waves[ upgradelevel ];
	int		delay		= emp_delay[ upgradelevel ];
	int		energy		= emp_energy[ upgradelevel ];

	// lifetime
	if ( emp_key_value[ KEY_EMP_LIFETIME ].value != NULL ) {

		if ( ScanKeyValueInt( &emp_key_value[ KEY_EMP_LIFETIME ], &lifetime ) < 0 ) {
			CON_AddLine( emp_inval_lifetime_spec );
			return TRUE;
		}
		if ( ( lifetime < EMP_MIN_LIFETIME ) || ( lifetime > EMP_MAX_LIFETIME ) ) {
			CON_AddLine( emp_inval_lifetime_spec );
			return TRUE;
		}
		if ( lifetime < fadeout ) {
			fadeout = lifetime;
		}
		alldefaults = FALSE;
	}

	// max_width
	if ( emp_key_value[ KEY_EMP_MAXWIDTH ].value != NULL ) {

		if ( ScanKeyValueFloat( &emp_key_value[ KEY_EMP_MAXWIDTH ], &max_width ) < 0 ) {
			CON_AddLine( emp_inval_maxwidth_spec );
			return TRUE;
		}
		if ( max_width <= 0.0 ) {
			CON_AddLine( emp_inval_maxwidth_spec );
			return TRUE;
		}
		alldefaults = FALSE;
	}

	// lambda
	if ( emp_key_value[ KEY_EMP_LAMBDA ].value != NULL ) {

		if ( ScanKeyValueFloat( &emp_key_value[ KEY_EMP_LAMBDA ], &lambda ) < 0 ) {
			CON_AddLine( emp_inval_lambda_spec );
			return TRUE;
		}
		if ( lambda <= 0.0 ) {
			CON_AddLine( emp_inval_maxwidth_spec );
			return TRUE;
		}
		alldefaults = FALSE;
	}

	// fadeout
    if ( emp_key_value[ KEY_EMP_FADEOUT ].value != NULL ) {

		if ( ScanKeyValueInt( &emp_key_value[ KEY_EMP_FADEOUT ], &fadeout ) < 0 ) {
        		CON_AddLine( emp_inval_fadeout_spec );
        		return TRUE;
       	}
		if ( ( fadeout < 0 ) || ( fadeout > EMP_MAX_LIFETIME ) ) {
        	CON_AddLine( emp_inval_fadeout_spec );
        	return TRUE;
		}
		if ( fadeout > lifetime ) {
			lifetime = fadeout;
		}
		alldefaults = FALSE;
	}

	// waves
	if ( emp_key_value[ KEY_EMP_WAVES ].value != NULL ) {

        if ( ScanKeyValueInt( &emp_key_value[ KEY_EMP_WAVES ], &waves ) < 0 ) {
            CON_AddLine( emp_inval_waves_spec );
        	return TRUE;
        }
		if ( ( waves < EMP_MIN_WAVES ) || ( waves > EMP_MAX_WAVES ) ) {
			CON_AddLine( emp_inval_waves_spec );
			return TRUE;
		}
		alldefaults = FALSE;
	}

	// delay
	if ( emp_key_value[ KEY_EMP_DELAY ].value != NULL ) {

		if ( ScanKeyValueInt( &emp_key_value[ KEY_EMP_DELAY ], &delay ) < 0 ) {
			CON_AddLine( emp_inval_delay_spec );
			return TRUE;
		}
		if ( ( waves < EMP_MIN_DELAY ) || ( waves > EMP_MAX_DELAY ) ) {
			CON_AddLine( emp_inval_delay_spec );
			return TRUE;
		}
		alldefaults = FALSE;
	}

	// energy
	if ( emp_key_value[ KEY_EMP_ENERGY ].value != NULL ) {

		if ( ScanKeyValueInt( &emp_key_value[ KEY_EMP_ENERGY ], &energy ) < 0 ) {
			CON_AddLine( emp_inval_energy_spec );
			return TRUE;
		}
		if ( ( energy < 0 ) ) {
			CON_AddLine( emp_inval_energy_spec );
			return TRUE;
		}
		alldefaults = FALSE;
    }

    if ( !alldefaults ) {
		emp_max_width[ upgradelevel ] = FLOAT_TO_GEOMV( max_width );
		emp_fadeout[ upgradelevel ] = fadeout;
		emp_waves[ upgradelevel ] = waves;
		emp_delay[ upgradelevel ] = delay;
		emp_energy[ upgradelevel ] = energy;
		if ( ( lifetime != emp_lifetime[ upgradelevel ] ) || ( lambda != emp_lambda[ upgradelevel ] ) ) {
			emp_lifetime[ upgradelevel ] = lifetime;
			emp_lambda[ upgradelevel ] = lambda;
			InitExpansionTableEmp( &emp_expansion_tab[ upgradelevel ], lifetime, lambda );
		}
	}
	else {
		//FIXME:
		// hack!
		switch ( upgradelevel ) {
			case 0:
				sprintf( paste_str, "maxwidth: %f (%f)", max_width, EMP_MAX_WIDTH );
				CON_AddLine( paste_str );
				sprintf( paste_str, "lifetime: %d (%d)", lifetime, EMP_LIFETIME );
				CON_AddLine( paste_str );
				sprintf( paste_str, "fadeout: %d (%d)", fadeout, EMP_FADEOUT );
				CON_AddLine( paste_str );
				sprintf( paste_str, "lambda: %f (%f)", lambda, EMP_LAMBDA );
				CON_AddLine( paste_str );
				sprintf( paste_str, "delay: %d (%d)", delay, EMP_DELAY );
				CON_AddLine( paste_str );
				sprintf( paste_str, "waves: %d (%d)", waves, EMP_WAVES );
				CON_AddLine( paste_str );
				sprintf( paste_str, "energy: %d (%d)", energy, EMP_ENERGY );
				CON_AddLine( paste_str );
				break;

			case 1:
				sprintf( paste_str, "maxwidth: %f (%f)", max_width, EMP_UP1_MAX_WIDTH );
				CON_AddLine( paste_str );
				sprintf( paste_str, "lifetime: %d (%d)", lifetime, EMP_UP1_LIFETIME );
				CON_AddLine( paste_str );
				sprintf( paste_str, "fadeout: %d (%d)", fadeout, EMP_UP1_FADEOUT );
				CON_AddLine( paste_str );
				sprintf( paste_str, "lambda: %f (%f)", lambda, EMP_UP1_LAMBDA );
				CON_AddLine( paste_str );
				sprintf( paste_str, "delay: %d (%d)", delay, EMP_UP1_DELAY );
				CON_AddLine( paste_str );
				sprintf( paste_str, "waves: %d (%d)", waves, EMP_UP1_WAVES );
				CON_AddLine( paste_str );
				sprintf( paste_str, "energy: %d (%d)", energy, EMP_UP1_ENERGY );
				CON_AddLine( paste_str );
				break;

			case 2:
				sprintf( paste_str, "maxwidth: %f (%f)", max_width, EMP_UP2_MAX_WIDTH );
				CON_AddLine( paste_str );
				sprintf( paste_str, "lifetime: %d (%d)", lifetime, EMP_UP2_LIFETIME );
				CON_AddLine( paste_str );
				sprintf( paste_str, "fadeout: %d (%d)", fadeout, EMP_UP2_FADEOUT );
				CON_AddLine( paste_str );
				sprintf( paste_str, "lambda: %f (%f)", lambda, EMP_UP2_LAMBDA );
				CON_AddLine( paste_str );
				sprintf( paste_str, "delay: %d (%d)", delay, EMP_UP2_DELAY );
				CON_AddLine( paste_str );
				sprintf( paste_str, "waves: %d (%d)", waves, EMP_UP2_WAVES );
				CON_AddLine( paste_str );
				sprintf( paste_str, "energy: %d (%d)", energy, EMP_UP2_ENERGY );
				CON_AddLine( paste_str );
				break;

			default:
				ASSERT( 0 );
		}
	}

	return TRUE;
}


PRIVATE
int Cmd_EMP_CONF( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// emp_conf_command	::= 'emp.conf' [<lifetime_spec>] [<maxwidth_spec>]
	//						[<lambda_spec>] [<fadeout_spec>] [<waves_spec>]
	//						[<delay_spec>] [<energy_spec>]
	// lifetime_spec	::= 'lifetime' <int>
	// maxwidth_spec	::= 'maxwidth' <float>
	// lambda_spec		::= 'lambda' <float>
	// fadeout_spec		::= 'fadeout' <int>
	// waves_spec		::= 'waves' <int>
	// delay_spec		::= 'delay' <int>
	// energy_spec		::= 'energy' <int>

	return EMP_CONF( paramstr, 0 );
}


PRIVATE
int Cmd_EMPUP1_CONF( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// emp_conf_command	::= 'empup1.conf' [<lifetime_spec>] [<maxwidth_spec>]
	//						[<lambda_spec>] [<fadeout_spec>] [<waves_spec>]
	//						[<delay_spec>] [<energy_spec>]
	// lifetime_spec	::= 'lifetime' <int>
	// maxwidth_spec	::= 'maxwidth' <float>
	// lambda_spec		::= 'lambda' <float>
	// fadeout_spec		::= 'fadeout' <int>
	// waves_spec		::= 'waves' <int>
	// delay_spec		::= 'delay' <int>
	// energy_spec		::= 'energy' <int>

	return EMP_CONF( paramstr, 1 );
}


PRIVATE
int Cmd_EMPUP2_CONF( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// emp_conf_command	::= 'empup2.conf' [<lifetime_spec>] [<maxwidth_spec>]
	//						[<lambda_spec>] [<fadeout_spec>] [<waves_spec>]
	//						[<delay_spec>] [<energy_spec>]
	// lifetime_spec	::= 'lifetime' <int>
	// maxwidth_spec	::= 'maxwidth' <float>
	// lambda_spec		::= 'lambda' <float>
	// fadeout_spec		::= 'fadeout' <int>
	// waves_spec		::= 'waves' <int>
	// delay_spec		::= 'delay' <int>
	// energy_spec		::= 'energy' <int>

	return EMP_CONF( paramstr, 2 );
}


// console command for activating the emp -------------------------------------
//
PRIVATE
int Cmd_EMP( char *argstr )
{
	//NOTE:
	//CONCOM:
	// emp_command	::= 'emp'

	ASSERT( argstr != NULL );
	HANDLE_COMMAND_DOMAIN( argstr );
#ifndef PARSEC_SERVER
	WFX_EmpBlast( MyShip );
#endif
	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( G_EMP )
{
	// register type
	EmpRegisterCustomTypes();

	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "emp.conf" command
	regcom.command	 = "emp.conf";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_EMP_CONF;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "empup1.conf" command
	regcom.command	 = "empup1.conf";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_EMPUP1_CONF;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "empup2.conf" command
	regcom.command	 = "empup2.conf";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_EMPUP2_CONF;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "emp" command
	regcom.command	 = "emp";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_EMP;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// init default expansion tables
	InitExpansionTableEmp( &emp_expansion_tab[ 0 ],     EMP_LIFETIME,     EMP_LAMBDA );
	InitExpansionTableEmp( &emp_expansion_tab[ 1 ], EMP_UP1_LIFETIME, EMP_UP1_LAMBDA );
	InitExpansionTableEmp( &emp_expansion_tab[ 2 ], EMP_UP2_LIFETIME, EMP_UP2_LAMBDA );
}



