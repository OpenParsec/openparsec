/*
 * PARSEC HEADER: aud_bkgn.h
 */

#ifndef _AUD_BKGN_H_
#define _AUD_BKGN_H_


// external functions

void	AUD_BackGroundPlayer_Init( int voice );
void	AUD_BackGroundPlayer_Kill();
int		AUD_BackGroundPlayer_AddSample( const char *name );
int		AUD_BackGroundPlayer_RemoveSample( char *name );
int		AUD_BackGroundPlayer_AddTrack( int track );
int		AUD_BackGroundPlayer_RemoveTrack( int track );
int 	AUD_BackGroundPlayer_AddStream( const char *name );
int 	AUD_BackGroundPlayer_AddSilence( int seconds );
int		AUD_BackGroundPlayer_RemoveItem( int item );
int		AUD_BackGroundPlayer_Start( int fromevent );
int		AUD_BackGroundPlayer_Stop();
int		AUD_BackGroundPlayer_IsPlaying();
int		AUD_BackGroundPlayer_Maintain();
void	AUD_BackGroundPlayer_List();


#endif // _AUD_BKGN_H_


