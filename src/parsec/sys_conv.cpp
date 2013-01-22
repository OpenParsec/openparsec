/*
 * PARSEC - Package Data Conversion
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:41 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   2000
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
#include "sys_conv.h"

// proprietary module headers
#include "sys_swap.h"



// convert all mp3 files contained in a package to wavs (in-place) ------------
//
PRIVATE
int ConvertPackageMP3sToWAVs( const char *packname, size_t baseofs, int numitems, pfileinfodisk_s *items )
{
#ifdef HAVE_MP3

	ASSERT( packname != NULL );
	ASSERT( numitems >= 0 );
	ASSERT( items != NULL );

	int numconverted = 0;

	// determine file size for append position
	FILE *fp = fopen( packname, "rb" );
	if ( fp == NULL ) {
		return -1;
	}
	if ( fseek( fp, 0, SEEK_END ) != 0 ) {
		fclose( fp );
		return -1;
	}

	dword appendpos = ftell( fp );

	fclose( fp );
	fp = NULL;

	// scan entire file list for mp3 files
	for ( int curit = 0; curit < numitems; curit++ ) {

		// check mp3 extension
		int len = strlen( items[ curit ].file );
		if ( len < 5 ) {
			continue;
		}
		if ( stricmp( &items[ curit ].file[ len - 4 ], ".mp3" ) != 0 ) {
			continue;
		}

		// display stand-by message
		if ( numconverted == 0 ) {
			MSGOUT( "You have started Parsec for the first time.\n" );
			MSGPUT( "Please wait while converting data..." );
		} else if ( ( numconverted & 0x03 ) == 0 ) {
			MSGPUT( "." );
		}

		// open pack for mp3 reading
		fp = fopen( packname, "rb" );
		if ( fp == NULL ) {
			return -1;
		}

		// set read position
		size_t readpos = baseofs + items[ curit ].foffset;
		if ( fseek( fp, readpos, SEEK_SET ) != 0 ) {
			fclose( fp );
			return -1;
		}

		// autocrop by default
		dword cropstart = 0;
		dword cropend   = 0;

		// hardcoded crop positions for looping samples
		if ( stricmp( items[ curit ].file, "back4-r.mp3" ) == 0 ) {
			cropstart = 1382;
			cropend   = 690825;
		} else if ( stricmp( items[ curit ].file, "blitz2-r.mp3" ) == 0 ) {
			cropstart = 1304;
			cropend   = 79687;
		} else if ( stricmp( items[ curit ].file, "thrus2-r.mp3" ) == 0 ) {
			cropstart = 1351;
			cropend   = 24062;
		} else if ( stricmp( items[ curit ].file, "helix2-r.mp3" ) == 0 ) {
			cropstart = 1387;
			cropend   = 3050;
		}

		// conversion will return size
		size_t wavdatasize;

		// convert mp3 data to wav
		char *memwav = SND_ConvertFileMP3ToMemWAV(
			fp, items[ curit ].flength, &wavdatasize, cropstart, cropend );
		if ( memwav == NULL ) {
			fclose( fp );
			return -1;
		}

		// done with mp3 reading
		if ( fclose( fp ) != 0 ) {
			FREEMEM( memwav );
			return -1;
		}

		// open pack for appending
		FILE *fp = fopen( packname, "ab" );
		if ( fp == NULL ) {
			FREEMEM( memwav );
			return -1;
		}

		// append wav data to pack
		size_t byteswritten = fwrite( memwav, 1, wavdatasize, fp );
		if ( byteswritten != wavdatasize ) {
			FREEMEM( memwav );
			fclose( fp );
			return -1;
		}

		// free mem allocated by SND_ConvertFileMP3ToMemWAV()
		FREEMEM( memwav );
		memwav = NULL;

		// done with appending wav data
		if ( fclose( fp ) != 0 ) {
			return -1;
		}

		// patch file info list
		items[ curit ].foffset = appendpos;
		items[ curit ].flength = wavdatasize;

		strcpy( &items[ curit ].file[ len - 4 ], ".wav" );

		appendpos += wavdatasize;
		numconverted++;
	}

	if ( numconverted > 0 ) {
		MSGOUT( "\n" );
	}

	return numconverted;

#else // !HAVE_MP3
	
	return 0;

#endif 
}


// convert all mp3 files contained in a package to wavs (in-place) ------------
//
int SYS_ConvertPackageMP3s( const char *packname, size_t baseofs )
{
	ASSERT( packname != NULL );

	// open package for reading at specified offset
	FILE *fp = fopen( packname, "rb" );
	if ( fp == NULL ) {
		return FALSE;
	}
	if ( fseek( fp, baseofs, SEEK_SET ) != 0 ) {
		fclose( fp );
		return FALSE;
	}

	packageheader_s packheader;

	// read fixed-size part of header
	size_t bytesread = fread( &packheader, 1, sizeof( packheader ), fp );
	if ( bytesread != sizeof( packheader ) ) {
		fclose( fp );
		return FALSE;
	}

	// file format validation
	if ( stricmp( packheader.signature, "msh DataPackage" ) != 0 ) {
		fclose( fp );
		return FALSE;
	}

	// swap endianness of fixed-size part of header
	SYS_SwapPackageHeader( &packheader );

	// fetch size of remaining header and number of contained items
	size_t headersize = packheader.headersize;
	dword numitems   = packheader.numitems;

	// read file list
	pfileinfodisk_s *items = (pfileinfodisk_s *) ALLOCMEM( headersize );
	if ( items == NULL ) {
		fclose( fp );
		return FALSE;
	}
	if ( fread( items, 1, headersize, fp ) != headersize ) {
		FREEMEM( items );
		fclose( fp );
		return FALSE;
	}

	// will be reopened later on
	fclose( fp );
	fp = NULL;

	// swap endianness of file info list
	dword curit = 0;
	for ( curit = 0; curit < numitems; curit++ ) {
		SYS_SwapPFileInfo( &items[ curit ] );
	}

	// convert all contained mp3s to wavs
	int numconverted = ConvertPackageMP3sToWAVs( packname, baseofs, numitems, items );
	if ( numconverted == -1 ) {
		FREEMEM( items );
		return FALSE;
	}

	// avoid patching if nothing changed
	if ( numconverted < 1 ) {
		FREEMEM( items );
		return TRUE;
	}

	// open package for writing the patched header
	fp = fopen( packname, "rb+" );
	if ( fp == NULL ) {
		FREEMEM( items );
		return FALSE;
	}
	if ( fseek( fp, baseofs + sizeof( packheader ), SEEK_SET ) != 0 ) {
		FREEMEM( items );
		fclose( fp );
		return FALSE;
	}

	// swap endianness of file info list
	for ( curit = 0; curit < numitems; curit++ ) {
		SYS_SwapPFileInfo( &items[ curit ] );
	}

	// write patched header
	if ( fwrite( items, 1, headersize, fp ) != headersize ) {
		FREEMEM( items );
		fclose( fp );
		return FALSE;
	}

	FREEMEM( items );

	if ( fclose( fp ) != 0 ) {
		return FALSE;
	}

	return TRUE;
}

