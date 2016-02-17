/*
 * PARSEC - List Windows
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

// subsystem headers
#include "aud_defs.h"
#include "inp_defs.h"
#include "net_defs.h"
#include "sys_defs.h"

// drawing subsystem
#include "d_font.h"

// local module header
#include "m_list.h"

// proprietary module headers
#include "con_aux.h"
#include "e_color.h"
#include "e_demo.h"
#include "e_draw.h"
#include "h_cockpt.h"
#include "m_main.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 2047
static char paste_str[ PASTE_STR_LEN + 1 ];


// string constants -----------------------------------------------------------
//
static char playerlist_caption_str[]	= "connected players";
static char playerlist_name_str[]		= "name";
static char playerlist_kills_str[]		= "kills";
static char playerlist_kill_limit_str[]	= "kill limit has been reached!";

static char demolist_caption_str[] 		= "   available demos   ";
static char demolist_freeflight_str[]	= " free flight mode ";

static char serverlist_caption_str[] 	= "  Click Connect  ";
static char serverlist_peertopeer[]		= "name <newname>";


// pointer to current caption of player list window ---------------------------
//
static char *playerlist_caption;


// player and demo list window configuration ----------------------------------
//
#define LIST_POS_RIGHT			0
#define LIST_POS_LEFT			( m_sintab_size - 1 )

#define LIST_SLIDE_SPEED_MENU	12
#define LIST_SLIDE_SPEED_GAME	6

#define LIST_MARGIN				4


// player and demo list state variables ---------------------------------------
//
static int			list_slidepos 		= LIST_POS_RIGHT;
static int			list_slidetarget	= LIST_POS_LEFT;
static refframe_t	list_lastref 		= REFFRAME_INVALID;

static int cur_demolist_selindx			= 0;
static int num_listed_demos				= 0;


// default list window --------------------------------------------------------
//
#if defined ( CLIENT_BUILD_SELF_RUNNING_DEMO ) || defined ( CLIENT_BUILD_LAN_ONLY_VERSION )
	#define LISTWIN_DEFAULT		LISTWIN_DEMOLIST
#else
	#define LISTWIN_DEFAULT		LISTWIN_SERVERLIST
#endif


// currently active mode of list window ---------------------------------------
//
PUBLIC int listwin_mode		= LISTWIN_DEFAULT;


// flag whether list window is active (visible and has focus) -----------------
//
PUBLIC int listwin_active	= FALSE;


// return current selection in demo list --------------------------------------
//
int DemoListSelection()
{
	if ( cur_demolist_selindx < num_listed_demos ) {

		return cur_demolist_selindx;

	} else {

		// this indicates "free flight mode"
		return -1;
	}
}


// one item up in demo list ---------------------------------------------------
//
void DemoListCursorUp()
{
	// wrap around to free flight mode
	if ( --cur_demolist_selindx < 0 )
		cur_demolist_selindx = num_listed_demos;
	AUD_Select2();
}


// one item down in demo list -------------------------------------------------
//
void DemoListCursorDown()
{
	// wrap around to first demo
	if ( ++cur_demolist_selindx > num_listed_demos )
		cur_demolist_selindx = 0;
	AUD_Select2();
}


// current metrics of demo list window contents -------------------------------
//
static list_window_metrics_s demolist_content_metrics = { FALSE };


// set mouse pointer into demo list window ------------------------------------
//
void DemoListMouseSet()
{
	if ( demolist_content_metrics.valid ) {

		int chwidth  = demolist_content_metrics.chwidth;
		int text_x   = demolist_content_metrics.text_x;
		int text_y   = demolist_content_metrics.text_y;
		int linemid2 = demolist_content_metrics.maxcontwidth * chwidth / 2;
		int lineh2   = demolist_content_metrics.chheight / 2;

		// set mouse pos
		mousestate_s mousestate;
		mousestate.xpos = (float)( text_x + linemid2 ) / Screen_Width;
		mousestate.ypos = (float)( text_y + lineh2   ) / Screen_Height;
		INPs_MouseSetState( &mousestate );
	}
}


// detect whether mouse position is over demo list item -----------------------
//
PRIVATE
int MouseOverDemoListItem( int mousex, int mousey )
{
	if ( !demolist_content_metrics.valid )
		return MOUSE_OVER_NOTHING;

	if ( !SlideFinishedListWindow() )
		return MOUSE_OVER_NOTHING;

	int chwidth   = demolist_content_metrics.chwidth;
	int chheight  = demolist_content_metrics.chheight;
	int text_x    = demolist_content_metrics.text_x;
	int text_y    = demolist_content_metrics.text_y;
	int linewidth = demolist_content_metrics.maxcontwidth * chwidth;

	// number of items must be guarded against window height overflow
	int numitems = num_registered_demos + 1;

	// caption: 2 lines, divider line: 1 line
	if ( numitems > demolist_content_metrics.maxcontheight - 3 ) {
		numitems = demolist_content_metrics.maxcontheight - 3;
	}

	// check against demolist items
	for ( int mid = 0; mid < numitems; mid++ ) {

		if ( ( mousex >= text_x ) && ( mousex < ( text_x + linewidth ) ) &&
			 ( mousey >= text_y ) && ( mousey < ( text_y + chheight ) ) ) {

			// set selected index directly
			if ( cur_demolist_selindx != mid )
				AUD_Select2();
			cur_demolist_selindx = mid;

			return MOUSE_OVER_OPTION;
		}

		text_y += chheight;
	}

	return MOUSE_OVER_NOTHING;
}


// detect whether mouse position is over player list item ---------------------
//
PRIVATE
int MouseOverPlayerListItem( int mousex, int mousey )
{
	return MOUSE_OVER_NOTHING;
}


// detect whether mouse position is over server list item ---------------------
//
PRIVATE
int MouseOverServerListItem( int mousex, int mousey )
{
	return MOUSE_OVER_NOTHING;
}


// detect whether mouse position is over item of current list -----------------
//
int MouseOverListItem( int mousex, int mousey )
{
	int item = MOUSE_OVER_NOTHING;

	switch ( listwin_mode ) {

		case LISTWIN_DEMOLIST:
			item = MouseOverDemoListItem( mousex, mousey );
			break;

		case LISTWIN_PLAYERLIST:
			item = MouseOverPlayerListItem( mousex, mousey );
			break;

		case LISTWIN_SERVERLIST:
			item = MouseOverServerListItem( mousex, mousey );
			break;
	}

	return item;
}


// set position and slide target for list window to left (in) -----------------
//
void MoveInListWindow()
{
	list_slidepos	 = LIST_POS_LEFT;
	list_slidetarget = list_slidepos;
}


// set position and slide target for list window to right (out) ---------------
//
void MoveOutListWindow()
{
	list_slidepos	 = LIST_POS_RIGHT;
	list_slidetarget = list_slidepos;
}


// set slide target for list window to left (in) ------------------------------
//
void SlideInListWindow()
{
	list_slidetarget = LIST_POS_LEFT;
}


// set slide target for list window to right (out) ----------------------------
//
void SlideOutListWindow()
{
	list_slidetarget = LIST_POS_RIGHT;
}


// check if list window is at its desired slide position ----------------------
//
int SlideFinishedListWindow()
{
	return ( list_slidepos == list_slidetarget );
}


// slide the list to its specifed target x position ---------------------------
//
PRIVATE
int SlideListWindow()
{
	// skip drawing code if list slided out
	if ( list_slidepos == list_slidetarget ) {
		list_lastref = REFFRAME_INVALID;
		return ( list_slidepos != LIST_POS_RIGHT );
	}

	// twice as fast in game than in menu
	int slidespeed = InFloatingMenu ?
		LIST_SLIDE_SPEED_MENU : LIST_SLIDE_SPEED_GAME;

	if ( list_slidepos < list_slidetarget ) {

		// slide right
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( list_lastref == REFFRAME_INVALID ) {
			list_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - list_lastref;
			for ( ; delta >= slidespeed; delta -= slidespeed ) {
				list_slidepos++;
				if ( list_slidepos >= list_slidetarget ) {
					list_slidepos = list_slidetarget;
					list_lastref  = REFFRAME_INVALID;
					break;
				}
				list_lastref += slidespeed;
			}
		}

	} else {

		// slide left
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( list_lastref == REFFRAME_INVALID ) {
			list_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - list_lastref;
			for ( ; delta >= slidespeed; delta -= slidespeed ) {
				list_slidepos--;
				if ( list_slidepos <= list_slidetarget ) {
					list_slidepos = list_slidetarget;
					list_lastref  = REFFRAME_INVALID;
					break;
				}
				list_lastref += slidespeed;
			}
		}
	}

	return TRUE;
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


// draw translucent background window for demo list window --------------------
//
PRIVATE
void DrawDemoListFrame()
{
	int frame_l		  = demolist_content_metrics.frame_l;
	int frame_r		  = demolist_content_metrics.frame_r;
	int frame_t		  = demolist_content_metrics.frame_t;
	int frame_b		  = demolist_content_metrics.frame_b;
	int chwidth		  = demolist_content_metrics.chwidth;
	int chheight	  = demolist_content_metrics.chheight;
	int text_x		  = demolist_content_metrics.text_x;
	int text_y		  = demolist_content_metrics.text_y;
	int maxcontwidth  = demolist_content_metrics.maxcontwidth;
	int maxcontheight = demolist_content_metrics.maxcontheight;

	int backx = text_x - frame_l * chwidth;
	int backy = text_y - frame_t * chheight;
	int backw = ( frame_l + frame_r + maxcontwidth ) * chwidth;
	int backh = ( frame_t + frame_b + maxcontheight ) * chheight;

	DRAW_ClippedTrRect( backx, backy, backw, backh, TRTAB_PANELBACK );
}


// draw caption of demo list window -------------------------------------------
//
PRIVATE
void DrawDemoListCaption( WSFP wstrfp )
{
	unsigned int chwidth		  = demolist_content_metrics.chwidth;
	unsigned int chheight	  = demolist_content_metrics.chheight;
	int text_x		  = demolist_content_metrics.text_x;
	int text_y		  = demolist_content_metrics.text_y;
	unsigned int maxcontwidth  = demolist_content_metrics.maxcontwidth;

	// draw caption
	ASSERT( maxcontwidth >= strlen( demolist_caption_str ) );
	int xofs = ( ( maxcontwidth - strlen( demolist_caption_str ) ) * chwidth ) / 2;
	wstrfp( demolist_caption_str, text_x + xofs, text_y, TRTAB_PANELTEXT );
	text_y += chheight * 2;

	// draw separator
	DrawDividerLine( wstrfp, text_x, text_y, maxcontwidth );
	text_y += chheight;

	demolist_content_metrics.text_y = text_y;
}


// draw all demo list items (demo names) --------------------------------------
//
PRIVATE
void DrawDemoListItems( WSFP wstrfp )
{
	unsigned int chwidth		  = demolist_content_metrics.chwidth;
	unsigned int chheight	  = demolist_content_metrics.chheight;
	int text_x		  = demolist_content_metrics.text_x;
	int text_y		  = demolist_content_metrics.text_y;
	unsigned int maxcontwidth  = demolist_content_metrics.maxcontwidth;
	unsigned int maxcontheight = demolist_content_metrics.maxcontheight;

	// display all demos
	unsigned int lid = 0;

	for ( unsigned int lid = 0; lid < (unsigned int)num_registered_demos; lid++ ) {

		// caption: 2 lines, divider line: 1 line, free flight: 1 line
		if ( lid >= maxcontheight - 4 )
			break;

		char *strp = registered_demo_titles[ lid ] ?
			registered_demo_titles[ lid ] : registered_demo_names[ lid ];

		ASSERT( maxcontwidth >= strlen( strp ) );
		int xofs = ( ( maxcontwidth - strlen( strp ) ) * chwidth ) / 2;

		wstrfp( strp, text_x + xofs, text_y, TRTAB_PANELTEXT );
		if ( ( (unsigned int)cur_demolist_selindx == lid ) && listwin_active ) {
			wstrfp( strp, text_x + xofs, text_y, TRTAB_PANELTEXT );
		}

		text_y += chheight;
	}

	// remember for cursor movement and selection
	num_listed_demos = lid;

	// display free flight option
	text_x += ( ( maxcontwidth - strlen( demolist_freeflight_str ) ) * chwidth ) / 2;
	wstrfp( demolist_freeflight_str, text_x, text_y, TRTAB_PANELTEXT );
	if ( ( cur_demolist_selindx == num_listed_demos ) && listwin_active ) {
		wstrfp( demolist_freeflight_str, text_x, text_y, TRTAB_PANELTEXT );
	}
}


// calc and store metrics for demo list window --------------------------------
//
PRIVATE
void CalcDemoListContentMetrics()
{
	// bounds for content
	int maxcontwidth  = strlen( demolist_caption_str ) + LIST_MARGIN - 2;
	int maxcontheight = 20;

	// ensure maximum content width is big enough
	for ( int lid = 0; lid < num_registered_demos; lid++ ) {
		int len = registered_demo_titles[ lid ] ?
			strlen( registered_demo_titles[ lid ] ) :
			strlen( registered_demo_names[ lid ] );
		if ( len > maxcontwidth ) {
			maxcontwidth = len + LIST_MARGIN;
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
	if ( list_slidepos != -1 )
		text_x += m_sintab[ list_slidepos ];
	int text_y = InFloatingMenu ? 62 : 140;

	// store metrics
	demolist_content_metrics.valid		   = TRUE;
	demolist_content_metrics.frame_l	   = frame_l;
	demolist_content_metrics.frame_r	   = frame_r;
	demolist_content_metrics.frame_t	   = frame_t;
	demolist_content_metrics.frame_b	   = frame_b;
	demolist_content_metrics.chwidth	   = chwidth;
	demolist_content_metrics.chheight	   = chheight;
	demolist_content_metrics.text_x		   = text_x;
	demolist_content_metrics.text_y		   = text_y;
	demolist_content_metrics.maxcontwidth  = maxcontwidth;
	demolist_content_metrics.maxcontheight = maxcontheight;
}


// draw translucent window for demo list ---------------------------------------
//
PRIVATE
void DrawDemoListWindow()
{
	// slide the window if necessary
	if ( !SlideListWindow() )
		return;

	CalcDemoListContentMetrics();

	// check whether actual drawing disabled
	if ( AUX_DISABLE_FLOATING_MENU_DRAWING )
		return;

	// determine whether translucency should be used
	int translucent = VID_TRANSLUCENCY_SUPPORTED;

	// write text transparent only for color depths below 32 bit per pixel
	WSFP wstrfp = translucent ? (WSFP)&D_WriteTrString : (WSFP)&D_WriteString;

	// draw frame
	if ( translucent ) {
		DrawDemoListFrame();
	}

	// draw caption
	DrawDemoListCaption( wstrfp );

	// draw items
	DrawDemoListItems( wstrfp );
}


// current metrics of player list window contents -----------------------------
//
static list_window_metrics_s playerlist_content_metrics = { FALSE };


// draw translucent background window for player list window ------------------
//
PRIVATE
void DrawPlayerListFrame()
{
	int frame_l		  = playerlist_content_metrics.frame_l;
	int frame_r		  = playerlist_content_metrics.frame_r;
	int frame_t		  = playerlist_content_metrics.frame_t;
	int frame_b		  = playerlist_content_metrics.frame_b;
	int chwidth		  = playerlist_content_metrics.chwidth;
	int chheight	  = playerlist_content_metrics.chheight;
	int text_x		  = playerlist_content_metrics.text_x;
	int text_y		  = playerlist_content_metrics.text_y;
	int maxcontwidth  = playerlist_content_metrics.maxcontwidth;
	int maxcontheight = playerlist_content_metrics.maxcontheight;

	int backx = text_x - frame_l * chwidth;
	int backy = text_y - frame_t * chheight;
	int backw = ( frame_l + frame_r + maxcontwidth ) * chwidth;
	int backh = ( frame_t + frame_b + maxcontheight ) * chheight;

	DRAW_ClippedTrRect( backx, backy, backw, backh, TRTAB_PANELBACK );
}


// draw caption of player list window -----------------------------------------
//
PRIVATE
void DrawPlayerListCaption( WSFP wstrfp )
{
	unsigned int chwidth		  = playerlist_content_metrics.chwidth;
	unsigned int chheight	  = playerlist_content_metrics.chheight;
	int text_x		  = playerlist_content_metrics.text_x;
	int text_y		  = playerlist_content_metrics.text_y;
	unsigned int maxcontwidth  = playerlist_content_metrics.maxcontwidth;
	
	// draw list caption
	ASSERT( maxcontwidth >= strlen( playerlist_caption ) );
	int xofs = ( ( maxcontwidth - strlen( playerlist_caption ) ) * chwidth ) / 2;
	wstrfp( playerlist_caption, text_x + xofs, text_y, TRTAB_PANELTEXT );
	text_y += chheight * 2;

	// draw "name" and "kills" caption
	wstrfp( playerlist_name_str, text_x, text_y, TRTAB_PANELTEXT );
	xofs = ( maxcontwidth - strlen( playerlist_kills_str ) ) * chwidth;
	wstrfp( playerlist_kills_str, text_x + xofs, text_y, TRTAB_PANELTEXT );
	text_y += chheight;

	// draw separator
	DrawDividerLine( wstrfp, text_x, text_y, maxcontwidth );
	text_y += chheight;

	playerlist_content_metrics.text_y = text_y;
}


// draw all player names and kills --------------------------------------------
//
PRIVATE
void DrawPlayerListItems( WSFP wstrfp )
{
	unsigned int chwidth		  = playerlist_content_metrics.chwidth;
	unsigned int chheight	  = playerlist_content_metrics.chheight;
	int text_x		  = playerlist_content_metrics.text_x;
	int text_y		  = playerlist_content_metrics.text_y;
	unsigned int maxcontwidth  = playerlist_content_metrics.maxcontwidth;
	
	//FIXME: this function is EXACT the same as M_STATUS::DrawStatusWindowItems
	const char *voidstr = "-";

	int	sortfilter[ MAX_NET_ALLOC_SLOTS ];
	int sid = 0;
	for ( sid = 0; sid < MAX_NET_ALLOC_SLOTS; sid++ ) {
		sortfilter[ sid ] = 0;
	}

	// print playerlist (optionally sorted by number of kills)
	for ( int pid = 0; pid < MAX_NET_PROTO_PLAYERS; pid++ ) {

		int slot  = pid;
		int kills = -1;

		// search for highest kills if sorting enabled
		if ( AUX_DISABLE_SORTED_PLAYERLIST ) {
			kills = Player_KillStat[ pid ];
		} else {

			// invalidate slot
			slot = -1;

			// sort for highest kills of connected players not yet displayed
			for ( sid = 0; sid < MAX_NET_PROTO_PLAYERS; sid++ ) {
				if ( !sortfilter[ sid ] && ( Player_KillStat[ sid ] > kills ) && Player_Status[ sid ] ) {
					slot  = sid;
					kills = Player_KillStat[ sid ];
				}
			}

			// check for valid slot
			if ( slot != -1 ) {

				// do not consider slot in next round
				sortfilter[ slot ] = 1;

				// invalidate slot if disconnected and not the local player
				if ( !( ( Player_Status[ slot ] ) || ( slot == LocalPlayerId ) ) )
					slot = -1;
			}
		}

		// get name to be displayed
		const char *strp = (slot != -1) ? Player_Name[ slot ] : voidstr;
		if ( slot == LocalPlayerId )
			strp = LocalPlayerName; // so local name can be changed on the fly

		// write name
		ASSERT( maxcontwidth >= strlen( strp ) );
		int xofs = ( ( maxcontwidth - strlen( strp ) ) * chwidth ) / 2;
		wstrfp( strp, text_x, text_y, TRTAB_PANELTEXT );

		// get kills to be displayed
		char ckills_str[ 5 ] = "";
		if ( slot != -1 ) {
			ASSERT( kills >= 0 );
			sprintf( ckills_str, "%d", kills );
		} else {
			sprintf( ckills_str, "-" );
		}

		// write kill
		xofs = ( maxcontwidth - strlen( ckills_str ) ) * chwidth;
		wstrfp( ckills_str, text_x + xofs, text_y, TRTAB_PANELTEXT );

		// advance to next line
		text_y += chheight;
	}
}


// calc and store metrics for player list window ------------------------------
//
PRIVATE
void CalcPlayerListContentMetrics()
{
	// determine caption
	extern int limit_reached;
	playerlist_caption = limit_reached ?
		playerlist_kill_limit_str : playerlist_caption_str;

	// bounds for content
	unsigned int maxcontwidth  = strlen( playerlist_caption ) + LIST_MARGIN - 2;
	unsigned int maxcontheight = 20;

	// ensure maximum content width is big enough
	for ( int lid = 0; lid < MAX_NET_PROTO_PLAYERS; lid++ ) {
		if ( strlen( Player_Name[ lid ] ) > maxcontwidth ) {
			maxcontwidth = strlen( Player_Name[ lid ] ) + LIST_MARGIN;
		}
	}

	// check local player name
	if ( strlen( LocalPlayerName ) > maxcontwidth ) {
		maxcontwidth = strlen( LocalPlayerName ) + LIST_MARGIN;
	}

	extern int hud_line_dist;

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
	if ( list_slidepos != -1 )
		text_x += m_sintab[ list_slidepos ];
	int text_y = InFloatingMenu ? 62 : (int)( 150 * Scaled_Screen_Height / 480 );

	// store metrics
	playerlist_content_metrics.valid		 = TRUE;
	playerlist_content_metrics.frame_l		 = frame_l;
	playerlist_content_metrics.frame_r		 = frame_r;
	playerlist_content_metrics.frame_t		 = frame_t;
	playerlist_content_metrics.frame_b		 = frame_b;
	playerlist_content_metrics.chwidth		 = chwidth;
	playerlist_content_metrics.chheight		 = chheight;
	playerlist_content_metrics.text_x		 = text_x;
	playerlist_content_metrics.text_y		 = text_y;
	playerlist_content_metrics.maxcontwidth  = maxcontwidth;
	playerlist_content_metrics.maxcontheight = maxcontheight;
}


// draw remote player list -----------------------------------------------------
//
void DrawRemotePlayerListWindow()
{
	// slide the window if necessary
	if ( !SlideListWindow() )
		return;

	CalcPlayerListContentMetrics();

	// check whether actual drawing disabled
	if ( AUX_DISABLE_FLOATING_MENU_DRAWING )
		return;

	// determine whether translucency should be used
	int translucent = VID_TRANSLUCENCY_SUPPORTED;

	// write text transparent only for color depths below 32 bit per pixel
	WSFP wstrfp = translucent ? (WSFP)&D_WriteTrString : (WSFP)&D_WriteString;

	// draw frame
	if ( translucent ) {
		DrawPlayerListFrame();
	}

	// draw caption
	DrawPlayerListCaption( wstrfp );

	// draw items
	DrawPlayerListItems( wstrfp );
}


// current metrics of server list window contents -----------------------------
//
static list_window_metrics_s serverlist_content_metrics = { FALSE };


// draw translucent background window for server list window ------------------
//
PRIVATE
void DrawServerListFrame()
{
	int frame_l		  = serverlist_content_metrics.frame_l;
	int frame_r		  = serverlist_content_metrics.frame_r;
	int frame_t		  = serverlist_content_metrics.frame_t;
	int frame_b		  = serverlist_content_metrics.frame_b;
	unsigned int chwidth	  = serverlist_content_metrics.chwidth;
	unsigned int chheight	  = serverlist_content_metrics.chheight;
	int text_x		  = serverlist_content_metrics.text_x;
	int text_y		  = serverlist_content_metrics.text_y;
	unsigned int maxcontwidth  = serverlist_content_metrics.maxcontwidth;
	unsigned int maxcontheight = serverlist_content_metrics.maxcontheight;

	int backx = text_x - frame_l * chwidth;
	int backy = text_y - frame_t * chheight;
	int backw = ( frame_l + frame_r + maxcontwidth ) * chwidth;
	int backh = ( frame_t + frame_b + maxcontheight ) * chheight;

	DRAW_ClippedTrRect( backx, backy, backw, backh, TRTAB_PANELBACK );
}


// draw caption of server list window -----------------------------------------
//
PRIVATE
void DrawServerListCaption( WSFP wstrfp )
{
	unsigned int chwidth	  = serverlist_content_metrics.chwidth;
	unsigned int chheight	  = serverlist_content_metrics.chheight;
	int text_x		  = serverlist_content_metrics.text_x;
	int text_y		  = serverlist_content_metrics.text_y;
	unsigned int maxcontwidth  = serverlist_content_metrics.maxcontwidth;
	
	// draw caption
	ASSERT( maxcontwidth >= strlen( serverlist_caption_str ) );
	int xofs = ( ( maxcontwidth - strlen( serverlist_caption_str ) ) * chwidth ) / 2;
	wstrfp( serverlist_caption_str, text_x + xofs, text_y, TRTAB_PANELTEXT );
	text_y += chheight * 2;

	// draw separator
	DrawDividerLine( wstrfp, text_x, text_y, maxcontwidth );
	text_y += chheight;

	serverlist_content_metrics.text_y = text_y;
}


// draw all server names and kills --------------------------------------------
//
PRIVATE
void DrawServerListItems( WSFP wstrfp )
{
	unsigned int chwidth	  = serverlist_content_metrics.chwidth;
	unsigned int chheight	  = serverlist_content_metrics.chheight;
	int text_x		  = serverlist_content_metrics.text_x;
	int text_y		  = serverlist_content_metrics.text_y;
	unsigned int maxcontwidth  = serverlist_content_metrics.maxcontwidth;
	
	// print serverlist
	int xofs = ( ( maxcontwidth - strlen( serverlist_peertopeer ) ) * chwidth ) / 2;
	wstrfp( serverlist_peertopeer, text_x + xofs, text_y, TRTAB_PANELTEXT );
	text_y += chheight;

	serverlist_content_metrics.text_y = text_y;
}


// calc and store metrics for server list window ------------------------------
//
PRIVATE
void CalcServerListContentMetrics()
{
	// bounds for content
	unsigned int maxcontwidth  = strlen( serverlist_caption_str ) + LIST_MARGIN - 2;
	unsigned int maxcontheight = 20;
/*
	// ensure maximum content width is big enough
	for ( int lid = 0; lid < num_registered_demos; lid++ ) {
		int len = registered_demo_titles[ lid ] ?
			strlen( registered_demo_titles[ lid ] ) :
			strlen( registered_demo_names[ lid ] );
		if ( len > maxcontwidth ) {
			maxcontwidth = len + LIST_MARGIN;
		}
	}
*/
	extern int hud_line_dist;

	// font metrics
	unsigned int chwidth  = CharsetInfo[ HUD_CHARSETNO ].width;
	unsigned int chheight = hud_line_dist;

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
	if ( list_slidepos != -1 )
		text_x += m_sintab[ list_slidepos ];
	int text_y = InFloatingMenu ? 62 : 140;

	// store metrics
	serverlist_content_metrics.valid		 = TRUE;
	serverlist_content_metrics.frame_l		 = frame_l;
	serverlist_content_metrics.frame_r		 = frame_r;
	serverlist_content_metrics.frame_t		 = frame_t;
	serverlist_content_metrics.frame_b		 = frame_b;
	serverlist_content_metrics.chwidth		 = chwidth;
	serverlist_content_metrics.chheight		 = chheight;
	serverlist_content_metrics.text_x		 = text_x;
	serverlist_content_metrics.text_y		 = text_y;
	serverlist_content_metrics.maxcontwidth	 = maxcontwidth;
	serverlist_content_metrics.maxcontheight = maxcontheight;
}


// draw server list -----------------------------------------------------------
//
void DrawServerListWindow()
{
	// slide the window if necessary
	if ( !SlideListWindow() )
		return;

	CalcServerListContentMetrics();

	// check whether actual drawing disabled
	if ( AUX_DISABLE_FLOATING_MENU_DRAWING )
		return;

	// determine whether translucency should be used
	int translucent = VID_TRANSLUCENCY_SUPPORTED;

	// write text transparent only for color depths below 32 bit per pixel
	WSFP wstrfp = translucent ? (WSFP)&D_WriteTrString : (WSFP)&D_WriteString;

	// draw frame
	if ( translucent ) {
		DrawServerListFrame();
	}

	// draw caption
	DrawServerListCaption( wstrfp );

	// draw items
	DrawServerListItems( wstrfp );
}


// draw translucent list window in menu (depending on current mode) -----------
//
void DrawMenuListWindow()
{
	switch ( listwin_mode ) {

		case LISTWIN_DEMOLIST:
			DrawDemoListWindow();
			break;

		case LISTWIN_PLAYERLIST:
			DrawRemotePlayerListWindow();
			break;

		case LISTWIN_SERVERLIST:
			DrawServerListWindow();
			break;
	}
}



