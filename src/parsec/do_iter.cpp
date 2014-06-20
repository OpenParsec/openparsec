/*
 * PARSEC - Iterated Primitives Drawing Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:33 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999-2000
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */ 

// C library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "vid_defs.h"

// drawing subsystem
#include "d_iter.h"

// subsystem linkage info
#include "linkinfo.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "do_iter.h"

// proprietary module headers
#include "con_aux.h"
#include "ro_api.h"
#include "ro_supp.h"


// flags
#define ALLOW_LINE_STRIP_RESTARTS
#define PERFORM_PRE_CLIPPING
#define EXPLICIT_GL_ORTHO



//NOTE:
// the 2-D iter functions don't do any clipping. the user
// must ensure that drawing works correctly without
// clipping. (e.g., clipping them before if they are not
// already known to be contained in the frustum.)
// but OpenGL does clipping anyway.



// set bias for iterated depth (polygon offset) -------------------------------
//
void D_BiasIterDepth( int bias )
{
	#define NO_DEPTH_BIAS	0

	// reset bias?
	if ( bias == NO_DEPTH_BIAS ) {

		glDisable( GL_POLYGON_OFFSET_FILL );

	} else {

		glEnable( GL_POLYGON_OFFSET_FILL );
		glPolygonOffset( (GLfloat) 0, (GLfloat) 0 );
//		glPolygonOffset( (GLfloat) 0, (GLfloat) bias );
//		glPolygonOffset( (GLfloat) bias, (GLfloat) bias );
	}
}


// storage for currently active transformation matrix -------------------------
//
static Xmatrx	_iter_matrix;
static Xmatrx	*iter_matrix = NULL;	// NULL means identity matrix
static Xmatrx	*prev_matrix;
static GLfloat	gl_cur_matrix[ 16 ];


// load transformation matrix to use for iterated primitives drawing ----------
//
void D_LoadIterMatrix( Xmatrx matrix )
{
	if ( matrix != NULL ) {

		iter_matrix = &_iter_matrix;
		memcpy( iter_matrix, matrix, sizeof( Xmatrx ) );

		gl_cur_matrix[  0 ] = GEOMV_TO_FLOAT( matrix[ 0 ][ 0 ] );
		gl_cur_matrix[  1 ] = GEOMV_TO_FLOAT( matrix[ 1 ][ 0 ] );
		gl_cur_matrix[  2 ] = GEOMV_TO_FLOAT( matrix[ 2 ][ 0 ] );
		gl_cur_matrix[  3 ] = GEOMV_0;
		gl_cur_matrix[  4 ] = GEOMV_TO_FLOAT( matrix[ 0 ][ 1 ] );
		gl_cur_matrix[  5 ] = GEOMV_TO_FLOAT( matrix[ 1 ][ 1 ] );
		gl_cur_matrix[  6 ] = GEOMV_TO_FLOAT( matrix[ 2 ][ 1 ] );
		gl_cur_matrix[  7 ] = GEOMV_0;
		gl_cur_matrix[  8 ] = GEOMV_TO_FLOAT( matrix[ 0 ][ 2 ] );
		gl_cur_matrix[  9 ] = GEOMV_TO_FLOAT( matrix[ 1 ][ 2 ] );
		gl_cur_matrix[ 10 ] = GEOMV_TO_FLOAT( matrix[ 2 ][ 2 ] );
		gl_cur_matrix[ 11 ] = GEOMV_0;
		gl_cur_matrix[ 12 ] = GEOMV_TO_FLOAT( matrix[ 0 ][ 3 ] );
		gl_cur_matrix[ 13 ] = GEOMV_TO_FLOAT( matrix[ 1 ][ 3 ] );
		gl_cur_matrix[ 14 ] = GEOMV_TO_FLOAT( matrix[ 2 ][ 3 ] );
		gl_cur_matrix[ 15 ] = GEOMV_1;

	} else {

		iter_matrix = NULL;
	}
}


// static vertex buffers for intermediate storage -----------------------------
//
#define VERTEX_BUFFERS_SIZE		256

struct iter_prim_s {

	word		flags;
	short		NumVerts;
	dword		itertype;
	word		raststate;
	word		rastmask;
	Plane3*		plane;
	TextureMap*	texmap;
	IterVertex3	Vtxs[ VERTEX_BUFFERS_SIZE ];
};

static iter_prim_s	_iter_prim[ 1 ];
static iter_prim_s	*iter_prim = _iter_prim;
static GLVertex3	_gl_array_vtxs[ VERTEX_BUFFERS_SIZE ];
static GLVertex3	*gl_array_vtxs = _gl_array_vtxs;


// current vertex array info --------------------------------------------------
//
static GLTexInfo	gl_array_texinfo;
static IterArray3*	gl_array_current = NULL;
static dword		gl_lockrange_base;
static dword		gl_lockrange_beyond;
static int			gl_array_zcmpstate;
static int			gl_array_zwritestate;


// determine whether primitive needs texture ----------------------------------
//
#define ITERPRIM_TEXTURED(p) ( ( (p)->itertype & iter_base_mask ) >= iter_texonly )


// opengl vertex array macros -------------------------------------------------
//
#define INIT_GL_ARRAYS(p,i,c) {																\
																							\
	if ( ITERPRIM_TEXTURED( p ) ) {															\
		RO_ClientState( VTXARRAY_VERTICES | VTXARRAY_COLORS | VTXARRAY_TEXCOORDS );			\
		if ( RO_ArrayMakeCurrent( VTXPTRS_DO_ITER_4, gl_array_vtxs ) ) {					\
			glVertexPointer( 3, GL_FLOAT, sizeof( GLVertex3 ), &gl_array_vtxs->x );			\
			glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( GLVertex3 ), &gl_array_vtxs->r );	\
			glTexCoordPointer( 4, GL_FLOAT, sizeof( GLVertex3 ), &gl_array_vtxs->s );		\
		}																					\
	} else {																				\
		RO_ClientState( VTXARRAY_VERTICES | VTXARRAY_COLORS );								\
		if ( RO_ArrayMakeCurrent( VTXPTRS_DO_ITER_2, gl_array_vtxs ) ) {					\
			glVertexPointer( 3, GL_FLOAT, sizeof( GLVertex3 ), &gl_array_vtxs->x );			\
			glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( GLVertex3 ), &gl_array_vtxs->r );	\
		}																					\
	}																						\
}

#define DEINIT_GL_ARRAYS(p) {													\
																				\
}


// retrieve texture info and select texture -----------------------------------
//
PRIVATE
void DO_SelectTexture( TextureMap *texmap, GLTexInfo *texinfo )
{
	ASSERT( texmap != NULL );
	ASSERT( texinfo != NULL );

	// fill texinfo structure and determine coscale/aratio
	RO_TextureMap2GLTexInfo( texinfo, texmap );

	// enforce texel source
	RO_SelectTexelSource( texinfo );
}


// prepare vertices for use by gl (convert to gl format) ----------------------
//
PRIVATE
void DO_PrepVertices2( dword flags, IterVertex2 *vtxs, int numverts, GLTexInfo *texinfo )
{
	ASSERT( vtxs != NULL );
	ASSERT( numverts > 0 );

	ASSERT( gl_array_vtxs == _gl_array_vtxs );
	ASSERT( iter_prim     == _iter_prim );

	// allocate dynamic array if static storage too small
	if ( numverts > VERTEX_BUFFERS_SIZE ) {
		gl_array_vtxs = (GLVertex3 *) ALLOCMEM( numverts * sizeof( GLVertex3 ) );
		if ( gl_array_vtxs == NULL ) {
			OUTOFMEM( "DO_ITER::DO_PrepVertices2(): no mem for vertex array." );
		}
	}

	// convert vertex info into opengl format
	if ( flags & ITERFLAG_VERTEXREFS ) {
		IterVertex2 **refs = (IterVertex2 **) vtxs;
		RO_IterVertex2GLVertexRef( gl_array_vtxs, refs, numverts, texinfo );
	} else {
		RO_IterVertex2GLVertex( gl_array_vtxs, vtxs, numverts, texinfo );
	}
}


