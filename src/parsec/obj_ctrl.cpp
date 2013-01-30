/*
 * PARSEC - Control Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:35 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-1999
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
#include <stddef.h>
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

// subsystem headers
#include "net_defs.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "obj_ctrl.h"

// proprietary module headers
#include "con_aux.h"
#include "con_com.h"
#include "con_main.h"
#include "g_supp.h"
#include "h_supp.h"
#include "obj_cust.h"
#include "obj_expl.h"
#include "obj_part.h"
#include "part_api.h"
#include "g_sfx.h"
#include "g_vapor.h"


// flags
#define ALLOW_OBJLIST_DISABLING
#define CULL_IN_WORLD_SPACE
#define CULL_BOUNDING_SPHERES
#define ALLOW_NEARPLANE_CLIPPING
//#define INSTANTIATE_FACE_STRUCTURES



// to determine whether an object should never be clipped by the nearplane ----
//
#ifdef ALLOW_NEARPLANE_CLIPPING
//	#define PREVENT_OBJECT_NEARPLANE_CLIPPING(o)	( !OBJECT_TYPE_LASER(o) )
	#define PREVENT_OBJECT_NEARPLANE_CLIPPING(o)	( !OBJECT_TYPE_LASER(o) && !AUX_DISABLE_NEARPLANE_TOUCH_CULLING )
//	#define PREVENT_OBJECT_NEARPLANE_CLIPPING(o)	( FALSE )
#else
	#define PREVENT_OBJECT_NEARPLANE_CLIPPING(o)	( TRUE )
#endif

//NOTE:
//CAVEAT:
// if ALLOW_NEARPLANE_CLIPPING is specified certain objects will not
// be culled as soon as they touch the near view-plane. therefore,
// 3-D clipping MUST be performed to avoid projection problems.
//
// !! if ALLOW_NEARPLANE_CLIPPING is specified here, CLIP_OBJECT_POLYGONS !!
// !! must be specified in Rx_OBJ.C to enable the geometry clipper.       !!


// init object maintenance structures -----------------------------------------
//
void InitObjCtrl()
{
	//NOTE:
	// this function gets called exactly once,
	// from G_BOOT::GameBoot().

	static int init_valid = TRUE;

	if ( init_valid ) {
		init_valid = FALSE;

		// allocate dummy heads of object lists
		PShipObjects = (ShipObject *)	 ALLOCMEM( sizeof( GenObject ) );
		LaserObjects = (LaserObject *)	 ALLOCMEM( sizeof( GenObject ) );
		MisslObjects = (MissileObject *) ALLOCMEM( sizeof( GenObject ) );
		ExtraObjects = (ExtraObject *)   ALLOCMEM( sizeof( GenObject ) );
		CustmObjects = (CustomObject *)  ALLOCMEM( sizeof( GenObject ) );
		VObjList	 = (GenObject *)     ALLOCMEM( sizeof( GenObject ) );

		// check if memory allocation failed
		if ( ( PShipObjects == NULL ) || ( LaserObjects == NULL ) ||
			 ( MisslObjects == NULL ) || ( ExtraObjects == NULL ) ||
			 ( CustmObjects == NULL ) || ( VObjList     == NULL ) ) {

			OUTOFMEM( 0 );
		}

		// initialize dummy heads of object lists
		memset( PShipObjects, 0, sizeof( GenObject ) );
		memset( LaserObjects, 0, sizeof( GenObject ) );
		memset( MisslObjects, 0, sizeof( GenObject ) );
		memset( ExtraObjects, 0, sizeof( GenObject ) );
		memset( CustmObjects, 0, sizeof( GenObject ) );
		memset( VObjList,     0, sizeof( GenObject ) );

	} else {

		// never happens
		ASSERT( 0 );
	}
}


// invoke constructor of custom object ----------------------------------------
//
INLINE
void InvokeCustomConstructor( GenObject *objectpo )
{
	ASSERT( OBJECT_TYPE_CUSTOM( objectpo ) );

	CustomObject *cobj = (CustomObject *) objectpo;
	if ( cobj->callback_instant != NULL ) {
		(*cobj->callback_instant)( cobj );
	}
}


// invoke destructor of custom object -----------------------------------------
//
INLINE
void InvokeCustomDestructor( GenObject *objectpo )
{
	ASSERT( OBJECT_TYPE_CUSTOM( objectpo ) );

	CustomObject *cobj = (CustomObject *) objectpo;
	if ( cobj->callback_destroy != NULL ) {
		(*cobj->callback_destroy)( cobj );
	}
}


// free memory occupied by an object ------------------------------------------
//
void FreeObjectMem( GenObject *objectpo )
{
	//NOTE:
	// this function should only be used
	// from within OBJ_xx modules.

	ASSERT( objectpo != NULL );

	// notify registered custom objects of impending object deletion
	if ( ( objectpo->ObjectType & TYPELISTMASK ) != CUSTM_LIST_NO ) {
		OBJ_NotifyCustomObjectsList( objectpo, CUSTOM_NOTIFY_GENOBJECT_DELETE );
	}

	switch ( objectpo->ObjectType & TYPELISTMASK ) {

		case PSHIP_LIST_NO:
			// ensure duration weapons are properly killed
			KillDurationWeapons( (ShipObject *) objectpo );
			break;

		case LASER_LIST_NO:
			// no special handling
			break;

		case MISSL_LIST_NO:
			// no special handling
			break;

		case EXTRA_LIST_NO:
			// for extras maintain count
			CurrentNumExtras--;
			break;

		case CUSTM_LIST_NO:
			// for custom objects invoke destructor
			InvokeCustomDestructor( objectpo );
			break;
	}

	// free attached particle clusters
	PRT_FreeAttachedClusterList( objectpo );

#ifndef INSTANTIATE_FACE_STRUCTURES
/*//FIXME:
	// non-virtual objects may have additional instance data
	dword objclass = objectpo->ObjectClass;
	if ( objclass != CLASS_ID_INVALID ) {

		// make sure we take the base lod
		Face *basefaces = objectpo->FaceList;
		if ( objectpo->NumLodObjects > 0 ) {
			ASSERT( objectpo->LodObjects != NULL );
			basefaces = objectpo->LodObjects[ 0 ].LodObject->FaceList;
		}

		// free instanced face data if attached separately
		ASSERT( objclass < NumObjClasses );
		if ( basefaces != ObjClasses[ objclass ]->FaceList ) {
			FREEMEM( basefaces );
		}
	}
