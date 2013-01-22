/*
 * PARSEC HEADER: sys_msg_sv.h
 */

#ifndef _SYS_MSG_SV_H_
#define _SYS_MSG_SV_H_

// ----------------------------------------------------------------------------
// SYSTEM SUBSYSTEM (SYS) MESSAGE OUTPUT                                      -
// ----------------------------------------------------------------------------

int		SYSs_MsgOut				( const char *format, ... );
int		SYSs_MsgPut				( const char *format, ... );
int		SYS_WriteLogFileMessage	( const char *format, ... );

// wrappers

#define MSGOUT			SYSs_MsgOut
#define MSGPUT			SYSs_MsgPut

#define DBGOUT			SYSs_MsgOut
#define DMSGOUT			SYSs_MsgOut

#undef LOGOUT
#define LOGOUT(x)		SYS_WriteLogFileMessage x

#endif // _SYS_MSG_SV_H_


