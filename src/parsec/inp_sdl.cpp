/*
 * PARSEC - SDL Input Interface
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:39 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-2001
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

#ifdef SYSTEM_TARGET_LINUX
	#include <SDL/SDL.h>
#else
	#include <SDL.h>
#endif

// C library
#include <signal.h>
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

// subsystem linkage info
#include "linkinfo.h"

// local module header
#include "inp_sdl.h"

// keyboard tables
#include "keycodes.h"
#include "isdl_keydf.h"

// proprietary headers
#include "con_main.h"
#include "con_aux.h"
#include "isdl_joy.h"
#include "isdl_supp.h"

#if SDL_VERSION_ATLEAST(2, 0, 0)
	// for the window handle
	#include "vsdl_ogl.h"
#endif


// flags
//#define DISABLE_JOYSTICK_CODE // moved to platform.lnx.h
//#define USE_XLOOKUPSTRING_TRANSLATION  //not using X so....



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// keyboard tables and buffers ------------------------------------------------
//
static keybflags_s	_keybflags	= { 0, 0, (byte)-1, 0, 0 };
static keyfunc_s	_depressedkeys;

keyfunc_s*			DepressedKeys	= &_depressedkeys;
keyfunc_s*			KeyAssignments	= _keyassignments;
keyaddctrl_s*		KeyAdditional	= NULL;
keybflags_s*		KeybFlags		= &_keybflags;


// globals ------------------------------------------------------------------
// TODO: work on cleaning this stuff up if it's not needed
char*			sdl_display_name = NULL;	// use DISPLAY environment variable
SDL_Surface*	sdl_display;				// server connection
//int				x_screen;				// screen on server
SDL_Surface*	sdl_window;				// main window id
SDL_Surface*	sdl_inpwin;				// window id that has input focus
//Visual*			x_visual;				// default visual
unsigned int	x_depth;				// default depth
SDL_Surface*	x_pixmap;				// pixmap of splash screen


// flags whether input handling already initialized ---------------------------
//
static int sdl_general_init_done	= FALSE;
static int sdl_keyb_init_done		= FALSE;
static int sdl_keyb_grab_active	= FALSE;


// determine bit shift from bit mask ------------------------------------------
//
INLINE
dword GetShiftFromMask( dword mask )
{
	if ( mask == 0 ) {
		return 0;
	}
	dword shift = 0;
	for ( shift = 0; ( mask & 0x01 ) == 0; mask >>= 1 ) {
		shift++;
	}

	return shift;
}


// determine number of color bits from bit mask -------------------------------
//
INLINE
dword GetColbitsFromMask( dword mask )
{
	dword colbits = 0;
	for ( colbits = 0; mask != 0; colbits++ ) {
		mask = mask & ( mask - 1 );
	}

	return colbits;
}


// convert splash image to needed visual/depth --------------------------------
//
PRIVATE
int ConvertSplashData(  )
{
	/*
	ASSERT( ximg != NULL );
	ASSERT( srcimg != NULL );
	ASSERT( dstimg != NULL );

	// determine channel shifts from masks
	dword redshift   = GetShiftFromMask( ximg->red_mask );
	dword greenshift = GetShiftFromMask( ximg->green_mask );
	dword blueshift  = GetShiftFromMask( ximg->blue_mask );

	// determine number of color bits from masks
	dword redcolbits   = GetColbitsFromMask( ximg->red_mask );
	dword greencolbits = GetColbitsFromMask( ximg->green_mask );
	dword bluecolbits  = GetColbitsFromMask( ximg->blue_mask );
*/

	// XXX: Is this function needed with SDL?

	return FALSE;
}



