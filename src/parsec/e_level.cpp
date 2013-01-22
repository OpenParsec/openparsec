/*
 * PARSEC - Level Management
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:34 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001
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

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "e_level.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_ext.h"
#include "con_info.h"
#include "con_main.h"
#include "net_defs.h"
#include "obj_ctrl.h"
#include "obj_clas.h"
#include "sys_date.h"
#include "sys_file.h"
#include "sys_path.h"

// constants ------------------------------------------------------------------
//
#define MAX_LEVEL_NAME 16

// module local functions -----------------------------------------------------
//
PRIVATE int LVL_SaveObjectsFromList( FILE* fp, GenObject* walkobjs, dword objclassid );

// module local variables -----------------------------------------------------
//
PRIVATE int		cur_int_level = -1;					// # of internal level ( == nebulaid )
PRIVATE char	cur_levelname[ MAX_LEVEL_NAME ];	// name of level


// flags specifying loadin/saving/clearing behaviour of object classes -------
//
#define LEVEL_OBJCLASS_SAVE		0x0001
#define LEVEL_OBJCLASS_LOAD		0x0002
#define LEVEL_OBJCLASS_CLEAR	0x0004

// object class descriptions for level loading/clearing/saving ---------------
//
struct level_objclass_s {
	
	const char* classname;	// name of the object class
	int		flags;
};


// all object classes to be handled by the level code -------------------------
//
PRIVATE level_objclass_s level_objclasses[] = {

	{ "teleporter",		LEVEL_OBJCLASS_SAVE | LEVEL_OBJCLASS_LOAD | LEVEL_OBJCLASS_CLEAR  },
	{ "telep_exit",													LEVEL_OBJCLASS_CLEAR  },
	{ "stargate",		LEVEL_OBJCLASS_SAVE | LEVEL_OBJCLASS_LOAD | LEVEL_OBJCLASS_CLEAR  },

	{ NULL,				0 }
};

// save all objects from a specific list --------------------------------------
//
PRIVATE
int LVL_SaveObjectsFromList( FILE* fp, GenObject* walkobjs, dword objclassid )
{
	ASSERT( fp != NULL );
	ASSERT( objclassid < (dword)NumObjClasses );
	
	int savecount = 0;
	
	for ( ; walkobjs; walkobjs = walkobjs->NextObj ) {
		if ( walkobjs->ObjectClass == objclassid ) {
			Vector3 pos;
			FetchTVector( walkobjs->ObjPosition, &pos );
			
			char *objclassname = ObjectInfo[ objclassid ].name;
			
			fprintf( fp, "summon class %s origin ( %f %f %f ) space ww frame ( %f %f %f %f %f %f %f %f %f )\n", 
					 objclassname, pos.X, pos.Y, pos.Z,
					 walkobjs->ObjPosition[ 0 ][ 0 ], walkobjs->ObjPosition[ 1 ][ 0 ], walkobjs->ObjPosition[ 2 ][ 0 ],
					 walkobjs->ObjPosition[ 0 ][ 1 ], walkobjs->ObjPosition[ 1 ][ 1 ], walkobjs->ObjPosition[ 2 ][ 1 ],
					 walkobjs->ObjPosition[ 0 ][ 2 ], walkobjs->ObjPosition[ 1 ][ 2 ], walkobjs->ObjPosition[ 2 ][ 2 ] );
			WritePropList( fp, walkobjs->ObjectNumber );
			fprintf( fp, ";----------------------------------------------------\n" );
			
			savecount++;
		}
	}
	
	return savecount;
}


// save a level to a console script -------------------------------------------
//
int LVL_SaveLevel( const char* levelname )
{
	ASSERT( levelname != NULL );
	
	int savecount = 0;
	
	// write console script containing summon commands for all teleporters found
	FILE *fp = fopen( levelname, "w" );
	if ( fp != NULL ) {
		
		fprintf( fp, "; parsec level script -----------------------------------\n" );
		fprintf( fp, "; automatically generated on %s;\n", SYS_SystemTime() );
		
		for( int objclassnameindex = 0; level_objclasses[ objclassnameindex ].classname != NULL; objclassnameindex++ ) {
			
			// determine, whether we want to save this object class
			if ( level_objclasses[ objclassnameindex ].flags & LEVEL_OBJCLASS_SAVE ) {

				dword objclassid = OBJ_FetchObjectClassId( level_objclasses[ objclassnameindex ].classname );
				if ( ( objclassid != CLASS_ID_INVALID ) && ( objclassid < (dword)NumObjClasses ) ) {
					savecount =  LVL_SaveObjectsFromList( fp, FetchFirstShip(),		objclassid );
					savecount += LVL_SaveObjectsFromList( fp, FetchFirstLaser(),	objclassid );
					savecount += LVL_SaveObjectsFromList( fp, FetchFirstMissile(),	objclassid );
					savecount += LVL_SaveObjectsFromList( fp, FetchFirstExtra(),	objclassid );
					savecount += LVL_SaveObjectsFromList( fp, FetchFirstCustom(),	objclassid );
				}
			}
		}
		
		fclose( fp );
		
		CON_AddLine( "levelfile sucessfully written" );
	} else {
		CON_AddLine( "could not open levelfile for writing" );
	}

	return savecount; 
}

// load a level from a console script -----------------------------------------
//
PUBLIC
int LVL_LoadLevel( const char* levelname )
{
	ASSERT( levelname != NULL );

	// level has changed
	if ( stricmp( cur_levelname, levelname ) != 0 ) {
		
		// kill all existing level objects
		for( int objclassnameindex = 0; level_objclasses[ objclassnameindex ].classname != NULL; objclassnameindex++ ) {

			// determine, whether we want to clear this object class
			if ( level_objclasses[ objclassnameindex ].flags & LEVEL_OBJCLASS_CLEAR ) {
				
				dword objclass = OBJ_FetchObjectClassId( level_objclasses[ objclassnameindex ].classname );
				if ( ( objclass != CLASS_ID_INVALID ) && ( objclass < (dword)NumObjClasses ) ) {
					int killcount = KillClassInstances( objclass );
				}
			}
		}
		
		// HACK: set NetConnected to NETWORK_GAME_OFF to allow summon command
		int Old_NetConnected = NetConnected;
		NetConnected = NETWORK_GAME_OFF;
		
			int old_aux_disable_level_console_messages = AUX_DISABLE_LEVEL_CONSOLE_MESSAGES;
			AUX_DISABLE_LEVEL_CONSOLE_MESSAGES = 1;

				// must be done to ensure the file can be found independently of
				// whether it is read from a package or from a real directory
				char *path = SYSs_ProcessPathString( (char *) levelname );
				
				// execute the console script that contains the teleporter level
				ExecConsoleFile( path, FALSE );
			
			AUX_DISABLE_LEVEL_CONSOLE_MESSAGES = old_aux_disable_level_console_messages;
		
		// restore old NetConnected status
		NetConnected = Old_NetConnected;
		
		// store the new levelname
		strcpy( cur_levelname, levelname );
		cur_int_level = -1;
	}
	
	return TRUE;
}

// internal level loading function --------------------------------------------
//
PUBLIC
int LVL_LoadIntLevel( int level )
{
    if(NET_ConnectedGMSV())
        return 0;
	int rc = 0;
	if ( cur_int_level != level ) {
		char levelname[ 128 ];
		sprintf( levelname, "intlev%d.con", level );
		rc = LVL_LoadLevel( levelname );
		
		cur_int_level = level;
	}
	return rc;
}


// console command for loading a level ----------------------------------------
//
PRIVATE
int Cmd_LEVEL_LOAD( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// level_load_command	::= 'level.load' levelname_spec
	// levelname_spec	    ::= <string>
	
	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );
	
	const char *levelname = GetStringBehindCommand( paramstr, FALSE );
	if ( levelname == NULL ) {
		CON_AddLine( "no level name specified" );
		return TRUE;
	}
	
	// must be done to ensure the file can be found independently of
	// whether it is read from a package or from a real directory
	char *path = SYSs_ProcessPathString( (char *) levelname );
	
	// check whether the file exists
	FILE* fp = SYS_fopen( path, "r" );
	if ( fp == NULL ) {
		char szBuffer[ 256 ];
		sprintf( szBuffer, "level file %s could not be found", levelname );
		CON_AddLine( szBuffer );
		return TRUE;	
	} else {
		SYS_fclose( fp );
	}
	
	// load the level
	LVL_LoadLevel( levelname );
	
	return TRUE;
}

// console command for saving a level -----------------------------------------
//
PRIVATE
int Cmd_LEVEL_SAVE( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// level_save_command	::= 'level.save' levelname_spec
	// levelname_spec	    ::= <string>
	
	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );
	
	const char *levelname = GetStringBehindCommand( paramstr, FALSE );
	if ( levelname == NULL ) {
		CON_AddLine( "no level name specified" );
		return TRUE;
	}
	
	// save the level
	LVL_SaveLevel( levelname );
	
	return TRUE;
}

// console command for getting info on a level --------------------------------
//
PRIVATE
int Cmd_LEVEL_INFO( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// level_info_command	::= 'level.info' 
	
	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	char szBuffer[ 512 ];
	if ( cur_int_level != -1 ) {
		sprintf( szBuffer, "current internal level #: %d\ncurrent level name: %s", cur_int_level, cur_levelname );
	} else {
		sprintf( szBuffer, "current level name: %s", cur_levelname );
	}

	CON_AddMessage( szBuffer );

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE_INIT( E_LEVEL )
{
	user_command_s regcom;
	
	// register "level.load" command
	memset( &regcom, 0, sizeof( user_command_s ) );
	regcom.command	 = "level.load";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_LEVEL_LOAD;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "level.save" command
	memset( &regcom, 0, sizeof( user_command_s ) );
	regcom.command	 = "level.save";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_LEVEL_SAVE;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "level.info" command
	memset( &regcom, 0, sizeof( user_command_s ) );
	regcom.command	 = "level.info";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_LEVEL_INFO;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );
	
	
	// init the current level
	strcpy( cur_levelname, "" );
	cur_int_level = -1;
}

// module deregistration function ---------------------------------------------
//
REGISTER_MODULE_KILL( E_LEVEL )
{
}

