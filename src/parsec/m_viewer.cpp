/*
 * PARSEC - Spacecraft/Objects Viewer
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
#include "od_class.h"

// global externals
#include "globals.h"

// subsystem headers
#include "sys_defs.h"

// drawing subsystem
#include "d_font.h"

// rendering subsystem
#include "r_obj.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "m_viewer.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_main.h"
#include "e_color.h"
#include "e_draw.h"
#include "e_mouse.h"
#include "e_supp.h"
#include "m_main.h"
#include "obj_clas.h"
#include "obj_creg.h"
#include "obj_ctrl.h"



// string constants -----------------------------------------------------------
//
static char no_ship_class[]			= "object class is no ship.";
static char no_registered_class[]	= "object class is not registered as ship.";
static char text_too_large[]		= "max text size exceeded.";
static char clear_arg_invalid[]		= "clear argument invalid.";


// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 2047
static char paste_str[ PASTE_STR_LEN + 1 ];


// spacecraft viewer areas for mouse-over detection ---------------------------
//
#define SCV_NUM_MOUSE_AREAS		5

static mouse_area_s scv_mouse_areas[ SCV_NUM_MOUSE_AREAS ] = {

	{ 248, 202,  29,  37 },		// button left
	{ 248, 240,  29,  37 },		// button right
	{ 248,  40,  29,  89 },		// button up
	{ 248, 359,  29,  89 },		// button down
	{ 282,   0, 640, 480 },		// ship area
};

static mouse_area_scaled_s scv_mouse_areas_scaled[ SCV_NUM_MOUSE_AREAS ];


// detect whether mouse position is over spacecraft viewer item ---------------
//
int MouseOverViewerItem( int mousex, int mousey )
{
	// make sure area coordinates match resolution
	static resinfo_s scv_last_screen_res;
	if (scv_last_screen_res != GameScreenRes) {
		MouseCalcScaledAreas( scv_mouse_areas, scv_mouse_areas_scaled, SCV_NUM_MOUSE_AREAS );
		scv_last_screen_res = GameScreenRes;
	}

	// check against clickable areas in spacecraft viewer
	if ( MouseOverArea( &scv_mouse_areas_scaled[ 0 ], mousex, mousey ) ) {
		return MOUSE_OVER_LEFTARROW;
	}
	if ( MouseOverArea( &scv_mouse_areas_scaled[ 1 ], mousex, mousey ) ) {
		return MOUSE_OVER_RIGHTARROW;
	}
	if ( MouseOverArea( &scv_mouse_areas_scaled[ 2 ], mousex, mousey ) ) {
		return MOUSE_OVER_UPARROW;
	}
	if ( MouseOverArea( &scv_mouse_areas_scaled[ 3 ], mousex, mousey ) ) {
		return MOUSE_OVER_DOWNARROW;
	}
	if ( MouseOverArea( &scv_mouse_areas_scaled[ 4 ], mousex, mousey ) ) {
		return MOUSE_OVER_SHIP;
	}

	return MOUSE_OVER_NOTHING;
}


// spacecraft viewer background texture info ----------------------------------
//
struct scv_background_info_s {

	float		xpos;
	float		ypos;
	float		width;
	float		height;
	int			owidth;
	int			oheight;

	TextureMap*	texmap;
	const char*	texname;
};

static scv_background_info_s scv_background_info[] = {

	{   0.0,   0.0, 0.4, 0.533, 256, 256,  NULL, "scv00" },
	{   0.4,   0.0, 0.4, 0.533, 256, 256,  NULL, "scv01" },
	{   0.8,     0, 0.2, 0.533, 128, 256,  NULL, "scv02" },
	{   0.0, 0.533, 0.4, 0.533, 256, 224,  NULL, "scv10" },
	{   0.4, 0.533, 0.4, 0.533, 256, 224,  NULL, "scv11" },
	{   0.8, 0.533, 0.2, 0.533, 128, 224,  NULL, "scv12" },

	{ 0, 0, 0, 0, 0, 0, NULL, NULL }
};

static int scv_background_valid = FALSE;


// try to acquire textures for logo animation ---------------------------------
//
PRIVATE
int AcquireScvTextures()
{
	if ( scv_background_valid )
		return TRUE;

	scv_background_valid = TRUE;

	for ( int tid = 0; scv_background_info[ tid ].texname; tid++ ) {

		TextureMap *texmap = FetchTextureMap( scv_background_info[ tid ].texname );
		scv_background_info[ tid ].texmap = texmap;

		if ( texmap == NULL ) {
			scv_background_valid = FALSE;
		}
	}

	return scv_background_valid;
}


// spacecraft viewer configuration and state variables ------------------------
//
static Camera SpacecraftCamera;

#define SCV_SHIP_RIGHT			0
#define SCV_SHIP_LEFT			( m_sintab_size - 1 )
#define SCV_SHIP_SCALE			1.0f	//2.0f
#define SCV_SHIP_CENTER_X		40.0f
#define SCV_SHIP_CENTER_Y		5.0f
#define SCV_SHIP_CENTER_Z		15.0f
#define SCV_SHIP_SLIDE_SPEED	12

#define SCV_TEXT_X				0.06	// 38
#define SCV_TEXT_Y				0.17	// 82
#define SCV_TEXT_W				0.28	// 182
#define SCV_TEXT_H				0.66	// 318

#define SCV_CAM_PITCH 			( (bams_t)  0x0d00 )
#define SCV_CAM_YAW				( (bams_t) -0x0d00 )
#define SCV_CAM_ROLL			( (bams_t)  0x0000 )
#define SCV_CAM_DIST			FIXED_TO_GEOMV( 0x600000 )

#define SCV_OBJROTSPEED			((bams_t) 0x00a0/10)

#define SCV_BG_ALPHA_LOW		0
#define SCV_BG_ALPHA_HIGH		255
#define SCV_BG_FADE_SPEED		12
#define SCV_BG_FADE_QUANTUM		10


//#define USE_SCV_SHIP_SCALE		// undef if SCV_SHIP_SCALE == 1.0f

#ifdef USE_SCV_SHIP_SCALE
static Xmatrx scv_ship_scale_matrx = {

	{ FLOAT_TO_GEOMV( SCV_SHIP_SCALE ), GEOMV_0, GEOMV_0, GEOMV_0 },
	{ GEOMV_0, FLOAT_TO_GEOMV( SCV_SHIP_SCALE ), GEOMV_0, GEOMV_0 },
	{ GEOMV_0, GEOMV_0, FLOAT_TO_GEOMV( SCV_SHIP_SCALE ), GEOMV_0 }
};
#endif

//NOTE:
// scv_ship_pos_matrx[ 0 ][ 3 ] will be modified (reset)
// by DrawSpacecraft() to reflect the correctly slided
// x position of the ship.

static Xmatrx scv_ship_pos_matrx = {

	{ FLOAT_TO_GEOMV( 1.0f ), FLOAT_TO_GEOMV( 0.0f ), FLOAT_TO_GEOMV( 0.0f ), FLOAT_TO_GEOMV( SCV_SHIP_CENTER_X ) },
	{ FLOAT_TO_GEOMV( 0.0f ), FLOAT_TO_GEOMV( 1.0f ), FLOAT_TO_GEOMV( 0.0f ), FLOAT_TO_GEOMV( SCV_SHIP_CENTER_Y ) },
	{ FLOAT_TO_GEOMV( 0.0f ), FLOAT_TO_GEOMV( 0.0f ), FLOAT_TO_GEOMV( 1.0f ), FLOAT_TO_GEOMV( SCV_SHIP_CENTER_Z ) }
};

#define MAX_TEXT_LEN		1024

enum {

	CAPTION_LINE,
	TEXT_LINE,

	NUM_SHIP_LINES
};

static int			scv_ship_slidepos 		= SCV_SHIP_RIGHT;
static int			scv_ship_slidetarget	= 0;
static refframe_t	scv_ship_lastref 		= REFFRAME_INVALID;

static int			scv_bg_fadepos 			= SCV_BG_ALPHA_LOW;
static int			scv_bg_fadetarget		= SCV_BG_ALPHA_LOW;
static refframe_t	scv_bg_lastref 			= REFFRAME_INVALID;

static int 			scv_ship_textcnt[ MAX_SHIP_CLASSES ];
static char*		scv_ship_text[ MAX_SHIP_CLASSES ][ NUM_SHIP_LINES ];

int					cur_scv_mode = SCVMODE_SPACECRAFT;
int					cur_scv_ship = 0;


// object viewer ring specifics -----------------------------------------------
//
#define RING_ARC			( 0x10000 / NUM_SCV_OBJECTS )
#define RING_RADIUS			70.0f

#define RING_POS_RIGHT		0
#define RING_POS_LEFT		( m_sintab_size - 1 )

#define RING_SLIDE_SPEED	12

int scv_ring_objects[ NUM_SCV_OBJECTS ] = {

	EXTRAINDX_AFTERBURNER_DEVICE,	// 0
	EXTRAINDX_DUMB_PACK,			// 1
	EXTRAINDX_GUIDE_PACK,			// 2
	EXTRAINDX_LASERUPGRADE1,		// 3
	EXTRAINDX_LASERUPGRADE2,		// 4
	EXTRAINDX_HELIX_DEVICE,			// 5
	EXTRAINDX_LIGHTNING_DEVICE,		// 6
	EXTRAINDX_PHOTON_DEVICE,		// 7
	EXTRAINDX_MINE_PACK,			// 8
	EXTRAINDX_REPAIR_EXTRA,			// 9
	EXTRAINDX_INVULNERABILITY,		// 10
	EXTRAINDX_ENERGY_EXTRA,			// 11
	EXTRAINDX_SWARM_PACK,			// 12
	EXTRAINDX_EMPUPGRADE1,			// 13
	EXTRAINDX_EMPUPGRADE2,			// 14
};

int scv_drawing_order[ NUM_SCV_OBJECTS ] = { 1, 2, 3, 4, 5, 6, 7, 8, 13, 14, 9, 11, 0, 10, 12 };

static bams_t		scv_ring_spin_ofs		= 0x0000;
static bams_t		scv_ring_correction_ofs	= 0xdd00;
static int			scv_ring_slidepos 		= RING_POS_RIGHT;
static int			scv_ring_slidetarget	= 0;
static refframe_t	scv_ring_lastref 		= REFFRAME_INVALID;

int					scv_ring_spinning		= 0;
int					scv_object_selindx		= 0;

#define NUM_OBJECT_LINES		2

static const char *object_text[ NUM_SCV_OBJECTS ][ NUM_OBJECT_LINES ] = {

	{ "energy boost",
	  "collecting this will increase your energy"
	},

	{ "damage repair",
	  "this will repair any damages of your ship"
	},

	{ "emp upgrade 1",
	  "increases the efficiency of your EMP device"
	},

	{ "emp upgrade 2",
	  "gives you ultimate EMP power"
	},
	
	{ "proximity mine pack",
	  "collect these to get some proximity mines"
    },

	{ "photon cannon",
	  "collect this device to equip your ship with a photon cannon"
	},

	{ "lightning device",
	  "adds a lightning cannon to your ship"
	},

	{ "helix cannon",
	  "collect this to get the helix shot"
	},

	{ "laser upgrade 2",
	  "the ultimate laser upgrade"
	},

	{ "laser upgrade 1",
	  "improves strength and speed of your laser"
	},

	{ "guided missile pack",
	  "this will give you some guided missiles"
	},

	{ "dumb missile pack",
	  "increases your number of dumb missiles"
	},

	{ "swarm missile pack",
	  "contains very dangerous swarm missiles"
	},

	{ "invulnerability shield",
	  "this power-up makes your ship invulnerable"
	},

	{ "afterburner",
	  "gives your ship a speed boost if activated"
	},

};


// fade the background alpha to a specified target ----------------------------
//
PRIVATE
void DoViewerFading()
{
	if ( scv_bg_fadepos == scv_bg_fadetarget ) {
		scv_bg_lastref = REFFRAME_INVALID;
		return;
	}

	if ( scv_bg_fadepos < scv_bg_fadetarget ) {

		// fade in
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( scv_bg_lastref == REFFRAME_INVALID ) {
			scv_bg_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - scv_bg_lastref;
			for ( ; delta >= SCV_BG_FADE_SPEED; delta -= SCV_BG_FADE_SPEED ) {
				scv_bg_fadepos += SCV_BG_FADE_QUANTUM;
				if ( scv_bg_fadepos >= scv_bg_fadetarget ) {
					scv_bg_fadepos = scv_bg_fadetarget;
					scv_bg_lastref = REFFRAME_INVALID;
					break;
				}
				scv_bg_lastref += SCV_BG_FADE_SPEED;
			}
		}

	} else {

		// fade out
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( scv_bg_lastref == REFFRAME_INVALID ) {
			scv_bg_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - scv_bg_lastref;
			for ( ; delta >= SCV_BG_FADE_SPEED; delta -= SCV_BG_FADE_SPEED ) {
				scv_bg_fadepos -= SCV_BG_FADE_QUANTUM;
				if ( scv_bg_fadepos <= scv_bg_fadetarget ) {
					scv_bg_fadepos = scv_bg_fadetarget;
					scv_bg_lastref = REFFRAME_INVALID;
					break;
				}
				scv_bg_lastref += SCV_BG_FADE_SPEED;
			}
		}
	}
}


// draw spacecraft viewer background ------------------------------------------
//
PRIVATE
void DrawViewerBackground()
{
	DoViewerFading();

	// check whether actual drawing disabled
	if ( AUX_DISABLE_FLOATING_MENU_DRAWING )
		return;

#if ( SCV_BG_ALPHA_LOW == 0 )

	// skip drawing code if background viewer is completely faded out
	if ( scv_bg_fadepos == SCV_BG_ALPHA_LOW )
		return;

#endif

	texscreenrect_s	rect;

	dword itertype = iter_texrgba | iter_premulblend;
	rect.alpha     = scv_bg_fadepos;

	for ( int tid = 0; tid < 6; tid++ ) {

		switch ( tid ) {

			case 0:
				rect.x = 0;
				rect.y = 0;
				rect.w = scv_background_info[ tid ].owidth;
				rect.h = scv_background_info[ tid ].oheight;
				rect.scaled_w = (int)(scv_background_info[ tid ].width * Screen_Width);
				rect.scaled_h = (int)(scv_background_info[ tid ].height * Screen_Height);
				break;

			case 1:
				rect.x += rect.scaled_w;
				rect.w = scv_background_info[ tid ].owidth;
				rect.h = scv_background_info[ tid ].oheight;
				rect.scaled_w = (int)(scv_background_info[ tid ].width * Screen_Width);
				break;

			case 2:
				rect.x += rect.scaled_w;
				rect.w = scv_background_info[ tid ].owidth;
				rect.h = scv_background_info[ tid ].oheight;
				rect.scaled_w = (int)(Screen_Width - rect.scaled_w * 2);
				break;

			case 3:
				rect.x  = 0;
				rect.y += rect.scaled_h;
				rect.w = scv_background_info[ tid ].owidth;
				rect.h = scv_background_info[ tid ].oheight;
				rect.scaled_w = (int)(scv_background_info[ tid ].width * Screen_Width);
				rect.scaled_h = (int)(Screen_Height - rect.scaled_h);
				break;

			case 4:
				rect.x += rect.scaled_w;
				rect.w = scv_background_info[ tid ].owidth;
				rect.h = scv_background_info[ tid ].oheight;
				rect.scaled_w = (int)(scv_background_info[ tid ].width * Screen_Width);
				break;

			case 5:
				rect.x += rect.scaled_w;
				rect.w = scv_background_info[ tid ].owidth;
				rect.h = scv_background_info[ tid ].oheight;
				rect.scaled_w = (int)(Screen_Width - rect.scaled_w * 2);
				break;
		}

		rect.texofsx  = 0;
		rect.texofsy  = 0;
		rect.itertype = itertype;
		rect.texmap   = scv_background_info[ tid ].texmap;

		DRAW_TexturedScreenRect( &rect, NULL );
	}
}


// set fade target for background to high (in) --------------------------------
//
void FadeInBackground()
{
	scv_bg_fadetarget = SCV_BG_ALPHA_HIGH;
}


// set fade target for background to low (out) --------------------------------
//
void FadeOutBackground()
{
	scv_bg_fadetarget = SCV_BG_ALPHA_LOW;
}


// check if background is at its desired fade position ------------------------
//
int FadeFinishedBackground()
{
	return ( scv_bg_fadepos == scv_bg_fadetarget );
}


// slide the ship to its specifed target x position ---------------------------
//
PRIVATE
void DoSpacecraftSliding()
{
	if ( scv_ship_slidepos == scv_ship_slidetarget ) {
		scv_ship_lastref = REFFRAME_INVALID;
		return;
	}

	if ( scv_ship_slidepos < scv_ship_slidetarget ) {

		// slide right
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( scv_ship_lastref == REFFRAME_INVALID ) {
			scv_ship_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - scv_ship_lastref;
			for ( ; delta >= SCV_SHIP_SLIDE_SPEED; delta -= SCV_SHIP_SLIDE_SPEED ) {
				scv_ship_slidepos++;
				if ( scv_ship_slidepos >= scv_ship_slidetarget ) {
					scv_ship_slidepos = scv_ship_slidetarget;
					scv_ship_lastref  = REFFRAME_INVALID;
					break;
				}
				scv_ship_lastref += SCV_SHIP_SLIDE_SPEED;
			}
		}

	} else {

		// slide left
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( scv_ship_lastref == REFFRAME_INVALID ) {
			scv_ship_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - scv_ship_lastref;
			for ( ; delta >= SCV_SHIP_SLIDE_SPEED; delta -= SCV_SHIP_SLIDE_SPEED ) {
				scv_ship_slidepos--;
				if ( scv_ship_slidepos <= scv_ship_slidetarget ) {
					scv_ship_slidepos = scv_ship_slidetarget;
					scv_ship_lastref  = REFFRAME_INVALID;
					break;
				}
				scv_ship_lastref += SCV_SHIP_SLIDE_SPEED;
			}
		}
	}
}


// draw currently selected ship in spacecraft viewer --------------------------
//
PRIVATE
void DrawSpacecraft()
{
	DoSpacecraftSliding();

	// check whether actual drawing disabled
	if ( AUX_DISABLE_FLOATING_MENU_DRAWING )
		return;

	float shippos = SCV_SHIP_CENTER_X;
	if ( scv_ship_slidepos != -1 )
		shippos += m_sintab[ scv_ship_slidepos ];
	scv_ship_pos_matrx[ 0 ][ 3 ] = FLOAT_TO_GEOMV( shippos );

	ASSERT( cur_scv_ship < NumShipClasses );
	GenObject *shippo = ObjClasses[ ShipClasses[ cur_scv_ship ] ];

	bams_t objview_roty = SCV_OBJROTSPEED * CurScreenRefFrames;
	ObjRotY( scv_ship_pos_matrx, -objview_roty );

	// reorthogonalize position matrix
	if ( !AUX_DISABLE_REORTHO_IN_VIEWERS ) {
		ReOrthoNormMtx( scv_ship_pos_matrx );
	}

	pXmatrx curmatrx = scv_ship_pos_matrx;

#ifdef USE_SCV_SHIP_SCALE

	// pre-scale
	MtxMtxMUL( scv_ship_pos_matrx, scv_ship_scale_matrx, DestXmatrx );
	curmatrx = DestXmatrx;

	//FIXME:
	//NOTE:
	// ( SCV_SHIP_SCALE != 1.0f ) is currently only a hack
	// for screenshots. the problem ist that the matrix is
	// not orthogonal afterwards and backface-culling (as
	// well as bsp-traversal if used) depends on this
	// orthogonality.

#endif

	// copy matrix into all ships (ensures that all
	// ships have the same orientation after switching)
	for ( int sid = 0; sid < NumShipClasses; sid++ ) {
		memcpy( ObjClasses[ ShipClasses[ sid ] ]->ObjPosition, curmatrx, sizeof( Xmatrx ) );
	}

	// open direct object rendering
	R_DirectObjectRendering( 0x01 );

	// render object
	OBJ_AutoSelectObjectLod( shippo );
	R_ReCalcAndRenderObject( shippo, SpacecraftCamera );

	// close direct object rendering
	R_DirectObjectRendering( 0x00 );
}


// set slide target for ship to left (in) -------------------------------------
//
void SlideInShip()
{
	scv_ship_slidetarget = SCV_SHIP_LEFT;
}


// set slide target for ship to right (out) -----------------------------------
//
void SlideOutShip()
{
	scv_ship_slidetarget = SCV_SHIP_RIGHT;
}


// check if ship is at its desirect slide position ----------------------------
//
int SlideFinishedShip()
{
	return ( scv_ship_slidepos == scv_ship_slidetarget );
}


// ----------------------------------------------------------------------------
//
PRIVATE
void MaintainObjectRingSpinning()
{

	extern int rotkey_active_left;
	extern int rotkey_active_right;

	if ( scv_ring_spinning == 1 ) {

		scv_ring_spin_ofs += ( 25 * CurScreenRefFrames );
		if ( scv_ring_spin_ofs >= RING_ARC ) {
			if ( !rotkey_active_left )
				scv_ring_spinning  = 0;
			scv_ring_spin_ofs = 0x0000;
			if ( ++scv_object_selindx >= NUM_SCV_OBJECTS ) {
				scv_object_selindx = 0;
			}
		}

	} else if ( scv_ring_spinning == -1 ) {

		scv_ring_spin_ofs -= ( 25 * CurScreenRefFrames );
		if ( scv_ring_spin_ofs <= -RING_ARC ) {
			if ( !rotkey_active_right )
				scv_ring_spinning  = 0;
			scv_ring_spin_ofs = 0x0000;
			if ( --scv_object_selindx < 0 ) {
				scv_object_selindx = NUM_SCV_OBJECTS - 1;
			}
		}
	}
}


// slide the ring to its specifed target x position ---------------------------
//
PRIVATE
void DoObjectsRingSliding()
{
	if ( scv_ring_slidepos == scv_ring_slidetarget ) {
		scv_ring_lastref = REFFRAME_INVALID;
		return;
	}

	if ( scv_ring_slidepos < scv_ring_slidetarget ) {

		// slide right
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( scv_ring_lastref == REFFRAME_INVALID ) {
			scv_ring_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - scv_ring_lastref;
			for ( ; delta >= RING_SLIDE_SPEED; delta -= RING_SLIDE_SPEED ) {
				scv_ring_slidepos++;
				if ( scv_ring_slidepos >= scv_ring_slidetarget ) {
					scv_ring_slidepos = scv_ring_slidetarget;
					scv_ring_lastref  = REFFRAME_INVALID;
					break;
				}
				scv_ring_lastref += RING_SLIDE_SPEED;
			}
		}

	} else {

		// slide left
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( scv_ring_lastref == REFFRAME_INVALID ) {
			scv_ring_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - scv_ring_lastref;
			for ( ; delta >= RING_SLIDE_SPEED; delta -= RING_SLIDE_SPEED ) {
				scv_ring_slidepos--;
				if ( scv_ring_slidepos <= scv_ring_slidetarget ) {
					scv_ring_slidepos = scv_ring_slidetarget;
					scv_ring_lastref  = REFFRAME_INVALID;
					break;
				}
				scv_ring_lastref += RING_SLIDE_SPEED;
			}
		}
	}
}


// draw currently selected ship in spacecraft viewer --------------------------
//
void DrawObjectsRing()
{
	// spin object ring
	if ( scv_ring_spinning != 0 ) {
		MaintainObjectRingSpinning();
	}

	DoObjectsRingSliding();

	// check whether actual drawing disabled
	if ( AUX_DISABLE_FLOATING_MENU_DRAWING )
		return;

	// open direct object rendering
	R_DirectObjectRendering( 0x01 );

	for ( int oid = 0; oid < NUM_SCV_OBJECTS; oid++ ) {

		// fetch current object class id in correct drawing order
		int extraindex = scv_ring_objects[ scv_drawing_order[ ( oid ) % NUM_SCV_OBJECTS ] ];

		dword classid = ExtraClasses[ extraindex ];

		if ( classid == CLASS_ID_INVALID ) {
			continue;
		}
		
		GenObject *objpo = ObjClasses[ classid ];

		// translate object to its position on the ring
		bams_t angle = ( RING_ARC * ( oid + scv_object_selindx ) ) +
						scv_ring_spin_ofs + scv_ring_correction_ofs;
		sincosval_s sincosv;
		GetSinCos( angle, &sincosv );
		float	sinus   = GEOMV_TO_FLOAT( sincosv.sinval );
		float	cosinus = GEOMV_TO_FLOAT( sincosv.cosval );

		float objposx = SCV_SHIP_CENTER_X + ( sinus * RING_RADIUS ) - 5.0f;
		if ( scv_ring_slidepos != -1 )
			objposx += m_sintab[ scv_ring_slidepos ];

		objpo->ObjPosition[ 0 ][ 3 ] = FLOAT_TO_GEOMV( objposx );
		objpo->ObjPosition[ 1 ][ 3 ] = FLOAT_TO_GEOMV( SCV_SHIP_CENTER_Z - 27.0 );
		objpo->ObjPosition[ 2 ][ 3 ] = FLOAT_TO_GEOMV( SCV_SHIP_CENTER_Y + ( cosinus * RING_RADIUS ) );

		// rotate object
		bams_t objview_rot = SCV_OBJROTSPEED * CurScreenRefFrames;
		ObjRotX( objpo->ObjPosition, objview_rot );
		ObjRotY( objpo->ObjPosition, objview_rot );
		ObjRotZ( objpo->ObjPosition, objview_rot );

		// reorthogonalize position matrix
		if ( !AUX_DISABLE_REORTHO_IN_VIEWERS ) {
			ReOrthoNormMtx( objpo->ObjPosition );
		}

		// render object
		OBJ_AutoSelectObjectLod( objpo );
		R_ReCalcAndRenderObject( objpo, SpacecraftCamera );
	}

	// close direct object rendering
	R_DirectObjectRendering( 0x00 );
}


// set slide target for objects ring to left (in) -----------------------------
//
void SlideInRing()
{
	scv_ring_slidetarget = RING_POS_LEFT;
}


// set slide target for objects ring to right (out) ---------------------------
//
void SlideOutRing()
{
	scv_ring_slidetarget = RING_POS_RIGHT;
}


// check if object ring is at its desired slide position ----------------------
//
int SlideFinishedRing()
{
	return ( scv_ring_slidepos == scv_ring_slidetarget );
}


// type for writestring function pointer --------------------------------------
//
typedef void (*WSFP)( ... );

//NOTE:
// this declaration is in global scope because of the extern "C".
// only gcc needs the otherwise redundant curly braces.


// draw description text in spacecraft and object viewer ----------------------
//
void DrawViewerTexts()
{
	// check whether actual drawing disabled
	if ( AUX_DISABLE_FLOATING_MENU_DRAWING )
		return;

#if ( SCV_BG_ALPHA_LOW == 0 )

	// skip drawing code if background viewer is completely faded out
	if ( scv_bg_fadepos == SCV_BG_ALPHA_LOW )
		return;

#endif

	int old_alpha = PanelTextColor.A;
	PanelTextColor.A = (int)(scv_bg_fadepos * ( (float) old_alpha / (float) SCV_BG_ALPHA_HIGH ));

	extern int hud_line_dist;

	int chwidth  = CharsetInfo[ HUD_CHARSETNO ].width;
	int chheight = hud_line_dist;

	// determine whether translucency should be used
	int translucent = VID_TRANSLUCENCY_SUPPORTED;

	D_SetWStrContext( CharsetInfo[ HUD_CHARSETNO ].charsetpointer,
					CharsetInfo[ HUD_CHARSETNO ].geompointer,
					NULL,
					CharsetInfo[ HUD_CHARSETNO ].width,
					CharsetInfo[ HUD_CHARSETNO ].height );

	// write text transparent only for color depths below 32 bit per pixel
	WSFP wstrfp = ( translucent ) ?
					(WSFP) &D_WriteTrString :
					(WSFP) &D_WriteString;

	unsigned int	text_x = (unsigned int)(SCV_TEXT_X * Screen_Width);
	unsigned int text_y = (unsigned int)(SCV_TEXT_Y * Screen_Height);
	unsigned int text_w = (unsigned int)(SCV_TEXT_W * Screen_Width);
	unsigned int max_text_y = (unsigned int)(( SCV_TEXT_Y + SCV_TEXT_H ) * Screen_Height);

	char 	**text;
	int 	ship_id = 0;
	char 	*text_ptr = paste_str;

	// select text source
	switch ( cur_scv_mode ) {

		case SCVMODE_SPACECRAFT:
			text = scv_ship_text[ cur_scv_ship ];
			break;

		case SCVMODE_OBJECTSRING:
			text = (char **) object_text[ scv_object_selindx ];
			break;

		default:
			ASSERT( 0 );
			// avoid crash
			return;
	}

	// draw caption (always centered)
	if ( text[ CAPTION_LINE ] != NULL ) {

		strcpy( paste_str, text[ CAPTION_LINE ] );

		unsigned int len = strlen( paste_str ) * chwidth;
		unsigned int cnt = 0;
		int last_space = 0;

		if ( len > text_w ) {

			// draw wrapped text (multiple lines)
			while ( cnt <= strlen( text_ptr ) ) {
				if ( text_ptr[ cnt ] == 0x20 || text_ptr[ cnt ] == 0x00 || text_ptr[ cnt ] == '.' ) {
					if ( ( cnt * chwidth ) < text_w ) {
						last_space = cnt;
						cnt++;
					} else {
						text_ptr[ last_space ] = 0x00;
						int len2 = strlen( text_ptr ) * chwidth;
						int xofs = ( text_w - len2 ) / 2;

						wstrfp( text_ptr, text_x + xofs, text_y, TRTAB_PANELTEXT );
						text_y += chheight;
						text_ptr = &text_ptr[ last_space + 1 ];
						cnt = 0;
					}
				}
				else
					cnt++;
			}

			// draw rest of string
			int len2 = strlen( text_ptr ) * chwidth;
			int xofs = ( text_w - len2 ) / 2;
			wstrfp( text_ptr, text_x + xofs, text_y, TRTAB_PANELTEXT );
			text_y += chheight;


		} else {

			// draw single line
			int xofs = ( text_w - len ) / 2;

			wstrfp( text[ CAPTION_LINE ], text_x + xofs, text_y, TRTAB_PANELTEXT );
			text_y += chheight;
		}
	}

	// create empty line between caption and normal text
	text_y += chheight;

	// draw normal text (not centered)
	if ( text[ TEXT_LINE ] != NULL ) {

		strcpy( paste_str, text[ TEXT_LINE ] );

		unsigned int len = strlen( paste_str ) * chwidth;
		unsigned int cnt = 0;
		int last_space = 0;
		text_ptr = paste_str;

		if ( len > text_w ) {

			// draw wrapped text (multiple lines)
			while ( cnt <= strlen( text_ptr ) && text_y <= max_text_y  ) {
				if ( text_ptr[ cnt ] == 0x20 || text_ptr[ cnt ] == 0x00 ) {
					if ( ( cnt * chwidth ) < text_w ) {
						last_space = cnt;
						cnt++;
					} else {
						text_ptr[ last_space ] = 0x00;

						wstrfp( text_ptr, text_x, text_y, TRTAB_PANELTEXT );
						text_y += chheight;
						text_ptr = &text_ptr[ last_space + 1 ];
						cnt = 0;
					}
				}
				else
					cnt++;
			}
			if ( text_y <= max_text_y ) {
				// draw rest of string
				wstrfp( text_ptr, text_x, text_y, TRTAB_PANELTEXT );
				text_y += chheight;
			}

		} else {

			// draw single line
			wstrfp( text[ TEXT_LINE ], text_x, text_y, TRTAB_PANELTEXT );
			text_y += chheight;
		}
	}

	PanelTextColor.A = old_alpha;
}


// draw spacecraft viewer -----------------------------------------------------
//
void DrawSpacecraftViewer()
{
	// make sure textures are available
	if ( !AcquireScvTextures() )
		return;

	// draw content
	switch ( cur_scv_mode ) {

		case SCVMODE_SPACECRAFT:
			DrawSpacecraft();
			break;

		case SCVMODE_OBJECTSRING:
			DrawObjectsRing();
			break;

		default:
			ASSERT( 0 );
			break;
	}

	// draw frame
	DrawViewerBackground();

	// draw description texts
	DrawViewerTexts();
}


// key table for shipdesc command --------------------------------------------
//
key_value_s shipdesc_key_value[] = {

	{ "class",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "id",			NULL,	KEYVALFLAG_NONE				},
	{ "text",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "caption",	NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "clear",		NULL,	KEYVALFLAG_NONE				},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_SHIPDESC_CLASS,
	KEY_SHIPDESC_ID,
	KEY_SHIPDESC_TEXT,
	KEY_SHIPDESC_CAPTION,
	KEY_SHIPDESC_CLEAR,
};


// console command for defining ship description texts ("shipdesc") -----------
//
PRIVATE
int Cmd_SHIPDESC( char *classstr )
{
	//NOTE:
	//CONCOM:
	// shipdesc_command	::= 'shipdesc' <class_spec> [<text_spec>]
	//						[<caption_spec>] [<clear_spec>]
	// class_spec		::= 'class' <classname> | 'id' <classid>
	// text_spec		::= 'text' <textline>
	// caption_spec		::= 'caption' <captiontext>
	// clear_spec		::= 'clear' ['caption'|'text']

	ASSERT( classstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( classstr );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( shipdesc_key_value, classstr ) )
		return TRUE;

	// get object class (either name or id)
	dword objclass = ScanKeyValueObjClass( shipdesc_key_value, KEY_SHIPDESC_CLASS, KEY_SHIPDESC_ID );
	if ( objclass == CLASS_ID_INVALID ) {
		return TRUE;
	}

	if ( !OBJECT_TYPE_SHIP( ObjClasses[ objclass ] ) ) {
		CON_AddLine( no_ship_class );
		return TRUE;
	}

	// reverse look-up ship id
	int shipindex = ObjClassShipIndex[ objclass ];
	if ( shipindex == SHIPINDEX_NO_SHIP ) {
		CON_AddLine( no_registered_class );
		return TRUE;
	}
	ASSERT( (dword)shipindex < (dword)NumShipClasses );

	char **text = scv_ship_text[ shipindex ];

	//NOTE:
	// the clear key is checked before all others in order to
	// make adding new text after clearing the old one possible.

	// check for clearing
	char *cleararg = shipdesc_key_value[ KEY_SHIPDESC_CLEAR ].value;
	if ( cleararg != NULL ) {

		if ( strcmp( cleararg, "caption" ) == 0 ) {

			// free caption
			if ( text[ CAPTION_LINE ] != NULL ) {
				FREEMEM( text[ CAPTION_LINE ] );
				text[ CAPTION_LINE ] = NULL;
			}

		} else if ( strcmp( cleararg, "text" ) == 0 ) {

			// free text
			if ( text[ TEXT_LINE ] != NULL ) {
				FREEMEM( text[ TEXT_LINE ] );
				text[ TEXT_LINE ] = NULL;
				scv_ship_textcnt[ shipindex ] = 0;
			}
			ASSERT( scv_ship_textcnt[ shipindex ] == 0 );

		} else {

			CON_AddLine( clear_arg_invalid );
			return TRUE;
		}
	}

	// add a textline to the ship description text if specified
	char *textline = shipdesc_key_value[ KEY_SHIPDESC_TEXT ].value;
	if ( textline != NULL ) {

		// check if text will fit
		if ( ( scv_ship_textcnt[ shipindex ] + strlen( textline ) ) >= MAX_TEXT_LEN ) {
			CON_AddLine( text_too_large );
			return TRUE;
		}

		if ( text[ TEXT_LINE ] == NULL ) {

			text[ TEXT_LINE ] = (char *) ALLOCMEM( MAX_TEXT_LEN );
			if ( text[ TEXT_LINE ] == NULL )
				OUTOFMEM( 0 );
		}

		strcpy( text[ TEXT_LINE ] + scv_ship_textcnt[ shipindex ], textline );
		scv_ship_textcnt[ shipindex ] += strlen( textline );
	}

	// overwrite ship description text caption if specified
	char *caption = shipdesc_key_value[ KEY_SHIPDESC_CAPTION ].value;
	if ( caption != NULL ) {

		if ( text[ CAPTION_LINE ] != NULL )
			FREEMEM( text[ CAPTION_LINE ] );
		text[ CAPTION_LINE ] = (char *) ALLOCMEM( strlen( caption ) + 1 );

		strcpy( text[ CAPTION_LINE ], caption );
	}

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( M_VIEWER )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "shipdesc" command
	regcom.command	 = "shipdesc";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_SHIPDESC;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// init start matrix of spacecraft viewer camera
	MakeIdMatrx( SpacecraftCamera );
	CamRotX( SpacecraftCamera, SCV_CAM_PITCH );
	CamRotY( SpacecraftCamera, SCV_CAM_YAW );
	CamRotZ( SpacecraftCamera, SCV_CAM_ROLL );
	SpacecraftCamera[ 2 ][ 3 ] += SCV_CAM_DIST;

	// initialize ship description text array
	for ( int sid = 0; sid < MAX_SHIP_CLASSES; sid++ ) {
		scv_ship_text[ sid ][ CAPTION_LINE ] = NULL;
		scv_ship_text[ sid ][ TEXT_LINE ] 	 = NULL;
		scv_ship_textcnt[ sid ]				 = 0;
	}
}



