/*
 * PARSEC HEADER: part_api.h
 */

#ifndef _PART_API_H_
#define _PART_API_H_


// particle system API functions
// -----------------------------
//	PRT_InitParticleSizes()
//	PRT_InitParticle()
//	PRT_InitClusterParticle()
//	PRT_NewCluster()
//	PRT_DeleteCluster()
//	PRT_DeleteCluster_NoListRemoval()
//	PRT_RemoveClusterFromAttachedList()
//	PRT_ObjectHasAttachedClusters()
//	PRT_ObjectHasAttachedClustersOfType()
//	PRT_DeleteAttachedClustersOfType()
//	PRT_AttachClusterToObject()
//	PRT_FreeAttachedClusterList()
//	PRT_AllocParticleDefinition()
//	PRT_RegisterParticleDefinition()
//	PRT_RegisterLoopTexParticle()
//	PRT_RegisterOneShotTexParticle()
//	PRT_AcquireParticleDefinition()
//	PRT_AcquireParticleDefinitionById()
//	PRT_InitParticleExtInfo()
//	PRT_CreateLinearParticle()
//	PRT_CreateCustomDrawParticle()
//	PRT_TranslateCustomParticles()
//	PRT_DrawCustomParticles()
//	PRT_CreateGenObjectParticleCluster()
//	PRT_CreateGenObjectParticle()
//	PRT_AddGenObjectParticle()
//	PRT_CreateObjectCenteredSphere()
//	PRT_CreateParticleSphereObject()


// ensure particle type info is available
#include "parttype.h"


// init reference z values for particles according to resolution --------------
//
void

PRT_InitParticleSizes (

	float resoscale
);


// set members of particle structure ------------------------------------------
//
inline void

PRT_InitParticle (

	particle_s&	particle,	// particle to initialize
	int 		bitmap,		// bitmap id for particle
	int 		color,		// color for particle
	int 		sizebound,	// lod threshold
	float 	refz,		// reference distance
	Vertex3*	position,	// position
	Vector3*	velocity,	// velocity vector
	int 		lifetime,	// particle lifetime
	int 		owner,		// owner id (remote player id)
	pextinfo_s*	extinfo		// extended drawing info
)

