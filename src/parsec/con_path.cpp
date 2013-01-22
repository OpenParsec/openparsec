/*
 * PARSEC - Path and Package Registration
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:23 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   2001
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

// local module header
#include "con_path.h"

// proprietary module headers
#include "con_arg.h"
#include "con_com.h"
#include "con_main.h"
#include "sys_file.h"



// string constants -----------------------------------------------------------
//
//static char a_b[]		= "";


// key table for package command ----------------------------------------------
//
key_value_s package_key_value[] = {

	{ "register",	NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "mod",		NULL,	KEYVALFLAG_NONE				},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_PACKAGE_REGISTER,
	KEY_PACKAGE_MOD,
};


// console command for package registration ("package") -----------------------
//
PRIVATE
int Cmd_PACKAGE( char *paramstr )
{
	//NOTE:
	//CONCOM:

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( package_key_value, paramstr ) )
		return TRUE;

	// get optional mod name parameter
	char *modname = package_key_value[ KEY_PACKAGE_MOD ].value;
	
	if ( package_key_value[ KEY_PACKAGE_REGISTER ].value != NULL ) {

		if ( !SYS_RegisterPackage( package_key_value[ KEY_PACKAGE_REGISTER ].value, 0, modname ) ) {
			CON_AddLine( "package registration failed." );
			return TRUE;
		}

	} else {

		CON_AddLine( "package name missing." );
		return TRUE;
	}

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( CON_PATH )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "package" command
	regcom.command	 = "package";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_PACKAGE;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );
}



