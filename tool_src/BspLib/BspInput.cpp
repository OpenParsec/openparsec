//-----------------------------------------------------------------------------
//	BSPLIB MODULE: BspInput.cpp
//
//  Copyright (c) 1996-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "BspInput.h"
#include "BspObject.h"
#include "BspTool.h"
#include "BoundingBox.h"
#include "Plane.h"


BSPLIB_NAMESPACE_BEGIN


// file is read and parsed immediately after construction of an object --------
//
BspInput::BspInput( BspObjectList objectlist, const char *filename ) :
	SingleInput( objectlist, filename )
{
	ParseObjectData();
}


// destructor destroys only class members and base classes --------------------
//
BspInput::~BspInput()
{
}


// assignment copies only the BspFormat part of the object --------------------
//
BspInput& BspInput::operator =( const BspInput& copyobj )
{
	*(BspFormat *)this = copyobj;
	return *this;
}


// print error message if error in object data-file ---------------------------
//
void BspInput::ParseError( int section )
{
	//NOTE:
	// GetSectionName() is overloaded and not virtual, therefore
	// SingleInput's ParseError() cannot be used!
	sprintf( line, "%sin section %s (line %d)", parser_err_str,
			 GetSectionName( section ), m_parser_lineno );
	ErrorMessage( line );
	HandleCriticalError();
}


// correct mapping coordinates if object scale, translation, etc. applied -----
//
void BspInput::CorrectMappingCoordinates()
{
	FaceChunk& facelist = m_baseobject->getFaceList();
	int numfaces = facelist.getNumElements();
	for ( int i = 0; i < numfaces; i++ )
		if ( facelist[ i ].MappingAttached() )
			for ( int j = 0; j < 3; j++ ) {
				Vertex2 vtx = facelist[ i ].MapXY( j );
				vtx.setX( vtx.getX() - NewOriginX );
				vtx.setY( vtx.getY() - NewOriginY );
				vtx.setW( vtx.getW() - NewOriginZ );
				if ( PostProcessingFlags & FILTER_SCALE_FACTORS ) {
					vtx.setX( vtx.getX() * ObjScaleX );
					vtx.setY( vtx.getY() * ObjScaleY );
					vtx.setW( vtx.getW() * ObjScaleZ );
				}
				facelist[ i ].MapXY( j ) = vtx;
			}
}


// read polygon index list for current face -----------------------------------
//
void BspInput::ReadPolyIndxs( int& facesread )
{
    while ( m_scanptr != NULL ) {
		// only end-of-line or comment terminates index-list
        if ( *m_scanptr == ';' ) break;
		// read polygon id
		int polynumber;
		ReadIntParameter( polynumber, 1 );
		// find polygon referenced via its id
		Polygon *poly = m_baseobject->getPolygonList().FindPolygon( polynumber );

		if ( poly != NULL ) {
			if ( poly->getFaceId() != -1 )
				ParseError( m_section );
			// store faceindex into polygon (converted to 0-start)
			poly->setFaceId( facesread );
		} else {
			sprintf( line, "%s[Polygonindex invalid]: polygon %d in line %d\n",
					 parser_err_str, polynumber + 1, m_parser_lineno );
			ErrorMessage( line );
			HandleCriticalError();
		}

        m_scanptr = strtok( NULL, ",/ \t\n\r" );
    }

	facesread++;
}


// read face correspondences as direct three point mapping --------------------
//
void BspInput::ReadDirectCorrespondences( int& mappingsread )
{
	FaceChunk& facelist = m_baseobject->getFaceList();

	int texmapcount = 0;
	int corrfaceno  = 0;
	for ( corrfaceno = 0; corrfaceno < facelist.getNumElements(); corrfaceno++ ) {
		if ( facelist[ corrfaceno ].FaceTexMapped() )
			++texmapcount;
		if ( texmapcount > mappingsread )
			break;
	}

	if ( corrfaceno == facelist.getNumElements() ) {
		sprintf( line, "%s[Too many mappings specified]: line %d\n",
				 parser_err_str, m_parser_lineno );
		ErrorMessage( line );
		HandleCriticalError();
	}

	double x, y, z;
	ReadThreeDoubles( x, y, z );
	facelist[ corrfaceno ].MapXY( 0 ) = Vertex2( x, y, z );

	for ( int k = 1; k < 6; k++ ) {
		if ( m_input.ReadLine( line, TEXTLINE_MAX ) == NULL )
			ParseError( m_section );
		m_parser_lineno++;
		if ( ( m_scanptr = strtok( line, ",/ \t\n\r" ) ) == NULL )
			continue;
		ReadThreeDoubles( x, y, z );
		if ( k < 3 ) {
			facelist[ corrfaceno ].MapXY( k ) = Vertex2( x, y, z );
		} else {
			facelist[ corrfaceno ].MapUV( k - 3 ) = Vertex2( x, y, z );
		}
	}

	mappingsread++;
}


