/*
 * PARSEC - Overlay Text Drawing
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:37 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1997-1999
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

#if 0

// C library
#include <stddef.h>
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
#include "vid_defs.h"

// drawing subsystem
#include "d_bmap.h"
#include "d_font.h"
#include "d_iter.h"

// rendering subsystem
#include "r_patch.h"

// local module header
#include "h_text.h"

// proprietary module headers
#include "e_color.h"
#include "e_supp.h"
#include "img_supp.h"


// demotext info --------------------------------------------------------------
//
#define DEMOTEXT_NUM_LINES		4
#define DEMOTEXT_SCROLL_SPEED	70
#define DEMOTEXT_MAX_LENGTH		127		// maximum string length per line
#define DEMOTEXT_FONT_HEIGHT	77

#define TAL_CHARSETNO_1			5
#define TAL_CHARSETNO_2			6

PRIVATE char	demotext_text[ DEMOTEXT_NUM_LINES ][ DEMOTEXT_MAX_LENGTH + 1 ];
PRIVATE char*	demotext_fontmem[ DEMOTEXT_NUM_LINES ];
PRIVATE int		demotext_position_x[ DEMOTEXT_NUM_LINES ];
PRIVATE int 	demotext_position_y[ DEMOTEXT_NUM_LINES ] =
	{	100,			185,				185+85,				185+85+85		};
PRIVATE int 	demotext_charset_id[ DEMOTEXT_NUM_LINES ] =
	{ TAL_CHARSETNO_2,	TAL_CHARSETNO_1,	TAL_CHARSETNO_1,	TAL_CHARSETNO_1	};
PRIVATE int		demotext_scroll_dir[ DEMOTEXT_NUM_LINES ] =
	{	+1,				-1,					-1,					-1				};
PRIVATE int		demotext_capitals[ DEMOTEXT_NUM_LINES ] =
	{	FALSE,			TRUE,				TRUE,				TRUE			};

PRIVATE int		demotext_pitch = 0;
PRIVATE int		demotext_width = 0;


// free cached bitmap/textures for demo text ----------------------------------
//
PRIVATE
void FreeDemoTextCache( int lineid )
{
	ASSERT( (dword)lineid < DEMOTEXT_NUM_LINES );
	ASSERT( demotext_fontmem[ lineid ] != NULL );

	// if bitmap was converted to textures they have to be invalidated
	if ( VidInfo_UseIterForDemoTexts ) {
		TextureMap *tmap = (TextureMap *)demotext_fontmem[ lineid ];
		int numtextures = *(int*)tmap;
		for ( int tid = numtextures; tid > 0; tid-- ) {
			R_InvalidateCachedTexture( &tmap[ tid ] );
		}
	}

	// free cache mem
	FREEMEM( demotext_fontmem[ lineid ] );
	demotext_fontmem[ lineid ] = NULL;
}


// stop display and scrolling of demo text if currently active ----------------
//
void HUD_InitDemoText()
{
	for ( int lineid = 0; lineid < DEMOTEXT_NUM_LINES; lineid++ ) {

		if ( demotext_fontmem[ lineid ] != NULL )
			FreeDemoTextCache( lineid );
		demotext_text[ lineid ][ 0 ]  = 0;
		demotext_position_x[ lineid ] = 0;
	}
}


// scroll all demo text lines out ---------------------------------------------
//
void HUD_ClearDemoText()
{
	for ( int lineid = 0; lineid < DEMOTEXT_NUM_LINES; lineid++ ) {
		demotext_position_x[ lineid ] = demotext_scroll_dir[ lineid ];
	}
}


// set demo text to be scrolled in --------------------------------------------
//
void HUD_SetDemoText( int lineid, char *text )
{
	ASSERT( (dword)lineid < DEMOTEXT_NUM_LINES );
	ASSERT( text != NULL );

	if ( demotext_fontmem[ lineid ] != NULL )
		FreeDemoTextCache( lineid );
	strncpy( demotext_text[ lineid ], text, DEMOTEXT_MAX_LENGTH );
	demotext_text[ lineid ][ DEMOTEXT_MAX_LENGTH ] = 0;
	demotext_position_x[ lineid ] = -Screen_Width * demotext_scroll_dir[ lineid ];

	AUDs_SpeakString( demotext_text[ lineid ] );
}


// scroll demo text line left -------------------------------------------------
//
PRIVATE
int ScrollDemoTextLineLeft( int lineid )
{
	ASSERT( (dword)lineid < DEMOTEXT_NUM_LINES );

	if ( demotext_position_x[ lineid ] < 0 ) {

		if ( demotext_position_x[ lineid ] > -Screen_Width ) {
			demotext_position_x[ lineid ] -= DEMOTEXT_SCROLL_SPEED;
		} else {
			// text is scrolled out now
			return TRUE;
		}

	} else if ( demotext_position_x[ lineid ] > 0 ) {

		if ( ( demotext_position_x[ lineid ] -= DEMOTEXT_SCROLL_SPEED ) < 0 ) {
			// text is scrolled in now
			demotext_position_x[ lineid ] = 0;
		}
	}

	return FALSE;
}


// scroll demo text line right ------------------------------------------------
//
PRIVATE
int ScrollDemoTextLineRight( int lineid )
{
	ASSERT( (dword)lineid < DEMOTEXT_NUM_LINES );

	if ( demotext_position_x[ lineid ] > 0 ) {

		if ( demotext_position_x[ lineid ] < Screen_Width ) {
			demotext_position_x[ lineid ] += DEMOTEXT_SCROLL_SPEED;
		} else {
			// text is scrolled out now
			return TRUE;
		}

	} else if ( demotext_position_x[ lineid ] < 0 ) {

		if ( ( demotext_position_x[ lineid ] += DEMOTEXT_SCROLL_SPEED ) > 0 ) {
			// text is scrolled in now
			demotext_position_x[ lineid ] = 0;
		}
	}

	return FALSE;
}


// convert demo text bitmap into textures -------------------------------------
//
PRIVATE
void CreateBitmapTextures( char **fontmem )
{
	ASSERT( fontmem != NULL );
//	ASSERT( (dword)lineid < DEMOTEXT_NUM_LINES );

	size_t pixsize = 4;

	// determine number of textures
	int textwidth = Screen_Width;
	int numtextures = 0;
	for ( numtextures = 0; textwidth > 0; numtextures++ ) {
		textwidth -= VidInfo_MaxTextureSize;
	}

	int srcbitmapw = Screen_Width;

	int srcw = VidInfo_MaxTextureSize;
	int srch = DEMOTEXT_FONT_HEIGHT;
	int dtw2 = CeilPow2Exp( VidInfo_MaxTextureSize );
	int dth2 = CeilPow2Exp( srch );
	int dstw = VidInfo_MaxTextureSize;
	int dsth = ( 1 << dth2 );

	int lsrw, ldtw, ldw2;
	if ( textwidth == 0 ) {
		lsrw = srcw;
		ldw2 = dtw2;
		ldtw = dstw;
	} else {
		lsrw = textwidth + VidInfo_MaxTextureSize;
		ldw2 = CeilPow2Exp( lsrw );
		ldtw = ( 1 << ldw2 );
	}

	// alloc mem for textures
	size_t firstsize = dstw * dsth * pixsize;
	size_t lastsize  = ldtw * dsth * pixsize;
	size_t datasize  = ( numtextures - 1 ) * firstsize + lastsize;
	char *texmem = (char *)
		ALLOCMEM( sizeof( TextureMap ) * ( numtextures + 1 ) + datasize );
	if ( texmem == NULL )
		OUTOFMEM( 0 );

	TextureMap *tmap = (TextureMap *)texmem + 1;
	char *texbitmap  = (char *) &tmap[ numtextures ];
	char *texsource  = *fontmem;

	// store number of textures
	*(int*)texmem = numtextures;

	// check for y-mirrored source bitmap
	int ymirrorofs = 0;
	if ( ColorSetupFlags & COLORSETUP_32BPP_FONT_YINVERSE ) {
		ymirrorofs = ( srch - 1 ) * srcbitmapw;
		srcbitmapw = -srcbitmapw;
	}

	// create textures
	for ( int ctex = numtextures; ctex > 0; ctex--, tmap++ ) {

		if ( ctex == 1 ) {
			srcw = lsrw;
			dtw2 = ldw2;
			dstw = ldtw;
			memset( texbitmap, 0, lastsize );
		} else {
			memset( texbitmap, 0, firstsize );
		}

		dword aspect	= IMG_DetermineTexGeoAspect( dstw, dsth );
		dword scalecode	= IMG_DetermineTexGeoScale( max( dstw, dsth ) );

		// init fields of texture map structure
		tmap->Flags			= TEXFLG_EXT_GEOMETRY | TEXFLG_LODRANGE_VALID;
		tmap->Geometry		= aspect | scalecode;
		tmap->Width			= dtw2;
		tmap->Height		= dth2;
		tmap->LOD_small		= dtw2;
		tmap->LOD_large		= dtw2;
		tmap->BitMap		= texbitmap;
		tmap->TexPalette	= NULL;
		tmap->TexMapName	= "demotext_texture";
		tmap->TexelFormat	= ( pixsize == 4 ) ? TEXFMT_RGBA_8888 : TEXFMT_RGB_565;

		// copy texture over (vertically mirrored if necessary)

		if ( pixsize == 4 ) {

			dword *dst = (dword *) texbitmap;
			dword *src = (dword *) texsource + ymirrorofs;
			for ( int row = srch; row > 0; --row ) {
				for ( dword *beyond = dst + srcw; dst < beyond; ) {
					*dst++ = *src++;
				}
				src += srcbitmapw - srcw;
				dst += dstw - srcw;
			}

		} else if ( pixsize == 2 ) {

			word *dst = (word *) texbitmap;
			word *src = (word *) texsource/* + ymirrorofs*/;
			for ( int row = srch; row > 0; --row ) {
				for ( word *beyond = dst + srcw; dst < beyond; ) {
					*dst++ = *src++;
				}
				src += srcbitmapw - srcw;
				dst += dstw - srcw;
			}
		}

		texbitmap += firstsize;
		texsource += srcw * pixsize;
	}

	// free bitmap mem
	FREEMEM( *fontmem );
	*fontmem = texmem;
}


