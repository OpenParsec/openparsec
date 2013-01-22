/*
 * PARSEC - Stargate Model
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:37 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   1999-2002
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999-2000
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
#include "sys_defs.h"

// drawing subsystem
#include "d_bmap.h"
#include "d_font.h"
#include "d_iter.h"
#include "d_misc.h"

// subsystem linkage info
#include "linkinfo.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "g_stgate.h"

// proprietary module headers
#include "con_info.h"
#include "e_callbk.h"
#include "e_color.h"
#include "e_supp.h"
#include "h_supp.h"
//#include "net_game_gmsv.h"
#include "net_serv.h"
#include "net_udpdf.h"
#include "net_wrap.h"
#include "obj_clas.h"
#include "obj_creg.h"
#include "obj_ctrl.h"
#include "obj_cust.h"
#include "part_api.h"


// flags
//#define _VISUALIZE_BBOX
//#define _VISUALIZE_BRECT
#define USE_COMPILED_VTX_ARRAYS
#define USE_STRIPORDER_TRIANGLES
//#define DISABLE_RING_FLARES



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// stargate limits and constants ----------------------------------------------
//
#define STARGATE_RADIUS						50.0f
#define STARGATE_NUM_PLATES					12						// number of plates the stargate consists of
#define STARGATE_NUM_FLARES					STARGATE_NUM_PLATES		// number of flares attached to the stargate
#define STARGATE_INTERIOR_NUM_RINGS			15
#define STARGATE_INTERIOR_VERTS_PER_RING	( STARGATE_NUM_PLATES * 2 )
#define STARGATE_INTERIOR_NUM_VERTS			( ( STARGATE_INTERIOR_VERTS_PER_RING * STARGATE_INTERIOR_NUM_RINGS ) + 1 )
#define STARGATE_INTERIOR_RADIUS			( STARGATE_RADIUS * 0.98f )

#define STARGATE_RING_NUM_PARTS				STARGATE_NUM_PLATES
#define STARGATE_PART_RADIUS				STARGATE_RADIUS
#define STARGATE_RADIUS_EXPANSION			1.0f
#define STARGATE_NUM_RINGS					20
#define STARGATE_RING_PART_ZSPACING			10.0f
#define STARGATE_PART_ZOFFSET				INT_TO_GEOMV( -2 )

#define STARGATE_INTERIOR_TEXTURE_NAME		"sg_int.3df"
#define STARGATE_FLARE_PDEF_NAME			"sgateposlt"


#if ( STARGATE_INTERIOR_NUM_RINGS > 32 )
	#error "too many rings in stargate interior"
#endif


// module local functions -----------------------------------------------------
//
PRIVATE int		StargateInstantiate_InitInteriorVertices( Stargate *stargate );

//PRIVATE int		StargateModify_RealizeNode( GenObject* base );
PRIVATE int		StargateModify_ActiveChanged( GenObject* base );
PRIVATE int		StargateModify_AutoactivateChanged( GenObject* base );
PRIVATE void	StargateModify_CreateActiveParticles( CustomObject *base );


// offset definitions into the Stargate ---------------------------------------
//
#define OFS_ROTSPEED		offsetof( Stargate, rotspeed )
#define OFS_RADIUS			offsetof( Stargate, radius )
#define OFS_DESTNAME		offsetof( Stargate, destination_name )
//#define OFS_DESTIP			offsetof( Stargate, destination_ip )
//#define OFS_DESTPORT		offsetof( Stargate, destination_port )
#define OFS_ACTDISTANCE		offsetof( Stargate, actdistance )
#define OFS_DORMANT			offsetof( Stargate, dormant )			
#define OFS_ACTIVE			offsetof( Stargate, active )
#define OFS_AUTOACTIVATE	offsetof( Stargate, autoactivate )
#define OFS_NUMPARTACTIVE	offsetof( Stargate, numpartactive )
#define OFS_ACTCYLLEN		offsetof( Stargate, actcyllen )
#define OFS_PARTVEL			offsetof( Stargate, partvel )
#define OFS_MODULSPEED		offsetof( Stargate, modulspeed )
#define OFS_MODULRAD1		offsetof( Stargate, modulrad1 )
#define OFS_MODULRAD2		offsetof( Stargate, modulrad2 )
#define OFS_ACTTIME			offsetof( Stargate, acttime )
#define OFS_FLARE_NAME		offsetof( Stargate, flare_name )
#define OFS_INTERIOR_NAME	offsetof( Stargate, interior_name )


// list of console-accessible properties --------------------------------------
//
PRIVATE
proplist_s Stargate_PropList[] = {

	{ "rotspeed",		OFS_ROTSPEED,		0,			0xffff,						PROPTYPE_INT,		NULL	},
	{ "radius",			OFS_RADIUS,			0x10000,	0x4000000,					PROPTYPE_FLOAT,		NULL	},
	//{ "destname",		OFS_DESTNAME,		0,			MAX_SERVER_NAME,			PROPTYPE_STRING,	NULL	},
	//{ "destip",			OFS_DESTIP,			0,			STARGATE_MAX_DEST_IP,		PROPTYPE_STRING,	StargateModify_RealizeNode	},
	//{ "destport",		OFS_DESTPORT,		1024,		65535,						PROPTYPE_INT,		StargateModify_RealizeNode	},
	{ "actdistance",	OFS_ACTDISTANCE,	0x10000,    0x4000000,					PROPTYPE_FLOAT,		NULL	},
	{ "dormant",		OFS_DORMANT,		0x0,		0x1,						PROPTYPE_INT,		NULL	},
	{ "active",			OFS_ACTIVE,			0x0,		0x1,						PROPTYPE_INT,		StargateModify_ActiveChanged		},
	{ "autoactivate",	OFS_AUTOACTIVATE,	0x0,		0x1,						PROPTYPE_INT,		StargateModify_AutoactivateChanged	},
	{ "numpartactive",	OFS_NUMPARTACTIVE,	1,			2048,						PROPTYPE_INT,		NULL	},
	{ "actcyllen",		OFS_ACTCYLLEN,		0,			2048,						PROPTYPE_INT,		NULL	},
	{ "partvel",		OFS_PARTVEL,		0x10000,    0x4000000,					PROPTYPE_FLOAT,		NULL	},
	{ "modulspeed",		OFS_MODULSPEED,		0,			0xffff,						PROPTYPE_INT,		NULL	},
	{ "modulrad1",		OFS_MODULRAD1,		0x10000,    0x4000000,					PROPTYPE_FLOAT,		NULL	},
	{ "modulrad2",		OFS_MODULRAD2,		0x10000,    0x4000000,					PROPTYPE_FLOAT,		NULL	},
	{ "acttime",		OFS_ACTTIME,		0,			FRAME_MEASURE_TIMEBASE*10,	PROPTYPE_INT,		NULL	},
	{ "flare_name",		OFS_FLARE_NAME,		0,			MAX_TEXNAME,				PROPTYPE_STRING,	NULL	},
	{ "interior_name",	OFS_INTERIOR_NAME,	0,			MAX_TEXNAME,				PROPTYPE_STRING,	NULL	},

	{ NULL,				0,				0,			0,							0,					NULL	},
};


// type fields init function for stargate -------------------------------------
//
PRIVATE
void StargateInitType( CustomObject *base )
{
	ASSERT( base != NULL );

	Stargate *stargate = (Stargate *) base;

	stargate->rotspeed			= 0x0004;
	stargate->radius			= STARGATE_PART_RADIUS;
	stargate->actdistance		= 300;
	stargate->dormant			= FALSE;
	stargate->active			= FALSE;
	stargate->autoactivate  	= TRUE;
	stargate->numpartactive 	= 600;
	stargate->actcyllen			= 300;
	stargate->partvel			= 150;
	stargate->modulspeed		= DEG_TO_BAMS( 1.0f );
	stargate->modulate_bams		= BAMS_DEG0;
	stargate->modulrad1			= 5.0f;
	stargate->modulrad2			= 2.0f;
	stargate->modulfade			= 2.0f;
	stargate->acttime			= FRAME_MEASURE_TIMEBASE;

	stargate->activating		 = FALSE;
	stargate->curactcyllen		 = 0;
	stargate->manually_activated = FALSE;
	stargate->actring			 = 0.0f;

	strcpy( stargate->flare_name, STARGATE_FLARE_PDEF_NAME );
	strcpy( stargate->interior_name, STARGATE_INTERIOR_TEXTURE_NAME );

	stargate->pcluster			= NULL;
	stargate->interior_vtxlist	= NULL;
	stargate->interior_viewvtxs	= NULL;
	stargate->interior_u		= NULL;
	stargate->interior_v		= NULL;
	stargate->interior_texmap	= NULL;

	stargate->ping				= -1;
	stargate->lastpinged		= 0;
}

/*
// notification callback when eihter IP or port is changed --------------------
//
PRIVATE 
int StargateModify_RealizeNode( GenObject* base )
{
	ASSERT( base != NULL );
	Stargate* stargate = (Stargate *) base;

	// try to resolve DNS name to IP address (still as string)
	char resolved_name[ MAX_IPADDR_LEN + 1 ];
	if ( NET_ResolveHostName( stargate->destination_ip, resolved_name, NULL ) ) {
		MSGOUT( "stargate->destinaiton %s resolved to %s", stargate->destination_ip, resolved_name );
	} else {
		// DNS lookup failed
		return FALSE;
	}

	// save server address for later use by NET_GMSV.C
	inet_aton( resolved_name, &stargate->destination_node );
	UDP_StoreNodePort( &stargate->destination_node, stargate->destination_port );

	return TRUE;
}
*/

