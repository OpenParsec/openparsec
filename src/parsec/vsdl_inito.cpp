/*
 * PARSEC - Video Init Functions (SDL)
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:37 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1999
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
#include "config.h"

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
#include "vsdl_inito.h"

// proprietary module headers
#include "con_aux.h"
#include "con_ext.h"
#include "e_color.h"
#include "vsdl_ogl.h"


static const char * initscript = "sys_gl";


// init display (video mode) using global vars --------------------------------
//
void VIDs_InitDisplay()
{
	// switch to desired video mode
	VSDL_InitOGLMode();

	// set gamma correction
	VIDs_SetGammaCorrection( GammaCorrection );

	// clear text mode flag
	TextModeActive = FALSE;
}


// restore display to text mode -----------------------------------------------
//
void VIDs_RestoreDisplay()
{
	// exit function if no graphics mode active
	if ( TextModeActive )
		return;

	// destroy canvas
	VSDL_ShutDownOGL();

	// set text mode flag once again
	TextModeActive = TRUE;
}


// calculate the global projective projection-matrix --------------------------
//
void VIDs_CalcProjectiveMatrix()
{
	VSDL_CalcProjectiveMatrix();
}


// calculate the global orthographic projection-matrix ------------------------
//
void VIDs_CalcOrthographicMatrix()
{
	VSDL_CalcOrthographicMatrix();
}


// execute subsystem initialization script ------------------------------------
//
PRIVATE
void SDL_ExecSubsysScript()
{
	//NOTE:
	// this function executes a script that can be
	// used to reload critical data on each video
	// subsystem switch.

	// only when non-initial subsystem init
	extern int console_init_done;
	if ( !console_init_done )
		return;

	if ( TextModeActive ) {
		MSGPUT( "Converting data for OpenGL rendering..." );
	}

	// put up conversion dialog
//	SM_ShowSplashScreen( TRUE );

	int oldstate_texture = AUX_ENABLE_TEXTURE_OVERLOADING;
	AUX_ENABLE_TEXTURE_OVERLOADING = 1;
	AUX_ALLOW_ONLY_TEXTURE_LOADING = 1;
	int oldstate_write = AUX_CMD_WRITE_DISABLE_OUTPUT;
	AUX_CMD_WRITE_DISABLE_OUTPUT = 1;

	// execute video subsystem init script
	int rc = ExecExternalFile( (char *) initscript );

	AUX_ENABLE_TEXTURE_OVERLOADING = oldstate_texture;
	AUX_ALLOW_ONLY_TEXTURE_LOADING = 0;
	AUX_CMD_WRITE_DISABLE_OUTPUT   = oldstate_write;

	if ( TextModeActive ) {
		if ( rc && AUX_CMD_WRITE_ACTIVE_IN_TEXTMODE ) {
			MSGOUT( "\nConversion finished." );
		} else {
			MSGOUT( "done." );
		}
	}
	
//	SM_DestroySplashScreen();
}


// tables for supported texture formats ---------------------------------------
//
static int texfmt_general[] = {

	FALSE,		// TEXFMT_STANDARD
	FALSE,		// TEXFMT_RGB_565
	FALSE,		// TEXFMT_RGBA_1555
	TRUE,		// TEXFMT_RGB_888
	TRUE,		// TEXFMT_RGBA_8888
	TRUE,		// TEXFMT_ALPHA_8
	TRUE,		// TEXFMT_INTENSITY_8
	TRUE,		// TEXFMT_LUMINANCE_8
};


// init internal video subsystem-specific flags -------------------------------
//
PRIVATE
void SDL_VideoFlagsSetup()
{
	// basic 32bpp conversion flags
	dword base32 = 0;
	base32 |= COLORSETUP_32BPP_PAL_SWAP_RGB;
	base32 |= COLORSETUP_32BPP_PAL_SET_ALPHA;
//	base32 |= COLORSETUP_32BPP_BITMAP_YINVERSE;
	base32 |= COLORSETUP_32BPP_FONT_YINVERSE;
	base32 |= COLORSETUP_32BPP_ENABLE_TRANSPARENCY;
	base32 |= COLORSETUP_32BPP_CONVERT_SCALE_BITMAPS;

	//NOTE:
	// COLORSETUP_32BPP_BITMAP_YINVERSE must be specified if
	// glDrawPixels() is used for bitmaps instead of textures.

	// texture conversion flags
	dword textures = 0;
	textures |= COLORSETUP_STANDARD_TO_RGB_888;
	textures |= COLORSETUP_GLIDE3DF_TO_RGB_888;		// RGB-only allowed
//	textures |= COLORSETUP_GLIDE3DF_TO_RGBA_8888;	// no RGB-only: use RGBA
	textures |= COLORSETUP_ALPHA_8_TO_RGBA_8888;	// no alpha-only textures

	// font conversion flags
	dword fonts = 0;
//	fonts |= COLORSETUP_FONT_TO_TEXTURE_ALPHA;		// alpha-only fonts
	fonts |= COLORSETUP_FONT_TO_TEXTURE_RGBA;		// no alpha-only fonts

	// color swapping flags
	dword colswap = 0;
	colswap |= COLORSETUP_SWAP_BITMAPS_32BPP;
	colswap |= COLORSETUP_SWAP_FONTS_32BPP;

	// set color translation for opengl
	ColorSetupFlags = base32 | textures | fonts | colswap;

	// set relevant vidinfo flags
	VidInfo_UseIterForDemoTexts = TRUE;				// demo texts as textures
	VidInfo_MaxTextureSize		= 2048;				// may decrease on mode init
	VidInfo_MaxTextureLod		= TEXLOD_2048;		// may decrease on mode init
	VidInfo_NumTextureUnits		= 16;

	// set supported texture formats
	VidInfo_SupportedTexFormats	= texfmt_general;
}


// init api responsible for frame buffer (wgl) --------------------------------
//
void VIDs_InitFrameBufferAPI()
{
	// init opengl and fill in available video modes
	if ( !VSDL_InitOGLInterface( PrintVidModeList ) ) {

		MSGOUT( "OpenGL could not be initialized.\n\n" );
		PANIC ( "OpenGL (SDL) init error." );
	}

	// set video flags (internal config)
	SDL_VideoFlagsSetup();

	// execute script if available
	SDL_ExecSubsysScript();
}
