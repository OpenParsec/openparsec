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

#define SIZEOF_KeybBuffer		( sizeof( keybbuffer_s ) )
#define SIZEOF_KeyAdditional	( sizeof( keyaddctrl_s ) + sizeof( keyaddition_s ) * KEY_ADDITIONS_MAX )


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


// disabling signal handler code because it's not cross-platform and probably not necessary
/*
// table of signals to catch --------------------------------------------------
//
static int sighandlers_num[] = {

	SIGHUP,		SIGINT,
	SIGQUIT,	SIGILL,
	SIGTRAP,	SIGIOT,
	SIGBUS,		SIGFPE,
	SIGSEGV,	SIGPIPE,
	SIGALRM,	SIGTERM,
	SIGXCPU,	SIGXFSZ,
	SIGVTALRM,	SIGPROF,
#if !defined(__APPLE__)
	SIGPWR,
#endif
	SIGABRT,
};

#define NUM_SIGHANDLERS		CALC_NUM_ARRAY_ENTRIES( sighandlers_num )


// table of previously installed signal handlers ------------------------------
//
static struct sigaction sighandlers_old[ NUM_SIGHANDLERS ];


// signal handler to restore state on critical signal -------------------------
//
PRIVATE
void signal_handler( int signum )
{
	// ctrl-c emulates escape function
	if ( signum == SIGINT ) {
	    DepressedKeys->key_Escape = 1;
	    return;
	}

	// kill input handlers
	INPs_MouseKillHandler();
	INPs_KeybKillHandler();
	INPs_KillGeneral();

	// look up other signals; restore previous handler and
	// raise signal again if the signal is caught
	for ( unsigned int sid = 0; sid < NUM_SIGHANDLERS; sid++ ) {
		if ( sighandlers_num[ sid ] == signum ) {
			sigaction( signum, &sighandlers_old[ sid ], NULL );
			raise( signum );
			return;
		}
	}
}


// install critical signal handlers -------------------------------------------
//
PRIVATE
void ISDLm_InstallSignalHandlers()
{
	// install signal handler for all catched signals
	// and save old handlers for chaining later on
	struct sigaction sa;
	memset( &sa, 0, sizeof( sa ) );
	for ( unsigned int sid = 0; sid < NUM_SIGHANDLERS; sid++ ) {
		sa.sa_handler = signal_handler;
		sigaction( sighandlers_num[ sid ], &sa, &sighandlers_old[ sid ] );
	}
}
*/

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


// key press/release flag values ----------------------------------------------
//
#define KEY_EVENTPRESS		1
#define KEY_EVENTRELEASE	0



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


