//-----------------------------------------------------------------------------
//	BSPLIB HEADER: BspFormat.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _BSPFORMAT_H_
#define _BSPFORMAT_H_

// bsplib header files
#include "BspLibDefs.h"
#include "SingleFormat.h"


BSPLIB_NAMESPACE_BEGIN


// class containing bsp format specifics --------------------------------------
//
class BspFormat : public virtual SingleFormat {

protected:

	// section enumeration
	// (additional sections not supported by SingleFormat)
	enum {
		_bsptree = _num_single_sections,
		_polygons,

		_num_bsp_sections
	};

public:
	BspFormat() { }
	~BspFormat() { }

	BspFormat( const BspFormat& copyobj ) : SingleFormat( copyobj ) { }
	BspFormat& operator =( const BspFormat& copyobj );

protected:
	static int			GetSectionId( char *section_name );
	static const char*	GetSectionName( int section_id );

public:
	static const char	_bspspec_polygon_str[];
	static const char	_bspspec_frontlist_str[];
	static const char	_bspspec_backlist_str[];
	static const char	_bspspec_fronttree_str[];
	static const char	_bspspec_backtree_str[];
	static const char	_bspspec_plane_str[];
	static const char	_bspspec_boundingbox_str[];

protected:
	static const char	_bspsig_str[];
	static const char	BSP_FILE_EXTENSION[];

	static const char	_bsptree_str[];
	static const char	_polygons_str[];

	static const char*	section_strings[];
	static const int	section_ids[];
};

// assignment operator --------------------------------------------------------
inline BspFormat& BspFormat::operator =( const BspFormat& copyobj )
{
	*(SingleFormat *)this = copyobj;
	return *this;
}


BSPLIB_NAMESPACE_END


#endif // _BSPFORMAT_H_

