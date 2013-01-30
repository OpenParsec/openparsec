/*
 * PARSEC - UDP driver
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:26:06 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// network subsystem & server headers 
#include "net_defs.h"
#include "e_defs.h"

// UNP header
#include "net_wrap.h"

// local module header
#include "net_udpdriver.h"

// proprietary module headers
#include "net_util.h"

#include "e_gameserver.h"
#include "e_global_sv.h"

#include "con_arg.h"
#include "con_main_sv.h"
#include "con_com_sv.h"
#include "con_aux_sv.h"


// string constants -----------------------------------------------------------
//
static const char node_address_str[]			= "IP address: %s:%d";
static const char api_detected[]  			= "TCP/IP networking support found.";
static const char no_api_detected[]			= "ERROR: TCP/IP networking support not found.";
static const char open_socket_error[]			= "ERROR: opening UDP socket.\n";
static const char pchain_alloc_error[]		= "ERROR: allocating packet chain.\n";
static const char queue_alloc_error[]			= "ERROR: allocating request queue.\n";
static const char cannot_get_local_hostname[] = "ERROR: cannot get local host name\n";
static const char dns_error[]					= "ERROR: DNS error: %s %s.\n";
static const char illegal_adress_type_dns[]   = "ERROR: cannot resolve local hostname: illegal address type in DNS entry.\n";
static const char no_network_interface_found[]= "ERROR: no network interface found.";
static const char selected_interface_invalid[]= "WARNING: selected interface not found - using default interface.";


// ----------------------------------------------------------------------------
// public methods
// ----------------------------------------------------------------------------

// default ctor ---------------------------------------------------------------
//
NET_UDPDriver::NET_UDPDriver()
	: m_socket( -1 ),
	  m_selected_port( 0 )
{
	m_PreferredInterface = 0;
	m_DriverRunning = 0;
	memset( m_szIP,			0, MAX_IPADDR_LEN + 1 );
	memset( m_szInterface,	0, MAX_IPADDR_LEN + 1 );
}

// default dtor ---------------------------------------------------------------
//
NET_UDPDriver::~NET_UDPDriver()
{
	KillDriver();
}


// init the UDP driver --------------------------------------------------------
//
int	NET_UDPDriver::InitDriver( char* szInterface, int selected_port )
{
	m_selected_port = selected_port;
	
	// initialize WINSOCK
	if ( _InitOSAPI() == FALSE ) {
		return FALSE;
	}
	
	// get the local IP corresponding to the (user)configured interface
	if ( _RetrieveLocalIP() == FALSE ) {
		return FALSE;
	}
	
	// open the UDP server socket
	if ( _OpenSocket() == FALSE ) {
		return FALSE;
	}
	
	m_DriverRunning = 1;
	// print local host address
	MSGOUT( node_address_str, m_szIP, m_selected_port );
	
	return TRUE;
}

// kill the UDP driver --------------------------------------------------------
//
int NET_UDPDriver::KillDriver()
{
	if ( _CloseSocket() == FALSE ) {
		return FALSE;
	}
	
	// kill WINSOCK
	if ( _KillOSAPI() == FALSE ) {
		return FALSE;
	}
	m_DriverRunning = 0;
	
	return TRUE;
}

// fetch a UDP packet from the socket ----------------------------------------
//
int NET_UDPDriver::FetchPacket( char* buf, int maxlen, SA* saddr )
{
	int len = sizeof( sockaddr_in );
	int rc  = recvfrom( m_socket, buf, maxlen, 0, saddr, (socklen_t *) &len );
	
	if ( rc < 0 ) {
		
		FETCH_ERRNO();
		if ( !ERRNO_EWOULDBLOCK && !ERRNO_ECONNREFUSED && !ERRNO_ECONNRESET ) {
			MSGOUT( "NET_UDPDriver::FetchPacket: error reading from UDP socket: %d.", ERRNO );
		}
	} 
	
	return rc;
}

#define UDP_MAX_SEND_RETRIES 10

// send a UDP packet on the socket --------------------------------------------
//
int NET_UDPDriver::SendPacket( char* buf, int pktsize, SA* saddr )
{
	ASSERT( buf   != NULL );
	ASSERT( pktsize >= 0  );
	ASSERT( saddr != NULL );

	int rc = 0;

	for( int nRetry = 0; nRetry < UDP_MAX_SEND_RETRIES; nRetry++ ) {
		rc = sendto( m_socket, buf, pktsize, 0, saddr, sizeof( sockaddr_in ) );

		//FIXME: do we really need to retry sendto if EWOULDBLOCK ??

		// check for succesful delivery of packet
		if ( rc >= 0 )
			return rc;
		
		//NOTE:
		// if the packet could not be sent (EWOULDBLOCK),
		// the sendto() call is retried until it succeeds.
		// this is done in order to ensure that no packets
		// are thrown away even before they have been sent.
		
		FETCH_ERRNO();
		if ( !ERRNO_EWOULDBLOCK && !ERRNO_ECONNREFUSED && !ERRNO_ENETDOWN ) {
			MSGOUT( "NET_UDPDriver::SendPacket: error writing to UDP socket: %d.", ERRNO );
			return rc;
		} else if ( ERRNO_ECONNREFUSED || ERRNO_ENETDOWN ) {
			MSGOUT( "NET_UDPDriver::SendPacket: host is unreachable: %d.", ERRNO );
			return rc;
		} else {
			ASSERT( ERRNO_EWOULDBLOCK );
			MSGOUT( "NET_UDPDriver::SendPacket: send choke: %d.", ERRNO );
		}
	}

	// print timeout message
	FETCH_ERRNO();
	MSGOUT( "NET_UDPDriver::SendPacket: timeout writing to UDP socket.", ERRNO );

	return rc;
}


// ----------------------------------------------------------------------------
// protected methods
// ----------------------------------------------------------------------------


// init tcp/ip stack (winsock) ------------------------------------------------
//
int	NET_UDPDriver::_InitOSAPI()
{
	MSGOUT( "Initializing TCP/IP stack..." );
	
	return TRUE;
}

// close tcp/ip stack (winsock) -----------------------------------------------
//
int	NET_UDPDriver::_KillOSAPI()
{
	// winsock cleanup
	return TRUE; 
}

// open and initialize the UDP socket -----------------------------------------
//
int	NET_UDPDriver::_OpenSocket()
{
	ASSERT( m_socket == -1 );
	
	// open UDP socket
	m_socket = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( m_socket < 0 ) {
		fprintf( stderr, open_socket_error );
		return FALSE;
	}
	
	// make socket non-blocking
	int arg = TRUE;
	ioctl( m_socket, FIONBIO, (char *)&arg );
	
	// bind listen socket to UDP port on selected interface
	sockaddr_in peeraddr;
	bzero( &peeraddr, sizeof( peeraddr ) );
	peeraddr.sin_family		 = AF_INET;
	// should bind to 0.0.0.0
	//memcpy ( &peeraddr.sin_addr, &m_Node.address, sizeof( IP_ADR_LENGTH ) );
	peeraddr.sin_port		 = htons( m_selected_port );
	
	// set SO_REUSEADDR option, to avoid bind errors when re-binding
	int	on = 1;
	setsockopt( m_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof( on ) );
	
	bind( m_socket, (SA *) &peeraddr, sizeof( peeraddr ) );
	
#if 0	
	// get the max msg size for the underlying UDP protocol 
	unsigned int msgsize;
	int cbOpt = sizeof( msgsize );
	int i = getsockopt( m_socket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&msgsize, (int*)&cbOpt );
	printf( "SO_MAX_MSG_SIZE: %d\n", msgsize ); 
#endif 
	
	return TRUE;
}

// close the UDP socket -------------------------------------------------------
//
int	NET_UDPDriver::_CloseSocket()
{
	if ( m_socket != -1 ) {
		CLOSESOCKET( m_socket );
		m_socket = -1;
	}
	
	return TRUE;
}


// resolve a hostname using the DNS service -----------------------------------
//
int NET_UDPDriver::ResolveHostName( char *hostname, node_t* node )
{
	ASSERT( hostname != NULL );
	ASSERT( node != NULL );

	if ( isdigit( hostname[ 0 ] ) ) {

		// hostname is assumed to be an ip address
		// -> just convert to binary representation

		inet_aton( hostname, (in_addr *) node );

		return TRUE;
	}

	hostent *hptr = gethostbyname( hostname );

	if ( hptr == NULL ) {
		MSGOUT( "NET_UDPDriver::ResolveHostName(): DNS error: %s %s.", hostname, hstrerror( h_errno ) );
		return FALSE;
	}

	if ( hptr->h_addrtype == AF_INET ) {
		char **pptr = hptr->h_addr_list;
		if ( *pptr != NULL ) {
			memcpy( node, *pptr, IP_ADR_LENGTH );
			return TRUE;
		}
	} else {
		MSGOUT( "NET_UDPDriver::ResolveHostName(): illegal address type in DNS entry." );
	}

	return FALSE;
}


// get the local IP corresponding to the configured interface -----------------
//
int	NET_UDPDriver::_RetrieveLocalIP()
{
	//FIXME: implement :)
	int selected_interface = 0;
	
	int sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( sockfd < 0 ) {
		MSGOUT( "NET_UDPDriver::_RetrieveLocalIP(): can't open socket." );
		return FALSE;
	}

	int   infolen = sizeof( struct ifreq ) * 10;
	char* infobuf = NULL;

	struct ifconf ifc;

	infobuf = (char *) ALLOCMEM( infolen );
	if ( infobuf == NULL )
		OUTOFMEM( 0 );

	ifc.ifc_len = infolen;
	ifc.ifc_buf = infobuf;


	// get all interface configurations into buffer
	int lastlen = sizeof( struct ifreq ) * 10;
	for ( ;; ) {

		infobuf = (char *) ALLOCMEM( infolen );
		if ( infobuf == NULL )
			OUTOFMEM( 0 );

		ifc.ifc_len = infolen;
		ifc.ifc_buf = infobuf;

		if ( ioctl( sockfd, SIOCGIFCONF, &ifc ) < 0 ) {
			if ( ( errno != EINVAL ) || ( lastlen != 0 ) ) {
				MSGOUT( "NET_UDPDriver::_RetrieveLocalIP(): ioctl error." );
				FREEMEM( infobuf );
				CLOSESOCKET( sockfd );
				return FALSE;
			}
		} else {
			// if ifc.ifc_len is less than lastlen, we got it all.
			if ( ifc.ifc_len < lastlen ){
				break;
			}
			// if not, save the last length
			lastlen = ifc.ifc_len;

			//make it bigger by 1 record
			infolen += sizeof( struct ifreq );

		}

		//infolen += sizeof( struct ifreq ) * 10;
		FREEMEM( infobuf );
	}

	int rc = 0;
	int ifnum = -1;

	// analyze all interface configurations in buffer
	for ( char *ptr = infobuf; ptr < infobuf + ifc.ifc_len; ) {

		struct ifreq* ifr = (struct ifreq *) ptr;

		// advance to next structure in buffer

#ifdef SYSTEM_MACOSX_UNUSED
		int len = max( sizeof(struct sockaddr), ifr->ifr_addr.sa_len );
#else
		int len = sizeof(struct sockaddr);
#endif

		//		int len = max( sizeof(struct sockaddr), ifr->ifr_addr.sa_len );

		ptr += sizeof( ifr->ifr_name ) + len;

		// only handle IPV4 entries
		if ( ifr->ifr_addr.sa_family != AF_INET ) {
			continue;
		}

		char if_addr[ MAX_IPADDR_LEN + 1 ];
		sockaddr_in	*sa = (struct sockaddr_in *) &ifr->ifr_addr;
		inet_ntop( AF_INET, &sa->sin_addr, if_addr, MAX_IPADDR_LEN + 1 );

		if ( strcmp( "127.0.0.1", if_addr ) == 0 ) {
			MSGOUT( "skipping loopback interface" );
			continue;
		}



		node_t tmp_node;
		memcpy( &tmp_node, &sa->sin_addr, IP_ADR_LENGTH );
		NetworkInterfaces.push_back(tmp_node);


		ifnum++;
		MSGOUT( "found interface #%d: %s", ifnum, if_addr );
	}

	ASSERT(ifnum > -1);


	// see if our preferred interface is in the vector
	if(NetworkInterfaces.size() >= m_PreferredInterface) {
		// let's use the preferred interface to set up the bind address

		_SetupInterface(&NetworkInterfaces[m_PreferredInterface]);


	} else {
		// else let's use interface 0
		_SetupInterface(&NetworkInterfaces[0]); // rc checked below
	}

	//FREEMEM( infobuf );
	CLOSESOCKET( sockfd );
	
	return TRUE;
}


// key table for netiface command -----------------------------------------
//
key_value_s netiface_key_value[] = {

	{ "interface",	NULL,	KEYVALFLAG_NONE				},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_NETIFACE_INTERFACE
};

PRIVATE
int Cmd_NETIFACE( char* niface_command )
{
	//NOTE:
	//CONCOM:
	// netiface_command	::= 'sv.network.iface' <iface spec>
	// iface_spec			::= 'interface' <iface num>


	ASSERT( niface_command != NULL );
	HANDLE_COMMAND_DOMAIN( niface_command );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( netiface_key_value, niface_command ) )
		return TRUE;

	char* pName = netiface_key_value[ KEY_NETIFACE_INTERFACE ].value;

	int selected_interface = atoi(pName);

	// set the selected interface on the UDPDRiver.
	TheUDPDriver->SetPreferredInterface(selected_interface);

	// restart the socket on change
	if(TheUDPDriver->IsRunning()){

		TheUDPDriver->KillDriver();
		TheUDPDriver->InitDriver(NULL, SV_NETCONF_PORT);
	}


	return TRUE;
}

REGISTER_MODULE( NET_UDPDriver )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "sv.network.iface" command
	regcom.command	 = "sv.network.iface";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_NETIFACE;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

}

void NET_UDPDriver::_SetupInterface(node_t *p_Node) {

	// store numeric
	NODE_Copy(&m_Node, p_Node);

	// store presentation
	inet_ntop( AF_INET, &m_Node, m_szIP, MAX_IPADDR_LEN + 1 );

	// store port number in ip address
	NODE_StorePort( &m_Node, m_selected_port );


}
