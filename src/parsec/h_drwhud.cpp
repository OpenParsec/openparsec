/*
 * PARSEC - Draw Heads Up Display
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:37 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
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
#include <time.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"
#include "od_class.h"

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "inp_defs.h"
#include "net_defs.h"
#include "sys_defs.h"
#include "vid_defs.h"

// drawing subsystem
#include "d_bmap.h"
#include "d_font.h"
#include "d_misc.h"

// rendering subsystem
#include "r_perf.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "h_drwhud.h"

// proprietary module headers
#include "con_aux.h"
#include "e_color.h"
#include "e_demo.h"
#include "e_draw.h"
#include "h_cockpt.h"
#include "h_frmplt.h"
#include "h_radar.h"
#include "h_supp.h"
#include "obj_ctrl.h"
#include "obj_game.h"
#include "g_bot_cl.h"


// flags
#define SHOW_ARMOR_INFO
#define NEW_ONLINE_HELP


// height of remoteplayer name
#define REMPLAYERNAME_HEIGHT 10

// crosshair bitmap to use
int 	bm_crosshair = BM_CROSSHAIR;

// online help geometry constants
#define HELP_FRAME_LEFT 	8
#define HELP_FRAME_TOP		5
#define NUM_HELP_LINES		27
#define HELP_LINE_WIDTH 	27
#define NUM_HELP_LINES_LORES (27-6)
#define HELP_LORES_FILT_LO	19
#define HELP_LORES_FILT_HI	24

// geometry of online help
int Help_Text_X;
int Help_Text_Y;
int Help_Text_LineDist;
int Num_Help_Lines;

// generic string paste area
#define PASTE_STR_LEN 255
static char	paste_str[ PASTE_STR_LEN + 1 ];


// hud coordinates ------------------------------------------------------------
//
int  hud_line_dist;
int  hud_displ;
int  hud_left_x;

int obj_cam_x;
int obj_cam_y;

int Tracking_Width;
int Tracking_Height;

char hud_lockd_str[] = "locked";
char hud_track_str[] = "tracking";

int hud_lockd_x;
int hud_lockd_y;
int hud_track_x;
int hud_track_y;

int TrackingLeftBoundX;
int TrackingRightBoundX;
int TrackingLeftBoundY;
int TrackingRightBoundY;

int hud_crosshair_x;
int hud_crosshair_y;


// init hud geometry for local ship -------------------------------------------
//
PRIVATE
void InitHudGeometryMyShip()
{
	if ( MyShip->ObjectType == SHIP1TYPE ) {
		
		bm_crosshair = BM_CROSSHAIR2;
		
		hud_track_x  = Screen_XOfs - 15 - ( 1-hud_displ );
		hud_track_y  = Screen_YOfs - 44 - hud_displ*8;
		hud_lockd_x  = Screen_XOfs - 11 - ( 1-hud_displ );
		hud_lockd_y  = Screen_YOfs + 37 + hud_displ*10;
		
	} else if ( MyShip->ObjectType == SHIP2TYPE ) {
		
		bm_crosshair = BM_CROSSHAIR;
		
		hud_track_x  = Screen_XOfs - 15 - ( 1-hud_displ );
		hud_track_y  = Screen_YOfs - 7/*52*/ - hud_displ*8;
		hud_lockd_x  = Screen_XOfs - 11 - ( 1-hud_displ );
		hud_lockd_y  = Screen_YOfs + 45 + hud_displ*10;
		
	} else {
		
		bm_crosshair = BM_CROSSHAIR3;
		
		hud_track_x  = Screen_XOfs - 15 - ( 1-hud_displ );
		hud_track_y  = Screen_YOfs - 44 - hud_displ*8;
		hud_lockd_x  = Screen_XOfs - 11 - ( 1-hud_displ );
		hud_lockd_y  = Screen_YOfs + 37 + hud_displ*10;
	}
}


