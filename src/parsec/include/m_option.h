/*
 * PARSEC HEADER: m_option.h
 */

#ifndef _M_OPTION_H_
#define _M_OPTION_H_

extern bool mod_player_name;

// options menu item ids

enum {

	RES_OPT,
	DEPTH_OPT,
	FULL_OPT,
	SYNC_OPT,
	APPLY_VID_OPT,
	TEXDETAIL_OPT,
	GEOMDETAIL_OPT,
	SFXDETAIL_OPT,
	APPLY_DETAIL_OPT,
	PROTOCOL_OPT,
	KILL_LIMIT_OPT,
	SOLAR_OPT,
	APPLY_NET_OPT,
	SOUND_OPT,
	CTRL_OPT,
	INVERT_OPT,
	SENSITIVITY_OPT,
	CENTER_OPT,
//	APPLY_MOUSE_OPT,
	NAME_OPT,
	EXIT_OPT,

	NUM_OPTIONS
};




// external functions

int		OptionsListSelection();
void	OptionsListSelectDefault();
void	OptionsListCursorUp();
void	OptionsListCursorDown();
void	OptionsKeyPressed(int key);
void    OptionsProcessTextInput(char character);

int		MouseOverOption( int mousex, int mousey );

void 	MoveInOptions();
void 	MoveOutOptions();
void 	SlideInOptions();
void 	SlideOutOptions();
int 	SlideFinishedOptions();

void 	InitOptionsSubsystemInfo();
void	InitOptionsWindow();

void 	ExecOptionSelection( int optno );
void 	DrawOptionsMenuWindow();


#endif // _M_OPTION_H_


