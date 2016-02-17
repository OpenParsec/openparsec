/*
 * PARSEC - Object Rendering Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:40 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1995-2000
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
#include <math.h>
#include <stddef.h>
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
#include "aud_defs.h"

// drawing subsystem
#include "d_iter.h"

// rendering subsystem
#include "r_obj.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "ro_obj.h"

// proprietary module headers
#include "con_aux.h"
#include "e_callbk.h"
#include "e_color.h"
#include "e_vtxani.h"
#include "obj_iter.h"
#include "obj_xtra.h"
#include "g_sfx.h"
#include "ro_api.h"
#include "ro_poly.h"
#include "ro_supp.h"
#include "g_explod.h"


// flags
#define UPDATE_SOUND
//#define DISABLE_ACTUAL_OBJECT_RENDERING
#define COUNT_RENDERED_POLYGONS
#define TWO_SIDES_POSSIBLE
#define CULL_BACKFACES_IN_BSP
//#define CHECK_BACKFACE_EPS
//#define ENABLE_BSP_ERROR_TRACING
#define RELAXED_DEBUG_MODE
#define CLIP_OBJECT_POLYGONS
//#define CHECK_FACE_TOUCHING



// include macros -------------------------------------------------------------
//
#include "ro_objmc.h"


// to determine whether a face has been touched in current frame --------------
//
#ifdef CHECK_FACE_TOUCHING
	#define FACE_TOUCHED(f)		( (f)->VisibleFrame == CurVisibleFrame )
	#define TOUCHFACE(f)		{ (f)->VisibleFrame = CurVisibleFrame; }
#else
	#define FACE_TOUCHED(f)		( FALSE )
	#define TOUCHFACE(f)		{}
#endif

//NOTE:
//CAVEAT:
// if CHECK_FACE_TOUCHING is specified the facelist must be part
// of the class instance itself, not only of the class. otherwise,
// a face will only be touched once per frame, not once per frame
// per object, as it should be.


// flag whether direct mode active (not rendering via R_DrawWorld()) ----------
//
static int direct_rendering = FALSE;


// module global variables for bsp processing ---------------------------------
//
static GenObject*	bsp_objectp;
static dword*		bsp_vpolylist;
static Vertex3*		bsp_vtxlist;
static Vertex3		bsp_cameravec;
static dword		bsp_vpolycount;


// visit single bsp node: insert polygon into visible list and tag vertices ---
//
INLINE
void RO_DrawBSPNode( dword bspnode, Poly *poly, dword *vindxs, Face *face )
{
	ASSERT( poly != NULL );
	ASSERT( vindxs != NULL );
	ASSERT( face != NULL );

	static int vct;

	// only for non-degenerated polygons
	if ( poly->NumVerts > 2 ) {

		// insert this polygon into list of visible polygons
		*bsp_vpolylist++ = bsp_objectp->BSPTree[ bspnode ].Polygon;
		bsp_vpolycount++;

		// visit all vertices of this polygon and set visible=true in turn
		for ( vct = poly->NumVerts; vct > 0; vct-- ) {
			bsp_vtxlist[ *vindxs++ ].VisibleFrame = CurVisibleFrame;
		}

		// calculate gradients for texture mapped polygons
		if ( !FACE_TOUCHED( face ) ) {
			TOUCHFACE( face );
			CALCTEXTUREGRADIENTS( bsp_objectp, face );
		}
	}

	// process polygons contained in same plane as splitter (current node)
	while ( bsp_objectp->BSPTree[ bspnode ].Contained != 0 ) {

		bspnode	= bsp_objectp->BSPTree[ bspnode ].Contained;
		poly	= &bsp_objectp->PolyList[ bsp_objectp->BSPTree[ bspnode ].Polygon ];
		face	= &bsp_objectp->FaceList[ poly->FaceIndx ];
		vindxs	= poly->VertIndxs;

		if ( poly->NumVerts > 2 ) {

			*bsp_vpolylist++ = bsp_objectp->BSPTree[ bspnode ].Polygon;
			bsp_vpolycount++;

			for ( vct = poly->NumVerts; vct > 0; vct-- ) {
				bsp_vtxlist[ *vindxs++ ].VisibleFrame = CurVisibleFrame;
			}

			if ( !FACE_TOUCHED( face ) ) {
				TOUCHFACE( face );
				CALCTEXTUREGRADIENTS( bsp_objectp, face );
			}
		}
	}
}


// process entire bsp (sub-)tree recursively ----------------------------------
//
PRIVATE
void RO_TraverseBSPTree( dword bspnode )
{
	Poly*			poly;
	Face*			face;
	dword*			vindxs;
	static Vertex3*	basevtx;
	static Vertex3	viewvec;
	static Vertex3*	normal;
	static geomv_t	sprod;

	CHECKOBJECTREF( bsp_objectp );
	ASSERT( bsp_vpolycount <= bsp_objectp->NumPolys );
	ASSERT( bspnode <= bsp_objectp->NumPolys );

	if ( bspnode == 0 )
		return;

	// fetch polygon contained in this node's plane
	poly = &bsp_objectp->PolyList[ bsp_objectp->BSPTree[ bspnode ].Polygon ];

	// fetch pointer to first vertex in polygon
	vindxs  = poly->VertIndxs;
	basevtx = &bsp_vtxlist[ *vindxs ];

	// calculate vector directed from polygon to viewpoint in objspace
	viewvec.X = bsp_cameravec.X - basevtx->X;
	viewvec.Y = bsp_cameravec.Y - basevtx->Y;
	viewvec.Z = bsp_cameravec.Z - basevtx->Z;

	// fetch pointer to face normal
	face   = &bsp_objectp->FaceList[ poly->FaceIndx ];
	normal = &bsp_vtxlist[ face->FaceNormalIndx ];

	// use dot-product to determine visibility
	sprod = DOT_PRODUCT( &viewvec, normal );

testplane:

	if ( CHECKBSPPLANE( sprod, bspnode, poly, vindxs, face ) ) {

		if ( bsp_objectp->BSPTree[ bspnode ].BackTree > 0 )
			RO_TraverseBSPTree( bsp_objectp->BSPTree[ bspnode ].BackTree );

		RO_DrawBSPNode( bspnode, poly, vindxs, face );

		if ( bsp_objectp->BSPTree[ bspnode ].FrontTree > 0 )
			RO_TraverseBSPTree( bsp_objectp->BSPTree[ bspnode ].FrontTree );

	} else {

		if ( bsp_objectp->BSPTree[ bspnode ].FrontTree > 0 )
			RO_TraverseBSPTree( bsp_objectp->BSPTree[ bspnode ].FrontTree );

		if ( face->ShadingFlags & FACE_SHADING_NOBACKCULLING ) {
			RO_DrawBSPNode( bspnode, poly, vindxs, face );
		} else {
			BACKFACINGBSPNODE( bspnode, poly, vindxs, face );
		}

		if ( bsp_objectp->BSPTree[ bspnode ].BackTree > 0 )
			RO_TraverseBSPTree( bsp_objectp->BSPTree[ bspnode ].BackTree );
	}
}


// process list of polys and set all frontfacing visible ----------------------
//
PRIVATE
void RO_FrontFaceVisibility()
{
	CHECKOBJECTREF( bsp_objectp );

	// current pass over list
	int listpass = 0;

	// for pass-skip speedup
	int numpolysleft = bsp_objectp->NumPolys;

	do {

		// process entire list linearly
		for ( unsigned int scanpid = 0; scanpid < bsp_objectp->NumPolys; scanpid++ ) {

			// map to actual polygon id
			int polyid = bsp_objectp->SortedPolyList ?
				bsp_objectp->SortedPolyList[ scanpid ] : scanpid;

			ASSERT( ( polyid >= 0 ) && ( (dword)polyid < bsp_objectp->NumPolys ) );
			Poly *poly = &bsp_objectp->PolyList[ polyid ];

			// reject degenerated polygons
			if ( poly->NumVerts < 3 ) {

				//NOTE:
				// needed by clipper for trivial
				// rejection of polygons.

				// for pass-skip speedup
				if ( listpass == 0 ) {
					numpolysleft--;
				}
				continue;
			}

			// fetch face this poly belongs to
			dword faceindx = poly->FaceIndx;
			ASSERT( faceindx < bsp_objectp->NumFaces );
			Face *face = &bsp_objectp->FaceList[ faceindx ];

			// determine whether backface culling should be used for this face
			int backculling = ( ( face->ShadingFlags & FACE_SHADING_NOBACKCULLING ) == 0 );

			// early exit if poly not processed in current pass
			if ( listpass < 1 ) {
				if ( ( face->ShadingFlags & FACE_SHADING_DRAW_LAST ) != 0 ) {
					continue;
				}
			} else {
				if ( ( face->ShadingFlags & FACE_SHADING_DRAW_LAST ) == 0 ) {
					continue;
				}
				if ( ( listpass == 1 ) && backculling ) {
					continue;
				}
			}

			// determine backfacing state
			int backfacing = FALSE;

			// fetch first vertex in polygon
			Vertex3	*basevtx = &bsp_vtxlist[ *poly->VertIndxs ];

			// calculate vector directed from polygon to viewpoint in objspace
			Vertex3 viewvec;
			viewvec.X = bsp_cameravec.X - basevtx->X;
			viewvec.Y = bsp_cameravec.Y - basevtx->Y;
			viewvec.Z = bsp_cameravec.Z - basevtx->Z;

			// fetch face normal
			Vertex3	*normal	= &bsp_vtxlist[ face->FaceNormalIndx ];

			// use dot-product to determine visibility
			geomv_t sprod = DOT_PRODUCT( &viewvec, normal );

			// cull face if backfacing
			if ( GEOMV_NEGATIVE( sprod ) ) {
				backfacing = TRUE;
			}

			// determine whether backfacing poly should be processed in this pass
			if ( ( listpass > 0 ) && !backculling ) {
				int mode = ( face->ShadingFlags & FACE_SHADING_BACK_FIRST ) && backfacing;
				if ( mode != ( listpass == 1 ) ) {
					continue;
				}
			}

			// for pass-skip speedup
			numpolysleft--;

			// cull poly if face backfacing and backface culling not disabled
			if ( backculling && backfacing ) {
				continue;
			}

			// insert polygon into list of visible polygons
			*bsp_vpolylist++ = polyid;
			bsp_vpolycount++;

			// visit all vertices of this polygon and set visible=true in turn
			dword *vindxs = poly->VertIndxs;
			for ( int vct = poly->NumVerts; vct > 0; vct-- ) {
				bsp_vtxlist[ *vindxs++ ].VisibleFrame = CurVisibleFrame;
			}

			// calculate gradients for texture mapped polygons
			if ( !FACE_TOUCHED( face ) ) {
				TOUCHFACE( face );
				CALCTEXTUREGRADIENTS( bsp_objectp, face );
			}
		}

		//TODO:
		// could use static hints to automatically skip passes that
		// are always null passes for certain objects.
	} while (numpolysleft > 0 && (++listpass) < 3);

	ASSERT( numpolysleft == 0 );
}


// draw single line (normal/polygon edge) -------------------------------------
//
PRIVATE
void RO_DrawLine( SPoint *base, SPoint *head )
{
	ASSERT( base != NULL );
	ASSERT( head != NULL );

	IterLine2 itline;
	memset( &itline, 0, sizeof( itline ) );

	itline.flags	 = ITERFLAG_LS_DEFAULT;
	itline.NumVerts	 = 2;
	itline.itertype	 = iter_rgba | iter_overwrite;
	itline.raststate = rast_chromakeyoff;
	itline.rastmask	 = rast_mask_zbuffer | rast_mask_texclamp |
					   rast_mask_mipmap  | rast_mask_texfilter;
	itline.texmap	 = NULL;

	itline.Vtxs[ 0 ].X = INT_TO_RASTV( base->X );
	itline.Vtxs[ 0 ].Y = INT_TO_RASTV( base->Y );
	itline.Vtxs[ 0 ].Z = RASTV_1;

	itline.Vtxs[ 0 ].R = 255;
	itline.Vtxs[ 0 ].G = 255;
	itline.Vtxs[ 0 ].B = 255;
	itline.Vtxs[ 0 ].A = 255;

	itline.Vtxs[ 1 ].X = INT_TO_RASTV( head->X );
	itline.Vtxs[ 1 ].Y = INT_TO_RASTV( head->Y );
	itline.Vtxs[ 1 ].Z = RASTV_1;

	itline.Vtxs[ 1 ].R = itline.Vtxs[ 0 ].R;
	itline.Vtxs[ 1 ].G = itline.Vtxs[ 0 ].G;
	itline.Vtxs[ 1 ].B = itline.Vtxs[ 0 ].B;
	itline.Vtxs[ 1 ].A = itline.Vtxs[ 0 ].A;

	Rectangle2 rect;
	rect.left   = INT_TO_RASTV( 0 );
	rect.right  = INT_TO_RASTV( Screen_Width );
	rect.top    = INT_TO_RASTV( 0 );
	rect.bottom = INT_TO_RASTV( Screen_Height );

	IterLine2 *clipline =
		CLIP_RectangleIterLine2( &itline, &rect );
	if ( clipline != NULL ) {
		D_DrawIterLine2( clipline );
	}
}


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


// animate faces of specified object ------------------------------------------
//
PRIVATE
void RO_PerformFaceAnims( GenObject *obj )
{
	ASSERT( obj != NULL );

	if ( obj->FaceAnimStates == NULL )
		return;

	// scan all active anim states
	for ( int stindx = 0; stindx < obj->ActiveFaceAnims; stindx++ ) {

		FaceAnimState *animstate = &obj->FaceAnimStates[ stindx ];

		// texture animation
		texanim_s *texanim = animstate->TexAnim;
		if ( texanim != NULL ) {

			// check texture-map frame
			if ( ( animstate->tex_time -= CurScreenRefFrames ) < 0 ) {

				if ( animstate->tex_pos == texanim->tex_end ) {
					// restart at repeat position
					animstate->tex_pos = texanim->tex_rep;
				} else {
					// advance to next table entry
					animstate->tex_pos++;
				}

				// set deltatime for next frame
				ASSERT( texanim->tex_table != NULL );
				texfrm_s *curtexframe = &texanim->tex_table[ animstate->tex_pos ];
				animstate->tex_time = curtexframe->deltatime;
			}

			// trafo is optional
			if ( texanim->xfo_table != NULL ) {

				// check texture-trafo frame
				if ( ( animstate->xfo_time -= CurScreenRefFrames ) < 0 ) {

					if ( animstate->xfo_pos == texanim->xfo_end ) {
						// restart at repeat position
						animstate->xfo_pos = texanim->xfo_rep;
					} else {
						// advance to next table entry
						animstate->xfo_pos++;
					}

					// set deltatime for next frame
					xfofrm_s *curxfoframe = &texanim->xfo_table[ animstate->xfo_pos ];
					animstate->xfo_time = curxfoframe->deltatime;
				}
			}
		}

		// color animation
		colanim_s *colanim = animstate->ColAnim;
		if ( colanim != NULL ) {

			// check color0 frame
			animstate->col_time0 -= CurScreenRefFrames;
			while ( animstate->col_time0 < 0 ) {

				if ( animstate->col_pos0 == colanim->col_end0 ) {
					// restart at beginning
					animstate->col_pos0 = 0;
				} else {
					// advance to next table entry
					animstate->col_pos0++;
				}

				// set deltatime for next frame
				ASSERT( colanim->col_table0 != NULL );
				colfrm_s *col0 = &colanim->col_table0[ animstate->col_pos0 ];
				animstate->col_time0 += col0->deltatime;
			}

			// fetch color0 frame
			ASSERT( colanim->col_table0 != NULL );
			colfrm_s *col0 = &colanim->col_table0[ animstate->col_pos0 ];

			// set (perhaps intermediate) output color
			animstate->ColOutput = col0->color;

			// color1 is optional
			if ( colanim->col_table1 != NULL ) {

				// check color1 frame
				animstate->col_time1 -= CurScreenRefFrames;
				while ( animstate->col_time1 < 0 ) {

					if ( animstate->col_pos1 == colanim->col_end1 ) {
						// restart at beginning
						animstate->col_pos1 = 0;
					} else {
						// advance to next table entry
						animstate->col_pos1++;
					}

					// set deltatime for next frame
					ASSERT( colanim->col_table1 != NULL );
					colfrm_s *col1 = &colanim->col_table1[ animstate->col_pos1 ];
					animstate->col_time1 += col1->deltatime;
				}

				// fetch color1 frame
				ASSERT( colanim->col_table1 != NULL );
				colfrm_s *col1 = &colanim->col_table1[ animstate->col_pos1 ];

				dword sourcemode = colanim->col_flags & COLANIM_SOURCE_MASK;
				if ( sourcemode != COLANIM_SOURCENOCOMBINE ) {

					// combine color1 with color0
					int colr, colg, colb, cola;
					if ( sourcemode == COLANIM_SOURCEADD ) {
						COLOR_ADD( colr, col1->color.R, animstate->ColOutput.R );
						COLOR_ADD( colg, col1->color.G, animstate->ColOutput.G );
						COLOR_ADD( colb, col1->color.B, animstate->ColOutput.B );
						COLOR_ADD( cola, col1->color.A, animstate->ColOutput.A );
					} else {
						ASSERT( sourcemode == COLANIM_SOURCEMUL );
						COLOR_MUL( colr, col1->color.R, animstate->ColOutput.R );
						COLOR_MUL( colg, col1->color.G, animstate->ColOutput.G );
						COLOR_MUL( colb, col1->color.B, animstate->ColOutput.B );
						COLOR_MUL( cola, col1->color.A, animstate->ColOutput.A );
					}

					// set final output color
					animstate->ColOutput.R = colr;
					animstate->ColOutput.G = colg;
					animstate->ColOutput.B = colb;
					animstate->ColOutput.A = cola;
				}
			}
		}
	}
}


// render dynamically attached object polygons --------------------------------
//
PRIVATE
void RO_RenderDynamicPolys( GenObject *obj, int inorder )
{
	ASSERT( obj != NULL );

	// only if enabled
	if ( !AUX_ATTACH_THRUST_OBJECTS )
		return;

	// only for ships
	if ( !OBJECT_TYPE_SHIP( obj ) )
		return;

	// only in actual game mode
	if ( !InGameLoop || direct_rendering )
		return;

	// save zcmp and zwrite state
	int zcmpstate   = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();

	OBJ_AttachIterPolygons( obj, inorder );

	// set rasterizer state to default
	RO_DefaultRasterizerState();

	// restore zcmp and zwrite state
	RO_RestoreDepthState( zcmpstate, zwritestate );
}


// draw polygons that are in the list of currently visible polygons -----------
//
void R_RenderObject( GenObject *objectp )
{
	ASSERT( objectp != NULL );

	// check validity of pointer to object
	CHECKOBJECTREF( objectp );

	// temporary vispoly list is part of object class
	int   vpolycount = objectp->NumVisPolys;
	dword *vpolylist = objectp->VisPolyList;
	CHECKHEAPREF( vpolylist );

	// scan list from head to tail and render each polygon in turn
	while ( vpolycount-- ) {
//		AUDs_MaintainSound();
		COUNTPOLYGONS( RO_RenderPolygon( objectp, *vpolylist++ ) );
	}

	// attach dynamic polys
	RO_RenderDynamicPolys( objectp, TRUE );

//	AUDs_MaintainSound();
}


// determine visible polygons as demanded by bsp tree -------------------------
//
PRIVATE
void RO_DetermineObjVisibility( GenObject *objectp )
{
	ASSERT( objectp != NULL );

	// check validity of pointer to object
	CHECKOBJECTREF( objectp );

	// create pointers to visible polygon and vertex list
	bsp_vpolylist = objectp->VisPolyList;
	bsp_vtxlist	  = objectp->VertexList;

	CHECKHEAPREF( bsp_vpolylist );
	CHECKHEAPREF( bsp_vtxlist );

	// calculate vector directed to the viewpoint in object space
	CalcObjSpaceCamera( objectp, &bsp_cameravec );

	// init count of visible polygons
	bsp_vpolycount = 0;

	// the bsp tree is now optional
	if ( objectp->BSPTree != NULL ) {

		// scan all polygons and determine visibility:
		// fill bsp_vpolylist with numbers of visible polygons
		// in back to front drawing order.
		// increase bsp_vpolycount for each polygon.
		// tag all vertices belonging to any visible polygons.
		bsp_objectp = objectp;
		RO_TraverseBSPTree( 1 );

	} else {

		// determine visibility by backface culling only
		bsp_objectp = objectp;
		RO_FrontFaceVisibility();
	}

	// store number of currently visible polygons
	// into list header (yields length of list)
	objectp->NumVisPolys = bsp_vpolycount;
}


// storage for scheduled wireframe polygons/normals ---------------------------
//
#define MAX_SCHEDULED_POLYGONS			512
#define MAX_SCHEDULED_POLYGON_EDGES		( MAX_SCHEDULED_POLYGONS * 8 )
#define MAX_SCHEDULED_NORMALS			512

static SPoint scheduled_polygons_edges[ MAX_SCHEDULED_POLYGON_EDGES ][ 2 ];
static int    scheduled_polygons_next		= 0;
static int    scheduled_polygons_next_edge	= 0;

static SPoint scheduled_normals[ MAX_SCHEDULED_NORMALS ][ 2 ];
static int    scheduled_normals_next = 0;


// schedule wireframe polygons (pre-clipping) ---------------------------------
//
PRIVATE
void RO_ScheduleWireframePolygons( GenObject *obj, int visonly )
{
	ASSERT( obj != NULL );

	if ( direct_rendering )
		return;

	Vertex3 *vtxs = obj->VertexList;

	// scan all object polygons
	for ( unsigned int pid = 0; pid < obj->NumPolys; pid++ ) {

		Poly *poly = &obj->PolyList[ pid ];
		int previd = poly->NumVerts - 1;

		// reject invisible polygons
		if ( poly->NumVerts < 3 )
			continue;
		if ( visonly && ( vtxs[ poly->VertIndxs[ 0 ] ].VisibleFrame != CurVisibleFrame ) )
			continue;

		// schedule this polygon's edges
		for ( unsigned int vid = 0; vid < poly->NumVerts; vid++ ) {

			if ( scheduled_polygons_next_edge >= MAX_SCHEDULED_POLYGON_EDGES )
				break;

			Vertex3 tempvert;

			MtxVctMUL( obj->CurrentXmatrx,
				&vtxs[ poly->VertIndxs[ previd ] ], &tempvert );
			PROJECT_TO_SCREEN( tempvert,
				scheduled_polygons_edges[ scheduled_polygons_next_edge ][ 0 ] );

			MtxVctMUL( obj->CurrentXmatrx,
				&vtxs[ poly->VertIndxs[ vid ] ], &tempvert );
			PROJECT_TO_SCREEN( tempvert,
				scheduled_polygons_edges[ scheduled_polygons_next_edge ][ 1 ] );

			scheduled_polygons_next_edge++;
			previd = vid;
		}
	}
}


// schedule wireframe polygons/normals for overlayed drawing later on ---------
//
PRIVATE
void RO_ScheduleWireframeOverlays( GenObject *obj )
{
	ASSERT( obj != NULL );

	if ( direct_rendering )
		return;

	// polygons as wireframe (post-clipping)
	if ( AUX_DRAW_WIREFRAME == 1 ) {
		RO_ScheduleWireframePolygons( obj, TRUE );
	}

	// vertex/wedge normals
	if ( AUX_DRAW_NORMALS && ( obj->WedgeNormals != NULL ) ) {

		dword *wedgeverts = obj->WedgeVertIndxs;
		ASSERT( wedgeverts != NULL );

		// scan all wedges
		for ( unsigned int cwedge = 0; cwedge < obj->NumWedges; cwedge++ ) {

			Vertex3 *vtx = &obj->VertexList[ wedgeverts[ cwedge ] ];
			ASSERT( vtx != NULL );

			// wedge only visible if vertex visible
			if ( vtx->VisibleFrame == CurVisibleFrame ) {

				SPoint base;
				base.X = obj->S_VertexList[ wedgeverts[ cwedge ] ].X + Screen_XOfs;
				base.Y = obj->S_VertexList[ wedgeverts[ cwedge ] ].Y + Screen_YOfs;

				Vertex3 vhead;
				vhead.X = vtx->X + obj->WedgeNormals[ cwedge ].X * 2;
				vhead.Y = vtx->Y + obj->WedgeNormals[ cwedge ].Y * 2;
				vhead.Z = vtx->Z + obj->WedgeNormals[ cwedge ].Z * 2;

				// transform tip of normal into view-space
				Vertex3 tempvert;
				MtxVctMUL( obj->CurrentXmatrx, &vhead, &tempvert );

				SPoint head;
				PROJECT_TO_SCREEN( tempvert, head );

				if ( scheduled_normals_next < MAX_SCHEDULED_NORMALS ) {

					scheduled_normals[ scheduled_normals_next ][ 0 ].X = base.X;
					scheduled_normals[ scheduled_normals_next ][ 0 ].Y = base.Y;
					scheduled_normals[ scheduled_normals_next ][ 1 ].X = head.X;
					scheduled_normals[ scheduled_normals_next ][ 1 ].Y = head.Y;

					scheduled_normals_next++;
				}
			}
		}
	}
}


// draw scheduled wireframe polygons/normals ----------------------------------
//
PRIVATE
void RO_DrawWireframeOverlays()
{
	// draw scheduled polygons
	for ( int eid = 0; eid < scheduled_polygons_next_edge; eid++ ) {
		RO_DrawLine( &scheduled_polygons_edges[ eid ][ 0 ],
					 &scheduled_polygons_edges[ eid ][ 1 ] );
	}
	scheduled_polygons_next      = 0;
	scheduled_polygons_next_edge = 0;

	// draw scheduled normals
	for ( int nid = 0; nid < scheduled_normals_next; nid++ ) {
		RO_DrawLine( &scheduled_normals[ nid ][ 0 ],
					 &scheduled_normals[ nid ][ 1 ] );
	}
	scheduled_normals_next = 0;
}


// generate colors in GenObject::WedgeLighted[] for this frame ----------------
//
PRIVATE
void RO_LightWedges( GenObject *obj, const Xmatrx objtoview )
{
	ASSERT( obj != NULL );
	ASSERT( objtoview != NULL );

	if ( obj->WedgeVertIndxs == NULL )
		return;
	if ( obj->WedgeLighted == NULL )
		return;
	if ( obj->WedgeNormals == NULL )
		return;

	// ambient light color
	colrgba_s *ambient = &LightColorAmbient;

	// diffuse light color
	colrgba_s *diffuse = &LightColorDiffuse;
	
	// specular light color
	colrgba_s *specular = &LightColorSpecular;

	// directional lightsource in world space
	Vector3 worldlightvec = (Vector3) FixedStars[SUN_STAR_NO].location;
	NormVctX(&worldlightvec);
	
//	worldlightvec.X = FLOAT_TO_GEOMV( 0.4472f );
//	worldlightvec.Y = FLOAT_TO_GEOMV( -0.4f );
//	worldlightvec.Z = FLOAT_TO_GEOMV( -0.8f );
	
	//GlobalDirLight

	// world space -> object space
	Xmatrx inv;
	inv[ 0 ][ 0 ] = objtoview[ 0 ][ 0 ];
	inv[ 0 ][ 1 ] = objtoview[ 1 ][ 0 ];
	inv[ 0 ][ 2 ] = objtoview[ 2 ][ 0 ];
	inv[ 1 ][ 0 ] = objtoview[ 0 ][ 1 ];
	inv[ 1 ][ 1 ] = objtoview[ 1 ][ 1 ];
	inv[ 1 ][ 2 ] = objtoview[ 2 ][ 1 ];
	inv[ 2 ][ 0 ] = objtoview[ 0 ][ 2 ];
	inv[ 2 ][ 1 ] = objtoview[ 1 ][ 2 ];
	inv[ 2 ][ 2 ] = objtoview[ 2 ][ 2 ];

	// directional lightsource in object space
	Vector3 objlightvec;
	MtxVctMULt( inv, &worldlightvec, &objlightvec );
	
//	Vector3 objlightvecneg = {-objlightvec.X, -objlightvec.Y, -objlightvec.Z, 0};
	
//	Vector3 reflectvec;
	
//	Vector3 viewvec = {ViewCamera[0][3], ViewCamera[1][3], ViewCamera[2][3], 0};
	
//	Vector3 objviewvec;
//	MtxVctMULt(inv, &viewvec, &objviewvec);
	
//	NormVct(&objviewvec);
//	objviewvec.X = -objviewvec.X;
//	objviewvec.Y = -objviewvec.Y;
//	objviewvec.Z = -objviewvec.Z;

	for ( dword wedge = 0; wedge < obj->NumWedges; wedge++ ) {

		Vector3 *normal  = &obj->WedgeNormals[ wedge ];
//		Vertex3 *basevtx = &obj->VertexList[ obj->WedgeVertIndxs[ wedge ] ];

		geomv_t ldot = DOT_PRODUCT( normal, &objlightvec );
		if ( ldot < GEOMV_0 )
			ldot = GEOMV_0;
		float fdot = GEOMV_TO_FLOAT( ldot );
		
//		VctReflect(&objlightvecneg, normal, &reflectvec);
		
//		float spec = max(DotProduct(&reflectvec, &objviewvec), 0.0f);
//		if (spec > 0.0f)
//			spec = min(powf(spec, 20.0f), 1.0f);
		float spec = 0.0f;
		
		for (int i = 0; i < 3; i++) {
			int lightcolor = (int) (ambient->index[i] + ((float) (diffuse->index[i] * fdot)) + spec * specular->index[i]);
			lightcolor = min(lightcolor, 255);
			
			obj->WedgeLighted[wedge].index[i] = lightcolor;
		}
		
		obj->WedgeLighted[wedge].A = 255;
	}
}


// calculate all visible polygons of a subobject and draw them afterwards -----
//
PRIVATE
void RO_ReCalcAndRenderSubObject( GenObject *objectp, const Xmatrx objtoview )
{
	ASSERT( objectp != NULL );
	ASSERT( objtoview != NULL );

	// update sound buffer if necessary
//	AUDs_MaintainSound();

	// schedule wireframe polygons (pre-clipping)
	if ( AUX_DRAW_WIREFRAME == 2 ) {
		RO_ScheduleWireframePolygons( objectp, FALSE );
	}

	// light wedges if enabled
	if ( AUX_ENABLE_LIGHTING_TYPES != 0x00 ) {
		RO_LightWedges( objectp, objtoview );
	}

#ifdef CLIP_OBJECT_POLYGONS

	// clip if not trivial accept
	if ( objectp->CullMask != 0x00 ) {

		// transform frustum into object-space and clip object there
		BackTransformVolume( objectp->CurrentXmatrx, View_Volume, Object_ViewVolume, objectp->CullMask );
		objectp = CLIP_VolumeGenObject( objectp, Object_ViewVolume, objectp->CullMask );

		// early-out if trivial reject
		if ( objectp == NULL ) {
			return;
		}
	}

#endif

	// create ordered list of visible polygons (VisPolyList)
	RO_DetermineObjVisibility( objectp );

	// calculate vertex coordinates that belong to visible polygons
	ProcessObject( objectp );

	// schedule wireframe overlays
	RO_ScheduleWireframeOverlays( objectp );

	// use calculated coordinates to draw object
	R_RenderObject( objectp );
}


// calculate all visible polygons of an object and draw them afterwards -------
//
void R_ReCalcAndRenderObject( GenObject *objectp, const Camera camera )
{
	ASSERT( objectp != NULL );
	ASSERT( camera != NULL );

#ifndef DISABLE_ACTUAL_OBJECT_RENDERING

	// check validity of pointer to object
	CHECKOBJECTREF( objectp );

	// animate faces (part of instance)
	RO_PerformFaceAnims( objectp );

	// render base object if not empty
	if ( objectp->NumVerts > 0 ) {

		// calculate objectspace to viewspace transformation
		MtxMtxMUL( camera, objectp->ObjPosition, objectp->CurrentXmatrx );

		// render base object
		RO_ReCalcAndRenderSubObject( objectp, objectp->ObjPosition );
	}

	// render subobjects if present
	if ( objectp->ActiveVtxAnims > 0 ) {

		// remember vertex anim state array
		VtxAnimState *vtxanimstatebase = objectp->VtxAnimStates;

		// temporarily offset vertex anim state array according to lod
		int vtxanimoffset = ( objectp->NumLodObjects > 0 ) ?
			( objectp->CurrentLod * ( objectp->NumVtxAnims + 1 ) ) : 0;
		objectp->VtxAnimStates = &objectp->VtxAnimStates[ vtxanimoffset ];

		// render subobjects
		for ( int animid = 0; animid < objectp->ActiveVtxAnims; animid++ ) {

			VtxAnimState *anim = &objectp->VtxAnimStates[ animid ];

			// make subobject current
			VtxAnimMakeCurrent( objectp, animid );

			// invoke animation callback
			ASSERT( anim->AnimCallback != NULL );
			(*anim->AnimCallback)( objectp, animid );

			// calculate objectspace to viewspace transformation
			MtxMtxMUL( objectp->ObjPosition, anim->CurrentXmatrx, DestXmatrx );
			MtxMtxMUL( camera, DestXmatrx, objectp->CurrentXmatrx );

			// render subobject
			RO_ReCalcAndRenderSubObject( objectp, DestXmatrx );
		}

		// make sure base object is current
		VtxAnimMakeCurrent( objectp, objectp->NumVtxAnims );

		// restore vertex anim state array
		objectp->VtxAnimStates = vtxanimstatebase;
	}

	// post-render attach dynamic polys
	RO_RenderDynamicPolys( objectp, FALSE );

#endif // !DISABLE_ACTUAL_OBJECT_RENDERING

}


// draw objects contained in already sorted visible objects list --------------
//
void R_DrawWorld( const Camera camera )
{
	ASSERT( camera != NULL );

	// turn on z-buffer
	RO_EnableDepthBuffer( true, true );

	// walk pre-callbacks
	CALLBACK_WalkCallbacks( CBTYPE_DRAW_PRE_WORLD );

	// walk visible objects list and draw objects as encountered
	GenObject *scanpo = VObjList->NextVisObj;
	for ( ; scanpo; scanpo = scanpo->NextVisObj ) {

		// check validity of pointer to object
		CHECKOBJECTREF( scanpo );

		// render object
		if ( OBJECT_TYPE_SHIP( scanpo ) ) {

			ShipObject *shippo = (ShipObject *) scanpo;
			int ecount = shippo->ExplosionCount;

			if ( ecount > 0 ) {

				// render ship only if explosion not too far yet
				if ( ecount >= BM_EXPLVANISHFRAME * EXPL_REF_SPEED ) {
					R_ReCalcAndRenderObject( scanpo, camera );
				} else {
					shippo->VisibleFrame = VISFRAME_NEVER;
				}

				// draw explosion bitmap frame
				DrawExpAnim( shippo );

				// create explosion particles if correct frame
				if ( ( ecount / EXPL_REF_SPEED == BM_EXPLPARTCLFRAME ) ) {

					if ( shippo->DelayExplosion == 0 ) {

						// start particle explosion
						if ( !AUX_USE_SIMPLE_EXPLOSION )
							SFX_ParticleExplosion( shippo );

						// let the ship lose its extras at the same time
						OBJ_CreateShipExtras( shippo );

						// only once
						shippo->DelayExplosion = -1;
					}
				}

			} else {
				// render ship if not exploding right now
				R_ReCalcAndRenderObject( scanpo, camera );
			}

		} else {
			// if object is not a ship render it always
			R_ReCalcAndRenderObject( scanpo, camera );
		}
	}

	// walk post-callbacks
	CALLBACK_WalkCallbacks( CBTYPE_DRAW_POST_WORLD );

	// turn off z-buffer
	RO_DisableDepthBuffer( true, true );

	// draw scheduled wireframe overlays
	RO_DrawWireframeOverlays();
}


// init/deinit direct object rendering ----------------------------------------
//
void R_DirectObjectRendering( int flags )
{
	direct_rendering = flags;

	if ( flags ) {

		// turn on z-buffer
		RO_EnableDepthBuffer( true, true );

	} else {

		// turn off z-buffer
		RO_DisableDepthBuffer( true, true );
	}
}



