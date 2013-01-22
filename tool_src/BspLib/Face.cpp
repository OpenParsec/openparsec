//-----------------------------------------------------------------------------
//	BSPLIB MODULE: Face.cpp
//
//  Copyright (c) 1996-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib headers
#include "Face.h"
#include "Chunk.h"


BSPLIB_NAMESPACE_BEGIN


// copy constructor for Face --------------------------------------------------
//
Face::Face( const Face& copyobj )
{
	faceid			= copyobj.faceid;
	facecolor_indx	= copyobj.facecolor_indx;
	facecolor_rgba	= copyobj.facecolor_rgba;
	shadingtype		= copyobj.shadingtype;
	colortype		= copyobj.colortype;

	faceplane		= copyobj.faceplane ? new Plane( *copyobj.faceplane ) : NULL;
	facematerial	= copyobj.facematerial ? new Material( *copyobj.facematerial ) : NULL;
	facemapping		= copyobj.facemapping ? new TriMapping( *copyobj.facemapping ) : NULL;
	texturename		= copyobj.texturename ? new char[ strlen( copyobj.texturename ) + 1 ] : NULL;

	if ( texturename != NULL )
		strcpy( texturename, copyobj.texturename );
}


// assignment operator for Face -----------------------------------------------
//
Face& Face::operator =( const Face& copyobj )
{
	if ( &copyobj != this ) {
		faceid			= copyobj.faceid;
		facecolor_indx	= copyobj.facecolor_indx;
		facecolor_rgba	= copyobj.facecolor_rgba;
		shadingtype		= copyobj.shadingtype;
		colortype		= copyobj.colortype;

		delete faceplane;
		delete facematerial;
		delete facemapping;
		delete texturename;

		faceplane		= copyobj.faceplane ? new Plane( *copyobj.faceplane ) : NULL;
		facematerial	= copyobj.facematerial ? new Material( *copyobj.facematerial ) : NULL;
		facemapping		= copyobj.facemapping ? new TriMapping( *copyobj.facemapping ) : NULL;
		texturename		= copyobj.texturename ? new char[ strlen( copyobj.texturename ) + 1 ] : NULL;

		if ( texturename != NULL )
			strcpy( texturename, copyobj.texturename );
	}
	return *this;
}


// set name of texture for this face ------------------------------------------
//
void Face::setTextureName( const char *tname )
{
	delete texturename;
	texturename = tname ? new char[ strlen( tname ) + 1 ] : NULL;
	if ( texturename != NULL )
		strcpy( texturename, tname );
	colortype = no_col;
}


// calculate face's plane (normal and offset) and attach it -------------------
//
void Face::CalcPlane( const Vertex3& vertex1, const Vertex3& vertex2, const Vertex3& vertex3 )
{
	if ( faceplane && faceplane->PlaneValid() )
		return;
	if ( faceplane && faceplane->NormalValid() ) {
		// if normal already valid only recalculate offset
		faceplane->CalcPlaneOffset( vertex1 );
		return;
	}
	// calculate completely new plane
	delete faceplane;
	faceplane = new Plane( vertex1, vertex2, vertex3 );

	if ( !faceplane->PlaneValid() ) {
		ErrorMessage( "\n***ERROR*** Collinear vertices encountered (BspLib::Face)." );
		HandleCriticalError();
	}
}


// attach normal after creating plane to store it in --------------------------
//
void Face::AttachNormal( const Vector3& normal )
{
	delete faceplane;
	faceplane = new Plane( normal );
}


// attach face's plane calculated somewhere else ------------------------------
//
void Face::AttachPlane( Plane *plane )
{
	if ( plane != NULL ) {
		delete faceplane;
		faceplane = plane;
	}
}

// attach material and set colortype accordingly ------------------------------
//
void Face::AttachMaterial( Material *mat )
{
	if ( mat != NULL ) {
		delete facematerial;
		facematerial = mat;
		colortype	 = material_col;
	}
}


// convert color index to RGB triplet using 256 color palette -----------------
//
int Face::ConvertColorIndexToRGB( char *palette, int changemode )
{
	if ( colortype == indexed_col ) {
		byte *palentry  = (byte *) palette + facecolor_indx * 3;
		float oomax = 255.0f / 63.0f;
		facecolor_rgba.R = (byte) ( palentry[ 0 ] * oomax );
		facecolor_rgba.G = (byte) ( palentry[ 1 ] * oomax );
		facecolor_rgba.B = (byte) ( palentry[ 2 ] * oomax );
		facecolor_rgba.A = 255;
		if ( changemode )
			colortype = rgb_col;
		return TRUE;
	}

	return FALSE;
}


// helper function ------------------------------------------------------------
//
inline void WriteRGBA( FILE *fp, const char *desc, ColorRGBA col )
{
	fprintf( fp, "%s %f %f %f %f ", desc,
			 col.R / 255.0f, col.G / 255.0f,
			 col.B / 255.0f, col.A / 255.0f );
}


