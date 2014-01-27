/*
 * PARSEC - Packet handler
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:46 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001-2003
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

#include <vector>

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

// mathematics header
#include "utl_math.h"

// subsystem & server headers 
#include "net_defs.h"
#include "e_defs.h"
#include "e_global_sv.h"
#include "sys_defs.h"

// network code config
#include "net_conf.h"

// net game header
#include "net_game_sv.h"

// local module header
#include "e_packethandler.h"

// SYS subsystem

// proprietary module headers
#include "con_aux_sv.h"
#include "net_csdf.h"
#include "net_packetdriver.h"
#include "net_udpdriver.h"
#include "net_stream.h"
#include "e_simplayerinfo.h"
#include "e_simnetinput.h"
#include "g_main_sv.h"
#include "e_connmanager.h"
#include "e_gameserver.h"
#include "e_relist.h"
#include "e_simulator.h"
#include "MasterServer.h"

// ----------------------------------------------------------------------------
// public methods
// ----------------------------------------------------------------------------

// handle the content of a packet received ------------------------------------
//
void E_PacketHandler::HandlePacket( PacketChainBlock* block )
{
	ASSERT( block != NULL );

	NetPacket_GMSV*	gamepacket	= block->gamepacket;
	int				nSenderID   = gamepacket->SendPlayerId;
	int				bufid		= block->bufferno;
	
	UPDTXT2(MSGOUT( "e_PacketHandler::HandlePacket(): command:%d", gamepacket->Command ));

	// check for senderid spoofing
	if ( !_IsLegitSender( gamepacket, bufid ) ) {
		return;
	}
	
	// if we are a Master server, process packets a bit differently
	if(TheServer->GetServerIsMaster()){
		// TODO: Process Master Server Packets
		switch( gamepacket->Command ) {
			case PKTP_COMMAND:


				/*if ( nSenderID == PLAYERID_MASTERSERVER ) {

					// Hmmmm... PLAYERID_MASTERSERVER is us.. we shouldn't
					// be sending packets to ourselves....  log an error
					// and return
					MSGOUT("e_PacketHandler::HandlePacket(): We sent ourselves a packet?! I think *not*!!!");
					return;

				} else {*/

					// handle a command packet from a client
					_Handle_COMMAND_MASTER( gamepacket, bufid );
				//}


				break;
			case PKTP_STREAM:
				if ( nSenderID == PLAYERID_MASTERSERVER ) {

					// Hmmmm... PLAYERID_MASTERSERVER is us.. we shouldn't
					// be sending packets to ourselves....  log an error
					// and return
					MSGOUT("e_PacketHandler::HandlePacket(): We sent ourselves a packet?! I think *not*!!!");
					return;

				} else {

					// handle a command packet from a client
					_Handle_STREAM_MASTER( gamepacket, bufid );
				}

				break;

			default:
				MSGOUT("Curious Packet Received: %d", gamepacket->Command);
				break;
		}
		// return to do no further processing.
		return;
	}

	// perform action according to packet type
	switch( gamepacket->Command ) {
		
			// these must not be received
		case PKTP_CONNECT:
		case PKTP_CONNECT_REPLY:
		case PKTP_DISCONNECT:
		case PKTP_SLOT_REQUEST:
		case PKTP_SUBDUE_SLAVE:
			DBGTXT( MSGOUT( "e_PacketHandler::HandlePacket: received PEER-only packet ( command: %d )", gamepacket->Command ); );
			
			//NOTE:
			// these should be ignored transparently since somebody
			// might try to connect peer-to-peer on the same
			// segment 
			
			//			ASSERT( 0 );
			break;

		case PKTP_JOIN:
		case PKTP_UNJOIN:
		case PKTP_GAME_STATE:
		case PKTP_NODE_ALIVE:
			//NOTE: legacy packet - removed in new GameServer ( >= build 0198 )
			DBGTXT( MSGOUT( "e_PacketHandler::HandlePacket: received legacy packet ( command: %d )", gamepacket->Command ); );
			break;

		case PKTP_STREAM:
			// handle all the events/proc.calls in the stream packet
			_Handle_STREAM( gamepacket );
			break;

		case PKTP_COMMAND:

			if ( nSenderID == PLAYERID_MASTERSERVER ) {

				// handle a command packet from the masterserver
				_Handle_COMMAND_MASV( gamepacket, bufid );

			} else {

				// handle a command packet from a client
				_Handle_COMMAND( gamepacket, bufid );
			}

			break;

		default:
			MSGOUT( "gameloop: received packet of unknown type: %d.", gamepacket->Command );
			ASSERT( 0 );
	}
}

// notify a client that another client connected ------------------------------
//
void E_PacketHandler::SendNotifyConnected( int nDestSlot, int nClientSlot )
{
	// get the client information for the client, we want to inform
	E_ClientInfo* pDestClientInfo = TheConnManager->GetClientInfo( nDestSlot );
	
	// get the client information for the client, we want the destnode to inform about
	E_ClientInfo* pAboutClientInfo = TheConnManager->GetClientInfo( nClientSlot );
	
	ASSERT( pDestClientInfo  != NULL );
	ASSERT( pAboutClientInfo != NULL );

	ASSERT( !pDestClientInfo->IsSlotFree() );
	
	// we do not notify the client about himself
	if ( NODE_Compare( &pAboutClientInfo->m_node, &pDestClientInfo->m_node ) == NODECMP_EQUAL ) {
		ASSERT( FALSE );
		return;
	}

	// prepare the command string
	char sendline[ MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ];
	snprintf( sendline, sizeof( sendline ), LISTSTR_ADDED_IN_SLOT, nClientSlot, pAboutClientInfo->m_szName );
	
	// send the server command ( RELIABLE )
	Send_COMMAND( sendline, &pDestClientInfo->m_node, nDestSlot, TRUE );
}

// notify a client that another client is disconnected ------------------------
//
void E_PacketHandler::SendNotifyDisconnected( int nDestSlot, int nClientSlot )
{
	// get the client information for the client, we want to inform
	E_ClientInfo* pDestClientInfo = TheConnManager->GetClientInfo( nDestSlot );
	
	// get the client information for the client, we want the destnode to inform about
	E_ClientInfo* pAboutClientInfo = TheConnManager->GetClientInfo( nClientSlot );
	
	ASSERT( pDestClientInfo  != NULL );
	ASSERT( pAboutClientInfo != NULL );

	ASSERT( !pDestClientInfo->IsSlotFree() );
	
	// we do not notify the client about himself
	if ( NODE_Compare( &pAboutClientInfo->m_node, &pDestClientInfo->m_node ) == NODECMP_EQUAL ) {
		ASSERT( FALSE );
		return;
	}
	
	// prepare the command string
	char sendline[ MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ];
	snprintf( sendline, sizeof(sendline), LISTSTR_REMOVED_FROM_SLOT, nClientSlot );

	// send the server command ( RELIABLE )
	Send_COMMAND( sendline, &pDestClientInfo->m_node, nDestSlot, TRUE );
}

