/*
 * PARSEC HEADER: obj_clas.h
 */

#ifndef _OBJ_CLAS_H_
#define _OBJ_CLAS_H_


// external functions

GenObject*	OBJ_FetchObjectClass( const char *classname );
dword		OBJ_FetchObjectClassId( const char *classname );
GenObject*	OBJ_ReacquireObjectClass( dword *classid, const char *classname );

void		OBJ_InitClass( dword classid );


#endif // _OBJ_CLAS_H_


