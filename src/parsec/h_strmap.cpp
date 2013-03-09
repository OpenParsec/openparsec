/*
 * PARSEC - Star Map
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:37 $
 *
 * Orginally written by:
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1999-2002
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
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
#include "od_class.h"

// global externals
#include "globals.h"

// subsystem headers
#include "inp_defs.h"
#include "net_defs.h"
#include "net_game.h"
#include "net_serv.h"
#include "sys_defs.h"

// drawing subsystem
#include "d_misc.h"
#include "d_font.h"
#include "d_iter.h"

// rendering subsystem
#include "r_obj.h"

// subsystem linkage info
#include "linkinfo.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "h_strmap.h"

// proprietary module headers
#include "e_callbk.h"
#include "e_color.h"
#include "e_draw.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_main.h"
#include "e_supp.h"
#include "m_main.h"
#include "net_csdf.h"
#include "net_univ.h"
#include "utl_model.h"


#define CLIENT_MAP




// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];

#define CIRCLE_POINTS 96

#define FONTS	0
#define FONTT	1

#define FONTSW	8
#define FONTSH	11
#define FONTTW	8
#define FONTTH	11

#define INSET 16
#define STAR_RADIUS 5

#define MAP_POS_INFINITY	32768
#define MAP_NEG_INFINITY	-32768

#define NUM_GREYS	8

#define INFO_BOX_X			0.015 
#define INFO_BOX_Y			0.715
#define INFO_BOX_W			0.715
#define INFO_BOX_H			0.266
#define INFO_BOX_TEXT_OFFX	INFO_BOX_X + 0.01
#define INFO_BOX_TEXT_OFFY	INFO_BOX_Y + 0.01
#define INFO_BOX_TEXT_H		CharsetInfo[ HUD_CHARSETNO ].height

PRIVATE int greyramp[ NUM_GREYS ];

PRIVATE int map_width;
PRIVATE int map_height;
PRIVATE int map_hwidth;
PRIVATE int map_hheight;
PRIVATE int map_xoffs;

PRIVATE int map_normalize_done;

PRIVATE float map_scale = 16.0f;

PRIVATE float map_pan_x = 0.0f;
PRIVATE float map_pan_y = 0.0f;


#define SCALE_X_COORD( x ) ( (int) ( ( map_width / 2 ) + x * map_scale ) )
#define SCALE_Y_COORD( y ) ( (int) ( ( map_width / 2 ) - y * map_scale ) )


enum {

	black = 0,
	white,
	green,
	darkgreen,
	lightblue,

	NUM_COLORS
};


PRIVATE colrgba_s map_colors[ NUM_COLORS + NUM_GREYS ] = {
	{ 0x00, 0x00, 0x00, 0xff },
	{ 0xff, 0xff, 0xff, 0xff },
	{ 0x00, 0xa3, 0x0b, 0xff },
	{ 0x00, 0x20, 0x00, 0xff },
	{ 0x66, 0x66, 0x99, 0xff },
};


// starmap background textore info --------------------------------------------
//
static starmap_bg_info_s starmap_bg_info[] = {

	{   0.0   , 0.266, 0.40 , 0.133, 256, 64,  NULL, "infobar11" },		// offset from left and bottom
	{   0.4   , 0.266, 0.40 , 0.133, 256, 64,  NULL, "infobar12" },		// offset from left and bottom
	{   0.0   , 0.080, 0.40 , 0.133, 256, 64,  NULL, "infobar21" },		// offset from left and bottom
	{   0.4   , 0.080, 0.40 , 0.133, 256, 64,  NULL, "infobar22" },		// offset from left and bottom
	{   0.2547,   0.0, 0.20 , 0.533, 128, 256,  NULL, "navbar11" },		// offset from right and top
	{   0.0547,   0.0, 0.10 , 0.533,  64, 256,  NULL, "navbar12" },		// offset from right and top
	{   0.2547, 0.533, 0.20 , 0.533, 128, 256,  NULL, "navbar21" },		// offset from right and top
	{   0.0547, 0.533, 0.10 , 0.533,  64, 256,  NULL, "navbar22" },		// offset from right and top
	{   0.2359, 0.304, 0.05 , 0.066,  32,  32,  NULL, "hilite"   },		// offset from right and top
	{   0.0   , 0.0  , 0.025, 0.033,  16,  16,  NULL, "symbol"   },

	{ 0, 0, 0, 0, 0, 0, NULL, NULL }
};

const char *inactive_button_textures [12] = {
	"mn_zin_i",
	"mn_up_i",
	"mn_blank_i",
	"mn_left_i",
	"mn_reset_i",
	"mn_right_i",
	"mn_zout_i",
	"mn_down_i",
	"mn_blank_i",
	"mn_connect_i",
	"mn_refresh_i",
	"mn_exit_i"
};
const char *active_button_textures [12] = {
	"mn_zin_a",
	"mn_up_a",
	"mn_blank_i",
	"mn_left_a",
	"mn_reset_a",
	"mn_right_a",
	"mn_zout_a",
	"mn_down_a",
	"mn_blank_a",
	"mn_connect_a",
	"mn_refresh_a",
	"mn_exit_a"
};

enum {
	MAP_INFOBAR11,
	MAP_INFOBAR12,
	MAP_INFOBAR21,
	MAP_INFOBAR22,
	MAP_NAVBAR11,
	MAP_NAVBAR12,
	MAP_NAVBAR21,
	MAP_NAVBAR22,
	MAP_HILITE,
	MAP_SYMBOL
};

static int starmap_bg_valid = FALSE;

#define NUM_SELECT_BUTTONS	3

static const char *select_button_text[ NUM_SELECT_BUTTONS ] = {
	"connect",
	"refresh",
	"exit",


};
/*
	"info",
	"list",

};*/

#define INFO_BOX_BUFFER (x) snprintf(tmp_buffer, 256, info_box_strings[i], x)
#define NUM_INFO_BOX_LINES	5
static const char *info_box_strings [ NUM_INFO_BOX_LINES ] ={
	"Server Name: %s",
	"Server Version: %d.%d",
	"Ping Time: %d ms",
	"Players %d of %d maximum",
	NULL
};



enum {
	MAPSEL_CONNECT,
	MAPSEL_REFRESH,
	MAPSEL_EXIT,
	MAPSEL_INFO,
	MAPSEL_LIST
};

static float sel_leftedge   	= 488.0f / 640.0f;
static float sel_rightedge  	= 608.0f / 640.0f;
static float sel_topedge    	= 186.0f / 480.0f;
static float sel_buttonoffs 	=  16.0f / 480.0f;
static float sel_buttonheight	=  64.0f / 480.0f;


#define NUM_NAV_BUTTONS 9


enum {
	MAPNAV_ZOOMIN,
	MAPNAV_PANUP,
	MAPNAV_NULL,
	MAPNAV_PANLEFT,
	MAPNAV_RESET,
	MAPNAV_PANRIGHT,
	MAPNAV_ZOOMOUT,
	MAPNAV_PANDOWN,
	MAPNAV_NULL2
};

static float nav_leftedge 	= 496.0f / 640.0f;
static float nav_topedge  	=  16.0f / 480.0f;
static float nav_buttonwidth  =  64.0f / 640.0f;
static float nav_buttonheight =  64.0f / 480.0f;

static float infopanel_height = 128.0f / 480.0f;
static float navpanel_leftx   = 480.0f / 640.0f;

static int infopanel_dragging  	= FALSE;
static int infopanel_dragorigin	= 0;


// mouse vars -----------------------------------------------------------------
//
static float	cur_mouse_x;
static float	cur_mouse_y;
static float	old_mouse_x			= -1;
static float	old_mouse_y			= -1;

extern int	cur_mouse_button_state;
extern int	old_mouse_button_state;


// current mouse position state -----------------------------------------------
//
static int mouse_position_state = MOUSE_OVER_NOTHING;

static int map_sel_hilite = -1;
static int map_sel_pressed = -1;

static int map_nav_hilite = -1;
static int map_srv_hilite = -1;
static int map_srv_pressed = -1; 

