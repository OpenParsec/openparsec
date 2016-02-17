/*
 * PARSEC - Packet driver
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:48 $
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

// UNP header
#include "net_wrap.h"

// mathematics header
#include "utl_math.h"

// server defs
#include "e_defs.h"

// network code config
#include "net_conf.h"
#include "net_csdf.h"

// local module header
#include "net_packetdriver.h"

// proprietary module headers
#include "con_aux_sv.h"
#include "net_game_sv.h"
#include "net_udpdriver.h"
#include "net_stream.h"
#include "e_gameserver.h"
#include "e_packethandler.h"
#include "e_simulator.h"
#include "sys_util_sv.h"
#include "sys_refframe_sv.h"


// flags ----------------------------------------------------------------------
//

// number of listen buffers in listen buffers array ---------------------------
//
#define NUM_LISTEN_BUFFERS		( MAX_NET_ALLOC_SLOTS * 2 )

// history buffer for received message ids to filter out duplicates -----------
//
#define MSGID_HISTORY_SIZE 64

// packet loss meter ----------------------------------------------------------
//
//FIXME: we need a packet loss meter for every client
#define PACKET_LOSS_METER_LENGTH	100						// horizontal size


// string constants -----------------------------------------------------------
//
static const char pchain_alloc_error[]= "Error allocating packet chain.\n";

// ----------------------------------------------------------------------------
// public methods
// ----------------------------------------------------------------------------

// default ctor ---------------------------------------------------------------
//
NET_PacketDriver::NET_PacketDriver()
{
	// init all to NULL for Reset to not delete the data
	ReceivedPacketsChain  = NULL;
	SendNetPacketExternal = NULL;
	RecvNetPacketExternal = NULL;
	m_Streams     = NULL;

	Reset();
}


// reset all data members -----------------------------------------------------
//
void NET_PacketDriver::Reset()
{
	// free when necessary 
	if ( SendNetPacketExternal != NULL ) FREEMEM( SendNetPacketExternal );
	if ( RecvNetPacketExternal != NULL ) FREEMEM( RecvNetPacketExternal );
	if ( ReceivedPacketsChain != NULL ) _FreePacketsChain();
	int bid;
    for ( bid = 0; bid < NUM_LISTEN_BUFFERS; bid++ ) {
		if ( NetPacketsInternal[ bid ] != NULL ) FREEMEM( NetPacketsInternal[ bid ] );
	}

	// allocate the external packets
	SendNetPacketExternal = (NetPacketExternal_GMSV *) ALLOCMEM( NET_UDP_DATA_LENGTH );
	RecvNetPacketExternal = (NetPacketExternal_GMSV *) ALLOCMEM( NET_UDP_DATA_LENGTH );
	memset( SendNetPacketExternal, 0, NET_UDP_DATA_LENGTH );
	memset( RecvNetPacketExternal, 0, NET_UDP_DATA_LENGTH );

	// allocate the internal packets
	for ( bid = 0; bid < NUM_LISTEN_BUFFERS; bid++ ) {
		NetPacketsInternal[ bid ] = (NetPacket_GMSV *) ALLOCMEM( NET_MAX_NETPACKET_INTERNAL_LEN );
		memset( NetPacketsInternal[ bid ],	0, NET_MAX_NETPACKET_INTERNAL_LEN );
		memset( &ListenAddress[ bid ], 0, sizeof( sockaddr_in ) );
	}

	// init the streams
	if ( m_Streams != NULL ) {
		delete []m_Streams;
		m_Streams = NULL;
	}
	m_Streams = new NET_Stream[ MAX_NUM_CLIENTS ];

	// set the client IDs the stream is connected to
	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
		m_Streams[ nClientID ].SetPeerID( nClientID );
		m_Streams[ nClientID ].SetSenderID( PLAYERID_SERVER );
	}
	
	// allocate the packet chain
	_AllocPacketsChain();
}


// get the stream message status ----------------------------------------------
//
NET_Stream* NET_PacketDriver::GetStream( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	return &m_Streams[ nClientID ];
}

// reset the stream of a specific client --------------------------------------
//
void NET_PacketDriver::ResetStream( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
#ifdef PARSEC_DEBUG
	MSGOUT( "resetting stream for client %d", nClientID );
#endif // PARSEC_DEBUG
	m_Streams[ nClientID ].Reset();
	m_Streams[ nClientID ].SetPeerID( nClientID );
	m_Streams[ nClientID ].SetSenderID( PLAYERID_SERVER );
}


// connect the stream of a client ---------------------------------------------
//
void NET_PacketDriver::ConnectStream( int nClientID )
{
	ResetStream( nClientID );
	m_Streams[ nClientID ].SetConnected();
}


// flush the reliable backbuffer of a stream ----------------------------------
//
void NET_PacketDriver::FlushReliableBuffer( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	m_Streams[ nClientID ].FlushReliableBuffer();
}

// default dtor ---------------------------------------------------------------
//	
NET_PacketDriver::~NET_PacketDriver()
{
	// free the packet chain
	_FreePacketsChain();
	
	// free internal/external packets
	FREEMEM( SendNetPacketExternal );
	FREEMEM( RecvNetPacketExternal );
	SendNetPacketExternal = NULL;
	RecvNetPacketExternal = NULL;

	for ( int bid = 0; bid < NUM_LISTEN_BUFFERS; bid++ ) {
		FREEMEM( NetPacketsInternal[ bid ] );
		NetPacketsInternal[ bid ] = NULL;
	}

	if ( m_Streams != NULL ) {
		delete []m_Streams;
		m_Streams = NULL;
	}
}

// invoke processing function for all packets currently in chain --------------
//
void NET_PacketDriver::NET_ProcessPacketChain()
{
	// build chain of received packets
	_BuildReceivedPacketsChain();
	
	// invoke processing function for received packets
	PacketChainBlock *block;
	while ( ( block = _FetchPacketFromChain() ) != NULL ) {
		
		// invoke packet processing function
		ThePacketHandler->HandlePacket( block );
		
		// release the packet from the packetchain
		_ReleasePacketFromChain( block );
	}
}


// retrieve sender's node address from listen header --------------------------
//
node_t* NET_PacketDriver::GetPktSender( int bufid )
{
	ASSERT( bufid >= 0 );
	ASSERT( bufid < NUM_LISTEN_BUFFERS );
	
	node_t *node;
	
	//NOTE:
	// instead of returning the address directly from ListenAdress[]
	// we make a copy here, since the port number will be appended and
	// we don't want to assume there is enough memory in the structure
	// after the sin_addr field. (usually there are eight irrelevant bytes)
	
	node = &ListenSenderStorage[ bufid ];
	
	memcpy( node, &ListenAddress[ bufid ].sin_addr, IP_ADR_LENGTH );
	NODE_StorePort( node, ntohs( ListenAddress[ bufid ].sin_port ) );
	
	return node;
}

// return the message id used in the next packet that is sent -----------------
//
int NET_PacketDriver::GetNextOutMessageId( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	return m_Streams[ nClientID ].GetNextOutMessageId();
}


// fill standard fields in gamepacket header ----------------------------------
//
void NET_PacketDriver::FillStdGameHeader( byte command, NetPacket_GMSV* gamepacket )
{
	ASSERT( gamepacket != NULL );
	
	// clear header and remote event list area
	memset( gamepacket, 0, NET_MAX_NETPACKET_INTERNAL_LEN );

	gamepacket->Command				= command;
	gamepacket->SendPlayerId		= PLAYERID_SERVER;

	// RE list size always includes the RE terminator
	((RE_Header* )&gamepacket->RE_List)->RE_Type = RE_EMPTY;
	gamepacket->RE_ListSize	= sizeof( dword );
}


// determine if packet should be artificially dropped -------------------------
//
int NET_PacketDriver::_CheckForcePacketDrop()
{
	
#ifdef ENABLE_PACKETDROP_TESTING
	
	//NOTE:
	// SVAUX_PACKETDROP_TESTING sets the probability
	// of packet-drop (well, approximately).
	
	if ( SVAUX_PACKETDROP_TESTING ) {
		
		int r = RAND() % 100;
		if ( r < SVAUX_PACKETDROP_TESTING ) {
			return TRUE;
		}
	}
	
#endif // ENABLE_PACKETDROP_TESTING
	
	return FALSE;
}


// send a datagram ( unreliable, unnumbered packet ) to a node ----------------
//
int NET_PacketDriver::SendDatagram( NetPacket_GMSV* gamepacket, node_t* node, int nClientID, E_REList* pUnreliable, byte clientProtoMajor, byte clientProtoMinor )
{
	ASSERT( gamepacket  != NULL );
	ASSERT( node	    != NULL );
	ASSERT( pUnreliable != NULL );

	RE_Header*	pFillPosition	= (RE_Header*)(&gamepacket->RE_List);
	size_t		maxfillsize		= RE_LIST_MAXAVAIL + sizeof( dword );

	// copy unreliable RE ( account maxsize for dword in NetPacket_GMSV )
	size_t unreliable_size = pUnreliable->WriteTo( pFillPosition, maxfillsize, TRUE );
	
	// unreliable overflow ?
	if ( unreliable_size == 0 ) {
		DBGTXT( MSGOUT( "NET_PacketDriver::SendDatagram(): WARNING: unreliable overflow" ); );
		return FALSE;
	}

	// fill message header/#s for datagram
	gamepacket->MessageId				= MSGID_DATAGRAM;
	gamepacket->ReliableMessageId		= 0;
	gamepacket->AckMessageId			= 0;
	gamepacket->AckReliableMessageId	= 0;

	// pack from internal packet to external packet
	size_t pktsize = NETs_HandleOutPacket( (NetPacket*)gamepacket, SendNetPacketExternal, clientProtoMajor,  clientProtoMinor );

	sockaddr_in SendAddress;
	bzero( &SendAddress, sizeof( SendAddress ) );
	SendAddress.sin_family = AF_INET;
	SendAddress.sin_port = htons( NODE_GetPort( node ) );
	memcpy ( &SendAddress.sin_addr, node, IP_ADR_LENGTH );

	TheUDPDriver->SendPacket( (char *) SendNetPacketExternal, pktsize, (SA*) &SendAddress );
	
	return pktsize;
}


// send a packet to a node ----------------------------------------------------
//
int NET_PacketDriver::SendPacket( NetPacket_GMSV* gamepacket, node_t* node, int nClientID, E_REList* pReliable, E_REList* pUnreliable )
{
	ASSERT( gamepacket != NULL );
	ASSERT( node	   != NULL );
	ASSERT( nClientID  != PLAYERID_MASTERSERVER );
	ASSERT( nClientID  != PLAYERID_ANONYMOUS );
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	
	NET_Stream* pStream = &m_Streams[ nClientID ];
	ASSERT( pStream != NULL );
	
	// check for artificial packet drop
	if ( _CheckForcePacketDrop() ) {
		pStream->LogPacketLossStats( 1, FALSE, FALSE );
		return -1;
	}

	//FIXME_MASV: eventually move all E_REList dependent code to util function

	// append reliable data to queue in stream and check for reliable choke
	if ( ( pReliable != NULL ) && ( pReliable->GetSize() > 0 ) ) {
		if ( pStream->AppendToReliableFIFO( pReliable ) == FALSE ) {
			ASSERT( nClientID != PLAYERID_MASTERSERVER );

			//FIXME: what do we do with the RELIABLE RE_COMMANDINFO containing the DISCONNECT

			// disconnect the client if we have a reliable choke
                     MSGOUT("Choking");
			TheConnManager->DisconnectClient( nClientID );
			return -1;
		}
	}

	// retrieve the first reliable RE list
	pReliable = pStream->GetNextReliableToSend();

	RE_Header*	pFillPosition	= (RE_Header*)(&gamepacket->RE_List);
	size_t		maxfillsize		= RE_LIST_MAXAVAIL + sizeof( dword );
	size_t		reliable_size   = 0;
	size_t		unreliable_size = 0;

	if ( ( pReliable != NULL ) && ( pReliable->GetSize() > 0 ) ) {
		ASSERT( nClientID != PLAYERID_MASTERSERVER );

		// copy reliable RE and check for reliable overflow
		reliable_size = pReliable->WriteTo( pFillPosition, maxfillsize, FALSE );
		if ( reliable_size == 0 ) {
			MSGOUT( "NET_PacketDriver::SendPacket(): ERROR: reliable overflow" );
			return -1;
		}
		// determine next fill position
		pFillPosition = (RE_Header*) ( ((char*)(&gamepacket->RE_List)) + reliable_size  );
		maxfillsize  -= ( reliable_size + 1 );
	}

	// handle unreliable part
	if ( ( pUnreliable != NULL ) && ( pUnreliable->GetSize() > 0 ) ) {

		// copy unreliable RE ( account maxsize for dword in NetPacket_GMSV and size of already inserted reliable )
		unreliable_size = pUnreliable->WriteTo( pFillPosition, maxfillsize, TRUE );
		
		// unreliable overflow ?
		if ( unreliable_size == 0 ) {
			DBGTXT( MSGOUT( "NET_PacketDriver::SendPacket(): WARNING: unreliable overflow" ); );
		}
	}

	// we do not send empty ( header only ) packets
	if ( ( reliable_size + unreliable_size ) == 0 ) {
		DBGTXT( MSGOUT( "NET_PacketDriver::SendPacket(): WARNING: skip sending of empty packet ( header only ) - possibly due to reliable buffering." ); );
		return 0;
	}

	// fill in message-IDs and ACKs
	pStream->OutPacket( gamepacket, ( pReliable != NULL ) );

	// pack from internal packet to external packet
	size_t pktsize = NETs_HandleOutPacket( (NetPacket*)gamepacket, SendNetPacketExternal );

	// write local packet to stream - for testing purposes only
#ifdef SAVE_PACKETSTREAM
	SaveLocalPacket( SendNetPacketExternal );
#endif // SAVE_PACKETSTREAM
	
	sockaddr_in SendAddress;
	bzero( &SendAddress, sizeof( SendAddress ) );
	SendAddress.sin_family = AF_INET;
	SendAddress.sin_port = htons( NODE_GetPort( node ) );
	memcpy ( &SendAddress.sin_addr, node, IP_ADR_LENGTH );

	int rc = TheUDPDriver->SendPacket( (char *) SendNetPacketExternal, pktsize, (SA*) &SendAddress );
	
	// log packet loss
	pStream->LogPacketLossStats( 1, ( rc >= 0 ), FALSE );

	return pktsize;
}


// ----------------------------------------------------------------------------
// protected methods
// ----------------------------------------------------------------------------

// retrieve next packet from received packets chain ---------------------------
//
PacketChainBlock* NET_PacketDriver::_FetchPacketFromChain()
{
	ASSERT( ReceivedPacketsChain != NULL );
	
	PacketChainBlock *temp = ReceivedPacketsChain->nextblock;
	if ( temp != NULL ) {
		ASSERT( ReceivedPacketsChain->chainlength > 0 );
		ReceivedPacketsChain->nextblock = temp->nextblock;
		temp->nextblock = NULL;
		ReceivedPacketsChain->chainlength--;
	}
	
	return temp;
}


// allocate received packets chain (head node) --------------------------------
//
int NET_PacketDriver::_AllocPacketsChain()
{
	ASSERT( ReceivedPacketsChain == NULL );
	ReceivedPacketsChain = (PacketChainHead *) ALLOCMEM( sizeof( PacketChainHead ) );
	if ( ReceivedPacketsChain == NULL ) {
		fprintf( stderr, pchain_alloc_error );
		return FALSE;
	}
	ReceivedPacketsChain->nextblock   = NULL;
	ReceivedPacketsChain->chainlength = 0;
	
	return TRUE;
}


// free all blocks in received packets chain ----------------------------------
//
void NET_PacketDriver::_FreePacketsChain()
{
	ASSERT( ReceivedPacketsChain != NULL );
	while ( ReceivedPacketsChain != NULL ) {
		PacketChainBlock *temp = ReceivedPacketsChain->nextblock;
		FREEMEM( ReceivedPacketsChain );
		ReceivedPacketsChain = (PacketChainHead *) temp;
	}
}


// release block containing pointer and info of packet ------------------------
//
void NET_PacketDriver::_ReleasePacketFromChain( PacketChainBlock *block )
{
	CHECKHEAPBASEREF( block );
	FREEMEM( block );
}


// pre-process received packets -----------------------------------------------
//
void NET_PacketDriver::_BuildReceivedPacketsChain()
{
	//NOTE:
	// this function checks all packets received since it was last called. 
	// packets are decrypted, CRC checked and converted from external to internal
	// packet format in _UDP_FetchPacket.
	// the packets have to be sequenced according to their id, as they may be
	// in any order, and then inserted into the chain (this is done by ChainPacket() ).
	
	ASSERT( ReceivedPacketsChain != NULL );
	
#ifdef ASSERT_PACKET_CHAIN_EMPTY
	ASSERT( ReceivedPacketsChain->nextblock == NULL );
	ASSERT( ReceivedPacketsChain->chainlength == 0 );
#endif

	// scan listen buffers
	for ( int bufid = 0; bufid < NUM_LISTEN_BUFFERS; bufid++ ) {

		// try to retrieve packet
		if ( !_UDP_FetchPacket( bufid ) ) 
			break;

		// process received packet
		if ( !ListenStatus[ bufid ] ) {

			// insert packet into chain
			_ChainPacket( NetPacketsInternal[ bufid ], bufid );
		}
	}
}


// insert received packet into chain ------------------------------------------
//
int NET_PacketDriver::_ChainPacket( NetPacket_GMSV* gamepacket, int bufid )
{
	ASSERT( gamepacket			 != NULL );
	ASSERT( ReceivedPacketsChain != NULL );

	PacketChainBlock*	scan	= ReceivedPacketsChain;
	int nClientID				= gamepacket->SendPlayerId;
	int messageid				= gamepacket->MessageId;

	// exclude non-regular packets (negative sender ids) from
	// duplicate checking: PLAYERID_SERVER, PLAYERID_ANONYMOUS
	switch( nClientID ) {
		case PLAYERID_SERVER:

			if(!TheServer->GetServerIsMaster()){
				DBGTXT( MSGOUT( "NET_PacketDriver::_ChainPacket(): filtering packet %d from %d", messageid, nClientID ); );
				return FALSE;
			}
			break;
		case PLAYERID_ANONYMOUS:
			break;

		case PLAYERID_MASTERSERVER:
			{	
				//FIXME: _IsLegitSender() is also called in E_PacketHandler::HandlePacket(), duplicate work ?

				// check src node with master server node
				node_t* node = GetPktSender( bufid );
				if ( !TheServer->IsMasterServerNode( node ) ) {
					//DBGTXT(
							MSGOUT( "NET_PacketDriver::_ChainPacket(): MasterServer NODE MISMATCH %s", NODE_Print( node ) );// );
					return FALSE;
				}
			}
			break;

		default:
			if( ( nClientID < 0 ) || ( nClientID >= MAX_NUM_CLIENTS ) ) {
				DBGTXT( MSGOUT( "NET_PacketDriver::_ChainPacket(): filtering packet %d from %d", messageid, nClientID ); );
				return FALSE;
			} else {

				if(!TheServer->GetServerIsMaster()){
					// check src node with stored node for already connected client
					node_t* clientnode	= GetPktSender( bufid );
					if ( !TheConnManager->CheckNodesMatch( nClientID, clientnode ) ) {
						DBGTXT( MSGOUT( "NET_PacketDriver::_ChainPacket(): NODE MISMATCH %d: %s", nClientID, NODE_Print( clientnode ) ); );
						return FALSE;
					}

					// check regular player packets for duplicates
					if ( _FilterPacketDuplicate( nClientID, messageid ) ) {
						DBGTXT( MSGOUT( "NET_PacketDriver::_ChainPacket(): filtering duplicate packet %d from %d", messageid, nClientID ); );
						return FALSE;
					}
				}
			}
			break;
	}

	// no duplicate checking/stream handling for datagrams
	if ( (dword)messageid != MSGID_DATAGRAM ) {

		// normal stream packets must come from identified clients
		ASSERT( nClientID != PLAYERID_ANONYMOUS );

		// search for packet of same sender already in chain
		for ( ; scan->nextblock; scan = scan->nextblock ) {
			if ( NODE_Compare( GetPktSender( scan->nextblock->bufferno ), GetPktSender( bufid ) ) == NODECMP_EQUAL ) {
				break;
			}
		}
		
		// search to right insert position (according to message id)
		for ( ; scan->nextblock; scan = scan->nextblock ) {
			if ( NODE_Compare( GetPktSender( scan->nextblock->bufferno ), GetPktSender( bufid ) ) != NODECMP_EQUAL ) {
				break;
			}
			if ( messageid <= scan->nextblock->messageid  ) {
				if ( messageid == scan->nextblock->messageid  )
					DBGTXT( MSGOUT( "NET_PacketDriver::_ChainPacket(): received same message twice: %d.", messageid ); );
				break;
			}
		}

		// maintain stream IDs/ACKs
		NET_Stream* pStream = GetStream( nClientID );
		if ( !pStream->InPacket( gamepacket ) ) {
			MSGOUT("Return in maintain stream IDs/ACKs");
			return FALSE;
		}
	}

	// record packet
	if ( bufid != VIRTUAL_BUFFER_ID ) {
		//MSGOUT("Recording Packet");
		_RecordPacket( bufid );
	}

	// alloc block in chain
	PacketChainBlock *temp = (PacketChainBlock *) ALLOCMEM( sizeof( PacketChainBlock ) );
	if ( temp == NULL ) {
		ASSERT( 0 );
		return FALSE;
	}

	// store critical info
	temp->bufferno	= bufid;
	temp->gamepacket= gamepacket;
	temp->messageid = messageid;

	// link packet into chain
	temp->nextblock = scan->nextblock;
	scan->nextblock = temp;
	ReceivedPacketsChain->chainlength++;

	return TRUE;
}


// record packet before inserting it into packet chain ------------------------
//
void NET_PacketDriver::_RecordPacket( int bufid )
{
/*#ifdef ENABLE_PACKET_RECORDING
	
	ASSERT( bufid != VIRTUAL_BUFFER_ID );

	// check whether we are currently recording packets
	if ( REC_IsRecordingPackets() ) {

		// translate NetPacketsInternal to corresponding DEMO NetPacketExternal
		NetPacketExternal* ext_gamepacket = (NetPacketExternal*)ALLOCMEM( RECORD_PACKET_SIZE );
		NETs_HandleOutPacket_DEMO( NetPacketsInternal[ bufid ], ext_gamepacket );


		//NOTE:
		// packets are recorded in network byte-order, that is, unswapped.
		// This is exactly the format that is held in a NetPackeExternal 
		// ( along with encryption, compression etc ).

		// record the external DEMO packet
		REC_RecordRemotePacket( ext_gamepacket );
	}

#endif // ENABLE_PACKET_RECORDING
*/
}


