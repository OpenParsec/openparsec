/*
 * keys_sdl.h
 *
 *  Created on: Nov 3, 2012
 *      Author: jasonw
 */

#ifndef KEYS_SDL_H_
#define KEYS_SDL_H_

#ifdef SYSTEM_TARGET_LINUX
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

// ----------------------------------------------------------------------------
//  SDL KEYBOARD KEYS                                                         -
// ----------------------------------------------------------------------------



// make codes for general keyboard handling -----------------------------------
//
enum {

	MKC_MAKE_CODE	=	0x00,
	MKC_BASE_MASK	=	0x00,

	MKC_EXT_MDIFR	=	0x00,
	MKC_PAUSE_MDIFR	=	SDLK_PAUSE,

	MKC_EXT_FLAG	=	0x00,
	MKC_SHIFT_FLAG	=	0x10000,

	MKC_NIL			=	0x00,

	MKC_ESCAPE		=	SDLK_ESCAPE,
	MKC_SPACE		=	SDLK_SPACE,
	MKC_ENTER 		=	SDLK_RETURN,
	MKC_CURSORUP	=	SDLK_UP,
	MKC_CURSORDOWN  =	SDLK_DOWN,
	MKC_CURSORLEFT  =	SDLK_LEFT,
	MKC_CURSORRIGHT =	SDLK_RIGHT,
	MKC_BACKSPACE   =	SDLK_BACKSPACE,
	MKC_INSERT		=	SDLK_INSERT,
	MKC_DELETE      =	SDLK_DELETE,
	MKC_HOME  		=	SDLK_HOME,
	MKC_END         =	SDLK_END,
	MKC_PAGEUP		=	SDLK_PAGEUP,
	MKC_PAGEDOWN    =	SDLK_PAGEDOWN,
	MKC_CAPSLOCK	=	SDLK_CAPSLOCK,
	MKC_TILDE		=	SDLK_BACKQUOTE,					// U.S.: `~		german: ^Ã¸

	MKC_MINUS		=	SDLK_MINUS,					// U.S.: -_		german: Ã¡?bsl
	MKC_EQUALS		=	SDLK_EQUALS,					// U.S.: =+		german: Ã¯`
	MKC_TAB			=	SDLK_TAB,
	MKC_LBRACKET	=	SDLK_LEFTBRACKET,					// U.S.: [{		german: ï¿½Å¡
	MKC_RBRACKET	=	SDLK_RIGHTBRACKET,					// U.S.: ]}		german: +*~
	MKC_SEMICOLON	=	SDLK_SEMICOLON,					// U.S.: ;:		german: â€�â„¢
	MKC_APOSTROPHE	=	SDLK_QUOTE,					// U.S.: '"		german: â€žÅ½
	MKC_GRAVE		=	SDLK_BACKQUOTE,					// U.S.: `~		german: ^Ã¸

	MKC_LSHIFT		=	SDLK_LSHIFT,
	MKC_RSHIFT		=	SDLK_RSHIFT,
	MKC_LCONTROL	=	SDLK_LCTRL,
	MKC_RCONTROL	=	SDLK_RCTRL,
	MKC_LALT		=	SDLK_LALT,
	MKC_RALT		=	SDLK_RALT,

	MKC_COMMA		=	SDLK_COMMA,					// U.S.: ,<		german: ,;
	MKC_PERIOD		=	SDLK_PERIOD,					// U.S.: .>		german: .:
	MKC_SLASH		=	SDLK_PERIOD,					// U.S.: /?		german: -_

	MKC_BACKSLASH	=	SDLK_BACKSLASH,					// U.S.: bsl|	german: #'
	MKC_GERARROWS	=	0x00,					//				german: <>|