static int map_dragging = FALSE;
static int map_dragorigin_x = 0;
static int map_dragorigin_y = 0;

static int map_animate_in = 255; 
static int map_animate_out = 255;
#define MAP_OVERLAY_ANI_SPEED	40

// fading control variables ---------------------------------------------------
//
#define MAP_ALPHA_LOW		25
#define MAP_ALPHA_HIGH		255

#define MAP_FADE_SPEED		100
#define MAP_FADE_QUANTUM	30

static int			map_fadepos 	= MAP_ALPHA_LOW;
static int			map_fadetarget	= MAP_ALPHA_HIGH;
static refframe_t	map_lastref 	= REFFRAME_INVALID;


// fade starmap in (called after the console has been closed) -----------------
//
void MAP_FadeInStarmap()
{
	map_fadetarget = MAP_ALPHA_HIGH;
}


// fade starmap out (called when the console is opened) -----------------------
//
void MAP_FadeOutStarmap()
{
	map_fadetarget = MAP_ALPHA_LOW;
}


// fade the starmap alpha to a specified target -------------------------------
//
INLINE
void MAP_FadeStarmapAlpha()
{
	if ( map_fadepos == map_fadetarget ) {
		map_lastref = REFFRAME_INVALID;
		return;
	}

	if ( map_fadepos < map_fadetarget ) {

		// fade in
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( map_lastref == REFFRAME_INVALID ) {
			map_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - map_lastref;
			for ( ; delta >= MAP_FADE_SPEED; delta -= MAP_FADE_SPEED ) {
				map_fadepos += MAP_FADE_QUANTUM;
				if ( map_fadepos >= map_fadetarget ) {
					map_fadepos = map_fadetarget;
					map_lastref = REFFRAME_INVALID;
					break;
				}
				map_lastref += MAP_FADE_SPEED;
			}
		}

	} else {

		// fade out
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( map_lastref == REFFRAME_INVALID ) {
			map_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - map_lastref;
			for ( ; delta >= MAP_FADE_SPEED; delta -= MAP_FADE_SPEED ) {
				map_fadepos -= MAP_FADE_QUANTUM;
				if ( map_fadepos <= map_fadetarget ) {
					map_fadepos = map_fadetarget;
					map_lastref = REFFRAME_INVALID;
					break;
				}
				map_lastref += MAP_FADE_SPEED;
			}
		}
	}
}


// try to acquire textures for logo animation ---------------------------------
//
PRIVATE
int AcquireStarmapTextures()
{
	if ( starmap_bg_valid )
		return TRUE;

	starmap_bg_valid = TRUE;

	for ( int tid = 0; starmap_bg_info[ tid ].texname; tid++ ) {

		TextureMap *texmap = FetchTextureMap( starmap_bg_info[ tid ].texname );
		starmap_bg_info[ tid ].texmap = texmap;

		if ( texmap == NULL ) {
			starmap_bg_valid = FALSE;
			MSGOUT( "starmap texture %s missing", starmap_bg_info[ tid ].texname );
		}
	}

	return starmap_bg_valid;
}


// ----------------------------------------------------------------------------
//
PRIVATE
void MAP_FetchClipRectangle( Rectangle2 *cliprect )
{
	ASSERT( cliprect != NULL );

	cliprect->left   = 0;
//	cliprect->right  = Screen_Width - starmap_bg_info[ MAP_NAVBAR11 ].xpos * Screen_Width;
	cliprect->right  = Screen_Width;
	cliprect->top    = 0;
//	cliprect->bottom = Screen_Height - starmap_bg_info[ MAP_INFOBAR11 ].ypos * Screen_Height;
	cliprect->bottom = Screen_Height;
	
}


// ----------------------------------------------------------------------------
//
void gdImageLine( void *dummy, int x1, int y1, int x2, int y2, int col )
{
	IterLine2 itline;
	itline.NumVerts  = 2;

	if ( x1 == x2 || y1 == y2 ) {
		itline.flags	 = ITERFLAG_LS_DEFAULT;
	} else {
		itline.flags	 = ITERFLAG_LS_ANTIALIASED;
	}

	itline.itertype  = iter_rgba | iter_alphablend;
	//itline.itertype  = iter_rgba;
	itline.raststate = rast_default;
	itline.rastmask  = rast_nomask;

	itline.Vtxs[ 0 ].X 	   = INT_TO_RASTV( map_xoffs + x1 + map_pan_x );
	itline.Vtxs[ 0 ].Y 	   = INT_TO_RASTV( y1 + map_pan_y );
	itline.Vtxs[ 0 ].Z 	   = RASTV_0;
	itline.Vtxs[ 0 ].R 	   = map_colors[ col ].R;
	itline.Vtxs[ 0 ].G 	   = map_colors[ col ].G;
	itline.Vtxs[ 0 ].B 	   = map_colors[ col ].B;
	itline.Vtxs[ 0 ].A 	   = map_fadepos;
	itline.Vtxs[ 0 ].flags = ITERVTXFLAG_NONE;

	itline.Vtxs[ 1 ].X 	   = INT_TO_RASTV( map_xoffs + x2 + map_pan_x );
	itline.Vtxs[ 1 ].Y 	   = INT_TO_RASTV( y2 + map_pan_y );
	itline.Vtxs[ 1 ].Z 	   = RASTV_0;
	itline.Vtxs[ 1 ].R 	   = map_colors[ col ].R;
	itline.Vtxs[ 1 ].G 	   = map_colors[ col ].G;
	itline.Vtxs[ 1 ].B 	   = map_colors[ col ].B;
	itline.Vtxs[ 1 ].A 	   = map_fadepos;
	itline.Vtxs[ 1 ].flags = ITERVTXFLAG_NONE;

	Rectangle2 cliprect;
	MAP_FetchClipRectangle( &cliprect );
	IterLine2 *clipline = CLIP_RectangleIterLine2( &itline, &cliprect );
	if ( clipline != NULL ) {
		D_DrawIterLine2( clipline );
	}
}


// ----------------------------------------------------------------------------
//
void gdImageArc( void *dummy, int cx, int cy, int w, int h, int s, int e, int col )
{
	ASSERT( s == 0 );
	ASSERT( e == 360 );

	IterLine2 *itline = (IterLine2 *) ALLOCMEM( (size_t)&((IterLine2*)0)->Vtxs[ CIRCLE_POINTS + 1 ] );
	if ( itline == NULL )
		OUTOFMEM( 0 );

	itline->NumVerts  = CIRCLE_POINTS + 1;
	itline->flags	  = ITERFLAG_LS_ANTIALIASED;
	itline->itertype  = iter_rgba | iter_alphablend;
	itline->raststate = rast_default;
	itline->rastmask  = rast_nomask;
	int cid = 0;
	for ( cid = 0; cid < CIRCLE_POINTS; cid++ ) {

		bams_t angle = cid * ( BAMS_DEG360 / CIRCLE_POINTS );

		sincosval_s sincosv;
		GetSinCos( angle, &sincosv );
		float	sinus   = GEOMV_TO_FLOAT( sincosv.sinval );
		float	cosinus = GEOMV_TO_FLOAT( sincosv.cosval );

		Vertex3	c_point;

		// draw circle in xy plane
		float x = sinus * w / 2;
		float y = cosinus * h / 2;

		itline->Vtxs[ cid ].X 	   = FLOAT2INT( map_xoffs + cx + x );
		itline->Vtxs[ cid ].Y 	   = FLOAT2INT( cy + y );
		itline->Vtxs[ cid ].R 	   = map_colors[ col ].R;
		itline->Vtxs[ cid ].G 	   = map_colors[ col ].G;
		itline->Vtxs[ cid ].B 	   = map_colors[ col ].B;
		itline->Vtxs[ cid ].A 	   = map_fadepos;
		itline->Vtxs[ cid ].flags = ITERVTXFLAG_NONE;
	}

	// duplicate first vertex
	itline->Vtxs[ cid ].X 	   = itline->Vtxs[ 0 ].X;
	itline->Vtxs[ cid ].Y 	   = itline->Vtxs[ 0 ].Y;
	itline->Vtxs[ cid ].R 	   = itline->Vtxs[ 0 ].R;
	itline->Vtxs[ cid ].G 	   = itline->Vtxs[ 0 ].G;
	itline->Vtxs[ cid ].B 	   = itline->Vtxs[ 0 ].B;
	itline->Vtxs[ cid ].A 	   = itline->Vtxs[ 0 ].A;
	itline->Vtxs[ cid ].flags  = ITERVTXFLAG_NONE;

	Rectangle2 cliprect;
	MAP_FetchClipRectangle( &cliprect );

	IterLine2 *clipline1 = CLIP_RectangleIterLine2( itline, &cliprect );
	if ( clipline1 != NULL )
		D_DrawIterLine2( clipline1 );

	FREEMEM( itline );
}


