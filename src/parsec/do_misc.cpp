/*
 * PARSEC - Miscellaneous Drawing Code
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
#include "d_misc.h"

// local module header
#include "do_misc.h"

// proprietary module headers
#include "e_color.h"
#include "ro_api.h"
#include "ro_supp.h"


// draw square with single color ----------------------------------------------
//
int D_DrawSquare( ugrid_t xko, ugrid_t yko, visual_t col, dword siz )
{
	// configure rasterizer
	dword itertype	= iter_rgb;
	dword raststate	= rast_chromakeyoff;
	dword rastmask	= rast_mask_zbuffer | rast_mask_texclamp |
					  rast_mask_mipmap  | rast_mask_texfilter;
	RO_InitRasterizerState( itertype, raststate, rastmask );

	// set color
	colrgba_s colrgba;
	VisualToRGBA( &colrgba, col );
	glColor4ub( (GLubyte)colrgba.R, (GLubyte)colrgba.G, (GLubyte)colrgba.B, 255 );

	RO_PointSize( (GLfloat)siz );
	
	GLshort vertices[] = {xko, yko};
	
	RO_ClientState(VTXARRAY_VERTICES);
	
	RO_ArrayMakeCurrent(VTXPTRS_NONE, NULL);
	glVertexPointer(2, GL_SHORT, 0, vertices);
	
	glDrawArrays(GL_POINTS, 0, 1);

	// set rasterizer state to default
	RO_DefaultRasterizerState();

	return 1;
}


// draw square of homogeneous color taking the z-buffer into account ----------
//
int D_DrawSquareZ( ugrid_t xko, ugrid_t yko, visual_t col, dword siz, dword zvalue )
{
	if ( (int)xko < 0 )
		return 0;
	if ( (int)yko < 0 )
		return 0;

	// save zcmp and zwrite state
	int zcmpstate   = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();

	// configure rasterizer
	dword itertype	= iter_rgba | iter_alphablend;
	dword raststate	= rast_zbuffer | rast_chromakeyoff;
	dword rastmask	= rast_mask_texwrap | rast_mask_mipmap |
					  rast_mask_texfilter;
	RO_InitRasterizerState( itertype, raststate, rastmask );

	// set color
	colrgba_s colrgba;
	VisualToRGBA( &colrgba, col );
	glColor4ub( (GLubyte)colrgba.R, (GLubyte)colrgba.G, (GLubyte)colrgba.B, 255 );

	RO_PointSize( siz );
	
	GLshort vertices[] = {xko, yko, 0}; // TODO: zvalue
	
	RO_ClientState(VTXARRAY_VERTICES);
	
	RO_ArrayMakeCurrent(VTXPTRS_NONE, NULL);
	glVertexPointer(3, GL_SHORT, 0, vertices);
	
	glDrawArrays(GL_POINTS, 0, 1);

	// set rasterizer state to default
	RO_DefaultRasterizerState();

	// restore zcmp and zwrite state
	if ( !zcmpstate || !zwritestate )
		RO_DisableDepthBuffer( !zcmpstate, !zwritestate );

	return 1;
}


// draw radar object ----------------------------------------------------------
//
void D_DrawRadarObj( ugrid_t xko, ugrid_t yko, visual_t col )
{
	// configure rasterizer
	dword itertype	= iter_rgb;
	dword raststate	= rast_chromakeyoff;
	dword rastmask	= rast_mask_zbuffer | rast_mask_texclamp |
					  rast_mask_mipmap  | rast_mask_texfilter;
	RO_InitRasterizerState( itertype, raststate, rastmask );

	// set color
	colrgba_s colrgba;
	VisualToRGBA( &colrgba, col );
	glColor4ub( (GLubyte)colrgba.R, (GLubyte)colrgba.G, (GLubyte)colrgba.B, 255 );

	RO_PointSize( 2.0f );
	
	GLshort vertices[] = {xko, yko};
	
	RO_ClientState(VTXARRAY_VERTICES);
	
	RO_ArrayMakeCurrent(VTXPTRS_NONE, NULL);
	glVertexPointer(2, GL_SHORT, 0, vertices);
	
	glDrawArrays(GL_POINTS, 0, 1);

	// set rasterizer state to default
	RO_DefaultRasterizerState();
}


// draw horizontal bar --------------------------------------------------------
//
void D_DrawHorzBar( ugrid_t xko, ugrid_t yko, visual_t col, dword leng )
{
	// configure rasterizer
	dword itertype	= iter_rgb;
	dword raststate	= rast_chromakeyoff;
	dword rastmask	= rast_mask_zbuffer | rast_mask_texclamp |
					  rast_mask_mipmap  | rast_mask_texfilter;
	RO_InitRasterizerState( itertype, raststate, rastmask );

	// set color
	colrgba_s colrgba;
	VisualToRGBA( &colrgba, col );
	glColor4ub( (GLubyte)colrgba.R, (GLubyte)colrgba.G, (GLubyte)colrgba.B, 255 );

	GLshort vertices[] = {xko, yko, xko + leng, yko};
	
	RO_ClientState(VTXARRAY_VERTICES);
	
	RO_ArrayMakeCurrent(VTXPTRS_NONE, NULL);
	glVertexPointer(2, GL_SHORT, 0, vertices);
	
	glDrawArrays(GL_LINES, 0, 2);

	// set rasterizer state to default
	RO_DefaultRasterizerState();
}


// draw vertical bar ----------------------------------------------------------
//
void D_DrawVertBar( ugrid_t xko, ugrid_t yko, visual_t col, dword leng )
{
	// configure rasterizer
	dword itertype	= iter_rgb;
	dword raststate	= rast_chromakeyoff;
	dword rastmask	= rast_mask_zbuffer | rast_mask_texclamp |
					  rast_mask_mipmap  | rast_mask_texfilter;
	RO_InitRasterizerState( itertype, raststate, rastmask );

	// set color
	colrgba_s colrgba;
	VisualToRGBA( &colrgba, col );
	glColor4ub( (GLubyte)colrgba.R, (GLubyte)colrgba.G, (GLubyte)colrgba.B, 255 );
	
	GLshort vertices[] = {xko, yko, xko, yko + leng};
	
	RO_ClientState(VTXARRAY_VERTICES);
	
	RO_ArrayMakeCurrent(VTXPTRS_NONE, NULL);
	glVertexPointer(2, GL_SHORT, 0, vertices);
	
	glDrawArrays(GL_LINES, 0, 2);

	// set rasterizer state to default
	RO_DefaultRasterizerState();
}


// draw arbitrarily oriented line ---------------------------------------------
//
void D_DrawLine( ugrid_t xko1, ugrid_t yko1, ugrid_t xko2, ugrid_t yko2, visual_t col1, visual_t col2, dword mode )
{
	// configure rasterizer
	dword blendtype = ( mode & D_DRAWLINE_STYLE_ANTIALIASED ) ?
						iter_alphablend : iter_overwrite;
	dword itertype	= iter_rgba | blendtype;
	dword raststate	= rast_chromakeyoff;
	dword rastmask	= rast_mask_zbuffer | rast_mask_texclamp |
					  rast_mask_mipmap  | rast_mask_texfilter;
	RO_InitRasterizerState( itertype, raststate, rastmask );

	// set line antialiasing mode
	if ( mode & D_DRAWLINE_STYLE_ANTIALIASED )
		RO_LineSmooth( TRUE );
	
	colrgba_s colrgba1, colrgba2;
	
	VisualToRGBA( &colrgba1, col1 );
	VisualToRGBA( &colrgba2, col2 );

	GLshort vertices[] = {xko1, yko1, xko2, yko2};
	GLubyte colors[] = {colrgba1.R, colrgba1.G, colrgba2.B, colrgba2.R, colrgba2.G, colrgba2.B};
	
	RO_ClientState(VTXARRAY_VERTICES | VTXARRAY_COLORS);
	
	RO_ArrayMakeCurrent(VTXPTRS_NONE, NULL);
	glVertexPointer(2, GL_SHORT, 0, vertices);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
	
	glDrawArrays(GL_LINES, 0, 2);


	// set rasterizer state to default
	if ( mode & D_DRAWLINE_STYLE_ANTIALIASED )
		RO_LineSmooth( FALSE );
	RO_DefaultRasterizerState();
}


// draw line strip (draws numvtxs - 1 lines) ----------------------------------
//
void D_DrawLineStrip( ugrid_t *vtxs, dword numvtxs, visual_t col, dword mode )
{
	ASSERT( vtxs != NULL );
	ASSERT( numvtxs > 1 );

	// configure rasterizer
	dword blendtype = ( mode & D_DRAWLINE_STYLE_ANTIALIASED ) ?
						iter_alphablend : iter_overwrite;
	dword itertype	= iter_rgba | blendtype;
	dword raststate	= rast_chromakeyoff;
	dword rastmask	= rast_mask_zbuffer | rast_mask_texclamp |
					  rast_mask_mipmap  | rast_mask_texfilter;
	RO_InitRasterizerState( itertype, raststate, rastmask );

	// set color
	colrgba_s colrgba;
	VisualToRGBA( &colrgba, col );
	glColor4ub( (GLubyte)colrgba.R, (GLubyte)colrgba.G, (GLubyte)colrgba.B, 255 );

	// set line antialiasing mode
	if ( mode & D_DRAWLINE_STYLE_ANTIALIASED )
		RO_LineSmooth( TRUE );

	// draw line strip/loop
	GLenum glmode = ( mode & D_DRAWLINE_STYLE_CLOSE_STRIP ) ?
					GL_LINE_LOOP : GL_LINE_STRIP;
	
	RO_ClientState(VTXARRAY_VERTICES);
	
	RO_ArrayMakeCurrent(VTXPTRS_NONE, NULL);
	glVertexPointer(2, GL_SHORT, 0, vtxs);
	
	glDrawArrays(glmode, 0, numvtxs);


	// set rasterizer state to default
	if ( mode & D_DRAWLINE_STYLE_ANTIALIASED )
		RO_LineSmooth( FALSE );
	RO_DefaultRasterizerState();
}