// init hud geometry for radar ------------------------------------------------
//
PRIVATE
void InitHudGeometryRadar()
{
	// must be offset by -<radar_bitmap_width>/2
	hud_radar_bitm_x = Screen_XOfs + hud_displ + 1;

	// must be offset by -<radar_bitmap_height>
	hud_radar_bitm_y = Screen_Height;

	// must be subtracted from <radar_bitmap_width>/2
	hud_radar_w = 3*hud_displ + 3;

	// must be subtracted from <radar_bitmap_height>/2
	hud_radar_h = 3*hud_displ + 3;

	// is valid as is
	hud_radar_x = Screen_XOfs;

	// must be offset by -<radar_bitmap_height>
	hud_radar_y = Screen_Height - 2 + 1;
}


// init hud geometry for object camera ----------------------------------------
//
PRIVATE
void InitHudGeometryObjCamera()
{
	obj_cam_x = 6 + (hud_displ+0)*hud_line_dist;
	obj_cam_y = 6 + (hud_displ+0)*hud_line_dist;
}


// init hud geometry for online help window -----------------------------------
//
PRIVATE
void InitHudGeometryOnlineHelp()
{
	Help_Text_X 		= CharsetInfo[ HUD_CHARSETNO ].width * 3;
	Help_Text_Y 		= hud_line_dist * ( 6 + 4 * hud_displ ) + 7 * hud_displ;
	Help_Text_LineDist	= hud_line_dist;
}


// init hud geometry for message area -----------------------------------------
//
PRIVATE
void InitHudGeometryMessageArea()
{
	Msg_ScreenTop = 4 + hud_displ * CharsetInfo[ MSG_CHARSETNO ].height;
}


// init hud geometry for target tracking --------------------------------------
//
PRIVATE
void InitHudGeometryTracking()
{
	Tracking_Width	= BitmapInfo[ bm_crosshair ].width / 2;
	Tracking_Height = BitmapInfo[ bm_crosshair ].height / 2;

	Tracking_Width	= (int)( Tracking_Width * (float)Screen_Width / 640.0 );
	Tracking_Height	= (int)( Tracking_Height * (float)Screen_Height / 480.0 );

	TrackingLeftBoundX	= Screen_XOfs - Tracking_Width;
	TrackingRightBoundX = Screen_XOfs + Tracking_Width;
	TrackingLeftBoundY	= Screen_YOfs + 5 - Tracking_Height;
	TrackingRightBoundY = Screen_YOfs + 5 + Tracking_Height;
}


// init hud geometry for crosshair --------------------------------------------
//
PRIVATE
void InitHudGeometryCrosshair()
{
	hud_crosshair_x = ((Screen_XOfs - BitmapInfo[ bm_crosshair ].width/2)&~3) + 4;
	hud_crosshair_y = Screen_YOfs - BitmapInfo[ bm_crosshair ].height/2 + 3;
}


// init coordinates of hud display --------------------------------------------
//
void InitHudDisplay()
{
	Hud_Char_ColOfs[ 0 ] = CalcHudCharAddress( 0 );
	Hud_Char_ColOfs[ 1 ] = CalcHudCharAddress( 1 );
	Hud_Char_ColOfs[ 2 ] = CalcHudCharAddress( 2 );
	Hud_Char_ColOfs[ 3 ] = CalcHudCharAddress( 3 );
	Hud_Char_ColOfs[ 4 ] = CalcHudCharAddress( 4 );
	Hud_Char_ColOfs[ 5 ] = CalcHudCharAddress( 5 );

	hud_displ		= 1;
	hud_line_dist	= CharsetInfo[ HUD_CHARSETNO ].height + 2;
	hud_left_x		= 12;
	Num_Help_Lines  = NUM_HELP_LINES;
	
	InitHudGeometryMyShip();

	InitHudGeometryOnlineHelp();
	InitHudGeometryMessageArea();
	InitHudGeometryObjCamera();
	InitHudGeometryTracking();
	InitHudGeometryCrosshair();
	InitHudGeometryRadar();
}


// set string context to MIN_CHARSETNO ----------------------------------------
//
INLINE
void SET_MIN_CHAR_CONTEXT()
{
	D_SetWStrContext( CharsetInfo[ MIN_CHARSETNO ].charsetpointer,
					  CharsetInfo[ MIN_CHARSETNO ].geompointer,
					  NULL,
					  CharsetInfo[ MIN_CHARSETNO ].width,
					  CharsetInfo[ MIN_CHARSETNO ].height );
}