// prepare vertices for use by gl (project and convert to gl format) ----------
//
PRIVATE
void DO_PrepVertices3( dword flags, IterVertex3 *vtxs, int numverts, GLTexInfo *texinfo )
{
	ASSERT( vtxs != NULL );
	ASSERT( numverts > 0 );

	ASSERT( gl_array_vtxs == _gl_array_vtxs );
	ASSERT( ( iter_prim   == _iter_prim ) ||
			( ( flags & ITERFLAG_NONDESTRUCTIVE ) == 0 ) );

	// allocate dynamic array if static storage too small
	if ( numverts > VERTEX_BUFFERS_SIZE ) {
		gl_array_vtxs = (GLVertex3 *) ALLOCMEM( numverts * sizeof( GLVertex3 ) );
		if ( gl_array_vtxs == NULL ) {
			OUTOFMEM( "DO_ITER::DO_PrepVertices3(): no mem for vertex array." );
		}
	}

	IterVertex3 **refs = (IterVertex3 **) vtxs;

	// project vertices/texture coordinates if specified
	if ( flags & ( ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_DIV_UVW ) ) {

		// make local copy if nondestructive specified
		if ( flags & ITERFLAG_NONDESTRUCTIVE ) {

			if ( numverts > VERTEX_BUFFERS_SIZE ) {
				iter_prim = (iter_prim_s *)
					ALLOCMEM( (size_t)&((iter_prim_s*)0)->Vtxs[ numverts ] );
				if ( iter_prim == NULL ) {
					OUTOFMEM( "DO_ITER::DO_PrepVertices3(): no mem for vertex copies." );
				}
			}

			if ( flags & ITERFLAG_VERTEXREFS ) {
				for ( int vtx = numverts - 1; vtx >= 0 ; vtx-- )
					iter_prim->Vtxs[ vtx ] = *refs[ vtx ];
				flags &= ~ITERFLAG_VERTEXREFS;
			} else {
				memcpy( iter_prim->Vtxs, vtxs, numverts * sizeof( IterVertex3 ) );
			}

			vtxs = iter_prim->Vtxs;
		}

		// project vertices/texture coordinates
		if ( flags & ITERFLAG_VERTEXREFS ) {

			for ( int v = numverts - 1; v >= 0 ; v-- ) {
				geomv_t ooz = SCREENSPACE_OOZ( *refs[ v ] );
				if ( flags & ITERFLAG_Z_DIV_XYZ ) {
					refs[ v ]->X = COORD_TO_GEOMV( SCREENSPACE_XOZ( *refs[ v ], ooz ) );
					refs[ v ]->Y = COORD_TO_GEOMV( SCREENSPACE_YOZ( *refs[ v ], ooz ) );
					refs[ v ]->Z = ( flags & ITERFLAG_Z_TO_DEPTH ) ?
						DEPTH_TO_GEOMV( DEPTHBUFF_VALUE( ooz ) ) : GEOMV_1;
				}
				if ( flags & ITERFLAG_Z_DIV_UVW ) {
					refs[ v ]->U = GEOMV_MUL( refs[ v ]->U, ooz );
					refs[ v ]->V = GEOMV_MUL( refs[ v ]->V, ooz );
					refs[ v ]->W = ( flags & ITERFLAG_W_NOT_ONE ) ?
						GEOMV_MUL( refs[ v ]->W, ooz ) : ooz;
				}
			}

		} else {

			for ( int v = numverts - 1; v >= 0 ; v-- ) {
				geomv_t ooz = SCREENSPACE_OOZ( vtxs[ v ] );
				if ( flags & ITERFLAG_Z_DIV_XYZ ) {
					vtxs[ v ].X = COORD_TO_GEOMV( SCREENSPACE_XOZ( vtxs[ v ], ooz ) );
					vtxs[ v ].Y = COORD_TO_GEOMV( SCREENSPACE_YOZ( vtxs[ v ], ooz ) );
					vtxs[ v ].Z = ( flags & ITERFLAG_Z_TO_DEPTH ) ?
						DEPTH_TO_GEOMV( DEPTHBUFF_VALUE( ooz ) ) : GEOMV_1;
				}
				if ( flags & ITERFLAG_Z_DIV_UVW ) {
					vtxs[ v ].U = GEOMV_MUL( vtxs[ v ].U, ooz );
					vtxs[ v ].V = GEOMV_MUL( vtxs[ v ].V, ooz );
					vtxs[ v ].W = ( flags & ITERFLAG_W_NOT_ONE ) ?
						GEOMV_MUL( vtxs[ v ].W, ooz ) : ooz;
				}
			}
		}
	}

	// convert vertex info into opengl format
	if ( flags & ITERFLAG_VERTEXREFS ) {
		RO_IterVertex3GLVertexRef( gl_array_vtxs, refs, numverts, texinfo );
	} else {
		RO_IterVertex3GLVertex( gl_array_vtxs, vtxs, numverts, texinfo );
	}
}


// transform vertices using current transformation matrix ---------------------
//
PRIVATE
iter_prim_s *DO_TransformVertices3( iter_prim_s *itprim, int numverts )
{
	ASSERT( itprim != NULL );
	ASSERT( numverts > 0 );

	if ( itprim->flags & ITERFLAG_NONDESTRUCTIVE ) {

		// allocate dynamic array if static storage too small
		if ( numverts > VERTEX_BUFFERS_SIZE ) {
			iter_prim = (iter_prim_s *)
				ALLOCMEM( (size_t)&((iter_prim_s*)0)->Vtxs[ numverts ] );
			if ( iter_prim == NULL ) {
				OUTOFMEM( "DO_ITER::DO_TransformVertices3(): no mem for primitive copy." );
			}
		}

		// transform vertices into copy
		if ( itprim->flags & ITERFLAG_VERTEXREFS ) {

			IterVertex3 **refs = (IterVertex3 **) itprim->Vtxs;
			for ( int v = numverts - 1; v >= 0 ; v-- ) {
				memcpy( &iter_prim->Vtxs[ v ], refs[ v ], sizeof( IterVertex3 ) );
				MtxVctMUL( *iter_matrix, (Vertex3 *)refs[ v ],
						   (Vertex3 *)&iter_prim->Vtxs[ v ] );
			}

		} else {

			IterVertex3 *vtxs = itprim->Vtxs;
			for ( int v = numverts - 1; v >= 0 ; v-- ) {
				memcpy( &iter_prim->Vtxs[ v ], &vtxs[ v ], sizeof( IterVertex3 ) );
				MtxVctMUL( *iter_matrix, (Vertex3 *)&vtxs[ v ],
						   (Vertex3 *)&iter_prim->Vtxs[ v ] );
			}
		}

		// copy header, use copy from here on
		memcpy( iter_prim, itprim, (size_t)((iter_prim_s*)0)->Vtxs );
		itprim = iter_prim;

		// no refs anymore, avoid second copy (copy may be destroyed)
		itprim->flags &= ( ~ITERFLAG_VERTEXREFS & ~ITERFLAG_NONDESTRUCTIVE );

	} else {

		// transform vertices in place
		Vertex3 tempvert;
		if ( itprim->flags & ITERFLAG_VERTEXREFS ) {

			IterVertex3 **refs = (IterVertex3 **) itprim->Vtxs;
			for ( int v = numverts - 1; v >= 0 ; v-- ) {
				MtxVctMUL( *iter_matrix, (Vertex3 *)refs[ v ], &tempvert );
				refs[ v ]->X = tempvert.X;
				refs[ v ]->Y = tempvert.Y;
				refs[ v ]->Z = tempvert.Z;
			}

		} else {

			IterVertex3 *vtxs = itprim->Vtxs;
			for ( int v = numverts - 1; v >= 0 ; v-- ) {
				MtxVctMUL( *iter_matrix, (Vertex3 *)&vtxs[ v ], &tempvert );
				vtxs[ v ].X = tempvert.X;
				vtxs[ v ].Y = tempvert.Y;
				vtxs[ v ].Z = tempvert.Z;
			}
		}
	}

	return itprim;
}


