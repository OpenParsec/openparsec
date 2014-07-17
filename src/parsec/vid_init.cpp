/*
 * PARSEC - Mode Init Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:36 $
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
#include <math.h>
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
#include "inp_defs.h"
#include "sys_defs.h"
#include "vid_defs.h"

// patching headers
#include "d_patch.h"
#include "r_patch.h"

// rendering subsystem
#include "r_supp.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "vid_init.h"

// proprietary module headers
#include "con_main.h"
#include "e_callbk.h"
#include "e_color.h"
#include "e_loader.h"
#include "e_supp.h"
#include "h_drwhud.h"
#include "h_supp.h"
#include "part_sys.h"


// names of control files depending on desired resolution and detail level ----
//
static char ctrl_h_h[] = "ctrl_h_h.dat";
static char ctrl_h_m[] = "ctrl_h_m.dat";
static char ctrl_h_l[] = "ctrl_h_l.dat";
//static char ctrl_l_h[] = "ctrl_l_h.dat";
//static char ctrl_l_m[] = "ctrl_l_m.dat";
//static char ctrl_l_l[] = "ctrl_l_l.dat";


// return control file-name corresponding to detail level and resolution ------
//
char *FetchCtrlFileName()
{
	switch ( Op_DetailLevel ) {

		case DETAIL_HIGH:
			return ctrl_h_h;

		case DETAIL_MEDIUM:
			return ctrl_h_m;

		case DETAIL_LOW:
			return ctrl_h_l;
	}

	return ctrl_h_h;
}


// load data according to resolution ------------------------------------------
//
void LoadResolutionData()
{
	// resolution data for 640x400 and above is the same
	/*if ( ( CurDataResolution >= MODE_HIRES_BASE ) &&
		 ( GameScreenMode >= MODE_HIRES_BASE ) )
		return;*/

	// load resolution-dependent data
//	ReloadResData( FetchCtrlFileName() );
}


// load data according to detail level ----------------------------------------
//
void LoadDetailData()
{
	// load detail-dependent data
	ReloadDetailData( FetchCtrlFileName() );
}


// map video mode specifier string "XRESxYRESxBPP" to video mode array -------
//
void VID_MapSpecifierToMode(char * modespec, int * modearray)
{
	ASSERT( modespec != NULL );

	// strip leading space
	while ( *modespec == ' ' )
		modespec++;

	// convert to lower case
	char _spec[ 15 + 1 ];
	strncpy( _spec, modespec, 15 );
	_spec[ 15 ] = 0;
	strlwr( _spec );
	modespec = _spec;

	// fetch x resolution
	char *scan = modespec;
	while ( ( *scan != 'x' ) && ( *scan != 0 ) )
		scan++;
	if ( *scan != 'x' )
		return;
	*scan++ = 0;

	char *errpart;
	dword xres = strtol( modespec, &errpart, 10 );
	if ( *errpart != 0 )
		return;

	// fetch y resolution
	modespec = scan;
	while ( ( *scan != 'x' ) && ( *scan != 0 ) )
		scan++;
	if ( *scan != 'x' )
		return;
	*scan++ = 0;

	dword yres = strtol( modespec, &errpart, 10 );
	if ( *errpart != 0 )
		return;

	// fetch color depth (strip trailing space)
	modespec = scan;
	while ( ( *scan != ' ' ) && ( *scan != 0 ) )
		scan++;
	if ( ( *scan != ' ' ) && ( *scan != 0 ) )
		return;
	*scan = 0;
	dword bpp = strtol( modespec, &errpart, 10 );
	if ( *errpart != 0 )
		return;
	
	modearray[0] = xres;
	modearray[1] = yres;
	modearray[2] = bpp;
}


