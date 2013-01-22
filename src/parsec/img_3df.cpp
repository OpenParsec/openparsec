/*
 * PARSEC - 3DF Format Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:42 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-2000
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
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// rendering subsystem
#include "r_patch.h"

// local module header
#include "img_3df.h"

// proprietary module headers
#include "con_aux.h"
#include "e_color.h"
#include "e_supp.h"
#include "img_supp.h"
#include "sys_file.h"
#include "sys_msg.h"



// string constants -----------------------------------------------------------
//
static char no_tex_conv_mem[]	= "not enough mem for texture data conversion.";
static char no_tex_file_mem[]	= "texture file too big.";
static char no_tex_head_mem[]	= "not enough mem for texture header.";


// convert glide (3df) texture formats to internal formats (if needed) --------
//
void Convert3DFToInternal( int insertindex )
{
	TextureMap *tmap = TextureInfo[ insertindex ].texpointer;
	ASSERT( tmap != NULL );

	char *texbitmap  = tmap->BitMap;
	ASSERT( texbitmap != NULL );

	if ( ColorSetupFlags & COLORSETUP_GLIDE3DF_DO_CONVERSION ) {

		// determine number of texels including mipmaps
		int numtexels = IMG_CountMipmapTexels(
			( 1 << tmap->Width ), ( 1 << tmap->Height ),
			tmap->LOD_large, tmap->LOD_small );

		// convert 8-bit paletted glide texture
		if ( tmap->TexelFormat == TEXFMT_GR_P_8  ) {

			colrgba_s *texpalette = (colrgba_s *) tmap->TexPalette;
			ASSERT( texpalette != NULL );

			// convert to RGB-only texture
			if ( ColorSetupFlags & COLORSETUP_GLIDE3DF_TO_RGB_888 ) {

				colrgb_s *cvblock = (colrgb_s *) ALLOCMEM( numtexels * sizeof( colrgb_s ) );
				if ( cvblock == NULL )
					OUTOFMEM( no_tex_conv_mem );
				colrgb_s *conv = cvblock;
				byte *texels = (byte *) texbitmap;
				for ( int tex = numtexels; tex > 0; tex--, conv++ ) {

					byte p = *texels++;

					// convert from (A,R,G,B) format
					// (identical for little/big-endian!)
					conv->R = texpalette[ p ].G;				// R A
					conv->G = texpalette[ p ].B;				// G R
					conv->B = texpalette[ p ].A;				// B G
																// A B
				}

				// this frees the palette along with the old texture
				FREEMEM( tmap->TexPalette );
				tmap->BitMap		= (char *) cvblock;
				tmap->TexelFormat	= TEXFMT_RGB_888;
				tmap->TexPalette	= NULL;

			// convert to RGBA texture although there is no source alpha
			} else if ( ColorSetupFlags & COLORSETUP_GLIDE3DF_TO_RGBA_8888 ) {

				colrgba_s *cvblock = (colrgba_s *) ALLOCMEM( numtexels * sizeof( colrgba_s ) );
				if ( cvblock == NULL )
					OUTOFMEM( no_tex_conv_mem );
				colrgba_s *conv = cvblock;
				byte *texels = (byte *) texbitmap;
				for ( int tex = numtexels; tex > 0; tex--, conv++ ) {

					byte p = *texels++;

					// convert from (A,R,G,B) format
					// (identical for little/big-endian!)
					conv->R = texpalette[ p ].G;				// R A
					conv->G = texpalette[ p ].B;				// G R
					conv->B = texpalette[ p ].A;				// B G
					conv->A = 255;								// A B
				}

				// this frees the palette along with the old texture
				FREEMEM( tmap->TexPalette );
				tmap->BitMap		= (char *) cvblock;
				tmap->TexelFormat	= TEXFMT_RGBA_8888;
				tmap->TexPalette	= NULL;

			} else {
				ASSERT( 0 );
			}

		// convert 16-bit paletted glide texture with additional alpha channel
		} else if ( tmap->TexelFormat == TEXFMT_GR_AP_88  ) {

			colrgba_s *texpalette = (colrgba_s *) tmap->TexPalette;
			ASSERT( texpalette != NULL );

			// convert to RGBA texture (even if rgb888 is specified)
			if ( ( ColorSetupFlags & COLORSETUP_GLIDE3DF_TO_RGB_888 ) ||
				 ( ColorSetupFlags & COLORSETUP_GLIDE3DF_TO_RGBA_8888 ) ) {

				colrgba_s *cvblock = (colrgba_s *) ALLOCMEM( numtexels * sizeof( colrgba_s ) );
				if ( cvblock == NULL )
					OUTOFMEM( no_tex_conv_mem );
				colrgba_s *conv = cvblock;
				byte *texels = (byte *) texbitmap;
				for ( int tex = numtexels; tex > 0; tex--, conv++ ) {

					//NOTE:
					// ap88 in 3DF files is stored in (A,P) order;
					// on little-endian systems this has already
					// been swapped at this point, though.

#ifdef SYSTEM_BIG_ENDIAN
					byte a = texels[ 0 ];
					byte p = texels[ 1 ];
#else
					byte a = texels[ 1 ];
					byte p = texels[ 0 ];
#endif
					texels += 2;

					// convert from (A,R,G,B) format
					// (identical for little/big-endian!)
					conv->R = texpalette[ p ].G;				// R A
					conv->G = texpalette[ p ].B;				// G R
					conv->B = texpalette[ p ].A;				// B G
					conv->A = a;								// A B
				}

				// this frees the palette along with the old texture
				FREEMEM( tmap->TexPalette );
				tmap->BitMap		= (char *) cvblock;
				tmap->TexelFormat	= TEXFMT_RGBA_8888;
				tmap->TexPalette	= NULL;

			} else {
				ASSERT( 0 );
			}

		// convert 16-bit glide texture without alpha
		} else if ( tmap->TexelFormat == TEXFMT_GR_RGB_565 ) {

			ASSERT( tmap->TexPalette == NULL );

			// convert to RGB-only texture
			if ( ColorSetupFlags & COLORSETUP_GLIDE3DF_TO_RGB_888 ) {

				colrgb_s *cvblock = (colrgb_s *) ALLOCMEM( numtexels * sizeof( colrgb_s ) );
				if ( cvblock == NULL )
					OUTOFMEM( no_tex_conv_mem );
				colrgb_s *conv = cvblock;
				word *texels = (word *) texbitmap;
				for ( int tex = numtexels; tex > 0; tex--, conv++ ) {

					//NOTE:
					// 16-bit texels have already been swapped to
					// accommodate current endianness.

					word p = *texels++;
					conv->R = colbits_5_to_8[ ( ( p >> 11 ) & 0x1f ) ];
					conv->G = colbits_6_to_8[ ( ( p >> 5 )  & 0x3f ) ];
					conv->B = colbits_5_to_8[ (   p         & 0x1f ) ];
				}

				// this frees the old texture
				FREEMEM( tmap->BitMap );
				tmap->BitMap		= (char *) cvblock;
				tmap->TexelFormat	= TEXFMT_RGB_888;

			// convert to RGBA texture although there is no source alpha
			} else if ( ColorSetupFlags & COLORSETUP_GLIDE3DF_TO_RGBA_8888 ) {

				colrgba_s *cvblock = (colrgba_s *) ALLOCMEM( numtexels * sizeof( colrgba_s ) );
				if ( cvblock == NULL )
					OUTOFMEM( no_tex_conv_mem );
				colrgba_s *conv = cvblock;
				word *texels = (word *) texbitmap;
				for ( int tex = numtexels; tex > 0; tex--, conv++ ) {

					//NOTE:
					// 16-bit texels have already been swapped to
					// accommodate current endianness.

					word p = *texels++;
					conv->R = colbits_5_to_8[ ( ( p >> 11 ) & 0x1f ) ];
					conv->G = colbits_6_to_8[ ( ( p >> 5 )  & 0x3f ) ];
					conv->B = colbits_5_to_8[ (   p         & 0x1f ) ];
					conv->A = 255;
				}

				// this frees the old texture
				FREEMEM( tmap->BitMap );
				tmap->BitMap		= (char *) cvblock;
				tmap->TexelFormat	= TEXFMT_RGBA_8888;

			} else {
				ASSERT( 0 );
			}

		// convert 16-bit glide texture with single-bit alpha
		} else if ( tmap->TexelFormat == TEXFMT_GR_ARGB_1555 ) {

			ASSERT( tmap->TexPalette == NULL );

			// convert to RGBA texture (even if rgb888 is specified)
			if ( ( ColorSetupFlags & COLORSETUP_GLIDE3DF_TO_RGB_888 ) ||
				 ( ColorSetupFlags & COLORSETUP_GLIDE3DF_TO_RGBA_8888 ) ) {

				colrgba_s *cvblock = (colrgba_s *) ALLOCMEM( numtexels * sizeof( colrgba_s ) );
				if ( cvblock == NULL )
					OUTOFMEM( no_tex_conv_mem );
				colrgba_s *conv = cvblock;
				word *texels = (word *) texbitmap;
				for ( int tex = numtexels; tex > 0; tex--, conv++ ) {

					//NOTE:
					// 16-bit texels have already been swapped to
					// accommodate current endianness.

					word p = *texels++;
					conv->R = colbits_5_to_8[ ( ( p >> 10 ) & 0x1f ) ];
					conv->G = colbits_5_to_8[ ( ( p >>  5 ) & 0x1f ) ];
					conv->B = colbits_5_to_8[ (   p         & 0x1f ) ];
					conv->A = colbits_1_to_8[ ( ( p >> 15 ) & 0x01 ) ];
				}

				// this frees the old texture
				FREEMEM( tmap->BitMap );
				tmap->BitMap		= (char *) cvblock;
				tmap->TexelFormat	= TEXFMT_RGBA_8888;

			} else {
				ASSERT( 0 );
			}

		// convert 16-bit glide texture with alpha
		} else if ( tmap->TexelFormat == TEXFMT_GR_ARGB_4444 ) {

			ASSERT( tmap->TexPalette == NULL );

			// convert to RGBA texture (even if rgb888 is specified)
			if ( ( ColorSetupFlags & COLORSETUP_GLIDE3DF_TO_RGB_888 ) ||
				 ( ColorSetupFlags & COLORSETUP_GLIDE3DF_TO_RGBA_8888 ) ) {

				colrgba_s *cvblock = (colrgba_s *) ALLOCMEM( numtexels * sizeof( colrgba_s ) );
				if ( cvblock == NULL )
					OUTOFMEM( no_tex_conv_mem );
				colrgba_s *conv = cvblock;
				word *texels = (word *) texbitmap;
				for ( int tex = numtexels; tex > 0; tex--, conv++ ) {

					//NOTE:
					// 16-bit texels have already been swapped to
					// accommodate current endianness.

					word p = *texels++;
					conv->R = colbits_4_to_8[ ( ( p >>  8 ) & 0x0f ) ];
					conv->G = colbits_4_to_8[ ( ( p >>  4 ) & 0x0f ) ];
					conv->B = colbits_4_to_8[ (   p         & 0x0f ) ];
					conv->A = colbits_4_to_8[ ( ( p >> 12 ) & 0x0f ) ];
				}

				// this frees the old texture
				FREEMEM( tmap->BitMap );
				tmap->BitMap		= (char *) cvblock;
				tmap->TexelFormat	= TEXFMT_RGBA_8888;

			} else {
				ASSERT( 0 );
			}

		// convert alpha-only texture
		} else if ( tmap->TexelFormat == TEXFMT_GR_ALPHA_8 ) {

			ASSERT( tmap->TexPalette == NULL );

			if ( ( ColorSetupFlags & COLORSETUP_GLIDE3DF_TO_RGB_888 ) ||
				 ( ColorSetupFlags & COLORSETUP_GLIDE3DF_TO_RGBA_8888 ) ) {

				if ( ColorSetupFlags & COLORSETUP_ALPHA_8_TO_RGBA_8888 ) {

					colrgba_s *cvblock = (colrgba_s *) ALLOCMEM( numtexels * sizeof( colrgba_s ) );
					if ( cvblock == NULL )
						OUTOFMEM( no_tex_conv_mem );
					colrgba_s *conv = cvblock;
					byte *texels = (byte *) texbitmap;
					for ( int tex = numtexels; tex > 0; tex--, conv++ ) {

						byte p = *texels++;
						conv->R = 255;
						conv->G = 255;
						conv->B = 255;
						conv->A = p;
					}

					// this frees the old texture
					FREEMEM( tmap->BitMap );
					tmap->BitMap		= (char *) cvblock;
					tmap->TexelFormat	= TEXFMT_RGBA_8888;

				} else {

					// simply change format id
					tmap->TexelFormat = TEXFMT_ALPHA_8;
				}

			} else {
				ASSERT( 0 );
			}

		// convert intensity-only texture
		} else if ( tmap->TexelFormat == TEXFMT_GR_INTENSITY_8 ) {

			ASSERT( 0 );
			ASSERT( tmap->TexPalette == NULL );

			if ( ( ColorSetupFlags & COLORSETUP_GLIDE3DF_TO_RGB_888 ) ||
				 ( ColorSetupFlags & COLORSETUP_GLIDE3DF_TO_RGBA_8888 ) ) {

				if ( ColorSetupFlags & COLORSETUP_INTENSITY_8_TO_RGBA_8888 ) {

					colrgba_s *cvblock = (colrgba_s *) ALLOCMEM( numtexels * sizeof( colrgba_s ) );
					if ( cvblock == NULL )
						OUTOFMEM( no_tex_conv_mem );
					colrgba_s *conv = cvblock;
					byte *texels = (byte *) texbitmap;
					for ( int tex = numtexels; tex > 0; tex--, conv++ ) {

						byte p = *texels++;
						conv->R = 255;
						conv->G = 255;
						conv->B = 255;
						conv->A = p;
					}

					// this frees the old texture
					FREEMEM( tmap->BitMap );
					tmap->BitMap		= (char *) cvblock;
					tmap->TexelFormat	= TEXFMT_RGBA_8888;

				} else {

					// simply change format id
					tmap->TexelFormat = TEXFMT_INTENSITY_8;
				}

			} else {
				ASSERT( 0 );
			}

		} else {

			//TODO: support other formats
			ASSERT( 0 );
		}

	} else {

#ifndef SYSTEM_BIG_ENDIAN

		char *texpalette = (char *) tmap->TexPalette;

		// convert palette byte order
		if ( texpalette != NULL ) {

			ASSERT( ( tmap->TexelFormat == TEXFMT_GR_P_8   ) ||
					( tmap->TexelFormat == TEXFMT_GR_AP_88 ) );

			// convert from (A,R,G,B) to (B,G,R,A)
			colrgba_s *scan = (colrgba_s *) texpalette;
			for ( int ict = 256; ict > 0; ict--, scan++ ) {
				colrgba_s col = *scan;
				scan->R = col.A;
				scan->G = col.B;
				scan->B = col.G;
				scan->A = col.R;
			}
		}

#endif // SYSTEM_BIG_ENDIAN

	}
}


// 3df format and lod specifier tables ----------------------------------------
//
static format_3df_s formats_3df[] = {

	{ "rgb332",  	TEXFMT_GR_RGB_332,				1	},
	{ "yiq",     	TEXFMT_GR_YIQ_422,				1	},
	{ "a8",      	TEXFMT_GR_ALPHA_8,				1	},
	{ "i8",      	TEXFMT_GR_INTENSITY_8,			1	},
	{ "ai44",    	TEXFMT_GR_ALPHA_INTENSITY_44,	1	},
	{ "p8",      	TEXFMT_GR_P_8,					1	},
	{ "rsvd1",   	TEXFMT_GR_RSVD1,				1	},
	{ "rsvd2",   	TEXFMT_GR_RSVD2,				1	},
	{ "argb8332",	TEXFMT_GR_ARGB_8332,			2	},
	{ "ayiq8422",	TEXFMT_GR_AYIQ_8422,			2	},
	{ "rgb565",  	TEXFMT_GR_RGB_565,				2	},
	{ "argb1555",	TEXFMT_GR_ARGB_1555,			2	},
	{ "argb4444",	TEXFMT_GR_ARGB_4444,			2	},
	{ "ai88",    	TEXFMT_GR_ALPHA_INTENSITY_88,	2	},
	{ "ap88",    	TEXFMT_GR_AP_88,				2	},

	{ NULL,			0,							0	}
};

static int lod_3df[] = {

	TEXLOD_1,
	TEXLOD_2,
	TEXLOD_4,
	TEXLOD_8,
	TEXLOD_16,
	TEXLOD_32,
	TEXLOD_64,
	TEXLOD_128,
	TEXLOD_256,
	TEXLOD_512,
	TEXLOD_1024,
	TEXLOD_2048,
};


// decode 3df format info (compare strings and retrieve from table) -----------
//
format_3df_s *TDF_DecodeFormatInfo( char *formatspec )
{
	ASSERT( formatspec != NULL );

	// determine format
	format_3df_s *scan = NULL;
	for ( scan = formats_3df; scan->spec; scan++ )
		if ( strcmp( scan->spec, formatspec ) == 0 )
			break;

	if ( scan->spec == NULL ) {
		return NULL;
	}

	return scan;
}


// read texture from 3df file -------------------------------------------------
//
int TDF_LoadTexture( char *fname, int insertindex, char *loaderparams, texfont_s *texfont )
{
	ASSERT( fname != NULL );
	ASSERT( insertindex >= 0 );

	// get file size
	ssize_t fsiz = SYS_GetFileLength( fname );
	if ( fsiz == -1 ) {
		return FALSE;
	}

	// open 3df file
	FILE *fp = SYS_fopen( fname, "rb" );
	if ( fp == NULL ) {
		return FALSE;
	}

	// header fields
    char version[ 7 ];
    char formatspec[ 11 ];
	int  lodsmall;
	int  lodlarge;
	int	 aspectw;
	int  aspecth;

	// scan out header
	if ( fscanf( fp, "3df v%6s %10s lod range: %i %i aspect ratio: %i %i",
		 version, formatspec, &lodsmall, &lodlarge, &aspectw, &aspecth ) != 6 ) {
		SYS_fclose( fp );
		return FALSE;
	}
	version[ 6 ]     = 0;
	formatspec[ 10 ] = 0;

	// read trailing new line
	if ( fgetc( fp ) != '\n' ) {
		SYS_fclose( fp );
		return FALSE;
	}

	// texture info
	int format;
	int texelsize;
	int aspect;
	int width;
	int height;

	// loading parameters may optionally contain a format override
	char *frmspec = loaderparams ? loaderparams : formatspec;

	// decode format info
	format_3df_s *format_3df = TDF_DecodeFormatInfo( frmspec );
	if ( format_3df == NULL ) {
		MSGOUT( "invalid format." );
		SYS_fclose( fp );
		return FALSE;
	}
	format    = format_3df->format;
	texelsize = format_3df->texelsize;

	// validate lod spec
	if ( ( ( lodsmall & ( lodsmall - 1 ) ) != 0 ) ||	// not a power of two?
		 ( ( lodlarge & ( lodlarge - 1 ) ) != 0 ) ||	// not a power of two?
		 ( lodsmall < 1 ) || ( lodsmall > 256 ) ||
		 ( lodlarge < 1 ) || ( lodlarge > 256 ) ||
		 ( lodlarge < lodsmall ) ) {
		MSGOUT( "invalid lod spec." );
		SYS_fclose( fp );
		return FALSE;
	}

	// determine geometry
	int aindx = ( aspectw << 4 ) | aspecth;
	switch ( aindx ) {

		case 0x81:
			aspect = TEXGEO_ASPECT_GR_8x1;
			width  = lodlarge;
			height = lodlarge / 8;
			break;

		case 0x41:
			aspect = TEXGEO_ASPECT_GR_4x1;
			width  = lodlarge;
			height = lodlarge / 4;
			break;

		case 0x21:
			aspect = TEXGEO_ASPECT_GR_2x1;
			width  = lodlarge;
			height = lodlarge / 2;
			break;

		case 0x11:
			aspect = TEXGEO_ASPECT_GR_1x1;
			width  = lodlarge;
			height = lodlarge;
			break;

		case 0x12:
			aspect = TEXGEO_ASPECT_GR_1x2;
			width  = lodlarge / 2;
			height = lodlarge;
			break;

		case 0x14:
			aspect = TEXGEO_ASPECT_GR_1x4;
			width  = lodlarge / 4;
			height = lodlarge;
			break;

		case 0x18:
			aspect = TEXGEO_ASPECT_GR_1x8;
			width  = lodlarge / 8;
			height = lodlarge;
			break;

		default :
			MSGOUT( "invalid aspect ratio." );
			SYS_fclose( fp );
			return FALSE;
	}

	// alloc memblock
	char *fdata = (char *) ALLOCMEM( fsiz );
	if ( fdata == NULL ) {
		MSGOUT( no_tex_file_mem );
		SYS_fclose( fp );
		return FALSE;
	}

	// read in binary part of file
	size_t bytesread = SYS_fread( fdata, 1, fsiz, fp );
	if ( bytesread == 0 ) {
		SYS_fclose( fp );
		return FALSE;
	}

	// close file
	if ( SYS_fclose( fp ) != 0 ) {
		return FALSE;
	}

	//NOTE:
	// the actual amount of data read at this point (bytesread) will
	// be less than fsiz since the header has already been read.
	// note that if the file is read from the package, bytesread will
	// be equal to fsiz, however, since the excess bytes will be
	// read from the next block in the package if the file is not
	// also at the end of the package file.

	// create texture map description
	int scalecode = TEXGEO_SCALE_1;
	TextureMap *tmap = NULL;

	if ( insertindex == NumLoadedTextures ) {

		tmap = (TextureMap *) ALLOCMEM( sizeof( TextureMap ) );
		if ( tmap == NULL )
			OUTOFMEM( no_tex_head_mem );
		memset( tmap, 0, sizeof( TextureMap ) );
		scalecode = IMG_DetermineTexGeoScale( lodlarge );

	} else {

		// keep old texture structure
		tmap = TextureInfo[ insertindex ].texpointer;

		if ( AUX_DONT_OVERLOAD_TEXTURE_GEOMETRY ) {

			// keep the old texture's geometry
			int oldsizu  = TextureInfo[ insertindex ].width;
			int oldsizv  = TextureInfo[ insertindex ].height;
			int oldlarge = ( oldsizu > oldsizv ) ? oldsizu : oldsizv;
			scalecode = IMG_DetermineTexGeoScale( oldlarge );

		} else {

			scalecode = IMG_DetermineTexGeoScale( lodlarge );
		}

		// free texture if not already done.
		// (the texture may have already been freed by the cache)
		char *block = tmap->TexPalette ? (char*)tmap->TexPalette : tmap->BitMap;
		if ( block != NULL ) {
			FREEMEM( block );
		}

		// invalidate texture in cache to prevent
		// texture from failing to download
		if ( !TextModeActive ) {
			R_InvalidateCachedTexture( tmap );
		}

		// reset data pointers
		tmap->BitMap	 = NULL;
		tmap->TexPalette = NULL;
	}

	ASSERT( tmap != NULL );

	#define TDF_PAL_SIZE	( 256 * 4 )

	// create pointers to texture bitmap and palette
	int readpal = ( ( format & TEXFMT_PALETTEDTEXTURE ) != 0 );
	char *texbitmap  = readpal ? ( fdata + TDF_PAL_SIZE ) : fdata;
	char *texpalette = readpal ? fdata : NULL;

	// init fields of texture map structure
	tmap->Flags			= TEXFLG_EXT_GEOMETRY | TEXFLG_LODRANGE_VALID | TEXFLG_CACHE_MAY_FREE;
	tmap->Geometry		= aspect | scalecode;
	tmap->Width			= CeilPow2Exp( width );
	tmap->Height		= CeilPow2Exp( height );
	tmap->LOD_small		= lod_3df[ CeilPow2Exp( lodsmall ) ];
	tmap->LOD_large		= lod_3df[ CeilPow2Exp( lodlarge ) ];
	tmap->BitMap		= texbitmap;
	tmap->TexPalette	= (word *) texpalette;
	tmap->TexelFormat	= format;

	//NOTE:
	// the texture name (TextureMap::TexMapName) is either still
	// valid from the old texture, or it has been reset to NULL
	// and must be set by the caller of this function.
	// e.g. CON_LOAD::ConLoadTexture().

	TextureInfo[ insertindex ].texpointer	  = tmap;
	TextureInfo[ insertindex ].standardbitmap = NULL;
	TextureInfo[ insertindex ].flags		  = TEXINFOFLAG_USERPALETTE;

	if ( ( insertindex == NumLoadedTextures ) ||
		 !AUX_DONT_OVERLOAD_TEXTURE_GEOMETRY ) {
		TextureInfo[ insertindex ].width  = width;
		TextureInfo[ insertindex ].height = height;
	}

	// check if byte order has to be reversed
	if ( texelsize > 1 ) {

		// only 16-bit texels supported!
		ASSERT( texelsize == 2 );

		// ap88 textures have texelsize = 2 AND a palette
		ASSERT( !readpal || ( format == TEXFMT_GR_AP_88 ) );
		ASSERT(  readpal || ( fdata == texbitmap ) );

		// determine size of map in bytes
		size_t mapsize = bytesread;
		if ( readpal ) {
			mapsize -= TDF_PAL_SIZE;
		}

		// not guaranteed, see NOTE above
//		ASSERT( ( mapsize & 0x01 ) == 0x00 );

#ifndef SYSTEM_BIG_ENDIAN

		// reverse byte order of 16-bit words for entire map
		// (words/dwords) in 3DF files are stored msb first)
		for ( unsigned int i = 0; i < mapsize - 1; i+=2 ) {
			byte lo = texbitmap[ i ];
			byte hi = texbitmap[ i + 1 ];
			texbitmap[ i + 1 ] = lo;
			texbitmap[ i ]     = hi;
		}

#endif // SYSTEM_BIG_ENDIAN

	}

	// perform other necessary format conversions
	Convert3DFToInternal( insertindex );

	return TRUE;
}