	MKC_NUMPADSLASH	=	SDLK_KP_DIVIDE,	// gray divide
	MKC_NUMPADSTAR	=	SDLK_KP_MULTIPLY,					// gray multiply
	MKC_NUMPADMINUS	=	SDLK_KP_MINUS,					// gray subtract
	MKC_NUMPADPLUS	=	SDLK_KP_PLUS,					// gray add
	MKC_NUMPADPERIOD=	SDLK_KP_PERIOD,				// gray decimal (period/comma)

	MKC_NUMLOCK		=	SDLK_NUMLOCKCLEAR,
	MKC_SCROLL		=	SDLK_SCROLLLOCK,
	
	MKC_NUMPAD0		=	SDLK_KP_0,
	MKC_NUMPAD1		=	SDLK_KP_1,
	MKC_NUMPAD2		=	SDLK_KP_2,
	MKC_NUMPAD3		=	SDLK_KP_3,
	MKC_NUMPAD4		=	SDLK_KP_4,
	MKC_NUMPAD5		=	SDLK_KP_5,
	MKC_NUMPAD6		=	SDLK_KP_6,
	MKC_NUMPAD7		=	SDLK_KP_7,
	MKC_NUMPAD8		=	SDLK_KP_8,
	MKC_NUMPAD9		=	SDLK_KP_9,

	MKC_LWIN		=	SDLK_LGUI,	// left windows/cmd key
	MKC_RWIN		=	SDLK_RGUI,	// right windows/cmd key

	MKC_APPS		=	SDLK_MODE,	// windows apps window key

	MKC_ENTER_GRAY 		=	MKC_ENTER		| MKC_EXT_FLAG,
	MKC_CURSORUP_GRAY	=	MKC_CURSORUP	| MKC_EXT_FLAG,
	MKC_CURSORDOWN_GRAY	=	MKC_CURSORDOWN	| MKC_EXT_FLAG,
	MKC_CURSORLEFT_GRAY	=	MKC_CURSORLEFT	| MKC_EXT_FLAG,
	MKC_CURSORRIGHT_GRAY=	MKC_CURSORRIGHT	| MKC_EXT_FLAG,
	MKC_BACKSPACE_GRAY	=	MKC_BACKSPACE	| MKC_EXT_FLAG,
	MKC_INSERT_GRAY		=	MKC_INSERT		| MKC_EXT_FLAG,
	MKC_DELETE_GRAY		=	MKC_DELETE		| MKC_EXT_FLAG,
	MKC_HOME_GRAY  		=	MKC_HOME		| MKC_EXT_FLAG,
	MKC_END_GRAY		=	MKC_END			| MKC_EXT_FLAG,
	MKC_PAGEUP_GRAY		=	MKC_PAGEUP		| MKC_EXT_FLAG,
	MKC_PAGEDOWN_GRAY	=	MKC_PAGEDOWN	| MKC_EXT_FLAG,

	MKC_A			=	SDLK_a,
	MKC_B			=	SDLK_b,
	MKC_C			=	SDLK_c,
	MKC_D			=	SDLK_d,
	MKC_E			=	SDLK_e,
	MKC_F			=	SDLK_f,
	MKC_G			=	SDLK_g,
	MKC_H			=	SDLK_h,
	MKC_I			=	SDLK_i,
	MKC_J			=	SDLK_j,
	MKC_K			=	SDLK_k,
	MKC_L			=	SDLK_l,
	MKC_M			=	SDLK_m,
	MKC_N			=	SDLK_n,
	MKC_O			=	SDLK_o,
	MKC_P			=	SDLK_p,
	MKC_Q			=	SDLK_q,
	MKC_R			=	SDLK_r,
	MKC_S			=	SDLK_s,
	MKC_T			=	SDLK_t,
	MKC_U			=	SDLK_u,
	MKC_V			=	SDLK_v,
	MKC_W			=	SDLK_w,
	MKC_X			=	SDLK_x,
	MKC_Y			=	SDLK_y,
	MKC_Z			=	SDLK_z,

