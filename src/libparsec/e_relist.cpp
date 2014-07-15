/*
 * PARSEC - Remote Events handler
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:43 $
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
#include <stdlib.h>
#include <stdio.h>
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

// mathematics header
#include "utl_math.h"

#ifdef PARSEC_SERVER

	// server defs
	#include "e_defs.h"

	// net game header
	#include "net_game_sv.h"

	// proprietary module headers
	#include "con_aux_sv.h"
	#include "g_main_sv.h"
	#include "obj_creg.h"
	#include "e_simplayerinfo.h"
	#include "e_simulator.h"
	#include "e_gameserver.h"
	#include "e_connmanager.h"

#else 

	#include "net_game.h"

#endif // PARSEC_SERVER

// local module header
#include "e_relist.h"


// table of remote event sizes ------------------------------------------------
//
static size_t re_sizes[] = {
	sizeof( RE_Header ),
	sizeof( RE_Header ),
	sizeof( RE_CreateObject ),
	sizeof( RE_CreateLaser ),
	sizeof( RE_CreateMissile ),
	sizeof( RE_CreateExtra ),
	sizeof( RE_KillObject ),
	sizeof( RE_SendText ),
	sizeof( RE_PlayerName ),
	sizeof( RE_ParticleObject ),
	sizeof( RE_PlayerList ),
	sizeof( RE_ConnectQueue ),
	sizeof( RE_WeaponState ),
	sizeof( RE_StateSync ),
	sizeof( RE_CreateSwarm ),
	sizeof( RE_CreateEmp ),
	sizeof( RE_OwnerSection ),
	sizeof( RE_PlayerStatus ),
	sizeof( RE_PlayerAndShipStatus ),
	sizeof( RE_KillStats ),
	sizeof( RE_GameState ),
	sizeof( RE_CommandInfo ),
	sizeof( RE_ClientInfo ),
	sizeof( RE_CreateExtra2 ),
	sizeof( RE_IPv4ServerInfo ),
	sizeof( RE_ServerLinkInfo ),
	sizeof( RE_MapObject ),
	sizeof( RE_Stargate ),
    sizeof( RE_CreateMine),
	sizeof( RE_Teleporter),
	sizeof( RE_Generic),

};

// table of remote event sizes ------------------------------------------------
//
static const char* re_names[] = {
	"RE_EMPTY",			
	"RE_DELETED",			
	"RE_CREATEOBJECT", 	
	"RE_CREATELASER",		
	"RE_CREATEMISSILE",	
	"RE_CREATEEXTRA",		
	"RE_KILLOBJECT",		
	"RE_SENDTEXT", 		
	"RE_PLAYERNAME",		
	"RE_PARTICLEOBJECT",	
	"RE_PLAYERLIST",		
	"RE_CONNECTQUEUE",	
	"RE_WEAPONSTATE",		
	"RE_STATESYNC",		
	"RE_CREATESWARM",		
	"RE_CREATEEMP",		
	"RE_OWNERSECTION",    
	"RE_PLAYERSTATUS",
	"RE_PLAYERANDSHIPSTATUS",
	"RE_KILLSTATS",
	"RE_GAMESTATE",
	"RE_COMMANDINFO",
	"RE_CLIENTINFO",
	"RE_CREATEEXTRA2",		
	"RE_IPV4SERVERINFO",
	"RE_SERVERLINKINFO",
	"RE_MAPOBJECT",
	"RE_STARGATE",
    "RE_CREATEMINE",
    "RE_TELEPORTER",
    "RE_GENERIC"
	//FIXME: use NET_UTIL::PrInf_* functions and E_REList::Dump
};

// default ctor ---------------------------------------------------------------
//
E_REList::E_REList()
{
	m_data		= NULL;
	m_nMaxSize	= 0;
	m_nRefCount = 0;
}

// ctor with init -------------------------------------------------------------
//
E_REList::E_REList( size_t size )
{
	m_data		= NULL;
	m_nMaxSize	= 0;
	m_nRefCount = 0;

	Init( size );
}

// standard dtor --------------------------------------------------------------
//
E_REList::~E_REList()
{
	if ( m_data != NULL ) 
		delete []m_data;
}

// init the remote event list -------------------------------------------------
//
void E_REList::Init( size_t nMaxSize )
{
	ASSERT( nMaxSize > 0 );

	if ( m_data != NULL ) 
		delete []m_data;

	m_nMaxSize = nMaxSize;
	
	// allocate max. size of remote event list
	m_data = new char[ nMaxSize ];
	
	// initialize the list to empty
	( (RE_Header*) m_data)->RE_Type = RE_EMPTY;
	
	m_CurPos    = m_data;
	m_Avail		= nMaxSize;

	ASSERT( m_Avail >= 0 );
}

// clear remote event list ----------------------------------------------------
//
int E_REList::Clear()
{
	ASSERT( m_data != NULL );
	ASSERT( m_nMaxSize > 0 );

	// initialize the list to empty
	( (RE_Header*) m_data)->RE_Type = RE_EMPTY;
	
	m_CurPos    = m_data;
	m_Avail		= m_nMaxSize;

	ASSERT( m_Avail >= 0 );

	return TRUE;
}

// append a E_REList --------------------------------------------------------
//
int E_REList::AppendList( E_REList* relist )
{
	ASSERT( relist != NULL );
	return AppendList( (RE_Header*) relist->m_data );
}


// append a remote event list -------------------------------------------------
//
int E_REList::AppendList( RE_Header* relist )
{
	ASSERT( m_data != NULL );

	// do not append an empty list
	if ( relist->RE_Type == RE_EMPTY )
		return TRUE;

	// determine the size of the list to append
	size_t lsize = DetermineListSize( relist );
	
	ASSERT( lsize <= m_Avail );
	if ( lsize <= m_Avail ) {
		// append the list
		memcpy( (void*)m_CurPos, (void*)relist, lsize );
		
		m_CurPos += lsize;
		m_Avail  -= lsize;
		
		ASSERT( m_Avail >= 0 );

		// ensure proper end for remote event list
		((RE_Header*) m_CurPos)->RE_Type = RE_EMPTY;
		
		return TRUE;
	} else {
		return FALSE;
	}
}

// append a remote event ------------------------------------------------------
//
size_t E_REList::AppendEvent( RE_Header* re, size_t size )
{
	ASSERT( re != NULL );
	ASSERT( RmEvGetSize( re ) == size );
	
	// check whether enough space is left ( account for the trailing RE_EMPTY )
	if ( m_Avail >= ( size + sizeof( RE_Header ) ) ) {

		// copy over the event
		memcpy( (void*)m_CurPos, (void*)re, size );

		m_CurPos += size;
		m_Avail  -= size;

		((RE_Header*)m_CurPos)->RE_Type = RE_EMPTY;

		return size;

	} else {
		return 0;
	}
}




// fill an external remote event list -----------------------------------------
//
size_t E_REList::WriteTo( RE_Header* dst, size_t maxsize, int allow_truncate )
{
	// determine size of remote event list ( include RE_EMPTY byte at end )
	size_t datasize = (size_t)( m_CurPos - m_data );

	if ( datasize == 0 )
		return 0;
	
	// ensure we have enough space
	if ( datasize > maxsize ) {

		if ( !allow_truncate ) {
			ASSERT( FALSE );
			return 0;
		}

		RE_Header*	relist = (RE_Header*)m_data;
		unsigned int filledsize = 0;

		// process remote event list
		while ( ( relist->RE_Type != RE_EMPTY ) && ( filledsize < maxsize ) )  {

			size_t resize = RmEvGetSize( relist );

			if ( ( resize + filledsize ) < maxsize ) {
				memcpy( (void*)dst, (void*)relist, resize * sizeof( char ) );
				dst += resize;

				filledsize += resize;
			}
			
			// advance to next event
			ASSERT( ( relist->RE_BlockSize == RE_BLOCKSIZE_INVALID ) ||
					( relist->RE_BlockSize == resize ) );
			relist = (RE_Header *) ( (char *) relist + resize );
		}

		return filledsize;


	} else {
		memcpy( (void*)dst, m_data, datasize * sizeof( char ) );
		return datasize;
	}
}

// determine size of remote event ---------------------------------------------
//
size_t E_REList::RmEvGetSize( RE_Header *relist )
{
	ASSERT( relist != NULL );
	byte retype = relist->RE_Type;
	
	// use stored size for remote-events of variable size
	if ( ( retype == RE_DELETED ) || ( retype == RE_SENDTEXT ) || ( retype == RE_COMMANDINFO ) ) {
		ASSERT( relist->RE_BlockSize != RE_BLOCKSIZE_INVALID );
		return (size_t)relist->RE_BlockSize;
	}
	
	ASSERT( retype > RE_DELETED );
	ASSERT( retype < RE_NUMEVENTS );
	
	return re_sizes[ retype ];
}

// determine size of remote event ---------------------------------------------
//
size_t E_REList::RmEvGetSizeFromType( byte retype )
{
	// use stored size for remote-events of variable size
	if ( ( retype == RE_DELETED ) || ( retype == RE_SENDTEXT ) || ( retype == RE_COMMANDINFO ) ) {
		return -1;
	}
	
	ASSERT( retype > RE_DELETED );
	ASSERT( retype < RE_NUMEVENTS );
	
	return re_sizes[ retype ];
}


// check whether the remote event list is well formed -------------------------
//
int E_REList::IsWellFormed( RE_Header *relist )
{
	ASSERT( relist != NULL );

	//FIXME: [1/30/2002] implement
	//FIXME: check NET_RMEV::NET_RmEvList_IsWellFormed
	
	//RE_LIST_MAXAVAIL
	
	//ASSERT( FALSE );
	return TRUE;
}

// return the max. size of the RE list that can fit in one external packet ----
//
size_t E_REList::GetMaxSizeInPacket()
{
	return ( NET_UDP_DATA_LENGTH - sizeof( NetPacketExternal_GMSV ) );
}

// dump contents of RE list ---------------------------------------------------
// 
void E_REList::Dump()
{
#ifndef PARSEC_MASTER
	
	RE_Header*	relist		= (RE_Header*)m_data;
	int			nNumEvents	= 0;

	// process remote event list
	while ( relist->RE_Type != RE_EMPTY ) {

		size_t resize = RmEvGetSize( relist );

		if ( relist->RE_Type == RE_COMMANDINFO ) {
			LOGOUT(( "%02d: %s size: %d command: %s", nNumEvents, re_names[ relist->RE_Type ], resize, ((RE_CommandInfo*)relist)->command ));
		} else {
			LOGOUT(( "%02d: %s size: %d", nNumEvents, re_names[ relist->RE_Type ], resize ));
		}
		
		// advance to next event
		ASSERT( ( relist->RE_BlockSize == RE_BLOCKSIZE_INVALID ) ||
				( relist->RE_BlockSize == resize ) );
		relist = (RE_Header *) ( (char *) relist + resize );

		nNumEvents++;
	}

	LOGOUT(( "# of events: %d", nNumEvents ));

#endif // !PARSEC_MASTER
}

// validate the RE according to all bounds ------------------------------------
//
int E_REList::ValidateRE( RE_Header* relist, size_t size )
{
	switch( relist->RE_Type ) {
		case RE_CLIENTINFO:
			{
				RE_ClientInfo* re_clientinfo = (RE_ClientInfo*)relist;
				re_clientinfo->client_sendfreq = max( re_clientinfo->client_sendfreq, (byte) CLIENT_SEND_FREQUENCY_MIN );
				re_clientinfo->client_sendfreq = min( re_clientinfo->client_sendfreq, (byte) CLIENT_SEND_FREQUENCY_MAX );
				re_clientinfo->server_sendrate = max( re_clientinfo->server_sendrate, PACK_SERVERRATE( CLIENT_RECV_RATE_MIN ) );
				re_clientinfo->server_sendrate = min( re_clientinfo->server_sendrate, PACK_SERVERRATE( CLIENT_RECV_RATE_MAX ) );
				return TRUE;
			}
		default:
			return TRUE;
	}
}


// check if enough space in RE_List for specified remote event ----------------
//
int E_REList::RmEvAllowed( int re_type )
{
	ASSERT( re_type > RE_DELETED );
	ASSERT( re_type < RE_NUMEVENTS );
	ASSERT( re_type != RE_SENDTEXT );
	ASSERT( re_type != RE_PLAYERLIST );
	ASSERT( re_type != RE_CONNECTQUEUE );
	ASSERT( re_type != RE_OWNERSECTION );

	return ( m_Avail < re_sizes[ re_type ] );
}

// insert state sync into RE_List ---------------------------------------------
//
int E_REList::RmEvStateSync( byte statekey, byte stateval )
{
	//NOTE:
	// calling E_REList::RmEvAllowed() is mandatory before calling this function.

	if ( m_Avail < sizeof( RE_StateSync ) ) {
		ASSERT( 0 );
		return 0;
	}

	RE_StateSync *re_statesync	= (RE_StateSync *) m_CurPos;
	re_statesync->RE_Type		= RE_STATESYNC;
	re_statesync->RE_BlockSize	= sizeof( RE_StateSync );
	re_statesync->StateKey		= statekey;
	re_statesync->StateValue	= stateval;

	re_statesync++;
	re_statesync->RE_Type		= RE_EMPTY;

	m_CurPos = (char *) re_statesync;
	m_Avail -= sizeof( RE_StateSync );

	return 1;
}

// insert command info into remote event list ---------------------------------
//
int E_REList::NET_Append_RE_CommandInfo( const char* commandstring )
{	
	ASSERT( m_data != NULL );
	ASSERT( commandstring != NULL );
	
	if ( m_Avail < sizeof( RE_CommandInfo ) ) {
		return FALSE;
	}

	size_t len = strlen( commandstring );
	if ( len > MAX_RE_COMMANDINFO_COMMAND_LEN ) {
		len = MAX_RE_COMMANDINFO_COMMAND_LEN;
	}

	RE_CommandInfo* re_commandinfo	= (RE_CommandInfo*) m_CurPos;
	re_commandinfo->RE_Type			= RE_COMMANDINFO;
	re_commandinfo->RE_BlockSize	= len + 1/*string terminator '\0'*/ + 2/*header*/ + 1/*code*/;
	strncpy( re_commandinfo->command , commandstring, MAX_RE_COMMANDINFO_COMMAND_LEN );
	re_commandinfo->command[ MAX_RE_COMMANDINFO_COMMAND_LEN ] = 0;

	m_CurPos += re_commandinfo->RE_BlockSize;
	((RE_Header*)m_CurPos)->RE_Type = RE_EMPTY;

	m_Avail -= sizeof( RE_CommandInfo );

	ASSERT( m_Avail >= 0 );
	
	return TRUE;
}