*/
#endif

	FREEMEM( objectpo );
}


// free all object memory blocks contained in specific list -------------------
//
int FreeObjList( GenObject *listpo )
{
	//NOTE:
	// in addition to FreeObjects() this is also
	// used by CON_ACT::AcDestroyObjectList().

	ASSERT( listpo != NULL );
	ASSERT( listpo != MyShip );

	GenObject *delpo = listpo;
	while ( delpo->NextObj != NULL ) {

		ASSERT( delpo->NextObj );
		ASSERT( delpo->NextObj != MyShip );

		GenObject *temppo = delpo->NextObj->NextObj;
		FreeObjectMem( delpo->NextObj );
		delpo->NextObj = temppo;
	}

	return 1;
}


// free all object memory blocks ----------------------------------------------
//
int FreeObjects()
{
	//NOTE:
	// this function is called every time the
	// game loop is entered (G_BOOT::GameInit())
	// and by CON_ACT::AcPrepareDataRestore() to
	// remove all objects.

	int rc = 0;

	rc += FreeObjList( PShipObjects );
	rc += FreeObjList( LaserObjects );
	rc += FreeObjList( MisslObjects );
	rc += FreeObjList( ExtraObjects );
	rc += FreeObjList( CustmObjects );

	return ( rc == 5 );
}


// correct pointers that have moved after object was instantiated -------------
//
void OBJ_CorrectObjectInstance( GenObject *dstobj, GenObject *srcobj )
{
	ASSERT( dstobj != NULL );
	ASSERT( srcobj != NULL );

	ptrdiff_t pdiff = (char*)dstobj - (char*)srcobj;

	if ( dstobj->FaceAnimStates != NULL ) {
		dstobj->FaceAnimStates = (FaceAnimState *) ( (char*)dstobj->FaceAnimStates + pdiff );
	}
	if ( dstobj->VtxAnimStates != NULL ) {
		dstobj->VtxAnimStates = (VtxAnimState *) ( (char*)dstobj->VtxAnimStates + pdiff );
	}
}


// internal flag to disable automatic stargate creation -----------------------
//
static int disable_ship_creation_stargate = FALSE;