	MKC_0			=	SDLK_0,
	MKC_1			=	SDLK_1,
	MKC_2			=	SDLK_2,
	MKC_3			=	SDLK_3,
	MKC_4			=	SDLK_4,
	MKC_5			=	SDLK_5,
	MKC_6			=	SDLK_6,
	MKC_7			=	SDLK_7,
	MKC_8			=	SDLK_8,
	MKC_9			=	SDLK_9,

	MKC_F1			=	SDLK_F1,
	MKC_F2			=	SDLK_F2,
	MKC_F3			=	SDLK_F3,
	MKC_F4			=	SDLK_F4,
	MKC_F5			=	SDLK_F5,
	MKC_F6			=	SDLK_F6,
	MKC_F7			=	SDLK_F7,
	MKC_F8			=	SDLK_F8,
	MKC_F9			=	SDLK_F9,
	MKC_F10 		=	SDLK_F10,
	MKC_F11 		=	SDLK_F11,
	MKC_F12			=	SDLK_F12
};



// key codes for console's keyboard buffer (only special codes/control keys) --
//
enum {

	CKC_NIL					= 0x000000,
	CKC_ESCAPE				= SDLK_ESCAPE,
	CKC_SPACE				= SDLK_SPACE,
	CKC_ENTER				= SDLK_RETURN,
	CKC_CURSORUP			= SDLK_UP,
	CKC_CURSORDOWN			= SDLK_DOWN,
	CKC_CURSORLEFT			= SDLK_LEFT,
	CKC_CURSORRIGHT			= SDLK_RIGHT,
	CKC_BACKSPACE			= SDLK_BACKSPACE,
	CKC_INSERT				= SDLK_INSERT,
	CKC_DELETE				= SDLK_DELETE,
	CKC_HOME				= SDLK_HOME,
	CKC_END					= SDLK_END,
	CKC_PAGEUP				= SDLK_PAGEUP,
	CKC_PAGEDOWN			= SDLK_PAGEDOWN,
	CKC_TAB					= SDLK_TAB,
	CKC_CURSORUP_SHIFTED	= SDLK_UP,
	CKC_CURSORDOWN_SHIFTED	= SDLK_DOWN,
	CKC_CURSORLEFT_SHIFTED	= SDLK_LEFT,
	CKC_CURSORRIGHT_SHIFTED	= SDLK_RIGHT,

	CKC_CTRL_MASK			= 0x800000
};



// key codes for flexible keyboard configuration (assignable keys) ------------
//
enum {

	AKC_EXT_FLAG	=	0x00000080,
	AKC_JOY_FLAG	=	0x10000000,	// joystick buttons, no actual keyboard keys
	AKC_SHIFT_FLAG	=	0x00000000,

	AKC_ESCAPE		=	SDLK_ESCAPE,
	AKC_SPACE		=	SDLK_SPACE,
	AKC_ENTER 		=	SDLK_RETURN,
	AKC_CURSORUP	=	SDLK_UP,
	AKC_CURSORDOWN  =	SDLK_DOWN,
	AKC_CURSORLEFT  =	SDLK_LEFT,
	AKC_CURSORRIGHT =	SDLK_RIGHT,
	AKC_BACKSPACE   =	SDLK_BACKSPACE,
	AKC_INSERT		=	SDLK_INSERT,
	AKC_DELETE      =	SDLK_DELETE,
	AKC_HOME  		=	SDLK_HOME,
	AKC_END         =	SDLK_END,
	AKC_PAGEUP		=	SDLK_PAGEUP,
	AKC_PAGEDOWN    =	SDLK_PAGEDOWN,
    AKC_CAPSLOCK	=	SDLK_CAPSLOCK,
	AKC_TILDE		=	SDLK_BACKQUOTE,

