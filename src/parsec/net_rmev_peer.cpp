/*
 * PARSEC - Remote Events
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:40 $
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
#include "od_class.h"

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "net_defs.h"
#include "sys_defs.h"

// network code config
#include "net_conf.h"

// local module header
#include "net_rmev.h"

// proprietary module headers
#include "con_aux.h"
#include "net_game.h"
#include "net_rmev.h"

// external flag
extern int name_change_notification;


// process entire remote event list (execute all contained events) ------------
//
PRIVATE
void ProcessRmEvList_PEER( NetPacket_PEER* pIntPkt )
{
	ASSERT( NetConnected );
	ASSERT( pIntPkt != NULL );

	NetPacket_PEER* gamepacket = (NetPacket_PEER*)pIntPkt;

	//NOTE:
	// if new remote events are added this function
	// should be updated.

	// identify sender
	int senderid = gamepacket->SendPlayerId;
	ASSERT( ( senderid >= 0 ) && ( senderid < MAX_NET_PROTO_PLAYERS ) );

	RE_Header *relist = (RE_Header *) &gamepacket->RE_List;

	// process remote event list
	while ( relist->RE_Type != RE_EMPTY ) {
		switch ( relist->RE_Type ) {

			case RE_OWNERSECTION:
				// change senderid on RE_OWNERSECTION
				senderid = ( (RE_OwnerSection*)relist )->owner;
				break;
			case RE_DELETED:
				// ignore
				break;

			case RE_CREATEOBJECT:
				NET_ExecRmEvCreateObject( relist, senderid );
				break;

			case RE_CREATELASER:
				NET_ExecRmEvCreateLaser( relist, senderid );
				break;

			case RE_CREATEMISSILE:
				NET_ExecRmEvCreateMissile( relist, senderid );
				break;

			case RE_PARTICLEOBJECT:
				NET_ExecRmEvParticleObject( relist, senderid );
				break;

			case RE_CREATEEXTRA:
				NET_ExecRmEvCreateExtra( relist, senderid );
				break;

			case RE_KILLOBJECT:
				NET_ExecRmEvKillObject( relist );
				break;

			case RE_SENDTEXT:
				NET_ExecRmEvSendText( relist, senderid );
				break;

			case RE_PLAYERNAME:
				NET_ExecRmEvPlayerName( relist, senderid );
				break;

			case RE_WEAPONSTATE:
				NET_ExecRmEvWeaponState( relist, senderid );
				break;

			case RE_STATESYNC:
				NET_ExecRmEvStateSync( relist, senderid );
				break;

			case RE_CREATESWARM:
				NET_ExecRmEvCreateSwarm( relist, senderid );
				break;

			case RE_CREATEEMP:
				NET_ExecRmEvCreateEmp( relist, senderid );
				break;

			case RE_PLAYERSTATUS:
			case RE_PLAYERANDSHIPSTATUS:
			case RE_KILLSTATS:
			case RE_GAMESTATE:
			case RE_COMMANDINFO:
			case RE_CLIENTINFO:
			case RE_CREATEEXTRA2:
			case RE_IPV4SERVERINFO:
			case RE_SERVERLINKINFO:
			case RE_MAPOBJECT:
			case RE_STARGATE:
				// remote event invalid in peer-to-peer mode
				ASSERT( FALSE );
				break;
				
			default:
				MSGOUT( "ProcessRmEvList_PEER(): unknown remote event (%d).", relist->RE_Type );
		}

		// advance to next event in list
		ASSERT( ( relist->RE_BlockSize == RE_BLOCKSIZE_INVALID ) ||
				( relist->RE_BlockSize == NET_RmEvGetSize( relist ) ) );
		relist = (RE_Header *) ( (char *) relist + NET_RmEvGetSize( relist ) );
	}
}


// process NODE_ALIVE packet --------------------------------------------------
//
void NET_ProcessNodeAlive_PEER( NetPacket_PEER* gamepacket )
{
	ASSERT( NetConnected );
	ASSERT( gamepacket != NULL );
	ASSERT( gamepacket->Command == PKTP_NODE_ALIVE );
	
	// identify sender
	int senderid = gamepacket->SendPlayerId;
	ASSERT( ( senderid >= 0 ) && ( senderid < MAX_NET_PROTO_PLAYERS ) );
	
	// player must be connected in order to send NODE_ALIVE
	if ( Player_Status[ senderid ] == PLAYER_INACTIVE ) {
		DBGTXT( MSGOUT( "filtering NODE_ALIVE for player who is not connected: %d.", senderid ); );
		return;
	}
	
	// player should not be joined, only connected when sending NODE_ALIVE
	if ( Player_Status[ senderid ] == PLAYER_JOINED ) {
		DBGTXT( MSGOUT( "warning: received NODE_ALIVE from joined player: %d.", senderid ); );
	}
	
	UPDTXT( MSGOUT( "received node_alive from id %d (pid=%d).", senderid, gamepacket->MessageId ); );
	
	// keep player from being kicked out
	Player_AliveCounter[ senderid ] = MAX_ALIVE_COUNTER;
	
	// update player kill stats
	//NETs_UpdateKillStats( gamepacket );
	
	// process remote event list
	name_change_notification = FALSE;
	ProcessRmEvList_PEER( gamepacket );
	name_change_notification = TRUE;
}


// dump the ship remote info to the console -----------------------------------
//
static
void DumpShipRemInfo( ShipRemInfo* pShipRemInfo )
{
	MSGOUT( "CurYaw %d CurPitch %d CurRoll %d CurSlideHorz %d CurSlideVert %d", pShipRemInfo->CurYaw, 
		pShipRemInfo->CurPitch, 
		pShipRemInfo->CurRoll,
		pShipRemInfo->CurSlideHorz,
		pShipRemInfo->CurSlideVert );
}


// update game state with data of remote player -------------------------------
//
void NET_UpdateGameState_PEER( NetPacket_PEER* gamepacket )
{
	ASSERT( NetConnected );
	ASSERT( gamepacket != NULL );
	ASSERT( gamepacket->Command == PKTP_GAME_STATE );

	// identify sender
	int senderid = gamepacket->SendPlayerId;
	ASSERT( ( senderid >= 0 ) && ( senderid < MAX_NET_PROTO_PLAYERS ) );

	// player must be joined in order to send GAME_STATE
	if ( Player_Status[ senderid ] != PLAYER_JOINED ) {
		DBGTXT( MSGOUT( "filtering GAME_STATE for player who is not joined: %d.", senderid ); );
		return;
	}

	UPDTXT(	MSGOUT( "updating game state from id %d (pid=%d).", senderid, gamepacket->MessageId ); );
	//DumpShipRemInfo( &gamepacket->ShipInfo );

	// keep player from being kicked out
	Player_AliveCounter[ senderid ] = MAX_ALIVE_COUNTER;

	// set up-to-date flag (i.e., no interpolation
	// need be performed since real data is available).
	Player_UpToDate[ senderid ] = TRUE;

	// fetch pointer to remote player's local ship-object
	ShipObject *playerobj = (ShipObject *) Player_Ship[ senderid ];
	ASSERT( playerobj != NULL );

	// ship status will only be updated on newer messages
	if ( gamepacket->MessageId > (dword)Player_LastUpdateGameStateMsgId[ senderid ] ) {

		// set ship remote info
		NET_ExecShipRemInfo( &gamepacket->ShipInfo, playerobj, senderid );

		//FIXME:
		//MSGOUT( "shipstate update from packet %d/%d ( diff = %d )", gamepacket->MessageId, 
		//		   Player_LastUpdateGameStateMsgId[ senderid ], 
		//         gamepacket->MessageId - Player_LastUpdateGameStateMsgId[ senderid ] );
	} else {
		//FIXME:
		MSGOUT( "NET_UpdateGameState_PEER(): supressed update from packet %d ( already had %d )", 
				gamepacket->MessageId,
				Player_LastUpdateGameStateMsgId[ senderid ] );
	}

	// ensure message ids are monotonically increasing
	Player_LastUpdateGameStateMsgId[ senderid ] = max( (dword)Player_LastUpdateGameStateMsgId[ senderid ], gamepacket->MessageId );

	// update player kill stats
	//NETs_UpdateKillStats( gamepacket );

	// process remote event list
	ProcessRmEvList_PEER( gamepacket );
}



