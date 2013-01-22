/*
 * PARSEC - Drawing Function Wrappers
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:22 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1999
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
#include "vid_defs.h"

// drawing subsystem
#include "d_bmap.h"
#include "d_iter.h"

// model header
#include "utl_model.h"

// local module header
#include "e_draw.h"

// proprietary module headers
#include "con_aux.h"
#include "e_color.h"
#include "e_supp.h"



// decoration texture definitions ---------------------------------------------
//
#define CORNER_WIDTH	16
#define CORNER_HEIGHT	16

enum {
	CORNER_LEFT_TOP,
	CORNER_RIGHT_TOP,
	CORNER_LEFT_BOTTOM,
	CORNER_RIGHT_BOTTOM,
};

const char *corner_texnames[ 4 ] = {
	"corner1",
	"corner2",
	"corner3",
	"corner4",
};


// draw decorations on corners of translucent panels --------------------------
//
void DRAW_PanelDecorations( sgrid_t putx, sgrid_t puty, int width, int height )
{
	if ( AUX_DISABLE_PANEL_DECORATIONS )
		return;

	int halfwidth = CORNER_WIDTH >> 1;

	texscreenrect_s rect;

	rect.itertype  = iter_texrgba | iter_specularadd;
	rect.alpha     = 255;
	rect.w = rect.scaled_w = CORNER_WIDTH;
	rect.h = rect.scaled_h = CORNER_HEIGHT;
	rect.texofsx = 0;
	rect.texofsy = 0;

	static TextureMap *texmap_lt = NULL;
	if ( texmap_lt == NULL ) {
		texmap_lt = FetchTextureMap( corner_texnames[ CORNER_LEFT_TOP ] );
	}
	if ( texmap_lt != NULL ) {

		rect.x = putx - halfwidth;
		rect.y = puty - halfwidth;
		rect.texmap = texmap_lt;

		DRAW_TexturedScreenRect( &rect, NULL );
	}

	static TextureMap *texmap_rt = NULL;
	if ( texmap_rt == NULL ) {
		texmap_rt = FetchTextureMap( corner_texnames[ CORNER_RIGHT_TOP ] );
	}
	if ( texmap_rt != NULL ) {

		rect.x = putx + width - halfwidth;
		rect.y = puty - halfwidth;
		rect.texmap = texmap_rt;

		DRAW_TexturedScreenRect( &rect, NULL );
	}

	static TextureMap *texmap_lb = NULL;
	if ( texmap_lb == NULL ) {
		texmap_lb = FetchTextureMap( corner_texnames[ CORNER_LEFT_BOTTOM ] );
	}
	if ( texmap_lb != NULL ) {

		rect.x = putx - halfwidth;
		rect.y = puty + height - halfwidth;
		rect.texmap = texmap_lb;

		DRAW_TexturedScreenRect( &rect, NULL );
	}

	static TextureMap *texmap_rb = NULL;
	if ( texmap_rb == NULL ) {
		texmap_rb = FetchTextureMap( corner_texnames[ CORNER_RIGHT_BOTTOM ] );
	}
	if ( texmap_rb != NULL ) {

		rect.x = putx + width - halfwidth;
		rect.y = puty + height - halfwidth;
		rect.texmap = texmap_rb;

		DRAW_TexturedScreenRect( &rect, NULL );
	}

	if ( width > CORNER_WIDTH ) {

		IterLine2 itline;
		itline.NumVerts  = 2;
		itline.flags	 = ITERFLAG_LS_DEFAULT;
		itline.itertype  = iter_rgba | iter_alphablend;
		itline.raststate = rast_default;
		itline.rastmask  = rast_nomask;

		itline.Vtxs[ 0 ].X 	   = INT_TO_RASTV( putx + halfwidth + 1 );
		itline.Vtxs[ 0 ].Y 	   = INT_TO_RASTV( puty - 3 );
		itline.Vtxs[ 0 ].Z	   = RASTV_0;
		itline.Vtxs[ 0 ].R 	   = PanelTextColor.R;
		itline.Vtxs[ 0 ].G 	   = PanelTextColor.G;
		itline.Vtxs[ 0 ].B 	   = PanelTextColor.B;
		itline.Vtxs[ 0 ].A 	   = PanelTextColor.A;
		itline.Vtxs[ 0 ].flags = ITERVTXFLAG_NONE;

		itline.Vtxs[ 1 ].X 	   = INT_TO_RASTV( putx + width - halfwidth + 1 );
		itline.Vtxs[ 1 ].Y 	   = INT_TO_RASTV( puty - 3 );
		itline.Vtxs[ 1 ].Z	   = RASTV_0;
		itline.Vtxs[ 1 ].R 	   = PanelTextColor.R;
		itline.Vtxs[ 1 ].G 	   = PanelTextColor.G;
		itline.Vtxs[ 1 ].B 	   = PanelTextColor.B;
		itline.Vtxs[ 1 ].A 	   = PanelTextColor.A;
		itline.Vtxs[ 1 ].flags = ITERVTXFLAG_NONE;

		Rectangle2 cliprect;

		cliprect.left   = 0;
		cliprect.right  = Screen_Width;
		cliprect.top    = 0;
		cliprect.bottom = Screen_Height;

		IterLine2 *clipline = CLIP_RectangleIterLine2( &itline, &cliprect );
		if ( clipline != NULL )
			D_DrawIterLine2( clipline );

		itline.Vtxs[ 0 ].Y 	   = INT_TO_RASTV( puty - 5 );
		itline.Vtxs[ 1 ].Y 	   = INT_TO_RASTV( puty - 5 );

		clipline = CLIP_RectangleIterLine2( &itline, &cliprect );
		if ( clipline != NULL )
			D_DrawIterLine2( clipline );

		itline.Vtxs[ 0 ].Y 	   = INT_TO_RASTV( puty + height + 4 );
		itline.Vtxs[ 1 ].Y 	   = INT_TO_RASTV( puty + height + 4 );

		clipline = CLIP_RectangleIterLine2( &itline, &cliprect );
		if ( clipline != NULL )
			D_DrawIterLine2( clipline );

		itline.Vtxs[ 0 ].Y 	   = INT_TO_RASTV( puty + height + 6 );
		itline.Vtxs[ 1 ].Y 	   = INT_TO_RASTV( puty + height + 6 );

		clipline = CLIP_RectangleIterLine2( &itline, &cliprect );
		if ( clipline != NULL )
			D_DrawIterLine2( clipline );

		itline.Vtxs[ 0 ].X 	   = INT_TO_RASTV( putx - 3 );
		itline.Vtxs[ 0 ].Y 	   = INT_TO_RASTV( puty + halfwidth + 1 );
		itline.Vtxs[ 1 ].X 	   = INT_TO_RASTV( putx - 3 );
		itline.Vtxs[ 1 ].Y 	   = INT_TO_RASTV( puty + height - halfwidth + 1 );

		clipline = CLIP_RectangleIterLine2( &itline, &cliprect );
		if ( clipline != NULL )
			D_DrawIterLine2( clipline );

		itline.Vtxs[ 0 ].X 	   = INT_TO_RASTV( putx - 5 );
		itline.Vtxs[ 1 ].X 	   = INT_TO_RASTV( putx - 5 );

		clipline = CLIP_RectangleIterLine2( &itline, &cliprect );
		if ( clipline != NULL )
			D_DrawIterLine2( clipline );

		itline.Vtxs[ 0 ].X 	   = INT_TO_RASTV( putx + width + 3 );
		itline.Vtxs[ 1 ].X 	   = INT_TO_RASTV( putx + width + 3 );

		clipline = CLIP_RectangleIterLine2( &itline, &cliprect );
		if ( clipline != NULL )
			D_DrawIterLine2( clipline );

		itline.Vtxs[ 0 ].X 	   = INT_TO_RASTV( putx + width + 5 );
		itline.Vtxs[ 1 ].X 	   = INT_TO_RASTV( putx + width + 5 );

		clipline = CLIP_RectangleIterLine2( &itline, &cliprect );
		if ( clipline != NULL )
			D_DrawIterLine2( clipline );
	}
}


// draw transparent rectangle with clipping -----------------------------------
//
void DRAW_ClippedTrRect( sgrid_t putx, sgrid_t puty, int width, int height, dword brightx )
{
	// reject if entirely invisible
	if ( ( putx >= Screen_Width ) || ( puty >= Screen_Height ) )
		return;

	// clip rectangle to the left
	if ( putx < 0 ) {
		width += putx;
		putx = 0;
	}

	// clip rectangle to the top
	if ( puty < 0 ) {
		height += puty;
		puty = 0;
	}

	// reject if entirely invisible
	if ( ( width <= 0 ) || ( height <= 0 ) )
		return;

	// clip rectangle to the right
	if ( ( putx + width ) > Screen_Width ) {
		width = Screen_Width - putx;
	}

	// clip rectangle to the bottom
	if ( ( puty + height ) > Screen_Height ) {
		height = Screen_Height - puty;
	}

	D_DrawTrRect( putx, puty, width, height, brightx );

	DRAW_PanelDecorations( putx, puty, width, height );
}


// draw translucent rectangle -------------------------------------------------
//
void DRAW_ColoredScreenRect( colscreenrect_s *rect )
{
	IterRectangle2 itrect;

	itrect.Vtxs[ 0 ].X = INT_TO_RASTV( rect->x );
	itrect.Vtxs[ 0 ].Y = INT_TO_RASTV( rect->y );
	itrect.Vtxs[ 0 ].Z = RASTV_1;
	itrect.Vtxs[ 0 ].R = rect->color.R;
	itrect.Vtxs[ 0 ].G = rect->color.G;
	itrect.Vtxs[ 0 ].B = rect->color.B;
	itrect.Vtxs[ 0 ].A = rect->color.A;

	itrect.Vtxs[ 1 ].X = INT_TO_RASTV( rect->x + rect->w );
	itrect.Vtxs[ 1 ].Y = INT_TO_RASTV( rect->y );
	itrect.Vtxs[ 1 ].Z = RASTV_1;
	itrect.Vtxs[ 1 ].R = itrect.Vtxs[ 0 ].R;
	itrect.Vtxs[ 1 ].G = itrect.Vtxs[ 0 ].G;
	itrect.Vtxs[ 1 ].B = itrect.Vtxs[ 0 ].B;
	itrect.Vtxs[ 1 ].A = itrect.Vtxs[ 0 ].A;

	itrect.Vtxs[ 2 ].X = INT_TO_RASTV( rect->x + rect->w );
	itrect.Vtxs[ 2 ].Y = INT_TO_RASTV( rect->y + rect->h );
	itrect.Vtxs[ 2 ].Z = RASTV_1;
	itrect.Vtxs[ 2 ].R = itrect.Vtxs[ 0 ].R;
	itrect.Vtxs[ 2 ].G = itrect.Vtxs[ 0 ].G;
	itrect.Vtxs[ 2 ].B = itrect.Vtxs[ 0 ].B;
	itrect.Vtxs[ 2 ].A = itrect.Vtxs[ 0 ].A;

	itrect.Vtxs[ 3 ].X = INT_TO_RASTV( rect->x );
	itrect.Vtxs[ 3 ].Y = INT_TO_RASTV( rect->y + rect->h );
	itrect.Vtxs[ 3 ].Z = RASTV_1;
	itrect.Vtxs[ 3 ].R = itrect.Vtxs[ 0 ].R;
	itrect.Vtxs[ 3 ].G = itrect.Vtxs[ 0 ].G;
	itrect.Vtxs[ 3 ].B = itrect.Vtxs[ 0 ].B;
	itrect.Vtxs[ 3 ].A = itrect.Vtxs[ 0 ].A;

	itrect.flags	 = ITERFLAG_NONE;
	itrect.itertype  = rect->itertype;
	itrect.raststate = rast_default;
	itrect.rastmask  = rast_nomask;
	itrect.texmap	 = NULL;

	D_DrawIterRectangle2( &itrect );
}


// draw (clipped) rectangular texture in 2-D ----------------------------------
//
void DRAW_TexturedScreenRect( texscreenrect_s *rect, Rectangle2 *cliprect )
{
	ASSERT( rect != NULL );
//	ASSERT( cliprect != NULL );

	//NOTE:
	// if ( cliprect == NULL ) no clipping will be performed.
	// in this case the rectangle MUST lie entirely the screen
	// extents.

	IterRectangle2 itrect;

	// to avoid texture seams due to filtering on all drivers
	// (especially the G400 needs this)
	geomv_t texbrdcorr = FLOAT_TO_GEOMV( 0.0 );

	itrect.Vtxs[ 0 ].X = INT_TO_RASTV( rect->x );
	itrect.Vtxs[ 0 ].Y = INT_TO_RASTV( rect->y );
	itrect.Vtxs[ 0 ].Z = RASTV_1;
	itrect.Vtxs[ 0 ].W = GEOMV_1;
	itrect.Vtxs[ 0 ].U = INT_TO_GEOMV( rect->texofsx );
	itrect.Vtxs[ 0 ].V = INT_TO_GEOMV( rect->texofsy );
	itrect.Vtxs[ 0 ].R = rect->alpha;
	itrect.Vtxs[ 0 ].G = rect->alpha;
	itrect.Vtxs[ 0 ].B = rect->alpha;
	itrect.Vtxs[ 0 ].A = rect->alpha;

	itrect.Vtxs[ 1 ].X = INT_TO_RASTV( rect->x + rect->scaled_w );
	itrect.Vtxs[ 1 ].Y = INT_TO_RASTV( rect->y );
	itrect.Vtxs[ 1 ].Z = itrect.Vtxs[ 0 ].Z;
	itrect.Vtxs[ 1 ].W = itrect.Vtxs[ 0 ].W;
	itrect.Vtxs[ 1 ].U = INT_TO_GEOMV( rect->texofsx + rect->w ) - texbrdcorr;
	itrect.Vtxs[ 1 ].V = INT_TO_GEOMV( rect->texofsy );
	itrect.Vtxs[ 1 ].R = itrect.Vtxs[ 0 ].R;
	itrect.Vtxs[ 1 ].G = itrect.Vtxs[ 0 ].G;
	itrect.Vtxs[ 1 ].B = itrect.Vtxs[ 0 ].B;
	itrect.Vtxs[ 1 ].A = itrect.Vtxs[ 0 ].A;

	itrect.Vtxs[ 2 ].X = INT_TO_RASTV( rect->x + rect->scaled_w );
	itrect.Vtxs[ 2 ].Y = INT_TO_RASTV( rect->y + rect->scaled_h );
	itrect.Vtxs[ 2 ].Z = itrect.Vtxs[ 0 ].Z;
	itrect.Vtxs[ 2 ].W = itrect.Vtxs[ 0 ].W;
	itrect.Vtxs[ 2 ].U = INT_TO_GEOMV( rect->texofsx + rect->w ) - texbrdcorr;
	itrect.Vtxs[ 2 ].V = INT_TO_GEOMV( rect->texofsy + rect->h ) - texbrdcorr;
	itrect.Vtxs[ 2 ].R = itrect.Vtxs[ 0 ].R;
	itrect.Vtxs[ 2 ].G = itrect.Vtxs[ 0 ].G;
	itrect.Vtxs[ 2 ].B = itrect.Vtxs[ 0 ].B;
	itrect.Vtxs[ 2 ].A = itrect.Vtxs[ 0 ].A;

	itrect.Vtxs[ 3 ].X = INT_TO_RASTV( rect->x );
	itrect.Vtxs[ 3 ].Y = INT_TO_RASTV( rect->y + rect->scaled_h );
	itrect.Vtxs[ 3 ].Z = itrect.Vtxs[ 0 ].Z;
	itrect.Vtxs[ 3 ].W = itrect.Vtxs[ 0 ].W;
	itrect.Vtxs[ 3 ].U = INT_TO_GEOMV( rect->texofsx );
	itrect.Vtxs[ 3 ].V = INT_TO_GEOMV( rect->texofsy + rect->h ) - texbrdcorr;
	itrect.Vtxs[ 3 ].R = itrect.Vtxs[ 0 ].R;
	itrect.Vtxs[ 3 ].G = itrect.Vtxs[ 0 ].G;
	itrect.Vtxs[ 3 ].B = itrect.Vtxs[ 0 ].B;
	itrect.Vtxs[ 3 ].A = itrect.Vtxs[ 0 ].A;

	itrect.flags	 = ITERFLAG_NONE;
	itrect.itertype  = rect->itertype;
	itrect.raststate = rast_texclamp;
	itrect.rastmask  = rast_nomask;
	itrect.texmap	 = rect->texmap;

	// if cliprect == NULL no clipping should be performed
	if ( cliprect != NULL ) {

		float xscale = ( (float) rect->w ) / ( rect->scaled_w );
		float yscale = ( (float) rect->h ) / ( rect->scaled_h );

		// reject if completely invisible
		if ( itrect.Vtxs[ 0 ].X > cliprect->right ||
			 itrect.Vtxs[ 0 ].Y > cliprect->bottom ||
			 itrect.Vtxs[ 1 ].X < cliprect->left ||
			 itrect.Vtxs[ 2 ].Y < cliprect->top ) {

			return;
		}

		// clip against left edge
		if ( itrect.Vtxs[ 0 ].X < cliprect->left ) {

			rastv_t offs = cliprect->left - itrect.Vtxs[ 0 ].X;
			itrect.Vtxs[ 0 ].U += offs * xscale;
			itrect.Vtxs[ 3 ].U += offs * xscale;
			itrect.Vtxs[ 0 ].X = cliprect->left;
			itrect.Vtxs[ 3 ].X = cliprect->left;
		}

		// clip against top edge
		if ( itrect.Vtxs[ 0 ].Y < cliprect->top ) {

			rastv_t offs = cliprect->top - itrect.Vtxs[ 0 ].Y;
			itrect.Vtxs[ 0 ].V += offs * yscale;
			itrect.Vtxs[ 1 ].V += offs * yscale;
			itrect.Vtxs[ 0 ].Y = cliprect->top;
			itrect.Vtxs[ 1 ].Y = cliprect->top;
		}

		// clip against right edge
		if ( itrect.Vtxs[ 1 ].X > cliprect->right ) {

			rastv_t offs = itrect.Vtxs[ 1 ].X - cliprect->right;
			itrect.Vtxs[ 1 ].U -= offs * xscale;
			itrect.Vtxs[ 2 ].U -= offs * xscale;
			itrect.Vtxs[ 1 ].X = cliprect->right;
			itrect.Vtxs[ 2 ].X = cliprect->right;
		}

		// clip against bottom edge
		if ( itrect.Vtxs[ 2 ].Y > cliprect->bottom ) {

			rastv_t offs = itrect.Vtxs[ 2 ].Y - cliprect->bottom;
			itrect.Vtxs[ 2 ].V -= offs * yscale;
			itrect.Vtxs[ 3 ].V -= offs * yscale;
			itrect.Vtxs[ 2 ].Y = cliprect->bottom;
			itrect.Vtxs[ 3 ].Y = cliprect->bottom;
		}
	}

	int oldwrapmode = AUX_DISABLE_TEXTURE_WRAPPING;
	AUX_DISABLE_TEXTURE_WRAPPING = 1;

	D_DrawIterRectangle2( &itrect );

	AUX_DISABLE_TEXTURE_WRAPPING = oldwrapmode;
}



