//-----------------------------------------------------------------------------
//	BSPLIB HEADER: Mapping.h
//
//  Copyright (c) 1996-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _MAPPING_H_
#define _MAPPING_H_

// bsplib header files
#include "BspLibDefs.h"
#include "Chunk.h"
#include "SystemIO.h"
#include "Vertex.h"


BSPLIB_NAMESPACE_BEGIN


#define V_FACEVTXMAX 32 // vertices per face (actually, per polygon)


// class that describes mapping for face with many vertices -------------------
//
class Mapping : public virtual SystemIO {

	void		Error( int vertindx );

public:
	Mapping() { numvertexindxs = 0; }
	~Mapping() { }

	void		InsertFaceVertex( int vindx ) { facevertexindxs[ numvertexindxs++ ] = vindx; }
	void		SetCorrespondence( int vindx, Vertex2 vertex );
	Vertex2		FetchMapPoint( int vertindx );

	int			getNumVertices() { return numvertexindxs; }
	void		setNumVertices( int num ) { numvertexindxs = num; }
	void		setMappingCoordinates( int k, const Vertex2& vertex);

	void		WriteMappingInfo( FILE *fp ) const;

private:
	int			numvertexindxs;
	int			facevertexindxs[ V_FACEVTXMAX ];
	Vertex2		mappingcoordinates[ V_FACEVTXMAX ];
};

// typedef chunk of mappings --------------------------------------------------
typedef Chunk<Mapping> MappingChunk;

// set mapping coordinates ----------------------------------------------------
inline void Mapping::setMappingCoordinates( int k, const Vertex2& vertex)
{
	if ( k < V_FACEVTXMAX )
		mappingcoordinates[ k ] = vertex;
	else
		Error( k );
}

// set single correspondence -------------------------------------------------
inline void Mapping::SetCorrespondence( int vindx, Vertex2 vertex )
{
	if ( numvertexindxs < V_FACEVTXMAX ) {
		facevertexindxs[ numvertexindxs ] = vindx;
		mappingcoordinates[ numvertexindxs ] = vertex;
		numvertexindxs++;
	} else {
		Error( numvertexindxs );
	}
}

// fetch corresponding mapping to face vertex ---------------------------------
inline Vertex2 Mapping::FetchMapPoint( int vertindx )
{
	for ( int i = 0; i < numvertexindxs; i++ )
		if ( facevertexindxs[ i ] == vertindx )
			return mappingcoordinates[ i ];
	Error( vertindx );
	return Vertex2( 0, 0 );	// never reached
}


BSPLIB_NAMESPACE_END


#endif // _MAPPING_H_

