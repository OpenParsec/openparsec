/*
 * PARSEC - Main Menu Module
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:37 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-2000
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1999-2000
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

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "inp_defs.h"
#include "net_defs.h"
#include "sys_defs.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "m_main.h"

// proprietary module headers
#include "con_aux.h"
#include "con_ext.h"
#include "e_demo.h"
#include "e_draw.h"
#include "e_supp.h"
#include "g_supp.h"
#include "h_drwhud.h"
#include "h_strmap.h"
#include "h_text.h"
#include "m_button.h"
#include "m_list.h"
#include "m_logo.h"
#include "m_status.h"
#include "m_option.h"
#include "m_viewer.h"
#include "net_conn.h"
#include "net_csdf.h"
#include "obj_creg.h"
#include "g_bot_cl.h"

// flags
#define PLAY_DEMO_BEHIND_MENU



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 2047
static char paste_str[ PASTE_STR_LEN + 1 ];


// demo playback behind menu --------------------------------------------------
//
#define DEMO_FILE		"menudemo"

static int demo_play_possible = TRUE;


// sintab for sliding ---------------------------------------------------------
//
#define SINTAB_SIZE				16
PUBLIC int m_sintab[ SINTAB_SIZE ];
PUBLIC int m_sintab_size		= SINTAB_SIZE;
PUBLIC int m_sintab_ampl		= 320;


// visibility flags for all parts of the menu ---------------------------------
//
static int itemvis_buttons		= TRUE;
static int itemvis_logo 		= TRUE;
static int itemvis_listwindow 	= TRUE;
static int itemvis_optionsmenu	= FALSE;

static int itemvis_viewer		= FALSE;
static int itemvis_gamestatus	= FALSE;

PUBLIC int itemvis_loading		= FALSE;


// transition flags -----------------------------------------------------------
//
enum {

	MENUTRANS_NONE				= 0,

	MENUTRANS_TO_SUBMENU		= 1,
	MENUTRANS_FROM_SUBMENU		= 2,

	MENUTRANS_TO_GAMESTATUS		= 3,
	MENUTRANS_FROM_GAMESTATUS	= 4,

	MENUTRANS_TO_VIEWER			= 5,
	MENUTRANS_FROM_VIEWER		= 6,

	MENUTRANS_OPTIONS_TO_LIST	= 7,
	MENUTRANS_LIST_TO_OPTIONS	= 8,

	MENUTRANS_SHIP_TO_OBJECTS	= 9,
	MENUTRANS_OBJECTS_TO_SHIP	= 10
};

static int menu_state_transition	= MENUTRANS_NONE;


// ----------------------------------------------------------------------------
//
static int prev_GameTime			= 0;


// used when kill limit has been reached --------------------------------------
//
PUBLIC int disable_join_game		= 0;


// mouse vars -----------------------------------------------------------------
//
static int	old_mouse_x				= -1;
static int	old_mouse_y				= -1;
static int	cur_mouse_x;
static int	cur_mouse_y;

int	old_mouse_button_state	= MOUSE_BUTTON_RELEASED;
int	cur_mouse_button_state;


// current mouse position state -----------------------------------------------
//
static int  mouse_position_state = MOUSE_OVER_NOTHING;


// perform options menu exit --------------------------------------------------
//
void ExitOptionsMenu()
{
	//NOTE:
	// called by M_OPTION::ExecOptionSelectExit() when
	// the user wants to exit the options menu.

	// disable options menu
	menu_state_transition = MENUTRANS_OPTIONS_TO_LIST;
	FadeInButtons();
}


// return whether status window is active -------------------------------------
//
int FloatingMenuStatusWindowActive()
{
	//NOTE:
	// this is needed by AUD_BackGroundPlayer_Maintain() to determine
	// whether the background player should be turned off automatically.

	if ( itemvis_gamestatus ) {
		return TRUE;
	}

	// avoid sequencing problems within the same frame
	return ( menu_state_transition == MENUTRANS_TO_GAMESTATUS );
}


// ----------------------------------------------------------------------------
//
static int scv_sound_played = FALSE;
static int opt_sound_played = FALSE;

extern int cur_scv_mode;
extern int cur_scv_ship;

extern int scv_object_selindx;
extern int scv_ring_spinning;


// check whether game status window should be activated -----------------------
//
PRIVATE
int CheckGameStatusWindowEnabling()
{
	if ( !AUX_ENABLE_GAME_STATUS_WINDOW )
		return FALSE;

	if ( itemvis_gamestatus )
		return FALSE;

	if ( menu_state_transition != MENUTRANS_NONE )
		return FALSE;

//FIXME:
/*	if ( NumRemPlayers <= 1 )
		return FALSE;//FIXME:*/
//	if ( !GAME_OVER() )
//		return FALSE;

	MoveOutButtons();
	MoveOutListWindow();

	menu_state_transition = MENUTRANS_TO_GAMESTATUS;

	return TRUE;
}


// activate the floating menu -------------------------------------------------
//
void ActivateFloatingMenu( int statuswin )
{
	ASSERT( InFloatingMenu );

	AUD_StopOnBoardComputer();

	if ( statuswin ) {

		// state switch to enable game status window
		if ( CheckGameStatusWindowEnabling() ) {
			return;
		}

	} else if ( itemvis_gamestatus ) {

		// fade out status window if already visible
		menu_state_transition = MENUTRANS_FROM_GAMESTATUS;
	}

	AUDs_OpenMenuSound();

	MoveOutFloatingMenu();
	SlideInFloatingMenu();
}


// set slide targets for menu items to in -------------------------------------
//
void SlideInFloatingMenu()
{
	if ( !InFloatingMenu )
		return;

	SlideInButtons();
	SlideInLogo();
	SlideInListWindow();
	SlideInOptions();

	FadeInStatusWindow();

	FadeInBackground();
	SlideInShip();
	SlideInRing();
}


