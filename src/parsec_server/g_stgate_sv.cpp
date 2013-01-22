/*
 * PARSEC - Stargate Model
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:47 $
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
#include "e_defs.h"
#include "sys_defs.h"

// subsystem linkage info
#include "linkinfo.h"

// UNP header
#include "net_wrap.h"

// mathematics header
#include "utl_math.h"

// model header
//#include "utl_model.h"

// local module header
#include "g_stgate.h"

// proprietary module headers
#include "con_info_sv.h"
#include "net_udpdriver.h"
#include "net_util.h"
#include "obj_clas.h"
#include "obj_creg.h"
#include "obj_cust.h"
#include "e_simulator.h"

// flags



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

PRIVATE int		StargateModify_RealizeNode( GenObject* base );
PRIVATE int		StargateModify_ActiveChanged( GenObject* base );
PRIVATE int		StargateModify_AutoactivateChanged( GenObject* base );
PRIVATE void	StargateModify_CreateActiveParticles( CustomObject *base );


// offset definitions into the Stargate ---------------------------------------
//
#define OFS_ROTSPEED		offsetof( Stargate, rotspeed )
#define OFS_RADIUS			offsetof( Stargate, radius )
#define OFS_DESTNAME		offsetof( Stargate, destination_name )
#define OFS_DESTIP			offsetof( Stargate, destination_ip )
#define OFS_DESTPORT		offsetof( Stargate, destination_port )
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


/*// notification callback when eihter IP or port is changed --------------------
//
PRIVATE 
int StargateModify_RealizeNode( GenObject* base )
{
	ASSERT( base != NULL );
	Stargate* stargate = (Stargate *) base;

	// try to resolve DNS name to IP address (still as string)
	char resolved_name[ MAX_IPADDR_LEN + 1 ];
	if ( TheUDPDriver->ResolveHostName( stargate->destination_ip, NULL ) ) {

		// save server address for later use by NET_GMSV.C
		inet_aton( resolved_name, &stargate->destination_node );
		NODE_StorePort( &stargate->destination_node, stargate->destination_port );

		MSGOUT( "stargate->destinaiton %s resolved to %s", stargate->destination_ip, NODE_Print( &stargate->destination_node ) );
	} else {
		// DNS lookup failed
		return FALSE;
	}

	return TRUE;
}*/


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

	if ( ( stargate->active ) && ( !stargate->manually_activated ) ) {
		stargate->manually_activated = TRUE;
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

// stargate constructor (class instantiation) ---------------------------------
//
PRIVATE
void StargateInstantiate( CustomObject *base )
{
	ASSERT( base != NULL );
	Stargate *stargate = (Stargate *) base;
}

/*

// ----------------------------------------------------------------------------
//
#define PING_WAIT_MAX 	(5 * 600)

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

		int ping = -1;
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
*/

// stargate animation callback ------------------------------------------------
//
PRIVATE
int StargateAnimate( CustomObject *base )
{
	ASSERT( base != NULL );
	Stargate *stargate = (Stargate *) base;

	// simply rotate around Z
	ObjRotZ( stargate->ObjPosition, stargate->rotspeed * TheSimulator->GetThisFrameRefFrames() );

	// determine whether stargate is dormant or not
	//DetermineStargateDormantState( stargate );

	return TRUE;
}


// stargate destructor (instance destruction) ---------------------------------
//
PRIVATE
void StargateDestroy( CustomObject *base )
{
	ASSERT( base != NULL );
	Stargate *stargate = (Stargate *) base;
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

	// not in range if ship in wrong halfspace
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

/*	int inrange = 0x00;

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
	geomv_t jumpdistance = FLOAT_TO_GEOMV( stargate->radius * 0.25f );

	// check local ship for jump
	if ( ( inrange == 0x01 ) && ShipInStargateJumpRange( stargate, MyShip, jumpdistance ) ) {

		// schedule server jump if not already done
		if ( CurJumpServer == NULL ) {

			CurJumpServer = (char *) ALLOCMEM( strlen( stargate->destination_ip ) + 1 );
			if ( CurJumpServer == NULL )
				OUTOFMEM( 0 );
			strcpy( CurJumpServer, stargate->destination_ip );

			//TODO:
			// initiate animation and sfx
			ShowMessage( "prepare to jump!!!" );
		}

	} else {

		// check all other ships
		ShipObject *walkships = FetchFirstShip();
		for ( ; walkships; walkships = (ShipObject *) walkships->NextObj ) {

			if ( ShipInStargateJumpRange( stargate, walkships, jumpdistance ) ) {

				stargate->modulfade = 120.0f;
				break;
			}
		}
	}

#endif // LINKED_PROTOCOL_GAMESERVER
*/
	return TRUE;
}

// handle persistency ---------------------------------------------------------
//
int StargatePersistToStream( CustomObject* base, int tostream, void* rl )
{
	ASSERT( base != NULL );
	ASSERT( tostream == TRUE );
	Stargate* stargate = (Stargate*) base;

	// determine size in packet
	size_t size = E_REList::RmEvGetSizeFromType( RE_STARGATE );

	// write to RE list
	if ( rl != NULL ) {

		E_REList* pREList = (E_REList*)rl;
		
		RE_Stargate* re_stg = (RE_Stargate*)pREList->NET_Allocate( RE_STARGATE );
		ASSERT( re_stg != NULL );

		re_stg->serverid	= stargate->serverid;
		
		re_stg->pos[ 0 ]	= stargate->ObjPosition[ 0 ][ 3 ];
		re_stg->pos[ 1 ]	= stargate->ObjPosition[ 1 ][ 3 ];
		re_stg->pos[ 2 ]	= stargate->ObjPosition[ 2 ][ 3 ];
		   
		re_stg->dir[ 0 ]	= stargate->ObjPosition[ 0 ][ 2 ];
		re_stg->dir[ 1 ]	= stargate->ObjPosition[ 1 ][ 2 ];
		re_stg->dir[ 2 ]	= stargate->ObjPosition[ 2 ][ 2 ];

		re_stg->dormant		= stargate->dormant;
		re_stg->active		= stargate->active;
		re_stg->rotspeed	= stargate->rotspeed;
		re_stg->radius		= stargate->radius;
		re_stg->actdistance	= stargate->actdistance;
		re_stg->numpartactive = stargate->numpartactive;
		re_stg->actcyllen	= stargate->actcyllen;
		re_stg->partvel		= stargate->partvel;
		re_stg->modulspeed	= stargate->modulspeed;
		re_stg->modulrad1	= stargate->modulrad1;
		re_stg->modulrad2	= stargate->modulrad2;
		re_stg->acttime		= stargate->acttime;
		re_stg->autoactivate= stargate->autoactivate;

		strncpy( re_stg->flare_name,	stargate->flare_name,	MAX_TEXNAME );
		re_stg->flare_name[ MAX_TEXNAME ] = 0;

		strncpy( re_stg->interior_name, stargate->interior_name, MAX_TEXNAME );
		re_stg->interior_name[ MAX_TEXNAME ] = 0;
	}

	return size;
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
	info.callback_persist   = StargatePersistToStream;

	OBJ_RegisterCustomType( &info );
	CON_RegisterCustomType( info.type_id, Stargate_PropList );
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( G_STGATE_SV )
{
	// register type
	StargateRegisterCustomType();
}
