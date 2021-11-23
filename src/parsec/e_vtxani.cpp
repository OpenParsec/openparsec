/*
 * PARSEC - Vertex Animations
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:34 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   2000
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

// local module header
#include "e_vtxani.h"

// proprietary module headers
#include "obj_ctrl.h"



// base index correction macro ------------------------------------------------
//
#define CORRECT_BASE(w,b)	( ( (w) != NULL ) ? ( &(w)[ b ] ) : NULL )


// make the specified vertex anim current in the supplied object --------------
//
int VtxAnimMakeCurrent( GenObject *gobj, dword animid )
{
	ASSERT( gobj != NULL );
	ASSERT( animid <= gobj->NumVtxAnims );
    //
    //  This get on pointer out of the array stored in gobj.
	VtxAnimState *anim = &gobj->VtxAnimStates[ animid ];

	// vertex anim beyond contains the base object info array
    vtxAnimBaseInfo *baseinfo = (vtxAnimBaseInfo *) &gobj->VtxAnimStates[ gobj->NumVtxAnims ];

	// store id of active subobject
	baseinfo->mNumVtxAnims = animid;

	// switch to base object
	gobj->NumVerts		 = baseinfo->mNumVerts;
	gobj->NumPolyVerts	 = baseinfo->mNumPolyVerts;
	gobj->NumNormals	 = baseinfo->mNumNormals;
	gobj->VertexList	 = baseinfo->mVertexList;
	gobj->NumPolys		 = baseinfo->mNumPolys;
	gobj->PolyList		 = baseinfo->mPolyList;
	gobj->NumFaces		 = baseinfo->mNumFaces;
	gobj->FaceList		 = baseinfo->mFaceList;
	gobj->SortedPolyList = baseinfo->mSortedPolyList;
	gobj->NumWedges		 = baseinfo->mNumWedges;
	gobj->WedgeVertIndxs = baseinfo->mWedgeVertIndxs;
	gobj->WedgeNormals	 = baseinfo->mWedgeNormals;
	gobj->WedgeColors	 = baseinfo->mWedgeColors;
	gobj->WedgeTexCoords = baseinfo->mWedgeTexCoords;
	gobj->WedgeLighted	 = baseinfo->mWedgeLighted;
	gobj->WedgeSpecular	 = baseinfo->mWedgeSpecular;
	gobj->WedgeFogged	 = baseinfo->mWedgeFogged;

	// switch to subobject
	if ( animid < gobj->NumVtxAnims ) {

		int vertexbase = anim->VertexBase;
		int polybase   = anim->PolyBase;
		int facebase   = anim->FaceBase;
		int wedgebase  = anim->WedgeBase;

		// switch geometry to subobject of vertex anim
		gobj->NumVerts		 = anim->NumVerts;
		gobj->NumPolyVerts	 = anim->NumPolyVerts;
		gobj->NumNormals	 = anim->NumNormals;
		gobj->VertexList	 = &gobj->VertexList[ vertexbase ];
		gobj->NumPolys		 = anim->NumPolys;
		gobj->PolyList		 = &gobj->PolyList[ polybase ];
		gobj->NumFaces		 = anim->NumFaces;
		gobj->FaceList		 = &gobj->FaceList[ facebase ];
		gobj->SortedPolyList = CORRECT_BASE( gobj->SortedPolyList, polybase );
		gobj->NumWedges		 = anim->NumWedges;
		gobj->WedgeVertIndxs = CORRECT_BASE( gobj->WedgeVertIndxs, wedgebase );
		gobj->WedgeNormals	 = CORRECT_BASE( gobj->WedgeNormals,   wedgebase );
		gobj->WedgeColors	 = CORRECT_BASE( gobj->WedgeColors,    wedgebase );
		gobj->WedgeTexCoords = CORRECT_BASE( gobj->WedgeTexCoords, wedgebase );
		gobj->WedgeLighted	 = CORRECT_BASE( gobj->WedgeLighted,   wedgebase );
		gobj->WedgeSpecular	 = CORRECT_BASE( gobj->WedgeSpecular,  wedgebase );
		gobj->WedgeFogged	 = CORRECT_BASE( gobj->WedgeFogged,    wedgebase );
	}

	return TRUE;
}


// create a new vertex anim (append) for the supplied set of faces ------------
//
VtxAnimState *VtxAnimCreateFromFaceList( GenObject *gobj, dword lod, int numfaces, dword *faceids )
{
	ASSERT( gobj != NULL );
	ASSERT( faceids != NULL );

	// offset of vertex anim state array (needed for lod)
	int vtxanimoffset = 0;

	// make sure destination lod is current
	if ( gobj->NumLodObjects > 0 ) {

		ASSERT( gobj->LodObjects != NULL );

		// check for invalid lod
		if ( lod >= gobj->NumLodObjects ) {
			return NULL;
		}

		// switch lod if necessary
		if ( lod != gobj->CurrentLod ) {
			OBJ_SwitchObjectLod( gobj, lod );
		}

		// we have an equal-sized vertex anim array for
		// each lod present. these are totally independent.
		vtxanimoffset = lod * ( gobj->NumVtxAnims + 1 );

	} else {

		ASSERT( gobj->CurrentLod == 0 );
		ASSERT( gobj->LodObjects == NULL );

		// check for invalid lod
		if ( lod != 0 ) {
			return NULL;
		}
	}

	// check for free state
	if ( gobj->ActiveVtxAnims >= gobj->NumVtxAnims ) {
		return NULL;
	}

	// create in next free state
	int animid = gobj->ActiveVtxAnims;
	VtxAnimState *anim = &gobj->VtxAnimStates[ animid + vtxanimoffset ];

	Face	*facelist	= gobj->FaceList;
	Poly	*polylist	= gobj->PolyList;
	Vertex3	*vertexlist	= gobj->VertexList;

	// alloc mem for sets
	size_t setmemsize = 0;
	setmemsize += gobj->NumFaces;
	setmemsize += gobj->NumPolys;
	setmemsize += gobj->NumVerts;
	setmemsize += gobj->NumWedges;
	byte *setmemblock = (byte *) ALLOCMEM( setmemsize );
	if ( setmemblock == NULL )
		OUTOFMEM( 0 );
	memset( setmemblock, 0, setmemsize );

	byte *faceset	= &setmemblock[ 0 ];
	byte *polyset	= &faceset[ gobj->NumFaces ];
	byte *vertexset	= &polyset[ gobj->NumPolys ];
	byte *wedgeset	= &vertexset[ gobj->NumVerts ];

	// construct in/out map for faces and normals
	int findx = 0;
	for ( findx = 0; findx < numfaces; findx++ ) {
		ASSERT( faceids[ findx ] < gobj->NumFaces );
		faceset[ faceids[ findx ] ] = 1;
		vertexset[ facelist[ faceids[ findx ] ].FaceNormalIndx ] = 1;
	}

	// construct in/out map for polys and vertices
	unsigned int polyid = 0;
	for ( polyid = 0; polyid < gobj->NumPolys; polyid++ ) {
		polyset[ polyid ] = faceset[ polylist[ polyid ].FaceIndx ];
		if ( polyset[ polyid ] ) {
			for ( unsigned int vindx = 0; vindx < polylist[ polyid ].NumVerts; vindx++ ) {
				vertexset[ polylist[ polyid ].VertIndxs[ vindx ] ] = 1;
			}
		}
	}

	// construct in/out map for wedges
	unsigned int wedgeid = 0;
	for ( wedgeid = 0; wedgeid < gobj->NumWedges; wedgeid++ ) {
		ASSERT( gobj->WedgeVertIndxs != NULL );
		wedgeset[ wedgeid ] = vertexset[ gobj->WedgeVertIndxs[ wedgeid ] ];
	}

	// count normals
	int numnormals = 0;
	unsigned int setindx = 0;
	for (  setindx = 0; setindx < gobj->NumNormals; setindx++ ) {
		numnormals += vertexset[ setindx ];
	}

	// count vertices
	int numpolyverts = 0;
	for ( ; setindx < gobj->NumVerts; setindx++ ) {
		numpolyverts += vertexset[ setindx ];
	}

	// count polys
	int numpolys = 0;
	for ( setindx = 0; setindx < gobj->NumPolys; setindx++ ) {
		numpolys += polyset[ setindx ];
	}

	// count wedges
	int numwedges = 0;
	for ( setindx = 0; setindx < gobj->NumWedges; setindx++ ) {
		numwedges += wedgeset[ setindx ];
	}

	// determine base indexes
	dword facebase   = gobj->NumFaces - numfaces;
	dword polybase   = gobj->NumPolys - numpolys;
	dword vertexbase = gobj->NumVerts - numnormals - numpolyverts;
	dword wedgebase  = gobj->NumWedges - numwedges;

	// alloc mem for maps
	size_t mapmemsize = 0;
	mapmemsize += gobj->NumFaces;
	mapmemsize += gobj->NumPolys;
	mapmemsize += gobj->NumVerts;
	mapmemsize += gobj->NumWedges;
	mapmemsize += gobj->NumPolys;
	mapmemsize *= sizeof( dword );
	dword *mapmemblock = (dword *) ALLOCMEM( mapmemsize );
	if ( mapmemblock == NULL )
		OUTOFMEM( 0 );
	memset( mapmemblock, 0, mapmemsize );

	dword *facemap   = &mapmemblock[ 0 ];
	dword *polymap   = &facemap[ gobj->NumFaces ];
	dword *vertexmap = &polymap[ gobj->NumPolys ];
	dword *wedgemap  = &vertexmap[ gobj->NumVerts ];
	dword *stpolymap = &wedgemap[ gobj->NumWedges ];

	// construct id map for faces
	int insetindx  = facebase;
	int outsetindx = 0;
	unsigned int faceid = 0;
	for ( faceid = 0; faceid < gobj->NumFaces; faceid++ ) {
		facemap[ faceid ] = faceset[ faceid ] ? insetindx++ : outsetindx++;
	}

	// construct id map for polys
	insetindx  = polybase;
	outsetindx = 0;
	for ( polyid = 0; polyid < gobj->NumPolys; polyid++ ) {
		polymap[ polyid ] = polyset[ polyid ] ? insetindx++ : outsetindx++;
	}

	// construct id map for vertices
	insetindx  = vertexbase;
	outsetindx = 0;
	unsigned int vtxid = 0;
	for ( vtxid = 0; vtxid < gobj->NumVerts; vtxid++ ) {
		vertexmap[ vtxid ] = vertexset[ vtxid ] ? insetindx++ : outsetindx++;
	}

	// construct id map for wedges
	insetindx  = wedgebase;
	outsetindx = 0;
	for ( wedgeid = 0; wedgeid < gobj->NumWedges; wedgeid++ ) {
		wedgemap[ wedgeid ] = wedgeset[ wedgeid ] ? insetindx++ : outsetindx++;
	}

	// construct index map for sorted poly list
	if ( gobj->SortedPolyList != NULL ) {
		insetindx  = polybase;
		outsetindx = 0;
		for ( polyid = 0; polyid < gobj->NumPolys; polyid++ ) {
			stpolymap[ polyid ] = polyset[ gobj->SortedPolyList[ polyid ] ] ?
				insetindx++ : outsetindx++;
		}
	}

	// apply map to normal ids embedded in faces
	for ( faceid = 0; faceid < gobj->NumFaces; faceid++ ) {
		int normalid = facelist[ faceid ].FaceNormalIndx;
		facelist[ faceid ].FaceNormalIndx = vertexmap[ normalid ];
		if ( vertexset[ normalid ] ) {
			facelist[ faceid ].FaceNormalIndx -= vertexbase;
		}
	}

	// apply map to face and vertex ids embedded in polys
	for ( polyid = 0; polyid < gobj->NumPolys; polyid++ ) {
		int faceid = polylist[ polyid ].FaceIndx;
		polylist[ polyid ].FaceIndx = facemap[ faceid ];
		if ( faceset[ faceid ] ) {
			polylist[ polyid ].FaceIndx -= facebase;
		}
		dword flags = polylist[ polyid ].Flags;
		int wbase = ( flags & POLYFLAG_CORNERCOLORS ) ? 2 : 1;
		dword *vertindxs  = polylist[ polyid ].VertIndxs;
		dword *wedgeindxs = &vertindxs[ polylist[ polyid ].NumVerts * wbase ];
		for ( unsigned int vindx = 0; vindx < polylist[ polyid ].NumVerts; vindx++ ) {
			int vtxid = vertindxs[ vindx ];
			vertindxs[ vindx ] = vertexmap[ vtxid ];
			if ( vertexset[ vtxid ] ) {
				vertindxs[ vindx ] -= vertexbase;
			}
			if ( flags & POLYFLAG_WEDGEINDEXES ) {
				int vtxwedge = wedgeindxs[ vindx ];
				wedgeindxs[ vindx ] = wedgemap[ vtxwedge ];
				if ( wedgeset[ vtxwedge ] ) {
					wedgeindxs[ vindx ] -= wedgebase;
				}
			}
		}
	}

	// apply map to wedge vertex indexes
	for ( wedgeid = 0; wedgeid < gobj->NumWedges; wedgeid++ ) {
		int vtxid = gobj->WedgeVertIndxs[ wedgeid ];
		gobj->WedgeVertIndxs[ wedgeid ] = vertexmap[ vtxid ];
		if ( vertexset[ vtxid ] ) {
			gobj->WedgeVertIndxs[ wedgeid ] -= vertexbase;
		}
	}

	// in-place reorder facelist according to map
	for ( faceid = 0; faceid < gobj->NumFaces; ) {

		// proceed if current pos correct
		if ( facemap[ faceid ] == faceid ) {
			faceid++;
			continue;
		}

		// swap face data
		Face temp = facelist[ facemap[ faceid ] ];
		facelist[ facemap[ faceid ] ] = facelist[ faceid ];
		facelist[ faceid ] = temp;

		// swap map ids
		dword dstid = facemap[ faceid ];
		facemap[ faceid ] = facemap[ dstid ];
		facemap[ dstid ] = dstid;
	}

	if ( gobj->SortedPolyList != NULL ) {

		// in-place reorder sorted poly id list
		for ( polyid = 0; polyid < gobj->NumPolys; ) {

			// proceed if current pos correct
			if ( stpolymap[ polyid ] == polyid ) {
				polyid++;
				continue;
			}

			// swap poly data
			dword temp = gobj->SortedPolyList[ stpolymap[ polyid ] ];
			gobj->SortedPolyList[ stpolymap[ polyid ] ] = gobj->SortedPolyList[ polyid ];
			gobj->SortedPolyList[ polyid ] = temp;

			// swap map ids
			dword dstid = stpolymap[ polyid ];
			stpolymap[ polyid ] = stpolymap[ dstid ];
			stpolymap[ dstid ] = dstid;
		}

		// apply map to sorted poly id list
		for ( polyid = 0; polyid < gobj->NumPolys; polyid++ ) {
			int sortid = gobj->SortedPolyList[ polyid ];
			gobj->SortedPolyList[ polyid ] = polymap[ sortid ];
			if ( polyset[ sortid ] ) {
				gobj->SortedPolyList[ polyid ] -= polybase;
			}
		}
	}

	// in-place reorder polylist according to map
	for ( polyid = 0; polyid < gobj->NumPolys; ) {

		// proceed if current pos correct
		if ( polymap[ polyid ] == polyid ) {
			polyid++;
			continue;
		}

		// swap poly data
		Poly temp = polylist[ polymap[ polyid ] ];
		polylist[ polymap[ polyid ] ] = polylist[ polyid ];
		polylist[ polyid ] = temp;

		// swap map ids
		dword dstid = polymap[ polyid ];
		polymap[ polyid ] = polymap[ dstid ];
		polymap[ dstid ] = dstid;
	}

	// in-place reorder vertexlist according to map
	for ( vtxid = 0; vtxid < gobj->NumVerts; ) {

		// proceed if current pos correct
		if ( vertexmap[ vtxid ] == vtxid ) {
			vtxid++;
			continue;
		}

		// swap vertex data
		Vertex3 temp = vertexlist[ vertexmap[ vtxid ] ];
		vertexlist[ vertexmap[ vtxid ] ] = vertexlist[ vtxid ];
		vertexlist[ vtxid ] = temp;

		// swap map ids
		dword dstid = vertexmap[ vtxid ];
		vertexmap[ vtxid ] = vertexmap[ dstid ];
		vertexmap[ dstid ] = dstid;
	}

	// in-place reorder wedgelists according to map
	for ( wedgeid = 0; wedgeid < gobj->NumWedges; ) {

		// proceed if current pos correct
		if ( wedgemap[ wedgeid ] == wedgeid ) {
			wedgeid++;
			continue;
		}

		// swap wedge data
		dword tempindx = gobj->WedgeVertIndxs[ wedgemap[ wedgeid ] ];
		gobj->WedgeVertIndxs[ wedgemap[ wedgeid ] ] = gobj->WedgeVertIndxs[ wedgeid ];
		gobj->WedgeVertIndxs[ wedgeid ] = tempindx;

		if ( gobj->WedgeNormals != NULL ) {
			Vector3 tempnormal = gobj->WedgeNormals[ wedgemap[ wedgeid ] ];
			gobj->WedgeNormals[ wedgemap[ wedgeid ] ] = gobj->WedgeNormals[ wedgeid ];
			gobj->WedgeNormals[ wedgeid ] = tempnormal;

		}

		if ( gobj->WedgeColors != NULL ) {
			colrgba_s tempcolor = gobj->WedgeColors[ wedgemap[ wedgeid ] ];
			gobj->WedgeColors[ wedgemap[ wedgeid ] ] = gobj->WedgeColors[ wedgeid ];
			gobj->WedgeColors[ wedgeid ] = tempcolor;

		}

		if ( gobj->WedgeTexCoords != NULL ) {
			TexCoord2 temptexcoord = gobj->WedgeTexCoords[ wedgemap[ wedgeid ] ];
			gobj->WedgeTexCoords[ wedgemap[ wedgeid ] ] = gobj->WedgeTexCoords[ wedgeid ];
			gobj->WedgeTexCoords[ wedgeid ] = temptexcoord;

		}

		// swap map ids
		dword dstid = wedgemap[ wedgeid ];
		wedgemap[ wedgeid ] = wedgemap[ dstid ];
		wedgemap[ dstid ] = dstid;
	}

	// free maps and sets
	FREEMEM( mapmemblock );
	FREEMEM( setmemblock );

	// set anim info
	anim->NumVerts		= numnormals + numpolyverts;
	anim->NumPolyVerts	= numpolyverts;
	anim->NumNormals	= numnormals;
	anim->VertexBase	= vertexbase;
	anim->NumPolys		= numpolys;
	anim->PolyBase		= polybase;
	anim->NumFaces		= numfaces;
	anim->FaceBase		= facebase;
	anim->NumWedges		= numwedges;
	anim->WedgeBase		= wedgebase;

	// shrink base object
	gobj->NumVerts		-= anim->NumVerts;
	gobj->NumPolyVerts	-= anim->NumPolyVerts;
	gobj->NumNormals	-= anim->NumNormals;
	gobj->NumPolys		-= anim->NumPolys;
	gobj->NumFaces		-= anim->NumFaces;
	gobj->NumWedges		-= anim->NumWedges;

	// make anim active
	gobj->ActiveVtxAnims++;

	// update lod info if necessary
	if ( gobj->NumLodObjects > 0 ) {

		GenLodObject *lodobj = gobj->LodObjects[ lod ].LodObject;
		ASSERT( lodobj != NULL );

		lodobj->NumVerts		= gobj->NumVerts;
		lodobj->NumPolyVerts	= gobj->NumPolyVerts;
		lodobj->NumNormals		= gobj->NumNormals;
		lodobj->NumPolys		= gobj->NumPolys;
		lodobj->NumFaces		= gobj->NumFaces;
		lodobj->NumWedges		= gobj->NumWedges;
		lodobj->ActiveVtxAnims	= gobj->ActiveVtxAnims;
	}

	// vertex anim beyond is treated as base object info array
	int baseinfoid  = gobj->NumVtxAnims + vtxanimoffset;
    vtxAnimBaseInfo*  baseinfo = (vtxAnimBaseInfo *) &gobj->VtxAnimStates[ baseinfoid ];

	// store id of base object as active subobject
    baseinfo->mNumVtxAnims = gobj->NumVtxAnims;

	// save base object info
    baseinfo->mNumVerts       = gobj->NumVerts;
    baseinfo->mNumPolyVerts   = gobj->NumPolyVerts;
    baseinfo->mNumNormals     = gobj->NumNormals;
    baseinfo->mVertexList     = gobj->VertexList;
    baseinfo->mNumPolys       = gobj->NumPolys;
    baseinfo->mPolyList       = gobj->PolyList;
    baseinfo->mNumFaces       = gobj->NumFaces;
    baseinfo->mFaceList       = gobj->FaceList;
    baseinfo->mSortedPolyList = gobj->SortedPolyList;
    baseinfo->mNumWedges      = gobj->NumWedges;
    baseinfo->mWedgeVertIndxs = gobj->WedgeVertIndxs;
    baseinfo->mWedgeNormals   = gobj->WedgeNormals;
    baseinfo->mWedgeColors    = gobj->WedgeColors;
    baseinfo->mWedgeTexCoords = gobj->WedgeTexCoords;
    baseinfo->mWedgeLighted   = gobj->WedgeLighted;
    baseinfo->mWedgeSpecular  = gobj->WedgeSpecular;
    baseinfo->mWedgeFogged    = gobj->WedgeFogged;

	return anim;
}