// set slide targets for menu items to out ------------------------------------
//
void SlideOutFloatingMenu()
{
	if ( !InFloatingMenu )
		return;

	SlideOutButtons();
	SlideOutLogo();
	SlideOutListWindow();
	SlideOutOptions();

	FadeOutStatusWindow();

	FadeOutBackground();
	SlideOutShip();
	SlideOutRing();
}


// move in all floating menu items --------------------------------------------
//
void MoveInFloatingMenu()
{
	MoveInButtons();
	MoveInLogo();
	MoveInListWindow();
	MoveInOptions();
}


// move out all floating menu items -------------------------------------------
//
void MoveOutFloatingMenu()
{
	MoveOutButtons();
	MoveOutLogo();
	MoveOutListWindow();
	MoveOutOptions();
}


// make floating menu items invisible, prepare for sliding in -----------------
//
void HideFloatingMenu()
{
	MoveOutFloatingMenu();
	SlideInFloatingMenu();
}


// called during transition from menu to spacecraft viewer --------------------
//
PRIVATE
void MaintainTransitionToViewer()
{
	SlideOutButtons();
	SlideOutLogo();
	SlideOutListWindow();
	SlideOutOptions();

	// play "spacecraft viewer in" sample
	if ( !scv_sound_played ) {
		scv_sound_played = TRUE;
		AUD_SpaceCraftViewerIn();
	}

	if ( SlideFinishedButtons() ) {

		FadeInBackground();
		SlideInShip();
		SlideInRing();

		itemvis_buttons		= FALSE;
		itemvis_logo		= FALSE;
		itemvis_listwindow	= FALSE;
		itemvis_viewer		= TRUE;

		cur_scv_mode = SCVMODE_SPACECRAFT;
		cur_scv_ship = ObjClassShipIndex[ LocalShipClass ];
		ASSERT( cur_scv_ship != SHIPINDEX_NO_SHIP );

		scv_sound_played = FALSE;
		menu_state_transition = MENUTRANS_NONE;
	}
}


// called during transition from spacecraft viewer to menu --------------------
//
PRIVATE
void MaintainTransitionFromViewer()
{
	FadeOutBackground();
	SlideOutShip();
	SlideOutRing();

	// play "spacecraft viewer out" sample
	if ( !scv_sound_played ) {
		scv_sound_played = TRUE;
		AUD_SpaceCraftViewerOut();
	}

	if ( FadeFinishedBackground() ) {

		itemvis_buttons		= TRUE;
		itemvis_logo		= TRUE;
		itemvis_listwindow	= TRUE;
		itemvis_viewer		= FALSE;

		SlideInButtons();
		SlideInLogo();
		SlideInListWindow();
		SlideInOptions();

		if ( SlideFinishedButtons() ) {
			scv_sound_played = FALSE;
			menu_state_transition = MENUTRANS_NONE;
		}
	}
}


// called during transition from player list to options menu ------------------
//
PRIVATE
void MaintainTransitionListToOptions()
{
	InitOptionsSubsystemInfo();

	if ( !opt_sound_played ) {
		opt_sound_played = TRUE;
		AUD_SlideIn( 0 );
	}

	SlideOutListWindow();
	if ( SlideFinishedListWindow() ) {

		itemvis_listwindow	= FALSE;
		itemvis_optionsmenu = TRUE;

		SlideInOptions();

		opt_sound_played = FALSE;
		menu_state_transition = MENUTRANS_NONE;
	}
}


// called during transition from options menu to player list ------------------
//
PRIVATE
void MaintainTransitionOptionsToList()
{
	if ( !opt_sound_played ) {
		AUD_SlideOut( 0 );
		opt_sound_played = TRUE;
	}

	SlideOutOptions();
	if ( SlideFinishedOptions() ) {

		itemvis_listwindow	= TRUE;
		itemvis_optionsmenu = FALSE;

		SlideInListWindow();

		opt_sound_played = FALSE;
		menu_state_transition = MENUTRANS_NONE;
	}
}


// called during transition from ship to objects in spacecraft viewer ---------
//
PRIVATE
void MaintainTransitionShipToObjects()
{
	SlideOutShip();
	if ( SlideFinishedShip() ) {
		scv_object_selindx = 0;
		cur_scv_mode = SCVMODE_OBJECTSRING;
		SlideInRing();
		menu_state_transition = MENUTRANS_NONE;
	}
}


// called during transition from objects to ship in spacecraft viewer ---------
//
PRIVATE
void MaintainTransitionObjectsToShip()
{
	SlideOutRing();
	if ( SlideFinishedRing() ) {
		cur_scv_mode = SCVMODE_SPACECRAFT;
		SlideInShip();
		menu_state_transition = MENUTRANS_NONE;
	}
}


// called during transition to submenu ----------------------------------------
//
PRIVATE
void MaintainTransitionToSubMenu()
{
	FadeOutButtons( BUTTON_ALPHA_LOW );

	if ( FadeFinishedButtons() ) {

		MenuButtonsEnterSubmenu();
		FadeInButtons();
		menu_state_transition = MENUTRANS_NONE;
	}
}


// called during transition from submenu --------------------------------------
//
PRIVATE
void MaintainTransitionFromSubMenu()
{
	FadeOutButtons( BUTTON_ALPHA_LOW );

	if ( FadeFinishedButtons() ) {

		MenuButtonsLeaveSubmenu();
		FadeInButtons();
		menu_state_transition = MENUTRANS_NONE;
	}
}


