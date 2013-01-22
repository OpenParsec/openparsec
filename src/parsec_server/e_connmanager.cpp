/*
 * PARSEC - Connection Manager
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
#include <ctype.h>
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

// UNP header
#include "net_wrap.h"

// server defs
#include "e_defs.h"

// network code config
#include "net_conf.h"

// net game header
#include "net_game_sv.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "e_connmanager.h"

// proprietary module headers
#include "net_csdf.h"
#include "net_packetdriver.h"
#include "net_util.h"
#include "g_main_sv.h"
#include "e_gameserver.h"
#include "e_simulator.h"
#include "e_simnetoutput.h"
#include "e_packethandler.h"
#include "sys_refframe_sv.h"

// max. number of challenge infos we keep -------------------------------------
//
#define MAX_NUM_CHALLENGEINFOS 1024

// timeout for sent challenges ------------------------------------------------
//
#define CHALLENGE_TIMEOUT_FRAMES		FRAME_MEASURE_TIMEBASE * 10


// ----------------------------------------------------------------------------
// E_ClientInfo public methods
// ----------------------------------------------------------------------------


// copy info from the connection information ----------------------------------
//
void E_ClientInfo::CopyClientConnectInfo( E_ClientConnectInfo* pClientConnectInfo )
{
	ASSERT( pClientConnectInfo != NULL );

	m_nVersionMajor = pClientConnectInfo->m_nVersionMajor;
	m_nVersionMinor = pClientConnectInfo->m_nVersionMinor;

	strncpy( m_szName,	pClientConnectInfo->m_szName, MAX_PLAYER_NAME  );
	m_szName[ MAX_PLAYER_NAME  ] = 0;

	strncpy( m_szOSLine,		pClientConnectInfo->m_szOSLine,		MAX_OSNAME_LEN );
	m_szOSLine[ MAX_OSNAME_LEN   ] = 0;

	strncpy( m_szHostName,		pClientConnectInfo->m_szHostName,	MAX_HOSTNAME_LEN );
	m_szHostName[ MAX_HOSTNAME_LEN ] = 0;

	NODE_Copy( &m_node,	&pClientConnectInfo->m_node );
	m_challenge		= pClientConnectInfo->m_challenge;
	m_nRecvRate		= pClientConnectInfo->m_nRecvRate;
	m_nSendFreq		= pClientConnectInfo->m_nSendFreq;
}

// mark the client alive ------------------------------------------------------
//
void E_ClientInfo::MarkAlive()
{
	m_nAliveCounter = SYSs_GetRefFrameCount() + MAX_ALIVE_COUNTER;
}

// check whether the client is alive ------------------------------------------
//
int E_ClientInfo::IsAlive()
{
	return ( m_nAliveCounter >= SYSs_GetRefFrameCount() );
}


// ----------------------------------------------------------------------------
// E_ConnManager public methods
// ----------------------------------------------------------------------------

// standard ctor --------------------------------------------------------------
//
E_ConnManager::E_ConnManager()
	: m_nMaxNumChallengeInfos( MAX_NUM_CHALLENGEINFOS ),
	  m_nCurChallengeInfo( 0 ),
	  m_ClientInfos( NULL ),
	  m_inDisconnectClient( FALSE )
{
	m_ChallengInfos = new E_ClientChallengeInfo[ MAX_NUM_CHALLENGEINFOS ];
	m_nNumConnected = 0;
	m_ClientInfos	= new E_ClientInfo[ MAX_NUM_CLIENTS ];
}


// standard dtor --------------------------------------------------------------
//
E_ConnManager::~E_ConnManager()
{
	delete []m_ChallengInfos;
	delete []m_ClientInfos;
}


// request a challenge for a client -------------------------------------------
//
int E_ConnManager::RequestChallenge( node_t* clientnode )
{
	ASSERT( clientnode			!= NULL );

	E_ClientChallengeInfo* pCurChallengeInfo = &m_ChallengInfos[ m_nCurChallengeInfo ];

	// generate a new unique challenge
	pCurChallengeInfo->m_challenge = RAND();
	pCurChallengeInfo->m_frame_generated = SYSs_GetRefFrameCount();
	NODE_Copy( &pCurChallengeInfo->m_node, clientnode );

	// point to next challenge info
	m_nCurChallengeInfo++;

	// wrap around
	if ( m_nCurChallengeInfo == m_nMaxNumChallengeInfos ) {
		m_nCurChallengeInfo = 0;
	}

	// send the response back to the client
	ThePacketHandler->SendChallengeResponse( pCurChallengeInfo, clientnode );

	return TRUE;
}

// check whether the client connection is valid -------------------------------
//
int E_ConnManager::CheckClientConnect( E_ClientConnectInfo* pClientConnectInfo )
{
	ASSERT( pClientConnectInfo	!= NULL );

	// default to connection failed
	int rc = FALSE;

	// check whether challenge is found 
	E_ClientChallengeInfo* pFoundChallengeInfo = NULL;
	if ( _IsChallengeCorrect( pClientConnectInfo, &pFoundChallengeInfo ) == FALSE ) {

		// output to logfile
		if ( pFoundChallengeInfo == NULL ) {
			MSGOUT( "challenge for client %s could not be found\n", pClientConnectInfo->m_szHostName );
		} else {
			MSGOUT( "challenge %d for joining client %s invalid - should be %d\n", 
				pClientConnectInfo->m_challenge, 
				pClientConnectInfo->m_szHostName, 
				pFoundChallengeInfo->m_challenge );
		}
		
		// set return code
		ThePacketHandler->SendConnectResponse( CONN_CHALLENGE_INVALID, pClientConnectInfo );

		// connect failed
		rc = FALSE;
	}
	// check whether client is compatible with this version
	else if ( _IsClientCompatible( pClientConnectInfo ) == FALSE ) {

		// output to logfile
		MSGOUT( "client %s with incompatible version %d.%d tried to join\n", pClientConnectInfo->m_szHostName,
			pClientConnectInfo->m_nVersionMajor, pClientConnectInfo->m_nVersionMinor );

		// send the response
		ThePacketHandler->SendConnectResponse( CONN_CLIENT_INCOMAPTIBLE, pClientConnectInfo );
		
		// connect failed
		rc = FALSE;
	}
	// check whether the client is banned
	else if ( _IsClientBanned( pClientConnectInfo ) == TRUE ) {
		
		// output to logfile
		MSGOUT( "banned client %s tried to join\n", pClientConnectInfo->m_szHostName );
		
		// send the response
		ThePacketHandler->SendConnectResponse( CONN_CLIENT_BANNED, pClientConnectInfo );
		
		// connect failed
		rc = FALSE;
	} 
	// check whether the server is not yet full
	else if ( m_nNumConnected < MAX_NUM_CLIENTS ) {
		
		// output to logfile
		MSGOUT( "client %s(%s) wants to connect, name: %s\n", pClientConnectInfo->m_szHostName, NODE_Print( &pClientConnectInfo->m_node ), pClientConnectInfo->m_szName );

		// ensure the client is not yet connected
		_EnsureClientIsDisconnected( pClientConnectInfo );
		
		// check whether the name is already taken
		if ( ( m_nNumConnected > 0 ) && _FindClientName( pClientConnectInfo->m_szName ) != -1 ) {

			ThePacketHandler->SendConnectResponse( CONN_NAME_TAKEN, pClientConnectInfo );

			// connect failed
			rc = FALSE;
		} else {

			// add the client
			int nClientID = _ConnectClient( pClientConnectInfo );
			
			// send the response
			if ( nClientID == -1 ) {

				// output to logfile
				MSGOUT( "Error: _ConnectClient() failed but MaxPlayers not reached" );
				MSGOUT( "client %s refused - server full\n", pClientConnectInfo->m_szHostName );

				// send the SERVER_FULL response
				ThePacketHandler->SendConnectResponse( CONN_SERVER_FULL, pClientConnectInfo );

				// connect failed
				rc = FALSE;

			} else {
				// send the OK response
				ThePacketHandler->SendConnectResponse( CONN_OK, pClientConnectInfo );

				//FIXME: this is already done in _ConnectClient
				// reset the stream to communicate with the client from now on
				//ThePacketDriver->ResetStream( pClientConnectInfo->m_selected_slot );

				// send connect notifications to the other clients and the list of connected clients to the newly connected
				_NotifyClientConnected( nClientID );
				
				// connect OK
				rc = TRUE;
			}
		}
	}
	// the server is full
	else {

		// output to logfile
		MSGOUT( "client %s refused - server full\n", pClientConnectInfo->m_szHostName );

		// send the response
		ThePacketHandler->SendConnectResponse( CONN_SERVER_FULL, pClientConnectInfo );
		
		// connect failed
		rc = FALSE;
	}
	
	return rc;
}


// check for a client disconnect ----------------------------------------------
//
int E_ConnManager::CheckClientDisconnect( node_t* clientnode )
{
	ASSERT( clientnode			!= NULL );

	// find the slot
	int nClientID = _FindSlotWithNode( clientnode );

	// we could find the slot
	if( nClientID != -1 ) {

		// send the response
		ThePacketHandler->SendDisconnectResponse( DISC_OK, clientnode, nClientID );
		
		// disconnect the client in the slot ( must be done after the packets have been sent )
		DisconnectClient( nClientID );
		
		// disconnect was OK
		return TRUE;
	} else {

		// client was not found
		ThePacketHandler->SendDisconnectResponse( DISC_NOT_CONNECTED, clientnode, PLAYERID_ANONYMOUS );
		
		return FALSE;
	}
}

// check for a client namechange ----------------------------------------------
//
int E_ConnManager::CheckNameChange( node_t* clientnode, char* newplayername )
{
	ASSERT( clientnode			!= NULL );
	ASSERT( newplayername		!= NULL );
	
	// find the slot
	int nSlot = _FindSlotWithNode( clientnode );

	// we could find the slot
	if( nSlot != -1 ) {

		MSGOUT( "client %s wants to be renamed to %s\n", NODE_Print( &m_ClientInfos[ nSlot ].m_node ), newplayername );

		// only check for name collision if not alone
		if ( m_nNumConnected > 1 ) {

			// check all the other connected slots for the same name
			for( int nOtherSlot = 0; nOtherSlot < MAX_NUM_CLIENTS; nOtherSlot++ ) {
				
				// check for different slot
				if ( nSlot != nOtherSlot ) {
					
					// check whether slot is free
					if( !m_ClientInfos[ nOtherSlot ].IsSlotFree() ) {
						
						// check whether this name is already taken
						if( strcmp( newplayername, m_ClientInfos[ nOtherSlot ].m_szName) == 0 ) {
							
							// name already taken
							ThePacketHandler->SendNameChangeRepsponse( NAMECHANGE_ALREADY_TAKEN, clientnode, nSlot );
							
							return FALSE;
						}
					}
				}
			}
		}

		// namechange OK
		ThePacketHandler->SendNameChangeRepsponse( NAMECHANGE_OK, clientnode, nSlot );

		// change the name
		_ChangeClientName( nSlot, newplayername );

		// mark the client alive
		m_ClientInfos[ nSlot ].MarkAlive();

		// send namechange notifications to the other clients
		_NotifyClientNameChange( nSlot );
		
		return TRUE;

	} else {
		
		// client is not connected
		ThePacketHandler->SendNameChangeRepsponse( NAMECHANGE_NOT_CONNECTED, clientnode, nSlot );
		
		return FALSE;
	}
}

// get the client information for a specific slot
//
E_ClientInfo* E_ConnManager::GetClientInfo( int nSlot )
{
	ASSERT( nSlot >= 0 );
	ASSERT( nSlot <= MAX_NUM_CLIENTS );

	return &m_ClientInfos[ nSlot ];
}

// check the alive counter of all connected clients and disconnect any timed out clients
//
int E_ConnManager::CheckAliveStatus()
{
	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
		E_ClientInfo* pClientInfo = &m_ClientInfos[ nClientID ];

		if ( !pClientInfo->IsSlotFree() ) {

			// check for client timeout
			if ( !pClientInfo->IsAlive() ) {
				
				MSGOUT( "force-removing timed-out client %s (name: %s)\n", NODE_Print( &pClientInfo->m_node ), pClientInfo->m_szName );

				// send the response
				ThePacketHandler->SendDisconnectResponse( DISC_OK, &pClientInfo->m_node, nClientID );

				// and disconnect the client
				DisconnectClient( nClientID );
			}
		}
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
// E_ConnManager protected methods
// ----------------------------------------------------------------------------


// send client connected notifications to the other clients -------------------
//
int E_ConnManager::_NotifyClientConnected( int nSlotConnected )
{
	ASSERT( nSlotConnected >= 0 );
	ASSERT( nSlotConnected < MAX_NUM_CLIENTS );

	// send 1 notify to all connected clients and N notifies to the connecting client
	for( int nSlot = 0; nSlot < MAX_NUM_CLIENTS; nSlot++ ) {

		if ( !m_ClientInfos[ nSlot ].IsSlotFree() ) {

			if ( nSlot != nSlotConnected ) {
				
				// send the notification to already connected clients
				ThePacketHandler->SendNotifyConnected( nSlot, nSlotConnected );

				// send the notification to the connecting client
				ThePacketHandler->SendNotifyConnected( nSlotConnected, nSlot );
			}
		}
	}
	
	return TRUE;
}


// send client disconnected notifications to the other clients ----------------
//
int E_ConnManager::_NotifyClientDisconnected( int nSlotDisconnected )
{
	ASSERT( nSlotDisconnected >= 0 );
	ASSERT( nSlotDisconnected < MAX_NUM_CLIENTS );

	// only send, if more than 1 connected
	if ( m_nNumConnected > 1 ) {
		for( int nSlot = 0; nSlot < MAX_NUM_CLIENTS; nSlot++ ) {
			
			// do not send to same client AND do not send to unconnected slots
			if ( ( nSlot != nSlotDisconnected ) && !m_ClientInfos[ nSlot ].IsSlotFree() ) {
				
				// send the notification
				ThePacketHandler->SendNotifyDisconnected( nSlot, nSlotDisconnected );
			}
		}
	}


	return TRUE;
}

// send client namechange notifications to the other clients ------------------
//
int E_ConnManager::_NotifyClientNameChange( int nSlotNamechanged )
{
	ASSERT( nSlotNamechanged >= 0 );
	ASSERT( nSlotNamechanged < MAX_NUM_CLIENTS );
	
	// only send, if more than 1 connected
	if ( m_nNumConnected > 1 ) {
		for( int nSlot = 0; nSlot < MAX_NUM_CLIENTS; nSlot++ ) {
		
			// do not send to same client AND do not send to unconnected slots
			if ( ( nSlot != nSlotNamechanged ) && !m_ClientInfos[ nSlot ].IsSlotFree() ) {
				
				// send the notification
				ThePacketHandler->SendNotifyNameChange( nSlot, nSlotNamechanged );
			}
		}
	}
	
	return TRUE;
}



// ensure a client is not connected -------------------------------------------
//
int E_ConnManager::_EnsureClientIsDisconnected( E_ClientConnectInfo* pClientConnectInfo )
{
	ASSERT( pClientConnectInfo					!= NULL );
	ASSERT( pClientConnectInfo->m_szHostName	!= NULL );
	ASSERT( &pClientConnectInfo->m_node          != NULL );
	ASSERT( pClientConnectInfo->m_szName	!= NULL );
	ASSERT( pClientConnectInfo->m_szOSLine		!= NULL );
	
	// check for duplicate connects from same node
	for( int nSlot = 0; nSlot < MAX_NUM_CLIENTS; nSlot++ ) {
		
		E_ClientInfo* pClientInfo = &m_ClientInfos[ nSlot ];
		
		if ( !pClientInfo->IsSlotFree() ) {
			if ( NODE_Compare( &pClientInfo->m_node, &pClientConnectInfo->m_node ) == NODECMP_EQUAL ) {

				MSGOUT( "Removing (possibly) dead client %s", NODE_Print( &pClientConnectInfo->m_node ) );

				// disconnect the client
				DisconnectClient( nSlot );
			}
		}
	}

	return TRUE;
}


// connect a client -----------------------------------------------------------
//
int E_ConnManager::_ConnectClient( E_ClientConnectInfo* pClientConnectInfo )
{
	ASSERT( pClientConnectInfo					!= NULL );
	ASSERT( pClientConnectInfo->m_szHostName	!= NULL );
	ASSERT( &pClientConnectInfo->m_node          != NULL );
	ASSERT( pClientConnectInfo->m_szName	!= NULL );
	ASSERT( pClientConnectInfo->m_szOSLine		!= NULL );

	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {

		E_ClientInfo* pClientInfo = &m_ClientInfos[ nClientID ];

		// check for a free slot
		if ( pClientInfo->IsSlotFree() ) {
		
			// copy the connection info
			pClientInfo->CopyClientConnectInfo( pClientConnectInfo );

			// this slot is used
			pClientInfo->SetSlotFree( FALSE );

			// init the alive counter for this client
			pClientInfo->MarkAlive();

			// return the selected slot
			pClientConnectInfo->m_selected_slot = nClientID;

			// connect the player in the simulation
			TheSimulator->ConnectPlayer( nClientID );

			// connect the player in the game
			TheGame->ConnectPlayer( nClientID );

			// connect stream for this client 
			ThePacketDriver->ConnectStream( nClientID );

			// increase the # of players connected
			m_nNumConnected++;

            return nClientID;
		}
	}

	return -1;
/*						  
							
	svg_Player_RcvdPackets[ i ]		= 0;
	svg_Player_LostPackets[ i ]		= 0;

	SVG_PrintClientList();
	#ifdef GENERATE_HTML
	SVG_GenerateHTMLList();
	#endif

	if ( register_on_masterserver && masterserver_ok ) {
	// update info at masterserver
	SVG_SendPlayersToMasterServer( m_servfd );
	}
*/	
}