// notification callback when "active" changed --------------------------------
//
PRIVATE
int StargateModify_ActiveChanged( GenObject* base )
{
	ASSERT( base != NULL );
	Stargate *stargate = (Stargate *) base;

	// manual activation/deactivation only if autoactivate off
	if ( stargate->autoactivate ) {
		return TRUE;
	}

	// prevent manual activation when stargate dormant
	if ( stargate->dormant ) {
		stargate->active = FALSE;
	}

	if ( stargate->active ) {

		if ( !stargate->manually_activated ) {
			stargate->manually_activated = TRUE;
			StargateModify_CreateActiveParticles( stargate );
		}

	} else {

		stargate->manually_activated = FALSE;
	}

	return TRUE;
}


// notification callback when "autoactivate" changed --------------------------
//
PRIVATE
int StargateModify_AutoactivateChanged( GenObject* base )
{
	ASSERT( base != NULL );
	Stargate *stargate = (Stargate *) base;

	// deactivate the stargate when it's active due to a manual activation
	if ( stargate->autoactivate && stargate->manually_activated ) {
		stargate->active = FALSE;
		stargate->manually_activated = FALSE;
	}

	return TRUE;
}


// particle animation callback for setting the stargate-active particles ------
//
PRIVATE
void StargateAnimate_Particles( genobject_pcluster_s* pcluster )
{
	ASSERT( pcluster != NULL );

	Stargate *stargate = (Stargate *) pcluster->baseobject;

	for ( int curp = 0; curp < pcluster->numel; curp++ ) {

		particle_s* curparticle = &pcluster->rep[ curp ];

		// skip inactive particles
		if ( ( curparticle->flags & PARTICLE_ACTIVE ) == 0 )
			continue;

		// move along vector
		Vector3 advvec;
		advvec.Z = curparticle->velocity.Z * CurScreenRefFrames;
		curparticle->position.Z += advvec.Z;

		// check whether particle is sucked into the stargate and
		// reposition it at the beginning of the tunnel
		if ( curparticle->position.Z > 0 ) {

			if ( stargate->active ) {

				geomv_t basez = STARGATE_PART_ZOFFSET + curparticle->position.Z;
				if ( stargate->activating ) {
					curparticle->position.Z =
						basez - INT_TO_GEOMV( stargate->curactcyllen );
				} else {
					curparticle->position.Z =
						basez - INT_TO_GEOMV( stargate->actcyllen );
				}

			} else {

				// if stargate switches to inactive, do not reactivate
				// the particle at the cylinder beginning
				curparticle->flags &= ~PARTICLE_ACTIVE;
			}
		}
	}
}


// init flares of stargate ring -----------------------------------------------
//
INLINE
void StargateInstantiate_InitFlares( Stargate *stargate )
{
	ASSERT( stargate != NULL );

	//TODO: this way resolution changes are not handled
	extern float cur_particle_resoscale;

	// attach flares to the stargate
	pdef_s *pdef = PRT_AcquireParticleDefinition( stargate->flare_name, NULL );
	if ( pdef == NULL ) {
		return;
	}

	// init the extinfo structure
	pextinfo_s extinfo;
	PRT_InitParticleExtInfo( &extinfo, pdef, NULL, NULL );

	// specify the reference z value ( directly modifies the width/height of a flare particle)
	float ref_z = 100.0f * cur_particle_resoscale;

	dword	rendflags = PART_REND_POINTVIS | PART_REND_NODEPTHCMP;
	int		bitmap    = iter_texrgba | iter_additiveblend | rendflags;
	int		color     = 0;

	// specify the angle and angleoffset for the flares
	float distangle = 360.0f / STARGATE_NUM_FLARES;
	float offsangle = distangle * 0.5f;

	// create a genobject particle cluster
	genobject_pcluster_s *pcluster = PRT_CreateGenObjectParticleCluster(
		stargate, STARGATE_NUM_FLARES, NULL, FALSE );

	// flare ring radius
	geomv_t radius = FLOAT_TO_GEOMV( STARGATE_PART_RADIUS );

	// create particles with equal angular distance
	for ( int flare = 0; flare < STARGATE_NUM_FLARES; flare++ ) {

		// calculate the sin/cos for the x/y positions between the stargate solids
		sincosval_s sincosv;
		bams_t flarebams = DEG_TO_BAMS( distangle * flare + offsangle );
		GetSinCos( flarebams, &sincosv );

		// set the position for each flare particle
		Vertex3 origin;
		origin.X = GEOMV_MUL( radius, sincosv.cosval );
		origin.Y = GEOMV_MUL( radius, sincosv.sinval );
		origin.Z = GEOMV_0;

		// init the particle
		particle_s particle;
		PRT_InitParticle( particle, bitmap, color, PRT_NO_SIZEBOUND, ref_z,
			&origin, NULL, INFINITE_LIFETIME, LocalPlayerId, &extinfo );

		// and attach it to the particle cluster
		PRT_AddGenObjectParticle( particle, stargate, pcluster, 0, NULL );

		// advance one texture frame
		extinfo.tex_pos++;

		// wrap around to repeat position if more flares than texture frames
		if ( extinfo.tex_pos > pdef->tex_end ) {
			extinfo.tex_pos = pdef->tex_rep;
		}
	}
}


