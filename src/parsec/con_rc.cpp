/*
 * PARSEC - RC Script Saving
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:23 $
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
#include "vid_defs.h"

// local module header
#include "con_rc.h"

// proprietary module headers
#include "con_aux.h"
#include "con_int.h"
#include "con_main.h"
#include "net_conn.h"
#include "sys_bind.h"
#include "sys_date.h"
#include "vid_plug.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// name of rc script ----------------------------------------------------------
//
static char rc_script_name[]	= "parsecrc" CON_FILE_EXTENSION;


// read launcher section from rc script into temporary mem --------------------
//
PRIVATE
char *ReadRcLauncherConfig()
{
	FILE *fp = fopen( rc_script_name, "r" );
	if ( fp == NULL )
		return NULL;

	#define MAX_LAUNCHER_CFG_LINES	20

	size_t launchsize = MAX_LAUNCHER_CFG_LINES * ( PASTE_STR_LEN + 1 ) + 1;
	char *launchercfg = (char *) ALLOCMEM( launchsize );
	if ( launchercfg == NULL ) {
		fclose( fp );
		return NULL;
	}

	int linecount = 0;
	int insection = FALSE;

	const char *strbegin = ";<launcher-begin>";
	const char *strend   = ";<launcher-end>";

	char *storeline = launchercfg;
	while ( fgets( paste_str, PASTE_STR_LEN, fp ) != NULL ) {

		if ( !insection ) {
			if ( strncmp( paste_str, strbegin, strlen( strbegin ) ) != 0 )
				continue;
			insection = TRUE;
		}

		strcpy( storeline, paste_str );
		storeline += PASTE_STR_LEN + 1;

		if ( strncmp( paste_str, strend, strlen( strend ) ) == 0 ) {
			break;
		}

		if ( ++linecount >= MAX_LAUNCHER_CFG_LINES ) {
			char *prevline = storeline - PASTE_STR_LEN - 1;
			strcpy( prevline, strend );
			strcat( prevline, "\n" );
			break;
		}
	}
	storeline[ 0 ] = 0;

	fclose( fp );
	return launchercfg;
}


// write rc script header -----------------------------------------------------
//
PRIVATE
void WriteRcHeader( FILE *fp )
{
	ASSERT( fp != NULL );

	fprintf( fp, "; parsec rc script -----------------------------------\n" );
	fprintf( fp, "; automatically generated on %s\n", SYS_SystemTime() );
}


// write launcher config area back to rc script -------------------------------
//
PRIVATE
void WriteRcLauncherConfig( FILE *fp, char *launchercfg )
{
	ASSERT( fp != NULL );

	// write sound on/off state
	if ( SoundDisabled ) {
		fprintf( fp, "; sound off\n" );
	} else {
		fprintf( fp, "; sound on\n" );
	}

	// write joystick on/off state
	if ( JoystickDisabled ) {
		fprintf( fp, "; joystick off\n" );
	} else {
		fprintf( fp, "; joystick on\n" );
	}

	fprintf( fp, "\n" );

	// return if no launcher config area
	if ( launchercfg == NULL )
		return;

	// preserve launcher config area
	const char *storeline = launchercfg;
	for ( ; storeline[ 0 ] != 0; storeline += PASTE_STR_LEN + 1 ) {
		fprintf( fp, "%s", storeline );
	}

	if ( storeline != launchercfg ) {
		fprintf( fp, "\n" );
	}
}


// write player name to rc script ---------------------------------------------
//
PRIVATE
void WriteRcPlayerName( FILE *fp )
{
	ASSERT( fp != NULL );

	fprintf( fp, "name %s\n\n", LocalPlayerName );
}


// write user commands to rc script -------------------------------------------
//
PRIVATE
void WriteRcUsrCommands( FILE *fp )
{
	ASSERT( fp != NULL );

	if ( !AUX_SAVE_PERSISTENT_USRCOMMANDS )
		return;

	//TODO:

//	fprintf( fp, "\n" );
}


// write int commands to rc script --------------------------------------------
//
PRIVATE
void WriteRcIntCommands( FILE *fp )
{
	ASSERT( fp != NULL );

	if ( !AUX_SAVE_PERSISTENT_INTCOMMANDS )
		return;

	// write state of persistent integer variable commands
	for ( int cid = 0; cid < num_int_commands; cid++ ) {

		// check persistence flag
		if ( int_commands[ cid ].persistence & 0x01 ) {

			int value = *int_commands[ cid ].intref;

			if ( int_commands[ cid ].intref == &ConsoleHeight )
				value -= 2;

			fprintf( fp, "%s %d\n", int_commands[ cid ].command, value );
		}
	}

	fprintf(fp, "vid.setmode %dx%dx%d %s\n",
			GameScreenRes.width, GameScreenRes.height,
			GameScreenBPP, GameScreenWindowed ? "windowed" : "fullscreen");
	

	// write audio configuration command
	int cursetting = 0x00;
	if ( Op_SoundEffects )
		cursetting |= 0x01;
	if ( Op_Music )
		cursetting |= 0x02;

	switch ( cursetting ) {
		
		case 0x00:
			fprintf( fp, "aud.conf off\n" );
			break;

		case 0x01:
			fprintf( fp, "aud.conf sfx\n" );
			break;

		case 0x02:
			fprintf( fp, "aud.conf music\n" );
			break;

		case 0x03:
			fprintf( fp, "aud.conf music sfx\n" );
			break;
	}

	// write input configuration command
	cursetting = 0x00;
	if ( Op_Joystick )
		cursetting |= 0x01;
	if ( Op_Mouse )
		cursetting |= 0x02;

	switch ( cursetting ) {
		
		case 0x00:
			fprintf( fp, "inp.conf off\n" );
			break;

		case 0x01:
			fprintf( fp, "inp.conf joystick\n" );
			break;

		case 0x02:
			fprintf( fp, "inp.conf mouse\n" );
			break;
	}

	fprintf( fp, "\n" );
}


// write net.subsys command to rc script --------------------------------------
//
PRIVATE
void WriteNetSubsysCommand( FILE *fp )
{
	ASSERT( fp != NULL );

	//NOTE:
	// this function must be called after WriteRcIntCommands()
	// to ensure that net.interface is before net.subsys in the rc
	// script. This is necessary because net.interface would be useless
	// otherwise, because the networking subsystem has already been initialized
	// before the rc script is parsed. Issuing a net.subsys command after 
	// the net.interface command ensures that the selected interface is actually
	// used by the networking subsystem

	const char *proto = NET_GetCurrentProtocolName();

	if ( strcmp( proto, "none" ) != 0 ) {
	
		fprintf( fp, "net.subsys %s\n\n", proto );
	}

}


// write keybord config to rc script ------------------------------------------
//
PRIVATE
void WriteRcKeybConfig( FILE *fp )
{
	ASSERT( fp != NULL );

#if defined ( SYSTEM_MACOSX )

	fprintf( fp, "keybconf\n\n" );

#endif // SYSTEM_MACOSX

}


// write aux commands to rc script --------------------------------------------
//
PRIVATE
void WriteRcAuxCommands( FILE *fp )
{
	ASSERT( fp != NULL );

	if ( !AUX_SAVE_PERSISTENT_AUX_ARRAY )
		return;

	ASSERT( AUX_ARRAY_NUM_ENTRIES_USED <= MAX_AUX_ENABLING );
	ASSERT( AUXDATA_ARRAY_NUM_ENTRIES_USED <= MAX_AUX_DATA );

	// save aux flags
	for ( int aindx = 0; aindx < AUX_ARRAY_NUM_ENTRIES_USED; aindx++ ) {
		ASSERT( aindx < 1000 );
		if ( aindx < 100 ) {
			DIG2_TO_STR( paste_str, aindx );
		} else {
			DIG3_TO_STR( paste_str, aindx );
		}
		fprintf( fp, ".aux%s %d\n", paste_str, AuxEnabling[ aindx ] );
	}
	fprintf( fp, "\n" );

	// save selected aux data entries
	ASSERT( 19 == ((size_t)&AUXDATA_TMM_MIPMAP_LOD_BIAS-(size_t)AuxData)/sizeof(AuxData[0]) );
	fprintf( fp, ".auxd19 %d\n", AUXDATA_TMM_MIPMAP_LOD_BIAS );
	ASSERT( 20 == ((size_t)&AUXDATA_SCREENSHOT_FORMAT-(size_t)AuxData)/sizeof(AuxData[0]) );
	fprintf( fp, ".auxd20 %d\n", AUXDATA_SCREENSHOT_FORMAT );
	ASSERT( 21 == ((size_t)&AUXDATA_SCREENSHOT_SUBFORMAT-(size_t)AuxData)/sizeof(AuxData[0]) );
	fprintf( fp, ".auxd21 %d\n", AUXDATA_SCREENSHOT_SUBFORMAT );
	ASSERT( 22 == ((size_t)&AUXDATA_MOVIE_FORMAT-(size_t)AuxData)/sizeof(AuxData[0]) );
	fprintf( fp, ".auxd22 %d\n", AUXDATA_MOVIE_FORMAT );
	ASSERT( 23 == ((size_t)&AUXDATA_MOVIE_SUBFORMAT-(size_t)AuxData)/sizeof(AuxData[0]) );
	fprintf( fp, ".auxd23 %d\n", AUXDATA_MOVIE_SUBFORMAT );
	ASSERT( 24 == ((size_t)&AUXDATA_MESSAGE_LIFETIME-(size_t)AuxData)/sizeof(AuxData[0]) );
	fprintf( fp, ".auxd24 %d\n", AUXDATA_MESSAGE_LIFETIME );
	ASSERT( 25 == ((size_t)&AUXDATA_MESSAGE_AREA_SIZE-(size_t)AuxData)/sizeof(AuxData[0]) );
	fprintf( fp, ".auxd25 %d\n", AUXDATA_MESSAGE_AREA_SIZE );
	ASSERT( 26 == ((size_t)&AUXDATA_MOUSE_SENSITIVITY-(size_t)AuxData)/sizeof(AuxData[0]) );
	fprintf( fp, ".auxd26 %d\n", AUXDATA_MOUSE_SENSITIVITY );
	ASSERT( 28 == ((size_t)&AUXDATA_LOD_DISCRETE_GEOMETRY_BIAS-(size_t)AuxData)/sizeof(AuxData[0]) );
	fprintf( fp, ".auxd28 %d\n", AUXDATA_LOD_DISCRETE_GEOMETRY_BIAS );
	ASSERT( 29 == ((size_t)&AUXDATA_SMOOTH_SHIP_CONTROL_FACTOR-(size_t)AuxData)/sizeof(AuxData[0]) );
	fprintf( fp, ".auxd29 %d\n", AUXDATA_SMOOTH_SHIP_CONTROL_FACTOR );
	ASSERT( 30 == ((size_t)&AUXDATA_BACKGROUND_NEBULA_ID-(size_t)AuxData)/sizeof(AuxData[0]) );
	fprintf( fp, ".auxd30 %d\n", AUXDATA_BACKGROUND_NEBULA_ID );

	fprintf( fp, "\n" );
}


// write persistent commands to rc script -------------------------------------
//
void WritePersistentCommands()
{
	//NOTE:
	// if something in this function fails, no
	// or an incomplete rc script will be written.

	if ( AUX_DISABLE_RC_SCRIPT_SAVING )
		return;

	// avoid writing the state when it hasn't been
	// completely initialized yet
	extern int console_init_done;
	if ( !console_init_done )
		return;

	// read launcher section into temp mem
	char *launchercfg = ReadRcLauncherConfig();

	// write rc script
	FILE *fp = fopen( rc_script_name, "w" );
	if ( fp != NULL ) {

		WriteRcHeader( fp );
		WriteRcLauncherConfig( fp, launchercfg );
		WriteRcPlayerName( fp );
		WriteRcUsrCommands( fp );
		WriteRcIntCommands( fp );
		WriteNetSubsysCommand( fp );
		WriteRcKeybConfig( fp );
		WriteRcAuxCommands( fp );

		fclose( fp );
	}

	// free temp mem
	if ( launchercfg != NULL ) {
		FREEMEM( launchercfg );
	}
}