// create object instance and insert into global objectlist -------------------
//
GenObject *CreateObject( int objclass, const Xmatrx startmatrx )
{
	ASSERT( objclass >= 0 );
	ASSERT( objclass < NumObjClasses );

	// check if class number is valid
	if ( ( objclass < 0 ) || ( objclass >= NumObjClasses ) )
		return NULL;

	size_t allocinstancesize = ObjClasses[ objclass ]->InstanceSize;

#ifdef INSTANTIATE_FACE_STRUCTURES

	// reserve space for copy of facelist
	allocinstancesize += ObjClasses[ objclass ]->NumFaces * sizeof( Face );

#endif // INSTANTIATE_FACE_STRUCTURES

	// create new object header
	GenObject *newinstance = (GenObject *) ALLOCMEM( allocinstancesize );
	if ( newinstance == NULL )
		OUTOFMEM( "no mem for object instance." );

	// fill header with class data
	memcpy( newinstance, ObjClasses[ objclass ], ObjClasses[ objclass ]->InstanceSize );
	OBJ_CorrectObjectInstance( newinstance, ObjClasses[ objclass ] );

#ifdef INSTANTIATE_FACE_STRUCTURES

	// make copy of facelist
	Face *instfacelist = (Face *) ( (char*)newinstance + newinstance->InstanceSize );
	memcpy( instfacelist, newinstance->FaceList,
			newinstance->NumFaces * sizeof( Face ) );
	newinstance->FaceList = instfacelist;

#endif // INSTANTIATE_FACE_STRUCTURES

	// set object number
	newinstance->ObjectNumber  = NextObjNumber;
	newinstance->HostObjNumber = CreateGlobalObjId( NextObjNumber );
	NextObjNumber++;

	// set start matrix
	memcpy( newinstance->ObjPosition, startmatrx, sizeof( Xmatrx ) );

	// link header into list of existing objects of this type
	GenObject *listhead;
	switch ( newinstance->ObjectType & TYPELISTMASK ) {

		case PSHIP_LIST_NO:
			listhead = ( PShipObjects->NextObj == MyShip ) ?
						PShipObjects->NextObj : PShipObjects;
			// create stargate for newly created spaceship
			if ( !disable_ship_creation_stargate ) {
				SFX_CreateStargate( (ShipObject *) newinstance );
			}
			break;

		case LASER_LIST_NO:
			listhead = LaserObjects;
			break;

		case MISSL_LIST_NO:
			listhead = MisslObjects;
			// create missile trail
			CreateVaporTrail( newinstance );
			break;

		case EXTRA_LIST_NO:
			listhead = ExtraObjects;
			// create generation animation
			SFX_CreateExtra( (ExtraObject *) newinstance );
			// for extras maintain count
			CurrentNumExtras++;
			break;

		case CUSTM_LIST_NO:
			listhead = CustmObjects;
			// for custom objects invoke constructor
			InvokeCustomConstructor( newinstance );
			break;

		default:
			PANIC( 0 );
	}

	// attach static particles that are
	// part of the object instance
	if ( AUX_ATTACH_OBJECT_PARTICLES ) {
		OBJ_AttachClassParticles( newinstance );
	}

	// append object at head of list
	newinstance->NextObj = listhead->NextObj;
	listhead->NextObj	 = newinstance;

	return newinstance;
}


// create object instance and insert into global objectlist -------------------
//
GenObject *SummonObject( int objclass, const Xmatrx startmatrx )
{
	ASSERT( objclass >= 0 );
	ASSERT( objclass < NumObjClasses );

	// disable stargate
	disable_ship_creation_stargate = TRUE;

	GenObject *obj = CreateObject( objclass, startmatrx );

	// enable stargate
	disable_ship_creation_stargate = FALSE;

	return obj;
}

// create object instance from a type and insert into global objectlist -------
//
GenObject* SummonObjectFromType( int objtypeid, const Xmatrx startmatrx )
{
	ASSERT( objtypeid >= 0 );
	//ASSERT( objtypeid < num_custom_types );

	// check whether this type is summonable
	int custtype_flags = OBJ_GetCustomTypeFlags( objtypeid );
	if ( !( custtype_flags & CUSTOM_TYPE_SUMMONABLE ) ) {
		return NULL;
	}
	
	CustomObject *obj = CreateVirtualObject( objtypeid );
	
	// set start matrix
	memcpy( obj->ObjPosition, startmatrx, sizeof( Xmatrx ) );
	
	return (GenObject*)obj;
}


