/*
 * PARSEC HEADER: m_button.h
 */

#ifndef _M_BUTTON_H_
#define _M_BUTTON_H_


// menu item ids

enum {

	M_GAME,
	M_SERVER,
	M_CONFIG,
	M_QUIT,

	M_CONNECT,
	M_JOIN_GAME,
	M_STARMAP,
	M_PLAY_DEMO,
	M_DISCONNECT,

	M_CREATE,
	M_GAME_MOD,
	M_SETTINGS,

	M_SPACECRAFT,
	M_CONTROLS,
	M_OPTIONS,
	M_BACK,

	M_BUTTON_CAP
};


// button metrics
//
#define BUTTON_WIDTH			320
#define BUTTON_HEIGHT			79
#define BUTTON_ALPHA			255

#define BUTTON_SLIDE_SPEED		12

#define BUTTON_POS_LEFT			0
#define BUTTON_POS_RIGHT		( m_sintab_size - 1 )

#define BUTTON_PHASE_OFS		( m_sintab_size >> 3 )

#define BUTTON_ALPHA_LOW		0
#define BUTTON_ALPHA_MID		128
#define BUTTON_ALPHA_HIGH		255

#define BUTTON_FADE_SPEED		6
#define BUTTON_FADE_QUANTUM		10


// number of items comprising every menu --------------------------------------
//
#define NUM_MENU_ITEMS		5


// external functions

void	MenuButtonsEnterSubmenu();
void	MenuButtonsLeaveSubmenu();
int		MenuButtonsSelection();
int		MenuButtonsEscape();
void	MenuButtonsCursorUp();
void	MenuButtonsCursorDown();
void	MenuButtonsToggleConnect();
void	MenuButtonsToggleDisconnect();

int		MouseOverMenuButton( int mousex, int mousey );

void 	MoveInButtons();
void 	MoveOutButtons();
void 	SlideInButtons();
void 	SlideOutButtons();
int		SlideFinishedButtons();
void	FadeInButtons();
void	FadeOutButtons( int target );
int		FadeFinishedButtons();

void	DrawMenuButtons();
void	DrawMenuConnected();
void	DrawMenuLoading();


#endif // _M_BUTTON_H_