// allocated tables needed for interior rendering -----------------------------
//
INLINE
void StargateInstantiate_AllocInterior( Stargate *stargate )
{
	ASSERT( stargate != NULL );

	size_t vtxssize  = sizeof( Vertex3 ) * STARGATE_INTERIOR_NUM_VERTS * 2;
	size_t texcosize = sizeof( geomv_t ) * STARGATE_INTERIOR_NUM_VERTS * 2;

	void *memblock = ALLOCMEM( vtxssize + texcosize );
	if ( memblock == NULL )
		OUTOFMEM( "no mem for stargate data." );

	stargate->interior_vtxlist	= (Vertex3 *)
		memblock;
	stargate->interior_viewvtxs	= (Vertex3 *)
		( (char*)memblock + sizeof( Vertex3 ) * STARGATE_INTERIOR_NUM_VERTS );
	stargate->interior_u		= (geomv_t *)
		( (char*)memblock + sizeof( Vertex3 ) * STARGATE_INTERIOR_NUM_VERTS * 2 );
	stargate->interior_v		= (geomv_t *)
		( (char*)stargate->interior_u + sizeof( geomv_t ) * STARGATE_INTERIOR_NUM_VERTS );

	stargate->interior_texmap = NULL;
}


// stargate constructor (class instantiation) ---------------------------------
//
PRIVATE
void StargateInstantiate( CustomObject *base )
{
	ASSERT( base != NULL );
	Stargate *stargate = (Stargate *) base;

#ifndef DISABLE_RING_FLARES

	// init ring flares
	StargateInstantiate_InitFlares( stargate );

#endif // !DISABLE_RING_FLARES

	// alloc interior data
	StargateInstantiate_AllocInterior( stargate );

	// init the interior mesh
	StargateInstantiate_InitInteriorVertices( stargate );

	// only when connected the stargate is dormant by default
	stargate->dormant = NET_ConnectedGMSV() ? TRUE : FALSE;
}


//FIXME:
pdef_s *PDEF_pflare03();


// do one step of the activation sequence -------------------------------------
//
PRIVATE
void StargateAnimate_StepActivationSequence( Stargate *stargate )
{
	ASSERT( stargate != NULL );

	//FIXME:
	// we need a better way to get access to the pcluster attached to the genobject
	// -> set pcluster->animtype to SAT_STARGATE_TUNNEL or something, to allow use
	// of PRT_ObjectHasAttachedClustersOfType

	ASSERT( stargate->pcluster != NULL );

	//FIXME:
	pdef_s *pdef = PDEF_pflare03();

	// check whether the required particle definition is available
	if ( pdef != NULL ) {

		pdef_s *pdef = PDEF_pflare03(); //FIXME: ????

		//TODO: this way resolution changes are not handled
		extern float cur_particle_resoscale;

		// create the extinfo structure
		pextinfo_s extinfo;
		PRT_InitParticleExtInfo( &extinfo, pdef, NULL, NULL );

		particle_s particle;

		dword	rendflags = PART_REND_NONE;
		int		bitmap    = iter_texrgba | iter_specularadd | rendflags;
		int		color     = 0;

		//FIXME: constant
		float ref_z = 50.0f * cur_particle_resoscale;

		//FIXME: constant for number of arcs
		sincosval_s sincosv;
		GetSinCos( DEG_TO_BAMS( 360 / STARGATE_RING_NUM_PARTS ), &sincosv );

		Vertex3 origin;

		Vertex3 velocity;
		velocity.X = GEOMV_0;
		velocity.Y = GEOMV_0;
		velocity.Z = FLOAT_TO_GEOMV( stargate->partvel / ( INT2FLOAT( FRAME_MEASURE_TIMEBASE ) ) );

		// number of particles to create in one step
		int numparticlesonce;
		int curseglen;

		// calculate the number of frames to complete the activation sequence
		ASSERT( CurScreenRefFrames > 0 );
		float fFrames2CompleteActivation = (float)stargate->acttime / CurScreenRefFrames;

		// check whether all particles should be added at once
		if ( fFrames2CompleteActivation <= 1.0f ) {

			numparticlesonce = stargate->numpartactive - stargate->pcluster->numel;

			// set the length of the cylinder segment in this iteration
			curseglen = stargate->actcyllen - stargate->curactcyllen;

		} else {

			numparticlesonce = (int)( (float)stargate->numpartactive / fFrames2CompleteActivation );

			// set the length of the cylinder segment in this iteration
			curseglen = (int)( (float)stargate->actcyllen / fFrames2CompleteActivation );

			// do not allow the tunnel to be longer than actcyllen
			curseglen = min( stargate->actcyllen - stargate->curactcyllen, curseglen );
		}

		stargate->actring = stargate->actring +
			INT2FLOAT( STARGATE_INTERIOR_NUM_RINGS ) / fFrames2CompleteActivation;

		// ensure that at least one particle is added each frame
		numparticlesonce = max( numparticlesonce, 1 );

		// add the particles to the cluster
		for ( int pcnt = 0; pcnt < numparticlesonce; pcnt++ ) {

			// randomize angle around 360 degrees
			// CAVEAT: this relies on RAND_MAX being >= 0x7fff
			ASSERT( RAND_MAX >= 0x7fff );
			bams_t	theta        = ( RAND() & 0x7fff ) << 2;
			float	rad_variance = (float)( ( RAND() % 101 ) - 50 ) / 1000.0f * STARGATE_PART_RADIUS;

			GetSinCos( theta, &sincosv );

			geomv_t radius = FLOAT_TO_GEOMV( STARGATE_PART_RADIUS + rad_variance );

			origin.X = GEOMV_MUL( radius, sincosv.cosval );
			origin.Y = GEOMV_MUL( radius, sincosv.sinval );

			if ( curseglen > 0 ) {
				origin.Z = STARGATE_PART_ZOFFSET -
						   INT_TO_GEOMV( stargate->curactcyllen + ( RAND() % curseglen ) );
			} else {
				origin.Z = STARGATE_PART_ZOFFSET -
						   INT_TO_GEOMV( stargate->curactcyllen );
			}

			// init the particle information
			PRT_InitParticle( particle, bitmap, color, PRT_NO_SIZEBOUND, ref_z,
				&origin, &velocity, INFINITE_LIFETIME, LocalPlayerId, &extinfo );

			PRT_AddGenObjectParticle(
				particle, stargate, stargate->pcluster, 0, StargateAnimate_Particles );
		}

		// advance the activation cylinder length
		stargate->curactcyllen += curseglen;

		// the activation is finished, so reset the flag for activation sequence
		if ( stargate->pcluster->numel >= stargate->numpartactive ) {
			stargate->activating = FALSE;
		}
	}
}


// create activated stargate particles ----------------------------------------
//
PRIVATE
void StargateModify_CreateActiveParticles( CustomObject *base )
{
	ASSERT( base != NULL );
	Stargate *stargate = (Stargate *) base;

	pdef_s *pdef = PDEF_pflare03();
	if ( pdef == NULL ) {
		return;
	}

	// create the genobject particle cluster
	stargate->pcluster = PRT_CreateGenObjectParticleCluster(
		stargate, stargate->numpartactive, StargateAnimate_Particles, FALSE );

	// ensure that cluster is not culled when genobject is culled
	stargate->pcluster->type |= CT_DONT_CULL_WITH_GENOBJECT;

	// set performance hints
	stargate->pcluster->type |= CT_HINT_PARTICLES_IDENTICAL;
	stargate->pcluster->type |= CT_CLUSTER_GLOBAL_EXTINFO;
	stargate->pcluster->type |= CT_HINT_PARTICLES_HAVE_EXTINFO;

	// set the flag to indicate we are currently activating the stargate
	stargate->activating = TRUE;

	// set the begin of the current activation cylinder part
	stargate->curactcyllen = 0;

	stargate->actring = 0.0f;

	// and do the first step of the animation sequence
	StargateAnimate_StepActivationSequence( stargate );
}