// free intermediate storage (temp primitive, converted vertex array) ---------
//
INLINE
void DO_KillTemporaries()
{
	// fall back to static storage
	if ( gl_array_vtxs != _gl_array_vtxs ) {
		FREEMEM( gl_array_vtxs );
		gl_array_vtxs = _gl_array_vtxs;
	}

	if ( iter_prim != _iter_prim ) {
		FREEMEM( iter_prim );
		iter_prim = _iter_prim;
	}
}


// setup gl projection matrix -------------------------------------------------
//
PRIVATE
void DO_PrepProjection( dword flags, IterVertex3 *vtxs, int numverts, GLTexInfo *texinfo )
{
	ASSERT( vtxs != NULL );
	ASSERT( numverts > 0 );

	ASSERT( gl_array_vtxs == _gl_array_vtxs );
	ASSERT( iter_prim     == _iter_prim );

	// allocate dynamic array if static storage too small
	if ( numverts > VERTEX_BUFFERS_SIZE ) {
		gl_array_vtxs = (GLVertex3 *) ALLOCMEM( numverts * sizeof( GLVertex3 ) );
		if ( gl_array_vtxs == NULL ) {
			OUTOFMEM( "DO_ITER::DO_PrepProjection(): no mem for vertex array." );
		}
	}

	// convert vertex info into opengl format
	GLVertex3 *glvtxs = gl_array_vtxs;
	IterVertex3 *itervtxs = vtxs;
	int num = numverts;

	if ( texinfo != NULL ) {

		float scale_u = texinfo->coscale;
		float scale_v = texinfo->aratio * scale_u;

		if ( flags & ITERFLAG_VERTEXREFS ) {

			// convert specified number of vertices
			for ( int v = 0; v < num; v++ ) {

				IterVertex3 *itervtx = ((IterVertex3 **)itervtxs)[ v ];
				ASSERT( itervtx != NULL );

				glvtxs[ v ].x = GEOMV_TO_FLOAT( itervtx->X );
				glvtxs[ v ].y = GEOMV_TO_FLOAT( itervtx->Y );
				glvtxs[ v ].z = GEOMV_TO_FLOAT( itervtx->Z );

				*(dword*)&glvtxs[ v ].r = *(dword*)&itervtx->R;

				glvtxs[ v ].s = GEOMV_TO_FLOAT( itervtx->U ) * scale_u;
				glvtxs[ v ].t = GEOMV_TO_FLOAT( itervtx->V ) * scale_v;
				glvtxs[ v ].p = 0.0f;
				glvtxs[ v ].q = GEOMV_TO_FLOAT( itervtx->W );
			}

		} else {

			// convert specified number of vertices
			for ( int v = 0; v < num; v++ ) {

				glvtxs[ v ].x = GEOMV_TO_FLOAT( itervtxs[ v ].X );
				glvtxs[ v ].y = GEOMV_TO_FLOAT( itervtxs[ v ].Y );
				glvtxs[ v ].z = GEOMV_TO_FLOAT( itervtxs[ v ].Z );

				*(dword*)&glvtxs[ v ].r = *(dword*)&itervtxs[ v ].R;

				glvtxs[ v ].s = GEOMV_TO_FLOAT( itervtxs[ v ].U ) * scale_u;
				glvtxs[ v ].t = GEOMV_TO_FLOAT( itervtxs[ v ].V ) * scale_v;
				glvtxs[ v ].p = 0.0f;
				glvtxs[ v ].q = GEOMV_TO_FLOAT( itervtxs[ v ].W );
			}
		}

	} else {

		if ( flags & ITERFLAG_VERTEXREFS ) {

			// convert specified number of vertices
			for ( int v = 0; v < num; v++ ) {

				IterVertex3 *itervtx = ((IterVertex3 **)itervtxs)[ v ];
				ASSERT( itervtx != NULL );

				glvtxs[ v ].x = GEOMV_TO_FLOAT( itervtx->X );
				glvtxs[ v ].y = GEOMV_TO_FLOAT( itervtx->Y );
				glvtxs[ v ].z = GEOMV_TO_FLOAT( itervtx->Z );

				*(dword*)&glvtxs[ v ].r = *(dword*)&itervtx->R;
			}

		} else {

			// convert specified number of vertices
			for ( int v = 0; v < num; v++ ) {

				glvtxs[ v ].x = GEOMV_TO_FLOAT( itervtxs[ v ].X );
				glvtxs[ v ].y = GEOMV_TO_FLOAT( itervtxs[ v ].Y );
				glvtxs[ v ].z = GEOMV_TO_FLOAT( itervtxs[ v ].Z );

				*(dword*)&glvtxs[ v ].r = *(dword*)&itervtxs[ v ].R;
			}
		}
	}

	// configure modelview matrix
	if ( iter_matrix != NULL ) {
		glMatrixMode( GL_MODELVIEW );
		glLoadMatrixf( gl_cur_matrix );
	}

	// configure projection matrix
	glMatrixMode( GL_PROJECTION );
	extern GLfloat gl_projective_matrix[];
	glLoadMatrixf( gl_projective_matrix );
}


// restore standard gl projection matrix, disable transformation --------------
//
PRIVATE
void DO_KillProjection()
{
	// set projection to screen coordinate identity
	glMatrixMode( GL_PROJECTION );
#ifdef EXPLICIT_GL_ORTHO
	glLoadIdentity();
	extern GLfloat gl_orthogonal_params[];
	//SDL_CalcOrthogonalMatrix();
	// FIXME: glOrthof on GLES1
	glOrtho( gl_orthogonal_params[ 0 ], gl_orthogonal_params[ 1 ],
			 gl_orthogonal_params[ 2 ], gl_orthogonal_params[ 3 ],
			 gl_orthogonal_params[ 4 ], gl_orthogonal_params[ 5 ] );
#else
	extern GLfloat gl_orthogonal_matrix[];
	glLoadMatrixf( gl_orthogonal_matrix );
#endif

	// set modelview identity
	if ( iter_matrix != NULL ) {
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
	}
}


// draw 2-D point (point-set) using iterated vertex attributes ----------------
//
void D_DrawIterPoint2( IterPoint2 *itpoint )
{
	ASSERT( itpoint != NULL );
	ASSERT( itpoint->NumVerts > 0 );

	ASSERT( !ITERPRIM_TEXTURED( itpoint ) );
	ASSERT( gl_array_current == NULL );

	// convert vertex info into opengl format
	dword flags = itpoint->flags;
	DO_PrepVertices2( flags, itpoint->Vtxs, itpoint->NumVerts, NULL );

	// save zcmp and zwrite state
	int zcmpstate   = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();

	// configure rasterizer
	RO_InitRasterizerState( itpoint->itertype, itpoint->raststate, itpoint->rastmask );

	// set line antialiasing mode
	if ( flags & ITERFLAG_PS_ANTIALIASED )
		RO_PointSmooth( TRUE );

	// set point size
	RO_PointSize( itpoint->pointsize );

	// draw line strip/loop
	INIT_GL_ARRAYS( itpoint, 0, itpoint->NumVerts );
	glDrawArrays( GL_POINTS, 0, itpoint->NumVerts );
	DEINIT_GL_ARRAYS( itpoint );

	// set rasterizer state to default
	if ( flags & ITERFLAG_PS_ANTIALIASED )
		RO_PointSmooth( FALSE );
	RO_DefaultRasterizerState();

	// restore zcmp and zwrite state
	RO_RestoreDepthState( zcmpstate, zwritestate );

	// free intermediate storage
	DO_KillTemporaries();
}


