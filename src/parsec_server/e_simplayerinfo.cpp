/*
 * PARSEC - Player Information  (Simulation)
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:46 $
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
#include <math.h>

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

// local module header
#include "e_simplayerinfo.h"

// proprietary module headers
#include "con_aux_sv.h"
//#include "g_colldet.h"
//#include "g_extra.h"
////#include "net_csdf.h"
//#include "net_packetdriver.h"
#include "net_game_sv.h"
//#include "net_stream.h"
#include "obj_creg.h"				// for ShipClasses
////#include "e_stats.h"
#include "g_main_sv.h"
#include "e_connmanager.h"
#include "e_gameserver.h"
#include "e_simulator.h"
//#include "e_packethandler.h"
//#include "e_relist.h"
//#include "sys_refframe_sv.h"

// reset all player fields to defaults ( not connected ) ----------------------
//
void E_SimPlayerInfo::Reset()
{
	m_Status			= PLAYER_INACTIVE;
	m_objclass			= -1;

	m_pShip				= NULL;
	m_nShipID			= SHIPID_NOSHIP;

	m_nClientID			= -1;
	m_IgnoreJoinUntilUnjoinFromClient = FALSE;
}

// set the desired player status ( inactive/joined/unjoined ) -----------------
//
void E_SimPlayerInfo::SetDesiredPlayerStatus( RE_PlayerStatus* playerstatus )
{
	ASSERT( playerstatus != NULL );

	// identify sender
	int nClientID = playerstatus->senderid;
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );

	switch( playerstatus->player_status ) {
		// desired player status is INACTIVE (disconnect)
		case PLAYER_INACTIVE:
			{
				ASSERT( FALSE );
			}
			break;
		// desired player_status is JOINED ( join/ensure joined )
		case PLAYER_JOINED:
			{
				// player must not be joined already in order to join
				if ( m_Status == PLAYER_JOINED ) {
					return;
				}
				
				// player must be connected in order to join
				if ( m_Status == PLAYER_INACTIVE ) {
					DBGTXT( MSGOUT( "SetDesiredPlayerStatus(): filtering join for inactive player: %d.", nClientID ); );
					return;
				}

				// check whether to ignore the JOIN
				if ( m_IgnoreJoinUntilUnjoinFromClient ) {
					DBGTXT( MSGOUT( "SetDesiredPlayerStatus(): filtering join until unjoin is ACK: %d.", nClientID ); );
					return;
				}

				// player status must be CONNECTED
				ASSERT( m_Status == PLAYER_CONNECTED );

				// only allow join of players when game is running
				if ( TheGame->IsGameRunning() ) {

					// update tables and create ship
					PerformJoin( playerstatus );

					// player status must now be JOINED 
					ASSERT( m_Status == PLAYER_JOINED );

					DBGTXT( MSGOUT( "SetDesiredPlayerStatus(): joined player %d.", nClientID ); );
				}
			}
			break;
		// desired player_status is CONNECTED ( unjoin/ensure unjoined )
		case PLAYER_CONNECTED:
			{
				// check whether this is an ACK package from the client that he received the unjoin packet from the server
				if ( m_IgnoreJoinUntilUnjoinFromClient && ( m_Status == PLAYER_CONNECTED ) ) {

					// reset flag ( we now again accept all states from the client )
					m_IgnoreJoinUntilUnjoinFromClient = FALSE;

					// reset last unjoin flag to USER_EXIT
					TheGame->ResetDeathInfo( nClientID );
				} else {

					// player must be joined in order to unjoin
					if ( m_Status == PLAYER_CONNECTED ) {
						return;
					}
					
					// player must be connected in order to unjoin
					if ( m_Status == PLAYER_INACTIVE ) {
						DBGTXT( MSGOUT( "SetDesiredPlayerStatus(): filtering unjoin for inactive player: %d.", nClientID ); );
						return;
					}

					// player status must be JOINED
					ASSERT( m_Status == PLAYER_JOINED );

					// update tables and delete ship
					PerformUnjoin( playerstatus );

					// reset the flag
					m_IgnoreJoinUntilUnjoinFromClient = FALSE;

					// player status must now be CONNECTED
					ASSERT( m_Status == PLAYER_CONNECTED );
					
					DBGTXT( MSGOUT( "SetDesiredPlayerStatus(): unjoined player %d.", nClientID ); );
				}
			}
			break;
		default:
			ASSERT( FALSE );
			break;
	}
}

// update tables and create ship for newly joined player ----------------------
//
void E_SimPlayerInfo::PerformJoin( RE_PlayerStatus* playerstatus )
{
	//NOTE: nearly identical to NET_GAME::PerformJoin

	ASSERT( playerstatus != NULL );
	ASSERT( playerstatus->player_status == PLAYER_JOINED );
	ASSERT( m_Status == PLAYER_CONNECTED );

	// update player status
	m_Status = PLAYER_JOINED;

	// there must not be a ship for this player
	ASSERT( m_nShipID == SHIPID_NOSHIP );
	ASSERT( m_pShip   == NULL );

	// get the objectclass
	m_objclass = ShipClasses[ playerstatus->objectindex ];

	ASSERT( (dword)m_objclass != CLASS_ID_INVALID );

	// fetch the latest state
	E_SimClientState* pSimClientState = TheSimulator->GetSimClientState( m_nClientID );

	// init the join position for the new player
	E_SimShipState _SimShipState;
	GAMECODE2( TheGame->JoinPlayer( m_nClientID, &_SimShipState ); );

	// reset the sim-state of the client
	pSimClientState->ForceNewState( &_SimShipState );

	// force a client resync
	pSimClientState->SetClientResync();

	// create ship object for new player
	GenObject* objectpo = TheWorld->CreateObject( m_objclass, _SimShipState.GetObjPosition(), playerstatus->senderid );
	ASSERT( objectpo != NULL );

	// set correct global objectid for player-ship
	objectpo->HostObjNumber = ShipHostObjId( playerstatus->senderid );
	m_pShip	= (ShipObject*)objectpo;
	m_nShipID = objectpo->ObjectNumber;

	// display object id of remote player's ship
	//FIXME: this could go into UI_PlayerJoinedFeedback
	DBGTXT( MSGOUT( "player %s joined and got object id %d.", TheConnManager->GetClientName( m_nClientID ), m_nShipID ); );
	
	MSGOUT( "joined client %d", m_nClientID );

	// give UI feedback when player has joined
	GAMECODE ( UI_PlayerJoinedFeedback( playerstatus ) );
}


// update tables and delete ship of player who unjoined -----------------------
//
void E_SimPlayerInfo::PerformUnjoin( RE_PlayerStatus* playerstatus )
{
	ASSERT( playerstatus != NULL );
	ASSERT( playerstatus->player_status == PLAYER_CONNECTED );
	if(m_Status != PLAYER_JOINED)
        return;
    ASSERT( m_pShip != NULL );
	ASSERT( m_nShipID != SHIPID_NOSHIP );
	
	//FIXME: check whether this should be handled here
		
	// downing of ship starts explosion
	if ( playerstatus->params[ 0 ] == SHIP_DOWNED ) {
		
		// exec gamecode when player ship is downed
		GAMECODE ( GC_UnjoinPlayer_ShipDowned( playerstatus ) );

		//FIXME: we must not yet kill the ShipObject as the E_Distributable might need it
		//FIXME: really ??????????????
		// kill ship object
		TheWorld->KillSpecificShipObject( m_nShipID );
		
		// user exit opens stargate and deletes ship
	} else if ( playerstatus->params[ 0 ] == USER_EXIT ) {
		
		// exec gamecode when player exits to menu
		GAMECODE ( GC_UnjoinPlayer_UserExit( playerstatus ) );

		// kill ship object
		TheWorld->KillSpecificShipObject( m_nShipID );
	}

	// unjoin the player in G_Main
	TheGame->UnjoinPlayer( m_nClientID );

	// signify that player is only connected, but not joined.
	m_Status  = PLAYER_CONNECTED;
	m_pShip   = NULL;
	m_nShipID = SHIPID_NOSHIP;
	m_objclass = -1;

	MSGOUT( "unjoined client %d", m_nClientID ); 
	
	// give UI feedback when player has unjoined
	GAMECODE ( UI_PlayerUnjoinedFeedback( playerstatus ) );
}

// set the player status to connected -----------------------------------------
//
void E_SimPlayerInfo::Connect( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	ASSERT( m_Status == PLAYER_INACTIVE );

	m_nClientID		= nClientID;
	m_Status		= PLAYER_CONNECTED;
}

// set the player status to disconnected --------------------------------------
//
void E_SimPlayerInfo::Disconnect()
{
	ASSERT( ( m_Status == PLAYER_CONNECTED ) || ( m_Status == PLAYER_JOINED ) );

	if ( m_Status == PLAYER_JOINED ) {

		DBGTXT( MSGOUT( "E_SimPlayerInfo::Disconnect() unjoining still joined player before disconnecting" ); );

		// fill rudimentary RE
		RE_PlayerStatus ps;
		ps.player_status	= PLAYER_CONNECTED;
		ps.params[ 0 ]		= USER_EXIT;
		ps.senderid         = m_nClientID;

		// set the state of the player
		SetDesiredPlayerStatus( &ps );
	}

	Reset();
}

