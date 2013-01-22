//-----------------------------------------------------------------------------
//	BSPLIB MODULE: ObjectAodFormat.cpp
//
//  Copyright (c) 1996-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "BspLibDefs.h"
#include "ObjectAodFormat.h"


BSPLIB_NAMESPACE_BEGIN


// write list of vertices -----------------------------------------------------
//
int ObjectAodFormat::WriteVertexList( FileAccess& output )
{
	if ( numvertices > 0 ) {

		output.WriteLine( "; list of all vertices comprising the object -----------------------------\n" );
		output.WriteLine( "; numbering starts with 1\n" );

		sprintf( line, "%s\n", _vertices_str );
		output.WriteLine( line );

		for ( int i = 0; i < numvertices; i++ ) {
			sprintf( line, "%f,\t%f,\t%f\t; %d\n",
					 vertexlist[ i ].getX(),
					 vertexlist[ i ].getY(),
					 vertexlist[ i ].getZ(),
					 i + 1 );
			output.WriteLine( line );
		}

		output.WriteLine( "\n" );
	}

	return output.Status();
}


// write list of faces --------------------------------------------------------
//
int ObjectAodFormat::WriteFaceList( FileAccess& output )
{
	if ( numpolygons > 0 ) {

		output.WriteLine( "; list of all faces ------------------------------------------------------\n" );
		output.WriteLine( "; numbering starts with 1\n" );

		sprintf( line, "%s\n", _faces_str );
		output.WriteLine( line );

		int i = 0;
		WriteFaceInfo( output, polygonlist.FetchHead(), i );
		output.WriteLine( "\n" );
	}

	return output.Status();
}


// write info about single face (vertexindexes comprising the face) -----------
//
void ObjectAodFormat::WriteFaceInfo( FileAccess& output, Polygon *poly, int& num )
{
#if 0
	if ( poly->getNext() )
		WriteFaceInfo( output, poly->getNext(), num );

	poly->WriteVertexList( output, FALSE );
	sprintf( line, "\t\t; %d\n", ++num );
	output.WriteLine( line );

#else
	// scan polygons according to face id
	Polygon *scan = NULL;
	for ( int facecount = 0; ; facecount++ ) {
		for ( scan = poly; scan; scan = scan->getNext() ) {
			if ( scan->getFaceId() == facecount )
				break;
		}
		if ( scan == NULL )
			break;
		scan->WriteVertexList( output, FALSE );
		sprintf( line, "\t\t; %d\n", ++num );
		output.WriteLine( line );
	}

	//NOTE:
	// if there is no exact 1:1 correspondence between
	// polygons and faces this will not work correctly!
	// since for each face only one polygon is written,
	// tessellated vrml cones and cylinders cannot be
	// correctly written to AOD files.

#endif
}


// write surface properties of faces ------------------------------------------
//
int ObjectAodFormat::WriteFaceProperties( FileAccess& output )
{
	if ( numfaces > 0 ) {

		output.WriteLine( "; face material properties (color, texture, etc.) ------------------------\n" );

		sprintf( line, "%s\n", _faceproperties_str );
		output.WriteLine( line );

		for ( int i = 0; i < numfaces; i++ )
			facelist[ i ].WriteFaceInfo( output );

		output.WriteLine( "\n" );
	}

	return output.Status();
}


// write face normals ---------------------------------------------------------
//
int ObjectAodFormat::WriteNormals( FileAccess& output )
{
	if ( numnormals > 0 ) {

		output.WriteLine( "; face normals -----------------------------------------------------------\n" );

		sprintf( line, "%s\n", _facenormals_str );
		output.WriteLine( line );

		for ( int i = 0; i < numfaces /*numnormals*/; i++ )
			facelist[ i ].WriteNormalInfo( output );

		output.WriteLine( "\n" );
	}

	return output.Status();
}


// write list of textures -----------------------------------------------------
//
int ObjectAodFormat::WriteTextureList( FileAccess& output )
{
	if ( numtextures > 0 ) {

		output.WriteLine( "; texture definitions (sizes and filenames) ------------------------------\n" );

		sprintf( line, "%s\n", _textures_str );
		output.WriteLine( line );

		for ( int i = 0; i < numtextures; i++ )
			texturelist[ i ].WriteInfo( output );
	}

	return output.Status();
}


// write list of mapping coordinates (correspondences) ------------------------
//
int ObjectAodFormat::WriteMappingList( FileAccess& output )
{
	if ( numtexmappedfaces > 0 ) {

		output.WriteLine( "; mapping parameters for textured faces ----------------------------------\n" );
		sprintf( line, "%s\n", _correspondences_str );
		output.WriteLine( line );
		output.WriteLine( "\n" );

		int curfaceno = 0;
		for ( int i = 0; i < numtexmappedfaces; i++, curfaceno++ ) {
			while ( !facelist[ curfaceno ].FaceTexMapped() )
				curfaceno++;
			sprintf( line, "; correspondence %d (face %d)\n", i + 1, curfaceno + 1 );
			output.WriteLine( line );

			if ( mappinglist.getNumElements() > i )
				mappinglist[ i ].WriteMappingInfo( output );
			else
				facelist[ curfaceno ].WriteMappingInfo( output );
		}
	}

	return output.Status();
}


// string scratchpad ----------------------------------------------------------
//
char ObjectAodFormat::line[ 1024 ] = "";


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
