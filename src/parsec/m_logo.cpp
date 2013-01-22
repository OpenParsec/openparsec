/*
 * PARSEC - Rotating Logo
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
#include "sys_defs.h"
#include "vid_defs.h"

// local module header
#include "m_logo.h"

// proprietary module headers
#include "con_aux.h"
#include "e_draw.h"
#include "e_supp.h"
#include "m_main.h"


// flags
#define FPS_INDEPENDENT_FLOATLOGO_ROTATION



// logo animation frame info --------------------------------------------------
//
struct logo_anim_info_s {

	const char*	texname;
	TextureMap*	texmap;
};

static logo_anim_info_s logo_anim_info[] = {

	{ "logani00", NULL },
	{ "logani01", NULL },
	{ "logani02", NULL },
	{ "logani03", NULL },
	{ "logani04", NULL },
	{ "logani05", NULL },
	{ "logani06", NULL },
	{ "logani07", NULL },
	{ "logani08", NULL },
	{ "logani09", NULL },
	{ "logani10", NULL },
	{ "logani11", NULL },
	{ "logani12", NULL },
	{ "logani13", NULL },
	{ "logani14", NULL },
	{ "logani15", NULL },
	{ "logani16", NULL },
	{ "logani17", NULL },
	{ "logani18", NULL },
	{ "logani19", NULL },
	{ "logani20", NULL },
	{ "logani21", NULL },
	{ "logani22", NULL },
	{ "logani23", NULL },
	{ "logani24", NULL },
	{ "logani25", NULL },
	{ "logani26", NULL },
	{ "logani27", NULL },
	{ "logani28", NULL },
	{ "logani29", NULL },

	{ NULL, NULL }
};


static int			logo_anim_valid = FALSE;
static int			logo_anim_frame	= 0;
static refframe_t	logo_anim_count = REFFRAME_INVALID;


// logo geometry and timing ---------------------------------------------------
//
#define LOGO_WIDTH			256
#define LOGO_HEIGHT			114 //128
#define LOGO_XOFS			-5
#define LOGO_YOFS			15
#define LOGO_ALPHA			255

#define LOGO_ANIM_LEN		30
#define LOGO_ANIM_SPEED		40

#define LOGO_POS_RIGHT		0
#define LOGO_POS_LEFT		( m_sintab_size - 1 )

#define LOGO_SLIDE_SPEED	12


// try to acquire textures for logo animation ---------------------------------
//
PRIVATE
int AcquireLogoTextures()
{
	if ( logo_anim_valid )
		return TRUE;

	logo_anim_valid = TRUE;

	for ( int tid = 0; logo_anim_info[ tid ].texname; tid++ ) {

		TextureMap *texmap = FetchTextureMap( logo_anim_info[ tid ].texname );
		logo_anim_info[ tid ].texmap = texmap;

		if ( texmap == NULL ) {
			logo_anim_valid = FALSE;
		}
	}

	return logo_anim_valid;
}


// logo sliding ---------------------------------------------------------------
//
static int			logo_slidepos 		= LOGO_POS_RIGHT;
static int			logo_slidetarget	= LOGO_POS_LEFT;
static refframe_t	logo_lastref 		= REFFRAME_INVALID;


// slide the logo to its specifed target x position ---------------------------
//
PRIVATE
void DoLogoSliding()
{
	if ( logo_slidepos == logo_slidetarget ) {
		logo_lastref = REFFRAME_INVALID;
		return;
	}

	if ( logo_slidepos < logo_slidetarget ) {

		// slide right
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( logo_lastref == REFFRAME_INVALID ) {
			logo_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - logo_lastref;
			for ( ; delta >= LOGO_SLIDE_SPEED; delta -= LOGO_SLIDE_SPEED ) {
				logo_slidepos++;
				if ( logo_slidepos >= logo_slidetarget ) {
					logo_slidepos = logo_slidetarget;
					logo_lastref  = REFFRAME_INVALID;
					break;
				}
				logo_lastref += LOGO_SLIDE_SPEED;
			}
		}

	} else {

		// slide left
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( logo_lastref == REFFRAME_INVALID ) {
			logo_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - logo_lastref;
			for ( ; delta >= LOGO_SLIDE_SPEED; delta -= LOGO_SLIDE_SPEED ) {
				logo_slidepos--;
				if ( logo_slidepos <= logo_slidetarget ) {
					logo_slidepos = logo_slidetarget;
					logo_lastref  = REFFRAME_INVALID;
					break;
				}
				logo_lastref += LOGO_SLIDE_SPEED;
			}
		}
	}
}


// let the logo rotate (texture frame animation) ------------------------------
//
PRIVATE
void DoLogoRotation()
{

#ifdef FPS_INDEPENDENT_FLOATLOGO_ROTATION

	// determine next animation frame
	if ( AUX_DISABLE_FLOATLOGO_ROTATION ) {

		logo_anim_count = REFFRAME_INVALID;
		logo_anim_frame = 0;

	} else {

		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( logo_anim_count == REFFRAME_INVALID ) {
			logo_anim_count = refframecount;
		} else {
			refframe_t delta = refframecount - logo_anim_count;
			if ( delta > LOGO_ANIM_SPEED * 5 ) {
				delta = LOGO_ANIM_SPEED;
				logo_anim_count = refframecount - LOGO_ANIM_SPEED;
			}
			for ( ; delta >= LOGO_ANIM_SPEED; delta -= LOGO_ANIM_SPEED ) {
				logo_anim_frame++;
				if ( logo_anim_frame >= LOGO_ANIM_LEN )
					logo_anim_frame -= LOGO_ANIM_LEN;
				logo_anim_count += LOGO_ANIM_SPEED;
			}
		}
	}

#else

	//NOTE:
	// this logo animation is not totally frame rate independent.
	// there will never be any frames dropped due to a very
	// low frame rate. if the frame rate is very low, the logo
	// will rotate with one logo-frame per screen-frame.

	int			incframe      = 1;
	refframe_t	refframecount = SYSs_GetRefFrameCount();

	if ( logo_anim_count == REFFRAME_INVALID ) {
		logo_anim_count = refframecount;
		incframe = 0;
	}

	if ( refframecount - logo_anim_count <= LOGO_ANIM_SPEED )
		incframe = 0;
	else
		logo_anim_count = refframecount;

	if ( incframe )
		logo_anim_frame++;

	if ( ( logo_anim_frame >= LOGO_ANIM_LEN ) || AUX_DISABLE_FLOATLOGO_ROTATION ) {
		logo_anim_frame = 0;
	}

#endif

}


// draw animated logo ---------------------------------------------------------
//
void DrawMenuLogo()
{
	// make sure textures are available
	if ( !AcquireLogoTextures() )
		return;

	DoLogoSliding();
	DoLogoRotation();

	// check whether actual drawing disabled
	if ( AUX_DISABLE_FLOATING_MENU_DRAWING )
		return;

	texscreenrect_s rect;

	rect.x = Screen_Width - LOGO_WIDTH + LOGO_XOFS;
	if ( logo_slidepos != -1 )
		rect.x += m_sintab[ logo_slidepos ];
	rect.y = Screen_Height - LOGO_HEIGHT + LOGO_YOFS;
	rect.w = rect.scaled_w = LOGO_WIDTH;
	rect.h = rect.scaled_h = LOGO_HEIGHT;
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.itertype = iter_texrgba | iter_premulblend;
	rect.texmap   = logo_anim_info[ logo_anim_frame ].texmap;
	rect.alpha    = LOGO_ALPHA;

	DRAW_TexturedScreenRect( &rect, NULL );
}


// set slide target for logo to left (in) -------------------------------------
//
void SlideInLogo()
{
	logo_slidetarget = LOGO_POS_LEFT;
}


// set slide target for logo to right (out) -----------------------------------
//
void SlideOutLogo()
{
	logo_slidetarget = LOGO_POS_RIGHT;
}


// set position and slide target for logo to left (in) ------------------------
//
void MoveInLogo()
{
	logo_slidepos	 = LOGO_POS_LEFT;
	logo_slidetarget = logo_slidepos;
}


// set position and slide target for logo to right (out) ----------------------
//
void MoveOutLogo()
{
	logo_slidepos	 = LOGO_POS_RIGHT;
	logo_slidetarget = logo_slidepos;
}



