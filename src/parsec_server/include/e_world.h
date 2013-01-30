/*
* PARSEC HEADER: E_WORLD.H
*/

#ifndef _E_WORLD_H_
#define _E_WORLD_H_
#include "parttype.h"

// flags ----------------------------------------------------------------------
//
//#define _SERVER_HAS_PARTICLE_CODE

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

#define PRTSB_ENERGYFIELD				PRT_NO_SIZEBOUND
// properties of energy field
#define ENERGY_FIELD_INITIAL_RADIUS 		FLOAT_TO_GEOMV( 0.0625f )
#define ENERGY_FIELD_LIFETIME				10000
#define SPHERE_PARTICLE_COLOR				255

// swarm behaviour constants --------------------------------------------------
//
#define NUM_PARTICLES		15						// number of particles in swarm
#define TIME_POSITIONS		3						// number of time positions recorded
#define PARTICLE_ACCEL		FLOAT_TO_GEOMV( 0.8f )	// acceleration of particles
#define PARTICLE_VELOCITY	FLOAT_TO_GEOMV( 2.0f )	// maximum particle velocity
#define PARTICLE_REFZ		300.0f					// particle refz
#define PARTICLE_LIFETIME	3000					// average lifetime of particles
#define LIFETIME_VARIANCE	400						// a small variance to avoid that all particles
// vanish at the same time
#define ANIM_TIMESLICE		6						// animate as if CurScreenRefFrames == 6
#define LIFETIME_AFTEREXPL	1200					// lifetime of particles after ship destruction


// swarm state structure ------------------------------------------------------
//
struct swarm_state_s {
    
	int						num;		// number of particles
	Point3h_f*				pos;		// current particle positions
    
	geomv_t*				x;			// particle position x[ time ][ partnum ]
	geomv_t*				y;			// particle position y[ time ][ partnum ]
	geomv_t*				z;			// particle position z[ time ][ partnum ]
    
	geomv_t*				xv;			// particle velocities xv[ partnum ]
	geomv_t*				yv;			// particle velocities xv[ partnum ]
	geomv_t*				zv;			// particle velocities xv[ partnum ]
    
	geomv_t					tx[ 3 ];	// target positions x
	geomv_t					ty[ 3 ];	// target positions y
	geomv_t					tz[ 3 ];	// target positions z
    
	ShipObject*				targetpo;	// the target object
	refframe_t				timerest;	// excess time from last animation frame
	GenObject				dummyobj; 	// just for sound position tracking
};

// photon properties
#define MIN_PHOTON_ENERGY                    10
#define PHOTON_ENERGY_CONSUMPTION           0x6000  // 0x10000 = 1.0

#define PHOTON_ROT_PITCH					0x0015  //0x01a0
#define PHOTON_ROT_YAW						-0x0022 //-0x02a0
#define PHOTON_ROT_ROLL						0x000d  //0x0100

#define PHOTON_SPHERE_PARTICLES				256
#define PHOTON_COLOR						123
#define PHOTON_REF_Z						75.0
#define PHOTON_CONTRACTION_TIME				50
#define PHOTON_CONTRACTION_SPEED			FIXED_TO_GEOMV( 0xA000 )
#define PHOTON_MAX_LOADING_TIME				1800
#define PHOTON_NUMLOADS						32

// helper macros --------------------------------------------------------------
//
#define X( t, b )			( swarm->x[ ( t ) * swarm->num + ( b ) ] )
#define Y( t, b )			( swarm->y[ ( t ) * swarm->num + ( b ) ] )
#define Z( t, b )			( swarm->z[ ( t ) * swarm->num + ( b ) ] )
#define balance_rand( v )	( ( SWARM_rand() % v ) - ( ( v ) / 2 ) )

#define LIGHTNING_SIZZLE_SPEED		10

static unsigned long int nextrand = 1;

int SWARM_rand();
void SWARM_srand( unsigned int seed );
void SWARM_TimedAnimate( callback_pcluster_s* cluster );