// disconnect a client --------------------------------------------------------
//
int E_ConnManager::DisconnectClient( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );

	// check recurse prevention flag
	if ( !m_inDisconnectClient ) {

		// set recurse prevention flag
		m_inDisconnectClient = TRUE;
		
		// send disconnect notifications to the other clients
		_NotifyClientDisconnected( nClientID );

		// clear all the client information
		m_ClientInfos[ nClientID ].Reset();

		// disconnect the player in the simulation
		TheSimulator->DisconnectPlayer( nClientID );

		// disconnect the player in the game
		TheGame->DisconnectPlayer( nClientID );

		// reset stream for this client 
		ThePacketDriver->ResetStream( nClientID );

		// decrease the # of players connected
		m_nNumConnected--;

		// clear recurse prevention flag
		m_inDisconnectClient = FALSE;
	}

	return TRUE;
}

// return the name of the client ----------------------------------------------
//
char* E_ConnManager::GetClientName( int nClientID )
{ 
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	return m_ClientInfos[ nClientID ].m_szName;
}


// change the name of a client ------------------------------------------------
//
int E_ConnManager::_ChangeClientName( int nSlot, char* newplayername )
{
	ASSERT( nSlot >= 0 );
	ASSERT( nSlot < MAX_NUM_CLIENTS );
	ASSERT( newplayername != NULL );

	// copy the new name
	strncpy( m_ClientInfos[ nSlot ].m_szName, newplayername, MAX_PLAYER_NAME );
	m_ClientInfos[ nSlot ].m_szName[ MAX_PLAYER_NAME ] = 0;

	return TRUE;
}