// notify a client that another client's name changed -------------------------
//
void E_PacketHandler::SendNotifyNameChange( int nDestSlot, int nClientSlot )
{
	// get the client information for the client, we want to inform
	E_ClientInfo* pDestClientInfo = TheConnManager->GetClientInfo( nDestSlot );
	
	// get the client information for the client, we want the destnode to inform about
	E_ClientInfo* pAboutClientInfo = TheConnManager->GetClientInfo( nClientSlot );
	
	ASSERT( pDestClientInfo  != NULL );
	ASSERT( pAboutClientInfo != NULL );
	
	// we do not notify the client about himself
	if ( NODE_Compare( &pAboutClientInfo->m_node, &pDestClientInfo->m_node ) == NODECMP_EQUAL ) {
		ASSERT( FALSE );
		return;
	}
	
	// prepare the command string
	char sendline[ MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ];
	snprintf( sendline, sizeof(sendline), LISTSTR_NAME_UPDATED, nClientSlot, pAboutClientInfo->m_szName );
	
	// send the server command ( RELIABLE )
	Send_COMMAND( sendline, &pDestClientInfo->m_node, nDestSlot, TRUE );
}


// send a challenge response back to a client ---------------------------------
//
void E_PacketHandler::SendChallengeResponse( E_ClientChallengeInfo* pChallengeInfo, node_t* clientnode )
{
	ASSERT( pChallengeInfo != NULL );
	ASSERT( clientnode     != NULL );
	
	// prepare response
	char sendline[ MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ];
	snprintf( sendline, sizeof(sendline), RECVSTR_CHALLENGE, pChallengeInfo->m_challenge );
	
	// send response datagram
	Send_COMMAND_Datagram( sendline, clientnode, PLAYERID_ANONYMOUS );
}

// send a connect response back to a client -----------------------------------
//
int E_PacketHandler::SendConnectResponse( E_ConnManager::ConnResults rc, E_ClientConnectInfo* pClientConnectInfo )
{
	ASSERT( rc >= E_ConnManager::CONN_CHALLENGE_INVALID );
	ASSERT( rc <= E_ConnManager::CONN_OK );
	ASSERT( pClientConnectInfo != NULL );
	
	char sendline[ MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ];
	
	// prepare correct response command
	switch( rc ) {
		case E_ConnManager::CONN_CHALLENGE_INVALID:
			ASSERT( pClientConnectInfo->m_selected_slot == -1 );
			snprintf( sendline, sizeof(sendline), RECVSTR_CHALL_INVALID );
			break;
		case E_ConnManager::CONN_CLIENT_INCOMAPTIBLE:
			ASSERT( pClientConnectInfo->m_selected_slot == -1 );
			snprintf( sendline, sizeof(sendline), RECVSTR_SERVER_INCOMP );
			break;
		case E_ConnManager::CONN_CLIENT_BANNED:
			ASSERT( pClientConnectInfo->m_selected_slot == -1 );
			snprintf( sendline, sizeof(sendline), RECVSTR_CLIENT_BANNED );
			break;
		case E_ConnManager::CONN_SERVER_FULL:
			ASSERT( pClientConnectInfo->m_selected_slot == -1 );
			snprintf( sendline, sizeof(sendline), RECVSTR_SERVER_FULL );
			break;
		case E_ConnManager::CONN_NAME_TAKEN:
			ASSERT( pClientConnectInfo->m_selected_slot == -1 );
			snprintf( sendline, sizeof(sendline), RECVSTR_NAME_INVALID );
			break;
		case E_ConnManager::CONN_OK:
			ASSERT( pClientConnectInfo->m_selected_slot != -1 );
			
			//FIXME: check whether IP/port is necessary in the connect reply packet
			//FIXME: NebulaID and other worldstate should be sent as a sync burst at the beginning
			snprintf( sendline, sizeof(sendline), RECVSTR_ACCEPTED, 
				pClientConnectInfo->m_szHostName, 
				NODE_GetPort( &pClientConnectInfo->m_node ),
				pClientConnectInfo->m_selected_slot, 
				MAX_NUM_CLIENTS,	//FIXME: remove this ????
				SV_SERVERID,
				TheServer->GetServerName()		//FIXME: remove this ????
				);

			break;
		default:
			ASSERT( FALSE );
			return FALSE;
	}
	
	// ensure proper destination id ( PLAYERID_ANONYMOUS on connection refused )
	int nClientID = ( pClientConnectInfo->m_selected_slot == -1 ) ? PLAYERID_ANONYMOUS : pClientConnectInfo->m_selected_slot;

	// send response datagram
	Send_COMMAND_Datagram( sendline, &pClientConnectInfo->m_node, nClientID );

	return TRUE;
}

// send a disconnect request response ------------------------------------------
//
int E_PacketHandler::SendDisconnectResponse( E_ConnManager::DisconnResults rc, node_t* clientnode, int nClientID )
{
	ASSERT( rc >= E_ConnManager::DISC_NOT_CONNECTED );
	ASSERT( rc <= E_ConnManager::DISC_OK );
	ASSERT( ( ( nClientID >= 0 ) && ( nClientID <= MAX_NET_ALLOC_SLOTS ) ) || ( nClientID == PLAYERID_ANONYMOUS ) );
	ASSERT( clientnode != NULL );
	
	char sendline[ MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ];
	
	// prepare correct response command
	switch( rc ) {
		case E_ConnManager::DISC_NOT_CONNECTED:
			snprintf( sendline, sizeof(sendline), RECVSTR_NOT_CONNECTED );
			break;
		case E_ConnManager::DISC_OK:
			snprintf( sendline, sizeof(sendline), RECVSTR_REMOVE_OK );
			break;
		default:
			ASSERT( FALSE );
			return FALSE;
	}

	// flush any pending NACK reliable 
	ThePacketDriver->FlushReliableBuffer( nClientID );
	
	// send response ( UNRELIABLE ) 
	//NOTE: disconnect response is not sent reliable, if packet is lost, client gets a "NOT CONNECTED" reply upon next try
	Send_COMMAND( sendline, clientnode, nClientID, FALSE );
	
	return TRUE;
}

