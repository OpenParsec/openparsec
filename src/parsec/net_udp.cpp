/*
 * PARSEC - Platform independent UDP Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:40 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001-2002
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1998-2000
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
#include <errno.h>
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
#include "aud_defs.h"
#include "net_defs.h"
#include "sys_defs.h"

// network code config
#include "net_conf.h"

// subsystem linkage info
#include "linkinfo.h"

// local module header
#include "net_udpdf.h"
#include "net_udp.h"

// proprietary module headers
#include "con_aux.h"
#include "e_record.h"
#include "net_game.h"
#ifdef LINKED_PROTOCOL_GAMESERVER
	#include "net_game_gmsv.h"
#endif // LINKED_PROTOCOL_GAMESERVER
#include "net_stream.h"
#include "net_swap.h"
#include "net_wrap.h"
#include "net_util.h"

// client <-->server network defs for protocol version
#include "net_csdf.h"

// flags
#define ALLOW_BROADCASTS
//#define SAVE_PACKETSTREAM


// number of listen buffers in listen buffers array ---------------------------
//
#define NUM_LISTEN_BUFFERS		( MAX_NET_ALLOC_SLOTS * 2 )

// max. # of send retries - if this exceeds, we call Houston ------------------
//
#define MAX_SEND_RETRY_COUNT	10

// filename to store the local packet stream to -------------------------------
//
#define PACKET_STREAM_FILENAME		"pcktstrm_udp.bin"	




// flags if init successfully done --------------------------------------------
//
static int api_init_done		= FALSE;
static int netcode_init_done	= FALSE;


// packet stream saving -------------------------------------------------------
//
FILE*	pcktfp					= NULL;
int		start_timecode			= 0;


// string constants -----------------------------------------------------------
//
static const char node_address_str[]	= "IP address: %s";
static const char api_detected[]  	= "TCP/IP networking support found.";
static const char no_api_detected[]	= "TCP/IP networking support not found.";
static const char open_socket_error[] = "Error opening UDP socket.\n";
static const char net_game_avail[]	= "Network game will be available.\n";
static const char net_game_unavail[]	= "Network game not available.\n";
static const char pchain_alloc_error[]= "Error allocating packet chain.\n";
static const char queue_alloc_error[] = "Error allocating request queue.\n";


// send and listen buffers ----------------------------------------------------
//
static NetPacketExternal*	SendNetPacketExternal;
static NetPacketExternal*	RecvNetPacketExternal;
static int					ListenStatus		[ NUM_LISTEN_BUFFERS ];
static sockaddr_in			ListenAddress		[ NUM_LISTEN_BUFFERS ];
static node_t				ListenSenderStorage	[ NUM_LISTEN_BUFFERS ];
static NetPacket* 			NetPacketsInternal	[ NUM_LISTEN_BUFFERS ];


// stream message status ( only valid in GMSV ) -------------------------------
//
NET_Stream				ServerStream;

// client udp socket for both sending and receiving ---------------------------
//
int						udp_socket		= -1;


// server (destination) ports -------------------------------------------------
//
int						server_udp_port = DEFAULT_GAMESERVER_UDP_PORT;


// local ip address strings (host and broadcast in presentation format) -------
//
char					local_ip[ MAX_IPADDR_LEN + 1 ];
char					local_broadcast[ MAX_IPADDR_LEN + 1 ];


// high level packet chain (preprocessed to meet ordering requirements) -------
//
struct PacketChainBlock {

	int 				bufferno;
	NetPacket*			gamepacket;
	int 				messageid;
	PacketChainBlock*	nextblock;

	// sizeof( PacketChainBlock ) = 16
};

struct PacketChainHead : PacketChainBlock {
	int chainlength;
};

static PacketChainHead *ReceivedPacketsChain = NULL;


// packet loss meter ----------------------------------------------------------
//
#define PACKET_LOSS_METER_LENGTH	100						// horizontal size
static char packet_graph_recv[ PACKET_LOSS_METER_LENGTH ];
static char packet_graph_send[ PACKET_LOSS_METER_LENGTH ];


// function prototypes --------------------------------------------------------
//
int			NETs_CompareNodes( node_t *node1, node_t *node2 );
node_t*		NETs_GetSender( int bufid );


// resolve a hostname using the DNS service -----------------------------------
//
int NET_ResolveHostName( const char* hostname, char* ipaddress, node_t* node )
{
	ASSERT( hostname != NULL );

	if ( isdigit( hostname[ 0 ] ) ) {

		// hostname is assumed to be an ip address -> just convert to binary representation
		if ( node != NULL ) {
			inet_aton( hostname, (in_addr*)node );
		}
		if ( ipaddress != NULL ) {
			strncpy( ipaddress, hostname, MAX_IPADDR_LEN );
			ipaddress[ MAX_IPADDR_LEN ] = 0;
		}

		return TRUE;
	}

	hostent *hptr = gethostbyname( hostname );

	if ( hptr == NULL ) {
		MSGOUT( "NET_ResolveHostName(): DNS error: %s %s.", hostname, hstrerror( h_errno ) );
		return FALSE;
	}

	if ( hptr->h_addrtype == AF_INET ) {
		char **pptr = hptr->h_addr_list;
		if ( *pptr != NULL ) {
			// store node ?
			if ( node != NULL ) {
				memcpy( node, *pptr, IP_ADR_LENGTH );
			}
			// store numeric IP adress string
			if ( ipaddress != NULL ) {
				inet_ntop( hptr->h_addrtype, *pptr, ipaddress, MAX_IPADDR_LEN + 1 );
			}
			
			return TRUE;
		}
	} else {
		MSGOUT( "NET_ResolveHostName(): illegal address type in DNS entry." );
	}

	return FALSE;
}


// store port number into node address structure ------------------------------
//
void UDP_StoreNodePort( node_t *node, word port )
{
	ASSERT( node != NULL );

	node->address[ 4 ] = port >> 8;
	node->address[ 5 ] = port & 0xff;
}


// fetch port number from node address structure ------------------------------
//
word UDP_GetNodePort( node_t *node )
{
	ASSERT( node != NULL );

	return ( ( (word)node->address[ 4 ] << 8 ) | node->address[ 5 ] );
}

// allocate received packets chain (head node) --------------------------------
//
PRIVATE
int AllocPacketsChain()
{
	ASSERT( ReceivedPacketsChain == NULL );
	ReceivedPacketsChain = (PacketChainHead *) ALLOCMEM( sizeof( PacketChainHead ) );
	if ( ReceivedPacketsChain == NULL ) {
		if ( TextModeActive ) {
			fprintf( stderr, pchain_alloc_error );
			fprintf( stderr, net_game_unavail );
		} else {
			OUTOFMEM( 0 );
		}
		return FALSE;
	}
	ReceivedPacketsChain->nextblock   = NULL;
	ReceivedPacketsChain->chainlength = 0;

	return TRUE;
}


// free all blocks in received packets chain ----------------------------------
//
PRIVATE
void FreePacketsChain()
{
	ASSERT( ReceivedPacketsChain != NULL );
	while ( ReceivedPacketsChain != NULL ) {
		PacketChainBlock *temp = ReceivedPacketsChain->nextblock;
		FREEMEM( ReceivedPacketsChain );
		ReceivedPacketsChain = (PacketChainHead *) temp;
	}
}


// init networking code regardless of whether driver is available or not ------
//
PRIVATE
int InitNetCode()
{
	//NOTE:
	// these initializations even have to be done if
	// only simulated networking is available.

	if ( !netcode_init_done ) {

		// no other players yet
		NET_InitRemotePlayerTables();

		// init remote event list
		NET_RmEvListReset();

		// allocate received packets chain
		if ( !AllocPacketsChain() ) {
			return FALSE;
		}

		netcode_init_done = TRUE;
	}

	return TRUE;
}


// kill networking code regardless of whether network is available or not -----
//
PRIVATE
int KillNetCode()
{
	//NOTE:
	// these kills even have to be done if
	// only simulated networking is available.

	if ( netcode_init_done ) {
		netcode_init_done = FALSE;

		// free received packets chain
		FreePacketsChain();
	}

	return TRUE;
}


// log packet loss statistics -------------------------------------------------
//
PRIVATE
void LogPacketLossStats( int numpackets, int packetok, int incoming )
{
	// negative values might happen if we used
	// mismatched packet ids to calc numpackets
	if ( numpackets < 1 ) {
		return;
	}

	// guard against more lost packets than we can display
	if ( numpackets > PACKET_LOSS_METER_LENGTH ) {
		numpackets = PACKET_LOSS_METER_LENGTH;
	}

	// select receive/transmit graph
	char *graph = incoming ? packet_graph_recv : packet_graph_send;

	// scroll packet graph leftward
	int pos = 0;
	for ( pos = 0; pos < PACKET_LOSS_METER_LENGTH - numpackets; pos++ ) {
		graph[ pos ] = graph[ pos + numpackets ];
	}

	// graph value to display
	int dispval = packetok ? 0 : 1;

	// insert new values at the right
	for ( ; pos < PACKET_LOSS_METER_LENGTH; pos++ ) {
		graph[ pos ] = dispval;
	}
}


// write packet to stream file ------------------------------------------------
//
PRIVATE
void SaveLocalPacket( NetPacketExternal* pExtPkt )
{
	ASSERT( pExtPkt != NULL );
	
	if ( pcktfp == NULL ) {
		if ( ( pcktfp = fopen( PACKET_STREAM_FILENAME, "wb" ) ) == NULL ) {
			PANIC( 0 );
		}
	}
	
	// save timecode (little-endian)
	if ( start_timecode == 0 ) {
		start_timecode = SYSs_GetRefFrameCount();
	}
	
	refframe_t current_refframes = SWAP_32( SYSs_GetRefFrameCount() - start_timecode );
	fwrite( &current_refframes, 1, sizeof( int ), pcktfp );
	
	// save packet
	fwrite( pExtPkt, 1, NET_UDP_DATA_LENGTH, pcktfp );
}

// yield processing to network layer ------------------------------------------
//
int NETs_SleepUntilInput( int timeout_msec )
{
	//NOTE: identical to NET_UDPDriver::SleepUntilNetInput() in server codebase

	fd_set			rset;
	struct timeval	select_timeout;
	
	// initialize socket set
	FD_ZERO( &rset );
	FD_SET( udp_socket , &rset );
	
	select_timeout.tv_sec  = 0;
	select_timeout.tv_usec = timeout_msec * 1000;
	
	// select for readable socket
	int rc = select( udp_socket + 1, &rset, NULL, NULL, &select_timeout );

	return rc;
}


// retrieve received udp packet and store into listen buffer ------------------
//
int UDP_FetchPacket( int bufid )
{
	// assume buffer is busy (nothing received)
	ListenStatus[ bufid ] = TRUE;

	int			len	= sizeof( sockaddr_in );
	sockaddr_in from_adress;

	// receive packet on socket
	int rc  = recvfrom( udp_socket, (char *) RecvNetPacketExternal, NET_UDP_DATA_LENGTH, 0, (SA *)&from_adress, (socklen_t *) &len );

	if ( rc >= 0 ) {

		// copy from adress
		memcpy( &ListenAddress[ bufid ], &from_adress, sizeof( sockaddr_in ) );

		// filter broadcast packets that are looping back
		if ( NETs_CompareNodes( &LocalNode, NETs_GetSender( bufid ) ) == NODECMP_EQUAL ) {
			DBGTXT( MSGOUT( "NET_UDP::UDP_FetchPacket(): dropped packet [loop-back]." ); );
			// indicate we want to try to receive more packets
			return TRUE;
		} 

		size_t ext_pktsize = (size_t )rc;

		// convert external packet to internal format
		if ( !NETs_HandleInPacket( RecvNetPacketExternal, ext_pktsize, NetPacketsInternal[ bufid ] ) ) {
			DBGTXT( MSGOUT( "NET_UDP::UDP_FetchPacket(): dropped packet [conversion error]." ); );
			return FALSE;
		}

		// indicate that buffer now contains received packet
		ListenStatus[ bufid ] = FALSE;

	} else {

		//NOTE:
		// if any packets have been received the first one
		// is retrieved and stored into the listen buffer.
		// this succeeds until there are no packets anymore,
		// whereupon EWOULDBLOCK will be returned.

		//FIXME: move to seperate function

		FETCH_ERRNO();
		if ( !ERRNO_EWOULDBLOCK && !ERRNO_ECONNREFUSED ) {

			// a previous send operation resulted in an ICMP "Port Unreachable" message
			if( ERRNO_ECONNRESET ) {

#ifdef LINKED_PROTOCOL_GAMESERVER
				//NOTE: ICMP port unreachable can also be received for ping packets to other servers ( stargates )
				//      so we disable this here.
				/*if ( NET_ConnectedGMSV() ) {
					NET_DisconnectNoConnection();
					return FALSE;
				}*/