// write face info to text-file -----------------------------------------------
//
void Face::WriteFaceInfo( FILE *fp ) const
{
	// write shading type specifier
	fprintf( fp, "%s\t", prop_strings[ shadingtype & base_mask ] );

	// write color if any attached and valid
	if ( shadingtype & color_mask ) {
		if ( colortype == indexed_col ) {
			// write color index
			fprintf( fp, "%s %d", color_strings[ colortype ], facecolor_indx );
		} else if ( colortype == rgb_col ) {
			// write (r,g,b) triplet
			fprintf( fp, "%s %f %f %f", color_strings[ colortype ],
										facecolor_rgba.R / 255.0f,
										facecolor_rgba.G / 255.0f,
										facecolor_rgba.B / 255.0f );
		} else if ( colortype == material_col ) {
			if ( facematerial != NULL ) {
				fprintf( fp, "%s ", color_strings[ colortype ] );
				ColorRGBA colquad;
				colquad = facematerial->getAmbientColor();
				WriteRGBA( fp, material_strings[ 0 ], colquad );
				colquad = facematerial->getDiffuseColor();
				WriteRGBA( fp, material_strings[ 1 ], colquad );
				colquad = facematerial->getSpecularColor();
				WriteRGBA( fp, material_strings[ 2 ], colquad );
				colquad = facematerial->getEmissiveColor();
				WriteRGBA( fp, material_strings[ 3 ], colquad );
				fprintf( fp, "%s %f ", material_strings[ 4 ], facematerial->getShininess() );
				fprintf( fp, "%s %f", material_strings[ 5 ], facematerial->getTransparency() );
			} else {
				fprintf( fp, "**INVALID MATERIAL**" );
			}
		}
	}

	// write texture info if any attached and valid
	if ( shadingtype & texmap_mask ) {
		if ( texturename != NULL )
			fprintf( fp, "\"%s\"", texturename );
		else
			fprintf( fp, "**INVALID TEXTURE NAME**" );
	}

	// write face id as comment
	fprintf( fp, "\t; %d\n", faceid + 1 );
}


// write facenormal to text-file ----------------------------------------------
//
void Face::WriteNormalInfo( FILE *fp ) const
{
	// write face's normal vector
	if ( faceplane != NULL ) {
		Vector3 normal = faceplane->getPlaneNormal();
		fprintf( fp, "%f,\t%f,\t%f\t\t; %d\n",
				 normal.getX(),
				 normal.getY(),
				 normal.getZ(),
				 faceid + 1 );
	} else {
		// normal vector invalid (no plane attached)
		fprintf( fp, "Normal of face #%d invalid.\n", faceid + 1 );
	}
}


// write mapping info to text-file --------------------------------------------
//
void Face::WriteMappingInfo( FILE *fp ) const
{
	// write three point correspondences for mapping
	if ( facemapping != NULL ) {
		// write (x,y) domain
		int i = 0 ;
		for (  i = 0; i < 3; i++ ) {
			Vector2 vertex = facemapping->getMapXY( i );
			fprintf( fp, "%f,\t%f,\t%f\n",
					 vertex.getX(),
					 vertex.getY(), 
					 vertex.getW() );
		}
		// write (u,v) domain
		for ( i = 0; i < 3; i++ ) {
			Vector2 vertex = facemapping->getMapUV( i );
			fprintf( fp, "%f,\t%f,\t%f\n",
					 vertex.getX(),
					 vertex.getY(), 
					 vertex.getW() );
		}
		fprintf( fp, "\n" );
	} else {
		// face mapping invalid (no affine mapping attached)
		fprintf( fp, "Mapping for face #%d invalid.\n\n", faceid + 1 );
	}
}


// unspecified error encountered ----------------------------------------------
//
void Face::Error() const
{
	ErrorMessage( "\n***ERROR*** in object of class BspLib::Face." );
	HandleCriticalError();
}


// get index to type specified as string (this function is static!) -----------
//
int Face::GetTypeIndex( const char *type )
{
	for ( int i = 0; i < num_shading_types; i++ )
		if ( stricmp( type, prop_strings[ i ] ) == 0 )
			return prop_ids[ i ];

	// -1 means type-string invalid, so no valid index can be returned
	return -1;
}


// get index to color model specified as string (this function is static!) ----
//
int Face::GetColorModelIndex( const char *type )
{
	for ( int i = 0; i < num_color_models; i++ )
		if ( stricmp( type, color_strings[ i ] ) == 0 )
			return color_ids[ i ];

	// -1 means type-string invalid, so no valid index can be returned
	return -1;
}


// strings for specification of face shading type -----------------------------
//
const char *Face::prop_strings[] = {
	"no_shad",
	"flat_shad",
	"gouraud_shad",
	"afftex_shad",
	"ipol1tex_shad",
	"ipol2tex_shad",
	"persptex_shad",
	"material_shad",
	"texmat_shad",
};

const int Face::prop_ids[] = {
	no_shad,
	flat_shad,
	gouraud_shad,
	afftex_shad,
	ipol1tex_shad,
	ipol2tex_shad,
	persptex_shad,
	material_shad,
	texmat_shad,
};

const char *Face::color_strings[] = {
	"no_col",
	"indexed_col",
	"rgb_col",
	"material_col",
};

const int Face::color_ids[] = {
	no_col,
	indexed_col,
	rgb_col,
	material_col,
};

const char *Face::material_strings[] = {
	"ambient_col",
	"diffuse_col",
	"specular_col",
	"emissive_col",
	"shininess",
	"transparency",
};


// base size of face chunk ----------------------------------------------------
//
template <> const int ChunkRep<Face>::CHUNK_SIZE = 512;


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