// ----------------------------------------------------------------------------
//
int gdImageString( void *dummy, int font, int x, int y, char *s, int col )
{
	ASSERT ( s != NULL );

	if ( col == lightblue ) {
		return 0;
	}

	if ( map_fadepos < MAP_ALPHA_HIGH ) {
		return 0;
	}

	texfont_s *texfont = FetchTexfont( "impact" );
	if ( texfont == NULL ) {
		return 0;
	}

	IterTexfont itexfont;
	itexfont.itertype  = iter_texrgba | iter_alphablend;
	itexfont.raststate = rast_texclamp;
	itexfont.rastmask  = rast_nomask;
	itexfont.texfont   = texfont;

	itexfont.Vtxs[ 0 ].U = 0.4f;
	itexfont.Vtxs[ 0 ].V = 0.4f;
	itexfont.Vtxs[ 0 ].R = 255;
	itexfont.Vtxs[ 0 ].G = 255;
	itexfont.Vtxs[ 0 ].B = 255;
	itexfont.Vtxs[ 0 ].A = 180;

	int xpos = (int)(map_xoffs + x + map_pan_x);
	int ypos = (int)(y + map_pan_y);

	itexfont.Vtxs[ 0 ].X = xpos;
	itexfont.Vtxs[ 0 ].Y = ypos;

	Rectangle2 cliprect;
	MAP_FetchClipRectangle( &cliprect );

	if ( xpos >= cliprect.left && xpos < cliprect.right &&
		 ypos >= cliprect.top  && ypos < cliprect.bottom ) {
		D_TexfontWrite( s, &itexfont );
	}

	int width = D_TexfontGetWidth( s, &itexfont );

	return width;
}

#if 0
// ----------------------------------------------------------------------------
//
void gdImageString( void *dummy, int font, int x, int y, char *s, int col )
{
	ASSERT ( s != NULL );

	if ( col == lightblue )
		return;

	if ( map_fadepos < MAP_ALPHA_HIGH )
		return;

	colrgba_s fontcol = { 220, 220, 220, map_fadepos };
	colrgba_s oldcol = PanelTextColor;

	/*
	 D_SetWStrContext( CharsetInfo[ MIN_CHARSETNO ].charsetpointer,
					CharsetInfo[ MIN_CHARSETNO ].geompointer,
					NULL,
					CharsetInfo[ MIN_CHARSETNO ].width,
					CharsetInfo[ MIN_CHARSETNO ].height );
	 */

	D_SetWStrContext( CharsetInfo[ HUD_CHARSETNO ].charsetpointer,
				   CharsetInfo[ HUD_CHARSETNO ].geompointer,
				   NULL,
				   CharsetInfo[ HUD_CHARSETNO ].width,
				   CharsetInfo[ HUD_CHARSETNO ].height );

	int xpos = map_xoffs + x + map_pan_x;
	int ypos = y + map_pan_y;

	Rectangle2 cliprect;
	MAP_FetchClipRectangle( &cliprect );

	PanelTextColor = fontcol;

	if ( xpos >= cliprect.left && xpos < cliprect.right &&
	  ypos >= cliprect.top  && ypos < cliprect.bottom ) {
		D_WriteTrString( s, xpos, ypos, TRTAB_PANELTEXT );
		//		D_WriteString( s, xpos, ypos );
	}

	PanelTextColor = oldcol;
}
#endif

// ----------------------------------------------------------------------------
//
void gdImageFillToBorder( void *dummy, int x, int y, int bcol, int col )
{
	// not supported yet
}


// ----------------------------------------------------------------------------
//
void gdImageCopyResized( void *image, TextureMap *obj, int dstx, int dsty, int srcx, int srcy,
						 int dstw, int dsth, int srcw, int srch )
{
	if ( obj == NULL )
		return;

	texscreenrect_s	rect;

	dword itertype = iter_texrgba | iter_specularadd;
	rect.alpha     = map_fadepos;

	rect.x = (int)(dstx + map_xoffs + map_pan_x);
	rect.y = (int)(dsty + map_pan_y);
	rect.w = srcw;
	rect.h = srch;
	rect.scaled_w = dstw;
	rect.scaled_h = dsth;
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.itertype = itertype;
	rect.texmap   = obj;

	Rectangle2 cliprect;
	MAP_FetchClipRectangle( &cliprect );

	if ( ( rect.x + dstw ) >= cliprect.left && rect.x < cliprect.right &&
		 ( rect.y + dsth ) >= cliprect.top  && rect.y < cliprect.bottom ) {
		DRAW_TexturedScreenRect( &rect, &cliprect );
	}

}


// normalize all star/object coordinates --------------------------------------
//
void MAP_NormalizeCoordinates()
{
	if ( num_servers_joined == 0 ) {
		// default map_scale 
		map_scale = 64.0f;

		return;
	}

	if ( map_normalize_done ) {
		return;
	}

	int maxx = 0;
	int maxy = 0;
	
	for ( int sid = 0; sid < MAX_SERVERS; sid++ ) {

		if ( !server_list[ sid ].slot_free ) {
	
			if ( abs( server_list[ sid ].xpos ) > maxx ) {
				maxx = abs( server_list[ sid ].xpos );
			}

			if ( abs( server_list[ sid ].ypos ) > maxy ) {
				maxy = abs( server_list[ sid ].ypos );
			}
		}
	}

	if ( maxx > maxy ) {
		if ( maxx != 0.0f ) {
			map_scale = ( (float) sqrt( (float)map_hheight*map_hheight/2 ) - 10 ) / maxx;
		}
	} else {
		if ( maxy != 0.0f ) {
			map_scale = ( (float) sqrt( (float)map_hheight*map_hheight/2 ) - 10 ) / maxy;
		}
	}
		
	ASSERT( map_scale != 0.0f );

	map_normalize_done = TRUE;
}


// draw starmap background objects (nebulae, etc...) --------------------------
//
PRIVATE
void MAP_DrawMapObjects( void *image )
{
	ASSERT( image != NULL );

	if ( num_map_objs == 0 )
		return;

	for ( int oid = 0; oid < num_map_objs; oid++ ) {
	
		mapobj_s *tmp = map_objs[ oid ];
		
		if ( tmp == NULL )
			continue;
	

#ifdef CLIENT_MAP

		if ( tmp->texmap == NULL ) {
			tmp->texmap = FetchTextureMap( tmp->texname );
		}	

		TextureMap *obj = tmp->texmap;
		
#else

		gdImagePtr obj = NULL;	

		if ( tmp->imgdata == NULL ) {
		
			FILE *fp = NULL;

			fp = fopen( tmp->filename, "rb" );
			if ( fp == NULL ) {
				continue;
			}
			
			obj = gdImageCreateFromPng( fp );
			if ( obj == NULL ) {
				continue;
			}
			
			// make black transparent

			int blackidx = gdImageColorExact( obj, 0, 0, 0 );

			if ( blackidx != -1 ) {
				gdImageColorTransparent( obj, blackidx );
			}

			fclose( fp );

			tmp->imgdata = (void *) obj;
		}

		obj = (gdImagePtr) tmp->imgdata;

#endif

		int xpos = SCALE_X_COORD( tmp->xpos );
		int ypos = SCALE_Y_COORD( tmp->ypos );

		int w = (int) ( map_scale * tmp->w );
		int h = (int) ( map_scale * tmp->h );
	
		gdImageCopyResized( image, obj, xpos, ypos, 0, 0, w, h, 256, 256 );	
	
		// draw corners and name
		/*
		gdImageLine( image, xpos, ypos, xpos + 4, ypos, green );
		gdImageLine( image, xpos, ypos, xpos, ypos + 4, green );
		gdImageLine( image, xpos + w, ypos, xpos + w - 4, ypos, green );
		gdImageLine( image, xpos + w, ypos, xpos + w, ypos + 4, green );
		gdImageLine( image, xpos, ypos + h, xpos + 4, ypos + h, green );
		gdImageLine( image, xpos, ypos + h, xpos, ypos + h - 4, green );
		gdImageLine( image, xpos + w, ypos + h, xpos + w - 4, ypos + h, green );
		gdImageLine( image, xpos + w, ypos + h, xpos + w, ypos + h - 4, green );

		gdImageString( image, FONTT, xpos, ypos + h + 2, tmp->name, green );
		*/

		/*
		gdImageString( image, FONTT, xpos + w / 2 + 1, ypos + h / 2 + 1, tmp->name, lightblue );
		gdImageString( image, FONTT, xpos + w / 2, ypos + h / 2, tmp->name, greyramp[ 6 ] );
		*/
	}
}