// read single node into bsp tree ---------------------------------------------
//
void BspInput::ReadBspTree()
{
	int polynumber	= -1;
	int clistindx	= 0;
	int blistindx	= 0;
	int fstindx		= 0;
	int bstindx		= 0;

	Plane *separator = NULL;
	BoundingBox *box = NULL;

	//NOTE:
	// m_scanptr points to "<nodenumber>:", but the
	// nodenumber contained in the file is ignored!

	m_scanptr = strtok( NULL, ", \t\n\r" );

	if ( isdigit( *m_scanptr ) ) {

		// read polygon number
		ReadIntParameter( polynumber, 1 );

		// read nodenumber of head of contained list
		m_scanptr = strtok( NULL, "|, \t\n\r" );
		ReadIntParameter( clistindx, 0 );

		// read nodenumber of head of backlist
		m_scanptr = strtok( NULL, "|, \t\n\r" );
		ReadIntParameter( blistindx, 0 );

		//NOTE:
		// the specification of a front- and back-subtree may be
		// omitted. empty subtrees (nil) will be assumed.

		// read index of front- and back-subtree if not contained node
		if ( ( m_scanptr = strtok( NULL, "|-, \t\n\r" ) ) != NULL ) {
			ReadIntParameter( fstindx, 0 );
			m_scanptr = strtok( NULL, "|-, \t\n\r" );
			ReadIntParameter( bstindx, 0 );
		}

	} else do {

		// read line specified in new format (key/value style)
		char *property = m_scanptr;
		int error = 1;
		if ( ( m_scanptr = strtok( NULL, " \t\n\r" ) ) != NULL ) {
			if ( stricmp( property, _bspspec_polygon_str ) == 0 ) {
				ReadIntParameter( polynumber, 1 );
				error = 0;
			} else if ( stricmp( property, _bspspec_frontlist_str ) == 0 ) {
				ReadIntParameter( clistindx, 0 );
				error = 0;
			} else if ( stricmp( property, _bspspec_backlist_str ) == 0 ) {
				ReadIntParameter( blistindx, 0 );
				error = 0;
			} else if ( stricmp( property, _bspspec_fronttree_str ) == 0 ) {
				ReadIntParameter( fstindx, 0 );
				error = 0;
			} else if ( stricmp( property, _bspspec_backtree_str ) == 0 ) {
				ReadIntParameter( bstindx, 0 );
				error = 0;
			} else if ( stricmp( property, _bspspec_plane_str ) == 0 ) {
				double x, y, z, d;
				ReadFourDoubles( x, y, z, d );
				Vertex3 normal( x, y, z );
				separator = new Plane( normal, d );
				error = 0;
			} else if ( stricmp( property, _bspspec_boundingbox_str ) == 0 ) {
				Vertex3 v1, v2;
				ReadSixDoubles( v1, v2 );
				box = new BoundingBox( v1, v2 );
				error = 0;
			}
		}
		if ( error ) {
			{
			StrScratch error;
			sprintf( error, "%s[Invalid property of bspnode]: %s (line %d)",
					 parser_err_str, property, m_parser_lineno );
			ErrorMessage( error );
			}
			HandleCriticalError();
		}

	} while ( ( m_scanptr = strtok( NULL, " \t\n\r" ) ) != NULL );

	// find polygon referenced via its id
	Polygon *poly = NULL;
	if ( polynumber >= 0 ) {
		poly = m_baseobject->getPolygonList().FindPolygon( polynumber );
		if ( poly == NULL ) {
			sprintf( line, "%s[Polygonindex in bsp tree invalid]: %d\n",
					 parser_err_str, polynumber + 1 );
			ErrorMessage( line );
			HandleCriticalError();
		}
	}

	// append new node to flat bsp tree
	BSPTreeFlat& flatbsp  = m_baseobject->getBSPTreeFlat();
	BSPNodeFlat *flatnode = flatbsp.AppendNode( fstindx, bstindx, poly, clistindx, blistindx );

	if ( separator != NULL ) {
		// attach separator plane
		flatnode->setSeparatorPlane( separator );
	}
	if ( box != NULL ) {
		// attach bounding box
		flatnode->setBoundingBox( box );
	}
}


