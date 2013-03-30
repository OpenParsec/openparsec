/*
 * PARSEC - World representation
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:46 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
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

// subsystem & headers
#include "net_defs.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "parttype.h"
#include "e_world.h"

// proprietary module headers
#include "net_game_sv.h"
#include "net_util.h"
#include "e_simulator.h"
#include "e_colldet.h"
#include "g_player.h"
#include "g_main_sv.h"

// local module header
#include "obj_cust.h"

// flags ----------------------------------------------------------------------
//
//#define HACKED_SHIPOBJECT_INSTANTIATION

// undefine macros used for transition ----------------------------------------
//
#undef CreateObject
#undef PShipObjects		
#undef LaserObjects		
#undef MisslObjects		
#undef ExtraObjects		
#undef CustmObjects		

#define PShipObjects		m_PShipObjects
#define LaserObjects		m_LaserObjects
#define MisslObjects		m_MisslObjects
#define ExtraObjects		m_ExtraObjects
#define CustmObjects		m_CustmObjects

// flag if initialization of particle system already done ---------------------
//
static int particle_init_done	= FALSE;

// reference z values for particles -------------------------------------------
//
float sphere_ref_z = 1.0f;
float lightning_ref_z = 1.0f;
float photon_ref_z = 1.0f;

void LinearParticleCollision( linear_pcluster_s *cluster, int pid );
// flags
#define USE_Z_BUFFER				// use z buffer to draw particles
//#define STORE_MAX_Z_VALS			// log max/min z values
//#define USER_SPHERE_CREATION		// enable user sphere creation (console)
#define DO_CLUSTER_CULLING			// enable culling of whole clusters
#define USE_NEW_CULLING_CODE		// use new culling function
//#define USE_BOUNDING_BOX_CULLING	// cull bounding box instead of sphere
#define ENABLE_PARTICLE_TRIANGLES	// enable triangle drawing for particles


// bitmap size below which it is replaced by a square of a single color
#define BITMAP_SIZE_BOUNDARY		3

// typical lower boundary of particle bitmap size
int partbitmap_size_bound		= BITMAP_SIZE_BOUNDARY;

pcluster_s *Particles			= NULL;
pcluster_s *CurLinearCluster	= NULL;
pcluster_s *CustomDrawCluster	= NULL;

// array of registered particle definitions -----------------------------------
//
PUBLIC int				NumParticleDefinitions = 0;
PUBLIC pdefref_s		ParticleDefinitions[ MAX_PARTICLE_DEFS ];


// ----------------------------------------------------------------------------
//
int E_World::_CountGenObjects( GenObject* walklist )
{
	int 	  listlength;

	listlength = 0;
	for ( ; walklist; walklist = walklist->NextObj )
		listlength++;

	return listlength;
}

// free memory occupied by an object ------------------------------------------
//
void E_World::_FreeObjectMem( GenObject* objectpo )
{
	//NOTE:
	// this function should only be used
	// from within OBJ_xx modules.

	ASSERT( objectpo != NULL );

	switch ( objectpo->ObjectType & TYPELISTMASK ) {

		case PSHIP_LIST_NO:
			// ensure duration weapons are properly killed
			//KillDurationWeapons( (ShipObject *) objectpo );
			break;

		case LASER_LIST_NO:
			// no special handling
			break;

		case MISSL_LIST_NO:
			// no special handling
			break;

		case EXTRA_LIST_NO:
			// for extras maintain count
			m_nCurrentNumExtras--;
			break;

		case CUSTM_LIST_NO:
			// for custom objects invoke destructor
			_InvokeCustomDestructor( objectpo );
			break;
	}

	// free attached particle clusters
#ifdef _SERVER_HAS_PARTICLE_CODE
	//FIXME: CBX: uncomment this, when particle subsys is integrated into servercode
	PRT_FreeAttachedClusterList( objectpo );
#endif // _SERVER_HAS_PARTICLE_CODE

#ifndef INSTANTIATE_FACE_STRUCTURES
/*//FIXME: MSH ????
	// non-virtual objects may have additional instance data
	dword objclass = objectpo->ObjectClass;
	if ( objclass != CLASS_ID_INVALID ) {

		// make sure we take the base lod
		Face* basefaces = objectpo->FaceList;
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


// invoke constructor of custom object ----------------------------------------
//
void E_World::_InvokeCustomConstructor( GenObject* objectpo )
{
	ASSERT( OBJECT_TYPE_CUSTOM( objectpo ) );

	CustomObject* cobj = (CustomObject *) objectpo;
	if ( cobj->callback_instant != NULL ) {
		(*cobj->callback_instant)( cobj );
	}
}


// invoke destructor of custom object -----------------------------------------
//
void E_World::_InvokeCustomDestructor( GenObject* objectpo )
{
	ASSERT( OBJECT_TYPE_CUSTOM( objectpo ) );

	CustomObject* cobj = (CustomObject *) objectpo;
	if ( cobj->callback_destroy != NULL ) {
		(*cobj->callback_destroy)( cobj );
	}
}


// kill all instances of passed in object class (search only one list) --------
//
int E_World::_KillClassInstancesFromList( int objclass, GenObject *listpo )
{
	ASSERT( listpo != NULL );
#ifdef PARSEC_CLIENT
	ASSERT( listpo != MyShip );
#endif // PARSEC_CLIENT

	GenObject *delpo = listpo;
	for ( int killcount = 0; ; killcount++ ) {

		while ( ( delpo->NextObj != NULL ) &&
				( delpo->NextObj->ObjectClass != (dword)objclass ) )
			delpo = delpo->NextObj;

		if ( delpo->NextObj == NULL )
			return killcount;

		ASSERT( delpo->NextObj );
#ifdef PARSEC_CLIENT
		ASSERT( delpo->NextObj != MyShip );
#endif // PARSEC_CLIENT

		GenObject *temppo = delpo->NextObj->NextObj;
		FreeObjectMem( delpo->NextObj );
		delpo->NextObj = temppo;
	}

	// never reached
	ASSERT( 0 );
}


// standard ctor --------------------------------------------------------------
//
E_World::E_World()
{
	m_PShipObjects = NULL;
	m_LaserObjects = NULL;
	m_MisslObjects = NULL;
	m_ExtraObjects = NULL;
	m_CustmObjects = NULL;

	m_last_summoned_objectid = 0;

	// current and maximum number of extras
	m_nCurrentNumExtras		= 0;
	m_nCurrentNumPrtExtras	= 0;
	//MaxNumExtras			= 20;

	NextObjNumber			= 1;

	MaxNumShots = 0;
	NumShots	= 0;

	_InitObjCtrl();
}

// correct pointers that have moved after object was instantiated -------------
//
void E_World::OBJ_CorrectObjectInstance( GenObject *dstobj, GenObject *srcobj )
{
	ASSERT( dstobj != NULL );
	ASSERT( srcobj != NULL );

#ifdef PARSEC_CLIENT

	ptrdiff_t pdiff = (char*)dstobj - (char*)srcobj;

	if ( dstobj->FaceAnimStates != NULL ) {
		dstobj->FaceAnimStates = (FaceAnimState *) ( (char*)dstobj->FaceAnimStates + pdiff );
	}
	if ( dstobj->VtxAnimStates != NULL ) {
		dstobj->VtxAnimStates = (VtxAnimState *) ( (char*)dstobj->VtxAnimStates + pdiff );
	}

#endif // PARSEC_CLIENT

}


// kill object contained in specific object list (search only one list) -------
//
int E_World::KillSpecificObject( dword objno, GenObject *listpo )
{
	ASSERT( listpo != NULL );
#ifdef PARSEC_CLIENT
	ASSERT( listpo != MyShip );
#endif // PARSEC_CLIENT

	GenObject *delpo = listpo;
	while ( ( delpo->NextObj != NULL ) &&
			( delpo->NextObj->ObjectNumber != objno ) )
		delpo = delpo->NextObj;

	if ( delpo->NextObj == NULL )
		return 0;

	ASSERT( delpo->NextObj );
#ifdef PARSEC_CLIENT
	ASSERT( delpo->NextObj != MyShip );
#endif // PARSEC_CLIENT

	GenObject *temppo = delpo->NextObj->NextObj;
	FreeObjectMem( delpo->NextObj );
	delpo->NextObj = temppo;

	return 1;
}


// kill a specific ship object from a list ------------------------------------
//
int	E_World::KillSpecificShipObject( dword objno )
{
	return KillSpecificObject( objno, m_PShipObjects );
}


// return pointer to first "real" ship in ship-objects list -------------------
//
ShipObject* E_World::FetchFirstShip()	
{
	// fetch head of ship-objects list
	ShipObject* head = (ShipObject *) m_PShipObjects->NextObj;

#ifdef PARSEC_CLIENT
	// if local ship is head of list return next object
	if ( head == MyShip ) {
		head = (ShipObject *) head->NextObj;
	}
#endif // PARSEC_CLIENT

	return head;
}

// fetch object contained in specific object list -----------------------------
//
GenObject* E_World::FetchSpecificObject( dword objno, GenObject* listpo )
{
	ASSERT( listpo != NULL );

	GenObject* scan = listpo->NextObj;
	while ( ( scan != NULL ) && ( scan->ObjectNumber != objno ) )
		scan = scan->NextObj;

#ifdef PARSEC_CLIENT
	// prevent fetching of local ship
	if ( scan == MyShip )
		scan = NULL;
#endif // PARSEC_CLIENT

	// return found object or NULL
	return scan;
}


// fetch object via object id (search in all object lists) --------------------
//
GenObject* E_World::FetchObject( dword objno )
{
	GenObject* obj;

	obj = FetchSpecificObject( objno, m_PShipObjects );
	if ( obj != NULL )
		return obj;
	obj = FetchSpecificObject( objno, m_LaserObjects );
	if ( obj != NULL )
		return obj;
	obj = FetchSpecificObject( objno, m_MisslObjects );
	if ( obj != NULL )
		return obj;
	obj = FetchSpecificObject( objno, m_ExtraObjects );
	if ( obj != NULL )
		return obj;
	obj = FetchSpecificObject( objno, m_CustmObjects );

	return obj;
}


// free memory occupied by an object ------------------------------------------
//
void E_World::FreeObjectMem( GenObject *objectpo )
{
	//NOTE:
	// this function should only be used
	// from within OBJ_xx modules.

	ASSERT( objectpo != NULL );

	switch ( objectpo->ObjectType & TYPELISTMASK ) {

		case PSHIP_LIST_NO:
#ifdef PARSEC_CLIENT
			// ensure duration weapons are properly killed
			KillDurationWeapons( (ShipObject *) objectpo );
#endif // PARSEC_CLIENT
			break;

		case LASER_LIST_NO:
			// no special handling
			break;

		case MISSL_LIST_NO:
			// no special handling
			break;

		case EXTRA_LIST_NO:
			// for extras maintain count
			m_nCurrentNumExtras--;
			break;

		case CUSTM_LIST_NO:
			// for custom objects invoke destructor
			_InvokeCustomDestructor( objectpo );
			break;
	}

#ifdef PARSEC_CLIENT
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
#endif // INSTANTIATE_FACE_STRUCTURES
#endif // PARSEC_CLIENT

	FREEMEM( objectpo );
}


// free all object memory blocks contained in specific list -------------------
//
int E_World::FreeObjList( GenObject* listpo )
{
	//NOTE:
	// in addition to FreeObjects() this is also
	// used by CON_ACT::AcDestroyObjectList().

	ASSERT( listpo != NULL );
#ifdef PARSEC_CLIENT
	ASSERT( listpo != MyShip );
#endif // PARSEC_CLIENT

	GenObject* delpo = listpo;
	while ( delpo->NextObj != NULL ) {

		ASSERT( delpo->NextObj );
#ifdef PARSEC_CLIENT
		ASSERT( delpo->NextObj != MyShip );
#endif // PARSEC_CLIENT

		GenObject* temppo = delpo->NextObj->NextObj;
		_FreeObjectMem( delpo->NextObj );
		delpo->NextObj = temppo;
	}

	return 1;
}


// free all object memory blocks ----------------------------------------------
//
int E_World::FreeObjects()
{
	//NOTE:
	// this function is called every time the
	// game loop is entered (G_BOOT::GameInit())
	// and by CON_ACT::AcPrepareDataRestore() to
	// remove all objects.

	int rc = 0;

	rc += FreeObjList( m_PShipObjects );
	rc += FreeObjList( m_LaserObjects );
	rc += FreeObjList( m_MisslObjects );
	rc += FreeObjList( m_ExtraObjects );
	rc += FreeObjList( m_CustmObjects );

	return ( rc == 5 );
}


// free all objects and particles ---------------------------------------------
//
void E_World::KillAllObjects()
{
	//NOTE:
	// the order of the following calls is crucial.
	// 3-D objects MUST BE freed before particle objects.

	FreeObjects();
#ifdef _SERVER_HAS_PARTICLE_CODE
	FreeParticles();
#endif // _SERVER_HAS_PARTICLE_CODE
}


// kill all instances of passed in object class (search in all object lists) --
//
int E_World::KillClassInstances( int objclass )
{
	//NOTE:
	// this function is used by CON_LOAD::ConLoadObject()
	// to remove objects prior to overloading them.

	int killcount = 0;

	killcount += _KillClassInstancesFromList( objclass, m_PShipObjects );
	killcount += _KillClassInstancesFromList( objclass, m_LaserObjects );
	killcount += _KillClassInstancesFromList( objclass, m_MisslObjects );
	killcount += _KillClassInstancesFromList( objclass, m_ExtraObjects );
	killcount += _KillClassInstancesFromList( objclass, m_CustmObjects );

	return killcount;
}

// ----------------------------------------------------------------------------
//FIXME: does this belong here ?
void E_World::IncreaseShotCounter() 
{
	NumShots++;
	if ( NumShots > MaxNumShots ) {
		MaxNumShots = NumShots;
	}
}

// ----------------------------------------------------------------------------
//FIXME: does this belong here ?
void E_World::DecreaseShotCounter()
{
	NumShots--;
}



// init object maintenance structures -----------------------------------------
//
void E_World::_InitObjCtrl()
{
	//NOTE: (client)
	// this function gets called exactly once,
	// from G_BOOT::GameBoot().

	//NOTE: (server)
	// this function gets called exactly once,
	// from E_World::E_World()

	static int init_valid = TRUE;

	if ( init_valid ) {
		init_valid = FALSE;

		// allocate dummy heads of object lists
		PShipObjects = (ShipObject *)	 ALLOCMEM( sizeof( GenObject ) );
		LaserObjects = (LaserObject *)	 ALLOCMEM( sizeof( GenObject ) );
		MisslObjects = (MissileObject *) ALLOCMEM( sizeof( GenObject ) );
		ExtraObjects = (ExtraObject *)   ALLOCMEM( sizeof( GenObject ) );
		CustmObjects = (CustomObject *)  ALLOCMEM( sizeof( GenObject ) );
#ifdef PARSEC_SERVER
		// check if memory allocation failed
		if ( ( PShipObjects == NULL ) || ( LaserObjects == NULL ) ||
			 ( MisslObjects == NULL ) || ( ExtraObjects == NULL ) ||
			 ( CustmObjects == NULL ) ) {

			OUTOFMEM( 0 );
		}
#else // !PARSEC_SERVER
		VObjList	 = (GenObject *)     ALLOCMEM( sizeof( GenObject ) );

		// check if memory allocation failed
		if ( ( PShipObjects == NULL ) || ( LaserObjects == NULL ) ||
			 ( MisslObjects == NULL ) || ( ExtraObjects == NULL ) ||
			 ( CustmObjects == NULL ) || ( VObjList     == NULL ) ) {

			OUTOFMEM( 0 );
		}
#endif // !PARSEC_SERVER

		// initialize dummy heads of object lists
		memset( PShipObjects, 0, sizeof( GenObject ) );
		memset( LaserObjects, 0, sizeof( GenObject ) );
		memset( MisslObjects, 0, sizeof( GenObject ) );
		memset( ExtraObjects, 0, sizeof( GenObject ) );
		memset( CustmObjects, 0, sizeof( GenObject ) );
#ifdef PARSEC_CLIENT
		memset( VObjList,     0, sizeof( GenObject ) );
#endif // PARSEC_CLIENT

	} else {

		// never happens
		ASSERT( 0 );
	}
}

// create virtual (type-only) instance of custom object -----------------------
//
CustomObject *E_World::CreateVirtualObject( dword objtypeid, dword dwOwner )
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
	newinstance->HostObjNumber = CreateGlobalObjId( NextObjNumber, dwOwner );
	NextObjNumber++;

	// invoke constructor
	if ( newinstance->callback_instant != NULL ) {
		(*newinstance->callback_instant)( newinstance );
	}

	// append object at head of list
	newinstance->NextObj  = CustmObjects->NextObj;
	CustmObjects->NextObj = newinstance;

	return newinstance;
}