// called during transition to game status window -----------------------------
//
PRIVATE
void MaintainTransitionToGameStatus()
{
	if ( !itemvis_gamestatus ) {

		SlideOutButtons();
		SlideOutListWindow();

		if ( SlideFinishedButtons() ) {

			itemvis_buttons		= FALSE;
			itemvis_listwindow	= FALSE;
			itemvis_gamestatus	= TRUE;

		} else {

			ASSERT( itemvis_buttons );
			ASSERT( itemvis_listwindow );
		}

	} else {

		ASSERT( !itemvis_buttons );
		ASSERT( !itemvis_listwindow );

		FadeInStatusWindow();

		if ( FadeFinishedStatusWindow() ) {
			menu_state_transition = MENUTRANS_NONE;
		}
	}
}


// called during transition from game status window ---------------------------
//
PRIVATE
void MaintainTransitionFromGameStatus()
{
	if ( itemvis_gamestatus ) {

		FadeOutStatusWindow();

		if ( FadeFinishedStatusWindow() ) {

			itemvis_buttons		= TRUE;
			itemvis_listwindow	= TRUE;
			itemvis_gamestatus	= FALSE;

		} else {

			ASSERT( !itemvis_buttons );
			ASSERT( !itemvis_listwindow );
		}

	} else {

		ASSERT( itemvis_buttons );
		ASSERT( itemvis_listwindow );

		SlideInButtons();
		SlideInListWindow();

		if ( SlideFinishedButtons() ) {
			menu_state_transition = MENUTRANS_NONE;
			AUDs_OpenMenuSound();
		}
	}
}


// check whether game status window should be deactivated ---------------------
//
PRIVATE
void CheckGameStatusWindowDisabling()
{
	if ( ( prev_GameTime >= 0 ) && ( CurGameTime >= 0 ) ) {

		// game is still running, change nothing
		prev_GameTime = CurGameTime;

	} else if ( ( prev_GameTime < 0 ) && ( CurGameTime >= 0 ) ) {

		// game has been started...
		menu_state_transition = MENUTRANS_FROM_GAMESTATUS;

		prev_GameTime = CurGameTime;

	} else {

		prev_GameTime = CurGameTime;
	}
}


// check whether demo should/can be started behind the menu -------------------
//
PRIVATE
void CheckMenuDemoReplay()
{

#ifdef PLAY_DEMO_BEHIND_MENU

	if ( AUX_DISABLE_DEMO_REPLAY_IN_MENU )
		return;

	if ( demo_play_possible ) {

		if ( !DEMO_ReplayActive() && !NetConnected ) {
			demo_play_possible = DEMO_PlayBehindMenu( DEMO_FILE );
			AUX_STOP_DEMO_REPLAY_ON_CONNECT = 1;
		}
	}

#endif

}


// restore menu state if it got stuck -----------------------------------------
//
void EnsureStateConsistency()
{
	//NOTE:
	// the following state must be detected as an invalid menu
	// condition:
	// itemvis_buttons		= 0
	// itemvis_logo			= 1
	// itemvis_listwindow	= 0
	// itemvis_optionsmenu	= 0
	// itemvis_viewer		= 0
	// itemvis_gamestatus	= 0
	// itemvis_loading		= 0

	if ( !itemvis_logo )
		return;
	if ( itemvis_buttons )
		return;
	if ( itemvis_listwindow )
		return;
	if ( itemvis_optionsmenu )
		return;
	if ( itemvis_viewer )
		return;
	if ( itemvis_gamestatus )
		return;
	if ( itemvis_loading )
		return;

	// we simply restore the buttons
	// and the listwindow
	itemvis_buttons		= TRUE;
	itemvis_listwindow	= TRUE;

	MoveInButtons();
	MoveInListWindow();
}


// call corresponding maintenance function if transition in progress ----------
//
PRIVATE
void MaintainMenuStateTransitions()
{
	// maintain state transitions
	switch ( menu_state_transition ) {

		case MENUTRANS_NONE:
			EnsureStateConsistency();
			break;

		// menu -> submenu
		case MENUTRANS_TO_SUBMENU:
			MaintainTransitionToSubMenu();
			break;

		// submenu -> menu
		case MENUTRANS_FROM_SUBMENU:
			MaintainTransitionFromSubMenu();
			break;

		// menu/game -> game status
		case MENUTRANS_TO_GAMESTATUS:
			MaintainTransitionToGameStatus();
			break;

		// game status -> menu
		case MENUTRANS_FROM_GAMESTATUS:
			MaintainTransitionFromGameStatus();
			break;

		// menu -> spacecraft viewer
		case MENUTRANS_TO_VIEWER:
			MaintainTransitionToViewer();
			break;

		// spacecraft viewer -> menu
		case MENUTRANS_FROM_VIEWER:
			MaintainTransitionFromViewer();
			break;

		// player list -> options menu
		case MENUTRANS_LIST_TO_OPTIONS:
			MaintainTransitionListToOptions();
			break;

		// options menu -> player list
		case MENUTRANS_OPTIONS_TO_LIST:
			MaintainTransitionOptionsToList();
			break;

		// ships -> objects in spacecraft viewer
		case MENUTRANS_SHIP_TO_OBJECTS:
			MaintainTransitionShipToObjects();
			break;

		// objects -> ships in spacecraft viewer
		case MENUTRANS_OBJECTS_TO_SHIP:
			MaintainTransitionObjectsToShip();
			break;
	}
}


// currently selected sample quality is defined in an audio module ------------
//
static int prev_sample_quality = -1;