// insert owner section into remote event list --------------------------------
//
int E_REList::NET_Append_RE_OwnerSection( int ownerid )
{	
	ASSERT( m_data != NULL );
	
	if ( m_Avail < sizeof( RE_OwnerSection ) ) {
		return FALSE;
	}
	
	RE_OwnerSection* re_ownersection = (RE_OwnerSection*) m_CurPos;
	re_ownersection->RE_Type		= RE_OWNERSECTION;
	re_ownersection->RE_BlockSize   = sizeof( RE_OwnerSection );
	re_ownersection->owner			= (byte) ownerid;
	
	re_ownersection++;
	re_ownersection->RE_Type 	   = RE_EMPTY;
	
	m_CurPos = (char *) re_ownersection;
	m_Avail -= sizeof( RE_OwnerSection );

	ASSERT( m_Avail >= 0 );
	
	return TRUE;
}

// append a RE_PlayerAndShipStatus event --------------------------------------
//
int E_REList::NET_Append_RE_PlayerAndShipStatus( int nClientID, E_SimPlayerInfo* pSimPlayerInfo, E_SimShipState* pSimShipState, refframe_t CurRefFrame, bool_t bUpdatePropsOnly )
{	
#ifdef PARSEC_SERVER
	//FIXME: 
	//FIXME: 
	//FIXME: 
	//FIXME: this is GAMECODE !!!
	//FIXME: 
	//FIXME: 
	//FIXME: 

	ASSERT( nClientID >= 0 && nClientID < MAX_NUM_CLIENTS );
	ASSERT( pSimShipState != NULL );

	if ( m_Avail < sizeof( RE_PlayerAndShipStatus ) ) {
		return FALSE;
	}

	RE_PlayerAndShipStatus* re_pas_status	= (RE_PlayerAndShipStatus*) m_CurPos;
	ASSERT( re_pas_status->RE_Type == RE_EMPTY );

	re_pas_status->RE_Type		= RE_PLAYERANDSHIPSTATUS;
	re_pas_status->RE_BlockSize	= sizeof( RE_PlayerAndShipStatus );

	// FIXME: probably set up what to update, but for now, since we are seding the
	// whole pas packet, update all fields

	// FIXME: What the hell?! UF_ALL == 0xFFFF which is a word.  UpdateFlags is a byte member....
	re_pas_status->UpdateFlags  = 0xFF;  //bUpdatePropsOnly ? UF_PROPERTIES : UF_ALL;

	//FIXME: redesign this !!
	//FIXME: we should have flags indicating which fields are transmitted in this event
	re_pas_status->player_status	= pSimPlayerInfo->GetStatus();
	re_pas_status->params[ 0 ]		= pSimPlayerInfo->IsPlayerConnected() ? TheGame->GetPlayerLastUnjoinFlag( nClientID ) : 0;
	re_pas_status->params[ 1 ]		= 0;
	re_pas_status->params[ 2 ]		= pSimPlayerInfo->IsPlayerConnected() ? ( TheGame->GetPlayerLastKiller( nClientID ) + KILLERID_BIAS ) : 0;
	re_pas_status->params[ 3 ]		= 0;
	re_pas_status->senderid			= nClientID;
	//FIXME: is this the global object class id ?
	re_pas_status->objectindex		= ObjClassShipIndex[ pSimPlayerInfo->GetShipObjectClass() ];

	// fill position and speed from sim-state
	memcpy( &re_pas_status->ObjPosition, &pSimShipState->m_ObjPosition, sizeof( Xmatrx ) );
	/*MSGOUT("NET_Append_RE_PlayerAndShipStatus: New Coorids: re_pas: x: %f, y: %f, z:%f.",
			re_pas_status->ObjPosition[0][3],
			re_pas_status->ObjPosition[1][3],
			re_pas_status->ObjPosition[2][3]
			);*/
	re_pas_status->CurSpeed		= pSimShipState->m_CurSpeed;		
	re_pas_status->CurYaw		= pSimShipState->m_CurYaw;			
	re_pas_status->CurPitch		= pSimShipState->m_CurPitch;		
	re_pas_status->CurRoll		= pSimShipState->m_CurRoll;		
	re_pas_status->CurSlideHorz		= pSimShipState->m_CurSlideHorz;	
	re_pas_status->CurSlideVert		= pSimShipState->m_CurSlideVert;	

	// get the properties directly from the ship object
	ShipObject* pShipObject = pSimPlayerInfo->GetShipObject();
	if ( pShipObject != NULL ) {
		re_pas_status->CurDamage	 = pShipObject->CurDamage;
		re_pas_status->CurShield	 = pShipObject->CurShield;
		re_pas_status->CurEnergy	 = pShipObject->CurEnergy;
        re_pas_status->NumMissls    = pShipObject->NumMissls;
        re_pas_status->NumHomMissls = pShipObject->NumHomMissls;
        re_pas_status->NumMines     = pShipObject->NumMines;
	    re_pas_status->NumPartMissls = pShipObject->NumPartMissls;
    }

	DBGTXT( MSGOUT( "sending RE_PlayerAndShipStatus: senderid: %d, status: %d, updateflags: %d", nClientID, pSimPlayerInfo->GetStatus(), re_pas_status->UpdateFlags ); );

	re_pas_status->RefFrame			= CurRefFrame;

	re_pas_status++;
	re_pas_status->RE_Type = RE_EMPTY;

	m_CurPos = (char *) re_pas_status;
	m_Avail -= sizeof( RE_PlayerAndShipStatus );

	ASSERT( m_Avail >= 0 );
#endif // PARSEC_SERVER

	return TRUE;
}

