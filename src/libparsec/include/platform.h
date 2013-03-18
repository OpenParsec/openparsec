#ifndef _PLATFORM_H
#define _PLATFORM_H
// platform.h - platform dependent include selection should go here.


/* Sick of this junk, hard coding some stuff because there's no reason for the mass
   system dependent code anymore */

#undef SYSTEM_WIN32_UNUSED
#undef SYSTEM_LINUX_UNUSED
#undef SYSTEM_MACOSX_UNUSED

#define SYSTEM_SDL


// New defines for different systems

#if defined(_WIN32)
#	define SYSTEM_TARGET_WINDOWS
// support XP, for some reason...
#define _WIN32_WINNT _WIN32_WINNT_WS03

#elif defined(__APPLE__) // TODO: check for iOS using TargetConditionals.h
#	define SYSTEM_TARGET_OSX
#elif defined(linux) || defined(__linux)
#	define SYSTEM_TARGET_LINUX
#endif


// compiler specification (parsec constants) ----------------------------------
//
#if defined(_MSC_VER)
#	define SYSTEM_COMPILER_MSVC
#elif defined(__clang__)
#	define SYSTEM_COMPILER_CLANG
#elif defined(__GNUC__)
#	if defined(__llvm__)
#		define SYSTEM_COMPILER_LLVM_GCC
#	else
#		define SYSTEM_COMPILER_GCC
#	endif
#endif

#include "platform_sdl.h"

//PARSEC_SERVER Needs this:
#define CPU_VENDOR_OS "i386-pc-win32"

#if ! defined ( PARSEC_CLIENT ) && ! defined ( PARSEC_SERVER )
#error "Error: Building libparsec must be called from the client or server make files... for now..."
#endif

// target system specification
// These should be defined by the compiler or by the 
// building IDE
/*
#if defined ( __WIN32__ ) || defined (WIN32) || defined( __linux__) || defined(__CYGWIN__) || defined(__APPLE__)
//	#define SYSTEM_WIN32_UNUSED
//	#undef SYSTEM_LINUX_UNUSED
//	#undef SYSTEM_MACOSX_UNUSED
//	#define CPU_VENDOR_OS "i386-pc-win32"
//#elif defined( __linux__ )
	#undef SYSTEM_WIN32_UNUSED
	#define SYSTEM_LINUX_UNUSED
	#undef SYSTEM_MACOSX_UNUSED
#ifndef __CYGWIN__
	#define __CYGWIN__ // temporary, for SDL testing.
#endif
	#define CPU_VENDOR_OS "i386-pc-linux-gnu"
#elif defined( __APPLE__ )
	#undef SYSTEM_WIN32_UNUSED
	#define SYSTEM_LINUX_UNUSED
    #undef SYSTEM_MACOSX_UNUSED
	#define CPU_VENDOR_OS "i386-pc-linux-gnu"
#endif

	*/

#endif //_PLATFORM_H
