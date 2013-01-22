/*
 * PARSEC HEADER: h_global.h
 */

#ifndef _H_GLOBAL_H_
#define _H_GLOBAL_H_


// global externals (GAME CODE/HUD) -----------------------


// message maintenance

struct msgmaint_s {

	int 			lifetimecount;
	char			message[ MAX_MESSAGELEN + 1 ];
};

extern msgmaint_s		MsgMaint[];


// texfont messages

struct texfontmsg_s {

	char*			message;
	IterTexfont*	itexfont;

	short			msgtype;
	word			flags;

	short			maxpulses;
	short			refalpha;
	float			decalpha;

	int 			lifetime;
	int				starttime;
	float			curalpha;
};

extern texfontmsg_s		TexfontMsgs[];


#endif // _H_GLOBAL_H_


