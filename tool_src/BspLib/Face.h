//-----------------------------------------------------------------------------
//	BSPLIB HEADER: Face.h
//
//  Copyright (c) 1996-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _FACE_H_
#define _FACE_H_

// bsplib header files
#include "BspLibDefs.h"
#include "Chunk.h"
#include "Material.h"
#include "Plane.h"
#include "SystemIO.h"
#include "Texture.h"
#include "TriMapping.h"
#include "Vertex.h"


BSPLIB_NAMESPACE_BEGIN


// class describing a face (mainly in terms of its surface properties) --------
//
class Face : public virtual SystemIO {

	void	Error() const;

public:

	// shading types for faces
	enum ShadingType {
		no_shad			= 0x1000,	// fixed color (lighting independent ambient color)
		flat_shad		= 0x1001,	// flat shading
		gouraud_shad	= 0x1002,	// gouraud shading
		afftex_shad		= 0x2003,	// affine texture mapping
		ipol1tex_shad	= 0x2004,	// first order interpolated texture mapping (lin)
		ipol2tex_shad	= 0x2005,	// second order interpolated texture mapping (quad)
		persptex_shad	= 0x2006,	// perspective correct mapping without any interpolation
		material_shad	= 0x1007,	// use material specification
		texmat_shad		= 0x3008,	// textures modulated by material specification
		num_shading_types = 9,		// MUST SET THIS MANUALLY!!
		base_mask		= 0x00ff,	// mask for basic shading type
		color_mask		= 0x1000,	// mask to denote faces with attached color
		texmap_mask		= 0x2000,	// mask to denote texture mapped faces
	};

	// color model identifiers
	enum ColorModel {
		no_col,					// shading type has no associated color
		indexed_col,			// indexed color (color look up table index)
		rgb_col,				// color specified directly via separate channels
		material_col,			// use material specification
		num_color_models
	};

public:
	Face();
	~Face() { delete faceplane; delete facematerial; delete facemapping; delete texturename; }

	Face( const Face& copyobj );
	Face& operator =( const Face& copyobj );

	int			getId() const { return faceid; }
	void		setId( int id ) { faceid = id; }

	Material	getMaterial() const { if ( facematerial == NULL ) Error(); return *facematerial; }
	Plane		getPlane() const { if ( faceplane == NULL ) Error(); return *faceplane; }

	Vector3		getPlaneNormal() const;
	void		setPlaneNormal( const Vector3& normal );

	int			getShadingType() const { return shadingtype; }
	void		setShadingType( int type );

	int			getColorType() const { return colortype; }
	void		getColorIndex( dword& col ) const { col = facecolor_indx; }
	void		getColorRGBA( ColorRGBA& col ) const { col = facecolor_rgba; }
	void		getColorChannels( float& r, float& g, float& b ) const;
	const char *getTextureName() { return texturename; }

	void		setFaceColor( dword col );
	void		setFaceColor( ColorRGBA col );
	void		setTextureName( const char *tname );

	int			NormalValid() const { return faceplane ? faceplane->NormalValid() : FALSE; }
	int			PlaneValid() const { return faceplane ? faceplane->PlaneValid() : FALSE; }
	int			MaterialAttached() const { return ( facematerial != NULL ); }
	int			MappingAttached() const { return ( facemapping != NULL ); }
	int			FaceTexMapped() const { return ( ( shadingtype & texmap_mask ) != 0 ); }

	void		CalcPlane( const Vertex3& vertex1, const Vertex3& vertex2, const Vertex3& vertex3 );
	void		AttachNormal( const Vector3& normal );
	void		AttachPlane( Plane *plane );
	void		AttachMaterial( Material *mat );

	int			ConvertColorIndexToRGB( char *palette, int changemode );

	Vertex2&	MapXY( int indx );
	Vertex2&	MapUV( int indx );

	void		WriteFaceInfo( FILE *fp ) const;
	void		WriteNormalInfo( FILE *fp ) const;
	void		WriteMappingInfo( FILE *fp ) const;

public:
	static int	GetTypeIndex( const char *type );
	static int	GetColorModelIndex( const char *type );

public:
	static const char*	material_strings[];	// strings for material specification

private:
	static const char*	prop_strings[];		// strings for shading types
	static const int	prop_ids[];			// ids corresponding to strings
	static const char*	color_strings[];	// strings for color models
	static const int	color_ids[];		// ids corresponding to strings

private:
	int			faceid;			// face's object-globally unique id
	Plane*		faceplane;		// face's plane (normal vector and offset)

	int			shadingtype;	// shading type, determines how face is rendered
	int			colortype;		// color type, determines how color is specified

	dword		facecolor_indx;	// face's color represented as color index
	ColorRGBA	facecolor_rgba;	// face's color represented as rgba quadruplet
	Material*	facematerial;	// OpenGL-style material properties
	TriMapping*	facemapping;	// mapping definition for this face
	char*		texturename;	// name of texture if any
};

// typedef chunk of faces -----------------------------------------------------
typedef Chunk<Face> FaceChunk;

// construct a Face -----------------------------------------------------------
inline Face::Face()
{
	faceid		 = -1;		// -1 means id invalid
	shadingtype  = -1;		// -1 means type invalid
	colortype	 = -1;		// -1 means type invalid
	faceplane	 = NULL;	// face's plane not calculated
	facematerial = NULL;	// no material attached
	facemapping	 = NULL;	// no mapping attached
	texturename	 = NULL;	// no texture name
}

// get face's normal vector ---------------------------------------------------
inline Vector3 Face::getPlaneNormal() const
{
	CHECK_DEREFERENCING(
		if ( ( faceplane == NULL ) || !faceplane->NormalValid() )
			Error();
	);
	return faceplane->getPlaneNormal();
}

// set face's normal vector ---------------------------------------------------
inline void Face::setPlaneNormal( const Vector3& normal )
{
	CHECK_DEREFERENCING(
		if ( ( faceplane == NULL ) || !faceplane->NormalValid() )
			Error();
	);
	faceplane->setPlaneNormal( normal );
}

// set shading type -----------------------------------------------------------
inline void Face::setShadingType( int type )
{
	CHECK_DEREFERENCING(
		if ( ( type & base_mask ) >= num_shading_types )
			Error();
	);
	shadingtype = type;
}

// set color specified in terms of a color index ------------------------------
inline void Face::setFaceColor( dword col )
{
	facecolor_indx	= col;
	colortype		= indexed_col;
}

// set color specified in terms of (r,g,b,a) ----------------------------------
inline void Face::setFaceColor( ColorRGBA col )
{
	facecolor_rgba	= col;
	colortype		= rgb_col;
}

// fetch rgb color channels converted to [0.0, 1.0] range ---------------------
inline void Face::getColorChannels( float& r, float& g, float& b ) const
{
	r = facecolor_rgba.R / 255.0f;
	g = facecolor_rgba.G / 255.0f;
	b = facecolor_rgba.B / 255.0f;
}

// return mapping coordinates in (x,y)-domain ---------------------------------
inline Vertex2& Face::MapXY( int indx )
{
	if ( facemapping == NULL )
		facemapping = new TriMapping;
	return facemapping->getMapXY( indx );
}

// return mapping coordinates in (u,v)-domain ---------------------------------
inline Vertex2& Face::MapUV( int indx )
{
	if ( facemapping == NULL )
		facemapping = new TriMapping;
	return facemapping->getMapUV( indx );
}


BSPLIB_NAMESPACE_END


#endif // _FACE_H_

