/*
 * PARSEC - Main Module
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:41 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-2000

   Updated By:
    Copyright (c) 2003 Sivaram Velauthapillai (sivaram33@hotmail.com or koalabear33@yahoo.com)

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

/*
  UPDATES:
    August 25, 2003:
      + Removed initialization of the old GSI sound server. OpenAL does not require
        it
*/ 
// compilation flags/debug support
#include "config.h"


// C library includes
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"
#include "debug.h"

// subsystem headers
#include "aud_defs.h"
#include "inp_defs.h"
#include "net_defs.h"
#include "sys_defs.h"
#include "vid_defs.h"

// local module header
#include "sl_main.h"

// proprietary module headers
#include "al_initg.h"
#include "con_main.h"
#include "con_rc.h"
#include "e_color.h"
#include "e_events.h"
#include "e_loader.h"
#include "g_boot.h"
#include "g_gameloop.h"
#include "h_supp.h"
#include "inp_init.h"
#include "part_sys.h"
#include "sys_bind.h"
#include "sys_conv.h"
#include "sys_file.h"
#include "vid_init.h"

#ifdef SYSTEM_TARGET_LINUX
	#include <SDL/SDL.h>
#else
	#include <SDL.h>
#endif

#ifdef SYSTEM_TARGET_OSX
	#include <CoreFoundation/CoreFoundation.h>
#endif


// flags
//#define SHOW_MEMORY_INFO_BEFORE_BOOT
#define SKIP_DEMO_INTRO
#define START_NETGAME_AFTER_DEMOINTRO


// file package location control
//#define LOAD_DATA_FROM_EXE


// size of parsec executable [bytes]
// used to load data directly from it
#define PARSEC_EXE_SIZE						396860


// info about separate data file
#ifdef LOAD_DATA_FROM_EXE

	#define DATA_PACKAGE_NAME				sys_ProgramName
	#define DATA_PACKAGE_OFFSET				PARSEC_EXE_SIZE

#else // LOAD_DATA_FROM_EXE

	#define DATA_PACKAGE_NAME				extra_pack_name
	#define DATA_PACKAGE_OFFSET				extra_pack_offset

	static char extra_pack_name[]			= "pscdata0.dat";
	static size_t extra_pack_offset			= 0;

#endif // LOAD_DATA_FROM_EXE


// define macro responsible for intro invocation
#ifndef SKIP_DEMO_INTRO
	#define PLAYDEMOINTRO() 				PlayDemoIntro()
#else
	#define PLAYDEMOINTRO() 				TRUE
#endif


// string constants
static char joystick_disabled[]		= "Joystick code is disabled.\n";
static char exiting_game[]			= "Exiting Parsec...\n";


#ifndef SYSTEM_TARGET_WINDOWS
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
#endif


#ifdef SYSTEM_TARGET_OSX

static char macresourcespath[1024];

// get absolute path to Parsec.app/Contents/Resources/, so we can chdir to it
//
PRIVATE
const char * SLm_GetMacResourcesPath()
{
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
	CFURLRef absoluteResourcesURL = CFURLCopyAbsoluteURL(resourcesURL);
	CFStringRef cfResourcesStringRef = CFURLCopyFileSystemPath(absoluteResourcesURL, kCFURLPOSIXPathStyle);
	
	CFStringGetCString(cfResourcesStringRef, macresourcespath, sizeof(macresourcespath), kCFStringEncodingASCII);
	
	CFRelease(resourcesURL);
	CFRelease(absoluteResourcesURL);
	CFRelease(cfResourcesStringRef);
	
	return macresourcespath;
}

#endif


// check available memory -----------------------------------------------------
//
void SYSs_CheckMemory()
{
	//TODO:
}

// cancel game mode -----------------------------------------------------------
//
INLINE
void SLm_GameCancel()
{
	// cancel game sound
	AUDs_CloseGameSound();
}


// start up game --------------------------------------------------------------
//
INLINE
int SLm_StartUpGame()
{
	// init game state and enter main menu
	if ( !GameBoot() )
		return FALSE;

	if ( GameInit( NETWORK_GAME_ON ) ) {
		
		// enter gameloop
		GameLoop();
		
		// cancel game sound
		AUDs_CloseGameSound();
	}
		
	// quit game
	return TRUE;
}