// draw 3-D point (point-set) using iterated vertex attributes ----------------
//
void D_DrawIterPoint3( IterPoint3 *itpoint, dword cullmask )
{
}


// draw 2-D line (line-strip) using iterated vertex attributes ----------------
//
void D_DrawIterLine2( IterLine2 *itline )
{
	ASSERT( itline != NULL );
	ASSERT( itline->NumVerts > 1 );
	ASSERT( itline->NumVerts <= MAX_ITERLINE_VERTICES );

	ASSERT( gl_array_current == NULL );

	// get info and select texture
	GLTexInfo _texinfo;
	GLTexInfo *texinfo = NULL;
	if ( ITERPRIM_TEXTURED( itline ) ) {
		texinfo = &_texinfo;
		DO_SelectTexture( itline->texmap, texinfo );
	}

	// convert vertex info into opengl format
	dword flags = itline->flags;
	DO_PrepVertices2( flags, itline->Vtxs, itline->NumVerts, texinfo );

	// save zcmp and zwrite state
	int zcmpstate   = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();

	// configure rasterizer
	RO_InitRasterizerState( itline->itertype, itline->raststate, itline->rastmask );
	RO_TextureCombineState( texcomb_decal );

	// set line antialiasing mode
	if ( flags & ITERFLAG_LS_ANTIALIASED )
		RO_LineSmooth( TRUE );

	GLenum glmode = ( flags & ITERFLAG_LS_CLOSE_STRIP ) ? GL_LINE_LOOP : GL_LINE_STRIP;

#ifdef ALLOW_LINE_STRIP_RESTARTS

	int stripbase = 0;

	INIT_GL_ARRAYS( itline, 0, itline->NumVerts );

	// draw all strips but last
	int subx = 1;
	for ( subx = 1; subx < itline->NumVerts; subx++ ) {
		if ( itline->Vtxs[ subx ].flags & ITERVTXFLAG_RESTART ) {
			// draw sub-strip
			glDrawArrays( GL_LINE_STRIP, stripbase, subx - stripbase );
			stripbase = subx;
		}
	}

	// looping works only correctly with a single strip
	ASSERT( ( ( flags & ITERFLAG_LS_CLOSE_STRIP ) == 0 ) ||
			( stripbase == 0 ) );

	// draw last strip/loop
	glDrawArrays( glmode, stripbase, subx - stripbase );

	DEINIT_GL_ARRAYS( itline );

#else // ALLOW_LINE_STRIP_RESTARTS

	// draw line strip/loop
	INIT_GL_ARRAYS( itline, 0, itline->NumVerts );
	glDrawArrays( glmode, 0, itline->NumVerts );
	DEINIT_GL_ARRAYS( itline );

#endif // ALLOW_LINE_STRIP_RESTARTS

	// set rasterizer state to default
	if ( flags & ITERFLAG_LS_ANTIALIASED )
		RO_LineSmooth( FALSE );
	RO_DefaultRasterizerState();

	// restore zcmp and zwrite state
	RO_RestoreDepthState( zcmpstate, zwritestate );

	// free intermediate storage
//	DO_KillTemporaries();
}


// draw 3-D line (line-strip) using iterated vertex attributes ----------------
//
void D_DrawIterLine3( IterLine3 *itline, dword cullmask )
{
	ASSERT( itline != NULL );
	ASSERT( itline->NumVerts > 1 );
	ASSERT( itline->NumVerts <= MAX_ITERLINE_VERTICES );

	ASSERT( gl_array_current == NULL );

	// transform vertices
	if ( iter_matrix != NULL ) {
		itline = (IterLine3 *)
			DO_TransformVertices3( (iter_prim_s *)itline, itline->NumVerts );
	}

#ifdef PERFORM_PRE_CLIPPING

	// clip if necessary
	if ( cullmask != 0x00 ) {

		//TODO:
		ASSERT( 0 );
		return;
/*
		IterLine3 *clipline =
			CLIP_VolumeIterLine3( itline, View_Volume, cullmask );
		if ( clipline == NULL )
			return;
		itline = clipline;
*/
	}

#endif //  PERFORM_PRE_CLIPPING

	// get info and select texture
	GLTexInfo _texinfo;
	GLTexInfo *texinfo = NULL;
	if ( ITERPRIM_TEXTURED( itline ) ) {
		texinfo = &_texinfo;
		DO_SelectTexture( itline->texmap, texinfo );
	}

	// project and convert vertices
	dword flags = itline->flags;
	DO_PrepVertices3( flags, itline->Vtxs, itline->NumVerts, texinfo );

	// save zcmp and zwrite state
	int zcmpstate   = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();

	// configure rasterizer
	RO_InitRasterizerState( itline->itertype, itline->raststate, itline->rastmask );
	RO_TextureCombineState( texcomb_decal );

	// set line antialiasing mode
	if ( flags & ITERFLAG_LS_ANTIALIASED )
		RO_LineSmooth( TRUE );

	GLenum glmode = ( flags & ITERFLAG_LS_CLOSE_STRIP ) ?
					GL_LINE_LOOP : GL_LINE_STRIP;

#ifdef ALLOW_LINE_STRIP_RESTARTS

	int stripbase = 0;

	INIT_GL_ARRAYS( itline, 0, itline->NumVerts );

	// draw all strips but last
	int subx = 1;
	for ( subx = 1; subx < itline->NumVerts; subx++ ) {
		if ( itline->Vtxs[ subx ].flags & ITERVTXFLAG_RESTART ) {
			// draw sub-strip
			glDrawArrays( GL_LINE_STRIP, stripbase, subx - stripbase );
			stripbase = subx;
		}
	}

	// looping works only correctly with a single strip
	ASSERT( ( ( flags & ITERFLAG_LS_CLOSE_STRIP ) == 0 ) ||
			( stripbase == 0 ) );

	// draw last strip/loop
	glDrawArrays( glmode, stripbase, subx - stripbase );

	DEINIT_GL_ARRAYS( itline );

#else // ALLOW_LINE_STRIP_RESTARTS

	// draw line strip/loop
	INIT_GL_ARRAYS( itline, 0, itline->NumVerts );
	glDrawArrays( glmode, 0, itline->NumVerts );
	DEINIT_GL_ARRAYS( itline );

#endif // ALLOW_LINE_STRIP_RESTARTS

	// set rasterizer state to default
	if ( flags & ITERFLAG_LS_ANTIALIASED )
		RO_LineSmooth( FALSE );
	RO_DefaultRasterizerState();

	// restore zcmp and zwrite state
	RO_RestoreDepthState( zcmpstate, zwritestate );

	// free intermediate storage
//	DO_KillTemporaries();
}


// draw 2-D triangle using iterated vertex attributes -------------------------
//
void D_DrawIterTriangle2( IterTriangle2 *ittri )
{
	ASSERT( ittri != NULL );
	ASSERT( gl_array_current == NULL );

	// get info and select texture
	GLTexInfo _texinfo;
	GLTexInfo *texinfo = NULL;
	if ( ITERPRIM_TEXTURED( ittri ) ) {
		texinfo = &_texinfo;
		DO_SelectTexture( ittri->texmap, texinfo );
	}

	// convert vertex info into opengl format
	DO_PrepVertices2( ittri->flags, ittri->Vtxs, 3, texinfo );

	// save zcmp and zwrite state
	int zcmpstate   = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();

	// configure rasterizer
	RO_InitRasterizerState( ittri->itertype, ittri->raststate, ittri->rastmask );
	RO_TextureCombineState( texcomb_decal );

	// draw triangle
	INIT_GL_ARRAYS( ittri, 0, 3 );
	glDrawArrays( GL_TRIANGLES, 0, 3 );
	DEINIT_GL_ARRAYS( ittri );

	// set rasterizer state to default
	RO_DefaultRasterizerState();

	// restore zcmp and zwrite state
	RO_RestoreDepthState( zcmpstate, zwritestate );

	// free intermediate storage
//	DO_KillTemporaries();
}