// send a namechange response back to the client ------------------------------
//
int E_PacketHandler::SendNameChangeRepsponse( E_ConnManager::NameChangeResults rc, node_t* clientnode, int nClientID )
{
	ASSERT( rc >= E_ConnManager::NAMECHANGE_NOT_CONNECTED );
	ASSERT( rc <= E_ConnManager::NAMECHANGE_OK );
	ASSERT( ( nClientID >= 0 ) && ( nClientID <= MAX_NET_ALLOC_SLOTS ) );
	ASSERT( clientnode != NULL );
	
	char sendline[ MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ];
	
	// prepare correct response command
	switch( rc ) {
		case E_ConnManager::NAMECHANGE_NOT_CONNECTED:
			snprintf( sendline, sizeof(sendline), RECVSTR_NOT_CONNECTED );
			break;
		case E_ConnManager::NAMECHANGE_ALREADY_TAKEN:
			snprintf( sendline, sizeof(sendline), RECVSTR_NAME_INVALID );
			break;
		case E_ConnManager::NAMECHANGE_OK:
			snprintf( sendline, sizeof(sendline), RECVSTR_NAME_OK );
			break;
		default:
			ASSERT( FALSE );
			return FALSE;
	}
	
	// send response ( RELIABLE )
	Send_COMMAND( sendline, clientnode, nClientID, TRUE );

	return TRUE;
}

// send a INFO response back to the client ------------------------------------
// 
int E_PacketHandler::SendInfoResponse( refframe_t client_ping_send_frame, node_t* clientnode, int nClientID )
{
	ASSERT( ( ( nClientID >= 0 ) && ( nClientID <= MAX_NET_ALLOC_SLOTS ) ) || ( nClientID == PLAYERID_ANONYMOUS ) );
	ASSERT( clientnode != NULL );
	
	E_REList* pUnreliable = E_REList::CreateAndAddRef( RE_LIST_MAXAVAIL );

	// prepare the command string
	char szBuffer[ MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ];
	snprintf( szBuffer, MAX_RE_COMMANDINFO_COMMAND_LEN, 
				RECVSTR_INFO_REPLY,
				SV_SERVERID,
				(unsigned int)client_ping_send_frame,
				TheConnManager->GetNumConnected(),
				MAX_NUM_CLIENTS,
				CLSV_PROTOCOL_MAJOR, CLSV_PROTOCOL_MINOR,
				TheServer->GetServerName()
			);

	// append a remote event containing the command
	int rc = pUnreliable->NET_Append_RE_CommandInfo( szBuffer );
	ASSERT( rc == TRUE );

	// send a datagram
	ThePacketHandler->Send_STREAM_Datagram( pUnreliable, clientnode, nClientID );

	// release the RE list from here
	pUnreliable->Release();

	return TRUE;
}


// send a PING response back to the client ------------------------------------
//
int E_PacketHandler::SendPingResponse( refframe_t client_ping_send_frame, node_t* clientnode, int nClientID )
{
	ASSERT( ( ( nClientID >= 0 ) && ( nClientID <= MAX_NET_ALLOC_SLOTS ) ) || ( nClientID == PLAYERID_ANONYMOUS ) );
	ASSERT( clientnode != NULL );
	
	E_REList* pUnreliable = E_REList::CreateAndAddRef( RE_LIST_MAXAVAIL );

	// prepare the command string
	char szBuffer[ MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ];
	snprintf( szBuffer, MAX_RE_COMMANDINFO_COMMAND_LEN, 
				RECVSTR_PING_REPLY,
				SV_SERVERID,
				(unsigned int)client_ping_send_frame,
				TheConnManager->GetNumConnected(),
				MAX_NUM_CLIENTS
			);

	// append a remote event containing the command
	int rc = pUnreliable->NET_Append_RE_CommandInfo( szBuffer );
	ASSERT( rc == TRUE );

	// send a datagram
	ThePacketHandler->Send_STREAM_Datagram( pUnreliable, clientnode, nClientID );

	// release the RE list from here
	pUnreliable->Release();

	return TRUE;
}

/*
// store gamestate as RE_List head --------------------------------------------
//
RE_GameState* NET_RmEvSingleGameState( NetPacket_GMSV* gamepacket )
{
	ASSERT( gamepacket != NULL );
	RE_GameState* re_gamestate = (RE_GameState*) &gamepacket->RE_List;
	re_gamestate->RE_Type		= RE_GAMESTATE;
	re_gamestate->RE_BlockSize	= sizeof( RE_GameState );

	// fill in the server gametime
	re_gamestate->GameTime = TheServer->GetCurGameTime();
	
	re_gamestate++;
	re_gamestate->RE_Type = RE_EMPTY;

	return re_gamestate;
}
*/

// send a STREAM packet to a client -------------------------------------------
//
size_t E_PacketHandler::Send_STREAM( int nClientID, E_REList* pReliable, E_REList* pUnreliable )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID <= MAX_NUM_CLIENTS ) );	

	char			buffer[ NET_MAX_NETPACKET_INTERNAL_LEN ];
	NetPacket_GMSV*	gamepacket = (NetPacket_GMSV*) buffer;
	
	E_ClientInfo* pClientInfo = TheConnManager->GetClientInfo( nClientID );
	ASSERT( pClientInfo != NULL );
	
	node_t* node = &pClientInfo->m_node;
	
	UPDTXT2( MSGOUT( "PKTP_STREAM S -> C to: %d node: %s", nClientID, NODE_Print( node ) ); );

	// fill gamepacket header
	ThePacketDriver->FillStdGameHeader( PKTP_STREAM, gamepacket );

	// send the packet
	return ThePacketDriver->SendPacket( gamepacket, node, nClientID, pReliable, pUnreliable );
}

// ----------------------------------------------------------------------------
// protected methods
// ----------------------------------------------------------------------------

// check whether the sender node matches the node for the senderid ------------
//
int E_PacketHandler::_IsLegitSender( NetPacket_GMSV* gamepacket, int bufid )
{
	ASSERT( gamepacket != NULL );
	
	// identify sender
	int senderid = gamepacket->SendPlayerId;
	
	if (TheServer->GetServerIsMaster() && senderid == PLAYERID_SERVER) {
		// this is a HB packet...
		return TRUE;
	}

	if(TheServer->GetServerIsMaster() && senderid >= 0) {
		// this is for the master server from a client.
		return TRUE;
	}

	// NODE_ALIVE only valid for normal clients
	if ( ( senderid >= 0 ) && ( senderid < MAX_NUM_CLIENTS ) ) {
		
		// get the clientinfo
		E_ClientInfo* pClientInfo = TheConnManager->GetClientInfo( senderid );
		
		// get the sending node of the packet
		node_t* packetnode = ThePacketDriver->GetPktSender( bufid );
		
		// check whether someone tries to spoof the senderid
		if ( NODE_AreSame( packetnode, &pClientInfo->m_node ) == FALSE ) {

			char szBuffer1[ 128 ];
			char szBuffer2[ 128 ];
			strcpy( szBuffer1, NODE_Print( packetnode )           );
			strcpy( szBuffer2, NODE_Print( &pClientInfo->m_node ) );
			
			MSGOUT( "Filtering spoofed packet for %d. packet from %s - should be %s.\n", 
				senderid, 
				szBuffer1, 
				szBuffer2 );
			
			//FIXME: check whether clients behind firewalls keep the same node ( ports might get remapped )
			return FALSE ;
		} 
	} else if ( senderid == PLAYERID_MASTERSERVER ) {

		// get the sending node of the packet
		node_t* packetnode = ThePacketDriver->GetPktSender( bufid );
		if ( !TheServer->IsMasterServerNode( packetnode ) ) {
			MSGOUT( "Filtering spoofed packet. claims to be from masterserver - is from %s.\n", NODE_Print( packetnode ) );
			return FALSE ;
		}
	}
	
	return TRUE;
}