// check if client version is compatible --------------------------------------
//
int E_ConnManager::_IsClientCompatible( E_ClientConnectInfo* pClientConnectInfo )
{
	ASSERT( pClientConnectInfo != NULL );
	
	//NOTE:
	// this is subject to change !!
	// in the future with same major version number should be compatible
	if ( ( pClientConnectInfo->m_nVersionMajor == CLSV_PROTOCOL_MAJOR ) && ( pClientConnectInfo->m_nVersionMinor == CLSV_PROTOCOL_MINOR ) ) {
		return TRUE;
	} else {
		return FALSE;
	}
}

// check if client is banned --------------------------------------------
//
int E_ConnManager::_IsClientBanned( E_ClientConnectInfo* pClientConnectInfo )
{
	ASSERT( pClientConnectInfo != NULL );
	
	//FIXME: implement
	/*for ( int cid = 0; cid < num_banned; cid++ ) {
	if ( strcmp( ip_address, banlist[ cid ] ) == 0 )
	return TRUE;
}*/
	
	return FALSE;
}

// check if challenge is correct ----------------------------------------------
//
int E_ConnManager::_IsChallengeCorrect( E_ClientConnectInfo* pClientConnectInfo, E_ClientChallengeInfo** pFoundChallengeInfo )
{
	ASSERT( pClientConnectInfo != NULL );

	node_t* client_node      = &pClientConnectInfo->m_node;
	int     client_challenge = pClientConnectInfo->m_challenge;

	// linear search from newest challenge
	for( int cid = ( m_nCurChallengeInfo - 1 ); cid != m_nCurChallengeInfo; cid-- ) {

		// check for wraparound
		if ( cid == -1 )
			cid = ( m_nMaxNumChallengeInfos - 1 );

		E_ClientChallengeInfo* pChallengeInfo = &m_ChallengInfos[ cid ];
		
		// check if we have a challenge from this node
		if ( NODE_AreSame( client_node, &pChallengeInfo->m_node ) ) {
			
			// check whether the challenge is the same we sent to the client
			if ( pChallengeInfo->m_challenge == client_challenge ) {
				
				// check whether challenge has not yet timed out
				//FIXME: CHALLENGE_TIMEOUT_FRAMES should be server configurable
				if ( pChallengeInfo->m_frame_generated >= ( SYSs_GetRefFrameCount() - CHALLENGE_TIMEOUT_FRAMES ) ) {
					
					// set the found challenge info
					*pFoundChallengeInfo = &m_ChallengInfos[ cid ];
					
					return TRUE;
				} else {
					return FALSE;
				}
			}
		}
	}




	return FALSE;
}