// draw 3-D triangle using iterated vertex attributes -------------------------
//
void D_DrawIterTriangle3( IterTriangle3 *ittri, dword cullmask )
{
	ASSERT( ittri != NULL );
	ASSERT( gl_array_current == NULL );

	// face sidedness culling (front/back)
	if ( ittri->flags & ITERFLAG_ONESIDED ) {

		//NOTE:
		// the ITERFLAG_BACKFACES flag means that
		// backfaces are _drawn_ not _culled_. the
		// bit for this is one in order for drawing
		// of frontfaces to be the default.

		ASSERT( ittri->plane != NULL );
		if ( ( PLANE_OFFSET( ittri->plane ) <= 0 ) ==
			 ( ittri->flags & ITERFLAG_BACKFACES ) ) {

			// cull faces if eye-point (origin) in
			// wrong half-space according to plane
			return;
		}
	}

	// transform vertices
	if ( iter_matrix != NULL ) {
		ittri = (IterTriangle3 *)
			DO_TransformVertices3( (iter_prim_s *)ittri, 3 );
	}

#ifdef PERFORM_PRE_CLIPPING

	// clip if necessary
	if ( cullmask != 0x00 ) {

		IterPolygon3 *clippoly =
			CLIP_VolumeIterTriangle3( ittri, View_Volume, cullmask );
		if ( clippoly == NULL )
			return;
		if ( clippoly != (IterPolygon3 *) ittri ) {
			prev_matrix = iter_matrix;
			iter_matrix = NULL;
			D_DrawIterPolygon3( clippoly, 0x00 );
			iter_matrix = prev_matrix;
			return;
		}
	}

#endif //  PERFORM_PRE_CLIPPING

	// get info and select texture
	GLTexInfo _texinfo;
	GLTexInfo *texinfo = NULL;
	if ( ITERPRIM_TEXTURED( ittri ) ) {
		texinfo = &_texinfo;
		DO_SelectTexture( ittri->texmap, texinfo );
	}

	// project and convert vertices
	DO_PrepVertices3( ittri->flags, ittri->Vtxs, 3, texinfo );

	// save zcmp and zwrite state
	int zcmpstate   = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();

	// configure rasterizer
	RO_InitRasterizerState( ittri->itertype, ittri->raststate, ittri->rastmask );
	RO_TextureCombineState( texcomb_decal );

	// draw triangle
	INIT_GL_ARRAYS( ittri, 0, 3 );
	glDrawArrays( GL_TRIANGLES, 0, 3 );
	DEINIT_GL_ARRAYS( ittri );

	// set rasterizer state to default
	RO_DefaultRasterizerState();

	// restore zcmp and zwrite state
	RO_RestoreDepthState( zcmpstate, zwritestate );

	// free intermediate storage
//	DO_KillTemporaries();
}


// draw 2-D rectangle using iterated vertex attributes ------------------------
//
void D_DrawIterRectangle2( IterRectangle2 *itrect )
{
	ASSERT( itrect != NULL );
	ASSERT( gl_array_current == NULL );

	// get info and select texture
	GLTexInfo _texinfo;
	GLTexInfo *texinfo = NULL;
	if ( ITERPRIM_TEXTURED( itrect ) ) {
		texinfo = &_texinfo;
		DO_SelectTexture( itrect->texmap, texinfo );
	}

	// convert vertex info into opengl format
	DO_PrepVertices2( itrect->flags, itrect->Vtxs, 4, texinfo );

	// save zcmp and zwrite state
	int zcmpstate   = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();

	// configure rasterizer
	RO_InitRasterizerState( itrect->itertype, itrect->raststate, itrect->rastmask );
	RO_TextureCombineState( texcomb_decal );

	// draw rectangle
	INIT_GL_ARRAYS( itrect, 0, 4 );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	DEINIT_GL_ARRAYS( itrect );

	// set rasterizer state to default
	RO_DefaultRasterizerState();

	// restore zcmp and zwrite state
	RO_RestoreDepthState( zcmpstate, zwritestate );

	// free intermediate storage
//	DO_KillTemporaries();
}


// draw 3-D rectangle using iterated vertex attributes ------------------------
//
void D_DrawIterRectangle3( IterRectangle3 *itrect, dword cullmask )
{
	ASSERT( itrect != NULL );
	ASSERT( gl_array_current == NULL );

	// face sidedness culling (front/back)
	if ( itrect->flags & ITERFLAG_ONESIDED ) {

		//NOTE:
		// the ITERFLAG_BACKFACES flag means that
		// backfaces are _drawn_ not _culled_. the
		// bit for this is one in order for drawing
		// of frontfaces to be the default.

		ASSERT( itrect->plane != NULL );
		if ( ( PLANE_OFFSET( itrect->plane ) <= 0 ) ==
			 ( itrect->flags & ITERFLAG_BACKFACES ) ) {

			// cull faces if eye-point (origin) in
			// wrong half-space according to plane
			return;
		}
	}

	// transform vertices
	if ( iter_matrix != NULL ) {
		itrect = (IterRectangle3 *)
			DO_TransformVertices3( (iter_prim_s *)itrect, 4 );
	}

#ifdef PERFORM_PRE_CLIPPING

	// clip if necessary
	if ( cullmask != 0x00 ) {

		IterPolygon3 *clippoly =
			CLIP_VolumeIterRectangle3( itrect, View_Volume, cullmask );
		if ( clippoly == NULL )
			return;
		if ( clippoly != (IterPolygon3 *) itrect ) {
			prev_matrix = iter_matrix;
			iter_matrix = NULL;
			D_DrawIterPolygon3( clippoly, 0x00 );
			iter_matrix = prev_matrix;
			return;
		}
	}

#endif //  PERFORM_PRE_CLIPPING

	// get info and select texture
	GLTexInfo _texinfo;
	GLTexInfo *texinfo = NULL;
	if ( ITERPRIM_TEXTURED( itrect ) ) {
		texinfo = &_texinfo;
		DO_SelectTexture( itrect->texmap, texinfo );
	}

	// project and convert vertices
	DO_PrepVertices3( itrect->flags, itrect->Vtxs, 4, texinfo );

	// save zcmp and zwrite state
	int zcmpstate   = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();

	// configure rasterizer
	RO_InitRasterizerState( itrect->itertype, itrect->raststate, itrect->rastmask );
	RO_TextureCombineState( texcomb_decal );

	// draw rectangle
	INIT_GL_ARRAYS( itrect, 0, 4 );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	DEINIT_GL_ARRAYS( itrect );

	// set rasterizer state to default
	RO_DefaultRasterizerState();

	// restore zcmp and zwrite state
	RO_RestoreDepthState( zcmpstate, zwritestate );

	// free intermediate storage
//	DO_KillTemporaries();
}


// draw 2-D n-gon using iterated vertex attributes ----------------------------
//
void D_DrawIterPolygon2( IterPolygon2 *itpoly )
{
	ASSERT( itpoly != NULL );
	ASSERT( itpoly->NumVerts > 2 );
	ASSERT( itpoly->NumVerts <= MAX_ITERPOLY_VERTICES );

	ASSERT( gl_array_current == NULL );

	// get info and select texture
	GLTexInfo _texinfo;
	GLTexInfo *texinfo = NULL;
	if ( ITERPRIM_TEXTURED( itpoly ) ) {
		texinfo = &_texinfo;
		DO_SelectTexture( itpoly->texmap, texinfo );
	}

	// convert vertex info into opengl format
	DO_PrepVertices2( itpoly->flags, itpoly->Vtxs, itpoly->NumVerts, texinfo );

	// save zcmp and zwrite state
	int zcmpstate   = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();

	// configure rasterizer
	RO_InitRasterizerState( itpoly->itertype, itpoly->raststate, itpoly->rastmask );
	RO_TextureCombineState( texcomb_decal );

	// draw n-gon
	INIT_GL_ARRAYS( itpoly, 0, itpoly->NumVerts );
	glDrawArrays( GL_POLYGON, 0, itpoly->NumVerts );
	DEINIT_GL_ARRAYS( itpoly );

	// set rasterizer state to default
	RO_DefaultRasterizerState();

	// restore zcmp and zwrite state
	RO_RestoreDepthState( zcmpstate, zwritestate );

	// free intermediate storage
//	DO_KillTemporaries();
}


