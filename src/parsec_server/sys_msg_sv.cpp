/*
 * PARSEC - Message Output Function
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:26:07 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001-2002
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

// C library 
#ifdef SYSTEM_TARGET_WINDOWS
#  include <time.h>
#else
#  include <sys/time.h>
#endif

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "net_defs.h"

// net game header
#include "net_game_sv.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "sys_msg_sv.h"

// proprietary module headers
#include "con_main_sv.h"
#include "e_relist.h"
#include "e_simulator.h"


//FIXME: Win32 specific
#ifdef SYSTEM_TARGET_WINDOWS
	#include "windows.h"
#endif

// flags ----------------------------------------------------------------------
//
//#define _DISABLE_ALL_OUTPUT
//#define _DISABLE_SCREEN_OUTPUT

// name of optional log file  -------------------------------------------------
//
static char log_file[]	= "parsec_server.log";

// paste buffer ---------------------------------------------------------------
//
#define MAX_MESSAGE_LEN		1024
static char paste_buffer[ MAX_MESSAGE_LEN ];


// function prototypes --------------------------------------------------------
//
int SYS_WriteLogFileMessage ( const char *format, ... );


// output messages (append new-line if missing ) ------------------------------
//
int SYSs_MsgOut( const char *format, ... )
{
#ifdef _DISABLE_ALL_OUTPUT 
	return TRUE;
#endif // _DISABLE_ALL_OUTPUT

	va_list	ap;
	va_start( ap, format );
	int	rc = vsnprintf( paste_buffer, MAX_MESSAGE_LEN, format, ap );
	va_end( ap );

#ifndef _DISABLE_SCREEN_OUTPUT
	
#if defined ( USE_CURSES ) && defined ( PARSEC_SERVER )
	int x,y;
	getyx( g_curses_in_win, y, x );

	wprintw( g_curses_out_win, paste_buffer );

	// append new-line if needed
	if ( format[ strlen( format ) - 1 ] != '\n' )
		wprintw( g_curses_out_win, "\n" );

	whline( g_curses_out_win, '-', COLS );
	wrefresh( g_curses_out_win );
	
	wmove( g_curses_in_win, y, x );
	wrefresh( g_curses_in_win );
#else
	printf( paste_buffer );
	// append new-line if needed
	if ( format[ strlen( format ) - 1 ] != '\n' )
		printf( "\n" );
#endif // USE_CURSES
	
#endif // _DISABLE_SCREEN_OUTPUT

	// write to logfile
	SYS_WriteLogFileMessage( paste_buffer );
	
	return rc;
}


// put messages (output them and flush afterwards) ----------------------------
//
int SYSs_MsgPut( const char *format, ... )
{
#if defined _DISABLE_ALL_OUTPUT || defined _DISABLE_SCREEN_OUTPUT
	return TRUE;
#endif // _DISABLE_ALL_OUTPUT

	//NOTE:
	// this function does not print a 
	// new-line at the end of the string, 
	// but flushes stdout after output. 
	// this can be used to continue printing 
	// in the same line.
	
#if defined ( USE_CURSES ) && defined ( PARSEC_SERVER )

	va_list	ap;
	va_start( ap, format );
	int	rc = vsnprintf( paste_buffer, MAX_MESSAGE_LEN, format, ap );
	va_end( ap );

	int x,y;
	getyx( g_curses_in_win, y, x );
	wprintw( g_curses_out_win, paste_buffer );
	whline( g_curses_out_win, '-', COLS );
	wrefresh( g_curses_out_win );
	wmove( g_curses_in_win, y, x );
	wrefresh( g_curses_in_win );

	return rc;
#else
	va_list	ap;
	va_start( ap, format );
	int	rc = vfprintf( stdout, format, ap );
	fflush( stdout );
	va_end( ap );

	return rc;
#endif
}

/*
// easy access to the singleton -----------------------------------------------
//
#define TheMicroSecondTimer		SYSs_MicroSecondTimer::GetMicroSecondTimer()

// a microsecond timer with the best resolution for a specific platform -------
//
class SYSs_MicroSecondTimer 
{
	//NOTE: based on ideas from the tcpdump/windump sources 

protected:
	LARGE_INTEGER PTime;
	LARGE_INTEGER SystemTime;
	LARGE_INTEGER StartTime;
	LARGE_INTEGER TimeFreq;

	LARGE_INTEGER m_LastCall;

	SYSs_MicroSecondTimer() 
	{
		// get the absolute value of the system boot time.   
		QueryPerformanceFrequency( &TimeFreq );
		QueryPerformanceCounter( &PTime );

		GetSystemTimeAsFileTime( (LPFILETIME)&SystemTime );

		StartTime.QuadPart = ( ( ( SystemTime.QuadPart ) % 10000000 ) * TimeFreq.QuadPart ) / 10000000;

		// convert from MS (1.1.1601) to UNIX (1.1.1970) seconds
		SystemTime.QuadPart = SystemTime.QuadPart / 10000000 - 11644473600;

		StartTime.QuadPart += ( ( SystemTime.QuadPart ) * TimeFreq.QuadPart ) - PTime.QuadPart;

		// init lastcall time to 
		m_LastCall.QuadPart  = -1;
	}

	~SYSs_MicroSecondTimer() {}

public:
	
	// SINGLETON pattern
	static SYSs_MicroSecondTimer* GetMicroSecondTimer()
	{
		static SYSs_MicroSecondTimer _TheMicroSecondTimer;
		return &_TheMicroSecondTimer;
	}

	void FillTimeval( timeval* tv, timeval* tv_diff = NULL )
	{
		ASSERT( tv != NULL );

		// get the current tickcount
		LARGE_INTEGER now;
		QueryPerformanceCounter( &now ); 

		// adjust to ticks since 1.1.1970
		now.QuadPart += StartTime.QuadPart;
		tv->tv_usec = (long) ( ( now.QuadPart % TimeFreq.QuadPart * 1000000 ) / TimeFreq.QuadPart );
		tv->tv_sec  = (long) ( now.QuadPart / TimeFreq.QuadPart );

		// fill difference to last call ?
		if ( tv_diff != NULL ) {

			// first time call ?
			if ( m_LastCall.QuadPart == -1 ) {
				tv_diff->tv_sec  = 0;
				tv_diff->tv_usec = 0;
			} else {
				// calc diff	
				LARGE_INTEGER diff;
				diff.QuadPart= now.QuadPart - m_LastCall.QuadPart;
				tv_diff->tv_usec = (long) ( ( diff.QuadPart % TimeFreq.QuadPart * 1000000 ) / TimeFreq.QuadPart );
				tv_diff->tv_sec  = (long) ( diff.QuadPart / TimeFreq.QuadPart );
			}
			m_LastCall = now;
		}
	}
};




// get the current time in the format 2002/05/16 16:16:04.800 -----------------
//
const char* SYSs_GetCurTimeString()
{
	static char szBuffer[ 128 ] = "";

#ifdef SYSTEM_WIN32_UNUSED


	// get the current timeval ( usec resolution )
	timeval tv, tv_diff;
	TheMicroSecondTimer->FillTimeval( &tv, &tv_diff );

	SYSTEMTIME st;
	GetLocalTime( &st );
	sprintf( szBuffer, "%4d/%02d/%02d %02d:%02d:%02d.%06d (%d) (%2d.%6d):\t", 
		st.wYear, 
		st.wMonth, 
		st.wDay, 
		st.wHour, 
		st.wMinute, 
		st.wSecond, 
		tv.tv_usec, 
		TheSimulator->GetCurSimRefFrame(),
		tv_diff.tv_sec,
		tv_diff.tv_usec );

#endif // SYSTEM_WIN32_UNUSED

	return szBuffer;
}
*/

// write message to log file --------------------------------------------------
//
int SYS_WriteLogFileMessage( const char *format, ... )
{
#ifdef _DISABLE_ALL_OUTPUT
	return TRUE;
#endif // _DISABLE_ALL_OUTPUT

	FILE *fp = fopen( log_file, "a" );
	if ( fp != NULL ) {

		int formatlen = strlen( format );
		if ( formatlen < 1 ) {
			return -1;
		}

		// prepend the current date/time
		
#ifdef SYSTEM_TARGET_WINDOWS

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

		fprintf( fp, "%ld.%06d: ", tv.tv_sec, tv.tv_usec );

		va_list	ap;
		va_start( ap, format );
		int	rc = vfprintf( fp, format, ap );
		va_end( ap );

		// append newline in logfile
		if ( format[ formatlen - 1 ] != '\n' ) {
			fputs( "\n", fp );
		}

		fclose( fp );
		return rc;
	}

	return -1;
}


