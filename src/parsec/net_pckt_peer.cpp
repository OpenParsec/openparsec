/*
 * PARSEC - Packet Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:39 $
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
//#include "sys_defs.h"

// mathematics header
#include "utl_math.h"

// network code config
#include "net_conf.h"

// subsystem linkage info
#include "linkinfo.h"

// local module header
#include "net_pckt_peer.h"

// proprietary module headers
#include "con_aux.h"
#include "net_game.h"
#include "net_pckt.h"
#include "net_rmev.h"
#include "net_swap.h"
#include "net_util.h"
#include "obj_creg.h"


#ifdef DBIND_PROTOCOL

	#undef  NETs_HandleOutPacket
	#undef  NETs_HandleOutPacket_DEMO
	#undef  NETs_HandleInPacket
	#undef  NETs_HandleInPacket_DEMO
	#undef	NETs_NetPacketExternal_DEMO_GetSize
	#undef  NETs_StdGameHeader
	#undef	NETs_WritePacketInfo

	#define NETs_HandleOutPacket				NETs_PEERTOPEER_HandleOutPacket
	#define NETs_HandleOutPacket_DEMO			NETs_PEERTOPEER_HandleOutPacket_DEMO
	#define NETs_HandleInPacket					NETs_PEERTOPEER_HandleInPacket
	#define NETs_HandleInPacket_DEMO			NETs_PEERTOPEER_HandleInPacket_DEMO
	#define NETs_NetPacketExternal_DEMO_GetSize NETs_PEERTOPEER_NetPacketExternal_DEMO_GetSize
	#define NETs_StdGameHeader					NETs_PEERTOPEER_StdGameHeader
	#define NETs_WritePacketInfo				NETs_PEERTOPEER_WritePacketInfo

#endif


// flags ----------------------------------------------------------------------
//
#define _SET_RE_LISTSIZE_ON_PACK

// protocol type signature ----------------------------------------------------
//
const char net_packet_signature_peer2peer[]     = "PP2";

// fill standard fields in gamepacket header  ( peer-to-peer ) --------------
//
void NETs_StdGameHeader( byte command, NetPacket* gamepacket )
{
	ASSERT( gamepacket != NULL );
	
	NetPacket_PEER* gamepacket_PEER = (NetPacket_PEER*) gamepacket;
	
	// clear header and remote event list area
	memset( gamepacket_PEER, 0, NET_MAX_DATA_LENGTH );
	
	gamepacket_PEER->Universe 		= MyUniverse;
	gamepacket_PEER->NumPlayers		= NumRemPlayers;
	
	gamepacket_PEER->Command		= command;
	gamepacket_PEER->SendPlayerId	= LocalPlayerId;
	gamepacket_PEER->MessageId		= CurSendMessageId_PEER++;
	
	// RE list size always includes the RE terminator
	((RE_Header* )&gamepacket_PEER->RE_List)->RE_Type = RE_EMPTY;
	gamepacket_PEER->RE_ListSize		= sizeof( dword );
}


// return the packetsize of a external DEMO packet ( PEER ) -------------------
//
size_t NETs_NetPacketExternal_DEMO_GetSize( const NetPacketExternal* ext_gamepacket )
{
	ASSERT( ext_gamepacket	!= NULL );
	ASSERT( NET_ProtocolPEER() );
	
	// header is always needed 
	size_t psize = sizeof( NetPacketExternal_DEMO_PEER ) - sizeof( dword );
	
	// add size of remote event list
	RE_Header* relist = (RE_Header *) &((NetPacketExternal_DEMO_PEER*)ext_gamepacket)->RE_List;
	psize += NET_RmEvList_GetSize( relist );
	
	ASSERT( psize <= (size_t)NET_MAX_DATA_LENGTH );
	return psize;
}

// determine actually needed size of packet (strip unused remote event area) --
//
PRIVATE
size_t PEER_NetPacketExternal_GetSize( NetPacketExternal_PEER* ext_gamepacket_PEER )
{
	//NOTE:
	// even though this function is operating on a packet in network byte-order, 
	// swapping is not necessary since all accesses are byte-sized. (that is, 
	// byte-ordering is entirely irrelevant.)
	
	// header is always needed 
	size_t psize = sizeof( NetPacketExternal_PEER ) - sizeof( dword );
	
	// add size of remote event list
	psize += NET_RmEvList_GetSize( (RE_Header *) &ext_gamepacket_PEER->RE_List );
	
	ASSERT( psize <= (size_t)NET_MAX_DATA_LENGTH );
	return psize;
}

// swap endianness of whole (external) network packet -------------------------
//
PRIVATE
void PEER_NetPacketExternal_Swap( NetPacketExternal* ext_gamepacket, int incoming )
{
	ASSERT( ext_gamepacket != NULL );
	
	NetPacketExternal_PEER* ext_gamepacket_PEER = (NetPacketExternal_PEER*)ext_gamepacket;
	
#ifdef ENABLE_PACKET_SWAPPING

	ext_gamepacket_PEER->Protocol	= NET_SWAP_16( ext_gamepacket_PEER->Protocol );
	
	ext_gamepacket_PEER->MessageId  = NET_SWAP_32( ext_gamepacket_PEER->MessageId );
	ext_gamepacket_PEER->param4		= NET_SWAP_32( ext_gamepacket_PEER->param4 );
	ext_gamepacket_PEER->GameTime	= NET_SWAP_32( ext_gamepacket_PEER->GameTime );
	
	//NOTE:
	// in PKTP_PING packets                                      ShipRemInfo == ServerInfo
	
	switch ( ext_gamepacket_PEER->Command ) {
		case PKTP_COMMAND:
			// invalid packets for PEER mode
			ASSERT( FALSE );
			break;
		default:
			{
				// swap endianness of ShipRemInfo in normal packets
				if ( incoming ) {
					SWAP_ShipRemInfo_in( &ext_gamepacket_PEER->ShipInfo );
				} else {
					SWAP_ShipRemInfo_out( &ext_gamepacket_PEER->ShipInfo );
				}
			}
			break;
	}
	
	// swap entire RE list
	NET_RmEvList_Swap( (RE_Header*) &ext_gamepacket_PEER->RE_List, incoming );
	
#endif // ENABLE_PACKET_SWAPPING
}

// swap endianness of whole (external) DEMO packet ----------------------------
//
PRIVATE
void PEER_NetPacketExternal_DEMO_Swap( NetPacketExternal* ext_gamepacket, int incoming )
{
	ASSERT( ext_gamepacket != NULL );
	
	NetPacketExternal_DEMO_PEER* ext_gamepacket_DEMO_PEER = (NetPacketExternal_DEMO_PEER*)ext_gamepacket;
	
#ifdef ENABLE_PACKET_SWAPPING
	
	ext_gamepacket_DEMO_PEER->MessageId  = NET_SWAP_32( ext_gamepacket_DEMO_PEER->MessageId );
	ext_gamepacket_DEMO_PEER->param4	 = NET_SWAP_32( ext_gamepacket_DEMO_PEER->param4 );
	ext_gamepacket_DEMO_PEER->GameTime	 = NET_SWAP_32( ext_gamepacket_DEMO_PEER->GameTime );
	ext_gamepacket_DEMO_PEER->pktsize	 = NET_SWAP_32( ext_gamepacket_DEMO_PEER->pktsize );
	ext_gamepacket_DEMO_PEER->crc32		 = NET_SWAP_32( ext_gamepacket_DEMO_PEER->crc32 );

	//NOTE:
	// in PKTP_PING packets                                      ShipRemInfo == ServerInfo
	
	switch ( ext_gamepacket_DEMO_PEER->Command ) {
		case PKTP_COMMAND:
			// no swapping needed
			break;
		default:
			{
				// swap endianness of ShipRemInfo in normal packets
				if ( incoming ) {
					SWAP_ShipRemInfo_in( &ext_gamepacket_DEMO_PEER->ShipInfo );
				} else {
					SWAP_ShipRemInfo_out( &ext_gamepacket_DEMO_PEER->ShipInfo );
				}
			}
			break;
	}
	
	// swap entire RE list
	NET_RmEvList_Swap( (RE_Header*) &ext_gamepacket_DEMO_PEER->RE_List, incoming );
	
#endif // ENABLE_PACKET_SWAPPING
}


// store the CRC in an external packet ----------------------------------------
//
PRIVATE
void PEER_NetPacketExternal_StoreCRC( NetPacketExternal_PEER* ext_gamepacket, size_t pktsize )
{
	// set the crc32 field to known value and calculate the CRC for the header
	ext_gamepacket->crc32 = 0;
	ext_gamepacket->crc32 = NET_CalcCRC( (void*)ext_gamepacket, pktsize );
	
	// swap CRC
	ext_gamepacket->crc32 = NET_SWAP_32( ext_gamepacket->crc32 );
}

// pack an internal packet to an external packet ( peer-to-peer ) ---------
//
size_t NETs_HandleOutPacket( const NetPacket* int_gamepacket, NetPacketExternal* ext_gamepacket )
{
	ASSERT( int_gamepacket	!= NULL );
	ASSERT( ext_gamepacket	!= NULL );
	ASSERT( NET_ProtocolPEER() );

	NetPacket_PEER*			int_gamepacket_PEER	= (NetPacket_PEER*)			int_gamepacket;
	NetPacketExternal_PEER*	ext_gamepacket_PEER = (NetPacketExternal_PEER*)	ext_gamepacket;
	
	ASSERT( ( int_gamepacket_PEER->SendPlayerId != int_gamepacket_PEER->DestPlayerId ) || 
		    ( ( int_gamepacket_PEER->Command >= PKTP_CONNECT ) && ( int_gamepacket_PEER->Command <= PKTP_SUBDUE_SLAVE ) ) );

	// fill header for external packets
	strncpy( ext_gamepacket_PEER->Signature, net_packet_signature_peer2peer, SIGNATURE_LEN_PEER2PEER );
	ext_gamepacket_PEER->Signature[ SIGNATURE_LEN_PEER2PEER ] = 0;
	ext_gamepacket_PEER->Protocol				= PROTOCOL_PEER2PEER;
	ext_gamepacket_PEER->MajorVersion			= P2P_PROTOCOL_MAJOR;
	ext_gamepacket_PEER->MinorVersion			= P2P_PROTOCOL_MINOR;
	ext_gamepacket_PEER->MessageId				= int_gamepacket_PEER->MessageId;
	
	// pack internal packet data to external packet
	ext_gamepacket_PEER->SendPlayerId			= (signed char)int_gamepacket_PEER->SendPlayerId;
	ext_gamepacket_PEER->MessageId				= int_gamepacket_PEER->MessageId;
	ext_gamepacket_PEER->Command				= (byte)int_gamepacket_PEER->Command;
	ext_gamepacket_PEER->param1					= (signed char)int_gamepacket_PEER->params[ 0 ];
	ext_gamepacket_PEER->param2					= (signed char)int_gamepacket_PEER->params[ 1 ];
	ext_gamepacket_PEER->param3					= (signed char)int_gamepacket_PEER->params[ 2 ];
	ext_gamepacket_PEER->param4					= int_gamepacket_PEER->params[ 3 ];
	
	ext_gamepacket_PEER->DestPlayerId			= (signed char)int_gamepacket_PEER->DestPlayerId;
	ext_gamepacket_PEER->Universe				= int_gamepacket_PEER->Universe;
	ext_gamepacket_PEER->NumPlayers				= int_gamepacket_PEER->NumPlayers;
	for( int nPlayer = 0; nPlayer < MAX_NET_ALLOC_SLOTS; nPlayer++ ) {
		ext_gamepacket_PEER->PlayerKills[ nPlayer ] = int_gamepacket_PEER->PlayerKills[ nPlayer ];
	}
	memcpy( &ext_gamepacket_PEER->ShipInfo, &int_gamepacket_PEER->ShipInfo, sizeof( ShipRemInfo ) );
	ext_gamepacket_PEER->GameTime				= int_gamepacket_PEER->GameTime;
	
	//FIXME: [1/30/2002] remove this, when RE_Listsize is correctly maintained
#ifdef _SET_RE_LISTSIZE_ON_PACK
	int_gamepacket_PEER->RE_ListSize = NET_RmEvList_GetSize( (RE_Header*)&int_gamepacket_PEER->RE_List );
#endif // _SET_RE_LISTSIZE_ON_PACK
	
	// check that RE_ListSize corresponds with the actual length of the remote event list
	ASSERT( int_gamepacket_PEER->RE_ListSize == NET_RmEvList_GetSize( (RE_Header*)&int_gamepacket_PEER->RE_List ) );
	
	memcpy( (void*)&ext_gamepacket_PEER->RE_List, (void*)&int_gamepacket_PEER->RE_List, int_gamepacket_PEER->RE_ListSize );

	// get the size of the packet
	size_t pktsize = PEER_NetPacketExternal_GetSize( ext_gamepacket_PEER );
	
	// establish network byte-order for external packet ( outgoing )
	PEER_NetPacketExternal_Swap( ext_gamepacket_PEER, FALSE );

	// store the CRC in the packet
	PEER_NetPacketExternal_StoreCRC( ext_gamepacket_PEER, pktsize );

#ifdef ENCRYPT_PACKETS
	
	if ( !( AUX_NETCODE_FLAGS & 2 ) ) {
		
		// encrypt packet payload
		NET_EncryptData( &ext_gamepacket_PEER->crc32, pktsize - offsetof( NetPacketExternal_PEER, crc32 ) );

		// establish host byte order, modify field, and establish network order again
		ext_gamepacket_PEER->Protocol = NET_SWAP_16( ext_gamepacket_PEER->Protocol );
		ext_gamepacket_PEER->Protocol |= PROTOCOL_ENCRYPTED; 
		ext_gamepacket_PEER->Protocol = NET_SWAP_16( ext_gamepacket_PEER->Protocol );
	}
	
#endif // ENCRYPT_PACKETS

	return pktsize;
}

// handle an incoming packet (peer-to-peer) -------------------------------
//
int NETs_HandleInPacket( const NetPacketExternal* ext_gamepacket, const int ext_pktsize, NetPacket* int_gamepacket )
{
	ASSERT( ext_gamepacket != NULL );
	ASSERT( ( ext_pktsize > 0 ) && ( ext_pktsize < CurMaxDataLength ) );
	ASSERT( int_gamepacket != NULL );
	ASSERT( NET_ProtocolPEER() );

	// sanity check for packet len
	if ( ext_pktsize > CurMaxDataLength ) {
		DBGTXT( MSGOUT( "NETs_HandleInPacket: dropped packet [invalid packet len]." ); );
		return FALSE;
	}
	
	NetPacket_PEER*			int_gamepacket_PEER	= (NetPacket_PEER*)		 int_gamepacket;
	NetPacketExternal_PEER*	ext_gamepacket_PEER = (NetPacketExternal_PEER*)ext_gamepacket;
	
#ifdef ENCRYPT_PACKETS

	// establish host byte order
	ext_gamepacket_PEER->Protocol = NET_SWAP_16( ext_gamepacket_PEER->Protocol );

	// check whether the packet supposedly is encrypted
	if ( ext_gamepacket_PEER->Protocol & PROTOCOL_ENCRYPTED ) {

		if ( !( AUX_NETCODE_FLAGS & 2 ) ) {
			
			// decrypt external packet
			NET_DecryptData( &ext_gamepacket_PEER->crc32, ext_pktsize - offsetof( NetPacketExternal_PEER, crc32 ) );
		}

		// revert to decrypted protocol
		ext_gamepacket_PEER->Protocol &= ~PROTOCOL_ENCRYPTED;
	}

	// establish network byte order
	ext_gamepacket_PEER->Protocol = NET_SWAP_16( ext_gamepacket_PEER->Protocol );

#endif // ENCRYPT_PACKETS

	if ( !( AUX_NETCODE_FLAGS & 1 ) ) {

		// as the CRC has been calculated with the crc32 field stored in the packet beeing set to zero.
		// we need to do this here as well, backup CRC for compare
		dword crc32_stored = NET_SWAP_32( ext_gamepacket_PEER->crc32 );
		ext_gamepacket_PEER->crc32 = 0;

		// calculate CRC over packet
		dword crc32 = NET_CalcCRC( ext_gamepacket_PEER, ext_pktsize );
		
		// check whether CRC is correct
		if ( crc32 != crc32_stored ) {
			DBGTXT( MSGOUT( "NETs_HandleInPacket: dropped packet [CRC check failed]." ); );
			return FALSE;
		}
	}

	// check for correct signature
	if ( strncmp( ext_gamepacket_PEER->Signature, net_packet_signature_peer2peer, SIGNATURE_LEN_PEER2PEER ) != 0 ) {
		DBGTXT( MSGOUT( "NETs_HandleInPacket: dropped packet [invalid signature]." ); );
		return FALSE;
	}

	// convert external packet from packet byte-order to host byte-order
	PEER_NetPacketExternal_Swap( ext_gamepacket_PEER, TRUE );

	// check for correct protocol
	if ( ext_gamepacket_PEER->Protocol != PROTOCOL_PEER2PEER ) {
		DBGTXT( MSGOUT( "NETs_HandleInPacket: dropped packet [invalid protocol]." ); );
		return FALSE;
	}

	// check for correct protocol version
	if ( ( ext_gamepacket_PEER->MajorVersion != P2P_PROTOCOL_MAJOR ) || ( ext_gamepacket_PEER->MinorVersion != P2P_PROTOCOL_MINOR ) ) {
		DBGTXT( MSGOUT( "NETs_HandleInPacket: dropped packet [incompatible protocol version]." ); );
		return FALSE;
	}

	// check for correct universe
	if ( ext_gamepacket_PEER->Universe != MyUniverse ) {
		DBGTXT( MSGOUT( "NETs_HandleInPacket: dropped packet [wrong universe]." ); );
		return FALSE;
	}
	
	// unpack external packet data to internal packet
	int_gamepacket_PEER->SendPlayerId			= (int)ext_gamepacket_PEER->SendPlayerId;
	int_gamepacket_PEER->MessageId 				= ext_gamepacket_PEER->MessageId;
	int_gamepacket_PEER->Command 				= (int)ext_gamepacket_PEER->Command;
	int_gamepacket_PEER->params[ 0 ] 			= (int)ext_gamepacket_PEER->param1;
	int_gamepacket_PEER->params[ 1 ] 			= (int)ext_gamepacket_PEER->param2;
	int_gamepacket_PEER->params[ 2 ] 			= (int)ext_gamepacket_PEER->param3;
	int_gamepacket_PEER->params[ 3 ] 			= ext_gamepacket_PEER->param4;
	
	int_gamepacket_PEER->DestPlayerId			= (int)ext_gamepacket_PEER->DestPlayerId;
	int_gamepacket_PEER->Universe				= ext_gamepacket_PEER->Universe;
	int_gamepacket_PEER->NumPlayers				= ext_gamepacket_PEER->NumPlayers;
	for( int nPlayer = 0; nPlayer < MAX_NET_ALLOC_SLOTS; nPlayer++ ) {
		int_gamepacket_PEER->PlayerKills[ nPlayer ] = ext_gamepacket_PEER->PlayerKills[ nPlayer ];
	}
	memcpy( &int_gamepacket_PEER->ShipInfo, &ext_gamepacket_PEER->ShipInfo, sizeof( ShipRemInfo ) );
	int_gamepacket_PEER->GameTime				= ext_gamepacket_PEER->GameTime;

	ASSERT( ( int_gamepacket_PEER->SendPlayerId != int_gamepacket_PEER->DestPlayerId ) || ( ( int_gamepacket_PEER->Command >= PKTP_CONNECT ) && ( int_gamepacket_PEER->Command <= PKTP_SUBDUE_SLAVE ) ) );
	
	// check whether external remote event list is well formed ( integrity and length )
	if ( NET_RmEvList_IsWellFormed( (RE_Header*)&ext_gamepacket_PEER->RE_List ) == FALSE ) { 
		DBGTXT( MSGOUT( "NETs_HandleInPacket: dropped packet [invalid RE list]." ); );
		return FALSE;
	}
	
	// determine remote event list size ( includes RE termination )
	int_gamepacket_PEER->RE_ListSize	= NET_RmEvList_GetSize( (RE_Header*)&ext_gamepacket_PEER->RE_List );
	memcpy( (void*)&int_gamepacket_PEER->RE_List, (void*)&ext_gamepacket_PEER->RE_List, int_gamepacket_PEER->RE_ListSize );
	
	return TRUE;
}

// translate internal packet to external DEMO packet (peer-to-peer) -----------
//
size_t NETs_HandleOutPacket_DEMO( const NetPacket* int_gamepacket, NetPacketExternal* ext_gamepacket )
{
	ASSERT( int_gamepacket	!= NULL );
	ASSERT( ext_gamepacket	!= NULL );
	ASSERT( NET_ProtocolPEER() );

	NetPacket_PEER*				int_gamepacket_PEER		 = (NetPacket_PEER*)				int_gamepacket;
	NetPacketExternal_DEMO_PEER*	ext_gamepacket_DEMO_PEER = (NetPacketExternal_DEMO_PEER*)	ext_gamepacket;

	ASSERT( ( int_gamepacket_PEER->SendPlayerId != int_gamepacket_PEER->DestPlayerId ) || ( ( int_gamepacket_PEER->Command >= PKTP_CONNECT ) && ( int_gamepacket_PEER->Command <= PKTP_SUBDUE_SLAVE ) ) );
	
	// fill header for external packets
	strncpy( ext_gamepacket_DEMO_PEER->Signature, net_packet_signature_peer2peer, SIGNATURE_LEN_PEER2PEER );
	ext_gamepacket_DEMO_PEER->Signature[ SIGNATURE_LEN_PEER2PEER ] = 0;
	ext_gamepacket_DEMO_PEER->Protocol				= PROTOCOL_PEER2PEER;
	ext_gamepacket_DEMO_PEER->MajorVersion			= P2P_PROTOCOL_MAJOR;
	ext_gamepacket_DEMO_PEER->MinorVersion			= P2P_PROTOCOL_MINOR;
	ext_gamepacket_DEMO_PEER->MessageId				= int_gamepacket_PEER->MessageId;
	
	// pack internal packet data to external packet
	ext_gamepacket_DEMO_PEER->SendPlayerId			= (signed char)int_gamepacket_PEER->SendPlayerId;
	ext_gamepacket_DEMO_PEER->MessageId				= int_gamepacket_PEER->MessageId;
	ext_gamepacket_DEMO_PEER->Command				= (byte)int_gamepacket_PEER->Command;
	ext_gamepacket_DEMO_PEER->param1				= (signed char)int_gamepacket_PEER->params[ 0 ];
	ext_gamepacket_DEMO_PEER->param2				= (signed char)int_gamepacket_PEER->params[ 1 ];
	ext_gamepacket_DEMO_PEER->param3				= (signed char)int_gamepacket_PEER->params[ 2 ];
	ext_gamepacket_DEMO_PEER->param4				= int_gamepacket_PEER->params[ 3 ];
	
	ext_gamepacket_DEMO_PEER->DestPlayerId			= (signed char)int_gamepacket_PEER->DestPlayerId;
	ext_gamepacket_DEMO_PEER->Universe				= int_gamepacket_PEER->Universe;
	ext_gamepacket_DEMO_PEER->NumPlayers			= int_gamepacket_PEER->NumPlayers;
	for( int nPlayer = 0; nPlayer < MAX_NET_ALLOC_SLOTS; nPlayer++ ) {
		ext_gamepacket_DEMO_PEER->PlayerKills[ nPlayer ] = int_gamepacket_PEER->PlayerKills[ nPlayer ];
	}
	memcpy( &ext_gamepacket_DEMO_PEER->ShipInfo, &int_gamepacket_PEER->ShipInfo, sizeof( ShipRemInfo ) );
	ext_gamepacket_DEMO_PEER->GameTime				= int_gamepacket_PEER->GameTime;
	
	//FIXME: [1/30/2002] remove this, when RE_Listsize is correctly maintained
#ifdef _SET_RE_LISTSIZE_ON_PACK
	int_gamepacket_PEER->RE_ListSize = NET_RmEvList_GetSize( (RE_Header*)&int_gamepacket_PEER->RE_List );
#endif // _SET_RE_LISTSIZE_ON_PACK
	
	// check that RE_ListSize corresponds with the actual length of the remote event list
	ASSERT( int_gamepacket_PEER->RE_ListSize == NET_RmEvList_GetSize( (RE_Header*)&int_gamepacket_PEER->RE_List ) );
	
	memcpy( (void*)&ext_gamepacket_DEMO_PEER->RE_List, (void*)&int_gamepacket_PEER->RE_List, int_gamepacket_PEER->RE_ListSize );

	// get the packet length and store it in the DEMO packet
	size_t pktsize = NETs_NetPacketExternal_DEMO_GetSize( ext_gamepacket_DEMO_PEER );
	ext_gamepacket_DEMO_PEER->pktsize = pktsize;
	
	// establish network byte-order for external packet ( outgoing )
	PEER_NetPacketExternal_DEMO_Swap( ext_gamepacket_DEMO_PEER, FALSE );

	// set the crc32 and crc_header fields to known value 
	// FIXME: in GMSV mode we could set the CRC to the challenge
	ext_gamepacket_DEMO_PEER->crc32			= 0;
	ext_gamepacket_DEMO_PEER->crc_header	= 0;
	
	// calculate the CRC for the whole packet and establish network byte order
	ext_gamepacket_DEMO_PEER->crc32	= NET_CalcCRC( (void*)ext_gamepacket_DEMO_PEER, pktsize );
	ext_gamepacket_DEMO_PEER->crc32 = NET_SWAP_32( ext_gamepacket_DEMO_PEER->crc32 );

#ifdef ENCRYPT_PACKETS
	
	if ( !( AUX_NETCODE_FLAGS & 2 ) ) {
		
		// encrypt packet payload and crc32 field
		NET_EncryptData( &ext_gamepacket_DEMO_PEER->MessageId,	pktsize - offsetof( NetPacketExternal_DEMO_PEER, MessageId ) );
		NET_EncryptData( &ext_gamepacket_DEMO_PEER->crc32,		sizeof( dword ) );

		// modify protocol to indicate an encrypted packet
		ext_gamepacket_DEMO_PEER->Protocol |= PROTOCOL_ENCRYPTED; 
	}

#endif // ENCRYPY_PACKETS

	// calculate the CRC for the whole packet and establish network byte order
	ext_gamepacket_DEMO_PEER->crc_header  = NET_CalcCRC( (void*)ext_gamepacket_DEMO_PEER, offsetof( NetPacketExternal_DEMO_PEER, crc_header ) );
	ext_gamepacket_DEMO_PEER->crc_header  = NET_SWAP_32( ext_gamepacket_DEMO_PEER->crc_header );
	
	return pktsize;
}


// translate an external PEER DEMO packet to internal PEER packet. also handles 0190 packets
// 
int NETs_HandleInPacket_DEMO( const NetPacketExternal* ext_gamepacket, NetPacket* int_gamepacket, size_t* psize_external )
{
	//NOTE: sets psize_external to the packetsize of the external packet 
	//		( needed for playback from compiled demos )
		
	//NOTE: recorded packets can be in the following formats: 
	//
	//	- NetPacketExternal_DEMO_PEER_0190 = OldNetGameData	( P2P_PROTOCOL: 0.1 )		build 0190
	//	- NetPacketExternal_PEER							( P2P_PROTOCOL: 0.3 )		build >=0198
	//	- NetPacketExternal_GMSV							( GMSV_PROTOCOL: >= 0.19 )	build >=0198

	ASSERT( ext_gamepacket != NULL );
	ASSERT( int_gamepacket	   != NULL );
	ASSERT( NET_ProtocolPEER() );

	// check whether this is a >= 0.3 PEER packet
	if( strncmp( ext_gamepacket->Signature, net_packet_signature_peer2peer, SIGNATURE_LEN_PEER2PEER ) == 0 ) {

		NetPacket_PEER*				int_gamepacket_PEER		 = (NetPacket_PEER*)				int_gamepacket;
		NetPacketExternal_DEMO_PEER*	ext_gamepacket_DEMO_PEER = (NetPacketExternal_DEMO_PEER*)	ext_gamepacket;

		//NOTE: 
		// as we must ensure that the pktsize stored in the packet is not tampered 
		// with, we calculate a seperate CRC for the header in a DEMO_PEER packet
		
		if ( !( AUX_NETCODE_FLAGS & 1 ) ) {

			// calculate the CRC over the header ( does not include crc_header field itself )
			dword crc_header = NET_CalcCRC( (void*)ext_gamepacket_DEMO_PEER, offsetof( NetPacketExternal_DEMO_PEER, crc_header ) );
			
			// check whether CRC of header matches
			if ( crc_header != NET_SWAP_32( ext_gamepacket_DEMO_PEER->crc_header ) ) {
				DBGTXT( MSGOUT( "NETs_HandleInPacket_DEMO: dropped packet [header CRC check failed]." ); );

				// indicate, that the stored packet len cannot be trusted, as the CRC check failed, this 
				// stops replaying the demo
				*psize_external = (size_t)-1;

				return FALSE;
			}
		}

		// get the stored length of the packet, as this can be trusted now
		size_t ext_pktsize = NET_SWAP_32( ext_gamepacket_DEMO_PEER->pktsize );
		
		// sanity check for packet len
		if ( ext_pktsize > (size_t)CurMaxDataLength ) {
			DBGTXT( MSGOUT( "NETs_HandleInPacket_DEMO: dropped packet [invalid packet len]." ); );

			// indicate, that the stored packet len cannot be trusted, as the CRC 
			// check failed, this stops replaying the demo
			*psize_external = (size_t)-1;
			
			return FALSE;
		}

		// establish host byte order
		ext_gamepacket_DEMO_PEER->Protocol = NET_SWAP_16( ext_gamepacket_DEMO_PEER->Protocol );

#ifdef ENCRYPT_PACKETS

		// check whether the packet supposedly is encrypted
		if ( ext_gamepacket_DEMO_PEER->Protocol & PROTOCOL_ENCRYPTED ) {
			
			if ( !( AUX_NETCODE_FLAGS & 2 ) ) {
				
				// decrypt packet payload and crc32 field
				NET_DecryptData( &ext_gamepacket_DEMO_PEER->MessageId,	ext_pktsize - offsetof( NetPacketExternal_DEMO_PEER, MessageId ) );
				NET_DecryptData( &ext_gamepacket_DEMO_PEER->crc32,		sizeof( dword ) );
			}
			
			// revert to decrypted protocol
			ext_gamepacket_DEMO_PEER->Protocol &= ~PROTOCOL_ENCRYPTED;
		}

#endif // ENCRYPT_PACKETS

		//NOTE: 
		// we need to return the proper packetsize, in order for CON_ACT::ExecBinCommands 
		// to know the stepsize for the next action command. thus we set it to 
		// the size stored in the DEMO packets
		*psize_external = ext_pktsize;

		if ( !( AUX_NETCODE_FLAGS & 1 ) ) {

			// as the CRC for the whole packet has been calculated with the crc32 
			// and crc_header field set to zero we need to save the CRC of the packet
			dword crc32_stored = NET_SWAP_32( ext_gamepacket_DEMO_PEER->crc32 );

			// establish same format as in outgoing packet
			ext_gamepacket_DEMO_PEER->crc32			= 0;
			ext_gamepacket_DEMO_PEER->crc_header	= 0;

			// calculate CRC over packet
			dword crc32 = NET_CalcCRC( (void*)ext_gamepacket_DEMO_PEER, ext_pktsize );
			
			// check whether CRC is correct
			if ( crc32 != crc32_stored ) {
				DBGTXT( MSGOUT( "NETs_HandleInPacket_DEMO: dropped packet [CRC check failed]." ); );
				return FALSE;
			}
		}
		
		// check for correct signature
		if ( strncmp( ext_gamepacket_DEMO_PEER->Signature, net_packet_signature_peer2peer, SIGNATURE_LEN_PEER2PEER ) != 0 ) {
			DBGTXT( MSGOUT( "NETs_HandleInPacket_DEMO: dropped packet [invalid signature]." ); );
			return FALSE;
		}
		
		// check for correct protocol
		if ( ext_gamepacket_DEMO_PEER->Protocol != PROTOCOL_PEER2PEER ) {
			DBGTXT( MSGOUT( "NETs_HandleInPacket_DEMO: dropped packet [invalid protocol]." ); );
			return FALSE;
		}
		
		// check for correct protocol version
		if ( ( ext_gamepacket_DEMO_PEER->MajorVersion != P2P_PROTOCOL_MAJOR ) || 
			 ( ext_gamepacket_DEMO_PEER->MinorVersion != P2P_PROTOCOL_MINOR ) ) {
			DBGTXT( MSGOUT( "NETs_HandleInPacket_DEMO: dropped packet [incompatible protocol version]." ); );
			return FALSE;
		}

		// convert external packet from packet byte-order to host byte-order
		PEER_NetPacketExternal_DEMO_Swap( ext_gamepacket_DEMO_PEER, TRUE );

		// determine remote event list size ( includes RE termination )
		int_gamepacket_PEER->RE_ListSize = NET_RmEvList_GetSize( 
			(RE_Header*)&ext_gamepacket_DEMO_PEER->RE_List );
		
		// sanety check, whether the length stored in the packet actually is correct 
		ASSERT( ext_pktsize == ( int_gamepacket_PEER->RE_ListSize + 
			sizeof( NetPacketExternal_DEMO_PEER ) - sizeof( dword ) ) );

		//NOTE: we do not check the universe here
		
		// unpack external packet data to internal packet
		int_gamepacket_PEER->SendPlayerId	= (int)ext_gamepacket_DEMO_PEER->SendPlayerId;
		int_gamepacket_PEER->MessageId 		= ext_gamepacket_DEMO_PEER->MessageId;
		int_gamepacket_PEER->Command 		= (int)ext_gamepacket_DEMO_PEER->Command;
		int_gamepacket_PEER->params[ 0 ] 	= (int)ext_gamepacket_DEMO_PEER->param1;
		int_gamepacket_PEER->params[ 1 ] 	= (int)ext_gamepacket_DEMO_PEER->param2;
		int_gamepacket_PEER->params[ 2 ] 	= (int)ext_gamepacket_DEMO_PEER->param3;
		int_gamepacket_PEER->params[ 3 ] 	= ext_gamepacket_DEMO_PEER->param4;
		
		int_gamepacket_PEER->DestPlayerId	= (int)ext_gamepacket_DEMO_PEER->DestPlayerId;
		int_gamepacket_PEER->Universe		= ext_gamepacket_DEMO_PEER->Universe;
		int_gamepacket_PEER->NumPlayers		= ext_gamepacket_DEMO_PEER->NumPlayers;
		for( int nPlayer = 0; nPlayer < MAX_NET_ALLOC_SLOTS; nPlayer++ ) {
			int_gamepacket_PEER->PlayerKills[ nPlayer ] = ext_gamepacket_DEMO_PEER->PlayerKills[ nPlayer ];
		}
		memcpy( &int_gamepacket_PEER->ShipInfo, &ext_gamepacket_DEMO_PEER->ShipInfo, sizeof( ShipRemInfo ) );
		int_gamepacket_PEER->GameTime		= ext_gamepacket_DEMO_PEER->GameTime;

		ASSERT( ( int_gamepacket_PEER->SendPlayerId != int_gamepacket_PEER->DestPlayerId ) || ( ( int_gamepacket_PEER->Command >= PKTP_CONNECT ) && ( int_gamepacket_PEER->Command <= PKTP_SUBDUE_SLAVE ) ) );

		// check whether external remote event list is well formed ( integrity and length )
		if ( NET_RmEvList_IsWellFormed( (RE_Header*)&ext_gamepacket_DEMO_PEER->RE_List ) == FALSE ) { 
			DBGTXT( MSGOUT( "NETs_HandleInPacket_DEMO: dropped packet [invalid RE list]." ); );
			return FALSE;
		}
		
		memcpy( (void*)&int_gamepacket_PEER->RE_List, (void*)&ext_gamepacket_DEMO_PEER->RE_List, int_gamepacket_PEER->RE_ListSize );

		return TRUE;

	} else {
		// convert NetPacketExternal_DEMO_PEER_0190 to NetPacket_PEER

		NetPacketExternal_DEMO_PEER_0190* ext_gamepacket_DEMO_PEER_0190 = (NetPacketExternal_DEMO_PEER_0190*)ext_gamepacket;

		// default to nothing stripped
		size_t pkt_stripped_size = 0;

		// check whether this is a packet with the signature removed ( old compiled demos ( < 0198 ) )
		if ( strncmp( ext_gamepacket_DEMO_PEER_0190->Signature, net_game_signature, PACKET_SIGNATURE_SIZE ) != 0 ) {
	
#define COPY_STRIPPED_PACKET			
#ifdef COPY_STRIPPED_PACKET
			// adjust for missing signature
			char tempbuffer[ RECORD_PACKET_SIZE ];
			memcpy( tempbuffer + PACKET_SIGNATURE_SIZE, ext_gamepacket_DEMO_PEER_0190, RECORD_PACKET_SIZE - PACKET_SIGNATURE_SIZE );
			memset( tempbuffer, 0, PACKET_SIGNATURE_SIZE );

			ext_gamepacket_DEMO_PEER_0190 = (NetPacketExternal_DEMO_PEER_0190*)tempbuffer;
#else
			//HACK: to avoid copy. this definitely has problems if ext_gamepacket_DEMO_PEER_0190 is at adress 0x00000007 or lower :)
			ext_gamepacket_DEMO_PEER_0190 = (NetPacketExternal_DEMO_PEER_0190*) (((long)ext_gamepacket_DEMO_PEER_0190) - PACKET_SIGNATURE_SIZE);
#endif // COPY_STRIPPED_PACKET

			pkt_stripped_size = PACKET_SIGNATURE_SIZE;
		}
		
		//TODO: we must find some additional "typical" pattern to identify packet as NetPacketExternal_DEMO_PEER_0190 !!!

		NetPacket_PEER* int_gamepacket_PEER = (NetPacket_PEER*)int_gamepacket;
		
		// clear internal packet
		memset( int_gamepacket_PEER, 0, NET_MAX_NETPACKET_INTERNAL_LEN );
	
		int_gamepacket_PEER->SendPlayerId		= (int) ext_gamepacket_DEMO_PEER_0190->SendPlayerId;
		int_gamepacket_PEER->MessageId			= NET_SWAP_32( ext_gamepacket_DEMO_PEER_0190->MessageId );
		int_gamepacket_PEER->Command			= (int) ext_gamepacket_DEMO_PEER_0190->Command;
		int_gamepacket_PEER->params[ 0 ]		= (int) ext_gamepacket_DEMO_PEER_0190->param1;
		int_gamepacket_PEER->params[ 1 ]		= (int) ext_gamepacket_DEMO_PEER_0190->param1;
		int_gamepacket_PEER->params[ 2 ]		= (int) ext_gamepacket_DEMO_PEER_0190->param1;
		int_gamepacket_PEER->params[ 3 ]		= (int) ext_gamepacket_DEMO_PEER_0190->param1;
		int_gamepacket_PEER->DestPlayerId		= (int) ext_gamepacket_DEMO_PEER_0190->DestPlayerId;
		int_gamepacket_PEER->Universe			= ext_gamepacket_DEMO_PEER_0190->Universe;
		int_gamepacket_PEER->NumPlayers			= ext_gamepacket_DEMO_PEER_0190->NumPlayers;
		int_gamepacket_PEER->PlayerKills[ 0 ]	= ext_gamepacket_DEMO_PEER_0190->PlayerKills[ 0 ];
		int_gamepacket_PEER->PlayerKills[ 1 ]	= ext_gamepacket_DEMO_PEER_0190->PlayerKills[ 1 ];
		int_gamepacket_PEER->PlayerKills[ 2 ]	= ext_gamepacket_DEMO_PEER_0190->PlayerKills[ 2 ];
		int_gamepacket_PEER->PlayerKills[ 3 ]	= ext_gamepacket_DEMO_PEER_0190->PlayerKills[ 3 ];
		int_gamepacket_PEER->GameTime			= NET_SWAP_32( ext_gamepacket_DEMO_PEER_0190->GameTime );
		
		memcpy( &int_gamepacket_PEER->ShipInfo, &ext_gamepacket_DEMO_PEER_0190->ShipInfo, sizeof( ShipRemInfo ) );
		SWAP_ShipRemInfo_in( &int_gamepacket_PEER->ShipInfo );
		
		// RE size can be determined without swapping
		int_gamepacket_PEER->RE_ListSize	= NET_RmEvList_GetSize( (RE_Header*)&ext_gamepacket_DEMO_PEER_0190->RE_List );
		memcpy( &int_gamepacket_PEER->RE_List, &ext_gamepacket_DEMO_PEER_0190->RE_List, int_gamepacket_PEER->RE_ListSize );
		
		// swap RE list
		NET_RmEvList_Swap( (RE_Header*) &int_gamepacket_PEER->RE_List, TRUE );

		//TODO: check RE list with RmEvList_IsWellFormed

		// replace class ids in RE_CREATEEXTRA remote events with current extra indexes
		for( RE_Header* relist = (RE_Header*) &int_gamepacket_PEER->RE_List; relist->RE_Type != RE_EMPTY; ) {
			
			if ( relist->RE_Type == RE_CREATEEXTRA ) {
				
				RE_CreateExtra* re_ce  = (RE_CreateExtra*) relist;
				
				//NOTE:
				// re_ce->ExtraIndex is actually a classid at this point
				// packet is already in host byte-order
				
				word classid = re_ce->ExtraIndex;
				
				ASSERT( classid				!= (word)CLASS_ID_INVALID );
				ASSERT( ObjClassExtraIndex[ classid ]	!= EXTRAINDEX_NO_EXTRA );
				
				re_ce->ExtraIndex = ObjClassExtraIndex[ classid ];
			}
			
			// advance to next event in list
			ASSERT( ( relist->RE_BlockSize == RE_BLOCKSIZE_INVALID ) ||
					( relist->RE_BlockSize == NET_RmEvGetSize( relist ) ) );

			relist = (RE_Header*) ( (char*) relist + NET_RmEvGetSize( relist ) );
		}

		// size of RE list already contains the terminating RE header, also account for the missing signature
		*psize_external = int_gamepacket_PEER->RE_ListSize + sizeof( NetPacketExternal_DEMO_PEER_0190 ) - sizeof( dword ) - pkt_stripped_size;

		return TRUE;
	}
}



// fp to which PacketInfo functions write -------------------------------------
//
extern FILE *dest_fp_PacketInfo;


// write verbose packet info as comment to recording file ---------------------
//
void NETs_WritePacketInfo( FILE *fp, NetPacketExternal* ext_gamepacket )
{
	ASSERT( fp != NULL );
	ASSERT( ext_gamepacket != NULL );
	
	// store fp for PrInf_ functions
	dest_fp_PacketInfo = fp;
	
	// create packet copy and swap from network byte-order
	NetPacketExternal_PEER* ext_gamepacket_copy = (NetPacketExternal_PEER*) ALLOCMEM( NET_MAX_DATA_LENGTH );
	if ( ext_gamepacket_copy == NULL )
		return;
	
	memcpy( ext_gamepacket_copy, ext_gamepacket, NET_MAX_DATA_LENGTH );

	// swap to host byte order
	PEER_NetPacketExternal_Swap( ext_gamepacket_copy, TRUE );
	
	NET_PacketInfo( ";-- command %d, param1=%d, param2=%d, param3=%d, param4=%d\n", 
					ext_gamepacket_copy->Command, 
					ext_gamepacket_copy->param1, 
					ext_gamepacket_copy->param2, 
					ext_gamepacket_copy->param3, 
					ext_gamepacket_copy->param4 );

	NET_PacketInfo( ";-- universe=%d, numplayers=%d, srcid=%d, destid=%d\n",		
					ext_gamepacket_copy->Universe, 
					ext_gamepacket_copy->NumPlayers, 
					ext_gamepacket_copy->SendPlayerId, 
					ext_gamepacket_copy->DestPlayerId );

	NET_PacketInfo( ";-- kills[0]=%d, kills[1]=%d, kills[2]=%d, kills[3]=%d\n",		
					ext_gamepacket_copy->PlayerKills[ 0 ], 
					ext_gamepacket_copy->PlayerKills[ 1 ],	
					ext_gamepacket_copy->PlayerKills[ 2 ], 
					ext_gamepacket_copy->PlayerKills[ 3 ] );

	if ( CurMaxPlayers > 4 ) {
		NET_PacketInfo( ";-- kills[4]=%d, kills[5]=%d, kills[6]=%d, kills[7]=%d\n",		
					ext_gamepacket_copy->PlayerKills[ 4 ], 
					ext_gamepacket_copy->PlayerKills[ 5 ],	
					ext_gamepacket_copy->PlayerKills[ 6 ], 
					ext_gamepacket_copy->PlayerKills[ 7 ] );
	}
	
	// process remote event list
	NET_RmEvList_WriteInfo( fp, (RE_Header *) &ext_gamepacket_copy->RE_List );
}