// set string context to HUD_CHARSETNO ----------------------------------------
//
INLINE
void SET_HUD_CHAR_CONTEXT()
{
	D_SetWStrContext( CharsetInfo[ HUD_CHARSETNO ].charsetpointer,
					  CharsetInfo[ HUD_CHARSETNO ].geompointer,
					  NULL,
					  CharsetInfo[ HUD_CHARSETNO ].width,
					  CharsetInfo[ HUD_CHARSETNO ].height );
}


// type for writestring function pointer --------------------------------------
//
typedef void (*WSFP)( ... );

//NOTE:
// this declaration is in global scope because of the extern "C".
// only gcc needs the otherwise redundant curly braces.


#include "inp_keyn.h"

#define NUM_KEYS			28


// text descriptions for gamefunc keys ----------------------------------------
//
static char help_gamefunc_names[ NUM_KEYS ][ HELP_LINE_WIDTH + 1 ] = {

	"escape",
	"turn left",
	"turn right",
	"dive down",
	"pull up",
	"roll left",
	"roll right",
	"shoot weapon",
	"launch missile",
	"next weapon",
	"next missile",
	"accelerate",
	"decelerate",
	"slide left",
	"slide right",
	"slide up",
	"slide down",
	"next target",
	"toggle frame rate info",
	"toggle object camera",
	"toggle help",
	"toggle console",
	"save screenshot",
	"show kill stats",
	"speed zero",
	"target speed",
	"activate after burner",
	"select target in front"
};

static int *keyb_config;


// draw online help for the new millenium -------------------------------------
//
void DrawOnlineHelp()
{
	int helpwidth = 0;
	int keynameoffs = 0;

	keyb_config = (int *) KeyAssignments;
	int kid =0;
	for ( kid = 0; kid < NUM_KEYS; kid++ ) {

		int key_code = keyb_config[ kid ];
		
		if (key_names[ key_code ] == NULL)
			key_code = 0;

		int linewidth = strlen( help_gamefunc_names[ kid ] );

		if ( linewidth > keynameoffs )
			keynameoffs = linewidth;

		linewidth = keynameoffs + strlen( key_names[ key_code ] );

		if ( linewidth > helpwidth )
			helpwidth = linewidth;
	}

	helpwidth   += 4;
	keynameoffs += 1;

	int chr_width = CharsetInfo[ HUD_CHARSETNO ].width;
	int yko 	  = Help_Text_Y;

	// determine whether translucency should be used
	int translucent = VID_TRANSLUCENCY_SUPPORTED;

	int wx = Help_Text_X - HELP_FRAME_LEFT;
	int wy = Help_Text_Y - HELP_FRAME_TOP;
	int ww = chr_width * helpwidth + HELP_FRAME_LEFT * 2;
	int wh = Help_Text_LineDist * NUM_KEYS + HELP_FRAME_TOP * 2;

	if ( translucent ) {
		D_DrawTrRect( wx, wy, ww, wh, TRTAB_PANELBACK );

		DRAW_PanelDecorations( wx, wy, ww, wh );
	}

	SET_HUD_CHAR_CONTEXT();

	// write text transparent only for color depths below 32 bit per pixel
	WSFP wsfp = ( translucent ) ?
				(WSFP) &D_WriteTrString :
				(WSFP) &D_WriteString;

	for ( kid = 0; kid < NUM_KEYS; kid++ ) {

		int key_code = keyb_config[ kid ];
		
		if (key_names[ key_code ] == NULL)
			key_code = 0;

		wsfp( help_gamefunc_names[ kid ] , Help_Text_X, yko, TRTAB_PANELTEXT );
		wsfp( key_names[ key_code ], Help_Text_X + keynameoffs * chr_width, yko, TRTAB_PANELTEXT );

		yko += Help_Text_LineDist;
	}
}