// append a RE_GameState event ------------------------------------------------
//
int E_REList::NET_Append_RE_GameState()
{
#ifdef PARSEC_SERVER

	//FIXME: 
	//FIXME: 
	//FIXME: 
	//FIXME: this is GAMECODE !!!
	//FIXME: 
	//FIXME: 
	//FIXME: 

	if ( m_Avail < sizeof( RE_GameState ) ) {
		return FALSE;
	}

	RE_GameState* re_gamestate = (RE_GameState*) m_CurPos;
	re_gamestate->RE_Type		= RE_GAMESTATE;
	re_gamestate->RE_BlockSize	= sizeof( RE_GameState );

	// fill in the server gametime
	re_gamestate->GameTime = TheGame->GetCurGameTime();
	
	re_gamestate++;
	re_gamestate->RE_Type = RE_EMPTY;

	m_CurPos = (char *) re_gamestate;
	m_Avail -= sizeof( RE_GameState );

	return TRUE;
#else
	ASSERT( FALSE );
	return FALSE;
#endif // PARSEC_SERVER
}

// insert particle object into RE_List ----------------------------------------
//
int E_REList::NET_Append_RE_ParticleObject( int type, const Vertex3& origin )
{
	if ( m_Avail < sizeof( RE_ParticleObject ) ) {
		return FALSE;
	}
        
	RE_ParticleObject *re_particleobject = (RE_ParticleObject *) m_CurPos;
	re_particleobject->RE_Type 	         = RE_PARTICLEOBJECT;
	re_particleobject->RE_BlockSize      = sizeof( RE_ParticleObject );
	re_particleobject->ObjectType        = type;
	re_particleobject->Origin	   		 = origin;
        
	re_particleobject++;
	re_particleobject->RE_Type 	         = RE_EMPTY;
        
	m_CurPos = (char *) re_particleobject;
	m_Avail -= sizeof( RE_ParticleObject );
	
	return TRUE;
}