// create object of class at position with orientation
GenObject* E_World::CreateObject( int objclass, const Xmatrx startmatrx, dword dwOwner )
{
    #ifdef HACKED_SHIPOBJECT_INSTANTIATION

	GenObject* newinstance = (ShipObject*)ALLOCMEM( sizeof( ShipObject ) );
	return newinstance;

#else // !HACKED_SHIPOBJECT_INSTANTIATION

	ASSERT( objclass >= 0 );
	//ASSERT( objclass < NumObjClasses );

	// check if class number is valid
	//if ( ( objclass < 0 ) || ( objclass >= NumObjClasses ) )
	//	return NULL;

	size_t allocinstancesize = ObjClasses[ objclass ]->InstanceSize;

#ifdef INSTANTIATE_FACE_STRUCTURES

	// reserve space for copy of facelist
	allocinstancesize += ObjClasses[ objclass ]->NumFaces * sizeof( Face );

#endif // INSTANTIATE_FACE_STRUCTURES

	// create new object header
	GenObject *newinstance = (GenObject *) ALLOCMEM( allocinstancesize );
	if ( newinstance == NULL ) {
		OUTOFMEM( "no mem for object instance." );
	}

	// fill header with class data
	memcpy( newinstance, ObjClasses[ objclass ], ObjClasses[ objclass ]->InstanceSize );
	OBJ_CorrectObjectInstance( newinstance, ObjClasses[ objclass ] );

#ifdef INSTANTIATE_FACE_STRUCTURES

	// make copy of facelist
	Face *instfacelist = (Face *) ( (char*)newinstance + newinstance->InstanceSize );
	memcpy( instfacelist, newinstance->FaceList,newinstance->NumFaces * sizeof( Face ) );
	newinstance->FaceList = instfacelist;

#endif // INSTANTIATE_FACE_STRUCTURES

#endif // !HACKED_SHIPOBJECT_INSTANTIATION

	// set object number
	//newinstance->ObjectNumber  = NextObjNumber;
	newinstance->HostObjNumber = CreateGlobalObjId( NextObjNumber, dwOwner );
	// no distinction between local and global id
	newinstance->ObjectNumber  = newinstance->HostObjNumber;
	NextObjNumber++;

	// set start matrix
	memcpy( newinstance->ObjPosition, startmatrx, sizeof( Xmatrx ) );

	// link header into list of existing objects of this type
	GenObject *listhead;
	switch ( newinstance->ObjectType & TYPELISTMASK ) {

		case PSHIP_LIST_NO:
			listhead = PShipObjects;
#ifdef PARSEC_CLIENT

			listhead = ( PShipObjects->NextObj == MyShip ) ? PShipObjects->NextObj : PShipObjects;
			// create stargate for newly created spaceship
			//FIXME: gamecode
			if ( !disable_ship_creation_stargate ) {
				SFX_CreateStargate( (ShipObject *) newinstance );
			}
#endif // PARSEC_CLIENT
			break;

		case LASER_LIST_NO:
			listhead = LaserObjects;
			break;

		case MISSL_LIST_NO:
			listhead = MisslObjects;
#ifdef PARSEC_CLIENT

			// create missile trail
			CreateVaporTrail( newinstance );

#endif // PARSEC_CLIENT
			break;

		case EXTRA_LIST_NO:
			listhead = ExtraObjects;
#ifdef PARSEC_CLIENT
			// create generation animation
			SFX_CreateExtra( (ExtraObject *) newinstance );
			// for extras maintain count
#endif // PARSEC_CLIENT
			m_nCurrentNumExtras++;
            break;

		case CUSTM_LIST_NO:
			listhead = CustmObjects;
			// for custom objects invoke constructor
			_InvokeCustomConstructor( newinstance );
			break;

		default:
			PANIC( 0 );
	}

#ifdef PARSEC_CLIENT
	// attach static particles that are
	// part of the object instance
	if ( AUX_ATTACH_OBJECT_PARTICLES ) {
		//FIXME: gamecode
		OBJ_AttachClassParticles( newinstance );
	}
#endif // PARSEC_CLIENT

	// append object at head of list
	newinstance->NextObj = listhead->NextObj;
	listhead->NextObj	 = newinstance;

	return newinstance;
}

//Particle land
// allocate new particle cluster and insert into list -------------------------
//
// init particle system -------------------------------------------------------
//
void E_World::InitParticleSystem()
{
	ASSERT( !particle_init_done );
    
	if ( !particle_init_done ) {
        
		// create single element for dummy head and tail
		if ( ( Particles = (pcluster_s *) ALLOCMEM( sizeof( pcluster_s ) ) ) == NULL )
			OUTOFMEM( 0 );
        
		// set pointers thusly
		Particles->next  = (pcluster_s *) ( (char*)Particles + sizeof( pcluster_s* ) );
		Particles->prec  = NULL;
		Particles->rep   = (particle_s *) Particles;
        
		// initially there are no elements and no allocated storage
		Particles->numel    = 0;
		Particles->maxnumel = 0;
        
	    
		particle_init_done = TRUE;
	}
}

pcluster_s * E_World::PRT_NewCluster (dword	type,int numelements, size_t auxstorage)
{
	ASSERT( numelements > 0 );
    
	// calc size of cluster header according to type
	int headsiz;
	switch ( type & CT_TYPEMASK ) {
            
		case CT_CONSTANT_VELOCITY:
			headsiz = sizeof( linear_pcluster_s );
			break;
            
		case CT_LIGHTNING:
			headsiz = sizeof( lightning_pcluster_s );
			break;
            
		case CT_OBJECTCENTERED_SPHERE:
			headsiz = sizeof( basesphere_pcluster_s );
			break;
            
        case CT_PHOTON_SPHERE:
			headsiz = sizeof( photon_sphere_pcluster_s );
			break;
            
        case CT_PARTICLE_SPHERE:
			headsiz = sizeof( sphereobj_pcluster_s );
			break;
            
		case CT_CALLBACK_TRAJECTORY:
			headsiz = sizeof( callback_pcluster_s );
			break;
            
		case CT_CUSTOMDRAW:
			headsiz = sizeof( customdraw_pcluster_s );
			break;
            
		case CT_GENOBJECT_PARTICLES:
			headsiz = sizeof( genobject_pcluster_s );
			break;
            
		default:
			PANIC( 0 );
	}
    
	pcluster_s *temp = (pcluster_s *) ALLOCMEM( headsiz );
	if ( temp == NULL )
		OUTOFMEM( 0 );
    
	// clear cluster header (including inherited fields!)
	memset( temp, 0, headsiz );
    
	// insert cluster at head of cluster list
	temp->next = Particles->next;
	temp->prec = Particles;
	temp->next->prec = temp;
	temp->prec->next = temp;
    
	// size of basic particle structures
	size_t storesiz = sizeof( particle_s ) * numelements;
    
	// size of extinfo if desired
	if ( type & CT_EXTINFO_STORAGE )
		storesiz += sizeof( pextinfo_s ) * numelements;
    
	// size of auxiliary info
	size_t auxofs = storesiz;
	if ( auxstorage > 0 )
		storesiz += auxstorage + sizeof( pusrinfo_s );
    
	// allocate storage for elements of cluster
	temp->rep	   = (particle_s *) ALLOCMEM( storesiz );
	temp->type	   = type;
	temp->numel	   = 0;
	temp->maxnumel = numelements;
    
	ASSERT( temp->rep != NULL );
	if ( temp->rep == NULL )
		OUTOFMEM( 0 );
    
	// init particle storage of cluster to zero
	memset( temp->rep, 0, storesiz );
    
	// init auxstorage header if allocated
	if ( auxstorage > 0 ) {
		pusrinfo_s *uinfo = (pusrinfo_s *) ( (char*)temp->rep + auxofs );
        
		uinfo->infovalid = FALSE;
		uinfo->blocksize = auxstorage;
        
		temp->userinfo = uinfo;
	}
    
	// add to global maximum number of elements
	Particles->maxnumel += numelements;
    
	// caller has to cast this pointer to its actual type!
	return temp;
}

