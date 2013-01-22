/*
 * PARSEC - Logfile class 
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:46 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
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
#include <string.h>
#include <math.h>
#include <stdarg.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// C library 
#ifdef _WIN32
#  include <time.h>
#else
#  include <sys/time.h>
#endif

// platform specific includes
#ifdef _WIN32
	#include "windows.h"
	#include "mmsystem.h"
	#pragma comment( lib, "winmm.lib" )
#endif // SYSTEM_WIN32


// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "sys_defs.h"

// local module header
#include "utl_logfile.h"


// flags ----------------------------------------------------------------------
//
#define ENABLE_AUTO_FLUSH

// constants ------------------------------------------------------------------
//
#define MAX_REGISTERED_LOGFILES		10



// global logfile -------------------------------------------------------------
//
#ifdef ENABLE_LOGOUT_COMMAND
	
	UTL_LogFile g_Logfile( "global.log" );

#endif // ENABLE_LOGOUT_COMMAND

// global list of registered logfiles -----------------------------------------
//
static UTL_LogFile*	g_RegisteredLogfiles[ MAX_REGISTERED_LOGFILES ];
static int			g_nNumRegisteredLogfiles = 0;


// register a logfile globally to ensure it is flushed upon program termination
//
PRIVATE 
int RegisterLogfile( UTL_LogFile* pLogfile )
{
	ASSERT( pLogfile != NULL );

	// check if list is full
	if ( g_nNumRegisteredLogfiles >= MAX_REGISTERED_LOGFILES ) {
		return FALSE;
	}

	// append to list
	g_RegisteredLogfiles[ g_nNumRegisteredLogfiles ] = pLogfile;
	g_nNumRegisteredLogfiles++;

	return TRUE;
}


// flush all registerd logfiles -----------------------------------------------
//
void UTL_FlushRegisteredLogfiles()
{
	for( int nLogfile = 0; nLogfile < g_nNumRegisteredLogfiles; nLogfile++ ) {
		g_RegisteredLogfiles[ nLogfile ]->Flush();
	}
}


// add an entry to the logfile ------------------------------------------------
//
void UTL_LogFile::_AddEntry( const char* szEntry, size_t len2 )
{
	ASSERT( szEntry != NULL );

	//FIXME: use SYSs_GetCurTimeString()

	// prepend the current date/time
	
#ifdef _WIN32

	//FIXME: recheck whether SYSs_GetCurTimeString() works properly
	//fprintf( fp, "%s: ", SYSs_GetCurTimeString() );

	LARGE_INTEGER now;
	LARGE_INTEGER TimeFreq;
	QueryPerformanceCounter( &now );
	QueryPerformanceFrequency( &TimeFreq );

	timeval tv;
	tv.tv_usec = (long) ( ( now.QuadPart % TimeFreq.QuadPart * 1000000 ) / TimeFreq.QuadPart );
	tv.tv_sec  = (long) ( now.QuadPart / TimeFreq.QuadPart );

#else

	struct timeval  tv;
	struct timezone tz;
	gettimeofday( &tv, &tz );
	
#endif

	char szTimeString[ 32 + 1 ];
	snprintf( szTimeString, 32, "%u.%06u\t", (unsigned int)tv.tv_sec, (unsigned int)tv.tv_usec );
	szTimeString[ 32 ] = 0;
	
	size_t len1 = strlen( szTimeString );

	if ( ( m_nBufferLen + len1 + len2 ) >= MAX_LOG_BUFFER_LEN ) {
		Flush();
	}

	// check whether to directly write to the file
	if ( ( len1 + len2 ) >= MAX_LOG_BUFFER_LEN ) {
		fwrite( szTimeString, 1, len1, m_pFile );
		fwrite( szEntry,      1, len2, m_pFile );
	} else {
		// copy strings to buffer
		memcpy( &m_szBuffer[ m_nBufferLen ],        szTimeString, len1 );
		memcpy( &m_szBuffer[ m_nBufferLen + len1 ], szEntry,      len2 );
		m_nBufferLen += ( len1 + len2 );
		m_szBuffer[ m_nBufferLen ] = 0;

#define REFFRAME_TO_FORCE_FLUSH		DEFAULT_REFFRAME_FREQUENCY

#ifdef ENABLE_AUTO_FLUSH
		// check for forced flush
		if ( ( m_LastFlushRefframe + REFFRAME_TO_FORCE_FLUSH ) < SYSs_GetRefFrameCount() )  {
			Flush();
		}
#endif // ENABLE_AUTO_FLUSH
	}
}

// standard ctor --------------------------------------------------------------
//
UTL_LogFile::UTL_LogFile( const char* szFilename /* = NULL */) :
	m_nBufferLen( 0 ),
	m_pFile( NULL )
{
	m_szBuffer[ 0 ] = 0;

	if ( szFilename != NULL ) {
		if ( !Open( szFilename ) ) {
			MSGOUT( "error opening logfile '%s'", szFilename );
		}
	}

	m_LastFlushRefframe = SYSs_GetRefFrameCount();

	// register the logfile
	RegisterLogfile( this );
}

// standard dtor --------------------------------------------------------------
//
UTL_LogFile::~UTL_LogFile()
{
	if ( m_pFile != NULL ) {
		Flush();
		fclose( m_pFile );
	}
}

// open the logfile -----------------------------------------------------------
//
int UTL_LogFile::Open( const char* szFilename )
{
	ASSERT( szFilename != NULL );
	ASSERT( m_pFile == NULL );

	m_pFile = fopen( szFilename, "a" );
	return ( m_pFile != NULL );
}

// printf like adding to the logfile ------------------------------------------
//
int UTL_LogFile::printf( const char *format, ... )
{
	long formatlen = strlen( format );
	if ( formatlen < 1 ) {
		return -1;
	}

	char szTemp[ MAX_LOG_BUFFER_LEN + 1 ];

	va_list	ap;
	va_start( ap, format );
	int rc = vsnprintf( szTemp, MAX_LOG_BUFFER_LEN, format, ap );
	va_end( ap );

	if ( rc > 0 ) {
		strcat( szTemp, "\n" );
		_AddEntry( szTemp, rc + 1 );
	}

	return rc;
}

// flush the buffer to the disk file ------------------------------------------
//
void UTL_LogFile::Flush()
{
	ASSERT( m_pFile != NULL );

	fwrite( m_szBuffer, m_nBufferLen, 1, m_pFile );

	m_szBuffer[ 0 ] = 0;
	m_nBufferLen	= 0;

	m_LastFlushRefframe = SYSs_GetRefFrameCount();

	fflush( m_pFile );
}
