/*
 * PARSEC - Network Game Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:30 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
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
//#include "aud_defs.h"
#include "net_defs.h"
#include "sys_defs.h"

// mathematics header
#include "utl_math.h"

// network code config
#include "net_conf.h"

// subsystem linkage info
#include "linkinfo.h"

// local module header
#include "net_game_peer.h"

// proprietary module headers
#include "con_aux.h"
//#include "aud_game.h"
//#include "g_supp.h"
//#include "h_supp.h"
#include "net_game.h"
#include "obj_creg.h"
//#include "obj_ctrl.h"
//#include "g_sfx.h"



// set correct function names for dynamic binding------------------------------
//
#ifdef DBIND_PROTOCOL

	#undef	NETs_RmEvList_GetMaxSize
	#undef  NETs_UpdateKillStats

	#define NETs_RmEvList_GetMaxSize		NETs_PEERTOPEER_RmEvList_GetMaxSize
	#define NETs_UpdateKillStats			NETs_PEERTOPEER_UpdateKillStats

#endif


// queue of slot requests received while trying to establish connection -------
//
int				NumSlotRequests = 0;
slotrequest_s	SlotReqQueue[ MAX_SLOT_REQUESTS ];


// return the max. size of the RE list that can fit in one external packet ----
//
size_t NETs_RmEvList_GetMaxSize()
{
	// NOTE: NET_MAX_DATA_LENGTH is either the max. external packet size for IPX or UDP

	return ( NET_MAX_DATA_LENGTH - sizeof( NetPacketExternal_PEER ) );
}


// if this is enabled the kill stats will not be updated from packets ---------
//
extern int kill_stats_frozen;

// do complete kill stats update from received remote event -------------------
//
void NETs_UpdateKillStats( RE_KillStats* killstats )
{
	ASSERT( NetConnected );
	ASSERT( killstats != NULL );

	//NOTE:
	// this function always sets the current kill stat
	// to the maximum of all received kill stats to
	// ensure that no kills are lost.
	
	// needed for kill stats reset
	if ( kill_stats_frozen ) {
		return;
	}
	
	// update entire kill stats list
	for ( int nPlayer = 0; nPlayer < MAX_NET_ALLOC_SLOTS; nPlayer++ ) {
		if ( Player_Status[ nPlayer ] ) {
			Player_KillStat[ nPlayer ] = max( Player_KillStat[ nPlayer ], (int) killstats->PlayerKills[ nPlayer ] );
		}
	}
}

// copy fields from the (old) ShipRemInfo to the (new) RE_PlayerAndShipStatus -
//
PRIVATE
void _ShipRemInfo_To_PlayerAndShipStatus( ShipRemInfo* pShipInfo, RE_PlayerAndShipStatus* pas_status )
{
	memcpy( &pas_status->ObjPosition, pShipInfo->ObjPosition, sizeof( Xmatrx ) );
	pas_status->CurDamage	= pShipInfo->CurDamage;
	pas_status->CurShield	= pShipInfo->CurShield;			
	pas_status->CurSpeed	= pShipInfo->CurSpeed;			
	pas_status->CurYaw		= pShipInfo->CurYaw;				
	pas_status->CurPitch	= pShipInfo->CurPitch;			
	pas_status->CurRoll		= pShipInfo->CurRoll;			
	pas_status->CurSlideHorz= pShipInfo->CurSlideHorz;		
	pas_status->CurSlideVert= pShipInfo->CurSlideVert;		
}


// translate PKTP_JOIN peer-to-peer packet to RE_PlayerAndShipStatus ----------
//
void NET_Translate_PKTP_JOIN( NetPacket_PEER* gamepacket, RE_PlayerAndShipStatus* pas_status )
{
	ASSERT( gamepacket  != NULL );
	ASSERT( pas_status != NULL );
	ASSERT( gamepacket->Command == PKTP_JOIN );
	ASSERT( ( gamepacket->SendPlayerId >= 0 ) && ( gamepacket->SendPlayerId < MAX_NET_PROTO_PLAYERS ) );
	
	pas_status->senderid = gamepacket->SendPlayerId;
	pas_status->objectindex = gamepacket->params[ 3 ];

	// copy fields from ShipRemInfo
	_ShipRemInfo_To_PlayerAndShipStatus( &gamepacket->ShipInfo, pas_status );

	pas_status->params[ 0 ] = gamepacket->params[ 0 ];
	pas_status->params[ 1 ] = gamepacket->params[ 1 ];
	pas_status->params[ 2 ] = gamepacket->params[ 2 ];
	pas_status->params[ 3 ] = gamepacket->params[ 3 ];
	
	// set desired player status
	pas_status->player_status = PLAYER_JOINED;
}

// translate PKTP_GAME_STATE peer-to-peer packet to RE_PlayerAndShipStatus/RE_KillStats 
//
void NET_Translate_PKTP_GAME_STATE( NetPacket_PEER* gamepacket, RE_PlayerAndShipStatus* pas_status, RE_KillStats* pKillStats )
{
	ASSERT( gamepacket  != NULL );
	ASSERT( pas_status != NULL );
	ASSERT( pKillStats  != NULL );
	ASSERT( gamepacket->Command == PKTP_GAME_STATE );
	ASSERT( ( gamepacket->SendPlayerId >= 0 ) && ( gamepacket->SendPlayerId < MAX_NET_PROTO_PLAYERS ) );
	
	pas_status->senderid = gamepacket->SendPlayerId;
	pas_status->objectindex = gamepacket->params[ 3 ];

	// copy fields from ShipRemInfo
	_ShipRemInfo_To_PlayerAndShipStatus( &gamepacket->ShipInfo, pas_status );

	pas_status->params[ 0 ] = gamepacket->params[ 0 ];
	pas_status->params[ 1 ] = gamepacket->params[ 1 ];
	pas_status->params[ 2 ] = gamepacket->params[ 2 ];
	pas_status->params[ 3 ] = gamepacket->params[ 3 ];
	
	// set desired player status
	pas_status->player_status = PLAYER_JOINED;
	
	// fill RE_KillStats
	for( int nPlayer = 0; nPlayer < MAX_NET_ALLOC_SLOTS; nPlayer++ ) {
		pKillStats->PlayerKills[ nPlayer ] = gamepacket->PlayerKills[ nPlayer ];
	}
}

// translate PKTP_UNJOIN peer-to-peer packet to RE_PlayerAndShipStatus/RE_KillStats
//
void NET_Translate_PKTP_UNJOIN( NetPacket_PEER* gamepacket, RE_PlayerAndShipStatus* pas_status, RE_KillStats* pKillStats )
{
	ASSERT( gamepacket  != NULL );
	ASSERT( pas_status != NULL );
	ASSERT( pKillStats  != NULL );
	ASSERT( gamepacket->Command == PKTP_UNJOIN );
	ASSERT( ( gamepacket->SendPlayerId >= 0 ) && ( gamepacket->SendPlayerId < MAX_NET_PROTO_PLAYERS ) );
	
	// fill RE_PlayerAndShipStatus
	pas_status->senderid = gamepacket->SendPlayerId;
	pas_status->objectindex = gamepacket->params[ 3 ];

	// copy fields from ShipRemInfo
	_ShipRemInfo_To_PlayerAndShipStatus( &gamepacket->ShipInfo, pas_status );

	pas_status->params[ 0 ] = gamepacket->params[ 0 ];
	pas_status->params[ 1 ] = gamepacket->params[ 1 ];
	pas_status->params[ 2 ] = gamepacket->params[ 2 ];
	pas_status->params[ 3 ] = gamepacket->params[ 3 ];
	
	// set desired player status
	pas_status->player_status = PLAYER_CONNECTED;
	
	// fill RE_KillStats
	for( int nPlayer = 0; nPlayer < MAX_NET_ALLOC_SLOTS; nPlayer++ ) {
		pKillStats->PlayerKills[ nPlayer ] = gamepacket->PlayerKills[ nPlayer ];
	}
}

// translate PKTP_NODE_ALIVE peer-to-peer packet to RE_PlayerAndShipStatus/RE_KillStats
//
void NET_Translate_PKTP_NODE_ALIVE( NetPacket_PEER* gamepacket, RE_PlayerAndShipStatus* pas_status, RE_KillStats* pKillStats )
{
	ASSERT( gamepacket  != NULL );
	ASSERT( pas_status != NULL );
	ASSERT( pKillStats  != NULL );
	ASSERT( gamepacket->Command == PKTP_NODE_ALIVE );
	ASSERT( ( gamepacket->SendPlayerId >= 0 ) && ( gamepacket->SendPlayerId < MAX_NET_PROTO_PLAYERS ) );
	
	// fill RE_PlayerAndShipStatus
	pas_status->senderid = gamepacket->SendPlayerId;
	pas_status->objectindex = gamepacket->params[ 3 ];

	// copy fields from ShipRemInfo
	_ShipRemInfo_To_PlayerAndShipStatus( &gamepacket->ShipInfo, pas_status );
	
	pas_status->params[ 0 ] = gamepacket->params[ 0 ];
	pas_status->params[ 1 ] = gamepacket->params[ 1 ];
	pas_status->params[ 2 ] = gamepacket->params[ 2 ];
	pas_status->params[ 3 ] = gamepacket->params[ 3 ];
	
	// set desired player status
	pas_status->player_status = PLAYER_CONNECTED;
	
	// fill RE_KillStats
	for( int nPlayer = 0; nPlayer < MAX_NET_ALLOC_SLOTS; nPlayer++ ) {
		pKillStats->PlayerKills[ nPlayer ] = gamepacket->PlayerKills[ nPlayer ];
	}
}


// acquire slot for remote player ---------------------------------------------
//
int NET_AcquireRemoteSlot( node_t *node )
{
	ASSERT( node != NULL );
	
	//NOTE:
	// this function is only used by NET_PEER::ConnectQueue()
	// to acquire an arbitrary slot for a new remote player.
	
	ASSERT( NumRemPlayers > 0 );
	
	int slotid		= SLOTID_CONNECT_REFUSED;
	int numplayers	= NumRemPlayers;
	
	node_t sendnode;
	NETs_MakeNodeRaw( &sendnode, node );
	
	// try all slots
	for ( int id = 0; id < MAX_NET_PROTO_PLAYERS; id++ ) {
		
		if ( Player_Status[ id ] != PLAYER_INACTIVE ) {
			
			// check whether remote player already has an active slot
			node_t slotnode;
			NETs_MakeNodeRaw( &slotnode, &Player_Node[ id ] );
			if ( NETs_CompareNodes( &sendnode, &slotnode ) == NODECMP_EQUAL ) {
				DBGTXT( MSGOUT( "NET_AcquireRemoteSlot(): slot %d reacquired.", id ); );
				slotid		  = id;
				NumRemPlayers = numplayers;
				break;
			}
			
		} else if ( slotid == SLOTID_CONNECT_REFUSED ) {
			
			// take first free slot if not already
			// allocated slot encountered later on
			slotid = id;
			NumRemPlayers++;
		}
	}
	
	return slotid;
}


// append single slot request if not duplicate --------------------------------
//
int NET_AddSlotRequest( node_t *node, char *name, int timetag )
{
	ASSERT( node != NULL );
	ASSERT( name != NULL );

	// compare to old requests
	for ( int creq = 0; creq < NumSlotRequests; creq++ )
		if ( SlotReqQueue[ creq ].slotid != SLOTID_DELETED )
			if ( NETs_CompareNodes( &SlotReqQueue[ creq ].node, node ) == NODECMP_EQUAL )
				return 0;

	if ( NumSlotRequests >= MAX_SLOT_REQUESTS ) {
		MSGOUT( "NET_AddSlotRequest(): too many slot requests." );
		ASSERT( 0 );
		return 0;
	}

	// set special slot id for local host
	int slotid = ( timetag == TIMETAG_LOCALHOST ) ?
		SLOTID_LOCALHOST : SLOTID_NOT_ASSIGNED;

	int qpos = NumSlotRequests++;

	SlotReqQueue[ qpos ].slotid	 = slotid;
	SlotReqQueue[ qpos ].timetag = timetag;
	SlotReqQueue[ qpos ].node	 = *node;
	CopyRemoteName( SlotReqQueue[ qpos ].name, name );

	DBGTXT( MSGOUT( "NET_AddSlotRequest(): appended entry." ); );
	ADXTXT( NETs_PrintNode( node ); );

	// return number of added slot requests
	return 1;
}


// delete single slot request from queue --------------------------------------
//
int NET_DelSlotRequest( node_t *node )
{
	ASSERT( node != NULL );
	
	int skipall   = TRUE;
	int nodefound = FALSE;
	
	// look for request with specified address
	for ( int creq = 0; creq < NumSlotRequests; creq++ )
		if ( SlotReqQueue[ creq ].slotid != SLOTID_DELETED ) {
			if ( NETs_CompareNodes( &SlotReqQueue[ creq ].node, node ) == NODECMP_EQUAL ) {
				// disable request (no actual deletion)
				SlotReqQueue[ creq ].slotid = SLOTID_DELETED;
				nodefound = TRUE;
			} else {
				skipall = FALSE;
			}
		}
		
		if ( nodefound ) {
			DBGTXT( MSGOUT( "NET_DelSlotRequest(): removed entry." ); );
			ADXTXT( NETs_PrintNode( node ); );
		}
		
		// reset queue if only deleted entries found
		if ( ( NumSlotRequests > 0 ) && skipall ) {
			DBGTXT( MSGOUT( "NET_DelSlotRequest(): reset queue." ); );
			NET_ResetSlotReqQueue();
		}
		
		return nodefound;
}


// merge slot request queue of remote host with local queue -------------------
//
int NET_MergeSlotRequests( NetPacket_PEER *gamepacket, int timetag )
{
	ASSERT( gamepacket != NULL );
	
	//NOTE:
	// the timetags stored into the queue are set by
	// the local host. they have nothing to do with
	// the original timetags of the sender. that is,
	// the receive times count, not the send times.
	// (the send times would have no meaning on
	// another host anyway, since there is no
	// synchronized timebase whatsoever.) there are
	// also special timetags like TIMETAG_REPLYNOW.
	
	RE_ConnectQueue *re_cq = (RE_ConnectQueue *) &gamepacket->RE_List;
	
	// return if no queue to merge
	if ( re_cq->RE_Type != RE_CONNECTQUEUE )
		return 0;
	
	ASSERT( re_cq->NumRequests >= 0 );
	ASSERT( re_cq->NumRequests <= MAX_SLOT_REQUESTS );
	
	// process all new slot requests
	int numadded = 0;
	for ( int newreq = 0; newreq < re_cq->NumRequests; newreq++ ) {
		numadded += NET_AddSlotRequest( &re_cq->AddressTable[ newreq ], re_cq->NameTable[ newreq ], timetag );
	}
	return numadded;
}


// delete all entries in slot request queue -----------------------------------
//
void NET_ResetSlotReqQueue()
{
	DBGTXT( MSGOUT( "NET_ResetSlotReqQueue(): reset queue." ); );
	NumSlotRequests = 0;
}

// store list of all remote players as RE_List head ---------------------------
//
void NET_RmEvSinglePlayerTable( NetPacket_PEER* gamepacket )
{
	ASSERT( gamepacket != NULL );

	RE_PlayerList *re_playerlist = (RE_PlayerList *) &gamepacket->RE_List;
	
	ASSERT( re_playerlist->RE_Type == RE_EMPTY );
	re_playerlist->RE_BlockSize  = RE_BLOCKSIZE_INVALID; //sizeof( RE_PlayerList ); *TOO BIG FOR BYTE*

	re_playerlist->RE_Type 	     = RE_PLAYERLIST;

	ASSERT( sizeof( RE_PlayerList ) <= RE_LIST_MAXAVAIL );

	// fill table
	for ( int id = 0; id < MAX_NET_IPX_PEER_PLAYERS; id++ ) {

		// store status
		re_playerlist->Status[ id ] = Player_Status[ id ];

		// store address and name if player connected
		if ( Player_Status[ id ] != PLAYER_INACTIVE ) {

			NETs_MakeNodeRaw( &re_playerlist->AddressTable[ id ], &Player_Node[ id ] );
			CopyRemoteName( re_playerlist->NameTable[ id ], Player_Name[ id ] );
		}

		// if player connected and joined store ship info
		if ( Player_Status[ id ] == PLAYER_JOINED ) {

			ShipObject *shippo = (ShipObject *) Player_Ship[ id ];
			ASSERT( shippo != NULL );

			int shipindex = ObjClassShipIndex[ shippo->ObjectClass ];
			ASSERT( shipindex != SHIPINDEX_NO_SHIP );

			re_playerlist->ShipInfoTable[ id ].ShipIndex = shipindex;
			memcpy( &re_playerlist->ShipInfoTable[ id ].ObjPosition, &shippo->ObjPosition, sizeof( Xmatrx ) );
		}
	}

	// store critical sync values
	re_playerlist->SyncValKillLimit	= AUX_KILL_LIMIT_FOR_GAME_END;
	re_playerlist->SyncValNebulaId	= AUXDATA_BACKGROUND_NEBULA_ID;

	re_playerlist++;
	re_playerlist->RE_Type = RE_EMPTY;

	if ( CurMaxPlayers > 4 ) {
	
		re_playerlist->RE_BlockSize  = RE_BLOCKSIZE_INVALID; //sizeof( RE_PlayerList ); *TOO BIG FOR BYTE*
		re_playerlist->RE_Type 	     = RE_PLAYERLIST;

		ASSERT( ( 2 * sizeof( RE_PlayerList ) ) <= RE_LIST_MAXAVAIL );
		
		// fill table
		for ( int id = 0; id < MAX_NET_IPX_PEER_PLAYERS; id++ ) {
			
			// store status
			re_playerlist->Status[ id ] = Player_Status[ id + 4 ];
			
			// store address and name if player connected
			if ( Player_Status[ id + 4 ] != PLAYER_INACTIVE ) {
				
				NETs_MakeNodeRaw( &re_playerlist->AddressTable[ id ], &Player_Node[ id + 4 ] );
				CopyRemoteName( re_playerlist->NameTable[ id ], Player_Name[ id + 4 ] );
			}
			
			// if player connected and joined store ship info
			if ( Player_Status[ id + 4 ] == PLAYER_JOINED ) {
				
				ShipObject *shippo = (ShipObject *) Player_Ship[ id + 4 ];
				ASSERT( shippo != NULL );
				
				int shipindex = ObjClassShipIndex[ shippo->ObjectClass ];
				ASSERT( shipindex != SHIPINDEX_NO_SHIP );
				
				re_playerlist->ShipInfoTable[ id ].ShipIndex = shipindex;
				memcpy( &re_playerlist->ShipInfoTable[ id ].ObjPosition, &shippo->ObjPosition, sizeof( Xmatrx ) );
			}
		}
		
		// store critical sync values
		re_playerlist->SyncValKillLimit	= AUX_KILL_LIMIT_FOR_GAME_END;
		re_playerlist->SyncValNebulaId	= AUXDATA_BACKGROUND_NEBULA_ID;
		
		re_playerlist++;
		re_playerlist->RE_Type = RE_EMPTY;
	}
}


// store global slot request queue as RE_List head ----------------------------
//
void NET_RmEvSingleConnectQueue( NetPacket_PEER* gamepacket )
{
	ASSERT( gamepacket != NULL );
	
	RE_ConnectQueue *re_connectqueue = (RE_ConnectQueue *) &gamepacket->RE_List;
	ASSERT( re_connectqueue->RE_Type == RE_EMPTY );
	re_connectqueue->RE_Type		 = RE_CONNECTQUEUE;
	re_connectqueue->RE_BlockSize	 = RE_BLOCKSIZE_INVALID; //sizeof( RE_ConnectQueue ); *TOO BIG FOR BYTE*
	
	ASSERT( sizeof( RE_ConnectQueue ) <= RE_LIST_MAXAVAIL );
	
	ASSERT( NumSlotRequests >= 0 );
	ASSERT( NumSlotRequests <= MAX_SLOT_REQUESTS );
	
	// fill table
	int numvalid = 0;
	for ( int req = 0; req < NumSlotRequests; req++ ) {
		
		// skip deleted entries
		if ( SlotReqQueue[ req ].slotid != SLOTID_DELETED ) {
			
			// store node address and name of sender
			CopyRemoteNode( re_connectqueue->AddressTable[ numvalid ],
				SlotReqQueue[ req ].node );
			CopyRemoteName( re_connectqueue->NameTable[ numvalid ],
				SlotReqQueue[ req ].name );
			numvalid++;
		}
	}
	
	// store size of queue
	re_connectqueue->NumRequests = numvalid;
	
	re_connectqueue++;
	re_connectqueue->RE_Type = RE_EMPTY;
}


// set player kill stats in packet to current values --------------------------
//
void NET_SetPacketKillStats( NetPacket_PEER* gamepacket )
{
	ASSERT( NetConnected );
	ASSERT( gamepacket != NULL );
	
	// simply copy kill stats into packet
	for ( int id = 0; id < MAX_NET_ALLOC_SLOTS; id++ ) {
		if ( Player_Status[ id ] ) {
			gamepacket->PlayerKills[ id ] = Player_KillStat[ id ];
		}
	}
}

