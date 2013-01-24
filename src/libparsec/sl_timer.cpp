/*
 * PARSEC - Frame Timing
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:42 $
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

#ifndef SYSTEM_TARGET_WINDOWS

// C library includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// time functions
#include <sys/time.h>
#include <unistd.h>

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

// local module header
#include "sl_timer.h"



// global variables for frame timing ------------------------------------------
//
volatile int		FrameRate;
volatile int		FrameCounter;
volatile int		RefTimeCount;
volatile int		RefFrameCount;


// static timer vars ----------------------------------------------------------
//
static refframe_t	sl_timfreqsec;
static hprec_t		sl_timfreqfac;
static int			sl_timsecbase;
static int			sl_frmsecbase;


// reference frame count pausing ----------------------------------------------
//
static int			refframe_count_paused	= FALSE;
static refframe_t	refframe_count_pauseval	= REFFRAME_INVALID;
static refframe_t	refframe_count_pauseofs	= 0;


// base frequency of gettimeofday() is 1MHz -----------------------------------
//
#define BASE_FREQ	1000000.0


// init frame timing ----------------------------------------------------------
//
int SYSs_InitFrameTimer()
{
	int static init_done = FALSE;

	if ( init_done ) {

		// get high-precision time
		struct timeval  time_timeval;
		struct timezone time_timezone;
		gettimeofday( &time_timeval, &time_timezone );

		// calc refframe count for old frequency
		time_t secframes = ( time_timeval.tv_sec - sl_timsecbase ) * sl_timfreqsec;
		refframe_t refframecount = refframe_t( time_timeval.tv_usec * sl_timfreqfac + secframes );

		// calc correction factor for new frequency
		sl_timfreqsec = FRAME_MEASURE_TIMEBASE;
		sl_timfreqfac = FRAME_MEASURE_TIMEBASE / BASE_FREQ;

		// calc offset to ensure monotonically increasing refframecount
		// even though we now use the new frequency
		secframes = ( time_timeval.tv_sec - sl_timsecbase ) * sl_timfreqsec;
		RefFrameCount = refframe_t( time_timeval.tv_usec * sl_timfreqfac + secframes );
		refframe_count_pauseofs += RefFrameCount - refframecount;

	} else {

		// calculate factor to adjust to our time quantum
		sl_timfreqsec = FRAME_MEASURE_TIMEBASE;
		sl_timfreqfac = FRAME_MEASURE_TIMEBASE / BASE_FREQ;

		// set initial count
		SYSs_InitRefFrameCount();

		init_done = TRUE;
	}

	return TRUE;
}


// deinit frame timing --------------------------------------------------------
//
int SYSs_KillFrameTimer()
{
	// not needed
	return 1;
}


// init reference frame counting ----------------------------------------------
//
void SYSs_InitRefFrameCount()
{
	// secbase for frame rate measurement not set
	sl_frmsecbase = -1;

	// get high-precision time
	struct timeval  time_timeval;
	struct timezone time_timezone;
	gettimeofday( &time_timeval, &time_timezone );

	// set base in seconds
	sl_timsecbase = (int) time_timeval.tv_sec;

	// calc number of elapsed reference frames
	int secframes = int(( time_timeval.tv_sec - sl_timsecbase ) * sl_timfreqsec );
	RefFrameCount = (int)( time_timeval.tv_usec * sl_timfreqfac ) + secframes;

	// init refframe count pausing/offset
	refframe_count_paused	= FALSE;
	refframe_count_pauseval	= REFFRAME_INVALID;
	refframe_count_pauseofs	= 0;
}


// get count of current reference frame ---------------------------------------
//
refframe_t SYSs_GetRefFrameCount()
{
	// get high-precision time
	struct timeval  time_timeval;
	struct timezone time_timezone;
	gettimeofday( &time_timeval, &time_timezone );

	// calc number of elapsed reference frames
	int secframes = int(( time_timeval.tv_sec - sl_timsecbase ) * sl_timfreqsec );
	RefFrameCount = (int)( time_timeval.tv_usec * sl_timfreqfac ) + secframes;

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
	int basecount = SYSs_GetRefFrameCount();

	while ( SYSs_GetRefFrameCount() - basecount < refframes ) {
		SYSs_Yield();
	}
}


// use yield function to calculate frame rate ---------------------------------
//
int SYSs_Yield()
{
	
#ifdef PARSEC_CLIENT

	// yield to sound driver
	AUDs_MaintainSound();

	// must be called to retrieve key strokes from buffer
	INPs_Collect();

#endif // PARSEC_CLIENT
	
	// get high-precision time
	struct timeval  time_timeval;
	struct timezone time_timezone;
	gettimeofday( &time_timeval, &time_timezone );

	// set base if not done yet
	if ( sl_frmsecbase == -1 ) {
		sl_frmsecbase = time_timeval.tv_sec;
	}

	// flush framecounter every second
	if ( ( time_timeval.tv_sec - sl_frmsecbase ) > 0 ) {
		sl_frmsecbase = time_timeval.tv_sec;
		FrameRate = FrameCounter;
		FrameCounter = 0;
	}

	return 1;
}


#ifdef PARSEC_SERVER

REGISTER_MODULE( SL_TIMER )
{
	SYSs_InitFrameTimer();
}

#endif

#endif // !SYSTEM_TARGET_WINDOWS