// check node of client with node used at connection --------------------------
//
int E_ConnManager::CheckNodesMatch( int nClientID, node_t* node )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	ASSERT( node != NULL );

	if ( m_ClientInfos[ nClientID ].IsSlotFree() )
		return FALSE;

	return ( NODE_Compare( &m_ClientInfos[ nClientID ].m_node, node ) == NODECMP_EQUAL );
}


// find the assigned slot for a specific node ---------------------------------
//
int E_ConnManager::_FindSlotWithNode( node_t* clientnode )
{
	ASSERT( clientnode    != NULL );
	
	for( int nSlot = 0; nSlot < MAX_NUM_CLIENTS; nSlot++ ) {
		E_ClientInfo* pClientInfo = &m_ClientInfos[ nSlot ];
		// only check filled slots
		if ( !pClientInfo->IsSlotFree() ) {
			// check for connected client node
			if( NODE_Compare( clientnode, &pClientInfo->m_node ) == NODECMP_EQUAL ) {
				return nSlot;
			}
		}
	}
	
	return -1;
}

// find the slot of a connected client by its name ----------------------------
//
int E_ConnManager::_FindClientName( char* name )
{
	ASSERT( name != NULL );

	for( int nSlot = 0; nSlot < MAX_NUM_CLIENTS; nSlot++ ) {
		if ( !m_ClientInfos[ nSlot ].IsSlotFree() ) {
			// check whether the names match
			if ( stricmp( name, m_ClientInfos[ nSlot ].m_szName ) == 0 ) {
				return nSlot;
			}
		}
	}

	return -1;
}

