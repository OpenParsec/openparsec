/* 
 * PARSEC HEADER: SYSS_REFFRAME.H
 */

#ifndef _SYSS_REFFRAME_H_
#define _SYSS_REFFRAME_H_

#if defined( SYSTEM_LINUX ) || defined (SYSTEM_SDL)

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

#elif defined ( SYSTEM_MACOSX )

	// init reference frame counting
	void		SYSs_InitRefFrameCount();

	// get current reference frame count
	refframe_t	SYSs_GetRefFrameCount();

	// wait for specified number of reference frames
	void		SYSs_Wait( refframe_t refframes );

	// called once per frame to pass control to a platform-specific handler
	int			SYSs_Yield();

	#include "e_global_sv.h"
	#include "sx_timer.h"

#elif defined( SYSTEM_WIN32 )

	// class for handling the refframe counting -----------------------------------
	//
	class SYS_RefFrameCount
	{
	protected:
		int			m_paused;
		refframe_t	m_pauseval;
		refframe_t	m_pauseofs;
		int			m_RefFrameCount;
		int			m_Frequency;
	public:
		SYS_RefFrameCount();

		void		Init();
		refframe_t	Get();
		void		Pause();
		void		Resume();
		void		Wait( refframe_t wait );
		refframe_t  GetFrequency();
	};

	// global refframe counter ----------------------------------------------------
	//
	//FIXME: SYS_RefFrameCount should be a member of the server class
	extern SYS_RefFrameCount		g_RefFrameCount;

	// macros to be compatible with client code -----------------------------------
	//
	#define SYSs_Wait				g_RefFrameCount.Wait
	#define SYSs_GetRefFrameCount	g_RefFrameCount.Get
	#define SYSs_InitRefFrameCount	g_RefFrameCount.Init

	#define RefFrameFrequency		g_RefFrameCount.GetFrequency()

#endif // SYSTEM_WIN32

// helper macros --------------------------------------------------------------
//
#define REFFRAMES_TO_USEC( x )    ( ( x ) * 1000000 ) / FRAME_MEASURE_TIMEBASE; 

#endif // !_SYSS_REFFRAME_H_

