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

#ifndef SYSTEM_TARGET_WINDOWS

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

// local module header
#include "net_udpdf.h"
#include "nl_udp.h"

// proprietary module headers
#include "con_aux.h"
#include "net_game.h"
#include "net_wrap.h"


// flags
#define USE_DIRECTED_BROADCASTS



// string constants -----------------------------------------------------------
//
//static char net_game_unavail[]	= "Network game not available.\n";
//static char no_api_detected[]	= "TCP/IP networking support not found.";
//static char winsock_init_ok[]	= "WinSock version %d.%d initialized.";
//static char winsock_init_error[]= "WinSock %d.%d could not be initialized.\n";

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
	int sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( sockfd < 0 ) {
		MSGOUT( "UDPs_GetLocalIP(): can't open socket." );
		return FALSE;
	}

	int   infolen = sizeof( struct ifreq ) * 10;
	char* infobuf = NULL;
	struct ifconf ifc;

	// get all interface configurations into buffer
	int lastlen = 0;
	for ( ;; ) {

		infobuf = (char *) ALLOCMEM( infolen );
		if ( infobuf == NULL )
			OUTOFMEM( 0 );

		ifc.ifc_len = infolen;
		ifc.ifc_buf = infobuf;

		if ( ioctl( sockfd, SIOCGIFCONF, &ifc ) < 0 ) {
			if ( ( errno != EINVAL ) || ( lastlen != 0 ) ) {
				MSGOUT( "GetLocalIP(): ioctl error." );
				FREEMEM( infobuf );
				CLOSESOCKET( sockfd );
				return FALSE;
			}
		} else {
			if ( ifc.ifc_len == lastlen )
				break;
			lastlen = ifc.ifc_len;
		}

		infolen += sizeof( struct ifreq ) * 10;
		FREEMEM( infobuf );
	}

	int ifnum = 0;

	// analyze all interface configurations in buffer
	for ( char *ptr = infobuf; ptr < infobuf + ifc.ifc_len; ) {

		struct ifreq* ifr = (struct ifreq *) ptr;

		// advance to next structure in buffer
		
#ifdef SYSTEM_MACOSX		
		int len = max( sizeof(struct sockaddr), ifr->ifr_addr.sa_len );
#else
		int len = sizeof(struct sockaddr);
#endif
		
		ptr += sizeof( ifr->ifr_name ) + len;

		// only handle IPV4 entries
		if ( ifr->ifr_addr.sa_family != AF_INET )
			continue;

		char if_addr[ MAX_IPADDR_LEN + 1 ];
		sockaddr_in	*sa = (struct sockaddr_in *) &ifr->ifr_addr;
		inet_ntop( AF_INET, &sa->sin_addr, if_addr, MAX_IPADDR_LEN + 1 );

		if ( strcmp( "127.0.0.1", if_addr ) == 0 ) {
			DBGTXT( MSGOUT( "skipping loopback interface" ); );
			continue;
		}

		DBGTXT( MSGOUT( "found interface #%d: %s", ifnum, if_addr ); );

		// is this our interface, then take the ip address
		if ( ifnum == NetInterfaceSelect ) {

			// store globally: numeric
			memcpy( &LocalNode, &sa->sin_addr, IP_ADR_LENGTH );

			// store globally: presentation
			inet_ntop( AF_INET, &LocalNode, local_ip, MAX_IPADDR_LEN + 1 );
			DBGTXT( MSGOUT( "taking ip addr %s", local_ip ); );

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
		}

		ifnum++;
	}

	FREEMEM( infobuf );
	CLOSESOCKET( sockfd );

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
	int sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( sockfd < 0 ) {
		MSGOUT( "GetLocalBroadcast(): can't open socket." );
		return;
	}

	int   infolen = sizeof( struct ifreq ) * 10;
	char* infobuf = NULL;
	struct ifconf ifc;

	// get all interface configurations into buffer
	int lastlen = 0;
	for ( ;; ) {

		infobuf = (char *) ALLOCMEM( infolen );
		if ( infobuf == NULL )
			OUTOFMEM( 0 );

		ifc.ifc_len = infolen;
		ifc.ifc_buf = infobuf;

		if ( ioctl( sockfd, SIOCGIFCONF, &ifc ) < 0 ) {
			if ( ( errno != EINVAL ) || ( lastlen != 0 ) ) {
				MSGOUT( "GetLocalBroadcast(): ioctl error: %s.", strerror( errno ) );
				FREEMEM( infobuf );
				CLOSESOCKET( sockfd );
				return;
			}
		} else {
			if ( ifc.ifc_len == lastlen )
				break;
			lastlen = ifc.ifc_len;
		}

		infolen += sizeof( struct ifreq ) * 10;
		FREEMEM( infobuf );
	}

	// analyze all interface configurations in buffer
	for ( char *ptr = infobuf; ptr < infobuf + ifc.ifc_len; ) {

		struct ifreq* ifr = (struct ifreq *) ptr;

		int len = sizeof( struct sockaddr );
		
		switch( ifr->ifr_addr.sa_family ) {
			case AF_INET6:
				len = sizeof( struct sockaddr_in6 );
				break;

			case AF_INET:
				len = sizeof( struct sockaddr );
				break;
		}

		// advance to next structure in buffer
		ptr += sizeof( ifr->ifr_name ) + len;
		
		// only handle IPV4 entries
		if ( ifr->ifr_addr.sa_family != AF_INET )
			continue;
		
		char if_addr[ MAX_IPADDR_LEN + 1 ];
		sockaddr_in	*sa = (struct sockaddr_in *) &ifr->ifr_addr;
		inet_ntop( AF_INET, &sa->sin_addr, if_addr, MAX_IPADDR_LEN + 1 );
		
		DBGTXT( MSGOUT( "found interface %s", if_addr ); );
		
		// is this our interface, then take the broadcast address
		if ( strcmp( local_ip, if_addr ) == 0 ) {
			
			if ( ioctl( sockfd, SIOCGIFBRDADDR, ifr ) < 0 ) {
				MSGOUT( "GetLocalBroadcast(): ioctl error: %s.", strerror( errno ) );
				FREEMEM( infobuf );
				CLOSESOCKET( sockfd );
				return;
			}
			
			// store globally: numeric
			sa = (struct sockaddr_in *) &ifr->ifr_broadaddr;
			memcpy( &LocalBroadcast, &sa->sin_addr, IP_ADR_LENGTH );
			
			// store globally: presentation
			inet_ntop( AF_INET, &LocalBroadcast, local_broadcast, MAX_IPADDR_LEN + 1 );
			DBGTXT( MSGOUT( "taking broadcast addr %s", local_broadcast ); );
		}
	}
	
	FREEMEM( infobuf );
	CLOSESOCKET( sockfd );
	
#endif // USE_DIRECTED_BROADCASTS
	
}

// init tcp/ip stack ----------------------------------------------------------
//
int UDPs_InitOSNetAPI()
{
	// empty in Linux
	return TRUE;
}

// close tcp/ip stack ---------------------------------------------------------
//
void UDPs_KillOSNetAPI()
{
	// empty in Linux
}


#endif // !SYSTEM_TARGET_WINDOWS


