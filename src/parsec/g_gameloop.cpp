/*
 * PARSEC - Gameflow Core
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:36 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   1999
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
#include "od_class.h"

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "inp_defs.h"
#include "net_defs.h"
#include "sys_defs.h"
#include "vid_defs.h"

// drawing subsystem
#include "d_font.h"

// rendering subsystem
#include "r_obj.h"
#include "r_part.h"
#include "r_sfx.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "g_gameloop.h"

// proprietary module headers
#include "con_aux.h"
#include "con_ext.h"
#include "con_kmap.h"
#include "con_main.h"
#include "e_callbk.h"
#include "e_demo.h"
#include "e_events.h"
#include "e_mouse.h"
#include "e_record.h"
#include "e_replay.h"
#include "e_supp.h"
#include "g_camera.h"
#include "g_stars.h"
#include "g_supp.h"
#include "h_drwhud.h"
#include "h_strmap.h"
#include "h_supp.h"
#include "h_text.h"
#include "inp_user.h"
#include "m_list.h"
#include "m_main.h"
#include "net_game.h"
#include "obj_ani.h"
#include "obj_coll.h"
#include "obj_ctrl.h"
#include "obj_expl.h"
#include "obj_xtra.h"
#include "g_wfx.h"



// strings displayed at game end ----------------------------------------------
//
static char youdidit_str[] 	  = "You did it!";
static char youhavelost_str[] = "You have lost!";


// game loop flags ------------------------------------------------------------
//
int glflag_firstframe;		// flag if initial frame is being rendered
int glflag_dazzle;			// flag if dazzling is in progress
int glflag_resetbackcol;	// flag if normal background color has to be restored


// name of game core script (executed on every entry of game loop) ------------
//
char gamecore_script[]		= "core" CON_FILE_EXTENSION;


// check integrity of object lists --------------------------------------------
//

#ifdef DEBUG
	#define CHECKLISTINTEGRITY() _CheckListIntegrity()
#else
	#define CHECKLISTINTEGRITY() {}
#endif

#ifdef DEBUG

void _CheckListIntegrity()
{
	GenObject *scan;

	scan = PShipObjects;
	while ( scan != NULL ) {
		if ( !( ObjCameraActive && ( scan == MyShip ) ) )
			CHECKHEAPBASEREF( scan );
		scan = scan->NextObj;
	}

	scan = LaserObjects;
	while ( scan != NULL ) {
		CHECKHEAPBASEREF( scan );
		scan = scan->NextObj;
	}

	scan = MisslObjects;
	while ( scan != NULL ) {
		CHECKHEAPBASEREF( scan );
		scan = scan->NextObj;
	}

	scan = ExtraObjects;
	while ( scan != NULL ) {
		CHECKHEAPBASEREF( scan );
		scan = scan->NextObj;
	}

	scan = CustmObjects;
	while ( scan != NULL ) {
		CHECKHEAPBASEREF( scan );
		scan = scan->NextObj;
	}

	scan = VObjList;
	while ( scan != NULL ) {
		CHECKHEAPBASEREF( scan );
		scan = scan->NextObj;
	}

	ASSERT( NumObjClasses == NumLoadedObjects );
	for ( int ocid = 0; ocid < NumLoadedObjects; ocid++ ) {
		CHECKHEAPBASEREF( ObjClasses[ ocid ] );
	}
}

#endif


// floating menu specific behavior --------------------------------------------
//
INLINE
void Gm_FloatingMenuSpecifics()
{
	ASSERT( FloatingMenu );
	ASSERT( InFloatingMenu );

	// keep extras turning
	OBJ_BackgroundAnimateExtras();

	// floating menu has its own keyhandler
	FloatingMenuKeyHandler();
}


// starmap specific behavior --------------------------------------------------
//
INLINE
void Gm_StarmapSpecifics()
{
	ASSERT( InStarMap );

	// starmap has its own keyhandler
	MAP_KeyHandler();
}


// entry mode specific behavior -----------------------------------------------
//
INLINE
void Gm_EntryModeSpecifics()
{
	ASSERT( EntryMode );
	ASSERT( NetConnected && !NetJoined );
	ASSERT( !FloatingMenu && !InFloatingMenu );

	// keep extras turning
	OBJ_BackgroundAnimateExtras();

	if ( SELECT_KEYS_PRESSED() ) {
		SELECT_KEYS_RESET();

		// enter network game
		InitJoinPosition();
		NETs_Join();

		EntryMode = FALSE;
		MSGOUT( "entering game mode." );
	}
}


// re-orthogonalize camera transformation -------------------------------------
//
INLINE
void Gm_ReOrthoCamera()
{
	if ( CameraMoved && !AUX_DISABLE_REORTHO_CAMERA_IN_GAME ) {
		CameraMoved = 0;

		if ( !ObjCameraActive ) {
			ReOrthoNormMtx( ShipViewCamera );
		} else {
			ReOrthoNormMtx( ObjectCamera );
		}
	}
}


// hardcoded lighting values according to background nebulae ------------------
//
static colrgba_s nebula_lighting_ambient[] = {

	{   0,   0,   0, 255 },
	{   0,   0,   0, 255 },

	{ byte(150*0.7), byte(150*0.7), byte(150*0.7), 255 },
	{ byte(150*0.7), byte(150*0.7), byte(150*0.7), 255 },
	{ byte(120*0.7), byte(150*0.7), byte(150*0.7), 255 },
	{ byte(130*0.7), byte(100*0.7), byte(160*0.7), 255 },
};

static colrgba_s nebula_lighting_diffuse[] = {

	{   0,   0,   0, 255 },
	{   0,   0,   0, 255 },

	{ 120, 180,  20, 255 },
	{ 220,  40,  20, 255 },
	{ 155, 205, 205, 255 },
	{ 210, 180, 255, 255 },
};

static colrgba_s nebula_lighting_specular[] = {

	{   0,   0,   0, 255 },
	{   0,   0,   0, 255 },

	{ 255, 255, 255, 255 },
	{ 255, 255, 255, 255 },
	{ 255, 255, 255, 255 },
	{ 255, 255, 255, 255 },
};


// draw panoramic background image and fixed stars ----------------------------
//
INLINE
void Gm_BackgroundNebulaLighting()
{
	if ( AUX_DISABLE_AUTOMATIC_NEBULA_LIGHTING ) {
		return;
	}

	int nid = AUXDATA_BACKGROUND_NEBULA_ID;
	if ( ( nid >= 0 ) && ( nid < 6 ) ) {

		LightColorAmbient  = nebula_lighting_ambient[ nid ];
		LightColorDiffuse  = nebula_lighting_diffuse[ nid ];
		LightColorSpecular = nebula_lighting_specular[ nid ];
	}
}


// draw panoramic background image and fixed stars ----------------------------
//
INLINE
void Gm_DrawFixedStars()
{
	// draw panorama first if enabled
	if ( AUX_ENABLE_PANORAMIC_BACKGROUND ) {
		R_DrawPanorama();
		Gm_BackgroundNebulaLighting();
	}

	// draw fixed stars if enabled
	if ( !AUX_DISABLE_FIXED_STAR_RENDERING ) {
		DrawFixedStars();
	}
}


// call DrawPseudoStars() if not disabled -------------------------------------
//
INLINE
void Gm_DrawPseudoStars()
{
	if ( !AUX_DISABLE_PSEUDO_STAR_RENDERING )
		DrawPseudoStars();
}


// render frame (3-D part) ----------------------------------------------------
//
INLINE
void Gm_RenderFrame()
{
	// maintain sound
	AUDs_MaintainSound();

	// reset polygon count
	NumRenderedPolygons = 0;

	// next visframe
	CurVisibleFrame++;
	if ( CurVisibleFrame == VISFRAME_NEVER ) {
		CurVisibleFrame = VISFRAME_START;
	}

	// viewcam for 3-D frame
	CAMERA_BeginFrameView();

	// transform frustum into world-space
	BackTransformVolume( ViewCamera, View_Volume, World_ViewVolume, 0x3f );
	CULL_MakeVolumeCullVolume( World_ViewVolume, World_CullVolume, 0x3f );

	// draw fixed stars/panorama
	Gm_DrawFixedStars();

	// draw pseudo stars
	Gm_DrawPseudoStars();

	// local ship always considered visible
	MyShip->VisibleFrame = CurVisibleFrame;

	// determine visible polygonal objects
	ScanActiveObjects( ViewCamera );

	CHECKMEMINTEGRITY();
	CHECKLISTINTEGRITY();

	// walk callbacks before drawing world objects
	CALLBACK_WalkCallbacks( CBTYPE_DRAW_OBJECTS );

	// draw visible polygonal objects
	R_DrawWorld( ViewCamera );

	// walk callbacks before drawing particles
	CALLBACK_WalkCallbacks( CBTYPE_DRAW_PARTICLES );

	// draw visible particles
	R_DrawParticles();

	// walk callbacks for custom iter objects
	CALLBACK_WalkCallbacks( CBTYPE_DRAW_CUSTOM_ITER );

	// walk callbacks before drawing effects
	CALLBACK_WalkCallbacks( CBTYPE_DRAW_EFFECTS );

	// draw lens flare
	R_DrawLensFlare();

	// standard viewcam
	CAMERA_EndFrameView();

	// maintain sound
	AUDs_MaintainSound();
}


// perform palette effects like flash white and blue --------------------------
//
INLINE
void Gm_PaletteEffects()
{
	// dazzling for lens-flare and lightning-hit
	if ( SetScreenWhite ) {

		if ( FlareIntensity > SetScreenWhite ) {

			// flare is brighter than dazzling, so
			// disable dazzling entirely
			SetScreenWhite = 0.0f;
			glflag_dazzle  = 0;

		} else {

			// dazzle screen with current intensity
			VIDs_SetScreenDazzle( SetScreenWhite );
			// set flag that palette will be restored
			// when dazzling has finished
			glflag_dazzle = 1;
			// decrease dazzling intensity for next
			// iteration (and time until normal)
			SetScreenWhite = max(SetScreenWhite - 16.0f * 60.0f * CurScreenRefFrames / RefFrameFrequency, 0.0f);
		}

	} else if ( glflag_dazzle ) {

		// reset restore flag
		glflag_dazzle = 0;

		// restore palette after dazzling has
		// finished
		AwaitVertSync();
	}

	// blue screen for enemy fire-hit
	if ( SetScreenBlue ) {

		// set background color to blue
		if ( !glflag_resetbackcol )
			AwaitVertSync();
		VIDs_SetColIndexZero( COL_BACKGROUND_BLUE );
		// decrease time screen will stay blue
		SetScreenBlue = max(SetScreenBlue - 1.0f * 60.0f * CurScreenRefFrames / RefFrameFrequency, 0.0f);
		// set flag that palette will be restored
		// when blue screen has finished
		glflag_resetbackcol = 1;

	} else if ( glflag_resetbackcol ) {

		// reset restore flag
		glflag_resetbackcol = 0;

		// restore background color to black if blue screen
		// has finished and other fading is not active
		if ( ( SetScreenWhite == 0 ) && ( FlareIntensity == 0 ) ) {
			AwaitVertSync();
			VIDs_SetColIndexZero( COL_BACKGROUND_BLACK );
		}

		// ensure that flare will behave properly
		FlareIntensity = 0;
	}

	// fade entire screen to a specified color with a specified speed.
	// (usually controlled by a demo being played back)
	if ( SetScreenFade != 0 ) {

		colrgba_s color = SetScreenFadeColor;

		if ( SetScreenFade > 0 ) {

			if ( SetScreenFade == 1 ) {

				// hold fading at opaque level
				color.A = 255;

			} else {

				// fade screen out (transparent->opaque)
				int fadeval = SetScreenFade >> 8;
				color.A = ( fadeval < 255 ) ? ( 255 - fadeval ) : 0;

				SetScreenFade -= SetScreenFadeSpeed * CurScreenRefFrames;
				if ( SetScreenFade < 1 ) {
					SetScreenFade = 1;
				}
			}

		} else {

			// fade screen in (opaque->transparent)
			int fadeval = -SetScreenFade >> 8;
			color.A = ( fadeval < 255 ) ? fadeval : 255;

			SetScreenFade += SetScreenFadeSpeed * CurScreenRefFrames;
			if ( SetScreenFade > 0 ) {
				SetScreenFade = 0;
			}
		}

		VIDs_SetScreenToColor( color );
	}
}


// fade screen from white to standard palette ---------------------------------
//
INLINE
void Gm_InitialFadeIn()
{
	if ( AUX_DISABLE_FLASHWHITE_ON_GAMEENTRY )
		return;

	if ( glflag_firstframe == 1 ) {
		// use dazzling to fade in from white
		SetScreenWhite    = 240;
		glflag_firstframe = 0;
	}
/*
	if ( glflag_firstframe == 1 ) {

		glflag_firstframe++;

	} else if ( glflag_firstframe == 2 ) {

		glflag_firstframe = 0;
		VIDs_FadeScreenFromWhite( PAL_GAME );

		if ( AUX_WAIT_FOR_KEYPRESS_AFTER_FADEIN )
			WaitForNewKeypress();
	}
*/
}