// append a RE_CreateExtra2 event ----------------------------------------------
//
int E_REList::NET_Append_RE_CreateExtra2( const ExtraObject *extrapo )
{
#ifdef PARSEC_SERVER

	//FIXME: 
	//FIXME: 
	//FIXME: 
	//FIXME: this is GAMECODE !!!
	//FIXME: 
	//FIXME: 
	//FIXME: 

	//NOTE:
	// calling NET_RmEvAllowed() is mandatory
	// before calling this function.

	ASSERT( extrapo != NULL );

	if ( m_Avail < sizeof( RE_CreateExtra2 ) ) {
		return FALSE;
	}

	// translate from objectclass to extra index
	int extraindex = ObjClassExtraIndex[ extrapo->ObjectClass ];
	ASSERT( extraindex != EXTRAINDEX_NO_EXTRA );

	RE_CreateExtra2 *re_ce2 = (RE_CreateExtra2 *) m_CurPos;
	re_ce2->RE_Type 	   = RE_CREATEEXTRA2;
	re_ce2->RE_BlockSize   = sizeof( RE_CreateExtra2 );
	re_ce2->ExtraIndex     = extraindex;
	re_ce2->HostObjId	   = extrapo->HostObjNumber;
	memcpy( &re_ce2->ObjPosition, &extrapo->ObjPosition, sizeof( Xmatrx ) );
	re_ce2->DriftVec.X     = extrapo->DriftVec.X;
	re_ce2->DriftVec.Y     = extrapo->DriftVec.Y;
	re_ce2->DriftVec.Z     = extrapo->DriftVec.Z;
	re_ce2->DriftTimeout   = extrapo->DriftTimeout;

	re_ce2++;
	re_ce2->RE_Type 	   = RE_EMPTY;

	m_CurPos = (char *) re_ce2;
	m_Avail -= sizeof( RE_CreateExtra2 );
    
	return TRUE;
#else
	ASSERT( FALSE );
	return FALSE;
#endif // PARSEC_SERVER
}


