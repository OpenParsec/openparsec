/*
 * PARSEC - Color Manipulation Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:33 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-1999
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

// local module header
#include "e_color.h"


// flags
#define CALC_TEXTURE_GOURAUD_TABLES



// number of different intensity levels used for gouraud shading
#define NUM_GOURAUD_INTENSITY_LEVELS	256

// size of pseudo palettes for direct color modes
#define PAL_XLAT_TABLE_TO_16BIT_SIZE	( VGA_PAL_NUMCOLORS * 2 )
#define PAL_XLAT_TABLE_TO_32BIT_SIZE	( VGA_PAL_NUMCOLORS * 4 )

// maximum size of a single translucency mapping table
#define TRTAB_MAXSIZE					( 65536 * 2 )

// maximum size of blueprint color lookup table
#define BLUEPRINT_CLUT_SIZE				( 4*256 * 5 )


// error strings
static char no_bitmap_mem[]	 = "no mem for bitmap color translation.";
static char no_font_mem[]	 = "no mem for font color translation.";


// initialization done flag
static int color_init_done = FALSE;



// translation tables for colors from low to high bit-resolution --------------
//
byte colbits_1_to_8[]	= { 0x00, 0xff };
byte colbits_2_to_8[]	= { 0x00, 0x55, 0xaa, 0xff };
byte colbits_3_to_8[]	= { 0x00, 0x24, 0x49, 0x6d, 0x92, 0xb6, 0xdb, 0xff };
byte colbits_4_to_8[]	= { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
							0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
byte colbits_5_to_8[]	= { 0x00, 0x08, 0x10, 0x18, 0x21, 0x29, 0x31, 0x39,
							0x42, 0x4a, 0x52, 0x5a, 0x63, 0x6b, 0x73, 0x7b,
							0x84, 0x8c, 0x94, 0x9c, 0xa5, 0xad, 0xb5, 0xbd,
							0xc6, 0xce, 0xd6, 0xde, 0xe7, 0xef, 0xf7, 0xff };
byte colbits_6_to_8[]	= { 0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c,
							0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
							0x41, 0x45, 0x49, 0x4d, 0x51, 0x55, 0x59, 0x5d,
							0x61, 0x65, 0x69, 0x6d, 0x71, 0x75, 0x79, 0x7d,
							0x82, 0x86, 0x8a, 0x8e, 0x92, 0x96, 0x9a, 0x9e,
							0xa2, 0xa6, 0xaa, 0xae, 0xb2, 0xb6, 0xba, 0xbe,
							0xc3, 0xc7, 0xcb, 0xcf, 0xd3, 0xd7, 0xdb, 0xdf,
							0xe3, 0xe7, 0xeb, 0xef, 0xf3, 0xf7, 0xfb, 0xff };



// color setup (conversion) flags ---------------------------------------------
//
dword ColorSetupFlags = COLORSETUP_DEFAULT;


// calculate 1K translation table for color indexes to visuals ----------------
//
PRIVATE
void SetupColorIndexToVisual( char *palette )
{
	ASSERT( palette != NULL );

	// destination table
	visual_t *xlattab = IndexToVisualTab;


	ASSERT( ( COLRGBA_CHANNELBITS == 8 ) && ( VGA_PAL_COLBITS == 6 ) );

	dword alpha = 0x00000000;
	if ( ColorSetupFlags & COLORSETUP_32BPP_PAL_SET_ALPHA ) {
		alpha = ( ColorSetupFlags & COLORSETUP_32BPP_PAL_ALPHA_IN_LSB ) ?
			0x000000ff : 0xff000000;
	}

	// first entry is always black
	*xlattab++ = 0;

	// calculate next 255 entries
	for ( int ppos = 3; ppos < VGA_PAL_NUMCOLORS * 3; ppos+=3 ) {

		byte r  = colbits_6_to_8[ palette[ ppos + 0 ] & 0x3f ];
		byte g  = colbits_6_to_8[ palette[ ppos + 1 ] & 0x3f ];
		byte b  = colbits_6_to_8[ palette[ ppos + 2 ] & 0x3f ];

		dword w = ( ColorSetupFlags & COLORSETUP_32BPP_PAL_SWAP_RGB ) ?
			( ( b << 16 ) | ( g << 8 ) | r ) :
			( ( r << 16 ) | ( g << 8 ) | b );

		if ( ColorSetupFlags & COLORSETUP_32BPP_PAL_ALPHA_IN_LSB )
			w <<= 8;

		if ( w == 0 )
			++w;
		*xlattab++ = ( w | alpha );
	}
}


// calculate 1K translation table for color indexes to RGBA colors ------------
//
PRIVATE
void SetupColorIndexToRGBA( char *palette )
{
	ASSERT( palette != NULL );

	// destination table
	colrgba_s *colrgba = IndexToRGBATab;

	// first entry is always black
	colrgba->R = 0;
	colrgba->G = 0;
	colrgba->B = 0;
	colrgba->A = 255;
	colrgba++;

	// calculate 256 entries
	for ( int ppos = 3; ppos < VGA_PAL_NUMCOLORS * 3; ppos+=3 ) {

		colrgba->R = colbits_6_to_8[ palette[ ppos + 0 ] & 0x3f ];
		colrgba->G = colbits_6_to_8[ palette[ ppos + 1 ] & 0x3f ];
		colrgba->B = colbits_6_to_8[ palette[ ppos + 2 ] & 0x3f ];
		colrgba->A = 255;
		colrgba++;
	}
}


// setup color index translation tables ---------------------------------------
//
void SetupTranslationPalette( char *palette )
{
	ASSERT( palette != NULL );

	SetupColorIndexToVisual( palette );
	SetupColorIndexToRGBA( palette );
}


// translate color index to current visual (color in framebuffer format) ------
//
visual_t ColorIndexToVisual( int colindx )
{
//	ASSERT( ( colindx >= 0 ) && ( colindx <= VGA_PAL_NUMCOLORS ) );

	//NOTE:
	// this function is available without checks
	// as COLINDX_TO_VISUAL() macro in COLOR.H

	return IndexToVisualTab[ colindx & 0xff ];
}


// translate color index to RGBA color ----------------------------------------
//
void ColorIndexToRGBA( colrgba_s *colrgba, int colindx )
{
	ASSERT( colrgba != NULL );
//	ASSERT( ( colindx >= 0 ) && ( colindx <= VGA_PAL_NUMCOLORS ) );

	//NOTE:
	// this function is available without checks
	// as COLINDX_TO_RGBA() macro in COLOR.H

	*colrgba = IndexToRGBATab[ colindx & 0xff ];
}


// translate visual (color in framebuffer format) into RGBA color -------------
//
void VisualToRGBA( colrgba_s *colrgba, visual_t visual )
{
	ASSERT( colrgba != NULL );

	// visual has [ABGR] layout
	colrgba->R = ( visual       ) & 0xff; // & 0xff redundant
	colrgba->G = ( visual >> 8  ) & 0xff; // & 0xff redundant
	colrgba->B = ( visual >> 16 ) & 0xff; // & 0xff redundant
	colrgba->A = 255;
}


// translate RGBA color to visual (color in framebuffer format) ---------------
//
visual_t RGBAToVisual( colrgba_s *colrgba )
{
	ASSERT( colrgba != NULL );

	visual_t visual = 0;

	// visual has [ABGR] layout
	visual  = (dword)colrgba->R;
	visual |= (dword)colrgba->G << 8;
	visual |= (dword)colrgba->B << 16;
	visual |= (dword)255        << 24;

	return visual;
}


// texture geometry table for font textures -----------------------------------
//
struct fonttex_info_s {

	int		width;
	int		height;
	int		width_log2;
	int		height_log2;

	dword	geometry;
	dword	lod;
};

static fonttex_info_s fonttex_info[] = {

	// fontid 0
	{ 256, 256, 8, 8, (TEXGEO_SCALE_1|TEXGEO_ASPECT_1x1), TEXLOD_256 },

	// fontid 1: 4x9
	{ 128, 128, 7, 7, (TEXGEO_SCALE_1|TEXGEO_ASPECT_1x1), TEXLOD_128 },

	// fontid 2: 8x8
	{ 256, 256, 8, 8, (TEXGEO_SCALE_1|TEXGEO_ASPECT_1x1), TEXLOD_256 },
//	{ 256, 128, 8, 7, (TEXGEO_SCALE_1|TEXGEO_ASPECT_2x1), TEXLOD_256 },

	// fontid 3
	{ 256, 256, 8, 8, (TEXGEO_SCALE_1|TEXGEO_ASPECT_1x1), TEXLOD_256 },
};


// convert font data to texture of 8-bit texels -------------------------------
//
PRIVATE
void CreateFontTextureData8( int fontid, int numvalid, char *texdata, word *uvtab )
{
	ASSERT( ( fontid >= 0 ) && ( fontid < NumLoadedCharsets ) );
	ASSERT( texdata != NULL );
	ASSERT( uvtab != NULL );

	dword*	fgeom = CharsetInfo[ fontid ].geompointer;
	char*	fdata = CharsetInfo[ fontid ].loadeddata;
	int		fontw = CharsetInfo[ fontid ].width;
	int		fonth = CharsetInfo[ fontid ].height;
	int		fsrcw = CharsetInfo[ fontid ].srcwidth;
	
	int texwidth  = fonttex_info[ fontid ].width;
	
	// convert data
	dword  destofs = 0;
	dword  curu    = 0;
	dword  curv    = 0;
	for ( int cidx = 0; cidx < numvalid; cidx++, uvtab+=2 ) {

		byte *src = (byte *) ( fdata   + fgeom[ cidx ] );
		byte *dst = (byte *) ( texdata + destofs );

		// convert to alpha/intensity char
		for ( int row = fonth; row > 0; row-- ) {
			for ( int col = fontw; col > 0; col-- ) {
				// TODO: fix or remove this
				// ASSERT( src < (byte*)( fdata + ftsiz ) );
				// ASSERT( dst < (byte*)( texdata + datasize ) );
				byte v = *src++;
				*dst++ = ( v != 0x00 ) ? 0xff : 0x00;
			}
			src += fsrcw    - fontw;
			dst += texwidth - fontw;
		}

		uvtab[ 0 ] = (word) curu;
		uvtab[ 1 ] = (word) curv;

		// advance in texture
		curu    += fontw * 2;
		destofs += fontw * 2;
		if ( curu > (dword)(texwidth - fontw * 2) ) {
			curu    = 0;
			curv   += fonth * 2;
			destofs = curv * texwidth;
		}

		// TODO: fix or remove this
		// ASSERT( curv <= (dword)(texheight - fonth * 2) );
		// ASSERT( destofs < datasize - fontw * fonth * 2 );
	}
}


// convert font data to texture of 32-bit texels ------------------------------
//
PRIVATE
void CreateFontTextureData32( int fontid, int numvalid, char *texdata, word *uvtab )
{
	ASSERT( ( fontid >= 0 ) && ( fontid < NumLoadedCharsets ) );
	ASSERT( texdata != NULL );
	ASSERT( uvtab != NULL );

	dword*	fgeom = CharsetInfo[ fontid ].geompointer;
	char*	fdata = CharsetInfo[ fontid ].loadeddata;
	int		fontw = CharsetInfo[ fontid ].width;
	int		fonth = CharsetInfo[ fontid ].height;
	int		fsrcw = CharsetInfo[ fontid ].srcwidth;
	
	int texwidth  = fonttex_info[ fontid ].width;
	
	// convert data
	dword  destofs = 0;
	dword  curu    = 0;
	dword  curv    = 0;
	for ( int cidx = 0; cidx < numvalid; cidx++, uvtab+=2 ) {

		byte  *src = (byte  *) ( fdata   + fgeom[ cidx ] );
		dword *dst = (dword *) ( texdata + destofs );

		// convert to rgba char
		for ( int row = fonth; row > 0; row-- ) {
			for ( int col = fontw; col > 0; col-- ) {
				// TODO: fix or remove this
				// ASSERT( src < (byte*)fdata + ftsiz );
				// ASSERT( dst < (dword*)texdata + datasize );
				byte v = *src++;
				*dst++ = ( v != 0x00 ) ? 0xffffffff : 0x00000000;
			}
			src += fsrcw    - fontw;
			dst += texwidth - fontw;
		}

		//NOTE:
		// we need not take any swapping into account here, since
		// we use only 0xffffffff and 0x00000000 as texel values.

		uvtab[ 0 ] = (word) curu;
		uvtab[ 1 ] = (word) curv;

		// advance in texture
		curu    += fontw * 2;
		destofs += fontw * 2 * 4;
		if ( curu > (dword)(texwidth - fontw * 2) ) {
			curu    = 0;
			curv   += fonth * 2;
			destofs = curv * texwidth * 4;
		}

		// TODO: fix or remove this
		// ASSERT( curv <= (dword)(texheight - fonth * 2) );
		// ASSERT( destofs < datasize - fontw * fonth * 2 * 4 );
	}
}


// convert font data to texture -----------------------------------------------
//
PRIVATE
int CreateFontTexture( int fontid )
{
	ASSERT( ( fontid >= 0 ) && ( fontid < NumLoadedCharsets ) );

	// use texture only for 4x9 and 8x8 font
	if ( /*( fontid != 1 ) &&*/ ( fontid != 2 ) )
		return FALSE;

	ASSERT( CharsetInfo[ fontid ].fonttexture == NULL );
	dword *geometry	= CharsetInfo[ fontid ].geompointer;

	// determine number of valid chars
	char startchar = ' ';
	if ( ( geometry != Char16x16Geom ) && ( geometry != CharGoldGeom  ) &&
		 ( geometry != Char04x09Geom ) && ( geometry != Char08x08Geom ) ) {
		startchar = (char) SWAP_32( *( geometry - 1 ) );
	}
	int numvalid = 128 - startchar;

	//CAVEAT:
	// this assumes that after the startchar
	// all chars will be valid!!

	// determine format and texelsize
	dword  texformat = TEXFMT_STANDARD;
	size_t texelsize = 1;
	if ( ColorSetupFlags & COLORSETUP_FONT_TO_TEXTURE_ALPHA ) {
		texformat = TEXFMT_ALPHA_8;
		texelsize = 1;
	} else if ( ColorSetupFlags & COLORSETUP_FONT_TO_TEXTURE_INTENSITY ) {
		texformat = TEXFMT_INTENSITY_8;
		texelsize = 1;
	} else if ( ColorSetupFlags & COLORSETUP_FONT_TO_TEXTURE_LUMINANCE ) {
		texformat = TEXFMT_LUMINANCE_8;
		texelsize = 1;
	} else if ( ColorSetupFlags & COLORSETUP_FONT_TO_TEXTURE_RGBA ) {
		texformat = TEXFMT_RGBA_8888;
		texelsize = 4;
	} else {
		ASSERT( 0 );
	}

	int texwidth  = fonttex_info[ fontid ].width;
	int texheight = fonttex_info[ fontid ].height;

	size_t headsize = sizeof( TextureMap ) + 8;
	size_t geomsize = 128 * sizeof( word ) * 2;
	size_t datasize = texwidth * texheight * texelsize;

	// alloc texture header and data
	char *dst = (char *) ALLOCMEM( headsize + geomsize + datasize );
	if ( dst == NULL ) {
		OUTOFMEM( no_font_mem );
	}

	// init header
	memset( dst, 0, headsize + geomsize + datasize );
	strcpy( dst + sizeof( TextureMap ), "TEXFONT" );

	// set pointers
	TextureMap *texmap = (TextureMap *) dst;
	CharsetInfo[ fontid ].charsetpointer = dst;
	CharsetInfo[ fontid ].fonttexture    = texmap;

	char *texdata = (char*)texmap + headsize + geomsize;
	word *uvtab   = (word *)( (char*)texmap + headsize ) + startchar * 2;

	// init texture header
	texmap->Flags		= TEXFLG_EXT_GEOMETRY | TEXFLG_LODRANGE_VALID;
	texmap->Geometry	= fonttex_info[ fontid ].geometry;
	texmap->Width		= fonttex_info[ fontid ].width_log2;
	texmap->Height		= fonttex_info[ fontid ].height_log2;
	texmap->LOD_small	= fonttex_info[ fontid ].lod;
	texmap->LOD_large	= fonttex_info[ fontid ].lod;
	texmap->BitMap		= texdata;
	texmap->TexPalette	= NULL;
	texmap->TexMapName	= "auto_font";
	texmap->TexelFormat	= texformat;

	// use conversion function corresponding to texel size
	if ( texelsize == 1 ) {
		CreateFontTextureData8( fontid, numvalid, texdata, uvtab );
	} else {
		CreateFontTextureData32( fontid, numvalid, texdata, uvtab );
	}

	return TRUE;
}


