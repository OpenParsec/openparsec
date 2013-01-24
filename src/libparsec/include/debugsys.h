/*
 * PARSEC HEADER
 * System Debugging Functions V1.15
 *
 * Copyright (c) Markus Hadwiger 1998-2000
 * All Rights Reserved.
 */

#ifndef _DEBUGSYS_H_
#define _DEBUGSYS_H_



// _SysAssert() calls this prior to aborting ----------------------------------
//
void SysAssertCallback( const char *file, unsigned line )
{

#if defined( SYSTEM_MACOSX_UNUSED )

	#ifdef PARSEC_CLIENT
		static char str[ 256 ];
		sprintf( str, "Assertion failed: %s, line %u\n", file, line );

		extern void SX_ReportErrorString( char *msg );
		SX_ReportErrorString( str );
	#endif // PARSEC_CLIENT

#elif defined( SYSTEM_WIN32_UNUSED ) && defined( PARSEC_CLIENT )

	const char *msg = ( *file == '"' ) ? "%s, line %u" : "file %s, line %u";

	extern void SW_WinError( const char *caption, const char *content, ... );
	SW_WinError( "Assertion failed", msg, file, line );

#else

	// nothing here ( 

#endif

}


// ascertain a function's caller ----------------------------------------------
//
// not able to determine caller
#define GETCALLER(c) c=_SysGetCaller();
void *_SysGetCaller()
{
	return NULL;
}


#endif // _DEBUGSYS_H_


