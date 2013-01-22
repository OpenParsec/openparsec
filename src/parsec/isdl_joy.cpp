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

#if defined( SYSTEM_SDL ) && ! defined ( DISABLE_JOYSTICK_CODE )  // TODO: Disabled for now.
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
#define				MAX_JOYSTICK_DEVICES 	4


// joystick device name -------------------------------------------------------
//
//#define 			JOY_DEVICENAME_STRING	"/dev/js%d"


// external variables ---------------------------------------------------------
//
//extern int			isdl_bDisableJoystick;	// set to disable joystick
extern int			isdl_bHasThrottle;		// indicates, the joystick has a throttle
extern int			isdl_bHasRudder;			// indicates, the joystick has a rudder
extern joystate_s	JoyState;				// generic joystick data


// string constants -----------------------------------------------------------
//
static char joystick_disabled[]		= "Joystick code is disabled.\n";


// module local variables -----------------------------------------------------
//
//int					il_fd = -1;				// handle to an open joystick
SDL_Joystick* 		isdl_joyHandle;			// handle to the open joystick
byte				isdl_NumAxes;				// number of axes for this joystick
byte				isdl_NumButtons;			// number of buttons for this joystick
//int					il_version;
//char				il_JoyName[ NAME_LENGTH ] = "Unknown";

int					isdl_nJoystickFound  	= 0;	// number of joysticks found

//int					il_InitJoyDone		= FALSE;// indicates whether Joystick was initialized

//dword				il_joyRange_X;			// joystick range X  axis
//dword				il_joyRange_Y;			// joystick range Y  axis
//dword				il_joyRange_Z;			// joystick range Z  axis
//dword				il_joyRange_R;			// joystick range R  axis

//dword				il_joyDeadZone_Min_X;   // joystick dead Zone min X
//dword				il_joyDeadZone_Max_X;   // joystick dead Zone max X
//dword				il_joyDeadZone_Min_Y;	// joystick dead Zone min Y
//dword				il_joyDeadZone_Max_Y;	// joystick dead Zone max Y
//dword				il_joyDeadZone_Min_Z;	// joystick dead Zone min Z
//dword				il_joyDeadZone_Max_Z;	// joystick dead Zone max Z
//dword				il_joyDeadZone_Min_R;	// joystick dead Zone min R
//dword				il_joyDeadZone_Max_R;	// joystick dead Zone max R



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

/* //TODO: Remove old ass linux-specific code 
PRIVATE
int ILm_JoyInit()
{
	if ( il_InitJoyDone )
		return 1;
	char szDeviceName[ 64 ];

	// try to open one of the joystick devices
	int nDevice = 0;
	for( nDevice = 0; ( il_fd < 0 ) && ( nDevice < MAX_JOYSTICK_DEVICES ); nDevice++ ) {
		sprintf( szDeviceName, JOY_DEVICENAME_STRING, nDevice );

		il_fd = open( szDeviceName, O_RDONLY );
	}

	if ( il_fd  < 0 ) {
		// disable joystick, if no device could be opened
		il_bDisableJoystick = TRUE;
	} else {
		// query number of axes, buttons, driver version and joystick name
		ioctl( il_fd, JSIOCGVERSION,			&il_version);
		ioctl( il_fd, JSIOCGAXES, 				&isdl_NumAxes);
		ioctl( il_fd, JSIOCGBUTTONS,			&isdl_NumButtons);
		ioctl( il_fd, JSIOCGNAME( NAME_LENGTH ),il_JoyName);

		// sanety check, as JoyState _ONLY_ :) has 32 buttons defined
		if( isdl_NumButtons > 32 )
			isdl_NumButtons = 32;

		MSGOUT("Using %s with %d axes and %d buttons. (Driver version %d.%d.%d on /dev/js%d)\n",
			il_JoyName, isdl_NumAxes, isdl_NumButtons, il_version >> 16, (il_version >> 8) & 0xff, il_version & 0xff, nDevice);

		//FIXME: eventually we should also support the 0.x driver version, which uses polling

		// set the number of found joysticks
		isdl_nJoystickFound = 1;

		// set flags for querying the rudder/throttle
		il_bHasRudder   = isdl_NumAxes > 2;
		il_bHasThrottle = isdl_NumAxes > 3;

		// calculate the ranges
		il_joyRange_X	= JOY_DEVICE_RANGE;
		il_joyRange_Y	= JOY_DEVICE_RANGE;
		il_joyRange_Z	= JOY_DEVICE_RANGE;
		il_joyRange_R   = JOY_DEVICE_RANGE;

		// calculate the dead zones
		il_joyDeadZone_Min_X = JOY_DEVICE_MIN + ( il_joyRange_X / 2 ) - ( il_joyRange_X / 8 );
		il_joyDeadZone_Max_X = JOY_DEVICE_MIN + ( il_joyRange_X / 2 ) + ( il_joyRange_X / 8 );

		il_joyDeadZone_Min_Y = JOY_DEVICE_MIN + ( il_joyRange_Y / 2 ) - ( il_joyRange_Y / 8 );
		il_joyDeadZone_Max_Y = JOY_DEVICE_MIN + ( il_joyRange_Y / 2 ) + ( il_joyRange_Y / 8 );

		il_joyDeadZone_Min_Z = JOY_DEVICE_MIN + ( il_joyRange_Z / 2 ) - ( il_joyRange_Z / 8 );
		il_joyDeadZone_Max_Z = JOY_DEVICE_MIN + ( il_joyRange_Z / 2 ) + ( il_joyRange_Z / 8 );

		il_joyDeadZone_Min_R = JOY_DEVICE_MIN + ( il_joyRange_R / 2 ) - ( il_joyRange_R / 8 );
		il_joyDeadZone_Max_R = JOY_DEVICE_MIN + ( il_joyRange_R / 2 ) + ( il_joyRange_R / 8 );


		// and set the joystick device in non-blocking mode
		fcntl(il_fd, F_SETFL, O_NONBLOCK);
	}

    isdl_nJoystickFound = SDL_NumJoysticks();
    MSGOUT("isdl_joy: Found %d joysticks.\n", isdl_nJoystickFound);

	il_InitJoyDone = TRUE;

	return 1;
} 
*/