// draw starmap background objects (nebulae, etc...) --------------------------
//
PRIVATE
void MAP_DrawMapAxis( void *image )
{
	ASSERT( image != NULL );

	// draw grid
	char numstr[ 10 ];

	for ( int xc = 5 ;; xc += 5 ) {

		int xpos = SCALE_X_COORD( xc );

		int mirror_xpos = map_hwidth - xpos + map_hwidth;
		
		if ( ( ( xpos + map_pan_x ) >= Screen_Width ) &&
			 ( ( mirror_xpos + map_pan_x ) <= 0 ) ) {
			break;
		}

		gdImageLine( image, xpos, MAP_NEG_INFINITY,
							xpos, MAP_POS_INFINITY, darkgreen );
		gdImageLine( image, mirror_xpos, MAP_NEG_INFINITY,
							mirror_xpos, MAP_POS_INFINITY, darkgreen );
		
		snprintf( numstr, 9, "%d", xc );
		size_t width = strlen( numstr ) * FONTTW;
		gdImageString( image, FONTT, xpos - width / 2 , map_hheight + 2, numstr, green );

		snprintf( numstr, 9, "-%d", xc );
		width = strlen( numstr ) * FONTTW;
		gdImageString( image, FONTT, map_hwidth - xpos + map_hwidth - width / 2, map_hheight + 2, numstr, green );
	}

	for ( int yc = 5 ;; yc += 5 ) {

		int ypos = SCALE_Y_COORD( yc );

		int mirror_ypos = map_hheight - ypos + map_hheight;

		if ( ( ( ypos + map_pan_y ) <= 0 ) &&
			 ( ( mirror_ypos + map_pan_y ) >= Screen_Height ) ) {
			break;
		}

		
		gdImageLine( image, MAP_NEG_INFINITY, ypos,
							MAP_POS_INFINITY, ypos, darkgreen );
		gdImageLine( image, MAP_NEG_INFINITY, mirror_ypos,
							MAP_POS_INFINITY, mirror_ypos, darkgreen );
	
		snprintf( numstr, 9, "%d", yc );
		size_t width = strlen( numstr ) * FONTTW;
		gdImageString( image, FONTT, map_hwidth - width - 2 , ypos - FONTTH / 2, numstr, green );

		snprintf( numstr, 9, "-%d", yc );
		width = strlen( numstr ) * FONTTW;
		gdImageString( image, FONTT, map_hwidth - width - 2 , map_hheight - ypos + map_hheight - FONTTH / 2, numstr, green );
	}

	// draw axis
	gdImageLine( image, map_hwidth, MAP_NEG_INFINITY, map_hwidth, MAP_POS_INFINITY, green );
	gdImageLine( image, MAP_NEG_INFINITY, map_hheight,
						MAP_POS_INFINITY, map_hheight, green );
	
	// draw surrounding circle
//	gdImageArc( image, map_hwidth, map_hheight, map_width - 1, map_height - 1, 0, 360, green );
	
	// fill area outside of circle with black
	gdImageFillToBorder( image, 0, 0, green, black );
	gdImageFillToBorder( image, 0, map_height - 1, green, black );
	gdImageFillToBorder( image, map_width - 1, map_hheight - 50, green, black );
	gdImageFillToBorder( image, map_width - 1, map_hheight + 50, green, black );
	
	// draw scale
	/*
	int unit = SCALE_X_COORD( 1 ) - map_hwidth;
	
	int sxp = map_width - 1 - map_pan_x;
	int syp = map_height - FONTTH - 2 - map_pan_y;
	
	gdImageLine( image, sxp, syp, sxp - unit, syp, green );
	gdImageLine( image, sxp, syp, sxp, syp - 4, green );
	gdImageLine( image, sxp - unit, syp, sxp - unit, syp - 4, green );

	int width = unit / 2 - 4 * FONTTW;
	
	if ( width < ( 8 * FONTTW ) ) {
		width = 8 * FONTTW + 1;
	}
		
	gdImageString( image, FONTT, map_width - width - map_pan_x, map_height - FONTTH - map_pan_y, "1 parsec", green ); 
	*/
}