// set members of particle structure and write into preexisting cluster -------
//
void E_World::PRT_InitClusterParticle (pcluster_s*	cluster,int	pid,int	bitmap,int color, int sizebound,float refz,Vertex3* position,Vector3*	velocity,int lifetime,int owner,pextinfo_s*	extinfo)
{
	ASSERT( pid < cluster->maxnumel );
    
	//NOTE:
	// the specified cluster member (particle)
	// simply gets overwritten!!
    
	PRT_InitParticle(cluster->rep[ pid ],color,sizebound,refz,position,velocity,lifetime,owner,extinfo );
    
	// increase number of elements
	cluster->numel++;
	Particles->numel++;
}

// set members of particle structure ------------------------------------------
//
void E_World::PRT_InitParticle (particle_s&	particle,int	color,int sizebound,float refz,Vertex3* position,Vector3* velocity, int lifetime,int	owner,pextinfo_s* extinfo)

{
	ASSERT( position != NULL );
    
	particle.owner     = owner;
	particle.flags     = PARTICLE_ACTIVE;
	particle.lifetime  = lifetime;
	particle.extinfo   = extinfo;
	//particle.bitmap    = bitmap;
	particle.color     = color;
	particle.sizebound = sizebound;
	particle.ref_z	   = refz;
	particle.position  = *position;
    
	if ( velocity ) {
		particle.velocity = *velocity;
	} else {
		particle.velocity.X = particle.velocity.Y = particle.velocity.Z = 0;
	}
}

// create new particle with linear animation (insert into next free slot) -----
//
particle_s * E_World::PRT_CreateLinearParticle (particle_s&	particle) {
	// try to reuse already allocated cluster
	if ( CurLinearCluster != NULL ) {
        
		ASSERT( ( CurLinearCluster->type & CT_TYPEMASK ) == CT_CONSTANT_VELOCITY );
        
		if ( CurLinearCluster->numel < CurLinearCluster->maxnumel ) {
            
			// copy particle struct into available slot in free cluster
			particle_s *pmem = CurLinearCluster->rep + CurLinearCluster->numel;
			*pmem = particle;
            
			// check whether extinfo attached
			if ( particle.extinfo != NULL ) {
                
				if ( ( CurLinearCluster->type & CT_EXTINFO_STORAGE ) == 0 ) {
					ASSERT( 0 );
					goto allocnew;
				}
                
				// copy over extinfo
				pextinfo_s *curextinfo =
                (pextinfo_s *)( CurLinearCluster->rep + CurLinearCluster->maxnumel );
				curextinfo += CurLinearCluster->numel;
				memcpy( curextinfo, particle.extinfo, sizeof( pextinfo_s ) );
                
				// set new extinfo pointer
				pmem->extinfo = curextinfo;
			}
            
			// increase number of cluster elements
			CurLinearCluster->numel++;
            
			// return new particle location
			return pmem;
		}
	}
    
allocnew:
    
	// always allocate clusters with storage for extinfo!
	dword clustertype = CT_CONSTANT_VELOCITY | CT_EXTINFO_STORAGE;
	int   numelements = DEFAULT_CLUSTER_SIZE;
    
	// create new cluster for linear particles
	CurLinearCluster = PRT_NewCluster( clustertype, numelements, 0 );
    
	// set callback to default
	linear_pcluster_s *lincluster = (linear_pcluster_s *) CurLinearCluster;
	ASSERT( lincluster->callback == NULL );
	lincluster->callback = LinearParticleCollision;
    //NOTE:
	// field bdsphere is zero.
    
	// insert recursively (tail rec)
	return PRT_CreateLinearParticle( particle );
}

void E_World::PAN_AnimateParticles()
{
	// walk list of clusters
	for ( pcluster_s *cluster = Particles->next; cluster->next; ) {
        
		// save next pointer to allow animation functions deletion of clusters
		pcluster_s *nextcluster = cluster->next;
        
		// behavior animation:
		// call trajectory animation function according to type identifier
		if ( ( cluster->type & CT_HINT_NO_POSITIONAL_ANIMATION ) == 0 ) {
			AnimateClusterBehavior( cluster );
		}
        
        TheGameCollDet->CheckEnergyField(cluster);
		// advance to next cluster (previously remembered)
		cluster = nextcluster;
        
#if defined( DEBUG ) && defined( PROTECT_ANIM_CLUSTER_WALK )
        
		//NOTE:
		// if an animation function has killed a cluster that was not
		// the cluster the function has been called for, the pointer
		// to the next cluster in the list may already be invalid.
        
		// assert that next cluster is still part of the list
		pcluster_s *tcl;
		for ( tcl = Particles->next; tcl; tcl = tcl->next )
			if ( tcl == cluster )
				break;
		ASSERT( tcl != NULL );
        
#endif
        
	}
}

// particle behavior animation: call trajectory animation function ------------
//
void E_World::AnimateClusterBehavior( pcluster_s *cluster )
{
	ASSERT( cluster != NULL );
    
	// call trajectory animation function according to type identifier
	switch ( cluster->type & CT_TYPEENUMERATIONMASK ) {
            
		case ( CT_CONSTANT_VELOCITY & CT_TYPEENUMERATIONMASK ):
            CalcConstantVelocityAnimation( (linear_pcluster_s *) cluster );
			break;
            
		case ( CT_LIGHTNING & CT_TYPEENUMERATIONMASK ):
			CalcLightningAnimation( (lightning_pcluster_s *) cluster );
			break;
            
		case ( CT_OBJECTCENTERED_SPHERE & CT_TYPEENUMERATIONMASK ):
		//	CalcObjectCenteredSphereAnimation( (basesphere_pcluster_s *) cluster );
			break;
            
		case ( CT_PHOTON_SPHERE & CT_TYPEENUMERATIONMASK ):
			// already called in G_SUPP::MaintainDurationWeapons()
			//CalcPhotonSphereAnimation( (photon_sphere_pcluster_s *) cluster );
			break;
            
        case ( CT_PARTICLE_SPHERE & CT_TYPEENUMERATIONMASK ):
			CalcSphereObjectAnimation( (sphereobj_pcluster_s *) cluster );
			break;
            
		case ( CT_CALLBACK_TRAJECTORY & CT_TYPEENUMERATIONMASK ):
			CalcCallbackTrajectoryAnimation( (callback_pcluster_s *) cluster );
			break;
            
		case ( CT_CUSTOMDRAW & CT_TYPEENUMERATIONMASK ):
		//	CalcCustomDrawAnimation( (customdraw_pcluster_s *) cluster );
			break;
            
		case ( CT_GENOBJECT_PARTICLES & CT_TYPEENUMERATIONMASK ):
		//	CalcGenObjectAnimation( (genobject_pcluster_s *) cluster );
			break;
         
	}
}

void E_World::CalcLightningAnimation( lightning_pcluster_s *cluster )
{
	ASSERT( cluster != NULL );
	ASSERT( ( cluster->type & CT_TYPEMASK ) == CT_LIGHTNING );
    
	if ( ( cluster->framecount -= TheSimulator->GetThisFrameRefFrames() ) < 0 ) {
		cluster->framecount = cluster->sizzlespeed;
        
		GenObject *shippo = cluster->baseobject;
		ASSERT( shippo != NULL );
		if ( !OBJECT_TYPE_SHIP( shippo ) ) {
            MSGOUT("E_WORLD::CalcLightningAnimation() 1130: Erroneous lightning particles detected and deleted --CrazySpence debug");
            PRT_DeleteCluster( cluster );
            return;
        }
        
		Xmatrx tmatrx;
#if ( CT_LIGHTNING & CT_GENOBJECTRELATIVE_OBJ_MASK )
		MakeIdMatrx( tmatrx );
#else
		MakeNonTranslationMatrx( &shippo->ObjPosition, &tmatrx );
#endif
        
		Vertex3 startpos;
        
#if ( CT_LIGHTNING & CT_GENOBJECTRELATIVE_OBJ_MASK )
		startpos = cluster->beamstart1;
#else
		MtxVctMUL( shippo->ObjPosition, &cluster->beamstart1, &startpos );
#endif
		SetLightningParticlePosition( cluster, cluster->rep, startpos, tmatrx );
        
#if ( CT_LIGHTNING & CT_GENOBJECTRELATIVE_OBJ_MASK )
		startpos = cluster->beamstart2;
#else
		MtxVctMUL( shippo->ObjPosition, &cluster->beamstart2, &startpos );
#endif
		SetLightningParticlePosition( cluster, cluster->rep + LIGHTNING_LENGTH, startpos, tmatrx );
	}
}
// flags
//#define ENERGYFIELD_DESTRUCTION_MESSAGE
#define DO_BRUTEFORCE_CDETECTION	// do collision detection for lightning
#define PROTECT_ANIM_CLUSTER_WALK



// lightning segment deviation/variation
#define MAX_LIGHTNING_SEG_DEVIATION			1120
#define LIGHTNING_SEG_DEVIATION_FAC			0x10

// reference z of lightning impact particles
#define IMPACT_PARTICLES_REF_Z				3.6f

void E_World::SetLightningParticlePosition( lightning_pcluster_s *cluster, particle_s particles[], Vertex3& current, Xmatrx tmatrx )
{
	ASSERT( cluster != NULL );
    
	// set position of initial particle
	particles[ 0 ].position.X = current.X;
	particles[ 0 ].position.Y = current.Y;
	particles[ 0 ].position.Z = current.Z;
    
	// iteratively calculate position of subsequent particles
	for ( int pid = 1; pid < LIGHTNING_LENGTH; pid++ ) {
        
		Vertex3 deviation;
		deviation.X = FIXED_TO_GEOMV(
                                     ( (long)( RAND() % MAX_LIGHTNING_SEG_DEVIATION ) -
                                      MAX_LIGHTNING_SEG_DEVIATION / 2 ) *
                                     LIGHTNING_SEG_DEVIATION_FAC );
		deviation.Y = FIXED_TO_GEOMV(
                                     ( (long)( RAND() % MAX_LIGHTNING_SEG_DEVIATION ) -
                                      MAX_LIGHTNING_SEG_DEVIATION / 2 ) *
                                     LIGHTNING_SEG_DEVIATION_FAC );
		deviation.Z = FLOAT_TO_GEOMV( 1.0 );
        
		Vertex3 segvec;
		MtxVctMUL( tmatrx, &deviation, &segvec );
        
		current.X += segvec.X * 2;
		current.Y += segvec.Y * 2;
		current.Z += segvec.Z * 2;
        
		particles[ pid ].position.X = current.X;
		particles[ pid ].position.Y = current.Y;
		particles[ pid ].position.Z = current.Z;
        
#ifdef DO_BRUTEFORCE_CDETECTION
        
		ASSERT( cluster->baseobject != NULL );
        
		Vertex3 cpos;
		MtxVctMUL( cluster->baseobject->ObjPosition, &particles[ pid ].position, &cpos );
        
		if ( TheGameCollDet->CheckLightningParticleShipCollision( cpos, particles[ pid ].owner ) ) {
			while ( pid < LIGHTNING_LENGTH ) {
				particles[ pid ].flags &= ~PARTICLE_ACTIVE;
				pid++;
			}
			return;
		}
        
		particles[ pid ].flags |= PARTICLE_ACTIVE;
#endif
        
	}
}

