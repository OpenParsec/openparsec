/*
 * PARSEC - Platform dependent UDP Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:31 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001
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
// compilation flags/debug support
#include "config.h"

#ifdef _WIN32

// C library
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "net_defs.h"
#include "sys_defs.h"

// network code config
#include "net_conf.h"
#define NEED_WINSOCK2

// local module header
#include "net_udpdf.h"
#include "nw_udp.h"

// proprietary module headers
#include "con_aux.h"
#include "net_game.h"
#include "net_wrap.h"


// flags
#define USE_DIRECTED_BROADCASTS

// required winsock version ---------------------------------------------------
//
#ifdef NEED_WINSOCK2
	#define WINSOCK_MAJOR		2
	#define WINSOCK_MINOR		0
#else
	#define WINSOCK_MAJOR		1
	#define WINSOCK_MINOR		1
#endif


// string constants -----------------------------------------------------------
//
static char net_game_unavail[]	= "Network game not available.\n";
static char no_api_detected[]	= "TCP/IP networking support not found.";
static char winsock_init_ok[]	= "WinSock version %d.%d initialized.";
static char winsock_init_error[]= "WinSock %d.%d could not be initialized.\n";

// server (destination) ports -------------------------------------------------
//
extern int				server_udp_port;		// defined in NET_UDP

// local ip address strings (host and broadcast in presentation format) -------
//
extern char				local_ip[ MAX_IPADDR_LEN + 1 ];
extern char				local_broadcast[ MAX_IPADDR_LEN + 1 ];

// determine local ip address -------------------------------------------------
//
int UDPs_GetLocalIP()
{
	// get dns name of local host
	char hostname[ MAXHOSTNAMELEN ];
	if ( gethostname( hostname, MAXHOSTNAMELEN ) == -1 ) {
		MSGOUT( "cannot get local host name." );
		return FALSE;
	}

	// resolve host name to hostent info
	hostent *hptr = gethostbyname( hostname );
	if ( hptr == NULL ) {
		MSGOUT( "DNS error: %s %s.", hostname, hstrerror( h_errno ) );
		return FALSE;
	}
	if ( hptr->h_addrtype != AF_INET ) {
		MSGOUT( "cannot resolve local hostname: illegal address type in DNS entry." );
		return FALSE;
	}

	// fetch list of ip addresses for local host
	char **addrlist = hptr->h_addr_list;
	if ( addrlist[ 0 ] == NULL ) {
		return FALSE;
	}

	// take either first interface or user-selected interface if valid
	int addrid = 0;
	for ( int aid = 1; addrlist[ aid ] != NULL; aid++ ) {
		if ( aid == NetInterfaceSelect ) {
			addrid = aid;
			break;
		}
	}

	// store globally: numeric
	memcpy( &LocalNode, addrlist[ addrid ], IP_ADR_LENGTH );

	// store globally: presentation
	inet_ntop( AF_INET, &LocalNode, local_ip, MAX_IPADDR_LEN + 1 );

	//NOTE:
	// the port number appended to the local ip address is only valid
	// in the peer-to-peer protocol, since it is the only protocol
	// using a well-known port number. the game server and slot server
	// protocols use an ephemeral port. for other protocols than
	// peer-to-peer this doesn't matter, however, since it need only
	// be valid for node address comparisons in order to filter
	// broadcast loop-backs. still, for those protocols, it MUST not
	// be the server port, or else we would filter all packets from a
	// server running on the same machine as the client. therefore, we
	// simply use a port number of zero in this case.

	if ( NET_ProtocolPEER() ) {

		// append port number to ip address
		UDP_StoreNodePort( &LocalNode, server_udp_port );

	} else {

		// append zero port number to ip address
		UDP_StoreNodePort( &LocalNode, 0 );
	}

	return TRUE;
}


// determine broadcast address for local subnet -------------------------------
//
void UDPs_GetLocalBroadcast()
{
	// use global broadcast
	memset( &LocalBroadcast, 0xff, IP_ADR_LENGTH );
	inet_ntop( AF_INET, &LocalBroadcast, local_broadcast, MAX_IPADDR_LEN + 1 );

	// append port number to ip address
	UDP_StoreNodePort( &LocalBroadcast, server_udp_port );

#ifdef USE_DIRECTED_BROADCASTS

	//NOTE:
	// in case of error we take the global broadcast address as a
	// fallback. this might not work everywhere, though.

	// determine real subnet-directed broadcast address
	SOCKET sockfd = WSASocket( AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, 0 );
	if ( sockfd == INVALID_SOCKET ) {
		MSGOUT( "GetLocalBroadcast(): can't open socket." );
   		return;
   	}

	DWORD infolen;
	INTERFACE_INFO infobuff[ 10 ];
	int wsaerr = WSAIoctl( sockfd, SIO_GET_INTERFACE_LIST, NULL, 0, &infobuff,
						   sizeof( infobuff ), &infolen, NULL, NULL );
	if ( wsaerr == SOCKET_ERROR ) {
		MSGOUT( "GetLocalBroadcast(): ioctl error." );
		return;
	}

	CLOSESOCKET( sockfd );

	// scan all interfaces
	int numinfos = infolen / sizeof( INTERFACE_INFO );
	for ( int infoid = 0; infoid < numinfos; infoid++ ) {

		char ifaddr[ MAX_IPADDR_LEN + 1 ];
		SOCKADDR_IN	*sa = (SOCKADDR_IN *) &infobuff[ infoid ].iiAddress;
		inet_ntop( AF_INET, &sa->sin_addr, ifaddr, MAX_IPADDR_LEN + 1 );

		DBGTXT( MSGOUT( "found interface %s", ifaddr ); );

		// fetch broadcast address for interface we are using
		if ( strcmp( local_ip, ifaddr ) == 0 ) {

//			sa = (SOCKADDR_IN *) &infobuff[ infoid ].iiBroadcastAddress;
			sa = (SOCKADDR_IN *) &infobuff[ infoid ].iiNetmask;

			// store globally: numeric
			memcpy( &LocalBroadcast, &sa->sin_addr, IP_ADR_LENGTH );

			byte *localcast = (byte *) &LocalBroadcast;
			byte *localnode = (byte *) &LocalNode;
			for ( int pos = 0; pos < IP_ADR_LENGTH; pos++ ) {
				if ( localcast[ pos ] == 255 ) {
					localcast[ pos ] = localnode[ pos ];
				} else {
					localcast[ pos ] = 255;
				}
			}

			// store globally: presentation
			inet_ntop( AF_INET, &LocalBroadcast, local_broadcast, MAX_IPADDR_LEN + 1 );
			DBGTXT( MSGOUT( "taking broadcast addr %s", local_broadcast ); );
		}
	}

#endif // USE_DIRECTED_BROADCASTS

}

// init tcp/ip stack (winsock) ------------------------------------------------
//
int UDPs_InitOSNetAPI()
{
	MSGOUT( "Initializing TCP/IP stack..." );

	WSADATA wsaData;
	word wVersionRequested = MAKE_WORD( WINSOCK_MAJOR, WINSOCK_MINOR );

	// initialize winsock
	if ( WSAStartup( wVersionRequested, &wsaData ) == SOCKET_ERROR ) {
		fprintf( stderr, winsock_init_error, WINSOCK_MAJOR, WINSOCK_MINOR );
		MSGOUT( no_api_detected );
		MSGOUT( net_game_unavail );
		return FALSE;
	}

	// print information on the version of WinSock found
	MSGOUT( winsock_init_ok,
			LO_BYTE( wsaData.wHighVersion ), HI_BYTE( wsaData.wHighVersion ) );
	MSGOUT( wsaData.szDescription );

	return TRUE;
}


// close tcp/ip stack (winsock) -----------------------------------------------
//
void UDPs_KillOSNetAPI()
{
	// winsock cleanup
	WSACleanup();
}


#endif // _WIN32
