//-----------------------------------------------------------------------------
//	BSPLIB MODULE: Mapping.cpp
//
//  Copyright (c) 1996-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib headers
#include "Mapping.h"
#include "Chunk.h"


BSPLIB_NAMESPACE_BEGIN


// write mapping info in vertexindexes with mappings format -------------------
//
void Mapping::WriteMappingInfo( FILE *fp ) const
{
	// write list of vertex indexes
	int i = 0;
	for (  i = 0; i < numvertexindxs; i++ ) {
		fprintf( fp, ( i == numvertexindxs - 1 ) ?
			"%d\n" : "%d, ", facevertexindxs[ i ] + 1 );
	}

	// write (u,v) coordinates
	for ( i = 0; i < numvertexindxs; i++ ) {
		fprintf( fp, "%f, \t%f\n",
				mappingcoordinates[ i ].getX(),
				mappingcoordinates[ i ].getY() );
	}

	fprintf( fp, "\n" );
}


// error in search of vertices ------------------------------------------------
//
void Mapping::Error( int vertindx )
{
	char *line = new char[ 1024 ];
//	fprintf( stderr, "***ERROR*** Correspondence not found (%d)!\n", corrno + 1 );
	sprintf( line, "***ERROR*** Correspondence not found!\nsought vertex %d", vertindx + 1 );
	ErrorMessage( line );
	delete line;
/*
	fprintf( stderr, "list of vertices:\n" );
	for ( int i = 0; i < numvertexindxs; i++ )
		fprintf( stderr, "vertex %d\n", facevertexindxs[ i ] + 1 );
*/
	HandleCriticalError();
}


// size of mapping chunk ------------------------------------------------------
//
template <> const int ChunkRep<Mapping>::CHUNK_SIZE = 256;


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