// set some client-info fields according to RE --------------------------------
//
void E_ConnManager::NET_ExecRmEvClientInfo( int nClientID, RE_ClientInfo* re_clientinfo )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	ASSERT( re_clientinfo != NULL );

	E_ClientInfo& _ClientInfo = m_ClientInfos[ nClientID ];
	if ( !_ClientInfo.IsSlotFree() ) {
		_ClientInfo.m_nSendFreq = re_clientinfo->client_sendfreq;
		_ClientInfo.m_nRecvRate = UNPACK_SERVERRATE( re_clientinfo->server_sendrate );

		//FIXME: resync client with correcter rates, if they are out of bounds
		// apply min/max bounds
		_ClientInfo.m_nSendFreq = max( _ClientInfo.m_nSendFreq, CLIENT_SEND_FREQUENCY_MIN );
		_ClientInfo.m_nSendFreq = min( _ClientInfo.m_nSendFreq, CLIENT_SEND_FREQUENCY_MIN );
		_ClientInfo.m_nRecvRate = max( _ClientInfo.m_nRecvRate, CLIENT_RECV_RATE_MIN );
		_ClientInfo.m_nRecvRate = min( _ClientInfo.m_nRecvRate, CLIENT_RECV_RATE_MAX );

		//DBGTXT( MSGOUT( "E_ConnManager::NET_ExecRmEvClientInfo(): client %d: new CLIENTRATE: %d new SERVERRATE: %d", 
		//	nClientID, _ClientInfo.m_nSendFreq, _ClientInfo.m_nRecvRate ); );

		// reconfigure the network output
		TheSimNetOutput->RateChangedForClient( nClientID );
	}
}
