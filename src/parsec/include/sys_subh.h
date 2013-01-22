/*
 * PARSEC HEADER: sys_subh.h
 */

#ifndef _SYS_SUBH_H_
#define _SYS_SUBH_H_


// ----------------------------------------------------------------------------
// SYSTEM SUBSYSTEM (SYS) system-dependent function declarations              -
// ----------------------------------------------------------------------------


// init reference frame counting
void		SYSs_InitRefFrameCount();

// get current reference frame count
refframe_t	SYSs_GetRefFrameCount();

// pause reference frame counting
void		SYSs_PauseRefFrameCount();

// resume reference frame counting
void		SYSs_ResumeRefFrameCount();

// wait for specified number of reference frames
void		SYSs_Wait( refframe_t refframes );

// called once per frame to pass control to a platform-specific handler
int			SYSs_Yield();

// check if enough memory to boot game proper
void		SYSs_CheckMemory();

// check and execute command line options
void		SYSs_CheckCommandLine( int argc, char **argv );


// frame timer install/deinstall
int	SYSs_InitFrameTimer();
int	SYSs_KillFrameTimer();


#endif // _SYS_SUBH_H_


