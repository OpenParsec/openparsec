/*
 * PARSEC - Path Processing
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:42 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1997-1999
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

#ifndef SYSTEM_TARGET_WINDOWS

// C library
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// POSIX headers
#include <dirent.h>

// compilation flags/debug support
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// local module header
#include "sl_path.h"

// proprietary module headers
#ifdef PARSEC_SERVER
#include "con_ext_sv.h"
#include "con_main_sv.h"
#else // !PARSEC_SERVER
#include "con_ext.h"
#include "con_main.h"
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

		// substitute slashes for backslashes
		if ( *scan == '\\' ) {
			*scan = '/';
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
	while ( ( scan != fname ) && ( *scan != '/' ) )
		scan--;

	if ( *scan == '/' )
		scan++;

	return scan;
}


// temporary storage for path modification ------------------------------------
//
static char modify_path[ PATH_MAX + 1 ];


// acquire scripts located at specified path ----------------------------------
//
int SYSs_AcquireScriptPath( char *path, int comtype, char *prefix )
{
	ASSERT( path != NULL );
	ASSERT( comtype >= 0 );
//	ASSERT( prefix != NULL );

	// wildcard extension must be supplied
	ASSERT( strlen( path ) > 4 );

	//NOTE:
	// this function strips the wildcard from the script path and
	// checks the extension manually, since the wildcard extension
	// to POSIX's opendir() is apparently not supported under linux.

	// make a copy we can modify
	strncpy( modify_path, path, PATH_MAX );
	modify_path[ PATH_MAX ] = 0;
	path = modify_path;

	static char con_wcard[] = "*" CON_FILE_EXTENSION;
	static char *con_fext   = con_wcard + 1;
	static size_t wcardlen    = strlen( con_wcard );

	// filter wildcard for console scripts
	size_t pathlen  = strlen( path );
	if ( pathlen >= wcardlen ) {
		if ( stricmp( path + pathlen - wcardlen, con_wcard ) == 0 ) {
			path[ pathlen - wcardlen     ] = '.';
			path[ pathlen - wcardlen + 1 ] = 0;
		}
	}

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

			size_t len = strlen( prefixed ) - wcardlen + 1;

			// do manual wildcard checking
			if ( len < 1 )
				continue;
			if ( stricmp( prefixed + len, con_fext ) != 0 )
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
	
	//NOTE:
	// this function strips the wildcard from the demo path and
	// checks the extension manually, since the wildcard extension
	// to POSIX's opendir() is apparently not supported under linux.

	// make a copy we can modify
	strncpy( modify_path, path, PATH_MAX );
	modify_path[ PATH_MAX ] = 0;
	path = modify_path;

	static char dem_wcard[] = "*" CON_FILE_COMPILED_EXTENSION;
	static char *dem_fext   = dem_wcard + 1;
	static size_t  wcardlen    = strlen( dem_wcard );

	// filter wildcard for demo names
	size_t pathlen  = strlen( path );
	if ( pathlen >= wcardlen ) {
		if ( stricmp( path + pathlen - wcardlen, dem_wcard ) == 0 ) {
			path[ pathlen - wcardlen     ] = '.';
			path[ pathlen - wcardlen + 1 ] = 0;
		}
	}

	// open demo directory
	DIR *dirp = opendir( path );

	if ( dirp != NULL ) {

		int countbase = num_registered_demos;
		dirent *direntp;

		// read demo names up to maximum number
		while ( ( direntp = readdir( dirp ) ) &&
				( num_registered_demos < max_registered_demos ) ) {

			long len = strlen( direntp->d_name ) - wcardlen + 1;

			// do manual wildcard checking
			if ( len < 1 )
				continue;
			if ( stricmp( direntp->d_name + len, dem_fext ) != 0 )
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

#endif // PARSEC_CLIENT

	return 0;
}

#endif // !SYSTEM_TARGET_WINDOWS

