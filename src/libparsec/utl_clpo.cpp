/*
 * PARSEC - Object Clipping Code
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
#include "utl_clpo.h"


// flags
#define CLIP_ORIENTED_EDGES
//#define INSANE_LIMITS



// clipped object limitations -------------------------------------------------
//
#ifdef INSANE_LIMITS
	#define CLIPPED_MAX_VERTS			16384
	#define CLIPPED_MAX_POLYS			8192
	#define CLIPPED_MAX_INSTANCE_SIZE	( sizeof( GenObject ) + 32768 )
#else
	#define CLIPPED_MAX_VERTS			4096
	#define CLIPPED_MAX_POLYS			1024
	#define CLIPPED_MAX_INSTANCE_SIZE	( sizeof( GenObject ) + 16384 )
#endif // INSANE_LIMITS

#define MAX_CLIPPOLY_VERTICES		31


// storage sizes for genobject data areas -------------------------------------
//
#define SZ_VERTEXLIST	( CLIPPED_MAX_VERTS * sizeof( Vertex3 ) )
#define SZ_XVERTEXLIST	( CLIPPED_MAX_VERTS * sizeof( Vertex3 ) )
#define SZ_SVERTEXLIST	( CLIPPED_MAX_VERTS * sizeof( SPoint ) )
#define SZ_POLYLIST		( CLIPPED_MAX_POLYS * sizeof( Poly ) )
#define SZ_POLYINDEXES	( CLIPPED_MAX_POLYS * MAX_CLIPPOLY_VERTICES * sizeof( dword ) * 2 )


// create clipped genobject from unclipped genobject (against volume) ---------
//
static GenObject *clipped_obj	= NULL;
static size_t clipped_obj_size	=
	CLIPPED_MAX_INSTANCE_SIZE +
	SZ_VERTEXLIST + SZ_XVERTEXLIST + SZ_SVERTEXLIST +
	SZ_POLYLIST + SZ_POLYINDEXES;


// globals needed by the following routines -----------------------------------
//
Vertex3*	clp_vtxlist;	// pointer to vertices
int			clp_nextindx;	// next vertex index for clipped vertices
dword*		clp_wcolors;	// lighted wedge colors


// static storage for clipped output polygons ---------------------------------
//
static Poly		clip_polys[ 2 ][ MAX_CLIPPOLY_VERTICES ];	// excessive
static geomv_t	plane_distances[ MAX_CLIPPOLY_VERTICES ];
static dword	outcode_acceptall[ MAX_CLIPPOLY_VERTICES ];
static dword	temp_colors[ MAX_CLIPPOLY_VERTICES ];


// clip Poly against single plane ---------------------------------------------
//
Poly *CLIP_PlanePoly( Poly *poly, Plane3 *plane )
{
	//NOTE:
	// this function returns:
	// - NULL if the polygon is trivial reject
	// - poly if the polygon is trivial accept
	// - pointer to a static Poly (the clipped polygon)

	//NOTE:
	// clp_vtxlist and clp_nextindx have to be
	// set correctly before calling this function.

	ASSERT( poly != NULL );
	ASSERT( plane != NULL );

	// fetch geometry
	dword *indxs = poly->VertIndxs;
	int numverts = poly->NumVerts;

	ASSERT( indxs != NULL );
	ASSERT( numverts > 2 );
	ASSERT( numverts <= MAX_CLIPPOLY_VERTICES );

	// check all vertices, calc outcode and distances
	dword outcode = 0x00;
	for ( int vcount = numverts; vcount > 0; vcount-- ) {

		plane_distances[ vcount-1 ] =
			PLANE_DOT( plane, &clp_vtxlist[ indxs[ vcount-1 ] ] ) -
			PLANE_OFFSET( plane );

		if ( GEOMV_GTZERO( plane_distances[ vcount-1 ] ) )
			outcode |= 0x01;
		outcode <<= 1;
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
	Poly  *destpoly  = (Poly *) clip_polys[ poly == (Poly*)clip_polys ];
	dword *destindxs = (dword *) ( destpoly + 1 );

	destpoly->FaceIndx  = poly->FaceIndx;
	destpoly->VertIndxs = destindxs;
	destpoly->Flags     = poly->Flags;

	// use last vertex first
	int prev_vtx = numverts - 1;

	// process all edges
	for ( int cur_vtx = 0; cur_vtx < numverts; outcode >>= 1, cur_vtx++ ) {

		switch ( outcode & 0x03 ) {

			// current neg, previous neg, stay invisible
//			case 0x00:
//				// skip vertex
//				break;

			// current neg, previous pos, switch to neg halfspace
			case 0x01:
				{

				ASSERT( clp_nextindx < CLIPPED_MAX_VERTS );

#ifdef CLIP_ORIENTED_EDGES

				// check direction (vertex index order)
				if ( indxs[ cur_vtx ] >= indxs[ prev_vtx ] ) {

					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = prevd - plane_distances[ cur_vtx ];
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( prevd, seglen );

						// interpolate xyz
						Vector3 dvec;
						VECSUB( &dvec, &clp_vtxlist[ indxs[ cur_vtx ] ],
								&clp_vtxlist[ indxs[ prev_vtx ] ] );
						CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
								&clp_vtxlist[ indxs[ prev_vtx ] ] );
						clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

						*destindxs++ = clp_nextindx++;
					}

				} else {

					geomv_t prevd  = plane_distances[ cur_vtx ];
					geomv_t seglen = plane_distances[ prev_vtx ] - prevd;
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( -prevd, seglen );

						// interpolate xyz
						Vector3 dvec;
						VECSUB( &dvec, &clp_vtxlist[ indxs[ prev_vtx ] ],
								&clp_vtxlist[ indxs[ cur_vtx ] ] );
						CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
								&clp_vtxlist[ indxs[ cur_vtx ] ] );
						clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

						*destindxs++ = clp_nextindx++;
					}
				}
#else
				geomv_t prevd  = plane_distances[ prev_vtx ];
				geomv_t seglen = prevd - plane_distances[ cur_vtx ];
				ASSERT( seglen >= 0 );

				// don't create intersection vertex if both on plane
				if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

					geomv_t tpara = GEOMV_DIV( prevd, seglen );

					// interpolate xyz
					Vector3 dvec;
					VECSUB( &dvec, &clp_vtxlist[ indxs[ cur_vtx ] ],
							&clp_vtxlist[ indxs[ prev_vtx ] ] );
					CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
							&clp_vtxlist[ indxs[ prev_vtx ] ] );
					clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

					*destindxs++ = clp_nextindx++;
				}
#endif
				}
				break;

			// current pos, previous neg, switch to pos halfspace
			case 0x02:
				{

				ASSERT( clp_nextindx < CLIPPED_MAX_VERTS );

#ifdef CLIP_ORIENTED_EDGES

				// check direction (vertex index order)
				if ( indxs[ cur_vtx ] >= indxs[ prev_vtx ] ) {

					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = plane_distances[ cur_vtx ] - prevd;
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( -prevd, seglen );

						// interpolate xyz
						Vector3 dvec;
						VECSUB( &dvec, &clp_vtxlist[ indxs[ cur_vtx ] ],
								&clp_vtxlist[ indxs[ prev_vtx ] ] );
						CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
								&clp_vtxlist[ indxs[ prev_vtx ] ] );
						clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

						*destindxs++ = clp_nextindx++;
					}

				} else {

					geomv_t prevd  = plane_distances[ cur_vtx ];
					geomv_t seglen = prevd - plane_distances[ prev_vtx ];
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( prevd, seglen );

						// interpolate xyz
						Vector3 dvec;
						VECSUB( &dvec, &clp_vtxlist[ indxs[ prev_vtx ] ],
								&clp_vtxlist[ indxs[ cur_vtx ] ] );
						CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
								&clp_vtxlist[ indxs[ cur_vtx ] ] );
						clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

						*destindxs++ = clp_nextindx++;
					}
				}
#else
				geomv_t prevd  = plane_distances[ prev_vtx ];
				geomv_t seglen = plane_distances[ cur_vtx ] - prevd;
				ASSERT( seglen >= 0 );

				// don't create intersection vertex if both on plane
				if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

					geomv_t tpara = GEOMV_DIV( -prevd, seglen );

					// interpolate xyz
					Vector3 dvec;
					VECSUB( &dvec, &clp_vtxlist[ indxs[ cur_vtx ] ],
							&clp_vtxlist[ indxs[ prev_vtx ] ] );
					CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
							&clp_vtxlist[ indxs[ prev_vtx ] ] );
					clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

					*destindxs++ = clp_nextindx++;
				}
#endif
				// store current index
				*destindxs++ = indxs[ cur_vtx ];
				}
				break;

			// current pos, previous pos, stay visible
			case 0x03:
				// store current index
				*destindxs++ = indxs[ cur_vtx ];
				break;
		}

		// remember previous vertex
		prev_vtx = cur_vtx;
	}

	// set new number of vertices
	destpoly->NumVerts = (dword) (destindxs - destpoly->VertIndxs);

	// test for degenerated polygons
	if ( destpoly->NumVerts < 3 )
		return NULL;

	// return clipped polygon in static storage
	return destpoly;
}


// attribute clipping macros --------------------------------------------------
//
#define CLIP_WEDGE_RGBA(d,t,c,p)	\
	int R = (int)((colrgba_s*)&clp_wcolors[ indxs[(c)+numverts] ])->R - (int)((colrgba_s*)&clp_wcolors[ indxs[(p)+numverts] ])->R; \
	int G = (int)((colrgba_s*)&clp_wcolors[ indxs[(c)+numverts] ])->G - (int)((colrgba_s*)&clp_wcolors[ indxs[(p)+numverts] ])->G; \
	int B = (int)((colrgba_s*)&clp_wcolors[ indxs[(c)+numverts] ])->B - (int)((colrgba_s*)&clp_wcolors[ indxs[(p)+numverts] ])->B; \
	int A = (int)((colrgba_s*)&clp_wcolors[ indxs[(c)+numverts] ])->A - (int)((colrgba_s*)&clp_wcolors[ indxs[(p)+numverts] ])->A; \
	((colrgba_s*)(d))->R = (int)((colrgba_s*)&clp_wcolors[ indxs[(p)+numverts] ])->R + (int)( GEOMV_TO_FLOAT( t ) * R + 0.5 ); \
	((colrgba_s*)(d))->G = (int)((colrgba_s*)&clp_wcolors[ indxs[(p)+numverts] ])->G + (int)( GEOMV_TO_FLOAT( t ) * G + 0.5 ); \
	((colrgba_s*)(d))->B = (int)((colrgba_s*)&clp_wcolors[ indxs[(p)+numverts] ])->B + (int)( GEOMV_TO_FLOAT( t ) * B + 0.5 ); \
	((colrgba_s*)(d))->A = (int)((colrgba_s*)&clp_wcolors[ indxs[(p)+numverts] ])->A + (int)( GEOMV_TO_FLOAT( t ) * A + 0.5 );


// clip Poly against single plane ---------------------------------------------
//
Poly *CLIP_PlanePolyWedgeColors( Poly *poly, Plane3 *plane )
{
	//NOTE:
	// this function returns:
	// - NULL if the polygon is trivial reject
	// - poly if the polygon is trivial accept
	// - pointer to a static Poly (the clipped polygon)

	//NOTE:
	// clp_vtxlist, clp_nextindx, and clp_wcolors have
	// to be set correctly before calling this function.

	ASSERT( poly != NULL );
	ASSERT( plane != NULL );

	ASSERT( ( poly->Flags & POLYFLAG_WEDGEINDEXES ) != 0 );
	ASSERT( ( poly->Flags & POLYFLAG_CORNERCOLORS ) == 0 );

	// fetch geometry
	dword *indxs = poly->VertIndxs;
	int numverts = poly->NumVerts;

	ASSERT( indxs != NULL );
	ASSERT( numverts > 2 );
	ASSERT( numverts <= MAX_CLIPPOLY_VERTICES );

	// check all vertices, calc outcode and distances
	dword outcode = 0x00;
	for ( int vcount = numverts; vcount > 0; vcount-- ) {

		plane_distances[ vcount-1 ] =
			PLANE_DOT( plane, &clp_vtxlist[ indxs[ vcount-1 ] ] ) -
			PLANE_OFFSET( plane );

		if ( GEOMV_GTZERO( plane_distances[ vcount-1 ] ) )
			outcode |= 0x01;
		outcode <<= 1;
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
	Poly  *destpoly   = (Poly *) clip_polys[ poly == (Poly*)clip_polys ];
	dword *destindxs  = (dword *) ( destpoly + 1 );
	dword *tempcolors = temp_colors;

	destpoly->FaceIndx  = poly->FaceIndx;
	destpoly->VertIndxs = destindxs;
	destpoly->Flags     = poly->Flags;

	// use last vertex first
	int prev_vtx = numverts - 1;

	// process all edges
	for ( int cur_vtx = 0; cur_vtx < numverts; outcode >>= 1, cur_vtx++ ) {

		switch ( outcode & 0x03 ) {

			// current neg, previous neg, stay invisible
//			case 0x00:
//				// skip vertex
//				break;

			// current neg, previous pos, switch to neg halfspace
			case 0x01:
				{

				ASSERT( clp_nextindx < CLIPPED_MAX_VERTS );

#ifdef CLIP_ORIENTED_EDGES

				// check direction (vertex index order)
				if ( indxs[ cur_vtx ] >= indxs[ prev_vtx ] ) {

					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = prevd - plane_distances[ cur_vtx ];
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( prevd, seglen );

						// interpolate xyz
						Vector3 dvec;
						VECSUB( &dvec, &clp_vtxlist[ indxs[ cur_vtx ] ],
								&clp_vtxlist[ indxs[ prev_vtx ] ] );
						CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
								&clp_vtxlist[ indxs[ prev_vtx ] ] );
						clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

						// interpolate rgba
						CLIP_WEDGE_RGBA( tempcolors, tpara, cur_vtx, prev_vtx );
						tempcolors++;

						*destindxs++ = clp_nextindx++;
					}

				} else {

					geomv_t prevd  = plane_distances[ cur_vtx ];
					geomv_t seglen = plane_distances[ prev_vtx ] - prevd;
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( -prevd, seglen );

						// interpolate xyz
						Vector3 dvec;
						VECSUB( &dvec, &clp_vtxlist[ indxs[ prev_vtx ] ],
								&clp_vtxlist[ indxs[ cur_vtx ] ] );
						CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
								&clp_vtxlist[ indxs[ cur_vtx ] ] );
						clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

						// interpolate rgba
						CLIP_WEDGE_RGBA( tempcolors, tpara, prev_vtx, cur_vtx );
						tempcolors++;

						*destindxs++ = clp_nextindx++;
					}
				}
#else
				geomv_t prevd  = plane_distances[ prev_vtx ];
				geomv_t seglen = prevd - plane_distances[ cur_vtx ];
				ASSERT( seglen >= 0 );

				// don't create intersection vertex if both on plane
				if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

					geomv_t tpara = GEOMV_DIV( prevd, seglen );

					// interpolate xyz
					Vector3 dvec;
					VECSUB( &dvec, &clp_vtxlist[ indxs[ cur_vtx ] ],
							&clp_vtxlist[ indxs[ prev_vtx ] ] );
					CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
							&clp_vtxlist[ indxs[ prev_vtx ] ] );
					clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

					// interpolate rgba
					CLIP_WEDGE_RGBA( tempcolors, tpara, cur_vtx, prev_vtx );
					tempcolors++;

					*destindxs++ = clp_nextindx++;
				}
#endif
				}
				break;

			// current pos, previous neg, switch to pos halfspace
			case 0x02:
				{

				ASSERT( clp_nextindx < CLIPPED_MAX_VERTS );

#ifdef CLIP_ORIENTED_EDGES

				// check direction (vertex index order)
				if ( indxs[ cur_vtx ] >= indxs[ prev_vtx ] ) {

					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = plane_distances[ cur_vtx ] - prevd;
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( -prevd, seglen );

						// interpolate xyz
						Vector3 dvec;
						VECSUB( &dvec, &clp_vtxlist[ indxs[ cur_vtx ] ],
								&clp_vtxlist[ indxs[ prev_vtx ] ] );
						CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
								&clp_vtxlist[ indxs[ prev_vtx ] ] );
						clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

						// interpolate rgba
						CLIP_WEDGE_RGBA( tempcolors, tpara, cur_vtx, prev_vtx );
						tempcolors++;

						*destindxs++ = clp_nextindx++;
					}

				} else {

					geomv_t prevd  = plane_distances[ cur_vtx ];
					geomv_t seglen = prevd - plane_distances[ prev_vtx ];
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( prevd, seglen );

						// interpolate xyz
						Vector3 dvec;
						VECSUB( &dvec, &clp_vtxlist[ indxs[ prev_vtx ] ],
								&clp_vtxlist[ indxs[ cur_vtx ] ] );
						CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
								&clp_vtxlist[ indxs[ cur_vtx ] ] );
						clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

						// interpolate rgba
						CLIP_WEDGE_RGBA( tempcolors, tpara, prev_vtx, cur_vtx );
						tempcolors++;

						*destindxs++ = clp_nextindx++;
					}
				}
#else
				geomv_t prevd  = plane_distances[ prev_vtx ];
				geomv_t seglen = plane_distances[ cur_vtx ] - prevd;
				ASSERT( seglen >= 0 );

				// don't create intersection vertex if both on plane
				if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

					geomv_t tpara = GEOMV_DIV( -prevd, seglen );

					// interpolate xyz
					Vector3 dvec;
					VECSUB( &dvec, &clp_vtxlist[ indxs[ cur_vtx ] ],
							&clp_vtxlist[ indxs[ prev_vtx ] ] );
					CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
							&clp_vtxlist[ indxs[ prev_vtx ] ] );
					clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

					// interpolate rgba
					CLIP_WEDGE_RGBA( tempcolors, tpara, cur_vtx, prev_vtx );
					tempcolors++;

					*destindxs++ = clp_nextindx++;
				}
#endif
				// store current index and color
				*destindxs++  = indxs[ cur_vtx ];
				*tempcolors++ = clp_wcolors[ indxs[ cur_vtx + numverts ] ];
				}
				break;

			// current pos, previous pos, stay visible
			case 0x03:
				// store current index and color
				*destindxs++  = indxs[ cur_vtx ];
				*tempcolors++ = clp_wcolors[ indxs[ cur_vtx + numverts ] ];
				break;
		}

		// remember previous vertex
		prev_vtx = cur_vtx;
	}

	// set new number of vertices
	destpoly->NumVerts = destindxs - destpoly->VertIndxs;

	// test for degenerated polygons
	if ( destpoly->NumVerts < 3 )
		return NULL;

	// apppend colors to vertex indexes
	ASSERT( (dword)( tempcolors - temp_colors ) == destpoly->NumVerts );
	memcpy( destindxs, temp_colors, destpoly->NumVerts * sizeof( dword ) );

	destpoly->Flags &= ~POLYFLAG_WEDGEINDEXES;
	destpoly->Flags |= POLYFLAG_CORNERCOLORS;

	// return clipped polygon in static storage
	return destpoly;
}


// attribute clipping macros --------------------------------------------------
//
#define CLIP_CORNER_RGBA(d,t,c,p)	\
	int R = (int)((colrgba_s*)&indxs[(c)+numverts])->R - (int)((colrgba_s*)&indxs[(p)+numverts])->R; \
	int G = (int)((colrgba_s*)&indxs[(c)+numverts])->G - (int)((colrgba_s*)&indxs[(p)+numverts])->G; \
	int B = (int)((colrgba_s*)&indxs[(c)+numverts])->B - (int)((colrgba_s*)&indxs[(p)+numverts])->B; \
	int A = (int)((colrgba_s*)&indxs[(c)+numverts])->A - (int)((colrgba_s*)&indxs[(p)+numverts])->A; \
	((colrgba_s*)(d))->R = (int)((colrgba_s*)&indxs[(p)+numverts])->R + (int)( GEOMV_TO_FLOAT( t ) * R + 0.5 ); \
	((colrgba_s*)(d))->G = (int)((colrgba_s*)&indxs[(p)+numverts])->G + (int)( GEOMV_TO_FLOAT( t ) * G + 0.5 ); \
	((colrgba_s*)(d))->B = (int)((colrgba_s*)&indxs[(p)+numverts])->B + (int)( GEOMV_TO_FLOAT( t ) * B + 0.5 ); \
	((colrgba_s*)(d))->A = (int)((colrgba_s*)&indxs[(p)+numverts])->A + (int)( GEOMV_TO_FLOAT( t ) * A + 0.5 );


// clip Poly against single plane ---------------------------------------------
//
Poly *CLIP_PlanePolyCornerColors( Poly *poly, Plane3 *plane )
{
	//NOTE:
	// this function returns:
	// - NULL if the polygon is trivial reject
	// - poly if the polygon is trivial accept
	// - pointer to a static Poly (the clipped polygon)

	//NOTE:
	// clp_vtxlist and clp_nextindx have to be
	// set correctly before calling this function.

	ASSERT( poly != NULL );
	ASSERT( plane != NULL );

	ASSERT( ( poly->Flags & POLYFLAG_CORNERCOLORS ) != 0 );
	ASSERT( ( poly->Flags & POLYFLAG_WEDGEINDEXES ) == 0 );

	// fetch geometry
	dword *indxs = poly->VertIndxs;
	int numverts = poly->NumVerts;

	ASSERT( indxs != NULL );
	ASSERT( numverts > 2 );
	ASSERT( numverts <= MAX_CLIPPOLY_VERTICES );

	// check all vertices, calc outcode and distances
	dword outcode = 0x00;
	for ( int vcount = numverts; vcount > 0; vcount-- ) {

		plane_distances[ vcount-1 ] =
			PLANE_DOT( plane, &clp_vtxlist[ indxs[ vcount-1 ] ] ) -
			PLANE_OFFSET( plane );

		if ( GEOMV_GTZERO( plane_distances[ vcount-1 ] ) )
			outcode |= 0x01;
		outcode <<= 1;
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
	Poly  *destpoly   = (Poly *) clip_polys[ poly == (Poly*)clip_polys ];
	dword *destindxs  = (dword *) ( destpoly + 1 );
	dword *tempcolors = temp_colors;

	destpoly->FaceIndx  = poly->FaceIndx;
	destpoly->VertIndxs = destindxs;
	destpoly->Flags     = poly->Flags;

	// use last vertex first
	int prev_vtx = numverts - 1;

	// process all edges
	for ( int cur_vtx = 0; cur_vtx < numverts; outcode >>= 1, cur_vtx++ ) {

		switch ( outcode & 0x03 ) {

			// current neg, previous neg, stay invisible
//			case 0x00:
//				// skip vertex
//				break;

			// current neg, previous pos, switch to neg halfspace
			case 0x01:
				{

				ASSERT( clp_nextindx < CLIPPED_MAX_VERTS );

#ifdef CLIP_ORIENTED_EDGES

				// check direction (vertex index order)
				if ( indxs[ cur_vtx ] >= indxs[ prev_vtx ] ) {

					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = prevd - plane_distances[ cur_vtx ];
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( prevd, seglen );

						// interpolate xyz
						Vector3 dvec;
						VECSUB( &dvec, &clp_vtxlist[ indxs[ cur_vtx ] ],
								&clp_vtxlist[ indxs[ prev_vtx ] ] );
						CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
								&clp_vtxlist[ indxs[ prev_vtx ] ] );
						clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

						// interpolate rgba
						CLIP_CORNER_RGBA( tempcolors, tpara, cur_vtx, prev_vtx );
						tempcolors++;

						*destindxs++ = clp_nextindx++;
					}

				} else {

					geomv_t prevd  = plane_distances[ cur_vtx ];
					geomv_t seglen = plane_distances[ prev_vtx ] - prevd;
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( -prevd, seglen );

						// interpolate xyz
						Vector3 dvec;
						VECSUB( &dvec, &clp_vtxlist[ indxs[ prev_vtx ] ],
								&clp_vtxlist[ indxs[ cur_vtx ] ] );
						CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
								&clp_vtxlist[ indxs[ cur_vtx ] ] );
						clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

						// interpolate rgba
						CLIP_CORNER_RGBA( tempcolors, tpara, prev_vtx, cur_vtx );
						tempcolors++;

						*destindxs++ = clp_nextindx++;
					}
				}
#else
				geomv_t prevd  = plane_distances[ prev_vtx ];
				geomv_t seglen = prevd - plane_distances[ cur_vtx ];
				ASSERT( seglen >= 0 );

				// don't create intersection vertex if both on plane
				if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

					geomv_t tpara = GEOMV_DIV( prevd, seglen );

					// interpolate xyz
					Vector3 dvec;
					VECSUB( &dvec, &clp_vtxlist[ indxs[ cur_vtx ] ],
							&clp_vtxlist[ indxs[ prev_vtx ] ] );
					CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
							&clp_vtxlist[ indxs[ prev_vtx ] ] );
					clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

					// interpolate rgba
					CLIP_CORNER_RGBA( tempcolors, tpara, cur_vtx, prev_vtx );
					tempcolors++;

					*destindxs++ = clp_nextindx++;
				}
#endif
				}
				break;

			// current pos, previous neg, switch to pos halfspace
			case 0x02:
				{

				ASSERT( clp_nextindx < CLIPPED_MAX_VERTS );

#ifdef CLIP_ORIENTED_EDGES

				// check direction (vertex index order)
				if ( indxs[ cur_vtx ] >= indxs[ prev_vtx ] ) {

					geomv_t prevd  = plane_distances[ prev_vtx ];
					geomv_t seglen = plane_distances[ cur_vtx ] - prevd;
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( -prevd, seglen );

						// interpolate xyz
						Vector3 dvec;
						VECSUB( &dvec, &clp_vtxlist[ indxs[ cur_vtx ] ],
								&clp_vtxlist[ indxs[ prev_vtx ] ] );
						CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
								&clp_vtxlist[ indxs[ prev_vtx ] ] );
						clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

						// interpolate rgba
						CLIP_CORNER_RGBA( tempcolors, tpara, cur_vtx, prev_vtx );
						tempcolors++;

						*destindxs++ = clp_nextindx++;
					}

				} else {

					geomv_t prevd  = plane_distances[ cur_vtx ];
					geomv_t seglen = prevd - plane_distances[ prev_vtx ];
					ASSERT( seglen >= 0 );

					// don't create intersection vertex if both on plane
					if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

						geomv_t tpara = GEOMV_DIV( prevd, seglen );

						// interpolate xyz
						Vector3 dvec;
						VECSUB( &dvec, &clp_vtxlist[ indxs[ prev_vtx ] ],
								&clp_vtxlist[ indxs[ cur_vtx ] ] );
						CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
								&clp_vtxlist[ indxs[ cur_vtx ] ] );
						clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

						// interpolate rgba
						CLIP_CORNER_RGBA( tempcolors, tpara, prev_vtx, cur_vtx );
						tempcolors++;

						*destindxs++ = clp_nextindx++;
					}
				}
#else
				geomv_t prevd  = plane_distances[ prev_vtx ];
				geomv_t seglen = plane_distances[ cur_vtx ] - prevd;
				ASSERT( seglen >= 0 );

				// don't create intersection vertex if both on plane
				if ( seglen > FLOAT_TO_GEOMV( 0.00001 ) ) {

					geomv_t tpara = GEOMV_DIV( -prevd, seglen );

					// interpolate xyz
					Vector3 dvec;
					VECSUB( &dvec, &clp_vtxlist[ indxs[ cur_vtx ] ],
							&clp_vtxlist[ indxs[ prev_vtx ] ] );
					CSAXPY( &clp_vtxlist[ clp_nextindx ], tpara, &dvec,
							&clp_vtxlist[ indxs[ prev_vtx ] ] );
					clp_vtxlist[ clp_nextindx ].VisibleFrame = 0;

					// interpolate rgba
					CLIP_CORNER_RGBA( tempcolors, tpara, cur_vtx, prev_vtx );
					tempcolors++;

					*destindxs++ = clp_nextindx++;
				}
#endif
				// store current index and color
				*destindxs++  = indxs[ cur_vtx ];
				*tempcolors++ = indxs[ cur_vtx + numverts ];
				}
				break;

			// current pos, previous pos, stay visible
			case 0x03:
				// store current index and color
				*destindxs++  = indxs[ cur_vtx ];
				*tempcolors++ = indxs[ cur_vtx + numverts ];
				break;
		}

		// remember previous vertex
		prev_vtx = cur_vtx;
	}

	// set new number of vertices
	destpoly->NumVerts = destindxs - destpoly->VertIndxs;

	// test for degenerated polygons
	if ( destpoly->NumVerts < 3 )
		return NULL;

	// apppend colors to vertex indexes
	ASSERT( (dword)( tempcolors - temp_colors ) == destpoly->NumVerts );
	memcpy( destindxs, temp_colors, destpoly->NumVerts * sizeof( dword ) );

	// return clipped polygon in static storage
	return destpoly;
}


// clip Poly against volume (set of planes) -----------------------------------
//
Poly *CLIP_VolumePoly( Poly *poly, Plane3 *volume, dword cullmask )
{
	//NOTE:
	// this function returns:
	// - NULL if the polygon is trivial reject
	// - poly if the polygon is trivial accept
	// - pointer to a static Poly (the clipped polygon)

	//NOTE:
	// clp_vtxlist and clp_nextindx have to be
	// set correctly before calling this function.

	ASSERT( poly != NULL );
	ASSERT( volume != NULL );

	// check all planes that have not been culled already
	for ( int curplane = 0; cullmask != 0x00; cullmask >>= 1, curplane++ ) {

		if ( cullmask & 0x01 ) {

			// clip against current plane
			poly = CLIP_PlanePoly( poly, &volume[ curplane ] );

			// one plane rejects: whole status is trivial reject
			if ( poly == NULL )
				return poly;
		}
	}

	// return clipped polygon/status
	return poly;
}


// clip Poly against volume (set of planes) -----------------------------------
//
Poly *CLIP_VolumePolyColors( Poly *poly, Plane3 *volume, dword cullmask )
{
	//NOTE:
	// this function returns:
	// - NULL if the polygon is trivial reject
	// - poly if the polygon is trivial accept
	// - pointer to a static Poly (the clipped polygon)

	//NOTE:
	// clp_vtxlist and clp_nextindx have to be
	// set correctly before calling this function.

	ASSERT( poly != NULL );
	ASSERT( volume != NULL );

	// check all planes that have not been culled already
	for ( int curplane = 0; cullmask != 0x00; cullmask >>= 1, curplane++ ) {

		if ( cullmask & 0x01 ) {

			// clip against current plane
			if ( poly->Flags & POLYFLAG_WEDGEINDEXES ) {
				poly = CLIP_PlanePolyWedgeColors( poly, &volume[ curplane ] );
			} else {
				poly = CLIP_PlanePolyCornerColors( poly, &volume[ curplane ] );
			}

			// one plane rejects: whole status is trivial reject
			if ( poly == NULL )
				return poly;
		}
	}

	// return clipped polygon/status
	return poly;
}


// create clipped genobject from unclipped genobject (against volume) ---------
//
GenObject *CLIP_VolumeGenObject( GenObject *clipobj, Plane3 *volume, dword cullmask )
{
	//NOTE:
	// this function returns:
	// - NULL if the object is trivial reject
	// - pointer to a "pseudo-static" GenObject
	//   (the clipped object, may be trivial accept)

	ASSERT( clipobj != NULL );
	ASSERT( volume != NULL );

	if ( clipped_obj == NULL ) {

		clipped_obj = (GenObject *) ALLOCMEM( clipped_obj_size );
		if ( clipped_obj == NULL ) {
			OUTOFMEM( 0 );
		}
	}

	// init first
#ifdef DEBUG
	memset( clipped_obj, 0xcc, clipped_obj_size );
#endif

	// fetch pointers to original vertex and polyon list
	Vertex3	*vtxlist = clipobj->VertexList;
	Poly	*srcpoly = clipobj->PolyList;
	int		numpolys = clipobj->NumPolys;

	// copy over old header (including type fields)
	ASSERT( clipobj->InstanceSize <= CLIPPED_MAX_INSTANCE_SIZE );
	memcpy( clipped_obj, clipobj, clipobj->InstanceSize );

	// set pointers to new lists
	clipped_obj->VertexList   = (Vertex3 *)   ( (char*)clipped_obj + clipped_obj->InstanceSize );
	clipped_obj->X_VertexList = (Vertex3 *)   ( (char*)clipped_obj->VertexList   + SZ_VERTEXLIST );
	clipped_obj->S_VertexList = (SPoint *)    ( (char*)clipped_obj->X_VertexList + SZ_XVERTEXLIST );
	clipped_obj->PolyList     = (Poly *)      ( (char*)clipped_obj->S_VertexList + SZ_SVERTEXLIST );

	// create pointers to new lists
	clp_vtxlist    = clipped_obj->VertexList;
	clp_nextindx   = clipped_obj->NumVerts;
	Poly  *dstpoly = clipped_obj->PolyList;
	dword *dstindx = (dword *) ( (char*)dstpoly + numpolys * sizeof( Poly ) );

	// copy over original vertices
	ASSERT( clipobj->NumVerts <= CLIPPED_MAX_VERTS );
	memcpy( clp_vtxlist, vtxlist, clipobj->NumVerts * sizeof( Vertex3 ) );

	// copy over original polygons (without indexes)
	ASSERT( numpolys <= CLIPPED_MAX_POLYS );
	memcpy( dstpoly, srcpoly, numpolys * sizeof( Poly ) );

	// clip all polygons
	int polyemitted = FALSE;
	for ( int pid = numpolys; pid > 0; pid--, dstpoly++ ) {

		Poly *clp = NULL;

		if ( dstpoly->Flags & POLYFLAG_CORNERCOLORS ) {

			// corner colors contained in polygon
			clp = CLIP_VolumePolyColors( dstpoly, volume, cullmask );

		} else if ( ( dstpoly->Flags & POLYFLAG_WEDGEINDEXES ) &&
					( clipobj->WedgeLighted != NULL ) ) {

			// use wedge indexes to fetch colors
			clp_wcolors = (dword *)clipobj->WedgeLighted;
			clp = CLIP_VolumePolyColors( dstpoly, volume, cullmask );

		} else {

			// only geometry
			dstpoly->Flags = POLYFLAG_DEFAULT;
			clp = CLIP_VolumePoly( dstpoly, volume, cullmask );
		}

		// check clip result
		if ( clp != NULL ) {

			if ( clp != dstpoly ) {

				// copy vertex indexlist
				size_t vindxnum = clp->NumVerts;
				if ( clp->Flags & POLYFLAG_CORNERCOLORS )
					vindxnum += clp->NumVerts;
				if ( clp->Flags & POLYFLAG_WEDGEINDEXES )
					vindxnum += clp->NumVerts;
				memcpy( dstindx, clp->VertIndxs, vindxnum * sizeof( dword ) );

				dstpoly->NumVerts  = clp->NumVerts;
				dstpoly->VertIndxs = dstindx;
				dstpoly->Flags     = clp->Flags;
				dstindx += vindxnum;
			}
			polyemitted = TRUE;

		} else {

			//NOTE:
			// the original vertex index-list stays
			// intact. so if someone wants to retrieve
			// vertices for, say, plane distance
			// computation, this is still possible,
			// although the polygon is clipped entirely.

			// make sure polygon gets rejected
			dstpoly->NumVerts = 0;
		}
	}

	// set correct number of vertices for entire object
	int numnewverts = clp_nextindx - clipped_obj->NumVerts;
	ASSERT( numnewverts >= 0 );
	clipped_obj->NumVerts     += numnewverts;
	clipped_obj->NumPolyVerts += numnewverts;

	return polyemitted ? clipped_obj : NULL;
}


// create clipped genobject from unclipped genobject (against single plane) ---
//
GenObject *CLIP_PlaneGenObject( GenObject *clipobj, Plane3 *plane )
{
	//NOTE:
	// this function returns:
	// - NULL if the object is trivial reject
	// - clipobj if the object is trivial accept
	// - pointer to a "pseudo-static" GenObject (the clipped object)

	ASSERT( clipobj != NULL );
	ASSERT( plane != NULL );

	//TODO:

	return NULL;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( UTL_CLPO )
{
	// limit is 31 due to bit-twiddling!!
	ASSERT( MAX_CLIPPOLY_VERTICES < 32 );

	// fill trivial accept table for all vertex counts
	dword outcode = 0x02;
	for ( int code = 0; code < MAX_CLIPPOLY_VERTICES; code++ ) {

		outcode_acceptall[ code ] = outcode;
		outcode = ( outcode << 1 ) | 0x02;
	}
}



