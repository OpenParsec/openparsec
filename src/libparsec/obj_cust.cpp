/*
 * PARSEC - Custom Object Types
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:44 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999
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
#ifdef PARSEC_SERVER
	#include "e_world_trans.h"
#endif // PARSEC_SERVER

// local module header
#include "obj_cust.h"

// proprietary module headers
//#include "obj_ctrl.h"



// string constants -----------------------------------------------------------
//
static char custom_type_invalid[]	= "invalid custom object type.";


// custom type table ----------------------------------------------------------
//
int					num_custom_types = 0;
custom_type_info_s	custom_type_info[ MAX_NUM_CUSTOM_TYPES ];


// look up custom type via name -----------------------------------------------
//
dword OBJ_FetchCustomTypeId( const char *name )
{
	ASSERT( name != NULL );
	
	// search for type name in table
	for ( int tid = 0; tid < num_custom_types; tid++ )
		if ( strcmp( custom_type_info[ tid ].type_name, name ) == 0 )
			return custom_type_info[ tid ].type_id;
		
		return TYPE_ID_INVALID;
}


// determine type name of custom type -----------------------------------------
//
const char *OBJ_FetchCustomTypeName( dword objtypeid )
{
	if ( !TYPEID_TYPE_CUSTOM( objtypeid ) )
		return NULL;
	
	int tindx = ( objtypeid & TYPENUMBERMASK ) - NUM_DISTINCT_OBJTYPES;
	
	ASSERT( tindx >= 0 );
	ASSERT( tindx < num_custom_types );
	
	if ( custom_type_info[ tindx ].type_id != objtypeid )
		return NULL;
	
	// fetch name from table
	return custom_type_info[ tindx ].type_name;
}


// get the flags for a custom type --------------------------------------------
//
int OBJ_GetCustomTypeFlags( dword objtypeid )
{
	if ( !TYPEID_TYPE_CUSTOM( objtypeid ) )
		return 0;
	
	int tindx = ( objtypeid & TYPENUMBERMASK ) - NUM_DISTINCT_OBJTYPES;
	
	ASSERT( tindx >= 0 );
	ASSERT( tindx < num_custom_types );
	
	if ( custom_type_info[ tindx ].type_id != objtypeid )
		return 0;
	
	// fetch flags from table
	return custom_type_info[ tindx ].type_flags;
}

// determine instance size of custom type -------------------------------------
//
size_t OBJ_FetchCustomTypeSize( dword objtypeid )
{
	ASSERT( TYPEID_TYPE_CUSTOM( objtypeid ) );
	int tindx = ( objtypeid & TYPENUMBERMASK ) - NUM_DISTINCT_OBJTYPES;

	ASSERT( tindx >= 0 );
	ASSERT( tindx < num_custom_types );
	ASSERT( custom_type_info[ tindx ].type_id == objtypeid );

	if ( custom_type_info[ tindx ].type_id != objtypeid ) {

		// point of no return
		PANIC( custom_type_invalid );
	}

	// fetch size from table
	return custom_type_info[ tindx ].type_size;
}


// fetch template (object with default/user-altered values) for custom type ---
//
CustomObject *OBJ_FetchCustomTypeTemplate( dword objtypeid )
{
	ASSERT( TYPEID_TYPE_CUSTOM( objtypeid ) );
	int tindx = ( objtypeid & TYPENUMBERMASK ) - NUM_DISTINCT_OBJTYPES;

	ASSERT( tindx >= 0 );
	ASSERT( tindx < num_custom_types );
	ASSERT( custom_type_info[ tindx ].type_id == objtypeid );

	if ( custom_type_info[ tindx ].type_id != objtypeid )
		return NULL;

	// fetch template from table
	return custom_type_info[ tindx ].type_template;
}


// init custom object from supplied custom type template ----------------------
//
int OBJ_InitFromCustomTypeTemplate( CustomObject *obj, CustomObject *templ )
{
	ASSERT( obj != NULL );

	// copy over template if available
	if ( templ != NULL ) {

		ASSERT( obj->InstanceSize >= sizeof( CustomObject ) );

		// base part must be left untouched
		size_t userareasize = obj->InstanceSize - sizeof( CustomObject );
		if ( userareasize > 0 ) {
			memcpy( (char*)obj + sizeof( CustomObject ),
					(char*)templ + sizeof( CustomObject ),
					userareasize );
		}

		return TRUE;
	}

	// no template, default init should be done
	return FALSE;
}


// init instance (class) of custom object type --------------------------------
//
void OBJ_InitCustomType( GenObject *classpo )
{
	ASSERT( classpo != NULL );
	ASSERT( OBJECT_TYPE_CUSTOM( classpo ) );

	CustomObject *obj = (CustomObject *) classpo;

	int customid = ( classpo->ObjectType & TYPENUMBERMASK ) - NUM_DISTINCT_OBJTYPES;
	if ( ( customid < 0 ) || ( customid >= num_custom_types ) ) {
		ASSERT( 0 );
		return;
	}

	if ( custom_type_info[ customid ].type_id != classpo->ObjectType ) {
		ASSERT( 0 );
		return;
	}

	// init fields of struct CustomObject
	obj->callback_instant	= custom_type_info[ customid ].callback_instant;
	obj->callback_destroy	= custom_type_info[ customid ].callback_destroy;
	obj->callback_animate	= custom_type_info[ customid ].callback_animate;
	obj->callback_collide	= custom_type_info[ customid ].callback_collide;
	obj->callback_notify	= custom_type_info[ customid ].callback_notify;
	obj->callback_persist	= custom_type_info[ customid ].callback_persist;

	// invoke class init callback if available
	if ( custom_type_info[ customid ].callback_init != NULL ) {
		(*custom_type_info[ customid ].callback_init)( obj );
	}
}


// register new custom type, assign next available id -------------------------
//
dword OBJ_RegisterCustomType( custom_type_info_s *info )
{
	ASSERT( info != NULL );

	// guard against maximum number of custom types
	if ( num_custom_types >= MAX_NUM_CUSTOM_TYPES ) {
		ASSERT( 0 );
		info->type_id = TYPE_ID_INVALID;
		return TYPE_ID_INVALID;
	}

	// guard against maximum number of type ids
	if ( num_custom_types + NUM_DISTINCT_OBJTYPES > TYPENUMBERMASK ) {
		ASSERT( 0 );
		info->type_id = TYPE_ID_INVALID;
		return TYPE_ID_INVALID;
	}

	// assign next sequential id
	dword tindx   = num_custom_types;
	info->type_id = ( NUM_DISTINCT_OBJTYPES + tindx ) | CUSTM_LIST_NO |
					( info->type_id & ~( TYPENUMBERMASK | TYPELISTMASK ) );

	// init table entry
	custom_type_info[ tindx ].type_name			= info->type_name;
	custom_type_info[ tindx ].type_id			= info->type_id;
	custom_type_info[ tindx ].type_size			= info->type_size;
	custom_type_info[ tindx ].type_template		= info->type_template;
	custom_type_info[ tindx ].type_flags		= info->type_flags;
	custom_type_info[ tindx ].callback_init		= info->callback_init;
	custom_type_info[ tindx ].callback_instant	= info->callback_instant;
	custom_type_info[ tindx ].callback_destroy	= info->callback_destroy;
	custom_type_info[ tindx ].callback_animate	= info->callback_animate;
	custom_type_info[ tindx ].callback_collide	= info->callback_collide;
	custom_type_info[ tindx ].callback_notify	= info->callback_notify;
	custom_type_info[ tindx ].callback_persist  = info->callback_persist;

	// one more custom type
	num_custom_types++;

	// return id (caller field type_id is now valid)
	return info->type_id;
}


// register custom object with genobject (insert at head of list) -------------
//
void OBJ_RegisterNotifyCustomObject( GenObject *genobj, CustomObject *customobj )
{
	ASSERT( genobj != NULL );
	ASSERT( customobj != NULL );

	//NOTE:
	// if the custom object needs a pointer to the GenObject it has
	// been registered with, it must store this pointer in its own
	// mem area, i.e., in the struct derived from CustomObject.

	// every custom object can only be part of a single list
	ASSERT( customobj->NotifyCustmObjects == NULL );

	// insert at head of list
	customobj->NotifyCustmObjects = genobj->NotifyCustmObjects;
	genobj->NotifyCustmObjects = customobj;
}


// unregister custom object from genobject (delete from list if contained) ----
//
int OBJ_UnregisterNotifyCustomObject( GenObject *genobj, CustomObject *customobj )
{
	ASSERT( genobj != NULL );
	ASSERT( customobj != NULL );

	// scan list for specified custom object
	GenObject *prev = genobj;
	CustomObject *scan = genobj->NotifyCustmObjects;
	for ( ; scan != NULL; scan = scan->NotifyCustmObjects ) {

		// remove from list when found
		if ( scan == customobj ) {
			prev->NotifyCustmObjects = scan->NotifyCustmObjects;
			scan->NotifyCustmObjects = NULL;
			return TRUE;
		}
		prev = scan;
	}

	// custom object was not registered
	return FALSE;
}


// notify custom objects registered with genobject ----------------------------
//
void OBJ_NotifyCustomObjectsList( GenObject *genobj, int event )
{
	ASSERT( genobj != NULL );

	// if the event is delete, the list must be unlinked
	int unlinklist = ( event == CUSTOM_NOTIFY_GENOBJECT_DELETE );

	// walk list of registered custom objects
	GenObject *prev = genobj;
	CustomObject *customobj = genobj->NotifyCustmObjects;
	for ( ; customobj != NULL; customobj = customobj->NotifyCustmObjects ) {

		// call notify callback if it is valid
		if ( customobj->callback_notify != NULL ) {
			(*customobj->callback_notify)( customobj, genobj, event );
		}

		// step to next element, unlink list if desired
		if ( unlinklist ) {
			prev->NotifyCustmObjects = NULL;
			prev = customobj;
		}
	}
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( OBJ_CUST )
{
	//NOTE:
	// register a type that consists only of
	// geometry. this can be used to view
	// objects using the summon command.

	custom_type_info_s info;
	memset( &info, 0, sizeof( info ) );

	info.type_name	= "geometry";
	info.type_size	= sizeof( CustomObject );

	OBJ_RegisterCustomType( &info );
}



