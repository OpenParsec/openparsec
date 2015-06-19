/*
 * PARSEC - Joystick Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:39 $
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
// compilation flags/debug support
#include "config.h"

#if !defined ( DISABLE_JOYSTICK_CODE )  // TODO: Disabled for now.

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
#include "isdl_joy.h"

// proprietary headers
#include "con_aux.h"
#include "con_int.h"
#include "keycodes.h"


// flags
#define JOY_AKC_SUPPORT



// ----------------------------------------------------------------------------
//
#define NAME_LENGTH		128


// maximum number of joystick devices we support ------------------------------
//
#define				MAX_JOYSTICK_DEVICES	4

// external variables ---------------------------------------------------------
//
extern int			isdl_bHasThrottle;		// indicates, the joystick has a throttle
extern int			isdl_bHasRudder;		// indicates, the joystick has a rudder
extern joystate_s	JoyState;				// generic joystick data


// string constants -----------------------------------------------------------
//
static char joystick_disabled[]		= "Joystick code is disabled.\n";


// module local variables -----------------------------------------------------
//
int                 isdl_joyOutput;         //When enabled output joy button states to console
SDL_Joystick*		isdl_joyHandle;			// handle to the open joystick
byte				isdl_NumAxes;			// number of axes for this joystick
byte				isdl_NumButtons;		// number of buttons for this joystick
int                 isdl_FireGun;
int                 isdl_FireMissile;
int                 isdl_Accelerate;
int                 isdl_Deccelerate;
int                 isdl_Rollleft;
int                 isdl_RollRight;
int                 isdl_NextGun;
int                 isdl_NextMissile;
int                 isdl_RudderToggle;
int                 isdl_ThrottleToggle;
int					isdl_nJoystickFound;	// number of joysticks found

// Joystick deadzones, in terms of SDL joystick range (-32768..32768)
static int          isdl_joyDeadZone_Min[4] = { -5, -5, -5, -5 };
static int          isdl_joyDeadZone_Max[4] = {  5,  5,  5,  5 };


// initialize joystick device -------------------------------------------------
//
void ISDL_JoyInitHandler()
{
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	isdl_nJoystickFound = SDL_NumJoysticks();
    MSGOUT("isdl_joy: Found %d joysticks.\n", isdl_nJoystickFound);
    if(isdl_nJoystickFound > 0)
    {
		//TODO: Implement support for more than one stick when we have more flexible input layer stuff
		MSGOUT("isdl_joy: ... but we don't care, because Parsec only has one JoyState\n");
		isdl_joyHandle = SDL_JoystickOpen(0);
		isdl_NumAxes = SDL_JoystickNumAxes(isdl_joyHandle);
		isdl_NumButtons = SDL_JoystickNumButtons(isdl_joyHandle);
		isdl_NumButtons = isdl_NumButtons > 32 ? 32 : isdl_NumButtons;
		MSGOUT("isdl_joy: Joystick%d has %d axes and %d buttons\n", 0, isdl_NumAxes, isdl_NumButtons);
		QueryJoystick = TRUE;
		JoystickDisabled = FALSE;
    }
}


// close joystick device ------------------------------------------------------
//
void ISDL_JoyKillHandler()
{
	SDL_JoystickClose(isdl_joyHandle);
	return;
	/*
	for (int i = 0; i < isdl_nJoystickFound; i++)
	{
		// TODO: implement
		//if (SDL_JoystickOpened(i))
			//SDL_JoystickClose(isdl_joyHandle[i]);
			SDL_JoystickClose(isdl_joyHandle);
	}
	 */
}

// simulate assignable keypresses bound to specific joystick buttons ----------
//
/*
PRIVATE
void ILm_HandleAssignableButtonKeys( dword joy_button_changed )
{

#ifdef JOY_AKC_SUPPORT

	// additional key mappings table
	keyaddition_s *kap = KeyAdditional->table;
	ASSERT( KeyAdditional->size >= 0 );
	ASSERT( KeyAdditional->size <= KEY_ADDITIONS_MAX );

	// check the additional assignments
	for ( int aid = KeyAdditional->size - 1; aid >= 0; aid-- ) {

		// skip non-joystick akcs
		if ( ( kap[ aid ].code & ~AKC_JOY_FLAG ) == 0 )
			continue;

		// fetch button id by masking out flag
		dword button = kap[ aid ].code & ~AKC_JOY_FLAG;
		printf("derp! aid: %d button:%d\n", aid, button);
		//ASSERT( ( button >= 0 ) && ( button <= 31 ) );
		if(! (( button >= 0 ) && ( button <= 31 )) )
			continue;

		// check button changed
		if ( joy_button_changed & ( 1 << button ) ) {

			//NOTE:
			// the actual execution of the mapped command will be
			// done by CON_KMAP::ExecBoundKeyCommands(). this
			// function only simulates some kind of special keys
			// using codes that cannot occur via the keyboard.

			// set the state flag
			kap[ aid ].state = JoyState.Buttons[ button ];
		}
	}

#endif // JOY_AKC_SUPPORT

}
*/

