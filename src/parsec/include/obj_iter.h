/*
 * PARSEC HEADER: obj_iter.h
 */

#ifndef _OBJ_ITER_H_
#define _OBJ_ITER_H_


// function pointer type for iter polygon attachment
typedef void (*attach_iterpolys_fpt)( GenObject *obj, int inorder );


// external functions

void	OBJ_AttachIterPolygons( GenObject *obj, int inorder );
void 	OBJ_ResetRegisteredClassIterPolys( dword objclass );
int 	OBJ_RegisterClassIterPolys( dword objclass, int apexid, int apexvtx, int apexcomp, int numprimvtxs, int numvtxs, float *vtxlist );


#endif // _OBJ_ITER_H_


