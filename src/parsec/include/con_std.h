/*
 * PARSEC HEADER: con_std.h
 */

#ifndef _CON_STD_H_
#define _CON_STD_H_


// standard commands table entry ----------------------------------------------
//
struct std_command_s {

	const char*	command;	// command string
	short	commlen;		// length of string
	short	numparams;		// number of parameters (used by tab-completion)
};


// enumeration of standard commands -------------------------------------------
//
enum enum_std_commands {

	CM_APPEND,
	CM_BASE,
	CM_BIND,
	CM_BUILD,
	CM_CAT,
	CM_CLASSINFO,
	CM_CLEAR,
	CM_CLOSE,
	CM_CLS,
	CM_COLANIM,
	CM_COMPILE,
	CM_CONNECT,
	CM_CREATE,
	CM_DEC,
	CM_DISABLE,
	CM_NOMACROS,
	CM_DISCONNECT,
	CM_DISPLAY,
	CM_ENABLE,
	CM_ENABLEMACROS,
	CM_EXIT,
	CM_EXITGAME,
	CM_FACEINFO,
	CM_FLASHBLUE,
	CM_FLASHWHITE,
	CM_GAMELOOPBATCH,
	CM_HEX,
	CM_HIDE,
	CM_KEY,
	CM_LISTBINDINGS,
	CM_LISTCLASSES,
	CM_LISTSTDCOMMANDS,
	CM_LISTDATA,
	CM_LISTDEMOS,
	CM_LISTGAMEFUNCKEYS,
	CM_LISTINTCOMMANDS,
	CM_LISTTEXTMACS,
	CM_LISTTYPES,
	CM_LOAD,
	CM_LOGIN,
	CM_LOGOUT,
	CM_LISTEXTCOMMS,
	CM_NAME,
	CM_NET_SUBSYS,
	CM_OBJECTCLEAR,
	CM_OBJECTCOUNT,
	CM_OBJECTINFO,
	CM_OCT,
	CM_OPEN,
	CM_PATH_ANIM_INTRO,
	CM_PATH_ASTREAM_INTRO,
	CM_PATH_ASTREAM_MENU,
	CM_PATH_ASTREAM,
	CM_PLAYDEMO,
	CM_PROP,
	CM_PROPC,
	CM_PROPO,
	CM_QUIT,
	CM_RECDEM,
	CM_RECORD,
	CM_REPINFO,
	CM_REPSTOP,
	CM_RESCAN,
	CM_SAY,
	CM_SET_MACRO,
	CM_SETORIGIN,
	CM_DEFSHADER,
	CM_SETSHADER,
	CM_SHIPS,
	CM_SHOWPOSITION,
	CM_TALKMODE,
	CM_TYPEINFO,
	CM_VID_LISTMODES,
	CM_VID_SETMODE,
	CM_VID_SUBSYS,
	CM_WORKINGDIRECTORY,
	CM_WRITESTRING,
	CM_AUD_CONF,
	CM_INP_CONF,

	// must be last in list!!
	NUM_STD_COMMAND_CONSTANTS
};


// command list and length array
extern std_command_s std_commands[];

// number of standard commands
extern int			num_std_commands;


// macro to retrieve standard command strings
#define CMSTR(x)	(std_commands[x].command)

// macro to retrieve string length of standard commmands
#define CMLEN(x)	(std_commands[x].commlen)

// macro to determine number of arguments of standard commmands
#define CMARG(x)	(std_commands[x].numparams)


#endif // _CON_STD_H_