// append a RE_CreateLaser event ----------------------------------------------
//
int E_REList::NET_Append_RE_CreateLaser( const LaserObject* laserpo )
{
#ifdef PARSEC_SERVER

	//FIXME: 
	//FIXME: 
	//FIXME: 
	//FIXME: this is GAMECODE !!!
	//FIXME: 
	//FIXME: 
	//FIXME: 


	//NOTE:
	// calling NET_RmEvAllowed() is mandatory before calling this function.

	ASSERT( laserpo != NULL );

	if ( m_Avail < sizeof( RE_CreateLaser ) ) {
		return FALSE;
	}

	RE_CreateLaser *re_createlaser = (RE_CreateLaser *) m_CurPos;
	re_createlaser->RE_Type		   = RE_CREATELASER;
	re_createlaser->RE_BlockSize   = sizeof( RE_CreateLaser );
	//FIXME: objectclass is not synced between client and server
	re_createlaser->ObjectClass    = laserpo->ObjectClass;
	re_createlaser->HostObjId	   = laserpo->HostObjNumber;
	memcpy( &re_createlaser->ObjPosition, &laserpo->ObjPosition, sizeof( Xmatrx ) );
	re_createlaser->DirectionVec   = laserpo->DirectionVec;

	re_createlaser++;
	re_createlaser->RE_Type 	   = RE_EMPTY;

	m_CurPos = (char *) re_createlaser;
	m_Avail -= sizeof( RE_CreateLaser );

	return TRUE;
#else
	ASSERT( FALSE );
	return FALSE;
#endif // PARSEC_SERVER
}