// create pixmap for splash screen --------------------------------------------
//
PRIVATE
int CreateSplashPixmap()
{

	// FIXME: Code this for SDL
/*
	// open splash screen rgb image
	FILE *fp = fopen( "./pscdata1.dat", "rb" );
	if ( fp == NULL ) {
		return FALSE;
	}

	// read header (width, height)
	int imghdr[ 2 ];
	if ( fread( imghdr, sizeof( imghdr ), 1, fp ) != 1 ) {
		fclose( fp );
		return FALSE;
	}
	int imgwidth  = imghdr[ 0 ];
	int imgheight = imghdr[ 1 ];

	DBGTXT( printf( "*width =%d\n", imgwidth ); );
	DBGTXT( printf( "*height=%d\n", imgheight ); );

	// use fixed-size splash screen for file validation
	if ( ( imgwidth != 640 ) || ( imgheight != 480 ) ) {
		fclose( fp );
		return FALSE;
	}

	// reserve mem for source and converted image
	size_t imgsize = imgwidth * imgheight;
	byte *splashmem = (byte *) ALLOCMEM( imgsize * ( 3 + 4 ) );
	if ( splashmem == NULL ) {
		fclose( fp );
		return FALSE;
	}
	byte *splashsrc = &splashmem[ 0 ];
	byte *splashimg = &splashmem[ imgsize * 3 ];

	// read rgb data
	if ( fread( splashsrc, 1, imgsize * 3, fp ) != imgsize * 3 ) {
		FREEMEM( splashmem );
		fclose( fp );
		return FALSE;
	}

	// all data read
	fclose( fp );
	fp = NULL;
*/
	// done reading the splash image into mem.
	/*
	 * imghdr = image header data
	 * imgwidth = image width
	 * imgheight = image height
	 * imgsize = size of the image data
	 * splashmem = the image data itself
	 * splashsrc = copy of the pointer to splashmem
	 */


	// let's toy with the idea of just displaying the splash window with SDL...
	//SDL_Surface *splashwindow = SDL_CreateRGBSurfaceFrom((void *)splashsrc,
	//		                        imgwidth, imgheight, 16, imgwidth*4, 0,0,0,0);
	
	
	// convert splash image to needed visual/depth


	// create a server pixmap resource


	// transfer client image into server pixmap


	// free x image structure (no data free)


	// free client image data.
	// (the splash screen is now a pixmap on the server)
	//FREEMEM( splashmem );


	return FALSE;
}


// flag whether splash screen pixmap is valid ---------------------------------
//
static int splash_screen_valid = FALSE;


// draw splash screen into main window ----------------------------------------
//
PRIVATE
void DrawSplashScreen()
{
	// TODO: Code this for SDL

	if ( !splash_screen_valid )
		return;

	// copy splash screen pixmap into window
}


// handle expose events -------------------------------------------------------
//
PRIVATE
void ISDLm_ExposeHandler(  )
{
		// Not Used
}


// handle configure notify events ---------------------------------------------
//
PRIVATE
void ISDLm_ConfigureHandler(  )
{
	//ASSERT( xconfigure != NULL );

	// XXX: Not Needed?

}


// init SDL input --------------------------------------------------------------
//
void INPs_InitGeneral()
{
	int rc = SDL_Init(SDL_INIT_EVERYTHING); // by the time we get here, we should want everything else up
	if(rc < 0){
		MSGOUT("Error: SDL_Init() error in INPs_InitGeneral() in inp_sdl.cpp");
		return;
	}

	// TODO: import sdl_video_init_done from vsdl.h

	//if ( sdl_video_init_done )
	//	return;


	// events we want to receive
	// XXX:Probably not used.

	// main window attributes
	// XXX:Probably not used.


	// create splash screen
	//splash_screen_valid = CreateSplashPixmap();

	// fill window with splash screen image
	//DrawSplashScreen();


	// install signal handlers
	// disabled because not cross-platform and probably not necessary
	//ISDLm_InstallSignalHandlers();


}


// de-init  input ------------------------------------------------------------
//
void INPs_KillGeneral()
{
	// TODO: Code SDL Shutdown if needed.
	if ( sdl_general_init_done ) {


	}
}


// check whether key repeat is enabled ----------------------------------------
//
PRIVATE
bool ISDLm_IsKeyRepeatEnabled()
{
#if SDL_VERSION_ATLEAST(2,0,0)
	return false;
#else
	int delay, interval;
	SDL_GetKeyRepeat(&delay, &interval);
	
	return delay > 0;
#endif
}


// enable or disable keyboard key repeat --------------------------------------
//
PRIVATE
void ISDLm_SetKeyRepeat(bool enable)
{
#if !SDL_VERSION_ATLEAST(2,0,0)
	bool isenabled = ISDLm_IsKeyRepeatEnabled();
	
	if (enable && !isenabled)
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	else if (!enable && isenabled)
		SDL_EnableKeyRepeat(0, 0);
#endif
}


PRIVATE
void ISDLm_ProcessTextInput(const char *text)
{
	if (text[0] > 31 && text[0] < 127) { // only handle ASCII for now
		CON_HandleTextInput(text[0]);
	}
}


