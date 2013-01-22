/*
 * PARSEC HEADER: obj_vani.h
 */

#ifndef _OBJ_VANI_H_
#define _OBJ_VANI_H_


// function pointer type for vtxanim registration
typedef void (*register_vtxanim_fpt)( GenObject *obj );


// external functions

int		OBJ_RegisterVtxAnimation( const char *classname, register_vtxanim_fpt regfunc );


#endif // _OBJ_VANI_H_