// create virtual (type-only) instance of custom object -----------------------
//
CustomObject *CreateVirtualObject( dword objtypeid )
{
	ASSERT( TYPEID_TYPE_CUSTOM( objtypeid ) );
	if ( !TYPEID_TYPE_CUSTOM( objtypeid ) )
		return NULL;

	// create new object header
	size_t custsize = OBJ_FetchCustomTypeSize( objtypeid );
	CustomObject *newinstance = (CustomObject *) ALLOCMEM( custsize );
	if ( newinstance == NULL )
		OUTOFMEM( "no mem for custom object." );

	// GenObject::NumVerts==0 ensures object
	// will be skipped by BuildVisList()
	memset( newinstance, 0, custsize );

	// init fields that must be valid
	newinstance->ObjectType   = objtypeid;
	newinstance->ObjectClass  = CLASS_ID_INVALID;
	newinstance->InstanceSize = custsize;

	// would normally have been done on class loading
	OBJ_InitCustomType( newinstance );

	// set object number
	newinstance->ObjectNumber  = NextObjNumber;
	newinstance->HostObjNumber = CreateGlobalObjId( NextObjNumber );
	NextObjNumber++;

	// invoke constructor
	InvokeCustomConstructor( newinstance );

	// append object at head of list
	newinstance->NextObj  = CustmObjects->NextObj;
	CustmObjects->NextObj = newinstance;

	return newinstance;
}


// fetch object contained in specific object list -----------------------------
//
GenObject *FetchSpecificObject( dword objno, GenObject *listpo )
{
	ASSERT( listpo != NULL );

	GenObject *scan = listpo->NextObj;
	while ( ( scan != NULL ) && ( scan->ObjectNumber != objno ) )
		scan = scan->NextObj;

	// prevent fetching of local ship
	if ( scan == MyShip )
		scan = NULL;

	// return found object or NULL
	return scan;
}


// fetch object contained in specific object list -----------------------------
//
GenObject *FetchSpecificHostObject( dword hostobjno, GenObject *listpo )
{
	ASSERT( listpo != NULL );

	// NOTE: Since MyShip->HostObjNumber seems not to be set to
	// 		 ShipHostObjId( LocalPlayerId ), ( scan->HostObjNumber == 0 )
	//		 could be MyShip in ObjectCamera-Mode, additionally present to
	//		 ship with real HostObjNumber of 0! So we try to check next
	//		 object.

	GenObject *scan = listpo->NextObj;
	while ( ( ( scan != NULL ) && ( scan->HostObjNumber != hostobjno ) ) || ( scan == MyShip )  )
		scan = scan->NextObj;

	ASSERT( scan != MyShip );

	// obsolete
	// prevent fetching of local ship
//	if ( scan == MyShip )
//		scan = NULL;

	// return found object or NULL
	return scan;
}


// fetch object via object id (search in all object lists) --------------------
//
GenObject *FetchObject( dword objno )
{
	GenObject *obj;

	obj = FetchSpecificObject( objno, PShipObjects );
	if ( obj != NULL )
		return obj;
	obj = FetchSpecificObject( objno, LaserObjects );
	if ( obj != NULL )
		return obj;
	obj = FetchSpecificObject( objno, MisslObjects );
	if ( obj != NULL )
		return obj;
	obj = FetchSpecificObject( objno, ExtraObjects );
	if ( obj != NULL )
		return obj;
	obj = FetchSpecificObject( objno, CustmObjects );

	return obj;
}


// fetch object via host object id (search in all object lists) ---------------
//
GenObject *FetchHostObject( dword hostobjno )
{
	GenObject *obj;

	obj = FetchSpecificHostObject( hostobjno, PShipObjects );
	if ( obj != NULL )
		return obj;
	obj = FetchSpecificHostObject( hostobjno, LaserObjects );
	if ( obj != NULL )
		return obj;
	obj = FetchSpecificHostObject( hostobjno, MisslObjects );
	if ( obj != NULL )
		return obj;
	obj = FetchSpecificHostObject( hostobjno, ExtraObjects );
	if ( obj != NULL )
		return obj;
	obj = FetchSpecificHostObject( hostobjno, CustmObjects );

	return obj;
}


// kill object contained in specific object list (search only one list) -------
//
int KillSpecificObject( dword objno, GenObject *listpo )
{
	ASSERT( listpo != NULL );
	ASSERT( listpo != MyShip );

	GenObject *delpo = listpo;
	while ( ( delpo->NextObj != NULL ) && ( delpo->NextObj->ObjectNumber != objno ) )
		delpo = delpo->NextObj;

	if ( delpo->NextObj == NULL )
		return 0;

	ASSERT( delpo->NextObj );
	ASSERT( delpo->NextObj != MyShip );

	GenObject *temppo = delpo->NextObj->NextObj;
	FreeObjectMem( delpo->NextObj );
	delpo->NextObj = temppo;

	return 1;
}