// special setup for bitmaps that may be used as alpha textures ---------------
//
int SetupBitmapAlphaTextures( int bmid )
{
	ASSERT( ( bmid >= 0 ) && ( bmid < NumLoadedBitmaps ) );

	//NOTE:
	// this is actually just a hack to accommodate OpenGL
	// drivers where GL_ALPHA textures are broken (Rage128).

	// special case for base lensflare bitmap
	if ( bmid != BM_LENSFLARE1 )
		return FALSE;

	int w = BitmapInfo[ bmid ].width;
	int h = BitmapInfo[ bmid ].height;
	char *src = BitmapInfo[ bmid ].loadeddata;

	colrgba_s *dst = (colrgba_s *) ALLOCMEM( w * h * 4 );
	if ( dst == NULL )
		OUTOFMEM( no_bitmap_mem );
	BitmapInfo[ bmid ].bitmappointer = (char *) dst;

	for ( int pix = w * h; pix > 0; pix--, dst++ ) {
		byte p = *src++;
		dst->R = 255;
		dst->G = 255;
		dst->B = 255;
		dst->A = p;
	}

	return TRUE;
}


// convert single bitmap to destination color format/model --------------------
//
void SetupSingleBitmapColors( int bmid )
{
	ASSERT( ( bmid >= 0 ) && ( bmid < NumLoadedBitmaps ) );

	char *src = BitmapInfo[ bmid ].loadeddata;

	// free previous conversion data
	if ( BitmapInfo[ bmid ].bitmappointer != src ) {
		ASSERT( BitmapInfo[ bmid ].bitmappointer != NULL );
		FREEMEM( BitmapInfo[ bmid ].bitmappointer );
		BitmapInfo[ bmid ].bitmappointer = src;
	}

	if ( bmid < BM_CONTROLFILE_NUMBER ) {

		// handle case when alpha textures must be avoided
		if ( ColorSetupFlags & COLORSETUP_ALPHA_8_TO_RGBA_8888 ) {
			if ( SetupBitmapAlphaTextures( bmid ) )
				return;
		}

		// determine whether conversion should be done for this bitmap
		if ( ColorSetupFlags & COLORSETUP_32BPP_CONVERT_SCALE_BITMAPS ) {
			if ( ( ( bmid > 24 ) && ( bmid < 29 ) ) || ( bmid > 52 ) )
				return;
		} else if ( bmid != 24 ) {
			if ( ( ( bmid > 11 ) && ( bmid < 29 ) ) || ( bmid > 42 ) )
				return;
		}
	}

	int w = BitmapInfo[ bmid ].width;
	int h = BitmapInfo[ bmid ].height;

	dword *dst = (dword *) ALLOCMEM( w * h * 4 );
	if ( dst == NULL )
		OUTOFMEM( no_bitmap_mem );
	BitmapInfo[ bmid ].bitmappointer = (char *) dst;

	if ( ColorSetupFlags & COLORSETUP_32BPP_BITMAP_YINVERSE ) {

		// flip bitmap vertically while converting
		if ( ColorSetupFlags & COLORSETUP_SWAP_BITMAPS_32BPP ) {
			src += w * h - w;
			for ( int row = h; row > 0; row-- ) {
				for ( int col = w; col > 0; col-- ) {
					byte  v = *src++;
					dword p = COLINDX_TO_VISUAL( v );
					*dst++ = FORCESWAP_32( p );
				}
				src -= w * 2;
			}
		} else {
			src += w * h - w;
			for ( int row = h; row > 0; row-- ) {
				for ( int col = w; col > 0; col-- ) {
					byte v = *src++;
					*dst++ = COLINDX_TO_VISUAL( v );
				}
				src -= w * 2;
			}
		}

	} else {

		if ( ColorSetupFlags & COLORSETUP_SWAP_BITMAPS_32BPP ) {
			for ( int pix = w * h; pix > 0; pix-- ) {
				byte  v = *src++;
				dword p = COLINDX_TO_VISUAL( v );
				*dst++ = FORCESWAP_32( p );
			}
		} else {
			for ( int pix = w * h; pix > 0; pix-- ) {
				byte v = *src++;
				*dst++ = COLINDX_TO_VISUAL( v );
			}
		}
	}
}