// screenspace bounding rectangle ---------------------------------------------
//
struct boundrect_s {

	sgrid_t	min_x;
	sgrid_t	max_x;

	sgrid_t	min_y;
	sgrid_t	max_y;
};


// calculate the conservative 2d bounding rectangle of a genobject in screenspace ----
//
PRIVATE
int CalculateBoundingRectFromBSphere( GenObject *object, boundrect_s* boundrect )
{
	ASSERT( object != NULL );
	ASSERT( boundrect != NULL );

	geomv_t 	target_bsphere = object->BoundingSphere;
	int			target_square;

	Vector3 	pos;
	Vector3		pos_t;

	pos.X = object->ObjPosition[ 0 ][ 3 ];
	pos.Y = object->ObjPosition[ 1 ][ 3 ];
	pos.Z = object->ObjPosition[ 2 ][ 3 ];

	MtxVctMUL( ViewCamera, &pos, &pos_t );

	// don't draw if object is behind local ship
	if ( pos_t.Z <= GEOMV_0 )
		return FALSE;

	// project bounding sphere radius to screenspace
	target_square = GEOMV_TO_COORD( GEOMV_DIV( target_bsphere, pos_t.Z ) );

	// calc position of ship in screenspace
	SPoint sloc;
	PROJECT_TO_SCREEN( pos_t, sloc );

	boundrect->min_x = sloc.X - target_square;
	boundrect->min_y = sloc.Y - target_square;

	boundrect->max_x = sloc.X + target_square;
	boundrect->max_y = sloc.Y + target_square;

	return TRUE;
}


// ----------------------------------------------------------------------------
//
#define PING_WAIT_MAX 	(5 * 60000)

static refframe_t stargate_ping_wait = 0;


// determine whether stargate is dormant or not -------------------------------
//
PRIVATE
void DetermineStargateDormantState( Stargate *stargate )
{
	ASSERT( stargate != NULL );

#ifdef LINKED_PROTOCOL_GAMESERVER
	
	// only while connected
	if ( !NET_ConnectedGMSV() ) {
		return;
	}

	// re-issue a server ping, once every 5 seconds
	if ( ( stargate_ping_wait += CurScreenRefFrames ) > PING_WAIT_MAX ) {

		stargate_ping_wait = 0;

//		int ping = -1;
		if ( stargate->destination_node.address[ 0 ] != 0 ) {

			NET_ServerPing( &stargate->destination_node, TRUE, FALSE );
		}

		//NOTE:
		// ping result is not valid here, since we didn't wait
		// for the arrival of the ping packet. The ping time will
		// be available once the NET_ProcessPingPacket() function
		// has processed the returned packet.

		//NOTE:
		// when a ping packet arrives, NET_ProcessPingPacket()
		// sets Stargate::dormant, Stargate::ping, Stargate::lastpinged.

		refframe_t curtime = SYSs_GetRefFrameCount();

		// if we didn't get a pingreply for quite some time, we assume the
		// server went down or was quit, so we switch the stargate to dormant state
		if ( ( curtime - stargate->lastpinged ) > ( PING_WAIT_MAX * 2 ) ) {

			stargate->dormant = TRUE;
			stargate->ping    = -1;
			stargate->active  = FALSE;
		}
	}

#endif // LINKED_PROTOCOL_GAMESERVER
	
}


// ----------------------------------------------------------------------------
//
PRIVATE
void DrawStargateText( char *text, boundrect_s *boundrect, int top )
{
	int width  = strlen( text ) * CharsetInfo[ HUD_CHARSETNO ].width;
	int height = CharsetInfo[ HUD_CHARSETNO ].height;

	int xpos = boundrect->min_x + ( boundrect->max_x - boundrect->min_x - width ) / 2;
	int ypos = top ? ( boundrect->min_y - height - 2 ) : boundrect->max_y;

	xpos -= 1;
	ypos -= 1;

	width  += 3;
	height += 3;

	// check against screen boundaries
	if ( xpos < 0 )
		return;
	if ( ypos < 0 )
		return;
	if ( ( xpos + width  ) > Screen_Width  )
		return;
	if ( ( ypos + height ) > Screen_Height )
		return;

	D_DrawTrRect( xpos, ypos, width, height, TRTAB_PANELBACK );
	D_WriteTrString( text, xpos + 1, ypos + 2, TRTAB_PANELTEXT );
}


// draw stargate destination text ---------------------------------------------
//
PRIVATE
int StargateDraw_HUD( Stargate *stargate )
{
	ASSERT( stargate != NULL );

	// set the string context for displaying HUD information
	D_SetWStrContext( CharsetInfo[ HUD_CHARSETNO ].charsetpointer,
					  CharsetInfo[ HUD_CHARSETNO ].geompointer,
					  NULL,
					  CharsetInfo[ HUD_CHARSETNO ].width,
					  CharsetInfo[ HUD_CHARSETNO ].height );

	// calculate the screenspace bounding rectangle of the stargate
	boundrect_s boundrect;
	if ( CalculateBoundingRectFromBSphere( stargate, &boundrect ) ) {

		if ( NetConnected == NETWORK_GAME_ON ) {
		
			// draw name of destination system
			DrawStargateText( stargate->destination_name, &boundrect, TRUE );
	
			// draw ping text
			if ( stargate->ping == -1 ) {
				sprintf( paste_str, "system unreachable" );
			} else {
				sprintf( paste_str, "ping: %dms", stargate->ping );
			}
			DrawStargateText( paste_str, &boundrect, FALSE );
		}
	}

	return TRUE;
}


