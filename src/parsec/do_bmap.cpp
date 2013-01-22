/*
 * PARSEC - Bitmap Drawing Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:22 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999
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

// subsystem linkage info
#include "linkinfo.h"

// local module header
#include "do_bmap.h"

// proprietary module headers
#include "con_aux.h"
#include "ro_api.h"
#include "ro_supp.h"


//char*	GlobalColXlat;
//char*	BrightxMap;


// put bitmap without clipping; check transparency ----------------------------
//
void D_PutTrBitmap( char *bitmap, dword width, dword height, ugrid_t putx, ugrid_t puty )
{
	ASSERT( bitmap != NULL );

	GLTexInfo texinfo;
	texinfo.texmap = NULL;
	texinfo.data   = bitmap;
	texinfo.width  = width;
	texinfo.height = height;
	texinfo.format = TEXFMT_RGBA_8888;

	// ensure filtering is turned off
	int filtmode = AUX_DISABLE_POLYGON_FILTERING;
	AUX_DISABLE_POLYGON_FILTERING = 1;

	// this also fills in texinfo.coscale
	RO_SelectTexelSource2( &texinfo );

	// restore filter state
	AUX_DISABLE_POLYGON_FILTERING = filtmode;

	// configure rasterizer
	dword itertype	= iter_texonly | iter_alphablend;
	dword raststate	= rast_nozbuffer | rast_chromakeyon | rast_mask_texwrap;
	dword rastmask	= rast_nomask;

	RO_InitRasterizerState( itertype, raststate, rastmask );
	RO_TextureCombineState( texcomb_decal );

	// draw rectangle (z=none, color=none)
	RO_Render2DRectangle( putx, puty,
						  width * texinfo.coscale, height * texinfo.coscale,
						  width, height, 0, NULL );

	// set rasterizer state to default
	RO_DefaultRasterizerState();
}


// put bitmap without clipping; check transparency; use color map -------------
//
void D_PutTrMapBitmap( char *bitmap, char *maps, dword width, dword height, ugrid_t putx, ugrid_t puty, dword color )
{
	//TODO:
}


// put bitmap and clip against screen extents ---------------------------------
//
void D_PutClipBitmap( char *bitmap, dword width, dword height, sgrid_t putx, sgrid_t puty )
{
	ASSERT( bitmap != NULL );

	dword dstw = width;
	dword dsth = height;

	//FIXME:
/*
	for ( int bindx = 0; ; bindx++ )
		if ( BitmapInfo[ bindx ].bitmappointer == bitmap )
			break;

	if ( bindx == BM_BIGPLANET1 ) {
		bindx = BM_MEDPLANET1;
		width  /= 2;
		height /= 2;
	}

	bitmap = BitmapInfo[ bindx ].loadeddata;
*/
	GLTexInfo texinfo;
	texinfo.texmap = NULL;
	texinfo.data   = bitmap;
	texinfo.width  = width;
	texinfo.height = height;
	texinfo.format = TEXFMT_RGBA_8888;

	// this also fills in texinfo.coscale
	RO_SelectTexelSource2( &texinfo );

	// configure rasterizer
	dword itertype	= iter_texonly | iter_overwrite;
	dword raststate	= rast_chromakeyoff | rast_mask_texwrap;
	dword rastmask	= rast_mask_zbuffer;

	RO_InitRasterizerState( itertype, raststate, rastmask );
	RO_TextureCombineState( texcomb_decal );

	// draw rectangle (z=none, color=none)
	RO_Render2DRectangle( putx, puty,
						  width * texinfo.coscale, height * texinfo.coscale,
						  dstw, dsth, 0, NULL );

	// set rasterizer state to default
	RO_DefaultRasterizerState();
}


// put transparent bitmap and clip against screen extents ---------------------
//
void D_PutTrClipBitmap( char *bitmap, dword width, dword height, sgrid_t putx, sgrid_t puty )
{
	ASSERT( bitmap != NULL );

	GLTexInfo texinfo;
	texinfo.texmap = NULL;
	texinfo.data   = bitmap;
	texinfo.width  = width;
	texinfo.height = height;
	texinfo.format = TEXFMT_RGBA_8888;

	// this also fills in texinfo.coscale
	RO_SelectTexelSource2( &texinfo );

	// configure rasterizer
	dword itertype	= iter_texonly | iter_alphablend;
	dword raststate	= rast_nozbuffer | rast_chromakeyon | rast_mask_texwrap;
	dword rastmask	= rast_nomask;

	RO_InitRasterizerState( itertype, raststate, rastmask );
	RO_TextureCombineState( texcomb_decal );

	// draw rectangle (z=none, color=none)
	RO_Render2DRectangle( putx, puty,
						  width * texinfo.coscale, height * texinfo.coscale,
						  width, height, 0, NULL );

	// set rasterizer state to default
	RO_DefaultRasterizerState();
}


