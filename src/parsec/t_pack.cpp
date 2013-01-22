/*
 * PARSEC - Data Package Tools
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:42 $
 *
 * Orginally written by:
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

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
#include "t_pack.h"

// proprietary module headers
#include "e_getopt.h"
#include "sys_swap.h"



// string constants -----------------------------------------------------------
//
static char pack_signature[]		= "msh DataPackage";
static char options_invalid[]		= "invalid command line option.\n";
static char pack_name_missing[]		= "name of package file missing.\n";


// variables settable via command line options --------------------------------
//
static char		arg_package_name[ PATH_MAX + 1 ];
static size_t	arg_package_file_offset = 0;
static char		arg_file_name[ PATH_MAX + 1 ];
static char		arg_list_name[ PATH_MAX + 1 ];
static int		arg_view_files = FALSE;


// message output wrappers ----------------------------------------------------
//
#define Printf		MSGOUT
#define PrintfNoNL	MSGPUT
#define Err_Printf	MSGOUT


// exit function that makes sure that messages can be read by the user --------
//
PRIVATE
void Exit( int code )
{
	SYSs_MsgPrompt();
	exit( code );
}


// set name of package --------------------------------------------------------
//
PRIVATE
int Tm_SetPackageName( char **packname )
{
	strncpy( arg_package_name, packname[ 0 ], PATH_MAX );
	arg_package_name[ PATH_MAX ] = 0;

	return TRUE;
}


// set offset of package start in package file --------------------------------
//
PRIVATE
int Tm_SetPackageOffset( int *packofs )
{
	arg_package_file_offset = packofs[ 0 ];

	return TRUE;
}


// set file name argument -----------------------------------------------------
//
PRIVATE
int Tm_SetFileName( char **filename )
{
	strncpy( arg_file_name, filename[ 0 ], PATH_MAX );
	arg_file_name[ PATH_MAX ] = 0;

	return TRUE;
}


// set list name argument -----------------------------------------------------
//
PRIVATE
int Tm_SetListName( char **listname )
{
	strncpy( arg_list_name, listname[ 0 ], PATH_MAX );
	arg_list_name[ PATH_MAX ] = 0;

	return TRUE;
}


// set flag to display file names ---------------------------------------------
//
PRIVATE
int Tm_ViewFileList()
{
	arg_view_files = TRUE;
	return TRUE;
}


// data file list item (info read from text file) -----------------------------
//
struct list_item_s {

	char	file[ PATH_MAX + 1 ];
	dword	foffset;
	dword	flength;
};


// read file list -------------------------------------------------------------
//
PRIVATE
int ReadFileList( const char *filename, list_item_s *listitems, int maxitems )
{
	ASSERT( filename != NULL );
	ASSERT( listitems != NULL );
	ASSERT( maxitems > 0 );

	FILE *fp = fopen( filename, "r" );
	if ( fp == NULL ) {
		Err_Printf( "file list \"%s\" not found.\n", filename );
		return -1;
	}

	Printf( "processing file list ..." );

	int pcount   = 32;
	int numitems = 0;

	static char list_line[ PATH_MAX + 1 ];

	// read file list
	while ( fgets( list_line, PATH_MAX + 1, fp ) != NULL ) {

		char *scan = strtok( list_line, " \t\n\r" );
		if ( scan == NULL ) {
			continue;
		}

		if ( ( *scan == ';' ) || ( *scan == '#' ) ) {
			continue;
		}

		if ( --pcount == 0 ) {
			PrintfNoNL( "." );
			pcount = 32;
		}

		if ( numitems >= maxitems ) {
			Err_Printf( "\ntoo many files. (%d max)\n", maxitems );
			fclose( fp );
			return -1;
		}

		strncpy( listitems[ numitems ].file, scan, PATH_MAX );
		listitems[ numitems ].file[ PATH_MAX ] = 0;
		numitems++;
	}

	Printf( "\n" );
	Printf( "file list read.\n" );
	Printf( "%d items found.\n", numitems );

	fclose( fp );
	return numitems;
}


// copy string and strip path -------------------------------------------------
//
PRIVATE
void StrcpyStripPath( char *dst, const char *src )
{
	ASSERT( dst != NULL );
	ASSERT( src != NULL );

	int baselen = 0;
	int pos = 0;
	for ( pos = strlen( src ) - 1; pos >= 0; pos-- ) {

		if ( src[ pos ] == '\\' )
			break;
		if ( src[ pos ] == '/' )
			break;
		if ( src[ pos ] == ':' )
			break;

		if ( ++baselen >= 12 ) {
			pos--;
			break;
		}
	}

	if ( baselen > 0 ) {
		strncpy( dst, &src[ pos + 1 ], baselen );
		dst[ baselen ] = 0;
	}
}


// write header/file info list into output package ----------------------------
//
PRIVATE
int WritePackageHeader( FILE *fp, list_item_s *listitems, int numitems )
{
	ASSERT( fp != NULL );
	ASSERT( listitems != NULL );
	ASSERT( numitems > 0 );

	// alloc file info list
	size_t headersize  = ( numitems + 1 ) * sizeof( pfileinfodisk_s );
	pfileinfodisk_s *items = (pfileinfodisk_s *) ALLOCMEM( headersize );
	if ( items == NULL )
		return FALSE;
	memset( items, 0, headersize );

	size_t datasize  = 0;
	size_t curoffset = sizeof( packageheader_s ) + headersize;

	// create file info list
	int curit = 0;
	for (  curit = 0; curit < numitems; curit++ ) {

		PrintfNoNL( "." );

		StrcpyStripPath( items[ curit ].file, listitems[ curit ].file );

		FILE *itfp = fopen( listitems[ curit ].file, "rb" );
		if ( itfp == NULL ) {
			Err_Printf( "\ndata file \"%s\" not found: %s\n",
					 listitems[ curit ].file, strerror( errno ) );
			FREEMEM( items );
			return FALSE;
		}
		if ( fseek( itfp, 0, SEEK_END ) != 0 ) {
			Err_Printf( "\ndata file \"%s\" access error: %s\n",
					 listitems[ curit ].file, strerror( errno ) );
			fclose( itfp );
			FREEMEM( items );
			return FALSE;
		}

		items[ curit ].flength     = (dword) ftell( itfp );
		listitems[ curit ].flength = items[ curit ].flength;

		items[ curit ].foffset     = curoffset;
		listitems[ curit ].foffset = items[ curit ].foffset;

		datasize  += items[ curit ].flength;
		curoffset += items[ curit ].flength;

		fclose( itfp );
	}

	// create fixed-size header
	packageheader_s packheader;
	packheader.numitems   = numitems;
	packheader.headersize = headersize;
	packheader.datasize   = datasize;
	packheader.packsize   = sizeof( packageheader_s ) + headersize + datasize;
	strcpy( packheader.signature, pack_signature );

	Printf( "\ndata size: %d\n", datasize );

	// swap endianness of fixed-size part of header
	SYS_SwapPackageHeader( &packheader );

	// write fixed-size header
	size_t byteswritten = fwrite( &packheader, 1, sizeof( packheader ), fp );
	if ( byteswritten != sizeof( packheader ) ) {
		FREEMEM( items );
		return FALSE;
	}

	// swap endianness of file info list
	for ( curit = 0; curit < numitems; curit++ ) {
		SYS_SwapPFileInfo( &items[ curit ] );
	}

	// write file info list
	byteswritten = fwrite( items, 1, headersize, fp );
	if ( byteswritten != headersize ) {
		FREEMEM( items );
		return FALSE;
	}

	FREEMEM( items );
	return TRUE;
}


// write actual file data into output package ---------------------------------
//
PRIVATE
int WritePackageData( FILE *fp, list_item_s *listitems, int numitems )
{
	ASSERT( fp != NULL );
	ASSERT( listitems != NULL );
	ASSERT( numitems > 0 );

	for ( int curit = 0; curit < numitems; curit++ ) {

		PrintfNoNL( "." );

		FILE *itfp = fopen( listitems[ curit ].file, "rb" );
		if ( itfp == NULL ) {
			Err_Printf( "\ndata file \"%s\" not found: %s\n",
					 listitems[ curit ].file, strerror( errno ) );
			return FALSE;
		}

		char *filedata = (char *) ALLOCMEM( listitems[ curit ].flength );
		if ( filedata == NULL ) {
			fclose( itfp );
			return FALSE;
		}

		size_t bytesread = fread( filedata, 1, listitems[ curit ].flength, itfp );
		if ( bytesread != listitems[ curit ].flength ) {
			FREEMEM( filedata );
			fclose( itfp );
			return FALSE;
		}

		size_t byteswritten = fwrite( filedata, 1, listitems[ curit ].flength, fp );
		if ( byteswritten != listitems[ curit ].flength ) {
			FREEMEM( filedata );
			fclose( itfp );
			return FALSE;
		}

		fclose( itfp );
		FREEMEM( filedata );
	}

	return TRUE;
}



// write package data ---------------------------------------------------------
//
PRIVATE
int WritePackageFile( const char* packname, list_item_s *listitems, int numitems )
{
	ASSERT( packname != NULL );
	ASSERT( listitems != NULL );
	ASSERT( numitems > 0 );

	Printf( "creating file package \"%s\" ...", packname );
	fflush( stdout );

	// create package file
	FILE *fp = fopen( packname, "wb" );
	if ( fp == NULL ) {
		Err_Printf( "\ncannot create output file: %s\n", strerror( errno ) );
		return FALSE;
	}

	if ( !WritePackageHeader( fp, listitems, numitems ) ) {
		fclose( fp );
		return FALSE;
	}

	if ( !WritePackageData( fp, listitems, numitems ) ) {
		fclose( fp );
		return FALSE;
	}

	fclose( fp );
	return TRUE;
}


// critical error exit --------------------------------------------------------
//
PRIVATE
void MakepackError()
{
	Err_Printf( "-makepack: critical error.\n" );
	Exit( EXIT_FAILURE );
}


// create data package --------------------------------------------------------
//
PRIVATE
int CreatePackage( const char* packname, const char *listname )
{
	ASSERT( packname != NULL );
	ASSERT( listname != NULL );

	#define MAX_DATA_FILES 4096

	list_item_s *listitems = (list_item_s *) ALLOCMEM( MAX_DATA_FILES * sizeof( list_item_s ) );
	if ( listitems == NULL ) {
		return FALSE;
	}

	int numitems = ReadFileList( listname, listitems, MAX_DATA_FILES );
	if ( numitems == -1 ) {
		FREEMEM( listitems );
		MakepackError();
	}

	if ( !WritePackageFile( packname, listitems, numitems ) ) {
		FREEMEM( listitems );
		MakepackError();
	}

	FREEMEM( listitems );
	return TRUE;
}


// strip extension from supplied file name ------------------------------------
//
PRIVATE
void StripExtension( char *fname )
{
	ASSERT( fname != NULL );

	for ( int len = strlen( fname ); len >= 0; len-- ) {
		if ( fname[ len ] == '.' ) {
			fname[ len ] = 0;
			break;
		}
		if ( fname[ len ] == '\\' ) {
			break;
		}
		if ( fname[ len ] == '/' ) {
			break;
		}
		if ( fname[ len ] == ':' ) {
			break;
		}
	}
}


// package creation tool main function ----------------------------------------
//
void MAKEPACK_main( int argc, char **argv )
{
	// clear options of main app
	OPT_ClearOptions();

	// register local options
	OPT_RegisterStringOption( "p", "pack", Tm_SetPackageName );
	OPT_RegisterStringOption( "f", "list", Tm_SetListName );

	// exec all registered command line options
	if ( !OPT_ExecRegisteredOptions( argc, argv ) ) {
		Err_Printf( options_invalid );
		Exit( EXIT_FAILURE );
	}

	// package name is mandatory
	if ( arg_package_name[ 0 ] == 0 ) {
		Err_Printf( pack_name_missing );
		Exit( EXIT_FAILURE );
	}

	// list name is optional (use package with different extension by default)
	if ( arg_list_name[ 0 ] == 0 ) {
		strcpy( arg_list_name, arg_package_name );
		StripExtension( arg_list_name );
		strcat( arg_list_name, ".lst" );
	}

	// create package
	CreatePackage( arg_package_name, arg_list_name );

	// end of sub-application
	Exit( EXIT_SUCCESS );
}


// optional file name parameters ----------------------------------------------
//
static char *extract_file_name	= NULL;
static char *list_file_name		= NULL;


// extract data of packaged file(s) -------------------------------------------
//
PRIVATE
int ExtractPackageData( FILE *fp, FILE *listfp, size_t baseofs, int numitems, pfileinfodisk_s *items )
{
	ASSERT( fp != NULL );
	ASSERT( numitems >= 0 );
	ASSERT( items != NULL );

	int numextracted = 0;

	// write out separate data files
	for ( int curit = 0; curit < numitems; curit++ ) {

		// extract only specified file if name set
		if ( extract_file_name != NULL ) {
			if ( stricmp( items[ curit ].file, extract_file_name ) != 0 ) {
				continue;
			}
		}

		// check for viewing mode (no actual file extraction)
		if ( arg_view_files ) {
			Printf( "file %s size %d\n", items[ curit ].file, items[ curit ].flength );
			continue;
		}

		char *curdata = (char *) ALLOCMEM( items[ curit ].flength );
		if ( curdata == NULL ) {
			return -1;
		}

		// set read position
		size_t readpos = baseofs + items[ curit ].foffset;
		if ( fseek( fp, readpos, SEEK_SET ) != 0 ) {
			FREEMEM( curdata );
			return -1;
		}

		// read packaged file data
		size_t bytesread = fread( curdata, 1, items[ curit ].flength, fp );
		if ( bytesread != items[ curit ].flength ) {
			FREEMEM( curdata );
			return -1;
		}

		// display current file and write it to list if open
		Printf( "*extracting %s\n", items[ curit ].file );
		if ( listfp != NULL ) {
			fprintf( listfp, "%s\n", items[ curit ].file );
		}

		// create output file
		FILE *wfp = fopen( items[ curit ].file, "wb" );
		if ( wfp == NULL ) {
			FREEMEM( curdata );
			return -1;
		}

		// write file data to separate file
		size_t byteswritten = fwrite( curdata, 1, items[ curit ].flength, wfp );
		if ( byteswritten != items[ curit ].flength ) {
			FREEMEM( curdata );
			fclose( wfp );
			return -1;
		}

		if ( fclose( wfp ) != 0 ) {
			FREEMEM( curdata );
			return -1;
		}

		FREEMEM( curdata );
		numextracted++;
	}

	return numextracted;
}


// critical error exit --------------------------------------------------------
//
PRIVATE
void GetpackError()
{
	Err_Printf( "-getpack: critical error.\n" );
	Exit( EXIT_FAILURE );
}


// extract file(s) from package -----------------------------------------------
//
PRIVATE
int ExtractPackage( const char* packname, size_t baseofs )
{
	ASSERT( packname != NULL );

	// open package for reading at specified offset
	FILE *fp = fopen( packname, "rb" );
	if ( fp == NULL ) {
		Err_Printf( "package %s not found.\n", packname );
		GetpackError();
	}
	if ( fseek( fp, baseofs, SEEK_SET ) != 0 ) {
		fclose( fp );
		GetpackError();
	}

	packageheader_s packheader;

	// read fixed-size part of header
	size_t bytesread = fread( &packheader, 1, sizeof( packheader ), fp );
	if ( bytesread != sizeof( packheader ) ) {
		fclose( fp );
		GetpackError();
	}

	// file format validation
	if ( stricmp( packheader.signature, pack_signature ) != 0 ) {
		Err_Printf( "package %s is invalid.\n", packname );
		fclose( fp );
		GetpackError();
	}

	// swap endianness of fixed-size part of header
	SYS_SwapPackageHeader( &packheader );

	// fetch size of remaining header and number of contained items
	dword headersize = packheader.headersize;
	dword numitems   = packheader.numitems;

	// display number of contained items if in viewing mode
	if ( arg_view_files ) {
		Printf( "package contains %d items.\n\n", numitems );
	}

	// read file list
	pfileinfodisk_s *items = (pfileinfodisk_s *) ALLOCMEM( headersize );
	if ( items == NULL ) {
		fclose( fp );
		GetpackError();
	}
	if ( fread( items, 1, headersize, fp ) != headersize ) {
		FREEMEM( items );
		fclose( fp );
		GetpackError();
	}

	// swap endianness of file info list
	for ( unsigned int curit = 0; curit < numitems; curit++ ) {
		SYS_SwapPFileInfo( &items[ curit ] );
	}

	// open list output file if specified
	FILE *listfp = NULL;
	if ( ( list_file_name != NULL ) && ( extract_file_name == NULL ) ) {
		listfp = fopen( list_file_name, "w" );
		fprintf( listfp, "\n" );
		fprintf( listfp, "; file list of package \"%s\"\n", packname );
		fprintf( listfp, "\n" );
	}

	// extract file data
	Printf( "extracting from package %s ...\n", packname );
	int numextracted = ExtractPackageData( fp, listfp, baseofs, numitems, items );

	// close/free resources
	if ( listfp != NULL ) {
		fprintf( listfp, "\n" );
		fclose( listfp );
		listfp = NULL;
	}
	fclose( fp );
	FREEMEM( items );

	if ( numextracted == -1 ) {
		GetpackError();
	}

	if ( extract_file_name == NULL ) {
		Printf( "%d files extracted.\n", numextracted );
		return TRUE;
	}

	if ( numextracted < 1 ) {
		Printf( "file %s not found in package %s\n", extract_file_name, packname );
		return FALSE;
	}

	return TRUE;
}


// package extraction tool main function --------------------------------------
//
void GETPACK_main( int argc, char **argv )
{
	// clear options of main app
	OPT_ClearOptions();

	// register local options
	OPT_RegisterStringOption( "p", "pack",   Tm_SetPackageName );
	OPT_RegisterIntOption(    "o", "offset", Tm_SetPackageOffset );
	OPT_RegisterStringOption( "f", "file",   Tm_SetFileName );
	OPT_RegisterStringOption( "l", "list",   Tm_SetListName );
	OPT_RegisterSetOption(    "v", "view",   Tm_ViewFileList );

	// exec all registered command line options
	if ( !OPT_ExecRegisteredOptions( argc, argv ) ) {
		Err_Printf( options_invalid );
		Exit( EXIT_FAILURE );
	}

	// package name is mandatory
	if ( arg_package_name[ 0 ] == 0 ) {
		Err_Printf( pack_name_missing );
		Exit( EXIT_FAILURE );
	}

	// file name is optional (operate on entire package by default)
	extract_file_name = ( arg_file_name[ 0 ] != 0 ) ? arg_file_name : NULL;

	// list name is optional (do not create a list file by default)
	list_file_name = ( arg_list_name[ 0 ] != 0 ) ? arg_list_name : NULL;

	// extract one file or all files from package
	ExtractPackage( arg_package_name, arg_package_file_offset );

	// end of sub-application
	Exit( EXIT_SUCCESS );
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( T_PACK )
{
	OPT_RegisterApplication( "-makepack", MAKEPACK_main );
	OPT_RegisterApplication( "-getpack",  GETPACK_main );
}



