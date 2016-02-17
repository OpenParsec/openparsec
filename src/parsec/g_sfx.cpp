/*
 * PARSEC - Special Effects
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:25 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1997-1999
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
#include "od_props.h"

// global externals
#include "globals.h"

// subsystem headers
#include "net_defs.h"
#include "vid_defs.h"

// mathematics header
#include "utl_math.h"

// particle types
#include "parttype.h"

// local module header
#include "g_sfx.h"

// proprietary module headers
#include "con_aux.h"
#include "e_color.h"
#include "e_record.h"
#include "h_supp.h"
#include "obj_ctrl.h"
#include "part_api.h"
#include "part_def.h"


// flags
#define NO_SIZE_BOUNDS


// particle size boundaries
#ifdef NO_SIZE_BOUNDS
	#define PRTSB_EXPLOSION					PRT_NO_SIZEBOUND
	#define PRTSB_ENERGYFIELD				PRT_NO_SIZEBOUND
	#define PRTSB_PROTSHIELD				PRT_NO_SIZEBOUND
	#define PRTSB_INVULSHIELD				PRT_NO_SIZEBOUND
#else
	#define PRTSB_EXPLOSION					partbitmap_size_bound
	#define PRTSB_ENERGYFIELD				partbitmap_size_bound
	#define PRTSB_PROTSHIELD				partbitmap_size_bound
	#define PRTSB_INVULSHIELD				partbitmap_size_bound
#endif

// geometry of particle explosion of ship
#define PARTICLE_EXPLOSION_STARTRADIUS 		FIXED_TO_GEOMV( 0xa00 )
#define PARTICLE_EXPLOSION_NUMPARTICLES 	150
#define PARTICLE_EXPLOSION_DURATION			400 //250 //400

// properties of energy field
#define ENERGY_FIELD_INITIAL_RADIUS 		FLOAT_TO_GEOMV( 0.0625f )
#define ENERGY_FIELD_LIFETIME				10000

// properties of protection shield
#define PROTSHIELD_BITMAP					BM_SHIELD2
#define PROTSHIELD_COLOR					228
#define PROTSHIELD_REF_Z					SPHERE_REF_Z

// if ship is hit its shield is displayed for this period of time
#define SHIELD_FLASH_DURATION				40

// properties of invulnerability shield
#define MEGASHIELD_BITMAP					BM_SHIELD1
#define MEGASHIELD_COLOR					123
#define MEGASHIELD_REF_Z					SPHERE_REF_Z
#define MEGASHIELD_LIFETIME					MEGASHIELD_STRENGTH * DEFAULT_REFFRAME_FREQUENCY //20000

// properties of propulsion fumes
#define PROPFUMES_BITMAP					BM_PROPFUMES1
#define PROPFUMES_COLOR						3
#define PROPFUMES_REF_Z						60.0f
#define PROPFUMES_SIZE_BOUNDARY				2
#define PROPFUMES_SCATTER_BOUNDARY			2
#define PROPFUMES_DRIFT_SPEED				0.0016f



// reference z values for particles -------------------------------------------
//
float sphere_explosion_ref_z	= 1.0f;
float protshield_ref_z		= 1.0f;
float megashield_ref_z		= 1.0f;
float propfumes_ref_z			= 1.0f;


// init reference z values for particles according to resolution --------------
//
void SFX_InitParticleSizes( float resoscale )
{
	sphere_explosion_ref_z	= resoscale * SPHERE_EXPLOSION_REF_Z;
	protshield_ref_z		= resoscale * PROTSHIELD_REF_Z;
	megashield_ref_z		= resoscale * MEGASHIELD_REF_Z;
	propfumes_ref_z			= resoscale * PROPFUMES_REF_Z;
}


// create particle explosion; meant to be called only externally --------------
//
void SFX_ParticleExplosion( ShipObject *shippo )
{
	ASSERT( shippo != NULL );
	ASSERT( OBJECT_TYPE_SHIP( shippo ) );

	// calc explosion origin
	Vertex3 origin;
	FetchTVector( shippo->ObjPosition, &origin );

	// some particle appearance properties
	int bitmap	  = SPHERE_EXPLOSION_BM_INDX;
	int color	  = SPHERE_EXPLOSION_COLOR;
	float ref_z = sphere_explosion_ref_z;
	int sizebound = PRTSB_EXPLOSION;
	int lifetime  = PARTICLE_EXPLOSION_DURATION;

	pextinfo_s extinfo;
	pextinfo_s *pextinfo = NULL;

	if ( AUX_EXPLOSION_PARTICLES_EXTINFO ) {

		// alter basic attributes
		ref_z    *= 2;
		bitmap    = iter_texrgba | iter_specularadd;
//		sizebound = PRT_NO_SIZEBOUND;

		// fetch pdef
		pdef_s *pdef = PDEF_pflare02();
		if ( pdef == NULL )
			return;

		// create pextinfo
		pextinfo = &extinfo;
		PRT_InitParticleExtInfo( pextinfo, pdef, NULL, NULL );
	}

	pdrwinfo_s drawinfo;
	drawinfo.bmindx  = bitmap;
	drawinfo.pcolor  = color;
	drawinfo.ref_z   = ref_z;
	drawinfo.extinfo = pextinfo;
	drawinfo.sizebnd = sizebound;

	PRT_CreateParticleSphereObject( origin,
										PARTICLE_EXPLOSION_STARTRADIUS,
										SAT_EXPLODING,
										PARTICLE_EXPLOSION_NUMPARTICLES,
								    	lifetime, &drawinfo,
										GetObjectOwner( shippo ) );
}


// create an energy field -----------------------------------------------------
//
void SFX_CreateEnergyField( Vertex3& origin )
{
	// some particle appearance properties
	int bitmap	  = SPHERE_BM_INDX;
	int color	  = SPHERE_PARTICLE_COLOR;
extern float sphere_ref_z;
	float ref_z = sphere_ref_z;
	int sizebound = PRTSB_ENERGYFIELD;
	int lifetime  = ENERGY_FIELD_LIFETIME;

	pextinfo_s extinfo;
	pextinfo_s *pextinfo = NULL;

	if ( AUX_ENERGYFIELD_PARTICLES_EXTINFO ) {

		// alter basic attributes
		ref_z    *= 4;
		bitmap    = iter_texrgba | iter_specularadd;
//		sizebound = PRT_NO_SIZEBOUND;

		// fetch pdef
		pdef_s *pdef = PDEF_pflare03();
		if ( pdef == NULL )
			return;

		// create pextinfo
		pextinfo = &extinfo;
		PRT_InitParticleExtInfo( pextinfo, pdef, NULL, NULL );
	}

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
										LocalPlayerId );
	// maintain particle extra count
	CurrentNumPrtExtras++;
}


// show protective shield surrounding a ship for a short period of time -------
//
void SFX_FlashProtectiveShield( ShipObject *shippo )
{
	ASSERT( shippo != NULL );
	ASSERT( OBJECT_TYPE_SHIP( shippo ) );

	// dont't flash shield if already any shield-cluster attached
	if ( PRT_ObjectHasAttachedClustersOfType( shippo, SAT_NO_ANIMATION ) )
		return;

	// some particle appearance properties
	int bitmap	  = PROTSHIELD_BITMAP;
	int color	  = PROTSHIELD_COLOR;
	float ref_z = protshield_ref_z;
	int sizebound = PRTSB_PROTSHIELD;
	int lifetime  = SHIELD_FLASH_DURATION;

	pextinfo_s extinfo;
	pextinfo_s *pextinfo = NULL;

	if ( AUX_PROTSHIELD_PARTICLES_EXTINFO ) {

		// alter basic attributes
		ref_z    *= 4;
		bitmap    = iter_texrgba | iter_specularadd;
//		sizebound = PRT_NO_SIZEBOUND;

		// fetch pdef
		pdef_s *pdef = PDEF_pflare04();
		if ( pdef == NULL )
			return;

		// create pextinfo
		pextinfo = &extinfo;
		PRT_InitParticleExtInfo( pextinfo, pdef, NULL, NULL );
	}

	pdrwinfo_s drawinfo;
	drawinfo.bmindx  = bitmap;
	drawinfo.pcolor  = color;
	drawinfo.ref_z   = ref_z;
	drawinfo.extinfo = pextinfo;
	drawinfo.sizebnd = sizebound;

	// create ship centered particle sphere without animation
	PRT_CreateObjectCenteredSphere( shippo, shippo->BoundingSphere,
										SAT_NO_ANIMATION,
										SPHERE_PARTICLES,
								    	lifetime, &drawinfo,
										GetObjectOwner( shippo ) );
}


// create particle sphere for invulnerability shield (megashield) -------------
//
PRIVATE
basesphere_pcluster_s *CreateMegaSphere( ShipObject *shippo )
{
	ASSERT( shippo != NULL );
	ASSERT( OBJECT_TYPE_SHIP( shippo ) );

	// some particle appearance properties
	int bitmap	  = MEGASHIELD_BITMAP;
	int color	  = MEGASHIELD_COLOR;
	float ref_z = megashield_ref_z;
	int sizebound = PRTSB_INVULSHIELD;
	int lifetime  = MEGASHIELD_LIFETIME;

	pextinfo_s extinfo;
	pextinfo_s *pextinfo = NULL;

	if ( AUX_MEGASHIELD_PARTICLES_EXTINFO ) {

		// alter basic attributes
		ref_z    *= 4;
		bitmap    = iter_texrgba | iter_specularadd;
//		sizebound = PRT_NO_SIZEBOUND;

		// fetch pdef
		pdef_s *pdef = PDEF_pflare02();//05();
		if ( pdef == NULL )
			return NULL;

		// create pextinfo
		pextinfo = &extinfo;
		PRT_InitParticleExtInfo( pextinfo, pdef, NULL, NULL );
	}

	pdrwinfo_s drawinfo;
	drawinfo.bmindx  = bitmap;
	drawinfo.pcolor  = color;
	drawinfo.ref_z   = ref_z;
	drawinfo.extinfo = pextinfo;
	drawinfo.sizebnd = sizebound;

	basesphere_pcluster_s *cluster =
		PRT_CreateObjectCenteredSphere( shippo, shippo->BoundingSphere,
										SAT_MEGASHIELD_SPHERE,
										SPHERE_PARTICLES,
										lifetime, &drawinfo,
										GetObjectOwner( shippo ) );
	return cluster;
}


// enable invulnerability function (mega-shield) ------------------------------
//
int SFX_EnableInvulnerabilityShield( ShipObject *shippo )
{
	ASSERT( shippo != NULL );
	ASSERT( OBJECT_TYPE_SHIP( shippo ) );

	if ( NET_ConnectedPEER() ) {
		// check if enough space in RE_List
		if ( !NET_RmEvAllowed( RE_PARTICLEOBJECT ) )
			return FALSE;
	}

	basesphere_pcluster_s *attached = (basesphere_pcluster_s *) PRT_ObjectHasAttachedClustersOfType( shippo, SAT_MEGASHIELD_SPHERE );

	// don't attach more than one, only reset lifetime
	if ( attached != NULL ) {
		attached->lifetime = attached->max_life;
		return TRUE;
	}

	// create particle sphere
	CreateMegaSphere( shippo );

	if ( NET_ConnectedPEER() ) {
		// insert remote event
		Vertex3 dummyorigin;
		NET_RmEvParticleObject( POBJ_MEGASHIELD, dummyorigin );
	} 

	return TRUE;
}


// fetch invulnerability shield and info --------------------------------------
//
basesphere_pcluster_s *SFX_FetchInvulnerabilityShield( ShipObject *shippo, float *strength )
{
	ASSERT( shippo != NULL );
	ASSERT( OBJECT_TYPE_SHIP( shippo ) );

	basesphere_pcluster_s *cluster = (basesphere_pcluster_s *)
		PRT_ObjectHasAttachedClustersOfType( shippo, SAT_MEGASHIELD_SPHERE );

	if ( cluster == NULL ) {
		return NULL;
	}

	if ( strength != NULL ) {
		*strength = (float)cluster->lifetime / (float)cluster->max_life;
	}

	return cluster;
}


// enable invulnerability function (mega-shield) for remote players -----------
//
void SFX_RemoteEnableInvulnerabilityShield( int owner )
{
	// fetch pointer to remote player's ship
	ShipObject *shippo = NET_FetchOwnersShip( owner );
	ASSERT( shippo != NULL );
	ASSERT( shippo != MyShip );

	basesphere_pcluster_s *attached = (basesphere_pcluster_s *)
		PRT_ObjectHasAttachedClustersOfType( shippo, SAT_MEGASHIELD_SPHERE );

	// don't attach more than one, only reset lifetime
	if ( attached != NULL ) {
		attached->lifetime = MEGASHIELD_LIFETIME;
		return;
	}

	// create particle sphere
	CreateMegaSphere( shippo );
}


// create hull impact particles -----------------------------------------------
//
void SFX_HullImpact( ShipObject *shippo, Vertex3 *impact, Plane3 *plane )
{
	ASSERT( shippo != NULL );
	ASSERT( impact != NULL );
	ASSERT( plane != NULL );

	ASSERT( OBJECT_TYPE_SHIP( shippo ) );

	// some particle appearance properties
	int bitmap	  = PROTSHIELD_BITMAP;
	int color	  = PROTSHIELD_COLOR;
	float ref_z = protshield_ref_z;
	int sizebound = PRTSB_PROTSHIELD;
	int lifetime  = 20000; //SHIELD_FLASH_DURATION;

	pextinfo_s extinfo;
	pextinfo_s *pextinfo = NULL;

	if ( AUX_PROTSHIELD_PARTICLES_EXTINFO ) {

		// alter basic attributes
		ref_z    *= 4;
		bitmap    = iter_texrgba | iter_specularadd;
//		sizebound = PRT_NO_SIZEBOUND;

		// fetch pdef
		pdef_s *pdef = PDEF_pflare04();
		if ( pdef == NULL )
			return;

		// create pextinfo
		pextinfo = &extinfo;
		PRT_InitParticleExtInfo( pextinfo, pdef, NULL, NULL );
	}

	Vertex3 world_space;
	MtxVctMUL( shippo->ObjPosition, impact, &world_space );

	Vector3 dirvec_obj;
	dirvec_obj.X = plane->X;
	dirvec_obj.Y = plane->Y;
	dirvec_obj.Z = plane->Z;

	Vector3 dirvec;
	MtxVctMULt( shippo->ObjPosition, &dirvec_obj, &dirvec );

	geomv_t speed = FLOAT_TO_GEOMV( 0.01 );
	dirvec.X = GEOMV_MUL( dirvec.X, speed );
	dirvec.Y = GEOMV_MUL( dirvec.Y, speed );
	dirvec.Z = GEOMV_MUL( dirvec.Z, speed );

	particle_s particle;
	PRT_InitParticle( particle, bitmap, color, sizebound,
					  ref_z, &world_space, &dirvec,
					  lifetime, GetObjectOwner( shippo ), pextinfo );
	PRT_CreateLinearParticle( particle );
}


// create propulsion fumes for spaceship --------------------------------------
//
void SFX_CreatePropulsionParticles( ShipObject *shippo )
{
	ASSERT( shippo != NULL );
	ASSERT( OBJECT_TYPE_SHIP( shippo ) );

//TEST:
//FIXME:
//////////////////////
return;
//////////////////////

	int bitmap	  = PROPFUMES_BITMAP;
	int color	  = PROPFUMES_COLOR;
	float ref_z = propfumes_ref_z;

	// alter basic attributes
//	ref_z /= 2;
	bitmap = iter_rgbtexa | iter_alphablend;
	color  = 0;

	// fetch pdef
	pdef_s *pdef = PDEF_psmoke02();
	if ( pdef == NULL )
		return;

	// create pextinfo
	pextinfo_s extinfo;
	PRT_InitParticleExtInfo( &extinfo, pdef, NULL, NULL );

	// first particle ---
	Vertex3 object_space;
	object_space.X = shippo->Fume_X[ 0 ];
	object_space.Y = shippo->Fume_Y;
	object_space.Z = shippo->Fume_Z;

	Vertex3 world_space;
	MtxVctMUL( shippo->ObjPosition, &object_space, &world_space );

	Vector3 dirvec;
	fixed_t speed = shippo->FumeSpeed;
	DirVctMUL( shippo->ObjPosition, FIXED_TO_GEOMV( speed ), &dirvec );

	particle_s particle;
	PRT_InitParticle( particle, bitmap, color, PROPFUMES_SIZE_BOUNDARY,
				  	  ref_z, &world_space, &dirvec,
					  shippo->FumeLifeTime, LocalPlayerId,
//					  NULL );
					  &extinfo );
	PRT_CreateLinearParticle( particle );

	// second particle ---
	object_space.X = shippo->Fume_X[ 3 ];
	MtxVctMUL( shippo->ObjPosition, &object_space, &world_space );

	PRT_InitParticle( particle, bitmap, color, PROPFUMES_SIZE_BOUNDARY,
					  ref_z, &world_space, &dirvec,
				  	  shippo->FumeLifeTime, LocalPlayerId,
//					  NULL );
					  &extinfo );
	PRT_CreateLinearParticle( particle );
}


// create propulsion fumes for missiles ---------------------------------------
//
void SFX_MissilePropulsion( MissileObject *missile )
{
	ASSERT( missile != NULL );
	ASSERT( OBJECT_TYPE_MISSILE( missile ) );

	int bitmap	  = PROPFUMES_BITMAP;
	int color	  = PROPFUMES_COLOR;
	float ref_z = propfumes_ref_z;
	int sizebound = PROPFUMES_SIZE_BOUNDARY;
	int lifetime  = PROP_FUME_LIFETIME;

	// alter basic attributes
//	ref_z    /= 2;
	bitmap    = iter_rgbtexa | iter_alphablend;
	color     = 0;
	sizebound = PRT_NO_SIZEBOUND;
	lifetime  = 1800;

	// fetch pdef
	pdef_s *pdef = PDEF_psmoke01();
	if ( pdef == NULL )
		return;

	// create pextinfo
	pextinfo_s extinfo;
	PRT_InitParticleExtInfo( &extinfo, pdef, NULL, NULL );

	//TODO:
	// make propulsion framerate independent

	// create particle ---
	Vertex3 object_space;
	int x = ( PROPFUMES_SCATTER_BOUNDARY ) - ( RAND() & ( PROPFUMES_SCATTER_BOUNDARY * 2 ) );
	int y = ( PROPFUMES_SCATTER_BOUNDARY ) - ( RAND() & ( PROPFUMES_SCATTER_BOUNDARY * 2 ) );
	object_space.X = INT_TO_GEOMV( x );
	object_space.Y = INT_TO_GEOMV( y );
	object_space.Z = GEOMV_0; //PROP_FUME_START_Z;

	Vertex3 world_space;
	MtxVctMUL( missile->ObjPosition, &object_space, &world_space );

	Vertex3 drift;
	object_space.X = FLOAT_TO_GEOMV( PROPFUMES_DRIFT_SPEED * x );
	object_space.Y = FLOAT_TO_GEOMV( PROPFUMES_DRIFT_SPEED * y );
	object_space.Z = GEOMV_0; //PROP_FUME_START_Z;

	MtxVctMULt( missile->ObjPosition, &object_space, &drift );

	Vector3 dirvec;
	fixed_t speed = PROP_FUME_SPEED; //missile->Speed/6;

	DirVctMUL( missile->ObjPosition, FIXED_TO_GEOMV( speed ), &dirvec );
	dirvec.X += drift.X;
	dirvec.Y += drift.Y;
	dirvec.Z += drift.Z;

	particle_s particle;
	PRT_InitParticle( particle, bitmap, color, sizebound,
				  	  ref_z, &world_space, &dirvec,
					  lifetime, LocalPlayerId,
//					  NULL );
					  &extinfo );
	PRT_CreateLinearParticle( particle );
}


// create stargate ------------------------------------------------------------
//
void SFX_CreateStargate( ShipObject *shippo )
{
	ASSERT( shippo != NULL );
	ASSERT( OBJECT_TYPE_SHIP( shippo ) );

	// basic attributes
	float ref_z = 2000.0f;
	int bitmap = iter_texrgba | iter_specularadd;
	int color  = 0;

	// fetch pdef
	pdef_s *pdef = PDEF_stargate();
	if ( pdef == NULL )
		return;

	// create pextinfo
	pextinfo_s extinfo;
	PRT_InitParticleExtInfo( &extinfo, pdef, NULL, NULL );

	// position in object space
	Vertex3 object_space;
	object_space.X = GEOMV_0;
	object_space.Y = GEOMV_0;
	object_space.Z = INT_TO_GEOMV( ( shippo == MyShip ) ? 50 : 0 );

	Vertex3 world_space;
	MtxVctMUL( shippo->ObjPosition, &object_space, &world_space );

	Vector3 dirvec;
	dirvec.X = GEOMV_0;
	dirvec.Y = GEOMV_0;
	dirvec.Z = GEOMV_0;

	particle_s particle;
	PRT_InitParticle( particle, bitmap, color, PRT_NO_SIZEBOUND,
				  	  ref_z, &world_space, &dirvec,
					  32*40*2, LocalPlayerId,
					  &extinfo );
	PRT_CreateLinearParticle( particle );
}


// extra generation animation -------------------------------------------------
//
void SFX_CreateExtra( ExtraObject *extra )
{
	ASSERT( extra != NULL );
	ASSERT( OBJECT_TYPE_EXTRA( extra ) );

	// basic attributes
	float ref_z = 350.0f;
	int bitmap = iter_texrgba | iter_specularadd;
	int color  = 0;

	// fetch pdef
	pdef_s *pdef = PDEF_extragen();
	if ( pdef == NULL )
		return;

	// create pextinfo
	pextinfo_s extinfo;
	PRT_InitParticleExtInfo( &extinfo, pdef, NULL, NULL );

	// position in object space
	Vertex3 object_space;
	object_space.X = GEOMV_0;
	object_space.Y = GEOMV_0;
	object_space.Z = GEOMV_0;

	Vertex3 world_space;
	MtxVctMUL( extra->ObjPosition, &object_space, &world_space );

	Vector3 dirvec;
	dirvec.X = GEOMV_0;
	dirvec.Y = GEOMV_0;
	dirvec.Z = GEOMV_0;

	particle_s particle;
	PRT_InitParticle( particle, bitmap, color, PRT_NO_SIZEBOUND,
				  	  ref_z, &world_space, &dirvec,
					  32*40*2, LocalPlayerId,
					  &extinfo );
	PRT_CreateLinearParticle( particle );
}


// extra vanishing animation --------------------------------------------------
//
void SFX_VanishExtra( ExtraObject *extra )
{
	SFX_CreateExtra( extra );
}