#endif // LINKED_PROTOCOL_GAMESERVER

			}

			MSGOUT( "error reading from udp socket: %d.", ERRNO );
			return FALSE;
		}
	} 

	return TRUE;
}

// open udp socket, set socket options, bind in peer-to-peer mode -------------
//
PRIVATE
int OpenClientSocket()
{
	ASSERT( udp_socket == -1 );

	// open udp socket
	udp_socket = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( udp_socket < 0 ) {
		fprintf( stderr, open_socket_error );
		return FALSE;
	}

#ifdef ALLOW_BROADCASTS

	// enable broadcasts for socket
	int brfl = TRUE;
	int err = setsockopt( udp_socket, SOL_SOCKET, SO_BROADCAST, (char *)&brfl, sizeof( brfl ) );
	if ( err < 0 ) {
		MSGOUT( "OpenClientSocket(): set socket option (broadcast) failed." );
		CLOSESOCKET( udp_socket );
		udp_socket = -1;
		return FALSE;
	}

#endif // ALLOW_BROADCASTS

	// make socket non-blocking
	int arg = TRUE;
	ioctl( udp_socket, FIONBIO, (char *)&arg );

	// bind port number for listening in peer-to-peer mode
	if ( NET_ProtocolPEER() ) {

		// bind socket to port
		sockaddr_in peeraddr;
		bzero( &peeraddr, sizeof( peeraddr ) );
		peeraddr.sin_family		 = AF_INET;
		peeraddr.sin_addr.s_addr = htonl( INADDR_ANY );
		peeraddr.sin_port		 = htons( server_udp_port );

		// set SO_REUSEADDR option, to avoid bind errors when re-binding
		int	on = 1;
		setsockopt( udp_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof( on ) );

		bind( udp_socket, (SA *) &peeraddr, sizeof( peeraddr ) );

	} 

	return TRUE;
}


