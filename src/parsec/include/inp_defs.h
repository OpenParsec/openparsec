/*
 * PARSEC HEADER: inp_defs.h
 */

#ifndef _INP_DEFS_H_
#define _INP_DEFS_H_


// ----------------------------------------------------------------------------
// INPUT SUBSYSTEM (INP) related definitions                                  -
// ----------------------------------------------------------------------------


// keys corresponding to game functions (gamefunckeys) ---------------------

struct keyfunc_s {

	dword	key_Escape;				// 00
	dword	key_TurnLeft;			// 01
	dword	key_TurnRight;			// 02
	dword	key_DiveDown;			// 03
	dword	key_PullUp;				// 04
	dword	key_RollLeft;			// 05
	dword	key_RollRight;			// 06
	dword	key_ShootWeapon;		// 07
	dword	key_LaunchMissile;		// 08
	dword	key_NextWeapon;			// 09
	dword	key_NextMissile;		// 10
	dword	key_Accelerate;			// 11
	dword	key_Decelerate;			// 12
	dword	key_SlideLeft;			// 13
	dword	key_SlideRight;			// 14
	dword	key_SlideUp;			// 15
	dword	key_SlideDown;			// 16
	dword	key_NextTarget;			// 17
	dword	key_ToggleFrameRate;	// 18
	dword	key_ToggleObjCamera;	// 19
	dword	key_ToggleHelp;			// 20
	dword	key_ToggleConsole;		// 21
	dword	key_SaveScreenShot;		// 22
	dword	key_ShowKillStats;		// 23
	dword	key_SpeedZero;			// 24
	dword	key_TargetSpeed;		// 25
	dword	key_AfterBurner;		// 26
	dword	key_FrontTarget;		// 27
	dword	key_Select;				// 28
	dword	key_CursorLeft;			// 29
	dword	key_CursorRight;		// 30
	dword	key_CursorUp;			// 31
	dword	key_CursorDown;			// 32

};

#define NUM_GAMEFUNC_KEYS ( sizeof( keyfunc_s ) / sizeof( ((keyfunc_s*)0)->key_Escape ) )


// single entry in table of additional key mappings
struct keyaddition_s {

	dword code;				// AKC_ code (full 32 bit valid!)
	int	  state;			// 0: released  1: depressed
};

// header of table containing additional key mappings
struct keyaddctrl_s {

	int				size;	// number of key entries in table
	keyaddition_s	table;	// first entry
};

//CAVEAT: MUST BE CONSISTENT WITH DEFINITION IN ID_KEYB.INC!!
#define KEY_ADDITIONS_MAX 128


// various keyboard state flags
struct keybflags_s {

	byte	ConEnabled;		// console visible and may potentially be typed into
	byte	ConActive;		// enables keyboard routing into console
	byte	ConTogReleased;	// -1 if contoggle is released (internal use only)
	byte	ExtOn;			// extended key (keyboard-code internal use only)
	byte	ShiftOn;		// shift is down (keyboard-code internal use only)
};

//NOTE:
// if ( ConEnabled == TRUE ) the console is visible and enabled. that is,
// the console toggle key is checked and used to toggle keyboard routing into
// the console on/off. routing is achieved by using ConActive.
//
// that is:
// console enabled/visible and may  be typed into:
//  ( ConEnabled == TRUE ) && ( ConActive == TRUE )
// console enabled/visible but may not be typed into:
//  ( ConEnabled == TRUE ) && ( ConActive == FALSE )
// console disabled/invisible (also no typing, of course)
//  ( ConEnabled == FALSE ) && ( ConActive == DONT_CARE )
//
// therefore, ConActive stores the current state of console
// keyboard routing (also if the console is disabled
// in the meantime!)


// Information for a single keypress ------------------------------------------
//
struct keypress_s {
	dword key;
	dword unicode;
	bool pressed;
};


// generic joystick data structure --------------------------------------------
//
struct joystate_s {

    int32	X;						// x-axis position
    int32	Y;						// y-axis position
    int32   Z;						// z-axis position
    int32   Rx;						// x-axis rotation
    int32   Ry;						// y-axis rotation
    int32   Rz;						// z-axis rotation
    int32	Slider[ 2 ];			// extra axes positions
    int32	POV[ 4 ];				// POV directions
    byte	Buttons[ 32 ];			// 32 buttons
};