// x position of debug info in hud --------------------------------------------
//
#define DI_XPOS		12


// show network packet info in hud --------------------------------------------
//
INLINE
void HUD_DrawDebugInfoPackets()
{
	#define DI_PACKETS_LINE1	130

	sprintf( paste_str, "last packet ids: %d %d %d %d",
						Player_LastMsgId[ 0 ],
						Player_LastMsgId[ 1 ],
						Player_LastMsgId[ 2 ],
						Player_LastMsgId[ 3 ] );
	D_WriteString( paste_str, DI_XPOS, DI_PACKETS_LINE1 );
}


// show memory allocation statistics in hud -----------------------------------
//
INLINE
void HUD_DrawDebugInfoMemory()
{
	#define DI_MEMORY_LINE1		140
	#define DI_MEMORY_LINE2		(DI_MEMORY_LINE1+10)
	#define DI_MEMORY_LINE3		(DI_MEMORY_LINE2+10)
	#define DI_MEMORY_LINE4		(DI_MEMORY_LINE3+10)

	extern unsigned int Num_Allocs;
	extern unsigned int Num_Frees;
	extern unsigned int Dyn_Mem_Size;

	sprintf( paste_str, "mallocs : %d", Num_Allocs );
	D_WriteString( paste_str, DI_XPOS, DI_MEMORY_LINE1 );

	sprintf( paste_str, "frees   : %d", Num_Frees );
	D_WriteString( paste_str, DI_XPOS, DI_MEMORY_LINE2 );

	sprintf( paste_str, "memobjs : %d", Num_Allocs - Num_Frees );
	D_WriteString( paste_str, DI_XPOS, DI_MEMORY_LINE3 );

	sprintf( paste_str, "heapsize: %d", Dyn_Mem_Size );
	D_WriteString( paste_str, DI_XPOS, DI_MEMORY_LINE4 );
}


// show hud debug info about object lists -------------------------------------
//
INLINE
void HUD_DrawDebugInfoObjects()
{
	#define DI_OBJECTS_LINE1	190
	#define DI_OBJECTS_LINE2	(DI_OBJECTS_LINE1+10)
	#define DI_OBJECTS_LINE3	(DI_OBJECTS_LINE2+10)
	#define DI_OBJECTS_LINE4	(DI_OBJECTS_LINE3+10)
	#define DI_OBJECTS_LINE5	(DI_OBJECTS_LINE4+10)

	ASSERT( PShipObjects != NULL );
	ASSERT( LaserObjects != NULL );
	ASSERT( MisslObjects != NULL );
	ASSERT( ExtraObjects != NULL );
	ASSERT( CustmObjects != NULL );

	GenObject *walklist;
	int 	  listlength;

	listlength = 0;
	for ( walklist = FetchFirstShip(); walklist; walklist = walklist->NextObj )
		listlength++;
	sprintf( paste_str, "ships   : %d", listlength );
	D_WriteString( paste_str, DI_XPOS, DI_OBJECTS_LINE1 );

	listlength = 0;
	for ( walklist = FetchFirstLaser(); walklist; walklist = walklist->NextObj )
		listlength++;
	sprintf( paste_str, "lasers  : %d", listlength );
	D_WriteString( paste_str, DI_XPOS, DI_OBJECTS_LINE2 );

	listlength = 0;
	for ( walklist = FetchFirstMissile(); walklist; walklist = walklist->NextObj )
		listlength++;
	sprintf( paste_str, "missiles: %d", listlength );
	D_WriteString( paste_str, DI_XPOS, DI_OBJECTS_LINE3 );

	listlength = 0;
	for ( walklist = FetchFirstExtra(); walklist; walklist = walklist->NextObj )
		listlength++;
	sprintf( paste_str, "extras  : %d (%d)", listlength, CurrentNumExtras );
	D_WriteString( paste_str, DI_XPOS, DI_OBJECTS_LINE4 );

	sprintf( paste_str, "partobjs: %d", CurrentNumPrtExtras );
	D_WriteString( paste_str, DI_XPOS, DI_OBJECTS_LINE5 );

	//TODO:
	// display custom objects.
/*
	listlength = 0;
	for ( walklist = FetchFirstCustom(); walklist; walklist = walklist->NextObj )
		listlength++;
	sprintf( paste_str, "customs : %d", listlength );
	D_WriteString( paste_str, DI_XPOS, DI_OBJECTS_LINE6 );
*/
}


