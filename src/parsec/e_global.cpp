/*
 * PARSEC - Global Variables
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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"
#include "od_class.h"

// subsystem headers
#include "vid_defs.h"

// local module header
#include "e_global.h"

// for definition of MAX_AUX_ENABLING and MAX_AUX_DATA
#include "con_aux.h"



// counters for numbers of textures/objects/bitmaps ---------------------------
//
int 	NumLoadedObjects		= 0;
int 	NumLoadedTextures		= 0;
int 	NumLoadedTexfonts		= 0;
int 	NumLoadedBitmaps		= 0;
int 	NumLoadedCharsets		= 0;
int 	NumLoadedSamples		= 0;
int 	NumLoadedSongs			= 0;
int 	NumLoadedPalettes		= 0;


// info structures for loaded data --------------------------------------------
//
objectinfo_s	ObjectInfo[ MAX_DISTINCT_OBJCLASSES ];
textureinfo_s	TextureInfo[ MAX_TEXTURES ];
texfontinfo_s	TexfontInfo[ MAX_TEXFONTS ];
bitmapinfo_s	BitmapInfo[ MAX_BITMAPS ];
charsetinfo_s	CharsetInfo[ MAX_CHARSETS ];
sampleinfo_s	SampleInfo[ MAX_SAMPLES ];
songinfo_s		SongInfo[ MAX_SONGS ];


// current working directory (for general use) --------------------------------
//
char	CurWorkDir[ PATH_MAX + 1 ] = ".";


// player name forced by command line option ----------------------------------
//
char	ForcedPlayerName[ MAX_PLAYER_NAME + 1 ];


// advanced rendering options -------------------------------------------------
//
int 	ParticleSysEnabled		= TRUE;		// drawing of particles
int 	ZBufferEnabled			= TRUE;		// Z Buffer
int 	ParticlesTranslucent	= FALSE;	// particle translucency


// pointer to programname (used in error messages) ----------------------------
//
char*	sys_ProgramName;


// pointer to texture memory (single block for all textures) ------------------
//
char*	TextureMem				= NULL;


// pointer to bitmap memory (single block for all bitmaps) --------------------
//
char*	BitmapMem				= NULL;


// pointer to sample memory (single block for all samples) --------------------
//
char*	SampleMem				= NULL;


// buffer for color palette ---------------------------------------------------
//
char*	PaletteMem				= NULL;


// table for color index to visual translation --------------------------------
//
visual_t	IndexToVisualTab[ VGA_PAL_NUMCOLORS ];


// table for color index to RGBA translation ----------------------------------
//
colrgba_s	IndexToRGBATab[ VGA_PAL_NUMCOLORS ];


// global color definitions ---------------------------------------------------
//
colrgba_s PanelBackColor		= {  15,  37,  67, 160 };
colrgba_s PanelTextColor		= { 120, 255, 255, 100 };
colrgba_s FlareBaseColor		= { 255, 255, 255, 102 };

colrgba_s LightColorAmbient		= { 255, 255, 255, 255 };
colrgba_s LightColorDiffuse		= { 255, 255, 255, 255 };
colrgba_s LightColorSpecular	= { 255, 255, 255, 255 };


// global directional light source --------------------------------------------
//
Vector3	GlobalDirLight			= { GEOMV_0, GEOMV_0, GEOMV_1, 0 };


// polygon count in current frame ---------------------------------------------
//
int     NumRenderedPolygons;


// frame measurement counters -------------------------------------------------
//
refframe_t 	CurScreenRefFrames	= 0;
refframe_t 	ScreenFrameBase		= 0;
refframe_t 	CurPacketRefFrames	= 0;
refframe_t 	PacketFrameBase		= 0;
refframe_t	RefFrameFrequency	= DEFAULT_REFFRAME_FREQUENCY;


// frame counter (so visibility flags need not be reset explicitly) -----------
//
dword		CurVisibleFrame		= VISFRAME_START;


// flag whether frame rate should be displayed --------------------------------
//
int 		ShowFrameRate		= 0;


// frame rate fixing ----------------------------------------------------------
//
int 		FixedFrameRate		= 300;
refframe_t 	FixedFrameRateVal	= FRAME_MEASURE_TIMEBASE / 300;


// maximum refframe count between real frames ---------------------------------
//
refframe_t	MaxRefFrameVal		= ( 20 * 8 );


// flag if currently recording ------------------------------------------------
//
int		RecordingActive	    	= FALSE;


// file pointer to recording file ---------------------------------------------
//
FILE*	RecordingFp				= NULL;


// remote packet recording ----------------------------------------------------
//
int		RecordRemotePackets 	= FALSE;
int		RemoteRecSessionId  	= 0;
int		RemoteRecPacketId   	= 0;


// availability and startup flags ---------------------------------------------
//
int 	SoundAvailable			= FALSE;
int		SoundDisabled			= FALSE;
int 	SkipCalibrationCode 	= FALSE;
int 	JoystickDisabled		= FALSE;
int 	DirectNetPlay			= FALSE;
int		PrintVidModeList		= FALSE;
int		PlayDemo				= FALSE;
int		FloatingMenu			= TRUE;
int		EnableLogWindow			= FALSE;


// flag whether joystick motion should be queried -----------------------------
//
int 	QueryJoystick			= FALSE;


// for user input disabling during demo replay --------------------------------
//
int		UserInputDisabled		= FALSE;


// lens flare control ---------------------------------------------------------
//
int 	DoLensFlare 			= TRUE;
int 	FlareIntensity			= 0;


// console variables ----------------------------------------------------------
//
int 	ConsoleSliding			= 0;
int 	ConsoleHeight			= 16;


// interpolation of player's actions ------------------------------------------
//
bams_t	CurYaw					= BAMS_DEG0;
bams_t	CurPitch				= BAMS_DEG0;
bams_t	CurRoll 				= BAMS_DEG0;
geomv_t	CurSlideHorz			= GEOMV_0;
geomv_t	CurSlideVert			= GEOMV_0;


// recording actions ----------------------------------------------------------
//
bams_t	RecYaw					= BAMS_DEG0;
bams_t	RecPitch				= BAMS_DEG0;
bams_t	RecRoll					= BAMS_DEG0;
geomv_t	RecSlideHorz			= GEOMV_0;
geomv_t	RecSlideVert			= GEOMV_0;


// recording vars -------------------------------------------------------------
//
bams_t	LastPitch 				= BAMS_DEG0;
bams_t	LastYaw   				= BAMS_DEG0;
bams_t	LastRoll  				= BAMS_DEG0;
geomv_t	LastSlideHorz			= GEOMV_0;
geomv_t	LastSlideVert			= GEOMV_0;
fixed_t	LastSpeed				= 0;
int 	IdleDuration			= 0;


// automatic action parameters ------------------------------------------------
//
int		CurActionWait  			= 0;
bams_t	AutomaticPitch 			= BAMS_DEG0;
bams_t	AutomaticYaw   			= BAMS_DEG0;
bams_t	AutomaticRoll			= BAMS_DEG0;
geomv_t	AutomaticSlideHorz		= GEOMV_0;
geomv_t	AutomaticSlideVert		= GEOMV_0;
fixed_t	AutomaticMovement 		= 0;


// object camera is active in demo currently being replayed -------------------
//
int		ReplayObjCamActive		= FALSE;


// absolute angle positions ---------------------------------------------------
//
bams_t	AbsYaw   				= BAMS_DEG0;
bams_t	AbsPitch 				= BAMS_DEG0;
bams_t	AbsRoll  				= BAMS_DEG0;


// camera matrices ------------------------------------------------------------
//
Camera	ViewCamera;
Camera	ObjectCamera;
Camera	ShipViewCamera;


// flag if object camera active -----------------------------------------------
//
int 		ObjCameraActive		= FALSE;


// local ship -----------------------------------------------------------------
//
int 		LocalShipClass		= SHIP_CLASS_1;
ShipObject*	MyShip				= NULL;
size_t		MyShipMaxInstanceSize = 0;


// singly-linked lists for specific kinds of geometric objects ----------------
//
ShipObject*		PShipObjects	= NULL; // list of existing spacecraft objects
LaserObject*	LaserObjects	= NULL; // list of existing laser objects
MissileObject*	MisslObjects	= NULL; // list of existing missile objects
ExtraObject*	ExtraObjects	= NULL; // list of existing extras (power-ups)
CustomObject*	CustmObjects	= NULL; // list of existing custom objects


// list of currently visible objects ------------------------------------------
//
GenObject*		VObjList		= NULL; // rebuilt each frame


// table of object classes (from which objects can be instantiated) -----------
//
GenObject*		ObjClasses[ MAX_DISTINCT_OBJCLASSES ];


// number of available object classes (object class table entries) ------------
//
int				NumObjClasses			= 0;


// next object number available -----------------------------------------------
//
dword			NextObjNumber			= 1; // 0 is free for special purposes


// current and maximum number of extras ---------------------------------------
//
int 			CurrentNumExtras		= 0;
int				CurrentNumPrtExtras		= 0;
int 			MaxNumExtras			= 20;


// flag indicating that camera has been moved since last frame ----------------
//
int 			CameraMoved				= FALSE;


// flags that screen should flash in color ------------------------------------
//
float 			SetScreenBlue			= 0;
float 			SetScreenWhite			= 0;
int				SetScreenFade			= 0;
int				SetScreenFadeSpeed		= 0;
colrgba_s		SetScreenFadeColor;


// auxiliary enabling flags ---------------------------------------------------
//
int				AuxEnabling[ MAX_AUX_ENABLING ];


// auxiliary data values ------------------------------------------------------
//
int				AuxData[ MAX_AUX_DATA ];



