/*
 * PARSEC HEADER: con_say.h
 */

#ifndef _CON_SAY_H_
#define _CON_SAY_H_


#define NUM_TEXT_MACROS			10
#define MAX_TEXTMACRO_LEN		127


// modes of talk display

enum enum_talk_modes {

	TALK_PUBLIC,	// name in < >
	TALK_PRIVATE,	// name in * *
	TALK_STATUS,	// no name in front of text

	// must be last in list!!
	NUM_TALK_MODES
};


// external variables

extern char text_macros[][ MAX_TEXTMACRO_LEN + 1 ];


// external functions

int		Cmd_SayText( char *cstr );
int		Cmd_SetTextMacro( char *cstr );

void	ShowSentText( const char *name, const char *text, int mode );


#endif


