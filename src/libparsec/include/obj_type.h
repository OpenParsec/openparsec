/*
 * PARSEC HEADER: obj_type.h
 */

#ifndef _OBJ_TYPE_H_
#define _OBJ_TYPE_H_


// external variables

extern const char* objtype_name[];
extern dword	objtype_id[];


// external functions

dword	OBJ_FetchTypeIdFromName( const char *typestr );
size_t	OBJ_FetchTypeSize( dword objtypeid );
void	OBJ_InitDefaultTypeFields( GenObject *classpo );


#endif // _OBJ_TYPE_H_