// send a datagram to a node --------------------------------------------------
//
int E_PacketHandler::Send_STREAM_Datagram( E_REList* pUnreliable, node_t* node, int nClientID )
{
	ASSERT( pUnreliable != NULL );
	ASSERT( node != NULL );
	ASSERT( ( nClientID == PLAYERID_MASTERSERVER ) || ( nClientID == PLAYERID_ANONYMOUS ) || ( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) ) );

	char			buffer[ NET_MAX_NETPACKET_INTERNAL_LEN ];
	NetPacket_GMSV*	gamepacket = (NetPacket_GMSV *) buffer;

	// fill game data header
	ThePacketDriver->FillStdGameHeader( PKTP_STREAM, gamepacket );

	if ( nClientID == PLAYERID_MASTERSERVER ) {

		UPDTXT2( MSGOUT( "DATAGRAM: PKTP_STREAM S -> M: node: %s id: %d msg: %d", 
						NODE_Print( node ), 
						nClientID, 
						gamepacket->MessageId ); );

	} else {

		UPDTXT2( MSGOUT( "DATAGRAM: PKTP_STREAM S -> C: node: %s id: %d msg: %d", 
						NODE_Print( node ), 
						nClientID, 
						gamepacket->MessageId ); );

	}


	// send the datagram
	int rc = ThePacketDriver->SendDatagram( gamepacket, node, nClientID, pUnreliable );
	
	return rc;
}


// send a datagram command to a specific client node --------------------------
//
int E_PacketHandler::Send_COMMAND_Datagram( const char* clientcommand, node_t* node, int nClientID )
{
	char			buffer[ NET_MAX_NETPACKET_INTERNAL_LEN ];
	NetPacket_GMSV*	gamepacket = (NetPacket_GMSV *) buffer;

	// fill game data header
	ThePacketDriver->FillStdGameHeader( PKTP_COMMAND, gamepacket );

	if ( nClientID == PLAYERID_MASTERSERVER ) {

		UPDTXT2( MSGOUT( "DATAGRAM: PKTP_COMMAND S -> M: node: %s msg: %d cmd: '%s'", 
						NODE_Print( node ), 
						nClientID, 
						clientcommand ); );

	} else {

		UPDTXT2( MSGOUT( "DATAGRAM: PKTP_COMMAND S -> C: node: %s msg: %d cmd: '%s'", 
						NODE_Print( node ), 
						nClientID, 
						clientcommand ); );

	}

	// append a remote event containing the command
	E_REList* pUnreliable = E_REList::CreateAndAddRef( RE_LIST_MAXAVAIL );
	if ( !pUnreliable->NET_Append_RE_CommandInfo( clientcommand ) ) {
		DBGTXT( MSGOUT( "e_PacketHandler::Send_COMMAND_Datagram(): RE list choke." ); );
	}
	
	// send the packet
	int rc = ThePacketDriver->SendDatagram( gamepacket, node, nClientID, pUnreliable );

	// release the RE list from here
	pUnreliable->Release();
	
	return rc;
}


// send a server command to a specific client node ----------------------------
//
int E_PacketHandler::Send_COMMAND( char* clientcommand, node_t* node, int nClientID, int reliable )
{
	char			buffer[ NET_MAX_NETPACKET_INTERNAL_LEN ];
	NetPacket_GMSV*	gamepacket = (NetPacket_GMSV *) buffer;

	// fill game data header
	ThePacketDriver->FillStdGameHeader( PKTP_COMMAND, gamepacket );

	if ( nClientID == PLAYERID_MASTERSERVER ) {

		UPDTXT2( MSGOUT( "PKTP_COMMAND S -> M: node: %s id: %d msg: %d cmd: '%s'", 
						NODE_Print( node ), 
						nClientID, 
						gamepacket->MessageId,
						clientcommand ); );

	} else {

		UPDTXT2( MSGOUT( "PKTP_COMMAND S -> C: node: %s id: %d msg: %d cmd: '%s'", 
						NODE_Print( node ), 
						nClientID, 
						gamepacket->MessageId,
						clientcommand ); );

	}

	// append a remote event containing the command
	E_REList* pREList = E_REList::CreateAndAddRef( RE_LIST_MAXAVAIL );
	if ( !pREList->NET_Append_RE_CommandInfo( clientcommand ) ) {
		DBGTXT( MSGOUT( "e_PacketHandler::_Send_COMMAND(): RE list choke." ); );
	}

	// determine whether to send reliable or unreliable
	E_REList* pReliable	= NULL;
	E_REList* pUnreliable	= NULL;
	if ( reliable ) { 
		pReliable = pREList;
	} else {
		pUnreliable = pREList;
	}
	
	// send the packet
	int rc = ThePacketDriver->SendPacket( gamepacket, node, nClientID, pReliable, pUnreliable );

	// release the RE list from here
	pREList->Release();
	
	return rc;
}


// do safe parsing of challenge request ---------------------------------------
//
int E_PacketHandler::_ParseChallengeRequest( char* recvline )
{
	ASSERT( recvline != NULL );

	// copy parseline for safe processing
	char parseline[ MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ];
	strncpy( parseline, recvline, MAX_RE_COMMANDINFO_COMMAND_LEN );
	parseline[ MAX_RE_COMMANDINFO_COMMAND_LEN ] = 0;
	
	// check for request identifier
	char* ident_str = strtok( parseline, " " );
	if ( ident_str == NULL )
		return FALSE;

	int len = strlen( ident_str );
	if ( strncmp( recvline, GIVECHALLSTRING1, len ) != 0 )
		return FALSE;
	
	return TRUE;
}