// draw actual starmap graphics into image ------------------------------------
//
int MAP_DrawStarmap()
{
	//NOTE:
	// the image is not used at all, but most not be NULL
	void *image = (void *) 0xdeadbeef;

	//MAP_NormalizeCoordinates();
	
	// draw background objects
	MAP_DrawMapObjects( image );

	// draw axis	
	MAP_DrawMapAxis( image );
	
	if ( num_servers_joined == 0 )
		return TRUE;

	// draw all links between the servers
	int nLinkIndex = 0;
	for( nLinkIndex = 0; nLinkIndex < g_nNumLinks; nLinkIndex++ ) {

		int serverid1 = link_list[ nLinkIndex ].serverid1;
		int serverid2 = link_list[ nLinkIndex ].serverid2;

		// find serverindex for serverid1
		int nServerIndex1 = 0;
		for( nServerIndex1 = 0; nServerIndex1 < MAX_SERVERS; nServerIndex1++ ) {

			if ( server_list[ nServerIndex1 ].slot_free ) {
				continue;
			}
			
			if ( server_list[ nServerIndex1 ].serverid == serverid1 ) {
				break;
			}
		}
		if ( nServerIndex1 == MAX_SERVERS ) {
			continue;
		}

		// find serverindex for serverid2
		int nServerIndex2 = 0;
		for( nServerIndex2 = 0; nServerIndex2 < MAX_SERVERS; nServerIndex2++ ) {

			if ( server_list[ nServerIndex2 ].slot_free ) {
				continue;
			}

			if ( server_list[ nServerIndex2 ].serverid == serverid2 ) {
				break;
			}
		}
		if ( nServerIndex2 == MAX_SERVERS ) {
			continue;
		}

		int xpos = server_list[ nServerIndex1 ].xpos;
		int ypos = server_list[ nServerIndex1 ].ypos;

		int xpos2 = server_list[ nServerIndex2 ].xpos;
		int ypos2 = server_list[ nServerIndex2 ].ypos;

		xpos = SCALE_X_COORD( xpos );
		ypos = SCALE_Y_COORD( ypos );

		xpos2 = SCALE_X_COORD( xpos2 );
		ypos2 = SCALE_Y_COORD( ypos2 );

		int dist = (int)sqrt( (float) (( xpos2 - xpos ) * ( xpos2 - xpos ) + ( ypos2 - ypos ) * ( ypos2 - ypos )) );  

		float xoffs1 = ( xpos2 - xpos ) * ( ( (float) STAR_RADIUS + 1 ) / dist );
		float yoffs1 = ( ypos2 - ypos ) * ( ( (float) STAR_RADIUS + 1 ) / dist );

		// arrowhead lines
		if ( link_list[ nLinkIndex ].flags & SERVERLINKINFO_1_TO_2 ) {

			int xarr1 = (int) ( ( xpos2 - 2.5 * xoffs1 ) + yoffs1 * 0.3f );
			int yarr1 = (int) ( ( ypos2 - 2.5 * yoffs1 ) - xoffs1 * 0.3f );
			int xarr2 = (int) ( ( xpos2 - 2.5 * xoffs1 ) - yoffs1 * 0.3f );
			int yarr2 = (int) ( ( ypos2 - 2.5 * yoffs1 ) + xoffs1 * 0.3f );

			gdImageLine( image, xarr1, yarr1, (int)(xpos2 - xoffs1), (int)(ypos2 - yoffs1), greyramp[ 6 ] );
			gdImageLine( image, xarr2, yarr2, (int)(xpos2 - xoffs1), (int)(ypos2 - yoffs1), greyramp[ 6 ] );
			gdImageLine( image, xarr1, yarr1, xarr2, yarr2, greyramp[ 6 ] );
		}

		float xoffs2 = ( xpos - xpos2 ) * ( ( (float) STAR_RADIUS + 1 ) / dist );
		float yoffs2 = ( ypos - ypos2 ) * ( ( (float) STAR_RADIUS + 1 ) / dist );
		 
		// actual link line
		gdImageLine( image, (int)(xpos + xoffs1), (int)(ypos + yoffs1), (int)(xpos2 - xoffs1), (int)(ypos2 - yoffs1), greyramp[ 6 ] );
	}

	
	for ( int sid = 0; sid < MAX_SERVERS; sid++ ) {

		server_s *srv = &server_list[ sid ];
		int xpos =0;
		int ypos = 0;
		if ( !srv->slot_free ) {
   
   			// don't draw unknown stars
			if ( srv->serverid == -1 || srv->ping_in_ms < 0)
   				continue;

   			 xpos = srv->xpos;
   			 ypos = srv->ypos;

			xpos = SCALE_X_COORD( xpos );
			ypos = SCALE_Y_COORD( ypos );

			// draw star symbol   

			texscreenrect_s	rect;

			dword itertype = iter_texrgba | iter_specularadd;
			rect.alpha     = map_fadepos;
			rect.x = (int)(xpos + map_xoffs - 6 + map_pan_x);
			rect.y = (int)(ypos - 6 + map_pan_y);
			rect.w = starmap_bg_info[ MAP_SYMBOL ].owidth;
			rect.h = starmap_bg_info[ MAP_SYMBOL ].oheight;
			rect.scaled_w = 11;
			rect.scaled_h = 11;
			rect.texofsx  = 0;
			rect.texofsy  = 0;
			rect.itertype = itertype;
			rect.texmap   = starmap_bg_info[ MAP_SYMBOL ].texmap;

			Rectangle2 cliprect;
			MAP_FetchClipRectangle( &cliprect );

			if ( ( rect.x + 11 ) >= cliprect.left && rect.x < cliprect.right &&
				 ( rect.y + 11 ) >= cliprect.top  && rect.y < cliprect.bottom ) {

				DRAW_TexturedScreenRect( &rect, NULL );
			}

			// draw server name
			char namestr[ MAX_SERVER_NAME + 1 ];
			strncpy( namestr, srv->server_name, MAX_SERVER_NAME );
			namestr[ MAX_SERVER_NAME ] = 0;

			// replace underscores with spaces
			for ( char *scan = namestr; *scan; scan++ ) {

				if ( *scan == '_' )
					*scan = ' ';
			}

			// draw servername
			gdImageString( image, FONTS, xpos + 6, ypos - FONTSH - 3, namestr, greyramp[ 6 ] );

			// draw coordinates
			sprintf( paste_str, "(%d:%d)", srv->xpos, srv->ypos );
			int coord_width = gdImageString( image, FONTT, xpos + 6, ypos - 1, paste_str, greyramp[ 6 ] );

			// draw ping
			sprintf( paste_str, " [%d ms]", srv->ping_in_ms );
			gdImageString( image, FONTT, xpos + 6 + coord_width, ypos - 1, paste_str, greyramp[ 6 ] );
   		}
   }

   return TRUE;

}


// ----------------------------------------------------------------------------
//
void MAP_DrawButtonCaptions()
{
	// set the string context for displaying HUD information
	D_SetWStrContext( CharsetInfo[ HUD_CHARSETNO ].charsetpointer,
					  CharsetInfo[ HUD_CHARSETNO ].geompointer,
					  NULL,
					  CharsetInfo[ HUD_CHARSETNO ].width,
					  CharsetInfo[ HUD_CHARSETNO ].height );

	colrgba_s oldcol;	
	colrgba_s fontcol1 = {  60,  60,  60, (byte) map_fadepos };
	colrgba_s fontcol2 = { 220, 220, 220, (byte) map_fadepos };

	oldcol = PanelTextColor;

	int chwidth   = CharsetInfo[ HUD_CHARSETNO ].width;
	int chheight  = CharsetInfo[ HUD_CHARSETNO ].height;
	float topedge;
	float bottomedge;

	for ( int bid = 0; bid < NUM_SELECT_BUTTONS; bid++ ) {

		int butstart = (Screen_Width - 150);
		int strwidth = chwidth * strlen( select_button_text[ bid ] );
		
		int xpos = (int)((butstart ));

		topedge = 292 + bid * 64 ;
				
		bottomedge = topedge + 64;
			
		int ypos = (int)(topedge  + ( ( bottomedge - topedge )  - chheight ) / 2);

		PanelTextColor = fontcol1;
		D_WriteTrString( select_button_text[ bid ], xpos + 1, ypos + 1, TRTAB_PANELTEXT );

		PanelTextColor = fontcol2;
		D_WriteTrString( select_button_text[ bid ], xpos, ypos, TRTAB_PANELTEXT );
	}

	PanelTextColor = oldcol;
}

