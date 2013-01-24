/*
 * PARSEC HEADER: general.h
 */

#ifndef _GENERAL_H_
#define _GENERAL_H_


// include type definitions ---------------------------------------------------
//
#include "gd_type.h"
#include "gd_color.h"


// include limits -------------------------------------------------------------
//
#include "gd_limit.h"


// include global constants ---------------------------------------------------
//
#include "gd_const.h"
#ifdef PARSEC_CLIENT
	#include "gd_bmap.h"
	#include "gd_font.h"
#endif // PARSEC_CLIENT


// include host byte order macros ---------------------------------------------
//
#include "gd_host.h"

// include general utility functions/classes ----------------------------------
//
#include "utl_list.h"


// include helper functions ---------------------------------------------------
//
#include "gd_help.h"


// include message output functions -------------------------------------------
//
#ifdef PARSEC_CLIENT
	#include "sys_msg.h"
#elif defined ( PARSEC_SERVER ) // !PARSEC_CLIENT
	#include "sys_msg_sv.h"
#endif // !PARSEC_CLIENT



// include error functions and define exit function macros --------------------
//
#ifdef PARSEC_CLIENT
	#include "sys_err.h"
#elif defined ( PARSEC_SERVER ) // !PARSEC_CLIENT
	#include "sys_err_sv.h"
#endif // !PARSEC_CLIENT

#ifndef SYSTEM_TARGET_WINDOWS
	// implemented in SL_MAIN.C
	int stricmp( const char *s1, const char *s2 );
	int strnicmp( const char *s1, const char *s2, int len );
	char *strlwr( char *str );
	char *strupr( char *str );
#elif !defined(__MINGW32__)
#include <BaseTsd.h>
#define ssize_t SSIZE_T
#define strtok_r strtok_s
#endif

#endif // _GENERAL_H_