// kill object contained in specific object list (search only one list) -------
//
int KillSpecificHostObject( dword hostobjno, GenObject *listpo )
{
	ASSERT( listpo != NULL );
	ASSERT( listpo != MyShip );

	GenObject *delpo = listpo;
	while ( ( delpo->NextObj != NULL ) && ( delpo->NextObj->HostObjNumber != hostobjno ) ) {
		delpo = delpo->NextObj;
	}

	if ( delpo->NextObj == NULL )
		return 0;

	ASSERT( delpo->NextObj );
	ASSERT( delpo->NextObj != MyShip );

	GenObject *temppo = delpo->NextObj->NextObj;
	FreeObjectMem( delpo->NextObj );
	delpo->NextObj = temppo;

	return 1;
}


// kill object instance (search in all object lists) --------------------------
//
int KillObject( dword objno )
{
	if      ( KillSpecificObject( objno, PShipObjects ) )
		return 1;
	else if ( KillSpecificObject( objno, LaserObjects ) )
		return 1;
	else if ( KillSpecificObject( objno, MisslObjects ) )
		return 1;
	else if ( KillSpecificObject( objno, ExtraObjects ) )
		return 1;
	else if ( KillSpecificObject( objno, CustmObjects ) )
		return 1;

	return 0;
}


// kill object instance (search in all object lists) --------------------------
//
int KillHostObject( dword hostobjno )
{
	//NOTE:
	// currently never used since KillSpecificHostObject()
	// is always used directly.

//FIXME:
	ASSERT( 0 ); // remove this on first use

	if      ( KillSpecificHostObject( hostobjno, PShipObjects ) )
		return 1;
	else if ( KillSpecificHostObject( hostobjno, LaserObjects ) )
		return 1;
	else if ( KillSpecificHostObject( hostobjno, MisslObjects ) )
		return 1;
	else if ( KillSpecificHostObject( hostobjno, ExtraObjects ) )
		return 1;
	else if ( KillSpecificHostObject( hostobjno, CustmObjects ) )
		return 1;

	return 0;
}


// kill all instances of passed in object class (search only one list) --------
//
PRIVATE
int KillClassInstancesFromList( int objclass, GenObject *listpo )
{
	ASSERT( listpo != NULL );
	ASSERT( listpo != MyShip );

	GenObject *delpo = listpo;
	for ( int killcount = 0; ; killcount++ ) {

		while ( ( delpo->NextObj != NULL ) &&
				( delpo->NextObj->ObjectClass != (dword)objclass ) )
			delpo = delpo->NextObj;

		if ( delpo->NextObj == NULL )
			return killcount;

		ASSERT( delpo->NextObj );
		ASSERT( delpo->NextObj != MyShip );

		GenObject *temppo = delpo->NextObj->NextObj;
		FreeObjectMem( delpo->NextObj );
		delpo->NextObj = temppo;
	}

	// never reached
	ASSERT( 0 );
}


// kill all instances of passed in object class (search in all object lists) --
//
int KillClassInstances( int objclass )
{
	//NOTE:
	// this function is used by CON_LOAD::ConLoadObject()
	// to remove objects prior to overloading them.

	int killcount = 0;

	killcount += KillClassInstancesFromList( objclass, PShipObjects );
	killcount += KillClassInstancesFromList( objclass, LaserObjects );
	killcount += KillClassInstancesFromList( objclass, MisslObjects );
	killcount += KillClassInstancesFromList( objclass, ExtraObjects );
	killcount += KillClassInstancesFromList( objclass, CustmObjects );

	return killcount;
}


//FIXME: seperate between object management functions and other functions in this module




