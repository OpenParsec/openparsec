/*
 * PARSEC - User Interface Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:38 $
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
#include "inp_defs.h"
#include "sys_defs.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "inp_user.h"

// proprietary module headers
#include "con_aux.h"
#include "e_demo.h"
#include "e_record.h"
#include "g_supp.h"
#include "h_supp.h"
#include "m_list.h"
#include "obj_ctrl.h"
#include "obj_game.h"
#include "g_wfx.h"
#include "g_emp.h"


// flags
#define TRACE_ABS_ANGLES



// object camera rotation and zooming speeds ----------------------------------
//
#define OBJCAM_YAW_PER_REFFRAME				10
#define OBJCAM_PITCH_PER_REFFRAME			10
#define OBJCAM_ROLL_PER_REFFRAME			25
#define OBJCAM_ZOOM_SPEED					FIXED_TO_GEOMV( 0x06000 )


// inverse of object camera ---------------------------------------------------
//
static Camera objcam_inverse;


// flag whether user action occurred in this frame ----------------------------
//
static int user_action_occurred = FALSE;


// to prevent multiple input methods from adding up ---------------------------
//
static dword curvisframe_user_rotx = VISFRAME_NEVER;
static dword curvisframe_user_roty = VISFRAME_NEVER;
static dword curvisframe_user_rotz = VISFRAME_NEVER;


// user rotates around x axis -------------------------------------------------
//
void INP_UserRotX( bams_t angle )
{
	//MSGOUT( "INP_UserRotX: %f degrees", BAMS_TO_DEG( angle ) );

	// only once per frame
	if ( curvisframe_user_rotx == CurVisibleFrame )
		return;
	curvisframe_user_rotx = CurVisibleFrame;

	CameraMoved			 = TRUE;
	user_action_occurred = TRUE;

	if ( !ObjCameraActive ) {

		CamRotX( ShipViewCamera, angle );
		CamRotX( PseudoStarMovement, angle );

#ifdef TRACE_ABS_ANGLES
		AbsPitch += angle &~0xf;
#endif
		angle = -angle / CurScreenRefFrames;
		CurPitch += angle;
		RecPitch += angle;

	} else {

		// object camera speeds need to be independent from ship steering
		angle = (bams_t)(angle * OBJCAM_PITCH_PER_REFFRAME / (float)MyShip->PitchPerRefFrame);

		CalcOrthoInverse( ObjectCamera, objcam_inverse );
		MtxMtxMUL( objcam_inverse, PseudoStarMovement, DestXmatrx );
		CamRotX( DestXmatrx, -angle * 4 );
		MtxMtxMUL( ObjectCamera, DestXmatrx, PseudoStarMovement );

		ObjRotX( ObjectCamera, -angle * 4 );

		RecPitch += -angle / CurScreenRefFrames;
	}
}


// user rotates around y axis -------------------------------------------------
//
void INP_UserRotY( bams_t angle )
{
	// only once per frame
	if ( curvisframe_user_roty == CurVisibleFrame )
		return;
	curvisframe_user_roty = CurVisibleFrame;

	CameraMoved			 = TRUE;
	user_action_occurred = TRUE;

	if ( !ObjCameraActive ) {

		CamRotY( ShipViewCamera, angle );
		CamRotY( PseudoStarMovement, angle );

#ifdef TRACE_ABS_ANGLES
		AbsYaw += angle &~0xf;
#endif
		angle = -angle / CurScreenRefFrames;
		CurYaw += angle;
		RecYaw += angle;

	} else {

		// object camera speeds need to be independent from ship steering
		angle = (bams_t)(angle * OBJCAM_YAW_PER_REFFRAME / (float)MyShip->YawPerRefFrame);

		CalcOrthoInverse( ObjectCamera, objcam_inverse );
		MtxMtxMUL( objcam_inverse, PseudoStarMovement, DestXmatrx );
		CamRotY( DestXmatrx, -angle * 4 );
		MtxMtxMUL( ObjectCamera, DestXmatrx, PseudoStarMovement );

		ObjRotY( ObjectCamera, -angle * 4 );

		RecYaw += -angle / CurScreenRefFrames;
	}
}


// user rotates around z axis -------------------------------------------------
//
void INP_UserRotZ( bams_t angle )
{
	// only once per frame
	if ( curvisframe_user_rotz == CurVisibleFrame )
		return;
	curvisframe_user_rotz = CurVisibleFrame;

	CameraMoved			 = TRUE;
	user_action_occurred = TRUE;

	if ( !ObjCameraActive ) {

		CamRotZ( ShipViewCamera, angle );
		CamRotZ( PseudoStarMovement, angle );

#ifdef TRACE_ABS_ANGLES
		AbsRoll += angle &~0xf;
#endif
		angle = -angle / CurScreenRefFrames;
		CurRoll += angle;
		RecRoll += angle;

	} else {

		// object camera speeds need to be independent from ship steering
		angle = (bams_t)(angle * OBJCAM_ROLL_PER_REFFRAME / (float)MyShip->RollPerRefFrame);

		CalcOrthoInverse( ObjectCamera, objcam_inverse );
		MtxMtxMUL( objcam_inverse, PseudoStarMovement, DestXmatrx );
		CamRotZ( DestXmatrx, -angle * 2 );
		MtxMtxMUL( ObjectCamera, DestXmatrx, PseudoStarMovement );

		ObjRotZ( ObjectCamera, -angle * 2 );

		RecRoll += -angle / CurScreenRefFrames;
	}
}


// user slides along x axis ---------------------------------------------------
//
void INP_UserSlideX( geomv_t slideval )
{
	user_action_occurred = TRUE;

	//FIXME: multiply slideval here by CurScreenRefFrames

	if ( !ObjCameraActive ) {

		ShipViewCamera[ 0 ][ 3 ]	 += slideval;
		PseudoStarMovement[ 0 ][ 3 ] += slideval;

		slideval = slideval / CurScreenRefFrames;
		CurSlideHorz -= slideval;
		RecSlideHorz -= slideval;
		CameraMoved = TRUE;

	} else {

		 if ( AUX_ENABLE_FREE_CAMERA ) {
			ObjectCamera[ 0 ][ 3 ]       += slideval;
			PseudoStarMovement[ 0 ][ 3 ] += slideval;
		 }
	}
}


// user slides along y axis ---------------------------------------------------
//
void INP_UserSlideY( geomv_t slideval )
{
	user_action_occurred = TRUE;

	//FIXME: multiply here by CurScreenRefFrames

	if ( !ObjCameraActive ) {

		ShipViewCamera[ 1 ][ 3 ]	 += slideval;
		PseudoStarMovement[ 1 ][ 3 ] += slideval;

		slideval = slideval / CurScreenRefFrames;
		CurSlideVert -= slideval;
		RecSlideVert -= slideval;
		CameraMoved = TRUE;

	} else {

		 if ( AUX_ENABLE_FREE_CAMERA ) {
			ObjectCamera[ 1 ][ 3 ]       += slideval;
			PseudoStarMovement[ 1 ][ 3 ] += slideval;
		 }
	}
}


// enforce maximum speed limit of ship ----------------------------------------
//
INLINE
void CheckLocalShipSpeedBounds()
{
	if ( MyShip->CurSpeed > MyShip->MaxSpeed ) {
		MyShip->CurSpeed = MyShip->MaxSpeed;
	} else if ( MyShip->CurSpeed < 0 ) {
		MyShip->CurSpeed = 0;
	}
}


// enforce minimum and maximum of object camera distance, respectively ---------
//
INLINE
void CheckObjectCameraDistance()
{
	// don't check in freecam mode
	if ( AUX_ENABLE_FREE_CAMERA )
		return;

	if ( ObjectCamera[ 2 ][ 3 ] < MyShip->ObjCamMinDistance ) {

		ObjectCamera[ 2 ][ 3 ] = MyShip->ObjCamMinDistance;

	} else if ( ObjectCamera[ 2 ][ 3 ] > MyShip->ObjCamMaxDistance ) {

		ObjectCamera[ 2 ][ 3 ] = MyShip->ObjCamMaxDistance;
	}
}


// user acceleration/deceleration and object camera movement, respectively ----
//
void INP_UserAcceleration( fixed_t c_speed )
{
	user_action_occurred = TRUE;

	if ( c_speed == 0 ) {
		return;
	}

	if ( !ObjCameraActive ) {

		// alter speed
		MyShip->CurSpeed += c_speed;
		CheckLocalShipSpeedBounds();

	} else {

		// move object camera closer / farther away

		geomv_t oldz = ObjectCamera[ 2 ][ 3 ];

		if ( c_speed > 0 ) {
			ObjectCamera[ 2 ][ 3 ] -= OBJCAM_ZOOM_SPEED * CurScreenRefFrames;
		} else {
			ObjectCamera[ 2 ][ 3 ] += OBJCAM_ZOOM_SPEED * CurScreenRefFrames;
		}
		CheckObjectCameraDistance();

		PseudoStarMovement[ 2 ][ 3 ] += ObjectCamera[ 2 ][ 3 ] - oldz;
	}
}


// cycle guns -----------------------------------------------------------------
//
void INP_UserCycleGuns()
{
	user_action_occurred = TRUE;

	// disallow gun cycling when one is currently active
	if ( MyShip->WeaponsActive != 0x0000 ) {
		return;
	}

	if ( ++SelectedLaser >= NUMBER_OF_SELECTABLE_GUNTYPES ) {
		SelectedLaser = 0;
	}

	// play sample for new selected gun
	AUD_SelectLaser( SelectedLaser );
}


// permutation table for missile cycling --------------------------------------
//
static int missile_cycle_perm[ NUMBER_OF_SELECTABLE_MISSILETYPES ] = {
	1, 3, 0, 2,
};


// cycle missiles -------------------------------------------------------------
//
void INP_UserCycleMissiles()
{
	user_action_occurred = TRUE;

	ASSERT( SelectedMissile < NUMBER_OF_SELECTABLE_MISSILETYPES );
	SelectedMissile = missile_cycle_perm[ SelectedMissile ];

	// play sample for new selected missile
	AUD_SelectMissile( SelectedMissile );
}


// cycle targets --------------------------------------------------------------
//
void INP_UserCycleTargets()
{
	user_action_occurred = TRUE;

	// set global TargetObjNumber to next target
	SelectNextTarget();
}


// select target in front -----------------------------------------------------
//
void INP_UserSelectFrontTarget()
{
	user_action_occurred = TRUE;
	
	// set global TargetObjNumber to front target
	SelectCrosshairTarget();
}


// set local ship speed to zero -----------------------------------------------
//
void INP_UserZeroSpeed()
{
	user_action_occurred = TRUE;

	MyShip->CurSpeed = 0;
}


// set local ship's speed to speed of current target --------------------------
//
void INP_UserTrackTargetSpeed()
{
	user_action_occurred = TRUE;

	GenObject *objscan = FetchFirstShip();

	for ( ; objscan; objscan = objscan->NextObj ) {

		if ( objscan->HostObjNumber == TargetObjNumber ) {

			// set local speed to target's speed
			MyShip->CurSpeed = ((ShipObject *)objscan)->CurSpeed;

			// check if ship is allowed to fly as fast as target
			CheckLocalShipSpeedBounds();

			break;
		}
	}
}


// activate afterburner -------------------------------------------------------
//
void INP_UserActivateAfterBurner()
{
	user_action_occurred = TRUE;

	OBJ_ActivateAfterBurner( MyShip );
}


// deactivate afterburner -----------------------------------------------------
//
void INP_UserDeactivateAfterBurner()
{
	OBJ_DeactivateAfterBurner( MyShip );
}


// cycle gun types ------------------------------------------------------------
//
INLINE
void User_CycleGuns()
{
	if ( DepressedKeys->key_NextWeapon ) {
		DepressedKeys->key_NextWeapon = 0;

		INP_UserCycleGuns();
	}
}


// cycle missile types --------------------------------------------------------
//
INLINE
void User_CycleMissiles()
{
	if ( DepressedKeys->key_NextMissile ) {
		DepressedKeys->key_NextMissile = 0;

		INP_UserCycleMissiles();
	}
}


// cycle target selection -----------------------------------------------------
//
INLINE
void User_CycleTargets()
{
	if ( DepressedKeys->key_NextTarget ) {
		DepressedKeys->key_NextTarget = 0;

		INP_UserCycleTargets();
	}
}


// select target in front  ----------------------------------------------------
//
INLINE
void User_SelectFrontTarget()
{
	if ( DepressedKeys->key_FrontTarget ) {
		DepressedKeys->key_FrontTarget = 0;
		
		INP_UserSelectFrontTarget();
	}
}


// set speed immediately to zero ----------------------------------------------
//
INLINE
void User_ZeroSpeed()
{
	if ( DepressedKeys->key_SpeedZero ) {
		DepressedKeys->key_SpeedZero = 0;

		INP_UserZeroSpeed();
	}
}


// set local ship's speed to speed of current target --------------------------
//
INLINE
void User_TrackTargetSpeed()
{
	if ( DepressedKeys->key_TargetSpeed ) {
		DepressedKeys->key_TargetSpeed = 0;

		INP_UserTrackTargetSpeed();
	}
}


// rotation of ship around x axis ---------------------------------------------
//
INLINE
void User_ShipPitch()
{
	if ( DepressedKeys->key_DiveDown ) {

		bams_t c_angle = MyShip->PitchPerRefFrame * CurScreenRefFrames;
		INP_UserRotX( c_angle );

	} else if ( DepressedKeys->key_PullUp ) {

		bams_t c_angle = MyShip->PitchPerRefFrame * CurScreenRefFrames;
		INP_UserRotX( -c_angle );
	}
}


// rotation of ship around y axis ---------------------------------------------
//
INLINE
void User_ShipYaw()
{
	if ( DepressedKeys->key_TurnLeft ) {

		bams_t c_angle = MyShip->YawPerRefFrame * CurScreenRefFrames;
		INP_UserRotY( c_angle );

	} else if ( DepressedKeys->key_TurnRight ) {

		bams_t c_angle = MyShip->YawPerRefFrame * CurScreenRefFrames;
		INP_UserRotY( -c_angle );
	}
}


// rotation of ship around z axis ---------------------------------------------
//
INLINE
void User_ShipRoll()
{
	if ( DepressedKeys->key_RollLeft ) {

		bams_t c_angle = MyShip->RollPerRefFrame * CurScreenRefFrames;
		INP_UserRotZ( c_angle );

	} else if ( DepressedKeys->key_RollRight ) {

		bams_t c_angle = MyShip->RollPerRefFrame * CurScreenRefFrames;
		INP_UserRotZ( -c_angle );
	}
}


// sliding in x direction -----------------------------------------------------
//
INLINE
void User_ShipSlideX()
{
	if ( DepressedKeys->key_SlideLeft ) {

		geomv_t slideval = MyShip->XSlidePerRefFrame * CurScreenRefFrames;
		INP_UserSlideX( slideval );

	} else if ( DepressedKeys->key_SlideRight ) {

		geomv_t slideval = MyShip->XSlidePerRefFrame * CurScreenRefFrames;
		INP_UserSlideX( -slideval );
	}
}


// sliding in y direction -----------------------------------------------------
//
INLINE
void User_ShipSlideY()
{
	if ( DepressedKeys->key_SlideUp ) {

		geomv_t slideval = MyShip->YSlidePerRefFrame * CurScreenRefFrames;
		INP_UserSlideY( slideval );

	} else if ( DepressedKeys->key_SlideDown ) {

		geomv_t slideval = MyShip->YSlidePerRefFrame * CurScreenRefFrames;
		INP_UserSlideY( -slideval );
	}
}


// speed control (acceleration/deceleration) ----------------------------------
//
INLINE
void User_SpeedControl()
{
	if ( DepressedKeys->key_Accelerate ) {

		int c_speed = MyShip->SpeedIncPerRefFrame * CurScreenRefFrames;
		INP_UserAcceleration( c_speed );

	} else if ( DepressedKeys->key_Decelerate ) {

		int c_speed = MyShip->SpeedDecPerRefFrame * CurScreenRefFrames;
		INP_UserAcceleration( -c_speed );
	}
}


// remember whether cannons are active ----------------------------------------
//
static int user_activated_helix		= FALSE;
static int user_activated_lightning	= FALSE;
static int user_activated_photon	= FALSE;
static int user_activated_emp		= FALSE;


// user fired laser -----------------------------------------------------------
//
INLINE
void User_FireLaser()
{
	if ( ( FireRepeat > 0 ) || ( FireDisable > 0 ) ) {
		return;
	}

	// create laser
	OBJ_ShootLaser( MyShip );

	if ( ( FireRepeat  += MyShip->FireRepeatDelay  ) <= 0 ) {
		FireRepeat = 1;
	}
	if ( ( FireDisable += MyShip->FireDisableDelay ) <= 0 ) {
		FireDisable = 1;
	}
}


// user fired helix cannon ----------------------------------------------------
//
INLINE
void User_FireHelix()
{
	if ( ( MyShip->WeaponsActive & WPMASK_CANNON_HELIX ) == 0 ) {
		user_activated_helix = WFX_ActivateHelix( MyShip );
	}
}


// user fired lightning device ------------------------------------------------
//
INLINE
void User_FireLightning()
{
	if ( ( MyShip->WeaponsActive & WPMASK_CANNON_LIGHTNING ) == 0 ) {
		user_activated_lightning = WFX_ActivateLightning( MyShip );
	}
}


// user fired photon cannon ---------------------------------------------------
//
INLINE
void User_FirePhoton()
{
	if ( ( MyShip->WeaponsActive & WPMASK_CANNON_PHOTON ) == 0 ) {
		user_activated_photon = WFX_ActivatePhoton( MyShip );
	}
}


// user fired emp device ------------------------------------------------------
//
INLINE
void User_FireEmp()
{

#ifdef EMP_FIRE_CONTINUOUSLY

	if ( ( MyShip->WeaponsActive & WPMASK_DEVICE_EMP ) == 0 ) {
		user_activated_emp = WFX_ActivateEmp( MyShip );
	}

#else // EMP_FIRE_CONTINUOUSLY

	if ( ( FireRepeat > 0 ) || ( FireDisable > 0 ) ) {
		return;
	}

	// create emp blast
	WFX_EmpBlast( MyShip );

	if ( ( FireRepeat  += MyShip->FireRepeatDelay  ) <= 0 ) {
		FireRepeat = 1;
	}
	if ( ( FireDisable += MyShip->FireDisableDelay ) <= 0 ) {
		FireDisable = 1;
	}

#endif // EMP_FIRE_CONTINUOUSLY

}


// check firing of guns -------------------------------------------------------
//
INLINE
void User_CheckGunFire()
{
	// shoot gun if activated
	if ( INPs_ActivateGun() ) {

		user_action_occurred = TRUE;

		if ( FireRepeat > 0 ) {
			FireRepeat -= CurScreenRefFrames;
		}

		// fire selected gun
		switch ( SelectedLaser ) {

			// laser
			case 0:
				User_FireLaser();
				break;

			// helix cannon
			case 1:
				User_FireHelix();
				break;

			// lightning device
			case 2:
				User_FireLightning();
				break;

			// photon cannon
			case 3:
				User_FirePhoton();
				break;

			// emp device
			case 4:
				User_FireEmp();
				break;

			default:
				ASSERT( 0 );
		}

	} else {

		// disable repeat delay in next frame
		FireRepeat = 1;

		//NOTE:
		// deactivation will only occur if a manual user activation
		// occured previously. this is needed to avoid turning
		// recorded activations off immediately after they have been
		// executed (due to no key being pressed).

		// delete lightning blast if key released and blast active
		if ( user_activated_lightning ) {
			if ( MyShip->WeaponsActive & WPMASK_CANNON_LIGHTNING ) {
				WFX_DeactivateLightning( MyShip );
			}
			user_activated_lightning = FALSE;
		}

		// deactivate if key released and helix active
		if ( user_activated_helix ) {
			if ( MyShip->WeaponsActive & WPMASK_CANNON_HELIX ) {
				WFX_DeactivateHelix( MyShip );
			}
			user_activated_helix = FALSE;
		}

		// deactivate if key released and photon active
		if ( user_activated_photon ) {
			if ( MyShip->WeaponsActive & WPMASK_CANNON_PHOTON ) {
				WFX_DeactivatePhoton( MyShip );
			}
			user_activated_photon = FALSE;
		}

		// deactivate if key released and emp active
		if ( user_activated_emp ) {
			if ( MyShip->WeaponsActive & WPMASK_DEVICE_EMP ) {
				WFX_DeactivateEmp( MyShip );
			}
			user_activated_emp = FALSE;
		}
	}
}


// check firing of missiles ---------------------------------------------------
//
INLINE
void User_CheckMissileLaunch()
{
	// launch missile if activated
	if ( INPs_ActivateMissile() ) {

		user_action_occurred = TRUE;

		if ( MissileDisable <= 0 ) {

			// fire selected missile
			switch ( SelectedMissile ) {

				// dumb missile
				case 0:
					OBJ_LaunchMissile( MyShip, CurLauncher );
					CurLauncher++;
					break;

				// homing missile
				case 1:
					OBJ_LaunchHomingMissile( MyShip, CurLauncher, TargetObjNumber );
					CurLauncher++;
					break;

				// mine
				case 2:
					OBJ_LaunchMine( MyShip );
					break;

				// swarm missiles
				case 3:
					OBJ_LaunchSwarmMissiles( MyShip, TargetObjNumber );
					break;

				default:
					ASSERT( 0 );
			}

			if ( ( MissileDisable += MyShip->MissileDisableDelay ) <= 0 ) {
				MissileDisable = 1;
			}
		}
	}
}

INLINE
void User_MaintainWeaponDelay()
{
    // count down missile fire disable delay
	if ( MissileDisable > 0 )
		MissileDisable -= CurScreenRefFrames;
    
    // count down fire disable delay
	if ( FireDisable > 0 ) {
		FireDisable -= CurScreenRefFrames;
	}
    
}

// display switching stuff ----------------------------------------------------
//
INLINE
void User_ViewKillStats()
{
	if ( GAME_MODE_ACTIVE() && AUX_HUD_ADVANCED_KILLSTATS ) {

		if ( DepressedKeys->key_ShowKillStats ) {
			SlideInListWindow();
		} else {
			SlideOutListWindow();
		}
	}
}


// check if afterburner should be activated -----------------------------------
//
INLINE
void User_AfterBurner()
{
	if ( DepressedKeys->key_AfterBurner ) {
		INP_UserActivateAfterBurner();
	} else {
		INP_UserDeactivateAfterBurner();
	}
}


// refframe count since last user action --------------------------------------
//
static refframe_t bored_base = REFFRAME_INVALID;


// reset activity checking ----------------------------------------------------
//
void INP_UserResetActivityChecking()
{
	bored_base = REFFRAME_INVALID;
}


// check for no user activity in too long a time ------------------------------
//
INLINE
void User_CheckActivity()
{
	#define BORED_WAIT_REFFRAMES_FIRST		( FRAME_MEASURE_TIMEBASE * 30 )
	#define BORED_WAIT_REFFRAMES_OTHER		( FRAME_MEASURE_TIMEBASE * 10 )

	ASSERT( BORED_WAIT_REFFRAMES_FIRST >= BORED_WAIT_REFFRAMES_OTHER );

	if ( user_action_occurred || DEMO_ReplayActive() ) {

		user_action_occurred = FALSE;
		bored_base			 = REFFRAME_INVALID;

	} else {

		refframe_t absref = SYSs_GetRefFrameCount();

		if ( bored_base == REFFRAME_INVALID ) {
			bored_base = absref;
		} else if ( ( absref - bored_base ) > BORED_WAIT_REFFRAMES_FIRST ) {
			bored_base = absref -
						 BORED_WAIT_REFFRAMES_FIRST +
						 BORED_WAIT_REFFRAMES_OTHER;
			AUD_JimCommenting( JIM_COMMENT_BORED );
		}
	}
}


// process user input: joystick position and keyboard input -------------------
//
void INP_UserProcessInput()
{
	// used during demo replay
	if ( UserInputDisabled ) {
		DEMO_UserInputDisabling();
	}

	REC_RecordKeys();

	User_ViewKillStats();

	User_CycleGuns();
	User_CycleMissiles();
	User_CycleTargets();
	
	User_SelectFrontTarget();

	if ( AUX_DISALLOW_USERCONTROLLED_MOTION )
		return;

	User_ShipYaw();
	User_ShipPitch();
	User_ShipRoll();
	User_ShipSlideX();
	User_ShipSlideY();

	User_SpeedControl();
	User_TrackTargetSpeed();
	User_ZeroSpeed();
	User_AfterBurner();

	// used during demo replay
	if ( UserInputDisabled ) {
		return;
	}

	// process additional input devices
	INPs_UserProcessAuxInput();
}


// query if gun fired and create objects accordingly --------------------------
//
void INP_UserCheckFiring()
{
	// used during demo replay
	if ( UserInputDisabled ) {
		return;
	}

	REC_RecordFiring();

	User_CheckGunFire();
	User_CheckMissileLaunch();
    User_MaintainWeaponDelay();
	User_CheckActivity();
}



