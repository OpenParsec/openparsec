/*
 * PARSEC - Play Binary Demo File
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:34 $
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
#include "gd_heads.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "inp_defs.h"
#include "net_defs.h"
#include "sys_defs.h"

// local module header
#include "e_demo.h"

// proprietary module headers
#include "con_act.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_ext.h"
#include "con_main.h"
#include "con_std.h"
#include "e_record.h"
#include "e_replay.h"
#include "e_supp.h"
#include "g_camera.h"
#include "g_supp.h"
#include "h_text.h"
#include "m_main.h"
#include "net_game.h"
#include "g_wfx.h"
#include "sys_file.h"
#include "sys_path.h"
#include "sys_swap.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// string constants -----------------------------------------------------------
//
static char dem_extension[]		= CON_FILE_COMPILED_EXTENSION;
static char cut_open_error[]	= "cut demo open error.";
static char cut_write_error[]	= "cut demo write error.";
static char cut_compile_error[]	= "cut state header compilation failed.";

static char dem_file_wldcard1[]	= "*" CON_FILE_COMPILED_EXTENSION;
static char dem_file_wldcard2[] = REFCON_COMMANDS_DIR "*" CON_FILE_COMPILED_EXTENSION;
static char dem_file_wldcard3[] = STDCON_COMMANDS_DIR "*" CON_FILE_COMPILED_EXTENSION;
static char dem_file_wldcard4[] = RECORD_COMMANDS_DIR "*" CON_FILE_COMPILED_EXTENSION;


// pointer to start of demo data in memory ------------------------------------
//
static char *demo_data		= NULL;


// pointer to current position in demo data -----------------------------------
//
static char *demo_replaypos	= NULL;


// pointer to cut start position ----------------------------------------------
//
static char *demo_cutpos	= NULL;


// name for cut output --------------------------------------------------------
//
static char demo_cut_name[ 16 ];


// default maximum number of registered demos ---------------------------------
//
#define DEFAULT_MAX_DEMOS	256


// registered demo tables -----------------------------------------------------
//
PUBLIC int		num_registered_demos = 0;
PUBLIC int		max_registered_demos = DEFAULT_MAX_DEMOS;
PUBLIC char*	registered_demo_names[ DEFAULT_MAX_DEMOS ];
PUBLIC char*	registered_demo_titles[ DEFAULT_MAX_DEMOS ];


// read and parse optional demo header ----------------------------------------
//
size_t DEMO_ParseDemoHeader( FILE *fp, int demoid, int verbose )
{
	ASSERT( fp != NULL );

	//NOTE:
	// a demoid of -1 ensures that no info will be stored anywhere.
	// this is useful for just skipping the header (with optional
	// output of the contained info, without storing it anywhere).

	//NOTE:
	// if the file does not contain a valid header, the file read position
	// will be reset to 0, in order to allow subsequent reads to start
	// at the beginning of the actual data transparently. if the header
	// is valid but incompatible it will simply be overread.

	// read in header
	DemHeader hdr;
	size_t bytesread = SYS_fread( &hdr, 1, sizeof( hdr ), fp );
	if ( bytesread != sizeof( hdr ) ) {
		SYS_fseek( fp, 0, SEEK_SET );
		return 0;
	}

	// swap endianness of header
	SYS_SwapDemHeader( &hdr );

	// test sig
	if ( stricmp( hdr.signature, DEM_SIGNATURE ) != 0 ) {
		SYS_fseek( fp, 0, SEEK_SET );
		return 0;
	}

	// test version
	if ( hdr.version < REQUIRED_DEM_VERSION ) {
		SYS_fseek( fp, hdr.headersize, SEEK_SET );
		return hdr.headersize;
	}

	// alloc temporary mem for info block
	size_t infomemlen = hdr.headersize - sizeof( hdr );
	char *infomem = (char *) ALLOCMEM( infomemlen );
	if ( infomem == NULL ) {
		SYS_fseek( fp, hdr.headersize, SEEK_SET );
		return hdr.headersize;
	}

	// read info block
	bytesread = SYS_fread( infomem, 1, infomemlen, fp );
	if ( bytesread != infomemlen ) {
		SYS_fseek( fp, hdr.headersize, SEEK_SET );
		return hdr.headersize;
	}

	char *demotitle  = NULL;
	char *demodesc   = NULL;
	char *demoauthor = NULL;

	// read sequence of (key,value) pairs
	for ( char *scan = infomem; scan < infomem + infomemlen; ) {

		byte demokey = *scan++;
		switch ( demokey ) {

			case DEMO_KEY_TITLE:
				demotitle = scan;
				scan += strlen( scan ) + 1;
				break;

			case DEMO_KEY_DESCRIPTION:
				demodesc = scan;
				scan += strlen( scan ) + 1;
				break;

			case DEMO_KEY_AUTHOR:
				demoauthor = scan;
				scan += strlen( scan ) + 1;
				break;

			default:
				ASSERT( 0 );
				break;
		}
	}

	// output info if verbose mode
	if ( verbose ) {

		if ( demotitle != NULL )
			MSGOUT( "TITLE      : %s", demotitle );
		if ( demodesc != NULL )
			MSGOUT( "DESCRIPTION: %s", demodesc );
		if ( demoauthor != NULL )
			MSGOUT( "AUTHOR     : %s", demoauthor );
	}

	// store info if demo id known
	if ( demoid != -1 ) {

		ASSERT( (dword)demoid < (dword)num_registered_demos );
		if ( registered_demo_titles[ demoid ] != NULL ) {
			FREEMEM( registered_demo_titles[ demoid ] );
			registered_demo_titles[ demoid ] = NULL;
		}

		if ( demotitle != NULL ) {

			registered_demo_titles[ demoid ] = (char *)
				ALLOCMEM( strlen( demotitle ) + 1 );
			if ( registered_demo_titles[ demoid ] == NULL )
				OUTOFMEM( 0 );
			strcpy( registered_demo_titles[ demoid ], demotitle );

			// convert to lower-case
			strlwr( registered_demo_titles[ demoid ] );
		}
	}

	// free temporary info block
	FREEMEM( infomem );

	// return size of header
	return hdr.headersize;
}


// fetch and store info from headers of all registered demos ------------------
//
PRIVATE
void CreateRegisteredDemoInfo()
{
	for ( int cdem = 0; cdem < num_registered_demos; cdem++ ) {

		FILE *fp = DEMO_BinaryOpenDemo( registered_demo_names[ cdem ] );
		if ( fp != NULL ) {

			DEMO_ParseDemoHeader( fp, cdem, FALSE );
			SYS_fclose( fp );
		}
	}
}


// free all acquired info for registered demos --------------------------------
//
PRIVATE
void FreeRegisteredDemoInfo()
{
	for ( int cdem = 0; cdem < num_registered_demos; cdem++ ) {

		if ( registered_demo_names[ cdem ] != NULL ) {
			FREEMEM( registered_demo_names[ cdem ] );
			registered_demo_names[ cdem ] = NULL;
		}

		if ( registered_demo_titles[ cdem ] != NULL ) {
			FREEMEM( registered_demo_titles[ cdem ] );
			registered_demo_titles[ cdem ] = NULL;
		}
	}

	num_registered_demos = 0;
}


// register demos on startup --------------------------------------------------
//
void DEMO_RegisterInitialDemos()
{
	//NOTE:
	// this function may also be called after startup to rescan the
	// entire info for all available demos. this is done on user-demand
	// by CON_EXT::Cmd_RescanExternalCommands().

	// free previous info if present
	FreeRegisteredDemoInfo();

	// acquire demos contained in packages
	SYS_AcquirePackageDemos();

	// acquire demos in search paths
	SYSs_AcquireDemoPath( dem_file_wldcard1, NULL );
	SYSs_AcquireDemoPath( dem_file_wldcard2, NULL );
	SYSs_AcquireDemoPath( dem_file_wldcard3, NULL );
	SYSs_AcquireDemoPath( dem_file_wldcard4, NULL );

	// store demo titles
	CreateRegisteredDemoInfo();
}


// user input disabling key maps ----------------------------------------------
//
static keyfunc_s _key_replay_map;
PUBLIC keyfunc_s* key_replay_map = &_key_replay_map;

static keyfunc_s key_disabling_map = {

	0,		// 00: key_Escape
	1,		// 01: key_TurnLeft
	1,		// 02: key_TurnRight
	1,		// 03: key_DiveDown
	1,		// 04: key_PullUp
	1,		// 05: key_RollLeft
	1,		// 06: key_RollRight
	1,		// 07: key_ShootWeapon
	1,		// 08: key_LaunchMissile
	2,      // 09: key_NextWeapon
	2,      // 10: key_NextMissile
	1,		// 11: key_Accelerate
	1,		// 12: key_Decelerate
	1,		// 13: key_SlideLeft
	1,		// 14: key_SlideRight
	1,		// 15: key_SlideUp
	1,		// 16: key_SlideDown
	2,      // 17: key_NextTarget
	0,      // 18: key_ToggleFrameRate
	0,      // 19: key_ToggleObjCamera
	0,      // 20: key_ToggleHelp
	0,      // 21: key_ToggleConsole
	0,      // 22: key_SaveScreenShot
	0,      // 23: key_ShowKillStats
	1,		// 24: key_SpeedZero
	1,		// 25: key_TargetSpeed
	1,		// 26: key_AfterBurner
	0,		// 27: key_FrontTarget
	0,      // 28: key_Select
	0,      // 29: key_CursorLeft
	0,      // 30: key_CursorRight
	0,      // 31: key_CursorUp
	0,      // 32: key_CursorDown
};


// for saving previous state of user input disabling flag ---------------------
//
static int old_disable_user_input = -1;


// set user input disabling flag to previous value ----------------------------
//
PRIVATE
void DEMO_EnableUserInput()
{
	// restore input disabling flag
	if ( old_disable_user_input != -1 ) {
		UserInputDisabled = old_disable_user_input;
		old_disable_user_input = -1;
	}
}


// disable user input during demo replay --------------------------------------
//
PRIVATE
void DEMO_DisableUserInput()
{
	// no user interference allowed
	old_disable_user_input = UserInputDisabled;
	UserInputDisabled = AUX_ENABLE_DEMO_REPLAY_USER_INPUT ? FALSE : TRUE;

	// clear map
	memset( key_replay_map, 0, sizeof( keyfunc_s ) );
}


// ensure replay of certain keys works regardless of input disabling ----------
//
void DEMO_UserInputDisabling()
{
	ASSERT( sizeof( DepressedKeys->key_Escape ) == sizeof( dword ) );
	int numkeys = NUM_GAMEFUNC_KEYS;

	for ( int keyid = 0; keyid < numkeys; keyid++ ) {

		if ( ((dword*)&key_disabling_map)[ keyid ] == 1 ) {
			((dword*)DepressedKeys)[ keyid ] = 0;
		} else if ( ((byte*)&key_disabling_map)[ keyid ] == 2 ) {
			((dword*)DepressedKeys)[ keyid ] = ((dword*)key_replay_map)[ keyid ];
		}
	}

	// clear map
	memset( key_replay_map, 0, sizeof( keyfunc_s ) );
}


// saved state  ---------------------------------------------------------------
//
#define STATE_VALID_NONE			0x0000
#define STATE_VALID_AUX				0x0001
#define STATE_VALID_NAME			0x0002
#define STATE_VALID_REFFRAMEFREQ	0x0004
#define STATE_VALID_LIGHTINGCONF	0x0008

static int			state_valid = STATE_VALID_NONE;
static int			state_aux_enabling[ AUX_ARRAY_NUM_ENTRIES_USED ];
static int			state_aux_data[ AUXDATA_ARRAY_NUM_ENTRIES_USED ];
static char			state_player_name[ MAX_PLAYER_NAME + 1 ];
static refframe_t	state_refframe_freq;
static colrgba_s	state_lightcolor_ambient;
static colrgba_s	state_lightcolor_diffuse;
static colrgba_s	state_lightcolor_specular;
static Vector3		state_global_dir_light;


// save part of state before starting replay if enabled -----------------------
//
PRIVATE
void SaveEntryState()
{
	if ( AUX_SAVE_AUX_ARRAY_ON_DEMO_REPLAY ) {

		// save aux flags
		ASSERT( sizeof( state_aux_enabling[ 0 ] ) == sizeof( AuxEnabling[ 0 ] ) );
		memcpy( state_aux_enabling, AuxEnabling,
				AUX_ARRAY_NUM_ENTRIES_USED * sizeof( state_aux_enabling[ 0 ] ) );

		// save aux data
		ASSERT( sizeof( state_aux_data[ 0 ] ) == sizeof( AuxData[ 0 ] ) );
		memcpy( state_aux_data, AuxData,
				AUXDATA_ARRAY_NUM_ENTRIES_USED * sizeof( state_aux_data[ 0 ] ) );

		// set valid bit
		state_valid |= STATE_VALID_AUX;
	}

	if ( AUX_SAVE_PLAYER_NAME_ON_DEMO_REPLAY ) {

		// save name of local player
		CopyRemoteName( state_player_name, LocalPlayerName );

		// set valid bit
		state_valid |= STATE_VALID_NAME;
	}

	if ( AUX_SAVE_REFFRAMEFREQ_ON_DEMO_REPLAY ) {

		// save current refframe frequency
		state_refframe_freq = RefFrameFrequency;

		// set valid bit
		state_valid |= STATE_VALID_REFFRAMEFREQ;
	}

	if ( AUX_SAVE_LIGHTINGCONF_ON_DEMO_REPLAY ) {

		// save current lighting configuration
		state_lightcolor_ambient  = LightColorAmbient;
		state_lightcolor_diffuse  = LightColorDiffuse;
		state_lightcolor_specular = LightColorSpecular;
		state_global_dir_light    = GlobalDirLight;

		// set valid bit
		state_valid |= STATE_VALID_LIGHTINGCONF;
	}
}


// restore saved state after replay if enabled --------------------------------
//
PRIVATE
void RestoreEntryState()
{
	if ( AUX_SAVE_AUX_ARRAY_ON_DEMO_REPLAY ) {

		if ( state_valid & STATE_VALID_AUX ) {

			// restore aux flags
			ASSERT( sizeof( state_aux_enabling[ 0 ] ) == sizeof( AuxEnabling[ 0 ] ) );
			memcpy( AuxEnabling, state_aux_enabling,
					AUX_ARRAY_NUM_ENTRIES_USED * sizeof( state_aux_enabling[ 0 ] ) );

			// restore aux data
			ASSERT( sizeof( state_aux_data[ 0 ] ) == sizeof( AuxData[ 0 ] ) );
			memcpy( AuxData, state_aux_data,
					AUXDATA_ARRAY_NUM_ENTRIES_USED * sizeof( state_aux_data[ 0 ] ) );
		}
	}

	if ( AUX_SAVE_PLAYER_NAME_ON_DEMO_REPLAY ) {

		if ( state_valid & STATE_VALID_NAME ) {

			// restore name of local player
			CopyRemoteName( LocalPlayerName, state_player_name );
			CopyRemoteName( Player_Name[ LocalPlayerId ], state_player_name );
		}
	}

	if ( AUX_SAVE_REFFRAMEFREQ_ON_DEMO_REPLAY ) {

		if ( state_valid & STATE_VALID_REFFRAMEFREQ ) {

			if ( RefFrameFrequency != state_refframe_freq ) {

				// restore refframe frequency
				RefFrameFrequency = state_refframe_freq;

				// reinit frame timer to apply frequency change
				SYSs_InitFrameTimer();
			}
		}
	}

	if ( AUX_SAVE_LIGHTINGCONF_ON_DEMO_REPLAY ) {

		if ( state_valid & STATE_VALID_LIGHTINGCONF ) {

			// restore lighting configuration
			LightColorAmbient  = state_lightcolor_ambient;
			LightColorDiffuse  = state_lightcolor_diffuse;
			LightColorSpecular = state_lightcolor_specular;
			GlobalDirLight     = state_global_dir_light;
		}
	}

	// reset valid bits
	state_valid = STATE_VALID_NONE;
}


// commit demo cut by saving the cut area to file -----------------------------
//
PRIVATE
void CommitDemoCut()
{
	ASSERT( demo_cutpos != NULL );
	ASSERT( demo_data != NULL );

	strcpy( paste_str, demo_cut_name );
	strcat( paste_str, dem_extension );

	FILE *fp = fopen( paste_str, "wb" );
	if ( fp == NULL ) {
		MSGOUT( cut_open_error );
		return;
	}

	// compile the state header
	if ( !CompileDemoCut( fp, demo_cut_name ) ) {
		MSGOUT( cut_write_error );
	}

	// extract the cut area from the original demo
	ASSERT( demo_replaypos >= demo_cutpos );
	size_t demsize = demo_replaypos - demo_cutpos;

	if ( fwrite( demo_cutpos, 1, demsize, fp ) != demsize ) {
		MSGOUT( cut_write_error );
		fclose( fp );
		return;
	}

	// write eof marker and close file
	int endmarker = 0;
	if ( fwrite( &endmarker, sizeof( endmarker ), 1, fp ) != 1 )
		MSGOUT( cut_write_error );
	fclose( fp );
}


// return whether any (binary or script) demo replay in progress --------------
//
int DEMO_ReplayActive()
{
	return ( ScriptReplayActive() || DEMO_BinaryReplayActive() );
}


// flag whether stop function has been called during demo clear ---------------
//
static int stop_from_clear_demo = FALSE;


// stop demo replay (binary and script) if still in progress ------------------
//
void DEMO_StopReplay()
{
	if ( !AUX_DISABLE_CLEARDEMO_ON_REPSTOP && !stop_from_clear_demo ) {

		// clear and stop demo
		DEMO_ClearDemo( TRUE );

	} else {

		// stop without clearing
		ScriptStopReplay();
		DEMO_BinaryStopReplay();
	}
}


// demo replay info variables (also used by scripts) --------------------------
//
PUBLIC refframe_t	demoinfo_curtime  = 0;
PUBLIC int			demoinfo_curline  = 0;
PUBLIC int			demoinfo_curframe = 0;
PUBLIC dword		demoinfo_curofs   = 0;
PUBLIC refframe_t	demoinfo_lastwait = 0;


// init demo replay info variables --------------------------------------------
//
void DEMO_InitInfo()
{
	demoinfo_curtime  = 0;
	demoinfo_curline  = 0;
	demoinfo_curframe = 0;
	demoinfo_curofs   = 0;
	demoinfo_lastwait = 0;
}


// count a recorded demo frame during replay (elapsed wait interval) ----------
//
void DEMO_CountDemoFrame()
{
	//NOTE:
	// this function is called by E_REPLAY::REPLAY_PerformAutomaticActions()
	// every time a wait interval has elapsed. wait intervals are used as
	// "recorded frames".

	// count number of elapsed wait intervals
	demoinfo_curframe++;

	// add elapsed wait interval to demo replay time
	demoinfo_curtime += demoinfo_lastwait;
}


// timedemo flags -------------------------------------------------------------
//
PUBLIC int			timedemo_enabled = FALSE;	// use timedemo on replay
static int			timedemo_active  = FALSE;	// timedemo replay is active
static refframe_t	timedemo_start_time;		// absolute start-time


// display results of timedemo demo replay ------------------------------------
//
PRIVATE
void DisplayTimedemoResults()
{
	// determine replay time and recorded time
	refframe_t deltatime = SYSs_GetRefFrameCount() - timedemo_start_time;
	float    demotime  = (float)deltatime / FRAME_MEASURE_TIMEBASE;
	float    recdtime  = (float)demoinfo_curtime / FRAME_MEASURE_TIMEBASE;

	// determine number of frames replayed and average frame rate
	int		frames  = demoinfo_curtime / DEMO_GetTimedemoBase() + 1;
	float frmrate = frames / demotime;

	CON_AddLineFeed();
	CON_DisableLineFeed();

	// display info
	MSGOUT( "- timedemo results -------------------------" );
	MSGOUT( "total playback time: %.2f", demotime          );
	MSGOUT( "total recorded time: %.2f", recdtime          );
	MSGOUT( "rendered frames:     %d",   frames            );
	MSGOUT( "recorded frames:     %d",   demoinfo_curframe );
	MSGOUT( "average frame rate:  %.2f", frmrate           );
	MSGOUT( "--------------------------------------------" );
}


// activate timedemo demo replay if it is enabled -----------------------------
//
PRIVATE
void DEMO_EnableTimedemo()
{
	// activate if enabled
	timedemo_active = timedemo_enabled;

	// store absolute time demo was started at
	timedemo_start_time = SYSs_GetRefFrameCount();
}


// deactivate timedemo demo replay  -------------------------------------------
//
PRIVATE
void DEMO_DisableTimedemo()
{
	// deactivate
	timedemo_active = FALSE;
}


// get timing for timedemo demo replay ----------------------------------------
//
refframe_t DEMO_GetTimedemoBase()
{
	// render as though we had constant 30fps, but as fast as possible.
	// this ensures we get consistent timing results that can be used
	// for comparison purposes, since everywhere the exact same frames
	// will be rendered (also the exact same number of frames).
	return ( FRAME_MEASURE_TIMEBASE / 30 );
}


// return whether timedemo is currently active --------------------------------
//
int DEMO_TimedemoActive()
{
	return ( DEMO_BinaryReplayActive() && timedemo_active );
}


// return state of binary demo replay -----------------------------------------
//
int DEMO_BinaryReplayActive()
{
	return ( demo_data != NULL );
}


// stop binary replay if still in progress ------------------------------------
//
void DEMO_BinaryStopReplay()
{
	// use full demo clear instead of only stopping if enabled
	if ( !AUX_DISABLE_CLEARDEMO_ON_REPSTOP && !stop_from_clear_demo ) {
		DEMO_ClearDemo( TRUE );
		return;
	}

	// display results if timedemo
	if ( DEMO_TimedemoActive() ) {
		DisplayTimedemoResults();
	}

	// check for cut mode
	if ( demo_cutpos != NULL ) {
		CommitDemoCut();
		demo_cutpos = NULL;
	}

	// free demo data
	if ( demo_data != NULL ) {
		FREEMEM( demo_data );
		demo_data = NULL;
	}

	// disable simulated network if enabled
	if ( NetConnected == NETWORK_GAME_SIMULATED ) {

		// turn off simulated network game and make
		// sure there's no dangling join state
		NetConnected = NETWORK_GAME_OFF;
		NetJoined    = FALSE;
	}

	// stop local ship
	MyShip->CurSpeed = 0;

	// kill dangling automatic actions
	REPLAY_StopAutomaticActions();

	// restore partial state
	RestoreEntryState();

	// set input disabling flag to previous state
	DEMO_EnableUserInput();

	// ensure no dangling action commands
	ResetScheduledActions();
}


// clear remnants of demo replay and stop demo optionally ---------------------
//
void DEMO_ClearDemo( int stopreplay )
{
	//NOTE:
	// this function has to be used after a demo containing network packets
	// has been replayed (and ended) to restore/ensure a proper game state.

	//NOTE:
	// if the network mode is not active or the net simulation has already
	// been stopped (e.g., the demo has finished) the network state will
	// be left untouched (i.e., not reset!).

	// make sure it's off
	ObjCamOff();

	// clean up network state
	if ( NetConnected ) {

		// update player status
		Player_Status[ LocalPlayerId ] = PLAYER_INACTIVE;

		// ensure no remote players
		NET_RemoveRemotePlayers();

		// remove dangling virtual packets
		NETs_FlushListenBuffers();
	}

	if ( stopreplay ) {

		// stop audio stream if started from demo
		if ( con_audio_stream_started ) {
			con_audio_stream_started = FALSE;
			AUDs_StopAudioStream();
		}

		// ensure replay stopped
		stop_from_clear_demo = TRUE;
		DEMO_StopReplay();
		stop_from_clear_demo = FALSE;

		// turn off fading
		SetScreenFade = 0;
	}

	// set proper entry-mode state
	if ( NetConnected ) {
		EntryMode		= NetJoined ? FALSE : TRUE;
		InFloatingMenu	= NetJoined ? FALSE : FloatingMenu;
	} else {
		EntryMode		= FALSE;
		InFloatingMenu	= FloatingMenu;
	}

	// reset the floating menu state
	if ( InFloatingMenu ) {
		ActivateFloatingMenu( FALSE );
	}

	// scroll demo text out
	if ( !AUX_DISABLE_CLEAR_TEXT_ON_CLEARDEMO ) {
//		HUD_ClearDemoText();
	}

	// stop local ship
	MyShip->CurSpeed = 0;

	// ensure automatic actions disabled
	REPLAY_StopAutomaticActions();

	// ensure local particle weapons are destroyed
	WFX_EnsureParticleWeaponsInactive( MyShip );

	// stop residual motion due to view camera filter
	if ( !AUX_DISABLE_CAMFILT_RESET_ON_CLEARDEMO ) {
		CAMERA_ResetFilter();
	}

	// reset mapping table for class ids
	ResetClassMapTable();

	// free objects and particles
	if ( !AUX_DISABLE_OBJECT_FREE_ON_CLEARDEMO ) {
		KillAllObjects();
	}
}


// execute commands in binary demo --------------------------------------------
//
void DEMO_BinaryExecCommands( int interactive )
{
	if ( DEMO_BinaryReplayActive() ) {

		ASSERT( demo_replaypos != NULL );

		if ( ExecBinCommands( demo_replaypos, interactive ) ) {

			// calc current offset (for info only)
			demoinfo_curofs = demo_replaypos - demo_data;

		} else {

			// free resources if demo replay finished
			DEMO_BinaryStopReplay();
		}
	}
}


// set name for demo cut output -----------------------------------------------
//
INLINE
void SetDemoCutName( const char *demo )
{
	ASSERT( demo != NULL );

	size_t len = strlen( demo );

	if ( len < 8 ) {

		// append underscore
		strcpy( demo_cut_name, demo );
		demo_cut_name[ len ] = '_';
		demo_cut_name[ len + 1 ] = 0;

	} else {

		// overwrite last letter with underscore
		strncpy( demo_cut_name, demo, 8 );
		demo_cut_name[ 7 ] = '_';
		demo_cut_name[ 8 ] = 0;
	}
}


// search all console script paths and open demo file if it exists ------------
//
FILE *DEMO_BinaryOpenDemo( const char *demoname )
{
	ASSERT( demoname != NULL );

	FILE *fp = NULL;

	strcpy( paste_str, demoname );
	strcat( paste_str, dem_extension );

	// highest priority: package and local directory
	if ( ( fp = SYS_fopen( paste_str, "rb" ) ) != NULL )
		return fp;

	strcpy( paste_str, REFCON_COMMANDS_DIR );
	strcat( paste_str, demoname );
	strcat( paste_str, dem_extension );

	if ( ( fp = fopen( paste_str, "rb" ) ) != NULL )
		return fp;

	strcpy( paste_str, STDCON_COMMANDS_DIR );
	strcat( paste_str, demoname );
	strcat( paste_str, dem_extension );

	if ( ( fp = fopen( paste_str, "rb" ) ) != NULL )
		return fp;

	strcpy( paste_str, RECORD_COMMANDS_DIR );
	strcat( paste_str, demoname );
	strcat( paste_str, dem_extension );

	return fopen( paste_str, "rb" );
}


// read binary demo file as one huge block ------------------------------------
//
PRIVATE
int DEMO_BinaryReadDemo( FILE *fp, int infomsgs, int errormsgs )
{
	ASSERT( fp != NULL );
	ASSERT( demo_data == NULL );

	// get file size
	if ( SYS_fseek( fp, 0, SEEK_END ) != 0 ) {
		return FALSE;
	}
	size_t demosize = SYS_ftell( fp );
	if ( SYS_fseek( fp, 0, SEEK_SET ) != 0 ) {
		return FALSE;
	}

	// check for demo header and parse it if present
	size_t headersize = DEMO_ParseDemoHeader( fp, -1, infomsgs );
	if ( demosize >= headersize ) {
		demosize -= headersize;
	}
	if ( demosize == 0 ) {
		return FALSE;
	}

	// allocate mem for demo data
	demo_data = (char *) ALLOCMEM( demosize );
	if ( demo_data == NULL ) {
		if ( errormsgs )
			CON_AddLine( "no mem for demo replay." );
		return FALSE;
	}

	// read entire file into single memory block
	if ( SYS_fread( demo_data, 1, demosize, fp ) != demosize ) {
		if ( errormsgs )
			CON_AddLine( "error reading demo file." );
		return FALSE;
	}

	return TRUE;
}


// start playback of binary demo data -----------------------------------------
//
PRIVATE
void DEMO_BinaryStartDemo( int interactive, int enablenet )
{
	ASSERT( demo_data != NULL );

	//NOTE:
	// replay without enabling network replay explicitly
	// is only use for playback behind the main menu.

	//NOTE:
	// if the actual network game is not available we enable
	// the simulation layer to allow replay of recorded packets
	// via E_RECORD::REC_PlayRemotePacketBin().

	// set start replay position
	demo_replaypos = demo_data;

	// init demo info
	DEMO_InitInfo();

	if ( enablenet ) {

		// save partial state
		SaveEntryState();

		// turn off kill limit
		AUX_KILL_LIMIT_FOR_GAME_END = 0;

		// enable simulated network if necessary
		if ( ( NetConnected != NETWORK_GAME_ON ) &&
			 !AUX_DISABLE_REPLAY_NET_SIMULATION ) {

			NetConnected = NETWORK_GAME_SIMULATED;

			// set maximum number of players to peer-to-peer by default.
			// should be set correctly by "ac.remotemaxplayers" later on.
			CurMaxPlayers = MAX_NET_UDP_PEER_PLAYERS;

			// simulated, so set LocalPlayer to 0
			LocalPlayerId =0 ;
		}

		// remove dangling virtual packets
		NETs_FlushListenBuffers();
	}

	// replay until first wait command or eof reached
	DEMO_BinaryExecCommands( interactive );
}


// play demo from binary file behind menu -------------------------------------
//
int DEMO_PlayBehindMenu( const char *demoname )
{
	ASSERT( demoname != NULL );

	// only possible if we are not yet connected
	if ( NetConnected )
		return FALSE;

	// exit if currently replaying
	if ( DEMO_BinaryReplayActive() )
		return FALSE;

	// remember demo name for cut output
	SetDemoCutName( demoname );

	// open demo file
	FILE *fp = DEMO_BinaryOpenDemo( demoname );
	if ( fp == NULL ) {
		return FALSE;
	}

	// read demo data as one big block
	if ( !DEMO_BinaryReadDemo( fp, FALSE, FALSE ) ) {
		SYS_fclose( fp );
		return FALSE;
	}

	// close file since it has been read in its entirety
	SYS_fclose( fp );

	// timedemo is not allowed
	DEMO_DisableTimedemo();

	// start loaded demo
	DEMO_BinaryStartDemo( FALSE, FALSE );

	return TRUE;
}


// start binary demo replay from menu -----------------------------------------
//
int DEMO_PlayFromMenu( const char *demoname )
{
	ASSERT( demoname != NULL );

	// exit if currently replaying
	if ( DEMO_BinaryReplayActive() ) {
		return FALSE;
	}

	// remember demo name for cut output
	SetDemoCutName( demoname );

	// open demo file
	FILE *fp = DEMO_BinaryOpenDemo( demoname );
	if ( fp == NULL ) {
		return FALSE;
	}

	// read demo data as one big block
	if ( !DEMO_BinaryReadDemo( fp, TRUE, TRUE ) ) {
		SYS_fclose( fp );
		return FALSE;
	}

	// close file since it has been read in its entirety
	SYS_fclose( fp );

	// timedemo is allowed
	DEMO_EnableTimedemo();

	// start loaded demo
	DEMO_BinaryStartDemo( FALSE, TRUE );

	// no user interference allowed
	DEMO_DisableUserInput();

	return TRUE;
}


// play demo from binary file -------------------------------------------------
//
int Cmd_PlayBinaryDemoFile( char *command )
{
	//NOTE:
	//CONCOM:
	// play_command ::= 'play' <demo_name>
	// demo_name    ::= "valid script name"

	ASSERT( command != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( command );

	// split off argument (name of demo)
	char *demoname = strtok( command, " " );
	if ( demoname == NULL ) {
		CON_AddLine( arg_missing );
		return TRUE;
	}

	// check for too many arguments
	if ( strtok( NULL, " " ) != NULL ) {
		CON_AddLine( too_many_args );
		return TRUE;
	}

	// exit if currently replaying
	if ( DEMO_BinaryReplayActive() ) {
		CON_AddLine( "demo replay already in progress." );
		return TRUE;
	}

	// remember demo name for cut output
	SetDemoCutName( demoname );

	// open demo file
	FILE *fp = DEMO_BinaryOpenDemo( demoname );
	if ( fp == NULL ) {
		CON_AddLine( "demo file not found." );
		return TRUE;
	}

	// read demo data as one big block
	if ( !DEMO_BinaryReadDemo( fp, TRUE, TRUE ) ) {
		SYS_fclose( fp );
		return TRUE;
	}

	// close file since it has been read in its entirety
	SYS_fclose( fp );

	// timedemo is allowed
	DEMO_EnableTimedemo();

	// start loaded demo
	DEMO_BinaryStartDemo( TRUE, TRUE );

	return TRUE;
}


// console command for cutting a demo during replay ("repcut") ----------------
//
PRIVATE
int Cmd_REPCUT( char *params )
{
	ASSERT( params != NULL );
	USERCOMMAND_NOPARAM( params );

	if ( !DEMO_BinaryReplayActive() ) {
		CON_AddLine( "no binary replay active." );
		return TRUE;
	}

	ASSERT( demo_cutpos == NULL );
	ASSERT( demo_replaypos != NULL );

	// create name for state header script
	ASSERT( demo_cut_name[ 0 ] != '\0' );
	strcpy( paste_str, demo_cut_name );
	strcat( paste_str, CON_FILE_EXTENSION );

	// open state header script
	FILE *fp = fopen( paste_str, "w" );
	if ( fp == NULL ) {
		CON_AddLine( "could not open output file." );
		return TRUE;
	}

	// save frame info as comment
	fprintf( fp, "; time: %d frame: %d line: %d ofs: %d\n",
			 demoinfo_curtime, demoinfo_curframe,
			 demoinfo_curline, demoinfo_curofs );

	// save current state
	Save_StateInfo( fp );

	// terminate with wait command
	fprintf( fp, "%s %d\n", ACMSTR( ACM_IDLEWAIT ), 20 );

	fclose( fp );

	// remember current position for starting cut
	demo_cutpos = demo_replaypos;

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( E_DEMO )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "repcut" command
	regcom.command	 = "repcut";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_REPCUT;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );
}



