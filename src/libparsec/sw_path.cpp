/*
 * PARSEC - Path Processing
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:43 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1997-1999
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   1998
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
#include "config.h"

#ifdef SYSTEM_TARGET_WINDOWS

// C library
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SYSTEM_COMPILER_GCC
// POSIX headers
#include <dirent.h>
#endif

// compilation flags/debug support
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// i/o system calls
#include "sys_io.h"

// local module header
#include "sw_path.h"

// proprietary module headers
#ifdef PARSEC_SERVER
	#include "con_ext_sv.h"
#else // !PARSEC_SERVER
	#include "con_ext.h"
	#include "e_demo.h"
#endif // !PARSEC_SERVER




// temporary path string ------------------------------------------------------
//
static char path_string[ PATH_MAX + 1 ];


// process path read from console/script and create valid system path ---------
//
char *SYSs_ProcessPathString( char *path )
{
	ASSERT( path != NULL );

	for ( char *scan = path; *scan != 0; scan++ ) {

		// substitute backslashes for slashes
		if ( *scan == '/' ) {
			*scan = '\\';
		}
	}

	return path;
}


// scan to file name extension in supplied string -----------------------------
//
char *SYSs_ScanToExtension( char *fname )
{
	ASSERT( fname != NULL );

	char *scan = fname + strlen( fname );
	while ( ( scan != fname ) && ( *scan != '.' ) )
		scan--;

	return scan;
}


// strip path (directory names) from file name --------------------------------
//
char *SYSs_StripPath( char *fname )
{
	ASSERT( fname != NULL );

	char *scan = fname + strlen( fname );
	while ( ( scan != fname ) && ( *scan != '\\' ) )
		scan--;

	if ( *scan == '\\' )
		scan++;

	return scan;
}


// acquire scripts located at specified path ----------------------------------
//
int SYSs_AcquireScriptPath( char *path, int comtype, char *prefix )
{
	ASSERT( path != NULL );
	ASSERT( comtype >= 0 );
//	ASSERT( prefix != NULL );

	// wildcard extension must be supplied
	ASSERT( strlen( path ) > 4 );

#ifdef SYSTEM_COMPILER_MSVC

	// open script directory
	struct _finddata_t  c_file;
	intptr_t hFile = _findfirst( path, &c_file );

	if ( hFile != -1LL ) {

		int countbase = num_external_commands;

		while ( num_external_commands < MAX_EXTERNAL_COMMANDS ) {

			char *prefixed = c_file.name;

			if ( prefix != NULL ) {
				strcpy( path_string, prefix );
				strcat( path_string, "/" );
				strcat( path_string, c_file.name );
				prefixed = SYSs_ProcessPathString( path_string );
			}

			// skip extension
			int len = strlen( prefixed ) - 4;
			ASSERT( len >= 0 );

			// release-safe; simply skip file names that are too long
			if ( ( len >= 0 ) && ( len <= COMMAND_NAME_ALLOC_LEN ) ) {

				// store base name
				strncpy( external_commands[ num_external_commands ], prefixed, len );
				external_commands[ num_external_commands ][ len ] = 0;

				// set type and convert to lower-case
				external_command_types[ num_external_commands ] = comtype;
				strlwr( external_commands[ num_external_commands ] );

				num_external_commands++;
			}

			if ( _findnext( hFile, &c_file ) != 0 ) {
				break;
			}
		}

		_findclose( hFile );
		return ( num_external_commands - countbase );
	}

#else // SYSTEM_COMPILER_MSVC

	// open script directory
	DIR *dirp = opendir( path );

	if ( dirp != NULL ) {

		int countbase = num_external_commands;
		dirent *direntp;

		// read script names up to maximum number
		while ( ( direntp = readdir( dirp ) ) &&
				( num_external_commands < MAX_EXTERNAL_COMMANDS ) ) {

			char *prefixed = direntp->d_name;

			if ( prefix != NULL ) {
				strcpy( path_string, prefix );
				strcat( path_string, "/" );
				strcat( path_string, direntp->d_name );
				prefixed = SYSs_ProcessPathString( path_string );
			}

			// skip extension
			int len = strlen( prefixed ) - 4;
			ASSERT( len >= 0 );

			// release-safe
			if ( len < 0 )
				continue;

			// simply skip file names that are too long
			if ( len > COMMAND_NAME_ALLOC_LEN )
				continue;

			// store base name
			strncpy( external_commands[ num_external_commands ], prefixed, len );
			external_commands[ num_external_commands ][ len ] = 0;

			// set type and convert to lower-case
			external_command_types[ num_external_commands ] = comtype;
			strlwr( external_commands[ num_external_commands ] );

			num_external_commands++;
		}

		closedir( dirp );
		return ( num_external_commands - countbase );
	}

#endif // SYSTEM_COMPILER_MSVC

	return 0;
}


// acquire demos located at specified path ------------------------------------
//
int SYSs_AcquireDemoPath( char *path, char *prefix )
{
	ASSERT( path != NULL );
//	ASSERT( prefix != NULL );

	// wildcard extension must be supplied
	ASSERT( strlen( path ) > 4 );

#ifdef PARSEC_CLIENT

#ifdef SYSTEM_COMPILER_MSVC

	// open demo directory
	struct _finddata_t  c_file;
	long hFile = _findfirst( path, &c_file );

	if ( hFile != -1L ) {

		int countbase = num_registered_demos;

		while ( num_registered_demos < max_registered_demos ) {

			// skip extension
			int len = strlen( c_file.name ) - 4;
			ASSERT( len >= 0 );

			// release-safe
			if ( len >= 0 ) {

				// store demo name
				int demoid = num_registered_demos;
				if ( registered_demo_names[ demoid ] != NULL ) {
					FREEMEM( registered_demo_names[ demoid ] );
					registered_demo_names[ demoid ] = NULL;
				}
				registered_demo_names[ demoid ] = (char *) ALLOCMEM( len + 1 );
				if ( registered_demo_names[ demoid ] == NULL )
					OUTOFMEM( 0 );
				strncpy( registered_demo_names[ demoid ], c_file.name, len );
				registered_demo_names[ demoid ][ len ] = 0;

				// convert to lower-case
				strlwr( registered_demo_names[ demoid ] );

				num_registered_demos++;
			}

			if ( _findnext( hFile, &c_file ) != 0 ) {
				break;
			}
		}

		_findclose( hFile );
		return ( num_registered_demos - countbase );
	}

#else // !SYSTEM_COMPILER_MSVC

	// open demo directory
	DIR *dirp = opendir( path );

	if ( dirp != NULL ) {

		int countbase = num_registered_demos;
		dirent *direntp;

		// read demo names up to maximum number
		while ( ( direntp = readdir( dirp ) ) &&
				( num_registered_demos < max_registered_demos ) ) {

			// skip extension
			int len = strlen( direntp->d_name ) - 4;
			ASSERT( len >= 0 );

			// release-safe
			if ( len < 0 )
				continue;

			// store demo name
			int demoid = num_registered_demos;
			if ( registered_demo_names[ demoid ] != NULL ) {
				FREEMEM( registered_demo_names[ demoid ] );
				registered_demo_names[ demoid ] = NULL;
			}
			registered_demo_names[ demoid ] = (char *) ALLOCMEM( len + 1 );
			if ( registered_demo_names[ demoid ] == NULL )
				OUTOFMEM( 0 );
			strncpy( registered_demo_names[ demoid ], direntp->d_name, len );
			registered_demo_names[ demoid ][ len ] = 0;

			// convert to lower-case
			strlwr( registered_demo_names[ demoid ] );

			num_registered_demos++;
		}

		closedir( dirp );
		return ( num_registered_demos - countbase );
	}

#endif // !SYSTEM_COMPILER_MSVC

#endif // PARSEC_CLIENT

	return 0;
}


#endif // SYSTEM_TARGET_WINDOWS
