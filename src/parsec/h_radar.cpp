/*
 * PARSEC - Radar Display Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:37 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-1999
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

// C library
#include <ctype.h>
#include <math.h>
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

// subsystem headers
#include "aud_defs.h"

// drawing subsystem
#include "d_bmap.h"
#include "d_misc.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "h_radar.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_main.h"
#include "e_color.h"
#include "e_supp.h"
#include "h_cockpt.h"
#include "obj_ctrl.h"
#include "obj_cust.h"



// epsilon for null vector ----------------------------------------------------
//
#define EPS_VECLEN				1e-7


// colors of radar objects ----------------------------------------------------
//
#define TARGET_RADAR_OBJ_COL 		255		// white
#define NORMAL_RADAR_OBJ_COL1 		80		// red
#define NORMAL_RADAR_OBJ_COL2 		87
#define NORMAL_RADAR_OBJ_COL3 		91
#define MISSILE_RADAR_OBJ_COL 		167		// yellow
#define STARGATE_RADAR_OBJ_COL 		225		// blue
#define PLANET_RADAR_OBJ_COL		102		// green
#define TELEPORTER_RADAR_OBJ_COL 	102		// green  // FIXME: use this for now

// geometry of radar ----------------------------------------------------------
//
int hud_radar_w;
int hud_radar_h;
int hud_radar_x;
int hud_radar_y;

int hud_radar_bitm_x;
int hud_radar_bitm_y;


// check vector length for zero -----------------------------------------------
//
INLINE
int LenZero( hprec_t len )
{
	ASSERT( len >= 0 );
	if ( len < EPS_VECLEN ) {
		MSGOUT( "radar: diminishing vector encountered." );
		return TRUE;
	}
	return FALSE;
}


// area of radar frame --------------------------------------------------------
//
static int area_x;
static int area_y;
static int area_width;
static int area_height;


// draw radar object after calculating its position ---------------------------
//
PRIVATE
void CalcDrawRadarObj( GenObject *objpo, int col )
{
	ASSERT( objpo != NULL );

	// fetch ship's position
	float ox = GEOMV_TO_FLOAT( objpo->CurrentXmatrx[ 0 ][ 3 ] );
	float oy = GEOMV_TO_FLOAT( objpo->CurrentXmatrx[ 1 ][ 3 ] );
	float oz = GEOMV_TO_FLOAT( objpo->CurrentXmatrx[ 2 ][ 3 ] );

	// square components
	hprec_t ox2 = ox * ox;
	hprec_t oy2 = oy * oy;
	hprec_t oz2 = oz * oz;

	// length of vector
	hprec_t len	= sqrt( ox2 + oy2 + oz2 );
	if ( LenZero( len ) )
		return;

	// length of projection onto (xz)-plane
	hprec_t lxz = sqrt( ox2 + oz2 );
	if ( LenZero( lxz ) )
		return;

	// yaw of projected vector (theta)
	hprec_t yaw = asin( ox / lxz );

	// bring range to [-PI..0] and [0..+PI]
	if ( oz < 0 )
		yaw = ( ox < 0 ) ? -HPREC_PI - yaw : HPREC_PI - yaw;

	// x axis is sin( yaw/2 ) weighted with cos( y-axis ):
	// combined left/right and front/back info in sine scale
	// [ formula: x = sin( theta/2 ) * cos( phi ) ]
	hprec_t rx	= sin( yaw / 2 ) * ( lxz / len );

	// y axis is sin( pitch ):
	// above/below info in sine scale
	// [ formula: y = sin( phi ) ]
	hprec_t ry	= oy / len;

	// draw actual radar object dot
	int radarx = (int)( rx * area_width );
	int radary = (int)( ry * area_height );
	D_DrawRadarObj( area_x + radarx , area_y + radary, COLINDX_TO_VISUAL( col ) );
}


// the range of the elite radar -----------------------------------------------
//
#define ELITE_RADAR_RANGE_MIN		4096.0
#define ELITE_RADAR_RANGE_MAX		65535.0

static int radar_range			= (int)ELITE_RADAR_RANGE_MIN;


// draw radar object after calculating its position ---------------------------
//
PRIVATE
void CalcDrawEliteRadarObj( GenObject *objpo, int col )
{
	ASSERT( objpo != NULL );

	// fetch ship's position
	float ox = GEOMV_TO_FLOAT( objpo->CurrentXmatrx[ 0 ][ 3 ] );
	float oy = GEOMV_TO_FLOAT( objpo->CurrentXmatrx[ 1 ][ 3 ] );
	float oz = GEOMV_TO_FLOAT( objpo->CurrentXmatrx[ 2 ][ 3 ] );

	int radar_width_2 	= 200 / 2;
	int radar_height_2 	= 32 / 2;
	int radar_centerx 	= Screen_XOfs + 2;
	int radar_centery 	= Screen_Height - 100 + 68;

	if ( AUX_DRAW_COCKPIT && AUX_DRAW_COCKPIT_RADAR ) {
		radar_centerx = (int)(Screen_Width - 0.4953f * Scaled_Screen_Width + 1);
		radar_centery = (int)(Screen_Height - 0.1187f * Scaled_Screen_Height);
		radar_width_2 	= (int)(( 130 / 2 ) * scale_tab[ cp_scale ]);
		radar_height_2 	= (int)(( 32 / 2 ) * scale_tab[ cp_scale ]);
	}

	// square components
	hprec_t ox2 = ox * ox;
	hprec_t oy2 = oy * oy;
	hprec_t oz2 = oz * oz;

	// length of vector
	hprec_t len	= sqrt( ox2 + oy2 + oz2 );
	if ( LenZero( len ) )
		return;

	// don't draw objects that are too far away
	if ( len > radar_range )
		return;

	hprec_t rx = ( ox / radar_range ) * radar_width_2;
	hprec_t ry = ( -oz / radar_range ) * radar_height_2;
	hprec_t rz = ( -oy / radar_range ) * ( Screen_Height - radar_centery - 3 );

	int radar_x = (int)(radar_centerx + rx)
;
	int radar_y = (int)(radar_centery + ry);

	int height = (int)rz;

	// draw actual radar object
	visual_t xlatcol = COLINDX_TO_VISUAL( col );
	if ( height > 0 ) {

		D_DrawVertBar( radar_x, radar_y - height, xlatcol, height + 1 );
		D_DrawHorzBar( radar_x, radar_y - height, xlatcol, 2 );

	} else if ( height < 0 ) {

		D_DrawVertBar( radar_x, radar_y, xlatcol, -height + 1 );
		D_DrawHorzBar( radar_x, radar_y - height, xlatcol, 2 );

	} else {

		D_DrawHorzBar( radar_x, radar_y, xlatcol, 2 );
	}
}


// scan ship objects for radar ------------------------------------------------
//
INLINE
void RadarScanShipObjects()
{
	GenObject *targetobj = NULL;

	for ( GenObject *walkobjs = FetchFirstShip(); walkobjs; ) {

		// set color for normal objects
		int rocolor = NORMAL_RADAR_OBJ_COL2;

		if ( AUX_ENABLE_DEPTHCUED_RADAR_DOTS && !AUX_ENABLE_ELITE_RADAR ) {

			Vertex3 shippos;
			FetchTVector( walkobjs->ObjPosition, &shippos );

			Vertex3 myshippos;
			FetchTVector( MyShip->ObjPosition, &myshippos );

			Vector3 dist;
			VECSUB( &dist, &shippos, &myshippos );

			geomv_t vx = dist.X;
			geomv_t vy = dist.Y;
			geomv_t vz = dist.Z;

			ABS_GEOMV( vx );
			ABS_GEOMV( vy );
			ABS_GEOMV( vz );

			int distance = 0;
			if ( ( vx > GEOMV_VANISHING ) ||
		 		 ( vy > GEOMV_VANISHING ) ||
		 		 ( vz > GEOMV_VANISHING ) ) {
				distance = GEOMV_TO_INT( VctLenX( &dist ) );
			}

			if ( distance > 4096 ) {
				rocolor = NORMAL_RADAR_OBJ_COL3;
			} else if ( distance > 2048 ) {
				rocolor = NORMAL_RADAR_OBJ_COL2;
			} else {
				rocolor = NORMAL_RADAR_OBJ_COL1;
			}
		}

		// ensure that the target object is drawn last
		if ( walkobjs->HostObjNumber == TargetObjNumber ) {

			if ( targetobj == NULL ) {
				targetobj = walkobjs;
				
				if ( ( walkobjs = walkobjs->NextObj ) == NULL ) {
					walkobjs = targetobj;
				}
				
				continue;
			}

			rocolor = TARGET_RADAR_OBJ_COL;
		}

		// depict ship's position on radar
		if ( AUX_ENABLE_ELITE_RADAR ) {
			CalcDrawEliteRadarObj( walkobjs, rocolor );
		} else {
			CalcDrawRadarObj( walkobjs, rocolor );
		}

		if ( walkobjs == targetobj ) {
			break;
		}

		if ( ( walkobjs = walkobjs->NextObj ) == NULL ) {
			walkobjs = targetobj;
		}
	}
}


// scan missile objects for radar ---------------------------------------------
//
INLINE
void RadarScanMissileObjects()
{
	GenObject *walkobjs = FetchFirstMissile();
	for ( ; walkobjs; walkobjs = walkobjs->NextObj ) {

		// set color for missile objects
		int rocolor = MISSILE_RADAR_OBJ_COL;

		// depict missile's position on radar
		if ( AUX_ENABLE_ELITE_RADAR ) {
			CalcDrawEliteRadarObj( walkobjs, rocolor );
		} else {
			CalcDrawRadarObj( walkobjs, rocolor );
		}
	}
}


// scan stargate objects for radar --------------------------------------------
//
INLINE
void RadarScanStargateObjects()
{
	GenObject *walkobjs = FetchFirstCustom();
	for ( ; walkobjs; walkobjs = walkobjs->NextObj ) {

		// get type id of the custom stargate
		static dword stargate_typeid = TYPE_ID_INVALID;
		if ( stargate_typeid == TYPE_ID_INVALID ) {
			stargate_typeid = OBJ_FetchCustomTypeId( "stargate" );
		}

		// we only want to walk stargates
		if ( walkobjs->ObjectType != stargate_typeid ) {
			continue;
		}

		// set color for stargate objects
		int rocolor = STARGATE_RADAR_OBJ_COL;

		// depict stargate's position on radar
		if ( AUX_ENABLE_ELITE_RADAR ) {
			CalcDrawEliteRadarObj( walkobjs, rocolor );
		} else {
			CalcDrawRadarObj( walkobjs, rocolor );
		}
	}
}

// scan teleporter objects for radar ------------------------------------------
//
INLINE
void RadarScanTeleporterObjects()
{
	GenObject *walkobjs = FetchFirstCustom();
	for ( ; walkobjs; walkobjs = walkobjs->NextObj ) {
		
		// get type id of the custom stargate
		static dword teleporter_typeid = TYPE_ID_INVALID;
		if ( teleporter_typeid == TYPE_ID_INVALID ) {
			teleporter_typeid = OBJ_FetchCustomTypeId( "teleporter" );
		}
		
		// we only want to walk stargates
		if ( walkobjs->ObjectType != teleporter_typeid ) {
			continue;
		}
		
		// set color for stargate objects
		int rocolor = TELEPORTER_RADAR_OBJ_COL;
		
		// depict stargate's position on radar
		if ( AUX_ENABLE_ELITE_RADAR ) {
			CalcDrawEliteRadarObj( walkobjs, rocolor );
		} else {
			CalcDrawRadarObj( walkobjs, rocolor );
		}
	}
}


// scan planet objects for radar -----------------------------------------------
//
INLINE
void RadarScanPlanetObjects()
{
	GenObject *walkobjs = FetchFirstCustom();
	for ( ; walkobjs; walkobjs = walkobjs->NextObj ) {

		// get type id of the custom planet
		static dword planet_typeid = TYPE_ID_INVALID;
		if ( planet_typeid == TYPE_ID_INVALID ) {
			planet_typeid = OBJ_FetchCustomTypeId( "planet" );
		}

		// we only want to walk planets
		if ( walkobjs->ObjectType != planet_typeid ) {
			continue;
		}

		// set color for planet objects
		int rocolor = PLANET_RADAR_OBJ_COL;

		// depict planet's position on radar
		if ( AUX_ENABLE_ELITE_RADAR ) {
			CalcDrawEliteRadarObj( walkobjs, rocolor );
		} else {
			CalcDrawRadarObj( walkobjs, rocolor );
		}
	}
}


// draw radar display (contents): dots denoting spacecrafts -------------------
//
void HUD_DrawHUDRadar()
{
	if ( !AUX_ENABLE_ELITE_RADAR ) {

		// make sure dots are positioned in/scaled to frame
		if ( AUX_DRAW_COCKPIT && AUX_DRAW_COCKPIT_RADAR ) {

			float center_x 	= 322.0f / 640.0f;	// offset from left edge of screen
			float center_y	= 65.0f / 480.0f;	// offset from top of texture
			float	width		= 43.0f / 640.0f;

			area_x 		= (int)(center_x * Screen_Width);
			area_y 		= (int)(Screen_Height - ( cockpitinfo[ RADAR ].ypos - center_y ) * Scaled_Screen_Height);
			area_width	= (int)(width * Scaled_Screen_Width);
			area_height = area_width;

		} else {

			area_x		= hud_radar_x;
			area_y		= hud_radar_y - BitmapInfo[ BM_RADAR ].height/2;
			area_width	= BitmapInfo[ BM_RADAR ].width/2  - hud_radar_w;
			area_height	= BitmapInfo[ BM_RADAR ].height/2 - hud_radar_h;
		}
	}

	// ship dots
	RadarScanShipObjects();

	// missile dots
	if ( AUX_DRAW_MISSILES_ON_RADAR ) {
		RadarScanMissileObjects();
	}

	// stargate dots
	if ( AUX_DRAW_STARGATES_ON_RADAR ) {
		RadarScanStargateObjects();
	}

	// planet dots
	if ( AUX_DRAW_PLANETS_ON_RADAR ) {
		RadarScanPlanetObjects();
	}

	// telepoter dots
	if ( AUX_DRAW_TELEPORTERS_ON_RADAR ) {
		RadarScanTeleporterObjects();
	}
	
}


// draw radar frame (bitmap, no contents) -------------------------------------
//
void HUD_DrawHUDRadarFrame()
{
	// no bitmap frame if new radar enabled
	if ( AUX_DRAW_COCKPIT && AUX_DRAW_COCKPIT_RADAR )
		return;

	// standard radar is default
	int radarid = BM_RADAR;

	// check for alternate radar
	if ( AUX_ENABLE_ELITE_RADAR ) {

		// elite-style radar
		static int eliteid = -1;
		if ( eliteid == -1 ) {
			eliteid = FetchBitmapId( BM_NAME_RADAR2 );
			if ( eliteid == -1 ) {
				return;
			}
		}

		radarid = eliteid;
	}

	int drawx = hud_radar_bitm_x - BitmapInfo[ radarid ].width/2;
	int drawy = hud_radar_bitm_y - BitmapInfo[ radarid ].height;

	// draw radar circle
	D_PutTrBitmap( BitmapInfo[ radarid ].bitmappointer,
				   BitmapInfo[ radarid ].width,
				   BitmapInfo[ radarid ].height,
				   drawx, drawy );
}


// change range of elite radar ------------------------------------------------
//
PRIVATE
int Cmd_RADAR_RANGE( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// range_command ::= 'radar.range' [<range>]

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	int range = radar_range;
	char *scan;

	if ( (scan = QueryIntArgumentEx( paramstr, "%d", &range )) ) {

		// determine if delta modification (++/--)
		int delta = 0;
		if ( ( scan[ 0 ] == '+' ) && ( scan[ 1 ] == '+' ) )
			delta = 1;
		else if ( ( scan[ 0 ] == '-' ) && ( scan[ 1 ] == '-' )  )
			delta = -1;
		if ( delta != 0 )
			scan += 2;

		if ( *scan != 0 ) {
			char *errpart;

			long sval = strtol( scan, &errpart, 10 );
			if ( *errpart == 0 ) {
				if ( delta != 0 )
					sval = range + sval * delta;
				if ( sval >= ELITE_RADAR_RANGE_MIN && sval <= ELITE_RADAR_RANGE_MAX ) {
					radar_range = sval;
				} else {

					//NOTE:
					// if delta modification was used, don't print a range_error
					// but just beep to give the user some feedback...

					if ( delta != 0 )
						AUD_Select2();
					else
						CON_AddLine( range_error );
				}
			} else {
				CON_AddLine( invalid_arg );
			}
		} else {
			CON_AddLine( invalid_arg );
		}
	}

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( H_RADAR )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "radar.range" command
	regcom.command	 = "radar.range";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_RADAR_RANGE;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );
}



