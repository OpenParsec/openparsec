//-----------------------------------------------------------------------------
//	BSPLIB MODULE: AodInput.cpp
//
//  Copyright (c) 1996-1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "AodInput.h"
#include "BspObject.h"
#include "BspTool.h"


BSPLIB_NAMESPACE_BEGIN


// file is read and parsed immediately after construction of an object --------
//
AodInput::AodInput( BspObjectList objectlist, const char *filename ) :
	SingleInput( objectlist, filename )
{
	ParseObjectData();
}


// destructor destroys only class members and the base classes ----------------
//
AodInput::~AodInput()
{
}


// assignment copies only the AodFormat part of the object --------------------
//
AodInput& AodInput::operator =( const AodInput& copyobj )
{
	*(AodFormat *)this = copyobj;
	return *this;
}


// print error message if error in object data-file ---------------------------
//
void AodInput::ParseError( int section )
{
	//NOTE:
	// GetSectionName() is overloaded and not virtual, therefore
	// SingleInput's ParseError() cannot be used!
	sprintf( line, "%sin section %s (line %d)", parser_err_str,
			 GetSectionName( section ), m_parser_lineno );
	ErrorMessage( line );
	HandleCriticalError();
}


// parse object data-file -----------------------------------------------------
//
int AodInput::ParseObjectData()
{
	int faceprop_itemsread = 0;
	int facenorm_itemsread = 0;

	InfoMessage( "Processing input data file (format='AOD V1.1') ..." );

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
			if ( strncmp( m_scanptr, _aodsig_str, strlen( _aodsig_str ) ) == 0 ) {
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

			// vertexnums of faces ---------------------------------
			case _faces :
				ReadFace( TRUE );
				break;

			// normals for face's planes ---------------------------
			case _facenormals :

				//NOTE:
				// face normals are currently ignored in the aod
				// file, since they are too inaccurate for later
				// BSP compilation purposes. if no normals are read
				// in, they will be calculated later on anyway.

				//ReadFaceNormal( facenorm_itemsread );
				break;

			// properties of faces ---------------------------------
			case _faceproperties :
				ReadFaceProperties( faceprop_itemsread );
				break;

			// texture->face correspondences ----------------------
			case _correspondences :
				ReadCorrespondences();
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

	// do post processing after parsing
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