// init interior vertexlist of stargate ---------------------------------------
//
PRIVATE
int StargateInstantiate_InitInteriorVertices( Stargate *stargate )
{
	ASSERT( stargate != NULL );

	// get pointer to texture map
	stargate->interior_texmap = FetchTextureMap( stargate->interior_name );
	if ( stargate->interior_texmap == NULL ) {
		MSGOUT( "texture '%s' was not found.", stargate->interior_name );
		return FALSE;
	}

	// middle point of interior mesh
	stargate->interior_vtxlist[ 0 ].X = INT_TO_GEOMV( 0 );
	stargate->interior_vtxlist[ 0 ].Y = INT_TO_GEOMV( 0 );
	stargate->interior_vtxlist[ 0 ].Z = INT_TO_GEOMV( 0 );

	int uWidth  = ( 1 << stargate->interior_texmap->Width  );
	int vHeight = ( 1 << stargate->interior_texmap->Height );

	geomv_t uScaleFactor = GEOMV_DIV( INT_TO_GEOMV( uWidth  ), FLOAT_TO_GEOMV( 2 * STARGATE_INTERIOR_RADIUS ) );
	geomv_t vScaleFactor = GEOMV_DIV( INT_TO_GEOMV( vHeight ), FLOAT_TO_GEOMV( 2 * STARGATE_INTERIOR_RADIUS ) );

	stargate->interior_u[ 0 ] = GEOMV_MUL( 0 + STARGATE_INTERIOR_RADIUS, uScaleFactor ); /*INT_TO_GEOMV ( ( 1L << stargate->interior_texmap->Width  ) >> 1 );*/
	stargate->interior_v[ 0 ] = GEOMV_MUL( 0 + STARGATE_INTERIOR_RADIUS, vScaleFactor );/*INT_TO_GEOMV ( ( 1L << stargate->interior_texmap->Height ) >> 1 );*/

	hprec_t interior_mesh_angle_deg = 360.0 / STARGATE_INTERIOR_VERTS_PER_RING;
	sincosval_s sincosv;

	for ( int nRingSegment = 0; nRingSegment < STARGATE_INTERIOR_VERTS_PER_RING; nRingSegment++ ) {

		// get the sin/cos for the current ringsegment
		bams_t interior_mesh_bams = DEG_TO_BAMS( interior_mesh_angle_deg * hprec_t(nRingSegment) ) ;
		GetSinCos( interior_mesh_bams, &sincosv );

		for( int nRing = 0; nRing < STARGATE_INTERIOR_NUM_RINGS; nRing++ ) {

			float fRadius = STARGATE_INTERIOR_RADIUS / STARGATE_INTERIOR_NUM_RINGS * ( nRing + 1 );

			int nVertexIndex = ( 1 + ( STARGATE_INTERIOR_VERTS_PER_RING * nRing  + nRingSegment ) );

			Vertex3* pVertex3 = &stargate->interior_vtxlist[ nVertexIndex ];

			pVertex3->X = GEOMV_MUL( FLOAT_TO_GEOMV( fRadius ) , sincosv.cosval );
			pVertex3->Y = GEOMV_MUL( FLOAT_TO_GEOMV( fRadius ) , sincosv.sinval );
			pVertex3->Z = GEOMV_0;

			stargate->interior_u[ nVertexIndex ] = GEOMV_MUL( pVertex3->X + STARGATE_INTERIOR_RADIUS, uScaleFactor );
			stargate->interior_v[ nVertexIndex ] = GEOMV_MUL( pVertex3->Y + STARGATE_INTERIOR_RADIUS, vScaleFactor );
		}
	}

	return TRUE;
}


// modulate the vertices of the interior mesh of the stargate -----------------
//
PRIVATE
int StargateAnimate_ModulateInteriorVertices( Stargate *stargate )
{
	ASSERT( stargate != NULL );

	if ( stargate->modulfade > stargate->modulrad2 ) {
		stargate->modulfade -= ( CurScreenRefFrames / 14.0f );
	} else {
		stargate->modulfade = stargate->modulrad2;
	}

	// modulate the wave
	stargate->modulate_bams += stargate->modulspeed * CurScreenRefFrames;

	sincosval_s sincosv;
	GetSinCos( stargate->modulate_bams, &sincosv );

	Vertex3* pVertex3 = &stargate->interior_vtxlist[ 0 ];

	//pVertex3->X += GEOMV_MUL( FLOAT_TO_GEOMV( 1.0f ) , sincosv.sinval );
	//pVertex3->Y += GEOMV_MUL( FLOAT_TO_GEOMV( 5.0f ) , sincosv.sinval );
	pVertex3->Z  = GEOMV_MUL( FLOAT_TO_GEOMV( stargate->modulrad1 ) , sincosv.sinval );

	for ( int ring = 0; ring < ( STARGATE_INTERIOR_NUM_RINGS - 1 ); ring++ ) {

		bams_t angdelt = DEG_TO_BAMS( 360.0f / ( STARGATE_INTERIOR_NUM_RINGS + 1 ) );
		bams_t angtemp = stargate->modulate_bams + ( ( ring + 1 ) * angdelt );
		GetSinCos( angtemp, &sincosv );

		for( int ringseg = 0; ringseg < STARGATE_INTERIOR_VERTS_PER_RING; ringseg++ ) {

			int vindx = 1 + ( STARGATE_INTERIOR_VERTS_PER_RING * ring + ringseg );
			Vertex3* pVertex3 = &stargate->interior_vtxlist[ vindx ];
			//pVertex3->X += GEOMV_MUL( FLOAT_TO_GEOMV( 1.0f ) , sincosv.sinval );
			//pVertex3->Y += GEOMV_MUL( FLOAT_TO_GEOMV( 5.0f ) , sincosv.sinval );
			pVertex3->Z  = GEOMV_MUL( FLOAT_TO_GEOMV( stargate->modulfade ) , sincosv.sinval );
		}
	}

	return TRUE;
}


#ifndef USE_COMPILED_VTX_ARRAYS

// macro to set the properties of a itervertex --------------------------------
//
#define SET_ITER_VTX( P_ITER_VTX, P_IN_VTX, P_OUT_VTX, P_VTX_U, P_VTX_V, VTX_RED, VTX_GREEN, VTX_BLUE, VTX_ALPHA ) \
		MtxVctMUL( DestXmatrx, (P_IN_VTX), (P_OUT_VTX) ); \
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


// draw interior using triangle strips ----------------------------------------
//
INLINE
void StargateDraw_InteriorTriStrips( Stargate *stargate )
{
	ASSERT( stargate != NULL );

	int numstripverts = STARGATE_INTERIOR_NUM_RINGS * 2 + 1;

	// use multiple tristrips
	IterTriStrip3* itstrip = (IterTriStrip3*)
		ALLOCMEM( (size_t)&((IterTriStrip3*)0)->Vtxs[ numstripverts ] );

	// set vertex color to white
	byte _red   = 255;
	byte _green = 255;
	byte _blue  = 255;
	byte _alpha = 255;

	for ( int ringseg = 0; ringseg < STARGATE_INTERIOR_VERTS_PER_RING; ringseg++ ) {
/*
		_alpha =  0;
		_red   =  0;
		_green =  0;
		_blue  =  0;
*/
		SET_ITER_VTX( &itstrip->Vtxs[ 0 ], &stargate->interior_vtxlist[ 0 ], &stargate->interior_viewvtxs[ 0 ],
				        &stargate->interior_u[ 0 ], &stargate->interior_u[ 0 ], _red, _green, _blue, _alpha );

		int nIndex = 1;
		for( int nRing = 0; nRing < STARGATE_INTERIOR_NUM_RINGS; nRing++ ) {

			// fade alpha to zero
			_alpha = 255 - (nRing + 1) * ( 255 / STARGATE_INTERIOR_NUM_RINGS );

			int _animring = nRing + FLOAT2INT( stargate->actring );
			_animring = max( STARGATE_INTERIOR_NUM_RINGS, _animring );
/*
			_alpha =  FLOAT2INT( INT2FLOAT( _animring ) * ( 255.0f / INT2FLOAT( STARGATE_INTERIOR_NUM_RINGS ) ) );
			_red   =  FLOAT2INT( INT2FLOAT( _animring ) * ( 128.0f / INT2FLOAT( STARGATE_INTERIOR_NUM_RINGS ) ) );
			_green =  FLOAT2INT( INT2FLOAT( _animring ) * ( 128.0f / INT2FLOAT( STARGATE_INTERIOR_NUM_RINGS ) ) );
			_blue  =  FLOAT2INT( INT2FLOAT( _animring ) * ( 128.0f / INT2FLOAT( STARGATE_INTERIOR_NUM_RINGS ) ) );
*/
			int nVertexIndex_1 = 1 + ( STARGATE_INTERIOR_VERTS_PER_RING * nRing  + ringseg );
			int nVertexIndex_2 = 1 + ( STARGATE_INTERIOR_VERTS_PER_RING * nRing  + ( ( ringseg + 1 ) % STARGATE_INTERIOR_VERTS_PER_RING ) );

			Vertex3* pVertex3_1 = &stargate->interior_vtxlist[ nVertexIndex_1 ];
			Vertex3* pVertex3_2 = &stargate->interior_vtxlist[ nVertexIndex_2 ];


			SET_ITER_VTX( &itstrip->Vtxs[ nIndex ], pVertex3_1, &stargate->interior_viewvtxs[ nIndex ],
					        &stargate->interior_u[ nVertexIndex_1 ], &stargate->interior_v[ nVertexIndex_1 ],
							_red, _green, _blue, _alpha );

			nIndex++;
			SET_ITER_VTX( &itstrip->Vtxs[ nIndex ], pVertex3_2, &stargate->interior_viewvtxs[ nIndex ],
					        &stargate->interior_u[ nVertexIndex_2 ], &stargate->interior_v[ nVertexIndex_2 ],
							_red, _green, _blue, _alpha );

			nIndex++;
		}

		itstrip->NumVerts  = numstripverts;
		itstrip->flags     = ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_DIV_UVW |
							 ITERFLAG_Z_TO_DEPTH;
		itstrip->itertype  = iter_texrgba | iter_specularadd;
		itstrip->raststate = rast_zcompare/*rast_zbuffer */| rast_texclamp | rast_chromakeyoff;
		itstrip->rastmask  = rast_nomask;
		itstrip->texmap    = stargate->interior_texmap;

		D_DrawIterTriStrip3( itstrip, 0x3f );
	}

	FREEMEM( itstrip );
}

