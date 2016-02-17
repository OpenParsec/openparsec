/*
 * PARSEC - Command Completion
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:40 $
 *
 * Orginally written by:
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1998-1999
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
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
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#ifdef PARSEC_CLIENT
	#include "aud_defs.h"
#endif // PARSEC_CLIENT

// local module header
#include "con_tab.h"

// proprietary module headers
#ifdef PARSEC_SERVER

	#include "con_act_sv.h"
	#include "con_aux_sv.h"
	#include "con_com_sv.h"
	#include "con_ext_sv.h"
	#include "con_int_sv.h"
	#include "con_main_sv.h"
	#include "con_std_sv.h"

#else // !PARSEC_SERVER

	#include "con_act.h"
	#include "con_aux.h"
	#include "con_com.h"
	#include "con_conf.h"
	#include "con_ext.h"
	#include "con_int.h"
	#include "con_main.h"
	#include "con_std.h"

#endif // PARSEC_SERVER



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];

#if ( PASTE_STR_LEN < MAX_CONSOLE_LINE_LENGTH )
	#error "CON_TAB::paste_str too short!"
#endif


// maximum number of possible completions that will be shown ------------------
//
#define MAX_COMPLETIONS		64


// list of possible completions found -----------------------------------------
//
static const char*	found_names[ MAX_COMPLETIONS ];
static int			found_param[ MAX_COMPLETIONS ];


// commands to check in current run -------------------------------------------
//
enum {

	CHECK_STDCOMMS,
	CHECK_USRCOMMS,
	CHECK_INTCOMMS,
	CHECK_EXTCOMMS,
	CHECK_ACTCOMMS,
#ifdef PARSEC_CLIENT
	CHECK_COLCOMMS,
#endif // PARSEC_CLIENT

	// must be last!!
	CHECK_NUM_COMMS
};

static int		num_commands[ CHECK_NUM_COMMS ];


// <tab> key pressed in console -----------------------------------------------
//
void CompleteCommand()
{
	// skip prompt
	char* curinput = &con_lines[ con_bottom ][ PROMPT_SIZE ];
	
	// only complete commands starting in first column
	if ( ( curinput[ 0 ] == ' ' ) || ( curinput[ 0 ] == 0 ) )
		return;

#ifdef PARSEC_CLIENT
	int   skiplen  = PROMPT_SIZE;
	// handle talkmode
	if ( con_in_talk_mode ) {

		// only complete commands
		if ( curinput[ 0 ] != talk_escape_char ) {

			if ( !AUX_DISABLE_TALK_ESCAPE_ON_TAB ) {
				int maxlen = MAX_CONSOLE_LINE_LENGTH - PROMPT_SIZE - 1;
				strncpy( paste_str, curinput, maxlen );
				paste_str[ maxlen ] = 0;
				curinput[ 0 ] = talk_escape_char;
				strcpy( curinput + 1, paste_str );
			} else {
				return;
			}
		}

		// skip command escape
		curinput++;
		skiplen++;
	}

#endif // PARSEC_CLIENT

	int inputlen = (int)strlen( curinput );

	// fetch current numbers for all types
	num_commands[ CHECK_STDCOMMS ] = num_std_commands;
	num_commands[ CHECK_USRCOMMS ] = num_user_commands;
	num_commands[ CHECK_INTCOMMS ] = num_int_commands;
	num_commands[ CHECK_EXTCOMMS ] = num_external_commands;
	num_commands[ CHECK_ACTCOMMS ] = ACM_NUM_COMMANDS;
#ifdef PARSEC_CLIENT
	num_commands[ CHECK_COLCOMMS ] = num_col_commands;
#endif // PARSEC_CLIENT

	// check all command types
	int numfound = 0;
	for ( int cmdtyp = 0; cmdtyp < CHECK_NUM_COMMS; cmdtyp++ ) {

		// check all commands of current type
		for ( int curcmd = 0; curcmd < num_commands[ cmdtyp ]; curcmd++ ) {

			if ( ( cmdtyp == CHECK_ACTCOMMS ) && !ACMCALLABLE( curcmd ) )
				continue;

			const char* name;
			int   para;

			switch ( cmdtyp ) {

				case CHECK_STDCOMMS:
					name = CMSTR( curcmd );
					para = CMARG( curcmd );
					break;

				case CHECK_USRCOMMS:
					name = USRSTR( curcmd );
					para = USRARG( curcmd );
					break;

				case CHECK_INTCOMMS:
					name = ICMSTR( curcmd );
					para = 1;
					break;

				case CHECK_EXTCOMMS:
					name = ECMSTR( curcmd );
					para = 0;
					break;

				case CHECK_ACTCOMMS:
					name = ACMSTR( curcmd );
					para = 0;
					break;
#ifdef PARSEC_CLIENT
				case CHECK_COLCOMMS:
					name = col_comms[ curcmd ].cmd;
					para = -1;
					break;
#endif // PARSEC_CLIENT
			}

			// count number of equal chars
			int cpos = 0;
			for ( cpos = 0; cpos < inputlen; cpos++ )
				if ( curinput[ cpos ] != name[ cpos ] )
					break;

			if ( cpos == inputlen ) {

				// we either found a valid command or at least
				// a partially complete one

				if ( numfound >= MAX_COMPLETIONS )
					break;

				found_names[ numfound ] = name;
				found_param[ numfound ] = para;
				numfound++;
			}
		}
	}

	if ( numfound == 1 ) {

		// we've found just a single valid completion, so let's take it
		strcpy( curinput, found_names[ 0 ] );

		// special case for color commands
		if ( found_param[ 0 ] == -1 ) {
			int len = (int)strlen( curinput );
			curinput[ len - 2 ] = 0;
			curinput[ len - 1 ] = 0;
			found_param[ 0 ] = 0;
		}

		// if command has parameters automatically append space
		if ( found_param[ 0 ] > 0 ) {
			strcat( curinput, " " );
		}

	} else {

#ifdef PARSEC_CLIENT
		// we beep if we have found multiple possible
		// completions or none at all
		AUD_Select2();
#endif // PARSEC_CLIENT

		// list possible completions and print common prefix
		if ( numfound > 1 ) {

			// save the original input line
			strcpy( paste_str, con_lines[ con_bottom ] );

			// overwrite old line in talkmode
			if ( con_in_talk_mode ) {
				con_bottom = con_talk_line;
				CON_DisableLineFeed();
			}

			const char *common = found_names[ 0 ];
			int   comlen = (int)strlen( common );

			// print the list of completions
			for ( int cnum = 0; cnum < numfound; cnum++ ) {

				const char *curname = found_names[ cnum ];

				// maintain common prefix length
				int cpos = 0;
				for ( cpos = 0; cpos < comlen; cpos++ )
					if ( curname[ cpos ] != common[ cpos ] )
						break;
				if ( cpos < comlen )
					comlen = cpos;

				CON_AddLine( curname );
			}
#ifdef PARSEC_CLIENT
			// print the common prefix
			strncpy( paste_str + skiplen, common, comlen );
			paste_str[ skiplen + comlen ] = 0;
			CON_AddLine( paste_str );
#elif PARSEC_SERVER
			extern void	EraseConLine( int line );
			// output supplied line
			EraseConLine( con_bottom );
			strcpy( con_lines[ con_bottom ], paste_str );
#endif // PARSEC_CLIENT
		}
	}

	// set cursor position to end of line
	cursor_x = (int)strlen( con_lines[ con_bottom ] + PROMPT_SIZE );
}