// button query masks ---------------------------------------------------------
//
#define JOYBUTT1MASK		0x10
#define JOYBUTT2MASK		0x20
#define JOYBUTT3MASK		0x30	// both buttons pressed -> button 3
#define JOYBUTTFIELDMASK	0x30


// external variables ---------------------------------------------------------
//
extern keyfunc_s *		DepressedKeys;
extern keyfunc_s *		KeyAssignments;
extern keyaddctrl_s *	KeyAdditional;
extern keybflags_s *	KeybFlags;


// joystick resolution --------------------------------------------------------
//
#define JOY_FLAT_CENTER				0.15
#define JOY_HALF_RES				32
#define EP_WAIT 					50


// joystick constraints -------------------------------------------------------
//
#define JOY_THROTTLE_MIN			0
#define JOY_THROTTLE_MAX			255
#define JOY_THROTTLE_DEADZONE_MASK  0xFFFFFFF0
#define JOY_THROTTLE_RANGE          256

#define JOY_X_MIN					-32
#define JOY_X_ZERO					  0
#define JOY_X_MAX					 32
#define JOY_X_RANGE					 65

#define JOY_Y_MIN					-32
#define JOY_Y_ZERO					  0
#define JOY_Y_MAX					 32
//FIXME: range should be 64 ??????
#define JOY_Y_RANGE					 65	

#define JOY_RUDDER_MIN				-32
#define JOY_RUDDER_ZERO				  0
#define JOY_RUDDER_MAX				 32
//FIXME: range should be 64 ??????
#define JOY_RUDDER_RANGE			 65


// mouse constants ------------------------------------------------------------
//
enum {

	MOUSE_BUTTON_LEFT,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_RIGHT,

	NUM_MOUSE_BUTTONS		// must be last!
};

#define MOUSE_BUTTON_RELEASED	0
#define MOUSE_BUTTON_PRESSED	1


// generic mouse data structure -----------------------------------------------
//
struct mousestate_s {

	float		xpos;							// normalized [ 0.0, 1.0 ]
	float		ypos;							// normalized [ 0.0, 1.0 ]
	byte		drawcursor;						// custom cursor drawing
	byte		buttons[ NUM_MOUSE_BUTTONS ];	// button state
};


// check if one of the select keys is pressed ---------------------------------
//
inline int SELECT_KEYS_PRESSED()
{
	return ( DepressedKeys->key_Select		  ||
			 DepressedKeys->key_ShootWeapon	  ||
			 DepressedKeys->key_LaunchMissile );
}


// reset all select keys ------------------------------------------------------
//
inline void SELECT_KEYS_RESET()
{
	DepressedKeys->key_Select		 = 0;
	DepressedKeys->key_ShootWeapon	 = 0;
	DepressedKeys->key_LaunchMissile = 0;
}


// wait for <space>/<enter>/<esc> while bios keyboard-handler inactive --------
//
inline void WaitForKeypress()
{
	while ( !DepressedKeys->key_Select		  &&
			!DepressedKeys->key_ShootWeapon	  &&
			!DepressedKeys->key_LaunchMissile &&
			!DepressedKeys->key_Escape ) NULL;

	DepressedKeys->key_Select		 = 0;
	DepressedKeys->key_ShootWeapon	 = 0;
	DepressedKeys->key_LaunchMissile = 0;
	DepressedKeys->key_Escape		 = 0;
}


// wait for new <space>/<enter>/<esc> while bios keyboard-handler inactive ----
//
inline void WaitForNewKeypress()
{
	DepressedKeys->key_Select		 = 0;
	DepressedKeys->key_ShootWeapon	 = 0;
	DepressedKeys->key_LaunchMissile = 0;
	DepressedKeys->key_Escape		 = 0;

	while ( !DepressedKeys->key_Select		  &&
			!DepressedKeys->key_ShootWeapon	  &&
			!DepressedKeys->key_LaunchMissile &&
			!DepressedKeys->key_Escape ) NULL;

	DepressedKeys->key_Select		 = 0;
	DepressedKeys->key_ShootWeapon	 = 0;
	DepressedKeys->key_LaunchMissile = 0;
	DepressedKeys->key_Escape		 = 0;
}



// include system-specific subsystem prototypes -------------------------------
//
#include "inp_subh.h"


#endif // _INP_DEFS_H_