// convert single font to destination color format/model ----------------------
//
void SetupSingleCharsetColors( int ftid )
{
	ASSERT( ( ftid >= 0 ) && ( ftid < NumLoadedCharsets ) );

	if ( ftid != 4 ) {

		char *src = CharsetInfo[ ftid ].loadeddata;
		int blocksize = CharsetInfo[ ftid ].datasize;

		// reset font texture pointer
		if ( CharsetInfo[ ftid ].fonttexture != NULL ) {

			ASSERT( (void*)CharsetInfo[ ftid ].charsetpointer ==
					(void*)CharsetInfo[ ftid ].fonttexture );

			// the corresponding mem will be released as
			// standard conversion data
			CharsetInfo[ ftid ].fonttexture = NULL;
		}

		// free previous conversion data
		if ( CharsetInfo[ ftid ].charsetpointer != src ) {
			ASSERT( CharsetInfo[ ftid ].charsetpointer != NULL );
			FREEMEM( CharsetInfo[ ftid ].charsetpointer );
			CharsetInfo[ ftid ].charsetpointer = NULL;
		}

		// check for texture fonts
		if ( ColorSetupFlags & COLORSETUP_FONT_DO_CONVERSION ) {

			// convert font to texture if possible
			if ( CreateFontTexture( ftid ) ) {

				// skip standard conversion code only if
				// texture conversion actually succeeded
				return;
			}
		}

		// color depth conversion

		dword *dst = (dword *) ALLOCMEM( blocksize * 4 );
		if ( dst == NULL )
			OUTOFMEM( no_font_mem );
		CharsetInfo[ ftid ].charsetpointer = (char *) dst;

		if ( ( ColorSetupFlags & COLORSETUP_32BPP_FONT_YINVERSE ) &&
			 ( ftid != 5 ) && ( ftid != 6 ) ) {

			int w = CharsetInfo[ ftid ].srcwidth;
			int h = blocksize / w;
			int rh = CharsetInfo[ ftid ].height;
			int frows = h / rh;

			// flip font rows vertically while converting
			if ( ColorSetupFlags & COLORSETUP_SWAP_FONTS_32BPP ) {
				for ( int frow = frows; frow > 0; frow-- ) {
					src += w * rh - w;
					for ( int row = rh; row > 0; row-- ) {
						for ( int col = w; col > 0; col-- ) {
							byte  v = *src++;
							dword p = COLINDX_TO_VISUAL( v );
							*dst++ = FORCESWAP_32( p );
						}
						src -= w * 2;
					}
					src += w * rh + w;
				}
			} else {
				for ( int frow = frows; frow > 0; frow-- ) {
					src += w * rh - w;
					for ( int row = rh; row > 0; row-- ) {
						for ( int col = w; col > 0; col-- ) {
							byte v = *src++;
							*dst++ = COLINDX_TO_VISUAL( v );
						}
						src -= w * 2;
					}
					src += w * rh + w;
				}
			}

		} else {

			if ( ColorSetupFlags & COLORSETUP_SWAP_FONTS_32BPP ) {
				for ( int pix = blocksize; pix > 0; pix-- ) {
					byte  v = *src++;
					dword p = COLINDX_TO_VISUAL( v );
					*dst++ = FORCESWAP_32( p );
				}
			} else {
				for ( int pix = blocksize; pix > 0; pix-- ) {
					byte v = *src++;
					*dst++ = COLINDX_TO_VISUAL( v );
				}
			}
		}
	}
}


// create bitmaps for 16bit and 32bit color depths, respectively --------------
//
void SetupBitmapColors()
{
	// setup bitmaps
	for ( int bmid = 0; bmid < NumLoadedBitmaps; bmid++ ) {
		SetupSingleBitmapColors( bmid );
	}

	// setup charsets
	for ( int ftid = 0; ftid < NumLoadedCharsets; ftid++ ) {
		SetupSingleCharsetColors( ftid );
	}
}


// init color maps (alloc mem, calc mappings, etc.) ---------------------------
//
void InitColorMaps()
{
	if ( !color_init_done ) {

		// calc mapping tables
		MSGPUT( "Calculating colors..." );
		SetupTranslationPalette( PaletteMem + PAL_GAME );
		MSGOUT( "done." );

		color_init_done = TRUE;
	}
}


// kill color maps (free resources) -------------------------------------------
//
void KillColorMaps()
{
	if ( color_init_done ) {
		color_init_done = FALSE;
	}
}



