//-----------------------------------------------------------------------------
//	BSPLIB MODULE: SingleFormat.cpp
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "SingleFormat.h"


BSPLIB_NAMESPACE_BEGIN


// construct class SingleFormat by initializing Single specific members -------------
//
inline SingleFormatBase::SingleFormatBase()
{
	strcpy( xchangecmd, "xyz" );

	AxesDirSwitch = Vertex3( 1.0, 1.0, 1.0 );

	ObjScaleX	= 1.0;
	ObjScaleY	= 1.0;
	ObjScaleZ	= 1.0;

	NewOriginX	= 0.0;
	NewOriginY	= 0.0;
	NewOriginZ	= 0.0;

	worldlocation_set	= FALSE;
	camera_set			= FALSE;
	xchange_set			= FALSE;
	scalefactors_set	= FALSE;
	setorigin_set		= FALSE;

	// init default world and view matrix
	for ( int j = 0; j < 4; j++ ) {
		for ( int i = 0; i < 4; i++ ) {
			int indx = j * 4 + i;
			vm[ indx ] = wm[ indx ] = ( j == i ) ? 1.0 : 0.0;
		}
	}
}


// constructor for SingleFormat -----------------------------------------------
//
SingleFormat::SingleFormat()
{
	// all initializations are done
	// in SingleFormatBase::SingleFormatBase()
}


// copy constructor for SingleFormat ------------------------------------------
//
SingleFormat::SingleFormat( const SingleFormat& copyobj ) :
	SingleFormatBase( copyobj ),
	m_palette_fname( copyobj.m_palette_fname )
{
}


// assignment operator for SingleFormat ---------------------------------------
//
SingleFormat& SingleFormat::operator =( const SingleFormat& copyobj )
{
	if ( &copyobj != this ) {
		*(SingleFormatBase *)this = copyobj;
		m_palette_fname = copyobj.m_palette_fname;
	}
	return *this;
}


// get index to section id specified as string (this function is static!) -----
//
int SingleFormat::GetSectionId( char *section_name )
{
	for ( int i = _nil + 1; i < _num_single_sections; i++ )
		if ( stricmp( section_name, section_strings[ i ] ) == 0 )
			return section_ids[ i ];

	// _nil means section name invalid, so no valid index can be returned
	return _nil;
}


// get name of section specified via id (this function is static!) ------------
//
const char *SingleFormat::GetSectionName( int section_id )
{
	return ( section_id < _num_single_sections ) ? section_strings[ section_id ] : NULL;
}


// SingleFormat specific static variables -------------------------------------
//

// origin translation automatically applied to all objects
const double SingleFormat::PREMOVEORIGIN_X = 0.0;
const double SingleFormat::PREMOVEORIGIN_Y = 0.0;
const double SingleFormat::PREMOVEORIGIN_Z = 0.0;

// section names
const char SingleFormat::_vertices_str[] 		= "#vertices";
const char SingleFormat::_faces_str[]			= "#faces";
const char SingleFormat::_faceproperties_str[]	= "#faceproperties";
const char SingleFormat::_correspondences_str[]	= "#correspondences";
const char SingleFormat::_textures_str[] 		= "#textures";
const char SingleFormat::_worldlocation_str[]	= "#worldlocation";
const char SingleFormat::_camera_str[]			= "#camera";
const char SingleFormat::_comment_str[]			= "#comment";
const char SingleFormat::_palette_str[]			= "#palette";
const char SingleFormat::_scalefactors_str[]	= "#scalefactors";
const char SingleFormat::_xchange_str[]			= "#xchange";
const char SingleFormat::_setorigin_str[]		= "#setorigin";
const char SingleFormat::_facenormals_str[]		= "#facenormals";

// section name table
const char *SingleFormat::section_strings[] = {
	NULL,
	_vertices_str,
	_faces_str,
	_faceproperties_str,
	_correspondences_str,
	_textures_str,
	_worldlocation_str,
	_camera_str,
	_comment_str,
	_palette_str,
	_scalefactors_str,
	_xchange_str,
	_setorigin_str,
	_facenormals_str
};
const int SingleFormat::section_ids[] = {
	_nil,
	_vertices,
	_faces,
	_faceproperties,
	_correspondences,
	_textures,
	_worldlocation,
	_camera,
	_comment,
	_palette,
	_scalefactors,
	_xchange,
	_setorigin,
	_facenormals
};


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
