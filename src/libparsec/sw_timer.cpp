/*
 * PARSEC - Frame Timing
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-2000
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */ 
#include "config.h"

#ifdef _WIN32

// C library includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// time functions

// compilation flags/debug support
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#ifdef PARSEC_CLIENT
#include "aud_defs.h"
#include "inp_defs.h"
#endif //PARSEC_CLIENT
#include "sys_defs.h"

// windows headers
#include <windows.h>

// local module header
#include "sw_timer.h"



// global variables for frame timing ------------------------------------------
//
volatile int		FrameRate;
volatile int		FrameCounter;
volatile int		RefTimeCount;
volatile int		RefFrameCount;


// timer vars -----------------------------------------------------------------
//
MMRESULT		sw_SecTimerID;			// ID of the timer that is called each second
__int64			sw_curTimer;			// current timer value
hprec_t			sw_dTimerFreqFac;		// factor from timer to refframes
__int64			sw_TimerFreq;			// frequency of performance timer


// reference frame count pausing ----------------------------------------------
//
static int			refframe_count_paused	= FALSE;
static refframe_t	refframe_count_pauseval	= REFFRAME_INVALID;
static refframe_t	refframe_count_pauseofs	= 0;


//  callback for Sec.Timer to set the current framerate -----------------------
//
void CALLBACK
SW_SecTimeProc( UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2 )
{
	FrameRate = FrameCounter;
	FrameCounter = 0;
}


// init frame timer handler ---------------------------------------------------
//
int SYSs_InitFrameTimer()
{
	int static init_done = FALSE;

	if ( init_done ) {

		// calc refframe count for old frequency
		QueryPerformanceCounter( (LARGE_INTEGER*) &sw_curTimer );
		refframe_t refframecount = (int) ( sw_curTimer * sw_dTimerFreqFac );

		// calc correction factor for new frequency
		sw_dTimerFreqFac = (hprec_t)FRAME_MEASURE_TIMEBASE / sw_TimerFreq;

		// calc offset to ensure monotonically increasing refframecount
		// even though we now use the new frequency
		RefFrameCount = (int) ( sw_curTimer * sw_dTimerFreqFac );
		refframe_count_pauseofs += RefFrameCount - refframecount;

	} else {

		// query the frequency of the performance counter and calculate
		// the factor to adjust to our FRAME_MEASURE_TIMEBASE
		QueryPerformanceFrequency( (LARGE_INTEGER *) &sw_TimerFreq );
		sw_dTimerFreqFac = (hprec_t)FRAME_MEASURE_TIMEBASE / sw_TimerFreq;

		// create the timer which is called every second
		sw_SecTimerID = timeSetEvent( 1000, 0, &SW_SecTimeProc, 0, TIME_PERIODIC );

		// set initial count
		SYSs_InitRefFrameCount();

		init_done = TRUE;
	}

	return 1;
}


// de-init frame timer handler ------------------------------------------------
//
int SYSs_KillFrameTimer()
{
	// terminate the seconds timer
	timeKillEvent( sw_SecTimerID );

	return 1;
}


// init reference frame counting ----------------------------------------------
//
void SYSs_InitRefFrameCount()
{
	SYSs_GetRefFrameCount();

	// init refframe count pausing/offset
	refframe_count_paused	= FALSE;
	refframe_count_pauseval	= REFFRAME_INVALID;
	refframe_count_pauseofs	= 0;
}


// get count of current reference frame ---------------------------------------
//
refframe_t SYSs_GetRefFrameCount()
{
	QueryPerformanceCounter( (LARGE_INTEGER*) &sw_curTimer );
	RefFrameCount = (int) ( sw_curTimer * sw_dTimerFreqFac );

	return RefFrameCount - refframe_count_pauseofs;
}


// pause reference frame counting ---------------------------------------------
//
void SYSs_PauseRefFrameCount()
{
	if ( !refframe_count_paused ) {

		SYSs_GetRefFrameCount();

		refframe_count_pauseval	= RefFrameCount;
		refframe_count_paused	= TRUE;
	}
}


// resume reference frame counting --------------------------------------------
//
void SYSs_ResumeRefFrameCount()
{
	if ( refframe_count_paused ) {

		SYSs_GetRefFrameCount();

		refframe_count_pauseofs	+= RefFrameCount - refframe_count_pauseval;
		refframe_count_pauseval	 = REFFRAME_INVALID;
		refframe_count_paused	 = FALSE;
	}
}


// wait for specified number of reference frames ------------------------------
//
void SYSs_Wait( refframe_t refframes )
{
	refframe_t basecount = SYSs_GetRefFrameCount();

	while ( SYSs_GetRefFrameCount() - basecount < refframes ) {
		SYSs_Yield();
	}
}


// guaranteed to be called once per frame -------------------------------------
//
int SYSs_Yield()
{
#ifdef PARSEC_CLIENT

	// yield to sound driver
	AUDs_MaintainSound();

	// must be called to retrieve key strokes from buffer
	INPs_Collect();

#endif // PARSEC_CLIENT

	return 1;
}

#ifdef PARSEC_SERVER

REGISTER_MODULE( SW_TIMER )
{
	SYSs_InitFrameTimer();
}

#endif
#endif
