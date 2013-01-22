/*
 * PARSEC - Camera Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:36 $
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

// mathematics header
#include "utl_math.h"

// local module header
#include "g_camera.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_main.h"


// flags
#define APPLY_SMOOTHING_TO_POSITION



// string constants -----------------------------------------------------------
//
static char sway_inval_period[]	= "period(2) invalid.";
static char sway_inval_ampl[]	= "amplitude(2) invalid.";
static char sway_inval_phase[]	= "phase invalid.";
static char sway_set_defaults[]	= "resetting to defaults.";


// init start position of camera ----------------------------------------------
//
void CAMERA_InitShipOrigin()
{
	// place camera at absolute origin
	MakeIdMatrx( ShipViewCamera );

	// rotate and translate camera ---------------------------
	// : matrix is "reversed":
	// : camerarotation right means matrixrotation left, etc.
	ShipViewCamera[ 0 ][ 3 ] += FLOAT_TO_GEOMV( 0.0 );
	ShipViewCamera[ 1 ][ 3 ] += FLOAT_TO_GEOMV( 0.0 );
	ShipViewCamera[ 2 ][ 3 ] += FLOAT_TO_GEOMV( 70.0/3.0 /*70.0/4.0*/ ); // higher means obj smaller
/*
	CamRotX( &ShipViewCamera, (bams_t) 0x0000 );
	CamRotY( &ShipViewCamera, (bams_t) 0x0000 );
	CamRotZ( &ShipViewCamera, (bams_t) 0x0000 );
*/
	// fill position matrix of local ship (MyShip)
	CalcOrthoInverse( ShipViewCamera, MyShip->ObjPosition );

	// make sure filter will work correctly
	CAMERA_ResetFilter();
}


// fill a matrix with the current fixed star camera ---------------------------
//
void CAMERA_MakeFixedStarCam( Camera dstcam )
{
	ASSERT( dstcam != NULL );

	memcpy( dstcam, ViewCamera, sizeof( Camera ) );

	dstcam[ 0 ][ 3 ] = GEOMV_0;
	dstcam[ 1 ][ 3 ] = GEOMV_0;
	dstcam[ 2 ][ 3 ] = GEOMV_0;
}


// get actual viewing (rendering) camera --------------------------------------
//
void CAMERA_GetViewCamera()
{
	// do transformations for object camera
	if ( ObjCameraActive ) {

		// create current view matrix for object camera
		MtxMtxMUL( ObjectCamera, ShipViewCamera, DestXmatrx );
		memcpy( ViewCamera, DestXmatrx, sizeof( Camera ) );

	} else {

		// use ship view camera directly
		memcpy( ViewCamera, ShipViewCamera, sizeof( Camera ) );
	}
}


// view camera storage --------------------------------------------------------
//
static Camera viewcam_prefilt;
static Camera viewcam_postfilt;
static int    viewcam_modify_in_progress = FALSE;


// reset/init view camera smoothing filter ------------------------------------
//
void CAMERA_ResetFilter()
{
	//NOTE:
	// this function will work both outside and during a modification
	// cycle. for both cases it ensures predictable filter behavior.

	// init view camera matrices
	memcpy( viewcam_prefilt, ShipViewCamera, sizeof( Camera ) );
	memcpy( viewcam_postfilt, ShipViewCamera, sizeof( Camera ) );
}


// begin view camera modification section -------------------------------------
//
void CAMERA_BeginModify()
{
	ASSERT( !viewcam_modify_in_progress );
	if ( viewcam_modify_in_progress )
		return;
	viewcam_modify_in_progress = TRUE;

	if ( !AUX_ENABLE_SMOOTH_SHIP_CONTROL ) {
		return;
	}

	// save filtered view camera
	memcpy( viewcam_postfilt, ShipViewCamera, sizeof( Camera ) );

	// restore non-filtered view camera
	memcpy( ShipViewCamera, viewcam_prefilt, sizeof( Camera ) );
}