// draw 3-D n-gon using iterated vertex attributes ----------------------------
//
void D_DrawIterPolygon3( IterPolygon3 *itpoly, dword cullmask )
{
	ASSERT( itpoly != NULL );
	ASSERT( itpoly->NumVerts > 2 );
	ASSERT( itpoly->NumVerts <= MAX_ITERPOLY_VERTICES );

	ASSERT( gl_array_current == NULL );

	// face sidedness culling (front/back)
	if ( itpoly->flags & ITERFLAG_ONESIDED ) {

		//NOTE:
		// the ITERFLAG_BACKFACES flag means that
		// backfaces are _drawn_ not _culled_. the
		// bit for this is one in order for drawing
		// of frontfaces to be the default.

		ASSERT( itpoly->plane != NULL );
		if ( ( PLANE_OFFSET( itpoly->plane ) <= 0 ) ==
			 ( itpoly->flags & ITERFLAG_BACKFACES ) ) {

			// cull faces if eye-point (origin) in
			// wrong half-space according to plane
			return;
		}
	}

	// transform vertices
	if ( iter_matrix != NULL ) {
		itpoly = (IterPolygon3 *)
			DO_TransformVertices3( (iter_prim_s *)itpoly, itpoly->NumVerts );
	}

#ifdef PERFORM_PRE_CLIPPING

	// clip if necessary
	if ( cullmask != 0x00 ) {

		IterPolygon3 *clippoly =
			CLIP_VolumeIterPolygon3( itpoly, View_Volume, cullmask );
		if ( clippoly == NULL )
			return;
		itpoly = clippoly;
	}

#endif //  PERFORM_PRE_CLIPPING

	// get info and select texture
	GLTexInfo _texinfo;
	GLTexInfo *texinfo = NULL;
	if ( ITERPRIM_TEXTURED( itpoly ) ) {
		texinfo = &_texinfo;
		DO_SelectTexture( itpoly->texmap, texinfo );
	}

	// project and convert vertices
	DO_PrepVertices3( itpoly->flags, itpoly->Vtxs, itpoly->NumVerts, texinfo );

	// save zcmp and zwrite state
	int zcmpstate   = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();

	// configure rasterizer
	RO_InitRasterizerState( itpoly->itertype, itpoly->raststate, itpoly->rastmask );
	RO_TextureCombineState( texcomb_decal );

	// draw n-gon
	INIT_GL_ARRAYS( itpoly, 0, itpoly->NumVerts );
	glDrawArrays( GL_POLYGON, 0, itpoly->NumVerts );
	DEINIT_GL_ARRAYS( itpoly );

	// set rasterizer state to default
	RO_DefaultRasterizerState();

	// restore zcmp and zwrite state
	RO_RestoreDepthState( zcmpstate, zwritestate );

	// free intermediate storage
//	DO_KillTemporaries();
}


// draw 2-D triangle strip using iterated vertex attributes -------------------
//
void D_DrawIterTriStrip2( IterTriStrip2 *itstrip )
{
	ASSERT( itstrip != NULL );
	ASSERT( itstrip->NumVerts > 2 );
//	ASSERT( itstrip->NumVerts <= MAX_ITERSTRIP_VERTICES );

	ASSERT( gl_array_current == NULL );

	// get info and select texture
	GLTexInfo _texinfo;
	GLTexInfo *texinfo = NULL;
	if ( ITERPRIM_TEXTURED( itstrip ) ) {
		texinfo = &_texinfo;
		DO_SelectTexture( itstrip->texmap, texinfo );
	}

	// convert vertex info into opengl format
	DO_PrepVertices2( itstrip->flags, itstrip->Vtxs, itstrip->NumVerts, texinfo );

	// save zcmp and zwrite state
	int zcmpstate   = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();

	// configure rasterizer
	RO_InitRasterizerState( itstrip->itertype, itstrip->raststate, itstrip->rastmask );
	RO_TextureCombineState( texcomb_decal );

	//NOTE:
	// drawing sequence will be
	// (012), (213), (234), (435), ...

	// draw triangle strip
	INIT_GL_ARRAYS( itstrip, 0, itstrip->NumVerts );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, itstrip->NumVerts );
	DEINIT_GL_ARRAYS( itstrip );

	// set rasterizer state to default
	RO_DefaultRasterizerState();

	// restore zcmp and zwrite state
	RO_RestoreDepthState( zcmpstate, zwritestate );

	// free intermediate storage
	DO_KillTemporaries();
}


// draw triangle strip as single triangles ------------------------------------
//
PRIVATE
void DO_DrawStripAsTriangles( IterTriStrip3 *itstrip, dword cullmask )
{
	ASSERT( itstrip != NULL );

	//NOTE:
	// drawing sequence will be
	// (012), (213), (234), (435), ...

	IterTriangle3 ittri;

	ittri.flags		= itstrip->flags | ITERFLAG_VERTEXREFS | ITERFLAG_NONDESTRUCTIVE;
	ittri.itertype	= itstrip->itertype;
	ittri.raststate	= itstrip->raststate;
	ittri.rastmask	= itstrip->rastmask;
	ittri.texmap	= itstrip->texmap;

	IterVertex3 **dstrefs = (IterVertex3 **) ittri.Vtxs;

	if ( itstrip->flags & ITERFLAG_VERTEXREFS ) {

		IterVertex3 **refs = (IterVertex3 **) itstrip->Vtxs;

		// draw triangle strip
		if ( itstrip->NumVerts & 0x01 ) {

			// draw odd number of triangles
			dstrefs[ 0 ] = refs[ 0 ];
			dstrefs[ 1 ] = refs[ 1 ];
			dstrefs[ 2 ] = refs[ 2 ];
			D_DrawIterTriangle3( &ittri, cullmask );

			for ( int v = 3; v < itstrip->NumVerts; v+=2 ) {

				dstrefs[ 0 ] = refs[ v-1 ];
				dstrefs[ 1 ] = refs[ v-2 ];
				dstrefs[ 2 ] = refs[ v+0 ];
				D_DrawIterTriangle3( &ittri, cullmask );

				dstrefs[ 0 ] = refs[ v-1 ];
				dstrefs[ 1 ] = refs[ v+0 ];
				dstrefs[ 2 ] = refs[ v+1 ];
				D_DrawIterTriangle3( &ittri, cullmask );
			}

		} else {

			// draw even number of triangles
			for ( int v = 2; v < itstrip->NumVerts; v+=2 ) {

				dstrefs[ 0 ] = refs[ v-2 ];
				dstrefs[ 1 ] = refs[ v-1 ];
				dstrefs[ 2 ] = refs[ v+0 ];
				D_DrawIterTriangle3( &ittri, cullmask );

				dstrefs[ 0 ] = refs[ v+0 ];
				dstrefs[ 1 ] = refs[ v-1 ];
				dstrefs[ 2 ] = refs[ v+1 ];
				D_DrawIterTriangle3( &ittri, cullmask );
			}
		}

	} else {

		// draw triangle strip
		if ( itstrip->NumVerts & 0x01 ) {

			// draw odd number of triangles
			dstrefs[ 0 ] = &itstrip->Vtxs[ 0 ];
			dstrefs[ 1 ] = &itstrip->Vtxs[ 1 ];
			dstrefs[ 2 ] = &itstrip->Vtxs[ 2 ];
			D_DrawIterTriangle3( &ittri, cullmask );

			for ( int v = 3; v < itstrip->NumVerts; v+=2 ) {

				dstrefs[ 0 ] = &itstrip->Vtxs[ v-1 ];
				dstrefs[ 1 ] = &itstrip->Vtxs[ v-2 ];
				dstrefs[ 2 ] = &itstrip->Vtxs[ v+0 ];
				D_DrawIterTriangle3( &ittri, cullmask );

				dstrefs[ 0 ] = &itstrip->Vtxs[ v-1 ];
				dstrefs[ 1 ] = &itstrip->Vtxs[ v+0 ];
				dstrefs[ 2 ] = &itstrip->Vtxs[ v+1 ];
				D_DrawIterTriangle3( &ittri, cullmask );
			}

		} else {

			// draw even number of triangles
			for ( int v = 2; v < itstrip->NumVerts; v+=2 ) {

				dstrefs[ 0 ] = &itstrip->Vtxs[ v-2 ];
				dstrefs[ 1 ] = &itstrip->Vtxs[ v-1 ];
				dstrefs[ 2 ] = &itstrip->Vtxs[ v+0 ];
				D_DrawIterTriangle3( &ittri, cullmask );

				dstrefs[ 0 ] = &itstrip->Vtxs[ v+0 ];
				dstrefs[ 1 ] = &itstrip->Vtxs[ v-1 ];
				dstrefs[ 2 ] = &itstrip->Vtxs[ v+1 ];
				D_DrawIterTriangle3( &ittri, cullmask );
			}
		}
	}
}