// check entire object against viewing frustum --------------------------------
//
INLINE
int ObjectInFrustum( const Camera camera, GenObject *objectp )
{
	ASSERT( objectp != NULL );

	// cull against entire frustum by default
	objectp->CullMask = 0x3f;

	// get type id of the custom planet
	static dword planet_typeid = TYPE_ID_INVALID;
	if ( planet_typeid == TYPE_ID_INVALID ) {
		planet_typeid = OBJ_FetchCustomTypeId( "planet" );
	}

	// disable far-plane clipping for planets
	if ( objectp->ObjectType == planet_typeid ) {
		objectp->CullMask = 0x3d;
	}


#ifdef CULL_IN_WORLD_SPACE

	// precalculated world-frustum for ViewCamera
	ASSERT( camera == ViewCamera );

	geomv_t radius = objectp->BoundingSphere;

	#ifdef CULL_BOUNDING_SPHERES

		Sphere3 sphere;
		sphere.X = objectp->ObjPosition[ 0 ][ 3 ];
		sphere.Y = objectp->ObjPosition[ 1 ][ 3 ];
		sphere.Z = objectp->ObjPosition[ 2 ][ 3 ];
		sphere.R = radius;

		// ensure near view plane doesn't clip object for most object types
		geomv_t nearplaneofs = PLANE_OFFSET( &World_ViewVolume[ 0 ] );
		if ( PREVENT_OBJECT_NEARPLANE_CLIPPING( objectp ) ) {
			PLANE_OFFSET( &World_ViewVolume[ 0 ] ) += radius * 2;
		}

		int cull = CULL_SphereAgainstVolume( &sphere, World_ViewVolume, &objectp->CullMask );

		PLANE_OFFSET( &World_ViewVolume[ 0 ] ) = nearplaneofs;

		return !cull;

	#else // CULL_BOUNDING_SPHERES

		Vertex3 origin;
		origin.X = objectp->ObjPosition[ 0 ][ 3 ];
		origin.Y = objectp->ObjPosition[ 1 ][ 3 ];
		origin.Z = objectp->ObjPosition[ 2 ][ 3 ];

		CullBox3 cullbox;

		cullbox.minmax[ 0 ] = origin.X - radius;
		cullbox.minmax[ 1 ] = origin.Y - radius;
		cullbox.minmax[ 2 ] = origin.Z - radius;

		cullbox.minmax[ 3 ] = origin.X + radius;
		cullbox.minmax[ 4 ] = origin.Y + radius;
		cullbox.minmax[ 5 ] = origin.Z + radius;

		// ensure near view plane doesn't clip object for most object types
		geomv_t nearplaneofs = PLANE_OFFSET( &World_CullVolume[ 0 ].plane );
		if ( PREVENT_OBJECT_NEARPLANE_CLIPPING( objectp ) ) {
			PLANE_OFFSET( &World_CullVolume[ 0 ].plane ) += radius * 2;
		}

		int cull = CULL_BoxAgainstCullVolume( &cullbox, World_CullVolume, &objectp->CullMask );

		PLANE_OFFSET( &World_CullVolume[ 0 ].plane ) = nearplaneofs;

		return !cull;

	#endif // CULL_BOUNDING_SPHERES

#else // CULL_IN_WORLD_SPACE

	// fetch object position
	geomv_t	objX = objectp->CurrentXmatrx[ 0 ][ 3 ];
	geomv_t objY = objectp->CurrentXmatrx[ 1 ][ 3 ];
	geomv_t objZ = objectp->CurrentXmatrx[ 2 ][ 3 ];

	// fetch radius of bounding sphere
	geomv_t bsphere = objectp->BoundingSphere;

	// calc potentially nearest z coordinate
	geomv_t nearestZ = objZ - bsphere;

	// far too near? :)
	if ( nearestZ < Near_View_Plane )
		return FALSE;
	// too far away?
	if ( nearestZ > Far_View_Plane )
		return FALSE;

	// calc potentially farthest z coordinate
	geomv_t farthestZ = objZ + bsphere;

	geomv_t criterionX = GEOMV_MUL( farthestZ, Criterion_X );
	// too far to the left or right?
	if ( ( objX - bsphere ) > criterionX )
		return FALSE;
	if ( ( objX + bsphere ) < -criterionX )
		return FALSE;

	geomv_t criterionY = GEOMV_MUL( farthestZ, Criterion_Y );
	// too far up or down?
	if ( ( objY - bsphere ) > criterionY )
		return FALSE;
	if ( ( objY + bsphere ) < -criterionY )
		return FALSE;

	return TRUE;

#endif // CULL_IN_WORLD_SPACE

}


// calc position of object in viewspace ---------------------------------------
//
INLINE
void CalcViewSpacePosition( const Camera camera, GenObject *object, Vertex3& pos )
{
	ASSERT( object != NULL );

	Vertex3 objt;
	objt.X = object->ObjPosition[ 0 ][ 3 ];
	objt.Y = object->ObjPosition[ 1 ][ 3 ];
	objt.Z = object->ObjPosition[ 2 ][ 3 ];

	MtxVctMUL( camera, &objt, &pos );
}


// check whether specified object is current target ---------------------------
//
INLINE
void ObjectIsTarget( const Camera camera, GenObject *object, Vertex3& pos )
{
	ASSERT( object != NULL );

	if ( ObjCameraActive )
		return;

	if ( OBJECT_TYPE_SHIP( object ) && ( object->HostObjNumber == TargetObjNumber ) ) {

		ShipObject *ship = (ShipObject *) object;

		if ( ( ship->ExplosionCount == 0 ) ||
		     ( ship->ExplosionCount >= BM_EXPLNOTARGFRAME * EXPL_REF_SPEED ) ) {
/*
#ifdef CULL_IN_WORLD_SPACE
			CalcViewSpacePosition( camera, object, pos );
#endif
*/			// set flag that current target is visible
			TargetVisible = TRUE;

			if ( ( object->HostObjNumber & 0xffff ) == 0 )
				TargetRemId = GetObjectOwner( object );
			else
				TargetRemId = TARGETID_NO_TARGET;

			// calc position of target tracker
			SPoint tloc;
			PROJECT_TO_SCREEN( pos, tloc );
			TargetScreenX = tloc.X;
			TargetScreenY = tloc.Y;
		 }
	}
}