// check regular player packets for duplicates --------------------------------
//
int NET_PacketDriver::_FilterPacketDuplicate( int senderid, int messageid )
{
	ASSERT( ( senderid >= 0 ) && ( senderid < MAX_NUM_CLIENTS ) );
	return m_Streams[ senderid ].FilterPacketDuplicate( messageid );
}


// retrieve received udp packet and store into listen buffer ------------------
//
int NET_PacketDriver::_UDP_FetchPacket( int bufid )
{
	// assume buffer is busy (nothing received)
	ListenStatus[ bufid ] = TRUE;

	// fetch packet
	sockaddr_in from_adress;
	//ListenDatLen[ bufid ] = 
	int rc  = TheUDPDriver->FetchPacket( (char *)RecvNetPacketExternal, NET_UDP_DATA_LENGTH, (SA *)&from_adress );

	if ( rc >= 0 ) {

		// copy from adress
		memcpy( &ListenAddress[ bufid ], &from_adress, sizeof( sockaddr_in ) );

		// filter broadcast packets that are looping back
		if ( NODE_Compare( TheUDPDriver->GetNode(), GetPktSender( bufid ) ) == NODECMP_EQUAL ) {
			//DBGTXT(
					MSGOUT( "NET_PacketDriver::_UDP_FetchPacket(): dropped packet [loop-back]." ); //);
			// indicate we want to try to receive more packets
			return TRUE;
		} 

		// convert external packet to internal format
		if ( !NETs_HandleInPacket( RecvNetPacketExternal, (size_t )rc, (NetPacket*) NetPacketsInternal[ bufid ] ) ) {
			// if we got kicked back because of a protocol mismatch, send the client an incompatible message with THEIR protocol version
			// numbers so the packet isn't dropped on their end.
			if((RecvNetPacketExternal->MajorVersion != CLSV_PROTOCOL_MAJOR) ||
					(RecvNetPacketExternal->MinorVersion != CLSV_PROTOCOL_MINOR)) {
				// first check to see if the client just wants a challenge, we can kick them after they get what they want.
				_CheckIncompatibleChallenge(GetPktSender(bufid));
				_SendClientIncompatible(GetPktSender(bufid), PLAYERID_SERVER, RecvNetPacketExternal->MajorVersion, RecvNetPacketExternal->MinorVersion);
			}
			DBGTXT( MSGOUT( "NET_PacketDriver::_UDP_FetchPacket(): dropped packet [conversion error]." ); );
			return FALSE;
		} else {

			UPDTXT2(MSGOUT( "NET_PacketDriver::_UDP_FetchPacket(): received packet msg: %d.", RecvNetPacketExternal->MessageId ));

			// indicate that buffer now contains received packet
			ListenStatus[ bufid ] = FALSE;
			return TRUE;
		}

	} else {

		//FIXME: could check for ERRNO_ECONNRESET (ICMP "Port Unreachable"), to identify dead clients
		return FALSE;
	}
}

