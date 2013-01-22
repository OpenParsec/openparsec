/*
 * PARSEC HEADER: sys_defs.h
 */

#ifndef _SYS_DEFS_H_
#define _SYS_DEFS_H_


// ----------------------------------------------------------------------------
// SYSTEM SUBSYSTEM (SYS) related definitions                                 -
// ----------------------------------------------------------------------------

#ifdef PARSEC_CLIENT

	// external variables

	extern volatile int	FrameRate;
	extern volatile int	FrameCounter;
	extern volatile int	RefTimeCount;
	extern volatile int	RefFrameCount;


	// include system-specific subsystem prototypes ---------------------------
	//
	#include "sys_subh.h"

#else // !PARSEC_CLIENT

	#include "sys_refframe_sv.h"
	#include "sys_util_sv.h"
	//#include "sys_msg_sv.h"

#endif // !PARSEC_CLIENT


#endif // _SYS_DEFS_H_


