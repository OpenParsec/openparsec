/*
 * PARSEC - Server Util Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:26:07 $
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
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// local module header
#include "sys_util_sv.h"

// do a hexdump ---------------------------------------------------------------
//
void SYS_HexDump( char* data, int len )
{
	char szASCII[ 17 ];
	memset( szASCII, 0, 17 );

	printf( "-----------------------------------------------------------------------------\n");
	unsigned char* p = (unsigned char*)data;
	for( int i = 0; i < len; i++ ) {
		if ( i % 16 == 0 ) {
			printf( "%08X: ", i );
		}
		printf( "%02X ", *p );
		szASCII[ i % 16 ] = isprint( *p ) ? *p : '.';
		int newline = ( ( ( i % 16 ) == 15 ) || ( i == ( len - 1 ) ) ) ;
		
		// align ASCII part in last line
		if ( i == ( len - 1 ) ) {
			int miss = ( 15 - ( i % 16 ) );
			for( int k = 0; k < miss; k++ ) {
				printf( "   " );
			}
		}
		
		if( newline )  {
			printf( " | %s\n", szASCII );
			memset( szASCII, 0, 17 );
		}
		p++;
	}
	printf( "-----------------------------------------------------------------------------\n");
}

