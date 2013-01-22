//-----------------------------------------------------------------------------
//	BSPLIB MODULE: BspTool.cpp
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "BspTool.h"

// flags
#define STRIP_PATH


BSPLIB_NAMESPACE_BEGIN


// create new string with altered extension -----------------------------------
//
String BspTool::ChangeExtension( const char *filename, const char *extension )
{
	if ( ( filename == NULL ) || ( extension == NULL ) )
		return String( NULL );

	int		len   = strlen( filename );
	int		pos	  = len;
	char	*name = NULL;
	const char *scan = filename + pos;

	while ( ( --pos > 0 ) && ( *--scan != '.' ) ) ;

	if ( pos > 0 ) {
		// change extension if there was an extension
		name = new char[ pos + strlen( extension ) + 1 ];
		strncpy( name, filename, pos );
		strcpy( name + pos, extension );
	} else {
		// append new extension if there was no extension
		name = new char[ len + strlen( extension ) + 1 ];
		strcpy( name, filename );
		strcpy( name + len, extension );
	}

#ifdef STRIP_PATH
	String ret( SkipPath( name ) );
#else
	String ret( name );
#endif

	delete name;
	return ret;
}


// create filename without path -----------------------------------------------
//
const String BspTool::SkipPath( const char *fullname )
{
	const char *scan = fullname + strlen( fullname ) - 1;
	while ( ( *scan != '\\' ) && ( *scan != ':' ) && ( scan >= fullname ) )
		scan--;
	return String( scan + 1 );
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
