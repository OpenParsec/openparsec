//-----------------------------------------------------------------------------
//	BSPLIB MODULE: BspFormat.cpp
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "BspFormat.h"


BSPLIB_NAMESPACE_BEGIN


// get index to section id specified as string (this function is static!) -----
//
int BspFormat::GetSectionId( char *section_name ) 
{ 
	int sectionid = SingleFormat::GetSectionId( section_name );
	if ( sectionid == _nil ) {
		for ( int i = 0; i < _num_bsp_sections - _num_single_sections; i++ )
			if ( stricmp( section_name, section_strings[ i ] ) == 0 )
				return section_ids[ i ];
	}
	return sectionid;
}


// get name of section specified via id (this function is static!) ------------
//
const char *BspFormat::GetSectionName( int section_id )
{
	const char *sectionname = SingleFormat::GetSectionName( section_id );
	if ( sectionname == NULL ) {
		return ( section_id < _num_bsp_sections ) ?
				section_strings[ section_id - _num_single_sections ] : NULL;
	}
	return sectionname;
}


// BspFormat specific static variables ----------------------------------------
//
const char BspFormat::_bspsig_str[]					= "#BSP";
const char BspFormat::BSP_FILE_EXTENSION[]			= ".bsp";

const char BspFormat::_bspspec_polygon_str[]		= "polygon";
const char BspFormat::_bspspec_frontlist_str[]		= "frontlist";
const char BspFormat::_bspspec_backlist_str[]		= "backlist";
const char BspFormat::_bspspec_fronttree_str[]		= "fronttree";
const char BspFormat::_bspspec_backtree_str[]		= "backtree";
const char BspFormat::_bspspec_plane_str[]			= "plane";
const char BspFormat::_bspspec_boundingbox_str[]	= "boundingbox";

const char BspFormat::_bsptree_str[]				= "#bsptree";
const char BspFormat::_polygons_str[] 				= "#polygons";

// additional section names table
const char *BspFormat::section_strings[] = {
	_bsptree_str,
	_polygons_str
};
const int BspFormat::section_ids[] = {
	_bsptree,
	_polygons
};


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
