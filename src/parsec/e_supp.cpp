/*
 * PARSEC - Supporting Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:22 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2001
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

// rendering subsystem
#include "r_patch.h"
#include "r_supp.h"

// subsystem headers
#include "sys_defs.h"
#include "vid_defs.h"

// local module header
#include "e_supp.h"

// proprietary module headers
#include "con_aux.h"
#include "con_com.h"
#include "con_main.h"
#include "e_demo.h"
#include "h_supp.h"
#include "img_api.h"
#include "obj_ctrl.h"
#include "part_sys.h"


// flags
#define SKIP_DEFAULT_BITMAPS_ON_RESOLVE



// init variables for reference frame measurement -----------------------------
//
void InitRefFrameVars()
{
	FrameRate		= 0;
	FrameCounter	= 0;
	ScreenFrameBase = 0;
	PacketFrameBase = 0;

//	ShowFrameRate	= 0;

	// init system dependent reference frame count
	SYSs_InitRefFrameCount();
}


// this waits until next frame time if frame rate is higher than set max ------
//
INLINE
void AwaitFrameStart()
{
	// wait until minimum difference of reference frames reached
	while ( ( SYSs_GetRefFrameCount() - ScreenFrameBase ) <	FixedFrameRateVal )
		SYSs_Yield();
}


// calc ref frame time values -------------------------------------------------
//
void DoFrameTimeCalculations()
{
	// allow fixing refframes for timedemo
	if ( DEMO_TimedemoActive() ) {

		// calc number of elapsed reference frames
		refframe_t refframecount = SYSs_GetRefFrameCount();
		CurPacketRefFrames = refframecount - PacketFrameBase;
		ScreenFrameBase    = refframecount;

		// replay "fixed" frame rate as fast as we can
		CurScreenRefFrames = DEMO_GetTimedemoBase();

	} else {

		// fix frame rate to given maximum if frame rate too
		// high and a maximum has been specified by the user
		if ( Op_FixFrameRate ) {
			AwaitFrameStart();
		}

		// calc number of elapsed reference frames
		refframe_t refframecount = SYSs_GetRefFrameCount();
		
		if (ScreenFrameBase == 0)
			ScreenFrameBase = refframecount;
		
		if (PacketFrameBase == 0)
			PacketFrameBase = refframecount;
		
		CurScreenRefFrames = refframecount - ScreenFrameBase;
		CurPacketRefFrames = refframecount - PacketFrameBase;
		ScreenFrameBase    = refframecount;

		// check safety bounds of reference frame count
		if ( CurScreenRefFrames > MaxRefFrameVal ) {
			CurScreenRefFrames = MaxRefFrameVal;
		} else if ( CurScreenRefFrames < 2 ) {
			CurScreenRefFrames = 2;
		}
	}
}


// valid values of AUXDATA_SCREENSHOT_FORMAT ----------------------------------
//
#define SCREENSHOT_FORMAT_TGA	0
#define SCREENSHOT_FORMAT_PNG	1
#define SCREENSHOT_FORMAT_BMP	2
#define SCREENSHOT_FORMAT_RAW	3
#define SCREENSHOT_FORMAT_JPG	4


// save image of current screen -----------------------------------------------
//
void SaveScreenShot()
{
	static char fname[]   		= "parsec00.raw";
	static char shotnum[] 		= "00";
	static char message[] 		= "screenshot 00 written";
	static char write_error[]	= "disk write error";
	static char not_available[]	= "screenshot function not available";
	static char too_many_shots[]= "too many screenshots";

	static char fext_raw[]		= ".raw";
	static char fext_tga[]		= ".tga";
	static char fext_jpg[]		= ".jpg";
	static char fext_png[]		= ".png";
	static char fext_bmp[]		= ".bmp";

	// image info
	img_info_s info;
	info.width  = Screen_Width;
	info.height = Screen_Height;

	// determine output format
	char *fext;
	switch ( AUXDATA_SCREENSHOT_FORMAT ) {
		
		case SCREENSHOT_FORMAT_RAW:
			fext = fext_raw;
			info.format = IMAGEFORMAT_RAW;
			break;

		case SCREENSHOT_FORMAT_TGA:
			fext = fext_tga;
			info.format = IMAGEFORMAT_TGA;
			break;

		case SCREENSHOT_FORMAT_PNG:
			fext = fext_png;
			info.format = IMAGEFORMAT_PNG;
			break;

		case SCREENSHOT_FORMAT_BMP:
			fext = fext_bmp;
			info.format = IMAGEFORMAT_BMP;
			break;

		/*case SCREENSHOT_FORMAT_JPG:
			fext = fext_jpg;
			info.format = IMAGEFORMAT_JPG;
			break;*/

		default:
			CON_AddLine( "invalid screenshot output format set in auxd20." );
			return;
	}

	// try to find available filename
	FILE *fp = NULL;
	int shid = 0;
	for ( shid = 0; shid < 1000; shid++ ) {

		DIG2_TO_STR( shotnum, shid );
		strcpy( fname + 6, shotnum );
		strcpy( fname + 8, fext );

		if ( ( fp = fopen( fname, "rb" ) ) == NULL )
			break;
		fclose( fp );
		fp = NULL;
	}

	// check if name available
	if ( shid >= 1000 ) {
		ShowMessage( too_many_shots );
		return;
	}

	// save screenshot file

	int bufsiz;
	char *shotbuff = VIDs_ScreenshotBuffer( TRUE, &bufsiz );

	if ( ( shotbuff != NULL ) && ( bufsiz > 0 ) ) {

		// set buffer size
		info.buffsize = bufsiz;

		// save buffer
		if ( IMG_SaveBuffer( fname, shotbuff, &info ) ) {
			if ( !AUX_DISABLE_SCREENSHOT_MESSAGE ) {
				message[ 11 ] = shotnum[ 0 ];
				message[ 12 ] = shotnum[ 1 ];
				ShowMessage( message );
			}
		} else {
			ShowMessage( write_error );
		}
	}
	
	VIDs_ScreenshotBuffer( FALSE, NULL );
}


