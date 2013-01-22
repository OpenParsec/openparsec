/*
 * PARSEC HEADER: vid_init.h
 */

#ifndef _VID_INIT_H_
#define _VID_INIT_H_


// external functions

char *	FetchCtrlFileName();
void	LoadResolutionData();
void	LoadDetailData();

void	VID_MapSpecifierToMode( char * modespec, int * modearray );

void	VID_SetupDependentData();
int		VID_MapBppToOpt( int bpp );
int		VID_MapOptToBpp( int bppopt );
void	VID_ApplyOptions();

void	VID_SwitchMode( int xres, int yres, int bpp );
void	VID_InitMode();

void	VID_InitSubsystem();


#endif // _VID_INIT_H_


