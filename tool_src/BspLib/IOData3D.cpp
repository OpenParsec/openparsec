//-----------------------------------------------------------------------------
//	BSPLIB MODULE: IOData3D.cpp
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "IOData3D.h"


BSPLIB_NAMESPACE_BEGIN


// constructor for generic I/O class ------------------------------------------
//
IOData3D::IOData3D( BspObjectList objectlist, const String& filename, int checkflags ) :
	m_objectlist( objectlist ),
	m_filename( filename )
{
	if ( m_filename.IsNULL() && ( checkflags & CHECK_FILENAME ) ) {
		ErrorMessage( "[IOData3D]: valid filename must be supplied." );
		HandleCriticalError();
	}
}


// signatures of recognized file formats --------------------------------------
//
const char IOData3D::AOD_SIGNATURE_1_1[]	= "#AOD V1.1 ascii";
const char IOData3D::BSP_SIGNATURE_1_1[]	= "#BSP V1.1 ascii";
const char IOData3D::VRML_SIGNATURE_1_0[]	= "#VRML V1.0 ascii";
const char IOData3D::_3DX_SIGNATURE_1_0[]	= "3DX File 1.0";
//ADD_FORMAT:


// static string scratch pad --------------------------------------------------
//
const int	IOData3D::TEXTLINE_MAX				= 1023;
char		IOData3D::line[ TEXTLINE_MAX + 1 ]	= "";


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