	AKC_MINUS		=	SDLK_MINUS,
	AKC_EQUALS		=	SDLK_EQUALS,
	AKC_TAB			=	SDLK_TAB,
	AKC_LBRACKET	=	SDLK_LEFTBRACKET,
	AKC_RBRACKET	=	SDLK_RIGHTBRACKET,
	AKC_SEMICOLON	=	SDLK_SEMICOLON,
	AKC_APOSTROPHE	=	SDLK_QUOTE,
	AKC_GRAVE		=	SDLK_BACKQUOTE,

	AKC_LSHIFT		=	SDLK_LSHIFT,
	AKC_RSHIFT		=	SDLK_RSHIFT,
	AKC_LCONTROL	=	SDLK_LCTRL,
	AKC_RCONTROL	=	SDLK_RCTRL,
	AKC_LALT		=	SDLK_LALT,
	AKC_RALT		=	SDLK_RALT,

	AKC_COMMA		=	SDLK_COMMA,
	AKC_PERIOD		=	SDLK_PERIOD,
	AKC_SLASH		=	SDLK_SLASH,

	AKC_BACKSLASH	=	SDLK_BACKSLASH,
	AKC_GERARROWS	=	0X000000,

	AKC_NUMPADSLASH	=	SDLK_KP_DIVIDE,
	AKC_NUMPADSTAR	=	SDLK_KP_MULTIPLY,
	AKC_NUMPADMINUS	=	SDLK_KP_MINUS,
	AKC_NUMPADPLUS	=	SDLK_KP_PLUS,
	AKC_NUMPADPERIOD=	SDLK_KP_PERIOD,

	AKC_NUMLOCK		=	SDLK_NUMLOCKCLEAR,
	AKC_SCROLL		=	SDLK_SCROLLLOCK,

	AKC_NUMPAD0		=	SDLK_KP_0,
	AKC_NUMPAD1		=	SDLK_KP_1,
	AKC_NUMPAD2		=	SDLK_KP_2,
	AKC_NUMPAD3		=	SDLK_KP_3,
	AKC_NUMPAD4		=	SDLK_KP_4,
	AKC_NUMPAD5		=	SDLK_KP_5,
	AKC_NUMPAD6		=	SDLK_KP_6,
	AKC_NUMPAD7		=	SDLK_KP_7,
	AKC_NUMPAD8		=	SDLK_KP_8,
	AKC_NUMPAD9		=	SDLK_KP_9,

	AKC_LWIN		=	SDLK_LGUI,	// left windows/cmd key
	AKC_RWIN		=	SDLK_RGUI,	// right windows/cmd key

	AKC_APPS		=	SDLK_MODE,

	AKC_ENTER_GRAY 		=	AKC_ENTER		| AKC_EXT_FLAG,
	AKC_CURSORUP_GRAY	=	AKC_CURSORUP	| AKC_EXT_FLAG,
	AKC_CURSORDOWN_GRAY	=	AKC_CURSORDOWN	| AKC_EXT_FLAG,
	AKC_CURSORLEFT_GRAY	=	AKC_CURSORLEFT	| AKC_EXT_FLAG,
	AKC_CURSORRIGHT_GRAY=	AKC_CURSORRIGHT	| AKC_EXT_FLAG,
	AKC_BACKSPACE_GRAY	=	AKC_BACKSPACE	| AKC_EXT_FLAG,
	AKC_INSERT_GRAY		=	AKC_INSERT		| AKC_EXT_FLAG,
	AKC_DELETE_GRAY		=	AKC_DELETE		| AKC_EXT_FLAG,
	AKC_HOME_GRAY  		=	AKC_HOME		| AKC_EXT_FLAG,
	AKC_END_GRAY		=	AKC_END			| AKC_EXT_FLAG,
	AKC_PAGEUP_GRAY		=	AKC_PAGEUP		| AKC_EXT_FLAG,
	AKC_PAGEDOWN_GRAY	=	AKC_PAGEDOWN	| AKC_EXT_FLAG,