// class representing the PARSEC game world -----------------------------------
//
class E_World
{
public:

	// singly-linked lists for specific kinds of geometric objects ------------
	//
	ShipObject*		m_PShipObjects; // list of existing spacecraft objects
	LaserObject*	m_LaserObjects; // list of existing laser objects
	MissileObject*	m_MisslObjects; // list of existing missile objects
	ExtraObject*	m_ExtraObjects; // list of existing extras (power-ups)
	CustomObject*	m_CustmObjects; // list of existing custom objects
       
	int				m_last_summoned_objectid;		// object id of last object summoned

	// client -> e_global.h
	// current and maximum number of extras 
	int				m_nCurrentNumExtras;
	int				m_nCurrentNumPrtExtras;

	// next object number available 
	dword			NextObjNumber; // 0 is free for special purposes

	// table of object classes (from which objects can be instantiated) -----------
	//
	GenObject*		ObjClasses[ MAX_DISTINCT_OBJCLASSES ];

    // counters for concurrently displayed laser objects ---------------------
	int 			MaxNumShots;
	int 			NumShots;

protected:
	int _CountGenObjects( GenObject* walklist );

	// free memory occupied by an object
	void _FreeObjectMem( GenObject* objectpo );

	// invoke constructor of custom object
	void _InvokeCustomConstructor( GenObject* objectpo );

	// invoke destructor of custom object
	void _InvokeCustomDestructor( GenObject* objectpo );

	// kill all instances of passed in object class (search only one list)
	//
	int _KillClassInstancesFromList( int objclass, GenObject *listpo );

	// init object maintenance structures
	void _InitObjCtrl();
    
    //Cluster behaviour 
    void AnimateClusterBehavior( pcluster_s *cluster );
    void CalcConstantVelocityAnimation( linear_pcluster_s *cluster );
    void CalcSphereParticlePosition( Vertex3& position, geomv_t radius, int spheretype );
    void CalcSphereParticleExplosion( Vertex3& position, geomv_t speed );
    void CalcSpherePulse( Vertex3& position, Vertex3& pulsebase, geomv_t pulseval );
    int CalcSphereContraction( Vertex3& position, geomv_t speed );
    void CalcSphereParticleRotation( Vertex3& position, bams_t pitch, bams_t yaw, bams_t roll );
    void CalcSphereObjectAnimation( sphereobj_pcluster_s *cluster );
    void CalcCallbackTrajectoryAnimation( callback_pcluster_s *cluster );
    void CalcLightningAnimation( lightning_pcluster_s *cluster );
    void SetLightningParticlePosition( lightning_pcluster_s *cluster, particle_s particles[], Vertex3& current, Xmatrx tmatrx );
    //Particle disable/delete
    void DisableParticle( pcluster_s *cluster, int pid );
    void PRT_DeleteCluster (pcluster_s* cluster);
    void PRT_RemoveClusterFromAttachedList (objectbase_pcluster_s* cluster);
    
    
	// standard ctor
	E_World();

public:

	// SINGLETON access
	static E_World* GetWorld()
	{
		static E_World _TheWorld;
		return &_TheWorld;
	}

	// correct pointers that have moved after object was instantiated
	void OBJ_CorrectObjectInstance( GenObject *dstobj, GenObject *srcobj );

	CustomObject *CreateVirtualObject( dword objtypeid, dword dwOwner );

	// create object of class at position with orientation
	GenObject* CreateObject( int objclass, const Xmatrx startmatrx, dword dwOwner );

	// kill object contained in specific object list (search only one list)
	int KillSpecificObject( dword objno, GenObject *listpo );

	// kill a specific ship object from a list
	int	KillSpecificShipObject( dword objno );

	// object counting
	int GetNumShipObjects()			{ return _CountGenObjects( m_PShipObjects ); }
	int GetNumLaserObjects()		{ return _CountGenObjects( m_LaserObjects ); }
	int GetNumMisslObjects()		{ return _CountGenObjects( m_MisslObjects ); }
	int GetNumExtraObjects()		{ return _CountGenObjects( m_ExtraObjects ); }
	int GetNumCustmObjects()		{ return _CountGenObjects( m_CustmObjects ); }

