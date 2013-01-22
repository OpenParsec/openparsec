/*
 * PARSEC - Interface/Utility Functions
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
#include "img_api.h"

// proprietary module headers
#include "img_3df.h"
#include "img_jpg.h"
#include "img_tga.h"
#include "img_load.h"
#include "sys_path.h"

// image file saving
#include <stb_image/stb_image_write.h>


// image file extensions ------------------------------------------------------
//
static const char img_ext_raw[]	= ".raw";
static const char img_ext_tga[]	= ".tga";
static const char img_ext_3df[]	= ".3df";
static const char img_ext_pcx[]	= ".pcx";
static const char img_ext_bmp[]	= ".bmp";
static const char img_ext_jpg[]	= ".jpg";
static const char img_ext_png[] = ".png";


// determine format of specified image file -----------------------------------
//
int IMG_DetermineFormat( char *fname )
{
	ASSERT( fname != NULL );

	// determine image type from extension
	char *fext = SYSs_ScanToExtension( fname );

	//TODO:
	// more elaborate checks (file sig or something).

	if ( strcmp( fext, img_ext_raw ) == 0 )
		return IMAGEFORMAT_RAW;
	else if ( strcmp( fext, img_ext_tga ) == 0 )
		return IMAGEFORMAT_TGA;
	else if ( strcmp( fext, img_ext_3df ) == 0 )
		return IMAGEFORMAT_3DF;
	else if ( strcmp( fext, img_ext_pcx ) == 0 )
		return IMAGEFORMAT_PCX;
	else if ( strcmp( fext, img_ext_bmp ) == 0 )
		return IMAGEFORMAT_BMP;
	else if ( strcmp( fext, img_ext_jpg ) == 0 )
		return IMAGEFORMAT_JPG;
	else if ( strcmp( fext, img_ext_png ) == 0 )
		return IMAGEFORMAT_PNG;

	return IMAGEFORMAT_INVALID;
}


// determine pixel size of image buffer of specified format -------------------
//
int IMG_BufferPixelSize( int format )
{
	int pixsiz = 0;
	switch ( format ) {

		case BUFFERFORMAT_INDEXED:
		case BUFFERFORMAT_RGB_332:
		case BUFFERFORMAT_ALPHA_8:
		case BUFFERFORMAT_INTENSITY_8:
			pixsiz = 1;
			break;

		case BUFFERFORMAT_RGB_555:
		case BUFFERFORMAT_RGB_565:
		case BUFFERFORMAT_ARGB_1555:
		case BUFFERFORMAT_ARGB_4444:
		case BUFFERFORMAT_AP_88:
		case BUFFERFORMAT_AI_88:
			pixsiz = 2;
			break;

		case BUFFERFORMAT_RGB_888:
			pixsiz = 3;
			break;

		case BUFFERFORMAT_ARGB_8888:
			pixsiz = 4;
			break;
	}

	return pixsiz;
}


// copy buffer of one format to another buffer of another format --------------
//
int IMG_CopyConvertBuffer( char *dstbuff, int dstfmt, char *srcbuff, int srcfmt, size_t bsiz )
{
	ASSERT( dstbuff != NULL );
	ASSERT( srcbuff != NULL );

	// just copy over if formats identical
	if ( dstfmt == srcfmt ) {
		memcpy( dstbuff, srcbuff, bsiz );
		return TRUE;
	}

//TODO:


	return FALSE;
}


// save image buffer to a format supported by STB_image_write ------------------
//
PRIVATE
int IMG_SaveGenericTexture(const char *filename, const char *buffer, img_info_s *info)
{
	ASSERT(info->width != 0);
	ASSERT(info->height != 0);
	ASSERT(info->buffsize != 0);

	int components = info->buffsize / (info->width * info->height);

	int success = FALSE;

	switch (info->format) {
		case IMAGEFORMAT_TGA:
			success = stbi_write_tga(filename, info->width, info->height, components, buffer);
			break;
		case IMAGEFORMAT_PNG:
			success = stbi_write_png(filename, info->width, info->height, components, buffer, 0);
			break;
		case IMAGEFORMAT_BMP:
			success = stbi_write_bmp(filename, info->width, info->height, components, buffer);
			break;
		default:
			break;
	}

	return success;
}


// save image buffer in specified format --------------------------------------
//
int IMG_SaveBuffer( const char *filename, char *buff, img_info_s *info )
{
	ASSERT( filename != NULL );
	ASSERT( buff != NULL );
	ASSERT( info != NULL );

	switch (info->format) {
		case IMAGEFORMAT_PNG:
		case IMAGEFORMAT_BMP:
			if (IMG_SaveGenericTexture(filename, buff, info)) {
				return TRUE;
			}
			MSGOUT("Couldn't save image \"%s\" to disk!\n", filename);
			break;
		case IMAGEFORMAT_TGA:
			ASSERT((size_t)(info->width * info->height * 3) == info->buffsize);
			return TGA_SaveBuffer(filename, buff, info->width, info->height);
			break;
		default:
			MSGOUT("Couldn't save image \"%s\" to disk: unrecognized format\n", filename);
			break;
	}

	// write buffer in RAW format
	/*if ( info->format == IMAGEFORMAT_RAW ) {

		size_t numwritten = fwrite( buff, 1, info->buffsize, fp );
		return ( numwritten == info->buffsize );
	}*/

	/*
	// write buffer in JPEG format
	if ( info->format == IMAGEFORMAT_JPG ) {

		ASSERT( (size_t)(info->width * info->height * 3) == info->buffsize );
		return JPG_SaveBuffer( fp, buff, info->width, info->height );
	}
	*/

	return FALSE;
}


// load texture from image file -----------------------------------------------
//
int IMG_LoadTexture( char *fname, int insertindex, char *loaderparams, texfont_s *texfont )
{
	ASSERT( fname != NULL );

	// determine texture format
	int format = IMG_DetermineFormat( fname );

	switch (format) {
		case IMAGEFORMAT_3DF:
			return TDF_LoadTexture( fname, insertindex, loaderparams, texfont );
			break;
		case IMAGEFORMAT_JPG:
		case IMAGEFORMAT_PNG:
		case IMAGEFORMAT_TGA:
			return IMG_LoadGenericTexture(fname, insertindex, loaderparams, texfont);
			break;
	}

	return FALSE;
}