// check important keypresses -------------------------------------------------
//
INLINE
void Gm_SpecialKeyFunctions()
{
	// save screenshot if function invoked
	if ( DepressedKeys->key_SaveScreenShot ) {
		DepressedKeys->key_SaveScreenShot  = 0;

		if ( !glflag_firstframe ) {
			SaveScreenShot();
		}
	}

	// escape function invoked ?
	if ( DepressedKeys->key_Escape ) {
		DepressedKeys->key_Escape = 0;

		// <ESC> in object camera mode exits only object camera view
		if ( ObjCameraActive ) {

			// simulate keypress
			DepressedKeys->key_ToggleObjCamera = 1;

		} else {

			if ( !AUX_DISABLE_USER_GAMELOOP_EXIT ) {

				if ( DEMO_ReplayActive() ) {
					// stop demo replay if active
					DEMO_StopReplay();
				} else {
					// set flag to exit game-loop
					ExitGameLoop = 1;
				}
			}
		}
	}

	// console toggled ?
	if ( DepressedKeys->key_ToggleConsole ) {
		DepressedKeys->key_ToggleConsole = 0;

		ToggleConsole();
	}

	// help toggled ?
	if ( DepressedKeys->key_ToggleHelp ) {
		DepressedKeys->key_ToggleHelp = 0;

		//NOTE:
		// if console is visible, help toggling is
		// disabled (as well as the help window itself).
		// in entry-mode help toggling is also disabled.

		if ( GAME_MODE_ACTIVE() && ( ConsoleSliding == 0 ) ) {
			HelpActive = !HelpActive;
		}
	}

	// toggle frame rate display
	if ( DepressedKeys->key_ToggleFrameRate ) {
		DepressedKeys->key_ToggleFrameRate = 0;

		ShowFrameRate = !ShowFrameRate;
	}
}


