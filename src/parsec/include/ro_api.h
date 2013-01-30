/*
 * PARSEC HEADER: ro_api.h
 */

#ifndef _RO_API_H_
#define _RO_API_H_

// default state not necessary
#define NO_DEFAULT_RASTERIZER_STATE


// need opengl headers
#include "r_gl.h"


// client state bits
enum {

	VTXARRAY_NONE		= 0x0000,
	VTXARRAY_COLORS		= 0x0001,
	VTXARRAY_EDGEFLAGS	= 0x0002,
	VTXARRAY_NORMALS	= 0x0004,
	VTXARRAY_TEXCOORDS	= 0x0008,
	VTXARRAY_VERTICES	= 0x0010
};


// vertex pointers config
enum {

	VTXPTRS_NONE,
	VTXPTRS_DO_ITER_1,
	VTXPTRS_DO_ITER_2,
	VTXPTRS_DO_ITER_3,
	VTXPTRS_DO_ITER_4,
	VTXPTRS_DO_FONT_1,
	VTXPTRS_DO_FONT_2,
	VTXPTRS_RO_PART_1,
	VTXPTRS_RO_POLY_1,
	VTXPTRS_RO_POLY_2,
	VTXPTRS_RO_POLY_3
};


// external functions

void	RO_InitializeState();

void	RO_SetCapability(GLenum cap, bool enable);

int		RO_DepthCmpEnabled();
int		RO_DepthWriteEnabled();

void	RO_EnableDepthTest( int enable );
void	RO_DepthFunc( GLenum func );
void	RO_DepthMask( GLboolean flag );
void	RO_BlendFunc( GLenum sfactor, GLenum dfactor );
void	RO_AlphaFunc( GLenum func, GLclampf ref );
void	RO_AlphaTest( int enable );
void	RO_PointSmooth( int enable );
void	RO_LineSmooth( int enable );
void	RO_PointSize( GLfloat size );

void	RO_ClientState( dword statebits );
int		RO_ArrayMakeCurrent( int userid, void *base );

void	RO_InvalidateRasterizerState();
void	RO_InitRasterizerState( dword itertype, dword raststate, dword rastmask );
void	RO_TextureCombineState( dword texcomb );


#define RO_AllowsExtendedTexenv()		FALSE


#ifdef NO_DEFAULT_RASTERIZER_STATE
	#define RO_DefaultRasterizerState()		{}
#else
	void	RO_DefaultRasterizerState();
#endif


#endif // _RO_API_H_