// do safe parsing of connect request ---------------------------------------
//
int E_PacketHandler::_ParseConnectRequest( char* recvline, E_ClientConnectInfo* pClientConnectInfo )
{
	ASSERT( recvline			!= NULL );
	ASSERT( pClientConnectInfo	!= NULL );

	// copy parseline for safe processing
	SAFE_STR_DUPLICATE( parseline, recvline, MAX_RE_COMMANDINFO_COMMAND_LEN );
	
	// check for request identifier
	char* ident_str = strtok( parseline, " " );
	if ( ident_str == NULL )
		return FALSE;

	int len = strlen( ident_str );
	
	if ( strncmp( ident_str, CONNSTRING2, len ) != 0 ) {
		return FALSE;
	}
	
	// get major version number
	char* v_str1 = strtok( NULL, "." );
	if ( v_str1 == NULL ) {
		return FALSE;
	}
	char* errpart;
	pClientConnectInfo->m_nVersionMajor = (int) strtol( v_str1, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}
	
	// get minor version number
	char* v_str2 = strtok( NULL, " " );
	if ( v_str2 == NULL ) {
		return FALSE;
	}
	pClientConnectInfo->m_nVersionMinor = (int) strtol( v_str2, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}
	
	// get challenge
	strtok( NULL, " " );
	char* ch_str = strtok( NULL, " " );
	if ( ch_str == NULL ) {
		return FALSE;
	}
	pClientConnectInfo->m_challenge = (int) strtol( ch_str, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}
	
	// get playername
	strtok( NULL, " " );
	char* n_str = strtok( NULL, " " );
	if ( n_str == NULL ) {
		return FALSE;
	}
	strncpy( pClientConnectInfo->m_szName, n_str, MAX_PLAYER_NAME );
	pClientConnectInfo->m_szName[ MAX_PLAYER_NAME ] = 0;
	
	// get NET.CLIENTRATE
	strtok( NULL, " " );
	char* clientrate_str = strtok( NULL, " " );
	if ( clientrate_str == NULL ) {
		return FALSE;
	}
	pClientConnectInfo->m_nSendFreq = (int) strtol( clientrate_str, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}

	// get NET.SERVERTRATE
	strtok( NULL, " " );
	char* serverrate_str = strtok( NULL, " " );
	if ( serverrate_str == NULL ) {
		return FALSE;
	}
	pClientConnectInfo->m_nRecvRate = (int) strtol( serverrate_str, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}

	// get operating system specifier
	strtok( NULL, " " );
	char* os_str = strtok( NULL, "\n" );
	if ( os_str == NULL ) {
		return FALSE;
	}
	
	strncpy( pClientConnectInfo->m_szOSLine, os_str, MAX_OSNAME_LEN );
	pClientConnectInfo->m_szOSLine[ MAX_OSNAME_LEN ] = 0;
	
	return TRUE;
}


// do safe parsing of disconnect request --------------------------------------
//
int E_PacketHandler::_ParseDisconnectRequest( char* recvline )
{
	ASSERT( recvline != NULL );
	
	// copy parseline for safe processing
	SAFE_STR_DUPLICATE( parseline, recvline, MAX_RE_COMMANDINFO_COMMAND_LEN );
	
	// check for request identifier
	char* ident_str = strtok( parseline, " " );
	if ( ident_str == NULL )
		return FALSE;

	int len = strlen( ident_str );
	if ( strncmp( recvline, RMVSTRING1, len ) != 0 )
		return FALSE;
	
	return TRUE;
}

// do safe parsing of namechange request --------------------------------------
//
int E_PacketHandler::_ParseNameChangeRequest( char* recvline, char* playername )
{
	ASSERT( recvline != NULL );
	
	// copy parseline for safe processing
	SAFE_STR_DUPLICATE( parseline, recvline, MAX_RE_COMMANDINFO_COMMAND_LEN );
	
	// check for request identifier
	char* ident_str = strtok( parseline, " " );
	if ( ident_str == NULL )
		return FALSE;

	int len = strlen( ident_str );
	if ( strncmp( recvline, NAMESTRING1, len ) != 0 )
		return FALSE;

	//FIXME: we MUST get rid of all NEWLINES in the commands
	//NOTE: the order of the token DOES matter here
	char *n_str = strtok( NULL, "\n\0" );
	if ( n_str == NULL )
		return FALSE;
	
	memset( playername, 0, MAX_PLAYER_NAME + 1 );
	strncpy( playername, n_str, MAX_PLAYER_NAME );
	playername[ MAX_PLAYER_NAME ] = 0;
	
	return TRUE;
}

// do safe parsing of INFO request --------------------------------------------
//
int E_PacketHandler::_ParseInfoRequest( char* recvline, refframe_t* client_ping_send_frame )
{
	ASSERT( recvline != NULL );
	ASSERT( client_ping_send_frame != NULL );
	
	// copy parseline for safe processing
	SAFE_STR_DUPLICATE( parseline, recvline, MAX_RE_COMMANDINFO_COMMAND_LEN );
	
	// check for PING request identifier
	char* ident_str = strtok( parseline, " " );
	if ( ident_str == NULL )
		return FALSE;

	int len = strlen( ident_str );
	if ( strncmp( recvline, INFOSTRING1, len ) != 0 ) {
		return FALSE;
	}

	// get frame
	char* frame_str = strtok( NULL, " \n" );
	if ( frame_str == NULL ) {
		return FALSE;
	}
	
	char* errpart;
	*client_ping_send_frame = (refframe_t) strtol( frame_str, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}

	return TRUE;
}


// do safe parsing of PING request --------------------------------------------
//
int E_PacketHandler::_ParsePingRequest( char* recvline, refframe_t* client_ping_send_frame )
{
	ASSERT( recvline != NULL );
	ASSERT( client_ping_send_frame != NULL );
	
	// copy parseline for safe processing
	SAFE_STR_DUPLICATE( parseline, recvline, MAX_RE_COMMANDINFO_COMMAND_LEN );
	
	// check for PING request identifier
	char* ident_str = strtok( parseline, " " );
	if ( ident_str == NULL )
		return FALSE;

	int len = strlen( ident_str );
	if ( strncmp( recvline, PINGSTRING1, len ) != 0 ) {
		return FALSE;
	}

	// get frame
	char* frame_str = strtok( NULL, " \n" );
	if ( frame_str == NULL ) {
		return FALSE;
	}
	
	char* errpart;
	*client_ping_send_frame = (refframe_t) strtol( frame_str, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}

	return TRUE;
}

int E_PacketHandler::_ParseListRequest_MASTER( char* recvline, int *serverid){

	ASSERT( recvline != NULL );

	// copy parseline for safe processing
	SAFE_STR_DUPLICATE( parseline, recvline, MAX_RE_COMMANDINFO_COMMAND_LEN );
	//MSGOUT("E_PacketHandler::_ParseListRequest_MASTER(): parseline: x%sx", parseline);

	// check for Master Sever List request identifier
	char* ident_str = strtok( parseline, " " );
	if ( ident_str == NULL ){
		//MSGOUT("Returning NNULL on first strtok");
		return FALSE;
	}

	int len = strlen( ident_str );
	if ( strncmp( recvline, MASV_LIST, len ) != 0 ) {
		//MSGOUT("first STRCMP: x%sx x%sx %d", recvline, MASV_LIST, len);
		if (strncmp (recvline, "info", 4) != 0) {
			//MSGOUT("returning on second strcmp");
			return false;
		}
	}

	// check for the server id
	ident_str = strtok( NULL , " " );
	if(ident_str == NULL){
		*serverid = -1;
		return TRUE;
	}
		// this should be the requested server id
		*serverid = atoi(ident_str);
	//}
	if(*serverid == 0){
		*serverid = -1;
	}



	return TRUE;
}