// show geometry stage statistics ---------------------------------------------
//
INLINE
void HUD_DrawDebugInfoGeometry()
{
	#define DI_GEOMETRY_LINE1	280

	geomperfstats_s perf;
	R_QueryGeometryPerfStats( &perf );

	int triangles_in  = ( perf.fieldstate & GEOMPERF_TRIANGLES_IN ) ? (int)perf.triangles_in  : -1;
	int triangles_out = ( perf.fieldstate & GEOMPERF_TRIANGLES_OUT) ? (int)perf.triangles_out : -1;

	sprintf( paste_str, "triangles: in:%d out:%d", triangles_in, triangles_out);
	D_WriteString( paste_str, DI_XPOS, DI_GEOMETRY_LINE1 );

	// reset counters before next frame
	R_ResetGeometryPerfStats();
}


// show rasterization stage statistics ----------------------------------------
//
INLINE
void HUD_DrawDebugInfoRasterization()
{
	#define DI_RASTERIZATION_LINE1	250
	#define DI_RASTERIZATION_LINE2	(DI_RASTERIZATION_LINE1+10)
	#define DI_RASTERIZATION_LINE3	(DI_RASTERIZATION_LINE2+10)

	rastperfstats_s perf;
	R_QueryRasterizationPerfStats( &perf );

	int fragments	= ( perf.fieldstate & RASTPERF_FRAGMENTS  ) ? (int)perf.fragments  : -1;
	int pixels	= ( perf.fieldstate & RASTPERF_PIXELS     ) ? (int)perf.pixels     : -1;
	int faildepth	= ( perf.fieldstate & RASTPERF_FAILDEPTH  ) ? (int)perf.faildepth  : -1;
	int failalpha	= ( perf.fieldstate & RASTPERF_FAILALPHA  ) ? (int)perf.failalpha  : -1;
	int failchroma	= ( perf.fieldstate & RASTPERF_FAILCHROMA ) ? (int)perf.failchroma : -1;

	sprintf( paste_str, "polygons: genobj:%d", NumRenderedPolygons );
	D_WriteString( paste_str, DI_XPOS, DI_RASTERIZATION_LINE1 );

	sprintf( paste_str, "fragment: in:%d out:%d", fragments, pixels );
	D_WriteString( paste_str, DI_XPOS, DI_RASTERIZATION_LINE2 );

	sprintf( paste_str, "failed  : z:%d a:%d c:%d", faildepth, failalpha, failchroma );
	D_WriteString( paste_str, DI_XPOS, DI_RASTERIZATION_LINE3 );

	// reset counters before next frame
	R_ResetRasterizationPerfStats();
}


