/*
 * PARSEC - Swap Endianness of Data
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:42 $
 *
 * Orginally written by:
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1998-1999
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-1999
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

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "gd_heads.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// local module header
#include "sys_swap.h"



// swap endianness of packageheader_s -----------------------------------------
//
void SYS_SwapPackageHeader( packageheader_s *p )
{
	ASSERT( p != NULL );

	p->numitems   = SWAP_32( p->numitems );
	p->headersize = SWAP_32( p->headersize );
	p->datasize   = SWAP_32( p->datasize );
	p->packsize   = SWAP_32( p->packsize );
}


// swap endianness of packageheader_s -----------------------------------------
//
void SYS_SwapPFileInfo( pfileinfodisk_s *p )
{
	ASSERT( p != NULL );

	p->foffset = SWAP_32( p->foffset );
	p->flength = SWAP_32( p->flength );
	p->fcurpos = SWAP_32( p->fcurpos );
}


// swap endianness of BdtHeader -----------------------------------------------
//
void SYS_SwapBdtHeader( BdtHeader *b )
{
	ASSERT( b != NULL );

	b->width  = SWAP_32( b->width  );
	b->height = SWAP_32( b->height );
}


// swap endianness of FntHeader -----------------------------------------------
//
void SYS_SwapFntHeader( FntHeader *f )
{
	ASSERT( f != NULL );

	f->width  = SWAP_32( f->width  );
	f->height = SWAP_32( f->height );
}


// swap endianness of TexHeader -----------------------------------------------
//
void SYS_SwapTexHeader( TexHeader *t )
{
	ASSERT( t != NULL );

	t->width  = SWAP_32( t->width  );
	t->height = SWAP_32( t->height );
}


// swap endianness of DemHeader -----------------------------------------------
//
void SYS_SwapDemHeader( DemHeader *d )
{
	ASSERT( d != NULL );

	d->headersize  = SWAP_32( d->headersize );
}


// swap endianness of PfgHeader -----------------------------------------------
//
void SYS_SwapPfgHeader( PfgHeader *p )
{
	ASSERT( p != NULL );

	p->srcwidth	= SWAP_32( p->srcwidth  );
	p->width	= SWAP_16( p->width  );
	p->height	= SWAP_16( p->height );
}


// swap endianness of font geometry table -------------------------------------
//
void SYS_SwapPfgTable( int fixsize, dword *geomtab, size_t tabsize )
{
	ASSERT( geomtab != NULL );

	//NOTE:
	// proportional fonts are swapped on-the-fly and need
	// not be swapped here.

#ifdef SYSTEM_BIG_ENDIAN

	if ( fixsize ) {

		ASSERT( ( tabsize & 0x03 ) == 0x00 );

		for ( dword indx = 0; indx < tabsize; indx+=4 ) {
			geomtab[ indx >> 2 ] = SWAP_32( geomtab[ indx >> 2 ] );
		}
	}

#endif

}



