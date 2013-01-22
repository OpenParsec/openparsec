/*
 * PARSEC - SERVER Verbose Server Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:47 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001
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

// rendering subsystem
//#include "r_patch.h"
//#include "r_supp.h"

// subsystem headers
#include "net_defs.h"

// local module header
#include "con_aux_sv.h"

// proprietary module headers
#include "con_com_sv.h"
#include "con_int_sv.h"
#include "g_main_sv.h"
//#include "e_supp.h"
//#include "net_csdf.h"
#include "net_ports.h"
#include "e_gameserver.h"

// flags
#define REGISTER_VERBOSE_SV_COMMANDS
//#define EXCLUDE_SPECIAL_SV_FLAGS


//NOTE:
// this module is only loosely related to the CONS_AUX.H
// header. that is, it may be omitted from linking
// entirely, if there should be no verbose SV commands.
// apart from this, project functionality will not be
// reduced. CONS_AUX.H must always be part of the project.


/*
//FIXME:
// perform remote event syncing for certain SV flags -------------------------
//
#define DEF_RMEV_STATESYNC( f, k, v ) \
PUBLIC \
void RESync##f() \
{ \
	if ( !NET_RmEvAllowed( RE_STATESYNC ) ) \
		return; \
	NET_RmEvStateSync( (k), (v) ); \
}

//FIXME_GAMESERVER: these fields are all part of the server config object

DEF_RMEV_STATESYNC( NebulaId,	RMEVSTATE_NEBULAID,		AUXDATA_BACKGROUND_NEBULA_ID	);
DEF_RMEV_STATESYNC( Amazing,	RMEVSTATE_AMAZING,		AUXDATA_SKILL_AMAZING_TIME		);
DEF_RMEV_STATESYNC( Brilliant,	RMEVSTATE_BRILLIANT,	AUXDATA_SKILL_BRILLIANT_TIME	);
DEF_RMEV_STATESYNC( KillLimit,	RMEVSTATE_KILLLIMIT,	AUX_KILL_LIMIT_FOR_GAME_END		);

*/

void G_RealizeGameVars()
{
	TheGame->RealizeVariables();
}

// for registration of SV flags/data as integer variable commands -----------------
//
PRIVATE
int_command_s verbose_sv_commands[] = {
	
	{ 0x80,	"sv.package_scripts",						0, 1,		&SV_PACKAGE_SCRIPTS,						NULL,					NULL,		TRUE	},
	{ 0x80,	"sv.console.level_messages",				0, 1,		&SV_CONSOLE_LEVEL_MESSAGES,					NULL,					NULL,		TRUE	},
	{ 0x80,	"sv.package_scripts.list",					0, 1,		&SV_PACKAGE_SCRIPT_LISTING,					NULL,					NULL,		TRUE	},
	{ 0x80,	"sv.class_map_table.disable_reset",			0, 1,		&SV_CLASS_MAP_TABLE_DISABLE_RESET,			NULL,					NULL,		FALSE	},
	{ 0x80,	"aux.netcode.flags",						0, 16,		&AUX_NETCODE_FLAGS,							NULL,					NULL,		0		},
#ifdef ENABLE_CHEAT_COMMANDS
	{ 0x80, "sv.cheats.energy_checks",					0, 1,   	&SV_CHEAT_ENERGY_CHECKS,					NULL,					NULL,		0		},
	{ 0x80,	"sv.cheats.device_checks",					0, 1,		&SV_CHEAT_DEVICE_CHECKS,					NULL,					NULL,		0		},
#endif // ENABLE_CHEAT_COMMANDS
	{ 0x80,	"aux_enable_object_overloading",			0, 1,		&SV_OBJECTCLASSES_OVERLOADING,				NULL,					NULL,		FALSE	},
	{ 0x80,	"sv.objectclasses.overloading",				0, 1,		&SV_OBJECTCLASSES_OVERLOADING,				NULL,					NULL,		FALSE	},

	{ 0x80,	"sv.game.extras.autocreate",				0, 1,		&SV_GAME_EXTRAS_AUTOCREATE,					NULL,					NULL,		TRUE	},
	{ 0x80, "sv.game.extras.max",						0, 100, 	&SV_GAME_EXTRAS_MAXNUM,						NULL,					NULL,		20		},
	{ 0x80, "sv.game.extras.testplace",					0, 1,		&SV_GAME_EXTRAS_TESTPLACE,					NULL,					NULL,		0		},
	{ 0x80, "sv.game.killlimit",						1, 4096,	&SV_GAME_KILLLIMIT,							G_RealizeGameVars,					NULL,		DEFAULT_KILL_LIMIT },
	{ 0x80, "sv.game.timelimit",						1, 86400,	&SV_GAME_TIMELIMIT,							G_RealizeGameVars,		NULL,		DEFAULT_GAME_TIMELIMIT_SEC			},
	{ 0x80, "sv.game.restart.timeout",					0, 600,		&SV_GAME_RESTART_TIMEOUT,					G_RealizeGameVars,		NULL,		DEFAULT_RESTART_TIMELIMIT_SEC		},

	{ 0x80,	"sv.masterserver.sendheartbeat",			0, 1,		&SV_MASTERSERVER_SENDHEARTBEAT,				NULL,					NULL,		TRUE	},
	{ 0x80, "sv.serverid",								0, 0xFFFF,	&SV_SERVERID,								NULL,					NULL,		0		},
	{ 0x80, "sv.fedid",									0, 0xFFFF,	&SV_FEDID,									NULL,					NULL,		0		},
	{ 0x80, "sv.netconf.port",							1024, 0xFFFF,&SV_NETCONF_PORT,							NULL,					NULL,		DEFAULT_GAMESERVER_UDP_PORT			},

	{ 0x80, "sv.debug.netstream.dump",					0, 15,		&SV_DEBUG_NETSTREAM_DUMP,					NULL,					NULL,		0 },
	{ 0x80, "sv.debug.console.messages",				0, 1,		&SV_DEBUG_CONSOLE_MESSAGES,					NULL,					NULL,		0 },

	{ 0x80, "sv.game.workdir.force_current",			0, 1,		&SV_GAME_WORKDIR_FORCE_CURRENT,				NULL,					NULL,		0 },
	{ 0x80, "sv.game.filepath.force_curent",			0, 1,		&SV_GAME_FILEPATH_FORCE_CURRENT,			NULL,					NULL,		0 },

	{ 0x80, "sv.game.extras.maxnum",					0, 1000,	&SV_GAME_EXTRAS_MAXNUM,						NULL,					NULL,		0 },
	{ 0x80, "aux_disable_package_data_files",			0, 1,		&AUX_DISABLE_PACKAGE_DATA_FILES	,			NULL,					NULL,		0 },
	{ 0x80, "aux_disable_package_scripts",				0, 1,		&AUX_DISABLE_PACKAGE_SCRIPTS,				NULL,					NULL,		0 },
	
};

