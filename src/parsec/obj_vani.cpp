/*
 * PARSEC - Vertex Animations
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:24 $
 *
 * Orginally written by:
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

// mathematics header
#include "utl_math.h"

// local module header
#include "obj_vani.h"

// proprietary module headers
#include "e_vtxani.h"
#include "obj_clas.h"
#include "obj_creg.h"
#include "obj_name.h"



// vertex animation callback that does not actually do any animation ----------
//
PRIVATE
int VtxAnimIdentity( GenObject *gobj, dword animid )
{
	ASSERT( gobj != NULL );
	ASSERT( animid < gobj->ActiveVtxAnims );

	return TRUE;
}


// vertex animation callback for hurricane ------------------------------------
//
PRIVATE
int VtxAnimHurricane( GenObject *gobj, dword animid )
{
	ASSERT( gobj != NULL );
	ASSERT( animid < gobj->ActiveVtxAnims );

	VtxAnimState *anim = &gobj->VtxAnimStates[ animid ];

	bams_t angle = 0x0020 * CurScreenRefFrames;
	CamRotZ( anim->CurrentXmatrx, angle );

	return TRUE;
}


// register vertex animations for hurricane -----------------------------------
//
PRIVATE
void RegisterVtxAnimsHurricane( GenObject *gobj )
{
	ASSERT( gobj != NULL );
	ASSERT( gobj->ActiveVtxAnims == 0 );
	ASSERT( gobj->VtxAnimStates != NULL );

	dword *faceids = (dword *) ALLOCMEM( ( 72 - 0 ) * sizeof( dword ) );
	if ( faceids == NULL ) {
		OUTOFMEM( 0 );
	}

	int faceid;
	dword *fillp = faceids;

	// fi06_17a.3df
	for ( faceid = 0; faceid <= 31; )
		*fillp++ = faceid++;
	// fi06_10a.3df (partly)
	for ( faceid = 32; faceid <= 63; )
		*fillp++ = faceid++;
	// fi06_18a.3df
	for ( faceid = 64; faceid <= 71; )
		*fillp++ = faceid++;

	VtxAnimState *anim = VtxAnimCreateFromFaceList( gobj, 0, ( 72 - 0 ), faceids );
	if ( anim != NULL ) {
		anim->AnimCallback = VtxAnimHurricane;
		MakeIdMatrx( anim->CurrentXmatrx );
	}

	FREEMEM( faceids );
}


// vertex animation callback for proximity mine extra -------------------------
//
PRIVATE
int VtxAnimProximityMine( GenObject *gobj, dword animid )
{
	ASSERT( gobj != NULL );
	ASSERT( animid < gobj->ActiveVtxAnims );

	VtxAnimState *anim = &gobj->VtxAnimStates[ animid ];

	bams_t angle = -0x0010 * CurScreenRefFrames;
	CamRotX( anim->CurrentXmatrx, angle );
	angle = -0x002a * CurScreenRefFrames;
	CamRotY( anim->CurrentXmatrx, angle );
	angle = -0x0014 * CurScreenRefFrames;
	CamRotZ( anim->CurrentXmatrx, angle );

	return TRUE;
}


// register vertex animations for proximity mine extra ------------------------
//
PRIVATE
void RegisterVtxAnimsProximityMine( GenObject *gobj )
{
	ASSERT( gobj != NULL );
	ASSERT( gobj->ActiveVtxAnims == 0 );
	ASSERT( gobj->VtxAnimStates != NULL );

	dword faceids[ 16 ];
	for ( int faceid = 0; faceid < 16; faceid++ ) {
		faceids[ faceid ] = faceid;
	}

	// mi01_01a.3df (core)
	VtxAnimState *anim = VtxAnimCreateFromFaceList( gobj, 0, 8, &faceids[ 0 ] );
	if ( anim != NULL ) {
		anim->AnimCallback = VtxAnimProximityMine;
		MakeIdMatrx( anim->CurrentXmatrx );
	}

	// mi01_01a.3df (hull)
	anim = VtxAnimCreateFromFaceList( gobj, 0, 8, &faceids[ 0 ] );
	if ( anim != NULL ) {
		anim->AnimCallback = VtxAnimIdentity;
		MakeIdMatrx( anim->CurrentXmatrx );
	}
}


// vertex animation callback for invulnerability extra ------------------------
//
PRIVATE
int VtxAnimInvulnerability( GenObject *gobj, dword animid )
{
	ASSERT( gobj != NULL );
	ASSERT( animid < gobj->ActiveVtxAnims );

	VtxAnimState *anim = &gobj->VtxAnimStates[ animid ];

	bams_t angle = -0x0010 * CurScreenRefFrames;
	CamRotX( anim->CurrentXmatrx, angle );
	angle = -0x0020 * CurScreenRefFrames;
	CamRotY( anim->CurrentXmatrx, angle );

	return TRUE;
}


// register vertex animations for invulnerability extra -----------------------
//
PRIVATE
void RegisterVtxAnimsInvulnerability( GenObject *gobj )
{
	ASSERT( gobj != NULL );
	ASSERT( gobj->ActiveVtxAnims == 0 );
	ASSERT( gobj->VtxAnimStates != NULL );

	dword *faceids = (dword *) ALLOCMEM( ( 224 - 0 ) * sizeof( dword ) );
	if ( faceids == NULL ) {
		OUTOFMEM( 0 );
	}

	int faceid;
	dword *fillp = faceids;

	// in01_00a.3df
	for ( faceid = 0; faceid <= 223; )
		*fillp++ = faceid++;

	VtxAnimState *anim = VtxAnimCreateFromFaceList( gobj, 0, ( 224 - 0 ), faceids );
	if ( anim != NULL ) {
		anim->AnimCallback = VtxAnimInvulnerability;
		MakeIdMatrx( anim->CurrentXmatrx );
	}

	FREEMEM( faceids );
}


// vertex animation callback for afterburner extra ----------------------------
//
PRIVATE
int VtxAnimAfterburner( GenObject *gobj, dword animid )
{
	ASSERT( gobj != NULL );
	ASSERT( animid < gobj->ActiveVtxAnims );

	VtxAnimState *anim = &gobj->VtxAnimStates[ animid ];

	bams_t angle = -0x0015 * CurScreenRefFrames;
	CamRotX( anim->CurrentXmatrx, angle );
	angle = -0x0024 * CurScreenRefFrames;
	CamRotZ( anim->CurrentXmatrx, angle );

	return TRUE;
}


// register vertex animations for afterburner extra ---------------------------
//
PRIVATE
void RegisterVtxAnimsAfterburner( GenObject *gobj )
{
	ASSERT( gobj != NULL );
	ASSERT( gobj->ActiveVtxAnims == 0 );
	ASSERT( gobj->VtxAnimStates != NULL );

	dword *faceids = (dword *) ALLOCMEM( ( 224 - 0 ) * sizeof( dword ) );
	if ( faceids == NULL ) {
		OUTOFMEM( 0 );
	}

	int faceid;
	dword *fillp = faceids;

	// in01_00a.3df
	for ( faceid = 0; faceid <= 223; )
		*fillp++ = faceid++;

	VtxAnimState *anim = VtxAnimCreateFromFaceList( gobj, 0, ( 224 - 0 ), faceids );
	if ( anim != NULL ) {
		anim->AnimCallback = VtxAnimAfterburner;
		MakeIdMatrx( anim->CurrentXmatrx );
	}

	FREEMEM( faceids );
}


// vertex animation callback for swarm missiles pack --------------------------
//
PRIVATE
int VtxAnimSwarmMissiles( GenObject *gobj, dword animid )
{
	ASSERT( gobj != NULL );
	ASSERT( animid < gobj->ActiveVtxAnims );

	VtxAnimState *anim = &gobj->VtxAnimStates[ animid ];

	bams_t angle = -0x0015 * CurScreenRefFrames;
	CamRotX( anim->CurrentXmatrx, angle );
	angle = -0x0024 * CurScreenRefFrames;
	CamRotZ( anim->CurrentXmatrx, angle );

	return TRUE;
}


// register vertex animations for swarm missiles pack -------------------------
//
PRIVATE
void RegisterVtxAnimsSwarmMissiles( GenObject *gobj )
{
	ASSERT( gobj != NULL );
	ASSERT( gobj->ActiveVtxAnims == 0 );
	ASSERT( gobj->VtxAnimStates != NULL );

	dword *faceids = (dword *) ALLOCMEM( ( 224 - 0 ) * sizeof( dword ) );
	if ( faceids == NULL ) {
		OUTOFMEM( 0 );
	}

	int faceid;
	dword *fillp = faceids;

	// in01_00a.3df
	for ( faceid = 0; faceid <= 223; )
		*fillp++ = faceid++;

	VtxAnimState *anim = VtxAnimCreateFromFaceList( gobj, 0, ( 224 - 0 ), faceids );
	if ( anim != NULL ) {
		anim->AnimCallback = VtxAnimSwarmMissiles;
		MakeIdMatrx( anim->CurrentXmatrx );
	}

	FREEMEM( faceids );
}


// register vertex animation function -----------------------------------------
//
int OBJ_RegisterVtxAnimation( const char *classname, register_vtxanim_fpt regfunc )
{
	ASSERT( classname != NULL );
	ASSERT( regfunc != NULL );

	// try to resolve name to id
	dword classid = OBJ_FetchObjectClassId( classname );
	if ( classid == CLASS_ID_INVALID ) {
		return FALSE;
	}

	GenObject *gobj = ObjClasses[ classid ];

	// double registration not allowed
	if ( gobj->ActiveVtxAnims > 0 ) {
		return FALSE;
	}

	// only possible if memory reserved
	if ( gobj->VtxAnimStates == NULL ) {
		return FALSE;
	}

	// call registration function
	(*regfunc)( gobj );

	return TRUE;
}


// register vertex animations for all relevant and accessible classes ---------
//
PRIVATE
void RegisterVertexAnimations( int numparams, int *params )
{
	//NOTE:
	// must be idempotent.

	// ships
	OBJ_RegisterVtxAnimation(
		OBJCLASSNAME_SHIP_HURRICANE, RegisterVtxAnimsHurricane );

	// extras
	OBJ_RegisterVtxAnimation(
		OBJCLASSNAME_MISSILE_MINE, RegisterVtxAnimsProximityMine );

	OBJ_RegisterVtxAnimation(
		OBJCLASSNAME_DEVICE_INVULNERABILITY, RegisterVtxAnimsInvulnerability );

	OBJ_RegisterVtxAnimation(
		OBJCLASSNAME_DEVICE_AFTERBURNER, RegisterVtxAnimsAfterburner );

	OBJ_RegisterVtxAnimation(
		OBJCLASSNAME_EXTRA_PACK_SWARM, RegisterVtxAnimsSwarmMissiles );
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( OBJ_VANI )
{
	// register on creg
	OBJ_RegisterClassRegistration( RegisterVertexAnimations );
}