// keyboard handler invoked by event processing loop --------------------------
//
PRIVATE
void ISDLm_KeyboardHandler(const SDL_Event &event)
{
	keypress_s kinfo;

	kinfo.pressed = event.type == SDL_KEYDOWN; // true if pressed, false if released
	kinfo.key = event.key.keysym.sym;
	kinfo.unicode = event.key.keysym.unicode; // FIXME for SDL 2
	
	
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


	// console enabling/disabling
	if ( kinfo.key == MKC_TILDE ) {

		if ( !kinfo.pressed ) {

			// maintain debounce
			KeybFlags->ConTogReleased = -1;

		} else if ( KeybFlags->ConEnabled && KeybFlags->ConTogReleased ) {

			// toggle console
			KeybFlags->ConTogReleased = 0;
			KeybFlags->ConActive      = ~KeybFlags->ConActive;
		}
	}

	// console keyboard buffer handling
	if ( kinfo.pressed && ( kinfo.key != MKC_TILDE ) ) {

		if ( KeybFlags->ConActive && KeybFlags->ConEnabled ) {

			if (kinfo.key == MKC_LSHIFT || kinfo.key == MKC_RSHIFT) {
				return;
			}

			// set shift flag
			//scode |= KeybFlags->ShiftOn;
			
			CON_HandleInput(kinfo);

			// console grabs all input
			return;
		}
	}

	// functional key handling
	dword *tab1 =  (dword *) &KeyAssignments[ 0 ];
	dword *tab2 =  (dword *) &KeyAssignments[ 1 ];
	dword *tabd =  (dword *) DepressedKeys;

	for( int fid = NUM_GAMEFUNC_KEYS - 1; fid >= 0; fid-- ) {
		if ( ( tab1[ fid ] == kinfo.key ) || ( tab2[ fid ] == kinfo.key ) ) {
			tabd[ fid ] = kinfo.pressed;
		}
	}

	/*
	// set akc including shift flag (only if not a gray key)
	dword shiftb = ( ( KeybFlags->ShiftOn & ~KeybFlags->ExtOn ) != 0 ) ?
					AKC_SHIFT_FLAG : 0;
	*/
	dword akcode = (dword) kinfo.key;

	// additional key mappings
	keyaddition_s *kap = &KeyAdditional->table;
	ASSERT( KeyAdditional->size >= 0 );
	ASSERT( KeyAdditional->size <= KEY_ADDITIONS_MAX );

	for ( int aid = KeyAdditional->size - 1; aid >= 0; aid-- ) {
		if ( kap[ aid ].code == akcode ) {
			kap[ aid ].state = kinfo.pressed;
		}
	}

//	printf("\nISDLm_KeyboardHandler(): Press Scancode: 0x%x; SDLsym: %i\n", event.key.keysym.scancode, event.key.keysym.sym);

/*
	switch (event.type) {
		case SDL_KEYDOWN:

			for(int i=0; i< NUM_GAMEFUNC_KEYS; i++){
				if((event.key.keysym.scancode == KeyAssignments_b[i]) || (event.key.keysym.scancode == KeyAssignments2_b[i])){
					DepressedKeys_b[i] = 1;
				}
			}
			break;
		case SDL_KEYUP:
			//printf("\nISDLm_KeyboardHandler(): Release Scancode: 0x%x; SDLsym: %i\n", event.key.keysym.scancode, event.key.keysym.sym);
			for(int i=0; i< NUM_GAMEFUNC_KEYS; i++){
				if((event.key.keysym.scancode == KeyAssignments_b[i]) || (event.key.keysym.scancode == KeyAssignments2_b[i])){
					DepressedKeys_b[i] = 0;
				}
			}
			break;
	}*/
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
	KeyAdditional	= (keyaddctrl_s *) ALLOCMEM( SIZEOF_KeyAdditional );

	// init all structs
	memset( DepressedKeys,	0, sizeof( keyfunc_s ) );
	memset( KeyAdditional,	0, SIZEOF_KeyAdditional );
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
static int			last_frame_called = VISFRAME_NEVER;
static mousestate_s	last_mouse_state;
static unsigned int	mouse_buttons;


// mouse button handler invoked by event processing loop ----------------------
//
PRIVATE
void ILm_MouseButtonHandler()
{
	// not used
}


// mouse motion handler invoked by event processing loop ----------------------
//
PRIVATE
void ILm_MouseMotionHandler()
{
	// not used
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


// get normalized mouse coordinates and button state --------------------------
//
PRIVATE
int ISDLm_GetMouseState( mousestate_s *state )
{
	ASSERT( state != NULL );


	int mposx = 0;
	int mposy = 0;
	int mouse_buttons = 0;

	// query current mouse state (position and buttons)
	mouse_buttons = SDL_GetMouseState(&mposx, &mposy);

	// detect whether pointer not in window
	if ( mposx < 0 )
		return FALSE;
	if ( mposy < 0 )
		return FALSE;
	if ( mposx > Screen_Width )
		return FALSE;
	if ( mposy > Screen_Height )
		return FALSE;
/*
	state->xpos = mposx;
	state->ypos = mposy;

	*/

	state->xpos = (float) mposx / Screen_Width;
	state->ypos = (float) mposy / Screen_Height;


	// retrieve and set mouse button states
	int lbstate = ( ( mouse_buttons & SDL_BUTTON(1) ) != 0 );
	int mbstate = ( ( mouse_buttons & SDL_BUTTON(2) ) != 0 );
	int rbstate = ( ( mouse_buttons & SDL_BUTTON(3) ) != 0 );

	state->buttons[ MOUSE_BUTTON_LEFT ]   = lbstate ?
		MOUSE_BUTTON_PRESSED : MOUSE_BUTTON_RELEASED;
	state->buttons[ MOUSE_BUTTON_MIDDLE ] = mbstate ?
		MOUSE_BUTTON_PRESSED : MOUSE_BUTTON_RELEASED;
	state->buttons[ MOUSE_BUTTON_RIGHT ]  = rbstate ?
		MOUSE_BUTTON_PRESSED : MOUSE_BUTTON_RELEASED;

	return TRUE;
}


// get mouse coordinates independent of current screen resolution -------------
//
int INPs_MouseGetState( mousestate_s *state )
{
	ASSERT( state != NULL );

	// make idempotent for each frame
	if ( (dword)last_frame_called == CurVisibleFrame ) {
		*state = last_mouse_state;
		return TRUE;
	}
	last_frame_called = CurVisibleFrame;

	// retrieve normalized mouse coordinates and button state
	if ( !ISDLm_GetMouseState( state ) ) {
		return FALSE;
	}

	// we always draw the custom cursor if the
	// mouse is over a valid area
	state->drawcursor = TRUE;
/*
	// retrieve and set mouse button states
	int lbstate = ( ( mouse_buttons & Button1Mask ) != 0 );
	int mbstate = ( ( mouse_buttons & Button2Mask ) != 0 );
	int rbstate = ( ( mouse_buttons & Button3Mask ) != 0 );

	state->buttons[ MOUSE_BUTTON_LEFT ]   = lbstate ?
		MOUSE_BUTTON_PRESSED : MOUSE_BUTTON_RELEASED;
	state->buttons[ MOUSE_BUTTON_MIDDLE ] = mbstate ?
		MOUSE_BUTTON_PRESSED : MOUSE_BUTTON_RELEASED;
	state->buttons[ MOUSE_BUTTON_RIGHT ]  = rbstate ?
		MOUSE_BUTTON_PRESSED : MOUSE_BUTTON_RELEASED;
*/
	// remember for multiple get
	last_mouse_state = *state;

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
			case SDL_QUIT:
				// clean up and quit ASAP
				ExitGameLoop = 3;
				break;
//			default:
//				printf("ISDLm_ProcessEvents(): Event type: %i\n", event.type);
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
