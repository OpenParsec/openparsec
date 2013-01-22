/*
 * PARSEC - Utils Module
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:26:07 $
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

// C library includes
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// local module header
#include "sl_util_sv.h"
#include "utl_logfile.h"


// emulate missing or differently named c library functions -------------------
//
int stricmp( const char *s1, const char *s2 )
{
        return strcasecmp( s1, s2 );
}


int strnicmp( const char *s1, const char *s2, int len )
{
        return strncasecmp( s1, s2, len );
}


char *strlwr( char *str )
{
        for ( char *scan = str; *scan; scan++ ) {
                *scan = tolower( *scan );
        }

        return str;
}


char *strupr( char *str )
{
        for ( char *scan = str; *scan; scan++ ) {
                *scan = toupper( *scan );
        }

        return str;
}


// cleanup function -----------------------------------------------------------
//
void SYSs_CleanUp()
{
	// flush all logfiles 
	UTL_FlushRegisteredLogfiles();
}


// signal handler to ensure correct termination of server ---------------------
//
void SignalHandler(int signum)
{
	SYSs_CleanUp();
}


// install the signal handlers ------------------------------------------------
//
void SYSs_InstallSignalHandlers()
{
	signal(SIGINT, SignalHandler );
	signal(SIGTERM, SignalHandler );
}





