/*
 * PARSEC - API Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:24 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1997-2000
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
#include <math.h>
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
#include "aud_defs.h"
#include "net_defs.h"
#include "vid_defs.h"

// mathematics header
#include "utl_math.h"

// particle types
#include "parttype.h"

// local module header
#include "part_api.h"

// proprietary module headers
#include "con_aux.h"
#include "e_color.h"
#include "e_record.h"
#include "e_supp.h"
#include "obj_ctrl.h"
#include "part_ani.h"
#include "part_sys.h"



// size of generic particle cluster
#define DEFAULT_CLUSTER_SIZE				256

// size of customdraw particle cluster
#define DEFAULT_CUSTOMDRAW_CLUSTER_SIZE		32

// size of genobject particle cluster
#define DEFAULT_GENOBJECT_CLUSTER_SIZE		128

// particle sphere rotation velocities
#define SPHERE_ROT_PITCH					0x0015  //0x01a0
#define SPHERE_ROT_YAW						-0x0022 //-0x02a0
#define SPHERE_ROT_ROLL						0x000d  //0x0100

// properties of pulsating sphere
#define SPHERE_PULSE_AMPLITUDE				FLOAT_TO_GEOMV( 12.0f )
#define SPHERE_PULSE_FREQUENCY				90

// properties of stochastic motion sphere
#define SPHERE_STOCHASTIC_MOTION_SPEED		100

// maximum number of registered particle definitions
#define MAX_PARTICLE_DEFS					256


// array of registered particle definitions -----------------------------------
//
PUBLIC int				NumParticleDefinitions = 0;
PUBLIC pdefref_s		ParticleDefinitions[ MAX_PARTICLE_DEFS ];


// reference z values for particles -------------------------------------------
//
float sphere_ref_z = 1.0f;


// init reference z values for particles according to resolution --------------
//
void PRT_InitParticleSizes( float resoscale )
{
	sphere_ref_z = resoscale * SPHERE_REF_Z;
}


// allocate new particle cluster and insert into list -------------------------
//
pcluster_s * PRT_NewCluster(
	dword	type,			// cluster type (CT_xx)
	int		numelements,	// number of cluster elements (particles)
	size_t	auxstorage		// size of user-defined storage in bytes
)
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


// delete entire particle cluster ---------------------------------------------
//
void PRT_DeleteCluster( pcluster_s* cluster )
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
}


// delete entire particle cluster (don't check any attachment lists) ----------
//
void PRT_DeleteCluster_NoListRemoval( pcluster_s* cluster )
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


// remove cluster from its attachment list if contained in any ----------------
//
void PRT_RemoveClusterFromAttachedList( objectbase_pcluster_s* cluster )
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


// check if genobject has attached particle clusters --------------------------
//
int PRT_ObjectHasAttachedClusters( GenObject* genobjpo )
{
	ASSERT( genobjpo != NULL );

	return ( genobjpo->AttachedPClusters != NULL );
}


// check if genobject has attached particle clusters of a certain type --------
//
objectbase_pcluster_s * PRT_ObjectHasAttachedClustersOfType( GenObject* genobjpo, int animtype )
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


// delete all clusters of certain type attached to a genobject ----------------
//
int PRT_DeleteAttachedClustersOfType( const GenObject * genobjpo, int animtype )
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


// attach cluster to genobject (insert at head of attachment list) ------------
//
void PRT_AttachClusterToObject( GenObject* genobjpo, objectbase_pcluster_s* cluster )
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


// free cluster list attached to genobject ------------------------------------
//
void PRT_FreeAttachedClusterList( GenObject* genobjpo )
{
	ASSERT( genobjpo != NULL );

	objectbase_pcluster_s *scan = genobjpo->AttachedPClusters;

	while ( scan != NULL ) {

		ASSERT( scan->type & CT_GENOBJECTRELATIVE_OBJ_MASK );

		objectbase_pcluster_s *temp = scan->attachlist;
		PRT_DeleteCluster_NoListRemoval( scan );
		scan = temp;
	}

	genobjpo->AttachedPClusters = NULL;
}


// allocate particle definition (fields have to be set afterwards!) -----------
//
pdef_s * PRT_AllocParticleDefinition(
	int		numtexframes,	// number of texture animation frames
	int		numxfoframes	// number of texture transformation frames
)
{
	//NOTE:
	// this function allocates a new pdef_s containing enough
	// space for the supplied number of animation frames.
	// NULL will be returned if there is not enough memory for
	// the pdef_s (including its animation tables).

	//NOTE:
	// the table entries themselves are not filled by this function.
	// this has to be done afterwards. (mandatory!!)

	ASSERT( numtexframes >= 1 );
	ASSERT( numxfoframes >= 0 );

	// alloc mem for particle definition struct + tables
	size_t texsiz = sizeof( texfrm_s ) * numtexframes;
	size_t xfosiz = sizeof( xfofrm_s ) * numxfoframes;
	size_t memsiz = sizeof( pdef_s ) + texsiz + xfosiz;

	pdef_s *pdef = (pdef_s *) ALLOCMEM( memsiz );
	if ( pdef == NULL )
		return NULL;

	// clear struct mem
	memset( pdef, 0, memsiz );

	// set table addresses
	pdef->tex_table = (texfrm_s *) ( (char*)pdef + sizeof( pdef_s ) );
	pdef->xfo_table = ( numxfoframes > 0 ) ?
		(xfofrm_s *) ( (char*)pdef + sizeof( pdef_s ) + texsiz ) : NULL;

	// set end indexes to table length (start and repeat are zero anyway)
	pdef->tex_end = numtexframes - 1;
	pdef->xfo_end = ( numxfoframes > 0 ) ? numxfoframes - 1 : 0;

	//NOTE:
	// this is just the default for start, repeat, and end index. the user
	// has to set them to their correct values, if different from the default.

	return pdef;
}


// register particle definition (create struct and associate name with it) ----
//
pdef_s * PRT_RegisterParticleDefinition(
	const char*	pdefname,	// unique name for particle definition
	pdefreg_s*	pdefreg,	// definition info
	int			overwrite	// overwrite old pdef if name already exists
)
{
	//NOTE:
	// this function takes a pdefreg_s, creates the actual pdef_s
	// and registers it using the supplied name.
	// NULL will be returned if it could not be registered for
	// some reason (too many pdefs, no mem, ...).

	ASSERT( pdefname != NULL );
	ASSERT( pdefreg != NULL );

	ptexreg_s *texdef = pdefreg->texinfo;
	pxforeg_s *xfodef = pdefreg->xfoinfo;

	int texframes = pdefreg->textabsize;
	int xfoframes = pdefreg->xfotabsize;

	ASSERT( texdef != NULL );
	ASSERT( texframes >= 1 );
	ASSERT( xfoframes >= 0 );
	ASSERT( ( xfoframes == 0 ) || ( xfodef != NULL ) );

	// check if slot for pdef available
	if ( NumParticleDefinitions >= MAX_PARTICLE_DEFS ) {
		ASSERT( NumParticleDefinitions == MAX_PARTICLE_DEFS );
		return NULL;
	}

	//NOTE:
	// even if an already existing pdef is going to be overwritten this
	// check is performed. thus at least one slot must always be available.

	// create header and alloc mem
	pdef_s *pdef = PRT_AllocParticleDefinition( texframes, xfoframes );
	if ( pdef == NULL ) {
		ASSERT( 0 );
		return NULL;
	}

	// fill in texture map info
	int curframe = 0;
	for ( curframe = 0; curframe < texframes; curframe++ ) {

		// look up texture via name
		TextureMap *texmap = FetchTextureMap( texdef[ curframe ].texname );
		if ( texmap == NULL ) {
			FREEMEM( pdef );
			return NULL;
		}

		// write frame info
		pdef->tex_table[ curframe ].deltatime = texdef[ curframe ].deltatime;
		pdef->tex_table[ curframe ].texmap    = texmap;
	}

	// set anim points
	pdef->tex_start = pdefreg->texstart;
	pdef->tex_rep   = pdefreg->texrep;
	pdef->tex_end   = pdefreg->texend;

	// fill in trafo info
	if ( xfoframes > 0 ) {

		// write frame info
		for ( curframe = 0; curframe < xfoframes; curframe++ ) {
			pdef->xfo_table[ curframe ].deltatime = xfodef[ curframe ].deltatime;
			pdef->xfo_table[ curframe ].imgtrafo  = xfodef[ curframe ].imgtrafo;
		}

		// set anim points
		pdef->xfo_start = pdefreg->xfostart;
		pdef->xfo_rep   = pdefreg->xforep;
		pdef->xfo_end   = pdefreg->xfoend;
	}

	// scan all registered particle definitions
	int curdef = 0;
	for ( curdef = 0; curdef < NumParticleDefinitions; curdef++ ) {
		if ( strcmp( ParticleDefinitions[ curdef ].defname, pdefname ) == 0 ) {
			break;
		}
	}

	// found already registered pdef of same name?
	if ( curdef < NumParticleDefinitions ) {

		// already existing pdefs (names) may be overwritten
		if ( overwrite ) {

			ParticleDefinitions[ curdef ].def = pdef;

			//NOTE:
			// the old pdef is kept around as zombie. it cannot be deleted
			// until the last particle using it has died. for this reason
			// the current approach is to never delete it. (note that functions
			// like PART_DEF::PDEF_explode1() also keep around the old pointer,
			// at least until they detect that the pdef has been changed.)

		} else {

			// delete new pdef and return old
			FREEMEM( pdef );
			pdef = ParticleDefinitions[ curdef ].def;
		}

	} else {

		// append new pdef
		curdef = NumParticleDefinitions++;

		ASSERT( ParticleDefinitions[ curdef ].defname == NULL );
		ASSERT( ParticleDefinitions[ curdef ].def     == NULL );

		// copy name to not depend on caller string semantics
		char *name = (char *) ALLOCMEM( strlen( pdefname ) + 1 );
		if ( name == NULL ) {
			ASSERT( 0 );
			return NULL;
		}
		strcpy( name, pdefname );

		ParticleDefinitions[ curdef ].defname	= name;
		ParticleDefinitions[ curdef ].def		= pdef;
	}

	ASSERT( pdef != NULL );
	return pdef;
}


// register particle definition with looping texture-only animation -----------
//
pdef_s * PRT_RegisterLoopTexParticle(
	const char*	pdefname,		// unique name for particle definition
	int			numframes,		// number of animation frames
	const char**nametable,		// pointer to table of texture names
	int			equidelta,		// deltatime that is the same for all frames
	int			overwrite		// overwrite old pdef if name already exists
)
{
	ASSERT( pdefname != NULL );
	ASSERT( numframes >= 1 );
	ASSERT( nametable != NULL );
	ASSERT( *nametable != NULL );
	ASSERT( equidelta > 0 );

	// alloc temporary table mem
	ptexreg_s *texinfo = (ptexreg_s *) ALLOCMEM( sizeof( ptexreg_s ) * numframes );
	if ( texinfo == NULL )
		return NULL;

	// init temporary table mem
	for ( int fid = 0; fid < numframes; fid++ ) {
		texinfo[ fid ].deltatime = equidelta;
		texinfo[ fid ].texname   = (char *) nametable[ fid ];
	}

	// init registration info
	pdefreg_s pdefreg;
	pdefreg.texinfo		= texinfo;
	pdefreg.textabsize	= numframes;
	pdefreg.texstart	= 0;
	pdefreg.texrep		= 0;
	pdefreg.texend		= numframes - 1;
	pdefreg.xfoinfo		= NULL;
	pdefreg.xfotabsize	= 0;

	// do registration
	pdef_s *pdef = PRT_RegisterParticleDefinition( pdefname, &pdefreg, overwrite );

	// free temporary table mem
	FREEMEM( texinfo );

	// return acquired particle definition (may be NULL)
	return pdef;
}


// register particle definition with one-shot texture-only animation ----------
//
pdef_s * PRT_RegisterOneShotTexParticle(
	const char*	pdefname,		// unique name for particle definition
	int			numframes,		// number of animation frames
	const char**nametable,		// pointer to table of texture names
	int			equidelta,		// deltatime that is the same for all frames
	int			overwrite		// overwrite old pdef if name already exists
)
{
	ASSERT( pdefname != NULL );
	ASSERT( numframes >= 1 );
	ASSERT( nametable != NULL );
	ASSERT( *nametable != NULL );
	ASSERT( equidelta > 0 );

	// alloc temporary table mem
	ptexreg_s *texinfo = (ptexreg_s *) ALLOCMEM( sizeof( ptexreg_s ) * numframes );
	if ( texinfo == NULL )
		return NULL;

	// init temporary table mem
	for ( int fid = 0; fid < numframes; fid++ ) {
		texinfo[ fid ].deltatime = equidelta;
		texinfo[ fid ].texname   = (char *) nametable[ fid ];
	}

	// init registration info
	pdefreg_s pdefreg;
	pdefreg.texinfo		= texinfo;
	pdefreg.textabsize	= numframes;
	pdefreg.texstart	= 0;
	pdefreg.texrep		= numframes - 1;
	pdefreg.texend		= numframes - 1;
	pdefreg.xfoinfo		= NULL;
	pdefreg.xfotabsize	= 0;

	// do registration
	pdef_s *pdef = PRT_RegisterParticleDefinition( pdefname, &pdefreg, overwrite );

	// free temporary table mem
	FREEMEM( texinfo );

	// return acquired particle definition (may be NULL)
	return pdef;
}


// acquire already registered particle definition via its unique name ---------
//
pdef_s * PRT_AcquireParticleDefinition(
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
}


// acquire already registered particle definition via its id ------------------
//
pdef_s * PRT_AcquireParticleDefinitionById(
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
}


// init extended particle info using particle definitions ---------------------
//
void PRT_InitParticleExtInfo(
	pextinfo_s*	extinfo,		// pextinfo_s to initialize
	pdef_s*		partdef,		// base particle definition (obligatory)
	pdef_s*		partdef_dest,	// destruction particle definition (optional)
	particle_s*	particle		// particle_s::extinfo will be set (optional)
)
{
	ASSERT( extinfo != NULL );
	ASSERT( partdef != NULL );
	ASSERT( partdef->tex_table != NULL );
	ASSERT( ( partdef_dest == NULL ) || ( partdef_dest->tex_table != NULL ) );

	//NOTE:
	// if the particle parameter is not NULL, a pointer to the
	// initialized extinfo will be stored into the particle
	// structure (field particle_s::extinfo).

	// clear struct mem
	memset( extinfo, 0, sizeof( pextinfo_s ) );

	// store pointer in particle structure
	if ( particle != NULL )
		particle->extinfo = extinfo;

	// set particle definitions
	extinfo->partdef	  = partdef;
	extinfo->partdef_dest = partdef_dest;

	// set first frame
	extinfo->tex_pos = partdef->tex_start;
	extinfo->xfo_pos = partdef->xfo_start;

	// set first deltatime for texture animation frame
	texfrm_s *firsttexframe = &partdef->tex_table[ extinfo->tex_pos ];
	extinfo->tex_time = firsttexframe->deltatime;

	// set first deltatime for texture transformation frame
	if ( partdef->xfo_table != NULL ) {
		xfofrm_s *firstxfoframe = &partdef->xfo_table[ extinfo->xfo_pos ];
		extinfo->xfo_time = firstxfoframe->deltatime;
	}
}


// create new particle with linear animation (insert into next free slot) -----
//
particle_s * PRT_CreateLinearParticle(
	particle_s&		particle		// already initialized particle
)
{
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


// create new particle with customdraw property -------------------------------
//
particle_s * PRT_CreateCustomDrawParticle(
	particle_s&			particle,	// already initialized particle
	const GenObject*	baseobject	// object customdraw particles belong to
)
{
	ASSERT( baseobject != NULL );

	// try to reuse already allocated cluster
	customdraw_pcluster_s *cluster =
		(customdraw_pcluster_s *) CustomDrawCluster;

	if ( cluster != NULL ) {

		ASSERT( ( cluster->type & CT_TYPEMASK ) == CT_CUSTOMDRAW );

		if ( ( cluster->baseobject == baseobject ) &&
			 ( cluster->numel < cluster->maxnumel ) ) {

			// copy particle struct into available slot in free cluster
			particle_s *pmem = cluster->rep + cluster->numel;
			*pmem = particle;

			// check whether extinfo attached
			if ( particle.extinfo != NULL ) {

				if ( ( cluster->type & CT_EXTINFO_STORAGE ) == 0 ) {
					ASSERT( 0 );
					goto allocnew;
				}

				// copy over extinfo
				pextinfo_s *curextinfo =
					(pextinfo_s *)( cluster->rep + cluster->maxnumel );
				curextinfo += cluster->numel;
				memcpy( curextinfo, particle.extinfo, sizeof( pextinfo_s ) );

				// set new extinfo pointer
				pmem->extinfo = curextinfo;
			}

			// increase number of cluster elements
			cluster->numel++;

			// return new particle location
			return pmem;
		}
	}

allocnew:

	// always allocate clusters with storage for extinfo!
	dword clustertype = CT_CUSTOMDRAW | CT_EXTINFO_STORAGE;
	int   numelements = DEFAULT_CUSTOMDRAW_CLUSTER_SIZE;

	// create new customdraw cluster
	cluster = (customdraw_pcluster_s *)
		PRT_NewCluster( clustertype, numelements, 0 );

	ASSERT( cluster != NULL );
	cluster->baseobject	= baseobject;
	cluster->callback	= NULL;

	//NOTE:
	// field bdsphere is zero.

	// insert recursively (tail rec)
	CustomDrawCluster = cluster;
	return PRT_CreateCustomDrawParticle( particle, baseobject );
}


// translate customdraw particles belonging to specified object ---------------
//
int PRT_TranslateCustomParticles(
	Vector3&			tvector,	// translation vector that should be applied
	const GenObject*	baseobject	// object customdraw particles belong to
)
{
	ASSERT( baseobject != NULL );

	int cfound = FALSE;

	//TODO:
	// use clusters attached to baseobject to
	// avoid scanning all clusters.

	// walk list of clusters
	pcluster_s *scan = Particles->next;
	for ( ; scan->next; scan = scan->next ) {
		if ( ( scan->type & CT_TYPEMASK ) == CT_CUSTOMDRAW ) {

			customdraw_pcluster_s *cluster = (customdraw_pcluster_s *) scan;

			if ( cluster->baseobject != baseobject )
				continue;

			// to return if at least one found
			cfound = TRUE;

			// translate particles in cluster
			for ( int curp = 0; curp < cluster->numel; curp++ ) {

				particle_s *particle = &cluster->rep[ curp ];

				particle->position.X += tvector.X;
				particle->position.Y += tvector.Y;
				particle->position.Z += tvector.Z;
			}
		}
	}

	return cfound;
}


// draw customdraw particles belonging to specified object --------------------
//
int PRT_DrawCustomParticles(
	const GenObject*	baseobject	// object whose customdraw particles to draw
)
{
	ASSERT( baseobject != NULL );

	//NOTE:
	// DrawCustomParticles() is exported by PART_SYS.C

	return DrawCustomParticles( baseobject );
}


// create an empty genobject particle cluster ---------------------------------
//
genobject_pcluster_s * PRT_CreateGenObjectParticleCluster(
	GenObject*				baseobject,	 // object particle should be attached to
	int						numelements, // number of particles to allocate
	genobject_pcluster_fpt	callback,	 // callback for cluster animation
	int						use_bdsphere // indicate whether the bounding-sphere of the parent should be used
)
{
	ASSERT( baseobject != NULL );
	ASSERT( numelements > 0 );

	// always allocate clusters with storage for extinfo!
	dword clustertype = CT_GENOBJECT_PARTICLES | CT_EXTINFO_STORAGE;

	// create new genobject cluster
	genobject_pcluster_s *cluster = (genobject_pcluster_s *)
		PRT_NewCluster( clustertype, numelements, 0 );

	// init custom fields
	ASSERT( cluster != NULL );
	cluster->animtype = SAT_GENOBJECT;
	cluster->callback = callback;

	// set bounding sphere to the same as the parent object
	cluster->bdsphere = use_bdsphere ? baseobject->BoundingSphere : 0;

	//TODO:
	// use flag to indicate whether the bounding sphere should
	// be checked/updated for each particle that is added.

	// attach cluster to referenced object
	PRT_AttachClusterToObject( baseobject, cluster );

	return cluster;
}


// create a genobject geometry particle and attach it to specified object -----
//
genobject_pcluster_s * PRT_CreateGenObjectParticle(
	particle_s&				particle,	// already initialized particle
	GenObject*				baseobject,	// object particle should be attached to
	genobject_pcluster_s*	trycluster	// try this cluster before scanning
)
{
	//NOTE:
	// this function is meant for attaching single particles.
	// if no cluster is supplied all clusters attached to the
	// baseobject are walked to try and find a free slot.
	// only if no slot can be found a new cluster will be allocated,
	// using a default size (DEFAULT_GENOBJECT_CLUSTER_SIZE).
	// such single particles most likely have a fixed position,
	// therefore the callback will be NULL by default. it can be set
	// afterwards, though, using the returned cluster pointer.

	//NOTE:
	// typical use:
	// ------------
	// genobject_pcluster_s *cluster = NULL;
	// cluster = PRT_CreateGenObjectParticle( ., ., cluster );
	// cluster = PRT_CreateGenObjectParticle( ., ., cluster );
	// ...

	ASSERT( baseobject != NULL );

	// try to reuse already allocated cluster
	if ( trycluster != NULL ) {

		ASSERT( ( trycluster->type & CT_TYPEMASK ) == CT_GENOBJECT_PARTICLES );
		ASSERT( ( trycluster->baseobject == baseobject ) );

		if ( trycluster->numel < trycluster->maxnumel ) {

			// copy particle struct into available slot in free cluster
			particle_s *pmem = trycluster->rep + trycluster->numel;
			*pmem = particle;

			// check whether extinfo attached
			if ( particle.extinfo != NULL ) {

				if ( ( trycluster->type & CT_EXTINFO_STORAGE ) == 0 ) {
					ASSERT( 0 );
					goto allocnew;
				}

				// copy over extinfo
				pextinfo_s *curextinfo =
					(pextinfo_s *)( trycluster->rep + trycluster->maxnumel );
				curextinfo += trycluster->numel;
				memcpy( curextinfo, particle.extinfo, sizeof( pextinfo_s ) );

				// set new extinfo pointer
				pmem->extinfo = curextinfo;
			}

			// increase number of cluster elements
			trycluster->numel++;

			// stick with cluster
			return trycluster;
		}

	} else {

		// try to find attached genobject cluster with enough free space
		objectbase_pcluster_s *scan = baseobject->AttachedPClusters;
		for ( ; scan; scan = scan->attachlist ) {

			ASSERT( scan->type & CT_GENOBJECTRELATIVE_OBJ_MASK );
			ASSERT( scan->baseobject == baseobject );

			if ( ( scan->type & CT_TYPEMASK ) == CT_GENOBJECT_PARTICLES ) {
				if ( scan->numel < scan->maxnumel ) {
					// insert recursively (tail rec)
					return PRT_CreateGenObjectParticle(
						particle, baseobject, (genobject_pcluster_s *) scan );
				}
			}
		}
	}

allocnew:

	//NOTE:
	// we assume all particles of genobject clusters are
	// contained within their parent object's bounding sphere.
	// if this is not the case culling won't work correctly.

	//TODO:
	// use flag to indicate whether the bounding sphere should
	// be checked/updated for each particle that is added.

	// create a new genobject particle cluster
	trycluster = PRT_CreateGenObjectParticleCluster(
		baseobject, DEFAULT_GENOBJECT_CLUSTER_SIZE, NULL, TRUE );

	// insert recursively (tail rec)
	return PRT_CreateGenObjectParticle( particle, baseobject, trycluster );
}


// create a genobject geometry particle and attach it to specified object -----
//
genobject_pcluster_s * PRT_AddGenObjectParticle(
	particle_s&				particle,	 // already initialized particle
	GenObject*				baseobject,	 // object particle should be attached to
	genobject_pcluster_s*	usecluster,	 // try this cluster before scanning
	int						numelements, // number of particles to allocate
	genobject_pcluster_fpt	callback	 // callback for cluster animation
)
{
	//NOTE:
	// this function is meant for filling a single attached
	// cluster with multiple particles belonging together.
	// it either uses the supplied cluster or creates a new
	// one with numelements if numelements > 0.
	// if numelements == 0 no new cluster will be created
	// in case there is no slot available. the particle
	// will simply be dropped then.

	//NOTE:
	// typical uses:
	// ------------
	// genobject_pcluster_s *cluster = NULL;
	// for ( int pid = 0; pid < csiz; pid++ )
	//     cluster = PRT_AddGenObjectParticle( ., ., cluster, csiz, . );
	// ------------
	// genobject_pcluster_s *cluster =
	//     PRT_CreateGenObjectParticleCluster( ., csiz, ., . );
	// for ( int pid = 0; pid < csiz; pid++ )
	//     cluster = PRT_AddGenObjectParticle( ., ., cluster, 0, . );
	// ------------

	ASSERT( baseobject != NULL );

	// try to reuse already allocated cluster
	if ( usecluster != NULL ) {

		ASSERT( ( usecluster->type & CT_TYPEMASK ) == CT_GENOBJECT_PARTICLES );
		ASSERT( ( usecluster->baseobject == baseobject ) );

		if ( usecluster->numel < usecluster->maxnumel ) {

			// copy particle struct into available slot in free cluster
			particle_s *pmem = usecluster->rep + usecluster->numel;
			*pmem = particle;

			// check whether extinfo attached
			if ( particle.extinfo != NULL ) {

				if ( ( usecluster->type & CT_EXTINFO_STORAGE ) == 0 ) {
					ASSERT( 0 );
					goto allocnew;
				}

				// copy over extinfo
				pextinfo_s *curextinfo =
					(pextinfo_s *)( usecluster->rep + usecluster->maxnumel );
				curextinfo += usecluster->numel;
				memcpy( curextinfo, particle.extinfo, sizeof( pextinfo_s ) );

				// set new extinfo pointer
				pmem->extinfo = curextinfo;
			}

			// increase number of cluster elements
			usecluster->numel++;

			// stick with cluster
			return usecluster;
		}
	}

allocnew:

	// do not create particle (fail quietly) if no slot
	// available and no new cluster should be allocated
	if ( numelements <= 0 ) {
		return NULL;
	}

	//NOTE:
	// we assume all particles of genobject clusters are
	// contained within their parent object's bounding sphere.
	// if this is not the case culling won't work correctly.

	//TODO:
	// use flag to indicate whether the bounding sphere should
	// be checked/updated for each particle that is added.

	// create a new genobject particle cluster
	usecluster = PRT_CreateGenObjectParticleCluster(
		baseobject, numelements, callback, TRUE );

	// insert recursively (tail rec)
	return PRT_AddGenObjectParticle( particle, baseobject, usecluster, numelements, callback );
}


// create a particle sphere centered around specific object origin ------------
//
basesphere_pcluster_s * PRT_CreateObjectCenteredSphere(
	GenObject*	objectpo,		// object this sphere should be attached to
	geomv_t 	radius,			// radius of sphere (also used for bounding)
	int 		animtype,		// animation type (SAT_xx)
	int 		clustersiz,		// cluster size (number of particles in sphere)
	int 		lifetime,		// lifetime of sphere (each sphere particle)
	pdrwinfo_s*	pdinfo,			// particle appearance (drawing) info
	int			owner			// owner id (remote player id)
)
{
	ASSERT( objectpo != NULL );

	// check if animation type is allowed for object-centered spheres
	if ( ( animtype & SAT_VALID_FOR_OBJECTCENTERED_SPHERE  ) == 0 ) {
		ASSERT( 0 );
		return NULL;
	}

	// fetch pdefinfo if supplied
	int bitmapindx = pdinfo ? pdinfo->bmindx  : SPHERE_BM_INDX;
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
	dword clustertype = CT_OBJECTCENTERED_SPHERE | hints;
	if ( extinfo != NULL ) {
		clustertype |= CT_EXTINFO_STORAGE | CT_HINT_PARTICLES_HAVE_EXTINFO;
	}

	// create new cluster
	basesphere_pcluster_s *cluster = (basesphere_pcluster_s *)
		PRT_NewCluster( clustertype, allocsiz, 0 );

	// fill in basic fields
	cluster->bdsphere = radius;
	cluster->animtype = animtype;
	cluster->lifetime = lifetime;
	cluster->max_life = lifetime;

	// fill in additional fields
	switch ( animtype & SAT_BASIC_ANIM_MASK ) {

		case SAT_ROTATING:
			cluster->rot.pitch	 = SPHERE_ROT_PITCH;
			cluster->rot.yaw	 = SPHERE_ROT_YAW;
			cluster->rot.roll	 = SPHERE_ROT_ROLL;
			break;

		case SAT_STOCHASTIC_MOTION:
			cluster->rand.radius = radius;
			cluster->rand.speed	 = SPHERE_STOCHASTIC_MOTION_SPEED;
			cluster->rand.fcount = cluster->rand.speed;
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

	// attach sphere's particle cluster to object
	PRT_AttachClusterToObject( objectpo, cluster );

	return cluster;
}


// create particle object comprising a sphere ---------------------------------
//
sphereobj_pcluster_s* PRT_CreateParticleSphereObject (
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
	int bitmapindx = pdinfo ? pdinfo->bmindx  : SPHERE_BM_INDX;
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



