/*
 * PARSEC - Support Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:39 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   2000
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

// compilation flags/debug support
#include "config.h"


// C library
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"


// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "inp_defs.h"

// local module header
#include "isdl_supp.h"

// proprietary headers
#include "con_int.h"
#include "inp_user.h"


// flags
//#define NEW_JOYBUTTON_CODE // Wtf is this crap?

extern int                  isdl_FireGun;
extern int                  isdl_FireMissile;
extern int                  isdl_Accelerate;
extern int                  isdl_Deccelerate;
extern int                  isdl_Rollleft;
extern int                  isdl_RollRight;
extern int                  isdl_NextGun;
extern int                  isdl_NextMissile;
extern int					isdl_Aburn;
extern int					isdl_Emp;
extern int					isdl_StraffeLeft;
extern int					isdl_StraffeRight;
extern int					isdl_StraffeUp;
extern int					isdl_StraffeDown;
extern int					isdl_Stop;
extern int					isdl_Dup;
extern int					isdl_Ddown;
extern int					isdl_Dleft;
extern int					isdl_Dright;
extern int					isdl_Shift;
extern int					isdl_Target;
extern int					isdl_TargetFront;

// joystick globals -----------------------------------------------------------
//
//int			isdl_bDisableJoystick = TRUE;	// joystick disabled
joystate_s	JoyState;						// generic joystick data
int			isdl_bHasThrottle = 0;			// indicates, the joystick has a throttle
int			isdl_bHasRudder   = 0;			// indicates, the joystick has a rudder


// shoot gun if activated -----------------------------------------------------
//
int ISDL_ActivateGun()
{
	// keyboard
	if ( DepressedKeys->key_ShootWeapon ) {
		return TRUE;
	}

	// joystick
	if ( QueryJoystick && Op_Joystick ) {

		if ( JoyState.Buttons[ isdl_FireGun ] ) {
			return TRUE;
		}

	}
	// mouse
	if ( Op_Mouse ) {

		mousestate_s mousestate;
		int mouseavailable = INPs_MouseGetState( &mousestate );
		if ( mouseavailable ) {
			return ( mousestate.buttons[ MOUSE_BUTTON_LEFT ] == MOUSE_BUTTON_PRESSED );
		}
	}

	return FALSE;
}


// launch missile if activated ------------------------------------------------
//
int ISDL_ActivateMissile()
{
	// keyboard
	if ( DepressedKeys->key_LaunchMissile ) {
		return TRUE;
	}

	// joystick
	if ( QueryJoystick && Op_Joystick ) {

		if ( JoyState.Buttons[ isdl_FireMissile	] ) {
			return TRUE;
		}

	}
	// mouse
	if ( Op_Mouse ) {

		mousestate_s mousestate;
		int mouseavailable = INPs_MouseGetState( &mousestate );
		if ( mouseavailable ) {
			return ( mousestate.buttons[ MOUSE_BUTTON_RIGHT ] == MOUSE_BUTTON_PRESSED );
		}
	}

	return FALSE;
}


// ----------------------------------------------------------------------------
//
PRIVATE fixed_t ISDLm_target_speed = 0;
PRIVATE fixed_t ISDLm_OldCurSpeed = 0;
PRIVATE fixed_t ISDLm_oldSetSpeed = -1;


// ----------------------------------------------------------------------------
//
void ISDLm_SetTargetSpeed(fixed_t _targetSpeed)
{
	// constrain to the min/max speed of a ship
	if ( _targetSpeed > MyShip->MaxSpeed )
		_targetSpeed = MyShip->MaxSpeed;
	if ( _targetSpeed < 0 )
		_targetSpeed = 0;

	ISDLm_target_speed = _targetSpeed;
	ISDLm_OldCurSpeed  = MyShip->CurSpeed;
}


// ----------------------------------------------------------------------------
//
void ISDLm_DoAcceleration()
{
	if ( ISDLm_OldCurSpeed == MyShip->CurSpeed ) {
		fixed_t speedDiff = ISDLm_target_speed - MyShip->CurSpeed;

		if ( speedDiff > 0 ) {
			INP_UserAcceleration( MyShip->SpeedIncPerRefFrame * CurScreenRefFrames );
			if ( ISDLm_target_speed < MyShip->CurSpeed )
				ISDLm_target_speed = MyShip->CurSpeed;
		} else if ( speedDiff < 0 ) {
			INP_UserAcceleration( -MyShip->SpeedDecPerRefFrame * CurScreenRefFrames );
			if ( ISDLm_target_speed > MyShip->CurSpeed )
				ISDLm_target_speed = MyShip->CurSpeed;
		}

		ISDLm_OldCurSpeed = MyShip->CurSpeed;
	}
}


// process joystick input for spacecraft motion -------------------------------
//
PRIVATE
void ISDLm_ProcessMotionJoystick()
{
	// may be disabled
	if ( !QueryJoystick || !Op_Joystick ) {
		return;
	}

	bams_t	c_angle;
	int	c_speed;

	if ( ( ( JoyState.Buttons[ isdl_Shift ] & 0x80 ) == 0 ) || ( isdl_bHasRudder && isdl_bHasThrottle && !ObjCameraActive ) ) {

		// shift button not pressed + up/down -> pitch
		c_angle  = (bams_t) -JoyState.Y;
		c_angle *= MyShip->PitchPerRefFrame * CurScreenRefFrames;
		c_angle = (bams_t)(c_angle * joy_pitch_corr_refframe);

		if ( c_angle != 0 ) {
			INP_UserRotX( c_angle );
		}

		// shift not pressed + left/right -> yaw
		c_angle  = (bams_t) -JoyState.X;
		c_angle *= MyShip->YawPerRefFrame * CurScreenRefFrames;
		c_angle = (bams_t)(c_angle * joy_yaw_corr_refframe);

		if ( c_angle != 0 ) {
			INP_UserRotY( c_angle );
		}

		if ( isdl_bHasRudder ) {

			// rudder
			c_angle  = (bams_t) -JoyState.Rz;
			c_angle *= MyShip->RollPerRefFrame * CurScreenRefFrames;
			c_angle = (bams_t)(c_angle * joy_roll_corr_refframe);

			if ( c_angle != 0 ) {
				INP_UserRotZ( c_angle );
			}
		}


		if ( isdl_bHasThrottle ) {

			// throttle only when in flight mode
			if ( !ObjCameraActive ) {
				//static fixed_t ILm_oldSetSpeed = -1;

				fixed_t SetSpeed = (JOY_THROTTLE_MAX - JoyState.Z);
				SetSpeed *= (MyShip->MaxSpeed>>6);
				SetSpeed = (SetSpeed / JOY_THROTTLE_MAX);
				SetSpeed &= JOY_THROTTLE_DEADZONE_MASK;
				SetSpeed = SetSpeed << 6;

				if ( ISDLm_oldSetSpeed == -1 ) {
					ISDLm_oldSetSpeed = SetSpeed;
				} else if ( ISDLm_oldSetSpeed != SetSpeed ) {
					ISDLm_SetTargetSpeed( SetSpeed );
					ISDLm_oldSetSpeed = SetSpeed;
				}
				ISDLm_DoAcceleration();
			}
		}

	} else {

		// shift button + left/right -> roll
		c_angle  = (bams_t) -JoyState.X;
		c_angle *= MyShip->RollPerRefFrame * CurScreenRefFrames;
		c_angle = (bams_t)(c_angle * joy_roll_corr_refframe);

		if ( c_angle != 0 ) {
			INP_UserRotZ( c_angle );
		}

		// shift button + up/down -> accelerate/decelerate
		c_speed  = (bams_t) -JoyState.Y;
		c_speed *= MyShip->SpeedIncPerRefFrame * CurScreenRefFrames;
		c_speed = (int)(c_speed * joy_acc_corr_refframe);

		INP_UserAcceleration( c_speed );
	}
}

PRIVATE
void ISDLm_ProcessJoystickBinds() {
	if(JoyState.Buttons[isdl_NextGun]) {
		INP_UserCycleGuns(); //Tired right now, fix this tommorow cause they hilariously cycle through at impossible speeds
	}
	if(JoyState.Buttons[isdl_NextMissile]) {
		INP_UserCycleMissiles();
	}
	if(JoyState.Buttons[ isdl_RollRight]) {
		bams_t c_angle = MyShip->RollPerRefFrame * CurScreenRefFrames;
		INP_UserRotZ( -c_angle );
	}
	if(JoyState.Buttons[ isdl_Rollleft	]) {
		bams_t c_angle = MyShip->RollPerRefFrame * CurScreenRefFrames;
		INP_UserRotZ( c_angle );
	}
	if(JoyState.Buttons[isdl_Accelerate ]) {
		int c_speed = MyShip->SpeedIncPerRefFrame * CurScreenRefFrames;
		INP_UserAcceleration( c_speed );
	}
	if(JoyState.Buttons[isdl_Deccelerate]) {
		int c_speed = MyShip->SpeedDecPerRefFrame * CurScreenRefFrames;
		INP_UserAcceleration( -c_speed );
	}
	if(JoyState.Buttons[isdl_Stop]) {
		INP_UserZeroSpeed();
	}
	if(JoyState.Buttons[isdl_StraffeLeft]) {
		geomv_t slideval = MyShip->XSlidePerRefFrame * CurScreenRefFrames;
		INP_UserSlideX( slideval );
	}
	if(JoyState.Buttons[isdl_StraffeRight]) {
		geomv_t slideval = MyShip->XSlidePerRefFrame * CurScreenRefFrames;
		INP_UserSlideX( -slideval );
	}
	if(JoyState.Buttons[isdl_StraffeUp]) {
		geomv_t slideval = MyShip->YSlidePerRefFrame * CurScreenRefFrames;
		INP_UserSlideY( slideval );
	}
	if(JoyState.Buttons[isdl_StraffeDown]) {
		geomv_t slideval = MyShip->YSlidePerRefFrame * CurScreenRefFrames;
		INP_UserSlideY( -slideval );
	}
	if(JoyState.Buttons[isdl_Aburn]) {
		INP_UserActivateAfterBurner();
	} else {
		INP_UserDeactivateAfterBurner();
	}
	if(JoyState.Buttons[isdl_Emp]) {
		INP_UserFiredEMP();
	}
	if(JoyState.Buttons[isdl_Target]) {
		INP_UserCycleTargets();	
	}
	if(JoyState.Buttons[isdl_TargetFront]) {
		INP_UserSelectFrontTarget();
	}
	if(JoyState.Buttons[isdl_Dup]) {
		bams_t c_angle = MyShip->PitchPerRefFrame * CurScreenRefFrames;
		INP_UserRotX( -c_angle );
	}
	if(JoyState.Buttons[isdl_Ddown]) {
		bams_t c_angle = MyShip->PitchPerRefFrame * CurScreenRefFrames;
		INP_UserRotX( c_angle );
	}
	if(JoyState.Buttons[isdl_Dright]) {
		bams_t c_angle = MyShip->YawPerRefFrame * CurScreenRefFrames;
		INP_UserRotY( -c_angle );
	}
	if(JoyState.Buttons[isdl_Dleft]) {
		bams_t c_angle = MyShip->YawPerRefFrame * CurScreenRefFrames;
		INP_UserRotY( c_angle );
	}
}
#define MOUSE_EDGE_EPS  0.1f

// process mouse input for spacecraft motion ----------------------------------
//
PRIVATE
void ISDLm_ProcessMotionMouse()
{
	// may be disabled
	if ( !Op_Mouse ) {
		return;
	}

	mousestate_s mousestate;
	int mouseavailable = INPs_MouseGetState( &mousestate );

	// make sure mouse is there
	if ( !mouseavailable ) {
		return;
	}

	static float last_xpos = 0.5f;
	static float last_ypos = 0.5f;

	if ( mousestate.xpos >= ( 1.0f - MOUSE_EDGE_EPS ) ) {
	
		mousestate.xpos -= 0.5f;
		last_xpos -= 0.5f;
		
		INPs_MouseSetState( &mousestate );
	}

	if ( mousestate.ypos >= ( 1.0f - MOUSE_EDGE_EPS ) ) {
	
		mousestate.ypos -= 0.5f;
		last_ypos -= 0.5f;
		
		INPs_MouseSetState( &mousestate );
	}
	
	if ( mousestate.xpos <= ( 0.0f + MOUSE_EDGE_EPS ) ) {
	
		mousestate.xpos += 0.5f;
		last_xpos += 0.5f;
		
		INPs_MouseSetState( &mousestate );
	}

	if ( mousestate.ypos <= ( 0.0f + MOUSE_EDGE_EPS ) ) {
	
		mousestate.ypos += 0.5f;
		last_ypos += 0.5f;
		
		INPs_MouseSetState( &mousestate );
	}


	float mappedxpos = mousestate.xpos;
	float mappedypos = mousestate.ypos;

	mappedxpos = ( mappedxpos - last_xpos ) * ( inp_mouse_sensitivity / 100.0f ) + 0.5f;
	mappedypos = ( mappedypos - last_ypos ) * ( inp_mouse_sensitivity / 100.0f ) + 0.5f;

	if ( mappedxpos < -3.0f ) {
		mappedxpos = -3.0f;
	}
	if ( mappedxpos > 3.0f ) {
		mappedxpos = 3.0f;
	}

	if ( mappedypos < -3.0f ) {
		mappedypos = -3.0f;
	}
	if ( mappedypos > 3.0f ) {
		mappedypos = 3.0f;
	}

	bams_t	c_angle;

	c_angle  = (bams_t) -( JOY_Y_MIN + JOY_Y_RANGE * mappedypos );
	c_angle *= MyShip->PitchPerRefFrame * CurScreenRefFrames;
	c_angle = (bams_t)(c_angle * joy_pitch_corr_refframe);

	if ( c_angle != 0 ) {
		INP_UserRotX( inp_mouse_invert_yaxis ? -c_angle : c_angle );
	}

	c_angle  = (bams_t) -( JOY_X_MIN + JOY_X_RANGE * mappedxpos );
	c_angle *= MyShip->YawPerRefFrame * CurScreenRefFrames;
	c_angle = (bams_t)(c_angle * joy_yaw_corr_refframe);

	if ( c_angle != 0 ) {
		INP_UserRotY( c_angle );
	}

	/*
	static float last_xpos = 0.5f;
	static float last_ypos = 0.5f;

	float posdifx = mousestate.xpos - last_xpos;
	float posdify = mousestate.ypos - last_ypos;

	last_xpos = mousestate.xpos;
	last_ypos = mousestate.ypos;

	if ( ( posdifx < -0.01f ) || ( posdifx > 0.01f ) ) {
		return;
	}
	if ( ( posdify < -0.01f ) || ( posdify > 0.01f ) ) {
		return;
	}
	*/

	if ( inp_mouse_drift < 0 ) {
		inp_mouse_drift = 0;
	}
	if ( inp_mouse_drift > 100 ) {
		inp_mouse_drift = 100;
	}

	float mousedrift = ( inp_mouse_drift / 1000.0f ) * CurScreenRefFrames;
	if ( mousedrift > 1.0f ) {
		mousedrift = 1.0f;
	}

	if ( last_xpos > mousestate.xpos ) {
		last_xpos -= ( last_xpos - mousestate.xpos ) * mousedrift;
	} else {
		last_xpos += ( mousestate.xpos - last_xpos ) * mousedrift;
	}

	if ( last_ypos > mousestate.ypos ) {
		last_ypos -= ( last_ypos - mousestate.ypos ) * mousedrift;
	} else {
		last_ypos += ( mousestate.ypos - last_ypos ) * mousedrift;
	}

	/*
	float mousedrift = ( inp_mouse_drift / 1000.0f ) * CurScreenRefFrames;
	if ( mousedrift > 1.0f ) {
		mousedrift = 1.0f;
	}

	if ( mousestate.xpos > 0.5f ) {
		mousestate.xpos -= ( mousestate.xpos - 0.5f ) * mousedrift;
	} else {
		mousestate.xpos += ( 0.5f - mousestate.xpos ) * mousedrift;
	}

	if ( mousestate.ypos > 0.5f ) {
		mousestate.ypos -= ( mousestate.ypos - 0.5f ) * mousedrift;
	} else {
		mousestate.ypos += ( 0.5f - mousestate.ypos ) * mousedrift;
	}

	// set current drift destination
	INPs_MouseSetState( &mousestate );
	*/

}