// calibrate and init joystick ------------------------------------------------
//
INLINE
void SLm_InitJoystickCode()
{
	if ( !SkipCalibrationCode ) {

//		IL_CalibrateJoystick();
		Op_Joystick 	 = QueryJoystick;
		JoystickDisabled = !QueryJoystick;

	} else {

		QueryJoystick	 = 0;
		Op_Joystick 	 = 0;
		JoystickDisabled = 1;
		MSGOUT( joystick_disabled );
	}
}


// init sound -----------------------------------------------------------------
//
INLINE
void SLm_InitSound()
{
	// init sound system if soundcard installed
	SoundAvailable  = AUDs_InitSoundSys();
	Op_SoundEffects = SoundAvailable;
	Op_Music		= SoundAvailable;
}


// package string constants ---------------------------------------------------
//
static char package_error[]		= "Data package error: file %s";
static char package_name_2[]	= "pscdata2.dat";
static char package_name_3[]	= "pscdata3.dat";


// init file system -----------------------------------------------------------
//
INLINE
void SLm_InitFileSystem()
{
	// make sure mp3s contained in sound package are converted to wav
	if ( !SYS_ConvertPackageMP3s( package_name_2, 0 ) ) {
		FERROR( package_error, package_name_2 );
	}

	// register mandatory main data package
	if ( !SYS_RegisterPackage( DATA_PACKAGE_NAME, DATA_PACKAGE_OFFSET, NULL ) ) {
		FERROR( package_error, DATA_PACKAGE_NAME );
	}

	// register optional data package
	SYS_RegisterPackage( package_name_2, 0, NULL );

	// register third package (new data introduced with build 0196)
	SYS_RegisterPackage( package_name_3, 0, NULL );
	
	//  duplicate files in pscdata2.dat with newer versions in pscdata3.dat
	SYS_OverridePackage( package_name_2, package_name_3 );
}


// free globally allocated storage buffers ------------------------------------
//
INLINE
void SLm_FreeBuffers()
{
	FreeLoaderDataBuffers();
}


// free resources and perform other exit functions ----------------------------
//
void SL_CleanUp()
{
	// write rc script
	WritePersistentCommands();

	// kill the event manager
	EVT_KillManager();

	// kill game systems
	KillParticleSystem();
	KillConsole();
	KillColorMaps();

	// kill networking system;
	// must not be done after SYSs_KillFrameTimer()
	NETs_KillAPI();

	// kill input code
	INP_KillSubsystem();

	// kill sound system
	AUDs_KillSoundSys();

	// kill timer code
	SYSs_KillFrameTimer();

	// free global buffers
	SLm_FreeBuffers();

	MSGOUT( exiting_game );
}


// system entry point ---------------------------------------------------------
//
int main( int argc, char **argv )
{

#ifdef DMALLOC
	/*
	 * Get environ variable DMALLOC_OPTIONS and pass the settings string
	 * on to dmalloc_debug_setup to setup the dmalloc debugging flags.
	*/
	dmalloc_debug_setup(getenv("DMALLOC_OPTIONS"));
#endif
	
#ifdef SYSTEM_TARGET_OSX
	chdir(SLm_GetMacResourcesPath());
#endif

	// set program name (used in error messages)
	sys_ProgramName = argv[ 0 ];

	// this is the first thing we must do in order to resemble the old module registration
	TheModuleManager->InitAllModules();

	// check command line options
	SYSs_CheckCommandLine( argc, argv );

	// bind dynamic subsystem functions
	SYS_BindDynamicFunctions();

	// init error handling
	SYSs_InitErrorHandling();

	// display startup messages
	StartupMessages();

	// check available memory
	SYSs_CheckMemory();

	// init extended file system
	SLm_InitFileSystem();

	// init video subsystem
	VID_InitSubsystem();

	// read objects and other data into memory
	LoadData( FetchCtrlFileName(), TRUE );

	// init networking api
	NETs_InitAPI();

	// init sound system
	SLm_InitSound();

	// calibrate joystick
	SLm_InitJoystickCode();

	// init game and start gameloop
	SLm_StartUpGame();

	// restore original display mode
	VIDs_RestoreDisplay();

	// clean up and exit
	SL_CleanUp();

	return EXIT_SUCCESS;
}