// end view camera modification section and filter camera movement ------------
//
void CAMERA_EndModify()
{
	ASSERT( viewcam_modify_in_progress );
	if ( !viewcam_modify_in_progress )
		return;
	viewcam_modify_in_progress = FALSE;

	// save non-filtered view camera
	memcpy( viewcam_prefilt, ShipViewCamera, sizeof( Camera ) );

	if ( !AUX_ENABLE_SMOOTH_SHIP_CONTROL ) {
		return;
	}

	// determine interpolation alpha from filter constant
	if ( AUXDATA_SMOOTH_SHIP_CONTROL_FACTOR < 1 )
		AUXDATA_SMOOTH_SHIP_CONTROL_FACTOR = 100;
	float filtalpha = AUXDATA_SMOOTH_SHIP_CONTROL_FACTOR / 1000.0f;
	filtalpha *= CurScreenRefFrames * 0.06f;
	if ( filtalpha > 1.0f )
		filtalpha = 1.0f;

	// filtered orientation (filter output last frame)
	Quaternion srcquat;
	QuaternionFromMatrx( &srcquat, viewcam_postfilt );
	QuaternionMakeUnit( &srcquat );

	// non-filtered orientation (filter input this frame)
	Quaternion dstquat;
	QuaternionFromMatrx( &dstquat, ShipViewCamera );
	QuaternionMakeUnit( &dstquat );

	// do slerp from src to dst (filter output this frame)
	Quaternion slerpquat;
	QuaternionSlerp( &slerpquat, &srcquat, &dstquat, filtalpha );
	QuaternionMakeUnit( &slerpquat );

	// fill R part of view camera matrix (filtered orientation this frame)
	MatrxFromQuaternion( ShipViewCamera, &slerpquat );

	//NOTE:
	// the T part of the view camera matrix has to be
	// corrected for the new R part.
	// if R=R2*R1 and R is changed by the filter to
	// just R1, T has to be corrected to R2^-1*T to
	// account for the missing rotation of the translation.

	// determine R2^-1
	Quaternion hrotquat = slerpquat;
	QuaternionInvertUnit( &hrotquat );
	QuaternionMUL( &hrotquat, &dstquat, &hrotquat );
	QuaternionInvertUnit( &hrotquat );

	// convert R2^-1 to rotation matrix (T irrelevant)
	Xmatrx hrotmatrx;
	MatrxFromQuaternion( hrotmatrx, &hrotquat );

	// calculate R2^-1*T and remember it as new T
	Vector3 oldtvec, newtvec;
	FetchTVector( ShipViewCamera, &oldtvec );
	MtxVctMULt( hrotmatrx, &oldtvec, &newtvec );

	// invert post-filter rotation (filter output last frame)
	Quaternion srcquatinv = srcquat;
	QuaternionInvertUnit( &srcquatinv );

	// determine incremental rotation for pseudo stars
	Quaternion incrotquat;
	QuaternionMUL( &incrotquat, &slerpquat, &srcquatinv );
	QuaternionMakeUnit( &incrotquat );

	if ( !ObjCameraActive ) {

		// fill R part of pseudo star matrix
		MatrxFromQuaternion( PseudoStarMovement, &incrotquat );

	} else {

		Xmatrx objcaminv;
		CalcOrthoInverse( ObjectCamera, objcaminv );
		MtxMtxMUL( objcaminv, PseudoStarMovement, DestXmatrx );
		memcpy( PseudoStarMovement, DestXmatrx, sizeof( Camera ) );

		Xmatrx incrotmat;
		MatrxFromQuaternion( incrotmat, &incrotquat );
		incrotmat[ 0 ][ 3 ] = GEOMV_0;
		incrotmat[ 1 ][ 3 ] = GEOMV_0;
		incrotmat[ 2 ][ 3 ] = GEOMV_0;

		MtxMtxMUL( incrotmat, PseudoStarMovement, DestXmatrx );

#ifndef APPLY_SMOOTHING_TO_POSITION
		MtxMtxMUL( ObjectCamera, DestXmatrx, PseudoStarMovement );
#endif

	}

#ifdef APPLY_SMOOTHING_TO_POSITION

	// translation from last frame to this frame
	Quaternion xlatquat;
	QuaternionMUL( &xlatquat, &slerpquat, &srcquatinv );
	QuaternionMakeUnit( &xlatquat );

	Xmatrx xlatmatrx;
	MatrxFromQuaternion( xlatmatrx, &xlatquat );

	// translate last t vector to this frame
	Vector3 lasttvec, thistvec;
	FetchTVector( viewcam_postfilt, &lasttvec );
	MtxVctMULt( xlatmatrx, &lasttvec, &thistvec );

	// interpolation vector from last filtered to new filtered t
	float lerpx = GEOMV_TO_FLOAT( newtvec.X - thistvec.X ) * filtalpha;
	float lerpy = GEOMV_TO_FLOAT( newtvec.Y - thistvec.Y ) * filtalpha;
	float lerpz = GEOMV_TO_FLOAT( newtvec.Z - thistvec.Z ) * filtalpha;

	//MSGOUT( "CAMERA_EndModify(): LERP: %6.3f/%6.3f/%6.3f", lerpx, lerpy, lerpz );

	Vector3 deltatvec;
	deltatvec.X = FLOAT_TO_GEOMV( lerpx );
	deltatvec.Y = FLOAT_TO_GEOMV( lerpy );
	deltatvec.Z = FLOAT_TO_GEOMV( lerpz );

	// set new t for view camera
	newtvec.X = thistvec.X + deltatvec.X;
	newtvec.Y = thistvec.Y + deltatvec.Y;
	newtvec.Z = thistvec.Z + deltatvec.Z;

	if ( !ObjCameraActive ) {

		// update T of pseudo star matrix (incremental)
		StoreTVector( PseudoStarMovement, &deltatvec );

	} else {

		DestXmatrx[ 0 ][ 3 ] += deltatvec.X;
		DestXmatrx[ 1 ][ 3 ] += deltatvec.Y;
		DestXmatrx[ 2 ][ 3 ] += deltatvec.Z;

		MtxMtxMUL( ObjectCamera, DestXmatrx, PseudoStarMovement );
	}

#endif // APPLY_SMOOTHING_TO_POSITION

	// update T of view camera
	StoreTVector( ShipViewCamera, &newtvec );
}