{
	ASSERT( position != NULL );

	particle.owner     = owner;
	particle.flags     = PARTICLE_ACTIVE;
	particle.lifetime  = lifetime;
	particle.extinfo   = extinfo;
	particle.bitmap    = bitmap;
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


// set members of particle structure and write into preexisting cluster -------
//
inline void

PRT_InitClusterParticle (

	pcluster_s*	cluster,	// cluster into which particle should be inserted
	int 		pid,		// id of particle to overwrite
	int 		bitmap,		// bitmap id for particle
	int 		color,		// color for particle
	int 		sizebound,	// lod threshold
	float 	refz,		// reference distance
	Vertex3*	position,	// position
	Vector3*	velocity,	// velocity vector
	int 		lifetime,	// particle lifetime
	int 		owner,		// owner id (remote player id)
	pextinfo_s*	extinfo		// extended drawing info
)

{
	ASSERT( pid < cluster->maxnumel );

	//NOTE:
	// the specified cluster member (particle)
	// simply gets overwritten!!

	PRT_InitParticle(
		cluster->rep[ pid ],
		bitmap,
		color,
		sizebound,
		refz,
		position,
		velocity,
		lifetime,
		owner,
		extinfo );

	// increase number of elements
	cluster->numel++;
	Particles->numel++;
}


// allocate new particle cluster and insert into list -------------------------
//
pcluster_s *

PRT_NewCluster (

	dword	type,			// cluster type (CT_xx)
	int		numelements,	// number of cluster elements (particles)
	size_t	auxstorage		// size of user-defined storage in bytes
);


// delete entire particle cluster ---------------------------------------------
//
void

PRT_DeleteCluster (

	pcluster_s*	cluster		// cluster to delete
);


// delete entire particle cluster (don't check any attachment lists) ----------
//
void

PRT_DeleteCluster_NoListRemoval (

	pcluster_s*	cluster		// cluster to delete
);


// remove cluster from attachment list if contained ---------------------------
//
void

PRT_RemoveClusterFromAttachedList (

	objectbase_pcluster_s*	cluster
);


// check if genobject has attached particle clusters --------------------------
//
int

PRT_ObjectHasAttachedClusters (

	GenObject*	genobjpo
);


// check if genobject has attached particle clusters of a certain type --------
//
objectbase_pcluster_s *

PRT_ObjectHasAttachedClustersOfType (

	GenObject*	genobjpo,
	int			animtype
);


// delete all clusters of certain type attached to a genobject ----------------
//
int

PRT_DeleteAttachedClustersOfType (

	const GenObject *	genobjpo,
	int					animtype
);


// attach cluster to genobject (insert at head of attachment list) ------------
//
void

PRT_AttachClusterToObject (

	GenObject*				genobjpo,
	objectbase_pcluster_s*	cluster
);


// free cluster list attached to genobject ------------------------------------
//
void

PRT_FreeAttachedClusterList (

	GenObject*	genobjpo
);


// allocate particle definition (fields have to be set afterwards!) -----------
//
pdef_s *

PRT_AllocParticleDefinition (

	int			numtexframes,	// number of texture animation frames
	int			numxfoframes	// number of texture transformation frames
);


// register particle definition (create struct and associate name with it) ----
//
pdef_s *

PRT_RegisterParticleDefinition (

	const char*	pdefname,		// unique name for particle definition
	pdefreg_s*	pdefreg,		// definition info
	int			overwrite		// overwrite old pdef if name already exists

);


// register particle definition with looping texture-only animation -----------
//
pdef_s *

PRT_RegisterLoopTexParticle (

	const char*	pdefname,		// unique name for particle definition
	int			numframes,		// number of animation frames
	const char**nametable,		// pointer to table of texture names
	int			equidelta,		// deltatime that is the same for all frames
	int			overwrite		// overwrite old pdef if name already exists
);


// register particle definition with one-shot texture-only animation ----------
//
pdef_s *

PRT_RegisterOneShotTexParticle (

	const char*	pdefname,		// unique name for particle definition
	int			numframes,		// number of animation frames
	const char**nametable,		// pointer to table of texture names
	int			equidelta,		// deltatime that is the same for all frames
	int			overwrite		// overwrite old pdef if name already exists

);


// acquire already registered particle definition via its unique name ---------
//
pdef_s *

PRT_AcquireParticleDefinition (

	const char*	pdefname,		// unique name for particle definition
	int*		retpdefid		// id of returned particle definition
);


// acquire already registered particle definition via its id ------------------
//
pdef_s *

PRT_AcquireParticleDefinitionById (

	int			pdefid			// id of particle definition
);


// init extended particle info using particle definitions ---------------------
//
void

PRT_InitParticleExtInfo (

	pextinfo_s*	extinfo,		// pextinfo_s to initialize
	pdef_s*		partdef,		// base particle definition (obligatory)
	pdef_s*		partdef_dest,	// destruction particle definition (optional)
	particle_s*	particle		// particle_s::extinfo will be set (optional)
);


// create new particle with linear animation (insert into next free slot) -----
//
particle_s *

PRT_CreateLinearParticle (

	particle_s&		particle		// already initialized particle
);


// create new particle with customdraw property -------------------------------
//
particle_s *

PRT_CreateCustomDrawParticle (

	particle_s&			particle,	// already initialized particle
	const GenObject*	baseobject	// object customdraw particles belong to
);


// translate customdraw particles belonging to specified object ---------------
//
int

PRT_TranslateCustomParticles (

	Vector3&			tvector,	// translation vector that should be applied
	const GenObject*	baseobject	// object customdraw particles belong to
);


// draw customdraw particles belonging to specified object --------------------
//
int

PRT_DrawCustomParticles (

	const GenObject*	baseobject	// object whose customdraw particles to draw
);


// create an empty genobject particle cluster ---------------------------------
//
genobject_pcluster_s *

PRT_CreateGenObjectParticleCluster (

	GenObject*				baseobject,	 // object particle should be attached to
	int						numelements, // number of particles to allocate
	genobject_pcluster_fpt	callback,	 // callback for cluster animation
	int						use_bdsphere // indicate whether the bounding-sphere of the parent should be used
);


// create a genobject geometry particle and attach it to specified object -----
//
genobject_pcluster_s *

PRT_CreateGenObjectParticle (

	particle_s&				particle,	// already initialized particle
	GenObject*				baseobject,	// object particle should be attached to
	genobject_pcluster_s*	trycluster	// try this cluster before scanning
);


// create a genobject geometry particle and attach it to specified object -----
//
genobject_pcluster_s *

PRT_AddGenObjectParticle (

	particle_s&				particle,	 // already initialized particle
	GenObject*				baseobject,	 // object particle should be attached to
	genobject_pcluster_s*	usecluster,	 // try this cluster before scanning
	int						numelements, // number of particles to allocate
	genobject_pcluster_fpt	callback	 // callback for cluster animation
);


// create a particle sphere centered around specific object origin ------------
//
basesphere_pcluster_s *

PRT_CreateObjectCenteredSphere (

	GenObject*	objectpo,		// object this sphere should be attached to
	geomv_t 	radius,			// radius of sphere (also used for bounding)
	int 		animtype,		// animation type (SAT_xx)
	int 		clustersiz,		// cluster size (number of particles in sphere)
	int 		lifetime,		// lifetime of sphere (each sphere particle)
	pdrwinfo_s*	pdinfo,			// particle appearance (drawing) info
	int			owner			// owner id (remote player id)
);


// create particle object comprising a sphere ---------------------------------
//
sphereobj_pcluster_s *

PRT_CreateParticleSphereObject (

	Vertex3&	origin,			// origin of particle object
	geomv_t 	radius,			// radius of sphere (also used for bounding)
	int 		animtype,		// animation type (SAT_xx)
	int 		clustersiz,		// cluster size (number of particles in sphere)
	int 		lifetime,		// lifetime of sphere (each sphere particle)
	pdrwinfo_s*	pdinfo,			// particle appearance (drawing) info
	int			owner			// owner id (remote player id)
);


#endif // _PART_API_H_