// close joystick device ------------------------------------------------------
//
void ISDL_JoyKillHandler()
{
	for (int i = 0; i < isdl_nJoystickFound; i++)
	{
#if SDL_VERSION_ATLEAST(2,0,0)
		// TODO: implement
#else
		if (SDL_JoystickOpened(i))
#endif
			//SDL_JoystickClose(isdl_joyHandle[i]);
			SDL_JoystickClose(isdl_joyHandle);
	}
}

/* //TODO: Remove old ass linux-specific code 
PRIVATE
int ILm_JoyExit()
{
	if ( !il_InitJoyDone )
		return 1;

	il_InitJoyDone = FALSE;

	// close the joystick device
	close( il_fd );

	return 1;
}
*/

// simulate assignable keypresses bound to specific joystick buttons ----------
//
/*
PRIVATE
void ILm_HandleAssignableButtonKeys( dword joy_button_changed )
{

#ifdef JOY_AKC_SUPPORT

	// additional key mappings table
	keyaddition_s *kap = &KeyAdditional->table;
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


// set value for joystick axis x ----------------------------------------------
//
/*PRIVATE
void ILm_JoyAxisX( Sint16 value )
{
	// check against dead zone
	if ( ( (dword)value < il_joyDeadZone_Min_X ) || ( (dword)value > il_joyDeadZone_Max_X ) ) {
		JoyState.X = JOY_X_MIN + int( float( value - JOY_DEVICE_MIN ) /
						float( il_joyRange_X ) * ( JOY_X_RANGE ) );
	} else {
		JoyState.X = JOY_X_ZERO;
	}
}


// set value for joystick axis y ----------------------------------------------
//
PRIVATE
void ILm_JoyAxisY( Sint16 value )
{
	// check against dead zone
	if ( ( (dword)value < il_joyDeadZone_Min_Y ) || ( (dword)value > il_joyDeadZone_Max_Y ) ) {
		JoyState.Y = JOY_Y_MIN + int( float( value - JOY_DEVICE_MIN ) /
						float( il_joyRange_Y ) * ( JOY_Y_RANGE ) );
	} else {
		JoyState.Y = JOY_Y_ZERO;
	}
}


// set value for joystick axis z (throttle) -----------------------------------
//
PRIVATE
void ILm_JoyAxisZ( Sint16 value )
{
	// NOTE: no dead zone check for the throttle
	JoyState.Z = JOY_THROTTLE_MIN + int( float( value - JOY_DEVICE_MIN ) /
					float( il_joyRange_Z ) * ( JOY_THROTTLE_RANGE ) );
}


// set value for joystick axis r (rudder) -------------------------------------
//
PRIVATE
void ILm_JoyAxisR( Sint16 value )
{
	if ( ( (dword)value < il_joyDeadZone_Min_R ) || ( (dword)value > il_joyDeadZone_Max_R ) ) {
		JoyState.Rz = JOY_RUDDER_MIN   + int( float( value - JOY_DEVICE_MIN ) /
						float( il_joyRange_R ) * ( JOY_RUDDER_RANGE ) );
	} else {
		JoyState.Rz = JOY_RUDDER_ZERO;
	}
 } */


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
			JoyState.X = SDL_JoystickGetAxis(isdl_joyHandle, 1) / JOY_Y_DIV;
			JoyState.Y = SDL_JoystickGetAxis(isdl_joyHandle, 0) / JOY_X_DIV;
		}
		else
		{
			JoyState.X = SDL_JoystickGetAxis(isdl_joyHandle, 0) / JOY_X_DIV;
			JoyState.Y = SDL_JoystickGetAxis(isdl_joyHandle, 1) / JOY_Y_DIV;
		}
	}
	if (isdl_NumAxes >= 3)
	{
		if(isdl_swap_axes23)
		{
			JoyState.Rz = SDL_JoystickGetAxis(isdl_joyHandle, 2) / JOY_RUDDER_DIV;
			isdl_bHasRudder = TRUE;
		}
		else
		{
			JoyState.Z = SDL_JoystickGetAxis(isdl_joyHandle, 2) / JOY_THROTTLE_DIV + JOY_THROTTLE_OFF;
			isdl_bHasThrottle = TRUE;
		}
	}
	if (isdl_NumAxes >= 4)
	{
		if(isdl_swap_axes23)
		{
			JoyState.Z = SDL_JoystickGetAxis(isdl_joyHandle, 3) / JOY_THROTTLE_DIV + JOY_THROTTLE_OFF;
			isdl_bHasThrottle = TRUE;
		}
		else
		{
			JoyState.Rz = SDL_JoystickGetAxis(isdl_joyHandle, 3) / JOY_RUDDER_DIV;
			isdl_bHasRudder = TRUE;
		}
	}

	// Read buttons
	keyaddition_s *kap = &KeyAdditional->table;
	ASSERT( KeyAdditional->size >= 0 );
	ASSERT( KeyAdditional->size <= KEY_ADDITIONS_MAX );
	for (int button = 0; button < isdl_NumButtons; button++)
	{
		JoyState.Buttons[button] = SDL_JoystickGetButton(isdl_joyHandle, button);
#ifdef JOY_AKC_SUPPORT
        if ( _OldJoyState.Buttons[button] != JoyState.Buttons[button])
        {
            for (int aid = 1; aid < KeyAdditional->size; aid++)
            {
                if (kap[aid].code == ((dword)button | (dword)AKC_JOY_FLAG))
                {
                	printf("TESTING: aid = %d code = %d button = %d looking for %d\n", 
                		   aid, kap[aid].code, button, (dword)button | (dword)AKC_JOY_FLAG);
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
    
/* //TODO: Remove ancient Linux-specific joystick code.
PRIVATE
int ILm_ReadJoystickData()
{
	if ( !QueryJoystick )
		return FALSE;
	struct js_event js;

#ifdef JOY_AKC_SUPPORT

	joystate_s _OldJoyState;

	if ( AUX_ENABLE_JOYSTICK_BINDING ) {
		// copy the previous joystate;
		memcpy( &_OldJoyState, &JoyState, sizeof( joystate_s ) );
	}

#endif // JOY_AKC_SUPPORT

	// read in all the pending joystick events
	while ( read( il_fd, &js, sizeof( struct js_event ) ) == sizeof( struct js_event ) )  {

			switch ( js.type & ~JS_EVENT_INIT ) {

				case JS_EVENT_BUTTON:
					JoyState.Buttons[ js.number ] = js.value ? 0x80 : 0x00;
					break;

				case JS_EVENT_AXIS:
					switch ( js.number ) {

						case 0:
							if ( !isdl_swap_axes01 ) {
								ILm_JoyAxisX( js.value );
							} else {
								ILm_JoyAxisY( js.value );
							}
							break;

						case 1:
							if ( !isdl_swap_axes01 ) {
								ILm_JoyAxisY( js.value );
							} else {
								ILm_JoyAxisX( js.value );
							}
							break;

						case 2:
							if ( !isdl_swap_axes23 ) {
								ILm_JoyAxisZ( js.value );
							} else {
								ILm_JoyAxisR( js.value );
							}
							break;

						case 3:
							if ( !isdl_swap_axes23 ) {
								ILm_JoyAxisR( js.value );
							} else {
								ILm_JoyAxisZ( js.value );
							}
							break;
					}
					break;
			}

//			MSGOUT( "Event: type %d, time %d, number %d, value %d\n",
//				    js.type, js.time, js.number, js.value);
	}

#ifdef JOY_AKC_SUPPORT

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

#endif // JOY_AKC_SUPPORT

	// check for error condition while reading
	if ( errno != EAGAIN ) {
		MSGOUT( "error reading from joystick." );
		return FALSE;
	}

	return TRUE;
}
*/



// registration table for joystick config flags -------------------------------
//
int_command_s il_joy_int_commands[] = {

	{ 0x00,	"isdl.swap_joyaxes_01", 0, 1, &isdl_swap_axes01,	NULL, NULL },
	{ 0x00,	"isdl.swap_joyaxes_23", 0, 1, &isdl_swap_axes23,	NULL, NULL },
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