// frame camera changes for pseudostars transformation ------------------------
//
int		pseudo_framecam_is_id = TRUE;
Xmatrx	pseudo_framecam;


// view camera before framecam changes are applied ----------------------------
//
static Camera prev_viewcam;


// full sine waves for cockpit swaying ----------------------------------------
//
static bams_t *cam_sway_tab_x = NULL;
static bams_t *cam_sway_tab_y = NULL;

static int cam_sway_len_x;
static int cam_sway_len_y;


// alter view camera for rendering the frame (frame camera) -------------------
//
void CAMERA_BeginFrameView()
{
	static refframe_t refframecount = 0;

	// make id implicitly
	pseudo_framecam_is_id = TRUE;

	if ( !AUX_ENABLE_COCKPIT_SWAYING ) {

		// avoid jumping after successive disabling/enabling
		refframecount = 0;

		if ( !AUX_ENABLE_COCKPIT_RATTLING ) {
			return;
		}
	}

	// save previous view camera
	memcpy( prev_viewcam, ViewCamera, sizeof( Camera ) );

	if ( InFloatingMenu || EntryMode || ObjCameraActive ) {

		// avoid jumping after successive mode switches
		refframecount = 0;

		return;
	}

	// pseudo must be incremental
	MakeIdMatrx( pseudo_framecam );

	if ( AUX_ENABLE_COCKPIT_SWAYING ) {

		// advance time only if no user interaction
		if ( ( CurYaw == 0 ) && ( CurPitch == 0 ) && ( CurRoll == 0 ) &&
			 ( CurSlideHorz == 0 ) && ( CurSlideVert == 0 ) ) {

			refframecount += CurScreenRefFrames;
		}

		ASSERT( cam_sway_tab_x != NULL );
		ASSERT( cam_sway_tab_y != NULL );

		// (yaw,pitch)/(screen_x,screen_y)-lissajous
		bams_t rotangle1 = cam_sway_tab_y[ refframecount % cam_sway_len_y ];
		bams_t rotangle2 = cam_sway_tab_x[ refframecount % cam_sway_len_x ];

		#define SWAY_SPEED_THRESHOLD	20000

		// no swaying above certain speed
		if ( MyShip->CurSpeed > SWAY_SPEED_THRESHOLD ) {

			// avoid jumping after resetting speed
			refframecount = 0;

		} else {

			if ( MyShip->CurSpeed > 0 ) {

				// linearly decrease swaying with increasing speed
				int invspeed = SWAY_SPEED_THRESHOLD - MyShip->CurSpeed;
				float sfac = (float)invspeed / SWAY_SPEED_THRESHOLD;

				rotangle1 = (bams_t)(rotangle1 * sfac);
				rotangle2 = (bams_t)(rotangle2 * sfac);
			}

			// pitch-sway
			CamRotX( ViewCamera, rotangle1 );
			CamRotX( pseudo_framecam, rotangle1 );

			if ( AUX_ENABLE_COCKPIT_SWAYING == 1 ) {

				// yaw-sway
				CamRotY( ViewCamera, rotangle2 );
				CamRotY( pseudo_framecam, rotangle2 );

			} else {

				// roll-sway
				CamRotZ( ViewCamera, rotangle2 );
				CamRotZ( pseudo_framecam, rotangle2 );
			}

			pseudo_framecam_is_id = FALSE;
		}
	}

	if ( AUX_ENABLE_COCKPIT_RATTLING && SetScreenBlue ) {

		bams_t rotangle_x = RAND() % 0x0100;
		bams_t rotangle_y = RAND() % 0x0100;
		bams_t rotangle_z = RAND() % 0x0100;

		CamRotX( ViewCamera, rotangle_x );
		CamRotX( pseudo_framecam, rotangle_x );

		CamRotY( ViewCamera, rotangle_y );
		CamRotY( pseudo_framecam, rotangle_y );

		CamRotZ( ViewCamera, rotangle_z );
		CamRotZ( pseudo_framecam, rotangle_z );

		pseudo_framecam_is_id = FALSE;
	}
}


