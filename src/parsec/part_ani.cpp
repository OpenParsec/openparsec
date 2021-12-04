/*
 * PARSEC - Behavior and Appearance Animation
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:36 $
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
#include "net_defs.h"

// mathematics header
#include "utl_math.h"

// particle types
#include "parttype.h"

// local module header
#include "part_ani.h"

// proprietary module headers
#include "obj_ctrl.h"
#include "obj_expl.h"
#include "obj_game.h"
#include "part_api.h"
#include "g_sfx.h"
#include "part_sys.h"
#include "g_wfx.h"


// flags
//#define ENERGYFIELD_DESTRUCTION_MESSAGE
#define DO_BRUTEFORCE_CDETECTION	// do collision detection for lightning
#define PROTECT_ANIM_CLUSTER_WALK



// lightning segment deviation/variation
#define MAX_LIGHTNING_SEG_DEVIATION			1120
#define LIGHTNING_SEG_DEVIATION_FAC			0x10

// reference z of lightning impact particles
#define IMPACT_PARTICLES_REF_Z				3.6f



// reference z values for particles -------------------------------------------
//
float impact_particles_ref_z = 1.0f;


// init reference z values for particles according to resolution --------------
//
void PAN_InitParticleSizes( float resoscale )
{
	impact_particles_ref_z	= resoscale * IMPACT_PARTICLES_REF_Z;
}


// disable a single particle in a specified cluster ---------------------------
//
PRIVATE
void DisableParticle( pcluster_s *cluster, int pid )
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


// brute force collision detection of linear particle with all ships ----------
//
void LinearParticleCollision( linear_pcluster_s *cluster, int pid )
{
	ASSERT( cluster != NULL );
	ASSERT( ( pid >= 0 ) && ( pid < cluster->numel ) );
	ASSERT( cluster->rep[ pid ].flags & PARTICLE_COLLISION );

	//NOTE:
	// this is set as callback by PART_WFX::MaintainHelix()
	// to detect collisions of helix particles.

	int owner = cluster->rep[ pid ].owner;

	// check local ship
	if ( ( owner != LocalPlayerId ) && NetJoined &&
		PRT_ParticleInBoundingSphere( MyShip, cluster->rep[ pid ].position ) ) {

		// disable particle
		cluster->rep[ pid ].flags &= ~PARTICLE_ACTIVE;

		//TODO:
		// detect hull impact

		OBJ_EventShipImpact( MyShip, TRUE );

        switch ( cluster->rep[ pid ].flags & PARTICLE_IS_MASK ) {

            case PARTICLE_IS_HELIX :
                OBJ_ShipHelixDamage( MyShip, owner );
                break;
            case PARTICLE_IS_PHOTON :
                OBJ_ShipPhotonDamage( MyShip, owner );
                break;
            default:
                break;
        }
		return;
	}

	// check shiplist
	ShipObject *walkships = FetchFirstShip();
	for ( ; walkships; walkships = (ShipObject*) walkships->NextObj ) {

		// prevent collision with owner of particle
		if ( NetConnected && ( GetObjectOwner( walkships ) == (dword)owner ) )
			continue;

		if ( !PRT_ParticleInBoundingSphere( walkships, cluster->rep[ pid ].position ) )
			continue;

		// disable particle
		cluster->rep[ pid ].flags &= ~PARTICLE_ACTIVE;

		//TODO:
		// detect hull impact

		OBJ_EventShipImpact( walkships, TRUE );

        switch ( cluster->rep[ pid ].flags & PARTICLE_IS_MASK ) {

            case PARTICLE_IS_HELIX :
                OBJ_ShipHelixDamage( walkships, owner );
                break;
            case PARTICLE_IS_PHOTON :
                OBJ_ShipPhotonDamage( walkships, owner );
                break;
            default:
                break;
        }
    }
}


// calculate position advance of constant velocity particles ------------------
//
PRIVATE
void CalcConstantVelocityAnimation( linear_pcluster_s *cluster )
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
		if ( ( cluster->rep[ curp ].lifetime -= CurScreenRefFrames ) < 0 )
			DisableParticle( cluster, curp );

		// move along vector
		Vector3 advvec;
		advvec.X = cluster->rep[ curp ].velocity.X * CurScreenRefFrames;
		advvec.Y = cluster->rep[ curp ].velocity.Y * CurScreenRefFrames;
		advvec.Z = cluster->rep[ curp ].velocity.Z * CurScreenRefFrames;

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


// calc init position for particle stochastically placed on spherical surface -
//
void CalcSphereParticlePosition( Vertex3& position, geomv_t radius, int spheretype )
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


// calc rotation of sphere particle -------------------------------------------
//
void CalcSphereParticleRotation( Vertex3& position, bams_t pitch, bams_t yaw, bams_t roll )
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


// calc explosion trajectory of sphere particle -------------------------------
//
PRIVATE
void CalcSphereParticleExplosion( Vertex3& position, geomv_t speed )
{
	Vector3 vec = position;
	NormVctX( &vec );

	position.X += GEOMV_MUL( vec.X, speed );
	position.Y += GEOMV_MUL( vec.Y, speed );
	position.Z += GEOMV_MUL( vec.Z, speed );
}


// calc contraction of sphere -------------------------------------------------
//
int CalcSphereContraction( Vertex3& position, geomv_t speed )
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


// calculate pulsating sphere -------------------------------------------------
//
PRIVATE
void CalcSpherePulse( Vertex3& position, Vertex3& pulsebase, geomv_t pulseval )
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


// calc animation of ship centered particle sphere ----------------------------
//
PRIVATE
void CalcObjectCenteredSphereAnimation( basesphere_pcluster_s *cluster )
{
	ASSERT( cluster != NULL );
	ASSERT( ( cluster->type & CT_TYPEMASK ) == CT_OBJECTCENTERED_SPHERE );

	if ( cluster->lifetime > 0 ) {

		cluster->lifetime -= CurScreenRefFrames;

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
				bams_t pitch = cluster->rot.pitch * CurScreenRefFrames;
				bams_t yaw   = cluster->rot.yaw   * CurScreenRefFrames;
				bams_t roll  = cluster->rot.roll  * CurScreenRefFrames;

				for ( int pid = 0; pid < cluster->numel; pid++ )
					CalcSphereParticleRotation( cluster->rep[ pid ].position,
												pitch, yaw, roll );
				}
				break;

			case SAT_STOCHASTIC_MOTION:
				if ( ( cluster->rand.fcount -= CurScreenRefFrames ) < 0 ) {
					cluster->rand.fcount = cluster->rand.speed;

					geomv_t radius = cluster->rand.radius;

					for ( int pid = 0; pid < cluster->numel; pid++ ) {
						CalcSphereParticlePosition( cluster->rep[ pid ].position,
													radius, SAT_SPHERETYPE_NORMAL );
					}
				}
				break;
		}

	} else {

		switch ( cluster->animtype & SAT_BASIC_ANIM_MASK ) {

			default:

				if ( cluster->animtype & SAT_RESET_MEGASHIELD_FLAG ) {
					ASSERT( cluster->baseobject != NULL );
					if ( OBJECT_TYPE_SHIP( cluster->baseobject ) ) {
						ShipObject *baseship = (ShipObject *) cluster->baseobject;
						baseship->Specials &= ~SPMASK_INVULNERABILITY;
						baseship->MegaShieldAbsorption = 0;
					}
				}

				PRT_DeleteCluster( cluster );
		}
	}
}


// calc animation of autonomous particle sphere -------------------------------
//
PRIVATE
void CalcSphereObjectAnimation( sphereobj_pcluster_s *cluster )
{
	ASSERT( cluster != NULL );
	ASSERT( ( cluster->type & CT_TYPEMASK ) == CT_PARTICLE_SPHERE );

	if ( cluster->lifetime > 0 ) {

		refframe_t refframes = CurScreenRefFrames;
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
				bams_t pitch = cluster->rot.pitch * CurScreenRefFrames;
				bams_t yaw   = cluster->rot.yaw   * CurScreenRefFrames;
				bams_t roll  = cluster->rot.roll  * CurScreenRefFrames;

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

				bams_t pitch = cluster->puls.pitch * CurScreenRefFrames;
				bams_t yaw   = cluster->puls.yaw   * CurScreenRefFrames;
				bams_t roll  = cluster->puls.roll  * CurScreenRefFrames;

				for ( int pid = 0; pid < cluster->numel; pid++ ) {
					CalcSphereParticleRotation( cluster->rep[ pid + cluster->numel ].position,
												pitch, yaw, roll );
					CalcSpherePulse( cluster->rep[ pid ].position,
									 cluster->rep[ pid + cluster->numel ].position,
									 pulseval );
				}
				// update cluster radius
				cluster->bdsphere = cluster->puls.midradius + pulseval;

				cluster->puls.current_t += cluster->puls.frequency * CurScreenRefFrames;
				}
				break;

			case SAT_CONTRACTING:
				{
				bams_t pitch = cluster->cont.pitch * CurScreenRefFrames;
				bams_t yaw   = cluster->cont.yaw   * CurScreenRefFrames;
				bams_t roll  = cluster->cont.roll  * CurScreenRefFrames;

				for ( int pid = 0; pid < cluster->numel; pid++ ) {
					CalcSphereParticleRotation( cluster->rep[ pid ].position,
												pitch, yaw, roll );
				}

				if ( cluster->cont.expandtime > 0 ) {
					cluster->cont.expandtime -= CurScreenRefFrames;

					geomv_t contraction = cluster->cont.speed * CurScreenRefFrames;
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
				bams_t pitch = cluster->cont.pitch * CurScreenRefFrames;
				bams_t yaw   = cluster->cont.yaw   * CurScreenRefFrames;
				bams_t roll  = cluster->cont.roll  * CurScreenRefFrames;

				for ( int pid = 0; pid < cluster->numel; pid++ ) {
					CalcSphereParticleRotation( cluster->rep[ pid ].position,
												pitch, yaw, roll );
				}
				int delit = 0;

				{
					geomv_t contraction = cluster->cont.speed * CurScreenRefFrames;
					for ( int pid = 0; pid < cluster->numel; pid++ ) {
						delit += CalcSphereContraction( cluster->rep[ pid ].position, contraction );
					}
					// update cluster radius
					cluster->bdsphere -= contraction;
				}

				if ( delit ) {

					if ( cluster->animtype & SAT_DECREMENT_EXTRA_COUNTER ) {
						CurrentNumPrtExtras--;

#ifdef ENERGYFIELD_DESTRUCTION_MESSAGE
						ShowMessage( "extracount decreased" );
#endif
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


// visualize impact of lightning beams onto protective hull -------------------
//
INLINE
void VisualizeLightningImpact( ShipObject *shippo, Vertex3& impactpoint )
{
	ASSERT( shippo != NULL );

	pdrwinfo_s drawinfo;
	drawinfo.bmindx  = SPHERE_BM_INDX;
	drawinfo.pcolor  = SPHERE_PARTICLE_COLOR;
	drawinfo.ref_z	 = impact_particles_ref_z;
	drawinfo.extinfo = NULL;
	drawinfo.sizebnd = partbitmap_size_bound;

	PRT_CreateParticleSphereObject( impactpoint,
										FLOAT_TO_GEOMV( 2.5 ),
										SAT_STOCHASTIC_MOTION,
										30, 20,
										&drawinfo,
										GetObjectOwner( shippo ) );
}


// brute force collision detection of lightning particle with all ships -------
//
PRIVATE
int CheckLightningParticleShipCollision( Vertex3& particlepos, int owner )
{
	// check local ship
	if ( ( owner != LocalPlayerId ) && NetJoined &&
		PRT_ParticleInBoundingSphere( MyShip, particlepos ) ) {

		if ( MyShip->MegaShieldAbsorption == 0 )
			SetScreenWhite = 20 * 16; //FIXME: nicht fix einstellen!!

		//TODO:
		// detect hull impact

		OBJ_EventShipImpact( MyShip, TRUE );
		VisualizeLightningImpact( MyShip, particlepos );
		OBJ_ShipLightningDamage( MyShip, owner );

		return TRUE;
	}

	// check shiplist
	ShipObject *walkships = FetchFirstShip();
	for ( ; walkships; walkships = (ShipObject*) walkships->NextObj ) {

		// prevent collision with owner of particle
		if ( NetConnected && ( GetObjectOwner( walkships ) == (dword)owner ) )
			continue;

		if ( !PRT_ParticleInBoundingSphere( walkships, particlepos ) )
			continue;

		//TODO:
		// detect hull impact

		OBJ_EventShipImpact( walkships, TRUE );
		VisualizeLightningImpact( walkships, particlepos );
		OBJ_ShipLightningDamage( walkships, owner );

		return TRUE;
	}

	return FALSE;
}


// reevaluate positions of all particles comprising a single lightning beam ---
//
PRIVATE
void SetLightningParticlePosition( lightning_pcluster_s *cluster, particle_s particles[], Vertex3& current, Xmatrx tmatrx )
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

		if ( CheckLightningParticleShipCollision( cpos, particles[ pid ].owner ) ) {
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


// animate lightning particles ------------------------------------------------
//
PRIVATE
void CalcLightningAnimation( lightning_pcluster_s *cluster )
{
	ASSERT( cluster != NULL );
	ASSERT( ( cluster->type & CT_TYPEMASK ) == CT_LIGHTNING );

	if ( ( cluster->framecount -= CurScreenRefFrames ) < 0 ) {
		cluster->framecount = cluster->sizzlespeed;

#if !( CT_LIGHTNING & CT_GENOBJECTRELATIVE_OBJ_MASK )
		ASSERT( OBJECT_TYPE_SHIP( shippo ) );
#endif

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


// invoke callback function for animation of generic callback clusters --------
//
INLINE
void CalcCallbackTrajectoryAnimation( callback_pcluster_s *cluster )
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


// animate customdraw particles -----------------------------------------------
//
INLINE
void CalcCustomDrawAnimation( customdraw_pcluster_s *cluster )
{
	ASSERT( cluster != NULL );
	ASSERT( ( cluster->type & CT_TYPEMASK ) == CT_CUSTOMDRAW );

	// invoke callback function
	if ( cluster->callback != NULL )
		cluster->callback( cluster );

	//NOTE:
	// the callback function is invoked for the
	// entire cluster, not single particles.

	// check if cluster should be removed
	int numactive = 0;
	for ( int curp = 0; curp < cluster->numel; curp++ ) {

		if ( ( cluster->rep[ curp ].flags & PARTICLE_ACTIVE ) == 0 )
			continue;
		else
			numactive++;

		// maintain lifetime
		if ( ( cluster->rep[ curp ].lifetime -= CurScreenRefFrames ) < 0 )
			DisableParticle( cluster, curp );
	}

	// remove cluster if no active particles contained anymore
	if ( numactive == 0 ) {
		if ( CustomDrawCluster == cluster )
			CustomDrawCluster = NULL;
		PRT_DeleteCluster( cluster );
	}
}


// animate genobject geometry particles ---------------------------------------
//
INLINE
void CalcGenObjectAnimation( genobject_pcluster_s *cluster )
{
	ASSERT( cluster != NULL );
	ASSERT( ( cluster->type & CT_TYPEMASK ) == CT_GENOBJECT_PARTICLES );

	// invoke callback function
	if ( cluster->callback != NULL ) {
		cluster->callback( cluster );
	}

	//NOTE:
	// the callback function is invoked for the
	// entire cluster, not single particles.

	//NOTE:
	// there is no lifetime checking whatsoever.
}


// particle behavior animation: call trajectory animation function ------------
//
INLINE
void AnimateClusterBehavior( pcluster_s *cluster )
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
			CalcObjectCenteredSphereAnimation( (basesphere_pcluster_s *) cluster );
			break;

		case ( CT_PHOTON_SPHERE & CT_TYPEENUMERATIONMASK ):
			// already called in G_SUPP::MaintainDurationWeapons()
			// WFX_CalcPhotonSphereAnimation( (photon_sphere_pcluster_s *) cluster );
			break;

        case ( CT_PARTICLE_SPHERE & CT_TYPEENUMERATIONMASK ):
			CalcSphereObjectAnimation( (sphereobj_pcluster_s *) cluster );
			break;

		case ( CT_CALLBACK_TRAJECTORY & CT_TYPEENUMERATIONMASK ):
			CalcCallbackTrajectoryAnimation( (callback_pcluster_s *) cluster );
			break;

		case ( CT_CUSTOMDRAW & CT_TYPEENUMERATIONMASK ):
			CalcCustomDrawAnimation( (customdraw_pcluster_s *) cluster );
			break;

		case ( CT_GENOBJECT_PARTICLES & CT_TYPEENUMERATIONMASK ):
			CalcGenObjectAnimation( (genobject_pcluster_s *) cluster );
			break;
	}
}


// particle appearance animation: perform texture animation -------------------
//
INLINE
void AnimateClusterAppearance( pcluster_s *cluster )
{
	ASSERT( cluster != NULL );

	//NOTE:
	// if no particle in this cluster has an extinfo attached,
	// CT_HINT_NO_APPEARANCE_ANIMATION should be specified to
	// avoid the unnecessary overhead of checking all particles.

	// avoid unnecessary computations if homogeneous extinfo
	int numanim = cluster->numel;
	if ( numanim == 0 )
		return;
	if ( cluster->type & CT_CLUSTER_GLOBAL_EXTINFO )
		numanim = 1;

	// process all particles
	for ( int curp = 0; curp < numanim; curp++ ) {

		particle_s *particle = &cluster->rep[ curp ];
		if ( particle->extinfo != NULL ) {

			pextinfo_s *extinfo = particle->extinfo;

			pdef_s *pdef = extinfo->partdef; //TODO: partdef_dest
			ASSERT( pdef != NULL );

			// check texture-map frame
			if ( ( extinfo->tex_time -= CurScreenRefFrames ) < 0 ) {

				if ( extinfo->tex_pos == pdef->tex_end ) {
					// restart at repeat position
					extinfo->tex_pos = pdef->tex_rep;
				} else {
					// advance to next table entry
					extinfo->tex_pos++;
				}

				// set deltatime for next frame
				ASSERT( pdef->tex_table != NULL );
				texfrm_s *curtexframe = &pdef->tex_table[ extinfo->tex_pos ];
				extinfo->tex_time = curtexframe->deltatime;
			}

			// trafo is optional
			if ( pdef->xfo_table != NULL ) {

				// check texture-trafo frame
				if ( ( extinfo->xfo_time -= CurScreenRefFrames ) < 0 ) {

					if ( extinfo->xfo_pos == pdef->xfo_end ) {
						// restart at repeat position
						extinfo->xfo_pos = pdef->xfo_rep;
					} else {
						// advance to next table entry
						extinfo->xfo_pos++;
					}

					// set deltatime for next frame
					xfofrm_s *curxfoframe = &pdef->xfo_table[ extinfo->xfo_pos ];
					extinfo->xfo_time = curxfoframe->deltatime;
				}
			}
		}
	}
}


// do particle animation stuff ------------------------------------------------
//
void PAN_AnimateParticles()
{
	//NOTE:
	// even invisible (culled) clusters will be animated by
	// this function. to alleviate the potential overhead
	// involved several hints will be used to cut down on
	// unnecessary computations. e.g., these hints indicate
	// whether animation of invisible clusters is necessary
	// for consistency of animation or if animation can be
	// stalled during invisibility (e.g., rotating spheres).
	// if an entire cluster consists of identical particles
	// it is also possible to just animate the appearance
	// of the first particle and reuse it for all others.

	// walk list of clusters
	for ( pcluster_s *cluster = Particles->next; cluster->next; ) {

		// save next pointer to allow animation functions deletion of clusters
		pcluster_s *nextcluster = cluster->next;

		//TODO:
		// - use hints whether to stall animation if cluster invisible.
		// CT_HINT_CULL_APPEARANCE_ANIMATION
		// CT_HINT_CULL_POSITIONAL_ANIMATION

		// appearance animation:
		// perform particle texture animation
		if ( ( cluster->type & CT_HINT_NO_APPEARANCE_ANIMATION ) == 0 ) {
			AnimateClusterAppearance( cluster );
		}

		// behavior animation:
		// call trajectory animation function according to type identifier
		if ( ( cluster->type & CT_HINT_NO_POSITIONAL_ANIMATION ) == 0 ) {
			AnimateClusterBehavior( cluster );
		}

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