// calc least power of two which is not less than supplied number -------------
//
int CeilPow2Exp( int number )
{
	ASSERT( number > 0 );

	if ( number <= 0 )
		PANIC( 0 );

	int p = 0;
	for ( int val = 1; val < number; val *= 2 )
		p++;

	return p;
}


// fetch texture map via name -------------------------------------------------
//
TextureMap *FetchTextureMap( const char *texname )
{
	ASSERT( texname != NULL );

	//NOTE:
	// texture names are case-sensitive!

	// scan entire table of textures
	for ( int texid = 0; texid < NumLoadedTextures; texid++ ) {
		if ( strcmp( TextureInfo[ texid ].name, texname ) == 0 ) {
			return TextureInfo[ texid ].texpointer;
		}
	}

	// no texture of this name found
	return NULL;
}


// fetch texture map via name -------------------------------------------------
//
texfont_s *FetchTexfont( const char *fontname )
{
	ASSERT( fontname != NULL );

	//NOTE:
	// texfont names are case-sensitive!

	// scan entire table of texfonts
	for ( int fontid = 0; fontid < NumLoadedTexfonts; fontid++ ) {
		if ( strcmp( TexfontInfo[ fontid ].name, fontname ) == 0 ) {
			return TexfontInfo[ fontid ].texfont;
		}
	}

	// no texfont of this name found
	return NULL;
}