// restore changes made for frame view camera ---------------------------------
//
void CAMERA_EndFrameView()
{
	if ( !AUX_ENABLE_COCKPIT_SWAYING && !AUX_ENABLE_COCKPIT_RATTLING )
		return;

	//NOTE:
	// pseudo stars matrix must not be restored here, since it
	// is incremental (and identity on call of this function).

	// restore previous view camera
	memcpy( ViewCamera, prev_viewcam, sizeof( Camera ) );
}


// cockpit swaying defaults ---------------------------------------------------
//
#define DEFAULT_TABLENGTH_X		900		// freqs x:y=1:2
#define DEFAULT_TABLENGTH_Y		450

#define DEFAULT_AMPLITUDE_X		85		// ampls x:y=2:1
#define DEFAULT_AMPLITUDE_Y		43

#define DEFAULT_PHASE			0		// in sync

#define MAX_SWAYTAB_LENGTH		4096
#define MAX_SWAY_AMPLITUDE		1000


// precalculate swaying tables for given parameters ---------------------------
//
PRIVATE
void InitSwayingTable( int tablena, int tablenb, int ampla, int amplb, int phase )
{
	ASSERT( ( tablena > 0 ) && ( tablena <= MAX_SWAYTAB_LENGTH ) );
	ASSERT( ( tablenb > 0 ) && ( tablenb <= MAX_SWAYTAB_LENGTH ) );
	ASSERT( ( ampla >= -MAX_SWAY_AMPLITUDE ) && ( ampla <= MAX_SWAY_AMPLITUDE ) );
	ASSERT( ( amplb >= -MAX_SWAY_AMPLITUDE ) && ( amplb <= MAX_SWAY_AMPLITUDE ) );

	if ( cam_sway_tab_x != NULL ) {
		FREEMEM( cam_sway_tab_x );
		cam_sway_tab_x = NULL;
		cam_sway_tab_y = NULL;
	}

	cam_sway_tab_x = (bams_t *) ALLOCMEM( ( tablena + tablenb ) * sizeof( bams_t ) );
	if ( cam_sway_tab_x == NULL )
		OUTOFMEM( 0 );
	cam_sway_tab_y = &cam_sway_tab_x[ tablena ];

	cam_sway_len_x = tablena;
	cam_sway_len_y = tablenb;

	// precalc full sine waves in time resolution
	// exactly as needed for cockpit swaying

	hprec_t angle = 0;
	hprec_t delta = HPREC_TWO_PI / tablena;
	int reft = 0;
	for ( reft = 0; reft < tablena; reft++ ) {
		cam_sway_tab_x[ reft ] = -(int)( ampla * sin( angle ) );
		angle += delta;
	}

	// ensure correct modulus for negative quotient
	// and produce inverted phase
	int phneg = ( phase < 0 );
	if ( phneg )
		phase = -phase;
	phase = phase % tablenb;
	if ( !phneg )
		phase = tablenb - phase;

	angle = 0;
	delta = HPREC_TWO_PI / tablenb;
	for ( reft = 0; reft < tablenb; reft++ ) {
		cam_sway_tab_y[ ( reft + phase ) % tablenb ] = -(int)( amplb * sin( angle ) );
		angle += delta;
	}
}