#endif // !USE_COMPILED_VTX_ARRAYS


#if defined( USE_COMPILED_VTX_ARRAYS ) && defined( USE_STRIPORDER_TRIANGLES )

// draw interior using compiled vertex arrays and strip order triangles -------
//
INLINE
void StargateDraw_InteriorStripOrderTris( IterArray3 *itarray )
{
	ASSERT( itarray != NULL );

	size_t numstrips    = STARGATE_INTERIOR_VERTS_PER_RING;
	size_t numstriptris = STARGATE_INTERIOR_NUM_RINGS * 2 - 1;
	size_t numtriindxs  = numstrips * numstriptris * 3;

	dword *vindxs = (dword *) ALLOCMEM( numtriindxs * sizeof( dword ) );
	if ( vindxs == NULL )
		OUTOFMEM( "no mem for interior indexes." );

	int dstindx = 0;

	// store all strips but last
	int seg = 0;
	for ( seg = 0; seg < STARGATE_INTERIOR_VERTS_PER_RING - 1; seg++ ) {

		int srcindx = 1 + seg;

		vindxs[ dstindx + 0 ] = 0;
		vindxs[ dstindx + 1 ] = srcindx;
		vindxs[ dstindx + 2 ] = srcindx + 1;
		dstindx += 3;
		int ring = 1; 
		for ( ring = 1; ring < STARGATE_INTERIOR_NUM_RINGS; ring++ ) {

			srcindx += STARGATE_INTERIOR_VERTS_PER_RING;

			vindxs[ dstindx + 0 ] = vindxs[ dstindx - 2 ];
			vindxs[ dstindx + 1 ] = vindxs[ dstindx - 1 ];
			vindxs[ dstindx + 2 ] = srcindx;
			dstindx += 3;

			vindxs[ dstindx + 0 ] = vindxs[ dstindx - 2 ];
			vindxs[ dstindx + 1 ] = vindxs[ dstindx - 1 ];
			vindxs[ dstindx + 2 ] = srcindx + 1;
			dstindx += 3;
		}
	}

	// store last strip
	{
		int srcindx = 1 + seg;

		vindxs[ dstindx + 0 ] = 0;
		vindxs[ dstindx + 1 ] = srcindx;
		vindxs[ dstindx + 2 ] = srcindx - seg;
		dstindx += 3;

		for ( int ring = 1; ring < STARGATE_INTERIOR_NUM_RINGS; ring++ ) {

			srcindx += STARGATE_INTERIOR_VERTS_PER_RING;

			vindxs[ dstindx + 0 ] = vindxs[ dstindx - 2 ];
			vindxs[ dstindx + 1 ] = vindxs[ dstindx - 1 ];
			vindxs[ dstindx + 2 ] = srcindx;
			dstindx += 3;

			vindxs[ dstindx + 0 ] = vindxs[ dstindx - 2 ];
			vindxs[ dstindx + 1 ] = vindxs[ dstindx - 1 ];
			vindxs[ dstindx + 2 ] = srcindx - seg;
			dstindx += 3;
		}
	}

	// draw indexed triangles in a single call
	D_DrawIterArrayIndexed(
		ITERARRAY_MODE_TRIANGLES, numtriindxs, vindxs, 0x3f );

	FREEMEM( vindxs );
}

#endif // USE_COMPILED_VTX_ARRAYS && USE_STRIPORDER_TRIANGLES


#if defined( USE_COMPILED_VTX_ARRAYS ) && !defined( USE_STRIPORDER_TRIANGLES )

// draw interior using compiled vertex arrays and indexed strips --------------
//
INLINE
void StargateDraw_InteriorIndexedStrips( IterArray3 *itarray )
{
	ASSERT( itarray != NULL );

	size_t numstripvtxs = STARGATE_INTERIOR_NUM_RINGS * 2 + 1;
	dword *vindxs = (dword *) ALLOCMEM( numstripvtxs * sizeof( dword ) );
	if ( vindxs == NULL )
		OUTOFMEM( "no mem for strip indexes." );

	// apex stays fixed
	vindxs[ 0 ] = 0;

	// draw all strips but last
	for ( int seg = 0; seg < STARGATE_INTERIOR_VERTS_PER_RING - 1; seg++ ) {

		int srcindx = 1 + seg;
		int dstindx = 1;
		for ( int ring = 0; ring < STARGATE_INTERIOR_NUM_RINGS; ring++ ) {

			vindxs[ dstindx++ ] = srcindx;
			vindxs[ dstindx++ ] = srcindx + 1;
			srcindx += STARGATE_INTERIOR_VERTS_PER_RING;
		}

		// draw indexed tristrip
		D_DrawIterArrayIndexed(
			ITERARRAY_MODE_TRISTRIP, numstripvtxs, vindxs, 0x3f );
	}

	// draw last strip
	{
		int srcindx = 1 + seg;
		int dstindx = 1;
		for ( int ring = 0; ring < STARGATE_INTERIOR_NUM_RINGS; ring++ ) {

			vindxs[ dstindx++ ] = srcindx;
			vindxs[ dstindx++ ] = srcindx - seg;
			srcindx += STARGATE_INTERIOR_VERTS_PER_RING;
		}

		// draw indexed tristrip
		D_DrawIterArrayIndexed(
			ITERARRAY_MODE_TRISTRIP, numstripvtxs, vindxs, 0x3f );
	}

	FREEMEM( vindxs );
}

#endif // USE_COMPILED_VTX_ARRAYS && !USE_STRIPORDER_TRIANGLES


#ifdef USE_COMPILED_VTX_ARRAYS

