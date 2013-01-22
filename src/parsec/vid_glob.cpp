/*
 * PARSEC - Global Variables
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:36 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
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

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// subsystem headers
#include "vid_defs.h"

// local module header
#include "vid_glob.h"


resinfo_s::resinfo_s()
	: width(0), height(0)
{
	// nothing
}

resinfo_s::resinfo_s(word w, word h)
	: width(w), height(h)
{
	// nothing
}

void resinfo_s::set(word w, word h)
{
	width = w;
	height = h;
}

bool resinfo_s::isValid() const
{
	return width > 0 && height > 0;
}

bool resinfo_s::operator < (const resinfo_s & res) const
{
	return width * height < res.width * res.height;
}

bool resinfo_s::operator == (const resinfo_s & res) const
{
	return width == res.width && height == res.height;
}

bool resinfo_s::operator != (const resinfo_s & res) const
{
	return !(*this == res);
}


// table of relevant graphics mode numbers (built at startup) -----------------
//
std::vector<resinfo_s>		Resolutions;


int GetResolutionIndex(word xres, word yres)
{
	std::vector<resinfo_s>::iterator it;
	for (it = Resolutions.begin(); it != Resolutions.end(); ++it) {
		if (it->width == xres && it->height == yres)
			return std::distance(Resolutions.begin(), it); // return index of resolution in list
	}
	
	// resolution not found!
	return -1;
}


// some constant init values --------------------------------------------------
//
#define NEAR_VIEW_PLANE		FIXED_TO_GEOMV( 0x00013333 )
#define FAR_VIEW_PLANE		FIXED_TO_GEOMV( 0x18000000 )


// maximum bit depth possible on current display, set when initializing vid sys
//
int		MaxScreenBPP		= 32;


// display mode game screen uses (also: currently active mode) ----------------
//
resinfo_s GameScreenRes		= resinfo_s(800, 600);
int 	GameScreenBPP		= MODE_COL_32BPP;	// overwritten by base mode
int		GameScreenWindowed	= FALSE;			// fullscreen by default


// info about currently loaded data -------------------------------------------
//
int 	CurDataColorBits	= -1;				// force initial conversion
int 	CurDataDetail		= DETAIL_HIGH;


// options menu selections related to video -----------------------------------
//
resinfo_s Op_Resolution		= resinfo_s(800, 600);
int 	Op_ColorDepth		= COLMODE_TRUECOLOR;// overwritten by base mode
int		Op_WindowedMode		= FALSE;			// windowed mode (!full-screen)


// init values for above options if the base mode should be overridden --------
//
resinfo_s InitOp_Resolution	= resinfo_s();
int 	InitOp_ColorDepth	= -1;				// resolution and color depth
int		InitOp_WindowedMode	= -1;				// -1 means don't override
int		InitOp_FlipSynched	= -1;				// -1 means don't override


// current gamma correction value ---------------------------------------------
//
float	GammaCorrection		= 1.0f;		// default gamma


// flag if text mode active ---------------------------------------------------
//
int 	TextModeActive		= TRUE;		// start in text mode


// flag if vertical sync is enabled -------------------------------------------
//
int 	FlipSynched 		= TRUE;		// vertical synchronization


// display geometry -----------------------------------------------------------
//
int 	Screen_Width;
int 	Screen_Height;
int 	Screen_XOfs;
int 	Screen_YOfs;
int		Screen_BytesPerPixel;
int 	D_Value;						// must never be greater 65536
int 	Star_Siz;
int 	RObj_Siz;


// viewing frustum ------------------------------------------------------------
//
geomv_t		Near_View_Plane 	= NEAR_VIEW_PLANE;
geomv_t		Far_View_Plane		= FAR_VIEW_PLANE;
geomv_t		Criterion_X;
geomv_t		Criterion_Y;
Plane3		View_Volume[ 6 ];	// view-space (plane defs)
CullPlane3	Cull_Volume[ 6 ];	// view-space (planes and reject/accept points)

Plane3		World_ViewVolume[ 6 ];	// world-space
CullPlane3	World_CullVolume[ 6 ];	// world-space

Plane3		Object_ViewVolume[ 6 ];	// object-space
CullPlane3	Object_CullVolume[ 6 ];	// object-space


// flags with specific info about the current video subsystem -----------------
//
int		VidInfo_NumTextureUnits			= 16;
int		VidInfo_MaxTextureSize			= 1024;
int		VidInfo_MaxTextureLod			= TEXLOD_1024;
int		VidInfo_UseIterForDemoTexts		= TRUE;
int*	VidInfo_SupportedTexFormats		= NULL;



// module registration function -----------------------------------------------
//
REGISTER_MODULE( VID_GLOB )
{

}