// enlist object in list of visible objects -----------------------------------
//
INLINE
void EnlistVisibleObject( GenObject *list, GenObject *obj )
{
	ASSERT( list != NULL );
	ASSERT( obj != NULL );

	if ( !AUX_DISABLE_OBJECT_DEPTH_SORT ) {

		geomv_t insertZ = obj->CurrentXmatrx[ 2 ][ 3 ];

		// search to right position
		while ( ( list->NextVisObj != NULL ) &&
				( list->NextVisObj->CurrentXmatrx[ 2 ][ 3 ] > insertZ ) )
			list = list->NextVisObj;
	}

	// indicate that object visible in this frame
	obj->VisibleFrame = CurVisibleFrame;

	// insert object into list
	obj->NextVisObj  = list->NextVisObj;
	list->NextVisObj = obj;
}


// switch object detail level -------------------------------------------------
//
void OBJ_SwitchObjectLod( GenObject *obj, dword lod )
{
	ASSERT( obj != NULL );
	ASSERT( lod < obj->NumLodObjects );

	// store active lod
	obj->CurrentLod = lod;

	// retrieve source geometry
	ASSERT( obj->LodObjects != NULL );
	GenLodObject *lodobj = obj->LodObjects[ lod ].LodObject;

	// switch geometry
	ASSERT( lodobj != NULL );
	obj->NumVerts			= lodobj->NumVerts;
	obj->NumPolyVerts		= lodobj->NumPolyVerts;
	obj->NumNormals			= lodobj->NumNormals;
	obj->VertexList			= lodobj->VertexList;
	obj->X_VertexList		= lodobj->X_VertexList;
	obj->S_VertexList		= lodobj->S_VertexList;
	obj->NumPolys			= lodobj->NumPolys;
	obj->PolyList			= lodobj->PolyList;
	obj->NumFaces			= lodobj->NumFaces;
	obj->FaceList			= lodobj->FaceList;
	obj->VisPolyList		= lodobj->VisPolyList;
	obj->SortedPolyList		= lodobj->SortedPolyList;
	obj->AuxList			= lodobj->AuxList;
	obj->BSPTree			= lodobj->BSPTree;
	obj->AuxBSPTree			= lodobj->AuxBSPTree;
	obj->AuxObject			= lodobj->AuxObject;
	obj->NumWedges			= lodobj->NumWedges;
	obj->NumLayers			= lodobj->NumLayers;
	obj->WedgeFlags			= lodobj->WedgeFlags;
	obj->WedgeVertIndxs		= lodobj->WedgeVertIndxs;
	obj->WedgeNormals		= lodobj->WedgeNormals;
	obj->WedgeColors		= lodobj->WedgeColors;
	obj->WedgeTexCoords		= lodobj->WedgeTexCoords;
	obj->WedgeLighted		= lodobj->WedgeLighted;
	obj->WedgeSpecular		= lodobj->WedgeSpecular;
	obj->WedgeFogged		= lodobj->WedgeFogged;
	obj->ActiveFaceAnims	= lodobj->ActiveFaceAnims;
	obj->ActiveVtxAnims		= lodobj->ActiveVtxAnims;
}


// check whether object lod needs to be changed -------------------------------
//
void OBJ_AutoSelectObjectLod( GenObject *obj )
{
	ASSERT( obj != NULL );

	if ( obj->NumLodObjects == 0 ) {
		return;
	}

	ASSERT( obj->LodObjects != NULL );
	ASSERT( obj->CurrentLod < obj->NumLodObjects );

	if ( AUXDATA_LOD_DISCRETE_GEOMETRY_BIAS < 0 )
		AUXDATA_LOD_DISCRETE_GEOMETRY_BIAS = 0;
	int lodbias = AUXDATA_LOD_DISCRETE_GEOMETRY_BIAS;

	// determine boundary for threshold checking
	int boundlod = obj->CurrentLod - lodbias;
	if ( boundlod < 0 ) {
		boundlod = 0;
	}

	// check for magnification
	int clod = 0;
	for ( clod = 0; clod < boundlod; clod++ ) {
		GenLodInfo *info = &obj->LodObjects[ clod ];
		if ( obj->CurrentXmatrx[ 2 ][ 3 ] < info->MagTreshold ) {
			clod += lodbias;
			if ( clod >= obj->NumLodObjects )
				clod = obj->NumLodObjects - 1;
			OBJ_SwitchObjectLod( obj, clod );
			return;
		}
	}

	// check for minification
	for ( clod = obj->NumLodObjects - 1; clod > boundlod; clod-- ) {
		GenLodInfo *info = &obj->LodObjects[ clod - 1 ];
		if ( obj->CurrentXmatrx[ 2 ][ 3 ] > info->MinTreshold ) {
			clod += lodbias;
			if ( clod >= obj->NumLodObjects )
				clod = obj->NumLodObjects - 1;
			OBJ_SwitchObjectLod( obj, clod );
			return;
		}
	}

	// check virtual minification boundary to the far left
	if ( obj->CurrentLod < lodbias ) {
		clod = lodbias;
		if ( clod >= obj->NumLodObjects )
			clod = obj->NumLodObjects - 1;
		OBJ_SwitchObjectLod( obj, clod );
	}
}