int E_PacketHandler::_ParseHBPacket_MASTER(char* recvline) {

	ASSERT( recvline != NULL );



	// copy parseline for safe processing
	SAFE_STR_DUPLICATE( parseline, recvline, MAX_RE_COMMANDINFO_COMMAND_LEN );

	// check for Master Sever List request identifier
	char* ident_str = strtok( parseline, " " );
	if ( ident_str == NULL )
		return FALSE;

	int len = strlen( ident_str );
	if ( strncmp( recvline, MASV_CHALLSTRING, len ) != 0 ) {
		return FALSE;
	}

	// ok we have a HeartBeat packet, lets get some information...

	// some declares
	int PMajor = 0;
	int PMinor = 0;
	int Challenge = 0;
	int CurrPlayers = 0;
	int MaxPlayers = 0;
	int ServerID = 0;
	char ServerName[ MAX_SERVER_NAME + 1 ];

	node_t _Node;
	char OS[MAX_OSNAME_LEN + 1 ];
	int srv_port = 0;

	ident_str = strtok( NULL, ".");
	if(ident_str == NULL){
		return FALSE;
	} else {
		// this should be the Protocol Major number
		PMajor = atoi(ident_str);
	}

	ident_str = strtok( NULL, " ");
	if(ident_str == NULL){
		return FALSE;
	} else {
		// this should be the Protocol Minor number
		PMinor = atoi(ident_str);
	}

	ident_str = strtok( NULL, " ");// skip a field, as it should just denote "ch" for the challenge.
	// FIXME: Check to make sure this is "ch" for proper packet structure maybe?

	// next....
	ident_str = strtok( NULL, " ");
	if(ident_str == NULL){
		return FALSE;
	} else {
		// this should be the Challenge
		Challenge = atoi(ident_str);
	}

	ident_str = strtok( NULL, " ");// skip a field, as it should just denote "n" for the name.
	// FIXME: Check to make sure this is "n" for proper packet structure maybe?

	// next....
	ident_str = strtok( NULL, " ");
	if(ident_str == NULL){
		return FALSE;
	} else {
		// this should be the Server Name
		strncpy(ServerName, ident_str, MAX_SERVER_NAME -1);
		ServerName[MAX_SERVER_NAME] = '\0';
	}

	ident_str = strtok( NULL, " ");// skip a field, as it should just denote "p" for the player date
	// FIXME: Check to make sure this is "p" for proper packet structure maybe?

	// next....
	ident_str = strtok( NULL, "/");
	if(ident_str == NULL){
		return FALSE;
	} else {
		// this should be the current players
		CurrPlayers = atoi(ident_str);
	}
	ident_str = strtok( NULL, " ");
	if(ident_str == NULL){
		return FALSE;
	} else {
		// this should be the max players
		MaxPlayers = atoi(ident_str);
	}
	ident_str = strtok( NULL, " ");// skip a field, as it should just denote "s" for the server id
	// FIXME: Check to make sure this is "s" for proper packet structure maybe?

	// next....
	ident_str = strtok( NULL, " ");
	if(ident_str == NULL){
		return FALSE;
	} else {
		// this should be the requested server id
		ServerID = atoi(ident_str);
	}

	ident_str = strtok( NULL, " ");// skip a field, as it should just denote "os" for the server os
	// FIXME: Check to make sure this is "os" for proper packet structure maybe?

	// next....
	ident_str = strtok( NULL, " ");
	if(ident_str == NULL){
		return FALSE;
	} else {
		// this should be the server os
		strncpy(OS, ident_str, MAX_OSNAME_LEN - 1 );
		OS[MAX_OSNAME_LEN] = '\0';
	}

	// if we get this far, then we should have everything to try to add the node to the MasterServer->ServerList.
	// BUT first let's see if the node's IP resolves...
	int result = TheUDPDriver->ResolveHostName(ServerName, &_Node);
	if(!result) {
		return FALSE;
	}

	// resolved host name, let's see if they sent us a port
	ident_str = strtok( NULL, " ");// skip a field, as it should just denote "p" for the server port
	// FIXME: Check to make sure this is "p" for proper packet structure maybe?
	if(ident_str == NULL) {
		// null here means they didn't denote a port.  Let's use the default
		// one
		NODE_StorePort(&_Node, DEFAULT_GAMESERVER_UDP_PORT);
	} else {
		// next....
		ident_str = strtok( NULL, " ");
		if(ident_str != NULL){
			// this should be the server port
			srv_port = atoi(ident_str);
		}
		if(srv_port > 0 ) {
			NODE_StorePort(&_Node, srv_port);
		} else {
			// error parsing the server port,
			// use the default one perhaps.
			NODE_StorePort(&_Node, DEFAULT_GAMESERVER_UDP_PORT);
		}
	}


	// check to see if the server already exists in the ServerList.  If so, update it.  If not, add it.
	// if the info doesn't match, ignore the packet
	int i = 0;
	for(i=0; i<TheMaster->ServerList.size(); i++){


		word SrvID = (word)TheMaster->ServerList[i].GetSrvID();
		if(SrvID == ServerID) {
			// double check the ID against the node address.
			node_t node_ck;
			TheMaster->ServerList[i].GetNode(&node_ck);
			if(NODE_AreSame(&node_ck, &_Node)){
				// they are the same, so update the record in the server list
				TheMaster->ServerList[i].update(ServerID,CurrPlayers,MaxPlayers,PMajor,PMinor,ServerName,OS,&_Node);
				return TRUE;
			} else {
				// if we get here, the requested ServerID exists, but the node address
				// doesn't match.  Ignore the packet
				// XXX: Perhaps send a packet back to the server to notify them the ID is taken?
				MSGOUT("Server ID config mismatch for existing server %s\n", ServerName);
				return FALSE;
			}
		}

	}

	//MasterServerItem *tmp = ;
	TheMaster->ServerList.push_back( MasterServerItem (ServerID,
			 CurrPlayers,
			 MaxPlayers,
			 PMajor,
			 PMinor,
			 ServerName,
			 OS,
			 &_Node ));
	MSGOUT("Added new server %s\n", ServerName);
	// if we get this far, we should have successfully parsed the packet.  So return true.
	return TRUE;
}