// axes swapping flags --------------------------------------------------------
//
static int isdl_swap_axes01 = FALSE;
static int isdl_swap_axes23 = FALSE;


// apply deadzones ------------------------------------------------------------
//
PRIVATE 
int ISDL_ApplyDZ(int value, int axis)
{
	if ( value > isdl_joyDeadZone_Min[axis] && 
		 value < isdl_joyDeadZone_Max[axis] )
		return 0;
	else
		return value;
}


// read current joystick state ( positions and buttons ) ----------------------
//
void ISDL_JoyCollect()
{
	if ( !QueryJoystick )
			return;

	joystate_s _OldJoyState = JoyState; // Save previous JoyState to detect changes

	// Read Axes
	if (isdl_NumAxes >= 2)
	{
		if(isdl_swap_axes01)
		{
			JoyState.X = ISDL_ApplyDZ(SDL_JoystickGetAxis(isdl_joyHandle, 1), 1) / JOY_Y_DIV;
			JoyState.Y = ISDL_ApplyDZ(SDL_JoystickGetAxis(isdl_joyHandle, 0), 0) / JOY_X_DIV;
		}
		else
		{
			JoyState.X = ISDL_ApplyDZ(SDL_JoystickGetAxis(isdl_joyHandle, 0), 0) / JOY_X_DIV;
			JoyState.Y = ISDL_ApplyDZ(SDL_JoystickGetAxis(isdl_joyHandle, 1), 1) / JOY_Y_DIV;
		}
	}
	if (isdl_NumAxes >= 3)
	{
		if(isdl_swap_axes23)
		{
			if(isdl_RudderToggle) {
				JoyState.Rz = ISDL_ApplyDZ(SDL_JoystickGetAxis(isdl_joyHandle, 2), 2) / JOY_RUDDER_DIV;
				isdl_bHasRudder = TRUE;
			} else {
				isdl_bHasRudder = FALSE;
			}
		}
		else
		{
			if(isdl_ThrottleToggle) {
				// Do not apply deadzones to throttle
				JoyState.Z = SDL_JoystickGetAxis(isdl_joyHandle, 2) / JOY_THROTTLE_DIV + JOY_THROTTLE_OFF;
				isdl_bHasThrottle = TRUE;
			} else {
				isdl_bHasThrottle = FALSE;
			}
		}
	}
	if (isdl_NumAxes >= 4)
	{
		if(isdl_swap_axes23)
		{
			if(isdl_ThrottleToggle) {
				// Do not apply deadzones to throttle
				JoyState.Z = SDL_JoystickGetAxis(isdl_joyHandle, 3) / JOY_THROTTLE_DIV + JOY_THROTTLE_OFF;
				isdl_bHasThrottle = TRUE;
			} else {
				isdl_bHasThrottle = FALSE;
			}
		}
		else
		{
			if(isdl_RudderToggle) {
				JoyState.Rz = ISDL_ApplyDZ(SDL_JoystickGetAxis(isdl_joyHandle, 3), 3) / JOY_RUDDER_DIV;
				isdl_bHasRudder = TRUE;
			} else {
				isdl_bHasRudder = FALSE;
			}
		}
	}

	// Read buttons
	keyaddition_s *kap = KeyAdditional->table;
	ASSERT( KeyAdditional->size >= 0 );
	ASSERT( KeyAdditional->size <= KEY_ADDITIONS_MAX );
	for (int button = 0; button < isdl_NumButtons; button++) {
		JoyState.Buttons[button] = SDL_JoystickGetButton(isdl_joyHandle, button);
		if(isdl_joyOutput && JoyState.Buttons[button])
			MSGOUT("Joy button %d pressed",button);
		
#ifdef JOY_AKC_SUPPORT
        if ( _OldJoyState.Buttons[button] != JoyState.Buttons[button]) {
            for (int aid = 1; aid < KeyAdditional->size; aid++) {
                if (kap[aid].code == ((dword)button | (dword)AKC_JOY_FLAG)) {
                    kap[aid].state = (JoyState.Buttons[button] ? TRUE : FALSE);
                }
            }
        }
#endif // JOY_AKC_SUPPORT
	}
/*
	// Fake AKC_ keys for joystick buttons that fit in it

	if ( AUX_ENABLE_JOYSTICK_BINDING ) {

		int mask			   = 0x0001;
		int joy_button_changed = 0x0000;

		// check which buttons have changed
		for ( int nButton = 0; nButton < isdl_NumButtons; nButton++ ) {

			// check for button changed
			joy_button_changed |= ( _OldJoyState.Buttons[ nButton ] != JoyState.Buttons[ nButton ] ) ? mask : 0;

			mask = mask << 1;
		}

		ILm_HandleAssignableButtonKeys( joy_button_changed );
	}
*/
	// Confused yet? Me too...
}
    
