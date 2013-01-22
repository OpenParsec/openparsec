/*
 * PARSEC HEADER: sys_msg.h
 */

#ifndef _SYS_MSG_H_
#define _SYS_MSG_H_


// ----------------------------------------------------------------------------
// SYSTEM SUBSYSTEM (SYS) MESSAGE OUTPUT                                      -
// ----------------------------------------------------------------------------

int		SYSs_MsgOut( const char *format, ... );
int		SYSs_MsgPut( const char *format, ... );
void	SYSs_MsgPrompt();

// wrappers

#define MSGOUT		SYSs_MsgOut 
#define MSGPUT		SYSs_MsgPut 

#define DBGOUT		SYSs_MsgOut 
#define DMSGOUT		SYSs_MsgOut 

#ifdef ENABLE_LOGOUT_COMMAND
	#include "utl_logfile.h"
	extern UTL_LogFile		g_Logfile;
	#define LOGOUT(x)		g_Logfile.printf x
#else // !ENABLE_LOGOUT_COMMAND

	#ifdef SYSTEM_COMPILER_MSVC
		#define LOGOUT(x)		
	#else // !SYSTEM_COMPILER_MSVC
		#define LOGOUT(x)			
	#endif // !SYSTEM_COMPILER_MSVC

#endif // !ENABLE_LOGOUT_COMMAND

#endif // _SYS_MSG_H_