// calculate position advance of constant velocity particles ------------------
//
void E_World::CalcConstantVelocityAnimation( linear_pcluster_s *cluster )
{
	ASSERT( cluster != NULL );
	ASSERT( ( cluster->type & CT_TYPEMASK ) == CT_CONSTANT_VELOCITY );
    
	// advance all particles in cluster along their
	// local velocity vectors and maintain lifetime
	int numactive = 0;
	for ( int curp = 0; curp < cluster->numel; curp++ ) {
        
		// skip already inactive particles
		if ( ( cluster->rep[ curp ].flags & PARTICLE_ACTIVE ) == 0 ) {
			continue;
		} else {
			numactive++;
		}
        
		// maintain lifetime
		if ( ( cluster->rep[ curp ].lifetime -= TheSimulator->GetThisFrameRefFrames() ) < 0 )
			DisableParticle( cluster, curp );
        
		// move along vector
		Vector3 advvec;
		advvec.X = cluster->rep[ curp ].velocity.X * TheSimulator->GetThisFrameRefFrames();
		advvec.Y = cluster->rep[ curp ].velocity.Y * TheSimulator->GetThisFrameRefFrames();
		advvec.Z = cluster->rep[ curp ].velocity.Z * TheSimulator->GetThisFrameRefFrames();
        
		cluster->rep[ curp ].position.X += advvec.X;
		cluster->rep[ curp ].position.Y += advvec.Y;
		cluster->rep[ curp ].position.Z += advvec.Z;
        
		// invoke callback for cluster/particle
		if ( cluster->rep[ curp ].flags & PARTICLE_COLLISION ) {
			if ( cluster->callback != NULL ) {
                (*cluster->callback)( cluster, curp );
			}
		}
	}
    
	// remove cluster if no active particles contained anymore
	if ( numactive == 0 ) {
		PRT_DeleteCluster( cluster );
	}
}

// disable a single particle in a specified cluster ---------------------------
//
void E_World::DisableParticle( pcluster_s *cluster, int pid )
{
	//NOTE:
	// individual particle lifetime is currently only
	// used by linear and customdraw particles. all other
	// clusters maintain only wholesale cluster lifetime.
    
	ASSERT( cluster != NULL );
	ASSERT( ( ( cluster->type & CT_TYPEMASK ) == CT_CONSTANT_VELOCITY ) ||
           ( ( cluster->type & CT_TYPEMASK ) == CT_CUSTOMDRAW ) );
   	cluster->rep[ pid ].flags &= ~PARTICLE_ACTIVE;
}

// delete entire particle cluster ---------------------------------------------
//
void E_World::PRT_DeleteCluster (pcluster_s* cluster)
{
	ASSERT( cluster != NULL );
    
	// maintain global cluster pointers
	if ( CurLinearCluster == cluster )
		CurLinearCluster = NULL;
	if ( CustomDrawCluster == cluster )
		CustomDrawCluster = NULL;
    
	// if cluster is object-relative remove from list if contained
	if ( cluster->type & CT_GENOBJECTRELATIVE_OBJ_MASK )
		PRT_RemoveClusterFromAttachedList( (objectbase_pcluster_s *) cluster );
    
	// sub from global current/maximum number of elements
	Particles->numel	-= cluster->numel;
	Particles->maxnumel -= cluster->maxnumel;
    
	// unlink cluster from list
	cluster->prec->next = cluster->next;
	cluster->next->prec = cluster->prec;
    
	// free storage (cluster header and particle storage)
	FREEMEM( cluster->rep );
	FREEMEM( cluster );
    MSGOUT("E_World::PRT_DeleteCluster(): Free'd particle cluster");
}

// remove cluster from its attachment list if contained in any ----------------
//
void E_World::PRT_RemoveClusterFromAttachedList (objectbase_pcluster_s* cluster)
{
	ASSERT( cluster != NULL );
	ASSERT( cluster->type & CT_GENOBJECTRELATIVE_OBJ_MASK );
	ASSERT( cluster->baseobject != NULL );
    
	objectbase_pcluster_s *scan		= cluster->baseobject->AttachedPClusters;
	objectbase_pcluster_s *precnode = NULL;
    
	for ( ; scan; scan = scan->attachlist ) {
        
		ASSERT( scan->type & CT_GENOBJECTRELATIVE_OBJ_MASK );
		ASSERT( scan->baseobject == cluster->baseobject );
        
		if ( scan == cluster ) {
            
			if ( precnode )
				precnode->attachlist = scan->attachlist;
			else
				cluster->baseobject->AttachedPClusters = scan->attachlist;
            
			return;
		}
        
		precnode = scan;
	}
}

// create particle object comprising a sphere ---------------------------------
//
sphereobj_pcluster_s*
E_World::PRT_CreateParticleSphereObject (
                                
                                Vertex3&	origin,			// origin of particle object
                                geomv_t 	radius,			// radius of sphere (also used for bounding)
                                int 		animtype,		// animation type (SAT_xx)
                                int 		clustersiz,		// cluster size (number of particles in sphere)
                                int 		lifetime,		// lifetime of sphere (each sphere particle)
                                pdrwinfo_s*	pdinfo,			// particle appearance (drawing) info
                                int			owner			// owner id (remote player id)
                                )

{
	// check if animation type is allowed for particle sphere objects
	if ( ( animtype & SAT_VALID_FOR_PSPHERE_OBJECT  ) == 0 ) {
		ASSERT( 0 );
		return NULL;
	}
    
	// fetch pdefinfo if supplied
    int bitmapindx = pdinfo ? pdinfo->bmindx  : 1;
	int pcolor	   = pdinfo ? pdinfo->pcolor  : SPHERE_PARTICLE_COLOR;
	float refz   = pdinfo ? pdinfo->ref_z   : sphere_ref_z;
	int sizebound  = pdinfo ? pdinfo->sizebnd : partbitmap_size_bound;
    
	// determine sphere's shape
	int spheretype = animtype & SAT_SPHERE_TYPE_MASK;
    
	// determine number of cluster elements
	int allocsiz = ( animtype & SAT_NEEDS_REFCOORDS_MASK ) ?
    clustersiz * 2 : clustersiz;
    
	// fetch extinfo if supplied
	pextinfo_s *extinfo = pdinfo ? pdinfo->extinfo : NULL;
    
	// determine cluster hints
	dword hints = CT_HINT_PARTICLES_IDENTICAL | CT_CLUSTER_GLOBAL_EXTINFO;
    
	// determine cluster type
	dword clustertype = CT_PARTICLE_SPHERE | hints;
	if ( extinfo != NULL ) {
		clustertype |= CT_EXTINFO_STORAGE | CT_HINT_PARTICLES_HAVE_EXTINFO;
	}
    
	// create new cluster
	sphereobj_pcluster_s *cluster =	(sphereobj_pcluster_s *)
    PRT_NewCluster( clustertype, allocsiz, 0 );
    
	// fill in basic fields
	cluster->bdsphere = radius;
	cluster->origin   = origin;
	cluster->animtype = animtype;
	cluster->lifetime = lifetime;
	cluster->max_life = lifetime;
    
	// fill in additional fields
	switch ( animtype & SAT_BASIC_ANIM_MASK ) {
            
		case SAT_ROTATING:
			cluster->rot.pitch		 = SPHERE_ROT_PITCH;
			cluster->rot.yaw		 = SPHERE_ROT_YAW;
			cluster->rot.roll		 = SPHERE_ROT_ROLL;
			break;
            
		case SAT_EXPLODING:
			cluster->expl.speed		 = SPHERE_EXPLOSION_SPEED;
			break;
            
		case SAT_PULSATING:
			cluster->puls.amplitude	 = SPHERE_PULSE_AMPLITUDE;
			cluster->puls.midradius	 = radius;
			cluster->puls.frequency  = SPHERE_PULSE_FREQUENCY;
			cluster->puls.current_t  = BAMS_DEG0;
			cluster->puls.pitch		 = SPHERE_ROT_PITCH;
			cluster->puls.yaw		 = SPHERE_ROT_YAW;
			cluster->puls.roll		 = SPHERE_ROT_ROLL;
			break;
            
		case SAT_CONTRACTING:
			cluster->cont.speed	     = SPHERE_CONTRACT_SPEED;
			cluster->cont.expandtime = CONTRACTING_SPHERE_EXPANSION_TIME;
			cluster->cont.pitch	     = SPHERE_ROT_PITCH;
			cluster->cont.yaw        = SPHERE_ROT_YAW;
			cluster->cont.roll       = SPHERE_ROT_ROLL;
			break;
	}
    
	// set particle properties
	int curp = 0;
	for ( curp = 0; curp < clustersiz; curp++ ) {
		Vertex3 particlepos;
		CalcSphereParticlePosition( particlepos, radius, spheretype );
        
		// copy extinfo into cluster
		pextinfo_s *curextinfo = NULL;
		if ( extinfo != NULL ) {
			curextinfo = (pextinfo_s *)( cluster->rep + allocsiz ) + curp;
			memcpy( curextinfo, extinfo, sizeof( pextinfo_s ) );
		}
        
		// init particle in cluster
		PRT_InitClusterParticle( cluster, curp, bitmapindx, pcolor,
                                sizebound, refz,
                                &particlepos, NULL,
                                INFINITE_LIFETIME, owner,
                                curextinfo );
	}
    
	// make second copy of particles if required by animation type
	if ( animtype & SAT_NEEDS_REFCOORDS_MASK ) {
		for ( int refp = 0; curp < clustersiz * 2; curp++, refp++ ) {
			cluster->rep[ curp ] = cluster->rep[ refp ];
		}
	}
    
	//NOTE:
	// the optional second copy of all particles is not drawn
	// because the number of cluster elements is not increased
	// for them. (the particles are simply copied into the cluster!)
	// therefore, this block of particles is inactive and invisible
	// and can safely be used for reference purposes.
	// note also that the duplicate extinfo pointers pose no problem
	// for the exact same reason.
    
	return cluster;
    
}