void MAP_WriteInfo(int serverid) {

	// if we have a valid server id
	if(serverid > -1) {
		char tmp_buffer[256];
		int i = 0;
		for(i=0; i<NUM_INFO_BOX_LINES; i++){
			memset((void *)tmp_buffer, 0, 256);
			switch(i) {
				case 0:
					snprintf(tmp_buffer, 256, info_box_strings[i], server_list[serverid].server_name);
					break;
				case 1: 
					snprintf(tmp_buffer, 256, info_box_strings[i], server_list[serverid].major_version, server_list[serverid].minor_version);
					break;
				case 2:
					snprintf(tmp_buffer, 256, info_box_strings[i],  server_list[serverid].ping_in_ms );
					break;
				case 3:
					snprintf(tmp_buffer, 256, info_box_strings[i],  server_list[serverid].number_of_players, server_list[serverid].max_players );
					break;
				case 4:
					tmp_buffer[0]='\0';
					break;
				default:
					break;
			}
			if(!tmp_buffer[0]=='\0'){
				int xpos = (INFO_BOX_X + .01) * Screen_Width;
				int ypos = ((INFO_BOX_Y + .01) * Screen_Height) + (INFO_BOX_TEXT_H * (i*3));
				D_WriteTrString(tmp_buffer, xpos, ypos , TRTAB_PANELTEXT);
			}
		}


	}

	return;
}
void Map_UndrawOverlay() {
		// draw an opaque backdrop over the background.
	colscreenrect_s overlay;

	if(map_animate_out < (Screen_Height-(MAP_OVERLAY_ANI_SPEED + 1))) {
		overlay.x= 0; 
		overlay.y= (Screen_Height / 2) - (((Screen_Height) - (map_animate_out))/ 2);
		overlay.h = (Screen_Height) - (map_animate_out);
		overlay.w = Screen_Width;
		overlay.itertype = iter_rgba | iter_alphablend;
		overlay.color.R = 0;
		overlay.color.G = 0;
		overlay.color.B = 0;
		overlay.color.A = 192;
		map_animate_out += MAP_OVERLAY_ANI_SPEED;
		DRAW_ColoredScreenRect(&overlay);

	} else {
		InStarMap = FALSE;
	}




}
void MAP_DrawOverlay(){
	// draw an opaque backdrop over the background.
	colscreenrect_s overlay;

	if(map_animate_in > 0) {
		overlay.x= 0; 
		overlay.y= (Screen_Height / 2) - (((Screen_Height) - (map_animate_in))/ 2);
		overlay.h = (Screen_Height) - (map_animate_in);
		overlay.w = Screen_Width;
		overlay.itertype = iter_rgba | iter_alphablend;
		overlay.color.R = 0;
		overlay.color.G = 0;
		overlay.color.B = 0;
		overlay.color.A = 192;
		map_animate_in -= MAP_OVERLAY_ANI_SPEED;


	} else {
	
		overlay.x=0;
		overlay.y=0;
		overlay.h = Screen_Height;
		overlay.w = Screen_Width;
		overlay.itertype = iter_rgba | iter_alphablend;
		overlay.color = PanelBackColor;

		overlay.color.R = 0;
		overlay.color.G = 0;
		overlay.color.B = 0;
		overlay.color.A = 192;



	}

	DRAW_ColoredScreenRect(&overlay);

}
// ----------------------------------------------------------------------------
//
void MAP_DrawBackground()
{
	texscreenrect_s	rect;

	dword itertype = iter_texrgba | iter_premulblend;
	rect.alpha     = map_fadepos;


	int tid = 0;



	// draw info box
	int ib_x=  0, ib_y = 0, ib_w = 0, ib_h = 0;

	ib_x = INFO_BOX_X * Screen_Width; 
	ib_y = INFO_BOX_Y * Screen_Height;
	ib_w = INFO_BOX_W * Screen_Width;
	ib_h = INFO_BOX_H * Screen_Height;
	
	DRAW_ClippedTrRect( ib_x, ib_y, ib_w, ib_h, TRTAB_PANELBACK );
	
	
	// draw the nav buttons
	int nbid=0;
	for(nbid=0; nbid<9; nbid++){

		rect.x = (int)((Screen_Width - (64*3+16))  + (nbid % 3) * 64 );
		rect.y =  (int)((16   +  (nbid / 3) * 64) );
		rect.w = 64;
		rect.h = 64;
		rect.scaled_w = 64;
		rect.scaled_h = 64;
		rect.texofsx  = 0;
		rect.texofsy  = 0;
		rect.itertype = itertype;
		rect.texmap = FetchTextureMap( inactive_button_textures[nbid]);
		if(rect.texmap == NULL){
			itertype = iter_texrgba | iter_specularadd;
		}
		
		DRAW_TexturedScreenRect( &rect, NULL );

	}

	//draw the other buttons
	for(nbid=9; nbid<12; nbid++){
		rect.x = (Screen_Width - (64+150)); 
		rect.y = 292 + (nbid-9) * 64 ;
		rect.w = 64;
		rect.h = 64;
		rect.scaled_w = 64;
		rect.scaled_h = 64;
		rect.texofsx  = 0;
		rect.texofsy  = 0;
		rect.itertype = itertype;
		rect.texmap = FetchTextureMap( inactive_button_textures[nbid]);
		if(rect.texmap == NULL){
			itertype = iter_texrgba | iter_specularadd;
		}
		
		DRAW_TexturedScreenRect( &rect, NULL );


	}

	// draw hilited selection indicator

	if ( map_sel_hilite != -1 || map_nav_hilite != -1 || map_srv_hilite != -1) {
		int xpos=0;
		int ypos=0;
		int scaled_width=0;
		int scaled_height=0;

		itertype = iter_texrgba | iter_specularadd;
		
		if(map_sel_hilite != -1) {

			rect.x = (Screen_Width - (64+150)); 
			rect.y = 292 + map_sel_hilite * 64 ;
				
			rect.w = 64;
			rect.h = 64;
			rect.scaled_w = 64;
			rect.scaled_h = 64;
			rect.texofsx  = 0;
			rect.texofsy  = 0;
			rect.itertype = itertype;
			rect.texmap   = FetchTextureMap( active_button_textures[9+map_sel_hilite]);

		
		} else if (map_nav_hilite != -1) {
			
			xpos = (int)((Screen_Width - (64*3+16))  + (map_nav_hilite % 3) * 64 );
			ypos = (int)((16   +  (map_nav_hilite / 3) * 64) );
			rect.texmap = FetchTextureMap( active_button_textures[map_nav_hilite]);
			rect.x = xpos; 
			rect.y = ypos;
				
			rect.w = 64;
			rect.h = 64;
			rect.scaled_w = 64;
			rect.scaled_h = 64;
			rect.texofsx  = 0;
			rect.texofsy  = 0;
			rect.itertype = itertype;
		

		} else if(map_srv_hilite != -1) {

			server_s *srv = &server_list[ map_srv_hilite ];

			if ( srv->serverid > 0 ) {

				
				scaled_width = 11;
				scaled_height = 11;
				xpos = (SCALE_X_COORD(srv->xpos)+ map_xoffs - 6 + map_pan_x);
				ypos = (SCALE_Y_COORD(srv->ypos ) - 6 + map_pan_y);
				rect.x = xpos; 
				rect.y = ypos;
			
				rect.w = starmap_bg_info[ MAP_HILITE ].owidth;
				rect.h = starmap_bg_info[ MAP_HILITE ].oheight;
				rect.scaled_w = scaled_width;
				rect.scaled_h = scaled_height;
				rect.texofsx  = 0;
				rect.texofsy  = 0;
				rect.itertype = itertype;
				rect.texmap   = starmap_bg_info[ MAP_HILITE ].texmap;
				
			}
		}


		DRAW_TexturedScreenRect( &rect, NULL );
	
		
	}


	MAP_DrawButtonCaptions();
		// draw the server selection too, if one exists
	if(map_srv_pressed != -1 ) {
			server_s *srv = &server_list[ map_srv_pressed ];

		if ( srv->serverid > 0 ) {

			texscreenrect_s	rect;

			dword itertype = iter_texrgba | iter_premulblend;
			rect.x = (SCALE_X_COORD(srv->xpos)+ map_xoffs - 6 + map_pan_x);
			rect.y = (SCALE_Y_COORD(srv->ypos ) - 6 + map_pan_y);
			
			rect.w = starmap_bg_info[ MAP_HILITE ].owidth;
			rect.h = starmap_bg_info[ MAP_HILITE ].oheight;
			rect.scaled_w = 11;
			rect.scaled_h = 11;
			rect.texofsx  = 0;
			rect.texofsy  = 0;
			rect.itertype = itertype;
			rect.texmap   = starmap_bg_info[ MAP_HILITE ].texmap;

			DRAW_TexturedScreenRect( &rect, NULL );
		}
	}

}


// ----------------------------------------------------------------------------
//
int MAP_ShowStarmap( void *param )
{
	if ( !InStarMap )
		return FALSE;
	
	MAP_FadeStarmapAlpha();


	// make sure textures are available
	if ( !AcquireStarmapTextures() )
		return FALSE;

	map_width	= (int)(( 460.0f / 640.0f ) * Screen_Width);
	map_height	= map_width;
	map_hwidth  = map_width / 2;
	map_hheight = map_height / 2;
	map_xoffs	= 0;
	
	if(map_animate_out >= Screen_Height){
		MAP_DrawOverlay();
	
		if(map_animate_in <= 0){
			MAP_DrawStarmap();
		
			MAP_DrawBackground();

			MAP_WriteInfo(map_srv_pressed);
		}
	} else {
		Map_UndrawOverlay();
	}
	return TRUE;
}


// execute "refresh" command --------------------------------------------------
//
PRIVATE
void MAP_ExecRefresh()
{
	NET_GetGameServerList();
}


// execute "exit" command -----------------------------------------------------
//
PRIVATE
void MAP_ExecExit()
{
	map_animate_out = 0;
	
}