// cache demo text as bitmap or textures --------------------------------------
//
PRIVATE
void InitDemoTextCache( int lineid )
{
	ASSERT(0);
	
	ASSERT( (dword)lineid < DEMOTEXT_NUM_LINES );

	// size of buffer for string blit
	size_t bufsize = Screen_Width * Screen_BytesPerPixel * 80;

	//NOTE:
	// using Screen_Pitch instead of ( Screen_Width * Screen_BytesPerPixel )
	// doesn't always work here because on a Voodoo3 Screen_Pitch will be set
	// to 2048, as on all other Voodoos. this is too low for resolutions
	// higher than 1024x768.

	demotext_fontmem[ lineid ] = (char *) ALLOCMEM( bufsize );
	if ( demotext_fontmem[ lineid ] == NULL )
		OUTOFMEM( 0 );
	memset( demotext_fontmem[ lineid ], 0, bufsize );

	int charsetid = demotext_charset_id[ lineid ];
	D_SetWStrContext( CharsetInfo[ charsetid ].charsetpointer,
					  CharsetInfo[ charsetid ].geompointer,
					  demotext_fontmem[ lineid ],
					  CharsetInfo[ charsetid ].width,
					  CharsetInfo[ charsetid ].height );

	if ( demotext_capitals[ lineid ] ) {
		strupr( demotext_text[ lineid ] );
	} else {
		strlwr( demotext_text[ lineid ] );
	}

	int swidth = D_GetPStringWidth( demotext_text[ lineid ] );
	if ( swidth <= Screen_Width ) {
//		D_WritePString( demotext_text[ lineid ], (Screen_Width-swidth)/2, 0 );
	} else {
		MSGOUT( "string too long." );
	}

	// split up bitmap into appropriately sized textures if necessary
	if ( VidInfo_UseIterForDemoTexts ) {
		CreateBitmapTextures( &demotext_fontmem[ lineid ] );
	}
}


