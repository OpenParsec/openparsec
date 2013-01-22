/*
 * PARSEC HEADER: vid_subh.h
 */

#ifndef _VID_SUBH_H_
#define _VID_SUBH_H_


// ----------------------------------------------------------------------------
// VIDEO SUBSYSTEM (VID) system-dependent function declarations               -
// ----------------------------------------------------------------------------


// SUPP group

int		VIDs_FadeScreenFromBlack( int palno );
int		VIDs_FadeScreenToBlack( int palno, int fademusic );
int		VIDs_FadePaletteToBlack( char *palette, int fademusic );
int		VIDs_FadeScreenFromWhite( int palno );
int		VIDs_SetScreenToColor( colrgba_s col );
int		VIDs_SetScreenDazzle( float strength );
int		VIDs_SetColIndexZero( int color );
int		VIDs_SetGammaCorrection( float gamma );

// BUFF group

void	VIDs_ClearRenderBuffer();
void	VIDs_CommitRenderBuffer();
char*	VIDs_ScreenshotBuffer( int create, int *size );

// INIT group

void	VIDs_InitDisplay();
void	VIDs_RestoreDisplay();
void	VIDs_CalcProjectiveMatrix();
void	VIDs_CalcOrthographicMatrix();
void	VIDs_InitFrameBufferAPI();


#endif // _VID_SUBH_H_