// execute "zoom out" command -------------------------------------------------
//
PRIVATE
void MAP_ExecZoomOut()
{
	if ( DepressedKeys->key_AfterBurner ) {
		map_scale -= 2;
	} else {
		map_scale--;
	}

	if ( map_scale < 3 ) {
		map_scale = 3;
	}
}


// execute "zoom in" command --------------------------------------------------
//
PRIVATE
void MAP_ExecZoomIn()
{
	if ( DepressedKeys->key_AfterBurner ) {
		map_scale += 2;
	} else {
		map_scale++;
	}

	if ( map_scale > 112 ) {
		map_scale = 112;
	}
}


// execute "pan up" command ---------------------------------------------------
//
PRIVATE
void MAP_ExecPanUp()
{
	if ( DepressedKeys->key_AfterBurner ) {
		map_pan_y += 5;
	} else {
		map_pan_y++;
	}
}


// execute "pan down" command -------------------------------------------------
//
PRIVATE
void MAP_ExecPanDown()
{
	if ( DepressedKeys->key_AfterBurner ) {
		map_pan_y -= 5;
	} else {
		map_pan_y--;
	}
}


// execute "pan left" command -------------------------------------------------
//
PRIVATE
void MAP_ExecPanLeft()
{
	if ( DepressedKeys->key_AfterBurner ) {
		map_pan_x += 5;
	} else {
		map_pan_x++;
	}
}


// execute "pan right" command ------------------------------------------------
//
PRIVATE
void MAP_ExecPanRight()
{
	if ( DepressedKeys->key_AfterBurner ) {
		map_pan_x -= 5;
	} else {
		map_pan_x--;
	}
}


// execute "reset" command ----------------------------------------------------
//
PRIVATE
void MAP_ExecReset()
{
	map_pan_x = 0;
	map_pan_y = 0;
	map_scale = 0;
	map_normalize_done = FALSE;
	MAP_NormalizeCoordinates();
}


// ----------------------------------------------------------------------------
//
PRIVATE
void MAP_MouseGetState()
{
	// avoid dangling position state
	mouse_position_state = MOUSE_OVER_NOTHING;

	// retrieve mouse state
	mousestate_s mousestate;
	int mouseavailable = INPs_MouseGetState( &mousestate );

	// make sure mouse is there
	if ( !mouseavailable ) {
		return;
	}

	cur_mouse_x = mousestate.xpos;
	cur_mouse_y = mousestate.ypos;
	cur_mouse_button_state = mousestate.buttons[ MOUSE_BUTTON_LEFT ];

	// if button released check only if mouse has been moved since last frame
	if ( cur_mouse_button_state == MOUSE_BUTTON_RELEASED ) {
		if ( ( cur_mouse_x == old_mouse_x ) && ( cur_mouse_y == old_mouse_y ) ) {
			return;
		}
	}

	old_mouse_x = cur_mouse_x;
	old_mouse_y = cur_mouse_y;
}


// ----------------------------------------------------------------------------
//
void MAP_CheckMouseSelButtons()
{
	float	topedge;
	float bottomedge;

	if ( (cur_mouse_x * Screen_Width) >= (Screen_Width - (64+150)) && (cur_mouse_x * Screen_Width) <= (Screen_Width - 150) ) {
		int bid = 0;
		for ( bid = 0; bid < NUM_SELECT_BUTTONS; bid++ ) {

			topedge = 292 + bid * 64 ;

			bottomedge = topedge + 64;		

			if ( (cur_mouse_y * Screen_Height) >= topedge && (cur_mouse_y * Screen_Height) <= bottomedge ) {

				map_sel_hilite = bid;
				break;
			}
		}

		if ( bid == NUM_SELECT_BUTTONS ) {
			map_sel_hilite = -1;
		}

	} else {
		map_sel_hilite = -1;
	}
}


// ----------------------------------------------------------------------------
//
void MAP_CheckMouseNavButtons()
{
	int bid = 0;
	for ( bid = 0; bid < NUM_NAV_BUTTONS; bid++ ) {

		int xidx = bid % 3; // 0 1 2
		float left = (Screen_Width - (64*3+16)) + xidx * 64; //bw = 10

		int yidx = bid / 3; // 0 1 2
		float top = 16 + yidx * 64;  // bh = 10



		if ( (cur_mouse_x * Screen_Width) >= left && (cur_mouse_x * Screen_Width) < ( left + 64 ) &&
			(cur_mouse_y * Screen_Height) >= top  && (cur_mouse_y * Screen_Height) < ( top  + 64 ) ) {

			map_nav_hilite = bid;
			break;
		}
	}


	if ( bid == NUM_NAV_BUTTONS || bid ==  MAPNAV_NULL || bid == MAPNAV_NULL2) {
		map_nav_hilite = -1;
	} 

}

// ----------------------------------------------------------------------------
//
int print_server_coords = 1;
int srv_left=0;
int srv_top=0;
int curr_mouse_xint = 0;
int curr_mouse_yint=0;
int srv_width=0;
int srv_height=0;
void MAP_CheckMouseServer()
{
	int sid = 0;
	for ( sid = 0; sid < MAX_SERVERS; sid++ ) {

		server_s *srv = &server_list[ sid ];

		if ( srv->serverid > 0 ) {
				
			
		
			srv_left = (SCALE_X_COORD(srv->xpos)+ map_xoffs - 6 + map_pan_x);
			srv_top = (SCALE_Y_COORD(srv->ypos ) - 6 + map_pan_y);			
		
			srv_width = 11;
			srv_height = 11;
			curr_mouse_xint = (int) (cur_mouse_x * Screen_Width);
			curr_mouse_yint = (int) (cur_mouse_y * Screen_Height);
			sid = sid+1;
			sid = sid-1;
			if ( curr_mouse_xint >= srv_left && curr_mouse_xint < ( srv_left + srv_width ) &&
				curr_mouse_yint >= srv_top  && curr_mouse_yint < ( srv_top  + srv_height ) ) {
				MSGOUT("Selecting Server %d", sid);
					map_srv_hilite = sid;
				break;
			}	
		}
	}
	print_server_coords=0;

	if ( sid == MAX_SERVERS ) {
		map_srv_hilite = -1;
	} 

}

// 0 1 2 = (0, 0) (1, 0) (2,0)
// 3 4 5 = (0, 1) (1, 1) (2, 1)
// 6 7 8 = (0, 2) (1, 2) (2, 2)

// ----------------------------------------------------------------------------
//
void MAP_ExecSelButtonChoice()
{
	if ( cur_mouse_button_state == MOUSE_BUTTON_PRESSED &&
		 map_sel_hilite != -1 && map_sel_pressed == -1 ) {

		map_sel_pressed = map_sel_hilite;

	} else if ( map_sel_hilite == map_sel_pressed ) {

		for ( int bid = 0; bid < NUM_SELECT_BUTTONS; bid++ ) {

			if ( ( old_mouse_button_state == MOUSE_BUTTON_PRESSED ) &&
				 ( cur_mouse_button_state == MOUSE_BUTTON_RELEASED ) &&
				 ( map_sel_pressed == bid ) ) {

				map_sel_pressed = -1;

				switch ( bid ) {

					case MAPSEL_REFRESH:

						MAP_ExecRefresh();
						break;

					case MAPSEL_EXIT:

						MAP_ExecExit();
						break;
					case MAPSEL_CONNECT:
						if(map_srv_pressed != -1) {
							MSGOUT("We want to connect to server %d", map_srv_pressed);
							// make a copy of the node_t structure for this server.
							// Server_Node is a special global that NET_ServerConnect() will use.
							memcpy(&Server_Node, &server_list[map_srv_pressed].node, sizeof( node_t ) );

							// because we are bypassing the resolver at this point....
							// set maximum number of players according to protocol
							CurMaxPlayers = MAX_NET_GMSV_PLAYERS;

							// (re)init tables
							NET_InitRemotePlayerTables();

							// discard old packets
							NETs_FlushListenBuffers();
							NumRemPlayers = 1;

							// try to establish connection
							int connect_success = NET_ServerConnect();
							if(connect_success) {
								//FREEMEM(CurServerToResolve);
								//CurServerToResolve = NULL;
								MenuNotifyConnect();
								MAP_ExecExit();
							} 

							// TODO: Check return and write to info box if it fails.
							
						}
						break;


				}
			}
		}
	}
}