// draw demo text line using iterrects ----------------------------------------
//
PRIVATE
void DrawIterText( char *fontmem, int width, int height, int swidth, int sheight, int xpos, int ypos )
{
	ASSERT( fontmem != NULL );

	IterRectangle2 itrect;

	//NOTE:
	// the Rage128 driver uses the vertex colors although
	// they are explicitly turned off in the texture environment.
	// therefore, we initialize the colors to white.

//	itrect.Vtxs[ 0 ].X = INT_TO_RASTV( xpos );
	itrect.Vtxs[ 0 ].Y = INT_TO_RASTV( ypos );
	itrect.Vtxs[ 0 ].Z = RASTV_1;
	itrect.Vtxs[ 0 ].W = GEOMV_1;
//	itrect.Vtxs[ 0 ].U = GEOMV_0;
	itrect.Vtxs[ 0 ].V = GEOMV_0;
	itrect.Vtxs[ 0 ].R = 255;
	itrect.Vtxs[ 0 ].G = 255;
	itrect.Vtxs[ 0 ].B = 255;
	itrect.Vtxs[ 0 ].A = 255;

//	itrect.Vtxs[ 1 ].X = INT_TO_RASTV( xpos + width );
	itrect.Vtxs[ 1 ].Y = INT_TO_RASTV( ypos );
	itrect.Vtxs[ 1 ].Z = RASTV_1;
	itrect.Vtxs[ 1 ].W = GEOMV_1;
//	itrect.Vtxs[ 1 ].U = INT_TO_GEOMV( width );
	itrect.Vtxs[ 1 ].V = GEOMV_0;
	itrect.Vtxs[ 1 ].R = 255;
	itrect.Vtxs[ 1 ].G = 255;
	itrect.Vtxs[ 1 ].B = 255;
	itrect.Vtxs[ 1 ].A = 255;

//	itrect.Vtxs[ 2 ].X = INT_TO_RASTV( xpos + width );
	itrect.Vtxs[ 2 ].Y = INT_TO_RASTV( ypos + sheight );
	itrect.Vtxs[ 2 ].Z = RASTV_1;
	itrect.Vtxs[ 2 ].W = GEOMV_1;
//	itrect.Vtxs[ 2 ].U = INT_TO_GEOMV( width );
	itrect.Vtxs[ 2 ].V = INT_TO_GEOMV( height );
	itrect.Vtxs[ 2 ].R = 255;
	itrect.Vtxs[ 2 ].G = 255;
	itrect.Vtxs[ 2 ].B = 255;
	itrect.Vtxs[ 2 ].A = 255;

//	itrect.Vtxs[ 3 ].X = INT_TO_RASTV( xpos );
	itrect.Vtxs[ 3 ].Y = INT_TO_RASTV( ypos + sheight );
	itrect.Vtxs[ 3 ].Z = RASTV_1;
	itrect.Vtxs[ 3 ].W = GEOMV_1;
//	itrect.Vtxs[ 3 ].U = GEOMV_0;
	itrect.Vtxs[ 3 ].V = INT_TO_GEOMV( height );
	itrect.Vtxs[ 3 ].R = 255;
	itrect.Vtxs[ 3 ].G = 255;
	itrect.Vtxs[ 3 ].B = 255;
	itrect.Vtxs[ 3 ].A = 255;

	itrect.flags	 = ITERFLAG_NONE;
	itrect.itertype  = iter_texonly | iter_alphablend;
	itrect.raststate = rast_default;
	itrect.rastmask  = rast_nomask;

	// to avoid texture seams due to filtering on all drivers
	// (especially the G400 needs this)
	geomv_t texbrdcorr = FLOAT_TO_GEOMV( 0.0 );

	int textwidth = width;

	float scalefactor = (float) swidth / (float) width;

	TextureMap *tmap = (TextureMap *)fontmem + 1;
	while ( textwidth > 0 ) {

		if ( textwidth < VidInfo_MaxTextureSize ) {
			width = textwidth;
			textwidth = 0;
		} else {
			width = VidInfo_MaxTextureSize;
			textwidth -= width;
		}

		int realwidth = (int)(width * scalefactor);

		itrect.Vtxs[ 0 ].X = INT_TO_RASTV( xpos );
		itrect.Vtxs[ 1 ].X = INT_TO_RASTV( xpos + realwidth );
		itrect.Vtxs[ 2 ].X = itrect.Vtxs[ 1 ].X;
		itrect.Vtxs[ 3 ].X = itrect.Vtxs[ 0 ].X;

		itrect.Vtxs[ 0 ].U = texbrdcorr;
		itrect.Vtxs[ 1 ].U = INT_TO_GEOMV( width ) - texbrdcorr;
		itrect.Vtxs[ 2 ].U = itrect.Vtxs[ 1 ].U;
		itrect.Vtxs[ 3 ].U = itrect.Vtxs[ 0 ].U;

		itrect.texmap = tmap++;
		xpos += realwidth;

		D_DrawIterRectangle2( &itrect );
	}
}