// insert missile into RE_List ------------------------------------------------
//
int E_REList::NET_Append_RE_CreateMissile( const MissileObject *missilepo, dword targetobjid )
{
#ifdef PARSEC_SERVER

	//FIXME: 
	//FIXME: 
	//FIXME: 
	//FIXME: this is GAMECODE !!!
	//FIXME: 
	//FIXME: 
	//FIXME: 
	
    //NOTE:
	// calling NET_RmEvAllowed() is mandatory
	// before calling this function.

	ASSERT( missilepo != NULL );

	
    if ( m_Avail < sizeof( RE_CreateMissile ) ) {
		return FALSE;
	}		

	RE_CreateMissile *re_createmissl = (RE_CreateMissile *) m_CurPos;;
	re_createmissl->RE_Type 		 = RE_CREATEMISSILE;
	re_createmissl->RE_BlockSize	 = sizeof( RE_CreateMissile );
	re_createmissl->ObjectClass 	 = missilepo->ObjectClass;
	re_createmissl->HostObjId		 = missilepo->HostObjNumber;
	re_createmissl->TargetHostObjId  = targetobjid;
	memcpy( &re_createmissl->ObjPosition, &missilepo->ObjPosition, sizeof( Xmatrx ) );
	re_createmissl->DirectionVec	 = missilepo->DirectionVec;

	re_createmissl++;
	re_createmissl->RE_Type 		 = RE_EMPTY;

	m_CurPos = (char *) re_createmissl;
	m_Avail -= sizeof( RE_CreateMissile );


	return TRUE;
#else
    return FALSE;
#endif //Parsec Server
}