// do game loop initializations -----------------------------------------------
//
INLINE
int Gm_InitGameLoop()
{
	// exec gameloop startup batch
	extern char gameloop_start_script[];
	ExecExternalFile( gameloop_start_script );

	// init game loop flags
	glflag_firstframe	= 1;
	glflag_dazzle		= 0;
	glflag_resetbackcol = 0;

	// exec game core script
	ExecConsoleFile( gamecore_script, FALSE );
	MSGOUT( "starting up game core." );

	// init game palette
	if ( !AUX_DISABLE_FLASHWHITE_ON_GAMEENTRY ) {

		if ( PlayDemo ) {
			// start demo off with blank screen and no automatic fade-in
			colrgba_s scol = COLRGBA_BLACK;
			VIDs_SetScreenToColor( scol );
			glflag_firstframe = 0;
		} else {
			// set screen white (to fade in from white later on)
			colrgba_s scol = COLRGBA_WHITE;
			VIDs_SetScreenToColor( scol );
		}
	}

	CAMERA_InitShipOrigin();
	InitRefFrameVars();

	return 1;
}


// enter entry mode from game mode --------------------------------------------
//
INLINE
void Gm_ReEnterEntryMode()
{
	// exit to entry mode and restart game loop
	EntryMode = TRUE;
	DepressedKeys->key_ShootWeapon = 0;

	// show floating menu
	InFloatingMenu = FloatingMenu;
	if ( InFloatingMenu ) {
		ActivateFloatingMenu( TRUE );
	}
}