// draw interior using compiled vertex arrays ---------------------------------
//
INLINE
void StargateDraw_InteriorCompiledArrays( Stargate *stargate )
{
	ASSERT( stargate != NULL );

	// create vertex array
	IterArray3 *itarray = (IterArray3 *) ALLOCMEM(
		(size_t)&((IterArray3*)0)->Vtxs[ STARGATE_INTERIOR_NUM_VERTS ] );
	if ( itarray == NULL )
		OUTOFMEM( "STARGATE: no mem for vertex array." );

	itarray->NumVerts	= STARGATE_INTERIOR_NUM_VERTS;
	itarray->arrayinfo	= ITERARRAY_USE_COLOR |
						  ITERARRAY_USE_TEXTURE | ITERARRAY_GLOBAL_TEXTURE;
	itarray->flags		= ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_DIV_UVW |
						  ITERFLAG_Z_TO_DEPTH;
	itarray->itertype	= iter_texrgba | iter_specularadd;
	itarray->raststate	= rast_zcompare/*rast_zbuffer */| rast_texclamp |
						  rast_chromakeyoff;
	itarray->rastmask	= rast_nomask;
	itarray->texmap		= stargate->interior_texmap;

	// fill vertex array
	itarray->Vtxs[ 0 ].X = stargate->interior_vtxlist[ 0 ].X;
	itarray->Vtxs[ 0 ].Y = stargate->interior_vtxlist[ 0 ].Y;
	itarray->Vtxs[ 0 ].Z = stargate->interior_vtxlist[ 0 ].Z;
	itarray->Vtxs[ 0 ].W = GEOMV_1;
	itarray->Vtxs[ 0 ].U = stargate->interior_u[ 0 ];
	itarray->Vtxs[ 0 ].V = stargate->interior_v[ 0 ];
	itarray->Vtxs[ 0 ].R = 255;
	itarray->Vtxs[ 0 ].G = 255;
	itarray->Vtxs[ 0 ].B = 255;
	itarray->Vtxs[ 0 ].A = 255;

	int dst = 1;
	for ( int seg = 0; seg < STARGATE_INTERIOR_VERTS_PER_RING; seg++ ) {
		for( int ring = 0; ring < STARGATE_INTERIOR_NUM_RINGS; ring++ ) {

			itarray->Vtxs[ dst ].X = stargate->interior_vtxlist[ dst ].X;
			itarray->Vtxs[ dst ].Y = stargate->interior_vtxlist[ dst ].Y;
			itarray->Vtxs[ dst ].Z = stargate->interior_vtxlist[ dst ].Z;
			itarray->Vtxs[ dst ].W = GEOMV_1;
			itarray->Vtxs[ dst ].U = stargate->interior_u[ dst ];
			itarray->Vtxs[ dst ].V = stargate->interior_v[ dst ];
			itarray->Vtxs[ dst ].R = itarray->Vtxs[ 0 ].R;
			itarray->Vtxs[ dst ].G = itarray->Vtxs[ 0 ].G;
			itarray->Vtxs[ dst ].B = itarray->Vtxs[ 0 ].B;
			itarray->Vtxs[ dst ].A = itarray->Vtxs[ 0 ].A -
				( ring + 1 ) * ( 255 / STARGATE_INTERIOR_NUM_RINGS );
			dst++;
		}
	}

	// setup transformation matrix
	D_LoadIterMatrix( DestXmatrx );

	// lock array
	D_LockIterArray3( itarray, 0, itarray->NumVerts );

#ifdef USE_STRIPORDER_TRIANGLES

	// draw interior with strip order triangles
	StargateDraw_InteriorStripOrderTris( itarray );

#else // USE_STRIPORDER_TRIANGLES

	// draw interior with indexed triangle strips
	StargateDraw_InteriorIndexedStrips( itarray );

#endif // USE_STRIPORDER_TRIANGLES

	// unlock array
	D_UnlockIterArray();

	// restore identity transformation
	D_LoadIterMatrix( NULL );

	// free vertex array
	FREEMEM( itarray );
}

#endif // USE_COMPILED_VTX_ARRAYS


// draw interior of stargate --------------------------------------------------
//
PRIVATE
int StargateDraw_Interior( void *param )
{
	ASSERT( param != NULL );
	Stargate *stargate = (Stargate *) param;

	// check whether the object is visible at all
	if ( stargate->VisibleFrame != CurVisibleFrame )
		return TRUE;

	if ( stargate->interior_texmap == NULL )
		return FALSE;

	// draw the HUD text stating the stargate target solarsystem
	if ( !ObjCameraActive ) {
		StargateDraw_HUD( stargate );
	}

	// determine whether stargate is dormant or not
	DetermineStargateDormantState( stargate );

	// don't draw interior if stargate is dormant
	// to be consistent with Stargate (movie and tv-series)
	if ( stargate->dormant ) {
		return TRUE;
	}

	// calculate transformation matrix
	MtxMtxMUL( ViewCamera, stargate->ObjPosition, DestXmatrx );

#ifdef USE_COMPILED_VTX_ARRAYS

	// draw interior using compiled vertex arrays
	StargateDraw_InteriorCompiledArrays( stargate );

#else // USE_COMPILED_VTX_ARRAYS

	// draw interior using triangle strips
	StargateDraw_InteriorTriStrips( stargate );

#endif // USE_COMPILED_VTX_ARRAYS

	return TRUE;
}


// callback type and flags ----------------------------------------------------
//
static int callback_type = CBTYPE_DRAW_CUSTOM_ITER | CBFLAG_REMOVE;


// stargate animation callback ------------------------------------------------
//
PRIVATE
int StargateAnimate( CustomObject *base )
{
	ASSERT( base != NULL );
	Stargate *stargate = (Stargate *) base;

	// simply rotate around Z
	ObjRotZ( stargate->ObjPosition, stargate->rotspeed * CurScreenRefFrames );

	// modulate the interior vertices (sinusoidal effect)
	StargateAnimate_ModulateInteriorVertices( stargate );

	// register the drawing callback for drawing the interior of the stargate
	CALLBACK_RegisterCallback( callback_type, StargateDraw_Interior, (void *) base );

	// and do the activation sequence, if activating
	if ( stargate->activating ) {
		StargateAnimate_StepActivationSequence( stargate );
	}

	return TRUE;
}


// stargate destructor (instance destruction) ---------------------------------
//
PRIVATE
void StargateDestroy( CustomObject *base )
{
	ASSERT( base != NULL );
	Stargate *stargate = (Stargate *) base;

	// destroy attached vertex info
	ASSERT( stargate->interior_vtxlist != NULL );
	FREEMEM( stargate->interior_vtxlist );
	stargate->interior_vtxlist	= NULL;
	stargate->interior_viewvtxs	= NULL;
	stargate->interior_u		= NULL;
	stargate->interior_v		= NULL;

	// ensure pending callbacks are destroyed to avoid
	// calling them with invalid pointers
	int numremoved = CALLBACK_DestroyCallback( callback_type, (void *) base );
	ASSERT( numremoved <= 1 );
}


// check whether a ship is in range of a stargate -----------------------------
//
PRIVATE
int ShipInStargateRange( Stargate *stargate, ShipObject *ship, geomv_t range )
{
	ASSERT( stargate != NULL );
	ASSERT( ship != NULL );

	//NOTE:
	// the ship is treated as a sphere for activation
	// range detection.

	Vector3 gatenormal;
	FetchZVector( stargate->ObjPosition, &gatenormal );

	Vertex3 gatepos;
	FetchTVector( stargate->ObjPosition, &gatepos );

	Vertex3 shippos;
	FetchTVector( ship->ObjPosition, &shippos );

	geomv_t shipdot  = -DOT_PRODUCT( &gatenormal, &shippos );
	geomv_t gatedot  = -DOT_PRODUCT( &gatenormal, &gatepos );
	geomv_t distance = shipdot - gatedot;

	// bounding sphere touching in negative halfspace is still ok
	distance += ship->BoundingSphere;

	// not in range if ship in wrong halfspace
	if ( GEOMV_NEGATIVE( distance ) ) {
		return FALSE;
	}

	// in range if ship bounding sphere intersects gate range hemisphere
	range += ship->BoundingSphere * 2;
	return ( distance < range );
}


