/* 
 * PARSEC HEADER: SYSS_REFFRAME.H
 */

#ifndef _SYSS_REFFRAME_H_
#define _SYSS_REFFRAME_H_


// init reference frame counting
void		SYSs_InitRefFrameCount();

// get current reference frame count
refframe_t	SYSs_GetRefFrameCount();

// wait for specified number of reference frames
void		SYSs_Wait( refframe_t refframes );

// called once per frame to pass control to a platform-specific handler
int			SYSs_Yield();

#include "e_global_sv.h"
#include "sl_timer.h"

// helper macros --------------------------------------------------------------
//
#define REFFRAMES_TO_USEC( x )    ( ( x ) * 1000000 ) / FRAME_MEASURE_TIMEBASE; 

#endif // !_SYSS_REFFRAME_H_