// end of game loop actions ---------------------------------------------------
//
INLINE
int Gm_CancelGameLoop()
{
	ASSERT( ( ExitGameLoop != 0 ) || ShipDowned );

	// ensure local particle weapons are destroyed
	WFX_EnsureParticleWeaponsInactive( MyShip );

	// exit to entrymode if previously in game, quit otherwise
	if ( NetConnected ) {

		if ( ExitGameLoop == 3 ) {

			// no actual disconnect if exiting in simulated net
			if ( NetConnected == NETWORK_GAME_ON ) {

				if ( NetJoined ) {
					ObjCamOff();
					NETs_Unjoin( USER_EXIT );
				}

				NETs_Disconnect();
			}

		} else {

			if ( NetJoined ) {
				
				if ( ShipDowned ) {
					ObjCamOff();
					ASSERT( !NET_ConnectedGMSV() );
					NETs_Unjoin( SHIP_DOWNED );
				} else {
					ObjCamOff();
					NETs_Unjoin( USER_EXIT );
				}
				
				Gm_ReEnterEntryMode();
				return 0;
				
			} else {
				
				//NOTE:
				// we must not disconnect if a simulated quit
				// or shipdown has brought as here due to
				// sequencing problems.
				
				if ( ExitGameLoop == 1 ) {
					// no actual disconnect if exiting in simulated net
					if ( NetConnected == NETWORK_GAME_ON ) {
						NETs_Disconnect();
					}
				} else {
					ASSERT( ExitGameLoop == 2 );
					Gm_ReEnterEntryMode();
					return 0;
				}
			}
		}

	} else {

		// show floating menu
		if ( FloatingMenu && !InFloatingMenu && ( ExitGameLoop != 3 ) ) {
			DepressedKeys->key_ShootWeapon = 0;
			InFloatingMenu = FloatingMenu;
			ActivateFloatingMenu( TRUE );
			return 0;
		}
	}

	// disable console input
	KeybFlags->ConEnabled = 0;

	// fade screen
	VIDs_FadeScreenToBlack( PAL_GAME, FALSE );

	return 1;
}


