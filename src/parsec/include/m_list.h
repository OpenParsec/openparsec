/*
 * PARSEC HEADER: m_list.h
 */

#ifndef _M_LIST_H_
#define _M_LIST_H_


// list window modes

enum {

	LISTWIN_DEMOLIST,
	LISTWIN_PLAYERLIST,
	LISTWIN_SERVERLIST,
};


// external functions

int		DemoListSelection();
void	DemoListCursorUp();
void	DemoListCursorDown();
void	DemoListMouseSet();

int		MouseOverListItem( int mousex, int mousey );

void 	MoveInListWindow();
void 	MoveOutListWindow();
void 	SlideInListWindow();
void 	SlideOutListWindow();
int 	SlideFinishedListWindow();

void 	DrawRemotePlayerListWindow();
void	DrawMenuListWindow();


// external variables

extern int listwin_mode;
extern int listwin_active;


#endif // _M_LIST_H_


