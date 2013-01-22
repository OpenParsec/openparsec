//-----------------------------------------------------------------------------
//	BSPLIB HEADER: VertexChunk.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _VERTEXCHUNK_H_
#define _VERTEXCHUNK_H_

// bsplib header files
#include "BspLibDefs.h"
#include "SystemIO.h"


BSPLIB_NAMESPACE_BEGIN


// chunk of 3-D vertices (actual representation) ------------------------------
//
class VertexChunkRep : public virtual SystemIO {

	friend class VertexChunk;

private:
	VertexChunkRep( int chunksize = 0 );
	~VertexChunkRep();

	Vertex3&		FetchVertex( int index );
	int				AddVertex( Vertex3 vertex );
	int				FindVertex( const Vertex3& vertex );
	int				FindCloseVertex( const Vertex3& vertex, int skip = -1 );
	int				CheckVertices( int verbose );

	int				getNumElements() const;

private:
	void			ChangeAxis( char xchar, hprec_t src );
	int				VerticesEqual( Vertex3 v1, Vertex3 v2 );

private:
	int				ref_count;

	int				numvertices;
	int				maxnumvertices;
	Vertex3*		vertices;
	VertexChunkRep*	next;

	int				nummultvertices;
	static int		eliminatedoublets;
	static const int CHUNK_SIZE;
};

// create new vertex chunk ----------------------------------------------------
inline VertexChunkRep::VertexChunkRep( int chunksize ) : ref_count( 0 )
{
	int challocsize	= chunksize > 0 ? chunksize : CHUNK_SIZE;
	vertices		= new Vertex3[ challocsize ];
	maxnumvertices	= challocsize;
	numvertices		= 0;
	next			= NULL;
}

// destroy list of vertex chunks recursively ----------------------------------
inline VertexChunkRep::~VertexChunkRep()
{
	delete next;
	delete[] vertices;
}

// check if two vertices are equal --------------------------------------------
inline int VertexChunkRep::VerticesEqual( Vertex3 v1, Vertex3 v2 )
{
//	return v1.IsInVicinity( v2 );
	return ( v1 == v2 );
}


// chunk of 3-D vertices (handle class) ---------------------------------------
//
class VertexChunk {

public:
	VertexChunk( int chunksize = 0 ) { rep = new VertexChunkRep( chunksize ); rep->ref_count = 1; }
	~VertexChunk() { if ( --rep->ref_count == 0 ) delete rep; }

	VertexChunk( const VertexChunk& copyobj );
	VertexChunk& operator =( const VertexChunk& copyobj );

	Vertex3& operator []( int index ) { return rep->FetchVertex( index ); }
	Vertex3& FetchVertex( int index ) { return rep->FetchVertex( index ); }

	int	AddVertex( const Vertex3 vertex ) { return rep->AddVertex( vertex ); }
	int	FindVertex( const Vertex3& vertex ) { return rep->FindVertex( vertex ); }
	int	FindCloseVertex( Vertex3 vertex, int skip = -1 ) { return rep->FindCloseVertex( vertex, skip ); }
	int	CheckVertices( int verbose ) { return rep->CheckVertices( verbose ); }

	int	getNumElements() const { return rep->getNumElements(); }

private:
	VertexChunkRep*	rep;
};

// copy constructor for VertexChunk -------------------------------------------
inline VertexChunk::VertexChunk( const VertexChunk& copyobj )
{
	rep = copyobj.rep;
	rep->ref_count++;
}

// assignment operator for VertexChunk ----------------------------------------
inline VertexChunk& VertexChunk::operator =( const VertexChunk& copyobj )
{
	if ( &copyobj != this ) {
		if ( --rep->ref_count == 0 ) {
			delete rep;
		}
		rep = copyobj.rep;
		rep->ref_count++;
	}
	return *this;
}


BSPLIB_NAMESPACE_END


#endif // _VERTEXCHUNK_H_