// calculate ship's matrix as inverse of ship view camera matrix --------------
//
INLINE
void Gm_MapShipcamToShippos()
{
	CalcOrthoInverse( ShipViewCamera, MyShip->ObjPosition );

	if ( NET_ConnectedGMSV() ) {
		//LOGOUT(( "refframes: %d, curspeed: %.2f, shippos: %.2f/%.2f/%.2f", 
		/*LOGOUT(( "DUMPSTATE: refframes,curspeed,shippos %d, %.2f, %.2f,%.2f,%.2f", 
				CurScreenRefFrames,
				FIXED_TO_FLOAT( MyShip->CurSpeed ),
				MyShip->ObjPosition[ 0 ][ 3 ],
				MyShip->ObjPosition[ 1 ][ 3 ],
				MyShip->ObjPosition[ 2 ][ 3 ] ));
		*/

		LOGOUT(( "%.4f", MyShip->ObjPosition[ 2 ][ 3 ] ));
	}
}


// draw screen displays -------------------------------------------------------
//
INLINE
void Gm_DrawDisplays()
{
	// draw hud (may look different depending on certain conditions)
	HUD_DrawHUD();

	// display demo text if active
//	HUD_DrawDemoText();
}


// do stuff before actual frame rendering starts ------------------------------
//
INLINE
void Gm_BeginFrame()
{
	// fade in screen from white to standard palette
	Gm_InitialFadeIn();

	// clear render buffer (on hardware renderers this
	// call also simultaneously clears the z-buffer)
	VIDs_ClearRenderBuffer();

	// maintain sound
	AUDs_MaintainSound();
}


