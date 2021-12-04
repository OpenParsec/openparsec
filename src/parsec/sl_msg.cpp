/*
 * PARSEC - Message Output Function
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:33 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-2000
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

// compilation flags/debug support
#include "config.h"


// C library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// subsystem headers
#include "vid_defs.h"

// global externals
#include "globals.h"

// local module header
#include "sl_msg.h"

// proprietary module headers
#include "con_aux.h"
#include "con_ext.h"
#include "con_main.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 1023
static char paste_str[ PASTE_STR_LEN + 1 ];


// console already accessible? ------------------------------------------------
//
extern int console_init_done;


// automatically append new-line in text-mode?  -------------------------------
//
static int append_nl	= TRUE;


// name of optional log file  -------------------------------------------------
//
static char log_file[]	= "parsec.log";


// write message to log file --------------------------------------------------
//
PRIVATE
void SLm_WriteLogFileMessage( const char *msg )
{
	ASSERT( msg != NULL );

	size_t len = strlen( msg );
	if ( len < 1 ) {
		return;
	}

	FILE *fp = fopen( log_file, "a" );
	if ( fp != NULL ) {

		fprintf( fp, "%s", msg );
		fclose( fp );
	}
}


// defined in IL_XLIB.C and IL_SVLIB.C ----------------------------------------
//
extern void ISDL_SplashTextLine( char *line );
extern void ISDL_LogWinTextLine( char *line );


// general message output function --------------------------------------------
//
PRIVATE
int SL_MsgOut( const char *format, va_list ap )
{
	int rc;
	extern int headless_bot;
	// check if string is terminated by new line
	int newlinemissing = ( format[ strlen( format ) - 1 ] != '\n' );
	if(headless_bot) {
		// compose string
		rc = vsprintf( paste_str, format, ap );
		//Dump to STDOUT
		printf("%s",paste_str);
		// optionally append to log file
		if ( AUX_ENABLE_CONSOLE_MESSAGE_LOG & 0x02 ) {
			SLm_WriteLogFileMessage( paste_str );
		}
		
	} else {
		if ( TextModeActive ) {

			//NOTE:
			// if no new-line terminates the string
			// it is appended automatically, except
			// SYSs_MsgPut() was used.

			// compose string
			rc = vsprintf( paste_str, format, ap );

			// output string in splash screen
			ISDL_SplashTextLine( paste_str );

			// append new-line if needed
			if ( newlinemissing && append_nl ) {
				size_t len = strlen( paste_str );
				paste_str[ len ]     = '\n';
				paste_str[ len + 1 ] = 0;
			}

			// optionally append to log file
			if ( AUX_ENABLE_CONSOLE_MESSAGE_LOG & 0x01 ) {
				SLm_WriteLogFileMessage( paste_str );
			}

			// output string
			ISDL_LogWinTextLine( paste_str );
			

		} else {

			if ( console_init_done ) {

				//NOTE:
				// if the string ends with a newline the current
				// input in the console is not preserved. that is,
				// the string is output on its own line, just below
				// the current input line. then, a cursor-only line
				// is output below the string output line.

				// create valid console string
				vsprintf( paste_str, format, ap );

				if ( newlinemissing ) {

					// optionally append to log file
					if ( AUX_ENABLE_CONSOLE_MESSAGE_LOG & 0x04 ) {
						if ( append_nl ) {
							int len = strlen( paste_str );
							paste_str[len]	= '\n';
							paste_str[len+1]= 0;
						}
					
						SLm_WriteLogFileMessage( paste_str );
					}
				
					ProcessExternalLine( paste_str );

					// preserve current console line
					CON_AddMessage( paste_str );

				} else {

					// optionally append to log file
					if ( AUX_ENABLE_CONSOLE_MESSAGE_LOG & 0x02 ) {
						SLm_WriteLogFileMessage( paste_str );
					}
				
					ProcessExternalLine( paste_str );

					// terminate old line's input before
					// printing the string on its own line.
					CON_AddLine( paste_str );
					CON_AddLineFeed();
				}
			}

			rc = 0;
		}
	}
	
	return rc;
}


// output messages (append new-line in text-mode) -----------------------------
//
int SYSs_MsgOut( const char *format, ... )
{
	//NOTE:
	// in text-mode this function always
	// prints a new-line at the end
	// of the string, regardless of whether
	// the string is terminated by one or not.
	// in graphics-mode the behavior depends
	// on whether the string is terminated by
	// a new-line. see SL_MsgOut().

	int		rc;
	va_list	ap;
	va_start( ap, format );

	rc = SL_MsgOut( format, ap );

	va_end( ap );
	return rc;
}


// put messages (output them and flush afterwards) ----------------------------
//
int SYSs_MsgPut( const char *format, ... )
{
	//NOTE:
	// in graphics-mode this function
	// is IDENTICAL to SYSs_MsgOut().

	//NOTE:
	// in text-mode this function does
	// not print a new-line at the end
	// of the string, but flushes stdout
	// after output. this can be used
	// to continue printing in the same
	// line.

	int		rc;
	va_list	ap;
	va_start( ap, format );

	append_nl = FALSE;

	rc = SL_MsgOut( format, ap );

	if ( TextModeActive ) {
		fflush( stdout );
	}

	append_nl = TRUE;

	va_end( ap );
	return rc;
}


// make sure that messages can be seen by the user ----------------------------
//
void SYSs_MsgPrompt()
{
	// not necessary
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( SL_MSG )
{
	// make sure log is on by default at the earliest
	// time possible (even before console init)
	AUX_ENABLE_CONSOLE_MESSAGE_LOG = 7;
}



