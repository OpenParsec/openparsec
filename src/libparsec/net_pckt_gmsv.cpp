/*
 * PARSEC - Packet Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:45 $
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
#include "net_pckt_gmsv.h"

// proprietary module headers
#if defined ( PARSEC_SERVER ) || defined ( PARSEC_MASTER )
	#include "con_aux_sv.h"
	#include "net_game_sv.h"
#elif defined PARSEC_CLIENT
	#include "con_aux.h"
	#include "net_stream.h"
	#include "net_game.h"
	#include "net_rmev.h"
#endif // PARSEC_SERVER

#include "net_csdf.h"
#include "net_pckt.h"
#include "net_swap.h"
#include "net_util.h"



// flags ----------------------------------------------------------------------
//
#ifndef PARSEC_MASTER
	//#define LOG_PACKETS		// define this to get logs of incoming/outgoing packets
#endif // PARSEC_MASTER


#ifdef PARSEC_CLIENT

	#ifdef DBIND_PROTOCOL

		#undef  NETs_HandleOutPacket
		#undef  NETs_HandleOutPacket_DEMO
		#undef  NETs_HandleInPacket
		#undef  NETs_HandleInPacket_DEMO
		#undef	NETs_NetPacketExternal_DEMO_GetSize
		#undef  NETs_StdGameHeader
		#undef	NETs_WritePacketInfo

		#define NETs_HandleOutPacket				NETs_GAMESERVER_HandleOutPacket
		#define NETs_HandleOutPacket_DEMO			NETs_GAMESERVER_HandleOutPacket_DEMO
		#define NETs_HandleInPacket					NETs_GAMESERVER_HandleInPacket
		#define NETs_HandleInPacket_DEMO			NETs_GAMESERVER_HandleInPacket_DEMO
		#define NETs_NetPacketExternal_DEMO_GetSize NETs_GAMESERVER_NetPacketExternal_DEMO_GetSize
		#define NETs_StdGameHeader					NETs_GAMESERVER_StdGameHeader
		#define NETs_WritePacketInfo				NETs_GAMESERVER_WritePacketInfo

	#endif // DBIND_PROTOCOL

#endif // PARSEC_CLIENT



#ifdef LOG_PACKETS

	#include "utl_logfile.h"

	static UTL_LogFile g_PacketInLogfile( "packets.in.log" );
	static UTL_LogFile g_PacketOutLogfile( "packets.out.log" );
#endif // LOG_PACKETS



// flags ----------------------------------------------------------------------
//
#define _SET_RE_LISTSIZE_ON_PACK
//#define SHOW_CRC32_WHEN_SENDING

// protocol type signature ----------------------------------------------------
//
const char net_packet_signature_gameserver[]	= "PCS";

#ifdef PARSEC_CLIENT

	extern NET_Stream ServerStream;

	// fill standard fields in gamedata header  ( gameserver ) ----------------
	//
	void NETs_StdGameHeader( byte command, NetPacket* gamepacket )
	{
		ASSERT( gamepacket != NULL );
		
		NetPacket_GMSV* gamepacket_GMSV = (NetPacket_GMSV*) gamepacket;
		
		// clear header and remote event list area
		memset( gamepacket_GMSV, 0, NET_MAX_NETPACKET_INTERNAL_LEN );
		
		gamepacket_GMSV->Command				= command;
		gamepacket_GMSV->SendPlayerId			= LocalPlayerId;

		//FIXME: for debugging purposes, we should call OutPacket() with filled RE list in gamepacket
		ServerStream.OutPacket( gamepacket_GMSV );
		
		// RE list size always includes the RE terminator
		((RE_Header* )&gamepacket_GMSV->RE_List)->RE_Type = RE_EMPTY;
		gamepacket_GMSV->RE_ListSize		= sizeof( dword );
	}

#endif // PARSEC_CLIENT

// return the packetsize of a external DEMO packet ( PEER ) -------------------
//
size_t NETs_NetPacketExternal_DEMO_GetSize( const NetPacketExternal* ext_gamepacket )
{
	ASSERT( ext_gamepacket	!= NULL );
#ifdef PARSEC_CLIENT
	ASSERT( NET_ProtocolGMSV() );
#endif // PARSEC_CLIENT

	// header is always needed 
	size_t psize = sizeof( NetPacketExternal_DEMO_GMSV ) - sizeof( dword );

	// add size of remote event list
	RE_Header* relist = (RE_Header *) &((NetPacketExternal_DEMO_GMSV*)ext_gamepacket)->RE_List;
	psize += NET_RmEvList_GetSize( relist );

	ASSERT( psize <= (size_t)NET_MAX_DATA_LENGTH );
	return psize;
}

// determine actually needed size of packet (strip unused remote event area) --
//
PRIVATE
size_t GMSV_NetPacketExternal_GetSize( NetPacketExternal_GMSV* ext_gamepacket_GMSV )
{
	//NOTE:
	// even though this function is operating on a packet in network byte-order, 
	// swapping is not necessary since all accesses are byte-sized. (that is, 
	// byte-ordering is entirely irrelevant.)
	
	// header is always needed 
	size_t psize = sizeof( NetPacketExternal_GMSV ) - sizeof( dword );
	
	// add size of remote event list
	psize += NET_RmEvList_GetSize( (RE_Header *) &ext_gamepacket_GMSV->RE_List );
	
	//ASSERT( psize <= (size_t)NET_MAX_DATA_LENGTH );
	if(psize >= (size_t)NET_MAX_DATA_LENGTH )
        MSGOUT("CRAZYSPENCE: Packet size larger than %d: %d",NET_MAX_DATA_LENGTH,psize);
    return psize;
}

// swap endianness of whole (external) network packet -------------------------
//
PRIVATE
void GMSV_NetPacketExternal_Swap( NetPacketExternal* ext_gamepacket, int incoming )
{
	ASSERT( ext_gamepacket != NULL );
	
	NetPacketExternal_GMSV* ext_gamepacket_GMSV = (NetPacketExternal_GMSV*)ext_gamepacket;
	
#ifdef ENABLE_PACKET_SWAPPING

	ext_gamepacket_GMSV->Protocol				= NET_SWAP_16( ext_gamepacket_GMSV->Protocol );

	ext_gamepacket_GMSV->MessageId				= NET_SWAP_32( ext_gamepacket_GMSV->MessageId			);
	ext_gamepacket_GMSV->SendPlayerId			= NET_SWAP_32( ext_gamepacket_GMSV->SendPlayerId		);
	ext_gamepacket_GMSV->ReliableMessageId		= NET_SWAP_32( ext_gamepacket_GMSV->ReliableMessageId	);
	ext_gamepacket_GMSV->AckMessageId			= NET_SWAP_32( ext_gamepacket_GMSV->AckMessageId		);
	ext_gamepacket_GMSV->AckReliableMessageId	= NET_SWAP_32( ext_gamepacket_GMSV->AckReliableMessageId );
	
	// swap entire RE list
	NET_RmEvList_Swap( (RE_Header*) &ext_gamepacket_GMSV->RE_List, incoming );
	
#endif // ENABLE_PACKET_SWAPPING
}


// store the CRC in an external packet ----------------------------------------
//
PRIVATE
void GMSV_NetPacketExternal_StoreCRC( NetPacketExternal_GMSV* ext_gamepacket, size_t pktsize )
{
	// challenge we got from server
	//FIXME: this should go into a module NET_GLOB_GMSV
	//extern int CS_connect_challenge;

	// set the crc32 field to known value and calculate the CRC for the header
	ext_gamepacket->crc32 = 0;
	ext_gamepacket->crc32 = NET_CalcCRC( (void*)ext_gamepacket, pktsize );

#ifdef SHOW_CRC32_WHEN_SENDING
	 DBGTXT( MSGOUT( "CRC32 of packet %d is %x", ext_gamepacket->MessageId, ext_gamepacket->crc32 ); );
#endif // SHOW_CRC32_WHEN_SENDING
	
	// swap CRC
	ext_gamepacket->crc32 = NET_SWAP_32( ext_gamepacket->crc32 );
}

// translate internal packet to external DEMO packet ( gameserver ) -----------
//
size_t NETs_HandleOutPacket_DEMO( const NetPacket* int_gamepacket, NetPacketExternal* ext_gamepacket )
{
	//FIXME: implement DEMO recording in GMSV mode
	ASSERT( FALSE );
	return 0;
}

// pack an internal packet to an external packet ( gameserver ) ---------------
//
size_t NETs_HandleOutPacket( const NetPacket* int_gamepacket, NetPacketExternal* ext_gamepacket, byte clientProtoMajor, byte clientProtoMinor )
{
	ASSERT( int_gamepacket != NULL );
	ASSERT( ext_gamepacket != NULL );
#ifdef PARSEC_CLIENT
	ASSERT( NET_ProtocolGMSV() );
#endif // PARSEC_CLIENT
	
	NetPacket_GMSV*			int_gamepacket_GMSV	= (NetPacket_GMSV*)			int_gamepacket;
	NetPacketExternal_GMSV*	ext_gamepacket_GMSV = (NetPacketExternal_GMSV*)	ext_gamepacket;
	
#ifdef PARSEC_CLIENT
	ASSERT( int_gamepacket_GMSV->SendPlayerId != PLAYERID_SERVER );
	ASSERT( int_gamepacket_GMSV->SendPlayerId != PLAYERID_MASTERSERVER );
#elif defined PARSEC_SERVER
	ASSERT( int_gamepacket_GMSV->SendPlayerId == PLAYERID_SERVER );
#elif defined PARSEC_MASTER
	ASSERT( int_gamepacket_GMSV->SendPlayerId == PLAYERID_MASTERSERVER );
#endif // PARSEC_MASTER

	// fill header for external packets
	strncpy( ext_gamepacket_GMSV->Signature, net_packet_signature_gameserver, SIGNATURE_LEN_GAMESERVER );
	ext_gamepacket_GMSV->Signature[ SIGNATURE_LEN_GAMESERVER ] = 0;
	ext_gamepacket_GMSV->Protocol				= PROTOCOL_GAMESERVER;
	ext_gamepacket_GMSV->MajorVersion			= clientProtoMajor;
	ext_gamepacket_GMSV->MinorVersion			= clientProtoMinor;
	
	// pack internal packet data to external packet
	ext_gamepacket_GMSV->SendPlayerId			= int_gamepacket_GMSV->SendPlayerId;

	ext_gamepacket_GMSV->MessageId				= int_gamepacket_GMSV->MessageId;
	ext_gamepacket_GMSV->ReliableMessageId		= int_gamepacket_GMSV->ReliableMessageId;
	ext_gamepacket_GMSV->AckMessageId			= int_gamepacket_GMSV->AckMessageId;
	ext_gamepacket_GMSV->AckReliableMessageId	= int_gamepacket_GMSV->AckReliableMessageId;

	ext_gamepacket_GMSV->Command				= (byte)int_gamepacket_GMSV->Command;


	//FIXME: [1/30/2002] remove this, when RE_Listsize is correctly maintained
#ifdef _SET_RE_LISTSIZE_ON_PACK
	int_gamepacket_GMSV->RE_ListSize = NET_RmEvList_GetSize( (RE_Header*)&int_gamepacket_GMSV->RE_List );
#endif // _SET_RE_LISTSIZE_ON_PACK
	
	// check that RE_ListSize corresponds with the actual length of the remote event list
	ASSERT( int_gamepacket_GMSV->RE_ListSize == NET_RmEvList_GetSize( (RE_Header*)&int_gamepacket_GMSV->RE_List ) );
	
	memcpy( (void*)&ext_gamepacket_GMSV->RE_List, (void*)&int_gamepacket_GMSV->RE_List, int_gamepacket_GMSV->RE_ListSize );
	
	// get the size of the packet
	size_t pktsize = GMSV_NetPacketExternal_GetSize( ext_gamepacket_GMSV );

	// establish network byte-order for external packet ( outgoing )
	GMSV_NetPacketExternal_Swap( ext_gamepacket_GMSV, FALSE );
	
	// store the CRC in the packet
	GMSV_NetPacketExternal_StoreCRC( ext_gamepacket_GMSV, pktsize );
	
#ifdef ENCRYPT_PACKETS

	// encrypt packet payload & modify protocol to indicate an encrypted packet
	if ( !( AUX_NETCODE_FLAGS & 2 ) ) {

		NET_EncryptData( &ext_gamepacket_GMSV->crc32, pktsize - offsetof( NetPacketExternal_GMSV, crc32 ) );

		// establish host byte order, modify field, and establish network order again
		ext_gamepacket_GMSV->Protocol = NET_SWAP_16( ext_gamepacket_GMSV->Protocol );
		ext_gamepacket_GMSV->Protocol |= PROTOCOL_ENCRYPTED; 
		ext_gamepacket_GMSV->Protocol = NET_SWAP_16( ext_gamepacket_GMSV->Protocol );
	}

#endif // ENCRYPT_PACKETS


#ifdef LOG_PACKETS
	g_PacketOutLogfile.printf( "%d", int_gamepacket_GMSV->MessageId );
#endif // LOG_PACKETS


	return pktsize;
}

// translate an external GMSV DEMO packet to internal GMSV packet -------------
// 
int NETs_HandleInPacket_DEMO( const NetPacketExternal* ext_gamepacket, NetPacket* int_gamepacket, size_t* psize_external )
{
	//NOTE: sets psize_external to the packetsize of the external packet 
	//      ( needed for playback from compiled demos )

	//FIXME: implement DEMO recording in GMSV mode
	ASSERT( FALSE );
	return 0;
}

// handle an incoming packet ( gameserver ) -------------------------------
//
int NETs_HandleInPacket( const NetPacketExternal* ext_gamepacket, const int ext_pktsize, NetPacket* int_gamepacket )
{
	ASSERT( int_gamepacket != NULL );
	ASSERT( ( ext_pktsize > 0 ) && ( ext_pktsize <= NET_MAX_DATA_LENGTH ) );
	ASSERT( ext_gamepacket != NULL );

#ifdef PARSEC_CLIENT
	ASSERT( NET_ProtocolGMSV() );
#endif // PARSEC_CLIENT
	
	// sanity check for packet len
	if ( ext_pktsize > NET_MAX_DATA_LENGTH ) {
		DBGTXT( MSGOUT( "NETs_HandleInPacket: dropped packet [invalid packet len]." ); );
		return FALSE;
	}
	
	NetPacket_GMSV*			int_gamepacket_GMSV	= (NetPacket_GMSV*)			int_gamepacket;
	NetPacketExternal_GMSV*	ext_gamepacket_GMSV = (NetPacketExternal_GMSV*)	ext_gamepacket;
	
	// establish host byte order
	ext_gamepacket_GMSV->Protocol = NET_SWAP_16( ext_gamepacket_GMSV->Protocol );
	
#ifdef ENCRYPT_PACKETS

	// check whether the packet supposedly is encrypted
	if ( ext_gamepacket_GMSV->Protocol & PROTOCOL_ENCRYPTED ) {

		// decrypt external packet
		if ( !( AUX_NETCODE_FLAGS & 2 ) ) {
			NET_DecryptData( &ext_gamepacket_GMSV->crc32, ext_pktsize - offsetof( NetPacketExternal_GMSV, crc32 ) );
		}
		
		// revert to decrypted protocol
		ext_gamepacket_GMSV->Protocol &= ~PROTOCOL_ENCRYPTED;
	}	

#endif // ENCRYPT_PACKETS

	// establish network byte order
	ext_gamepacket_GMSV->Protocol = NET_SWAP_16( ext_gamepacket_GMSV->Protocol );

	if ( !( AUX_NETCODE_FLAGS & 1 ) ) {
		
		// as the CRC has been calculated with the crc32 field stored in the packet beeing set to zero.
		// we need to do this here as well, backup CRC for compare
		dword crc32_stored = NET_SWAP_32( ext_gamepacket_GMSV->crc32 );
		ext_gamepacket_GMSV->crc32 = 0;

		// calculate CRC over packet
		dword crc32 = NET_CalcCRC( ext_gamepacket_GMSV, ext_pktsize );
		
		// check whether CRC is correct
		if ( crc32 != crc32_stored ) {
			DBGTXT( MSGOUT( "NETs_HandleInPacket(): dropped packet [CRC check failed]. calc.:%x stored:=%x", crc32, crc32_stored ); );
			return FALSE;
		}
	}

	// convert external packet from packet byte-order to host byte-order
	GMSV_NetPacketExternal_Swap( ext_gamepacket_GMSV, TRUE );

	// check for correct signature
	if ( strncmp( ext_gamepacket_GMSV->Signature, net_packet_signature_gameserver, SIGNATURE_LEN_GAMESERVER ) != 0 ) {
		DBGTXT( MSGOUT( "NETs_HandleInPacket(): dropped packet [invalid signature]." ); );
		return FALSE;
	}
	
	// check for correct protocol
	if ( ext_gamepacket_GMSV->Protocol != PROTOCOL_GAMESERVER ) {
		DBGTXT( MSGOUT( "NETs_HandleInPacket(): dropped packet [invalid protocol]." ); );
		return FALSE;
	}
	
	// check for correct protocol version
#ifndef PARSEC_SERVER
	if ( ( ext_gamepacket_GMSV->MajorVersion != CLSV_PROTOCOL_MAJOR ) || ( ext_gamepacket_GMSV->MinorVersion != CLSV_PROTOCOL_MINOR ) ) {
		DBGTXT( MSGOUT( "NETs_HandleInPacket(): dropped packet [incompatible protocol version]." ); );
		return FALSE;
	}
#else
	if ( ( ext_gamepacket_GMSV->MajorVersion != CLSV_PROTOCOL_MAJOR ) || ( ext_gamepacket_GMSV->MinorVersion != CLSV_PROTOCOL_MINOR ) ) {
			DBGTXT( MSGOUT( "NETs_HandleInPacket(): dropped packet [incompatible protocol version]." ); );
			return FALSE;
		}
#endif

	// unpack external packet data to internal packet
	int_gamepacket_GMSV->SendPlayerId			= ext_gamepacket_GMSV->SendPlayerId;

	int_gamepacket_GMSV->MessageId 				= ext_gamepacket_GMSV->MessageId;
	int_gamepacket_GMSV->ReliableMessageId 		= ext_gamepacket_GMSV->ReliableMessageId;
	int_gamepacket_GMSV->AckMessageId			= ext_gamepacket_GMSV->AckMessageId;
	int_gamepacket_GMSV->AckReliableMessageId	= ext_gamepacket_GMSV->AckReliableMessageId;

	int_gamepacket_GMSV->Command 				= (int)ext_gamepacket_GMSV->Command;
	//int_gamepacket_GMSV->params[ 0 ] 			= (int)ext_gamepacket_GMSV->param1;
	//int_gamepacket_GMSV->params[ 1 ] 			= (int)ext_gamepacket_GMSV->param2;
	//int_gamepacket_GMSV->params[ 2 ] 			= (int)ext_gamepacket_GMSV->param3;				
	//int_gamepacket_GMSV->params[ 3 ] 			= ext_gamepacket_GMSV->param4;

/*
#ifdef PARSEC_SERVER // disable this because the server will send the master server packets with this id.
	ASSERT( int_gamepacket_GMSV->SendPlayerId != PLAYERID_SERVER );
#endif // PARSEC_SERVER
*/
	// check whether external remote event list is well formed ( integrity and length )
	if ( NET_RmEvList_IsWellFormed( (RE_Header*)&ext_gamepacket_GMSV->RE_List ) == FALSE ) { 
		DBGTXT( MSGOUT( "NETs_HandleInPacket(): dropped packet [invalid RE list]." ); );
		return FALSE;
	}
	
	// determine remote event list size ( includes RE termination )
	int_gamepacket_GMSV->RE_ListSize	= NET_RmEvList_GetSize( (RE_Header*)&ext_gamepacket_GMSV->RE_List );
	memcpy( (void*)&int_gamepacket_GMSV->RE_List, (void*)&ext_gamepacket_GMSV->RE_List, int_gamepacket_GMSV->RE_ListSize );

