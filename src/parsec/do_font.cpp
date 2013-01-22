/*
 * PARSEC - Font Drawing Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:33 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999-2001
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
#include "d_font.h"
#include "d_fontt.h"

// subsystem linkage info
#include "linkinfo.h"

// local module header
#include "do_font.h"

// opengl headers
#include "r_gl.h"

// proprietary module headers
#include "con_aux.h"
#include "h_supp.h"
#include "ro_api.h"
#include "ro_supp.h"


// flags
#define DONT_USE_DRAWPIXEL_ALPHA


char*		CharsetDataBase;
//	extern "C"	dword ColMode=0;


// ----------------------------------------------------------------------------
//
#define CHARSRCWIDTH	320

dword*		do_wstr_geometry = NULL;
char*		do_wstr_outseg	 = NULL;
dword		do_wstr_width	 = 16;
dword		do_wstr_width_d	 = 16;
dword		do_wstr_height	 = 16;

int			do_textured_font = FALSE;
TextureMap*	do_font_texture  = NULL;


// set string output context --------------------------------------------------
//
void D_SetWStrContext( char *charset, dword *geometry, char *outseg, dword width, dword height )
{
	ASSERT( charset != NULL );
	CharsetDataBase = charset;

	do_wstr_geometry = geometry;
	do_wstr_outseg	= NULL;
	do_wstr_width 	= width;
	do_wstr_width_d = width;
	do_wstr_height 	= height;

	do_wstr_width -= (width & 3);

	ASSERT( do_wstr_geometry != 0 );
//	ASSERT( do_wstr_outseg != 0 );
	ASSERT( do_wstr_width >= 4 );
	ASSERT( do_wstr_height >= 4 );

	// use "TEXF" sig to determine if texture
	char *sig = charset + sizeof( TextureMap );
	if ( *(dword *)sig == SWAP_32( 0x46584554 ) ) {
		do_textured_font = TRUE;
		do_font_texture  = (TextureMap *) charset;
	} else {
		do_textured_font = FALSE;
		do_font_texture  = NULL;
//		ASSERT(0);
//		PANIC(0);
	}

	// reset hud font color
	cur_hud_char_color = 0;
}


// determine char code geometry table starts with -----------------------------
//
INLINE
char DO_GetStartChar( dword *geom )
{
	//NOTE:
	//CAVEAT:
	// start-char ' ' (0x20) is always assumed
	// with the static tables.

	if ( ( geom == Char16x16Geom ) || ( geom == CharGoldGeom  ) ||
		 ( geom == Char04x09Geom ) || ( geom == Char08x08Geom ) ) {

		return ' ';

	} else {

		return ( (char) SWAP_32( *( geom - 1 ) ) );
	}
}


// write a string using a font texture ----------------------------------------
//
PRIVATE
void DO_WriteStringTextured( const char *string, ugrid_t putx, ugrid_t puty )
{
	ASSERT( string != NULL );

	ASSERT( do_font_texture != NULL );

	TextureMap *texmap = do_font_texture;
	word *uvtab = (word *) ( (char*)texmap + sizeof( TextureMap ) + 8 );

	// clip string at right edge of screen
	int sumwidth = putx + do_wstr_width;
	int numtris  = 0;
	for ( ; string[ numtris ] != 0; numtris++, sumwidth+=do_wstr_width ) {
		if ( sumwidth > Screen_Width ) {
			break;
		}
	}

	// avoid handling empty strings
	if ( numtris == 0 ) {
		return;
	}

	// ensure filtering is turned off
	int filtmode = AUX_DISABLE_POLYGON_FILTERING;
	AUX_DISABLE_POLYGON_FILTERING = 1;

	GLTexInfo texinfo;
	RO_TextureMap2GLTexInfo( &texinfo, texmap );
	RO_SelectTexelSource( &texinfo );

	// restore filter state
	AUX_DISABLE_POLYGON_FILTERING = filtmode;

	GLVertex2 *trilist = (GLVertex2 *) ALLOCMEM( numtris * sizeof( GLVertex2 ) * 3 );
	if ( trilist == NULL ) {
		ASSERT( 0 );
		return;
	}

	float scaleu		= texinfo.coscale;
	float scalev		= texinfo.aratio * scaleu;

	GLfloat destx		= (GLfloat) putx;
	GLfloat desty		= (GLfloat) puty;
	GLfloat triwidth	= (GLfloat) ( do_wstr_width  * 2 );
	GLfloat triheight	= (GLfloat) ( do_wstr_height * 2 );
	GLfloat deltau		= triwidth  * scaleu;
	GLfloat deltav		= triheight * scalev;

	// fill vertex array
	int vtx = 0;
	for ( int tid = 0; tid < numtris; tid++ ) {

		int indx = *string++;

		// draw anything but spaces
		if ( indx != 0x20 ) {

			ASSERT( indx >= 0x00 );
			ASSERT( indx <= 0x7f );

			indx *= 2;
			GLfloat curu = uvtab[ indx + 0 ] * scaleu;
			GLfloat curv = uvtab[ indx + 1 ] * scalev;

			trilist[ vtx + 0 ].x = destx;
			trilist[ vtx + 0 ].y = desty;
			trilist[ vtx + 0 ].s = curu;
			trilist[ vtx + 0 ].t = curv;

			trilist[ vtx + 1 ].x = destx + triwidth;
			trilist[ vtx + 1 ].y = desty;
			trilist[ vtx + 1 ].s = curu + deltau;
			trilist[ vtx + 1 ].t = curv;

			trilist[ vtx + 2 ].x = destx;
			trilist[ vtx + 2 ].y = desty + triheight;
			trilist[ vtx + 2 ].s = curu;
			trilist[ vtx + 2 ].t = curv + deltav;

			vtx += 3;
		}

		destx += do_wstr_width_d;
	}

	// actual number of vertices to draw
	int numvtxs = vtx;

	// draw non-empty strings
	if ( numvtxs > 0 ) {

		RO_ClientState( VTXARRAY_VERTICES | VTXARRAY_TEXCOORDS );
		if ( RO_ArrayMakeCurrent( VTXPTRS_DO_FONT_1, trilist ) ) {
			glVertexPointer( 2, GL_FLOAT, sizeof( GLVertex2 ), &trilist->x );
			glTexCoordPointer( 2, GL_FLOAT, sizeof( GLVertex2 ), &trilist->s );
		}

		// draw vertex array
		glDrawArrays( GL_TRIANGLES, 0, numvtxs );

//		RO_ClientState( VTXARRAY_NONE );
	}

	FREEMEM( trilist );
}


// color values for hud charset -----------------------------------------------
//
static colrgba_s hud_col_table[ 6 ] = {

	{  13, 156,   0, 255 },
	{ 192,  23,   7, 255 },
	{  97,  97, 177, 255 },
	{ 169,  64, 215, 255 },
	{ 197, 197, 197, 255 },
	{   0, 136, 136, 255 },
};


// write string in fixed size charset -----------------------------------------
//
void D_WriteString( const char *string, ugrid_t putx, ugrid_t puty )
{
	ASSERT( string != NULL );

	// distinguish texture and bitmap fonts
	if ( do_textured_font ) {

		// configure rasterizer
		dword itertype	= iter_rgbatexa;
		dword raststate	= rast_chromakeyoff;
		dword rastmask	= rast_mask_zbuffer | rast_mask_texclamp |
						  rast_mask_mipmap  | rast_mask_texfilter;
		RO_InitRasterizerState( itertype, raststate, rastmask );

		RO_AlphaFunc( GL_GREATER, 0.0f );
		RO_AlphaTest( TRUE );

		// set color (only correct for hud font)
		int cix = cur_hud_char_color;
		ASSERT( (dword)cix < NUM_HUD_CHARSET_COLORS );
		glColor3ub( hud_col_table[ cix ].R, hud_col_table[ cix ].G, hud_col_table[ cix ].B );

		DO_WriteStringTextured( string, putx, puty );

		RO_AlphaTest( FALSE );

	} else {
		
		// removed
		ASSERT(0);
		
	}

	// set rasterizer state to default
	RO_DefaultRasterizerState();
}


// write translucent string in fixed size charset -----------------------------
//
void D_WriteTrString( const char *string, ugrid_t putx, ugrid_t puty, dword brightx )
{
	// distinguish texture and bitmap fonts
	if ( do_textured_font ) {

		// configure rasterizer
		dword itertype	= iter_rgbatexa | iter_alphablend;
		dword raststate	= rast_chromakeyoff;
		dword rastmask	= rast_mask_zbuffer | rast_mask_texclamp |
						  rast_mask_mipmap  | rast_mask_texfilter;
		RO_InitRasterizerState( itertype, raststate, rastmask );

		glColor4ub( PanelTextColor.R, PanelTextColor.G, PanelTextColor.B, PanelTextColor.A );
		DO_WriteStringTextured( string, putx, puty );

	} else {
		
		// removed
		ASSERT(0);

	}

	// set rasterizer state to default
	RO_DefaultRasterizerState();
}


// get width of string in proportional font -----------------------------------
//
int D_GetPStringWidth( char *string )
{
	ASSERT( string != NULL );

	ASSERT( CharsetDataBase != NULL );
	ASSERT( do_wstr_geometry != NULL );
//	ASSERT( do_wstr_outseg != NULL );
	ASSERT( do_wstr_width >= 4 );
	ASSERT( do_wstr_height >= 4 );

	char_dat *fonttab = (char_dat *) do_wstr_geometry;

	char start = DO_GetStartChar( do_wstr_geometry );

	int i = 0;
	int width = 0;
	char c;
	while ( (c = string[ i ]) ) {
		if ( c != ' ' )
			width += fonttab[ c - start ].width;
		else
			width += do_wstr_width;		// space

		i++;
	}

	return width;
}


// write string with texfont --------------------------------------------------
//
void D_TexfontWrite( const char *string, IterTexfont *itexfont )
{
	ASSERT( string != NULL );
	ASSERT( itexfont != NULL );

	texfont_s *texfont = itexfont->texfont;
	ASSERT( texfont != NULL );

	TextureMap *texmap = texfont->texmap;
	ASSERT( texmap != NULL );

	// get info and select texture
	GLTexInfo texinfo;
	RO_TextureMap2GLTexInfo( &texinfo, texmap );
	RO_SelectTexelSource( &texinfo );

	float scale_u = texinfo.coscale;
	float scale_v = texinfo.aratio * scale_u;

	// allocate vertex array
	size_t numchars = strlen( string );
	size_t numvtxs  = numchars * 4;
	GLVertex2 *glvtxs = (GLVertex2 *) ALLOCMEM( numvtxs * sizeof( GLVertex2 ) );
	if ( glvtxs == NULL ) {
		OUTOFMEM( 0 );
	}

	// coordinates of upper left point
	float dstx = RASTV_TO_FLOAT( itexfont->Vtxs[ 0 ].X ) + 0.5f;
	float dsty = RASTV_TO_FLOAT( itexfont->Vtxs[ 0 ].Y ) + 0.5f;

	// output scale factors
	float scalex = GEOMV_TO_FLOAT( itexfont->Vtxs[ 0 ].U );
	float scaley = GEOMV_TO_FLOAT( itexfont->Vtxs[ 0 ].V );

	float srcheight = (float)texfont->height;
	float dstheight = srcheight * scaley;

	// fill vertex array
	for ( int vtx = 0; vtx < numvtxs; ) {

		int chcode = (byte) *string++;

		// skip undefined chars
		if ( chcode >= texfont->numtexchars ) {
			numvtxs -= 4;
			continue;
		}

		texchar_s *curchar = &texfont->texchars[ chcode ];

		float srcu = (float)( curchar->tex_u + curchar->tex_ulead );
		float srcv = (float)( curchar->tex_v );

		float srcwidth = (float)curchar->tex_ustep;
		float dstwidth = srcwidth * scalex;

		// special texture id for spacing
		if ( curchar->tex_id == -1 ) {
			dstx += dstwidth;
			numvtxs -= 4;
			continue;
		} else {
			// currently only one texture supported
			ASSERT( curchar->tex_id == 0 );
		}

		glvtxs[ vtx + 0 ].x = dstx;
		glvtxs[ vtx + 0 ].y = dsty;
		*(dword*)&glvtxs[ vtx + 0 ].r = *(dword*)&itexfont->Vtxs[ 0 ].R;
		glvtxs[ vtx + 0 ].s = srcu * scale_u;
		glvtxs[ vtx + 0 ].t = srcv * scale_v;

		glvtxs[ vtx + 1 ].x = dstx + dstwidth;
		glvtxs[ vtx + 1 ].y = dsty;
		*(dword*)&glvtxs[ vtx + 1 ].r = *(dword*)&itexfont->Vtxs[ 0 ].R;
		glvtxs[ vtx + 1 ].s = ( srcu + srcwidth ) * scale_u;
		glvtxs[ vtx + 1 ].t = srcv * scale_v;

		glvtxs[ vtx + 2 ].x = dstx + dstwidth;
		glvtxs[ vtx + 2 ].y = dsty + dstheight;
		*(dword*)&glvtxs[ vtx + 2 ].r = *(dword*)&itexfont->Vtxs[ 0 ].R;
		glvtxs[ vtx + 2 ].s = ( srcu + srcwidth ) * scale_u;
		glvtxs[ vtx + 2 ].t = ( srcv + srcheight ) * scale_v;

		glvtxs[ vtx + 3 ].x = dstx;
		glvtxs[ vtx + 3 ].y = dsty + dstheight;
		*(dword*)&glvtxs[ vtx + 3 ].r = *(dword*)&itexfont->Vtxs[ 0 ].R;
		glvtxs[ vtx + 3 ].s = srcu * scale_u;
		glvtxs[ vtx + 3 ].t = ( srcv + srcheight ) * scale_v;

		vtx  += 4;
		dstx += dstwidth;
	}

	// draw non-empty strings
	if ( numvtxs > 0 ) {

		// save zcmp and zwrite state
		int zcmpstate   = RO_DepthCmpEnabled();
		int zwritestate = RO_DepthWriteEnabled();

		// configure rasterizer

		itexfont->raststate = rast_chromakeyoff;
		itexfont->rastmask = rast_mask_zbuffer | rast_mask_texclamp |
			rast_mask_mipmap  | rast_mask_texfilter;

		RO_InitRasterizerState( itexfont->itertype, itexfont->raststate, itexfont->rastmask );
		RO_TextureCombineState( texcomb_decal );

		// draw entire string
		RO_ClientState( VTXARRAY_VERTICES | VTXARRAY_COLORS | VTXARRAY_TEXCOORDS );
		if ( RO_ArrayMakeCurrent( VTXPTRS_DO_FONT_2, glvtxs ) ) {
			glVertexPointer( 2, GL_FLOAT, sizeof( GLVertex2 ), &glvtxs->x );
			glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( GLVertex2 ), &glvtxs->r );
			glTexCoordPointer( 2, GL_FLOAT, sizeof( GLVertex2 ), &glvtxs->s );
		}
		glDrawArrays( GL_QUADS, 0, numvtxs );

		// set rasterizer state to default
		RO_DefaultRasterizerState();

		// restore zcmp and zwrite state
		RO_RestoreDepthState( zcmpstate, zwritestate );
	}

	// free intermediate storage
	FREEMEM( glvtxs );
}


// get width on screen of string written with textfont ------------------------
//
int D_TexfontGetWidth( const char *string, IterTexfont *itexfont )
{
	ASSERT( string != NULL );
	ASSERT( itexfont != NULL );

	texfont_s *texfont = itexfont->texfont;
	ASSERT( texfont != NULL );

	int numchars = strlen( string );

	float scalex = RASTV_TO_FLOAT( itexfont->Vtxs[ 0 ].U );

	float pixwidth = 0.0f;
	for ( int curpos = 0; curpos < numchars; curpos++ ) {

		// skip undefined chars
		int chcode = (byte)string[ curpos ];
		if ( chcode >= texfont->numtexchars ) {
			continue;
		}

		texchar_s *curchar = &texfont->texchars[ chcode ];
		pixwidth += (float)curchar->tex_ustep * scalex;
	}

	return (int) pixwidth;
}



