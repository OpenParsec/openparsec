/*
 * PARSEC - Build Info
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:42 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-1999
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

// C library
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// local module header
#include "sys_date.h"



// determine target system

#if defined( SYSTEM_TARGET_WINDOWS )
	#define SYSTEM_TEXT			"/windows "
#elif defined( SYSTEM_TARGET_LINUX )
	#define SYSTEM_TEXT			"/linux "
#elif defined( SYSTEM_TARGET_OSX )
	#define SYSTEM_TEXT			"/osx "
#elif defined( SYSTEM_TARGET_IOS )
	#define SYSTEM_TEXT			"/ios "
#else
	#define SYSTEM_TEXT			"/unknown "
#endif


// determine build

#ifdef DEBUG
	#define BUILD_TEXT "openparsec" SYSTEM_TEXT "debug build " CLIENT_BUILD_NUMBER
#else
	#define BUILD_TEXT "openparsec" SYSTEM_TEXT "release build " CLIENT_BUILD_NUMBER
#endif


// helper macros

#define BUILD_STR(x)			#x
#define BUILD_VER(x)			BUILD_STR(x)


// determine compiler

#if defined( SYSTEM_COMPILER_MSVC )
	#define BUILD_COMPILER		"compiler: microsoft visual c++ (" BUILD_VER(_MSC_VER) ")"
#elif defined( SYSTEM_COMPILER_CLANG )
	#define BUILD_COMPILER		"compiler: clang " BUILD_VER(__clang_major__.__clang_minor__.__clang_patchlevel__)
#elif defined( SYSTEM_COMPILER_LLVM_GCC )
	#define BUILD_COMPILER		"compiler: llvm-gcc " BUILD_VER(__GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__)
#elif defined( SYSTEM_COMPILER_GCC )
	#define BUILD_COMPILER		"compiler: gnu c++ " BUILD_VER(__GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__)
#else
	#error "unsupported compiler!"
#endif


// determine binding

#define BUILD_BINDING			"subsystem binding: static"


// determine endianness

#if defined( SYSTEM_BIG_ENDIAN )
	#define BUILD_ENDIANNESS	"endianness: big endian"
#else
	#define BUILD_ENDIANNESS	"endianness: little endian"
#endif


//NOTE:
// create build information.
// this module is compiled on every make/build to
// ensure build information is always up to date.

const char build_text[] = BUILD_TEXT;
const char build_date[] = __DATE__;
const char build_time[] = __TIME__;

const char build_comp[] = BUILD_COMPILER;
const char build_bind[] = BUILD_BINDING;
const char build_endn[] = BUILD_ENDIANNESS;



// return current system date and time ----------------------------------------
//
char *SYS_SystemTime()
{
	// fetch system time
	time_t curtime = time( NULL );
	char * timestr = ctime( &curtime );

	return timestr;
}