// text input handler for SDL 2 -----------------------------------------------
//
PRIVATE
void ISDLm_TextInputHandler(const SDL_Event &event)
{
#if SDL_VERSION_ATLEAST(2,0,0)
	if (event.type != SDL_TEXTINPUT)
		return;

	ISDLm_ProcessTextInput(event.text.text);
#endif
}


// key press handler invoked by event processing loop -------------------------
//
PRIVATE
void ISDLm_KeyboardHandler(const SDL_Event &event)
{
	bool pressed = event.type == SDL_KEYDOWN;
	dword key = event.key.keysym.sym;

#if SDL_VERSION_ATLEAST(2,0,0)
	// only process a key repeat event if we're in the console or quicksay console
	if (event.key.repeat && !(KeybFlags->ConActive && KeybFlags->ConEnabled))
		return;
#else
	// enable key repeat on keyup if in the console or quicksay console
	// TODO: only set when toggling the console and quicksay console
	if (event.type == SDL_KEYUP)
		ISDLm_SetKeyRepeat(KeybFlags->ConActive && KeybFlags->ConEnabled);
#endif

	// console keyboard buffer handling
	if ( pressed && ( key != MKC_TILDE ) ) {

		if ( KeybFlags->ConActive && KeybFlags->ConEnabled ) {

			if (key == MKC_LSHIFT || key == MKC_RSHIFT) {
				return;
			}

			CON_HandleKeyPress(key);

#if !SDL_VERSION_ATLEAST(2,0,0)
			const char text[] = {(char) event.key.keysym.unicode, '\0'};
			ISDLm_ProcessTextInput(text);
#endif

			// console grabs all input
			return;
		}
	}

	// functional key handling
	dword *tab1 =  (dword *) &KeyAssignments[ 0 ];
	dword *tab2 =  (dword *) &KeyAssignments[ 1 ];
	dword *tabd =  (dword *) DepressedKeys;

	for( int fid = NUM_GAMEFUNC_KEYS - 1; fid >= 0; fid-- ) {
		if ( ( tab1[ fid ] == key ) || ( tab2[ fid ] == key ) ) {
			tabd[ fid ] = pressed;
		}
	}

	/*
	// set akc including shift flag (only if not a gray key)
	dword shiftb = ( ( KeybFlags->ShiftOn & ~KeybFlags->ExtOn ) != 0 ) ?
					AKC_SHIFT_FLAG : 0;
	*/

	// additional key mappings
	keyaddition_s *kap = KeyAdditional->table;
	ASSERT( KeyAdditional->size >= 0 );
	ASSERT( KeyAdditional->size <= KEY_ADDITIONS_MAX );

	for ( int aid = KeyAdditional->size - 1; aid >= 0; aid-- ) {
		if ( kap[ aid ].code == key ) {
			kap[ aid ].state = pressed;
		}
	}
}


// init keyboard code (alloc mem, setup tables, install handlers) -------------
//
void INPs_KeybInitHandler()
{
	// enable unicode character handling for keypress events
	
#if !SDL_VERSION_ATLEAST(2,0,0)
	SDL_EnableUNICODE(SDL_ENABLE);
#endif
	
	// XXX: Not sure if this is needed either....

	// prevent multiple inits
	if ( sdl_keyb_init_done ) {
		return;
	}

	// alloc key tables
	KeyAdditional	= (keyaddctrl_s *) ALLOCMEM( sizeof( keyaddctrl_s ) );

	// init all structs
	memset( DepressedKeys,	0, sizeof( keyfunc_s ) );
	memset( KeyAdditional,	0, sizeof( keyaddctrl_s ) );
	memset( KeybFlags,		0, sizeof( keybflags_s ) );

	KeybFlags->ConTogReleased = (byte)-1;
/*

	// grab input

*/
	sdl_keyb_init_done = TRUE;
}


// deinit keyboard code (free mem, restore driver state) ----------------------
//
void INPs_KeybKillHandler()
{
	if ( sdl_keyb_init_done ) {
		sdl_keyb_init_done = FALSE;

		// release keyboard grab

	}

	if ( KeyAdditional != NULL ) {
		FREEMEM( KeyAdditional );
		KeyAdditional = NULL;
	}
}


// switch input focus to specified window -------------------------------------
//
void ISDL_SwitchInputFocus( void *win )
{
	// re-grab input
}


// init mouse handling code ---------------------------------------------------
//
void INPs_MouseInitHandler()
{
	// not necessary
}


// de-init mouse handling code ------------------------------------------------
//
void INPs_MouseKillHandler()
{
	// not necessary
}


