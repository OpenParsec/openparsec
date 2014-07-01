/*
 * PARSEC - Standard Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:35 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-1999
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

// local module header
#include "con_std.h"


// strings of standard commands -----------------------------------------------
//
std_command_s std_commands[] = {

	{ "append",				0,	1	},

	{ "base",				0,	0	},
	{ "bind",               0,	2	},
	{ "build",              0,	0	},

	{ "cat",                0,	1	},
	{ "classinfo",          0,	1	},
	{ "clear",				0,	0	},
	{ "close",				0,	0	},
	{ "cls",                0,	0	},
	{ "colanim",            0,	1	},
	{ "compile",            0,	1	},
	{ "connect",            0,	1	},
	{ "create",				0,	1	},

	{ "dec",				0,	0	},
	{ "disable",            0,	0	},
	{ "disablemacros",      0,	0	},
	{ "disconnect",         0,	0	},
	{ "display",            0,	1	},

	{ "enable",				0,	0	},
	{ "enablemacros",       0,	0	},
	{ "exit",               0,	0	},
	{ "exitgame",			0,  0   },

	{ "faceinfo",			0,	1	},
	{ "flashblue",			0,	0	},
	{ "flashwhite",         0,	0	},

	{ "glbatch",			0,	1	},

	{ "hex",				0,	0	},
	{ "hide",               0,	0	},

	{ "key",				0,	2	},

	{ "listbindings",		0,	0	},
	{ "listclasses",        0,	0	},
	{ "listcommands",		0,	0	},
	{ "listdata",			0,	1	},
	{ "listdemos",			0,	0	},
	{ "listgamefunckeys",   0,	0	},
	{ "listintcommands",	0,	0	},
	{ "listsaymacros",      0,	0	},
	{ "listtypes",          0,	0	},
	{ "load",               0,	1	},
	{ "login",				0,	0	},
	{ "logout",				0,	0	},
	{ "ls",                 0,	0	},

	{ "name",				0,	1	},
	{ "net.subsys",         0,	1	},

	{ "objectclear",		0,	0	},
	{ "objectcount",		0,	0	},
	{ "objectinfo",			0,	1	},
	{ "oct",                0,	0	},
	{ "open",               0,	1	},

	{ "path.anim.intro",	0,	1	},
	{ "path.astream.intro",	0,	1	},
	{ "path.astream.menu",  0,	1	},
	{ "path.audiostreams",  0,	1	},
	{ "play",               0,	1	},
	{ "prop",               0,	1	},
	{ "propc",              0,	1	},
	{ "propo",              0,	1	},

	{ "quit",				0,	0	},

	{ "recdem",				0,	1	},
	{ "record",             0,	1	},
	{ "repinfo",			0,	0	},
	{ "repstop",			0,	0	},
	{ "rescan",				0,	0	},

	{ "say",                0,	0	},
	{ "set",                0,	0	},
	{ "setorigin",          0,	0	},
	{ "shader.def",        	0,	1	},
	{ "shader.set",        	0,	1	},
	{ "ships",              0,	0	},
	{ "showposition",       0,	0	},

	{ "talk",				0,	0	},
	{ "typeinfo",           0,	1	},

	{ "vid.listmodes",      0,	0	},
	{ "vid.setmode",        0,	1	},
	{ "vid.subsys",         0,	1	},

	{ "workdir",            0,	1	},
	{ "write",              0,	1	},
	{ "aud.conf",           0,	1	},
	{ "inp.conf",           0,	1	},
	{ "netver",			0,	1	},
};

#define NUM_STD_COMMANDS	CALC_NUM_ARRAY_ENTRIES( std_commands )


// number of standard commands ------------------------------------------------
//
int num_std_commands = NUM_STD_COMMANDS;


// registration function precalculates string lengths -------------------------
//
REGISTER_MODULE( CON_STD )
{
	// ensure table and constants are of consistent length
	ASSERT( NUM_STD_COMMANDS == NUM_STD_COMMAND_CONSTANTS );

	// init string length table
	for ( unsigned int cid = 0; cid < NUM_STD_COMMANDS; cid++ )
		std_commands[ cid ].commlen = strlen( std_commands[ cid ].command );
}