	// return the object id of the last object that was summoned
	int	GetLastSummonedObjectID()   { return m_last_summoned_objectid; }

	// return pointer to first "real" ship in ship-objects list
	ShipObject* FetchFirstShip();

	// return pointer to first laser in laser-objects list 
	LaserObject* FetchFirstLaser() { return (LaserObject*) m_LaserObjects->NextObj; }

	// return pointer to first missile in missile-objects list
	MissileObject* FetchFirstMissile() { return (MissileObject*) m_MisslObjects->NextObj; }

	// return pointer to first extra in extra-objects list
	ExtraObject* FetchFirstExtra()	{ return (ExtraObject *) m_ExtraObjects->NextObj; }

	// return pointer to first custom object in custom-objects list
	CustomObject* FetchFirstCustom() { return  (CustomObject *) m_CustmObjects->NextObj; }

	// fetch object contained in specific object list
	GenObject* FetchSpecificObject( dword objno, GenObject* listpo );

	// fetch object via object id (search in all object lists)
	GenObject* FetchObject( dword objno );
    GenObject* SWARM_Init( int owner, Vertex3 *origin, ShipObject *targetpo, int randseed );

	// free memory occupied by an object
	void FreeObjectMem( GenObject *objectpo );

	// free all object memory blocks contained in specific list
	int FreeObjList( GenObject* listpo );

	// free all object memory blocks
	int FreeObjects();

	// free all objects and particles
	void KillAllObjects();

	// kill all instances of passed in object class (search in all object lists)
	int KillClassInstances( int objclass );

	//FIXME: here ?
	void IncreaseShotCounter();

	//FIXME: here ?
	void DecreaseShotCounter();
    
    //Energy Field
    void SFX_CreateEnergyField( Vertex3& origin );
    
    //particle man
    void InitParticleSystem();
    void PRT_InitParticle (particle_s&	particle,int color,int sizebound,float refz,Vertex3* position,Vector3* velocity, int lifetime,int	owner,pextinfo_s* extinfo);
    void PRT_InitClusterParticle (pcluster_s*	cluster,int	pid,int	bitmap,int color, int sizebound,float refz,Vertex3* position,Vector3*	velocity,int lifetime,int owner,pextinfo_s*	extinfo);
    void PRT_AttachClusterToObject( GenObject* genobjpo, objectbase_pcluster_s* cluster );
    int  PRT_DeleteAttachedClustersOfType( const GenObject * genobjpo, int animtype );
    void PRT_DeleteCluster_NoListRemoval( pcluster_s* cluster );
    
    // allocate new particle cluster and insert into list -------------------------
    //
    pcluster_s* PRT_NewCluster (dword type, int numelements, size_t auxstorage);

    particle_s* PRT_CreateLinearParticle (particle_s&	particle);
    sphereobj_pcluster_s* PRT_CreateParticleSphereObject (Vertex3& origin,geomv_t radius,int animtype,int clustersiz,int lifetime,pdrwinfo_s* pdinfo,int owner);
    
    pdef_s* PRT_AcquireParticleDefinitionById(int pdefid );
    pdef_s* PRT_AcquireParticleDefinition(const char* pdefname,int* retpdefid);
    objectbase_pcluster_s* PRT_ObjectHasAttachedClustersOfType( GenObject* genobjpo, int animtype );
    
    //Particle Animation call
    void PAN_AnimateParticles();
    int SWARM_Animate( callback_pcluster_s* cluster );
    
    //Lightning particles
    lightning_pcluster_s* CreateLightningParticles( ShipObject *shippo, int owner );
    
    //Photon
    photon_sphere_pcluster_s* CreatePhotonSphere( ShipObject *shippo );
    void CalcPhotonSphereAnimation( photon_sphere_pcluster_s *cluster );
};

#endif // _E_WORLD_H_