void E_PacketHandler::_Handle_STREAM_MASTER(NetPacket_GMSV* gamepacket, int bufid) {

	ASSERT( gamepacket != NULL );
	ASSERT( gamepacket->Command == PKTP_STREAM );
	ASSERT( gamepacket->SendPlayerId != PLAYERID_MASTERSERVER );

	// handle an incoming stream for the master server.
	RE_Header* relist = (RE_Header*)&gamepacket->RE_List;
	ASSERT( relist != NULL );

	// do not handle empty remote event lists
	if ( relist->RE_Type == RE_EMPTY ) {
		DBGTXT( MSGOUT( "E_PacketHandler::_Handle_STREAM_MASTER(): ignoring - no input" ); );
		return;
	}

	int nNumEventsAppended = 0;
	RE_CommandInfo * re_commandinfo = NULL;
	node_t*			clientnode		= ThePacketDriver->GetPktSender( bufid );

	char *clientIP;
	
	clientIP=NODE_Print(clientnode);
//	inet_ntop( AF_INET, &clientnode, clientIP, MAX_IPADDR_LEN + 1 );

	// process remote event list
	while ( relist->RE_Type != RE_EMPTY ) {

		// determine size of the remote event
		size_t size = E_REList::RmEvGetSize( relist );
		int serverid = 0;
		// check whether RE is valid
		if ( !E_REList::ValidateRE( relist, size ) ) {
			DBGTXT( MSGOUT( "E_PacketHandler::_Handle_STREAM_MASTER(): ignoring invalid RE %d", relist->RE_Type ); );
			continue;
		}

		switch( relist->RE_Type ) {

			case RE_COMMANDINFO:
				// valid in master server in streams.
				// pass this off to the Master Server Command Packet handler
				// trick the packet handler a little bit... just a little...
				re_commandinfo = (RE_CommandInfo *)relist;

				if ( _ParseListRequest_MASTER( re_commandinfo->command, &serverid ) ) {
					/*
					// walk the MasterServer->ServerList and send out IPV4 packets for all
					std::vector<MasterServerItem>::iterator MSI;
					for(MSI = TheMaster->ServerList.begin(); MSI != TheMaster->ServerList.end(); ++MSI){
						SendIPV4Response(clientnode, (MasterServerItem *)&MSI,  nClientID);
					}*/
					//MSGOUT("E_PacketHandler::_Handle_STREAM_MASTER(): Got LIST from client %s\n", clientIP);
				}
				if ( _ParseHBPacket_MASTER(re_commandinfo->command)) {
					//MSGOUT("E_PacketHandler::_Handle_STREAM_MASTER(): Got HB from client: %s\n", clientIP);
					return;
				} else {
					MSGOUT("E_PacketHandler::_Handle_STREAM_MASTER(): Failed to parse HeartBeat Packet from: %s\n", clientIP);
					return;
				}

				break;
			case RE_SERVERLINKINFO:
				// server link information.

				break;

			default:
				MSGOUT( "E_PacketHandler::_Handle_STREAM_MASTER(): Invalid Packet type %d for Master Server from client %s", relist->RE_Type, clientIP);
				break;

		}

		// advance to next event in list
		ASSERT( ( relist->RE_BlockSize == RE_BLOCKSIZE_INVALID ) || ( relist->RE_BlockSize == size ) );
		relist = (RE_Header *) ( (char *) relist + size );
	}

	// dump the input RE list
	UPDTXT2( m_pInputREList->Dump(); );

}

// do safe parsing of new challenge from MASV ---------------------------------
//
int E_PacketHandler::_ParseMASVChallenge( char* recvline, int* pChallenge )
{
	ASSERT( recvline != NULL );
	ASSERT( pChallenge != NULL );
	
	// copy parseline for safe processing
	char parseline[ MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ];
	strncpy( parseline, recvline, MAX_RE_COMMANDINFO_COMMAND_LEN );
	parseline[ MAX_RE_COMMANDINFO_COMMAND_LEN ] = 0;
	
	// check for string identifier
	char *ident_str = strtok( parseline, " " );
	if ( ident_str == NULL )
		return FALSE;

	int len = strlen( ident_str );
	if ( strncmp( ident_str, RECVSTR_CHALLENGE, len ) != 0 )
		return FALSE;
	
	// get challenge number
	char* chall_str = strtok( NULL, "\0" );
	if ( chall_str == NULL )
		return FALSE;
	
	char *errpart;
	*pChallenge = (int) strtol( chall_str, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}
	

	return TRUE;
}


// handle a packet containing a command from the masterserver -----------------
//
void E_PacketHandler::_Handle_COMMAND_MASV( NetPacket_GMSV* gamepacket, int bufid )
{
	ASSERT( gamepacket != NULL );
	ASSERT( gamepacket->Command == PKTP_COMMAND );
	ASSERT( gamepacket->SendPlayerId == PLAYERID_MASTERSERVER );

	// check whether from masterserver
	if ( gamepacket->SendPlayerId != PLAYERID_MASTERSERVER ) {
		return;
	}

	// get the commandstring
	RE_CommandInfo* re_commandinfo	= (RE_CommandInfo*) &gamepacket->RE_List;

	UPDTXT2( MSGOUT( "PKTP_COMMAND M -> S: id: %d msg: %d cmd: '%s'", nClientID, gamepacket->MessageId, re_commandinfo->command ); );


	// parse for challenge request
	int nMASVChallenge = 0;
	if ( _ParseMASVChallenge( re_commandinfo->command, &nMASVChallenge ) ) {
		
		// set the new challenge
		TheServer->SetMasterServerChallenge( nMASVChallenge );
	}
}


