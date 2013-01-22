/*
 * PARSEC - Object Class Management
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:44 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
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
#include "od_class.h"

// global externals
#include "globals.h"
#ifdef PARSEC_SERVER
	#include "e_world_trans.h"
#endif // PARSEC_SERVER

// local module header
#include "obj_clas.h"

// proprietary module headers
#include "obj_cust.h"
#include "obj_type.h"


// flags
//#define ALLOW_ONLY_KNOWN_CLASSES



// fetch object class via name ------------------------------------------------
//
GenObject *OBJ_FetchObjectClass( const char *classname )
{
	ASSERT( classname != NULL );
	ASSERT( NumObjClasses == NumLoadedObjects );

	// scan entire table of object classes
	int classid = 0;
	for ( classid = 0; classid < NumLoadedObjects; classid++ )
		if ( stricmp( ObjectInfo[ classid ].name, classname ) == 0 )
			break;

	if ( classid < NumObjClasses ) {

		ASSERT( ObjClasses[ classid ] != NULL );
		ASSERT( ObjClasses[ classid ]->ObjectClass == (dword)classid );
		return ObjClasses[ classid ];

	} else {

		return NULL;
	}
}


// fetch object class id via name ---------------------------------------------
//
dword OBJ_FetchObjectClassId( const char *classname )
{
	ASSERT( classname != NULL );
	ASSERT( NumObjClasses == NumLoadedObjects );

	// scan entire table of object classes
	for ( int classid = 0; classid < NumLoadedObjects; classid++ )
		if ( stricmp( ObjectInfo[ classid ].name, classname ) == 0 )
			return classid;

	// no object class of this name found
	return CLASS_ID_INVALID;
}


// fetch object class via name or id if already acquired ----------------------
//
GenObject *OBJ_ReacquireObjectClass( dword *classid, const char *classname )
{
	ASSERT( classid != NULL );
	ASSERT( classname != NULL );

	GenObject *classpo = NULL;
	dword tmpclassid   = *classid;

	if ( tmpclassid == CLASS_ID_INVALID ) {
		tmpclassid = OBJ_FetchObjectClassId( classname );
	}
	if ( tmpclassid != CLASS_ID_INVALID ) {
		ASSERT( tmpclassid < (dword)NumObjClasses );
		classpo = ObjClasses[ tmpclassid ];
	}

	*classid = tmpclassid;
	return classpo;
}


// init default values for object class ---------------------------------------
//
PRIVATE
void InitDefaultClassFields( GenObject *classpo, dword classid )
{
	ASSERT( classpo != NULL );
	ASSERT( classid < MAX_DISTINCT_OBJCLASSES );
	ASSERT( classid <= (dword)NumObjClasses );

	//NOTE:
	// ( classid == NumObjClasses ) is allowed since
	// this will occur when a new class is being added.
	// (NumObjClasses will only be increased afterwards.)

	switch ( classid ) {

		case SHIP_CLASS_1:
			break;

		case SHIP_CLASS_2:
			break;

		case SHIP_CLASS_3:
			break;

		case LASER0_CLASS_1:
			break;

		case LASER0_CLASS_2:
			break;

		case LASER1_CLASS_1:
			break;

		case LASER2_CLASS_1:
			break;

		case DUMB_CLASS_1:
			break;

		case GUIDE_CLASS_1:
			break;

		case SWARM_CLASS_1:
			break;

		case ENERGY_EXTRA_CLASS:
			break;

		case DUMB_PACK_CLASS:
			break;

		case GUIDE_PACK_CLASS:
			break;

		case HELIX_DEVICE_CLASS:
			break;

		case LIGHTNING_DEVICE_CLASS:
			break;

		case MINE_PACK_CLASS:
			break;

		case MINE_CLASS_1:
			break;

		case REPAIR_EXTRA_CLASS:
			break;

		case AFTERBURNER_DEVICE_CLASS:
			break;

		case SWARM_PACK_CLASS:
			break;

		case INVISIBILITY_CLASS:
			break;

		case INVULNERABILITY_CLASS:
			break;

		case PHOTON_DEVICE_CLASS:
			break;

		case DECOY_DEVICE_CLASS:
			break;

		case LASERUPGRADE1_CLASS:
			break;

		case LASERUPGRADE2_CLASS:
			break;

#ifdef ALLOW_ONLY_KNOWN_CLASSES

		default:
			PERROR( "unknown object class id: %d.", classid );
#endif

	}
}


// init class member variables to default values ------------------------------
//
void OBJ_InitClass( dword classid )
{
	ASSERT( classid < MAX_DISTINCT_OBJCLASSES );
	ASSERT( classid <= (dword)NumObjClasses );

	GenObject *classpo = ObjClasses[ classid ];

	//NOTE:
	// ( classid == NumObjClasses ) is allowed since
	// this will occur when a new class is being added.
	// (NumObjClasses will only be increased afterwards.)

	//NOTE:
	// distinction between "object classes" and "object types"
	// -------------------------------------------------------
	// different _objclasses_ originate in different object data files;
	// i.e. the list of objects that should be loaded is parsed and each
	// entry is read and inserted into the array containing pointers to
	// all _objclasses_. Thus each object loaded gets a sequentially
	// (in the order encountered in the control file) assigned number,
	// that is, its *objclass_number*. (these numbers are also contained
	// in file "od_class.h" in order to be used by the object control
	// code. If the order of entries in the control file doesn't
	// correspond to this header file then faulty behavior will occur.
	//
	// different objclass does not necessarily mean that objects *behave*
	// differently in the game as it does only mean that geometry and
	// shading data are different. therefore another concept manages
	// *behavior* of objects:
	//
	// an _objtype_ describes how an object should be handled by the
	// engine and also what data structure is used to manage the object
	// in memory. e.g. all objects of the same type are managed by the
	// same data structure and handled equally by the engine.
	//
	// i.e. objectclasses Laser#1..Laser#5 could all be the same type
	// of laser (concerning their behavior) but look differently.
	// The game *always* instantiates _objectclasses_ (as the look of
	// the object has to be determined), their type is determined
	// automatically. (as it is contained in the objectclass structure.)

	if ( ( classpo->ObjectType & TYPENUMBERMASK ) < NUM_DISTINCT_OBJTYPES ) {

		// init default values for object *type*
		OBJ_InitDefaultTypeFields( classpo );

		// init default values for object *class*
		InitDefaultClassFields( classpo, classid );

	} else {

		// init type and class fields for custom type
		OBJ_InitCustomType( classpo );
	}

	// ensure correct clipping when rendering
	// directly from class template
	classpo->CullMask = 0x3f;
}