// init interface code to udp functions ---------------------------------------
//
int UDP_InitInterface()
{
	if ( !api_init_done ) {

		NetAvailable = FALSE;

		// init tcp/ip stack
		if ( !UDPs_InitOSNetAPI() ) {
			return NetAvailable;
		}

		// from here on UDP_KillInterface() will need
		// to be invoked in order to terminate cleanly
		api_init_done = TRUE;

		// set ports to use
		if ( NET_ProtocolGMSV() ) {
			server_udp_port = DEFAULT_GAMESERVER_UDP_PORT;
			MSGOUT("OpenParsec Client Network Protocol Version: %i.%i", CLSV_PROTOCOL_MAJOR, CLSV_PROTOCOL_MINOR);
		} else {
			server_udp_port = DEFAULT_PEERTOPEER_UDP_PORT;
		}

		// open udp socket
		if ( !OpenClientSocket() ) {
			return NetAvailable;
		}

		CurMaxPlayers		= MAX_NET_UDP_PEER_PLAYERS;
		CurMaxDataLength	= NET_UDP_DATA_LENGTH;

		// allocate the external packets
		SendNetPacketExternal = (NetPacketExternal *) ALLOCMEM( NET_UDP_DATA_LENGTH );
		RecvNetPacketExternal = (NetPacketExternal *) ALLOCMEM( NET_UDP_DATA_LENGTH );
		memset( SendNetPacketExternal, 0, NET_UDP_DATA_LENGTH );
		memset( RecvNetPacketExternal, 0, NET_UDP_DATA_LENGTH );

		// allocate the internal packets
		for ( int bid = 0; bid < NUM_LISTEN_BUFFERS; bid++ ) {
			NetPacketsInternal[ bid ] = (NetPacket *) ALLOCMEM( NET_MAX_NETPACKET_INTERNAL_LEN );
			memset( NetPacketsInternal[ bid ],	0, NET_MAX_NETPACKET_INTERNAL_LEN );
			memset( &ListenAddress[ bid ], 0, sizeof( sockaddr_in ) );
		}

		// fetch local host address into global variable
		// (numeric: LocalNode, presentation: local_ip)
		if ( !UDPs_GetLocalIP() ) {
			MSGOUT( net_game_unavail );
			return NetAvailable;
		}

		// determine local broadcast address
		// (numeric: LocalBroadcast, presentation: local_broadcast)
		UDPs_GetLocalBroadcast();

		// print local host address
		MSGOUT( node_address_str, local_ip );

		NetAvailable = TRUE;
		MSGOUT( api_detected );
		MSGOUT( net_game_avail );
	}

	return NetAvailable;
}