// show current camera position in hud ----------------------------------------
//
INLINE
void HUD_DrawDebugInfoPosition()
{
	#define DI_POSITION_LINE1	300
	#define DI_POSITION_LINE2	(DI_POSITION_LINE1+10)
	#define DI_POSITION_LINE3	(DI_POSITION_LINE2+10)
	#define DI_POSITION_LINE4	(DI_POSITION_LINE3+20)
	#define DI_POSITION_LINE5	(DI_POSITION_LINE4+10)
	#define DI_POSITION_LINE6	(DI_POSITION_LINE5+10)
	#define DI_POSITION_LINE7	(DI_POSITION_LINE6+10)
	#define DI_POSITION_LINE8	(DI_POSITION_LINE7+10)
	#define DI_POSITION_LINE9	(DI_POSITION_LINE8+10)

	sprintf( paste_str, "pitch: %d", (short)AbsPitch / 256  );
	D_WriteString( paste_str, DI_XPOS, DI_POSITION_LINE1 );

	sprintf( paste_str, "yaw  : %d", (short)AbsYaw / 256 );
	D_WriteString( paste_str, DI_XPOS, DI_POSITION_LINE2 );

	sprintf( paste_str, "roll : %d", (short)AbsRoll / 256 );
	D_WriteString( paste_str, DI_XPOS, DI_POSITION_LINE3 );

	sprintf( paste_str, "x-pos: %f", GEOMV_TO_FLOAT( MyShip->ObjPosition[ 0 ][ 3 ] ) );
	D_WriteString( paste_str, DI_XPOS, DI_POSITION_LINE4 );

	sprintf( paste_str, "y-pos: %f", GEOMV_TO_FLOAT( MyShip->ObjPosition[ 1 ][ 3 ] ) );
	D_WriteString( paste_str, DI_XPOS, DI_POSITION_LINE5 );

	sprintf( paste_str, "z-pos: %f", GEOMV_TO_FLOAT( MyShip->ObjPosition[ 2 ][ 3 ] ) );
	D_WriteString( paste_str, DI_XPOS, DI_POSITION_LINE6 );

	sprintf( paste_str, "txpos: %f", GEOMV_TO_FLOAT( ViewCamera[ 0 ][ 3 ] ) );
	D_WriteString( paste_str, DI_XPOS, DI_POSITION_LINE7 );

	sprintf( paste_str, "typos: %f", GEOMV_TO_FLOAT( ViewCamera[ 1 ][ 3 ] ) );
	D_WriteString( paste_str, DI_XPOS, DI_POSITION_LINE8 );

	sprintf( paste_str, "tzpos: %f", GEOMV_TO_FLOAT( ViewCamera[ 2 ][ 3 ] ) );
	D_WriteString( paste_str, DI_XPOS, DI_POSITION_LINE9 );
}


// draw permanent debugging info in hud ---------------------------------------
//
INLINE
void HUD_DrawDebugInfo()
{
	SetHudCharColor( 0 );

	if ( AUX_HUD_DEBUGINFO_PACKETS )
		HUD_DrawDebugInfoPackets();

	if ( AUX_HUD_DEBUGINFO_MEMORY )
		HUD_DrawDebugInfoMemory();

	if ( AUX_HUD_DEBUGINFO_OBJECTS )
		HUD_DrawDebugInfoObjects();

	if ( AUX_HUD_DEBUGINFO_GEOMETRY )
		HUD_DrawDebugInfoGeometry();

	if ( AUX_HUD_DEBUGINFO_RASTERIZATION )
		HUD_DrawDebugInfoRasterization();

	if ( AUX_HUD_DEBUGINFO_POSITION )
		HUD_DrawDebugInfoPosition();
}


// draw demo replay info in hud -----------------------------------------------
//
INLINE
void HUD_DrawDemoInfo()
{
	if ( DEMO_ReplayActive() ) {

		SetHudCharColor( 4 );

		int seconds = ( demoinfo_curtime / FRAME_MEASURE_TIMEBASE ) % 60;
		int minutes = demoinfo_curtime / ( FRAME_MEASURE_TIMEBASE * 60 );

		char secstr[ 3 ];
		DIG2_TO_STR( secstr, seconds );

		sprintf( paste_str, "time: %d:%s (%u) frame: %d line: %d ofs: %u",
				 minutes, secstr,
				 (unsigned int)demoinfo_curtime, demoinfo_curframe,
				 demoinfo_curline, (unsigned int)demoinfo_curofs );
		D_WriteString( paste_str, 0, 0 );
	}
}


// draw hud bitmaps (crosshair, radar) ----------------------------------------
//
PUBLIC
void HUD_DrawBitmaps()
{
	//NOTE:
	// also used by H_COCKPT::COCKPIT_DrawDisplay().

	// draw crosshair
	D_PutTrBitmap( BitmapInfo[ bm_crosshair ].bitmappointer,
				   BitmapInfo[ bm_crosshair ].width,
				   BitmapInfo[ bm_crosshair ].height,
				   hud_crosshair_x, hud_crosshair_y );

	// draw radar frame (no contents)
	HUD_DrawHUDRadarFrame();
}