// set up data depending on video mode ----------------------------------------
//
void VID_SetupDependentData()
{
	// ensure correct data for selected resolution is loaded
	LoadResolutionData();

	// ensure correct data for selected detail level is loaded
	if ( CurDataDetail != Op_DetailLevel ) {
		LoadDetailData();
		CurDataDetail = Op_DetailLevel;
	}

	// setup data for selected color depth
	if ( CurDataColorBits != GameScreenBPP ) {
		SetupTranslationPalette( PaletteMem + PAL_GAME );
		SetupBitmapColors();
		CurDataColorBits = GameScreenBPP;
	}

	// evict texture data resident in an unsupported format
	EvictUnsupportedTextureFormatData();

	// make sure texture data that might have been freed
	// by the texture cache is available once again
	ReloadFreedTextureBitmaps();

	// precache textures
	R_PrecacheTextures();	
}


// map bpp color mode specification to options menu color mode specification --
//
int VID_MapBppToOpt( int bpp )
{
	int bppopt = -1;

	switch ( bpp ) {

		case MODE_COL_16BPP:
			bppopt = COLMODE_HICOLOR;
			break;

		case MODE_COL_32BPP:
			bppopt = COLMODE_TRUECOLOR;
			break;

		default:
			ASSERT( 0 );
	}

	return bppopt;
}


// determine color depth corresponding to options menu selection --------------
//
int VID_MapOptToBpp( int bppopt )
{
	int bpp = -1;

	switch ( bppopt ) {

		case COLMODE_HICOLOR:
			bpp = MODE_COL_16BPP;
			break;

		case COLMODE_TRUECOLOR:
			bpp = MODE_COL_32BPP;
			break;

		default:
			ASSERT( 0 );
	}

	return bpp;
}


// apply graphics options settings (video mode may be switched!) --------------
//
void VID_ApplyOptions()
{
	//NOTE:
	// this function is used to ensure the currently active
	// graphics mode satisfies the currently set video options.
	// it is not used to switch between text mode and graphics mode.
	// it performs mode switching only if a video option (resolution,
	// color depth, windowed/full-screen) has indeed changed.

	//NOTE:
	// this function is used by VID_PLUG::VID_HotChangeMode() and
	// to switch from menu mode to game mode.

	// determine color depth
	int bpp = VID_MapOptToBpp(Op_ColorDepth);

	// no unavailable mode can be selected
	if (!VID_MODE_AVAILABLE(GetResolutionIndex(Op_Resolution.width, Op_Resolution.height))) {
		Op_Resolution.set(800, 600); // assume 800x600 is always available
	}

	// switch to selected video mode if not already active
	VID_SwitchMode(Op_Resolution.width, Op_Resolution.height, bpp);

	// set up data depending on video mode
	VID_SetupDependentData();

	// init hud geometry
	InitHudDisplay();

	// init sizes of particles
	InitParticleSizes();

	// check console extents
	CheckConExtents();
}


// patch graphics code to new resolution --------------------------------------
//
PRIVATE
void VID_PatchResolution()
{
	R_PatchSubSystem();
}