// kill resources needed by udp code ------------------------------------------
//
int UDP_KillInterface()
{
	if ( api_init_done ) {
		api_init_done = FALSE;

		// close udp socket
		if ( udp_socket >= 0 ) {
			CLOSESOCKET( udp_socket );
			udp_socket = -1;
		}

		// close tcp/ip stack
		UDPs_KillOSNetAPI();

		// return if no further deinit necessary
		if ( !NetAvailable ) {
			return TRUE;
		}

		FREEMEM( SendNetPacketExternal );
		FREEMEM( RecvNetPacketExternal );
		SendNetPacketExternal = NULL;
		RecvNetPacketExternal = NULL;

		for ( int bid = 0; bid < NUM_LISTEN_BUFFERS; bid++ ) {
			FREEMEM( NetPacketsInternal[ bid ] );
			NetPacketsInternal[ bid ] = NULL;
		}

		return TRUE;
	}

	return FALSE;
}


// ----------------------------------------------------------------------------
// ABSTRACT NETWORKING SUBSYSTEM FUNCTIONS                                    -
// ----------------------------------------------------------------------------


// check for simulated network game -------------------------------------------
//
#define CHECK_SIMULATION() if ( NetConnected == NETWORK_GAME_SIMULATED ) return; else {}


// compare two node addresses (equal, less than, greater than) ----------------
//
int NETs_CompareNodes( node_t *node1, node_t *node2 )
{
	byte *ip1 = (byte *) node1;
	byte *ip2 = (byte *) node2;

	// compare as many digits as necessary
	for ( int pos = 0; pos < NODE_ADR_LENGTH; pos++ ) {

		if ( ip1[ pos ] < ip2[ pos ] ) {
			return NODECMP_LESSTHAN;
		} else if ( ip1[ pos ] > ip2[ pos ] ) {
			return NODECMP_GREATERTHAN;
		}
	}

	return NODECMP_EQUAL;
}


// determine if destination player is virtual ---------------------------------
//
int NETs_VirtualNode( node_t *node )
{
	byte *ip = (byte *) node;

	//NOTE:
	// ip address 0.0.0.0 is
	// used to denote virtual packets.
	// (packets coming from demo replay, not
	// from an actual network connection.)

	for ( int pos = 0; pos < NODE_ADR_LENGTH; pos++ ) {
		if ( ip[ pos ] != 0 ) {
			return FALSE;
		}
	}

	return TRUE;
}


// fill destination player address with virtual identifier --------------------
//
void NETs_SetVirtualAddress( node_t *node )
{
	byte *ip = (byte *) node;

	memset( ip, 0x00, NODE_ADR_LENGTH );
}


// fill destination player address with broadcast address ---------------------
//
void NETs_SetBroadcastAddress( node_t *node )
{
	byte *ip = (byte *) node;

	memcpy( ip, &LocalBroadcast, NODE_ADR_LENGTH );
}


// resolve node address -------------------------------------------------------
//
void NETs_ResolveNode( node_t* node_dst, node_t* node_src )
{
	// only copy operation for IP addresses
	memcpy( (byte*)node_dst, (byte*)node_src, MAX_NODE_ADDRESS_BYTES );
}


// retrieve raw node address from already resolved address --------------------
//
void NETs_MakeNodeRaw( node_t* node_dst, node_t* node_src )
{
	// only copy operation for IP addresses
	memcpy( (byte*) node_dst, (byte*) node_src, MAX_NODE_ADDRESS_BYTES );
}


// static unresolved virtual node address (must be set correctly!) ------------
//
static node_t virtual_node = { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 } };