// calc init position for particle stochastically placed on spherical surface -
//
void E_World::CalcSphereParticlePosition( Vertex3& position, geomv_t radius, int spheretype )
{
	Vertex3 sphereloc;
	sphereloc.X = 0;
	sphereloc.Y = 0;
	sphereloc.Z = radius;
    
	Xmatrx rotmatrx;
	MakeIdMatrx( rotmatrx );
    
	if ( spheretype == SAT_SPHERETYPE_NORMAL ) {
        
		bams_t pitch = RAND() * 2 - BAMS_DEG180;
		bams_t yaw	 = RAND() * 2 - BAMS_DEG180;
        
		CamRotX( rotmatrx, pitch );
		CamRotY( rotmatrx, yaw );
        
	} else if ( spheretype == SAT_SPHERETYPE_DISC ) {
        
		int r = ( RAND() >> 10 ) & 1;
        
		if ( r == 0 ) {
            
			sphereloc.Z = FIXED_TO_GEOMV( RAND() * 0x03 + 0x0c0000 );
            
			bams_t pitch = RAND() * 2 - BAMS_DEG180;
			CamRotX( rotmatrx, pitch );
            
		} else {
            
			sphereloc.Z = FIXED_TO_GEOMV( RAND() * 0x03 + 0x150000 );
            
			bams_t pitch = RAND() * 2 - BAMS_DEG180;
			CamRotX( rotmatrx, pitch );
            
		}
        
	} else if ( spheretype == SAT_SPHERETYPE_DISCWITHCORE ) {
        
		int r = RAND() & 3;
        
		if ( r == 0 ) {
            
			sphereloc.Z = FIXED_TO_GEOMV( RAND() * 0x03 + 0x0c0000 );
            
			bams_t pitch = RAND() * 2 - BAMS_DEG180;
			CamRotX( rotmatrx, pitch );
            
		} else if ( r == 1 ) {
            
			sphereloc.Z = FIXED_TO_GEOMV( RAND() * 0x03 + 0x150000 );
            
			bams_t pitch = RAND() * 2 - BAMS_DEG180;
			CamRotX( rotmatrx, pitch );
            
		} else {
            
			bams_t pitch = RAND() * 2 - BAMS_DEG180;
			bams_t yaw	 = RAND() * 2 - BAMS_DEG180;
            
			CamRotX( rotmatrx, pitch );
			CamRotY( rotmatrx, yaw );
		}
        
	}
    
	MtxVctMUL( rotmatrx, &sphereloc, &position );
}

// create an energy field -----------------------------------------------------
//
void E_World::SFX_CreateEnergyField( Vertex3& origin )
{
	// some particle appearance properties
	int bitmap	  = 1;
	int color	  = SPHERE_PARTICLE_COLOR;
    extern float sphere_ref_z;
	float ref_z = sphere_ref_z;
	int sizebound = PRTSB_ENERGYFIELD;
	int lifetime  = ENERGY_FIELD_LIFETIME;
    
	pextinfo_s *pextinfo = NULL;
    	   
	pdrwinfo_s drawinfo;
	drawinfo.bmindx  = bitmap;
	drawinfo.pcolor  = color;
	drawinfo.ref_z   = ref_z;
	drawinfo.extinfo = pextinfo;
	drawinfo.sizebnd = sizebound;
    
	PRT_CreateParticleSphereObject( origin,
                                   ENERGY_FIELD_INITIAL_RADIUS,
                                   SAT_ENERGYFIELD_SPHERE,
                                   SPHERE_PARTICLES,
                                   lifetime, &drawinfo,
                                   PLAYERID_ANONYMOUS);
   // MSGOUT("E_World::SFX_CreateEnergField() X:%.2f Y:%.2f Z:%.2f",origin.X,origin.Y,origin.Z);
	// maintain particle extra count
	m_nCurrentNumPrtExtras++;
    
}

// ----------------------------------------------------------------------------
//
int SWARM_rand()
{
	nextrand = nextrand * 1103515245 + 12345;
    
	return((nextrand >> 16) & 0x7FFF);
}


// ----------------------------------------------------------------------------
//
void SWARM_srand( unsigned int seed )
{
	nextrand = seed;
}

// ----------------------------------------------------------------------------
//
GenObject* E_World::SWARM_Init( int owner, Vertex3 *origin, ShipObject *targetpo, int randseed )
{
    static pextinfo_s extinfo;
	// set rand seed
	SWARM_srand( randseed );
    
	size_t swarmsiz = sizeof( swarm_state_s );
	swarmsiz += sizeof( Point3h_f ) * NUM_PARTICLES;
	swarmsiz += 3 * sizeof( geomv_t ) * NUM_PARTICLES * TIME_POSITIONS;
	swarmsiz += 3 * sizeof( geomv_t ) * NUM_PARTICLES;
    
	// create new particle cluster with auxstorage for swarm
	dword clustertype = CT_CALLBACK_TRAJECTORY | CT_EXTINFO_STORAGE;
    
	callback_pcluster_s *cluster =
    (callback_pcluster_s *) PRT_NewCluster( clustertype, NUM_PARTICLES, swarmsiz );
    
	ASSERT( cluster != NULL );
    
	// init custom fields
	cluster->callback = (callback_pcluster_fpt) SWARM_TimedAnimate;
    
	// set performance hints
	cluster->type |= CT_HINT_PARTICLES_IDENTICAL;
	cluster->type |= CT_HINT_PARTICLES_HAVE_EXTINFO;
	cluster->type |= CT_HINT_NO_APPEARANCE_ANIMATION;
    
	cluster->userinfo->infovalid = TRUE;
    
	char *auxstorage = (char *) &cluster->userinfo[ 1 ];
    
	// get pointer to auxstorage
	swarm_state_s *swarm = (swarm_state_s *) auxstorage;
	auxstorage += sizeof( swarm_state_s );
    
	swarm->num = NUM_PARTICLES;
    
	swarm->pos 	= (Point3h_f *) auxstorage;
	auxstorage += sizeof( Point3h_f ) * NUM_PARTICLES;
    
	swarm->x	= (geomv_t *) auxstorage;
	auxstorage += sizeof( geomv_t ) * swarm->num * TIME_POSITIONS;
    
	swarm->y	= (geomv_t *) auxstorage;
	auxstorage += sizeof( geomv_t ) * swarm->num * TIME_POSITIONS;
    
	swarm->z	= (geomv_t *) auxstorage;;
	auxstorage += sizeof( geomv_t ) * swarm->num * TIME_POSITIONS;
    
	swarm->xv	= (geomv_t *) auxstorage;;
	auxstorage += sizeof( geomv_t ) * NUM_PARTICLES;
    
	swarm->yv	= (geomv_t *) auxstorage;;
	auxstorage += sizeof( geomv_t ) * NUM_PARTICLES;
    
	swarm->zv	= (geomv_t *) auxstorage;;
    
	// store pointer to object for use in animation function
	swarm->targetpo = targetpo;
    
	// init dummy object position
	swarm->dummyobj.ObjPosition[ 0 ][ 3 ] = origin->X;
	swarm->dummyobj.ObjPosition[ 1 ][ 3 ] = origin->Y;
	swarm->dummyobj.ObjPosition[ 2 ][ 3 ] = origin->Z;
    
	// init excess time
	swarm->timerest = 0;
    
	// init target position
	swarm->tx[ 0 ] = targetpo->ObjPosition[ 0 ][ 3 ];
	swarm->ty[ 0 ] = targetpo->ObjPosition[ 1 ][ 3 ];
	swarm->tz[ 0 ] = targetpo->ObjPosition[ 2 ][ 3 ];
	swarm->tx[ 1 ] = swarm->tx[ 0 ];
	swarm->ty[ 1 ] = swarm->ty[ 0 ];
	swarm->tz[ 1 ] = swarm->tz[ 0 ];
	swarm->tx[ 2 ] = swarm->tx[ 0 ];
	swarm->ty[ 2 ] = swarm->ty[ 0 ];
	swarm->tz[ 2 ] = swarm->tz[ 0 ];
    
	// get pointer to ship of owner
	ShipObject *shippo = FetchFirstShip();
    while ( shippo && ( shippo->HostObjNumber != ShipHostObjId( owner ) ) ) {
		shippo = (ShipObject *) shippo->NextObj;
	}
    
	Vector3	dirvec;
    
	if ( shippo != NULL ) {
        
		DirVctMUL( shippo->ObjPosition, FLOAT_TO_GEOMV( 1.5f ), &dirvec );
        
	} else {
        
		dirvec.X = FLOAT_TO_GEOMV( 1.5f );
		dirvec.Y = FLOAT_TO_GEOMV( 1.5f );
		dirvec.Z = FLOAT_TO_GEOMV( 1.5f );
        
	}
    
	// init particle positions and velocities
	int pid = 0;
	for ( pid = 0; pid < swarm->num; pid++ ) {
        
		X( 0, pid ) = origin->X;
		X( 1, pid ) = X( 0, pid );
		Y( 0, pid ) = origin->Y;
		Y( 1, pid ) = Y( 0, pid );
		Z( 0, pid ) = origin->Z;
		Z( 1, pid ) = Z( 0, pid );
        
		swarm->xv[ pid ] = dirvec.X;
		swarm->yv[ pid ] = dirvec.Y;
		swarm->zv[ pid ] = dirvec.Z;
        
        //		swarm->xv[ pid ] = GEOMV_DIV( ( balance_rand( 100 ) * PARTICLE_VELOCITY ),
        //										100 * PARTICLE_VELOCITY );
        //		swarm->yv[ pid ] = GEOMV_DIV( ( balance_rand( 100 ) * PARTICLE_VELOCITY ),
        //										100 * PARTICLE_VELOCITY );
        //		swarm->zv[ pid ] = GEOMV_DIV( ( balance_rand( 100 ) * PARTICLE_VELOCITY ),
        //										100 * PARTICLE_VELOCITY );
	}
    /*
	// create particles
	pdef_s *pdef = PRT_AcquireParticleDefinition( "swarm1", NULL );
	if ( pdef == NULL ) {
		return NULL;
	}
    
	// create extinfo
	
	PRT_InitParticleExtInfo( &extinfo, pdef, NULL, NULL );
    */
	float cur_particle_resoscale = 1.0f;
    
	float ref_z = cur_particle_resoscale * PARTICLE_REFZ;
	int	bitmap    = iter_texrgba | iter_specularadd;
	int	color     = 0;
    
	// create all particles of swarm
	for ( pid = 0; pid < swarm->num; pid++ ) {
        
		Vector3 pos;
		pos.X = X( 0, pid );
		pos.Y = Y( 0, pid );
		pos.Z = Z( 0, pid );
        
		// init the particles and hook them up with cluster
		PRT_InitClusterParticle( cluster, pid, bitmap, color, PRT_NO_SIZEBOUND,
                                ref_z, &pos, NULL,
                                PARTICLE_LIFETIME + balance_rand( LIFETIME_VARIANCE ),
                                owner, &extinfo );
	}
    
	return &swarm->dummyobj;
}