// process additional input devices -------------------------------------------
//
void ISDL_UserProcessAuxInput()
{
#ifndef DISABLE_JOYSTICK_CODE
	ISDLm_ProcessMotionJoystick();
	ISDLm_ProcessJoystickBinds();
#endif
	ISDLm_ProcessMotionMouse();
}


// registration table for mouse config flags ----------------------------------
//
int_command_s inp_mouse_int_commands[] = {

	{ 0x01,	"inp.mouse_invert_yaxis", 0, 1,    &inp_mouse_invert_yaxis, NULL, NULL },
	{ 0x01,	"inp.mouse_sensitivity",  0, 2000, &inp_mouse_sensitivity,  NULL, NULL },
	{ 0x01,	"inp.mouse_drift",        0, 100,  &inp_mouse_drift,        NULL, NULL },
};

#define NUM_INP_MOUSE_INT_COMMANDS CALC_NUM_ARRAY_ENTRIES( inp_mouse_int_commands )


// module registration function -----------------------------------------------
//
REGISTER_MODULE( ISDL_SUPP )
{
	// register mouse config flags
	for ( unsigned int curcmd = 0; curcmd < NUM_INP_MOUSE_INT_COMMANDS; curcmd++ ) {
		CON_RegisterIntCommand( &inp_mouse_int_commands[ curcmd ] );
	}
}