// ----------------------------------------------------------------------------
//
PRIVATE
void HandleMenuLoading()
{
	if ( prev_sample_quality == -1 ) {
		prev_sample_quality = aud_sample_quality;
	}

	if ( itemvis_loading ) {

		// draw loading bitmap
		DrawMenuLoading();

		// disable loading bitmap now, which triggers the loading and
		// ensures that the bitmap is removed as soon as loading has
		// finished
		itemvis_loading = FALSE;

		prev_sample_quality = aud_sample_quality;

		extern int cur_sfxquality;

		if ( cur_sfxquality == 0 ) {
			aud_sample_quality = 44100;
		} else if ( cur_sfxquality == 1 ) {
			aud_sample_quality = 22050;
		} else {
			aud_sample_quality = 11025;
		}

	} else if ( aud_sample_quality != prev_sample_quality ) {
		const char * soundfile = "_sound";
		ExecExternalFile( (char *) soundfile );
		prev_sample_quality = aud_sample_quality;
	}
}


// draw all visible menu items ------------------------------------------------
//
PRIVATE
void DrawMenuItems()
{
	// draw the menu buttons
	if ( itemvis_buttons ) {
		DrawMenuButtons();
	}

	// draw the rotating logo
	if ( itemvis_logo ) {
		DrawMenuLogo();
	}

	// draw the currently active list window
	if ( itemvis_listwindow ) {
		DrawMenuListWindow();
	}

	// draw the options menu
	if ( itemvis_optionsmenu ) {
		DrawOptionsMenuWindow();
	}

	// draw the blinking connect indicator
	if ( NetConnected && !itemvis_viewer ) {
		DrawMenuConnected();
	}

	// draw the game status window
	if ( itemvis_gamestatus ) {
		DrawStatusWindow();
	}

	// draw the spacecraft viewer
	if ( itemvis_viewer ) {
		DrawSpacecraftViewer();
	}
}


// draw floating menu (overlay menu in entry mode) ----------------------------
//
void DrawFloatingMenu()
{
	if(headless_bot)
		return;
	ASSERT( InFloatingMenu );
/*
	// automatic disabling of game status window
	if ( itemvis_gamestatus ) {
		CheckGameStatusWindowDisabling();
	}
*/
	// init vidmode state variables for options menu
	InitOptionsWindow();

	// play demo in the background if enabled
	CheckMenuDemoReplay();

	// maintain state transitions
	MaintainMenuStateTransitions();

	// draw all visible menu items
	DrawMenuItems();

	// handle automatic reloads in menu
	HandleMenuLoading();
}


// notify menu code of successful network connect -----------------------------
//
void MenuNotifyConnect()
{
/*	//TODO:
	// make server list current
	listwin_mode = LISTWIN_SERVERLIST;
*/
	// make player list current
	listwin_mode = LISTWIN_PLAYERLIST;

	// toggle buttons
	MenuButtonsToggleConnect();
}


// notify menu code of successful network disconnect --------------------------
//
void MenuNotifyDisconnect()
{
	// make server list current
	listwin_mode = LISTWIN_SERVERLIST;

	// toggle buttons
	MenuButtonsToggleDisconnect();
}


// enter game mode ("join game") ----------------------------------------------
//
void MENU_FloatingMenuEnterGame()
{
	HideFloatingMenu();
	HideStatusWindow();

	// close menu sound
	AUDs_CloseMenuSound();

	// play sample for booting the on-board computer
	AUD_BootOnBoardComputer();

#ifdef PLAY_DEMO_BEHIND_MENU

	// stop the demo playback
	if ( DEMO_BinaryReplayActive() ) {
		DEMO_BinaryStopReplay();
	}

	AUX_STOP_DEMO_REPLAY_ON_CONNECT = 0;

#endif

	// init ship stuff
	ASSERT( LocalShipClass < MAX_DISTINCT_OBJCLASSES );
	ASSERT( ObjClassShipIndex[ LocalShipClass ] < NumShipClasses );
	InitLocalShipStatus( LocalShipClass );
	InitHudDisplay();
	InitFloatingMyShip();

	// enter network game if possible
	if ( NetConnected ) {

		InitJoinPosition();
		NETs_Join();
	}

	EntryMode		= FALSE;
	InFloatingMenu	= FALSE;
	ExitGameLoop	= 0;
}


// determine whether joining is possible in the game status window ------------
//
INLINE
int GameStatusJoinAllowed()
{
	if ( GAME_RUNNING() )
		return TRUE;
	if ( GAME_NO_SERVER() )
		return TRUE;
	if ( !NetConnected )
		return TRUE;
	if ( NumRemPlayers <= 1 )
		return TRUE;

	return FALSE;
}


// join game from game status window ------------------------------------------
//
PRIVATE
void GameStatusEnterGame()
{
	if ( AUX_DISABLE_JOIN_GAME_BUTTON || disable_join_game )
		return;

	if ( !GameStatusJoinAllowed() )
		return;

	itemvis_gamestatus = FALSE;

	MENU_FloatingMenuEnterGame();
}


// exit game status window ----------------------------------------------------
//
PRIVATE
void GameStatusKeyEscape()
{
	menu_state_transition = MENUTRANS_FROM_GAMESTATUS;
}


// the escape key quits submenus, or selects quit in the toplevel menu --------
//
void ExecMenuEscape()
{
	if ( MenuButtonsEscape() ) {

		// play sound to denote quit selection
		AUD_Select1();

	} else {

		// in submenus, <ESC> goes back to the previous menu
		menu_state_transition = MENUTRANS_FROM_SUBMENU;
	}
}


// main menu item selected: game submenu --------------------------------------
//
INLINE
void MenuItemSelectGame()
{
	menu_state_transition = MENUTRANS_TO_SUBMENU;
}


// main menu item selected: server submenu ------------------------------------
//
INLINE
void MenuItemSelectServer()
{
	//if ( AUX_DISABLE_SERVER_MENU_BUTTON )
	//	return;

	// make server list current if suitable
	if ( listwin_mode != LISTWIN_PLAYERLIST ) {
		listwin_mode = LISTWIN_SERVERLIST;
	}

	menu_state_transition = MENUTRANS_TO_SUBMENU;
}


