//-----------------------------------------------------------------------------
//	BSPLIB HEADER: Chunk.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _CHUNK_H_
#define _CHUNK_H_

// bsplib header files
#include "BspLibDefs.h"
#include "SystemIO.h"


// if this flag is set storage is increased by using lists
//#define EXPAND_STORAGE_WITH_LISTS

// if this flag is set storage is always increased two-fold
#define EXPAND_EXPONENTIALLY

// if this flag is set assignment operators for elements are not invoked
//#define IGNORE_ELEMENT_ASSIGNMENT_OPERATORS


BSPLIB_NAMESPACE_BEGIN


// added for Visual C++ 6.0
template<class T> class Chunk;


// template for simple chunk class (actual representation) --------------------
//
template<class T>
class ChunkRep : public virtual SystemIO {

	friend class Chunk<T>;

	// error identifiers
	enum {
		E_INVALIDINDX
	};

	void		Error( int err ) const;

private:
	ChunkRep( int chunksize = 0 );
	~ChunkRep();

	int			AddElement( const T& element );
	T&			FetchElement( int index );

	int			getNumElements() const;

private:
	int			ref_count;
	int			numelements;
	int			maxnumelements;
	T*			elements;
	ChunkRep*	next;

	static const int CHUNK_SIZE;
};


// create new chunk -----------------------------------------------------------
//
template<class T>
ChunkRep<T>::ChunkRep( int chunksize ) : ref_count( 0 )
{
	int challocsize	= chunksize > 0 ? chunksize : CHUNK_SIZE;
	elements		= new T[ challocsize ];
	maxnumelements	= challocsize;
	numelements		= 0;
	next			= NULL;
}


// destroy list of chunks recursively -----------------------------------------
//
template<class T>
ChunkRep<T>::~ChunkRep()
{
	delete[] elements;
	delete next;
}


// error message handler ------------------------------------------------------
//
template<class T>
void ChunkRep<T>::Error( int err ) const
{
	{
	StrScratch message;
	sprintf( message, "***ERROR*** in object of a class of template Chunk" );

	switch ( err ) {

	case E_INVALIDINDX:
		sprintf( message + strlen( message ), ": [Invalid index]" );
		break;
	}

	ErrorMessage( message );
	}

	HandleCriticalError();
}


// fetch element corresponding to specific index ------------------------------
//
template<class T>
T& ChunkRep<T>::FetchElement( int index )
{

#ifdef EXPAND_STORAGE_WITH_LISTS

	ChunkRep *vlist = this;
	while ( ( vlist != NULL ) && ( index >= vlist->numelements ) ) {
		index -= vlist->numelements;
		vlist  = vlist->next;
	}

	if ( vlist == NULL )
		Error( E_INVALIDINDX );

	// return element
	return vlist->elements[ index ];

#else

	if ( index >= numelements )
		Error( E_INVALIDINDX );

	// return element
	return elements[ index ];

#endif

}


// insert element into chunk, return index ------------------------------------
//
template<class T>
int ChunkRep<T>::AddElement( const T& element )
{

#ifdef EXPAND_STORAGE_WITH_LISTS

	// if space in head-chunk insert element directly
	if ( numelements < maxnumelements ) {
		elements[ numelements ] = element;
		return numelements++;

	} else {

		int		 numskipped	= numelements;
		ChunkRep *vlist		= this;

		// scan until non-full chunk found
		while ( ( vlist->next != NULL ) && ( vlist->next->numelements == vlist->next->maxnumelements ) ) {
			vlist		= vlist->next;
			numskipped += vlist->numelements;
		}

		// insert new chunk if all existing are full
		if ( vlist->next == NULL ) {

#ifdef EXPAND_EXPONENTIALLY
			vlist->next = new ChunkRep( vlist->maxnumelements * 2 );
#else
			vlist->next = new ChunkRep;
#endif
		}

		// insert new element
		vlist->next->elements[ vlist->next->numelements ] = element;
		return vlist->next->numelements++ + numskipped;
	}

#else

	// expand storage if necessary
	if ( numelements == maxnumelements ) {

#ifdef EXPAND_EXPONENTIALLY
		maxnumelements *= 2;
#else
		maxnumelements += CHUNK_SIZE;
#endif
		T *temp = new T[ maxnumelements ];

#ifdef IGNORE_ELEMENT_ASSIGNMENT_OPERATORS
		memcpy( temp, elements, numelements * sizeof( T ) );
#else
		for ( int i = 0; i < numelements; i++ )
			temp[ i ] = elements[ i ];
#endif
		delete[] elements;
		elements = temp;
	}

	elements[ numelements ] = element;
	return numelements++;

#endif

}


// return number of elements in entire chunk ----------------------------------
//
template<class T>
int ChunkRep<T>::getNumElements() const
{
	int num = 0;
	for ( const ChunkRep *vlist = this; vlist; vlist = vlist->next )
		num += vlist->numelements;
	return num;
}


// template for simple chunk class (handle class) -----------------------------
//
template<class T>
class Chunk {

public:
	Chunk( int chunksize = 0 ) { rep = new ChunkRep<T>( chunksize ); rep->ref_count = 1; }
	~Chunk() { if ( --rep->ref_count == 0 ) delete rep; }

	Chunk( const Chunk& copyobj );
	Chunk& operator =( const Chunk& copyobj );

	T&				operator []( int index ) { return rep->FetchElement( index ); }
	T&				FetchElement( int index ) { return rep->FetchElement( index ); }
	int				AddElement( const T& element ) { return rep->AddElement( element ); }

	int				getNumElements() const { return rep->getNumElements(); }

private:
	ChunkRep<T>*	rep;
};


// copy constructor for a Chunk -----------------------------------------------
//
template<class T>
Chunk<T>::Chunk( const Chunk<T>& copyobj )
{
	rep = copyobj.rep;
	rep->ref_count++;
}


// assignment operator for a Chunk --------------------------------------------
//
template<class T>
Chunk<T>& Chunk<T>::operator =( const Chunk<T>& copyobj )
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


#endif // _CHUNK_H_

