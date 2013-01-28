/*
 * PARSEC HEADER: m_viewer.h
 */

#ifndef _M_VIEWER_H_
#define _M_VIEWER_H_


// possible active viewer modes

enum {

	SCVMODE_SPACECRAFT,
	SCVMODE_OBJECTSRING
};


#define NUM_SCV_OBJECTS		15


// external functions

int		MouseOverViewerItem( int mousex, int mousey );

void 	FadeInBackground();
void 	FadeOutBackground();
int 	FadeFinishedBackground();

void 	SlideInShip();
void 	SlideOutShip();
int		SlideFinishedShip();

void 	SlideInRing();
void 	SlideOutRing();
int 	SlideFinishedRing();

void 	DrawSpacecraftViewer();


#endif // _M_VIEWER_H_


