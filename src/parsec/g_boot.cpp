/*
 * PARSEC - Startup Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:25 $
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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"
#include "od_class.h"

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "inp_defs.h"
#include "net_defs.h"
#include "sys_defs.h"
#include "vid_defs.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "g_boot.h"

// proprietary module headers
#include "con_aux.h"
#include "con_main.h"
#include "e_color.h"
#include "e_demo.h"
#include "e_events.h"
#include "e_mouse.h"
#include "e_replay.h"
#include "e_supp.h"
#include "g_stars.h"
#include "h_drwhud.h"
#include "h_frmplt.h"
#include "h_supp.h"
#include "h_text.h"
#include "inp_init.h"
#include "net_conn.h"
#include "obj_creg.h"
#include "obj_ctrl.h"
#include "part_sys.h"
#include "sys_bind.h"
#include "vid_init.h"


// flags
//#define NO_NET_GAME_POSSIBLE

#define PRERELEASE_MESSAGE
#define OPENSOURCE_RELEASE
//#define LINUXTAG99_MESSAGE
//#define MATROXTEST_MESSAGE
//#define SELFRUNNING_MESSAGE
//#define BRAUNSCHWEIG_MESSAGE
//#define LINUXTAG2K_MESSAGE
//#define REDHAT7DISTRI_MESSAGE
//#define LANTEST_MESSAGE
#define RANDOM_STARTUP_SHIPCLASS
//#define CHECK_HARD_DATE_LIMIT



// minimum amount of memory needed [bytes] ------------------------------------
//
#define MIN_MEM_NEEDED	15000000 //3072000 war mal :)
dword MemNeeded = MIN_MEM_NEEDED;


// save area for locations of pseudo and fixed stars --------------------------
//
pseudostar_s			s_PseudoStars[ MAX_PSEUDO_STARS ];
fixedstar_s				s_FixedStars[ MAX_FIXED_STARS ];



// display startup messages ---------------------------------------------------
//
void StartupMessages()
{
	MSGOUT( "\n" );

	MSGOUT( " OPENPARSEC build " CLIENT_BUILD_NUMBER "." );
	MSGOUT( "---------------------------------------------------" );
	MSGOUT( "            http://www.openparsec.com              " );
	MSGOUT( "---------------------------------------------------" );
	MSGOUT( "\n" );

 

#if defined( PRERELEASE_MESSAGE )

	MSGOUT( " UNTESTED VERSION CURRENTLY UNDER DEVELOPMENT." );
	MSGOUT( "\n" );
	MSGOUT( "---------------------------------------------------" );
	MSGOUT( "\n" );

#endif

}


// restore locations of pseudo and fixed stars, respectively ------------------
//
INLINE
void ReInitStars()
{
	// init starting locations of fixed stars
	for ( int fsindx = 0; fsindx < NumFixedStars; fsindx++ ) {
		FixedStars[ fsindx ] = s_FixedStars[ fsindx ];
	}

	// init starting locations of pseudo stars
	MakeIdMatrx( PseudoStarMovement );
	for ( int psindx = 0; psindx < NumPseudoStars; psindx++ ) {
		PseudoStars[ psindx ] = s_PseudoStars[ psindx ];
	}
}


// init global variables to default values ------------------------------------
//
PRIVATE
void InitGlobals()
{
	CurrentNumExtras	= 0;
	CurrentNumPrtExtras	= 0;

	CurYaw				= BAMS_DEG0;
	CurPitch			= BAMS_DEG0;
	CurRoll				= BAMS_DEG0;
	CurSlideHorz		= GEOMV_0;
	CurSlideVert		= GEOMV_0;

	LastPitch 			= BAMS_DEG0;
	LastYaw   			= BAMS_DEG0;
	LastRoll			= BAMS_DEG0;
	LastSlideHorz		= GEOMV_0;
	LastSlideVert		= GEOMV_0;
	LastSpeed			= 0;
	IdleDuration 		= 0;

	CurActionWait		= 0;
	AutomaticPitch		= BAMS_DEG0;
	AutomaticYaw		= BAMS_DEG0;
	AutomaticRoll		= BAMS_DEG0;
	AutomaticSlideHorz	= GEOMV_0;
	AutomaticSlideVert	= GEOMV_0;
	AutomaticMovement	= 0;
	ReplayObjCamActive	= FALSE;

	AbsYaw	 = BAMS_DEG0;
	AbsPitch = BAMS_DEG0;
	AbsRoll  = BAMS_DEG0;

	FireDisable			= 1;
	FireRepeat			= 1;
	CurGun				= 0;
	MissileDisable		= 1;
	CurLauncher 		= 0;

	ObjCameraActive 	= 0;
	HelpActive			= 0;
	ConsoleSliding		= 0;

	KeybFlags->ConEnabled = 0;

	TargetObjNumber		= TARGETID_NO_TARGET;
	TargetRemId 		= TARGETID_NO_TARGET;
	TargetVisible		= FALSE;
	TargetLocked		= FALSE;

	MaxNumShots 		= 0;
	NumShots			= 0;

	MaxNumMissiles 		= 0;
	NumMissiles    		= 0;

	SelectedLaser		= 0;
	SelectedMissile 	= 0;

	CameraMoved			= 0;

	SetScreenBlue		= 0;
	SetScreenWhite		= 0;

	ResetMessageArea();

	ReInitStars();

//	HUD_InitDemoText();
}


// save locations of fixed and pseudo stars, respectively ---------------------
//
INLINE
void SaveStarLocations()
{
	for ( int fsindx = 0; fsindx < NumFixedStars; fsindx++ ) {
		s_FixedStars[ fsindx ] = FixedStars[ fsindx ];
	}

	for ( int psindx = 0; psindx < NumPseudoStars; psindx++ ) {
		s_PseudoStars[ psindx ] = PseudoStars[ psindx ];
	}
}


// init display for the first time --------------------------------------------
//
INLINE
void BootDisplay()
{
	// set video mode specified by global vars
	if ( TextModeActive && !DirectNetPlay ) {
		VID_InitMode();
	}
}


// init the local ship class that will be selected after startup --------------
//
INLINE
void InitLocalShipClass()
{

#ifdef RANDOM_STARTUP_SHIPCLASS

	ASSERT( NumShipClasses >= 3 );

	time_t curtime = time( NULL );
	int randval = (int) curtime;

	int shipno = randval % NumShipClasses;
	LocalShipClass = ShipClasses[ shipno ];

#else

	// done in G_GLOBAL.C
//	LocalShipClass = SHIP_CLASS_1;

#endif

}


// alloc storage for MyShip ---------------------------------------------------
//
PRIVATE
void AllocMyShip()
{
	ASSERT( MyShip == NULL );
	ASSERT( MyShipMaxInstanceSize == 0 );

	// need storage for additional instance data
	MyShipMaxInstanceSize = sizeof( ShipObject ) + 16384;

	// this will never be freed and the
	// pointer must never be changed!!
	MyShip = (ShipObject *) ALLOCMEM( MyShipMaxInstanceSize );
	if ( MyShip == NULL )
		OUTOFMEM( 0 );

	// clear is mandatory
	memset( MyShip, 0, MyShipMaxInstanceSize );
}


// command line option must override name set in init script ------------------
//
PRIVATE
void OverridePlayerName()
{
	// override player name if set by command line option
	if ( ForcedPlayerName[ 0 ] != 0 ) {
		strncpy( LocalPlayerName, ForcedPlayerName, MAX_PLAYER_NAME );
		LocalPlayerName[ MAX_PLAYER_NAME ] = 0;
	}
}

// ----------------------------------------------------------------------------
//
PUBLIC int aud_do_open_menu_sound_on_aud_conf = FALSE;


// do necessary initializations that are only done once -----------------------
//
int GameBoot()
{
	MSGOUT( "Booting game core." );

	// dummy init
	limit_timeval_cur = 1000000000;

	// init color maps
	InitColorMaps();

	// alloc storage for MyShip
	AllocMyShip();

	// init object maintenance
	InitObjCtrl();

	// init fixed stars
	InitFixedStars();

	// init pseudo stars
	InitPseudoStars();

	// save star locations
	SaveStarLocations();

	// init geometry of screen displays
	InitHudDisplay();

	// init particle system
	InitParticleSystem();

	// init the event manager
	EVT_InitManager();

	// init input code
	INP_InitSubsystem();

	// enable AUDs_OpenMenuSound call in AUD.CONF before console files have been loaded
	aud_do_open_menu_sound_on_aud_conf = TRUE;

	// init console
	InitConsole();			// *MUST NOT* be called earlier!!!

	// command line option must override
	// name set in init script
	OverridePlayerName();

	// stuff list of demos into table
	DEMO_RegisterInitialDemos();

	// may be chosen at random
	InitLocalShipClass();

	// init frame timer
	SYSs_InitFrameTimer();

	return TRUE;
}


// create objects always created on game mode entry by default ----------------
//
PRIVATE
void CreateDefaultObjects()
{
	if ( !NetworkGameMask ) {

		Xmatrx startm;

		MakeIdMatrx( startm );
		startm[ 1 ][ 3 ] = FLOAT_TO_GEOMV( -80.0 );
		CreateObject( SHIP_CLASS_1, startm );

		MakeIdMatrx( startm );
		startm[ 1 ][ 3 ] = FLOAT_TO_GEOMV( +80.0 );
		CreateObject( SHIP_CLASS_2, startm );

		MakeIdMatrx( startm );
		startm[ 1 ][ 3 ] = FLOAT_TO_GEOMV( 256.0 );
		CreateObject( SHIP_CLASS_3, startm );

		MakeIdMatrx( startm );
		startm[ 1 ][ 3 ] = FLOAT_TO_GEOMV( 336.0 );
		CreateObject( SHIP_CLASS_1, startm );
	}
}


// handle network game entry --------------------------------------------------
//
PRIVATE
int HandleEntryMode()
{
	int rc = TRUE;

	// enter network game session if necessary
	//FIXME: [2/7/2002] && ??????
	if ( NetworkGameMask && AUX_PERFORM_AUTOMATIC_CONNECT ) {

		if ( sys_BindType_PROTOCOL == BT_PROTOCOL_PEERTOPEER ) {

			// automatically try to establish connection
			if ( NET_AutomaticConnect() ) {

				// always start network game in entry mode
				EntryMode = TRUE;
				return TRUE;
			}

			rc = FALSE;

		} else {

			MSGOUT( "automatic connect only valid in peer mode." );
		}
	}

	// no entrymode since not connected
	EntryMode = FALSE;

	return rc;
}


// handle init of graphics mode -----------------------------------------------
//
INLINE
void HandleGraphicsModeSwitch()
{
	// switch to mode specified in option vars (Op_xx)
	VID_ApplyOptions();
}


// set network game flag appropriately ----------------------------------------
//
INLINE
void SetNetFlag( int use_net )
{
#ifndef NO_NET_GAME_POSSIBLE

	// decide if network game or weapons testing mode
	NetworkGameMask = NetAvailable ? use_net : NETWORK_GAME_OFF;

#else

	NetworkGameMask = NETWORK_GAME_OFF;

#endif
}


// init necessary game functions (done every time game mode is entered) -------
//
int GameInit( int use_net )
{
	// free objects and particles
	KillAllObjects();

	// reinit globals
	InitGlobals();

	// check files used by console
	InitConsoleFileHandling();

	// set net flag appropriately
	SetNetFlag( use_net );

	// handle network game entry
	if ( !HandleEntryMode() )
		return FALSE;

	// handle init of graphics mode
	HandleGraphicsModeSwitch();

	// create default objects
	CreateDefaultObjects();

	// init game sound
//RAM	AUDs_OpenMenuSound();
	AUDs_OpenGameSound();
	AUDs_OpenMenuSound();

	return TRUE;
}


