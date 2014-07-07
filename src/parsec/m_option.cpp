/*
 * PARSEC - Options Window
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

#include "con_com.h"

// subsystem headers
#include "aud_defs.h"
#include "net_defs.h"
#include "sys_defs.h"
#include "vid_defs.h"

// drawing subsystem
#include "d_font.h"

// rendering subsystem
#include "r_patch.h"
#include "r_supp.h"

// local module header
#include "m_option.h"

// proprietary module headers
#include "con_aux.h"
#include "e_color.h"
#include "e_demo.h"
#include "e_draw.h"
#include "e_supp.h"
#include "m_main.h"
#include "net_conn.h"
#include "sys_bind.h"
#include "vid_plug.h"

#include "keycodes.h"

// flags
#define DISABLE_VID_SUBSYS_SWITCHING



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 2047
static char paste_str[ PASTE_STR_LEN + 1 ];

static char tmp_name[MAX_PLAYER_NAME + 1];

bool mod_player_name;

// string constants -----------------------------------------------------------
//
static char not_available[]			= "not available  ";

bool highlight_locked = false;


// options menu configuration and state variables -----------------------------
//
#define OPT_POS_RIGHT				0
#define OPT_POS_LEFT				( m_sintab_size - 1 )
#define OPT_SLIDE_SPEED				12

#define OPT_MAX_BLINK_COUNT			900
#define OPT_MAX_TEXTLEN 			255

static const char *option_lines[ NUM_OPTIONS ] = {

	"  resolution: ",
	"  color depth: ",
	"  fullscreen: ",
	"  vertical sync: ",
	"  apply video settings  ",
	"  texture detail: ",
	"  geometry detail: ",
	"  sound-fx detail: ",
	"  apply detail settings  ",
	"  protocol: ",
	"  kill limit: ",
	"  solar system: ",
	"  apply network settings  ",
	"  audio settings: ",
	"  controller: ",
	"  invert mouse y axis: ",
	"  mouse sensitivity: ",
	"  mouse centering speed: ",
	//" ",
	"  name: ",
	"  exit options menu  ",
};

enum {

	OPT_SPACING_NONE   	= 0,
	OPT_SPACING_NEWLINE	= 1,
	OPT_SPACING_DIVIDER	= 2
};

static int option_spacing[ NUM_OPTIONS ] = {

	OPT_SPACING_NONE,		// RES_OPT,
	OPT_SPACING_NONE,		// DEPTH_OPT,
	OPT_SPACING_NONE,		// FULL_OPT,
	OPT_SPACING_NEWLINE,	// SYNC_OPT,
	OPT_SPACING_DIVIDER,	// APPLY_VID_OPT,
	OPT_SPACING_NONE,		// TEXDETAIL_OPT,
	OPT_SPACING_NONE,		// GEOMDETAIL_OPT,
	OPT_SPACING_NEWLINE,	// SFXDETAIL_OPT,
	OPT_SPACING_DIVIDER,	// APPLY_DETAIL_OPT
	OPT_SPACING_NONE,		// PROTOCOL_OPT,
	OPT_SPACING_NONE,		// KILL_LIMIT_OPT,
	OPT_SPACING_NEWLINE,	// SOLAR_OPT,
	OPT_SPACING_DIVIDER,	// APPLY_NET_OPT,
	OPT_SPACING_NONE,		// SOUND_OPT,
	OPT_SPACING_NONE,		// CTRL_OPT,
	OPT_SPACING_NONE,		// INVERT_OPT,
	OPT_SPACING_NONE,		// SENSITIVITY_OPT,
	OPT_SPACING_DIVIDER,	// CENTER_OPT,
	//OPT_SPACING_DIVIDER,    // APPLY_MOUSE_OPT (Actually a blank line),
	OPT_SPACING_NEWLINE,	// NAME_OPT,
	OPT_SPACING_DIVIDER,	// EXIT_OPT,
};

static int option_apply_blinking[ NUM_OPTIONS ] = {

	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,	// APPLY_VID_OPT,
	FALSE,
	FALSE,
	FALSE,
	FALSE,	// APPLY_DETAIL_OPT
	FALSE,
	FALSE,
	FALSE,
	FALSE,	// APPLY_NET_OPT,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
};

static int enable_dynamic_options = TRUE;

static int option_dynamic[ NUM_OPTIONS ] = {

	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,	// APPLY_VID_OPT,
	FALSE,
	FALSE,
	FALSE,
	FALSE,	// APPLY_DETAIL_OPT
	FALSE,
	FALSE,
	FALSE,
	FALSE,	// APPLY_NET_OPT,
	FALSE,
	FALSE,
	TRUE,
	TRUE,
	TRUE,
	FALSE,
};


static const char *binopt_strings[] = {

	"off  ",
	"on  ",
};

static const char *detailopt_strings[] = {

	"high  ",
	"low  ",
};

static const char *qualityopt_strings[] = {

	"high  ",
	"medium  ",
	"low  ",
};

static const char *soundopt_strings[] = {

	"off  ",
	"sfx  ",
	"music  ",
	"music+sfx  ",
};

static const char *ctrlopt_strings[] = {

	"off  ",
	"joystick  ",
	"mouse  ",
	"joy+mouse  ",
};


static int			opt_slidepos 			= OPT_POS_RIGHT;
static int			opt_slidetarget			= OPT_POS_RIGHT;
static refframe_t	opt_lastref 			= REFFRAME_INVALID;

static int			opt_blink_count			= 0;
static int  		opt_blink_ctr			= 0;
static int 			opt_show_warning_text 	= FALSE;
static char 		opt_warning_str[] 		= "disconnect first";

static char 		opt_caption_str[] 		= "OPTIONS MENU";

static char			opt_text_buffer[ OPT_MAX_TEXTLEN + 1 ];

static resinfo_s	cur_res				= resinfo_s();
static int  		cur_depthmode 		= 0;
static int  		cur_winstatus 		= 0;
static int  		cur_flipsync		= 0;
static int  		cur_mipmapbias		= 0;
static int  		cur_geombias		= 0;

PUBLIC int  		cur_sfxquality		= 0;
PUBLIC int  		prev_sfxquality		= 0;

static int			cur_killlimit		= 0;
static int			cur_solarsys		= 0;

static int 			cur_sensitivity		= 0;
static int 			cur_centerspeed		= 0;

static int			vidsubsys_id		= 0;
static int			prev_vidsubsys_id	= 0;

static int			protsubsys_id		= 0;
static int			prev_protsubsys_id	= 0;


struct user_names_s {

	const char *internal_name;
	const char *visible_name;
};

static user_names_s user_names[] = {

	{ "glx", 	"opengl"		},
	{ "udp", 	"tcp/ip"		},

	{ NULL,		NULL			},
};

// number of option menu entries and currently selected entry -----------------
//
static int cur_opmenu_size		= NUM_OPTIONS;
static int cur_opmenu_selindx	= 0;


// return current selection in options menu -----------------------------------
//
int OptionsListSelection()
{
	return cur_opmenu_selindx;
}


// select default item in options menu ----------------------------------------
//
void OptionsListSelectDefault()
{
	cur_opmenu_selindx = 0;
}


// one item up in options menu ------------------------------------------------
//
void OptionsListCursorUp()
{
	if(!highlight_locked) {
		if ( --cur_opmenu_selindx < 0 )
			cur_opmenu_selindx = cur_opmenu_size - 1;

		AUD_Select2();
	}
}


// one item down in options menu ----------------------------------------------
//
void OptionsListCursorDown()
{
	if(!highlight_locked) {
		if ( ++cur_opmenu_selindx >= cur_opmenu_size )
			cur_opmenu_selindx = 0;

		AUD_Select2();
	}
}



// current metrics of options menu contents -----------------------------------
//
static list_window_metrics_s options_content_metrics = { FALSE };


// detect whether mouse position is over options menu item --------------------
//
int MouseOverOption( int mousex, int mousey )
{
	if(!highlight_locked){
		if ( !options_content_metrics.valid )
			return MOUSE_OVER_NOTHING;

		if ( !SlideFinishedOptions() )
			return MOUSE_OVER_NOTHING;

		int chwidth   = options_content_metrics.chwidth;
		int chheight  = options_content_metrics.chheight;
		int	text_x    = options_content_metrics.text_x;
		int text_y    = options_content_metrics.text_y;
		int linewidth = options_content_metrics.maxcontwidth * chwidth;
			
		// check against options menu items
		for ( int mid = 0; mid < cur_opmenu_size; mid++ ) {

			if ( ( mousex >= text_x ) && ( mousex < ( text_x + linewidth ) ) &&
				 ( mousey >= text_y ) && ( mousey < ( text_y + chheight ) ) ) {
						
				// set selected index directly
				if ( cur_opmenu_selindx != mid )
					AUD_Select2();
				cur_opmenu_selindx = mid;

				return MOUSE_OVER_OPTION;
			}

			text_y += chheight;
			if ( option_spacing[ mid ] != OPT_SPACING_NONE ) {
				text_y += chheight;
			}
		}

		return MOUSE_OVER_NOTHING;
	} else {
		return MOUSE_OVER_OPTION;
	}
}


// set position and slide target for options menu to left (in) ----------------
//
void MoveInOptions()
{
	opt_slidepos	= OPT_POS_LEFT;
	opt_slidetarget	= opt_slidepos;
}


// set position and slide target for options menu to right (out) --------------
//
void MoveOutOptions()
{
	opt_slidepos	= OPT_POS_RIGHT;
	opt_slidetarget	= opt_slidepos;
}


// set slide target for options menu to left (in) -----------------------------
//
void SlideInOptions()
{
	opt_slidetarget = OPT_POS_LEFT;
}


// set slide target for options menu to right (out) ---------------------------
//
void SlideOutOptions()
{
	opt_slidetarget = OPT_POS_RIGHT;
}


// check if options menu is at its desired slide position ---------------------
//
int SlideFinishedOptions()
{
	return ( opt_slidepos == opt_slidetarget );
}


// init info on current subsystems for options menu ---------------------------
//
void InitOptionsSubsystemInfo()
{
	// get current mode values
	cur_res			= GameScreenRes;
	cur_depthmode	= GameScreenBPP;
	cur_winstatus	= GameScreenWindowed;
	

	vidsubsys_id		= 0;
	prev_vidsubsys_id	= 0;


	cur_mipmapbias = AUXDATA_TMM_MIPMAP_LOD_BIAS;
	cur_geombias   = AUXDATA_LOD_DISCRETE_GEOMETRY_BIAS;

	// search for currently selected protocol in table
	int id = 0;
	for ( id = 0; protocol_table[ id ].name; id++ )
		if ( protocol_table[ id ].id == sys_BindType_PROTOCOL )
			break;

	protsubsys_id			= id;
	prev_protsubsys_id		= id;

	//NOTE:
	// ( protocol_table[ id ].name == NULL ) must be
	// handled correctly by users of protsubsys_id.

	cur_killlimit = AUX_KILL_LIMIT_FOR_GAME_END;
	cur_solarsys = AUXDATA_BACKGROUND_NEBULA_ID;
	
	enable_dynamic_options = Op_Mouse;
}


// init vidmode state variables for options menu ------------------------------
//
void InitOptionsWindow()
{
	static int mode_init_done = FALSE;
	if ( mode_init_done )
		return;

	// init mode state variables
	cur_res			= GameScreenRes;
	cur_depthmode	= GameScreenBPP;
	cur_winstatus	= GameScreenWindowed;
	cur_flipsync	= FlipSynched;

	int id = 0;
	
	vidsubsys_id = 0;

	// search for currently selected protocol in table
	for ( id = 0; protocol_table[ id ].name; id++ )
		if ( protocol_table[ id ].id == sys_BindType_PROTOCOL )
			break;

	protsubsys_id = prev_protsubsys_id = id;


	cur_sensitivity = inp_mouse_sensitivity;
	cur_centerspeed = inp_mouse_drift;
	
	enable_dynamic_options = Op_Mouse;

	mode_init_done = TRUE;
	highlight_locked = false;
	mod_player_name = false;
}


// exec options menu item selection: screen resolution ------------------------
//
PRIVATE
void ExecOptionSelectResolution()
{
	if (!cur_res.isValid())
		return;
	
	int resindex = GetResolutionIndex(cur_res.width, cur_res.height);
	resindex = (resindex + 1) % Resolutions.size();
	
	if (VID_MODE_AVAILABLE(resindex)) {
		cur_res = Resolutions[resindex];
	}
}


// exec options menu item selection: color depth ------------------------------
//
PRIVATE
void ExecOptionSelectColordepth()
{
	if ( cur_depthmode == -1 )
		return;

	int prev_depthmode = cur_depthmode;

	if ( cur_depthmode == MODE_COL_16BPP )
		cur_depthmode = MODE_COL_32BPP;
	else if ( cur_depthmode == MODE_COL_32BPP )
		cur_depthmode = MODE_COL_16BPP;
	
	if (cur_depthmode > MaxScreenBPP)
		cur_depthmode = prev_depthmode;
}


// exec options menu item selection: fullscreen/windowed mode -----------------
//
PRIVATE
void ExecOptionSelectFullscreen()
{
	if ( cur_winstatus == -1 )
		return;

	// toggle winstatus flag
	cur_winstatus = !cur_winstatus;
}


// exec options menu item selection: sync to screen ---------------------------
//
PRIVATE
void ExecOptionSelectFlipsync()
{
	// toggle flipsync flag
	if ( cur_flipsync == -1 )
		return;

	cur_flipsync = !cur_flipsync;
}


// exec options menu item selection: texture detail ---------------------------
//
PRIVATE
void ExecOptionSelectTextureDetail()
{
	if ( ++cur_mipmapbias > 1 ) {
		cur_mipmapbias = 0;
	}
}


// exec options menu item selection: geometry detail --------------------------
//
PRIVATE
void ExecOptionSelectGeometryDetail()
{
	if ( ++cur_geombias > 1 ) {
		cur_geombias = 0;
	}
}


// exec options menu item selection: sound detail -----------------------------
//
PRIVATE
void ExecOptionSelectSoundDetail()
{
	if ( ++cur_sfxquality > 2 ) {
		cur_sfxquality = 0;
	}
}


// exec options menu item selection: select game protocol ---------------------
//
PRIVATE
void ExecOptionSelectProtocol()
{
	// ensure that there is atleast one protocol linked
	if ( protocol_table[ 0 ].id == -1 )
		return;

	// take next available protocol
	if ( protocol_table[ protsubsys_id ].id != -1 )
		protsubsys_id++;
	if ( protocol_table[ protsubsys_id ].id == -1 )
		protsubsys_id = 0;
}


// exec options menu item selection: kill limit -------------------------------
//
PRIVATE
void ExecOptionSelectKillLimit()
{
	cur_killlimit += 10;
	if ( cur_killlimit > 99 ) {
		cur_killlimit = 0;
	}
}


// exec options menu item selection: solar system -----------------------------
//
PRIVATE
void ExecOptionSelectSolarSystem()
{
	// currently 2 to 5 are valid
	if ( ++cur_solarsys > 5 ) {
		cur_solarsys = 2;
	}
}


// exec options menu item selection: enable/disable sound effects -------------
//
PRIVATE
void ExecOptionSelectSoundEnabling()
{
	if ( SoundDisabled )
		return;

	// determine current setting
	int cursetting = 0x00;
	if ( Op_SoundEffects )
		cursetting |= 0x01;
	if ( Op_Music )
		cursetting |= 0x02;

	// go to next setting
	cursetting = ( cursetting + 1 ) & 0x03;

	// determine new setting
	Op_SoundEffects	= ( ( cursetting & 0x01 ) != 0x00 );
	Op_Music		= ( ( cursetting & 0x02 ) != 0x00 );

	// start/stop according to setting
	switch ( cursetting ) {

		//NOTE:
		// the sequence of AUDs_CloseMenuSound() and AUDs_StopAudioStream()
		// must not be changed, or the stream active flag will wrongly stick.

		case 0x00:
			//AUDs_CDStop();
			AUDs_CloseMenuSound();
			AUDs_StopAudioStream();
			break;

		case 0x01:
			//AUDs_CDStop();
			AUDs_CloseMenuSound();
			AUDs_StopAudioStream();
			break;

		case 0x02:
			AUDs_OpenMenuSound();
			break;

		case 0x03:
			AUDs_OpenMenuSound();
			break;

		default:
			ASSERT( 0 );
	}
}


// exec options menu item selection: select controller ------------------------
//
PRIVATE
void ExecOptionSelectControllerEnabling()
{
	// determine current setting
	int cursetting = 0x00;
	if ( Op_Joystick )
		cursetting |= 0x01;
	if ( Op_Mouse )
		cursetting |= 0x02;

	
	bool foundsetting = false;
	
	while (!foundsetting) {
		// go to next setting
		cursetting = ( cursetting + 1 ) & 0x03;
		
		foundsetting = true;

		// prevent setting both mouse and joystick
		if ( cursetting == 3 ) {
			foundsetting = false;
		}

		// ensure joystick is available
		if ( ( cursetting & 0x01 ) && JoystickDisabled ) {
			foundsetting = false;
		}
	};

	// determine new setting
	Op_Joystick	= ( ( cursetting & 0x01 ) != 0x00 );
	Op_Mouse	= ( ( cursetting & 0x02 ) != 0x00 );
	
	enable_dynamic_options = Op_Mouse;
}


// exec options menu item selection: select controller ------------------------
//
PRIVATE
void ExecOptionSelectInvertYAxis()
{
	if ( !enable_dynamic_options )
		return;
	
	inp_mouse_invert_yaxis ^= 1;
}


// exec options menu item selection: select mouse sensitivity -----------------
//
PRIVATE
void ExecOptionSelectSensitivity()
{
	if ( !enable_dynamic_options )
		return;

	cur_sensitivity += 100;
	if ( cur_sensitivity > 2000 ) {
		cur_sensitivity = 100;
	}
	
	inp_mouse_sensitivity = cur_sensitivity;
}


// exec options menu item selection: select mouse sensitivity -----------------
//
PRIVATE
void ExecOptionSelectCenterSpeed()
{
	if ( !enable_dynamic_options )
		return;

	cur_centerspeed += 10;
	if ( cur_centerspeed > 100 ) {
		cur_centerspeed = 10;
	}
	
	inp_mouse_drift = cur_centerspeed;
}

// exec options menu item selection: select name-----------------
//
PRIVATE
void ExecOptionSelectName()
{

	// toggle the flag to modify the LocalPlayerName
	mod_player_name = !mod_player_name;
	if(mod_player_name) {
		// if we are modifying the player, copy the local player
		// name to paste_str for editing.
		strncpy(tmp_name, LocalPlayerName, MAX_PLAYER_NAME + 1);
		highlight_locked = true;
	} else {
		CheckPlayerName(tmp_name);
		highlight_locked = false;
		//strncpy(LocalPlayerName, tmp_name, MAX_PLAYER_NAME + 1);
	}
		
	
}

// exec options menu item selection: exit options menu ------------------------
//
PRIVATE
void ExecOptionSelectExit()
{
	ExitOptionsMenu();
}


// exec options menu item selection: apply video settings ---------------------
//
PRIVATE
void ExecOptionApplyVideo()
{
	option_apply_blinking[ APPLY_VID_OPT ] = FALSE;
	
	FlipSynched = cur_flipsync;

	// apply video mode change
	int resindex = GetResolutionIndex(cur_res.width, cur_res.height);
	if ( VID_MODE_AVAILABLE( resindex ) ) {
		VID_HotChangeMode(cur_res.width, cur_res.height, !cur_winstatus, cur_depthmode);	}

	// get the default mode that is active after switching
	cur_res			= GameScreenRes;
	cur_depthmode	= GameScreenBPP;
	cur_winstatus	= GameScreenWindowed;
}


// exec options menu item selection: apply detail settings --------------------
//
PRIVATE
void ExecOptionApplyDetail()
{
	option_apply_blinking[ APPLY_DETAIL_OPT ] = FALSE;

	if ( AUXDATA_TMM_MIPMAP_LOD_BIAS != cur_mipmapbias ) {

		// set new mip map bias
		AUXDATA_TMM_MIPMAP_LOD_BIAS = cur_mipmapbias;

		// flush entire texture cache
		R_InvalidateCachedTexture( NULL );

		// make sure texture data that might have been freed
		// by the texture cache is available once again
		ReloadFreedTextureBitmaps();

		// precache textures
		R_PrecacheTextures();
	}

	AUXDATA_LOD_DISCRETE_GEOMETRY_BIAS = cur_geombias;

	// trigger reloading samples if quality has been changed
	if ( cur_sfxquality != prev_sfxquality ) {

		if ( DEMO_ReplayActive() ) {
			DEMO_StopReplay();
		}

		extern int itemvis_loading;
		itemvis_loading = TRUE;

		prev_sfxquality = cur_sfxquality;
	}
}


// defined in CON_AUX.C -------------------------------------------------------
//
extern void RESyncKillLimit();
extern void RESyncNebulaId();


// exec options menu item selection: apply network settings -------------------
//
PRIVATE
void ExecOptionApplyNetwork()
{
	option_apply_blinking[ APPLY_NET_OPT ] = FALSE;

	// apply kill limit change
	if ( AUX_KILL_LIMIT_FOR_GAME_END != cur_killlimit ) {
		AUX_KILL_LIMIT_FOR_GAME_END = cur_killlimit;
		RESyncKillLimit();
	}

	// apply nebula change
	if ( AUXDATA_BACKGROUND_NEBULA_ID != cur_solarsys ) {
		AUXDATA_BACKGROUND_NEBULA_ID = cur_solarsys;
		RESyncNebulaId();
	}

	// plug in new network subsystem only if different
	if (  protsubsys_id  == prev_protsubsys_id  ) {
		return;
	}

	// only if not connected
	if ( NetConnected ) {

		opt_show_warning_text = TRUE;
		opt_blink_ctr = 0;

		return;
	}

	// deinit old subsystem
	NETs_KillAPI();

	// rebind protocol
	sys_BindType_PROTOCOL = protocol_table[ protsubsys_id ].id;
	SYS_Bind_PROTOCOL();

	// init new subsystem
	NETs_InitAPI();

	prev_protsubsys_id   = protsubsys_id;
}


// user has selected option <optno> -------------------------------------------
//
void ExecOptionSelection( int optno )
{
	// reset the mod_player_name flag 
	mod_player_name = 0;

	// invoke corresponding function
	switch ( optno ) {

		case RES_OPT:
			ExecOptionSelectResolution();
			break;

		case DEPTH_OPT:
			ExecOptionSelectColordepth();
			break;

		case FULL_OPT:
			ExecOptionSelectFullscreen();
			break;

		case SYNC_OPT:
			ExecOptionSelectFlipsync();
			break;

		case APPLY_VID_OPT:
			ExecOptionApplyVideo();
			break;

		case TEXDETAIL_OPT:
			ExecOptionSelectTextureDetail();
			break;

		case GEOMDETAIL_OPT:
			ExecOptionSelectGeometryDetail();
			break;

		case SFXDETAIL_OPT:
			ExecOptionSelectSoundDetail();
			break;

		case APPLY_DETAIL_OPT:
			ExecOptionApplyDetail();
			break;

		case PROTOCOL_OPT:
			ExecOptionSelectProtocol();
			break;

		case KILL_LIMIT_OPT:
			ExecOptionSelectKillLimit();
			break;

		case SOLAR_OPT:
			ExecOptionSelectSolarSystem();
			break;

		case APPLY_NET_OPT:
			ExecOptionApplyNetwork();
			break;

		case SOUND_OPT:
			ExecOptionSelectSoundEnabling();
			break;

		case CTRL_OPT:
			ExecOptionSelectControllerEnabling();
			break;

		case INVERT_OPT:
			ExecOptionSelectInvertYAxis();
			break;

		case SENSITIVITY_OPT:
			ExecOptionSelectSensitivity();
			break;

		case CENTER_OPT:
			ExecOptionSelectCenterSpeed();
			break;

		case EXIT_OPT:
			ExecOptionSelectExit();
			break;
		case NAME_OPT:
			ExecOptionSelectName();
			break;

		default:
			ASSERT( 0 );
	}
}


// create options menu item text: screen resolution ---------------------------
//
PRIVATE
void CreateOptionTextResolution( int id )
{
	ASSERT( strlen( option_lines[ id ] ) <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );

	if ( !cur_res.isValid() )
		sprintf( paste_str, "default" );
	else
		sprintf( paste_str, "%dx%d", cur_res.width, cur_res.height );

	ASSERT( strlen( paste_str ) + 2 + strlen( opt_text_buffer ) <= OPT_MAX_TEXTLEN );
	strcat( opt_text_buffer, paste_str );
	strcat( opt_text_buffer, "  " );
}


// create options menu item text: color depth ---------------------------------
//
PRIVATE
void CreateOptionTextColordepth( int id )
{
	ASSERT( strlen( option_lines[ id ] ) <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );

	if ( cur_depthmode == -1 ) {
		sprintf( paste_str, "default" );
	} else {

		int colbits = cur_depthmode;
		sprintf( paste_str, "%d bit", colbits );
	}

	ASSERT( strlen( paste_str ) + 2 + strlen( opt_text_buffer ) <= OPT_MAX_TEXTLEN );
	strcat( opt_text_buffer, paste_str );
	strcat( opt_text_buffer, "  " );
}


// create options menu item text: fullscreen/windowed mode --------------------
//
PRIVATE
void CreateOptionTextFullscreen( int id )
{
	ASSERT( strlen( option_lines[ id ] ) <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );

	if ( cur_winstatus == -1 ) {
		ASSERT( 9 + strlen( opt_text_buffer ) <= OPT_MAX_TEXTLEN );
		strcat( opt_text_buffer, "default  " );
	} else {
		ASSERT( strlen( binopt_strings[ cur_winstatus == 0 ] ) + strlen( opt_text_buffer ) <= OPT_MAX_TEXTLEN );
		strcat( opt_text_buffer, binopt_strings[ cur_winstatus == 0 ] );
	}
}


// create options menu item text: sync to screen ------------------------------
//
PRIVATE
void CreateOptionTextFlipsync( int id )
{
	ASSERT( strlen( option_lines[ id ] ) + strlen( binopt_strings[ cur_flipsync ] ) <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );
	strcat( opt_text_buffer, binopt_strings[ cur_flipsync ] );
}


// create options menu item text: texture detail ------------------------------
//
PRIVATE
void CreateOptionTextTextureDetail( int id )
{
	ASSERT( strlen( option_lines[ id ] ) + strlen( detailopt_strings[ cur_mipmapbias ] ) <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );
	strcat( opt_text_buffer, detailopt_strings[ cur_mipmapbias ] );
}


// create options menu item text: geometry detail -----------------------------
//
PRIVATE
void CreateOptionTextGeometryDetail( int id )
{
	ASSERT( strlen( option_lines[ id ] ) + strlen( detailopt_strings[ cur_geombias ] ) <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );
	strcat( opt_text_buffer, detailopt_strings[ cur_geombias ] );
}


// create options menu item text: sound detail --------------------------------
//
PRIVATE
void CreateOptionTextSoundDetail( int id )
{
	ASSERT( strlen( option_lines[ id ] ) + strlen( qualityopt_strings[ cur_sfxquality ] ) <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );
	strcat( opt_text_buffer, qualityopt_strings[ cur_sfxquality ] );
}


// create options menu item text: game protocol -------------------------------
//
PRIVATE
void CreateOptionTextProtocol( int id )
{
	ASSERT( strlen( option_lines[ id ] ) <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );

	if ( protocol_table[ protsubsys_id ].name == NULL ) {
		ASSERT( strlen( not_available ) + strlen( opt_text_buffer ) <= OPT_MAX_TEXTLEN );
		strcat( opt_text_buffer, not_available );
		return;
	}

	ASSERT( strlen( protocol_table[ protsubsys_id ].name ) + 2 + strlen( opt_text_buffer ) <= OPT_MAX_TEXTLEN );
	strcat( opt_text_buffer, protocol_table[ protsubsys_id ].name );
	strcat( opt_text_buffer, "  " );
}


// create options menu item text: kill limit ----------------------------------
//
PRIVATE
void CreateOptionTextKillLimit( int id )
{
	ASSERT( strlen( option_lines[ id ] ) + 2 <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );

	if ( cur_killlimit == 0 ) {
		sprintf( paste_str, "off" );
	} else {
		sprintf( paste_str, "%d", cur_killlimit );
	}

	strcat( opt_text_buffer, paste_str );
	strcat( opt_text_buffer, "  " );
}


// create options menu item text: kill limit ----------------------------------
//
PRIVATE
void CreateOptionTextSolarSystem( int id )
{
	ASSERT( strlen( option_lines[ id ] ) + 2 <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );

	sprintf( paste_str, "%d", cur_solarsys - 1 );

	strcat( opt_text_buffer, paste_str );
	strcat( opt_text_buffer, "  " );
}


// create options menu item text: enable/disable sound effects ----------------
//
PRIVATE
void CreateOptionTextSoundEnabling( int id )
{
	// determine current setting
	int cursetting = 0x00;
	if ( Op_SoundEffects )
		cursetting |= 0x01;
	if ( Op_Music )
		cursetting |= 0x02;

	ASSERT( strlen( option_lines[ id ] ) + strlen( soundopt_strings[ cursetting ] ) <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );
	strcat( opt_text_buffer, soundopt_strings[ cursetting ] );
}


// create options menu item text: select controller ---------------------------
//
PRIVATE
void CreateOptionTextControllerEnabling( int id )
{
	// determine current setting
	int cursetting = 0x00;
	if ( Op_Joystick )
		cursetting |= 0x01;
	if ( Op_Mouse )
		cursetting |= 0x02;

	ASSERT( strlen( option_lines[ id ] ) + strlen( ctrlopt_strings[ cursetting ] ) <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );
	strcat( opt_text_buffer, ctrlopt_strings[ cursetting ] );
}


// create options menu item text: invert y axis of mouse ----------------------
//
PRIVATE
void CreateOptionTextInvertYAxis( int id )
{
	ASSERT( strlen( option_lines[ id ] ) + strlen( binopt_strings[ inp_mouse_invert_yaxis ] ) <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );
	strcat( opt_text_buffer, binopt_strings[ inp_mouse_invert_yaxis ] );
}


// create options menu item text: set mouse sensitivity -----------------------
//
PRIVATE
void CreateOptionTextMouseSensitivity( int id )
{
	ASSERT( strlen( option_lines[ id ] ) + 2 <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );

	sprintf( paste_str, "%d", cur_sensitivity );

	strcat( opt_text_buffer, paste_str );
	strcat( opt_text_buffer, "  " );
}


// create options menu item text: set mouse centering speed -------------------
//
PRIVATE
void CreateOptionTextCenterSpeed( int id )
{
	ASSERT( strlen( option_lines[ id ] ) + 2 <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );

	sprintf( paste_str, "%d", cur_centerspeed );

	strcat( opt_text_buffer, paste_str );
	strcat( opt_text_buffer, "  " );
}

// create options menu item text: set mouse centering speed -------------------
//
PRIVATE
void CreateOptionTextName( int id )
{
	ASSERT( strlen( option_lines[ id ] ) + 2 <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );
	if(!mod_player_name) {	
		sprintf( paste_str, "%s", LocalPlayerName );
	} else {
		sprintf(paste_str, "%s", tmp_name);
	}

	strcat( opt_text_buffer, paste_str );
	strcat( opt_text_buffer, "  " );
}
// create options menu item text: default -------------------------------------
//
PRIVATE
void CreateOptionTextDefault( int id )
{
	ASSERT( strlen( option_lines[ id ] ) <= OPT_MAX_TEXTLEN );
	strcpy( opt_text_buffer, option_lines[ id ] );
}


// create text for one line in the options menu -------------------------------
//
PRIVATE
void CreateOptionsText( int id )
{
	switch ( id ) {

		case RES_OPT:
			CreateOptionTextResolution( id );
			break;

		case DEPTH_OPT:
			CreateOptionTextColordepth( id );
			break;

		case FULL_OPT:
			CreateOptionTextFullscreen( id );
			break;

		case SYNC_OPT:
			CreateOptionTextFlipsync( id );
			break;

		case TEXDETAIL_OPT:
			CreateOptionTextTextureDetail( id );
			break;

		case GEOMDETAIL_OPT:
			CreateOptionTextGeometryDetail( id );
			break;

		case SFXDETAIL_OPT:
			CreateOptionTextSoundDetail( id );
			break;

		case PROTOCOL_OPT:
			CreateOptionTextProtocol( id );
			break;

		case KILL_LIMIT_OPT:
			CreateOptionTextKillLimit( id );
			break;

		case SOLAR_OPT:
			CreateOptionTextSolarSystem( id );
			break;

		case SOUND_OPT:
			CreateOptionTextSoundEnabling( id );
			break;

		case CTRL_OPT:
			CreateOptionTextControllerEnabling( id );
			break;

		case INVERT_OPT:
			CreateOptionTextInvertYAxis( id );
			break;

		case SENSITIVITY_OPT:
			CreateOptionTextMouseSensitivity( id );
			break;
		
		case CENTER_OPT:
			CreateOptionTextCenterSpeed( id );
			break;
		case NAME_OPT:
			CreateOptionTextName( id );
			break;
		default:
			CreateOptionTextDefault( id );
			break;
	}
}

// filters unsafe charaters from the key pressed buffer.
//void OptionsNameCharaterFilter(dword key){




// used to modify name, modifies the past_str buffer thing
void OptionsKeyPressed(int key)
{
    if (!mod_player_name) {
        return;
    }

	if (key == MKC_BACKSPACE) {
		if (tmp_name[0] != '\0') {
			tmp_name[strlen(tmp_name) - 1] = '\0';
		}
	} else if (key == MKC_ENTER) {
		ExecOptionSelectName();
	} else if (key == MKC_ESCAPE) {
		
		mod_player_name = false; // exit with no modification
		highlight_locked = false;
	}
}

void OptionsProcessTextInput(char character)
{
    if (!mod_player_name) {
        return;
    }

    // We can only handle ASCII for now.
    if (character < 32 || character >= 127) {
        return;
    }

    size_t namelen = strlen(tmp_name);

    if (namelen + 1 < MAX_PLAYER_NAME) {
        tmp_name[namelen] = character;
        tmp_name[namelen+1] = 0;
    }
}


// slide the options menu to its specifed target x position -------------------
//
PRIVATE
void DoOptionsSliding()
{
	if ( opt_slidepos == opt_slidetarget ) {
		opt_lastref = REFFRAME_INVALID;
		return;
	}

	if ( opt_slidepos < opt_slidetarget ) {

		// slide right
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( opt_lastref == REFFRAME_INVALID ) {
			opt_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - opt_lastref;
			for ( ; delta >= OPT_SLIDE_SPEED; delta -= OPT_SLIDE_SPEED ) {
				opt_slidepos++;
				if ( opt_slidepos >= opt_slidetarget ) {
					opt_slidepos = opt_slidetarget;
					opt_lastref  = REFFRAME_INVALID;
					break;
				}
				opt_lastref += OPT_SLIDE_SPEED;
			}
		}

	} else {

		// slide left
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( opt_lastref == REFFRAME_INVALID ) {
			opt_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - opt_lastref;
			for ( ; delta >= OPT_SLIDE_SPEED; delta -= OPT_SLIDE_SPEED ) {
				opt_slidepos--;
				if ( opt_slidepos <= opt_slidetarget ) {
					opt_slidepos = opt_slidetarget;
					opt_lastref  = REFFRAME_INVALID;
					break;
				}
				opt_lastref += OPT_SLIDE_SPEED;
			}
		}
	}
}


// get blink states, reset each automatically if option and state identical ---
//
PRIVATE
void UpdateBlinkStates()
{
	// blink state of apply video options
	int diffvid = 0;
	diffvid += ( vidsubsys_id	!= prev_vidsubsys_id );
	diffvid += ((cur_res		!= GameScreenRes));
	diffvid += ( cur_depthmode	!= GameScreenBPP );
	diffvid += ( (dword)cur_winstatus != GameScreenWindowed );
	diffvid += ( cur_flipsync	!= FlipSynched );

	option_apply_blinking[ APPLY_VID_OPT ] = ( diffvid > 0 );

	// blink state of apply detail options
	int diffdet = 0;
	diffdet += ( cur_mipmapbias != AUXDATA_TMM_MIPMAP_LOD_BIAS );
	diffdet += ( cur_geombias   != AUXDATA_LOD_DISCRETE_GEOMETRY_BIAS );
	diffdet += ( cur_sfxquality != prev_sfxquality );

	option_apply_blinking[ APPLY_DETAIL_OPT ] = ( diffdet > 0 );

	// blink state of apply net options
	int diffnet = 0;
	diffnet += ( protsubsys_id   != prev_protsubsys_id );
	diffnet += ( cur_killlimit 	 != AUX_KILL_LIMIT_FOR_GAME_END );
	diffnet += ( cur_solarsys	 != AUXDATA_BACKGROUND_NEBULA_ID );

	option_apply_blinking[ APPLY_NET_OPT ] = ( diffnet > 0 );
}


// type for writestring function pointer --------------------------------------
//
typedef void (*WSFP)( ... );

//NOTE:
// this declaration is in global scope because of the extern "C".
// only gcc needs the otherwise redundant curly braces.


// draw horizontal divider line built from minus signs ------------------------
//
static
void DrawDividerLine( WSFP wstrfp, int lx, int ly, int len )
{
	ASSERT( wstrfp != NULL );

	ASSERT( len <= PASTE_STR_LEN );
	int dpos = 0;
	for ( dpos = 0; dpos < len; dpos++ ) {
		paste_str[ dpos ] = '-';
	}
	paste_str[ dpos ] = 0;

	wstrfp( paste_str, lx, ly, TRTAB_PANELTEXT );
}


// draw translucent background window for options menu ------------------------
//
PRIVATE
void DrawOptionsFrame()
{
	int frame_l		  = options_content_metrics.frame_l;
	int frame_r		  = options_content_metrics.frame_r;
	int frame_t		  = options_content_metrics.frame_t;
	int frame_b		  = options_content_metrics.frame_b;
	int chwidth		  = options_content_metrics.chwidth;
	int chheight	  = options_content_metrics.chheight;
	int text_x		  = options_content_metrics.text_x;
	int text_y		  = options_content_metrics.text_y;
	int maxcontwidth  = options_content_metrics.maxcontwidth;
	int maxcontheight = options_content_metrics.maxcontheight;

	int backx = text_x - frame_l * chwidth;
	int backy = text_y - frame_t * chheight;
	int backw = ( frame_l + frame_r + maxcontwidth ) * chwidth;
	int backh = ( frame_t + frame_b + maxcontheight ) * chheight;

	DRAW_ClippedTrRect( backx, backy, backw, backh, TRTAB_PANELBACK );
}


// draw caption of options window ---------------------------------------------
//
PRIVATE
void DrawOptionsCaption( WSFP wstrfp )
{
	unsigned int chwidth		  = options_content_metrics.chwidth;
	unsigned int chheight	  = options_content_metrics.chheight;
	int text_x		  = options_content_metrics.text_x;
	int text_y		  = options_content_metrics.text_y;
	unsigned int maxcontwidth  = options_content_metrics.maxcontwidth;
	unsigned int maxcontheight = options_content_metrics.maxcontheight;

	// display options menu caption
	ASSERT( maxcontwidth >= strlen( opt_caption_str ) );
	int xofs = ( ( maxcontwidth - strlen( opt_caption_str ) ) * chwidth ) / 2;
	wstrfp( opt_caption_str, text_x + xofs, text_y, TRTAB_PANELTEXT );
	text_y += chheight * 2;

	// this destroys paste_str
	DrawDividerLine( wstrfp, text_x, text_y, maxcontwidth );
	text_y += chheight;

	options_content_metrics.text_y = text_y;
}


// draw all options menu items (text lines) -----------------------------------
//
void DrawOptionsItems( WSFP wstrfp )
{
	unsigned int chwidth		 = options_content_metrics.chwidth;
	unsigned int chheight	 = options_content_metrics.chheight;
	int text_x		 = options_content_metrics.text_x;
	int text_y		 = options_content_metrics.text_y;
	unsigned int maxcontwidth = options_content_metrics.maxcontwidth;

	// display all options
	for ( int id = 0; id < cur_opmenu_size; id++ ) {

		CreateOptionsText( id );

		// draw/erase cursor
		char cursor = ( cur_opmenu_selindx == id ) ? '-' : ' ';
		*opt_text_buffer = cursor;
		*( opt_text_buffer + strlen( opt_text_buffer ) - 1 ) = cursor;

		ASSERT( maxcontwidth >= strlen( opt_text_buffer ) );
		int xofs = ( ( maxcontwidth - strlen( opt_text_buffer ) ) * chwidth ) / 2;

		int oldalpha = PanelTextColor.A;
		
		if ( option_dynamic[ id ] && !enable_dynamic_options ) {
			PanelTextColor.A = 0x20;
		}

		if ( option_apply_blinking[ id ] ) {

			if ( opt_blink_count < OPT_MAX_BLINK_COUNT/2 ) {
				// double bright
				wstrfp( opt_text_buffer, text_x + xofs, text_y, TRTAB_PANELTEXT );
				wstrfp( opt_text_buffer, text_x + xofs, text_y, TRTAB_PANELTEXT );
			}

		} else {

			// double bright if line of cursor
			if ( cur_opmenu_selindx == id && enable_dynamic_options )
				wstrfp( opt_text_buffer, text_x + xofs, text_y, TRTAB_PANELTEXT );
			wstrfp( opt_text_buffer, text_x + xofs, text_y, TRTAB_PANELTEXT );
		}

		if ( option_dynamic[ id ] && !enable_dynamic_options ) {
			PanelTextColor.A = oldalpha;
		}

		text_y += chheight;
		if ( option_spacing[ id ] != OPT_SPACING_NONE ) {
			if ( option_spacing[ id ] == OPT_SPACING_DIVIDER ) {
				// this destroys paste_str
				DrawDividerLine( wstrfp, text_x, text_y, maxcontwidth );
			}
			text_y += chheight;
		}
	}
}


// draw warning text in options menu if active --------------------------------
//
PRIVATE
void DrawOptionsWarnings( WSFP wstrfp )
{
	unsigned int chwidth		  = options_content_metrics.chwidth;
	unsigned int chheight	  = options_content_metrics.chheight;
	int text_x		  = options_content_metrics.text_x;
	int text_y		  = options_content_metrics.text_y;
	unsigned int maxcontwidth  = options_content_metrics.maxcontwidth;
	unsigned int maxcontheight = options_content_metrics.maxcontheight;

	// draw connect warning
	if ( opt_show_warning_text ) {

		if ( opt_blink_ctr >= 4 ) {

			opt_show_warning_text = FALSE;
			opt_blink_ctr = 0;

		} else {

			if ( opt_blink_count < OPT_MAX_BLINK_COUNT/2 ) {

				ASSERT( maxcontwidth >= strlen( opt_warning_str ) );

				int xofs = ( ( maxcontwidth - strlen( opt_warning_str ) ) * chwidth ) / 2;
				int stry = text_y + ( maxcontheight - 1 ) * chheight;

				// double bright
				wstrfp( opt_warning_str, text_x + xofs, stry, TRTAB_PANELTEXT );
				wstrfp( opt_warning_str, text_x + xofs, stry, TRTAB_PANELTEXT );
			}
		}
	}
}


// calc and store metrics for options window ----------------------------------
//
PRIVATE
void CalcOptionsContentMetrics()
{
	// bounds for content
	int maxcontwidth  = strlen( opt_caption_str );
	int maxcontheight = 32;

	// ensure maximum content width is big enough
	for ( int id = 0; id < cur_opmenu_size; id++ ) {

		CreateOptionsText( id );

		int optexlen = strlen( opt_text_buffer );
		if ( optexlen > maxcontwidth ) {
			maxcontwidth = optexlen;
		}
	}

	extern int hud_line_dist;

	// font metrics
	int chwidth  = CharsetInfo[ HUD_CHARSETNO ].width;
	int chheight = hud_line_dist;

	D_SetWStrContext( CharsetInfo[ HUD_CHARSETNO ].charsetpointer,
					  CharsetInfo[ HUD_CHARSETNO ].geompointer,
					  NULL,
					  CharsetInfo[ HUD_CHARSETNO ].width,
					  CharsetInfo[ HUD_CHARSETNO ].height );

	// frame metrics
	int frame_l = 1;
	int frame_r = 1;
	int frame_t = 2;
	int frame_b = 1;

	// text position
	int text_x = Screen_Width - ( frame_r + maxcontwidth ) * chwidth - 30;
	if ( opt_slidepos != -1 )
		text_x += m_sintab[ opt_slidepos ];
	int text_y = 62;

	// store metrics
	options_content_metrics.valid		  = TRUE;
	options_content_metrics.frame_l		  = frame_l;
	options_content_metrics.frame_r		  = frame_r;
	options_content_metrics.frame_t		  = frame_t;
	options_content_metrics.frame_b		  = frame_b;
	options_content_metrics.chwidth		  = chwidth;
	options_content_metrics.chheight	  = chheight;
	options_content_metrics.text_x		  = text_x;
	options_content_metrics.text_y		  = text_y;
	options_content_metrics.maxcontwidth  = maxcontwidth;
	options_content_metrics.maxcontheight = maxcontheight;
}


// draw options menu window ---------------------------------------------------
//
void DrawOptionsMenuWindow()
{
	DoOptionsSliding();
	UpdateBlinkStates();

	CalcOptionsContentMetrics();

	// check whether actual drawing disabled
	if ( AUX_DISABLE_FLOATING_MENU_DRAWING )
		return;

	// determine whether translucency should be used
	int translucent = VID_TRANSLUCENCY_SUPPORTED;

	// write text transparent only for color depths below 32 bit per pixel
	WSFP wstrfp = translucent ? (WSFP)&D_WriteTrString : (WSFP)&D_WriteString;

	// draw frame
	if ( translucent ) {
		DrawOptionsFrame();
	}

	// draw caption
	DrawOptionsCaption( wstrfp );

	// draw items
	DrawOptionsItems( wstrfp );

	// draw warnings if active
	DrawOptionsWarnings( wstrfp );

	// maintain blink counter
	if ( ( opt_blink_count += CurScreenRefFrames ) > OPT_MAX_BLINK_COUNT ) {
		opt_blink_count = 0;
		opt_blink_ctr++;
	}
}



