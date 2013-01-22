/*
 * PARSEC HEADER
 * Debugging Support V1.20
 *
 * Copyright (c) Markus Hadwiger 1996-1999
 * All Rights Reserved.
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_


// debug control --------------------------------------------------------------
//
#define PARSEC_DEBUG


// memory allocation control (also used in non-debug mode) --------------------
//
#define USE_ALIGNED_MEMBLOCKS


// verbose assertions include the actual text of the evaluated expression -----
//
//#define VERBOSE_ASSERTIONS


// debug control --------------------------------------------------------------

//NOTE:
// the following flags determine what checks are performed in debug mode.

//#define LOG_MEMORY_BLOCKS
//#define EXTENSIVE_MEMORY_CHECKS
//#define CHECK_MEM_INTEGRITY_ON_FREE


// ----------------------------------------------------------------------------



#ifdef PARSEC_DEBUG

	// debug only statement encapsulation (pass through)
	#define DBG(x) x

	// don't inline in debug build
	#define INLINE

	// put private functions into global namespace
	#define PRIVATE
	#define PUBLIC

#else // PARSEC_DEBUG

	// debug only statement encapsulation (eliminate)
	#define DBG(x)

	// do inlining in release build
	#define INLINE inline

	// declare private functions as static
	#define PRIVATE static
	#define PUBLIC

	// print warning message, when compiling RELEASE version
	#ifdef ENABLE_CHEAT_COMMANDS
		#ifdef SYSTEM_COMPILER_MSVC
			#pragma message ( "WARNING: ENABLE_CHEAT_COMMANDS is defined in RELEASE build !" )
		#endif // SYSTEM_COMPILER_MSVC
	#endif // ENABLE_CHEAT_COMMANDS

#endif // PARSEC_DEBUG



// define heap functions to use

#define HEAP_ALLOC					malloc		// C runtime library malloc()
#define HEAP_FREE					free		// C runtime library free()


// used in function declarations to tell the compiler that the function won't return
#ifdef SYSTEM_COMPILER_MSVC
	#define FUNCTION_NORETURN(x) __declspec(noreturn) x
#else
	#define FUNCTION_NORETURN(x) x __attribute__((__noreturn__))
#endif


// external functions

void* _AlignedMalloc( size_t );
void  _AlignedFree( void* );

void  _SysAssert( const char*, unsigned );
void* _LogAllocMem( size_t );
void  _LogFreeMem( void* );
void  _CheckHeapBaseRef( void* );
void  _CheckHeapRef( void* );
void  _CheckMemIntegrity();


// define function wrappers ---------------------------------------------------

#ifdef PARSEC_DEBUG

	#include <assert.h>
	#define ASSERT( f ) assert( f )

	#ifdef LOG_MEMORY_BLOCKS

		#define ALLOCMEM( s )			_LogAllocMem( s )
		#define FREEMEM( m )			_LogFreeMem( m )

	#else // LOG_MEMORY_BLOCKS

		#ifdef USE_ALIGNED_MEMBLOCKS

			#define ALLOCMEM( s )		_AlignedMalloc( s )
			#define FREEMEM( m )		_AlignedFree( m )

		#else // USE_ALIGNED_MEMBLOCKS

			#define ALLOCMEM( s )		HEAP_ALLOC( s )
			#define FREEMEM( m )		HEAP_FREE( m )

		#endif // USE_ALIGNED_MEMBLOCKS

	#endif // LOG_MEMORY_BLOCKS

	#ifdef EXTENSIVE_MEMORY_CHECKS

		#define CHECKHEAPBASEREF( r )	_CheckHeapBaseRef( r )
		#define CHECKHEAPREF( r )		_CheckHeapRef( r )
		#define CHECKMEMINTEGRITY()		_CheckMemIntegrity()

	#else // EXTENSIVE_MEMORY_CHECKS

		#define CHECKHEAPBASEREF( r )	{}
		#define CHECKHEAPREF( r )		{}
		#define CHECKMEMINTEGRITY()		{}

	#endif // EXTENSIVE_MEMORY_CHECKS

#else // PARSEC_DEBUG

	#ifdef USE_ALIGNED_MEMBLOCKS

		#define ALLOCMEM( s )		_AlignedMalloc( s )
		#define FREEMEM( m )		_AlignedFree( m )

	#else // USE_ALIGNED_MEMBLOCKS

		#define ALLOCMEM( s )		HEAP_ALLOC( s )
		#define FREEMEM( m )		HEAP_FREE( m )

	#endif // USE_ALIGNED_MEMBLOCKS

	#define ASSERT( f )				{}
	#define CHECKHEAPBASEREF( r )	{}
	#define CHECKHEAPREF( r )		{}
	#define CHECKMEMINTEGRITY()		{}

#endif // PARSEC_DEBUG






// message control ------------------------------------------------------------
//

//NOTE: the following flags determine, what messages get printed out by the
//      various DBGTXT, UPDTXT, UPDTXT2, UPDTXT3, RMEVTXT, ADXTXT macros
//      these flags were moved from NET_CONF.H as they prove generally useful

#define DEBUG_TEXTS
#define SHOW_UPDATE_TEXT
#define RMEV_MESSAGES
#define PRINT_ADDRESSES

#ifdef DEBUG_TEXTS
	#define DBGTXT(x)	{ if ( AUX_ENABLE_CONSOLE_DEBUG_MESSAGES ) { x } else {} }
#else // !DEBUG_TEXTS
	#define DBGTXT(x)
#endif // !DEBUG_TEXTS

#ifdef SHOW_UPDATE_TEXT
	#define UPDTXT(x)	DBGTXT(x)
#else // !SHOW_UPDATE_TEXT
	#define UPDTXT(x)
#endif // !SHOW_UPDATE_TEXT

#ifdef SHOW_UPDATE_TEXT2
	#define UPDTXT2(x)	DBGTXT(x)
#else // !SHOW_UPDATE_TEXT2
	#define UPDTXT2(x)
#endif // !SHOW_UPDATE_TEXT2

#ifdef SHOW_UPDATE_TEXT3
	#define UPDTXT3(x)	DBGTXT(x)
#else // !SHOW_UPDATE_TEXT3
	#define UPDTXT3(x)
#endif // !SHOW_UPDATE_TEXT3

#ifdef RMEV_MESSAGES
	#define RMEVTXT(x)	DBGTXT(x)
#else // !RMEV_MESSAGES
	#define RMEVTXT(x)
#endif // !RMEV_MESSAGES

#ifdef PRINT_ADDRESSES
	#define ADXTXT(x)	DBGTXT(x)
#else // !PRINT_ADDRESSES
	#define ADXTXT(x)
#endif // !PRINT_ADDRESSES

#ifdef DMALLOC
#include "dmalloc.h"
#endif


#endif // _DEBUG_H_


