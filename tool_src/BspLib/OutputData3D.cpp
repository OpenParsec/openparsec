//-----------------------------------------------------------------------------
//	BSPLIB MODULE: OutputData3D.cpp
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "OutputData3D.h"
#include "AodOutput.h"
#include "BspOutput.h"
//#include "VrmlOutput.h"

//#define VRML_OUTPUT_POSSIBLE


BSPLIB_NAMESPACE_BEGIN


// "virtual" constructor for different output format objects ------------------
//
OutputData3D::OutputData3D( BspObjectList objectlist, const char *filename, int format ) :
	IOData3D( objectlist, String( filename ) ),
	m_data( NULL )
{
	// create "real" output object according to specified format of output file
	if ( format != DONT_CREATE_OBJECT ) {
		switch ( m_objectformat = format ) {

		case AOD_FORMAT_1_1:
			m_data = new AodOutput( objectlist, filename );
			break;

		case BSP_FORMAT_1_1:
			m_data = new BspOutput( objectlist, filename );
			break;

#ifdef VRML_OUTPUT_POSSIBLE
		case VRML_FORMAT_1_0:
			m_data = new VrmlFile( objectlist, filename );
			break;
#endif
		default:
			ErrorMessage( "[OutputData3D]: Unrecognized output file format." );
			m_objectformat = UNKNOWN_FORMAT;
		}
	} else {
		m_objectformat = DONT_CREATE_OBJECT;
	}
}


// constructor using an InputData3D object directly to get data ---------------
//
OutputData3D::OutputData3D( const InputData3D& inputdata, int format ) :
	IOData3D( inputdata.getObjectList(), inputdata.getFileName() ),
	m_data( NULL )
{
	// create "real" output object according to specified format of output file
	if ( format != DONT_CREATE_OBJECT ) {
		switch ( m_objectformat = format ) {

		case AOD_FORMAT_1_1:
			m_data = new AodOutput( inputdata );
			break;

		case BSP_FORMAT_1_1:
			m_data = new BspOutput( inputdata );
			break;

#ifdef VRML_OUTPUT_POSSIBLE
		case VRML_FORMAT_1_0:
			m_data = new VrmlFile( inputdata );
			break;
#endif
		default:
			ErrorMessage( "[OutputData3D]: Unrecognized output file format." );
			m_objectformat = UNKNOWN_FORMAT;
		}
	} else {
		m_objectformat = DONT_CREATE_OBJECT;
	}
}


// virtual destructor ---------------------------------------------------------
//
OutputData3D::~OutputData3D()
{
	// delete "real" object
	delete m_data;
}


// redirection to WriteOutputFile() of "real" object --------------------------
//
int OutputData3D::WriteOutputFile()
{
	return m_data ? m_data->WriteOutputFile() : FALSE;
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
