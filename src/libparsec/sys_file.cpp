/*
 * PARSEC - File System Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:46 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2001
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

// i/o system calls
#include "sys_io.h"

// local module header
#include "sys_file.h"

// proprietary module headers
#ifdef PARSEC_SERVER
	#include "con_aux_sv.h"
	#include "con_ext_sv.h"
	#include "con_main_sv.h"
#else // !PARSEC_SERVER
	#include "con_aux.h"
	#include "con_ext.h"
	#include "con_main.h"
	#include "e_demo.h"
#endif // !PARSEC_SERVER


#include "sys_path.h"
#include "sys_swap.h"


// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// default maximum number of data packages ------------------------------------
//
#define DEFAULT_MAX_PACKAGES	32


// current and maximum number of data packages --------------------------------
//
PUBLIC int				num_data_packages = 0;
static int				max_data_packages = DEFAULT_MAX_PACKAGES;


// data package info tables ---------------------------------------------------
//
PUBLIC char*			package_filename[ DEFAULT_MAX_PACKAGES ];
static size_t			package_filebase[ DEFAULT_MAX_PACKAGES ];
static packageheader_s	package_header[ DEFAULT_MAX_PACKAGES ];
static pfileinfo_s*		package_items[ DEFAULT_MAX_PACKAGES ];
static size_t			package_headersize[ DEFAULT_MAX_PACKAGES ];
static size_t			package_numitems[ DEFAULT_MAX_PACKAGES ];


// register a data package ----------------------------------------------------
//
int SYS_RegisterPackage( const char *packname, size_t baseofs, char *prefix )
{
	ASSERT( packname != NULL );
//	ASSERT( prefix != NULL );

	// guard against overflow
	if ( num_data_packages >= max_data_packages ) {
		return FALSE;
	}

	int packid = num_data_packages;

	// may be necessary if error occurred earlier on
	if ( package_filename[ packid ] != NULL ) {
		FREEMEM( package_filename[ packid ] );
		package_filename[ packid ] = NULL;
	}

	// must be done here since it is not required of the caller to convert the
	// supplied path prior to calling this function (to ease mod registration)
	char *path = SYSs_ProcessPathString( (char*)packname );

	// alloc filename
	package_filename[ packid ] = (char *) ALLOCMEM( strlen( path ) + 1 );
	if ( package_filename[ packid ] == NULL ) {
		return FALSE;
	}

	// store base info
	strcpy( package_filename[ packid ], path );
	package_filebase[ packid ] = baseofs;

	// open for reading at specified offset
	FILE *fp = fopen( package_filename[ packid ], "rb" );
	if ( fp == NULL ) {
		return FALSE;
	}
	if ( fseek( fp, package_filebase[ packid ], SEEK_SET ) != 0 ) {
		fclose( fp );
		return FALSE;
	}

	// read fixed-size part of header
	size_t bytesread = fread( &package_header[ packid ], 1, sizeof( packageheader_s ), fp );
	if ( bytesread != sizeof( packageheader_s ) ) {
		fclose( fp );
		return FALSE;
	}

	// swap endianness of fixed-size part of header
	SYS_SwapPackageHeader( &package_header[ packid ] );

	// fetch size of remaining header and number of contained items
	package_headersize[ packid ] = package_header[ packid ].headersize;
	package_numitems[ packid ]   = package_header[ packid ].numitems;

	// may be necessary if error occurred earlier on
	if ( package_items[ packid ] != NULL ) {
		FREEMEM( package_items[ packid ] );
		package_items[ packid ] = NULL;
	}

	// read file list
	pfileinfodisk_s *diskitems = (pfileinfodisk_s *) ALLOCMEM( package_headersize[ packid ] );
	if ( diskitems == NULL ) {
		fclose( fp );
		return FALSE;
	}
	bytesread = fread( diskitems, 1, package_headersize[ packid ], fp );
	if ( bytesread != package_headersize[ packid ] ) {
		fclose( fp );
		return FALSE;
	}

	// swap endianness of file info list
	pfileinfodisk_s	*items = diskitems;
	unsigned int curit = 0;
	for ( curit = 0; curit < package_numitems[ packid ]; curit++ ) {
		SYS_SwapPFileInfo( &items[ curit ] );
	}

	fclose( fp );
	num_data_packages++;

	// copy file info list for use in memory
	package_items[ packid ] = (pfileinfo_s *) ALLOCMEM( package_numitems[ packid ] * sizeof( pfileinfo_s ) );
	if ( package_items[ packid ] == NULL ) {
		return FALSE;
	}

	char *prefixpath = NULL;
	if ( prefix != NULL ) {

		// must be done to ensure files containing path delimiters
		// can be found later on when comparing with paths that have
		// also been processed to match the host system conventions
		strncpy( paste_str, prefix, PASTE_STR_LEN - 1 );
		paste_str[ PASTE_STR_LEN - 1 ] = 0;
		strcat( paste_str, "/" );
		prefixpath = SYSs_ProcessPathString( paste_str );
	}

	int sizenames = 0;
	size_t prefixlen = ( prefixpath != NULL ) ? ( strlen( prefixpath ) + 1 ) : 0;

	// count total storage amount for filenames
	for ( curit = 0; curit < package_numitems[ packid ]; curit++ ) {
		sizenames += prefixlen + strlen( diskitems[ curit ].file ) + 1;
	}

	// allocate filename storage
	char *namelist = (char *) ALLOCMEM( sizenames );
	if ( namelist == NULL ) {
		return FALSE;
	}

	// copy filenames into storage and pfileinfodisk_s into pfileinfo_s
	for ( curit = 0; curit < package_numitems[ packid ]; curit++ ) {

		if ( prefix == NULL || prefixpath == NULL ) {
			strcpy( namelist, diskitems[ curit ].file );
		} else {
			strcpy( namelist, prefixpath );
			strcat( namelist, diskitems[ curit ].file );
		}
		
		pfileinfodisk_s * diskitem = &diskitems[ curit ];

		package_items[ packid ][ curit ].file	 = namelist;
		package_items[ packid ][ curit ].foffset = diskitem->foffset;
		package_items[ packid ][ curit ].flength = diskitem->flength;
		package_items[ packid ][ curit ].fp		 = (FILE *) diskitem->fp;
		package_items[ packid ][ curit ].fcurpos = diskitem->fcurpos;

		namelist += prefixlen + strlen( diskitems[ curit ].file ) + 1;
	}

	FREEMEM( diskitems );

	return TRUE;
}


// override all files in old package with newer versions in new package -------
//
int SYS_OverridePackage( const char* oldpackname, const char* newpackname )
{
	//NOTE:
	// packages must have already been registered 

	int oldid = -1;
	int newid = -1;

	// find old package
	for ( int packid = 0; packid < num_data_packages; packid++ ) {

		if ( stricmp( package_filename[ packid ], oldpackname ) == 0 ) {
		
			oldid = packid;
			
		} else if ( stricmp( package_filename[ packid ], newpackname ) == 0 ) {
		
			newid = packid;
		}
	}

	// packages must exist and be different
	ASSERT( oldid != -1 );
	ASSERT( newid != -1 );
	ASSERT( newid != oldid );

	if ( oldid == -1 || 
		 newid == -1 ||
		 oldid == newid ) {
		return FALSE;
	}

	// replace all items found in both packages with a dummy name in old package

	pfileinfo_s	*newitems = package_items[ newid ];

	// scan all items of new package
	for ( unsigned int curit = 0; curit < package_numitems[ newid ]; curit++ ) {
	
		// scan all items of old package
		pfileinfo_s	*olditems = package_items[ oldid ];

		for ( unsigned int curoldit = 0; curoldit < package_numitems[ oldid ]; curoldit++ ) {
		
			if ( stricmp( newitems[ curit ].file, olditems[ curoldit ].file ) == 0 ) {
				
				strcpy( olditems[ curoldit ].file, "dummy.old" );
			}
		}
	}
	
	return TRUE;
}


// acquire scripts contained in package files ---------------------------------
//
int SYS_AcquirePackageScripts( int comtype )
{
	// scan all registered packages
	for ( int packid = 0; packid < num_data_packages; packid++ ) {

		// scan all items of this package
		for ( unsigned int curit = 0; curit < package_numitems[ packid ]; curit++ ) {

			// guard against overflow of maximum number of scripts
			if ( num_external_commands >= MAX_EXTERNAL_COMMANDS )
				break;

			int scriptid = num_external_commands;
			pfileinfo_s	*item = &package_items[ packid ][ curit ];

			// extension is mandatory
			long len = strlen( item->file ) - 4;

//			ASSERT( ( len >= 0 ) && ( len <= COMMAND_NAME_ALLOC_LEN ) );
			if ( len < 0 )
				continue;

			// check script extension
			if ( strcmp( item->file + len, CON_FILE_EXTENSION ) != 0 )
				continue;

			// store base name
			strncpy( external_commands[ scriptid ], item->file, len );
			external_commands[ scriptid ][ len ] = 0;

			// set type and convert to lower-case
			external_command_types[ scriptid ] = comtype;
			strlwr( external_commands[ scriptid ] );

			num_external_commands++;
		}
	}

	return TRUE;
}


// acquire demos contained in package files -----------------------------------
//
int SYS_AcquirePackageDemos()
{
#ifdef PARSEC_CLIENT
	// scan all registered packages
	for ( unsigned int packid = 0; packid < (unsigned int)num_data_packages; packid++ ) {

		// scan all items of this package
		for ( unsigned int curit = 0; curit < package_numitems[ packid ]; curit++ ) {

			// guard against overflow of registered demos
			if ( num_registered_demos >= max_registered_demos )
				break;

			int demoid = num_registered_demos;
			pfileinfo_s	*item = &package_items[ packid ][ curit ];

			// extension is mandatory
			long len = strlen( item->file ) - 4;
//			ASSERT( ( len >= 0 ) && ( len <= COMMAND_NAME_ALLOC_LEN ) );
			if ( len < 0 )
				continue;

			// check demo extension
			if ( strcmp( item->file + len, CON_FILE_COMPILED_EXTENSION ) != 0 )
				continue;

			// store demo name
			if ( registered_demo_names[ demoid ] != NULL ) {
				FREEMEM( registered_demo_names[ demoid ] );
				registered_demo_names[ demoid ] = NULL;
			}
			registered_demo_names[ demoid ] = (char *) ALLOCMEM( len + 1 );
			if ( registered_demo_names[ demoid ] == NULL )
				continue;
			strncpy( registered_demo_names[ demoid ], item->file, len );
			registered_demo_names[ demoid ][ len ] = 0;

			// convert to lower-case
			strlwr( registered_demo_names[ demoid ] );

			num_registered_demos++;
		}
	}
#endif // PARSEC_CLIENT
	return TRUE;
}


// determine file length of real system file ----------------------------------
//
PRIVATE
long int SYS_SysGetFileLength( const char *filename )
{
	ASSERT( filename != NULL );

	FILE *fp = fopen( filename, "rb" );
	if ( fp == NULL )
		return -1L;

	if ( fseek( fp, 0, SEEK_END ) != 0 )
		return -1L;

	long int siz = ftell( fp );

	fclose( fp );
	return siz;
}

// check the internal version agains the pscdata*.dat version
//
int SYS_CheckDataVersion(){
	// attempt to open the version file
	FILE *ver_file = SYS_fopen("psdatver.txt", "rb");
	if(ver_file == NULL) {
		MSGOUT("Unable to open psdatver.txt. Maybe you need to update your pscdata*.dat files?");
		return false;
	}

	char verstr[3];
	// attempt to read the file into the verstr buffer
	int read_rc = SYS_fread((void *)verstr, sizeof(char), 3, ver_file);
	if (read_rc <= 0) {
		MSGOUT("Unable to read psdatver.txt. Maybe you need to update your pscdata*.dat files?");
		return false;
	}

	// now let's try to convert the string to a integer so we can check the version
	//int data_version = 0;
	//data_version = (int)strtol(verstr, NULL, 10);
	MSGOUT("pscdata version %s, internal version %s", PSCDATA_VERSION, verstr);
	if(strcmp(verstr, PSCDATA_VERSION)) {
		MSGOUT("psdatver.txt version mismatch: Internal version %s does not match data file version %s.", PSCDATA_VERSION, verstr);
		return false;
	}

	SYS_fclose(ver_file);

	return true;
}


// fetch item return structure for the following two routines -----------------
//
struct fetchitem_s {

	int				packid;
	pfileinfo_s*	item;
};

#define FETCH_CACHE_MASK	0x03
#define FETCH_CACHE_SIZE	( FETCH_CACHE_MASK + 1 )

static int _fetch_cache_pos = 0;
static fetchitem_s _fetch_item;
static fetchitem_s _fetch_cache[ FETCH_CACHE_SIZE ];


// fetch item by file name ----------------------------------------------------
//
PRIVATE
fetchitem_s	*FetchItemByName( const char *filename )
{
	ASSERT( filename != NULL );

	// scan all registered packages
	for ( int packid = 0; packid < num_data_packages; packid++ ) {

		// scan all items of this package
		pfileinfo_s	*items = package_items[ packid ];

		for ( unsigned int curit = 0; curit < package_numitems[ packid ]; curit++ ) {
			if ( stricmp( items[ curit ].file, filename ) == 0 ) {
				_fetch_item.packid = packid;
				_fetch_item.item   = &items[ curit ];
				return &_fetch_item;
			}
		}
	}

	return NULL;
}


// fetch item by file pointer -------------------------------------------------
//
PRIVATE
fetchitem_s	*FetchItemByFile( FILE *fp )
{
	ASSERT( fp != NULL );

	// try cache first
	for ( int cp = 0; cp < FETCH_CACHE_SIZE; cp++ ) {
		if ( _fetch_cache[ cp ].item == NULL ) {
			continue;
		}
		if ( _fetch_cache[ cp ].item->fp == fp ) {
			return &_fetch_cache[ cp ];
		}
	}

	// scan all registered packages
	for ( int packid = 0; packid < num_data_packages; packid++ ) {

		// scan all items of this package
		pfileinfo_s	*items = package_items[ packid ];
		unsigned int curit = 0;
		for ( curit = 0; curit < package_numitems[ packid ]; curit++ ) {
			if ( items[ curit ].fp == fp )
				break;
		}

		if ( curit < package_numitems[ packid ] ) {

			// enter into cache
			int cp = _fetch_cache_pos;
			_fetch_cache[ cp ].packid = packid;
			_fetch_cache[ cp ].item   = &items[ curit ];

			_fetch_cache_pos++;
			_fetch_cache_pos &= FETCH_CACHE_MASK;

			// return info for found item
			return &_fetch_cache[ cp ];
		}
	}

	return NULL;
}


// determine file length ------------------------------------------------------
//
long int SYS_GetFileLength( const char *filename )
{
	ASSERT( filename != NULL );

	if ( AUX_DISABLE_PACKAGE_DATA_FILES ) {
		return SYS_SysGetFileLength( filename );
	}

	fetchitem_s	*fetchitem = FetchItemByName( filename );
	if ( fetchitem != NULL ) {
		return fetchitem->item->flength;
	}

	// fallback
	return SYS_SysGetFileLength( filename );
}


// open file ------------------------------------------------------------------
//
FILE *SYS_fopen( const char *filename, const char *mode )
{
	ASSERT( filename != NULL );
	ASSERT( mode != NULL );

	if ( AUX_DISABLE_PACKAGE_DATA_FILES ) {
		return fopen( filename, mode );
	}

	// open package for reading at correct offset if file found as item
	fetchitem_s	*fetchitem = FetchItemByName( filename );
	if ( fetchitem != NULL ) {

		// open package file
		FILE *fp = fopen( package_filename[ fetchitem->packid ], mode );

		// this may indeed happen if too few handles available
		ASSERT( fp != NULL );
		if ( fp == NULL ) {
			return NULL;
		}

		// set read position to start of item in package
		size_t readpos = package_filebase[ fetchitem->packid ];
		readpos += fetchitem->item->foffset;
		fseek( fp, readpos, SEEK_SET );

		// init item state
		fetchitem->item->fp      = fp;
		fetchitem->item->fcurpos = 0;

		return fp;
	}

	// fallback
	return fopen( filename, mode );
}


// close file -----------------------------------------------------------------
//
int SYS_fclose( FILE *fp )
{
	ASSERT( fp != NULL );

	if ( AUX_DISABLE_PACKAGE_DATA_FILES ) {
		return fclose( fp );
	}

	// clear file pointer if file found as item
	fetchitem_s	*fetchitem = FetchItemByFile( fp );
	if ( fetchitem != NULL ) {
		fetchitem->item->fp = NULL;
	}

	// close file (package file/real file)
	return fclose( fp );
}


// read from file -------------------------------------------------------------
//
size_t SYS_fread( void *buf, size_t elsize, size_t nelem, FILE *fp )
{
	ASSERT( buf != NULL );
	ASSERT( fp != NULL );

	if ( AUX_DISABLE_PACKAGE_DATA_FILES ) {
		return fread( buf, elsize, nelem, fp );
	}

	// read file from package if found as item
	fetchitem_s	*fetchitem = FetchItemByFile( fp );
	if ( fetchitem != NULL ) {

		// size in bytes
		size_t readsize = nelem * elsize;

		// make sure we are not reading past eof
		pfileinfo_s	*item = fetchitem->item;
		size_t beyond = item->fcurpos + readsize;
		if ( beyond > item->flength ) {
			readsize = ( item->flength > item->fcurpos ) ?
				item->flength - item->fcurpos : 0;
		}

		// read data
		if ( readsize == 0 )
			return 0;
		size_t bytesread = fread( buf, 1, readsize, fp );
		item->fcurpos += bytesread;
		return bytesread;
	}

	// fallback
	return fread( buf, elsize, nelem, fp );
}


// seek to file position ------------------------------------------------------
//
int SYS_fseek( FILE *fp, long int offset, int whence )
{
	ASSERT( fp != NULL );

	if ( AUX_DISABLE_PACKAGE_DATA_FILES ) {
		return fseek( fp, offset, whence );
	}

	// position in package if file found as item
	fetchitem_s	*fetchitem = FetchItemByFile( fp );
	if ( fetchitem != NULL ) {

		switch ( whence ) {

			case SEEK_SET:
				fetchitem->item->fcurpos = (dword) offset;
				break;

			case SEEK_CUR:
				fetchitem->item->fcurpos += offset;
				break;

			case SEEK_END:
				fetchitem->item->fcurpos  = fetchitem->item->flength;
				fetchitem->item->fcurpos += offset;
				break;

			default:
				return -1;
		}

		long int packofs = package_filebase[ fetchitem->packid ];
		packofs += fetchitem->item->foffset;
		packofs += fetchitem->item->fcurpos;
		return fseek( fp, packofs, SEEK_SET );
	}

	// fallback
	return fseek( fp, offset, whence );
}


// tell file position ---------------------------------------------------------
//
long int SYS_ftell( FILE *fp )
{
	ASSERT( fp != NULL );

	if ( AUX_DISABLE_PACKAGE_DATA_FILES ) {
		return ftell( fp );
	}

	// position in package if file found as item
	fetchitem_s	*fetchitem = FetchItemByFile( fp );
	if ( fetchitem != NULL ) {
		return fetchitem->item->fcurpos;
	}

	// fallback
	return ftell( fp );
}


// get eof state of file ------------------------------------------------------
//
int SYS_feof( FILE *fp )
{
	ASSERT( fp != NULL );

	if ( AUX_DISABLE_PACKAGE_DATA_FILES ) {
		return feof( fp );
	}

	//NOTE:
	// the semantics of this function when used for files contained in
	// packages is not exactly identical to the semantics of feof().
	// the eof state is directly queried using the current (read) position
	// in the file, whereas feof() doesn't return eof until a read operation
	// has actually been performed beyond eof. therefore it will also return
	// eof if the read position has only been set with SYS_fseek(), but not
	// yet been used by SYS_fread(), which differs from the reference behavior.

	// query position in package if file found as item
	fetchitem_s	*fetchitem = FetchItemByFile( fp );
	if ( fetchitem != NULL ) {
		return ( fetchitem->item->fcurpos >= fetchitem->item->flength );
	}

	// fallback
	return feof( fp );
}


// last character read by Item_getc() -----------------------------------------
//
static int last_c;


// getc() for package file ----------------------------------------------------
//
inline
int Item_getc( pfileinfo_s *item )
{
	if ( item->fcurpos >= item->flength ) {
		last_c = EOF;
	} else {
		last_c = getc( item->fp );
		item->fcurpos++;
	}

	return last_c;
}


// ungetc() for package file --------------------------------------------------
//
inline
int Item_ungetc( pfileinfo_s *item )
{
	if ( last_c != EOF ) {
		ungetc( last_c, item->fp );
		item->fcurpos--;
	}

	return last_c;
}


// read a line of text from a file --------------------------------------------
//
char *SYS_fgets( char *string, int n, FILE *fp )
{
	ASSERT( string != NULL );
	ASSERT( fp != NULL );

	if ( AUX_DISABLE_PACKAGE_SCRIPTS ) {
		return fgets( string, n, fp );
	}

	fetchitem_s	*fetchitem = FetchItemByFile( fp );
	if ( fetchitem != NULL ) {

		// n includes the terminator
		if ( n <= 0 ) {
			return NULL;
		}

		// read until eol, eof, or string mem end
		int	c = EOF;
		char *output = NULL;
		for ( output = string; --n > 0; ) {

			c = Item_getc( fetchitem->item );

			// check for eof
			if ( c == EOF ) {
				if ( output == string ) {
					return NULL;
				} else {
					break;
				}
			}

			// store read character
			*output++ = c;

			if ( c == 0x0d ) {
				break;
			}
			if ( c == 0x0a ) {
				break;
			}
		}

		// try to read 0x0a after 0x0d
		if ( c == 0x0d ) {

			if ( Item_getc( fetchitem->item ) == 0x0a ) {
				output[ -1 ] = '\n';
			} else {
				Item_ungetc( fetchitem->item );
			}

		// try to read 0x0d after 0x0a
		} else if ( c == 0x0a ) {

			if ( Item_getc( fetchitem->item ) == 0x0d ) {
				output[ -1 ] = '\n';
			} else {
				Item_ungetc( fetchitem->item );
			}
		}

		// terminate string
		*output = 0;
		return string;
	}

	// fallback
	return fgets( string, n, fp );
}


// open file (operating system level) -----------------------------------------
//
int SYS_open( const char *path, int access )
{
	// dummy
	return 0;
}


// close file (operating system level) ----------------------------------------
//
int SYS_close( int handle )
{
	// dummy
	return 1;
}


// read file (operating system level) -----------------------------------------
//
int SYS_read( int handle, char *buffer, int len )
{
	// dummy
	return 0;
}


// get file length (operating system level) -----------------------------------
//
long int SYS_filelength( int handle )
{
	// dummy
	return 0;
}