// registration table for joystick config flags -------------------------------
//
int_command_s il_joy_int_commands[] = {

	{ 0x01,	"isdl.swap_joyaxes_01"    ,      0, 1, &isdl_swap_axes01,	     NULL, NULL     },
	{ 0x01,	"isdl.swap_joyaxes_23"    ,      0, 1, &isdl_swap_axes23,	     NULL, NULL     },
	{ 0x01, "isdl.deadzone_min_axis_0", -32768, 0, &isdl_joyDeadZone_Min[0] , NULL, NULL    },
	{ 0x01, "isdl.deadzone_min_axis_1", -32768, 0, &isdl_joyDeadZone_Min[1] , NULL, NULL    },
	{ 0x01, "isdl.deadzone_min_axis_2", -32768, 0, &isdl_joyDeadZone_Min[2] , NULL, NULL    },
	{ 0x01, "isdl.deadzone_min_axis_3", -32768, 0, &isdl_joyDeadZone_Min[3] , NULL, NULL    },
	{ 0x01, "isdl.deadzone_max_axis_0", 0,  32767, &isdl_joyDeadZone_Max[0] , NULL, NULL    },
	{ 0x01, "isdl.deadzone_max_axis_1", 0,  32767, &isdl_joyDeadZone_Max[1] , NULL, NULL    },
	{ 0x01, "isdl.deadzone_max_axis_2", 0,  32767, &isdl_joyDeadZone_Max[2] , NULL, NULL    },
	{ 0x01, "isdl.deadzone_max_axis_3", 0,  32767, &isdl_joyDeadZone_Max[3] , NULL, NULL    },
	{ 0x01, "isdl.showjoy_input"      , 0,      1, &isdl_joyOutput          , NULL, NULL,0  }, //default off
	{ 0x01, "isdl.jbind_gun"          , -1,     64,&isdl_FireGun            , NULL, NULL,0  }, //button 0 default
	{ 0x01, "isdl.jbind_missile"      , -1,     64,&isdl_FireMissile        , NULL, NULL,1  }, //button 1 default
	{ 0x01, "isdl.jbind_accel"        , -1,     64,&isdl_Accelerate         , NULL, NULL,-1 }, //disabled until bound
	{ 0x01, "isdl.jbind_decel"        , -1,     64,&isdl_Deccelerate        , NULL, NULL,-1 },
	{ 0x01, "isdl.jbind_rollleft"     , -1,     64,&isdl_Rollleft           , NULL, NULL,-1 },
	{ 0x01, "isdl.jbind_rollright"    , -1,     64,&isdl_RollRight          , NULL, NULL,-1 },
	{ 0x01, "isdl.jbind_nextgun"      , -1,     64,&isdl_NextGun            , NULL, NULL,-1 },
	{ 0x01, "isdl.jbind_nextmissile"  , -1,     64,&isdl_NextMissile        , NULL, NULL,-1 },
	{ 0x01, "isdl.analogthrottle"     , -1,      1, &isdl_RudderToggle      , NULL, NULL,0  }, //default off
	{ 0x01, "isdl.analogrudder"       , -1,      1, &isdl_ThrottleToggle    , NULL, NULL,0  },

};

#define NUM_IL_JOY_INT_COMMANDS CALC_NUM_ARRAY_ENTRIES( il_joy_int_commands )


// module registration function -----------------------------------------------
//
REGISTER_MODULE( IL_JOY )
{
	// register joystick config flags
	for ( unsigned int curcmd = 0; curcmd < NUM_IL_JOY_INT_COMMANDS; curcmd++ ) {
		CON_RegisterIntCommand( &il_joy_int_commands[ curcmd ] );
	}
}

#endif
