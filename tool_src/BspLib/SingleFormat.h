//-----------------------------------------------------------------------------
//	BSPLIB HEADER: SingleFormat.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _SINGLEFORMAT_H_
#define _SINGLEFORMAT_H_

// bsplib header files
#include "BspLibDefs.h"
#include "SystemIO.h"
#include "Vertex.h"


BSPLIB_NAMESPACE_BEGIN


// baseclass for single format type file input/output -------------------------
//
class SingleFormatBase {

	friend class SingleFormat;

private:
	SingleFormatBase();
	~SingleFormatBase() { }

protected:
	char	xchangecmd[4];

	Vertex3 AxesDirSwitch;

	double	ObjScaleX;
	double	ObjScaleY;
	double	ObjScaleZ;

	double	NewOriginX;
	double	NewOriginY;
	double	NewOriginZ;

	int		worldlocation_set;
	int		camera_set;
	int		xchange_set;
	int		scalefactors_set;
	int		setorigin_set;

	hprec_t	wm[16];	// world matrix
	hprec_t	vm[16];	// view matrix
};

class SingleFormat : public SingleFormatBase {

protected:

	// section enumeration
	enum {
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
		_facenormals,

		_num_single_sections
	};

public:
	SingleFormat();
	~SingleFormat() { }

	SingleFormat( const SingleFormat& copyobj );
	SingleFormat& operator =( const SingleFormat& copyobj );

protected:
	String				m_palette_fname;

protected:
	static int			GetSectionId( char *section_name );
	static const char*	GetSectionName( int section_id );

protected:
	static const double PREMOVEORIGIN_X;
	static const double PREMOVEORIGIN_Y;
	static const double PREMOVEORIGIN_Z;

	static const char	_vertices_str[];
	static const char	_faces_str[];
	static const char	_faceproperties_str[];
	static const char	_correspondences_str[];
	static const char	_textures_str[];
	static const char	_worldlocation_str[];
	static const char	_camera_str[];
	static const char	_comment_str[];
	static const char	_palette_str[];
	static const char	_scalefactors_str[];
	static const char	_xchange_str[];
	static const char	_setorigin_str[];
	static const char	_facenormals_str[];

	static const char*	section_strings[];
	static const int	section_ids[];
};


BSPLIB_NAMESPACE_END


#endif // _SINGLEFORMAT_H_

