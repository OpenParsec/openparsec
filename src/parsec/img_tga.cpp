/*
 * PARSEC - TGA Format Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:42 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-2001
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
#include "img_tga.h"

// proprietary module headers
#include "con_aux.h"
#include "e_color.h"
#include "e_supp.h"
#include "img_api.h"
#include "img_conv.h"
#include "img_load.h"
#include "img_supp.h"
#include "sys_file.h"



// string constants -----------------------------------------------------------
//
static const char err_write_header[]	= "error writing tga header.";
static const char err_write_raw_image[]	= "error writing raw tga image data.";
static const char err_write_raw_frame[]	= "error writing tga raw frame.";
static const char err_write_rep_frame[]	= "error writing tga rep frame.";

// tga file types -------------------------------------------------------------
//
#define TGA_NULL		0
#define TGA_CMAP		1
#define TGA_TRUE		2
#define TGA_MONO		3
#define TGA_CMAP_RLE	9
#define TGA_TRUE_RLE	10
#define TGA_MONO_RLE	11


// tga file header ------------------------------------------------------------
//
struct tga_header_s {

    byte IDLength;
    byte CMapType;
    byte ImgType;
    byte CMapStartLo;
    byte CMapStartHi;
    byte CMapLengthLo;
    byte CMapLengthHi;
    byte CMapDepth;
    byte XOffSetLo;
    byte XOffSetHi;
    byte YOffSetLo;
    byte YOffSetHi;
    byte WidthLo;
    byte WidthHi;
    byte HeightLo;
    byte HeightHi;
    byte PixelDepth;
    byte ImageDescriptor;
};

//NOTE:
// the word members of struct tga_header_s are separated into their lo- and
// hi-bytes for independence from endianness and structure packing.

#define GET_CMAPSTART(s)	MAKE_WORD( (s)->CMapStartLo, (s)->CMapStartHi )
#define GET_CMAPLENGTH(s)	MAKE_WORD( (s)->CMapLengthLo, (s)->CMapLengthHi )
#define GET_XOFFSET(s)		MAKE_WORD( (s)->XOffsetLo, (s)->XOffsetHi )
#define GET_YOFFSET(s)		MAKE_WORD( (s)->YOffsetLo, (s)->YOffsetHi )
#define GET_WIDTH(s)		MAKE_WORD( (s)->WidthLo, (s)->WidthHi )
#define GET_HEIGHT(s)		MAKE_WORD( (s)->HeightLo, (s)->HeightHi )


// valid values of AUXDATA_SCREENSHOT_SUBFORMAT when output format is tga -----
//
#define SCREENSHOT_SUBFORMAT_COMPRESSED		0
#define SCREENSHOT_SUBFORMAT_UNCOMPRESSED	1


// save buffer in tga format --------------------------------------------------
//
int TGA_SaveBuffer( const char *filename, char *buff, int width, int height )
{
	ASSERT( filename != NULL );
	ASSERT( buff != NULL );
	ASSERT( width > 0 );
	ASSERT( height > 0 );

	FILE *fp = fopen(filename, "wb");
	if (fp == NULL) {
		MSGOUT("Error opening file \"%s\" for writing.\n", filename);
		return FALSE;
	}

	word cmapstart	= 0;
	word cmaplength	= 0;
	word xoffset	= 0;
	word yoffset	= 0;

	// fill header
	tga_header_s header;
    header.IDLength		= 0;					// no image id field
    header.CMapType		= 0;					// no color map
	header.ImgType		= TGA_TRUE_RLE;			// rle compressed true color
    header.CMapStartLo	= LO_BYTE( cmapstart );
    header.CMapStartHi	= HI_BYTE( cmapstart );
    header.CMapLengthLo	= LO_BYTE( cmaplength );
    header.CMapLengthHi	= HI_BYTE( cmaplength );
    header.CMapDepth	= 0;
    header.XOffSetLo	= LO_BYTE( xoffset );
    header.XOffSetHi	= HI_BYTE( xoffset );
    header.YOffSetLo	= LO_BYTE( yoffset );
    header.YOffSetHi	= HI_BYTE( yoffset );
    header.WidthLo		= LO_BYTE( width );		// width in pixels
    header.WidthHi		= HI_BYTE( width );
    header.HeightLo		= LO_BYTE( height );	// height in pixels
    header.HeightHi		= HI_BYTE( height );
    header.PixelDepth	= 24;					// RGB888 image
    header.ImageDescriptor = 0x20;				// top left scan order

	// allow uncompressed images
	if ( AUXDATA_SCREENSHOT_SUBFORMAT == SCREENSHOT_SUBFORMAT_UNCOMPRESSED )
		header.ImgType = TGA_TRUE;		// uncompressed true color image

	// write header
	size_t numwritten = fwrite( &header, 1, sizeof( header ), fp );
	if ( numwritten != sizeof( header ) ) {
		MSGOUT( err_write_header );
		fclose(fp);
		return FALSE;
	}

	if ( AUXDATA_SCREENSHOT_SUBFORMAT == SCREENSHOT_SUBFORMAT_UNCOMPRESSED ) {

		// 24-bit pixels
		size_t datasize = width * height * 3;

		// swap (R,G,B) byte-order
		byte *curswp = (byte *) buff;
		byte *beyond = curswp + datasize;
		for ( ; curswp < beyond; curswp += 3 ) {
			curswp[ 0 ] ^= curswp[ 2 ];
			curswp[ 2 ] ^= curswp[ 0 ];
			curswp[ 0 ] ^= curswp[ 2 ];
		}

		// write uncompressed data
		numwritten = fwrite( buff, 1, datasize, fp );
		if ( numwritten != datasize ) {
			MSGOUT( err_write_raw_image );
			fclose(fp);
			return FALSE;
		}

	} else {

		ASSERT( AUXDATA_SCREENSHOT_SUBFORMAT == SCREENSHOT_SUBFORMAT_COMPRESSED );

		// compress and save each line separately
		byte *img = (byte *) buff;
		for ( int line = 0; line < height; line++ ) {

			// process all columns of current line
			int curcol = 0;
			while ( curcol < width ) {

				byte cur_r;
				byte cur_g;
				byte cur_b;

				int copycount  = 0;
				byte *copyfrom = img;
				while ( ( copycount < 128 ) && ( curcol < width ) ) {

					// fetch current pixel
					cur_r = img[ 0 ];
					cur_g = img[ 1 ];
					cur_b = img[ 2 ];

					// advance one to the right
					img += 3;
					curcol++;

					// count last pixel in line
					if ( curcol == width ) {
						copycount++;
						break;
					}

					// stop if identical pixels found
					if ( ( img[ 0 ] == cur_r ) &&
						 ( img[ 1 ] == cur_g ) &&
						 ( img[ 2 ] == cur_b ) )
						break;

					// count unequal pixels
					copycount++;
				}

				// save raw frame
				if ( copycount > 0 ) {

					byte cpcnt = ( copycount - 1 );
					if ( fwrite( &cpcnt, 1, 1, fp ) != 1 ) {
						MSGOUT( err_write_raw_frame );
						fclose(fp);
						return FALSE;
					}

					// swap (R,G,B) byte-order
					byte *curswp = copyfrom;
					byte *beyond = copyfrom + copycount * 3;
					for ( ; curswp < beyond; curswp += 3 ) {
						curswp[ 0 ] ^= curswp[ 2 ];
						curswp[ 2 ] ^= curswp[ 0 ];
						curswp[ 0 ] ^= curswp[ 2 ];
					}

					if ( fwrite( copyfrom, 3, copycount, fp ) != (size_t)copycount ) {
						MSGOUT( err_write_raw_frame );
						fclose(fp);
						return FALSE;
					}
				}

				// continue copying if count overflowed
				if ( ( copycount == 128 ) || ( curcol == width ) )
					continue;

				// create rep frame
				int repcount = 1;
				byte *reppix = img;
				while ( ( img[ 0 ] == cur_r ) &&
						( img[ 1 ] == cur_g ) &&
						( img[ 2 ] == cur_b ) &&
						( repcount < 128 ) && ( curcol < width ) ) {

					// count duplicate pixel and advance
					repcount++;
					img += 3;
					curcol++;
				}

				// we found at least two identical pixels
				ASSERT( repcount > 1 );

				// swap (R,G,B) byte-order
				reppix[ 0 ] ^= reppix[ 2 ];
				reppix[ 2 ] ^= reppix[ 0 ];
				reppix[ 0 ] ^= reppix[ 2 ];

				// save rep frame
				byte rpcnt = ( repcount - 1 ) | 0x80;
				if ( fwrite( &rpcnt, 1, 1, fp ) != 1 ) {
					MSGOUT( err_write_rep_frame );
					fclose(fp);
					return FALSE;
				}
				if ( fwrite( reppix, 3, 1, fp ) != 1 ) {
					MSGOUT( err_write_rep_frame );
					fclose(fp);
					return FALSE;
				}
			}
		}
	}

	fclose(fp);

	return TRUE;
}