// retrieve sender's node address from listen header --------------------------
//
node_t* NETs_GetSender( int bufid )
{
	ASSERT( ( NetConnected != NETWORK_GAME_SIMULATED ) ||
			( bufid == VIRTUAL_BUFFER_ID ) );

	ASSERT( bufid >= VIRTUAL_BUFFER_ID );
	ASSERT( bufid < NUM_LISTEN_BUFFERS );

	node_t *node;

	if ( bufid != VIRTUAL_BUFFER_ID ) {

		//NOTE:
		// instead of returning the address directly from ListenAdress[]
		// we make a copy here, since the port number will be appended and
		// we don't want to assume there is enough memory in the structure
		// after the sin_addr field. (usually there are eight irrelevant bytes)

		node = &ListenSenderStorage[ bufid ];

		memcpy( node, &ListenAddress[ bufid ].sin_addr, IP_ADR_LENGTH );
		UDP_StoreNodePort( node, ntohs( ListenAddress[ bufid ].sin_port ) );

	} else {

		node = &virtual_node;
		DBGTXT( MSGOUT( "NETs_GetSender(): sender identification for virtual packet." ); );
	}

	return node;
}


// resolve node address of sender of packet in specified buffer ---------------
//
void NETs_ResolveSender( node_t *node, int bufid )
{
	ASSERT( ( NetConnected != NETWORK_GAME_SIMULATED ) ||
			( bufid == VIRTUAL_BUFFER_ID ) );

	if ( bufid != VIRTUAL_BUFFER_ID ) {

		NETs_ResolveNode( node, NETs_GetSender( bufid ) );

	} else {

		DBGTXT( MSGOUT( "NETs_ResolveSender(): resolving virtual packet." ); );

		// set address to "virtual player"
		NETs_SetVirtualAddress( node );
	}
}


// output complete ip address (resolved) -------------------------------------
//
void NETs_PrintNode( node_t *node )
{
	char ipstring[ MAX_IPADDR_LEN + 1 ];
	inet_ntop( AF_INET, node, ipstring, MAX_IPADDR_LEN + 1 );

	MSGOUT( node_address_str, ipstring );
}


// flush all listen buffers ---------------------------------------------------
//
void NETs_FlushListenBuffers()
{
	// may be called from non-network code
	if ( !netcode_init_done ) {
		return;
	}

	//NOTE:
	// for ordinary network mode the packets chain would not need
	// to be flushed here, since it is always empty anyway (i.e.,
	// filled and emptied in direct succession). with virtual
	// packets, however, packets may be dangling, which particularly
	// causes problems during demo replay when replaying one demo
	// after another.

	// remove all blocks from packets chain
	// (apart from the head node)
	FreePacketsChain();
	AllocPacketsChain();

	// avoid accessing uninitialized structures
	if ( !NetAvailable ) {
		return;
	}

//TODO:

}


// record packet before inserting it into packet chain ------------------------
//
INLINE
void RecordPacket( int bufid )
{
#ifdef ENABLE_PACKET_RECORDING
	
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

}

// history buffer for received message ids to filter out duplicates -----------
//
#define MSGID_HISTORY_SIZE 64
static int message_id_history[ MAX_NET_ALLOC_SLOTS ][ MSGID_HISTORY_SIZE ];


// check regular player packets for duplicates --------------------------------
//
int FilterPacketDuplicate( int senderid, int messageid )
{
	ASSERT( senderid >= 0 );

	// init message id for last packet with sane value
	// if this is the first packet from this player
	if ( Player_LastMsgId[ senderid ] == 0 ) {
		//FIXME: cbx - 2002/02/28 - we must init message_id_history to 0 if 
		//       this is the FIRST packet not the SECOND. This caused a very
		//       subtle bug, preventing recorded demos from playing a second time
		//       in one session, mainly because PKTP_CONNECT packets are droppped 
		//       as they have a SendPlayerId of 0, which did not have a 
		//       cleared message_id_history at the first packet !
		if ( messageid >= 1 ) {
			int previd = messageid - 1;
			Player_LastMsgId[ senderid ] = previd;
			for ( int hid = 0; hid < MSGID_HISTORY_SIZE; hid++ ) {
				message_id_history[ senderid ][ hid ] = previd;
			}
		}
	}

	// discard duplicate packets remembered in history buffer
	int hid = 0;
	for ( hid = MSGID_HISTORY_SIZE - 1; hid >= 0; hid-- ) {
		if ( message_id_history[ senderid ][ hid ] == messageid ) {
			return TRUE;
		}
	}

	// update history buffer
	for ( hid = 1; hid < MSGID_HISTORY_SIZE; hid++ ) {
		message_id_history[ senderid ][ hid - 1 ] = message_id_history[ senderid ][ hid ];
	}
	message_id_history[ senderid ][ MSGID_HISTORY_SIZE - 1 ] = messageid;

	// calculate packet loss using message id comparisons
	int previd  = Player_LastMsgId[ senderid ];
	int numlost = messageid - ( previd + 1 );

	// log missing packets
	LogPacketLossStats( numlost, FALSE, TRUE );

	// got this packet
	LogPacketLossStats( 1, TRUE, TRUE );

	// ensure message ids are monotonically increasing
	Player_LastMsgId[ senderid ] = max( Player_LastMsgId[ senderid ], messageid );

	// take this packet
	return FALSE;
}