int NET_PacketDriver::_SendClientIncompatible(node_t* node, int nClientID, byte clientProtoMajor, byte clientProtoMinor){
	char clientcommand[MAX_RE_COMMANDINFO_COMMAND_LEN + 1];

	snprintf( clientcommand, sizeof(clientcommand), RECVSTR_SERVER_INCOMP );

	MSGOUT("NET_PacketDriver::_SendClientIncompatible(): Incompatible Protocol Notification going to %s", NODE_Print(node));

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
	int rc = SendDatagram( gamepacket, node, nClientID, pUnreliable, clientProtoMajor, clientProtoMinor );

	// release the RE list from here
	pUnreliable->Release();

	return rc;
}

int NET_PacketDriver::_CheckIncompatibleChallenge(node_t *node){

	// This is a pretty bad hack....

	// get a pointer to the RE_List and cast it as a command.
	RE_CommandInfo *pckt_cmd = (RE_CommandInfo* )&RecvNetPacketExternal->RE_List;

	if(!strncmp((const char*)&pckt_cmd->command,"givechall", 9)) {
		// spoof a challenge to the client so we can kick them
		E_ClientChallengeInfo* pCurChallengeInfo = new E_ClientChallengeInfo();

		// generate a new unique challenge
		pCurChallengeInfo->m_challenge = RAND();
		pCurChallengeInfo->m_frame_generated = SYSs_GetRefFrameCount();
		NODE_Copy( &pCurChallengeInfo->m_node, node );


		// send the response back to the client
		// prepare response
		char sendline[ MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ];
		snprintf( sendline, sizeof(sendline), RECVSTR_CHALLENGE, pCurChallengeInfo->m_challenge );

		// send response datagram

		char			buffer[ NET_MAX_NETPACKET_INTERNAL_LEN ];
		NetPacket_GMSV*	gamepacket = (NetPacket_GMSV *) buffer;

		// fill game data header
		ThePacketDriver->FillStdGameHeader( PKTP_COMMAND, gamepacket );


		// append a remote event containing the command
		E_REList* pUnreliable = E_REList::CreateAndAddRef( RE_LIST_MAXAVAIL );
		if ( !pUnreliable->NET_Append_RE_CommandInfo( sendline ) ) {
			DBGTXT( MSGOUT( "NET_PacketDriver::_CheckIncompatibleChallenge: RE list choke." ); );
		}

		// send the packet
		SendDatagram( gamepacket, node, PLAYERID_SERVER, pUnreliable, RecvNetPacketExternal->MajorVersion, RecvNetPacketExternal->MinorVersion );

		// release the RE list from here
		pUnreliable->Release();

	}



}
