/*
 * PARSEC - Utility Functions - server
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:26:06 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001-2002
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
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

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// network subsystem & server headers 
#include "net_defs.h"
#include "e_defs.h"

// network code config
//#include "net_conf.h"

// UNP header
#include "net_wrap.h"

// local module header
#include "net_util.h"


// flags ----------------------------------------------------------------------
//

// copy one node over another -------------------------------------------------
//
void NODE_Copy( node_t* dst, node_t* src )
{
	ASSERT( dst != NULL );
	ASSERT( src != NULL );

	memcpy( dst, src, sizeof( node_t ) );
}


// store port number into node address structure ------------------------------
//
void NODE_StorePort( node_t *node, word port )
{
	ASSERT( node != NULL );
	
	node->address[ 4 ] = port >> 8;
	node->address[ 5 ] = port & 0xff;
}


// fetch port number from node address structure ------------------------------
//
word NODE_GetPort( node_t *node )
{
	ASSERT( node != NULL );
	
	return ( ( (word)node->address[ 4 ] << 8 ) | node->address[ 5 ] );
}


// compare two node addresses (equal, less than, greater than) ----------------
//
int NODE_Compare( node_t *node1, node_t *node2 )
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

// compare two node addresses ( fast version ) --------------------------------
//
int NODE_AreSame( node_t *node1, node_t *node2 )
{
	return ( memcmp( (void*)node1, (void*)node2, sizeof( node_t ) ) == 0 );
}


// static print area for a node string ----------------------------------------
//
//FIXME: IPv6 ?
static char szNodePrintBuffer[ sizeof "255.255.255.255:65535" ];

// return a STATIC string that contains the node in printable form ------------
//
char* NODE_Print( node_t* node )
{
	ASSERT( node != NULL );

	// convert IP
	char ipstring[ MAX_IPADDR_LEN + 1 ];
	inet_ntop( AF_INET, node, ipstring, MAX_IPADDR_LEN + 1 );

	//NOTE: this function uses a STATIC string buffer that holds the printable
	//      form of the node. Do not store the pointer, as it may get overwritten
	//      by subsequent calls to NODE_Print

	sprintf( szNodePrintBuffer, "%s:%d", ipstring,  NODE_GetPort( node ) );
	return szNodePrintBuffer;
}