// insert received packet into chain ------------------------------------------
//
PRIVATE
int ChainPacket( NetPacket *gamepacket, int bufid )
{
	ASSERT( gamepacket				!= NULL );
	ASSERT( ReceivedPacketsChain	!= NULL );
	ASSERT( ( bufid == VIRTUAL_BUFFER_ID )  || ( gamepacket == NetPacketsInternal[ bufid ] ) );

	//FIXME: [1/24/2002] add virtual functions for gameserver/peer2peer
	PacketChainBlock* scan	= ReceivedPacketsChain;
	int senderid			= gamepacket->SendPlayerId;
	dword messageid			= gamepacket->MessageId;

	if ( messageid != MSGID_DATAGRAM ) {

		// distinguish per protocol
		if ( NET_ProtocolGMSV() ) {
			
			// sender must be server when in gameserver mode
			ASSERT( senderid == PLAYERID_SERVER );
			
			// check regular player packets for duplicates 
			// NOTE: playerid 0 is used here for storing the message id of packets from the server
			if ( FilterPacketDuplicate( 0, messageid ) ) {
				DBGTXT( MSGOUT( "ChainPacket(): filtering duplicate packet %d from %d", messageid, senderid ); );
				return FALSE;
			}
			
		} else if ( NET_ProtocolPEER() ) {

			// PLAYERID_ANONYMOUS only valid for PKTP_CONNECT/PKTP_SLOT_REQUEST/PKTP_SUBDUE_SLAVE packets
			ASSERT( ( senderid != PLAYERID_ANONYMOUS ) || 
					( gamepacket->Command == PKTP_CONNECT      ) || 
					( gamepacket->Command == PKTP_SLOT_REQUEST ) ||
					( gamepacket->Command == PKTP_SUBDUE_SLAVE )
				);
			
			// exclude non-regular packets (negative sender ids) from
			// duplicate checking: PLAYERID_SERVER, PLAYERID_ANONYMOUS
			if ( senderid >= 0 ) {
				
				// check regular player packets for duplicates
				if ( FilterPacketDuplicate( senderid, messageid ) ) {
					DBGTXT( MSGOUT( "ChainPacket(): filtering duplicate packet %d from %d", messageid, senderid ); );
					return FALSE;
				}
			}
			
		} else {
			// should never get here
			ASSERT( FALSE );
			return FALSE;
		}

		// search for packet of same sender already in chain
		for ( ; scan->nextblock; scan = scan->nextblock ) {
			if ( NETs_CompareNodes( NETs_GetSender( scan->nextblock->bufferno ), NETs_GetSender( bufid ) ) == NODECMP_EQUAL ) {
				break;
			}
		}

		// search to right insert position (according to message id)
		for ( ; scan->nextblock; scan = scan->nextblock ) {
			if ( NETs_CompareNodes( NETs_GetSender( scan->nextblock->bufferno ), NETs_GetSender( bufid ) ) != NODECMP_EQUAL ) {
				break;
			}
			if ( messageid <= (dword)scan->nextblock->messageid  ) {
				if ( messageid == (dword)scan->nextblock->messageid  ) {
					// NOTE: [1/24/2002] this should never happen, as we have already checked for duplicate messages
					DBGTXT( MSGOUT( "ChainPacket(): received same message twice: %d.", messageid ); );
				}
				break;
			}
		}

#ifdef LINKED_PROTOCOL_GAMESERVER

		// maintain stream IDs/ACKs 
		if ( NET_ProtocolGMSV() ) {	

			if ( !ServerStream.InPacket( (NetPacket_GMSV*)NetPacketsInternal[ bufid ]) ) {
				return FALSE;
			}

			// reset server connection timeout
			ServerTimeoutFrames = DEFAULT_TIMEOUT_SERVER_CONNECTION;
		}
	
#endif // LINKED_PROTOCOL_GAMESERVER

	}

	// beep when we get a packet
	if ( AUX_ENABLE_BEEP_ON_RECV ) {
		AUD_Select2();
	}

	// packet data processing
	if ( bufid != VIRTUAL_BUFFER_ID ) {
		RecordPacket( bufid );
	}

	// alloc block in chain
	PacketChainBlock *temp = (PacketChainBlock *) ALLOCMEM( sizeof( PacketChainBlock ) );
	if ( temp == NULL ) {
		OUTOFMEM( "PacketChainBlock" );
	}

	// store critical info
	temp->bufferno				= bufid;
	temp->gamepacket			= gamepacket;
	temp->messageid				= messageid;

	// link packet into chain
	temp->nextblock = scan->nextblock;
	scan->nextblock = temp;
	ReceivedPacketsChain->chainlength++;

	return TRUE;
}


// pre-process received packets -----------------------------------------------
//
INLINE
void BuildReceivedPacketsChain()
{
	//NOTE:
	// this function checks all packets received since
	// it was last called. packet signature and universe are
	// checked to determine validity. the packets have to be
	// sequenced according to their id, as they may be
	// in any order, and then inserted into the chain (this
	// is done by ChainPacket() ).

	ASSERT( ReceivedPacketsChain != NULL );

	CHECK_SIMULATION();

#ifdef ASSERT_PACKET_CHAIN_EMPTY
	ASSERT( ReceivedPacketsChain->nextblock == NULL );
	ASSERT( ReceivedPacketsChain->chainlength == 0 );
#endif

	// scan listen buffers
	for ( int bufid = 0; bufid < NUM_LISTEN_BUFFERS; bufid++ ) {

		// try to retrieve packet
		if ( !UDP_FetchPacket( bufid ) ) 
			break;

		// process received packet
		if ( !ListenStatus[ bufid ] ) {

			// insert packet into chain
			ChainPacket( NetPacketsInternal[ bufid ], bufid );
		}
	}
}


// retrieve next packet from received packets chain ---------------------------
//
INLINE
PacketChainBlock *FetchPacketFromChain()
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