// draw miscellaneous hud texts (tracking text) -------------------------------
//
PUBLIC
void HUD_DrawTrackingText( int targetvisible )
{
	if(headless_bot)
		return;
	//NOTE:
	// also used by H_COCKPT::COCKPIT_DrawDisplay().

	static int trackcount   = 0;
	static int newlylocked  = 1;
	static int newlytracked = 1;
	static int beepdone     = 0;

	int tracking_y = 0;
	int locked_y = 0;

	if ( AUX_DRAW_COCKPIT && AUX_DRAW_COCKPIT_CROSSHAIR ) {
		tracking_y = (int)(Screen_YOfs - ( cockpitinfo[ CROSS ].height * Scaled_Screen_Height * 0.4f )) ;
		tracking_y -= CharsetInfo[ HUD_CHARSETNO ].height;
		locked_y = (int)(Screen_YOfs + ( cockpitinfo[ CROSS ].height * Scaled_Screen_Height * 0.4f ));
	} else {
		tracking_y = hud_track_y;
		locked_y = hud_lockd_y;
	}

	int homingactive = ( SelectedMissile == 1 ) && ( MyShip->NumHomMissls > 0 );
	int swarmactive  = ( SelectedMissile == 3 ) && ( MyShip->NumPartMissls > 0 );

	if ( targetvisible && ( homingactive || swarmactive ) ) {

#define TRACK_COUNT_MAX 500

		SET_HUD_CHAR_CONTEXT();

		if ( TargetLocked ) {
			D_WriteString( hud_lockd_str, hud_lockd_x, locked_y );
			if ( newlylocked ) {
				newlylocked = 0;
				AUD_Locked();
			}
		} else {
			trackcount += CurScreenRefFrames;
			if ( trackcount > TRACK_COUNT_MAX ) trackcount = 0;
			if ( trackcount < TRACK_COUNT_MAX/2 ) {
				D_WriteString( hud_track_str, hud_track_x, tracking_y );
				if ( !beepdone ) {
					AUD_TrackingBeep();
					beepdone = 1;
				}
			} else {
				beepdone = 0;
			}
			if ( newlytracked ) {
				newlytracked = 0;
				AUD_Tracking();
			}
			newlylocked = 1;
		}

		SET_HUD_CHAR_CONTEXT();

	} else {

		newlytracked = 1;
		trackcount   = 0;
	}
}


// write string for ammo of missiles ------------------------------------------
//
PUBLIC
void HUD_WriteAmmoString( int ammo, int x, int y )
{
	//NOTE:
	// also used by H_COCKPT::COCKPIT_DrawWeapons().

	char ammo_str[4];
	DIG2_TO_STR( ammo_str, ammo );

	SET_HUD_CHAR_CONTEXT();
	SetHudCharColor( 4 );

	D_WriteString( ammo_str, x, y );
}