	AKC_A			=	SDLK_a,
	AKC_B			=	SDLK_b,
	AKC_C			=	SDLK_c,
	AKC_D			=	SDLK_d,
	AKC_E			=	SDLK_e,
	AKC_F			=	SDLK_f,
	AKC_G			=	SDLK_g,
	AKC_H			=	SDLK_h,
	AKC_I			=	SDLK_i,
	AKC_J			=	SDLK_j,
	AKC_K			=	SDLK_k,
	AKC_L			=	SDLK_l,
	AKC_M			=	SDLK_m,
	AKC_N			=	SDLK_n,
	AKC_O			=	SDLK_o,
	AKC_P			=	SDLK_p,
	AKC_Q			=	SDLK_q,
	AKC_R			=	SDLK_r,
	AKC_S			=	SDLK_s,
	AKC_T			=	SDLK_t,
	AKC_U			=	SDLK_u,
	AKC_V			=	SDLK_v,
	AKC_W			=	SDLK_w,
	AKC_X			=	SDLK_x,
	AKC_Y			=	SDLK_y,
	AKC_Z			=	SDLK_z,

	AKC_A_SHIFTED	=	AKC_A | AKC_SHIFT_FLAG,
	AKC_B_SHIFTED	=	AKC_B | AKC_SHIFT_FLAG,
	AKC_C_SHIFTED	=	AKC_C | AKC_SHIFT_FLAG,
	AKC_D_SHIFTED	=	AKC_D | AKC_SHIFT_FLAG,
	AKC_E_SHIFTED	=	AKC_E | AKC_SHIFT_FLAG,
	AKC_F_SHIFTED	=	AKC_F | AKC_SHIFT_FLAG,
	AKC_G_SHIFTED	=	AKC_G | AKC_SHIFT_FLAG,
	AKC_H_SHIFTED	=	AKC_H | AKC_SHIFT_FLAG,
	AKC_I_SHIFTED	=	AKC_I | AKC_SHIFT_FLAG,
	AKC_J_SHIFTED	=	AKC_J | AKC_SHIFT_FLAG,
	AKC_K_SHIFTED	=	AKC_K | AKC_SHIFT_FLAG,
	AKC_L_SHIFTED	=	AKC_L | AKC_SHIFT_FLAG,
	AKC_M_SHIFTED	=	AKC_M | AKC_SHIFT_FLAG,
	AKC_N_SHIFTED	=	AKC_N | AKC_SHIFT_FLAG,
	AKC_O_SHIFTED	=	AKC_O | AKC_SHIFT_FLAG,
	AKC_P_SHIFTED	=	AKC_P | AKC_SHIFT_FLAG,
	AKC_Q_SHIFTED	=	AKC_Q | AKC_SHIFT_FLAG,
	AKC_R_SHIFTED	=	AKC_R | AKC_SHIFT_FLAG,
	AKC_S_SHIFTED	=	AKC_S | AKC_SHIFT_FLAG,
	AKC_T_SHIFTED	=	AKC_T | AKC_SHIFT_FLAG,
	AKC_U_SHIFTED	=	AKC_U | AKC_SHIFT_FLAG,
	AKC_V_SHIFTED	=	AKC_V | AKC_SHIFT_FLAG,
	AKC_W_SHIFTED	=	AKC_W | AKC_SHIFT_FLAG,
	AKC_X_SHIFTED	=	AKC_X | AKC_SHIFT_FLAG,
	AKC_Y_SHIFTED	=	AKC_Y | AKC_SHIFT_FLAG,
	AKC_Z_SHIFTED	=	AKC_Z | AKC_SHIFT_FLAG,

    AKC_0			=	SDLK_0,
    AKC_1			=	SDLK_1,
    AKC_2			=	SDLK_2,
    AKC_3			=	SDLK_3,
    AKC_4			=	SDLK_4,
    AKC_5			=	SDLK_5,
    AKC_6			=	SDLK_6,
    AKC_7			=	SDLK_7,
    AKC_8			=	SDLK_8,
    AKC_9			=	SDLK_9,