// main menu item selected: config submenu ------------------------------------
//
INLINE
void MenuItemSelectConfig()
{
	menu_state_transition = MENUTRANS_TO_SUBMENU;
}


// main menu item selected: quit game -----------------------------------------
//
INLINE
void MenuItemSelectQuit()
{
	if ( AUX_DISABLE_QUIT_BUTTON )
		return;

	ExitGameLoop = 1;
}


// game menu item selected: connect -------------------------------------------
//
INLINE
void MenuItemSelectConnect()
{
	// connect and notify menu
	if(NET_ProtocolGMSV()) {
        if ( NET_CommandConnect( "servers.openparsec.com" ) ) {
		    MenuNotifyConnect();
	    }
    } else {
        if ( NET_CommandConnect( "peers" ) ) {
		    MenuNotifyConnect();
	    }    
    }
}


// game menu item selected: join game -----------------------------------------
//
PRIVATE
void MenuItemSelectJoinGame()
{
	if ( AUX_DISABLE_JOIN_GAME_BUTTON || disable_join_game )
		return;

	MENU_FloatingMenuEnterGame();
}


// game menu item selected: starmap -------------------------------------------
//
INLINE
void MenuItemSelectStarmap()
{
	MAP_ActivateStarmap();
}


// game menu item selected: play demo -----------------------------------------
//
INLINE
void MenuItemSelectPlayDemo()
{
	if ( listwin_active || itemvis_gamestatus )
		return;

	// make demo list current
	listwin_mode = LISTWIN_DEMOLIST;

	// activate demo list if not already active
	listwin_active = TRUE;

	// make sure the button does at least something for mouse users
	if ( mouse_position_state != MOUSE_OVER_NOTHING ) {
		DemoListMouseSet();
	}
}


// game menu item selected: disconnect ----------------------------------------
//
INLINE
void MenuItemSelectDisconnect()
{
	if ( AUX_DISABLE_DISCONNECT_BUTTON )
		return;

	// disconnect and notify menu
	if ( NET_CommandDisconnect() ) {
		MenuNotifyDisconnect();
	}
}


// server menu item selected: create server -----------------------------------
//
INLINE
void MenuItemSelectCreate()
{
	//TODO:
}


// server menu item selected: game mod ----------------------------------------
//
INLINE
void MenuItemSelectGameMod()
{
//	menu_state_transition = MENUTRANS_LIST_TO_GAMEMODS;
}


// server menu item selected: server settings ---------------------------------
//
INLINE
void MenuItemSelectSettings()
{
	if ( AUX_DISABLE_SERVER_SETTINGS_BUTTON )
		return;

//	menu_state_transition = MENUTRANS_LIST_TO_SETTINGS;
}


// conf menu item selected: spacecraft viewer ---------------------------------
//
INLINE
void MenuItemSelectSpacecraft()
{
	extern int scv_ring_objects[];

	// init all objects for the ring
	for ( int oid = 0; oid < NUM_SCV_OBJECTS; oid++ ) {
		
		dword classid = ExtraClasses[ scv_ring_objects[ oid ] ];
		
		if ( classid == CLASS_ID_INVALID ) {
			continue;
		}
		
		MakeIdMatrx( ObjClasses[ classid ]->ObjPosition );
	}

	menu_state_transition = MENUTRANS_TO_VIEWER;
}


// conf menu item selected: controls dialog -----------------------------------
//
INLINE
void MenuItemSelectControls()
{
//	menu_state_transition = MENUTRANS_LIST_TO_CONTROLS;
}


// conf menu item selected: show options menu ---------------------------------
//
INLINE
void MenuItemSelectOptions()
{
	menu_state_transition = MENUTRANS_LIST_TO_OPTIONS;
	FadeOutButtons( BUTTON_ALPHA_MID );
	OptionsListSelectDefault();
}


// submenu item selected: back to previous menu -------------------------------
//
INLINE
void MenuItemSelectBack()
{
	menu_state_transition = MENUTRANS_FROM_SUBMENU;
}


// select item of main menu ---------------------------------------------------
//
PRIVATE
void ExecMenuitemSelection( int menuitem )
{
	// invoke corresponding function
	switch ( menuitem ) {

		// game submenu
		case M_GAME:
			MenuItemSelectGame();
			break;

		// server submenu
		case M_SERVER:
			MenuItemSelectServer();
			break;

		// config submenu
		case M_CONFIG:
			MenuItemSelectConfig();
			break;

		// quit
		case M_QUIT:
			MenuItemSelectQuit();
			break;

		// connect
		case M_CONNECT:
			MenuItemSelectConnect();
			break;

		// join game
		case M_JOIN_GAME:
			MenuItemSelectJoinGame();
			break;

		// starmap
		case M_STARMAP:
			MenuItemSelectStarmap();
			break;

		// play demo
		case M_PLAY_DEMO:
			MenuItemSelectPlayDemo();
			break;

		// disconnect
		case M_DISCONNECT:
			MenuItemSelectDisconnect();
			break;

		// create server
		case M_CREATE:
			MenuItemSelectCreate();
			break;

		// game mod
		case M_GAME_MOD:
			MenuItemSelectGameMod();
			break;

		// server settings
		case M_SETTINGS:
			MenuItemSelectSettings();
			break;

		// spacecraft
		case M_SPACECRAFT:
			MenuItemSelectSpacecraft();
			break;

		// controls
		case M_CONTROLS:
			MenuItemSelectControls();
			break;

		// options
		case M_OPTIONS:
			MenuItemSelectOptions();
			break;

		// back
		case M_BACK:
			MenuItemSelectBack();
			break;
	}
}


