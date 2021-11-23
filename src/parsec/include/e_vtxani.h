/*
 * PARSEC HEADER: e_vtxani.h
 */

#ifndef _E_VTXANI_H_
#define _E_VTXANI_H_

struct vtxAnimBaseInfo {
	dword      mNumVtxAnims;
	dword      mNumVerts;
	dword      mNumPolyVerts;
	dword      mNumNormals;
	Vertex3*   mVertexList;
	dword      mNumPolys;
	Poly*      mPolyList;
	dword      mNumFaces;
	Face*      mFaceList;
	dword*     mSortedPolyList;
	dword      mNumWedges;
	dword*     mWedgeVertIndxs;
	Vector3*   mWedgeNormals;
	colrgba_s* mWedgeColors;
	TexCoord2* mWedgeTexCoords;
	colrgba_s* mWedgeLighted;
	colrgba_s* mWedgeSpecular;
	colrgba_s* mWedgeFogged;
};

// external functions

int				VtxAnimMakeCurrent( GenObject *gobj, dword animid );
VtxAnimState*	VtxAnimCreateFromFaceList( GenObject *gobj, dword lod, int numfaces, dword *faceids );


#endif // _E_VTXANI_H_