// ----------------------------------------------------------------------------
//
int E_World::SWARM_Animate( callback_pcluster_s* cluster )
{
	ASSERT( cluster != NULL );
    
	// get pointer to auxstorage
	swarm_state_s *swarm = (swarm_state_s *) &cluster->userinfo[ 1 ];
    
	// age the target arrays
	swarm->tx[ 2 ] = swarm->tx[ 1 ];
	swarm->ty[ 2 ] = swarm->ty[ 1 ];
	swarm->tz[ 2 ] = swarm->tz[ 1 ];
	swarm->tx[ 1 ] = swarm->tx[ 0 ];
	swarm->ty[ 1 ] = swarm->ty[ 0 ];
	swarm->tz[ 1 ] = swarm->tz[ 0 ];
    
	// check of target is still available
	ShipObject *shippo = (ShipObject *) FetchFirstShip();
	while ( shippo && ( shippo != swarm->targetpo ) )
		shippo = (ShipObject *) shippo->NextObj;
    
	// update target position
	if ( ( shippo != NULL ) ) {
		swarm->tx[ 0 ] = swarm->targetpo->ObjPosition[ 0 ][ 3 ];
		swarm->ty[ 0 ] = swarm->targetpo->ObjPosition[ 1 ][ 3 ];
		swarm->tz[ 0 ] = swarm->targetpo->ObjPosition[ 2 ][ 3 ];
	}
    
	// avoid settling
	swarm->xv[ SWARM_rand() % swarm->num ] += balance_rand( 3 );
	swarm->yv[ SWARM_rand() % swarm->num ] += balance_rand( 3 );
	swarm->zv[ SWARM_rand() % swarm->num ] += balance_rand( 3 );
    
	int numactive = 0;
	int refposset = FALSE;
    
	// update all particles
	for ( int pid = 0; pid < swarm->num; pid++ ) {
        
		particle_s* curparticle = &cluster->rep[ pid ];
        
		// skip already inactive particles
		if ( ( curparticle->flags & PARTICLE_ACTIVE ) == 0 ) {
			continue;
		}
        
		// maintain lifetime
		if ( ( curparticle->lifetime -= ANIM_TIMESLICE ) < 0 ) {
			curparticle->flags &= ~PARTICLE_ACTIVE;
			continue;
		}
        
		// count as still active
		numactive++;
        
		// age the particle arrays
		X( 2, pid ) = X( 1, pid );
		Y( 2, pid ) = Y( 1, pid );
		Z( 2, pid ) = Z( 1, pid );
		X( 1, pid ) = X( 0, pid );
		Y( 1, pid ) = Y( 0, pid );
		Z( 1, pid ) = Z( 0, pid );
        
		Vector3 delta;
        
		// accelerate to target position
		delta.X = swarm->tx[ 1 ] - X( 1, pid );
		delta.Y = swarm->ty[ 1 ] - Y( 1, pid );
		delta.Z = swarm->tz[ 1 ] - Z( 1, pid );
        
		geomv_t distance = VctLenX( &delta );
		if ( distance == GEOMV_0 )
			distance = GEOMV_1;
        
		swarm->xv[ pid ] += GEOMV_DIV( GEOMV_MUL( delta.X, PARTICLE_ACCEL ) , distance );
		swarm->yv[ pid ] += GEOMV_DIV( GEOMV_MUL( delta.Y, PARTICLE_ACCEL ) , distance );
		swarm->zv[ pid ] += GEOMV_DIV( GEOMV_MUL( delta.Z, PARTICLE_ACCEL ) , distance );
        
		// speed limit checks
		if ( swarm->xv[ pid ] > PARTICLE_VELOCITY )
			swarm->xv[ pid ] = PARTICLE_VELOCITY;
        
		if ( swarm->xv[ pid ] < - PARTICLE_VELOCITY )
			swarm->xv[ pid ] = - PARTICLE_VELOCITY;
        
		if ( swarm->yv[ pid ] > PARTICLE_VELOCITY )
			swarm->yv[ pid ] = PARTICLE_VELOCITY;
        
		if ( swarm->yv[ pid ] < - PARTICLE_VELOCITY )
			swarm->yv[ pid ] = - PARTICLE_VELOCITY;
        
		if ( swarm->zv[ pid ] > PARTICLE_VELOCITY )
			swarm->zv[ pid ] = PARTICLE_VELOCITY;
        
		if ( swarm->zv[ pid ] < - PARTICLE_VELOCITY )
			swarm->zv[ pid ] = - PARTICLE_VELOCITY;
        
		// move
		X( 0, pid ) = X( 1, pid ) + swarm->xv[ pid ] * ANIM_TIMESLICE;
		Y( 0, pid ) = Y( 1, pid ) + swarm->yv[ pid ] * ANIM_TIMESLICE;
		Z( 0, pid ) = Z( 1, pid ) + swarm->zv[ pid ] * ANIM_TIMESLICE;
        
		// fill the position list
		swarm->pos[ pid ].X = X( 0, pid );
		swarm->pos[ pid ].Y = Y( 0, pid );
		swarm->pos[ pid ].Z = Z( 0, pid );
        
		// update particle positions
		curparticle->position.X = swarm->pos[ pid ].X;
		curparticle->position.Y = swarm->pos[ pid ].Y;
		curparticle->position.Z = swarm->pos[ pid ].Z;
        
		if ( !refposset ) {
            
			swarm->dummyobj.ObjPosition[ 0 ][ 3 ] = cluster->rep[ pid ].position.X;
			swarm->dummyobj.ObjPosition[ 1 ][ 3 ] = cluster->rep[ pid ].position.Y;
			swarm->dummyobj.ObjPosition[ 2 ][ 3 ] = cluster->rep[ pid ].position.Z;
            
			refposset = TRUE;
		}
        
		// check for collision with other ships
		ShipObject *walkships = FetchFirstShip();
		for ( ; walkships; walkships = (ShipObject*) walkships->NextObj ) {
            
			// prevent collision with owner of particle
			if ( ( GetObjectOwner( walkships ) == (dword)curparticle->owner ) )
				continue;
            
			if ( !TheGameCollDet->PRT_ParticleInBoundingSphere( walkships, curparticle->position ) )
				continue;
            
#ifdef PARTICLES_DIE_AFTER_FIRST_COLLISION
			// disable particle
			curparticle->flags &= ~PARTICLE_ACTIVE;
			numactive--;
#endif
            
			TheGameCollDet->OBJ_ShipSwarmDamage( walkships, curparticle->owner );
            
			if ( walkships->CurDamage > walkships->MaxDamage ) {
				for ( int ppid = 0; ppid < swarm->num; ppid++ ) {
                    
					refframe_t newlifetime = LIFETIME_AFTEREXPL + balance_rand( LIFETIME_VARIANCE );
                    
					if ( cluster->rep[ ppid ].lifetime >= newlifetime )
						cluster->rep[ ppid ].lifetime = newlifetime;
				}
			}
		}
	}
    
	// destory swarm if no active particles contained anymore
	if ( numactive == 0 ) {
		PRT_DeleteCluster( cluster );
		return FALSE;
	}
    
	return TRUE;
}

// invoke callback function for animation of generic callback clusters --------
//
void E_World::CalcCallbackTrajectoryAnimation( callback_pcluster_s *cluster )
{
	ASSERT( cluster != NULL );
	ASSERT( ( cluster->type & CT_TYPEMASK ) == CT_CALLBACK_TRAJECTORY );
	ASSERT( cluster->callback != NULL );
    
	// invoke callback function
	if ( cluster->callback != NULL ) {
		cluster->callback( cluster );
	}
    
	//NOTE:
	// the callback function is invoked for the
	// entire cluster, not single particles.
}

// calc animation of autonomous particle sphere -------------------------------
//
PRIVATE
void E_World::CalcSphereObjectAnimation( sphereobj_pcluster_s *cluster )
{
	ASSERT( cluster != NULL );
	ASSERT( ( cluster->type & CT_TYPEMASK ) == CT_PARTICLE_SPHERE );
    
	if ( cluster->lifetime > 0 ) {
        
		refframe_t refframes = TheSimulator->GetThisFrameRefFrames();
		if ( ( cluster->lifetime -= refframes ) < 0 ) {
			refframes += cluster->lifetime;
		}
        
		// handle optional particle auto-depletion
		if ( cluster->animtype & SAT_AUTO_DEPLETE_PARTICLES ) {
            
			ASSERT( cluster->lifetime <= cluster->max_life );
			float strength = (float)cluster->lifetime / cluster->max_life;
            
			int maxnumel = cluster->maxnumel;
			if ( cluster->animtype & SAT_NEEDS_REFCOORDS_MASK )
				maxnumel /= 2;
			int numel = (int)(maxnumel * strength);
			if ( numel < cluster->maxnumel ) {
				cluster->numel = numel;
			}
		}
        
		// perform basic animation
		switch ( cluster->animtype & SAT_BASIC_ANIM_MASK ) {
                
			case SAT_ROTATING:
            {
				bams_t pitch = cluster->rot.pitch * TheSimulator->GetThisFrameRefFrames();
				bams_t yaw   = cluster->rot.yaw   * TheSimulator->GetThisFrameRefFrames();
				bams_t roll  = cluster->rot.roll  * TheSimulator->GetThisFrameRefFrames();
                
				for ( int pid = 0; pid < cluster->numel; pid++ ) {
					CalcSphereParticleRotation( cluster->rep[ pid ].position,
                                               pitch, yaw, roll );
				}
            }
				break;
                
			case SAT_EXPLODING:
            {
				geomv_t speed = cluster->expl.speed * refframes;
                
				for ( int pid = 0; pid < cluster->numel; pid++ ) {
					CalcSphereParticleExplosion( cluster->rep[ pid ].position,
                                                speed );
				}
				// update cluster radius
				cluster->bdsphere += speed;
            }
				break;
                
			case SAT_PULSATING:
            {
				sincosval_s sincosv;
				GetSinCos( cluster->puls.current_t, &sincosv );
                
				geomv_t pulseval = GEOMV_MUL( sincosv.sinval, cluster->puls.amplitude );
                
				bams_t pitch = cluster->puls.pitch * TheSimulator->GetThisFrameRefFrames();
				bams_t yaw   = cluster->puls.yaw   * TheSimulator->GetThisFrameRefFrames();
				bams_t roll  = cluster->puls.roll  * TheSimulator->GetThisFrameRefFrames();
                
				for ( int pid = 0; pid < cluster->numel; pid++ ) {
					CalcSphereParticleRotation( cluster->rep[ pid + cluster->numel ].position,
                                               pitch, yaw, roll );
					CalcSpherePulse( cluster->rep[ pid ].position,
                                    cluster->rep[ pid + cluster->numel ].position,
                                    pulseval );
				}
				// update cluster radius
				cluster->bdsphere = cluster->puls.midradius + pulseval;
                
				cluster->puls.current_t += cluster->puls.frequency * TheSimulator->GetThisFrameRefFrames();
            }
				break;
                
			case SAT_CONTRACTING:
            {
				bams_t pitch = cluster->cont.pitch * TheSimulator->GetThisFrameRefFrames();
				bams_t yaw   = cluster->cont.yaw   * TheSimulator->GetThisFrameRefFrames();
				bams_t roll  = cluster->cont.roll  * TheSimulator->GetThisFrameRefFrames();
                
				for ( int pid = 0; pid < cluster->numel; pid++ ) {
					CalcSphereParticleRotation( cluster->rep[ pid ].position,
                                               pitch, yaw, roll );
				}
                
				if ( cluster->cont.expandtime > 0 ) {
					cluster->cont.expandtime -= TheSimulator->GetThisFrameRefFrames();
                    
					geomv_t contraction = cluster->cont.speed * TheSimulator->GetThisFrameRefFrames();
					for ( int pid = 0; pid < cluster->numel; pid++ ) {
						CalcSphereContraction( cluster->rep[ pid ].position, -contraction );
					}
					// update cluster radius
					cluster->bdsphere += contraction;
				}
                
            }
				break;
		}
        
	} else {
        
		// animation type-dependent behavior after lifetime is spent
		switch ( cluster->animtype & SAT_BASIC_ANIM_MASK ) {
                
			case SAT_CONTRACTING:
            {
				bams_t pitch = cluster->cont.pitch * TheSimulator->GetThisFrameRefFrames();
				bams_t yaw   = cluster->cont.yaw   * TheSimulator->GetThisFrameRefFrames();
				bams_t roll  = cluster->cont.roll  * TheSimulator->GetThisFrameRefFrames();
                
				for ( int pid = 0; pid < cluster->numel; pid++ ) {
					CalcSphereParticleRotation( cluster->rep[ pid ].position,
                                               pitch, yaw, roll );
				}
				int delit = 0;
                
				{
					geomv_t contraction = cluster->cont.speed * TheSimulator->GetThisFrameRefFrames();
					for ( int pid = 0; pid < cluster->numel; pid++ ) {
						delit += CalcSphereContraction( cluster->rep[ pid ].position, contraction );
					}
					// update cluster radius
					cluster->bdsphere -= contraction;
				}
                
				if ( delit ) {
                    
					if ( cluster->animtype & SAT_DECREMENT_EXTRA_COUNTER ) {
						m_nCurrentNumPrtExtras--;
      
					}
                    
					PRT_DeleteCluster( cluster );
				}
                
            }
				break;
                
			default:
				PRT_DeleteCluster( cluster );
		}
	}
}

