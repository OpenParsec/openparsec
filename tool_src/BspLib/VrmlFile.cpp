//-----------------------------------------------------------------------------
//	BSPLIB MODULE: VrmlFile.cpp
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "VrmlFile.h"
#include "BspObject.h"
#include "BspTool.h"

// qvlib header files
#include <QvDB.h>
#include <QvInput.h>
#include <QvNode.h>
#include <QvState.h>


BSPLIB_NAMESPACE_BEGIN


// file is read and parsed immediately after construction ---------------------
//
VrmlFile::VrmlFile( BspObjectList objectlist, const char *filename ) :
	InputData3D( objectlist, filename, DONT_CREATE_OBJECT ),
	m_bboxlist( NULL )
{
	ParseObjectData();
}


// write outputfile before destruction ----------------------------------------
//
VrmlFile::~VrmlFile()
{
	if ( m_inputok && !m_filename.IsNULL() )
		WriteOutputFile();
	delete m_bboxlist;
}


// apply all transformations to scene -----------------------------------------
//
void VrmlFile::ApplySceneTransformations()
{
	BspObject *sceneobj = m_objectlist.getListHead();
	for ( ; sceneobj; sceneobj = sceneobj->getNext() )
		sceneobj->ApplyTransformation();
}


// enforce maximum extents for entire scene -----------------------------------
//
void VrmlFile::EnforceSceneExtents( BoundingBox& unionbox )
{
	double fmext = getMaxExtents();

	Vertex3 minvertex = unionbox.getMinVertex();
	Vertex3 maxvertex = unionbox.getMaxVertex();

	double extent_x = maxvertex.getX() - minvertex.getX();
	double extent_y = maxvertex.getY() - minvertex.getY();
	double extent_z = maxvertex.getZ() - minvertex.getZ();

	double mext = extent_x;
	if ( extent_y > mext ) mext = extent_y;
	if ( extent_z > mext ) mext = extent_z;
	double cfac = fmext / mext;
	
	BspObject *sceneobj = m_objectlist.getListHead();
	for ( ; sceneobj; sceneobj = sceneobj->getNext() )
		sceneobj->ApplyScale( cfac );
}


// parse vrml 1.0 file and build data structures for all contained objects ----
//
int VrmlFile::ParseObjectData()
{
	// open input data file
	FileAccess input( m_filename, "r" );
	InfoMessage( "Processing input data file (format='VRML V1.0') ..." );

	// init qvlib database
    QvDB::init();

	// tie input file to QvInput object
    QvInput	inputdata;
	inputdata.setFilePointer( input );

	// read and parse vrml input data
	QvNode *vrmlroot = NULL;
	if ( QvDB::read( &inputdata, vrmlroot ) && ( vrmlroot != NULL ) ) {
		InfoMessage( "\nObject data ok.\n" );
		m_inputok = TRUE;
	} else {
		sprintf( line, "%sVRML tree data invalid.\n", parser_err_str );
		ErrorMessage( line );
		HandleCriticalError( CRITERR_ALLOW_RET );
		delete vrmlroot;
		return ( m_inputok = FALSE );
	}

	// create object for traversal state
	QvState state( this );

	// traverse entire vrml tree and construct scene representation for bsplib
	vrmlroot->traverse( &state );

	// if scene scale desired calculate union bounding box and scale entire scene
	if ( getEnforceExtentsFlag() && ( m_bboxlist != NULL ) ) {
		BoundingBox unionbox;
		m_bboxlist->BoundingBoxListUnion( unionbox );
		EnforceSceneExtents( unionbox );
	}

	delete vrmlroot;
	return m_inputok;
}


// write output file(s) for objects constructed out of vrml file --------------
//
int VrmlFile::WriteOutputFile()
{
	return FALSE;
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