// release block containing pointer and info of packet ------------------------
//
INLINE
void ReleasePacketFromChain( PacketChainBlock *block )
{
	ASSERT( ( NetConnected != NETWORK_GAME_SIMULATED ) ||
			( block->bufferno == VIRTUAL_BUFFER_ID ) );

	CHECKHEAPBASEREF( block );

	if ( block->bufferno == VIRTUAL_BUFFER_ID ) {

		//NOTE:
		// for virtual packets gamedata itself must also be freed,
		// since it has been allocated dynamically by 
		// E_RECORD::REC_PlayRemotePacketBin

		CHECKHEAPBASEREF( block->gamepacket );
		FREEMEM( block->gamepacket );
	}

	FREEMEM( block );
}


// invoke processing function for all packets currently in chain --------------
//
void NETs_ProcessPacketChain( void (*procfunc)( NetPacket *gamepacket,int bufid ) )
{
	// build chain of received packets
	BuildReceivedPacketsChain();
	
	// invoke processing function for received packets
	PacketChainBlock *block;
	while ( ( block = FetchPacketFromChain() ) != NULL ) {
		(*procfunc)( block->gamepacket, block->bufferno );
		ReleasePacketFromChain( block );
	}
}

// insert packet of virtual remote player into packets chain ------------------
//
int NETs_InsertVirtualPacket( NetPacket* gamepacket )
{
	ASSERT( gamepacket != NULL );
	
	DBGTXT( MSGOUT( "NETs_InsertVirtualPacket(): trying to insert virtual packet." ); );

	//FIXME: virtual functions for NET_UDP_PEER and NET_UDP_GMSV

	if ( NET_ProtocolPEER() ) { 

		NetPacket_PEER* gamepacket_PEER = (NetPacket_PEER*)gamepacket;

		//NOTE:
		// this function sets node addresses that may be
		// contained in the packet to the virtual node address.

		// check for RE_PLAYERLIST
		RE_PlayerList *re_playerlist = (RE_PlayerList *) &gamepacket_PEER->RE_List;
		if ( re_playerlist->RE_Type == RE_PLAYERLIST ) {
			DBGTXT( MSGOUT( "NETs_InsertVirtualPacket(): substituting addresses in RE_PLAYERLIST." ); );
			for ( int id = 0; id < MAX_NET_UDP_PEER_PLAYERS; id++ )
				re_playerlist->AddressTable[ id ] = virtual_node;
		}
		
		// check for second RE_PLAYERLIST
		if ( CurMaxPlayers > 4 ) {
			re_playerlist++;
			if ( re_playerlist->RE_Type == RE_PLAYERLIST ) {
				DBGTXT( MSGOUT( "NETs_InsertVirtualPacket(): substituting addresses in RE_PLAYERLIST." ); );
				for ( int id = 0; id < MAX_NET_UDP_PEER_PLAYERS; id++ )
					re_playerlist->AddressTable[ id ] = virtual_node;
			}
		}
		
		// check for RE_CONNECTQUEUE
		RE_ConnectQueue *re_connectqueue = (RE_ConnectQueue *) &gamepacket_PEER->RE_List;
		if ( re_connectqueue->RE_Type == RE_CONNECTQUEUE ) {
			DBGTXT( MSGOUT( "NETs_InsertVirtualPacket(): substituting addresses in RE_CONNECTQUEUE." ); );
			short numreqs = NET_SWAP_16( re_connectqueue->NumRequests );
			ASSERT( ( numreqs >= 0 ) && ( numreqs <= MAX_SLOT_REQUESTS ) );
			for ( int id = 0; id < numreqs; id++ )
				re_connectqueue->AddressTable[ id ] = virtual_node;
		}
		
		//NOTE:
		// the packet's signature may have been stripped away before recording
		// to file. ChainPacket() doesn't check the packet signature, though.

		//NOTE:
		// the new peerto-peer protocol ALWAYS includes the header of a packet
	} else {

		//TODO: implement recording for GMSV mode 
		ASSERT( FALSE );
	}

	
	return ChainPacket( gamepacket, VIRTUAL_BUFFER_ID );
}

// acquire packet loss statistics ---------------------------------------------
//
int NETs_DeterminePacketLoss( char **info, int *isiz, int incoming )
{
	ASSERT( info != NULL );
	ASSERT( isiz != NULL );
	
	*info = incoming ? packet_graph_recv : packet_graph_send;
	*isiz = PACKET_LOSS_METER_LENGTH;
	
	return TRUE;
}


// determine if packet should be artificially dropped -------------------------
//
INLINE
int ForcePacketDrop()
{
	
#ifdef ENABLE_PACKETDROP_TESTING
	
	//NOTE:
	// AUX_PACKETDROP_TESTING sets the probability
	// of packet-drop (well, approximately).
	
	if ( AUX_PACKETDROP_TESTING ) {
		
		int r = RAND() % 100;
		if ( r < AUX_PACKETDROP_TESTING ) {
			return TRUE;
		}
	}
	
#endif // ENABLE_PACKETDROP_TESTING
	
	return FALSE;
}

