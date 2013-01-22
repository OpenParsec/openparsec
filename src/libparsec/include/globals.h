/*
 * PARSEC HEADER: globals.h
 */

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#ifdef PARSEC_SERVER

	// include subsystems globals ----------------------------

	//#include "aud_glob.h"	// AUDIO
	//#include "inp_glob.h"	// INPUT
	//#include "net_glob.h"	// NETWORKING
	//#include "sys_glob.h"	// SYSTEM
	//#include "vid_glob.h"	// VIDEO

	// include engine core globals ---------------------------

	#include "e_global_sv.h"	// ENGINE CORE ( SERVER )

	// include game code globals -----------------------------

	#include "e_world.h"

#else

	// include subsystems globals ----------------------------

	#include "aud_glob.h"	// AUDIO
	#include "inp_glob.h"	// INPUT
	#include "net_glob.h"	// NETWORKING
	#include "sys_glob.h"	// SYSTEM
	#include "vid_glob.h"	// VIDEO

	#include "d_glob.h"		// DRAWING
	#include "r_glob.h"		// RENDERING

	// include engine core globals ---------------------------

	#include "e_global.h"	// ENGINE CORE


	// include game code globals -----------------------------

	#include "g_global.h"	// GAME CODE
	#include "h_global.h"	// GAME CODE/HUD

#endif // PARSEC_SERVER

#endif // _GLOBALS_H_