// append a RE_KillOjbect event -----------------------------------------------
//
int E_REList::NET_Append_RE_KillObject( dword objectid, byte listno )
{
#ifdef PARSEC_SERVER

	//FIXME: 
	//FIXME: 
	//FIXME: 
	//FIXME: this is GAMECODE !!!
	//FIXME: 
	//FIXME: 
	//FIXME: 

#ifdef NO_PROJECTILE_KILL_MESSAGES

	// for projectiles depend on local client lifetime expiration
	if ( ( listno == LASER_LIST ) || ( listno == MISSL_LIST ) ) {
		return 1;
	}
	ASSERT( listno == EXTRA_LIST );

#endif

	if ( m_Avail < sizeof( RE_KillObject ) ) {
		return FALSE;
	}

	RE_KillObject *re_killobject = (RE_KillObject *) m_CurPos;
	re_killobject->RE_Type		 = RE_KILLOBJECT;
	re_killobject->RE_BlockSize  = sizeof( RE_KillObject );
	re_killobject->HostObjId	 = objectid;
	re_killobject->ListId		 = listno;

	re_killobject++;
	re_killobject->RE_Type		 = RE_EMPTY;

	m_CurPos = (char *) re_killobject;
	m_Avail -= sizeof( RE_KillObject );

	return TRUE;
#else
	ASSERT( FALSE );
	return FALSE;
#endif // PARSEC_SERVER
}


// append a RE_KillStats event ------------------------------------------------
//
int E_REList::NET_Append_RE_KillStats()
{
#ifdef PARSEC_SERVER

	//FIXME: 
	//FIXME: 
	//FIXME: 
	//FIXME: this is GAMECODE !!!
	//FIXME: 
	//FIXME: 
	//FIXME: 

	if ( m_Avail < sizeof( RE_KillStats ) ) {
		return FALSE;
	}

	RE_KillStats* re_killstats = (RE_KillStats*) m_CurPos;
	re_killstats->RE_Type		= RE_KILLSTATS;
	re_killstats->RE_BlockSize	= sizeof( RE_KillStats );

	// fill in the server gametime
	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
		re_killstats->PlayerKills[ nClientID ] = TheGame->GetPlayerKills( nClientID );
	}
	
	re_killstats++;
	re_killstats->RE_Type = RE_EMPTY;

	m_CurPos = (char*) re_killstats;
	m_Avail -= sizeof( RE_KillStats );

	return TRUE;
#else
	ASSERT( FALSE );
	return FALSE;
#endif // PARSEC_SERVER
}

// append a RE_IPv4ServerInfo event -------------------------------------------
//
int E_REList::NET_Append_RE_IPv4ServerInfo( node_t* node, word nServerID, int xpos, int ypos, word flags )
{


	ASSERT( node != NULL );

	if ( m_Avail < sizeof( RE_IPv4ServerInfo ) ) {
		return FALSE;
	}

	RE_IPv4ServerInfo* re_ipv4serverinfo = (RE_IPv4ServerInfo*) m_CurPos;
	re_ipv4serverinfo->RE_Type			 = RE_IPV4SERVERINFO;
	re_ipv4serverinfo->RE_BlockSize		 = sizeof( RE_IPv4ServerInfo );

	memcpy( re_ipv4serverinfo->node, node, NODE_ADR_LENGTH );

	re_ipv4serverinfo->flags	= flags;
	re_ipv4serverinfo->serverid = nServerID;
	re_ipv4serverinfo->xpos     = xpos;
	re_ipv4serverinfo->ypos     = ypos;
	
	re_ipv4serverinfo++;
	re_ipv4serverinfo->RE_Type = RE_EMPTY;

	m_CurPos = (char *) re_ipv4serverinfo;
	m_Avail -= sizeof( RE_IPv4ServerInfo );

	return TRUE;
}