// draw 3-D triangle strip using iterated vertex attributes -------------------
//
void D_DrawIterTriStrip3( IterTriStrip3 *itstrip, dword cullmask )
{
	ASSERT( itstrip != NULL );
	ASSERT( itstrip->NumVerts > 2 );
//	ASSERT( itstrip->NumVerts <= MAX_ITERSTRIP_VERTICES );

	ASSERT( gl_array_current == NULL );

	//TODO:
	// face culling. (ITERFLAG_PLANESTRIP,...)
/*
	// transform vertices
	if ( iter_matrix != NULL ) {
		itstrip = (IterTriStrip3 *)
			DO_TransformVertices3( (iter_prim_s *)itstrip, itstrip->NumVerts );
	}

#ifdef PERFORM_PRE_CLIPPING

	// clip if necessary
	if ( cullmask != 0x00 ) {

		prev_matrix = iter_matrix;
		iter_matrix = NULL;
		DO_DrawStripAsTriangles( itstrip, cullmask );
		iter_matrix = prev_matrix;
		return;
*/
		//TODO:
/*
		IterPolygon3 *clipstrip =
			CLIP_VolumeIterTriStrip3( itstrip, View_Volume, cullmask );
		if ( clipstrip == NULL )
			return;
		itstrip = clipstrip;
*/
/*	}

#endif
*/
	// get info and select texture
	GLTexInfo _texinfo;
	GLTexInfo *texinfo = NULL;
	if ( ITERPRIM_TEXTURED( itstrip ) ) {
		texinfo = &_texinfo;
		DO_SelectTexture( itstrip->texmap, texinfo );
	}

	// project and convert vertices
//	DO_PrepVertices3( itstrip->flags, itstrip->Vtxs, itstrip->NumVerts, texinfo );

	// use gl projection matrix
	DO_PrepProjection( itstrip->flags, itstrip->Vtxs, itstrip->NumVerts, texinfo );

	// save zcmp and zwrite state
	int zcmpstate   = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();

	// configure rasterizer
	RO_InitRasterizerState( itstrip->itertype, itstrip->raststate, itstrip->rastmask );
	RO_TextureCombineState( texcomb_decal );

	//NOTE:
	// drawing sequence will be
	// (012), (213), (234), (435), ...

	// draw triangle strip
	INIT_GL_ARRAYS( itstrip, 0, itstrip->NumVerts );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, itstrip->NumVerts );
	DEINIT_GL_ARRAYS( itstrip );

	// set rasterizer state to default
	RO_DefaultRasterizerState();

	// restore zcmp and zwrite state
	RO_RestoreDepthState( zcmpstate, zwritestate );

	// free intermediate storage
	DO_KillTemporaries();

	// restore gl projection matrix
	DO_KillProjection();
}


// enable needed vertex arrays and set appropriate pointers -------------------
//
INLINE
void DO_PrepArrays( dword arrayinfo )
{
	ASSERT( ITERARRAY_USE_COLOR   == 0x01 );
	ASSERT( ITERARRAY_USE_TEXTURE == 0x02 );

	int stateindx = arrayinfo & 0x03;

	switch ( stateindx ) {

		case 0x00:
			RO_ClientState( VTXARRAY_VERTICES );
			if ( RO_ArrayMakeCurrent( VTXPTRS_DO_ITER_1, gl_array_vtxs ) ) {
				glVertexPointer( 3, GL_FLOAT, sizeof( GLVertex3 ), &gl_array_vtxs->x );
			}
			break;

		case 0x01:
			RO_ClientState( VTXARRAY_VERTICES | VTXARRAY_COLORS );
			if ( RO_ArrayMakeCurrent( VTXPTRS_DO_ITER_2, gl_array_vtxs ) ) {
				glVertexPointer( 3, GL_FLOAT, sizeof( GLVertex3 ), &gl_array_vtxs->x );
				glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( GLVertex3 ), &gl_array_vtxs->r );
			}
			break;

		case 0x02:
			RO_ClientState( VTXARRAY_VERTICES | VTXARRAY_TEXCOORDS );
			if ( RO_ArrayMakeCurrent( VTXPTRS_DO_ITER_3, gl_array_vtxs ) ) {
				glVertexPointer( 3, GL_FLOAT, sizeof( GLVertex3 ), &gl_array_vtxs->x );
				glTexCoordPointer( 4, GL_FLOAT, sizeof( GLVertex3 ), &gl_array_vtxs->s );
			}
			break;

		case 0x03:
			RO_ClientState( VTXARRAY_VERTICES | VTXARRAY_COLORS | VTXARRAY_TEXCOORDS );
			if ( RO_ArrayMakeCurrent( VTXPTRS_DO_ITER_4, gl_array_vtxs ) ) {
				glVertexPointer( 3, GL_FLOAT, sizeof( GLVertex3 ), &gl_array_vtxs->x );
				glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( GLVertex3 ), &gl_array_vtxs->r );
				glTexCoordPointer( 4, GL_FLOAT, sizeof( GLVertex3 ), &gl_array_vtxs->s );
			}
			break;
	}
}