// select (and play) demo from demo list --------------------------------------
//
PRIVATE
void ExecDemoListSelection( int selindx )
{
	// check for "free flight mode" demo
	if ( selindx == -1 ) {

		MenuItemSelectJoinGame();

	} else if ( !NetConnected ) {

		// close menu sound
		AUDs_CloseMenuSound();

		if ( DEMO_BinaryReplayActive() ) {
			DEMO_BinaryStopReplay();
		}

		DEMO_PlayFromMenu( registered_demo_names[ selindx ] );
		AUX_STOP_DEMO_REPLAY_ON_CONNECT = 1;
	}

	listwin_active = FALSE;
}


// select item from list ------------------------------------------------------
//
PRIVATE
void ExecListSelection()
{
	switch ( listwin_mode ) {

		case LISTWIN_DEMOLIST:
			ExecDemoListSelection( DemoListSelection() );
			break;

		case LISTWIN_PLAYERLIST:
//			ExecPlayerListSelection( selindx );
			break;

		case LISTWIN_SERVERLIST:
	//		ExecServerListSelection( selindx );
			break;
	}
}


// move cursor up in list window ----------------------------------------------
//
PRIVATE
void ListCursorUp()
{
	switch ( listwin_mode ) {

		case LISTWIN_DEMOLIST:
			DemoListCursorUp();
			break;

		case LISTWIN_PLAYERLIST:
//			PlayerListCursorUp();
			break;

		case LISTWIN_SERVERLIST:
//			ServerListCursorUp( selindx );
			break;
	}
}


// move cursor down in list window --------------------------------------------
//
PRIVATE
void ListCursorDown()
{
	switch ( listwin_mode ) {

		case LISTWIN_DEMOLIST:
			DemoListCursorDown();
			break;

		case LISTWIN_PLAYERLIST:
//			PlayerListCursorDown();
			break;

		case LISTWIN_SERVERLIST:
//			ServerListCursorDown( selindx );
			break;
	}
}


// action for <ENTER> pressed in spacecraft viewer ----------------------------
//
PRIVATE
void SpacecraftViewerKeyEnter()
{
	switch ( cur_scv_mode ) {

		case SCVMODE_SPACECRAFT:
			ASSERT( cur_scv_ship < NumShipClasses );
			LocalShipClass = ShipClasses[ cur_scv_ship ];
			menu_state_transition = MENUTRANS_FROM_VIEWER;
			break;

		case SCVMODE_OBJECTSRING:
			menu_state_transition = MENUTRANS_OBJECTS_TO_SHIP;
			break;

		default:
			ASSERT( 0 );
			break;
	}
}


// action for <ESC> pressed in spacecraft viewer ------------------------------
//
PRIVATE
void SpacecraftViewerKeyEscape()
{
	// exit spacecraft viewer
	menu_state_transition = MENUTRANS_FROM_VIEWER;
}


// action for <CURSOR-UP> pressed in spacecraft viewer ------------------------
//
PRIVATE
void SpacecraftViewerKeyCursorUp()
{
	switch ( cur_scv_mode ) {

		case SCVMODE_SPACECRAFT:
			menu_state_transition = MENUTRANS_SHIP_TO_OBJECTS;
			AUD_Select1();
			break;

		case SCVMODE_OBJECTSRING:
			menu_state_transition = MENUTRANS_OBJECTS_TO_SHIP;
			AUD_Select1();
			break;

		default:
			ASSERT( 0 );
			break;
	}
}


// action for <CURSOR-DOWN> pressed in spacecraft viewer ----------------------
//
PRIVATE
void SpacecraftViewerKeyCursorDown()
{
	switch ( cur_scv_mode ) {

		case SCVMODE_SPACECRAFT:
			menu_state_transition = MENUTRANS_SHIP_TO_OBJECTS;
			AUD_Select1();
			break;

		case SCVMODE_OBJECTSRING:
			menu_state_transition = MENUTRANS_OBJECTS_TO_SHIP;
			AUD_Select1();
			break;

		default:
			ASSERT( 0 );
			break;
	}
}


// action for <CURSOR-LEFT> pressed in spacecraft viewer ----------------------
//
PRIVATE
void SpacecraftViewerKeyCursorLeft()
{
	switch ( cur_scv_mode ) {

		case SCVMODE_SPACECRAFT:
			if ( --cur_scv_ship < 0 ) {
				cur_scv_ship = NumShipClasses - 1;
			}
			AUD_Select2();
			break;

		case SCVMODE_OBJECTSRING:
			scv_ring_spinning = 1;
			AUD_Select2();
			break;

		default:
			ASSERT( 0 );
			break;
	}
}


// action for <CURSOR-RIGHT> pressed in spacecraft viewer ---------------------
//
PRIVATE
void SpacecraftViewerKeyCursorRight()
{
	switch ( cur_scv_mode ) {

		case SCVMODE_SPACECRAFT:
			if ( ++cur_scv_ship >= NumShipClasses ) {
				cur_scv_ship = 0;
			}
			AUD_Select2();
			break;

		case SCVMODE_OBJECTSRING:
			scv_ring_spinning = -1;
			AUD_Select2();
			break;

		default:
			ASSERT( 0 );
			break;
	}
}


// action for <ENTER> pressed in menu -----------------------------------------
//
PRIVATE
void FloatingMenuKeyEnter()
{
	if ( itemvis_optionsmenu ) {

		ExecOptionSelection( OptionsListSelection() );

	} else if ( itemvis_viewer ) {

		SpacecraftViewerKeyEnter();

	} else if ( itemvis_gamestatus ) {

		GameStatusEnterGame();

	} else if ( listwin_active ) {

		ExecListSelection();

	} else {

		ExecMenuitemSelection( MenuButtonsSelection() );
	}
}


