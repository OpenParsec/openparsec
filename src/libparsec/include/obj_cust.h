/*
 * PARSEC HEADER: obj_cust.h
 */

#ifndef _OBJ_CUST_H_
#define _OBJ_CUST_H_


// maximum number of custom (additional) types --------------------------------
//
#define MAX_NUM_CUSTOM_TYPES	64


// custom type flags ----------------------------------------------------------
//
#define CUSTOM_TYPE_DEFAULT			0x0000
#define CUSTOM_TYPE_SUMMONABLE		0x0001
#define CUSTOM_TYPE_NOT_PERSISTANT  0x0002


// custom object notify events ------------------------------------------------
//
enum {

	CUSTOM_NOTIFY_GENOBJECT_DELETE = 0	// GenObject will be deleted
};


// structure for registration/maintenance of custom types ---------------------
//
struct custom_type_info_s {

	const char*		type_name;
	dword			type_id;
	size_t			type_size;
	CustomObject*	type_template;
	int				type_flags;
	
	void			(*callback_init)( CustomObject *base );

	void			(*callback_instant)( CustomObject *base );
	void			(*callback_destroy)( CustomObject *base );

	int				(*callback_animate)( CustomObject *base );
	int				(*callback_collide)( CustomObject *base );

	void			(*callback_notify)( CustomObject *base, GenObject *genobj, int event );
	int				(*callback_persist)( CustomObject* base, int ToStream, void* relist );

	dword			_mksiz64[ 4 ];
};


// external functions

dword			OBJ_FetchCustomTypeId( const char *name );
const char*		OBJ_FetchCustomTypeName( dword objtypeid );
size_t			OBJ_FetchCustomTypeSize( dword objtypeid );
CustomObject*	OBJ_FetchCustomTypeTemplate( dword objtypeid );
int				OBJ_InitFromCustomTypeTemplate( CustomObject *obj, CustomObject *templ );
void			OBJ_InitCustomType( GenObject *classpo );
dword			OBJ_RegisterCustomType( custom_type_info_s *info );
int				OBJ_GetCustomTypeFlags( dword objtypeid );
void			OBJ_RegisterNotifyCustomObject( GenObject *genobj, CustomObject *customobj );
int				OBJ_UnregisterNotifyCustomObject( GenObject *genobj, CustomObject *customobj );
void			OBJ_NotifyCustomObjectsList( GenObject *genobj, int event );


// external variables

extern int					num_custom_types;
extern custom_type_info_s	custom_type_info[];


#endif // _OBJ_CUST_H_