// ----------------------------------------------------------------------------
//
static int no_depth_buffer = 0;


// put clipped and scaled transparent bitmap (check/alter z-buffer) -----------
//
void D_PutSTCBitmapZ( char *bitmap, dword srcw, dword srch, dword dstw, dword dsth, sgrid_t putx, sgrid_t puty, dword zvalue )
{
	ASSERT( bitmap != NULL );

	// vertical culling
	if ( puty + (int)dsth <= 0 )
		return;
	if ( puty >= Screen_Height )
		return;

	// horizontal culling
	if ( putx + (int)dstw <= 0 )
		return;
	if ( putx >= Screen_Width )
		return;

// --

	GLTexInfo texinfo;
	texinfo.texmap = NULL;
	texinfo.data   = bitmap;
	texinfo.width  = srcw;
	texinfo.height = srch;
	texinfo.format = TEXFMT_RGBA_8888;

	// this also fills in texinfo.coscale
	RO_SelectTexelSource2( &texinfo );

	// save zcmp and zwrite state
	int zcmpstate   = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();

	// determine if depth buffer should be turned on
	dword rastzctrl = no_depth_buffer ? rast_nozbuffer : rast_zbuffer;

	// configure rasterizer
	dword itertype	= iter_texconsta | iter_overwrite; // iter_alphablend;
	dword raststate	= rastzctrl | rast_chromakeyon | rast_mask_texwrap;
	dword rastmask	= rast_nomask;

	RO_InitRasterizerState( itertype, raststate, rastmask );
	RO_TextureCombineState( texcomb_decal );

	// set constant (rgba)
	if ( bitmap == BitmapInfo[ BM_LIGHTNING1 ].bitmappointer ) {
		glColor4ub( 0xff, 0xff, 0xff, 0xff );
	} else if ( bitmap == BitmapInfo[ BM_SHIELD1 ].bitmappointer ) {
		glColor4ub( 0xff, 0xff, 0xff, 0xe8 );
	} else if ( bitmap == BitmapInfo[ BM_SHIELD2 ].bitmappointer ) {
		glColor4ub( 0xff, 0xff, 0xff, 0xe8 );
	} else if ( bitmap == BitmapInfo[ BM_FIREBALL3 ].bitmappointer ) {
		glColor4ub( 0xff, 0xff, 0xff, 0xc0 );
	} else {
		glColor4ub( 0xff, 0xff, 0xff, 0x90 );
	}

	RO_AlphaFunc( GL_GREATER, 0.0f );
	RO_AlphaTest( TRUE );

	// draw rectangle (z=zvalue, color=none)
	RO_Render2DRectangle( putx, puty,
						  srcw * texinfo.coscale, srch * texinfo.coscale,
						  dstw, dsth, zvalue, NULL );

	RO_AlphaTest( FALSE );

	// set rasterizer state to default
	RO_DefaultRasterizerState();

	// restore zcmp and zwrite state
	if ( no_depth_buffer ) {
		if ( zcmpstate || zwritestate )
			RO_EnableDepthBuffer( zcmpstate, zwritestate );
	} else {
		if ( !zcmpstate || !zwritestate )
			RO_DisableDepthBuffer( !zcmpstate, !zwritestate );
	}
}


// put clipped and scaled transparent bitmap ----------------------------------
//
void D_PutSTCBitmap( char *bitmap, dword srcw, dword srch, dword dstw, dword dsth, sgrid_t putx, sgrid_t puty )
{
	no_depth_buffer = 1;

	D_PutSTCBitmapZ( bitmap, srcw, srch, dstw, dsth, putx, puty, 0 );

	no_depth_buffer = 0;
}


// draw translucent rectangle -------------------------------------------------
//
void D_DrawTrRect( ugrid_t putx, ugrid_t puty, dword width, dword height, dword brightx )
{
	// configure rasterizer
	dword itertype	= iter_rgba | iter_alphablend;
	dword raststate	= rast_chromakeyoff;
	dword rastmask	= rast_mask_zbuffer | rast_mask_texclamp |
					  rast_mask_mipmap  | rast_mask_texfilter;
	RO_InitRasterizerState( itertype, raststate, rastmask );

	//NOTE:
	// parameter brightx is not used anymore. instead, the
	// global PanelBackColor determines RGBA.

	glColor4ub( PanelBackColor.R, PanelBackColor.G, PanelBackColor.B, PanelBackColor.A );
	
	GLshort vertices[] = {
		putx, puty,
		putx, puty + height,
		putx + width, puty,
		putx + width, puty + height,
	};
	
	RO_ClientState(VTXARRAY_VERTICES);
	
	RO_ArrayMakeCurrent(VTXPTRS_NONE, NULL);
	glVertexPointer(2, GL_SHORT, 0, vertices);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// set rasterizer state to default
	RO_DefaultRasterizerState();
}