// action for <ESC> pressed in menu -------------------------------------------
//
PRIVATE
void FloatingMenuKeyEscape()
{
	// key checking only if no transition in progress
	if ( menu_state_transition != MENUTRANS_NONE )
		return;

	if ( itemvis_gamestatus ) {

		GameStatusKeyEscape();

	} else if ( itemvis_viewer ) {

		SpacecraftViewerKeyEscape();

	} else if ( itemvis_optionsmenu ){

		// exit options menu
		ExecOptionSelection( EXIT_OPT );

	} else if ( listwin_active ) {

		// move cursor from list window to buttons
		listwin_active = FALSE;

	} else {

		ExecMenuEscape();
	}
}


// action for <CURSOR-UP> pressed in menu -------------------------------------
//
PRIVATE
void FloatingMenuKeyCursorUp()
{
	if ( itemvis_optionsmenu ) {

		OptionsListCursorUp();

 	} else if ( itemvis_viewer ) {

		SpacecraftViewerKeyCursorUp();

 	} else if ( listwin_active ) {

		ListCursorUp();

 	} else {

		MenuButtonsCursorUp();
	}
}


// action for <CURSOR-DOWN> pressed in menu -----------------------------------
//
PRIVATE
void FloatingMenuKeyCursorDown()
{
	if ( itemvis_optionsmenu ) {

		OptionsListCursorDown();

 	} else if ( itemvis_viewer ) {

		SpacecraftViewerKeyCursorDown();

	} else if ( listwin_active ) {

		ListCursorDown();

 	} else {

		MenuButtonsCursorDown();
	}
}


// action for <CURSOR-LEFT> pressed in menu -----------------------------------
//
PRIVATE
void FloatingMenuKeyCursorLeft()
{
	if ( itemvis_viewer ) {

		SpacecraftViewerKeyCursorLeft();
	}
}


// action for <CURSOR-RIGHT> pressed in menu ----------------------------------
//
PRIVATE
void FloatingMenuKeyCursorRight()
{
	if ( itemvis_viewer ) {

		SpacecraftViewerKeyCursorRight();
	}
}


// ensure mouse is properly turned off ----------------------------------------
//
INLINE
void FloatingMenuMouseOff()
{
	// ensure it will be detected as
	// moved when turned on again
	old_mouse_x = -1;
	old_mouse_y = -1;

	// kill dangling button presses
	cur_mouse_button_state = MOUSE_BUTTON_RELEASED;
}


// handle mouse actions for floating menu -------------------------------------
//
PRIVATE
void FloatingMenuMouseGetState()
{
	// avoid dangling position state
	mouse_position_state = MOUSE_OVER_NOTHING;

	// retrieve mouse state
	mousestate_s mousestate;
	int mouseavailable = INPs_MouseGetState( &mousestate );

	// make sure mouse is there
	if ( !mouseavailable ) {
		FloatingMenuMouseOff();
		return;
	}

	cur_mouse_x = (int) ( mousestate.xpos * Screen_Width );
	cur_mouse_y = (int) ( mousestate.ypos * Screen_Height );
	cur_mouse_button_state = mousestate.buttons[ MOUSE_BUTTON_LEFT ];

	// if button released check only if mouse has been moved since last frame
	if ( cur_mouse_button_state == MOUSE_BUTTON_RELEASED ) {
		if ( ( cur_mouse_x == old_mouse_x ) && ( cur_mouse_y == old_mouse_y ) ) {
			return;
		}
	}

	old_mouse_x = cur_mouse_x;
	old_mouse_y = cur_mouse_y;

	// area checking only if no transition in progress
	if ( menu_state_transition != MENUTRANS_NONE )
		return;

	// check if mouse is over any clickable area
	if ( itemvis_optionsmenu ) {
		mouse_position_state = MouseOverOption( cur_mouse_x, cur_mouse_y );
	} else if ( itemvis_viewer ) {
		mouse_position_state = MouseOverViewerItem( cur_mouse_x, cur_mouse_y );
	} else {
		if ( itemvis_listwindow ) {
			mouse_position_state = MouseOverListItem( cur_mouse_x, cur_mouse_y );
			if ( mouse_position_state != MOUSE_OVER_NOTHING ) {
				listwin_active = TRUE;
			}
		}
		if ( mouse_position_state == MOUSE_OVER_NOTHING ) {
			mouse_position_state = MouseOverMenuButton( cur_mouse_x, cur_mouse_y );
			if ( mouse_position_state != MOUSE_OVER_NOTHING ) {
				listwin_active = FALSE;
			}
		}
	}
}


// must be called before using key via mouse simulation functions -------------
//
INLINE
void FloatingMenuMouseBegin()
{
	if ( AUX_ENABLE_MOUSE_FOR_MENU_SCREENS ) {
		FloatingMenuMouseGetState();
	} else {
		FloatingMenuMouseOff();
	}
}


// must be called after using key via mouse simulation functions --------------
//
INLINE
void FloatingMenuMouseEnd()
{
	// remember state for edge detection
	old_mouse_button_state = cur_mouse_button_state;
}


// determine whether enter simulated with mouse button ------------------------
//
INLINE
int FloatingMenuMousePressedEnter()
{
	int mousepressed =
		( ( cur_mouse_button_state != old_mouse_button_state ) &&
		  ( cur_mouse_button_state == MOUSE_BUTTON_PRESSED ) &&
		  ( mouse_position_state & MOUSE_OVER_ENTER_SIMUL ) );

	return mousepressed;
}


// determine whether cursor up simulated with mouse button --------------------
//
INLINE
int FloatingMenuMousePressedCursorUp()
{
	int mousepressed =
		( ( cur_mouse_button_state != old_mouse_button_state ) &&
		  ( cur_mouse_button_state == MOUSE_BUTTON_PRESSED ) &&
		  ( mouse_position_state & MOUSE_OVER_CURUP_SIMUL ) );

	return mousepressed;
}


