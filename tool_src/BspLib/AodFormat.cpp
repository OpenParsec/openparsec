//-----------------------------------------------------------------------------
//	BSPLIB MODULE: AodFormat.cpp
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "AodFormat.h"


BSPLIB_NAMESPACE_BEGIN


// get index to section id specified as string (this function is static!) -----
//
int AodFormat::GetSectionId( char *section_name ) 
{ 
	return SingleFormat::GetSectionId( section_name ); 
}


// get name of section specified via id (this function is static!) ------------
//
const char *AodFormat::GetSectionName( int section_id )
{
	return SingleFormat::GetSectionName( section_id );
}


// AodFormat specific static variables ----------------------------------------
//
const char AodFormat::_aodsig_str[]			= "#AOD";
const char AodFormat::AOD_FILE_EXTENSION[]	= ".aod";


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
