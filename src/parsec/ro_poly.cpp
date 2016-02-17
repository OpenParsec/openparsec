/*
 * PARSEC - Polygon Rendering Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:33 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999
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
#include <stddef.h>
#include <stdlib.h>

// compilation flags/debug support
#include "config.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "vid_defs.h"


// local module header
#include "ro_poly.h"

// proprietary module headers
#include "con_aux.h"
#include "e_color.h"
#include "ro_api.h"
#include "ro_supp.h"

#include "debug.h"
// mathematics header
#include "utl_math.h"


// flags
//#define DISABLE_FLAT_FACES
//#define DISABLE_TEXTURED_FACES
//#define DRAW_TEXTURES_AS_FLAT
#define NO_NEGATIVE_W_COORDINATES



// static vertex array for one polygon ----------------------------------------
//
#define MAX_NUM_GLVTXS	64
static GLVertex3 poly_vtxs[ MAX_NUM_GLVTXS ];


// shortcut whether shading changes the depth buffer config -------------------
//
#define DEPTH_CONFIG_CHANGE	( FACE_SHADING_NODEPTHCMP | FACE_SHADING_NODEPTHWRITE )


// color combination macros ---------------------------------------------------
//
#define COLOR_ADD(t,a,b)	{ \
	(t) = (int)(a) + (int)(b); \
	if ( (t) > 255 ) \
		(t) = 255; \
}

#define COLOR_MUL(t,a,b)	{ \
	(t) = ( (int)(a) * (int)(b) ) / 255; \
	if ( (t) > 255 ) \
		(t) = 255; \
}


// perform face animations for a polygon filled with solid color --------------
//
INLINE
void RO_SolidPolyFaceAnims( GenObject *baseobj, Face *face, colrgba_s *facecolor )
{
	ASSERT( baseobj != NULL );
	ASSERT( face != NULL );
	ASSERT( facecolor != NULL );

	// extinfo is optional
	FaceExtInfo *extinfo = face->ExtInfo;
	if ( ( extinfo == NULL ) || ( extinfo->Flags == FACE_EXT_NONE ) ) {
		return;
	}
	// and may be disabled temporarily, without erasing the previous flags
	if ( extinfo->Flags & FACE_EXT_DISABLED ) {
		return;
	}

	// ignore out-of-range anim ids
	ASSERT( extinfo->StateId < baseobj->NumFaceAnims );
	if ( extinfo->StateId >= baseobj->ActiveFaceAnims ) {
		return;
	}

	// retrieve object-global face anim
	FaceAnimState *animstate = &baseobj->FaceAnimStates[ extinfo->StateId ];

	// color animation
	if ( extinfo->Flags & FACE_EXT_ANIMATECOLORS ) {

		dword basemode = animstate->ColFlags & FACE_ANIM_BASE_MASK;
		if ( basemode == FACE_ANIM_BASEIGNORE ) {

			facecolor->R = animstate->ColOutput.R;
			facecolor->G = animstate->ColOutput.G;
			facecolor->B = animstate->ColOutput.B;
			facecolor->A = animstate->ColOutput.A;

		} else {

			int colr, colg, colb, cola;
			if ( basemode == FACE_ANIM_BASEADD ) {
				COLOR_ADD( colr, facecolor->R, animstate->ColOutput.R );
				COLOR_ADD( colg, facecolor->G, animstate->ColOutput.G );
				COLOR_ADD( colb, facecolor->B, animstate->ColOutput.B );
				COLOR_ADD( cola, facecolor->A, animstate->ColOutput.A );
			} else {
				ASSERT( basemode == FACE_ANIM_BASEMUL );
				COLOR_MUL( colr, facecolor->R, animstate->ColOutput.R );
				COLOR_MUL( colg, facecolor->G, animstate->ColOutput.G );
				COLOR_MUL( colb, facecolor->B, animstate->ColOutput.B );
				COLOR_MUL( cola, facecolor->A, animstate->ColOutput.A );
			}

			facecolor->R = colr;
			facecolor->G = colg;
			facecolor->B = colb;
			facecolor->A = cola;
		}
	}
}


// draw polygon filled with solid color ---------------------------------------
//
void RO_SolidFillPoly( SVertexExList *svertexlist, GenObject *baseobj, Face *face, int colorsvalid )
{
	ASSERT( svertexlist != NULL );
	ASSERT( baseobj != NULL );
	ASSERT( face != NULL );

	SPointEx* slist  = svertexlist->Vertices;
	int       vcount = svertexlist->VCount;

	ASSERT( vcount > 2 );
	ASSERT( vcount <= MAX_NUM_GLVTXS );

	// fetch constant face color
	colrgba_s facecolor = *(colrgba_s*)&face->ColorRGB;
	if ( face->ShadingFlags & FACE_SHADING_USECOLORINDEX ) {
		COLINDX_TO_RGBA( &facecolor, face->ColorIndx );
	}

	// perform face animations
	RO_SolidPolyFaceAnims( baseobj, face, &facecolor );

	// configure rasterizer
	dword itertype  = face->ShadingIter;
	dword raststate	= rast_chromakeyoff;
	dword rastmask	= rast_mask_zbuffer | rast_mask_texclamp |
					  rast_mask_mipmap  | rast_mask_texfilter;
	
	// save zcmp and zwrite state
	int zcmpstate   = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();
	
	if ( face->ShadingFlags & DEPTH_CONFIG_CHANGE ) {
		if ( face->ShadingFlags & FACE_SHADING_NODEPTHCMP ) {
			raststate &= ~rast_zcompare;
			rastmask  &= ~rast_mask_zcompare;
		}
		if ( face->ShadingFlags & FACE_SHADING_NODEPTHWRITE ) {
			raststate &= ~rast_zwrite;
			rastmask  &= ~rast_mask_zwrite;
		}
	}

	RO_InitRasterizerState( itertype, raststate, rastmask );

	// force vertex alpha to one if specified
	if ( face->ShadingFlags & FACE_SHADING_NOVERTEXALPHA ) {
		facecolor.A = 255;
	}

	// constant color and alpha must ignore iterated color
	if ( face->ShadingFlags & FACE_SHADING_CONSTANTCOLOR ) {
		colorsvalid = FALSE;
	}

	// generate vertex list
	if ( colorsvalid ) {

		// face color, iterated color is vertex color
		for ( int v = 0; v < vcount; v++, slist++ ) {

			poly_vtxs[ v ].x = slist->X;
			poly_vtxs[ v ].y = slist->Y;
			poly_vtxs[ v ].z = DEPTH_TO_FLOAT( slist->View_Z ) * OPENGL_DEPTH_RANGE;

			int colr, colg, colb;
			COLOR_MUL( colr, ((colrgba_s*)&slist->RGBA)->R, facecolor.R );
			COLOR_MUL( colg, ((colrgba_s*)&slist->RGBA)->G, facecolor.G );
			COLOR_MUL( colb, ((colrgba_s*)&slist->RGBA)->B, facecolor.B );

			poly_vtxs[ v ].r = colr;
			poly_vtxs[ v ].g = colg;
			poly_vtxs[ v ].b = colb;
			poly_vtxs[ v ].a = facecolor.A;
		}

	} else {

		// face color, iterated color is default (white)
		for ( int v = 0; v < vcount; v++, slist++ ) {

			poly_vtxs[ v ].x = slist->X;
			poly_vtxs[ v ].y = slist->Y;
			poly_vtxs[ v ].z = DEPTH_TO_FLOAT( slist->View_Z ) * OPENGL_DEPTH_RANGE;

			poly_vtxs[ v ].r = facecolor.R;
			poly_vtxs[ v ].g = facecolor.G;
			poly_vtxs[ v ].b = facecolor.B;
			poly_vtxs[ v ].a = facecolor.A;
		}
	}

	// specify vertex arrays
	RO_ClientState( VTXARRAY_VERTICES | VTXARRAY_COLORS );
	if ( RO_ArrayMakeCurrent( VTXPTRS_RO_POLY_1, poly_vtxs ) ) {
		glVertexPointer( 3, GL_FLOAT, sizeof( GLVertex3 ), &poly_vtxs->x );
		glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( GLVertex3 ), &poly_vtxs->r );
	}

	// actually draw polygon
#ifndef GL_POLYGON
	// GL_POLYGON isn't supported in GLES or modern GL.
	// FIXME: naive triangle fans can't represent all simple polygons!
	glDrawArrays( GL_TRIANGLE_FAN, 0, vcount );
#else
	glDrawArrays( GL_POLYGON, 0, vcount );
#endif

	// disable vertex arrays
//	RO_ClientState( VTXARRAY_NONE );

	// set rasterizer state to default
	RO_DefaultRasterizerState();

	if ( face->ShadingFlags & DEPTH_CONFIG_CHANGE ) {
		// restore zcmp and zwrite state
		RO_RestoreDepthState( zcmpstate, zwritestate );
	}
}


// perform face animations for a texture mapped polygon -----------------------
//
INLINE
TextureMap *RO_TexMapPolyFaceAnims( GenObject *baseobj, Face *face, colrgba_s *facecolor )
{
	ASSERT( baseobj != NULL );
	ASSERT( face != NULL );
	ASSERT( facecolor != NULL );

	// fetch base texture as default
	TextureMap *texmap = face->TexMap;

	// extinfo is optional
	FaceExtInfo *extinfo = face->ExtInfo;
	if ( ( extinfo == NULL ) || ( extinfo->Flags == FACE_EXT_NONE ) ) {
		return texmap;
	}
	// and may be disabled temporarily, without erasing the previous flags
	if ( extinfo->Flags & FACE_EXT_DISABLED ) {
		return texmap;
	}

	// ignore out-of-range anim ids
	ASSERT( extinfo->StateId < baseobj->NumFaceAnims );
	if ( extinfo->StateId >= baseobj->ActiveFaceAnims ) {
		return texmap;
	}

	// retrieve object-global face anim
	FaceAnimState *animstate = &baseobj->FaceAnimStates[ extinfo->StateId ];

	// texture animation
	if ( extinfo->Flags & FACE_EXT_ANIMATETEXTURES ) {

		texanim_s *texanim = animstate->TexAnim;
		ASSERT( texanim != NULL );

		ASSERT( texanim->tex_table != NULL );
		texfrm_s *curtexframe = &texanim->tex_table[ animstate->tex_pos ];

		texmap = curtexframe->texmap;
	}

	// color animation
	if ( extinfo->Flags & FACE_EXT_ANIMATECOLORS ) {

		dword basemode = animstate->ColFlags & FACE_ANIM_BASE_MASK;
		if ( basemode == FACE_ANIM_BASEIGNORE ) {

			facecolor->R = animstate->ColOutput.R;
			facecolor->G = animstate->ColOutput.G;
			facecolor->B = animstate->ColOutput.B;
			facecolor->A = animstate->ColOutput.A;

		} else {

			int colr, colg, colb, cola;
			if ( basemode == FACE_ANIM_BASEADD ) {
				COLOR_ADD( colr, facecolor->R, animstate->ColOutput.R );
				COLOR_ADD( colg, facecolor->G, animstate->ColOutput.G );
				COLOR_ADD( colb, facecolor->B, animstate->ColOutput.B );
				COLOR_ADD( cola, facecolor->A, animstate->ColOutput.A );
			} else {
				ASSERT( basemode == FACE_ANIM_BASEMUL );
				COLOR_MUL( colr, facecolor->R, animstate->ColOutput.R );
				COLOR_MUL( colg, facecolor->G, animstate->ColOutput.G );
				COLOR_MUL( colb, facecolor->B, animstate->ColOutput.B );
				COLOR_MUL( cola, facecolor->A, animstate->ColOutput.A );
			}

			facecolor->R = colr;
			facecolor->G = colg;
			facecolor->B = colb;
			facecolor->A = cola;
		}
	}

	return texmap;
}


// draw texture mapped polygon ------------------------------------------------
//
void RO_TexMapPoly( SVertexExList *svertexlist, GenObject *baseobj, Face *face, int colorsvalid )
{
	ASSERT( svertexlist != NULL );
	ASSERT( baseobj != NULL );
	ASSERT( face != NULL );

	SPointEx* slist  = svertexlist->Vertices;
	int       vcount = svertexlist->VCount;

	ASSERT( vcount > 2 );
	ASSERT( vcount <= MAX_NUM_GLVTXS );

	// fetch constant face color
	colrgba_s facecolor = *(colrgba_s*)&face->ColorRGB;
	if ( face->ShadingFlags & FACE_SHADING_USECOLORINDEX ) {
		COLINDX_TO_RGBA( &facecolor, face->ColorIndx );
	}

	// perform face animations
	TextureMap *texmap = RO_TexMapPolyFaceAnims( baseobj, face, &facecolor );
	ASSERT( texmap != NULL );

	ASSERT( texmap->TexelFormat > TEXFMT_STANDARD );
	ASSERT( texmap->TexelFormat <= TEXFMT_LUMINANCE_8 );

	// fill texinfo structure and determine scale factor for coordinates
	GLTexInfo texinfo;
	RO_TextureMap2GLTexInfo( &texinfo, texmap );

	float scale_u = texinfo.coscale;
	float scale_v = texinfo.aratio * scale_u;

	// enforce texel source
	RO_SelectTexelSource( &texinfo );

	// configure rasterizer
	dword itertype  = face->ShadingIter;
	dword raststate = AUX_DISABLE_TEXTURE_WRAPPING ?
					  ( rast_texclamp | rast_chromakeyoff ) :
					  ( rast_texwrap  | rast_chromakeyoff );
	dword rastmask  = rast_mask_zbuffer;
	
	// save zcmp and zwrite state
	int zcmpstate = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();
	
	if ( face->ShadingFlags & DEPTH_CONFIG_CHANGE ) {

		if ( face->ShadingFlags & FACE_SHADING_NODEPTHCMP ) {
			raststate &= ~rast_zcompare;
			rastmask  &= ~rast_mask_zcompare;
		}
		if ( face->ShadingFlags & FACE_SHADING_NODEPTHWRITE ) {
			raststate &= ~rast_zwrite;
			rastmask  &= ~rast_mask_zwrite;
		}
	}

	RO_InitRasterizerState( itertype, raststate, rastmask );
	RO_TextureCombineState( texcomb_decal );

	// force vertex alpha to one if specified
	if ( face->ShadingFlags & FACE_SHADING_NOVERTEXALPHA ) {
		facecolor.A = 255;
	}

	// set constant color and alpha
	if ( face->ShadingFlags & FACE_SHADING_CONSTANTCOLOR ) {
		glColor4ub( facecolor.R, facecolor.G, facecolor.B, facecolor.A );
		colorsvalid = FALSE;
	}

	// calculate projective mapping
	geomv_t *mapping = (geomv_t *) face->CurTexXmatrx;

	float A = GEOMV_TO_FLOAT( mapping[ 0 ] ) * scale_u;
	float B = GEOMV_TO_FLOAT( mapping[ 1 ] ) * scale_u;
	float C = GEOMV_TO_FLOAT( mapping[ 2 ] ) * scale_u;
	float D = GEOMV_TO_FLOAT( mapping[ 3 ] ) * scale_v;
	float E = GEOMV_TO_FLOAT( mapping[ 4 ] ) * scale_v;
	float F = GEOMV_TO_FLOAT( mapping[ 5 ] ) * scale_v;
	float G = GEOMV_TO_FLOAT( mapping[ 6 ] );
	float H = GEOMV_TO_FLOAT( mapping[ 7 ] );
	float I = GEOMV_TO_FLOAT( mapping[ 8 ] );

	// generate vertex list
	if ( face->ShadingFlags & FACE_SHADING_FACECOLOR ) {

		if ( colorsvalid ) {

			// face color, iterated color is vertex color
			for ( int v = 0; v < vcount; v++, slist++ ) {

				poly_vtxs[ v ].x = slist->X;
				poly_vtxs[ v ].y = slist->Y;
				poly_vtxs[ v ].z = DEPTH_TO_FLOAT( slist->View_Z ) * OPENGL_DEPTH_RANGE;

				int colr, colg, colb;
				COLOR_MUL( colr, ((colrgba_s*)&slist->RGBA)->R, facecolor.R );
				COLOR_MUL( colg, ((colrgba_s*)&slist->RGBA)->G, facecolor.G );
				COLOR_MUL( colb, ((colrgba_s*)&slist->RGBA)->B, facecolor.B );

				poly_vtxs[ v ].r = colr;
				poly_vtxs[ v ].g = colg;
				poly_vtxs[ v ].b = colb;
				poly_vtxs[ v ].a = facecolor.A;

				float X = slist->X - Screen_XOfs;
				float Y = slist->Y - Screen_YOfs;

				float U = A * X + B * Y + C * D_Value;
				float V = D * X + E * Y + F * D_Value;
				float W = G * X + H * Y + I * D_Value;

#ifdef NO_NEGATIVE_W_COORDINATES

				if ( W < 0 ) {
					U *= -1;
					V *= -1;
					W *= -1;
				}
#endif
				poly_vtxs[ v ].s = U;
				poly_vtxs[ v ].t = V;
				poly_vtxs[ v ].p = 0;
				poly_vtxs[ v ].q = W;
			}

		} else {

			// face color, iterated color is default (white)
			for ( int v = 0; v < vcount; v++, slist++ ) {

				poly_vtxs[ v ].x = slist->X;
				poly_vtxs[ v ].y = slist->Y;
				poly_vtxs[ v ].z = DEPTH_TO_FLOAT( slist->View_Z ) * OPENGL_DEPTH_RANGE;

				poly_vtxs[ v ].r = facecolor.R;
				poly_vtxs[ v ].g = facecolor.G;
				poly_vtxs[ v ].b = facecolor.B;
				poly_vtxs[ v ].a = facecolor.A;

				float X = slist->X - Screen_XOfs;
				float Y = slist->Y - Screen_YOfs;

				float U = A * X + B * Y + C * D_Value;
				float V = D * X + E * Y + F * D_Value;
				float W = G * X + H * Y + I * D_Value;

#ifdef NO_NEGATIVE_W_COORDINATES

				if ( W < 0 ) {
					U *= -1;
					V *= -1;
					W *= -1;
				}
#endif
				poly_vtxs[ v ].s = U;
				poly_vtxs[ v ].t = V;
				poly_vtxs[ v ].p = 0;
				poly_vtxs[ v ].q = W;
			}
		}

	} else {

		if ( colorsvalid ) {

			// no face color, iterated color is vertex color
			for ( int v = 0; v < vcount; v++, slist++ ) {

				poly_vtxs[ v ].x = slist->X;
				poly_vtxs[ v ].y = slist->Y;
				poly_vtxs[ v ].z = DEPTH_TO_FLOAT( slist->View_Z ) * OPENGL_DEPTH_RANGE;

				poly_vtxs[ v ].r = ((colrgba_s*)&slist->RGBA)->R;
				poly_vtxs[ v ].g = ((colrgba_s*)&slist->RGBA)->G;
				poly_vtxs[ v ].b = ((colrgba_s*)&slist->RGBA)->B;
				poly_vtxs[ v ].a = 255;

				float X = slist->X - Screen_XOfs;
				float Y = slist->Y - Screen_YOfs;

				float U = A * X + B * Y + C * D_Value;
				float V = D * X + E * Y + F * D_Value;
				float W = G * X + H * Y + I * D_Value;

#ifdef NO_NEGATIVE_W_COORDINATES

				if ( W < 0 ) {
					U *= -1;
					V *= -1;
					W *= -1;
				}
#endif
				poly_vtxs[ v ].s = U;
				poly_vtxs[ v ].t = V;
				poly_vtxs[ v ].p = 0;
				poly_vtxs[ v ].q = W;
			}

		} else {

			// no face color, iterated color is default (white)
			for ( int v = 0; v < vcount; v++, slist++ ) {

				poly_vtxs[ v ].x = slist->X;
				poly_vtxs[ v ].y = slist->Y;
				poly_vtxs[ v ].z = DEPTH_TO_FLOAT( slist->View_Z ) * OPENGL_DEPTH_RANGE;

				poly_vtxs[ v ].r = 255;
				poly_vtxs[ v ].g = 255;
				poly_vtxs[ v ].b = 255;
				poly_vtxs[ v ].a = 255;

				float X = slist->X - Screen_XOfs;
				float Y = slist->Y - Screen_YOfs;

				float U = A * X + B * Y + C * D_Value;
				float V = D * X + E * Y + F * D_Value;
				float W = G * X + H * Y + I * D_Value;

#ifdef NO_NEGATIVE_W_COORDINATES

				if ( W < 0 ) {
					U *= -1;
					V *= -1;
					W *= -1;
				}
#endif
				poly_vtxs[ v ].s = U;
				poly_vtxs[ v ].t = V;
				poly_vtxs[ v ].p = 0;
				poly_vtxs[ v ].q = W;
			}
		}
	}

	// specify vertex arrays
	if ( ( ( itertype & iter_base_mask ) == iter_texonly ) ||
		 ( face->ShadingFlags & FACE_SHADING_CONSTANTCOLOR ) ) {

		RO_ClientState( VTXARRAY_VERTICES | VTXARRAY_TEXCOORDS );
		if ( RO_ArrayMakeCurrent( VTXPTRS_RO_POLY_2, poly_vtxs ) ) {
			glVertexPointer( 3, GL_FLOAT, sizeof( GLVertex3 ), &poly_vtxs->x );
			glTexCoordPointer( 4, GL_FLOAT, sizeof( GLVertex3 ), &poly_vtxs->s );
		}

	} else {

		RO_ClientState( VTXARRAY_VERTICES | VTXARRAY_COLORS | VTXARRAY_TEXCOORDS );
		if ( RO_ArrayMakeCurrent( VTXPTRS_RO_POLY_3, poly_vtxs ) ) {
			glVertexPointer( 3, GL_FLOAT, sizeof( GLVertex3 ), &poly_vtxs->x );
			glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( GLVertex3 ), &poly_vtxs->r );
			glTexCoordPointer( 4, GL_FLOAT, sizeof( GLVertex3 ), &poly_vtxs->s );
		}
	}

	// actually draw polygon
#ifndef GL_POLYGON
	// GL_POLYGON isn't supported in GLES or modern GL.
	// FIXME: naive triangle fans can't represent all simple polygons!
	glDrawArrays( GL_TRIANGLE_FAN, 0, vcount );
#else
	glDrawArrays( GL_POLYGON, 0, vcount );
#endif

	// disable vertex arrays
//	RO_ClientState( VTXARRAY_NONE );

	// set rasterizer state to default
	RO_DefaultRasterizerState();

	if ( face->ShadingFlags & DEPTH_CONFIG_CHANGE ) {
		// restore zcmp and zwrite state
		RO_RestoreDepthState( zcmpstate, zwritestate );
	}
}


// keep track of maximum and minimum values of 1/z ----------------------------
//
INLINE
void RO_TrackMaxZVals( depth_t zval )
{
#ifdef STORE_MAX_Z_VALS
	
	int zint = DEPTH_TO_INTEGER( zval );

	if ( AUXDATA_LEAST_VERTEX_1_OVER_Z == 0 )
		AUXDATA_LEAST_VERTEX_1_OVER_Z = zint;
	else if ( zint < AUXDATA_LEAST_VERTEX_1_OVER_Z ) {
		AUXDATA_LEAST_VERTEX_1_OVER_Z = zint;
	}

	if ( AUXDATA_GREATEST_VERTEX_1_OVER_Z == 0 )
		AUXDATA_GREATEST_VERTEX_1_OVER_Z = zint;
	else if ( zint > AUXDATA_GREATEST_VERTEX_1_OVER_Z ) {
		AUXDATA_GREATEST_VERTEX_1_OVER_Z = zint;
	}

#endif
}


// polygon render functions redefinitions -------------------------------------
//
#ifndef DISABLE_TEXTURED_FACES
	#ifndef DRAW_TEXTURES_AS_FLAT
		#define PERSPTEXMAPPOLY		RO_TexMapPoly
	#else
		#define PERSPTEXMAPPOLY		RO_SolidFillPoly
	#endif
#else
	#define PERSPTEXMAPPOLY(a,b,c)	NULL
#endif

#ifndef DISABLE_FLAT_FACES
	#define SOLIDFILLPOLY			RO_SolidFillPoly
#else
	#define SOLIDFILLPOLY(a,b,c)	NULL
#endif


// static screen vertex list --------------------------------------------------
//
static SPointEx      _svertex_list[ MAX_NUM_GLVTXS + 1 ];
static SVertexExList *svertex_list = (SVertexExList *) _svertex_list;


// render single polygon specified by polygon id in given object --------------
//
int RO_RenderPolygon( GenObject *baseobj, dword polyid )
{
	ASSERT( baseobj != NULL );
	ASSERT( polyid < baseobj->NumPolys );

	Poly	*poly      = &baseobj->PolyList[ polyid ];
	SPoint	*s_vtxlist = baseobj->S_VertexList;
	Vertex3	*x_vtxlist = baseobj->X_VertexList;

	CHECKHEAPREF( poly );
	CHECKHEAPREF( s_vtxlist );
	CHECKHEAPREF( x_vtxlist );

	int numverts = poly->NumVerts;
	ASSERT( numverts > 2 );
	ASSERT( numverts <= MAX_NUM_GLVTXS );

	// check for wedge colors (wedge lighting)
	int colorsvalid    = FALSE;
	dword *wedgecolors = NULL;

	if ( AUX_ENABLE_LIGHTING_TYPES != 0x00 ) {
		if ( poly->Flags & POLYFLAG_WEDGEINDEXES ) {
			wedgecolors = (dword *) baseobj->WedgeLighted;
			colorsvalid = TRUE;
		}
		if ( poly->Flags & POLYFLAG_CORNERCOLORS ) {
			// corner colors override wedge indexes
			wedgecolors = NULL;
			colorsvalid = TRUE;
		}
	}

	// copy vertices from object structure into list for renderer
	SPointEx* svtxs		= svertex_list->Vertices;
	dword*    vertindxs	= poly->VertIndxs;
	for ( int vct = numverts; vct > 0; vct--, svtxs++, vertindxs++ ) {

		svtxs->X = s_vtxlist[ vertindxs[ 0 ] ].X + Screen_XOfs;
		svtxs->Y = s_vtxlist[ vertindxs[ 0 ] ].Y + Screen_YOfs;

		svtxs->View_Z = DEPTHBUFF_OOZ( x_vtxlist[ vertindxs[ 0 ] ].Z );
		RO_TrackMaxZVals( svtxs->View_Z );

		// copy lighted wedge color
		if ( colorsvalid ) {
			svtxs->RGBA = ( wedgecolors != NULL ) ?
				wedgecolors[ vertindxs[ numverts ] ] : vertindxs[ numverts ];
		}
	}

	// store number of screen vertices
	svertex_list->VCount = numverts;

	// get pointer to corresponding face (to locate material/shading info)
	Face *face = &baseobj->FaceList[ poly->FaceIndx ];
	ASSERT( face != NULL );

	// call appopriate polygon drawing function
	if ( face->ShadingFlags & FACE_SHADING_ENABLETEXTURE ) {

		// textured polygon
		PERSPTEXMAPPOLY( svertex_list, baseobj, face, colorsvalid );

	} else {

		// non-textured polygon
		SOLIDFILLPOLY( svertex_list, baseobj, face, colorsvalid );
	}

	return TRUE;
}



