/*
 * PARSEC HEADER: vsdl_ogl.h
 */

#ifndef _VSDL_OGL_H_
#define _VSDL_OGL_H_

#if defined(_WIN32) || defined(__APPLE__)
	#include <SDL.h>
#else
	#include <SDL/SDL.h>
#endif

#include "platform.h"


#if SDL_VERSION_ATLEAST(2, 0, 0)

extern SDL_Window *		curwindow;
extern SDL_GLContext	curcontext;

#endif


// external functions


void	VSDL_CommitOGLBuff();

int		VSDL_InitOGLInterface( int printmodelistflags );
int		VSDL_InitOGLMode();

void	VSDL_CalcProjectiveMatrix();
void	VSDL_CalcOrthographicMatrix();

void	VSDL_ShutDownOGL();

void 	SDL_RCDisplayInfo();


#endif // _VSDL_OGL_H_