// calculate viewing volume for current screen geometry -----------------------
//
PUBLIC
void VID_SetViewingVolume()
{
	// world-space screen rectangle (one pixel safety)
	Criterion_X = INT_TO_GEOMV( Screen_XOfs + 1 ) / D_Value;
	Criterion_Y = INT_TO_GEOMV( Screen_YOfs + 1 ) / D_Value;

	// frustum boundary planes

//	PLANE_MAKEAXIAL( &volume[ 0 ],  GEOMV_1, Near_View_Plane );
//	PLANE_MAKEAXIAL( &volume[ 1 ], -GEOMV_1, Far_View_Plane );

	PLANE_NORMAL( &View_Volume[ 0 ] )->X = 0;
	PLANE_NORMAL( &View_Volume[ 0 ] )->Y = 0;
	PLANE_NORMAL( &View_Volume[ 0 ] )->Z = GEOMV_1;
	PLANE_OFFSET( &View_Volume[ 0 ] )	 = Near_View_Plane;

	PLANE_NORMAL( &View_Volume[ 1 ] )->X = 0;
	PLANE_NORMAL( &View_Volume[ 1 ] )->Y = 0;
	PLANE_NORMAL( &View_Volume[ 1 ] )->Z = -GEOMV_1;
	PLANE_OFFSET( &View_Volume[ 1 ] )	 = -Far_View_Plane;

	Vector3 ul, ur, ll, lr;
	ul.X = -Criterion_X;
	ul.Y = -Criterion_Y;
	ul.Z = GEOMV_1;
	ur.X =  Criterion_X;
	ur.Y = -Criterion_Y;
	ur.Z = GEOMV_1;
	ll.X = -Criterion_X;
	ll.Y =  Criterion_Y;
	ll.Z = GEOMV_1;
	lr.X =  Criterion_X;
	lr.Y =  Criterion_Y;
	lr.Z = GEOMV_1;

	CrossProduct( &ul, &ur, PLANE_NORMAL( &View_Volume[ 2 ] ) );
	NormVctX( PLANE_NORMAL( &View_Volume[ 2 ] ) );
	PLANE_OFFSET( &View_Volume[ 2 ] ) = 0;

	CrossProduct( &ur, &lr, PLANE_NORMAL( &View_Volume[ 3 ] ) );
	NormVctX( PLANE_NORMAL( &View_Volume[ 3 ] ) );
	PLANE_OFFSET( &View_Volume[ 3 ] ) = 0;

	CrossProduct( &lr, &ll, PLANE_NORMAL( &View_Volume[ 4 ] ) );
	NormVctX( PLANE_NORMAL( &View_Volume[ 4 ] ) );
	PLANE_OFFSET( &View_Volume[ 4 ] ) = 0;

	CrossProduct( &ll, &ul, PLANE_NORMAL( &View_Volume[ 5 ] ) );
	NormVctX( PLANE_NORMAL( &View_Volume[ 5 ] ) );
	PLANE_OFFSET( &View_Volume[ 5 ] ) = 0;

	// augment view planes by reject/accept points for culling
	CULL_MakeVolumeCullVolume( View_Volume, Cull_Volume, 0x3f );
}


// init resolution-dependent variables ----------------------------------------
//
PRIVATE
void VID_SetResolutionVars()
{
	int modeindx = GetResolutionIndex(GameScreenRes.width, GameScreenRes.height);

	// copy basic mode info from table
	//ASSERT( VID_MODE_AVAILABLE( modeindx ) );
	if( !VID_MODE_AVAILABLE( modeindx) )
	{
		ASSERT( Resolutions.size() > 0 );
		GameScreenRes.width = Resolutions[0].width;
		GameScreenRes.height = Resolutions[0].height;
	}
	int xres = GameScreenRes.width;
	int yres = GameScreenRes.height;
	
	int bpp  = GameScreenBPP;
	
	Screen_Width  = xres;
	Screen_Height = yres;
	
	// set variables according to resolution
	
	Star_Siz = RObj_Siz = (int) 3.0f * ((float) Screen_Height / 720.0f);
	D_Value = Screen_Width * 0.78f; // lower value compared to base screen width = greater FOV

	// init additional geometry variables
	Screen_XOfs		 = Screen_Width  / 2;				// x-midpoint coordinate
	Screen_YOfs		 = Screen_Height / 2;				// y-midpoint coordinate

	// set viewing volume planes
	VID_SetViewingVolume();
	
	Screen_BytesPerPixel = (int) floorf(bpp / 8.0f);
}


// init everything that needs to be changed when graphics mode is changed -----
//
PRIVATE
void VID_InitModeGeometry()
{
	VID_SetResolutionVars();
	VID_PatchResolution();
}


// wrapper for VIDs_InitDisplay() ---------------------------------------------
//
void VID_InitMode()
{
	// set video mode
	VIDs_InitDisplay();
	
	// notify other parts of the game about current video mode
	// by walking the corresponding callback list
	CALLBACK_WalkCallbacks( CBTYPE_VIDMODE_CHANGED );
}


// helper function only used by the following function ------------------------
//
PRIVATE
void VID_PerformSwitch()
{
	//NOTE:
	// even if the resolution stays the same (in case we are only
	// switching between fullscreen and windowed) we need to call
	// VID_InitModeGeometry() to ensure the subsystem knows about
	// the reinitialization. otherwise the texture memory manager
	// wouldn't be informed of the reinitialization, for example.

	// init resolution-dependent vars and code
	VID_InitModeGeometry();

	// actually change video mode
	VID_InitMode();

	// make sure mouse tracking works correctly
	INPs_MouseKillHandler();
	INPs_MouseInitHandler();
}