// determine whether cursor down simulated with mouse button ------------------
//
INLINE
int FloatingMenuMousePressedCursorDown()
{
	int mousepressed =
		( ( cur_mouse_button_state != old_mouse_button_state ) &&
		  ( cur_mouse_button_state == MOUSE_BUTTON_PRESSED ) &&
		  ( mouse_position_state & MOUSE_OVER_CURDN_SIMUL ) );

	return mousepressed;
}


// determine whether cursor left simulated with mouse button ------------------
//
INLINE
int FloatingMenuMousePressedCursorLeft()
{
	int mousepressed =
		( ( cur_mouse_button_state != old_mouse_button_state ) &&
		  ( cur_mouse_button_state == MOUSE_BUTTON_PRESSED ) &&
		  ( mouse_position_state & MOUSE_OVER_CURLF_SIMUL ) );

	return mousepressed;
}


// determine whether cursor right simulated with mouse button -----------------
//
INLINE
int FloatingMenuMousePressedCursorRight()
{
	int mousepressed =
		( ( cur_mouse_button_state != old_mouse_button_state ) &&
		  ( cur_mouse_button_state == MOUSE_BUTTON_PRESSED ) &&
		  ( mouse_position_state & MOUSE_OVER_CURRT_SIMUL ) );

	return mousepressed;
}


// for sticky rotation of objects ring ----------------------------------------
//
PUBLIC int rotkey_active_left  = FALSE;
PUBLIC int rotkey_active_right = FALSE;


// check if enter key is pressed (or simulated) -------------------------------
//
INLINE
int FloatingMenuKeyPressedEnter()
{
	int keypressed = SELECT_KEYS_PRESSED();
	if ( keypressed ) {
		SELECT_KEYS_RESET();
	}

	return ( keypressed || FloatingMenuMousePressedEnter() );
}


// check if cursor up is pressed (or simulated) -------------------------------
//
INLINE
int FloatingMenuKeyPressedCursorUp()
{
	int keypressed = DepressedKeys->key_CursorUp;
//	if ( keypressed ) {
		DepressedKeys->key_CursorUp = 0;
//	}

	return ( keypressed || FloatingMenuMousePressedCursorUp() );
}


// check if cursor down is pressed (or simulated) -----------------------------
//
INLINE
int FloatingMenuKeyPressedCursorDown()
{
	int keypressed = DepressedKeys->key_CursorDown;
//	if ( keypressed ) {
		DepressedKeys->key_CursorDown = 0;
//	}

	return ( keypressed || FloatingMenuMousePressedCursorDown() );
}


// check if cursor left is pressed (or simulated) -----------------------------
//
INLINE
int FloatingMenuKeyPressedCursorLeft()
{
	int keypressed = ( DepressedKeys->key_CursorLeft && !rotkey_active_left );
	if ( keypressed ) {
//		DepressedKeys->key_CursorLeft = 0;
		rotkey_active_left = TRUE;
	}

	return ( keypressed || FloatingMenuMousePressedCursorLeft() );
}


// check if cursor right is pressed (or simulated) ----------------------------
//
INLINE
int FloatingMenuKeyPressedCursorRight()
{
	int keypressed = ( DepressedKeys->key_CursorRight && !rotkey_active_right );
	if ( keypressed ) {
//		DepressedKeys->key_CursorRight = 0;
		rotkey_active_right = TRUE;
	}

	return ( keypressed || FloatingMenuMousePressedCursorRight() );
}


// handle standard keys in floating menu --------------------------------------
//
PRIVATE
void FloatingMenuKeyStandard()
{
	// standard key checking only if no transition in progress
	if ( menu_state_transition != MENUTRANS_NONE )
		return;

	if ( FloatingMenuKeyPressedEnter() ) {

		FloatingMenuKeyEnter();

	} else if ( FloatingMenuKeyPressedCursorUp() ) {

		FloatingMenuKeyCursorUp();

	} else if ( FloatingMenuKeyPressedCursorDown() ) {

		FloatingMenuKeyCursorDown();

	} else if ( FloatingMenuKeyPressedCursorLeft() ) {

		FloatingMenuKeyCursorLeft();

	} else if ( FloatingMenuKeyPressedCursorRight() ) {

		FloatingMenuKeyCursorRight();
	}
}


// handle keyboard input for menu ---------------------------------------------
//
void FloatingMenuKeyHandler()
{
	ASSERT( FloatingMenu );
	ASSERT( InFloatingMenu );

	//NOTE:
	// this function is called each frame by the
	// gameloop via G_MAIN::Gm_FloatingMenuSpecifics().

	// allow mouse for keyboard simulation
	FloatingMenuMouseBegin();

	if ( ExitGameLoop && ExitGameLoop != 3 ) {

		//NOTE:
		// we intercept the game loop exit caused by pressing escape
		// because we never exit the game loop immediately when the
		// floating menu is active. it will only be exited when the
		// user actually presses enter on the quit button.

		ExitGameLoop = 0;
		FloatingMenuKeyEscape();

	} else {

		FloatingMenuKeyStandard();
	}

	// reset sticky object rotation if necessary
	if ( !DepressedKeys->key_CursorLeft ) {
		rotkey_active_left  = FALSE;
	}
	if ( !DepressedKeys->key_CursorRight ) {
		rotkey_active_right = FALSE;
	}

	FloatingMenuMouseEnd();
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( M_MAIN )
{
	// precalculate a quarter of a sine wave ampl*[1.0,0.0] for all sliding
	hprec_t anorm = ( m_sintab_size - 1 ) * 2;
	for ( int tid = 0; tid < m_sintab_size; tid++ ) {
		hprec_t angle = HPREC_PI * tid;
		m_sintab[ tid ] = (int)(m_sintab_ampl * sin( angle / anorm ));
		m_sintab[ tid ] = m_sintab_ampl - m_sintab[ tid ];
	}
}



