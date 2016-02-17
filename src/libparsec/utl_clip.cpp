/*
 * PARSEC - Iter-Primitive Clipping Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:46 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-1999
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
#include <math.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "utl_clip.h"



// clip IterTriangle3 against volume (set of planes); (XYZ UVW RGBA) ----------
//
IterPolygon3 *CLIP_VolumeIterTriangle3( IterTriangle3 *poly, Plane3 *volume, dword cullmask )
{
	//NOTE:
	// the original triangle's NumVerts field will be set to 3
	// here, in order to enable processing as general polygon.

	IterPolygon3 *iterpoly	= (IterPolygon3 *) poly;
	iterpoly->NumVerts		= 3;

	return CLIP_VolumeIterPolygon3( iterpoly, volume, cullmask );
}


// clip IterTriangle3 against volume (set of planes); (XYZ UVW) ---------------
//
IterPolygon3 *CLIP_VolumeIterTriangle3_UVW( IterTriangle3 *poly, Plane3 *volume, dword cullmask )
{
	//NOTE:
	// the original triangle's NumVerts field will be set to 3
	// here, in order to enable processing as general polygon.

	IterPolygon3 *iterpoly	= (IterPolygon3 *) poly;
	iterpoly->NumVerts		= 3;

	return CLIP_VolumeIterPolygon3_UVW( iterpoly, volume, cullmask );
}


// clip IterTriangle3 against volume (set of planes); (XYZ RGBA) --------------
//
IterPolygon3 *CLIP_VolumeIterTriangle3_RGBA( IterTriangle3 *poly, Plane3 *volume, dword cullmask )
{
	//NOTE:
	// the original triangle's NumVerts field will be set to 3
	// here, in order to enable processing as general polygon.

	IterPolygon3 *iterpoly	= (IterPolygon3 *) poly;
	iterpoly->NumVerts		= 3;

	return CLIP_VolumeIterPolygon3_RGBA( iterpoly, volume, cullmask );
}


// clip IterRectangle3 against volume (set of planes); (XYZ UVW RGBA) ---------
//
IterPolygon3 *CLIP_VolumeIterRectangle3( IterRectangle3 *poly, Plane3 *volume, dword cullmask )
{
	//NOTE:
	// the original rectangle's NumVerts field will be set to 4
	// here, in order to enable processing as general polygon.

	IterPolygon3 *iterpoly	= (IterPolygon3 *) poly;
	iterpoly->NumVerts		= 4;

	return CLIP_VolumeIterPolygon3( iterpoly, volume, cullmask );
}


// clip IterRectangle3 against volume (set of planes); (XYZ UVW) --------------
//
IterPolygon3 *CLIP_VolumeIterRectangle3_UVW( IterRectangle3 *poly, Plane3 *volume, dword cullmask )
{
	//NOTE:
	// the original rectangle's NumVerts field will be set to 4
	// here, in order to enable processing as general polygon.

	IterPolygon3 *iterpoly	= (IterPolygon3 *) poly;
	iterpoly->NumVerts		= 4;

	return CLIP_VolumeIterPolygon3_UVW( iterpoly, volume, cullmask );
}


// clip IterRectangle3 against volume (set of planes); (XYZ RGBA) -------------
//
IterPolygon3 *CLIP_VolumeIterRectangle3_RGBA( IterRectangle3 *poly, Plane3 *volume, dword cullmask )
{
	//NOTE:
	// the original rectangle's NumVerts field will be set to 4
	// here, in order to enable processing as general polygon.

	IterPolygon3 *iterpoly	= (IterPolygon3 *) poly;
	iterpoly->NumVerts		= 4;

	return CLIP_VolumeIterPolygon3_RGBA( iterpoly, volume, cullmask );
}


// clip IterTriangle3 against single plane; (XYZ UVW RGBA) --------------------
//
IterPolygon3 *CLIP_PlaneIterTriangle3( IterTriangle3 *poly, Plane3 *plane )
{
	//NOTE:
	// the original triangle's NumVerts field will be set to 3
	// here, in order to enable processing as general polygon.

	IterPolygon3 *iterpoly	= (IterPolygon3 *) poly;
	iterpoly->NumVerts		= 3;

	return CLIP_PlaneIterPolygon3( iterpoly, plane );
}


// clip IterTriangle3 against single plane; (XYZ UVW) -------------------------
//
IterPolygon3 *CLIP_PlaneIterTriangle3_UVW( IterTriangle3 *poly, Plane3 *plane )
{
	//NOTE:
	// the original triangle's NumVerts field will be set to 3
	// here, in order to enable processing as general polygon.

	IterPolygon3 *iterpoly	= (IterPolygon3 *) poly;
	iterpoly->NumVerts		= 3;

	return CLIP_PlaneIterPolygon3_UVW( iterpoly, plane );
}


// clip IterTriangle3 against single plane; (XYZ RGBA) ------------------------
//
IterPolygon3 *CLIP_PlaneIterTriangle3_RGBA( IterTriangle3 *poly, Plane3 *plane )
{
	//NOTE:
	// the original triangle's NumVerts field will be set to 3
	// here, in order to enable processing as general polygon.

	IterPolygon3 *iterpoly	= (IterPolygon3 *) poly;
	iterpoly->NumVerts		= 3;

	return CLIP_PlaneIterPolygon3_RGBA( iterpoly, plane );
}


// clip IterRectangle3 against single plane; (XYZ UVW RGBA) -------------------
//
IterPolygon3 *CLIP_PlaneIterRectangle3( IterRectangle3 *poly, Plane3 *plane )
{
	//NOTE:
	// the original rectangle's NumVerts field will be set to 4
	// here, in order to enable processing as general polygon.

	IterPolygon3 *iterpoly	= (IterPolygon3 *) poly;
	iterpoly->NumVerts		= 4;

	return CLIP_PlaneIterPolygon3( iterpoly, plane );
}


// clip IterRectangle3 against single plane; (XYZ UVW) ------------------------
//
IterPolygon3 *CLIP_PlaneIterRectangle3_UVW( IterRectangle3 *poly, Plane3 *plane )
{
	//NOTE:
	// the original rectangle's NumVerts field will be set to 4
	// here, in order to enable processing as general polygon.

	IterPolygon3 *iterpoly	= (IterPolygon3 *) poly;
	iterpoly->NumVerts		= 4;

	return CLIP_PlaneIterPolygon3_UVW( iterpoly, plane );
}


// clip IterRectangle3 against single plane; (XYZ RGBA) -----------------------
//
IterPolygon3 *CLIP_PlaneIterRectangle3_RGBA( IterRectangle3 *poly, Plane3 *plane )
{
	//NOTE:
	// the original rectangle's NumVerts field will be set to 4
	// here, in order to enable processing as general polygon.

	IterPolygon3 *iterpoly	= (IterPolygon3 *) poly;
	iterpoly->NumVerts		= 4;

	return CLIP_PlaneIterPolygon3_RGBA( iterpoly, plane );
}


// clip IterPolygon3 against volume (set of planes); (XYZ UVW RGBA) -----------
//
IterPolygon3 *CLIP_VolumeIterPolygon3( IterPolygon3 *poly, Plane3 *volume, dword cullmask )
{
	//NOTE:
	// this function returns:
	// - NULL if the polygon is trivial reject
	// - poly if the polygon is trivial accept
	// - pointer to a static IterPolygon3 (the clipped polygon)

	ASSERT( poly != NULL );
	ASSERT( volume != NULL );

	// check all planes that have not been culled already
	for ( int curplane = 0; cullmask != 0x00; cullmask >>= 1, curplane++ ) {

		if ( cullmask & 0x01 ) {

			// clip against current plane
			poly = CLIP_PlaneIterPolygon3( poly, &volume[ curplane ] );

			// one plane rejects: whole status is trivial reject
			if ( poly == NULL )
				return poly;
		}
	}

	// return clipped polygon/status
	return poly;
}


// clip IterPolygon3 against volume (set of planes); (XYZ UVW) ----------------
//
IterPolygon3 *CLIP_VolumeIterPolygon3_UVW( IterPolygon3 *poly, Plane3 *volume, dword cullmask )
{
	//NOTE:
	// this function returns:
	// - NULL if the polygon is trivial reject
	// - poly if the polygon is trivial accept
	// - pointer to a static IterPolygon3 (the clipped polygon)

	ASSERT( poly != NULL );
	ASSERT( volume != NULL );

	// check all planes that have not been culled already
	for ( int curplane = 0; cullmask != 0x00; cullmask >>= 1, curplane++ ) {

		if ( cullmask & 0x01 ) {

			// clip against current plane
			poly = CLIP_PlaneIterPolygon3_UVW( poly, &volume[ curplane ] );

			// one plane rejects: whole status is trivial reject
			if ( poly == NULL )
				return poly;
		}
	}

	// return clipped polygon/status
	return poly;
}


// clip IterPolygon3 against volume (set of planes); (XYZ RGBA) ---------------
//
IterPolygon3 *CLIP_VolumeIterPolygon3_RGBA( IterPolygon3 *poly, Plane3 *volume, dword cullmask )
{
	//NOTE:
	// this function returns:
	// - NULL if the polygon is trivial reject
	// - poly if the polygon is trivial accept
	// - pointer to a static IterPolygon3 (the clipped polygon)

	ASSERT( poly != NULL );
	ASSERT( volume != NULL );

	// check all planes that have not been culled already
	for ( int curplane = 0; cullmask != 0x00; cullmask >>= 1, curplane++ ) {

		if ( cullmask & 0x01 ) {

			// clip against current plane
			poly = CLIP_PlaneIterPolygon3_RGBA( poly, &volume[ curplane ] );

			// one plane rejects: whole status is trivial reject
			if ( poly == NULL )
				return poly;
		}
	}

	// return clipped polygon/status
	return poly;
}


// static storage for clipped output polygons ---------------------------------
//
static IterPolygon3 clip_polys[ 2 ][ MAX_ITERPOLY_VERTICES ];	// excessive
static geomv_t plane_distances[ MAX_ITERPOLY_VERTICES ];
static dword outcode_acceptall[ MAX_ITERPOLY_VERTICES ];


// attribute clipping macros --------------------------------------------------
//
#define POLY_CLIP_RGBA(d,t,c,p)	\
	int R = (int)(c)->R - (int)(p)->R; \
	int G = (int)(c)->G - (int)(p)->G; \
	int B = (int)(c)->B - (int)(p)->B; \
	int A = (int)(c)->A - (int)(p)->A; \
	(d)->R = (int)(p)->R + (int)( GEOMV_TO_FLOAT( t ) * R + 0.5 ); \
	(d)->G = (int)(p)->G + (int)( GEOMV_TO_FLOAT( t ) * G + 0.5 ); \
	(d)->B = (int)(p)->B + (int)( GEOMV_TO_FLOAT( t ) * B + 0.5 ); \
	(d)->A = (int)(p)->A + (int)( GEOMV_TO_FLOAT( t ) * A + 0.5 );


// clip IterPolygon3 against single plane; (XYZ UVW RGBA) ---------------------
//
IterPolygon3 *CLIP_PlaneIterPolygon3( IterPolygon3 *poly, Plane3 *plane )
{
	//NOTE:
	// this function returns:
	// - NULL if the polygon is trivial reject
	// - poly if the polygon is trivial accept
	// - pointer to a static IterPolygon3 (the clipped polygon)

	ASSERT( poly != NULL );
	ASSERT( plane != NULL );

	// fetch geometry
	IterVertex3	*vtxs    = poly->Vtxs;
	int			numverts = poly->NumVerts;

	ASSERT( vtxs != NULL );
	ASSERT( numverts > 2 );
	ASSERT( numverts <= MAX_ITERPOLY_VERTICES );

	dword outcode = 0x00;

	// refs instead of embedded vertex data?
	IterVertex3** refs = NULL;

	if ( poly->flags & ITERFLAG_VERTEXREFS ) {
		refs = (IterVertex3**) vtxs;

		// check all vertices, calc outcode and distances
		for ( int vcount = numverts; vcount > 0; vcount-- ) {
			plane_distances[ vcount-1 ] = PLANE_DOT( plane, refs[ vcount-1 ] ) -
										  PLANE_OFFSET( plane );
			if ( GEOMV_GTZERO( plane_distances[ vcount-1 ] ) )
				outcode |= 0x01;
			outcode <<= 1;
		}

	} else {

		// check all vertices, calc outcode and distances
		for ( int vcount = numverts; vcount > 0; vcount-- ) {
			plane_distances[ vcount-1 ] = PLANE_DOT( plane, &vtxs[ vcount-1 ] ) -
										  PLANE_OFFSET( plane );
			if ( GEOMV_GTZERO( plane_distances[ vcount-1 ] ) )
				outcode |= 0x01;
			outcode <<= 1;
		}
	}

	// trivial reject if no vertex in positive halfspace
	if ( outcode == 0x00 )
		return NULL;

	// trivial accept if all vertices in negative halfspace
	if ( outcode == outcode_acceptall[ numverts-1 ] )
		return poly;

	// duplicate last vertex code
	if ( outcode & ( ~outcode_acceptall[ numverts-1 ] >> 1 ) )
		outcode |= 0x01;

	// toggle static output storage
	IterPolygon3 *destpoly = (IterPolygon3 *) clip_polys[ poly == (IterPolygon3*)clip_polys ];
	IterVertex3	 *destvtxs = destpoly->Vtxs;

	// use last vertex first
	int prev_vtx = numverts - 1;

	if ( refs == NULL ) {

		// process all edges
		for ( int cur_vtx = 0; cur_vtx < numverts; outcode >>= 1, cur_vtx++ ) {

			switch ( outcode & 0x03 ) {

				// current neg, previous neg, stay invisible
//				case 0x00:
//					// skip vertex
//					break;

				// current neg, previous pos, switch to neg halfspace
				case 0x01:
					{
					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = prevd - plane_distances[ cur_vtx ];
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( prevd, seglen );

						// interpolate xyz, uvw
						IterVertex3 dvec;
						VECSUB_UVW( &dvec, &vtxs[ cur_vtx ], &vtxs[ prev_vtx ] );
						CSAXPY_UVW( destvtxs, tpara, &dvec, &vtxs[ prev_vtx ] );

						// interpolate rgba
						POLY_CLIP_RGBA( destvtxs, tpara, &vtxs[ cur_vtx ], &vtxs[ prev_vtx ] );

						destvtxs++;
					}
					}
					break;

				// current pos, previous neg, switch to pos halfspace
				case 0x02:
					{
					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = plane_distances[ cur_vtx ] - prevd;
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( -prevd, seglen );

						// interpolate xyz, uvw
						IterVertex3 dvec;
						VECSUB_UVW( &dvec, &vtxs[ cur_vtx ], &vtxs[ prev_vtx ] );
						CSAXPY_UVW( destvtxs, tpara, &dvec, &vtxs[ prev_vtx ] );

						// interpolate rgba
						POLY_CLIP_RGBA( destvtxs, tpara, &vtxs[ cur_vtx ], &vtxs[ prev_vtx ] );

						destvtxs++;
					}

					// store current vertex
					*destvtxs++ = vtxs[ cur_vtx ];
					}
					break;

				// current pos, previous pos, stay visible
				case 0x03:
					// store current vertex
					*destvtxs++ = vtxs[ cur_vtx ];
					break;
			}

			// remember previous vertex
			prev_vtx = cur_vtx;
		}

	} else {

		// process all edges
		for ( int cur_vtx = 0; cur_vtx < numverts; outcode >>= 1, cur_vtx++ ) {

			switch ( outcode & 0x03 ) {

				// current neg, previous neg, stay invisible
//				case 0x00:
//					// skip vertex
//					break;

				// current neg, previous pos, switch to neg halfspace
				case 0x01:
					{
					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = prevd - plane_distances[ cur_vtx ];
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( prevd, seglen );

						// interpolate xyz, uvw
						IterVertex3 dvec;
						VECSUB_UVW( &dvec, refs[ cur_vtx ], refs[ prev_vtx ] );
						CSAXPY_UVW( destvtxs, tpara, &dvec, refs[ prev_vtx ] );

						// interpolate rgba
						POLY_CLIP_RGBA( destvtxs, tpara, refs[ cur_vtx ], refs[ prev_vtx ] );

						destvtxs++;
					}
					}
					break;

				// current pos, previous neg, switch to pos halfspace
				case 0x02:
					{
					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = plane_distances[ cur_vtx ] - prevd;
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( -prevd, seglen );

						// interpolate xyz, uvw
						IterVertex3 dvec;
						VECSUB_UVW( &dvec, refs[ cur_vtx ], refs[ prev_vtx ] );
						CSAXPY_UVW( destvtxs, tpara, &dvec, refs[ prev_vtx ] );

						// interpolate rgba
						POLY_CLIP_RGBA( destvtxs, tpara, refs[ cur_vtx ], refs[ prev_vtx ] );

						destvtxs++;
					}

					// store current vertex
					*destvtxs++ = *refs[ cur_vtx ];
					}
					break;

				// current pos, previous pos, stay visible
				case 0x03:
					// store current vertex
					*destvtxs++ = *refs[ cur_vtx ];
					break;
			}

			// remember previous vertex
			prev_vtx = cur_vtx;
		}
	}

	// set new number of vertices
	destpoly->NumVerts = destvtxs - destpoly->Vtxs;

	// test for degenerated polygons
	if ( destpoly->NumVerts < 3 )
		return NULL;

	// set poly info
	destpoly->flags		= poly->flags & ~ITERFLAG_VERTEXREFS;
	destpoly->itertype	= poly->itertype;
	destpoly->raststate	= poly->raststate;
	destpoly->rastmask	= poly->rastmask;
	destpoly->plane		= poly->plane;
	destpoly->texmap	= poly->texmap;

	// return clipped polygon in static storage
	return destpoly;
}


// clip IterPolygon3 against single plane; (XYZ UVW) --------------------------
//
IterPolygon3 *CLIP_PlaneIterPolygon3_UVW( IterPolygon3 *poly, Plane3 *plane )
{
	//NOTE:
	// this function returns:
	// - NULL if the polygon is trivial reject
	// - poly if the polygon is trivial accept
	// - pointer to a static IterPolygon3 (the clipped polygon)

	ASSERT( poly != NULL );
	ASSERT( plane != NULL );

	// fetch geometry
	IterVertex3	*vtxs    = poly->Vtxs;
	int			numverts = poly->NumVerts;

	ASSERT( vtxs != NULL );
	ASSERT( numverts > 2 );
	ASSERT( numverts <= MAX_ITERPOLY_VERTICES );

	dword outcode = 0x00;

	// refs instead of embedded vertex data?
	IterVertex3** refs = NULL;

	if ( poly->flags & ITERFLAG_VERTEXREFS ) {
		refs = (IterVertex3**) vtxs;

		// check all vertices, calc outcode and distances
		for ( int vcount = numverts; vcount > 0; vcount-- ) {
			plane_distances[ vcount-1 ] = PLANE_DOT( plane, refs[ vcount-1 ] ) -
										  PLANE_OFFSET( plane );
			if ( GEOMV_GTZERO( plane_distances[ vcount-1 ] ) )
				outcode |= 0x01;
			outcode <<= 1;
		}

	} else {

		// check all vertices, calc outcode and distances
		for ( int vcount = numverts; vcount > 0; vcount-- ) {
			plane_distances[ vcount-1 ] = PLANE_DOT( plane, &vtxs[ vcount-1 ] ) -
										  PLANE_OFFSET( plane );
			if ( GEOMV_GTZERO( plane_distances[ vcount-1 ] ) )
				outcode |= 0x01;
			outcode <<= 1;
		}
	}

	// trivial reject if no vertex in positive halfspace
	if ( outcode == 0x00 )
		return NULL;

	// trivial accept if all vertices in negative halfspace
	if ( outcode == outcode_acceptall[ numverts-1 ] )
		return poly;

	// duplicate last vertex code
	if ( outcode & ( ~outcode_acceptall[ numverts-1 ] >> 1 ) )
		outcode |= 0x01;

	// toggle static output storage
	IterPolygon3 *destpoly = (IterPolygon3 *) clip_polys[ poly == (IterPolygon3*)clip_polys ];
	IterVertex3	 *destvtxs = destpoly->Vtxs;

	// use last vertex first
	int prev_vtx = numverts - 1;

	if ( refs == NULL ) {

		// process all edges
		for ( int cur_vtx = 0; cur_vtx < numverts; outcode >>= 1, cur_vtx++ ) {

			switch ( outcode & 0x03 ) {

				// current neg, previous neg, stay invisible
//				case 0x00:
//					// skip vertex
//					break;

				// current neg, previous pos, switch to neg halfspace
				case 0x01:
					{
					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = prevd - plane_distances[ cur_vtx ];
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( prevd, seglen );

						// interpolate xyz, uvw
						IterVertex3 dvec;
						VECSUB_UVW( &dvec, &vtxs[ cur_vtx ], &vtxs[ prev_vtx ] );
						CSAXPY_UVW( destvtxs, tpara, &dvec, &vtxs[ prev_vtx ] );

						// duplicate rgba
						*(dword*)&destvtxs->R = *(dword*)&vtxs[ cur_vtx ].R;

						destvtxs++;
					}
					}
					break;

				// current pos, previous neg, switch to pos halfspace
				case 0x02:
					{
					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = plane_distances[ cur_vtx ] - prevd;
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( -prevd, seglen );

						// interpolate xyz, uvw
						IterVertex3 dvec;
						VECSUB_UVW( &dvec, &vtxs[ cur_vtx ], &vtxs[ prev_vtx ] );
						CSAXPY_UVW( destvtxs, tpara, &dvec, &vtxs[ prev_vtx ] );

						// duplicate rgba
						*(dword*)&destvtxs->R = *(dword*)&vtxs[ cur_vtx ].R;

						destvtxs++;
					}

					// store current vertex
					*destvtxs++ = vtxs[ cur_vtx ];
					}
					break;

				// current pos, previous pos, stay visible
				case 0x03:
					// store current vertex
					*destvtxs++ = vtxs[ cur_vtx ];
					break;
			}

			// remember previous vertex
			prev_vtx = cur_vtx;
		}

	} else {

		// process all edges
		for ( int cur_vtx = 0; cur_vtx < numverts; outcode >>= 1, cur_vtx++ ) {

			switch ( outcode & 0x03 ) {

				// current neg, previous neg, stay invisible
//				case 0x00:
//					// skip vertex
//					break;

				// current neg, previous pos, switch to neg halfspace
				case 0x01:
					{
					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = prevd - plane_distances[ cur_vtx ];
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( prevd, seglen );

						// interpolate xyz, uvw
						IterVertex3 dvec;
						VECSUB_UVW( &dvec, refs[ cur_vtx ], refs[ prev_vtx ] );
						CSAXPY_UVW( destvtxs, tpara, &dvec, refs[ prev_vtx ] );

						// duplicate rgba
						*(dword*)&destvtxs->R = *(dword*)&refs[ cur_vtx ]->R;

						destvtxs++;
					}
					}
					break;

				// current pos, previous neg, switch to pos halfspace
				case 0x02:
					{
					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = plane_distances[ cur_vtx ] - prevd;
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( -prevd, seglen );

						// interpolate xyz, uvw
						IterVertex3 dvec;
						VECSUB_UVW( &dvec, refs[ cur_vtx ], refs[ prev_vtx ] );
						CSAXPY_UVW( destvtxs, tpara, &dvec, refs[ prev_vtx ] );

						// duplicate rgba
						*(dword*)&destvtxs->R = *(dword*)&refs[ cur_vtx ]->R;

						destvtxs++;
					}

					// store current vertex
					*destvtxs++ = *refs[ cur_vtx ];
					}
					break;

				// current pos, previous pos, stay visible
				case 0x03:
					// store current vertex
					*destvtxs++ = *refs[ cur_vtx ];
					break;
			}

			// remember previous vertex
			prev_vtx = cur_vtx;
		}
	}

	// set new number of vertices
	destpoly->NumVerts = destvtxs - destpoly->Vtxs;

	// test for degenerated polygons
	if ( destpoly->NumVerts < 3 )
		return NULL;

	// set poly info
	destpoly->flags		= poly->flags & ~ITERFLAG_VERTEXREFS;
	destpoly->itertype	= poly->itertype;
	destpoly->raststate	= poly->raststate;
	destpoly->rastmask	= poly->rastmask;
	destpoly->plane		= poly->plane;
	destpoly->texmap	= poly->texmap;

	// return clipped polygon in static storage
	return destpoly;
}


// clip IterPolygon3 against single plane; (XYZ RGBA) -------------------------
//
IterPolygon3 *CLIP_PlaneIterPolygon3_RGBA( IterPolygon3 *poly, Plane3 *plane )
{
	//NOTE:
	// this function returns:
	// - NULL if the polygon is trivial reject
	// - poly if the polygon is trivial accept
	// - pointer to a static IterPolygon3 (the clipped polygon)

	ASSERT( poly != NULL );
	ASSERT( plane != NULL );

	// fetch geometry
	IterVertex3	*vtxs    = poly->Vtxs;
	int			numverts = poly->NumVerts;

	ASSERT( vtxs != NULL );
	ASSERT( numverts > 2 );
	ASSERT( numverts <= MAX_ITERPOLY_VERTICES );

	dword outcode = 0x00;

	// refs instead of embedded vertex data?
	IterVertex3** refs = NULL;

	if ( poly->flags & ITERFLAG_VERTEXREFS ) {
		refs = (IterVertex3**) vtxs;

		// check all vertices, calc outcode and distances
		for ( int vcount = numverts; vcount > 0; vcount-- ) {
			plane_distances[ vcount-1 ] = PLANE_DOT( plane, refs[ vcount-1 ] ) -
										  PLANE_OFFSET( plane );
			if ( GEOMV_GTZERO( plane_distances[ vcount-1 ] ) )
				outcode |= 0x01;
			outcode <<= 1;
		}

	} else {

		// check all vertices, calc outcode and distances
		for ( int vcount = numverts; vcount > 0; vcount-- ) {
			plane_distances[ vcount-1 ] = PLANE_DOT( plane, &vtxs[ vcount-1 ] ) -
										  PLANE_OFFSET( plane );
			if ( GEOMV_GTZERO( plane_distances[ vcount-1 ] ) )
				outcode |= 0x01;
			outcode <<= 1;
		}
	}

	// trivial reject if no vertex in positive halfspace
	if ( outcode == 0x00 )
		return NULL;

	// trivial accept if all vertices in negative halfspace
	if ( outcode == outcode_acceptall[ numverts-1 ] )
		return poly;

	// duplicate last vertex code
	if ( outcode & ( ~outcode_acceptall[ numverts-1 ] >> 1 ) )
		outcode |= 0x01;

	// toggle static output storage
	IterPolygon3 *destpoly = (IterPolygon3 *) clip_polys[ poly == (IterPolygon3*)clip_polys ];
	IterVertex3	 *destvtxs = destpoly->Vtxs;

	// use last vertex first
	int prev_vtx = numverts - 1;

	if ( refs == NULL ) {

		// process all edges
		for ( int cur_vtx = 0; cur_vtx < numverts; outcode >>= 1, cur_vtx++ ) {

			switch ( outcode & 0x03 ) {

				// current neg, previous neg, stay invisible
//				case 0x00:
//					// skip vertex
//					break;

				// current neg, previous pos, switch to neg halfspace
				case 0x01:
					{
					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = prevd - plane_distances[ cur_vtx ];
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( prevd, seglen );

						// interpolate xyz
						IterVertex3 dvec;
						VECSUB( &dvec, &vtxs[ cur_vtx ], &vtxs[ prev_vtx ] );
						CSAXPY( destvtxs, tpara, &dvec, &vtxs[ prev_vtx ] );

						// duplicate uvw
						destvtxs->W = vtxs[ cur_vtx ].W;
						destvtxs->U = vtxs[ cur_vtx ].U;
						destvtxs->V = vtxs[ cur_vtx ].V;

						// interpolate rgba
						POLY_CLIP_RGBA( destvtxs, tpara, &vtxs[ cur_vtx ], &vtxs[ prev_vtx ] );

						destvtxs++;
					}
					}
					break;

				// current pos, previous neg, switch to pos halfspace
				case 0x02:
					{
					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = plane_distances[ cur_vtx ] - prevd;
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( -prevd, seglen );

						// interpolate xyz
						IterVertex3 dvec;
						VECSUB( &dvec, &vtxs[ cur_vtx ], &vtxs[ prev_vtx ] );
						CSAXPY( destvtxs, tpara, &dvec, &vtxs[ prev_vtx ] );

						// duplicate uvw
						destvtxs->W = vtxs[ cur_vtx ].W;
						destvtxs->U = vtxs[ cur_vtx ].U;
						destvtxs->V = vtxs[ cur_vtx ].V;

						// interpolate rgba
						POLY_CLIP_RGBA( destvtxs, tpara, &vtxs[ cur_vtx ], &vtxs[ prev_vtx ] );

						destvtxs++;
					}

					// store current vertex
					*destvtxs++ = vtxs[ cur_vtx ];
					}
					break;

				// current pos, previous pos, stay visible
				case 0x03:
					// store current vertex
					*destvtxs++ = vtxs[ cur_vtx ];
					break;
			}

			// remember previous vertex
			prev_vtx = cur_vtx;
		}

	} else {

		// process all edges
		for ( int cur_vtx = 0; cur_vtx < numverts; outcode >>= 1, cur_vtx++ ) {

			switch ( outcode & 0x03 ) {

				// current neg, previous neg, stay invisible
//				case 0x00:
//					// skip vertex
//					break;

				// current neg, previous pos, switch to neg halfspace
				case 0x01:
					{
					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = prevd - plane_distances[ cur_vtx ];
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( prevd, seglen );

						// interpolate xyz
						IterVertex3 dvec;
						VECSUB( &dvec, refs[ cur_vtx ], refs[ prev_vtx ] );
						CSAXPY( destvtxs, tpara, &dvec, refs[ prev_vtx ] );

						// duplicate uvw
						destvtxs->W = refs[ cur_vtx ]->W;
						destvtxs->U = refs[ cur_vtx ]->U;
						destvtxs->V = refs[ cur_vtx ]->V;

						// interpolate rgba
						POLY_CLIP_RGBA( destvtxs, tpara, refs[ cur_vtx ], refs[ prev_vtx ] );

						destvtxs++;
					}
					}
					break;

				// current pos, previous neg, switch to pos halfspace
				case 0x02:
					{
					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = plane_distances[ cur_vtx ] - prevd;
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( -prevd, seglen );

						// interpolate xyz
						IterVertex3 dvec;
						VECSUB( &dvec, refs[ cur_vtx ], refs[ prev_vtx ] );
						CSAXPY( destvtxs, tpara, &dvec, refs[ prev_vtx ] );

						// duplicate uvw
						destvtxs->W = refs[ cur_vtx ]->W;
						destvtxs->U = refs[ cur_vtx ]->U;
						destvtxs->V = refs[ cur_vtx ]->V;

						// interpolate rgba
						POLY_CLIP_RGBA( destvtxs, tpara, refs[ cur_vtx ], refs[ prev_vtx ] );

						destvtxs++;
					}

					// store current vertex
					*destvtxs++ = *refs[ cur_vtx ];
					}
					break;

				// current pos, previous pos, stay visible
				case 0x03:
					// store current vertex
					*destvtxs++ = *refs[ cur_vtx ];
					break;
			}

			// remember previous vertex
			prev_vtx = cur_vtx;
		}
	}

	// set new number of vertices
	destpoly->NumVerts = destvtxs - destpoly->Vtxs;

	// test for degenerated polygons
	if ( destpoly->NumVerts < 3 )
		return NULL;

	// set poly info
	destpoly->flags		= poly->flags & ~ITERFLAG_VERTEXREFS;
	destpoly->itertype	= poly->itertype;
	destpoly->raststate	= poly->raststate;
	destpoly->rastmask	= poly->rastmask;
	destpoly->plane		= poly->plane;
	destpoly->texmap	= poly->texmap;

	// return clipped polygon in static storage
	return destpoly;
}


// static storage for clipped output lines ------------------------------------
//
static IterLine2 clip_lines2[ 2 ][ MAX_ITERLINE_VERTICES ];	// excessive

// attribute clipping macros --------------------------------------------------
//
#define LINE_CLIP_RGBA()	\
	int R = (int)vtxs[ v1 ].R - (int)vtxs[ v0 ].R; \
	int G = (int)vtxs[ v1 ].G - (int)vtxs[ v0 ].G; \
	int B = (int)vtxs[ v1 ].B - (int)vtxs[ v0 ].B; \
	int A = (int)vtxs[ v1 ].A - (int)vtxs[ v0 ].A; \
	destvtxs[ d0 ].R = (int)vtxs[ v0 ].R + (int)( tpara * R + 0.5 ); \
	destvtxs[ d0 ].G = (int)vtxs[ v0 ].G + (int)( tpara * G + 0.5 ); \
	destvtxs[ d0 ].B = (int)vtxs[ v0 ].B + (int)( tpara * B + 0.5 ); \
	destvtxs[ d0 ].A = (int)vtxs[ v0 ].A + (int)( tpara * A + 0.5 );
/*
#define LINE_CLIP_UVW()		\
	float du = vtxs[ v1 ].U - vtxs[ v0 ].U; \
	float dv = vtxs[ v1 ].V - vtxs[ v0 ].V; \
	float dw = vtxs[ v1 ].W - vtxs[ v0 ].W; \
	destvtxs[ d0 ].U = vtxs[ v0 ].U + ( tpara * du ); \
	destvtxs[ d0 ].V = vtxs[ v0 ].V + ( tpara * du ); \
	destvtxs[ d0 ].W = vtxs[ v0 ].W + ( tpara * du );
*/


