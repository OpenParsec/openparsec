/*
 * PARSEC HEADER: sys_bind.h
 */

#ifndef _SYS_BIND_H_
#define _SYS_BIND_H_


// PROTOCOL bindings

enum {

	BT_PROTOCOL_PEERTOPEER,
	BT_PROTOCOL_GAMESERVER,
	BT_PROTOCOL_DEMONULL,

	BT_PROTOCOL_NUMTYPES
};



// external variables

extern int	sys_BindType_PROTOCOL;


// external functions

void	SYS_Bind_PROTOCOL();

void	SYS_BindDynamicFunctions();


#endif // _SYS_BIND_H_


