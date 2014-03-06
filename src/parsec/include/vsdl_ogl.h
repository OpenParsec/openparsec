/*
 * PARSEC HEADER: vsdl_ogl.h
 */

#ifndef _VSDL_OGL_H_
#define _VSDL_OGL_H_

#ifdef SYSTEM_TARGET_LINUX
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif


extern SDL_Window *		curwindow;
extern SDL_GLContext	curcontext;


// external functions


void	VSDL_CommitOGLBuff();

int		VSDL_InitOGLInterface( int printmodelistflags );
int		VSDL_InitOGLMode();

void	VSDL_CalcProjectiveMatrix();
void	VSDL_CalcOrthographicMatrix();

void	VSDL_ShutDownOGL();

void 	SDL_RCDisplayInfo();


#endif // _VSDL_OGL_H_


