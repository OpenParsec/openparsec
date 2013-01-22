/*
 * PARSEC HEADER: ro_supp.h
 */

#ifndef _RO_SUPP_H_
#define _RO_SUPP_H_


// ro_supp.c implements the following functions
// -------------------------------------------
//	int		R_PrecacheTextures( GenObject *objectp );


// conversion factor from source (internal) depth to destination (opengl) depth

#ifdef FRACTIONAL_DEPTH_VALUES
	#define OPENGL_DEPTH_RANGE		1.0f			// [ 0,     1 ] -> [ 0, 1 ]
#else
	#define OPENGL_DEPTH_RANGE		(1/65535.0f)	// [ 0, 65535 ] -> [ 0, 1 ]
#endif


// need opengl headers
#include "r_gl.h"


// structures

struct GLTexInfo {

	TextureMap*	texmap;
	void*		data;
	int			width;
	int			height;
	dword		format;
	float		coscale;
	float		aratio;
	word		lodsmall;
	word		lodlarge;
};


// external functions

void	RO_EnableDepthBuffer( bool checking, bool writing );
void	RO_DisableDepthBuffer( bool checking, bool writing );
void	RO_RestoreDepthState( int zcmpstate, int zwritestate );
void	RO_InvalidateTextureMem();
void	RO_InvalidateCachedTexture( TextureMap *tmap );
void	RO_SelectTexelSource( GLTexInfo *texinfo );
void	RO_SelectTexelSource2( GLTexInfo *p2info );
void	RO_IterVertex2GLVertex( GLVertex3 *glvtxs, IterVertex2 *itervtxs, int num, GLTexInfo *texinfo );
void	RO_IterVertex3GLVertex( GLVertex3 *glvtxs, IterVertex3 *itervtxs, int num, GLTexInfo *texinfo );
void	RO_IterVertex2GLVertexRef( GLVertex3 *glvtxs, IterVertex2 **itervtxs, int num, GLTexInfo *texinfo );
void	RO_IterVertex3GLVertexRef( GLVertex3 *glvtxs, IterVertex3 **itervtxs, int num, GLTexInfo *texinfo );
void	RO_TextureMap2GLTexInfo( GLTexInfo *texinfo, TextureMap *texmap );
void	RO_Render2DRectangle( sgrid_t putx, sgrid_t puty, float srcw, float srch, dword dstw, dword dsth, dword zvalue, colrgba_s *color );


#endif // _RO_SUPP_H_


