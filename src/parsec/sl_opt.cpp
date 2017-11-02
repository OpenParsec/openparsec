/*
 * PARSEC - Command Line Options
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:33 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-2001
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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "vid_defs.h"
#include "net_defs.h"
#include "sys_defs.h"

// local module header
#include "sl_opt.h"

// proprietary module headers
#include "con_aux.h"
#include "con_ext.h"
#include "e_getopt.h"
#include "e_loader.h"
#include "sys_bind.h"
#include "sys_file.h"
#include "vid_init.h"
#include "vid_plug.h"
#include "g_bot_cl.h"


// flags
#define DISABLE_DEMO_OPTION



// string constants -----------------------------------------------------------
//
static char options_invalid[]		= "\nUse \"--help\" for a list of command line options.\n";
static char default_player_name[]	= "fragme";


// print a list of valid command line options ---------------------------------
//
PRIVATE
int SLm_PrintCommandLineOptions()
{
	MSGOUT( "\nSome valid command line options:\n\n" );

	MSGOUT( " -h or --help               display this help page.\n\n" );

	MSGOUT( " -n or --playername <name>  set nickname for network game.\n" );
	MSGOUT( " -p or --pack <filename>    register additional data package.\n\n" );
	MSGOUT( " -m or --mod <dirname>      register mod directory.\n\n" );
	MSGOUT( " -f or --modforce <dirname> register mod directory with override.\n\n" );

	MSGOUT( " -j or --nojoystick         disable joystick.\n" );
	MSGOUT( " -s or --nosound            disable music and sound effects.\n\n" );

	MSGOUT( " --vidmode <mode>           specify video mode (like: 1024x768x32).\n" );
	MSGOUT( " --fullscreen               use fullscreen mode.\n" );
	MSGOUT( " --windowed                 use windowed mode.\n" );
	MSGOUT( " --flipsync                 synchronize with screen.\n" );
	MSGOUT( " --noflipsync               do not synchronize with screen.\n\n" );

	MSGOUT( "\n" );

	exit( EXIT_SUCCESS );
}


// disable joystick code ------------------------------------------------------
//
PRIVATE
int SLm_DisableJoystickCode()
{
	Op_Joystick 		= 0;
	SkipCalibrationCode = 1;
	return TRUE;
}


// disable sound code ---------------------------------------------------------
//
PRIVATE
int SLm_DisableSoundCode()
{
	SoundDisabled = 1;
	return TRUE;
}


// set flag for demo start-through --------------------------------------------
//
PRIVATE
int SLm_StartDemo()
{
	PlayDemo = 1;
	return TRUE;
}


// set loader flag to display file names --------------------------------------
//
PRIVATE
int SLm_ShowLoadedFiles()
{
	ShowFilesLoaded( TRUE );
	return TRUE;
}


// check user supplied network nickname ---------------------------------------
//
INLINE
int SLm_CheckPlayerName( char *name )
{
	ASSERT( name != NULL );

	for ( ; *name != 0; name++ ) {
		*name = tolower( *name );
		if ( !isalpha( *name ) ) {
			return FALSE;
		}
	}

	return TRUE;
}


// set player name ------------------------------------------------------------
//
PRIVATE
int SLm_SetPlayerName( char **playername )
{
	char *name = playername[ 0 ];
	int len = strlen( name );

	if ( ( len < 2 ) || ( len > MAX_PLAYER_NAME ) ) {
		return FALSE;
	}

	strncpy( ForcedPlayerName, name, MAX_PLAYER_NAME );
	ForcedPlayerName[ MAX_PLAYER_NAME ] = 0;

	if ( !SLm_CheckPlayerName( ForcedPlayerName ) ) {
		MSGOUT( "CLI error: player name contains invalid characters.\n" );
		return FALSE;
	}

	return TRUE;
}


// set startup video mode -----------------------------------------------------
//
PRIVATE
int SLm_SetVideoMode( char ** modespec )
{	
	int modearray[] = {0, 0, 0}; // xres, yres, bpp
	
	VID_MapSpecifierToMode(modespec[0], modearray);
	
	int xres = modearray[0];
	int yres = modearray[1];
	int bpp = modearray[2];
	
	if ( xres == 0 || yres == 0 || bpp == 0 )
		return FALSE;
	
	InitOp_ColorDepth = VID_MapBppToOpt(bpp);	
	InitOp_Resolution.set(xres, yres);

	return TRUE;
}


// set windowed startup video mode flag ---------------------------------------
//
PRIVATE
int SLm_SetWindowedMode()
{
	InitOp_WindowedMode = TRUE;
	return TRUE;
}


// set full screen startup video mode flag ------------------------------------
//
PRIVATE
int SLm_SetFullscreenMode()
{
	InitOp_WindowedMode = FALSE;
	return TRUE;
}


// set flipsync to on ---------------------------------------------------------
//
PRIVATE
int SLm_EnableFlipSync()
{
	InitOp_FlipSynched = TRUE;
	return TRUE;
}

// enable headless operation
PRIVATE
int SLm_EnableHeadlessBot()
{
	headless_bot = 1;
	return TRUE;

}


// set flipsync to off --------------------------------------------------------
//
PRIVATE
int SLm_DisableFlipSync()
{
	InitOp_FlipSynched = FALSE;
	return TRUE;
}


// register additional data package -------------------------------------------
//
PRIVATE
int SLm_RegisterPackage( char **packagename )
{
	char *name = packagename[ 0 ];

	if ( !SYS_RegisterPackage( name, 0, NULL ) ) {
		MSGOUT( "CLI error: package registration failed (%s).\n", name );
		MSGOUT( "CLI error: continuing without package.\n" );
	}

	return TRUE;
}


// register mod ---------------------------------------------------------------
//
PRIVATE
int SLm_RegisterMod( char **modname )
{
	char *name = modname[ 0 ];

	//NOTE:
	// mod_names[] is the global storage for the current mod names,
	// declared in CON_EXT.C
	// If mod_names[ 0 ] != NULL, a mod is currently active

	if ( mod_numnames >= MAX_REGISTERED_MODS ) {
		return FALSE;
	}

	mod_names[ mod_numnames ] = (char *) ALLOCMEM( strlen( name ) + 1 );
	if ( mod_names[ mod_numnames ] == NULL ) {
		return FALSE;
	}
	strcpy( mod_names[ mod_numnames ], name );

	// allocate memory for "name/name.dat" string
	char *packagename = (char *) ALLOCMEM( strlen( name ) * 2 + 6 );
	if ( packagename == NULL ) {
		return FALSE;
	}

	strcpy( packagename, name );
	strcat( packagename, "/" );
	strcat( packagename, name );
	strcat( packagename, ".dat" );

	if ( !SYS_RegisterPackage( packagename, 0, name ) ) {
		MSGOUT( "CLI error: package registration failed (%s).\n", name );
		MSGOUT( "CLI error: continuing without package.\n" );
	}

	mod_numnames++;
	FREEMEM( packagename );
	return TRUE;
}


// register mod which overrides internal packages -----------------------------
//
PRIVATE
int SLm_RegisterModForce( char **modname )
{
	char *name = modname[ 0 ];

	//NOTE:
	// mod_names[] is the global storage for the current mod names,
	// declared in CON_EXT.C
	// If mod_names[ 0 ] != NULL, a mod is currently active

	if ( mod_numnames >= MAX_REGISTERED_MODS ) {
		return FALSE;
	}

	mod_names[ mod_numnames ] = (char *) ALLOCMEM( strlen( name ) + 1 );
	if ( mod_names[ mod_numnames ] == NULL ) {
		return FALSE;
	}
	strcpy( mod_names[ mod_numnames ], name );

	// allocate memory for "name/name.dat" string
	char *packagename = (char *) ALLOCMEM( strlen( name ) * 2 + 6 );
	if ( packagename == NULL ) {
		return FALSE;
	}

	strcpy( packagename, name );
	strcat( packagename, "/" );
	strcat( packagename, name );
	strcat( packagename, ".dat" );

	if ( !SYS_RegisterPackage( packagename, 0, name ) ) {
		MSGOUT( "CLI error: package registration failed (%s).\n", name );
		MSGOUT( "CLI error: continuing without package.\n" );
	}

	mod_numnames++;
	FREEMEM( packagename );

	// set override flag
	mod_override = TRUE;

	return TRUE;
}


// set a global aux flag to the specified value -------------------------------
//
PRIVATE
int SLm_SetAuxFlag( int *auxinfo )
{
	int auxflag = auxinfo[ 0 ];
	int auxval  = auxinfo[ 1 ];

	// check flag id
	if ( ( auxflag < 0 ) || ( auxflag >= MAX_AUX_ENABLING ) ) {
		MSGOUT( "CLI error: auxflag out of range.\n" );
		return FALSE;
	}

	//NOTE:
	// the value will not be checked against any bounds here.
	// as long as the flag id is valid, the value will be used as is.

	// set value for flag
	AuxEnabling[ auxflag ] = auxval;

	return TRUE;
}


// register option for setting aux flags from the command line ----------------
//
PRIVATE
int SLm_RegisterAuxOption()
{
	cli_option_s clioption;
	memset( &clioption, 0, sizeof( cli_option_s ) );

	clioption.opt_short	= NULL;
	clioption.opt_long	= "aux";
	clioption.num_int	= 2;
	clioption.exec_int	= SLm_SetAuxFlag;

	return OPT_RegisterOption( &clioption );
}


// register default command line options --------------------------------------
//
PRIVATE
void RegisterDefaultOptions()
{
	OPT_RegisterSetOption( "h",  "help",			SLm_PrintCommandLineOptions );
	OPT_RegisterSetOption( "j",  "nojoystick",		SLm_DisableJoystickCode );
	OPT_RegisterSetOption( "s",  "nosound",			SLm_DisableSoundCode );
	OPT_RegisterSetOption( "o",  "showfiles",		SLm_ShowLoadedFiles );
	OPT_RegisterSetOption( NULL, "windowed",		SLm_SetWindowedMode );
	OPT_RegisterSetOption( NULL, "fullscreen",		SLm_SetFullscreenMode );
	OPT_RegisterSetOption( NULL, "flipsync",		SLm_EnableFlipSync );
	OPT_RegisterSetOption( NULL, "noflipsync",		SLm_DisableFlipSync );
	OPT_RegisterSetOption( "b",  "headlessbot",		SLm_EnableHeadlessBot );
#ifndef DISABLE_DEMO_OPTION
	OPT_RegisterSetOption( "x", "startdemo",		SLm_StartDemo );
#endif

	SLm_RegisterAuxOption();

	OPT_RegisterStringOption( "n",  "playername",	SLm_SetPlayerName );
	OPT_RegisterStringOption( NULL, "vidmode",		SLm_SetVideoMode );
	OPT_RegisterStringOption( "p",  "pack",			SLm_RegisterPackage );
	OPT_RegisterStringOption( "m",  "mod",			SLm_RegisterMod );
	OPT_RegisterStringOption( "f",  "modforce",		SLm_RegisterModForce );
	
}


// register default command line options and exec all registered options ------
//
void SYSs_CheckCommandLine( int argc, char **argv )
{
	// set default name of local player
	srand(time(NULL));
	int playerRand = rand() % 999 + 1;
	
	strncpy( LocalPlayerName,default_player_name,MAX_PLAYER_NAME );
	sprintf(LocalPlayerName,"%s%d",LocalPlayerName,playerRand);
	LocalPlayerName[ MAX_PLAYER_NAME ] = 0;

	// register default command line options
	RegisterDefaultOptions();

	// exec all registered command line options
	if ( !OPT_ExecRegisteredOptions( argc, argv ) ) {
		MSGOUT( options_invalid );
		exit( EXIT_SUCCESS );
	}
}