    AKC_0_SHIFTED	=	SDLK_0 | MKC_SHIFT_FLAG,
    AKC_1_SHIFTED	=	SDLK_1 | MKC_SHIFT_FLAG,
    AKC_2_SHIFTED	=	SDLK_2 | MKC_SHIFT_FLAG,
    AKC_3_SHIFTED	=	SDLK_3 | MKC_SHIFT_FLAG,
    AKC_4_SHIFTED	=	SDLK_4 | MKC_SHIFT_FLAG,
    AKC_5_SHIFTED	=	SDLK_5 | MKC_SHIFT_FLAG,
    AKC_6_SHIFTED	=	SDLK_6 | MKC_SHIFT_FLAG,
    AKC_7_SHIFTED	=	SDLK_7 | MKC_SHIFT_FLAG,
    AKC_8_SHIFTED	=	SDLK_8 | MKC_SHIFT_FLAG,
    AKC_9_SHIFTED	=	SDLK_9 | MKC_SHIFT_FLAG,

	AKC_F1			=	SDLK_F1,
	AKC_F2			=	SDLK_F2,
	AKC_F3			=	SDLK_F3,
	AKC_F4			=	SDLK_F4,
	AKC_F5			=	SDLK_F5,
	AKC_F6			=	SDLK_F6,
	AKC_F7			=	SDLK_F7,
	AKC_F8			=	SDLK_F8,
	AKC_F9			=	SDLK_F9,
	AKC_F10 		=	SDLK_F10,
	AKC_F11 		=	SDLK_F11,
	AKC_F12			=	SDLK_F12,

	AKC_F1_SHIFTED	=	AKC_F1  | AKC_SHIFT_FLAG,
	AKC_F2_SHIFTED	=	AKC_F2  | AKC_SHIFT_FLAG,
	AKC_F3_SHIFTED	=	AKC_F3  | AKC_SHIFT_FLAG,
	AKC_F4_SHIFTED	=	AKC_F4  | AKC_SHIFT_FLAG,
	AKC_F5_SHIFTED	=	AKC_F5  | AKC_SHIFT_FLAG,
	AKC_F6_SHIFTED	=	AKC_F6  | AKC_SHIFT_FLAG,
	AKC_F7_SHIFTED	=	AKC_F7  | AKC_SHIFT_FLAG,
	AKC_F8_SHIFTED	=	AKC_F8  | AKC_SHIFT_FLAG,
	AKC_F9_SHIFTED	=	AKC_F9  | AKC_SHIFT_FLAG,
	AKC_F10_SHIFTED =	AKC_F10 | AKC_SHIFT_FLAG,
	AKC_F11_SHIFTED =	AKC_F11 | AKC_SHIFT_FLAG,
	AKC_F12_SHIFTED	=	AKC_F12 | AKC_SHIFT_FLAG,

	// joystick buttons, no actual keyboard keys
	AKC_JOY_BUTTON1 =   0x00000000 | AKC_JOY_FLAG,
	AKC_JOY_BUTTON2 =   0x00000001 | AKC_JOY_FLAG,
	AKC_JOY_BUTTON3 =   0x00000002 | AKC_JOY_FLAG,
	AKC_JOY_BUTTON4 =   0x00000003 | AKC_JOY_FLAG,
	AKC_JOY_BUTTON5 =   0x00000004 | AKC_JOY_FLAG,
	AKC_JOY_BUTTON6 =   0x00000005 | AKC_JOY_FLAG,
	AKC_JOY_BUTTON7 =   0x00000006 | AKC_JOY_FLAG,
	AKC_JOY_BUTTON8 =   0x00000007 | AKC_JOY_FLAG
};


#endif /* KEYS_SDL_H_ */