// clip IterLine2 against 2-D rectangle; (XYZ UVW RGBA) -----------------------
//
IterLine2 *CLIP_RectangleIterLine2( IterLine2 *line, Rectangle2 *rect )
{
	//NOTE:
	// this function returns:
	// - NULL if the line is trivial reject
	// - line if the line is trivial accept
	// - pointer to a static IterLine2 (the clipped line)

	//NOTE:
	//TODO:
	// not supported:
	// - closed line-strips
	// - texture coordinates
	// - depth coordinate

	ASSERT( line != NULL );
	ASSERT( rect != NULL );

	ASSERT( rect->left <= rect->right );
	ASSERT( rect->top <= rect->bottom );

	// fetch geometry
	IterVertex2	*vtxs    = line->Vtxs;
	int			numverts = line->NumVerts;

	ASSERT( vtxs != NULL );
	ASSERT( numverts > 1 );
	ASSERT( numverts <= MAX_ITERLINE_VERTICES );

	byte outcode[ 2 ];

	// first outcode
	outcode[ 0 ] = 0x00;
	if ( vtxs[ 0 ].X < rect->left )
		outcode[ 0 ] |= 0x01;
	if ( vtxs[ 0 ].X > rect->right )
		outcode[ 0 ] |= 0x02;
	if ( vtxs[ 0 ].Y < rect->top )
		outcode[ 0 ] |= 0x04;
	if ( vtxs[ 0 ].Y > rect->bottom )
		outcode[ 0 ] |= 0x08;

	// special case: single line
	if ( numverts == 2 ) {

		// second outcode
		outcode[ 1 ] = 0x00;
		if ( vtxs[ 1 ].X < rect->left )
			outcode[ 1 ] |= 0x01;
		if ( vtxs[ 1 ].X > rect->right )
			outcode[ 1 ] |= 0x02;
		if ( vtxs[ 1 ].Y < rect->top )
			outcode[ 1 ] |= 0x04;
		if ( vtxs[ 1 ].Y > rect->bottom )
			outcode[ 1 ] |= 0x08;

		// trivial accept if both vertices in rectangle
		if ( ( outcode[ 0 ] == 0 ) && ( outcode[ 1 ] == 0 ) ) {
			return line;
		}

		// trivial reject if both vertices in same outside
		if ( ( outcode[ 0 ] & outcode[ 1 ] ) != 0 ) {
			return NULL;
		}
	}

	// toggle static output storage
	IterLine2   *destline = (IterLine2 *) clip_lines2[ line == (IterLine2*)clip_lines2 ];
	IterVertex2 *destvtxs = destline->Vtxs;

	float rectl = RASTV_TO_FLOAT( rect->left );
	float rectr = RASTV_TO_FLOAT( rect->right );
	float rectt = RASTV_TO_FLOAT( rect->top );
	float rectb = RASTV_TO_FLOAT( rect->bottom );

	// source indexes
	int vtx0 = 0;
	int vtx1 = 1;

	// destination indexes
	int dst0 = 0;
	int dst1 = 1;

	// emitted vertices
	destline->NumVerts = 0;

	#define RESTART_OFF		0x00
	#define RESTART_VERTEX	0x01
	#define RESTART_OUTCODE	0x02

	// initial restart
	int restart = RESTART_VERTEX;

	// process numverts - 1 lines
	for ( int lct = numverts - 1; lct > 0; lct-- ) {

		int o0 = vtx0 & 0x01;
		int o1 = vtx1 & 0x01;

		int deltaverts = ( restart & RESTART_VERTEX ) + 1;
		destline->NumVerts += deltaverts;

		if ( restart ) {

			// copy restart vertex
			destvtxs[ dst0 ] = vtxs[ vtx0 ];
			destvtxs[ dst0 ].flags |= ITERVTXFLAG_RESTART;

			if ( restart & RESTART_OUTCODE ) {

				outcode[ o0 ] = 0x00;
				if ( vtxs[ vtx0 ].X < rect->left )
					outcode[ o0 ] |= 0x01;
				if ( vtxs[ vtx0 ].X > rect->right )
					outcode[ o0 ] |= 0x02;
				if ( vtxs[ vtx0 ].Y < rect->top )
					outcode[ o0 ] |= 0x04;
				if ( vtxs[ vtx0 ].Y > rect->bottom )
					outcode[ o0 ] |= 0x08;
			}

			restart = RESTART_OFF;
		}

		// copy next vertex
		destvtxs[ dst1 ] = vtxs[ vtx1 ];

		// next outcode
		outcode[ o1 ] = 0x00;
		if ( vtxs[ vtx1 ].X < rect->left )
			outcode[ o1 ] |= 0x01;
		if ( vtxs[ vtx1 ].X > rect->right )
			outcode[ o1 ] |= 0x02;
		if ( vtxs[ vtx1 ].Y < rect->top )
			outcode[ o1 ] |= 0x04;
		if ( vtxs[ vtx1 ].Y > rect->bottom )
			outcode[ o1 ] |= 0x08;

		// trivial accept if both vertices in rectangle
		if ( ( outcode[ o0 ] == 0 ) && ( outcode[ o1 ] == 0 ) ) {
			goto accept;
		}

		// trivial reject if both vertices in same outside
		if ( ( outcode[ o0 ] & outcode[ o1 ] ) != 0 ) {
			destline->NumVerts -= deltaverts;
			restart = RESTART_VERTEX;
			goto reject;
		}

		// read source vertices
		float fx[ 2 ], fy[ 2 ];
		fx[ vtx0 & 1 ] = RASTV_TO_FLOAT( vtxs[ vtx0 ].X );
		fy[ vtx0 & 1 ] = RASTV_TO_FLOAT( vtxs[ vtx0 ].Y );
		fx[ vtx1 & 1 ] = RASTV_TO_FLOAT( vtxs[ vtx1 ].X );
		fy[ vtx1 & 1 ] = RASTV_TO_FLOAT( vtxs[ vtx1 ].Y );

		for ( ;; ) {

			int v0 = vtx0;
			int v1 = vtx1;
			int d0 = dst0;

			if ( outcode[ v0 & 1 ] == 0 ) {

				// swap if first is inside
				SWAP_VALUES_32( v0, v1 );
				d0 = dst1;

				// endvertex will change
				restart = RESTART_VERTEX | RESTART_OUTCODE;
			}

			int f0 = v0 & 0x01;
			int f1 = v1 & 0x01;

			float deltax = fx[ f1 ] - fx[ f0 ];
			float deltay = fy[ f1 ] - fy[ f0 ];

			if ( outcode[ f0 ] & 0x01 ) {

				// clip against left
				float lseg0 = rectl - fx[ f0 ];
				float tpara = lseg0 / ( fx[ f1 ] - fx[ f0 ] );

				fx[ f0 ]  = rectl;
				fy[ f0 ] += tpara * deltay;

				LINE_CLIP_RGBA();

			} else if ( outcode[ f0 ] & 0x02 ) {

				// clip against right
				float lseg0 = fx[ f0 ] - rectr;
				float tpara = lseg0 / ( fx[ f0 ] - fx[ f1 ] );

				fx[ f0 ]  = rectr;
				fy[ f0 ] += tpara * deltay;

				LINE_CLIP_RGBA();

			} else if ( outcode[ f0 ] & 0x04 ) {

				// clip against top
				float lseg0 = rectt - fy[ f0 ];
				float tpara = lseg0 / ( fy[ f1 ] - fy[ f0 ] );

				fx[ f0 ] += tpara * deltax;
				fy[ f0 ]  = rectt;

				LINE_CLIP_RGBA();

			} else if ( outcode[ f0 ] & 0x08 ) {

				// clip against bottom
				float lseg0 = fy[ f0 ] - rectb;
				float tpara = lseg0 / ( fy[ f0 ] - fy[ f1 ] );

				fx[ f0 ] += tpara * deltax;
				fy[ f0 ]  = rectb;

				LINE_CLIP_RGBA();
			}

			// new first outcode
			outcode[ f0 ] = 0x00;
			if ( fx[ f0 ] < rectl )
				outcode[ f0 ] |= 0x01;
			if ( fx[ f0 ] > rectr )
				outcode[ f0 ] |= 0x02;
			if ( fy[ f0 ] < rectt )
				outcode[ f0 ] |= 0x04;
			if ( fy[ f0 ] > rectb )
				outcode[ f0 ] |= 0x08;

			// trivial accept if both vertices in rectangle
			if ( ( outcode[ f0 ] == 0 ) && ( outcode[ f1 ] == 0 ) ) {
				break;
			}

			// trivial reject if both vertices in same outside
			if ( ( outcode[ f0 ] & outcode[ f1 ] ) != 0 ) {
				destline->NumVerts -= deltaverts;
				restart = RESTART_VERTEX;
				goto reject;
			}
		}

		// store clipped vertices
		destvtxs[ dst0 ].X = FLOAT_TO_RASTV( fx[ o0 ] );
		destvtxs[ dst0 ].Y = FLOAT_TO_RASTV( fy[ o0 ] );
		destvtxs[ dst1 ].X = FLOAT_TO_RASTV( fx[ o1 ] );
		destvtxs[ dst1 ].Y = FLOAT_TO_RASTV( fy[ o1 ] );

		// make room for two new vertices
		// instead of just one
		if ( restart ) {
			dst0++;
			dst1++;
		}
accept:
		// advance destination
		dst0++;
		dst1++;
reject:
		// advance source
		vtx0++;
		vtx1++;
	}

	// test for degenerated lines
	if ( destline->NumVerts < 2 )
		return NULL;

	// set line info
	destline->flags		= line->flags;
	destline->itertype	= line->itertype;
	destline->raststate	= line->raststate;
	destline->rastmask	= line->rastmask;
	destline->texmap	= line->texmap;

	// return clipped line in static storage
	return destline;
}


// clip IterLine3 against single plane; (XYZ UVW RGBA) ------------------------
//
IterLine3 *CLIP_PlaneIterLine3( IterLine3 *line, Plane3 *plane )
{
	//NOTE:
	// this function returns:
	// - NULL if the line is trivial reject
	// - line if the line is trivial accept
	// - pointer to a static IterLine3 (the clipped line)

	ASSERT( line != NULL );
	ASSERT( plane != NULL );

	//TODO:

	return line;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( UTL_CLIP )
{
	// limit is 31 due to bit-twiddling!!
	ASSERT( MAX_ITERPOLY_VERTICES < 32 );

	// fill trivial accept table for all vertex counts
	dword outcode = 0x02;
	for ( int code = 0; code < MAX_ITERPOLY_VERTICES; code++ ) {

		outcode_acceptall[ code ] = outcode;
		outcode = ( outcode << 1 ) | 0x02;
	}
}



