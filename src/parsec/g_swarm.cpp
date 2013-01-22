/*
 * PARSEC - Swarm Missiles
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:37 $
 *
 * Orginally written by:
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   2000
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
#include "aud_defs.h"
#include "net_defs.h"
#include "sys_defs.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "g_swarm.h"

// proprietary module headers
#include "con_arg.h"
#include "con_com.h"
#include "con_main.h"
#include "obj_ctrl.h"
#include "obj_expl.h"
#include "obj_game.h"
#include "part_api.h"
#include "part_def.h"
#include "part_sys.h"


// flags
//#define PARTICLES_DIE_AFTER_FIRST_COLLISION


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
#define ANIM_TIMESLICE		7						// animate as if CurScreenRefFrames == 7
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


// helper macros --------------------------------------------------------------
//
#define X( t, b )			( swarm->x[ ( t ) * swarm->num + ( b ) ] )
#define Y( t, b )			( swarm->y[ ( t ) * swarm->num + ( b ) ] )
#define Z( t, b )			( swarm->z[ ( t ) * swarm->num + ( b ) ] )
#define balance_rand( v )	( ( SWARM_rand() % v ) - ( ( v ) / 2 ) )


static unsigned long int nextrand = 1;


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
GenObject *SWARM_Init( int owner, Vertex3 *origin, ShipObject *targetpo, int randseed )
{
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
	if ( MyShip->HostObjNumber == ShipHostObjId( owner ) ) {

		shippo = MyShip;

	} else {

		while ( shippo && ( shippo->HostObjNumber != ShipHostObjId( owner ) ) )
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

	// create particles
	pdef_s *pdef = PRT_AcquireParticleDefinition( "swarm1", NULL );
	if ( pdef == NULL ) {
		return NULL;
	}

	// create extinfo
	static pextinfo_s extinfo;
	PRT_InitParticleExtInfo( &extinfo, pdef, NULL, NULL );

	extern float cur_particle_resoscale;

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
int SWARM_Animate( callback_pcluster_s* cluster )
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
	if ( ( shippo != NULL ) || ( swarm->targetpo == MyShip ) ) {
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

		// check for collision with local ship
		if ( ( curparticle->owner != LocalPlayerId ) && NetJoined &&
			 PRT_ParticleInBoundingSphere( MyShip, curparticle->position ) ) {

#ifdef PARTICLES_DIE_AFTER_FIRST_COLLISION
			// disable particle
			curparticle->flags &= ~PARTICLE_ACTIVE;
			numactive--;
#endif

			OBJ_EventShipImpact( MyShip, TRUE );
			OBJ_ShipSwarmDamage( MyShip, curparticle->owner );

			if ( MyShip->CurDamage > MyShip->MaxDamage ) {
				for ( int ppid = 0; ppid < swarm->num; ppid++ ) {

					refframe_t newlifetime = LIFETIME_AFTEREXPL + balance_rand( LIFETIME_VARIANCE );

					if ( cluster->rep[ ppid ].lifetime >= newlifetime )
						cluster->rep[ ppid ].lifetime = newlifetime;
				}
			}

			continue;
		}

		// check for collision with other ships
		ShipObject *walkships = FetchFirstShip();
		for ( ; walkships; walkships = (ShipObject*) walkships->NextObj ) {

			// prevent collision with owner of particle
			if ( NetConnected && ( GetObjectOwner( walkships ) == (dword)curparticle->owner ) )
				continue;

			if ( !PRT_ParticleInBoundingSphere( walkships, curparticle->position ) )
				continue;

#ifdef PARTICLES_DIE_AFTER_FIRST_COLLISION
			// disable particle
			curparticle->flags &= ~PARTICLE_ACTIVE;
			numactive--;
#endif

			OBJ_EventShipImpact( walkships, TRUE );
			OBJ_ShipSwarmDamage( walkships, curparticle->owner );

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
		AUD_SwarmMissilesOff( &swarm->dummyobj );
		PRT_DeleteCluster( cluster );
		return FALSE;
	}

	return TRUE;
}


// ----------------------------------------------------------------------------
//
void SWARM_TimedAnimate( callback_pcluster_s* cluster )
{
	ASSERT( cluster != NULL );

	// get pointer to auxstorage
	swarm_state_s *swarm = (swarm_state_s *) &cluster->userinfo[ 1 ];

	refframe_t elapsedtime = CurScreenRefFrames + swarm->timerest;

	elapsedtime -= ANIM_TIMESLICE;

	while ( elapsedtime >= 0 ) {
		if ( SWARM_Animate( cluster ) == FALSE )
			return;
		elapsedtime -= ANIM_TIMESLICE;
	}

	swarm->timerest = ANIM_TIMESLICE + elapsedtime;
}