#define NUM_VERBOSE_SV_COMMANDS	CALC_NUM_ARRAY_ENTRIES( verbose_sv_commands )


// dummy command handler for all commands that are invalid for the server
//
int Cmd_DUMMY( char *classstr )
{
	return TRUE;
}


// register all commands that are NOP for the server, to allow sharing console scripts
// between server and client
PRIVATE
void RegisterServerNOPCommands()
{

	const char* szNOPCommands[] = {	"shipdesc",
									"classiter", 
									"classpart", 
									"shader.set",
									NULL };

	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	for( int nNOPCommand = 0; szNOPCommands[ nNOPCommand ] != NULL; nNOPCommand++ ) {
		// register dummy command
		regcom.command = szNOPCommands[ nNOPCommand ];
		regcom.numparams = 1;
		regcom.execute	 = Cmd_DUMMY;
		regcom.statedump = NULL;
		CON_RegisterUserCommand( &regcom );
	}
}


// module registration function -----------------------------------------------
//
void CON_AUX_SV_Register()
{
	ASSERT( NUM_VERBOSE_SV_COMMANDS <= AUX_ARRAY_NUM_ENTRIES_USED );

#ifdef REGISTER_VERBOSE_SV_COMMANDS
    unsigned int csv;
	// register SV flags
	for ( csv = 0; csv < NUM_VERBOSE_SV_COMMANDS; csv++ ) {

		//NOTE:
		// bit 7 of the persistence field is used to denote
		// special SV flags that may be excluded from registration.

		#ifdef EXCLUDE_SPECIAL_SV_FLAGS

			if ( verbose_sv_commands[ csv ].persistence & 0x80 )
				continue;

		#endif

		// destructive but doesn't matter here
		verbose_sv_commands[ csv ].persistence &= ~0x80;

		CON_RegisterIntCommand( verbose_sv_commands + csv );
	}

#endif // REGISTER_VERBOSE_SV_COMMANDS

	// init sv flags with default value
	for( csv = 0; csv < NUM_VERBOSE_SV_COMMANDS; csv++ ) {

		int_command_s* cmd = &verbose_sv_commands[ csv ];

		if ( ( cmd->bmin <= cmd->default_val ) && ( cmd->bmax >= cmd->default_val ) ) {
			*cmd->intref = cmd->default_val;

			// do not call realize functions if the server is a master
			if ( cmd->realize != NULL && !TheServer->GetServerIsMaster())
				cmd->realize();

		} else {
			// default value specified was not inside min/max bounds
			ASSERT( FALSE );

			// default to min
			*cmd->intref = cmd->bmin;

			// do not call realize functions if the server is a master
			if ( cmd->realize != NULL && !TheServer->GetServerIsMaster())
				cmd->realize();
		}
	}

	// register all commands that are NOPs for the server
	RegisterServerNOPCommands();
}