// append a RE_ServerLinkInfo event -------------------------------------------
//
int E_REList::NET_Append_RE_ServerLinkInfo( word nServerID_1, word nServerID_2, word flags )
{
	if ( m_Avail < sizeof( RE_ServerLinkInfo ) ) {
		return FALSE;
	}

	RE_ServerLinkInfo* re_sli	= (RE_ServerLinkInfo*) m_CurPos;
	re_sli->RE_Type			= RE_SERVERLINKINFO;
	re_sli->RE_BlockSize	= sizeof( RE_ServerLinkInfo );

	re_sli->flags			= flags;
	re_sli->serverid1		= nServerID_1;
	re_sli->serverid2		= nServerID_2;
	
	re_sli++;
	re_sli->RE_Type			= RE_EMPTY;

	m_CurPos = (char *) re_sli;
	m_Avail -= sizeof( RE_ServerLinkInfo );

	return TRUE;
}


// append a RE_Generic -------------------------------------------
//
int E_REList::NET_Append_RE_Generic( word flags, dword HostObjId, dword TargetId, dword padding )
{
	if ( m_Avail < sizeof( RE_Generic ) ) {
		return FALSE;
	}

	RE_Generic* re_gen	= (RE_Generic*) m_CurPos;
	re_gen->RE_Type			= RE_GENERIC;
	re_gen->RE_BlockSize	= sizeof( RE_Generic );

	re_gen->RE_ActionFlags	= flags;
	re_gen->HostObjId		= HostObjId;
	re_gen->TargetId		= TargetId;
	re_gen->Padding			= padding;
	
	re_gen++;
	re_gen->RE_Type			= RE_EMPTY;

	m_CurPos = (char *) re_gen;
	m_Avail -= sizeof( RE_Generic );

	return TRUE;
}

// append a RE_MapObject ------------------------------------------------------
//
int E_REList::NET_Append_RE_MapObject( int map_objectid, char* name, int xpos, int ypos, int w, int h, char* texname )
{
	ASSERT( ( map_objectid >= 0 ) && ( map_objectid < MAX_MAP_OBJECTS ) );
	ASSERT( name    != NULL );
	ASSERT( texname != NULL );

	if ( m_Avail < sizeof( RE_MapObject ) ) {
		return FALSE;
	}

	RE_MapObject* re_mo		= (RE_MapObject*) m_CurPos;
	re_mo->RE_Type			= RE_MAPOBJECT;
	re_mo->RE_BlockSize		= sizeof( RE_MapObject );

	re_mo->map_objectid		= map_objectid;
	strncpy( re_mo->name, name, MAX_MAP_OBJ_NAME );
	re_mo->name[ MAX_MAP_OBJ_NAME ] = 0;
	re_mo->xpos = xpos;
	re_mo->ypos = ypos;
	re_mo->w    = w;
	re_mo->h    = h;
	strncpy( re_mo->texname, texname, MAX_TEXNAME);
	re_mo->texname[ MAX_TEXNAME ] = 0;
	
	re_mo++;
	re_mo->RE_Type			= RE_EMPTY;

	m_CurPos = (char *) re_mo;
	m_Avail -= sizeof( RE_MapObject );

	return TRUE;
}

// allocate space for a specific RE -------------------------------------------
//
RE_Header* E_REList::NET_Allocate( int retype ) 
{
	ASSERT( ( retype >= RE_EMPTY   ) && ( retype < RE_NUMEVENTS ) );
	ASSERT( ( retype != RE_DELETED ) && ( retype != RE_SENDTEXT ) && ( retype != RE_COMMANDINFO ) );
	
	ssize_t size = RmEvGetSizeFromType( retype );
	ASSERT( size != -1 );

	if( m_Avail < (size_t)size ) {
		return NULL;
	}

	RE_Header* pBegin		= (RE_Header*)m_CurPos;
	pBegin->RE_Type         = retype;
	pBegin->RE_BlockSize	= size;
	
	m_CurPos += size;
	m_Avail  -= size;

	((RE_Header*)m_CurPos)->RE_Type = RE_EMPTY;

	return pBegin;
}