// mouse variables ------------------------------------------------------------
//
static mousestate_s	cur_mouse_state;


// mouse motion handler invoked by event processing loop ----------------------
//
PRIVATE
void ISDLm_MouseEventHandler(const SDL_Event &event)
{
	if (event.type == SDL_MOUSEMOTION) {
		cur_mouse_state.xpos = (float) event.motion.x / Screen_Width;
		cur_mouse_state.ypos = (float) event.motion.y / Screen_Height;
	}

	if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
		int button = NUM_MOUSE_BUTTONS;
		
		switch (event.button.button) {
			case SDL_BUTTON_LEFT:
				button = MOUSE_BUTTON_LEFT;
				break;
			case SDL_BUTTON_MIDDLE:
				button = MOUSE_BUTTON_MIDDLE;
				break;
			case SDL_BUTTON_RIGHT:
				button = MOUSE_BUTTON_RIGHT;
				break;
		}

		if (button == NUM_MOUSE_BUTTONS)
			return; // we don't care about any other buttons
		
		if (event.button.state == SDL_PRESSED)
			cur_mouse_state.buttons[button] = MOUSE_BUTTON_PRESSED;
		else
			cur_mouse_state.buttons[button] = MOUSE_BUTTON_RELEASED;
	}
}


// set mouse coordinates independent of current screen resolution -------------
//
int INPs_MouseSetState( mousestate_s *state )
{
	ASSERT( state != NULL );

	int mposx = (int)( state->xpos * Screen_Width );
	int mposy = (int)( state->ypos * Screen_Height );

	// set new mouse position
#if SDL_VERSION_ATLEAST(2,0,0)
	SDL_WarpMouseInWindow(curwindow, mposx, mposy);
#else
	SDL_WarpMouse(mposx, mposy);
#endif

	return TRUE;
}


// get mouse coordinates independent of current screen resolution -------------
//
int INPs_MouseGetState( mousestate_s *state )
{
	ASSERT( state != NULL );

	if (cur_mouse_state.xpos < 0 || cur_mouse_state.xpos > 1.0)
		return FALSE;

	if (cur_mouse_state.ypos < 0 || cur_mouse_state.ypos > 1.0)
		return FALSE;

	// copy current state
	*state = cur_mouse_state;

	// we always draw the custom cursor if the
	// mouse is over a valid area
	state->drawcursor = TRUE;

	return TRUE;
}


// init joystick device -------------------------------------------------------
//
void INPs_JoyInitHandler()
{
#ifndef DISABLE_JOYSTICK_CODE

	ISDL_JoyInitHandler();

#endif
}


// close joystick device ------------------------------------------------------
//
void INPs_JoyKillHandler()
{
#ifndef DISABLE_JOYSTICK_CODE

	ISDL_JoyKillHandler();

#endif
}


// shoot gun if activated -----------------------------------------------------
//
int INPs_ActivateGun()
{
	return ISDL_ActivateGun();
}


// launch missile if activated ------------------------------------------------
//
int INPs_ActivateMissile()
{
	return ISDL_ActivateMissile();
}


// process additional input devices -------------------------------------------
//
void INPs_UserProcessAuxInput()
{
	ISDL_UserProcessAuxInput();
}


// display text line in log window --------------------------------------------
//
void ISDL_LogWinTextLine( char *line )
{
	// process pending expose events
}


// display text line in splash screen -----------------------------------------
//
void ISDL_SplashTextLine( char *line )
{
	ASSERT( line != NULL );

	//TODO:
}


// process pending x events ---------------------------------------------------
//
PRIVATE
void ISDLm_ProcessEvents()
{
	// TODO: process pending events from SDL
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				ISDLm_KeyboardHandler(event);
				break;
			case SDL_MOUSEMOTION:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				ISDLm_MouseEventHandler(event);
				break;
#if SDL_VERSION_ATLEAST(2,0,0)
			case SDL_TEXTINPUT:
				ISDLm_TextInputHandler(event);
				break;
#endif
			case SDL_QUIT:
				// clean up and quit ASAP
				ExitGameLoop = 3;
				break;
		}
	}
}


// called once per frame via SYSs_Yield() -------------------------------------
//
void INPs_Collect()
{
	// process events
	ISDLm_ProcessEvents();

#ifndef DISABLE_JOYSTICK_CODE

	// read the current joystick state
	ISDL_JoyCollect();

#endif

}