// draw packet loss graph -----------------------------------------------------
//
PRIVATE
void HUD_DrawPacketLossMeter()
{
	#define PACKET_GRAPH_COLOR1	102
	#define PACKET_GRAPH_COLOR2	85

	if ( !NetConnected || !NetJoined )
		return;

	visual_t xlatcol1 = COLINDX_TO_VISUAL( PACKET_GRAPH_COLOR1 );
	visual_t xlatcol2 = COLINDX_TO_VISUAL( PACKET_GRAPH_COLOR2 );

	char *packetgraph;
	int  graphsize;
	if ( NETs_DeterminePacketLoss( &packetgraph, &graphsize, TRUE ) ) {

		int	ratio = 0;
		for ( int i = 0; i < graphsize; i++ ) {

			if ( packetgraph[ i ] == 0 ) {
				D_DrawVertBar( DI_XPOS + (i << 1), 110 + 15, xlatcol1, 1 );
			} else {
				D_DrawVertBar( DI_XPOS + (i << 1), 110, xlatcol2, 16 );
				ratio++;
			}
		}

		//FIXME:
		// only works if ( graphsize == 100 )

		ASSERT( graphsize == 100 );

		SetHudCharColor( 0 );
		D_WriteString( "recv packet loss:", DI_XPOS, 100 );

		char ratio_str[ 5 ];
		DIG3_TO_STR( ratio_str, ratio );
		ratio_str[ 3 ] = '%';
		ratio_str[ 4 ] = 0;
		int dofs = 0;
		if ( ( ratio_str[ 0 ] == '0' ) && ( ratio_str[ 1 ] == '0' ) )
			dofs = 2;
		else if ( ratio_str[ 0 ] == '0' )
			dofs = 1;
		D_WriteString( ratio_str + dofs, DI_XPOS+17*8+4, 100 );
	}

	if ( NETs_DeterminePacketLoss( &packetgraph, &graphsize, FALSE ) ) {

		int	ratio = 0;
		for ( int i = 0; i < graphsize; i++ ) {

			if ( packetgraph[ i ] == 0 ) {
				D_DrawVertBar( DI_XPOS+220 + (i << 1), 110 + 15, xlatcol1, 1 );
			} else {
				D_DrawVertBar( DI_XPOS+220 + (i << 1), 110, xlatcol2, 16 );
				ratio++;
			}
		}

		//FIXME:
		// only works if ( graphsize == 100 )

		ASSERT( graphsize == 100 );

		SetHudCharColor( 0 );
		D_WriteString( "send packet loss:", DI_XPOS+220, 100 );

		char ratio_str[ 5 ];
		DIG3_TO_STR( ratio_str, ratio );
		ratio_str[ 3 ] = '%';
		ratio_str[ 4 ] = 0;
		int dofs = 0;
		if ( ( ratio_str[ 0 ] == '0' ) && ( ratio_str[ 1 ] == '0' ) )
			dofs = 2;
		else if ( ratio_str[ 0 ] == '0' )
			dofs = 1;
		D_WriteString( ratio_str + dofs, DI_XPOS+220+17*8+4, 100 );
	}
}


// counter for blinking speed of objcam text ----------------------------------
//
PUBLIC
int objcam_textcount = 0;	// reset by G_SUPP::ObjCamOn()


// display hud text in object camera view -------------------------------------
//
PRIVATE
void HUD_DrawObjCamDisplay()
{
	#define OBJCAM_TEXT_COUNT_MAX	900

	if ( AUX_FLAGWORD_SCREENSHOT_HELPERS & 0x10 ) {
		// allow text disabling for screenshots
		return;
	}

	if ( ( objcam_textcount += CurScreenRefFrames ) > OBJCAM_TEXT_COUNT_MAX ) {
		objcam_textcount = 0;
	}

	if ( objcam_textcount < OBJCAM_TEXT_COUNT_MAX/2 ) {
		D_WriteString( "object camera", obj_cam_x, obj_cam_y );
	}
}


// draw heads up display ------------------------------------------------------
//
void HUD_DrawHUD()
{
	SET_HUD_CHAR_CONTEXT();

	// never overlay any displays in floating menu
	if ( !InFloatingMenu ) {

		// check whether user displays should be drawn
		if ( !EntryMode && !AUX_HUD_DISABLE_USER_DISPLAYS ) {

			if ( ObjCameraActive ) {

				// object camera view
				HUD_DrawObjCamDisplay();

			} else {

				if ( AUX_DRAW_COCKPIT ) {

					// draw 'advanced' cockpit
					COCKPIT_DrawDisplay();

				}
			}
		}

		// normally debuginfo is disabled while console is visible
		if ( ( ConsoleSliding == 0 ) || AUX_HUD_DEBUGINFO_OVER_CONSOLE ) {

			// draw debug info
			if ( AUX_HUD_ENABLE_DEBUGINFO )
				HUD_DrawDebugInfo();

			// draw packet loss info
			if ( AUX_HUD_ENABLE_PACKET_LOSS_METER )
				HUD_DrawPacketLossMeter();
		}
	}
	
	// draw quick say (console line)
	COCKPIT_DrawQuickSayBuffer();

	// draw demo info if enabled
	if ( AUX_ENABLE_DEMO_REPLAY_INFO_DISPLAY ) {
		HUD_DrawDemoInfo();
	}
}