// key table for swaying command ----------------------------------------------
//
key_value_s swaying_key_value[] = {

	{ "period",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "amplitude",	NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "phase",		NULL,	KEYVALFLAG_NONE				},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_SWAYING_PERIOD,
	KEY_SWAYING_AMPLITUDE,
	KEY_SWAYING_PHASE,
};


// specify swaying by setting lissajous figure parameters ---------------------
//
PRIVATE
int Cmd_SWAYING( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// swaying_command	::= 'gfx.swaying' [<period_spec>] [<ampl_spec>] [<phase_spec>]
	// period_spec		::= '(' <int> <int> ')'
	// ampl_spec		::= '(' <int> <int> ')'
	// phase_spec		::= <int>

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( swaying_key_value, paramstr ) )
		return TRUE;

	// defaults
	int tablengths[] = { DEFAULT_TABLENGTH_X, DEFAULT_TABLENGTH_Y };
	int amplitudes[] = { DEFAULT_AMPLITUDE_X, DEFAULT_AMPLITUDE_Y };
	int phase        = DEFAULT_PHASE;
	int alldefaults  = TRUE;

	if ( swaying_key_value[ KEY_SWAYING_PERIOD ].value != NULL ) {
		if ( ScanKeyValueIntList( &swaying_key_value[ KEY_SWAYING_PERIOD ],
								  tablengths, 2, 2 ) == 0 ) {
			CON_AddLine( sway_inval_period );
			return TRUE;
		}
		alldefaults = FALSE;
	}

	if ( ( tablengths[ 0 ] < 1 ) || ( tablengths[ 0 ] > MAX_SWAYTAB_LENGTH ) ||
		 ( tablengths[ 1 ] < 1 ) || ( tablengths[ 1 ] > MAX_SWAYTAB_LENGTH ) ) {
		CON_AddLine( sway_inval_period );
		return TRUE;
	}

	if ( swaying_key_value[ KEY_SWAYING_AMPLITUDE ].value != NULL ) {
		if ( ScanKeyValueIntList( &swaying_key_value[ KEY_SWAYING_AMPLITUDE ],
								  amplitudes, 2, 2 ) == 0 ) {
			CON_AddLine( sway_inval_ampl );
			return TRUE;
		}
		alldefaults = FALSE;
	}

	if ( ( amplitudes[ 0 ] < -MAX_SWAY_AMPLITUDE ) || ( amplitudes[ 0 ] > MAX_SWAY_AMPLITUDE ) ||
	     ( amplitudes[ 1 ] < -MAX_SWAY_AMPLITUDE ) || ( amplitudes[ 1 ] > MAX_SWAY_AMPLITUDE ) ) {
		CON_AddLine( sway_inval_ampl );
		return TRUE;
	}

	if ( ScanKeyValueInt( &swaying_key_value[ KEY_SWAYING_PHASE ], &phase ) < 0 ) {
		CON_AddLine( sway_inval_phase );
		return TRUE;
	}

	if ( alldefaults && ( phase == DEFAULT_PHASE ) ) {
		CON_AddLine( sway_set_defaults );
	}

	// init swaying table
	InitSwayingTable( tablengths[ 0 ], tablengths[ 1 ],
					  amplitudes[ 0 ], amplitudes[ 1 ], phase );
	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( G_CAMERA )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "gfx.swaying" command
	regcom.command	 = "gfx.swaying";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_SWAYING;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// init default swaying table
	InitSwayingTable( DEFAULT_TABLENGTH_X, DEFAULT_TABLENGTH_Y,
					  DEFAULT_AMPLITUDE_X, DEFAULT_AMPLITUDE_Y, DEFAULT_PHASE );
}



