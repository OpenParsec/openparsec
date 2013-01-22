//-----------------------------------------------------------------------------
//	BSPLIB MODULE: AodOutput.cpp
//
//  Copyright (c) 1996-1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "AodOutput.h"
#include "BspObject.h"
#include "BspTool.h"
#include "ObjectAodFormat.h"


BSPLIB_NAMESPACE_BEGIN


// construct AodOutput object -------------------------------------------------
//
AodOutput::AodOutput( BspObjectList objectlist, const char *filename ) :
	SingleOutput( objectlist, BspTool::ChangeExtension( filename, AOD_FILE_EXTENSION ) )
{
}


// construct AodOutput object using an InputData3D object ---------------------
//
AodOutput::AodOutput( const InputData3D& inputdata ) :
	SingleOutput( inputdata.getObjectList(),
				  BspTool::ChangeExtension( inputdata.getFileName(), AOD_FILE_EXTENSION ) )
{
	// get pointer to "real" input object
	InputData3D *inputobj = inputdata.getRealObject();

	// try to isolate the AodFormat part of the input object
	AodFormat *aodformat = dynamic_cast<AodFormat*>(inputobj);

	// copy over if cast succeeded
	if ( aodformat != NULL ) {

		*(AodFormat *)this = *aodformat;

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
AodOutput::~AodOutput()
{
	if ( m_output.Status() == SYSTEM_IO_OK )
		WriteOutputFile();
}


// assignment copies only the AodFormat part of the object --------------------
//
AodOutput& AodOutput::operator =( const AodOutput& copyobj )
{
	*(AodFormat *)this = copyobj;
	return *this;
}


// write '.aod' file for this object ------------------------------------------
//
int AodOutput::WriteOutputFile()
{
	sprintf( line, "Writing ascii object data to \"%s\"...\n", (const char *) m_filename );
	InfoMessage( line );

	sprintf( line, "%s\n\n", AOD_SIGNATURE_1_1 );
	m_output.WriteLine( line );

	// write bsplib banner to file and init output data
	SingleOutput::InitOutput();

	// create 3-D object knowing about '.aod' format
	ObjectAodFormat obj( *m_baseobject, *this );

	// write attribute lists
	obj.WriteVertexList( m_output );
	obj.WriteFaceList( m_output );
	obj.WriteNormals( m_output );
	obj.WriteFaceProperties( m_output );
	obj.WriteMappingList( m_output );
	obj.WriteTextureList( m_output );

	// write common part
	SingleOutput::WriteOutputFile();

	return m_output.Status();
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
