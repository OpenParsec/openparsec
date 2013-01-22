//-----------------------------------------------------------------------------
//	BSPLIB MODULE: SingleOutput.cpp
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "SingleOutput.h"
#include "BspObject.h"
#include "BspTool.h"


BSPLIB_NAMESPACE_BEGIN


// construct SingleOutput object ----------------------------------------------
//
SingleOutput::SingleOutput( BspObjectList objectlist, const char *filename ) :
	OutputData3D( objectlist, filename, DONT_CREATE_OBJECT ),
	m_baseobject( objectlist.getListHead() ),
	m_output( filename, "w" )
{
}


// destroy SingleOutput object ------------------------------------------------
//
SingleOutput::~SingleOutput()
{
}


// assignment copies only the SingleFormat part of the object -----------------
//
SingleOutput& SingleOutput::operator =( const SingleOutput& copyobj )
{
	*(SingleFormat *)this = copyobj;
	return *this;
}


// write bsplib banner to file and init output data ---------------------------
//
void SingleOutput::InitOutput()
{
	String filename = BspTool::SkipPath( m_filename );
	
	// write file's name
	sprintf( line, "; %s ----------------------------------------------\n",
			  (const char *) filename );
	m_output.WriteLine( line );

	// write banner
	sprintf( line, ";  automatically created by BspLib V%s\n", version_str );
	m_output.WriteLine( line );
	m_output.WriteLine( ";  Copyright (c) by Markus Hadwiger 1996-1998\n\n" );

	// ensure correctness of attribute counts
	m_baseobject->UpdateAttributeNumbers();
}


// write single format part of file for this object ---------------------------
//
int SingleOutput::WriteOutputFile()
{

#define sprintf m_output.WriteLine( line ); sprintf

#ifndef SHORT_OUTPUT_FORMAT

	strcpy( line, "\n" );

	if ( m_palette_fname ) {
		sprintf( line, "; name of palette file ---------------------------------------------------\n" );
		sprintf( line, "%s\n", _palette_str );
		sprintf( line, "%s\n", (const char *) m_palette_fname );
		sprintf( line, "\n" );
	}

	sprintf( line, "; scale factors for each axis --------------------------------------------\n" );
	sprintf( line, "%s\n", _scalefactors_str );
	sprintf( line, "%f, %f, %f\n", ObjScaleX, ObjScaleY, ObjScaleZ );
	sprintf( line, "\n" );

	sprintf( line, "; translation of origin --------------------------------------------------\n" );
	sprintf( line, "%s\n", _setorigin_str );
	sprintf( line, "%f, %f, %f\n", NewOriginX, NewOriginY, NewOriginZ );
	sprintf( line, "\n" );

	sprintf( line, "; exchange command for axes ----------------------------------------------\n" );
	sprintf( line, "%s\n", _xchange_str );
	sprintf( line, "%s\n", xchangecmd );
	sprintf( line, "\n" );

	sprintf( line, "; starting object location in objectviewer -------------------------------\n" );
	sprintf( line, "%s\n", _worldlocation_str );
	int i = 0;
	for ( i = 0; i < 15; i++ ) {
		sprintf( line, "%f, ", wm[ i ] );
	}
	sprintf( line, "%f\n", wm[ 15 ] );
	sprintf( line, "\n" );

	sprintf( line, "; starting camera location in objectviewer -------------------------------\n" );
	sprintf( line, "%s\n", _camera_str );
	for ( i = 0; i < 15; i++ ) {
		sprintf( line, "%f, ", vm[ i ] );
	}
	sprintf( line, "%f\n", vm[ 15 ] );
	sprintf( line, "\n" );

#endif

	sprintf( line, "<end-of-file ------------------------------------------------------------------>\n" );
	m_output.WriteLine( line );

	return m_output.Status();
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
