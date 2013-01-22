//-----------------------------------------------------------------------------
//	BSPLIB MODULE: Texture.cpp
//
//  Copyright (c) 1996-1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib headers
#include "Texture.h"
#include "Chunk.h"


BSPLIB_NAMESPACE_BEGIN


// write texture information to text file -------------------------------------
//
void Texture::WriteInfo( FILE *fp )
{
	fprintf( fp, "%d/%d\t\t\"%s\"\t\t%s\n", m_width, m_height, m_name, m_file );
}


// constructor for Texture ----------------------------------------------------
//
Texture::Texture( int w, int h, char *tname, char *fname )
{
	*m_name = 0;
	*m_file = 0;

	if ( tname != NULL ) {
		setName( tname );
	}

	if ( fname != NULL ) {
		setFile( fname );
	}

	m_width  = w;
	m_height = h;
}


// set name of texture --------------------------------------------------------
//
void Texture::setName( const char *tname )
{
	if ( tname != NULL ) {
		strncpy( m_name, tname, MAX_TEXNAME );
		m_name[ MAX_TEXNAME ] = 0;
	} else {
		*m_name = 0;
	}
}


// set filename of texture data file ------------------------------------------
//
void Texture::setFile( const char *fname )
{
	if ( fname != NULL ) {
		strncpy( m_file, fname, PATH_MAX );
		m_file[ PATH_MAX ] = 0;
	} else {
		*m_file = 0;
	}
}


// size of texture chunk ------------------------------------------------------
//
template <> const int ChunkRep<Texture>::CHUNK_SIZE = 128;


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
