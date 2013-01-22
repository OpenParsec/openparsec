//-----------------------------------------------------------------------------
//	BSPLIB MODULE: VertexChunk.cpp
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib headers
#include "Vertex.h"


// if this flag is set storage is increased by using lists.
// this can be very inefficient and should only be used for
// testing purposes!
// if not set, storage is expanded in realloc() style.
//#define EXPAND_STORAGE_WITH_LISTS

// if this flag is set storage is always increased two-fold
#define EXPAND_EXPONENTIALLY


BSPLIB_NAMESPACE_BEGIN


// return number of vertices in entire vertex chunk list ----------------------
//
int VertexChunkRep::getNumElements() const
{
	int num = 0;
	for ( const VertexChunkRep *vlist = this; vlist; vlist = vlist->next )
		num += vlist->numvertices;
	return num;
}


// fetch index to specified vertex if vertex already exists -------------------
//
int VertexChunkRep::FindVertex( const Vertex3& vertex )
{
	int numskipped = 0;

	VertexChunkRep *vlist = this;
	while ( ( vlist != NULL ) && ( vlist->numvertices > 0 ) ) {
		for ( int i = 0; i < vlist->numvertices; i++ )
			if ( vertices[ i ] == vertex )
				return i + numskipped;
		numskipped += vlist->numvertices;
		vlist		= vlist->next;
	}

	// vertex not found
	return -1;
}


// fetch vertex-index if close enough vertex already exists -------------------
//
int VertexChunkRep::FindCloseVertex( const Vertex3& vertex, int skip )
{
	int numskipped = 0;

	VertexChunkRep *vlist = this;
	while ( ( vlist != NULL ) && ( vlist->numvertices > 0 ) ) {
		for ( int i = 0; i < vlist->numvertices; i++ )
			if ( ( i + numskipped > skip ) && vertices[ i ].IsInVicinity( vertex ) )
				return i + numskipped;
		numskipped += vlist->numvertices;
		vlist		= vlist->next;
	}

	// vertex not found
	return -1;
}


// fetch vertex corresponding to specific index -------------------------------
//
Vertex3& VertexChunkRep::FetchVertex( int index )
{

#ifdef EXPAND_STORAGE_WITH_LISTS

	VertexChunkRep *vlist = this;
	while ( ( vlist != NULL ) && ( index >= vlist->numvertices ) ) {
		index -= vlist->numvertices;
		vlist  = vlist->next;
	}

	if ( vlist == NULL ) {
		// invalid index (too large)
		ErrorMessage( "\n***ERROR*** Invalid index in VertexChunk.\n" );
		HandleCriticalError();
	}

	// return vertex
	return vlist->vertices[ index ];

#else

	if ( index >= numvertices ) {
		// invalid index (too large)
		ErrorMessage( "\n***ERROR*** Invalid index in VertexChunk.\n" );
		HandleCriticalError();
	}

	// return vertex
	return vertices[ index ];

#endif

}


// check if multiple vertices exist for the same actual point -----------------
//
int VertexChunkRep::CheckVertices( int verbose )
{
	StrScratch message;

	if ( verbose ) {
		InfoMessage( "\nChecking vertices...\n" );
	}

	nummultvertices = 0;
	int	multi		= 0;
	for ( int i = 0; i < getNumElements(); i++ ) {
		int	vertindx = FindCloseVertex( FetchVertex( i ), i );
		if ( vertindx != -1 ) {
			if ( verbose ) {
				if ( multi == 0 ) {
					InfoMessage( "*** multiple vertices found ***\n" );
					multi++;
				}
				sprintf( message, "Vertex %d is same as %d\n", i + 1, vertindx + 1 );
				InfoMessage( message );
			}
			nummultvertices++;
		}
	}

	return nummultvertices;
}


// insert vertex into list of vertex chunks, return index; check doublets -----
//
int VertexChunkRep::AddVertex( Vertex3 vertex )
{

#ifdef EXPAND_STORAGE_WITH_LISTS

	// if vertex contained in head-chunk return index
	if ( eliminatedoublets )
		for ( int i = 0; i < numvertices; i++ )
			if ( VerticesEqual( vertices[ i ], vertex ) )
				return i;

	// if space in head-chunk insert vertex directly
	if ( numvertices < maxnumvertices ) {

		vertices[ numvertices ] = vertex;
		return numvertices++;

	} else {

		int numskipped = numvertices;
		VertexChunkRep *vlist = this;

		// scan until non-full chunk found
		while ( ( vlist->next != NULL ) && ( vlist->next->numvertices == vlist->next->maxnumvertices ) ) {

			if ( eliminatedoublets )
				for ( int i = 0; i < vlist->next->numvertices; i++ )
					if ( VerticesEqual( vlist->next->vertices[ i ], vertex ) )
						return i + numskipped;
			vlist		= vlist->next;
			numskipped += vlist->numvertices;
		}

		if ( vlist->next == NULL ) {
			// insert new chunk if all existing are full
#ifdef EXPAND_EXPONENTIALLY
			vlist->next = new VertexChunkRep( vlist->maxnumvertices * 2 );
#else
			vlist->next = new VertexChunkRep;
#endif
		} else {
			if ( eliminatedoublets )
				for ( int i = 0; i < vlist->next->numvertices; i++ )
					if ( VerticesEqual( vlist->next->vertices[ i ], vertex ) )
						return i + numskipped;
		}

		// insert new vertex
		vlist->next->vertices[ vlist->next->numvertices ] = vertex;
		return vlist->next->numvertices++ + numskipped;
	}

#else

	// if vertex already contained in chunk return index
	if ( eliminatedoublets )
		for ( int i = 0; i < numvertices; i++ )
			if ( VerticesEqual( vertices[ i ], vertex ) )
				return i;

	// expand storage if necessary
	if ( numvertices == maxnumvertices ) {

#ifdef EXPAND_EXPONENTIALLY
		maxnumvertices *= 2;
#else
		maxnumvertices += CHUNK_SIZE;
#endif
		Vertex3 *temp = new Vertex3[ maxnumvertices ];
		memcpy( temp, vertices, numvertices * sizeof( Vertex3 ) );
		delete[] vertices;
		vertices = temp;
	}

	vertices[ numvertices ] = vertex;
	return numvertices++;

#endif

}


// VertexChunkRep specific static variables -----------------------------------
//
const int VertexChunkRep::CHUNK_SIZE = 256;
int	VertexChunkRep::eliminatedoublets = 0;


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
