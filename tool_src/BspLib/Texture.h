//-----------------------------------------------------------------------------
//	BSPLIB HEADER: Texture.h
//
//  Copyright (c) 1996-1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _TEXTURE_H_
#define _TEXTURE_H_

// bsplib header files
#include "BspLibDefs.h"
#include "Chunk.h"
#include "SystemIO.h"


BSPLIB_NAMESPACE_BEGIN


#define MAX_TEXNAME 31


// class Texture; file oriented, only used for descriptive purposes -----------
//
class Texture : public virtual SystemIO {

public:
	Texture( int w = -1, int h = -1, char *tname = NULL, char *fname = NULL );
	~Texture() { }

	int			getWidth() const { return m_width; }
	int			getHeight() const { return m_height; }
	const char*	getName() const { return m_name; }
	const char*	getFile() const { return m_file; }

	void		setWidth( int w ) { m_width = w; }
	void		setHeight( int h ) { m_height = h; }
	void		setName( const char *tname );
	void		setFile( const char *fname );

	void		WriteInfo( FILE *fp );

private:
	int			m_width;
	int			m_height;
	char		m_name[ MAX_TEXNAME + 1 ];
	char		m_file[ PATH_MAX + 1 ];
};

// typedef chunk of textures --------------------------------------------------
typedef Chunk<Texture> TextureChunk;


BSPLIB_NAMESPACE_END


#endif // _TEXTURE_H_

