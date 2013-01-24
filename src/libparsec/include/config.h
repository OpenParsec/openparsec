/*
 * PARSEC HEADER: config.h
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_


//-----------------------------------------------------------------------------
// PROJECT GLOBAL FLAGS FOR CONDITIONAL COMPILATION                           -
//-----------------------------------------------------------------------------

#include "platform.h"


// determine if fractional depth buffer values should be used (depth_t) -------
//
#define FRACTIONAL_DEPTH_VALUES


// determine if screen space coordinates should be subpixel accurate ----------
//
#define SCREENVERTEX_SUBPIXEL_ACCURACY


// determine whether the refframe frequency can be changed from the console ---
//
#define VARIABLE_REFFRAME_FREQUENCY


// determine whether cheats are allowed ---------------------------------------
//
#define ENABLE_CHEAT_COMMANDS


// determine whether to enable the LOGOUT command -----------------------------
//
//#define ENABLE_LOGOUT_COMMAND


// determine whether to build the self running demo ---------------------------
//
//#define CLIENT_BUILD_SELF_RUNNING_DEMO

// determine whether to build the lan-only version ----------------------------
//
//#define CLIENT_BUILD_LAN_ONLY_VERSION

// define these at the project level for the corresponding builds -------------
//
//#define PARSEC_CLIENT			// client build
//#define PARSEC_SERVER			// gameserver build
//#define PARSEC_MASTER			// masterserver build


// define current build number ------------------------------------------------
//
#define CLIENT_BUILD_NUMBER	"0198"
#define SERVER_BUILD_NUMBER "0200"
#define MASTER_BUILD_NUMBER "0200"


// determine that this is an internal only ( testing ) version ----------------
//
#define INTERNAL_VERSION


// enable/disable dynamic function binding ------------------------------------
//
//#define NO_DBIND

#if !defined( NO_DBIND ) && !defined( PARSEC_SERVER )
	#define DBIND_PROTOCOL
#endif // NO_DBIND

// define this to use the CURSES library for the server -----------------------
//
#define USE_CURSES

// miscellaneous conditions ---------------------------------------------------
//
//#define SHOW_COMPILER				    			// used only in this file


// automatic host cpu selection if not already set ----------------------------
// TODO: fixme?
//
#if !( defined( SYSTEM_CPU_INTEL ) || defined( SYSTEM_CPU_POWERPC ) )
	#define SYSTEM_CPU_INTEL
#endif


// define endianness constants according to host cpu --------------------------
//
#if defined( SYSTEM_CPU_INTEL )
	#define SYSTEM_LITTLE_ENDIAN
#elif defined( SYSTEM_CPU_POWERPC )
	#define SYSTEM_BIG_ENDIAN
#else
	#error "endianness not defined!"
#endif


// legacy defines -------------------------------------------------------------
//
#ifdef SERVER_GAMESERVER
	#error "replace SERVER_GAMESERVER with PARSEC_SERVER"
#endif // SERVER_GAMESERVER

#ifdef SERVER_MASTERSERVER
	#error "replace SERVER_MASTERSERVER with PARSEC_MASTER"
#endif // SERVER_MASTERSERVER 


// check for correct project level defines ------------------------------------
//
#if !( defined( PARSEC_CLIENT ) || defined( PARSEC_SERVER ) || defined( PARSEC_MASTER ) )
	#error "missing project level define PARSEC_CLIENT, PARSEC_SERVER or PARSEC_MASTER"
#endif

// conditional code generation macros
//
#if defined ( PARSEC_CLIENT )
	#define CLIENT_ONLY( x )	{ x }
	#define SERVER_ONLY( x )	{ }
	#define MASTER_ONLY( x )	{ }
#elif defined ( PARSEC_SERVER ) 
	#define CLIENT_ONLY( x )	{ }
	#define SERVER_ONLY( x )	{ x }
	#define MASTER_ONLY( x )	{ }
#elif defined ( PARSEC_MASTER )
	#define CLIENT_ONLY( x )	{ }
	#define SERVER_ONLY( x )	{ }
	#define MASTER_ONLY( x )	{ x }
#endif 


// compiler-specific definitions ----------------------------------------------
//
#ifdef SHOW_COMPILER
	#if defined( SYSTEM_COMPILER_MSVC )
		#pragma message( "Compiling with Visual C++" )
	#elif defined( SYSTEM_COMPILER_CLANG )
		#pragma message( "Compiling with Clang" )
	#elif defined( SYSTEM_COMPILER_LLVM_GCC )
		#pragma message( "Compiling with LLVM-GCC" )
	#elif defined( SYSTEM_COMPILER_GCC )
		#pragma message( "Compiling with GCC" )
	#endif
#endif // SHOW_COMPILER

#ifdef SYSTEM_COMPILER_MSVC
	#pragma warning ( disable : 4305 )
	#define PATH_MAX		_MAX_PATH
//	#define vsnprintf		_vsnprintf
	#define snprintf		_snprintf
#endif // SYSTEM_COMPILER_MSVC

#if defined(SYSTEM_COMPILER_GCC) || defined(SYSTEM_COMPILER_CLANG) || defined(SYSTEM_COMPILER_LLVM_GCC)

#ifndef PATH_MAX
	#define PATH_MAX		255				// not always defined
#endif

#endif //defined(SYSTEM_COMPILER_GCC) || defined(SYSTEM_COMPILER_CLANG) || defined(SYSTEM_COMPILER_LLVM_GCC)


#endif // _CONFIG_H_