// calc rotation of sphere particle -------------------------------------------
//
void E_World::CalcSphereParticleRotation( Vertex3& position, bams_t pitch, bams_t yaw, bams_t roll )
{
	Xmatrx rotmatrx;
	MakeIdMatrx( rotmatrx );
	CamRotX( rotmatrx, pitch );
	CamRotY( rotmatrx, yaw );
	CamRotZ( rotmatrx, roll );
    
	Vertex3 rotpos;
	MtxVctMUL( rotmatrx, &position, &rotpos );
	position = rotpos;
}

// calc contraction of sphere -------------------------------------------------
//
int E_World::CalcSphereContraction( Vertex3& position, geomv_t speed )
{
	// check for null vector
	geomv_t vx = position.X;
	geomv_t vy = position.Y;
	geomv_t vz = position.Z;
    
	ABS_GEOMV( vx );
	ABS_GEOMV( vy );
	ABS_GEOMV( vz );
    
	if ( ( vx <= GEOMV_VANISHING ) &&
        ( vy <= GEOMV_VANISHING ) &&
        ( vz <= GEOMV_VANISHING ) ) {
		return TRUE;
	}
    
	// create direction unit-vector
	Vector3 vec = position;
	NormVctX( &vec );
    
	int sign_x = ( position.X < 0 ) ? -1 : 1;
	int sign_y = ( position.Y < 0 ) ? -1 : 1;
	int sign_z = ( position.Z < 0 ) ? -1 : 1;
    
	position.X -= GEOMV_MUL( vec.X, speed );
	position.Y -= GEOMV_MUL( vec.Y, speed );
	position.Z -= GEOMV_MUL( vec.Z, speed );
    
	int asign_x = ( position.X < 0 ) ? -1 : 1;
	int asign_y = ( position.Y < 0 ) ? -1 : 1;
	int asign_z = ( position.Z < 0 ) ? -1 : 1;
    
	int collapsed = FALSE;
    
	if ( sign_x != asign_x ) collapsed = TRUE;
	if ( sign_y != asign_y ) collapsed = TRUE;
	if ( sign_z != asign_z ) collapsed = TRUE;
    
	return collapsed;
}

lightning_pcluster_s* E_World::CreateLightningParticles( ShipObject *shippo, int owner )
{
	ASSERT( shippo != NULL );
    
	// create appearance
	int bitmap	  = 1;
	int color	  = 1;
	float ref_z = lightning_ref_z;
	int sizebound = partbitmap_size_bound;
    
	// create cluster
	dword clustertype = CT_LIGHTNING;
    
	lightning_pcluster_s *cluster = (lightning_pcluster_s *)
    PRT_NewCluster( clustertype, LIGHTNING_LENGTH * 2, 0 );
    
	// fill in basic fields
	cluster->animtype	 = SAT_LIGHTNING;
	cluster->sizzlespeed = LIGHTNING_SIZZLE_SPEED;
    
	Vertex3 startpos;
    
	startpos.X = shippo->Beam_X[ 0 ];
	startpos.Y = shippo->Beam_Y;
	startpos.Z = shippo->Beam_Z;
	cluster->beamstart1 = startpos;
    
	startpos.X = shippo->Beam_X[ 3 ];
	cluster->beamstart2 = startpos;
    
	// create particles
    
	int curp = 0;

	for ( curp = 0; curp < LIGHTNING_LENGTH; curp++ )
    	PRT_InitClusterParticle( cluster, curp, bitmap, color,
                                    sizebound, ref_z,
                                    &cluster->beamstart1, NULL,
                                    INFINITE_LIFETIME, owner,
                                    NULL );
	for ( ; curp < LIGHTNING_LENGTH * 2; curp++ )
		PRT_InitClusterParticle( cluster, curp, bitmap, color,
                                    sizebound, ref_z,
                                    &cluster->beamstart2, NULL,
                                    INFINITE_LIFETIME, owner,
                                    NULL );
	// attach lightning particle cluster to ship
	PRT_AttachClusterToObject( shippo, cluster );
    
	return cluster;
}

// attach cluster to genobject (insert at head of attachment list) ------------
//
void E_World::PRT_AttachClusterToObject( GenObject* genobjpo, objectbase_pcluster_s* cluster )
{
	ASSERT( genobjpo != NULL );
	ASSERT( cluster != NULL );
	ASSERT( cluster->type & CT_GENOBJECTRELATIVE_OBJ_MASK );
    
	// set reference to genobject
	cluster->baseobject = genobjpo;
    
	// insert at head of list
	cluster->attachlist			= genobjpo->AttachedPClusters;
	genobjpo->AttachedPClusters	= cluster;
}

// delete all clusters of certain type attached to a genobject ----------------
//
int E_World::PRT_DeleteAttachedClustersOfType( const GenObject * genobjpo, int animtype )
{
	ASSERT( genobjpo != NULL );
    
	// count number of removed clusters
	int numremoved = 0;
    
	objectbase_pcluster_s *scan     = genobjpo->AttachedPClusters;
	objectbase_pcluster_s *precnode = NULL;
    
	while ( scan != NULL ) {
        
		ASSERT( scan->type & CT_GENOBJECTRELATIVE_OBJ_MASK );
        
		if ( scan->animtype == animtype ) {
            
			if ( precnode )
				precnode->attachlist = scan->attachlist;
			else
				((GenObject*)genobjpo)->AttachedPClusters = scan->attachlist;
            
			pcluster_s *temp = scan;
			scan = scan->attachlist;
            
			PRT_DeleteCluster_NoListRemoval( temp );
            
			numremoved++;
			continue;
		}
        
		precnode = scan;
		scan     = scan->attachlist;
	}
    
	// return number of removed clusters
	return numremoved;
}

// delete entire particle cluster (don't check any attachment lists) ----------
//
void E_World::PRT_DeleteCluster_NoListRemoval( pcluster_s* cluster )
{
	ASSERT( cluster != NULL );
    
	// maintain global cluster pointers
	if ( CurLinearCluster == cluster )
		CurLinearCluster = NULL;
	if ( CustomDrawCluster == cluster )
		CustomDrawCluster = NULL;
    
	// sub from global current/maximum number of elements
	Particles->numel	-= cluster->numel;
	Particles->maxnumel -= cluster->maxnumel;
    
	// unlink cluster from list
	cluster->prec->next = cluster->next;
	cluster->next->prec = cluster->prec;
    
	// free storage (cluster header and particle storage)
	FREEMEM( cluster->rep );
	FREEMEM( cluster );
}

// calculate pulsating sphere -------------------------------------------------
//
void E_World::CalcSpherePulse( Vertex3& position, Vertex3& pulsebase, geomv_t pulseval )
{
	// create direction unit-vector
	Vector3 vec = pulsebase;
	NormVctX( &vec );
    
	// pulseval is a sine or cosine function and determines
	// the deviation from the base position (radius)
	position.X = pulsebase.X + GEOMV_MUL( vec.X, pulseval );
	position.Y = pulsebase.Y + GEOMV_MUL( vec.Y, pulseval );
	position.Z = pulsebase.Z + GEOMV_MUL( vec.Z, pulseval );
}

// calc explosion trajectory of sphere particle -------------------------------
//
void E_World::CalcSphereParticleExplosion( Vertex3& position, geomv_t speed )
{
	Vector3 vec = position;
	NormVctX( &vec );
    
	position.X += GEOMV_MUL( vec.X, speed );
	position.Y += GEOMV_MUL( vec.Y, speed );
	position.Z += GEOMV_MUL( vec.Z, speed );
}

// acquire already registered particle definition via its id ------------------
//
pdef_s * E_World::PRT_AcquireParticleDefinitionById(
                                           int			pdefid			// id of particle definition
                                           )
{
	// ensure id is valid
	if ( ( pdefid < 0 ) || ( pdefid >= NumParticleDefinitions ) )
		return NULL;
    
	// fetch from table
	pdef_s *pdef = ParticleDefinitions[ pdefid ].def;
	ASSERT( pdef != NULL );
    
	return pdef;
};

// acquire already registered particle definition via its unique name ---------
//
pdef_s * E_World::PRT_AcquireParticleDefinition(
                                       const char*	pdefname,		// unique name for particle definition
                                       int*		retpdefid		// id of returned particle definition
                                       )
{
	ASSERT( pdefname != NULL );
    
	// scan all registered particle definitions
	for ( int curdef = 0; curdef < NumParticleDefinitions; curdef++ ) {
		if ( strcmp( ParticleDefinitions[ curdef ].defname, pdefname ) == 0 ) {
            
			// return id if desired
			if ( retpdefid != NULL )
				*retpdefid = curdef;
            
			// return pointer to pdef
			return ParticleDefinitions[ curdef ].def;
		}
	}
    
	return NULL;
};

// create particle sphere for photon cannon -----------------------------------
//
photon_sphere_pcluster_s * E_World::CreatePhotonSphere( ShipObject *shippo )
{
	ASSERT( shippo != NULL );
	ASSERT( OBJECT_TYPE_SHIP( shippo ) );
    
    pextinfo_s extinfo;

    /*
	// fetch pdef
	static int pdefid = -1;
	pdef_s *pdef = ( pdefid != -1 ) ? PRT_AcquireParticleDefinitionById( pdefid ) : PRT_AcquireParticleDefinition( "photon1", &pdefid );
	if ( pdef == NULL ) {
        MSGOUT( "photon particles invalid." );
        return NULL;
	}
    
	// create extinfo
    pextinfo_s extinfo;
	//PRT_InitParticleExtInfo( &extinfo, pdef, NULL, NULL );
    */
	// determine cluster type and hints
    dword clustertype = CT_PHOTON_SPHERE;
	clustertype |= CT_EXTINFO_STORAGE;
	clustertype |= CT_CLUSTER_GLOBAL_EXTINFO;
	clustertype |= CT_HINT_PARTICLES_HAVE_EXTINFO;
	clustertype |= CT_HINT_PARTICLES_IDENTICAL;
    
    // create new cluster
    int clustersiz = PHOTON_SPHERE_PARTICLES;
	photon_sphere_pcluster_s *cluster = (photon_sphere_pcluster_s *)
    PRT_NewCluster( clustertype, clustersiz, 0 );
    
	// fill in basic fields
	cluster->animtype				= SAT_PHOTON;
	cluster->bdsphere				= shippo->BoundingSphere;
	cluster->contraction_time		= PHOTON_CONTRACTION_TIME;
	cluster->cur_contraction_time	= 0;
	cluster->contraction_speed		= PHOTON_CONTRACTION_SPEED;
	cluster->max_loading_time		= PHOTON_MAX_LOADING_TIME;
	cluster->firing					= FALSE;
	cluster->numloads				= PHOTON_NUMLOADS;
	cluster->alive					= 0;
	cluster->center.X				= 0;
	cluster->center.Y				= 0;
	cluster->center.Z				= 0;
	cluster->pitch					= PHOTON_ROT_PITCH;
	cluster->yaw					= PHOTON_ROT_YAW;
	cluster->roll					= PHOTON_ROT_ROLL;
    
	// set particle properties
	for ( int curp = 0; curp < clustersiz; curp++ ) {
        
		Vertex3 particlepos;
        CalcSphereParticlePosition( particlepos, shippo->BoundingSphere, SAT_SPHERETYPE_NORMAL );
        
		// copy extinfo into cluster
		pextinfo_s *curextinfo = (pextinfo_s *)( cluster->rep + clustersiz ) + curp;
		memcpy( curextinfo, &extinfo, sizeof( pextinfo_s ) );
        
		// init particle in cluster
        cluster->rep[ curp ].owner		= GetObjectOwner( shippo );
        cluster->rep[ curp ].flags		= PARTICLE_ACTIVE;
        cluster->rep[ curp ].lifetime	= INFINITE_LIFETIME;
        cluster->rep[ curp ].extinfo	= curextinfo;
        cluster->rep[ curp ].bitmap		= iter_texrgba | iter_specularadd;
        cluster->rep[ curp ].color		= PHOTON_COLOR;
        cluster->rep[ curp ].sizebound	= PRT_NO_SIZEBOUND;
        cluster->rep[ curp ].ref_z		= photon_ref_z;
        cluster->rep[ curp ].position	= particlepos;
        cluster->rep[ curp ].velocity.X	= GEOMV_0;
		cluster->rep[ curp ].velocity.Y	= GEOMV_0;
		cluster->rep[ curp ].velocity.Z	= GEOMV_0;
        
        // increase number of elements
        Particles->numel++;
    }
    
	// at least one particle has to be visible
    cluster->numel = 1;
    
    // attach sphere's particle cluster to object
	PRT_AttachClusterToObject( shippo, (objectbase_pcluster_s *)cluster );
    
    return cluster;
}

