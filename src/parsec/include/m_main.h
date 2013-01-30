/*
 * PARSEC HEADER: m_main.h
 */

#ifndef _M_MAIN_H_
#define _M_MAIN_H_


// menu mouse states

enum {

	MOUSE_OVER_NOTHING		= 0x0000,

	MOUSE_OVER_ENTER_SIMUL	= 0x0100,
	MOUSE_OVER_CURUP_SIMUL	= 0x0200,
	MOUSE_OVER_CURDN_SIMUL	= 0x0400,
	MOUSE_OVER_CURLF_SIMUL	= 0x0800,
	MOUSE_OVER_CURRT_SIMUL	= 0x1000,

	MOUSE_OVER_BUTTON		= 0x0001 | MOUSE_OVER_ENTER_SIMUL,
	MOUSE_OVER_OPTION		= 0x0002 | MOUSE_OVER_ENTER_SIMUL,
	MOUSE_OVER_UPARROW		= 0x0003 | MOUSE_OVER_CURUP_SIMUL,
	MOUSE_OVER_DOWNARROW	= 0x0004 | MOUSE_OVER_CURDN_SIMUL,
	MOUSE_OVER_LEFTARROW	= 0x0005 | MOUSE_OVER_CURLF_SIMUL,
	MOUSE_OVER_RIGHTARROW	= 0x0006 | MOUSE_OVER_CURRT_SIMUL,
	MOUSE_OVER_SHIP			= 0x0007 | MOUSE_OVER_ENTER_SIMUL
};


// metrics of list window

struct list_window_metrics_s {

	int valid;

	int frame_l;
	int frame_r;
	int frame_t;
	int frame_b;
	int chwidth;
	int chheight;
	int text_x;
	int text_y;
	int maxcontwidth;
	int maxcontheight;
};


// external functions

void	DrawFloatingMenu();
void	FloatingMenuKeyHandler();

void	MenuNotifyConnect();
void	MenuNotifyDisconnect();

void	ActivateFloatingMenu( int statuswin );

void 	SlideInFloatingMenu();
void 	SlideOutFloatingMenu();
void	MoveInFloatingMenu();
void	MoveOutFloatingMenu();
void 	HideFloatingMenu();

void	ExitOptionsMenu();

// externally callable functions for automation -------------------------------
//
void	MENU_FloatingMenuEnterGame();




// external variables

extern int m_sintab[];
extern int m_sintab_size;
extern int m_sintab_ampl;
extern int disable_join_game;


#endif // _M_MAIN_H_