// check expiration of pulses triggered by events (pulse-events) --------------
//
INLINE
void Gm_EventExpiration()
{
	// missile lock detected
	if ( IncomingMissile > 0 ) {
		IncomingMissile -= CurScreenRefFrames;
		if ( IncomingMissile < 0 ) {
			IncomingMissile = 0;
		}
	}

	// current target hit
	if ( HitCurTarget > 0 ) {
		HitCurTarget--;
	}
}


// do stuff after actual frame has been rendered ------------------------------
//
INLINE
void Gm_FinishFrame()
{
	// maintain sound
	AUDs_MaintainSound();

	// wait for vertical retrace
	if ( AUX_AWAITRETRACE_ON_FRAMEFINISH )
		AwaitVertSync();

	// perform palette effects
	Gm_PaletteEffects();

	// maintain pulse events
	Gm_EventExpiration();

	// maintain sound
	AUDs_MaintainSound();

	// commit rendering buffer
	VIDs_CommitRenderBuffer();
}


// reset current motion tracking values ---------------------------------------
//
INLINE
void Gm_ResetMotionTracking()
{
	CurPitch	 = RecPitch		= BAMS_DEG0;
	CurYaw		 = RecYaw		= BAMS_DEG0;
	CurRoll		 = RecRoll		= BAMS_DEG0;
	CurSlideHorz = RecSlideHorz = GEOMV_0;
	CurSlideVert = RecSlideVert = GEOMV_0;
}


// game end flags -------------------------------------------------------------
//
int limit_reached = 0;

static refframe_t game_end_time;


// handle end of game ---------------------------------------------------------
//
PRIVATE
void Gm_HandleGameOver()
{
	// demo must not be interrupted by kill limit
	int klimitenabled = ( AUX_KILL_LIMIT_FOR_GAME_END > 0 );
	if ( DEMO_BinaryReplayActive() ) {
		klimitenabled = FALSE;
	}

	if ( AUX_ENABLE_GAME_STATUS_WINDOW && !GAME_NO_SERVER() ) {

		//NOTE:
		// when the server decides to end the current game
		// (either because the kill or the time limit has been reached)
		// the player is forced to unjoin and the floating menu will be
		// reactivated, which will only show the game status window
		// in this state.

		if ( NetConnected && ( NumRemPlayers > 0 ) && !InFloatingMenu && GAME_OVER() ) {
			ExitGameLoop = 1;
		}

	} else if ( klimitenabled ) {

		if ( NetConnected ) {

			if ( NetJoined ) {

				// reset flag
				limit_reached = 0;

				if ( NET_KillStatLimitReached( AUX_KILL_LIMIT_FOR_GAME_END ) ) {

					// exit to menu (forces unjoin)
					disable_join_game = 2;
					ExitGameLoop  = 1;
					limit_reached = 1;
					game_end_time = SYSs_GetRefFrameCount();

					int leaderkills = 0;
					int leaderid    = -1;

					for ( int pid = 0; pid < MAX_NET_PROTO_PLAYERS; pid++ ) {
						if ( Player_Status[ pid ] == PLAYER_INACTIVE )
							continue;
						if ( Player_KillStat[ pid ] > leaderkills ) {
							leaderkills = Player_KillStat[ pid ];
							leaderid = pid;
						}
					}

					char *msgstring;
					
					if ( leaderid == LocalPlayerId ) {

						AUD_YouDidIt();
						msgstring = youdidit_str;

					} else {

						AUD_YouHaveLost();
						msgstring = youhavelost_str;
					}
					
					texfont_s *texfont = FetchTexfont( "impact" );
					if ( texfont != NULL ) {
		
						IterTexfont itexfont;
						itexfont.itertype  = iter_texrgba | iter_alphablend;
						itexfont.raststate = rast_texclamp;
						itexfont.rastmask  = rast_nomask;
						itexfont.texfont   = texfont;
		
						itexfont.Vtxs[ 0 ].U = 2.0f;
						itexfont.Vtxs[ 0 ].V = 2.0f;
						itexfont.Vtxs[ 0 ].R = 255;
						itexfont.Vtxs[ 0 ].G = 255;
						itexfont.Vtxs[ 0 ].B = 255;
						itexfont.Vtxs[ 0 ].A = 180;
		
						int width = D_TexfontGetWidth( msgstring, &itexfont );
		
						itexfont.Vtxs[ 0 ].X = ( Screen_Width - width ) / 2;
						itexfont.Vtxs[ 0 ].Y = 140 + 20*8 + 40;
		
						texfontmsg_s msg;
						msg.message   = msgstring;
						msg.itexfont  = &itexfont;
						msg.msgtype   = 0;
		
						msg.starttime = 0;
						msg.lifetime  = 30000;
						msg.maxpulses = 16;
						msg.decalpha  = 0.8f;
		
						ShowTexfontMessage( &msg );
					}
					
					
					
				}

			} else if ( limit_reached ) {

				#define SHOW_KILLSTATS_TIME		6000
				#define RESET_STATS_SYNC_TIME	( 1000 + SHOW_KILLSTATS_TIME )

				switch ( disable_join_game ) {

					case 0:
						NET_FreezeKillStats( FALSE );
						break;

					case 1:
						{

						refframe_t elapsed = SYSs_GetRefFrameCount() - game_end_time;
						if ( elapsed > RESET_STATS_SYNC_TIME ) {
							disable_join_game = 0;
						} else if ( AUX_KILL_LIMIT_RESET_ON_GAME_END ) {
							NET_KillStatForceIdleZero();
						}

						}
						break;

					case 2:
						{

						refframe_t elapsed = SYSs_GetRefFrameCount() - game_end_time;
						if ( elapsed > SHOW_KILLSTATS_TIME ) {
							disable_join_game = 1;
						}
						NET_FreezeKillStats( TRUE );

						}
						break;
				}
			}

		} else {

			// make sure flag is reset
			limit_reached = 0;
		}

	} else {

		// make sure flag is reset
		limit_reached = 0;
	}
}


