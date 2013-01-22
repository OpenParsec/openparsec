//-----------------------------------------------------------------------------
//	BSPLIB MODULE: BspOutput.cpp
//
//  Copyright (c) 1996-1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "BspOutput.h"
#include "BspObject.h"
#include "BspTool.h"
#include "ObjectBspFormat.h"


BSPLIB_NAMESPACE_BEGIN


// construct BspOutput object -------------------------------------------------
//
BspOutput::BspOutput( BspObjectList objectlist, const char *filename ) :
	SingleOutput( objectlist, BspTool::ChangeExtension( filename, BSP_FILE_EXTENSION ) )
{
}


// construct BspOutput object using an InputData3D object ---------------------
//
BspOutput::BspOutput( const InputData3D& inputdata ) :
	SingleOutput( inputdata.getObjectList(),
				  BspTool::ChangeExtension( inputdata.getFileName(), BSP_FILE_EXTENSION ) )
{
	// get pointer to "real" input object
	InputData3D *inputobj = inputdata.getRealObject();

	// try to isolate the BspFormat part of the input object
	BspFormat *bspformat = dynamic_cast<BspFormat*>(inputobj);

	// copy over if cast succeeded
	if ( bspformat != NULL ) {

		*(BspFormat *)this = *bspformat;

	} else {

		// try to isolate the SingleFormat part of the input object
		SingleFormat *singleformat = dynamic_cast<SingleFormat*>(inputobj);

		// copy over if cast succeeded
		if ( singleformat != NULL )
			*(SingleFormat *)this = *singleformat;
	}
}


// write outputfile before destruction ----------------------------------------
//
BspOutput::~BspOutput()
{
	if ( m_output.Status() == SYSTEM_IO_OK )
		WriteOutputFile();
}


// assignment copies only the BspFormat part of the object --------------------
//
BspOutput& BspOutput::operator =( const BspOutput& copyobj )
{
	*(BspFormat *)this = copyobj;
	return *this;
}


// write '.bsp' file for this object ------------------------------------------
//
int BspOutput::WriteOutputFile()
{
	sprintf( line, "Writing compiled object data to \"%s\"...\n", (const char *) m_filename );
	InfoMessage( line );

	sprintf( line, "%s\n\n", BSP_SIGNATURE_1_1 );
	m_output.WriteLine( line );

	// write bsplib banner to file and init output data
	SingleOutput::InitOutput();

	// create 3-D object knowing about '.bsp' format
	ObjectBspFormat obj( *m_baseobject, *this );

	// write attribute lists
	obj.WriteVertexList( m_output );
	obj.WritePolygonList( m_output );
	obj.WriteFaceList( m_output );
	obj.WriteNormals( m_output );
	obj.WriteFaceProperties( m_output );
	obj.WriteMappingList( m_output );
	obj.WriteBSPTree( m_output );
	obj.WriteTextureList( m_output );

	// write common part
	SingleOutput::WriteOutputFile();

	return m_output.Status();
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