#ifdef LOG_PACKETS
	g_PacketInLogfile.printf( "%d", int_gamepacket_GMSV->MessageId );
#endif // LOG_PACKETS
	
	return TRUE;
}


// fp to which PacketInfo functions write -------------------------------------
//
extern FILE *dest_fp_PacketInfo;


#ifndef PARSEC_MASTER

// write verbose packet info as comment to recording file ---------------------
//
void NETs_WritePacketInfo( FILE *fp, NetPacketExternal* ext_gamepacket )
{
	ASSERT( fp != NULL );
	ASSERT( ext_gamepacket != NULL );
	
	// store fp for PrInf_ functions
	dest_fp_PacketInfo = fp;
	
	// create packet copy and swap from network byte-order
	NetPacketExternal_GMSV* ext_gamepacket_copy = (NetPacketExternal_GMSV*) ALLOCMEM( NET_MAX_DATA_LENGTH );
	if ( ext_gamepacket_copy == NULL )
		return;
	
	memcpy( ext_gamepacket_copy, ext_gamepacket, NET_MAX_DATA_LENGTH );

	// swap to host byte order
	GMSV_NetPacketExternal_Swap( ext_gamepacket_copy, TRUE );
	
	NET_PacketInfo( ";-- srcid=%d, msgid=%d, relmsgid=%d, ackmsgid=%d, ackrelmsgid=%d\n",		
					ext_gamepacket_copy->SendPlayerId, 
					ext_gamepacket_copy->MessageId,
					ext_gamepacket_copy->ReliableMessageId,
					ext_gamepacket_copy->AckMessageId,
					ext_gamepacket_copy->AckReliableMessageId );

	NET_PacketInfo( ";-- command %d\n", ext_gamepacket_copy->Command );

	// process remote event list
	NET_RmEvList_WriteInfo( fp, (RE_Header *) &ext_gamepacket_copy->RE_List );
}

#endif // !PARSEC_MASTER