// main game loop -------------------------------------------------------------
//
void GameLoop()
{
	// do initializations
	if ( !Gm_InitGameLoop() )
		return;

	// set flag to indicate that in game loop
	InGameLoop = TRUE;

	// activate floating menu if in use
	InFloatingMenu = FloatingMenu;

	//NOTE:
	// if the floating menu is used the gameloop
	// will only be started once and run until
	// the entire application exits.
	// (that is, InGameLoop will always be TRUE.)

	// actual game loop starts here --------------------------------------
	do {

		InitLocalShipStatus( LocalShipClass );
		InitHudDisplay(); //FIXME: second time; (because of MyShip)

		// loop while neither exit desired nor ship downed --------------
		ExitGameLoop = 0;
		while ( !( ExitGameLoop || ShipDowned ) ) {

			// must be called once per frame
			SYSs_Yield();

			// in debug version periodically check mem integrity
			CHECKMEMINTEGRITY();
			CHECKLISTINTEGRITY();

			// maintain sound
			AUDs_MaintainSound();

			// check selected keypresses
			Gm_SpecialKeyFunctions();

			// start rendering of next buffer
			Gm_BeginFrame();

			// do frame measurement
			DoFrameTimeCalculations();

			// maintain explosion counts for all ships;
			// kill objects if necessary
			OBJ_CheckExplosions();

			// advance lasers and missiles
			OBJ_AnimateProjectiles();

			// call animation callback for custom objects
			OBJ_AnimateCustomObjects();

			// for smooth ship control
			CAMERA_BeginModify();

			// init recording matrices
			REC_InitMatrices();

			// reset current motion values
			Gm_ResetMotionTracking();

			// perform automatic actions
			// (demo/console script replay)
			REPLAY_PerformAutomaticActions();

			// walk callbacks before getting user input
			CALLBACK_WalkCallbacks( CBTYPE_USER_INPUT );

			// discriminate starmap, floating menu, entry mode, and gameplay
			if ( InStarMap ) {

				Gm_StarmapSpecifics();
				INP_UserResetActivityChecking();

			} else if ( InFloatingMenu ) {

				Gm_FloatingMenuSpecifics();
				INP_UserResetActivityChecking();

			} else if ( EntryMode ) {

				Gm_EntryModeSpecifics();
				INP_UserResetActivityChecking();

			} else {

				// check keyboard and joystick
				INP_UserProcessInput();

				// do dof ship movement if not moved automatically
				if ( AutomaticMovement == 0 ) {
					MoveLocalShip();
				}

				// object collision detection
				OBJ_CheckCollisions();

				// perform ship actions like bouncing
				OBJ_PerformShipActions();

				// do ship intelligence
				ShipIntelligence();
			}

			// for smooth ship control
			CAMERA_EndModify();

			// place extras
			OBJ_DoExtraPlacement();

			// maintain the event manager
			EVT_Maintain();

			// reorthogonalize camera matrix if necessary
			Gm_ReOrthoCamera();

			// map ship view camera to ship position
			Gm_MapShipcamToShippos();

			// check whether user activated/deactivated objcam
			ObjectCameraControl();

			// get actual viewing (rendering) camera
			CAMERA_GetViewCamera();

			if ( GAME_MODE_ACTIVE() ) {

				// check if gun or missile fired;
				// create corresponding objects
				if ( !InStarMap ) {
					INP_UserCheckFiring();
				}

				// maintain duration weapon firing
				// for local ship
				if ( ( LocalPlayerId != PLAYERID_ANONYMOUS ) && !ShipDowned )
					MaintainDurationWeapons( LocalPlayerId );
			}

			// record actions if recording is on
			REC_RecordActions();

			// do network stuff
			NETs_MaintainNet();

			if ( InStarMap ) {

				///int flag = AUX_DISABLE_BUFFER_CLEAR;
				//AUX_DISABLE_BUFFER_CLEAR = FALSE;
				//VIDs_ClearRenderBuffer();
				//AUX_DISABLE_BUFFER_CLEAR = flag;

				// next visframe
				/*CurVisibleFrame++;
				if ( CurVisibleFrame == VISFRAME_NEVER ) {
					CurVisibleFrame = VISFRAME_START;
				}*/

				//NOTE:
				// CurVisibleFrame is usually increased in Gm_RenderFrame(), but it
				// must also be updated here. mouse position updates on Linux and Win32
				// depend on this, for example.

				// render 3-D part of frame
				Gm_RenderFrame();

				// draw screen displays
				Gm_DrawDisplays();

				// maintain on-screen message area
				MaintainMessages();

				// render the starmap
				MAP_ShowStarmap( NULL );

			} else {

				// render 3-D part of frame
				Gm_RenderFrame();

				// draw screen displays
				Gm_DrawDisplays();

				// maintain on-screen message area
				MaintainMessages();

				// switch to game status window on end of game
				Gm_HandleGameOver();

				// draw floating menu
				if ( InFloatingMenu ) {

					DrawFloatingMenu();

				} else if ( EntryMode ) {

					// draw entry mode text
					NET_DrawEntryModeText();
				}

				// display current game time
				WriteGameTime();

				// display current server ping
				WriteServerPing();

				// draw on-line help
				if ( GAME_MODE_ACTIVE() && HelpActive ) {
					DrawOnlineHelp();
				}

				// draw translucent player list
				if ( GAME_MODE_ACTIVE() && AUX_HUD_ADVANCED_KILLSTATS ) {
					DrawRemotePlayerListWindow();
				}

				// display current frame rate
				if ( ShowFrameRate ) {
					WriteFrameRate();
				}

				// walk callbacks after drawing everything
				// except the console
				CALLBACK_WalkCallbacks( CBTYPE_DRAW_OVERLAYS );
			}

			// draw mouse cursor
			DrawMouseCursor();

			// increment frame counter
			++FrameCounter;

			// exec console commands mapped to additional keys
			ExecBoundKeyCommands();

			// check console input and draw its window
			ConsoleControl();

			// finish frame; mainly blitting to screen
			Gm_FinishFrame();

		} // end gameloop

	} while ( !Gm_CancelGameLoop() );

	// clear flag to indicate that not in game loop
	InGameLoop = FALSE;
}