// calc animation of photon sphere --------------------------------------------
//
void E_World::CalcPhotonSphereAnimation( photon_sphere_pcluster_s *cluster )
{
	ASSERT( cluster != NULL );
    ASSERT( ( cluster->type & CT_TYPEMASK ) == CT_PHOTON_SPHERE );
    
    ShipObject *shippo = (ShipObject *) cluster->baseobject;
        
    float part_prf = ( cluster->max_loading_time / cluster->maxnumel );
	Vertex3 old_center;
	geomv_t contraction;
    
    if ( !cluster->firing ) {
        
        // determine maximum remaining loading time
        int working_time = cluster->max_loading_time - cluster->alive;
        
        // determine actual loading time
        if ( working_time > TheSimulator->GetThisFrameRefFrames() ) {
            working_time = TheSimulator->GetThisFrameRefFrames();
        }
        cluster->alive += working_time;
        
        dword energy_consumption = shippo->CurEnergyFrac +
        ( working_time * PHOTON_ENERGY_CONSUMPTION );
        G_Player*  pPlayer = TheGame->GetPlayer( GetObjectOwner( shippo ) );
        
        // check if enough energy to build
        if ( (dword)shippo->CurEnergy < ( MIN_PHOTON_ENERGY + energy_consumption >> 16 ) ) {
            cluster->alive = (int)(cluster->alive -( working_time - ( shippo->CurEnergy - MIN_PHOTON_ENERGY ) /
                                                    FIXED_TO_FLOAT( PHOTON_ENERGY_CONSUMPTION ) ));
            shippo->CurEnergy = MIN_PHOTON_ENERGY - 1;
            pPlayer->_WFX_DeactivatePhoton();
        } else {
            shippo->CurEnergyFrac = ( energy_consumption & 0xffff );
            shippo->CurEnergy    -= ( energy_consumption >> 16 );
        }
        
        // calculate number of visible particles
        if ( ( cluster->alive < cluster->max_loading_time ) ) {
            cluster->numel = (int)( cluster->alive / part_prf );
            if ( cluster->numel == 0 ) {
                // at least one particle must be visible
                cluster->numel = 1;
            }
        }
        else {
            cluster->numel = cluster->maxnumel;
            
            //NOTE:
            // CurScreenRefFrames get added some lines below anyway,
            // so we must not add them here
            cluster->cur_contraction_time -= working_time;
            
            pPlayer->_WFX_DeactivatePhoton();
        }
    }
    
    if ( cluster->firing ) {
        
        // increase counter
        cluster->cur_contraction_time += TheSimulator->GetThisFrameRefFrames();
        
        // create linear particles
        
        // number of particles in full load
        int single_load = ( cluster->maxnumel / cluster->numloads );
        int old_numel = cluster->numel;
        
        // number of full loads to be created
        int numloads = ( cluster->cur_contraction_time / cluster->contraction_time );
        // minimize on full loads available
        if ( numloads > ( old_numel / single_load ) ) {
            numloads = ( old_numel / single_load );
        }
        
        // set particle properties
   
        int color     = PHOTON_COLOR;
        float ref_z = photon_ref_z;
        int sizebound = PRT_NO_SIZEBOUND;
        int lifetime  = shippo->PhotonLifeTime;
        int playerid  = GetObjectOwner( shippo );
        
        pextinfo_s extinfo;

        /*
        // fetch pdef
        static int pdefid = -1;
        pdef_s *pdef = ( pdefid != -1 ) ?
        PRT_AcquireParticleDefinitionById( pdefid ) :
        PRT_AcquireParticleDefinition( "photon1", &pdefid );
        
        // create extinfo
        pextinfo_s extinfo;
       // PRT_InitParticleExtInfo( &extinfo, pdef, NULL, NULL );
        */
        // set speed and direction vector
        Vector3 dirvec;
        fixed_t speed = shippo->PhotonSpeed + shippo->CurSpeed;
        DirVctMUL( shippo->ObjPosition, FIXED_TO_GEOMV( speed ), &dirvec );
        
        // radius of load
        geomv_t radius = shippo->BoundingSphere - GEOMV_MUL( cluster->contraction_speed, cluster->contraction_time );
        
        fixed_t timefrm;
        fixed_t timepos;
        Vertex3 object_space;
		int cur_load = 0;
        
        // create full loads
        for ( cur_load = 0; cur_load < numloads; cur_load++ ) {
			timefrm = cluster->cur_contraction_time - ( ( cur_load + 1 ) * cluster->contraction_time );
			timepos = timefrm * shippo->PhotonSpeed;
            
			// create one full frame set back because the current frame will
			// be added by the linear particle animation code in the same frame
			timepos -= speed * TheSimulator->GetThisFrameRefFrames();
            
            for ( int i = 0 ; i < single_load; i++ ) {
                CalcSphereParticlePosition( object_space, radius, SAT_SPHERETYPE_NORMAL );
                object_space.Z += FIXED_TO_GEOMV( 0x10000 * cluster->contraction_time )
                + FIXED_TO_GEOMV( timepos );
                
                Vertex3 world_space;
                MtxVctMUL( shippo->ObjPosition, &object_space, &world_space );
                
                particle_s particle;
                PRT_InitParticle( particle, color, sizebound,
                                 ref_z, &world_space, &dirvec,
                                 lifetime, playerid, &extinfo );
                particle.flags |= PARTICLE_COLLISION;
                particle.flags |= PARTICLE_IS_PHOTON;
                PRT_CreateLinearParticle( particle );
            }
        }
     
        // check if last load should be created
        if ( ( cluster->cur_contraction_time / cluster->contraction_time ) > ( old_numel / single_load ) ) {
			timefrm = cluster->cur_contraction_time - ( ( cur_load + 1 ) * cluster->contraction_time );
			timepos = timefrm * shippo->PhotonSpeed;
            
			// create one full frame set back because the current frame will
			// be added by the linear particle animation code in the same frame
			timepos -= speed * TheSimulator->GetThisFrameRefFrames();
            
            for ( int i = 0 ; i < ( old_numel % single_load ); i++ ) {
                CalcSphereParticlePosition( object_space, radius, SAT_SPHERETYPE_NORMAL );
                object_space.Z += FIXED_TO_GEOMV( 0x10000 * cluster->contraction_time )
                + FIXED_TO_GEOMV( timepos );
                
                Vertex3 world_space;
                MtxVctMUL( shippo->ObjPosition, &object_space, &world_space );
                
                particle_s particle;
                PRT_InitParticle( particle, color, sizebound,
                                 ref_z, &world_space, &dirvec,
                                 lifetime, playerid, &extinfo );
                particle.flags |= PARTICLE_COLLISION;
                particle.flags |= PARTICLE_IS_PHOTON;
                PRT_CreateLinearParticle( particle );
            }
            
        }
        
        // calculate remaining visible sphere particles
        cluster->numel -= ( cluster->cur_contraction_time / cluster->contraction_time ) * single_load;
        if ( cluster->numel < 0 ) {
            cluster->numel = 0;
        }
        
        int next_numel = cluster->numel - single_load;
        if ( next_numel < 0 ) {
            next_numel = 0;
        }
        
        if ( cluster->numel == 0 ) {
            PRT_DeleteCluster(cluster);
        }
        else {
            // contract and rotate appropriate amount of particles
            
            // set old and new center values
            old_center.X = cluster->center.X;
            old_center.Y = cluster->center.Y;
            old_center.Z = ( cluster->cur_contraction_time / cluster->contraction_time ) ? 0 : cluster->center.Z;
            cluster->center.Z = FIXED_TO_GEOMV( 0x10000
                                               * ( cluster->cur_contraction_time % cluster->contraction_time ) );
            
            contraction = ( cluster->cur_contraction_time < cluster->contraction_time )
            ? ( cluster->contraction_speed * TheSimulator->GetThisFrameRefFrames() )
            : ( cluster->contraction_speed * ( cluster->cur_contraction_time % cluster->contraction_time ) );
            
            bams_t pitch = cluster->pitch * TheSimulator->GetThisFrameRefFrames();
            bams_t yaw   = cluster->yaw   * TheSimulator->GetThisFrameRefFrames();
            bams_t roll  = cluster->roll  * TheSimulator->GetThisFrameRefFrames();
            int pid = 0;
            for ( pid = ( cluster->numel - 1 ); pid >= next_numel; pid-- ) {
                
                cluster->rep[ pid ].position.X -= old_center.X;
                cluster->rep[ pid ].position.Y -= old_center.Y;
                cluster->rep[ pid ].position.Z -= old_center.Z;
                
                //FIXME ?:
                // particles may not be contained in
                // ship-bounding-sphere any more
                CalcSphereParticleRotation( cluster->rep[ pid ].position,
                                           pitch, yaw, roll );
                CalcSphereContraction( cluster->rep[ pid ].position, contraction );
                
                // move particles forward
                cluster->rep[ pid ].position.X += cluster->center.X;
                cluster->rep[ pid ].position.Y += cluster->center.Y;
                cluster->rep[ pid ].position.Z += cluster->center.Z;
            }
            for ( pid = next_numel - 1; pid >= 0; pid-- ) {
                CalcSphereParticleRotation( cluster->rep[ pid ].position,
                                           pitch, yaw, roll );
            }
            // update cluster radius ?
            cluster->bdsphere -= contraction;
        }
        
        cluster->cur_contraction_time %= cluster->contraction_time;
    }
    else {
        bams_t pitch = cluster->pitch * TheSimulator->GetThisFrameRefFrames();
        bams_t yaw   = cluster->yaw   * TheSimulator->GetThisFrameRefFrames();
        bams_t roll  = cluster->roll  * TheSimulator->GetThisFrameRefFrames();
        
        for ( int pid = 0; pid < cluster->numel; pid++ ) {
            CalcSphereParticleRotation( cluster->rep[ pid ].position,
                                       pitch, yaw, roll );
        }
    }
}

// check if genobject has attached particle clusters of a certain type --------
//
objectbase_pcluster_s * E_World::PRT_ObjectHasAttachedClustersOfType( GenObject* genobjpo, int animtype )
{
	ASSERT( genobjpo != NULL );
    
	objectbase_pcluster_s *scan = genobjpo->AttachedPClusters;
	for ( ; scan; scan = scan->attachlist ) {
        
		ASSERT( scan->type & CT_GENOBJECTRELATIVE_OBJ_MASK );
        
		if ( scan->animtype == animtype )
			return scan;
	}
    
	return NULL;
}

void LinearParticleCollision( linear_pcluster_s *cluster, int pid ) {
    TheGameCollDet->LinearParticleCollision(cluster, pid);
}

// ----------------------------------------------------------------------------
//
void SWARM_TimedAnimate( callback_pcluster_s* cluster )
{
	ASSERT( cluster != NULL );
    
	if ( TheWorld->SWARM_Animate( cluster ) == FALSE )
		return;
}


