/*
 * PARSEC - Supporting Video Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:36 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   1999
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
#include "vid_defs.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "vid_supp.h"

// proprietary module headers
#include "e_callbk.h"
#include "con_arg.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_main.h"

// paste string area ----------------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// min/max fov ----------------------------------------------------------------
//
static const int min_fov = 1;
static const int max_fov = 180;


// manually set fov -----------------------------------------------------------
//
static int vid_set_fov = -1;
static int vid_setmode = -1;


//  calculate the viewing parameters ( viewdistance etc. ) --------------------
//
void VID_SetViewParameters( int mode )
{
	// set variables according to resolution
	
	Star_Siz = RObj_Siz = (int) 3.0f * ((float) Screen_Height / 720.0f);
	D_Value = Screen_Width;

	vid_setmode = mode;
}


// calculate viewing volume for current screen geometry -----------------------
/*
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
*/

// return the currently used FOV ----------------------------------------------
//
int VID_GetFOV()
{
	if ( AUX_ENABLE_FOV_CONTROL && vid_set_fov != -1 ) {

		return vid_set_fov;

	} else {
		double hypotenuse = hypot( Screen_XOfs, D_Value ); 

		if ( hypotenuse < 1e-7 ) 
			return -1;

		double fov_rad = asin( ( (double)Screen_XOfs ) / hypotenuse ) * 2;
		//double fov = fov_rad * 360.0 / ( 2 * 3.14159265359 );

		return (int) RAD_TO_DEG( fov_rad );
	}
}

// set the fov ----------------------------------------------------------------
//
int VID_SetFOV( int fov2set )
{
	if ( ( fov2set >= min_fov ) && ( fov2set <= max_fov ) ) {

		vid_set_fov = fov2set;

		double fov_half_rad  = DEG_TO_RAD( ( (double)fov2set ) / 2.0 );
		double sin_fov_half2 = sin( fov_half_rad );
		sin_fov_half2 *= sin_fov_half2;

		D_Value = (int) ( ( (double)Screen_XOfs ) * sqrt( ( 1 / sin_fov_half2 ) - 1 ) );	

		// set viewing volume planes
		VID_SetViewingVolume();
		
		// recalc projective matrix (needed in opengl)
		VIDs_CalcProjectiveMatrix();

		return TRUE;
	} else {

		return FALSE;
	}
}



// console command for debugging ----------------------------------------------
//
PRIVATE
int Cmd_FOV( char *paramstr )
{
	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN( paramstr );

	if ( ( *paramstr != ' ' ) && ( *paramstr != 0 ) ) {
		// there must be a space between command and argument
		CON_AddLine( unknown_command );
		return TRUE;
	}

	int32 fov2set = 0;

	if ( GetIntBehindCommand( paramstr, &fov2set, 10, -1 ) != -1 ) {

		if ( AUX_ENABLE_FOV_CONTROL ) {
			
			if ( VID_SetFOV( fov2set ) == FALSE ) {

				CON_AddLine( range_error );
			}

		} else {
			CON_AddLine( "custom fov disabled" );
		}

	} else {
		sprintf( paste_str, "%d", VID_GetFOV() );
		CON_AddLine( paste_str );
	}

	return TRUE;
}

// callback function after toggling AUX_DISABLE_FOV ---------------------------
//
void VID_RealizeFOV()
{
	if ( !AUX_ENABLE_FOV_CONTROL ) {

		// set the default viewparas according to the set videomode
		VID_SetViewParameters( vid_setmode );
		VID_SetViewingVolume();

	} else {

		// reapply fov setting, if custom fov
		if ( vid_set_fov != -1 ) {

			VID_SetFOV( vid_set_fov );
		}

	}
}

// draw interior of stargate --------------------------------------------------
//
PRIVATE
int _ReApplyFOV( void* param )
{
	// if we have a custom fov set and the auxflag is enabled, we reapply the fov
	if ( AUX_ENABLE_FOV_CONTROL && ( vid_set_fov != -1 ) ) {

		VID_SetFOV( vid_set_fov );
	}

	return TRUE;
}

// module registration function -----------------------------------------------
//
REGISTER_MODULE( VID_SUPP )
{
	user_command_s regcom;

	// register "fov" command
	regcom.command	 = "fov";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_FOV;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register callback for evt. fov setting after resolution changed
	CALLBACK_RegisterCallback( CBTYPE_VIDMODE_CHANGED | CBFLAG_PERSISTENT, _ReApplyFOV, NULL );
}