// switch to another video mode if not already active -------------------------
//
void VID_SwitchMode( int xres, int yres, int bpp )
{
	int resindex = GetResolutionIndex(xres, yres);
	
	ASSERT(VID_MODE_AVAILABLE(resindex));

	//NOTE:
	// this function is used to switch the currently active
	// graphics mode. it is not used to switch between text mode
	// and graphics mode. it performs mode switching only if
	// something (resolution, color depth, windowed/full-screen)
	// has indeed changed.

	//NOTE:
	// this function is used by VID_INIT::VID_ApplyOptions() and
	// to switch from game mode back to menu mode.

	// filter mode-switch if nothing has changed
	if (xres != GameScreenRes.width || yres != GameScreenRes.height || bpp != GameScreenBPP) {

		printf("Resolution change.\n");
		GameScreenRes.set(xres, yres);
		GameScreenBPP  = bpp;
		GameScreenWindowed = Op_WindowedMode;

		// perform actual mode switch
		VID_PerformSwitch();

	} else {

		// even if same mode with respect to resolution and color depth
		// allow switching between full-screen and windowed mode
		printf("No Resolution change.\n");
//		if ( GameScreenWindowed != Op_WindowedMode ) { // disabled so vsync switching will work
				
			GameScreenWindowed = Op_WindowedMode;

			// re-init same mode
			if ( VID_MODE_AVAILABLE( resindex ) ) {
				// perform actual mode switch
				VID_PerformSwitch();
			}
//		}
	}
}


// determine initial video options settings for game screen mode --------------
//
PRIVATE
void VID_SetInitOptions()
{
	// override only if forced by user
	if ( InitOp_FlipSynched != -1 ) {
		FlipSynched = InitOp_FlipSynched;
	}

	// default is base screen mode
	Op_Resolution	= GameScreenRes;
	Op_ColorDepth   = VID_MapBppToOpt( GameScreenBPP );
	Op_WindowedMode = GameScreenWindowed;

	// override with init options if available
	if (InitOp_Resolution.isValid()) {

		// determine windowed/fullscreen
		int opwin = ( InitOp_WindowedMode != -1 ) ?
			InitOp_WindowedMode : Op_WindowedMode;

		// determine init mode for overriding default
		int mindx = GetResolutionIndex(InitOp_Resolution.width, InitOp_Resolution.height);

		// set the init mode if it is valid. the switch will
		// be done on next invocation of VID_ApplyOptions().
		if ( VID_MODE_AVAILABLE( mindx ) ) {
			Op_Resolution	= InitOp_Resolution;
			Op_ColorDepth	= InitOp_ColorDepth;
			Op_WindowedMode	= opwin;
		}

	} else if ( InitOp_WindowedMode != -1 ) {

		// only override windowed/fullscreen; resolution and bpp are default
		int resindex = GetResolutionIndex(GameScreenRes.width, GameScreenRes.height);
		if ( VID_MODE_AVAILABLE( resindex ) ) {
			Op_WindowedMode = InitOp_WindowedMode;
		}
	}
}


// set safe default values for internal video subsystem-specific flags --------
//
PRIVATE
void VID_DefaultVideoFlags()
{
	// reset color setup (conversion) flags
	ColorSetupFlags = COLORSETUP_DEFAULT;

	// set same default values as in VID_GLOB.C
	VidInfo_NumTextureUnits		= 1;
	VidInfo_MaxTextureSize		= 2048;
	VidInfo_MaxTextureLod		= TEXLOD_2048;
	VidInfo_UseIterForDemoTexts	= FALSE;
	VidInfo_SupportedTexFormats	= NULL;
}


// video subsystem initialization ---------------------------------------------
//
void VID_InitSubsystem()
{
	// set video flags to defaults
	VID_DefaultVideoFlags();

	// init frame buffer api and establish available video mode list
	VIDs_InitFrameBufferAPI();

	// set video options for game screen mode
	VID_SetInitOptions();

	// init resolution-dependent vars and code for the first time
	VID_InitModeGeometry();
}