// lock (specify and convert) array of 2-D iterated vertices ------------------
//
void D_LockIterArray2( IterArray2 *itarray, dword first, dword count )
{
	//NOTE:
	// on the first array lock (itarray != NULL) the entire
	// vertex array will be processed (i.e. converted).
	// then, only the subrange [first,first+count-1] will be
	// locked in the gl.
	// subsequent calls with (itarray == NULL) may follow to
	// gl-lock different subranges. also, on these calls the
	// following fields of the IterArray2 are allowed to be
	// modified to render using a different rasterizer state:
	// - IterArray2::itertype
	// - IterArray2::raststate
	// - IterArray2::rastmask
	// - IterArray2::texmap

	// array must not be already locked if new array
	ASSERT( ( gl_array_current == NULL ) || ( itarray == NULL ) );
	if ( gl_array_current != NULL ) {
		if ( itarray != NULL ) {
			// full unlock for release-safety
			D_UnlockIterArray();
		}
	}

	// itarray may be NULL to reuse last
	int newarray = ( itarray != NULL );
	if ( newarray ) {
		gl_array_current = (IterArray3 *) itarray;
	} else {
		itarray = (IterArray2 *) gl_array_current;
	}

	ASSERT( itarray != NULL );
	ASSERT( first < (dword)itarray->NumVerts );
	ASSERT( first + count <= (dword)itarray->NumVerts );

	// select texture
	GLTexInfo *texinfo = NULL;
	if ( itarray->arrayinfo & ITERARRAY_USE_TEXTURE ) {

		ASSERT( ITERPRIM_TEXTURED( itarray ) );
		texinfo = &gl_array_texinfo;

		// fetch texture map
		TextureMap *texmap = itarray->texmap;
		ASSERT( texmap != NULL );

		// fill texinfo structure and determine coscale/aratio
		RO_TextureMap2GLTexInfo( texinfo, texmap );

		// also select source if there will only be one texture
		if ( itarray->arrayinfo & ITERARRAY_GLOBAL_TEXTURE ) {

			// enforce texel source
			RO_SelectTexelSource( texinfo );
		}
	}

	// setup array on first lock
	if ( newarray ) {

		// convert vertex array into opengl format
		DO_PrepVertices2( itarray->flags, itarray->Vtxs, itarray->NumVerts, texinfo );

		// specify gl arrays
		DO_PrepArrays( itarray->arrayinfo );

		// save zcmp and zwrite state
		gl_array_zcmpstate   = RO_DepthCmpEnabled();
		gl_array_zwritestate = RO_DepthWriteEnabled();
	}

	// configure rasterizer
	RO_InitRasterizerState( itarray->itertype, itarray->raststate, itarray->rastmask );
	RO_TextureCombineState( texcomb_decal );

	// remember locked range
	gl_lockrange_base	= first;
	gl_lockrange_beyond	= first + count;
}


// lock (specify and convert) array of 3-D iterated vertices ------------------
//
void D_LockIterArray3( IterArray3 *itarray, dword first, dword count )
{
	//NOTE:
	// on the first array lock (itarray != NULL) the entire
	// vertex array will be processed (i.e. converted).
	// then, only the subrange [first,first+count-1] will be
	// locked in the gl.
	// subsequent calls with (itarray == NULL) may follow to
	// gl-lock different subranges. also, on these calls the
	// following fields of the IterArray3 are allowed to be
	// modified to render using a different rasterizer state:
	// - IterArray3::itertype
	// - IterArray3::raststate
	// - IterArray3::rastmask
	// - IterArray3::texmap

	// array must not be already locked if new array
	ASSERT( ( gl_array_current == NULL ) || ( itarray == NULL ) );
	if ( gl_array_current != NULL ) {
		if ( itarray != NULL ) {
			// full unlock for release-safety
			D_UnlockIterArray();
		}
	}

	// itarray may be NULL to reuse last
	int newarray = ( itarray != NULL );
	if ( newarray ) {
		gl_array_current = itarray;
	} else {
		itarray = gl_array_current;
	}

	ASSERT( itarray != NULL );
	ASSERT( first < (dword)itarray->NumVerts );
	ASSERT( first + count <= (dword)itarray->NumVerts );

	// select texture
	GLTexInfo *texinfo = NULL;
	if ( itarray->arrayinfo & ITERARRAY_USE_TEXTURE ) {

		ASSERT( ITERPRIM_TEXTURED( itarray ) );
		texinfo = &gl_array_texinfo;

		// fetch texture map
		TextureMap *texmap = itarray->texmap;
		ASSERT( texmap != NULL );

		// fill texinfo structure and determine coscale/aratio
		RO_TextureMap2GLTexInfo( texinfo, texmap );

		// also select source if there will only be one texture
		if ( itarray->arrayinfo & ITERARRAY_GLOBAL_TEXTURE ) {

			// enforce texel source
			RO_SelectTexelSource( texinfo );
		}
	}

	// setup array on first lock
	if ( newarray ) {

		// project and convert vertices
//		DO_PrepVertices3( itarray->flags, itarray->Vtxs, itarray->NumVerts, texinfo );

		// use gl projection matrix
		DO_PrepProjection( itarray->flags, itarray->Vtxs, itarray->NumVerts, texinfo );

		// specify gl arrays
		DO_PrepArrays( itarray->arrayinfo );

		// save zcmp and zwrite state
		gl_array_zcmpstate   = RO_DepthCmpEnabled();
		gl_array_zwritestate = RO_DepthWriteEnabled();
	}

	// configure rasterizer
	RO_InitRasterizerState( itarray->itertype, itarray->raststate, itarray->rastmask );
	RO_TextureCombineState( texcomb_decal );

	// remember locked range
	gl_lockrange_base	= first;
	gl_lockrange_beyond	= first + count;
}


// unlock (free) array of iterated vertices -----------------------------------
//
void D_UnlockIterArray()
{
	// array must be locked
	ASSERT( gl_array_current != NULL );

	if ( gl_array_current != NULL ) {

		IterArray3 *itarray = gl_array_current;
		gl_array_current = NULL;

		// disable gl arrays
//		RO_ClientState( VTXARRAY_NONE );

		// free intermediate storage
		DO_KillTemporaries();

		// restore gl projection matrix
		DO_KillProjection();

		// set rasterizer state to default
		RO_DefaultRasterizerState();

		// restore zcmp and zwrite state
		RO_RestoreDepthState( gl_array_zcmpstate, gl_array_zwritestate );
	}
}


// convert mode spec (ITERMODE_xx) into GL_xx mode spec -----------------------
//
static GLenum gl_array_modes[] = {

	GL_TRIANGLES,		// ITERARRAY_MODE_TRIANGLES
	GL_TRIANGLE_STRIP,	// ITERARRAY_MODE_TRISTRIP
	GL_TRIANGLE_FAN,	// ITERARRAY_MODE_TRIFAN
};


// draw vertex array using iterated vertex attributes -------------------------
//
void D_DrawIterArray( dword mode, dword first, dword count, dword cullmask )
{
	IterArray3 *itarray = gl_array_current;
	ASSERT( itarray != NULL );

	ASSERT( mode < ITERARRAY_MODE_NUM_MODES );
	ASSERT( first >= gl_lockrange_base );
	ASSERT( first + count <= gl_lockrange_beyond );

	// select another texture if last cannot be safely reused
	if ( itarray->arrayinfo & ITERARRAY_USE_TEXTURE ) {
		ASSERT( itarray->texmap != NULL );
		if ( ( itarray->arrayinfo & ITERARRAY_GLOBAL_TEXTURE ) == 0 ) {
			DO_SelectTexture( itarray->texmap, &gl_array_texinfo );
		}
	}

	// determine gl mode
	ASSERT( CALC_NUM_ARRAY_ENTRIES( gl_array_modes ) ==
			ITERARRAY_MODE_NUM_MODES );
	GLenum glmode = gl_array_modes[ mode ];

	// draw linear array subrange
	glDrawArrays( glmode, first, count );
}


// draw vertex array with indexes using iterated vertex attributes ------------
//
void D_DrawIterArrayIndexed( dword mode, dword count, uint16 *indexes, dword cullmask )
{
	ASSERT( indexes != NULL );

	IterArray3 *itarray = gl_array_current;
	ASSERT( itarray != NULL );

	ASSERT( mode < ITERARRAY_MODE_NUM_MODES );

	// select another texture if last cannot be safely reused
	if ( itarray->arrayinfo & ITERARRAY_USE_TEXTURE ) {
		ASSERT( itarray->texmap != NULL );
		if ( ( itarray->arrayinfo & ITERARRAY_GLOBAL_TEXTURE ) == 0 ) {
			DO_SelectTexture( itarray->texmap, &gl_array_texinfo );
		}
	}

	// determine gl mode
	ASSERT( CALC_NUM_ARRAY_ENTRIES( gl_array_modes ) ==
			ITERARRAY_MODE_NUM_MODES );
	GLenum glmode = gl_array_modes[ mode ];

	// draw indexed array elements
	glDrawElements( glmode, count, GL_UNSIGNED_SHORT, indexes );
}