// fetch bitmap id via name ---------------------------------------------------
//
int FetchBitmapId( const char *bmname )
{
	ASSERT( bmname != NULL );

#ifdef SKIP_DEFAULT_BITMAPS_ON_RESOLVE

	// scan table of bitmaps (skip default bitmaps)
	for ( int bmid = BM_CONTROLFILE_NUMBER; bmid < NumLoadedBitmaps; bmid++ ) {
		if ( strcmp( BitmapInfo[ bmid ].name, bmname ) == 0 ) {
			return bmid;
		}
	}

#else // SKIP_DEFAULT_BITMAPS_ON_RESOLVE

	// scan entire table of bitmaps
	for ( int bmid = 0; bmid < NumLoadedBitmaps; bmid++ ) {
		if ( strcmp( BitmapInfo[ bmid ].name, bmname ) == 0 ) {
			return bmid;
		}
	}

#endif // SKIP_DEFAULT_BITMAPS_ON_RESOLVE

	// no bitmap of this name found
	return -1;
}


// evict texture data that is resident in an unsupported format ---------------
//
int EvictUnsupportedTextureFormatData()
{
	// skip if no support info available
	if ( VidInfo_SupportedTexFormats == NULL ) {
		return TRUE;
	}

	int evicted = 0;

	// check format of all textures
	for ( int texid = 0; texid < NumLoadedTextures; texid++ ) {

		TextureMap *texmap = TextureInfo[ texid ].texpointer;
		ASSERT( texmap != NULL );

		dword format = texmap->TexelFormat;
		int supported = ( format & TEXFMT_GLIDEFORMAT ) ?
			FALSE : VidInfo_SupportedTexFormats[ format & TEXFMT_FORMATMASK ];

		// keep if format is ok
		if ( supported ) {
			continue;
		}

		// simply free texture data of unsupported format in order to
		// force it to be reloaded/converted by ReloadFreedTextureBitmaps()
		if ( texmap->BitMap != NULL ) {

			char *block = texmap->TexPalette ? (char*)texmap->TexPalette : texmap->BitMap;
			ASSERT( block != NULL );
			FREEMEM( block );
			texmap->BitMap     = NULL;
			texmap->TexPalette = NULL;

			evicted++;
		}
	}

	if ( evicted > 0 ) {
		MSGOUT( "-Evicted %d textures of wrong format.", evicted );
	}

	return TRUE;
}


// reload bitmap data for an existing texture ---------------------------------
//
PRIVATE
int ReloadTextureBitmap( int texid )
{
	textureinfo_s *textab = &TextureInfo[ texid ];

	// make sure we load from original location
	int oldpackdisabling = AUX_DISABLE_PACKAGE_DATA_FILES;
	if ( textab->flags & TEXINFOFLAG_PACKWASDISABLED ) {
		AUX_DISABLE_PACKAGE_DATA_FILES = 1;
	} else {
		AUX_DISABLE_PACKAGE_DATA_FILES = 0;
	}

	// reload the texture data if it is still available
	int rc = IMG_LoadTexture( textab->file, texid, textab->loaderparams, NULL );

	// restore pack disabling flag
	AUX_DISABLE_PACKAGE_DATA_FILES = oldpackdisabling;

	// if we cannot locate the original texture we're screwed
	if ( !rc ) {
		PANIC( "Could not restore texture data." );
	}

	return rc;
}

// make sure texture data that got freed by the cache is reloaded -------------
//
int ReloadFreedTextureBitmaps()
{
	int reloaded = 0;

	// scan entire table of textures and reload source data
	// if it has been freed by the texture cache
	for ( int texid = 0; texid < NumLoadedTextures; texid++ ) {

		TextureMap *texmap = TextureInfo[ texid ].texpointer;
		ASSERT( texmap != NULL );

		if ( texmap->BitMap == NULL ) {
			ReloadTextureBitmap( texid );
			if ( ++reloaded == 1 ) {
				MSGOUT( "-Restoring evicted texture data..." );
			}
		}
	}

	if ( reloaded > 0 ) {
		MSGOUT( "-Restored %d textures.", reloaded );
		return TRUE;
	}

	return FALSE;
}


// free all objects and particles ---------------------------------------------
//
void KillAllObjects()
{
	//NOTE:
	// the order of the following calls is crucial.
	// 3-D objects MUST BE freed before particle objects.

	FreeObjects();
	FreeParticles();
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( E_SUPP )
{
	
}


