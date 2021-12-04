/*
 * PARSEC - Game Status Window
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:38 $
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
#include "net_defs.h"
#include "sys_defs.h"

// drawing subsystem
#include "d_font.h"

// local module header
#include "m_status.h"

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
static char status_name_str[]			= "name";
static char status_kills_str[]			= "kills";
static char status_caption_str[]		= "         game status        ";
static char status_kill_limit_str[]		= "kill limit has been reached!";
static char status_time_limit_str[]		= "   the game time is over!   ";


// status window configuration and state variables ----------------------------
//
#define STATUS_ALPHA_LOW		0
#define STATUS_ALPHA_HIGH		255

#define STATUS_FADE_SPEED		12
#define STATUS_FADE_QUANTUM		10

#define STATUS_MARGIN	25

static int			status_fadepos 		= STATUS_ALPHA_LOW;
static int			status_fadetarget	= STATUS_ALPHA_LOW;
static refframe_t	status_lastref 		= REFFRAME_INVALID;


// fade the background alpha to a specified target ----------------------------
//
PRIVATE
int DoStatusWindowFading()
{
	if ( status_fadepos == status_fadetarget ) {

		status_lastref = REFFRAME_INVALID;

		// skip drawing code if status window is completely faded out
#if ( STATUS_ALPHA_LOW == 0 )
		return ( status_fadepos != STATUS_ALPHA_LOW );
#else
		return TRUE;
#endif

	}

	if ( status_fadepos < status_fadetarget ) {

		// fade in
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( status_lastref == REFFRAME_INVALID ) {
			status_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - status_lastref;
			for ( ; delta >= STATUS_FADE_SPEED; delta -= STATUS_FADE_SPEED ) {
				status_fadepos += STATUS_FADE_QUANTUM;
				if ( status_fadepos >= status_fadetarget ) {
					status_fadepos = status_fadetarget;
					status_lastref = REFFRAME_INVALID;
					break;
				}
				status_lastref += STATUS_FADE_SPEED;
			}
		}

	} else {

		// fade out
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( status_lastref == REFFRAME_INVALID ) {
			status_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - status_lastref;
			for ( ; delta >= STATUS_FADE_SPEED; delta -= STATUS_FADE_SPEED ) {
				status_fadepos -= STATUS_FADE_QUANTUM;
				if ( status_fadepos <= status_fadetarget ) {
					status_fadepos = status_fadetarget;
					status_lastref = REFFRAME_INVALID;
					break;
				}
				status_lastref += STATUS_FADE_SPEED;
			}
		}
	}

	return TRUE;
}


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


// pointer to current caption of status window --------------------------------
//
static char *statuswindow_caption;


// current metrics of status window contents ----------------------------------
//
static list_window_metrics_s statuswindow_content_metrics = { FALSE };


// draw translucent background window for status window -----------------------
//
PRIVATE
void DrawStatusWindowFrame()
{
	int frame_l		  = statuswindow_content_metrics.frame_l;
	int frame_r		  = statuswindow_content_metrics.frame_r;
	int frame_t		  = statuswindow_content_metrics.frame_t;
	int frame_b		  = statuswindow_content_metrics.frame_b;
	int chwidth		  = statuswindow_content_metrics.chwidth;
	int chheight	  = statuswindow_content_metrics.chheight;
	int text_x		  = statuswindow_content_metrics.text_x;
	int text_y		  = statuswindow_content_metrics.text_y;
	int maxcontwidth  = statuswindow_content_metrics.maxcontwidth;
	int maxcontheight = statuswindow_content_metrics.maxcontheight;

	int backx = text_x - frame_l * chwidth;
	int backy = text_y - frame_t * chheight;
	int backw = ( frame_l + frame_r + maxcontwidth ) * chwidth;
	int backh = ( frame_t + frame_b + maxcontheight ) * chheight;

	DRAW_ClippedTrRect( backx, backy, backw, backh, TRTAB_PANELBACK );
}


// draw caption of status window ----------------------------------------------
//
PRIVATE
void DrawStatusWindowCaption( WSFP wstrfp )
{
	unsigned int chwidth		  = statuswindow_content_metrics.chwidth;
	unsigned int chheight	  = statuswindow_content_metrics.chheight;
	int text_x		  = statuswindow_content_metrics.text_x;
	int text_y		  = statuswindow_content_metrics.text_y;
	unsigned int maxcontwidth  = statuswindow_content_metrics.maxcontwidth;

	// draw list caption
	ASSERT( maxcontwidth >= strlen( statuswindow_caption ) );
	int xofs = ( ( maxcontwidth - strlen( statuswindow_caption ) ) * chwidth ) / 2;
	wstrfp( statuswindow_caption, text_x + xofs, text_y, TRTAB_PANELTEXT );
	text_y += chheight * 2;

	// draw "name" and "kills" caption
	wstrfp( status_name_str, text_x, text_y, TRTAB_PANELTEXT );
	xofs = ( maxcontwidth - strlen( status_kills_str ) ) * chwidth;
	wstrfp( status_kills_str, text_x + xofs, text_y, TRTAB_PANELTEXT );
	text_y += chheight;

	// draw separator
	DrawDividerLine( wstrfp, text_x, text_y, maxcontwidth );
	text_y += chheight;

	statuswindow_content_metrics.text_y = text_y;
}


// draw all player names and kills --------------------------------------------
//
PRIVATE
void DrawStatusWindowItems( WSFP wstrfp )
{
	unsigned int chwidth		  = statuswindow_content_metrics.chwidth;
	unsigned int chheight	  = statuswindow_content_metrics.chheight;
	int text_x		  = statuswindow_content_metrics.text_x;
	int text_y		  = statuswindow_content_metrics.text_y;
	unsigned int maxcontwidth  = statuswindow_content_metrics.maxcontwidth;

	//FIXME: this function is EXACT the same as M_LIST::DrawPlayerListItems
	const char *voidstr = "-";

	int	sortfilter[ MAX_NET_ALLOC_SLOTS ];
	int sid = 0;
	for ( sid = 0; sid < MAX_NET_ALLOC_SLOTS; sid++ ) {
		sortfilter[ sid ] = 0;
	}

	// print playerlist (optionally sorted by number of kills)
	int pid = 0;
	for ( pid = 0; pid < MAX_NET_PROTO_PLAYERS; pid++ ) {

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
		int xofs;
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


// calc and store metrics for status window -----------------------------------
//
PRIVATE
void CalcStatusWindowContentMetrics()
{
	// determine caption
	statuswindow_caption = status_caption_str;

	if ( GAME_NO_SERVER() ) {

		extern int limit_reached;
		if ( limit_reached ) {
			statuswindow_caption = status_kill_limit_str;
		}

	} else {

		if ( CurGameTime == GAME_FINISHED_KILLS ) {
			statuswindow_caption = status_kill_limit_str;
		} else if ( CurGameTime == GAME_FINISHED_TIME ) {
			statuswindow_caption = status_time_limit_str;
		}
	}

	// bounds for content
	size_t maxcontwidth  = strlen( statuswindow_caption ) + STATUS_MARGIN - 2;
	unsigned int maxcontheight = 20;

	// ensure maximum content width is big enough
	for ( int lid = 0; lid < MAX_NET_PROTO_PLAYERS; lid++ )
		if ( strlen( Player_Name[ lid ] ) > maxcontwidth )
			maxcontwidth  = strlen( Player_Name[ lid ] ) + STATUS_MARGIN;

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
	int text_x = ( Screen_Width - ( frame_r + maxcontwidth ) * chwidth ) / 2;
	int text_y = InFloatingMenu ? 62 : 140;

	// store metrics
	statuswindow_content_metrics.valid			= TRUE;
	statuswindow_content_metrics.frame_l		= frame_l;
	statuswindow_content_metrics.frame_r		= frame_r;
	statuswindow_content_metrics.frame_t		= frame_t;
	statuswindow_content_metrics.frame_b		= frame_b;
	statuswindow_content_metrics.chwidth		= chwidth;
	statuswindow_content_metrics.chheight		= chheight;
	statuswindow_content_metrics.text_x			= text_x;
	statuswindow_content_metrics.text_y			= text_y;
	statuswindow_content_metrics.maxcontwidth	= maxcontwidth;
	statuswindow_content_metrics.maxcontheight	= maxcontheight;
}


// draw translucent window with game status displays ---------------------------
//
void DrawStatusWindow()
{
	// fade the window if necessary
	if ( !DoStatusWindowFading() )
		return;

	CalcStatusWindowContentMetrics();

	// check whether actual drawing disabled
	if ( AUX_DISABLE_FLOATING_MENU_DRAWING )
		return;

	// used to branch on VID_TRANSLUCENCY_SUPPORTED
	WSFP wstrfp = D_WriteTrString;

	// fade window alpha
	int old_text_alpha = PanelTextColor.A;
	int old_back_alpha = PanelBackColor.A;
	PanelTextColor.A = (int)(status_fadepos * ( (float) old_text_alpha / (float) STATUS_ALPHA_HIGH ));
	PanelBackColor.A = (int)(status_fadepos * ( (float) old_back_alpha / (float) STATUS_ALPHA_HIGH ));

	// draw frame
	DrawStatusWindowFrame();

	// draw caption
	DrawStatusWindowCaption( wstrfp );

	// draw items
	DrawStatusWindowItems( wstrfp );

	// restore window alpha
	PanelTextColor.A = old_text_alpha;
	PanelBackColor.A = old_back_alpha;
}


// check if status window is at its desired fade position ---------------------
//
int FadeFinishedStatusWindow()
{
	return ( status_fadepos == status_fadetarget );
}


// set fade target for status window to high (in) -----------------------------
//
void FadeInStatusWindow()
{
	status_fadetarget = STATUS_ALPHA_HIGH;
}


// set fade target for status window to low (out) -----------------------------
//
void FadeOutStatusWindow()
{
	status_fadetarget = STATUS_ALPHA_LOW;
}


// set fade level of status window to invisible (used upon join game) ---------
//
void HideStatusWindow()
{
	status_fadepos = STATUS_ALPHA_LOW;
}



