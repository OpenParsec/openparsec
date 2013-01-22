/*
 * PARSEC - Image/Texture Helper Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:42 $
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

// local module header
#include "img_supp.h"



// convert numerical aspect ratio to texgeo_aspect spec (glide-style) ---------
//
static dword tex_aspect_to_texgeo_gr[] = {

	0,
	TEXGEO_ASPECT_GR_1x1,
	TEXGEO_ASPECT_GR_2x1,
	0,
	TEXGEO_ASPECT_GR_4x1,
	0,
	0,
	0,
	TEXGEO_ASPECT_GR_8x1,

	0,
	TEXGEO_ASPECT_GR_1x1,
	TEXGEO_ASPECT_GR_1x2,
	0,
	TEXGEO_ASPECT_GR_1x4,
	0,
	0,
	0,
	TEXGEO_ASPECT_GR_1x8,
};


// determine aspect ratio code for texture coordinates ------------------------
//
dword IMG_DetermineTexGeoAspect( dword width, dword height )
{
	ASSERT( ( width > 0 ) && ( height > 0 ) );
	ASSERT( ( width & ( width - 1 ) ) == 0 );
	ASSERT( ( height & ( height - 1 ) ) == 0 );

	if ( width >= height ) {
		dword aspect = width / height;
		if ( aspect > 8 )
			PANIC( 0 );
		return tex_aspect_to_texgeo_gr[ aspect ];
	} else {
		dword aspect = height / width;
		if ( aspect > 8 )
			PANIC( 0 );
		return tex_aspect_to_texgeo_gr[ aspect + 9 ];
	}
}


// mapping tables for general aspect ratio codes up to 1024 -------------------
//
static dword tex_aspect_to_texgeo_horizontal[] = {

	0x04000001, TEXGEO_ASPECT_1024x1,
	0x02000001, TEXGEO_ASPECT_512x1,
	0x01000001, TEXGEO_ASPECT_256x1,
	0x00800001, TEXGEO_ASPECT_128x1,
	0x00400001, TEXGEO_ASPECT_64x1,
	0x00200001, TEXGEO_ASPECT_32x1,
	0x00100001, TEXGEO_ASPECT_16x1,
	0x00080001, TEXGEO_ASPECT_8x1,
	0x00040001, TEXGEO_ASPECT_4x1,
	0x00020001, TEXGEO_ASPECT_2x1,
	0x00010001, TEXGEO_ASPECT_1x1,
};

static dword tex_aspect_to_texgeo_vertical[] = {

	0x00010002, TEXGEO_ASPECT_1x2,
	0x00010004, TEXGEO_ASPECT_1x4,
	0x00010008, TEXGEO_ASPECT_1x8,
	0x00010010, TEXGEO_ASPECT_1x16,
	0x00010020, TEXGEO_ASPECT_1x32,
	0x00010040, TEXGEO_ASPECT_1x64,
	0x00010080, TEXGEO_ASPECT_1x128,
	0x00010100, TEXGEO_ASPECT_1x256,
	0x00010200, TEXGEO_ASPECT_1x512,
	0x00010400, TEXGEO_ASPECT_1x1024,
};


// determine (extended) aspect ratio code for texture coordinates -------------
//
int IMG_DetermineTexGeoAspectExt( dword width, dword height )
{
	ASSERT( ( width > 0 ) && ( height > 0 ) );
	ASSERT( ( width & ( width - 1 ) ) == 0 );
	ASSERT( ( height & ( height - 1 ) ) == 0 );

	int asp;
	unsigned int aindx;

	if ( width > height ) {
		asp   = width / height;
		aindx = ( asp << 16 ) | 0x0001;
	} else {
		asp = height / width;
		aindx = ( 0x0001 << 16 ) | asp;
	}

	// 1024 at most
	if ( asp > 1024 ) {
		return -1;
	}

	int aspect = -1;

	if ( ( aindx & 0xffff ) == 0x0001 ) {
		int atlen = CALC_NUM_ARRAY_ENTRIES( tex_aspect_to_texgeo_horizontal );
		for ( int ati = 0; ati < atlen; ati += 2 ) {
			if ( tex_aspect_to_texgeo_horizontal[ ati ] == aindx ) {
				aspect = tex_aspect_to_texgeo_horizontal[ ati + 1 ];
				break;
			}
		}
	} else {
		int atlen = CALC_NUM_ARRAY_ENTRIES( tex_aspect_to_texgeo_vertical );
		for ( int ati = 0; ati < atlen; ati += 2 ) {
			if ( tex_aspect_to_texgeo_vertical[ ati ] == aindx ) {
				aspect = tex_aspect_to_texgeo_vertical[ ati + 1 ];
				break;
			}
		}
	}

	return aspect;
}


// determine scale code for texture coordinates -------------------------------
//
dword IMG_DetermineTexGeoScale( dword maxside )
{
	ASSERT( ( maxside > 0 ) && ( maxside <= 1024 ) );
	ASSERT( ( maxside & ( maxside - 1 ) ) == 0 );

	//NOTE:
	// texture coordinate scale factors take care of the mapping from texture
	// coordinates as stored in an object to native api coordinates. object
	// texture coordinates are stored unnormalized, i.e., they are actual
	// absolute coordinates where [0, length-1] covers the texture once.
	// in glide, the larger side of a texture always has to be scaled to 256.
	// the smaller side has to be scaled using the same scale factor, thus
	// preserving the aspect ratio (which is supplied to the api).
	// in opengl, both sides need to be scaled to 1.0, i.e., the aspect ratio
	// is not preserved in the coordinates themselves.
	// in order to derive a scale factor for a texture that can be used by all
	// renderers, the factor determined here scales the larger side to 256 if
	// it is smaller or equal to 256 (small textures). larger textures simply
	// store the power of two of the largest side (not supported by glide).

	// default: take as is
	dword scalecode = TEXGEO_SCALE_1;

	if ( maxside < 256 ) {

		// glide-style scale factor (1..256)
		scalecode = ( ( 256 / maxside ) << TEXGEO_SCALESHIFT ) & TEXGEO_SCALEMASK;

		// glide-style scale factor as power of two (0..8)
		dword sl2 = 0;
		for ( dword sc = TEXGEO_SCALE_1; sc < scalecode; sc <<= 1 )
			++sl2;
		ASSERT( sl2 <= 8 );
		scalecode |= ( sl2 << TEXGEO_SCALE2SHIFT ) & TEXGEO_SCALE2MASK;

	} else if ( maxside > 256 ) {

		// extended scaling is not to 256 anymore, but to 1024
		// only the power of two version is stored as table index
		dword sl2 = ( maxside == 1024 ) ? 10 : 9;
		scalecode = ( sl2 << TEXGEO_SCALE2SHIFT ) & TEXGEO_SCALE2MASK;
	}

	return scalecode;
}


// determine number of texels in a texture including all mipmap levels --------
//
dword IMG_CountMipmapTexels( dword texw, dword texh, dword lodlarge, dword lodsmall )
{
	ASSERT( lodlarge >= lodsmall );

	dword numtexels = texw * texh;
	dword miptexels = numtexels;

	// count additional texels of lower mipmap levels
	for ( dword lev = lodlarge; lev > lodsmall; lev-- ) {

		//NOTE:
		// the next lower mipmap level not always has one forth
		// the texels of its predecessor. it has half the texels
		// for an aspect ratio != 1 if the shorter side has
		// already reached length 1.

		ASSERT( ( texw > 1 ) || ( texh > 1 ) );

		if ( texw > 1 ) {
			texw >>= 1;
			miptexels >>= 1;
		}
		if ( texh > 1 ) {
			texh >>= 1;
			miptexels >>= 1;
		}

		numtexels += miptexels;
	}

	return numtexels;
}