// ----------------------------------------------------------------------------
//
void MAP_ExecNavButtonChoice()
{
	for ( int bid = 0; bid < NUM_NAV_BUTTONS; bid++ ) {

		if ( ( cur_mouse_button_state == MOUSE_BUTTON_PRESSED ) &&
			 ( map_nav_hilite == bid ) ) {

			switch ( bid ) {

				case MAPNAV_ZOOMOUT:

					MAP_ExecZoomOut();
					break;

				case MAPNAV_ZOOMIN:

					MAP_ExecZoomIn();
					break;

				case MAPNAV_RESET:

					MAP_ExecReset();
					break;

				case MAPNAV_PANUP:

					MAP_ExecPanUp();
					break;

				case MAPNAV_PANDOWN:

					MAP_ExecPanDown();
					break;

				case MAPNAV_PANLEFT:

					MAP_ExecPanLeft();
					break;

				case MAPNAV_PANRIGHT:

					MAP_ExecPanRight();
					break;
			}
		}
	}
}

void MAP_ExecServerChoice()
{
	if(cur_mouse_button_state == MOUSE_BUTTON_RELEASED && old_mouse_button_state == MOUSE_BUTTON_PRESSED){
		if ( (map_srv_hilite != -1 && map_srv_pressed == -1)  ||
			 (map_srv_hilite != -1 && map_srv_pressed != map_srv_hilite)) {
			// set the global pressed var on click
			map_srv_pressed = map_srv_hilite;

			// TODO: display the info.
			

		} else if ( map_srv_hilite == -1 && map_srv_pressed != -1) {
			// user clicked off that server.
			map_srv_pressed = -1;
		}
	}
}


// handle mouse actions for starmap -------------------------------------------
//
void MAP_MouseHandler()
{
	if ( !AUX_ENABLE_MOUSE_FOR_MENU_SCREENS ) {
		return;
	}

	MAP_MouseGetState();

	// check if mouse is over any select button
	MAP_CheckMouseSelButtons();

	// check if mouse is over any navigation button
	MAP_CheckMouseNavButtons();

	// check if the mouse is over any server 
	MAP_CheckMouseServer();

	MAP_ExecSelButtonChoice();

	MAP_ExecNavButtonChoice();

	MAP_ExecServerChoice();

	int mousex = (int)(cur_mouse_x * Screen_Width);
	int mousey = (int)(cur_mouse_y * Screen_Height);

	// check mouse panning
	if ( DepressedKeys->key_ShootWeapon ) {
	
		Rectangle2 cliprect;
		MAP_FetchClipRectangle( &cliprect );

		if ( mousex >= cliprect.left && mousex < cliprect.right &&
			 mousey >= cliprect.top  && mousey < cliprect.bottom ) {
		
			if ( old_mouse_button_state == MOUSE_BUTTON_RELEASED &&
				 cur_mouse_button_state == MOUSE_BUTTON_PRESSED ) {
				 
				map_dragging = TRUE;
				map_dragorigin_x = mousex;
				map_dragorigin_y = mousey;
			}
		}

		if ( map_dragging ) {
		
			int mousepanx = mousex - map_dragorigin_x;
			int mousepany = mousey - map_dragorigin_y;
			
			map_pan_x += mousepanx;
			map_pan_y += mousepany;
			
			map_dragorigin_x = mousex;
			map_dragorigin_y = mousey;
		}


		if ( map_dragging && cur_mouse_button_state == MOUSE_BUTTON_RELEASED ) {

			map_dragging = FALSE;
		}
		
	// check mouse dragging of info panel
	} else if ( cur_mouse_x < navpanel_leftx ) {
	
		int infopanel_y = (int)(Screen_Height - ( infopanel_height - 20.0f/480.0f ) * Screen_Height);
		int infopanel_y2 = (int)(Screen_Height - ( infopanel_height - 40.0f/480.0f ) * Screen_Height);
		
		if ( mousey > infopanel_y && mousey < infopanel_y2 ) {

			if ( old_mouse_button_state == MOUSE_BUTTON_RELEASED &&
				 cur_mouse_button_state == MOUSE_BUTTON_PRESSED ) {
				 
				infopanel_dragging   = TRUE;
				infopanel_dragorigin = mousey;
			}
		
		}
	}

	if ( infopanel_dragging ) {
	
		int mousepany = mousey - infopanel_dragorigin;
		
		float tmp = infopanel_height - ( (float) mousepany / 480.0f);
		
		int tmp_y = (int)(Screen_Height - ( tmp - 20.0f/480.0f ) * Screen_Height);
		
		if ( tmp_y <= 398 ) {
		
			infopanel_height -= ( (float) mousepany / 480.0f);
	
			infopanel_dragorigin = mousey;

		} else {
		
			infopanel_height = 102.0f / 480.0f;
		}

	}
			
		
	if ( infopanel_dragging && cur_mouse_button_state == MOUSE_BUTTON_RELEASED ) {

		infopanel_dragging = FALSE;
	}

	// remember state for edge detection
	old_mouse_button_state = cur_mouse_button_state;
}


// ----------------------------------------------------------------------------
//
void MAP_KeyHandler()
{
	ASSERT( InStarMap );

	MAP_MouseHandler();

	if ( ExitGameLoop ) {

		//NOTE:
		// we intercept the game loop exit caused by pressing escape
		// because we never exit the game loop immediately when the
		// floating menu is active. it will only be exited when the
		// user actually presses enter on the quit button.

		ExitGameLoop = 0;
		map_animate_out = 0;
		return;
	}


	if ( DepressedKeys->key_CursorUp ) {

		MAP_ExecPanUp();

	} else if ( DepressedKeys->key_CursorDown ) {

		MAP_ExecPanDown();
	}

	if ( DepressedKeys->key_CursorLeft ) {

		MAP_ExecPanLeft();

	} else if ( DepressedKeys->key_CursorRight ) {

		MAP_ExecPanRight();
	}

	if ( DepressedKeys->key_Accelerate ) {

		MAP_ExecZoomIn();

	} else if ( DepressedKeys->key_Decelerate ) {

		MAP_ExecZoomOut();
	}
}


// ----------------------------------------------------------------------------
//
void MAP_ActivateStarmap()
{

#ifdef LINKED_PROTOCOL_GAMESERVER

	// toggle global flag to activate/deactivate StarMap viewer
	InStarMap = !InStarMap;

	if ( NET_ProtocolGMSV() ) {

		if ( num_servers_joined == 0 ) {

			if ( !NET_GetGameServerList() ) {
				return;
			}
		}
	}

#endif // LINKED_PROTOCOL_GAMESERVER
	map_animate_in=Screen_Height;
	map_animate_out = Screen_Height;
}


// ----------------------------------------------------------------------------
//
PRIVATE
void MAP_RegisterCallbacks()
{
	// specify callback type and flags
	int callbacktype = CBTYPE_DRAW_OVERLAYS | CBFLAG_PERSISTENT;

	// register the drawing callback for drawing the star map
	CALLBACK_RegisterCallback( callbacktype, MAP_ShowStarmap, (void*) NULL );
}


// console command to activate/deactivate the starmap -------------------------
//
PRIVATE
int Cmd_STARMAP( char *paramstr )
{
	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	MAP_ActivateStarmap();

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( H_STARMAP )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "starmap" command
	regcom.command	 = "starmap";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_STARMAP;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

//	MAP_RegisterCallbacks();

	// init greyramp
	int cid = 0;
	for ( cid = 0; cid < NUM_GREYS; cid++ ) {

		int level = 0xff * cid / NUM_GREYS;

		map_colors[ NUM_COLORS + cid ].R = level;
		map_colors[ NUM_COLORS + cid ].G = level;
		map_colors[ NUM_COLORS + cid ].B = level;
		map_colors[ NUM_COLORS + cid ].A = 0xff;

		greyramp[ cid ] = NUM_COLORS + cid;
	}
}



