/*
 * PARSEC HEADER: e_vtxani.h
 */

#ifndef _E_VTXANI_H_
#define _E_VTXANI_H_


// external functions

int				VtxAnimMakeCurrent( GenObject *gobj, dword animid );
VtxAnimState*	VtxAnimCreateFromFaceList( GenObject *gobj, dword lod, int numfaces, dword *faceids );


#endif // _E_VTXANI_H_


