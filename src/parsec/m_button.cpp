/*
 * PARSEC - Menu Buttons
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:26 $
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

// subsystem headers
#include "aud_defs.h"
#include "sys_defs.h"
#include "vid_defs.h"

// local module header
#include "m_button.h"

// proprietary module headers
#include "con_aux.h"
#include "e_draw.h"
#include "e_supp.h"
#include "m_main.h"
#include "sys_bind.h"



// menu item structure --------------------------------------------------------
//
struct menuitem_s {

	int				item;
	menuitem_s*		submenu;
	int				disabled;
	int				_mksiz16;
};


// menu definition ------------------------------------------------------------
//
static menuitem_s game_menu[ NUM_MENU_ITEMS ] = {

	{ M_CONNECT,	NULL,		FALSE	},
#ifdef CLIENT_BUILD_LAN_ONLY_VERSION
	{ M_STARMAP,	NULL,		FALSE	},
#else
	{ M_STARMAP,	NULL,		FALSE	},
#endif
//	{ M_PLAY_DEMO,	NULL,		FALSE	},
	{ M_BACK,		NULL,		FALSE	},
};

static menuitem_s serv_menu[ NUM_MENU_ITEMS ] = {

	{ M_CREATE,		NULL,		TRUE	},
	{ M_GAME_MOD,	NULL,		TRUE	},
	{ M_SETTINGS,	NULL,		FALSE	},
	{ M_BACK,		NULL,		FALSE	},
};

static menuitem_s conf_menu[ NUM_MENU_ITEMS ] = {

	{ M_SPACECRAFT,	NULL,		FALSE	},
	{ M_CONTROLS,	NULL,		TRUE	},
	{ M_OPTIONS,	NULL,		FALSE	},
	{ M_BACK,		NULL,		FALSE	},
};

static menuitem_s root_menu[ NUM_MENU_ITEMS ] = {

	{ M_CONNECT,	NULL	    ,	FALSE	},
//	{ M_SERVER,		serv_menu,	FALSE	}, //how to do sub menus see above examples incase ever needed
	{ M_STARMAP, NULL, FALSE },
    { M_SPACECRAFT,         NULL,           FALSE   },
	{ M_OPTIONS,		NULL,	        FALSE	},
	{ M_QUIT,		NULL,		FALSE	},
};

// button info structures -----------------------------------------------------
//
struct buttoninfo_s {

	short		width;
	short		height;
	TextureMap*	texmap;
	const char*	texname;
	dword		reqmask;
};

struct buttonpos_s {

	int			buttonx;
	int			buttony;
};


// masks for required buttons depending on build ------------------------------
//
#define BUTTREQ_GAME		0x0001
#define BUTTREQ_DEMO		0x0002

#ifdef CLIENT_BUILD_SELF_RUNNING_DEMO
	#define BUTTREQ_BUILD	BUTTREQ_DEMO
#else
	#define BUTTREQ_BUILD	BUTTREQ_GAME
#endif


// button info tables ---------------------------------------------------------
//
static buttoninfo_s button_info[] = {

	// root menu
	{ 256, 128, NULL, "game_off", BUTTREQ_GAME					},
	{ 256, 128, NULL, "game_on",  BUTTREQ_GAME					},
	{ 256, 128, NULL, "serv_off", BUTTREQ_GAME					},
	{ 256, 128, NULL, "serv_on",  BUTTREQ_GAME					},
	{ 256, 128, NULL, "conf_off", BUTTREQ_GAME | BUTTREQ_DEMO	},
	{ 256, 128, NULL, "conf_on",  BUTTREQ_GAME | BUTTREQ_DEMO	},
	{ 256, 128, NULL, "quit_off", BUTTREQ_GAME | BUTTREQ_DEMO	},
	{ 256, 128, NULL, "quit_on",  BUTTREQ_GAME | BUTTREQ_DEMO	},

	// game menu
	{ 256, 128, NULL, "conn_off", BUTTREQ_GAME					},
	{ 256, 128, NULL, "conn_on",  BUTTREQ_GAME					},
	{ 256, 128, NULL, "join_off", BUTTREQ_GAME					},
	{ 256, 128, NULL, "join_on",  BUTTREQ_GAME					},
	{ 256, 128, NULL, "sta_off",  BUTTREQ_GAME | BUTTREQ_DEMO	},
	{ 256, 128, NULL, "sta_on",   BUTTREQ_GAME | BUTTREQ_DEMO	},
	{ 256, 128, NULL, "pla_off",  BUTTREQ_GAME | BUTTREQ_DEMO	},
	{ 256, 128, NULL, "pla_on",   BUTTREQ_GAME | BUTTREQ_DEMO	},
	{ 256, 128, NULL, "dcon_off", BUTTREQ_GAME					},
	{ 256, 128, NULL, "dcon_on",  BUTTREQ_GAME					},

	// serv menu
	{ 256, 128, NULL, "crea_off", BUTTREQ_GAME					},
	{ 256, 128, NULL, "crea_on",  BUTTREQ_GAME					},
	{ 256, 128, NULL, "mod_off",  BUTTREQ_GAME					},
	{ 256, 128, NULL, "mod_on",   BUTTREQ_GAME					},
	{ 256, 128, NULL, "sett_off", BUTTREQ_GAME					},
	{ 256, 128, NULL, "sett_on",  BUTTREQ_GAME					},

	// conf menu
	{ 256, 128, NULL, "scv_off",  BUTTREQ_GAME | BUTTREQ_DEMO	},
	{ 256, 128, NULL, "scv_on",   BUTTREQ_GAME | BUTTREQ_DEMO	},
	{ 256, 128, NULL, "cont_off", BUTTREQ_GAME | BUTTREQ_DEMO	},
	{ 256, 128, NULL, "cont_on",  BUTTREQ_GAME | BUTTREQ_DEMO	},
	{ 256, 128, NULL, "opt_off",  BUTTREQ_GAME | BUTTREQ_DEMO	},
	{ 256, 128, NULL, "opt_on",   BUTTREQ_GAME | BUTTREQ_DEMO	},
	{ 256, 128, NULL, "back_off", BUTTREQ_GAME | BUTTREQ_DEMO	},
	{ 256, 128, NULL, "back_on",  BUTTREQ_GAME | BUTTREQ_DEMO	},

	// right button cap
	{  64, 128, NULL, "button_r", BUTTREQ_GAME | BUTTREQ_DEMO	},

	{   0,   0, NULL, NULL,       0x0000						}
};

static buttonpos_s button_pos[ NUM_MENU_ITEMS ] = {

	{ 5,   7 },
	{ 5,  87 },
	{ 5, 167 },
	{ 5, 247 },
	{ 5, 327 },
};

static buttoninfo_s*	button_cap_info	= &button_info[ M_BUTTON_CAP * 2 ];
static buttonpos_s		button_cap_pos	= { 5 + 256, 0 };


// current menu info ----------------------------------------------------------
//
static menuitem_s* 	cur_menu			= root_menu;
static menuitem_s* 	prev_menu			= NULL;
static int 			cur_menu_size		= NUM_MENU_ITEMS;
static int 			cur_menu_selindx	= 0;
static int 			prev_menu_selindx	= 0;


// select submenu -------------------------------------------------------------
//
void MenuButtonsEnterSubmenu()
{
	ASSERT( prev_menu == NULL );

	prev_menu = cur_menu;
	cur_menu  = cur_menu[ cur_menu_selindx ].submenu;

	prev_menu_selindx = cur_menu_selindx;
	cur_menu_selindx  = 0;

	// skip to first enabled button
	while ( cur_menu[ cur_menu_selindx ].disabled ) {
		if ( ++cur_menu_selindx >= cur_menu_size ) {
			cur_menu_selindx = 0;
		}
	}
}


// leave submenu to root menu -------------------------------------------------
//
void MenuButtonsLeaveSubmenu()
{
	ASSERT( prev_menu != NULL );

	cur_menu  = prev_menu;
	prev_menu = NULL;

	cur_menu_selindx  = prev_menu_selindx;
	prev_menu_selindx = 0;
}


// return currently selected menu item id -------------------------------------
//
int MenuButtonsSelection()
{
	return cur_menu[ cur_menu_selindx ].item;
}


// escape pressed in menu: select quit button or denote submenu exit ----------
//
int MenuButtonsEscape()
{
	if ( cur_menu == root_menu ) {

		// in root menu, <ESC> selects QUIT menu item
		cur_menu_selindx = 4;
		return TRUE;

	} else {

		// this will cause a submenu exit
		return FALSE;
	}
}


// move one button up in menu -------------------------------------------------
//
void MenuButtonsCursorUp()
{
	if ( --cur_menu_selindx < 0 ) {
		cur_menu_selindx = cur_menu_size - 1;
	}

	// skip disabled buttons
	while ( cur_menu[ cur_menu_selindx ].disabled ) {
		if ( --cur_menu_selindx < 0 ) {
			cur_menu_selindx = cur_menu_size - 1;
		}
	}

	AUD_Select1();
}


// move one button down in menu -----------------------------------------------
//
void MenuButtonsCursorDown()
{
	if ( ++cur_menu_selindx >= cur_menu_size ) {
		cur_menu_selindx = 0;
	}

	// skip disabled buttons
	while ( cur_menu[ cur_menu_selindx ].disabled ) {
		if ( ++cur_menu_selindx >= cur_menu_size ) {
			cur_menu_selindx = 0;
		}
	}

	AUD_Select1();
}


// toggle menu buttons on connect ---------------------------------------------
//
void MenuButtonsToggleConnect()
{
	// connect -> join game
	root_menu[ 0 ].item = M_JOIN_GAME;

	// change starmap button to disconnect
	root_menu[1].item = M_DISCONNECT;

}


// toggle menu buttons on disconnect ------------------------------------------
//
void MenuButtonsToggleDisconnect()
{
	// join game -> connect
	root_menu[ 0 ].item = M_CONNECT;

	// bring back the starmap button
	root_menu[1].item = M_STARMAP;
}


// button animation state vars ------------------------------------------------
//
static int button_slidepos[ NUM_MENU_ITEMS ] =
{ BUTTON_POS_LEFT, BUTTON_POS_LEFT, BUTTON_POS_LEFT, BUTTON_POS_LEFT, BUTTON_POS_LEFT };

static int button_slidetarget[ NUM_MENU_ITEMS ] =
{  BUTTON_POS_RIGHT, BUTTON_POS_RIGHT, BUTTON_POS_RIGHT, BUTTON_POS_RIGHT, BUTTON_POS_RIGHT };

static refframe_t button_lastref[ NUM_MENU_ITEMS ] =
{ REFFRAME_INVALID, REFFRAME_INVALID, REFFRAME_INVALID, REFFRAME_INVALID , REFFRAME_INVALID};

static int button_sound_played[ NUM_MENU_ITEMS ] =
{  FALSE, FALSE, FALSE, FALSE, FALSE };

static int button_fadepos[ NUM_MENU_ITEMS ] =
	{ BUTTON_ALPHA_HIGH, BUTTON_ALPHA_HIGH, BUTTON_ALPHA_HIGH, BUTTON_ALPHA_HIGH, BUTTON_ALPHA_HIGH };

static int button_fadetarget[ NUM_MENU_ITEMS ] =
	{ BUTTON_ALPHA_HIGH, BUTTON_ALPHA_HIGH, BUTTON_ALPHA_HIGH, BUTTON_ALPHA_HIGH, BUTTON_ALPHA_HIGH };

static refframe_t button_lastfaderef[ NUM_MENU_ITEMS ] =
	{ REFFRAME_INVALID, REFFRAME_INVALID, REFFRAME_INVALID, REFFRAME_INVALID, REFFRAME_INVALID };


// determine whether mouse cursor is over a menu button -----------------------
//
int MouseOverMenuButton( int mousex, int mousey )
{
	// check against menu buttons
	for ( int bno = 0; bno < NUM_MENU_ITEMS; bno++ ) {

		if ( button_slidepos[ bno ] != BUTTON_POS_RIGHT )
			continue;

		// calc index into button_info[]
		int bindx = cur_menu[ bno ].item * 2;
		ASSERT( button_info[ bindx ].texname != NULL );

		int buttonwidth  = button_info[ bindx ].width + button_cap_info->width;
		int buttonheight = BUTTON_HEIGHT;

		if ( ( mousex >= button_pos[ bno ].buttonx ) &&
			 ( mousex < ( button_pos[ bno ].buttonx + buttonwidth ) ) &&
			 ( mousey >= button_pos[ bno ].buttony ) &&
			 ( mousey < ( button_pos[ bno ].buttony + buttonheight ) ) ) {

			// set selected index directly
			if ( !cur_menu[ bno ].disabled ) {

				if ( cur_menu_selindx != bno ) {
					cur_menu_selindx = bno;
					AUD_Select1();
				}

				return MOUSE_OVER_BUTTON;

			} else {

				return MOUSE_OVER_NOTHING;
			}
		}
	}

	return MOUSE_OVER_NOTHING;
}


// set position and slide target for menu buttons to right (in) ---------------
//
void MoveInButtons()
{
	for ( int bno = 0; bno < NUM_MENU_ITEMS; bno++ ) {

		button_slidepos[ bno ]    = BUTTON_POS_RIGHT;
		button_slidetarget[ bno ] = button_slidepos[ 0 ];
	}
}


// set position and slide target for menu buttons to left (out) ---------------
//
void MoveOutButtons()
{
	for ( int bno = 0; bno < NUM_MENU_ITEMS; bno++ ) {

		button_slidepos[ bno ]		= BUTTON_POS_LEFT;
		button_slidetarget[ bno ]	= button_slidepos[ 0 ];
		button_sound_played[ bno ]	= FALSE;
	}
}


// set slide target for menu buttons to right (in) ----------------------------
//
void SlideInButtons()
{
	for ( int bno = 0; bno < NUM_MENU_ITEMS; bno++ ) {

		button_slidetarget[ bno ] = BUTTON_POS_RIGHT;
	}
}


// set slide target for menu buttons to left (out) ----------------------------
//
void SlideOutButtons()
{
	for ( int bno = 0; bno < NUM_MENU_ITEMS; bno++ ) {

		button_slidetarget[ bno ]  = BUTTON_POS_LEFT;
		button_sound_played[ bno ] = FALSE;
	}
}


// check if button sliding has finished ---------------------------------------
//
int SlideFinishedButtons()
{
	int testindx = NUM_MENU_ITEMS - 1;

	return ( button_slidepos[ testindx ] == button_slidetarget[ testindx ] );
}


// set fade target for buttons to high (in) -----------------------------------
//
void FadeInButtons()
{
	for ( int bno = 0; bno < NUM_MENU_ITEMS; bno++ ) {

		button_fadetarget[ bno ] = BUTTON_ALPHA_HIGH;
	}
}


// set fade target for buttons to low (out) -----------------------------------
//
void FadeOutButtons( int target )
{
	for ( int bno = 0; bno < NUM_MENU_ITEMS; bno++ ) {

		button_fadetarget[ bno ] = target;
	}
}


// check if button fading has finished ----------------------------------------
//
int FadeFinishedButtons()
{
	return ( button_fadepos[ 0 ] == button_fadetarget[ 0 ] );
}


// slide a button to its specifed target x position ---------------------------
//
PRIVATE
int DoButtonSliding( int bno )
{
	ASSERT( ( bno >= 0 ) && ( bno < NUM_MENU_ITEMS ) );

	if ( button_slidepos[ bno ] == button_slidetarget[ bno ] ) {

		button_lastref[ bno ] = REFFRAME_INVALID;

		if ( button_slidepos[ bno ] == BUTTON_POS_LEFT ) {
			return FALSE;
		}
		if ( !button_sound_played[ bno ] ) {
			AUD_ButtonSlided();
			button_sound_played[ bno ] = TRUE;
		}
		return TRUE;
	}

	if ( button_slidepos[ bno ] < button_slidetarget[ bno ] ) {

		// slide right
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( button_lastref[ bno ] == REFFRAME_INVALID ) {
			button_lastref[ bno ] = refframecount;
		} else {
			refframe_t delta = refframecount - button_lastref[ bno ];
			for ( ; delta >= BUTTON_SLIDE_SPEED; delta -= BUTTON_SLIDE_SPEED ) {
				button_slidepos[ bno ]++;
				if ( button_slidepos[ bno ] >= button_slidetarget[ bno ] ) {
					button_slidepos[ bno ] = button_slidetarget[ bno ];
					button_lastref[ bno ]  = REFFRAME_INVALID;
					break;
				}
				button_lastref[ bno ] += BUTTON_SLIDE_SPEED;
			}
		}

		// phase shifting
		if ( ( bno > 0 ) && ( button_slidepos[ bno - 1 ] < button_slidetarget[ bno - 1 ] ) ) {
			button_slidepos[ bno ] = button_slidepos[ bno - 1 ] - BUTTON_PHASE_OFS;
			if ( button_slidepos[ bno ] < 0 ) {
				button_slidepos[ bno ] = 0;
			}
		}

	} else {

		// slide left
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( button_lastref[ bno ] == REFFRAME_INVALID ) {
			button_lastref[ bno ] = refframecount;
		} else {
			refframe_t delta = refframecount - button_lastref[ bno ];
			for ( ; delta >= BUTTON_SLIDE_SPEED; delta -= BUTTON_SLIDE_SPEED ) {
				button_slidepos[ bno ]--;
				if ( button_slidepos[ bno ] <= button_slidetarget[ bno ] ) {
					button_slidepos[ bno ] = button_slidetarget[ bno ];
					button_lastref[ bno ]  = REFFRAME_INVALID;
					break;
				}
				button_lastref[ bno ] += BUTTON_SLIDE_SPEED;
			}
		}

		// phase shifting
		if ( ( bno > 0 ) && ( button_slidepos[ bno - 1 ] > button_slidetarget[ bno - 1 ] ) ) {
			button_slidepos[ bno ] = button_slidepos[ bno - 1 ] + BUTTON_PHASE_OFS;
			if ( button_slidepos[ bno ] >= m_sintab_size ) {
				button_slidepos[ bno ] = m_sintab_size - 1;
			}
		}
	}

	return TRUE;
}


// fade a button alpha to a specified target ----------------------------------
//
PRIVATE
int DoButtonFading( int bno )
{
	ASSERT( ( bno >= 0 ) && ( bno < NUM_MENU_ITEMS ) );

	if ( button_fadepos[ bno ] == button_fadetarget[ bno ] ) {

		// skip drawing code if button is completely faded out
#if ( BUTTON_ALPHA_LOW == 0 )
		return ( button_fadepos[ bno ] != BUTTON_ALPHA_LOW );
#else
		return TRUE;
#endif

	}

	if ( button_fadepos[ bno ] < button_fadetarget[ bno ] ) {

		// fade in
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( button_lastfaderef[ bno ] == REFFRAME_INVALID ) {
			button_lastfaderef[ bno ] = refframecount;
		} else {
			refframe_t delta = refframecount - button_lastfaderef[ bno ];
			for ( ; delta >= BUTTON_FADE_SPEED; delta -= BUTTON_FADE_SPEED ) {
				button_fadepos[ bno ] += BUTTON_FADE_QUANTUM;
				if ( button_fadepos[ bno ] >= button_fadetarget[ bno ] ) {
					button_fadepos[ bno ] = button_fadetarget[ bno ];
					button_lastfaderef[ bno ] = REFFRAME_INVALID;
					break;
				}
				button_lastfaderef[ bno ] += BUTTON_FADE_SPEED;
			}
		}

	} else {

		// fade out
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( button_lastfaderef[ bno ] == REFFRAME_INVALID ) {
			button_lastfaderef[ bno ] = refframecount;
		} else {
			refframe_t delta = refframecount - button_lastfaderef[ bno ];
			for ( ; delta >= BUTTON_FADE_SPEED; delta -= BUTTON_FADE_SPEED ) {
				button_fadepos[ bno ] -= BUTTON_FADE_QUANTUM;
				if ( button_fadepos[ bno ] <= button_fadetarget[ bno ] ) {
					button_fadepos[ bno ] = button_fadetarget[ bno ];
					button_lastfaderef[ bno ] = REFFRAME_INVALID;
					break;
				}
				button_lastfaderef[ bno ] += BUTTON_FADE_SPEED;
			}
		}
	}

	return TRUE;
}


// draw single menu button ----------------------------------------------------
//
PRIVATE
void DrawButton( int bno, int lit )
{
	ASSERT( ( bno >= 0 ) && ( bno < NUM_MENU_ITEMS ) );
	ASSERT( ( lit == 0 ) || ( lit == 1 ) );

	// disabled buttons must never be lit
	if ( cur_menu[ bno ].disabled ) {
		lit = 0;
	}

	// calc index into button_info[]
	int bindx = cur_menu[ bno ].item * 2 + lit;
	ASSERT( button_info[ bindx ].texname != NULL );

	texscreenrect_s	rect;
	rect.itertype = iter_texrgba | iter_premulblend;
	rect.alpha    = cur_menu[ bno ].disabled ? button_fadepos[ bno ] >> 1 : button_fadepos[ bno ];

	// draw caption
	rect.x = button_pos[ bno ].buttonx;
	if ( button_slidepos[ bno ] != -1 )
		rect.x -= m_sintab[ button_slidepos[ bno ] ];
	rect.y = button_pos[ bno ].buttony;
	rect.w = rect.scaled_w = button_info[ bindx ].width;
	rect.h = rect.scaled_h = BUTTON_HEIGHT;
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = button_info[ bindx ].texmap;

	DRAW_TexturedScreenRect( &rect, NULL );

	// draw right button cap
	rect.x = button_cap_pos.buttonx;
	if ( button_slidepos[ bno ] != -1 )
		rect.x -= m_sintab[ button_slidepos[ bno ] ];
	rect.y = button_pos[ bno ].buttony;
	rect.w = rect.scaled_w = button_cap_info->width;
	rect.h = rect.scaled_h = BUTTON_HEIGHT;
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = button_cap_info->texmap;

	DRAW_TexturedScreenRect( &rect, NULL );
}


// flags whether button textures are valid ------------------------------------
//
static int button_textures_valid = FALSE;


// try to acquire all textures for menu buttons -------------------------------
//
PRIVATE
int AcquireButtonTextures()
{
	if ( button_textures_valid )
		return TRUE;

	button_textures_valid = TRUE;

	for ( int tid = 0; button_info[ tid ].texname; tid++ ) {

		TextureMap *texmap = FetchTextureMap( button_info[ tid ].texname );
		button_info[ tid ].texmap = texmap;

		if ( ( texmap == NULL ) && ( button_info[ tid ].reqmask & BUTTREQ_BUILD ) ) {
			button_textures_valid = FALSE;
		}
	}

	return button_textures_valid;
}


// draw all visible menu buttons ----------------------------------------------
//
void DrawMenuButtons()
{
	// slide and fade buttons
	int drawbutton[ NUM_MENU_ITEMS ];
	int bno = 0;
	for ( bno = 0; bno < NUM_MENU_ITEMS; bno++ ) {

		drawbutton[ bno ] = FALSE;

		if ( !DoButtonSliding( bno ) ) {
			continue;
		}
		if ( !DoButtonFading( bno ) ) {
			continue;
		}

		drawbutton[ bno ] = TRUE;
	}

	//NOTE:
	// we slide and fade before we draw to prevent
	// sliding and fading from being disabled when
	// not all button textures are loaded/valid.

	// check whether actual drawing disabled
	if ( AUX_DISABLE_FLOATING_MENU_DRAWING ) {
		return;
	}

	// fetch all textures
	if ( !AcquireButtonTextures() ) {
		return;
	}

	// draw all buttons
	for ( bno = 0; bno < NUM_MENU_ITEMS; bno++ ) {
		if ( drawbutton[ bno ] ) {
			DrawButton( bno, ( cur_menu_selindx == bno ) );
		}
	}
}


// "connected" display metrics ------------------------------------------------
//
#define CONNECT_WIDTH					64
#define CONNECT_HEIGHT					32
#define CONNECT_ALPHA					255
#define CONNECT_XOFS					10
#define CONNECT_YOFS					7

#define CONN_BLINK_COUNT_MAX			900


// "connected" display vars ---------------------------------------------------
//
static int			conn_blink_count 	= 0;
static char		   	conn_texname[]		= "connected";
static TextureMap*	conn_texmap			= NULL;


// draw "connected" display ---------------------------------------------------
//
void DrawMenuConnected()
{
	// check whether actual drawing disabled
	if ( AUX_DISABLE_FLOATING_MENU_DRAWING )
		return;

	// acquire texture
	if ( conn_texmap == NULL )
		conn_texmap = FetchTextureMap( conn_texname );
	if ( conn_texmap == NULL )
		return;

	// maintain blinking
	if ( ( conn_blink_count += CurScreenRefFrames ) > CONN_BLINK_COUNT_MAX )
		conn_blink_count = 0;

	if ( conn_blink_count < CONN_BLINK_COUNT_MAX/2 ) {

		texscreenrect_s rect;

		rect.x		  = Screen_Width - CONNECT_WIDTH - CONNECT_XOFS;
		rect.y		  = CONNECT_YOFS;
		rect.w		  = rect.scaled_w = CONNECT_WIDTH;
		rect.h		  = rect.scaled_h = CONNECT_HEIGHT;
		rect.texofsx  = 0;
		rect.texofsy  = 0;
		rect.texmap   = conn_texmap;
		rect.itertype = iter_texrgba | iter_premulblend;
		rect.alpha    = CONNECT_ALPHA;

		DRAW_TexturedScreenRect( &rect, NULL );
	}
}


// "loading" display vars -----------------------------------------------------
//
static char		   	loading_texname[]	= "loading";
static TextureMap*	loading_texmap		= NULL;


// draw "loading" display -----------------------------------------------------
//
void DrawMenuLoading()
{
	// check whether actual drawing disabled
	if ( AUX_DISABLE_FLOATING_MENU_DRAWING )
		return;

	// acquire texture
	if ( loading_texmap == NULL )
		loading_texmap = FetchTextureMap( loading_texname );
	if ( loading_texmap == NULL )
		return;

	int old_disable_buffer_clear  = AUX_DISABLE_BUFFER_CLEAR;
	int old_disable_zbuffer_clear = AUX_DISABLE_ZBUFFER_CLEAR;

	AUX_DISABLE_BUFFER_CLEAR = 0;
	AUX_DISABLE_ZBUFFER_CLEAR = 0;

	VIDs_ClearRenderBuffer();

	texscreenrect_s	rect;

	rect.x		  = ( Screen_Width - 128 ) / 2;
	rect.y		  = ( Screen_Height - 32 ) / 2;
	rect.w		  = rect.scaled_w = 128;
	rect.h		  = rect.scaled_h = 32;
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = loading_texmap;
	rect.itertype = iter_texrgba;
	rect.alpha    = 255;

	DRAW_TexturedScreenRect( &rect, NULL );

	VIDs_CommitRenderBuffer();
	
	AUX_DISABLE_BUFFER_CLEAR  = old_disable_buffer_clear;
	AUX_DISABLE_ZBUFFER_CLEAR = old_disable_zbuffer_clear;
}



