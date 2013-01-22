/*
 * PARSEC HEADER: h_supp.h
 */

#ifndef _H_SUPP_H_
#define _H_SUPP_H_


// external functions

char*			CalcHudCharAddress( int no );
void			SetHudCharColor( int no );

void			ResetMessageArea();
void			MaintainMessages();
void			ShowMessage( const char *msg );

void			ShowTexfontMessage( texfontmsg_s *msg );

void			WriteFrameRate();
void 			WriteGameTime();
void 			WriteServerPing();
void			WriteExtraCollectMessage( char* pszText );



// external variables

extern int		Msg_ScreenTop;
extern int		cur_hud_char_color;
extern char*	Hud_Char_ColOfs[];


// number of different colors in hud font
#define NUM_HUD_CHARSET_COLORS	6


#endif // _H_SUPP_H_