// parse object data-file -----------------------------------------------------
//
int BspInput::ParseObjectData()
{
	int facedef_itemsread  = 0;
	int faceprop_itemsread = 0;
	int facenorm_itemsread = 0;
	int mappdef_itemsread  = 0;

	InfoMessage( "Processing input data file (format='BSP V1.1') ..." );

	// parse sections and read data -------------------------------
	m_section = _nil;
	while ( m_input.ReadLine( line, TEXTLINE_MAX ) != NULL ) {

		if ( ( m_parser_lineno++ & PARSER_DOT_SIZE ) == 0 )
			printf( "." ), fflush( stdout );

		if ( ( m_scanptr = strtok( line, "/, \t\n\r" ) ) == NULL )
			continue;
		else if ( *m_scanptr == ';' )
			continue;
		else if ( strnicmp( m_scanptr, "<end", 4 ) == 0 )
			break;
		else if ( *m_scanptr == '#' ) {
			if ( strncmp( m_scanptr, _bspsig_str, strlen( _bspsig_str ) ) == 0 ) {
				m_section = _comment;
			} else if ( ( m_section = GetSectionId( m_scanptr ) ) == _nil ) {
				{
				StrScratch error;
				sprintf( error, "%s[Undefined section-name]: %s (line %d)",
						 parser_err_str, m_scanptr, m_parser_lineno );
				ErrorMessage( error );
				}
				HandleCriticalError();
			}
		} else {
			switch ( m_section ) {

			// list of vertices ------------------------------------
			case _vertices :
				ReadVertex();
				break;

			// vertexnums of polygons ------------------------------
			case _polygons :
				ReadFace( FALSE );
				break;

			// definition of faces (consisting of polygons) --------
			case _faces :
				ReadPolyIndxs( facedef_itemsread );
				break;

			// bsptree ---------------------------------------------
			case _bsptree :
				ReadBspTree();
				break;

			// normals for face's planes ---------------------------
			case _facenormals :
				ReadFaceNormal( facenorm_itemsread );
				break;

			// properties of faces ---------------------------------
			case _faceproperties :
				ReadFaceProperties( faceprop_itemsread );
				break;

			// texture->face correspondences ----------------------
			case _correspondences :
				ReadDirectCorrespondences( mappdef_itemsread );
				break;

			// texturing data -----------------------------------
			case _textures :
				ReadTextures();
				break;

			// location of the object ---------------------------
			case _worldlocation :
				ReadWorldLocation();
				break;

			// location of camera -------------------------------
			case _camera :
				ReadCameraLocation();
				break;

			// filename of palette file -------------------------
			case _palette :
				ReadPaletteFilename();
				break;

			// scalefactors for object --------------------------
			case _scalefactors :
				ReadScaleFactors();
				break;

			// exchange command for axes ------------------------
			case _xchange :
				ReadXChangeCommand();
				break;

			// set new object origin ----------------------------
			case _setorigin :
				ReadOrigin();
				break;

			}
		}
	}

	// do post processing after parse
	CorrectMappingCoordinates();
	ApplyOriginTranslation();
	FilterAxesDirSwitch();
	FilterScaleFactors();
	FilterAxesExchange();
	EnforceMaximumExtents();
	m_baseobject->CheckParsedData();

	InfoMessage( "\nObject data ok.\n" );

	// do colorindex to rgb conversion
	ConvertColIndxs();

	return ( m_inputok = TRUE );
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
