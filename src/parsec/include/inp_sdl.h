/*
 * PARSEC HEADER: inp_sdl.h
 */

#ifndef _INP_SDL_H_
#define _INP_SDL_H_


// il_sdl.c implements the following functions
// --------------------------------------------
//	void	INPs_InitGeneral();
//	void	INPs_KillGeneral();
//	void	INPs_KeybInitHandler();
//	void	INPs_KeybKillHandler();
//	void	INPs_JoyInitHandler();
//	void	INPs_JoyKillHandler();
//	int		INPs_ActivateGun();
//	int		INPs_ActivateMissile();
//	void	INPs_Collect();
//	void	INPs_UserProcessAuxInput();


#if defined(_WIN32) || defined(__APPLE__)
	#include <SDL.h>
#else
	#include <SDL/SDL.h>
#endif

#endif //_INP_SDL_H_