// handle a packet containing a client command --------------------------------
//
void E_PacketHandler::_Handle_COMMAND( NetPacket_GMSV* gamepacket, int bufid )
{
	ASSERT( gamepacket != NULL );
	ASSERT( gamepacket->Command == PKTP_COMMAND );

	// identify sender, get the commandstring & get the sender node
	int				nClientID		= gamepacket->SendPlayerId;
	RE_CommandInfo* re_commandinfo	= (RE_CommandInfo*) &gamepacket->RE_List;
	node_t*			clientnode		= ThePacketDriver->GetPktSender( bufid );

	UPDTXT2( MSGOUT( "PKTP_COMMAND C -> S: id: %d msg: %d cmd: '%s'", nClientID, gamepacket->MessageId, re_commandinfo->command ); );

	// temporary storage for client connection info
	E_ClientConnectInfo _ClientConnectInfo;
	char playername[ MAX_PLAYER_NAME + 1 ];

	refframe_t client_ping_send_frame;
/*
	if ((_ClientConnectInfo->m_nVersionMajor != CLSV_PROTOCOL_MAJOR ) &&
			( _ClientConnectInfo->m_nVersionMinor != CLSV_PROTOCOL_MINOR ) ){

			// output to logfile
			MSGOUT( "client %s with incompatible version %d.%d tried to join\n", _ClientConnectInfo->m_szHostName,
					_ClientConnectInfo->m_nVersionMajor, _ClientConnectInfo->m_nVersionMinor );

			// send the response
			SendConnectResponse( E_ConnManager::CONN_CLIENT_INCOMAPTIBLE, _ClientConnectInfo );

			return;
	}*/

	// parse for challenge request
	if ( _ParseChallengeRequest( re_commandinfo->command ) ) {

		// handle challenge request
		ASSERT( TheConnManager != NULL );
		TheConnManager->RequestChallenge( clientnode );
		
		// parse for connect request
	} else if ( _ParseConnectRequest( re_commandinfo->command, &_ClientConnectInfo ) ) {
		
		// store the sender node
		NODE_Copy( &_ClientConnectInfo.m_node, clientnode );
		
		// retrieve client hostname from address structure
		inet_ntop( AF_INET, &_ClientConnectInfo.m_node, _ClientConnectInfo.m_szHostName, MAX_IPADDR_LEN + 1 );

		// check whether client connection is valid ( version/banned/server full etc. )
		ASSERT( TheConnManager != NULL );
		TheConnManager->CheckClientConnect( &_ClientConnectInfo );
		
		// parse for disconnect request
	} else if ( _ParseDisconnectRequest( re_commandinfo->command ) ) {
		
		// handle disconnect request
		ASSERT( TheConnManager != NULL );
		TheConnManager->CheckClientDisconnect( clientnode );
		
		// parse for namechange request
	} else if ( _ParseNameChangeRequest( re_commandinfo->command, playername ) ) {
		
		// handle namechange request
		ASSERT( TheConnManager != NULL );
		TheConnManager->CheckNameChange( clientnode, playername );

	// parse for a PING request
	} else if ( _ParsePingRequest( re_commandinfo->command, &client_ping_send_frame ) ) {

		// send the PING response
		SendPingResponse( client_ping_send_frame, clientnode, nClientID );

	// parse for a INFO request
	} else if ( _ParseInfoRequest( re_commandinfo->command, &client_ping_send_frame ) ) {

		// send the INFO response
		SendInfoResponse( client_ping_send_frame, clientnode, nClientID );
	}
}

// handle a packet containing a client command --------------------------------
//
void E_PacketHandler::_Handle_COMMAND_MASTER( NetPacket_GMSV* gamepacket, int bufid )
{
	ASSERT( gamepacket != NULL );
	ASSERT( gamepacket->Command == PKTP_COMMAND );

	// identify sender, get the commandstring & get the sender node
	int				nClientID		= gamepacket->SendPlayerId;
	RE_CommandInfo* re_commandinfo	= (RE_CommandInfo*) &gamepacket->RE_List;
	node_t*			clientnode		= ThePacketDriver->GetPktSender( bufid );

	char *clientIP;
	
	clientIP=NODE_Print(clientnode);

	UPDTXT2( MSGOUT( "PKTP_COMMAND C -> S: id: %d msg: %d cmd: '%s'", nClientID, gamepacket->MessageId, re_commandinfo->command ); );

	// temporary storage for client connection info
	E_ClientConnectInfo _ClientConnectInfo;
	char playername[ MAX_PLAYER_NAME + 1 ];

	refframe_t client_ping_send_frame;

	int serverid = 0;
	if ( _ParseListRequest_MASTER( re_commandinfo->command, &serverid ) ) {

		// walk the MasterServer->ServerList and send out IPV4 packets for all or the one

		SendIPV4Response(clientnode,   nClientID, serverid);

		MSGOUT("List Request from client %s\n", clientIP);
		return;
	}
	if ( _ParseHBPacket_MASTER(re_commandinfo->command)) {
		MSGOUT("E_PacketHandler::_Handle_COMMAND_MASTER(): Got HB from client: %s\n", clientIP);
		return;
	} else {
		MSGOUT("E_PacketHandler::_Handle_COMMAND_MASTER(): Failed to parse HeartBeat Packet from: %s\n", clientIP);
		return;
	}
	MSGOUT("E_PacketHandler::_Handle_COMMAND_MASTER(): Unimplemented packet %s from client %s\n", re_commandinfo->command, clientIP);

}

// handle a stream packet -----------------------------------------------------
//
void E_PacketHandler::_Handle_STREAM( NetPacket_GMSV* gamepacket )
{
	ASSERT( gamepacket != NULL );
	ASSERT( gamepacket->Command == PKTP_STREAM );
	ASSERT( gamepacket->SendPlayerId != PLAYERID_MASTERSERVER );
	
	// identify sender & get the sender node
	int	nClientID = gamepacket->SendPlayerId;

	UPDTXT2( MSGOUT( "PKTP_STREAM C -> S: id: %d msg: %d", nClientID, gamepacket->MessageId ); );

	// ignore PKTP_STREAM packets from invalid senders
	if ( ( nClientID < 0 ) || ( nClientID >= MAX_NUM_CLIENTS ) ) {
		MSGOUT( "ignoring PKTP_STREAM packet from invalid sender id: %d", nClientID );
		return;
	}

	E_ClientInfo* pClientInfo = TheConnManager->GetClientInfo( nClientID );
	
	// player must be connected in order to send NODE_ALIVE
	if ( pClientInfo->IsSlotFree() ) {
		DBGTXT( MSGOUT( "_Handle_STREAM(): filtering GAME_STATE for player who is not connected: %d.", nClientID ); );
		return;
	}
	
	// get the player info
	E_SimPlayerInfo* pSimPlayerInfo = TheSimulator->GetSimPlayerInfo( nClientID );
	ASSERT( pSimPlayerInfo != NULL );
	ASSERT( pSimPlayerInfo->IsPlayerDisconnected() == FALSE );
	
	// mark the sender alive
	pClientInfo->MarkAlive();

	// handle the network input
	TheSimNetInput->HandleOneClient( nClientID, (RE_Header*)&gamepacket->RE_List );
}



int E_PacketHandler::SendIPV4Response(node_t* clientnode, int nClientID, int serverid) {


	ASSERT( ( ( nClientID >= 0 ) && ( nClientID <= MAX_NET_ALLOC_SLOTS ) ) || ( nClientID == PLAYERID_ANONYMOUS ) );
	ASSERT( clientnode != NULL );
	int i = 0;
	for(i=0; i<TheMaster->ServerList.size(); i++){

		if(serverid == -1 || serverid == TheMaster->ServerList[i].GetSrvID()){

			E_REList* pUnreliable = E_REList::CreateAndAddRef( RE_LIST_MAXAVAIL );
			// append a remote event containing the command


			node_t node;
			TheMaster->ServerList[i].GetNode( &node);
			word SrvID = (word)TheMaster->ServerList[i].GetSrvID();


			int rc = pUnreliable->NET_Append_RE_IPv4ServerInfo( &node, SrvID, SrvID, SrvID, 0 );
			ASSERT( rc == TRUE );

			// send a datagram
			ThePacketHandler->Send_STREAM_Datagram( pUnreliable, clientnode, nClientID );

			// release the RE list from here
			pUnreliable->Release();
		}
	}
	return TRUE;
}