// scan a single objectlist; determine visibility, and build list -------------
//
PRIVATE
void BuildVisList( GenObject *sourcelist, GenObject *destlist, const Camera camera )
{
	//NOTE:
	// sourcelist may be NULL if an empty
	// list is being merged.

	ASSERT( destlist != NULL );

	// walk list of objects
	for ( GenObject *scanpo = sourcelist; scanpo; scanpo = scanpo->NextObj ) {

		//NOTE:
		// there are two possibilities for GenObject::NumVerts to be zero at
		// this stage (later on the clipper might clear it for object rejection)
		// 1. the object consists of subobjects only (baseobject is empty)
		// 2. the object is a virtual custom object (contains no geometry)
		// in case 1 we have to render the object, whereas in case 2 we
		// do not even insert it into the list of visible objects.

		if ( scanpo->NumVerts == 0 ) {

			ASSERT( scanpo->NumPolys == 0 );
			ASSERT( scanpo->NumFaces == 0 );

			// skip virtual (type-only) custom objects
			if ( OBJECT_TYPE_CUSTOM( scanpo ) ) {
				continue;
			}
		}

		Vertex3 objpos;

		//NOTE:
		// the radar needs the position in view-space
		// even if we cull in world-space and wouldn't
		// need the position for culling.

//#ifndef CULL_IN_WORLD_SPACE

		CalcViewSpacePosition( camera, scanpo, objpos );

		scanpo->CurrentXmatrx[ 0 ][ 3 ] = objpos.X;
		scanpo->CurrentXmatrx[ 1 ][ 3 ] = objpos.Y;
		scanpo->CurrentXmatrx[ 2 ][ 3 ] = objpos.Z;

//#endif

		// cull objects, enlist visible ones
		if ( ObjectInFrustum( camera, scanpo ) && ( scanpo->VisibleFrame != VISFRAME_NEVER ) ) {

			// check if object is current target
			ObjectIsTarget( camera, scanpo, objpos );

			// insert object in list of visible objects
			EnlistVisibleObject( destlist, scanpo );

			// check for lod change
			OBJ_AutoSelectObjectLod( scanpo );
		}
	}
}


// scan objectlists; determine visibility; create list of visibles ------------
//
void ScanActiveObjects( const Camera camera )
{
	ASSERT( VObjList != NULL );

	VObjList->NextVisObj = NULL;

#ifdef ALLOW_OBJLIST_DISABLING

	if ( !AUX_OBJCTRL_DISABLE_LASEROBJECTS )
		BuildVisList( FetchFirstLaser(),   VObjList, camera );

	if ( !AUX_OBJCTRL_DISABLE_EXTRAOBJECTS )
		BuildVisList( FetchFirstExtra(),   VObjList, camera );

	if ( !AUX_OBJCTRL_DISABLE_CUSTOMOBJECTS )
		BuildVisList( FetchFirstCustom(),  VObjList, camera );

	if ( !AUX_OBJCTRL_DISABLE_MISSLOBJECTS )
		BuildVisList( FetchFirstMissile(), VObjList, camera );

	if ( !AUX_OBJCTRL_DISABLE_SHIPOBJECTS )
		BuildVisList( FetchShipListHead(), VObjList, camera );
#else

	BuildVisList( FetchFirstLaser(),   VObjList, camera );
	BuildVisList( FetchFirstExtra(),   VObjList, camera );
	BuildVisList( FetchFirstCustom(),  VObjList, camera );
	BuildVisList( FetchFirstMissile(), VObjList, camera );
	BuildVisList( FetchShipListHead(), VObjList, camera );

#endif

}