// check whether a ship is in jump range of a stargate ------------------------
//
PRIVATE
int ShipInStargateJumpRange( Stargate *stargate, ShipObject *ship, geomv_t range )
{
	ASSERT( stargate != NULL );
	ASSERT( ship != NULL );

	//NOTE:
	// the ship is treated as a point for jump
	// range detection.

	Vector3 gatenormal;
	FetchZVector( stargate->ObjPosition, &gatenormal );

	Vertex3 gatepos;
	FetchTVector( stargate->ObjPosition, &gatepos );

	Vertex3 shippos;
	FetchTVector( ship->ObjPosition, &shippos );

	geomv_t shipdot  = -DOT_PRODUCT( &gatenormal, &shippos );
	geomv_t gatedot  = -DOT_PRODUCT( &gatenormal, &gatepos );
	geomv_t distance = shipdot - gatedot;

	// not in range if ship in wrong halfspace ( already behind stargate )
	if ( GEOMV_NEGATIVE( distance ) ) {
		return FALSE;
	}

	// check whether inside of boundingsphere around stargate
	Vector3 stargate_ship;
	VECSUB( &stargate_ship, &shippos, &gatepos );

	geomv_t stargate_ship_len = VctLenX( &stargate_ship );
	if ( stargate_ship_len > stargate->BoundingSphere ) {
		return FALSE;
	}

	// inside the activation distance/range ?
	if ( distance < range ) {

		Vector3 shipnormal;
		FetchZVector( ship->ObjPosition, &shipnormal );

		// ship must be flying approximately head-on into the gate
		//FIXME: cone_angle like for teleporter
		geomv_t dirdot = DOT_PRODUCT( &gatenormal, &shipnormal );
		return ( dirdot > FLOAT_TO_GEOMV( 0.7f ) );
	}

	return FALSE;
}


// stargate collision callback ------------------------------------------------
//
PRIVATE
int StargateCollide( CustomObject *base )
{
	ASSERT( base != NULL );

	Stargate *stargate = (Stargate *) base;

	// dormant stargate is simply graphics
	if ( stargate->dormant ) {
		return TRUE;
	}

	int inrange = 0x00;

	// check local ship
	if ( ShipInStargateRange( stargate, MyShip, stargate->actdistance ) ) {
		inrange |= 0x01;
	}

	// check all other ships
	if ( inrange == 0x00 ) {

		ShipObject *walkships = FetchFirstShip();
		for ( ; walkships; walkships = (ShipObject *) walkships->NextObj ) {

			if ( ShipInStargateRange( stargate, walkships, stargate->actdistance ) ) {
				inrange |= 0x02;
				break;
			}
		}
	}

	// only toggle active when autoactivation is set
	if ( stargate->autoactivate ) {

		if ( inrange != 0x00 ) {

			if ( !stargate->active ) {

				// create the active stargate particles
				StargateModify_CreateActiveParticles( stargate );

				// activate stargate
				stargate->active = TRUE;
			}

		} else {

			// deactivate stargate
			stargate->active = FALSE;

			// and clear the activating sequence to stop adding particles
			stargate->activating = FALSE;
		}
	}

	if ( !stargate->active ) {
		return TRUE;
	}

	if ( inrange == 0x00 ) {
		return TRUE;
	}

#ifdef LINKED_PROTOCOL_GAMESERVER

	// check for fly-through sequence if connected to game server
	if ( !NET_ConnectedGMSV() ) {
		return TRUE;
	}

	// radius of hemisphere used for jump range detection
	geomv_t jumpdistance = FLOAT_TO_GEOMV( stargate->BoundingSphere * 0.25f );

	// check local ship for jump
	if ( ( inrange == 0x01 ) && ShipInStargateJumpRange( stargate, MyShip, jumpdistance ) ) {

		// schedule server jump if not already done
		if ( CurJumpServerNode == NULL ) {

			CurJumpServerNode = (node_t *) ALLOCMEM( sizeof( node_t ) );
			if ( CurJumpServerNode == NULL )
				OUTOFMEM( 0 );
			memcpy( CurJumpServerNode, &stargate->destination_node, sizeof( node_t ) );

			//TODO:
			// initiate animation and sfx
			ShowMessage( "prepare to jump!!!" );
		}

	} else {

		// check all other ships
		ShipObject *walkships = FetchFirstShip();
		for ( ; walkships; walkships = (ShipObject *) walkships->NextObj ) {

			if ( ShipInStargateJumpRange( stargate, walkships, jumpdistance ) ) {

				//FIXME: constants
				stargate->modulfade = 120.0f;
				break;
			}
		}
	}

#endif // LINKED_PROTOCOL_GAMESERVER

	return TRUE;
}

// handle persistency ---------------------------------------------------------
//
int StargatePersistFromStream( CustomObject* base, int tostream, void* rl )
{
	ASSERT( base != NULL );
	ASSERT( tostream == FALSE );
	Stargate* stargate = (Stargate*) base;

	// read from RE
	if ( rl != NULL ) {

		RE_Stargate* re_stg = (RE_Stargate*)rl;
		ASSERT( re_stg != NULL );

		int active_changed		= ( stargate->active       != re_stg->active );
		int autoactivate_changed= ( stargate->autoactivate != re_stg->autoactivate );

		stargate->serverid		= re_stg->serverid;

		stargate->dormant		= re_stg->dormant;
		stargate->active		= re_stg->active;
		stargate->rotspeed		= re_stg->rotspeed;
		stargate->radius		= re_stg->radius;

		stargate->actdistance	= re_stg->actdistance;
		stargate->numpartactive = re_stg->numpartactive;
		stargate->actcyllen		= re_stg->actcyllen;
		stargate->partvel		= re_stg->partvel;
		stargate->modulspeed	= re_stg->modulspeed;
		stargate->modulrad1		= re_stg->modulrad1;
		stargate->modulrad2		= re_stg->modulrad2;

		strncpy( stargate->flare_name,	re_stg->flare_name,	MAX_TEXNAME );
		stargate->flare_name[ MAX_TEXNAME ] = 0;

		strncpy( stargate->interior_name, re_stg->interior_name, MAX_TEXNAME );
		stargate->interior_name[ MAX_TEXNAME ] = 0;

		stargate->acttime		= re_stg->acttime;
		stargate->autoactivate	= re_stg->autoactivate;

		if ( active_changed ) {
			StargateModify_ActiveChanged( stargate );
		}

		if ( autoactivate_changed ) {
			StargateModify_AutoactivateChanged( stargate );
		}
	}

	return TRUE;
}



// register object type for stargate ------------------------------------------
//
PRIVATE
void StargateRegisterCustomType()
{
	custom_type_info_s info;
	memset( &info, 0, sizeof( info ) );

	info.type_name			= "stargate";
	info.type_id			= 0x00000000;
	info.type_size			= sizeof( Stargate );
	info.type_template		= NULL;
	info.type_flags			= CUSTOM_TYPE_DEFAULT;
	info.callback_init		= StargateInitType;
	info.callback_instant	= StargateInstantiate;
	info.callback_destroy	= StargateDestroy;
	info.callback_animate	= StargateAnimate;
	info.callback_collide	= StargateCollide;
	info.callback_notify	= NULL;
	info.callback_persist   = StargatePersistFromStream;

	OBJ_RegisterCustomType( &info );
	CON_RegisterCustomType( info.type_id, Stargate_PropList );
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( G_STGATE )
{
	// register type
	StargateRegisterCustomType();
}