// low-level packet sending function -------------------------------------------
// 
void _SendPacket( NetPacket* gamepacket, node_t* node, int include_REList, int enforce_packet_sent )
{
	CHECK_SIMULATION();
	
	// don't send packets addressed to virtual players
	if ( NETs_VirtualNode( node ) )
		return;
	
	// check for artificial packet drop
	if ( ForcePacketDrop() ) {
		LogPacketLossStats( 1, FALSE, FALSE );
		return;
	}
	
	// copy remote event-list into packet
	if ( include_REList ) {
		if ( NET_ProtocolPEER() ) {
			NetPacket_PEER* gamepacket_PEER = (NetPacket_PEER*)gamepacket;
			//ASSERT( ( (RE_Header*)&gamepacket_PEER->RE_List )->RE_Type == RE_EMPTY );
			memcpy( &gamepacket_PEER->RE_List, REListMem, RE_LIST_MAXAVAIL + sizeof( gamepacket_PEER->RE_List ) );
		} else {
			ASSERT( NET_ProtocolGMSV() );
			NetPacket_GMSV* gamepacket_GMSV = (NetPacket_GMSV*)gamepacket;
			//ASSERT( ( (RE_Header*)&gamepacket_GMSV->RE_List )->RE_Type == RE_EMPTY );
			memcpy( &gamepacket_GMSV->RE_List, REListMem, RE_LIST_MAXAVAIL + sizeof( gamepacket_GMSV->RE_List ) );
		}
	}

	// pack from internal packet to external packet
	size_t pktsize = NETs_HandleOutPacket( gamepacket, SendNetPacketExternal );

	// write local packet to stream - for testing purposes only
#ifdef SAVE_PACKETSTREAM
	SaveLocalPacket( SendNetPacketExternal );
#endif // SAVE_PACKETSTREAM
	
	sockaddr_in SendAddress;
	bzero( &SendAddress, sizeof( SendAddress ) );
	SendAddress.sin_family = AF_INET;
	SendAddress.sin_port = htons( UDP_GetNodePort( node ) );
	memcpy ( &SendAddress.sin_addr, node, IP_ADR_LENGTH );

	int rc = -1;
	int retrycount = 0;
	for( retrycount = 0; retrycount < MAX_SEND_RETRY_COUNT ;retrycount++ ) { 

		rc = sendto( udp_socket, (char *) SendNetPacketExternal, pktsize,             0, (SA *)&SendAddress, sizeof( SendAddress ) );
		
		//NOTE:
		// if the packet could not be sent (EWOULDBLOCK),
		
		// enforce_packet_sent == TRUE
		// the sendto() call is retried until it succeeds.
		// this is done in order to ensure that no packets
		// are thrown away even before they have been sent.
		
		// enforce_packet_sent == FALSE
		// it is simply thrown away. this is the case when
		// there is currently not enough bandwidth for the
		// packet send-frequency.
		
		if ( rc < 0 ) {
			
			FETCH_ERRNO();
			if ( !ERRNO_EWOULDBLOCK && !ERRNO_ECONNREFUSED && !ERRNO_ENETDOWN ) {
				MSGOUT( "_SendPacket(): error writing to udp socket: %d.", ERRNO );
			} else if ( ERRNO_ECONNREFUSED || ERRNO_ENETDOWN ) {
				MSGOUT( "_SendPacket(): host is unreachable: %d.", ERRNO );
			} else {
				
				ASSERT( ERRNO_EWOULDBLOCK );
				
				if ( enforce_packet_sent ) {
					continue;
				}
			}

		// check whether the complete packet was sent
		} else if( (size_t)rc < pktsize ) {
			ASSERT( FALSE );
			MSGOUT( "_SendPacket(): packet truncated ( %d of %d sent ).", rc, pktsize );
		}

		break;
	}

	// we have a serious problem, if we exceed the send retry count ( socket buffer is choked )
	if ( retrycount == MAX_SEND_RETRY_COUNT ) {
		FETCH_ERRNO();
		MSGOUT( "_SendPacket(): seriuos error writing to udp socket: %d.", ERRNO );
	}

	// log packet loss
	LogPacketLossStats( 1, ( rc >= 0 ), FALSE );
}


// send udp packet after inserting global remote-event list -------------------
//
void NETs_SendPacket( NetPacket* gamepacket, node_t *node )
{
	// remote event list is included here and packet is discarded if sendto fails with E_WOULDBLOCK
	_SendPacket( gamepacket, node, TRUE, FALSE );
}


// send udp packet ------------------------------------------------------------
//
void NETs_AuxSendPacket( NetPacket* gamepacket, node_t *node )
{
	// remote event list is NOT included here and packet is retried if sendto fails with E_WOULDBLOCK
	_SendPacket( gamepacket, node, FALSE, TRUE );
}


// re-init networking api -----------------------------------------------------
//
int NETs_ResetAPI()
{
	if ( api_init_done ) {

		// ensure disconnected
		NET_ForceDisconnect();

		// close udp socket
		if ( udp_socket >= 0 ) {
			CLOSESOCKET( udp_socket );
			udp_socket = -1;
		}

		// reopen udp socket
		if ( !OpenClientSocket() ) {
			return FALSE;
		}
	}

	return TRUE;
}


// init networking api --------------------------------------------------------
//
int NETs_InitAPI()
{
	// init networking code
	if ( !InitNetCode() ) {
		return FALSE;
	}

	// init UDP functions
	return UDP_InitInterface();
}


// shutdown networking api ----------------------------------------------------
//
int NETs_KillAPI()
{
	// ensure disconnected
	NET_ForceDisconnect();

	// kill networking code
	if ( !KillNetCode() ) {
		return FALSE;
	}

	// deinit UDP functions
	return UDP_KillInterface();
}