// draw demo text line --------------------------------------------------------
//
PRIVATE
void DrawDemoTextLine( int lineid )
{
	ASSERT( (dword)lineid < DEMOTEXT_NUM_LINES );

	// skip empty lines
	if ( demotext_text[ lineid ][ 0 ] == 0 )
		return;

	// scroll text if scrolling active
	int scrolledout = ( demotext_scroll_dir[ lineid ] == -1 ) ?
		ScrollDemoTextLineLeft( lineid ) : ScrollDemoTextLineRight( lineid );

	// clear text if entirely scrolled out
	if ( scrolledout ) {
		if ( demotext_fontmem[ lineid ] != NULL ) {
			FreeDemoTextCache( lineid );
		}
		demotext_text[ lineid ][ 0 ] = 0;
		return;
	}

	// cache text as bitmap or textures
	if ( demotext_fontmem[ lineid ] == NULL ) {
		InitDemoTextCache( lineid );
	}

	if ( VidInfo_UseIterForDemoTexts ) {

		// draw text using cached textures
		DrawIterText( demotext_fontmem[ lineid ], Screen_Width, DEMOTEXT_FONT_HEIGHT,
					  Screen_Width, DEMOTEXT_FONT_HEIGHT,
					  demotext_position_x[ lineid ], demotext_position_y[ lineid ] );

	} else {

		// draw text using cached bitmap
		D_PutTrClipBitmap( demotext_fontmem[ lineid ], Screen_Width, DEMOTEXT_FONT_HEIGHT,
			demotext_position_x[ lineid ], demotext_position_y[ lineid ] );
	}
}


// display up to four lines of demo text and scroll them if specified ---------
//
void HUD_DrawDemoText()
{
	// check for change of screen geometry
	if ( ( demotext_pitch != Screen_Pitch ) ||
		 ( demotext_width != Screen_Width ) ) {

		demotext_pitch = Screen_Pitch;
		demotext_width = Screen_Width;

		// invalidate cached bitmaps/textures
		for ( int lineid = 0; lineid < DEMOTEXT_NUM_LINES; lineid++ ) {
			if ( demotext_fontmem[ lineid ] != NULL ) {
				FreeDemoTextCache( lineid );
			}
		}
	}

	// draw all active demo text lines
	for ( int lineid = 0; lineid < DEMOTEXT_NUM_LINES; lineid++ ) {
		DrawDemoTextLine( lineid );
	}
}

#endif
