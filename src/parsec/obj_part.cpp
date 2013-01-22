/*
 * PARSEC - Attached Object Particles
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:35 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999-2001
 *   Copyright (c) Michael Woegerbauer <maiki@parsec.org> 1999
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

// local module header
#include "obj_part.h"

// proprietary module headers
#include "obj_clas.h"
#include "obj_creg.h"
#include "obj_name.h"
#include "part_api.h"
#include "part_def.h"


// flags
#define EXTRA_CLUSTER_FOR_THRUST_PARTICLES



// render modes for attached particles ----------------------------------------
//
static dword particle_rendermode[] = {

	// PARTICLE_RENDERMODE_POSLIGHT
	PART_REND_POINTVIS | PART_REND_NODEPTHCMP | PART_REND_NODEPTHSCALE,

	// PARTICLE_RENDERMODE_THRUST
	PART_REND_POINTVIS | PART_REND_NODEPTHCMP,
//	PART_REND_NONE,

	// PARTICLE_RENDERMODE_MISSILE
	PART_REND_NONE,
//	PART_REND_POINTVIS | PART_REND_NODEPTHCMP,
};


// size of clusters for thrust particles --------------------------------------
//
#define THRUST_CLUSTER_SIZE		256


// ----------------------------------------------------------------------------
//
extern float cur_particle_resoscale;


// macros to ease particle instantiation and attachment -----------------------
//
#define CREATE_EXT_INFO(r) \
	int bitmap = ( iter_texrgba | iter_specularadd ) | (r); \
	Vertex3 origin; \
	particle_s particle; \
	pextinfo_s extinfo; \
	PRT_InitParticleExtInfo( &extinfo, pdef, NULL, NULL );

#define CREATE_OBJECT_PARTICLE(x,y,z) \
	origin.X = FLOAT_TO_GEOMV( (x) + (OBJPARTOFS_X) ); \
	origin.Y = FLOAT_TO_GEOMV( (y) + (OBJPARTOFS_Y) ); \
	origin.Z = FLOAT_TO_GEOMV( (z) + (OBJPARTOFS_Z) ); \
	PRT_InitParticle( particle, bitmap, 0, PRT_NO_SIZEBOUND, \
					  ref_z, &origin, NULL, INFINITE_LIFETIME, \
					  LocalPlayerId, &extinfo ); \
	cluster = PRT_CreateGenObjectParticle( particle, obj, cluster );


// fixed offset to hard-coded coordinate specification ------------------------
//
#define OBJPARTOFS_X	0.0
#define OBJPARTOFS_Y	0.0
#define OBJPARTOFS_Z	0.0


// attach object particles to firebird ----------------------------------------
//
PRIVATE
void AttachParticlesFirebird( GenObject *obj )
{
	ASSERT( obj != NULL );
	ASSERT( OBJECT_TYPE_SHIP( obj ) );

	pdef_s				 *pdef	  = NULL;
	genobject_pcluster_s *cluster = NULL;

	// poslights
	pdef = PRT_AcquireParticleDefinition( "poslt04", NULL );
	if ( pdef != NULL ) {

		// extinfo: flare
		CREATE_EXT_INFO( particle_rendermode[ PARTICLE_RENDERMODE_POSLIGHT ] );

		float ref_z = 0.23f * cur_particle_resoscale;
		CREATE_OBJECT_PARTICLE(  9.9, 0, 25 );
//		CREATE_OBJECT_PARTICLE( -7.8, 0, 25 );
	}

	pdef = PRT_AcquireParticleDefinition( "poslt05", NULL );
	if ( pdef != NULL ) {

		// extinfo: flare
		CREATE_EXT_INFO( particle_rendermode[ PARTICLE_RENDERMODE_POSLIGHT ] );

		float ref_z = 0.23f * cur_particle_resoscale;
//		CREATE_OBJECT_PARTICLE(  9.9, 0, 25 );
		CREATE_OBJECT_PARTICLE( -7.8, 0, 25 );
	}

	// thrust
	pdef = PRT_AcquireParticleDefinition( "thrust1", NULL );
	if ( pdef != NULL ) {

		// extinfo: light
		CREATE_EXT_INFO( particle_rendermode[ PARTICLE_RENDERMODE_THRUST ] );

#ifdef EXTRA_CLUSTER_FOR_THRUST_PARTICLES

		// one cluster for thrust particles only
		cluster = PRT_CreateGenObjectParticleCluster(
					obj, THRUST_CLUSTER_SIZE, NULL, TRUE );
		cluster->type |= CT_HINT_PARTICLES_IDENTICAL;
		cluster->type |= CT_CLUSTER_GLOBAL_EXTINFO;
		cluster->type |= CT_HINT_PARTICLES_HAVE_EXTINFO;
#endif
		float ref_z = 12.0f * cur_particle_resoscale;
		float z_offset = -0.6f; // offset to prevent clipping against ship
		CREATE_OBJECT_PARTICLE(  7.50, -5.00,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE(  8.53, -4.68,  -6.60 );
		CREATE_OBJECT_PARTICLE(  9.56, -4.36,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( 10.59, -4.04,  -6.60 );
		CREATE_OBJECT_PARTICLE( 11.62, -3.72,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( 12.65, -3.40,  -6.60 );
		CREATE_OBJECT_PARTICLE( 13.68, -3.08,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( 14.71, -2.76,  -6.60 );
		CREATE_OBJECT_PARTICLE( 15.74, -2.44,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( 16.77, -2.12,  -6.60 );
		CREATE_OBJECT_PARTICLE( 17.80, -1.80,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( 18.83, -1.48,  -6.60 );
		CREATE_OBJECT_PARTICLE( 19.86, -1.16,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( 20.89, -0.84,  -6.60 );
		CREATE_OBJECT_PARTICLE( 21.92, -0.52,  -6.60 + z_offset);
		CREATE_OBJECT_PARTICLE( 23.00, -0.20,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( 23.00,  0.20,  -6.60 );
//		CREATE_OBJECT_PARTICLE( 21.92,  0.52,  -6.60 );
		CREATE_OBJECT_PARTICLE( 20.89,  0.84,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( 19.86,  1.16,  -6.60 );
		CREATE_OBJECT_PARTICLE( 18.83,  1.48,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( 17.80,  1.80,  -6.60 );
		CREATE_OBJECT_PARTICLE( 16.77,  2.12,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( 15.74,  2.44,  -6.60 );
		CREATE_OBJECT_PARTICLE( 14.71,  2.76,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( 13.68,  3.08,  -6.60 );
		CREATE_OBJECT_PARTICLE( 12.65,  3.40,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( 11.62,  3.72,  -6.60 );
		CREATE_OBJECT_PARTICLE( 10.59,  4.04,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE(  9.56,  4.36,  -6.60 );
		CREATE_OBJECT_PARTICLE(  8.53,  4.68,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE(  7.50,  5.00,  -6.60 );
//		CREATE_OBJECT_PARTICLE(  7.98,  4.00,  -6.98 );
		CREATE_OBJECT_PARTICLE(  8.46,  3.00,  -7.36 + z_offset);
//		CREATE_OBJECT_PARTICLE(  8.94,  2.00,  -7.84 );
		CREATE_OBJECT_PARTICLE(  9.42,  1.00,  -8.32 + z_offset);
//		CREATE_OBJECT_PARTICLE(  9.90,  0.00,  -8.50 );
		CREATE_OBJECT_PARTICLE(  9.42, -1.00,  -8.32 + z_offset);
//		CREATE_OBJECT_PARTICLE(  8.94, -2.00,  -7.84 );
		CREATE_OBJECT_PARTICLE(  8.46, -3.00,  -7.36 + z_offset);
//		CREATE_OBJECT_PARTICLE(  7.98, -4.00,  -6.98 );

		CREATE_OBJECT_PARTICLE(  -5.00, -5.00,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE(  -6.03, -4.68,  -6.60 );
		CREATE_OBJECT_PARTICLE(  -7.06, -4.36,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE(  -8.09, -4.04,  -6.60 );
		CREATE_OBJECT_PARTICLE(  -9.12, -3.72,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( -10.15, -3.40,  -6.60 );
		CREATE_OBJECT_PARTICLE( -11.18, -3.08,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( -12.21, -2.76,  -6.60 );
		CREATE_OBJECT_PARTICLE( -13.24, -2.44,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( -14.27, -2.12,  -6.60 );
		CREATE_OBJECT_PARTICLE( -15.30, -1.80,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( -16.33, -1.48,  -6.60 );
		CREATE_OBJECT_PARTICLE( -17.36, -1.16,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( -18.39, -0.84,  -6.60 );
		CREATE_OBJECT_PARTICLE( -19.42, -0.52,  -6.60 + z_offset);
		CREATE_OBJECT_PARTICLE( -20.50, -0.20,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( -20.50,  0.20,  -6.60 );
//		CREATE_OBJECT_PARTICLE( -19.42,  0.52,  -6.60 );
		CREATE_OBJECT_PARTICLE( -18.39,  0.84,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( -17.36,  1.16,  -6.60 );
		CREATE_OBJECT_PARTICLE( -16.33,  1.48,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( -15.30,  1.80,  -6.60 );
		CREATE_OBJECT_PARTICLE( -14.27,  2.12,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( -13.24,  2.44,  -6.60 );
		CREATE_OBJECT_PARTICLE( -12.21,  2.76,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE( -11.18,  3.08,  -6.60 );
		CREATE_OBJECT_PARTICLE( -10.15,  3.40,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE(  -9.12,  3.72,  -6.60 );
		CREATE_OBJECT_PARTICLE(  -8.09,  4.04,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE(  -7.06,  4.36,  -6.60 );
//		CREATE_OBJECT_PARTICLE(  -6.03,  4.68,  -6.60 );
		CREATE_OBJECT_PARTICLE(  -5.00,  5.00,  -6.60 + z_offset);
//		CREATE_OBJECT_PARTICLE(  -5.48,  4.00,  -6.98 );
		CREATE_OBJECT_PARTICLE(  -5.96,  3.00,  -7.36 + z_offset);
//		CREATE_OBJECT_PARTICLE(  -6.44,  2.00,  -7.84 );
		CREATE_OBJECT_PARTICLE(  -6.92,  1.00,  -8.32 + z_offset);
//		CREATE_OBJECT_PARTICLE(  -7.40,  0.00,  -8.50 );
		CREATE_OBJECT_PARTICLE(  -6.92, -1.00,  -8.32 + z_offset);
//		CREATE_OBJECT_PARTICLE(  -6.44, -2.00,  -7.84 );
		CREATE_OBJECT_PARTICLE(  -5.96, -3.00,  -7.36 + z_offset);
//		CREATE_OBJECT_PARTICLE(  -5.48, -4.00,  -6.98 );

	}
}


// fixed offset to hard-coded coordinate specification ------------------------
//
#undef  OBJPARTOFS_X
#undef  OBJPARTOFS_Y
#undef  OBJPARTOFS_Z
#define OBJPARTOFS_X	0.0
#define OBJPARTOFS_Y	0.0
#define OBJPARTOFS_Z	0.0


// attach object particles to bluespire ---------------------------------------
//
PRIVATE
void AttachParticlesBluespire( GenObject *obj )
{
	ASSERT( obj != NULL );
	ASSERT( OBJECT_TYPE_SHIP( obj ) );

	pdef_s				 *pdef	  = NULL;
	genobject_pcluster_s *cluster = NULL;

	// poslights
	pdef = PRT_AcquireParticleDefinition( "poslt06", NULL );
	if ( pdef != NULL ) {

		// extinfo: flare
		CREATE_EXT_INFO( particle_rendermode[ PARTICLE_RENDERMODE_POSLIGHT ] );

		float ref_z = 0.23f * cur_particle_resoscale;
		CREATE_OBJECT_PARTICLE(  27.7, 12.5, 0.07 );
//		CREATE_OBJECT_PARTICLE( -27.2, 12.5, 0.02 );
	}

	pdef = PRT_AcquireParticleDefinition( "poslt07", NULL );
	if ( pdef != NULL ) {

		// extinfo: flare
		CREATE_EXT_INFO( particle_rendermode[ PARTICLE_RENDERMODE_POSLIGHT ] );

		float ref_z = 0.23f * cur_particle_resoscale;
//		CREATE_OBJECT_PARTICLE(  27.7, 12.5, 0.07 );
		CREATE_OBJECT_PARTICLE( -27.2, 12.5, 0.02 );
	}

	// thrust
	pdef = PRT_AcquireParticleDefinition( "thrust1", NULL );
	if ( pdef != NULL ) {

		// extinfo: light
		CREATE_EXT_INFO( particle_rendermode[ PARTICLE_RENDERMODE_THRUST ] );

#ifdef EXTRA_CLUSTER_FOR_THRUST_PARTICLES

		// one cluster for thrust particles only
		cluster = PRT_CreateGenObjectParticleCluster(
					obj, THRUST_CLUSTER_SIZE, NULL, TRUE );
		cluster->type |= CT_HINT_PARTICLES_IDENTICAL;
		cluster->type |= CT_CLUSTER_GLOBAL_EXTINFO;
		cluster->type |= CT_HINT_PARTICLES_HAVE_EXTINFO;
#endif
		float ref_z = 15.0f * cur_particle_resoscale;
		float z_offset = -0.45f; // offset to prevent clipping against ship
		CREATE_OBJECT_PARTICLE(  0,   -2,   -13.15 + z_offset );
		CREATE_OBJECT_PARTICLE(  2,   -2,   -13.15 + z_offset );
		CREATE_OBJECT_PARTICLE( -2,   -2,   -13.15 + z_offset );
		CREATE_OBJECT_PARTICLE(  3.3, -2,   -13.15 + z_offset );
		CREATE_OBJECT_PARTICLE( -3.2, -2,   -13.15 + z_offset );
		CREATE_OBJECT_PARTICLE(  0,   -0.4, -13.15 + z_offset );
		CREATE_OBJECT_PARTICLE(  2,   -0.4, -13.15 + z_offset );
		CREATE_OBJECT_PARTICLE( -2,   -0.4, -13.15 + z_offset );
		CREATE_OBJECT_PARTICLE(  3.3, -0.4, -13.15 + z_offset );
		CREATE_OBJECT_PARTICLE( -3.2, -0.4, -13.15 + z_offset );

		CREATE_OBJECT_PARTICLE( 16.1, 2.0, -14.3 + z_offset );
		CREATE_OBJECT_PARTICLE( -15.6, 2.0, -14.3 + z_offset );
		ref_z = 14.0f * cur_particle_resoscale;
		CREATE_OBJECT_PARTICLE( 15.8, 0.0, -13.8 + z_offset );
		CREATE_OBJECT_PARTICLE( 17.7, 0.6, -13.8 + z_offset );
		CREATE_OBJECT_PARTICLE( 17.7, 2.8, -13.8 + z_offset );
		CREATE_OBJECT_PARTICLE( 15.8, 3.4, -13.8 + z_offset );
		CREATE_OBJECT_PARTICLE( 14.2, 1.7, -13.8 + z_offset );

		CREATE_OBJECT_PARTICLE( -15.3, 0.0, -13.8 + z_offset );
		CREATE_OBJECT_PARTICLE( -17.2, 0.6, -13.8 + z_offset );
		CREATE_OBJECT_PARTICLE( -17.2, 2.8, -13.8 + z_offset );
		CREATE_OBJECT_PARTICLE( -15.3, 3.4, -13.8 + z_offset );
		CREATE_OBJECT_PARTICLE( -13.7, 1.7, -13.8 + z_offset );
	}
}


// fixed offset to hard-coded coordinate specification ------------------------
//
#undef  OBJPARTOFS_X
#undef  OBJPARTOFS_Y
#undef  OBJPARTOFS_Z
#define OBJPARTOFS_X	0.0
#define OBJPARTOFS_Y	0.0
#define OBJPARTOFS_Z	0.0


// attach object particles to cormoran ----------------------------------------
//
PRIVATE
void AttachParticlesCormoran( GenObject *obj )
{
	ASSERT( obj != NULL );
	ASSERT( OBJECT_TYPE_SHIP( obj ) );

	pdef_s				 *pdef	  = NULL;
	genobject_pcluster_s *cluster = NULL;

	// poslights
	pdef = PRT_AcquireParticleDefinition( "poslt01", NULL );
	if ( pdef != NULL ) {

		// extinfo: flare
		CREATE_EXT_INFO( particle_rendermode[ PARTICLE_RENDERMODE_POSLIGHT ] );

		float ref_z = 0.23f * cur_particle_resoscale;
		CREATE_OBJECT_PARTICLE( -0.14, -20.3, -1.5 );
//		CREATE_OBJECT_PARTICLE( -0.14,  21.5, -2.4 );
//		CREATE_OBJECT_PARTICLE( 41.20,  -2.2,  2.1 );
	}

	pdef = PRT_AcquireParticleDefinition( "poslt02", NULL );
	if ( pdef != NULL ) {

		// extinfo: flare
		CREATE_EXT_INFO( particle_rendermode[ PARTICLE_RENDERMODE_POSLIGHT ] );

		float ref_z = 0.23f * cur_particle_resoscale;
//		CREATE_OBJECT_PARTICLE( -0.14, -20.3, -1.5 );
		CREATE_OBJECT_PARTICLE( -0.14,  21.5, -2.4 );
//		CREATE_OBJECT_PARTICLE( 41.20,  -2.2,  2.1 );
	}

	pdef = PRT_AcquireParticleDefinition( "poslt03", NULL );
	if ( pdef != NULL ) {

		// extinfo: flare
		CREATE_EXT_INFO( particle_rendermode[ PARTICLE_RENDERMODE_POSLIGHT ] );

		float ref_z = 0.23f * cur_particle_resoscale;
//		CREATE_OBJECT_PARTICLE( -0.14, -20.3, -1.5 );
//		CREATE_OBJECT_PARTICLE( -0.14,  21.5, -2.4 );
		CREATE_OBJECT_PARTICLE( 41.20,  -2.2,  2.1 );
	}

	// thrust
	pdef = PRT_AcquireParticleDefinition( "thrust1", NULL );
	if ( pdef != NULL ) {

		// extinfo: light
		CREATE_EXT_INFO( particle_rendermode[ PARTICLE_RENDERMODE_THRUST ] );

#ifdef EXTRA_CLUSTER_FOR_THRUST_PARTICLES

		// one cluster for thrust particles only
		cluster = PRT_CreateGenObjectParticleCluster(
					obj, THRUST_CLUSTER_SIZE, NULL, TRUE );
		cluster->type |= CT_HINT_PARTICLES_IDENTICAL;
		cluster->type |= CT_CLUSTER_GLOBAL_EXTINFO;
		cluster->type |= CT_HINT_PARTICLES_HAVE_EXTINFO;
#endif
		float ref_z = 13.5f * cur_particle_resoscale;
		float z_offset = -0.425f; // offset to prevent clipping against ship
		CREATE_OBJECT_PARTICLE(  0, -3,   -5 + z_offset );
		CREATE_OBJECT_PARTICLE(  0, -3,   -5 + z_offset );
		CREATE_OBJECT_PARTICLE(  2, -3,   -5 + z_offset );
		CREATE_OBJECT_PARTICLE(  1, -1.6, -5 + z_offset );
		CREATE_OBJECT_PARTICLE( -1, -1.6, -5 + z_offset );
		CREATE_OBJECT_PARTICLE( -2, -3,   -5 + z_offset );
		CREATE_OBJECT_PARTICLE( -1, -4.4, -5 + z_offset );
		CREATE_OBJECT_PARTICLE(  1, -4.4, -5 + z_offset );
	}
}


// fixed offset to hard-coded coordinate specification ------------------------
//
#undef  OBJPARTOFS_X
#undef  OBJPARTOFS_Y
#undef  OBJPARTOFS_Z
#define OBJPARTOFS_X	0.0
#define OBJPARTOFS_Y	0.0
#define OBJPARTOFS_Z	0.0


// attach object particles to stingray ----------------------------------------
//
PRIVATE
void AttachParticlesStingray( GenObject *obj )
{
	ASSERT( obj != NULL );
	ASSERT( OBJECT_TYPE_SHIP( obj ) );

	pdef_s				 *pdef	  = NULL;
	genobject_pcluster_s *cluster = NULL;

	// thrust
	pdef = PRT_AcquireParticleDefinition( "thrust1", NULL );
	if ( pdef != NULL ) {

		// extinfo: light
		CREATE_EXT_INFO( particle_rendermode[ PARTICLE_RENDERMODE_THRUST ] );

#ifdef EXTRA_CLUSTER_FOR_THRUST_PARTICLES

		// one cluster for thrust particles only
		cluster = PRT_CreateGenObjectParticleCluster(
					obj, THRUST_CLUSTER_SIZE, NULL, TRUE );
		cluster->type |= CT_HINT_PARTICLES_IDENTICAL;
		cluster->type |= CT_CLUSTER_GLOBAL_EXTINFO;
		cluster->type |= CT_HINT_PARTICLES_HAVE_EXTINFO;
#endif
		float ref_z = 15.0f * cur_particle_resoscale;;
		float z_offset = -0.48f; // offset to prevent clipping against ship
		CREATE_OBJECT_PARTICLE( -9.5, -2.5, -9 + z_offset );
		CREATE_OBJECT_PARTICLE( -7.5, -2.6, -9 + z_offset );
		CREATE_OBJECT_PARTICLE( -5.5, -2.7, -9 + z_offset );
		CREATE_OBJECT_PARTICLE( -3.5, -2.8, -9 + z_offset );
		CREATE_OBJECT_PARTICLE( -1.5, -2.9, -9 + z_offset );
		CREATE_OBJECT_PARTICLE(  0.0, -2.9, -9 + z_offset );
		CREATE_OBJECT_PARTICLE(  1.5, -2.9, -9 + z_offset );
		CREATE_OBJECT_PARTICLE(  3.5, -2.8, -9 + z_offset );
		CREATE_OBJECT_PARTICLE(  5.5, -2.7, -9 + z_offset );
		CREATE_OBJECT_PARTICLE(  7.5, -2.6, -9 + z_offset );
		CREATE_OBJECT_PARTICLE(  9.5, -2.5, -9 + z_offset );
		CREATE_OBJECT_PARTICLE(  9.5, -1.5, -9 + z_offset );
		CREATE_OBJECT_PARTICLE(  9.5, -0.5, -9 + z_offset );
		CREATE_OBJECT_PARTICLE(  9.5,  0.5, -9 + z_offset );
		CREATE_OBJECT_PARTICLE(  9.5,  1.5, -9 + z_offset );
		CREATE_OBJECT_PARTICLE(  9.5,  2.5, -9 + z_offset );
		CREATE_OBJECT_PARTICLE(  7.5,  2.6, -9 + z_offset );
		CREATE_OBJECT_PARTICLE(  5.5,  2.7, -9 + z_offset );
		CREATE_OBJECT_PARTICLE(  3.5,  2.8, -9 + z_offset );
		CREATE_OBJECT_PARTICLE(  1.5,  2.9, -9 + z_offset );
		CREATE_OBJECT_PARTICLE(  0.0,  2.9, -9 + z_offset );
		CREATE_OBJECT_PARTICLE( -1.5,  2.9, -9 + z_offset );
		CREATE_OBJECT_PARTICLE( -3.5,  2.8, -9 + z_offset );
		CREATE_OBJECT_PARTICLE( -5.5,  2.7, -9 + z_offset );
		CREATE_OBJECT_PARTICLE( -7.5,  2.6, -9 + z_offset );
		CREATE_OBJECT_PARTICLE( -9.5,  2.5, -9 + z_offset );
		CREATE_OBJECT_PARTICLE( -9.5,  1.5, -9 + z_offset );
		CREATE_OBJECT_PARTICLE( -9.5,  0.5, -9 + z_offset );
		CREATE_OBJECT_PARTICLE( -9.5, -0.5, -9 + z_offset );
		CREATE_OBJECT_PARTICLE( -9.5, -1.5, -9 + z_offset );

		CREATE_OBJECT_PARTICLE( 15.0, -2, -25.2 + z_offset );
		CREATE_OBJECT_PARTICLE( 16.5, -1, -24.1 + z_offset );
		CREATE_OBJECT_PARTICLE( 18.0,  0, -23.0 + z_offset );
		CREATE_OBJECT_PARTICLE( 16.5,  1, -24.1 + z_offset );
		CREATE_OBJECT_PARTICLE( 15.0,  2, -25.2 + z_offset );
		CREATE_OBJECT_PARTICLE( 15.0,  1, -25.2 + z_offset );
		CREATE_OBJECT_PARTICLE( 15.0,  0, -25.2 + z_offset );
		CREATE_OBJECT_PARTICLE( 15.0, -1, -25.2 + z_offset );

		CREATE_OBJECT_PARTICLE( -15.0, -2, -25.2 + z_offset );
		CREATE_OBJECT_PARTICLE( -16.5, -1, -24.1 + z_offset );
		CREATE_OBJECT_PARTICLE( -18.0,  0, -23.0 + z_offset );
		CREATE_OBJECT_PARTICLE( -16.5,  1, -24.1 + z_offset );
		CREATE_OBJECT_PARTICLE( -15.0,  2, -25.2 + z_offset );
		CREATE_OBJECT_PARTICLE( -15.0,  1, -25.2 + z_offset );
		CREATE_OBJECT_PARTICLE( -15.0,  0, -25.2 + z_offset );
		CREATE_OBJECT_PARTICLE( -15.0, -1, -25.2 + z_offset );
	}
}


// fixed offset to hard-coded coordinate specification ------------------------
//
#undef  OBJPARTOFS_X
#undef  OBJPARTOFS_Y
#undef  OBJPARTOFS_Z
#define OBJPARTOFS_X	0.0
#define OBJPARTOFS_Y	0.0
#define OBJPARTOFS_Z	2.2


// attach object particles to claymore ----------------------------------------
//
PRIVATE
void AttachParticlesClaymore( GenObject *obj )
{
	ASSERT( obj != NULL );
	ASSERT( OBJECT_TYPE_SHIP( obj ) );

	pdef_s				 *pdef	  = NULL;
	genobject_pcluster_s *cluster = NULL;

	// thrust
	pdef = PRT_AcquireParticleDefinition( "thrust1", NULL );
	if ( pdef != NULL ) {

		// extinfo: light
		CREATE_EXT_INFO( particle_rendermode[ PARTICLE_RENDERMODE_THRUST ] );

#ifdef EXTRA_CLUSTER_FOR_THRUST_PARTICLES

		// one cluster for thrust particles only
		cluster = PRT_CreateGenObjectParticleCluster(
					obj, THRUST_CLUSTER_SIZE, NULL, TRUE );
		cluster->type |= CT_HINT_PARTICLES_IDENTICAL;
		cluster->type |= CT_CLUSTER_GLOBAL_EXTINFO;
		cluster->type |= CT_HINT_PARTICLES_HAVE_EXTINFO;
#endif
		float ref_z = 14.8f * cur_particle_resoscale;;
		float z_offset = -0.485f; // offset to prevent clipping against ship
		CREATE_OBJECT_PARTICLE( -11.0, -3.0, -29 + z_offset );
		CREATE_OBJECT_PARTICLE(  -9.8, -1.8, -29 + z_offset );
		CREATE_OBJECT_PARTICLE(  -9.2, -1.2, -29 + z_offset );
		CREATE_OBJECT_PARTICLE(  -8.0,  0.0, -29 + z_offset );
		CREATE_OBJECT_PARTICLE(  -9.2,  1.2, -29 + z_offset );
		CREATE_OBJECT_PARTICLE(  -9.8,  1.8, -29 + z_offset );
		CREATE_OBJECT_PARTICLE( -11.0,  3.0, -29 + z_offset );
		CREATE_OBJECT_PARTICLE( -12.2,  1.8, -29 + z_offset );
		CREATE_OBJECT_PARTICLE( -12.8,  1.2, -29 + z_offset );
		CREATE_OBJECT_PARTICLE( -14.0,  0.0, -29 + z_offset );
		CREATE_OBJECT_PARTICLE( -12.8, -1.2, -29 + z_offset );
		CREATE_OBJECT_PARTICLE( -12.2, -1.8, -29 + z_offset );

		CREATE_OBJECT_PARTICLE( 11.0, -3.0, -29 + z_offset );
		CREATE_OBJECT_PARTICLE(  9.8, -1.8, -29 + z_offset );
		CREATE_OBJECT_PARTICLE(  9.2, -1.2, -29 + z_offset );
		CREATE_OBJECT_PARTICLE(  8.0,  0.0, -29 + z_offset );
		CREATE_OBJECT_PARTICLE(  9.2,  1.2, -29 + z_offset );
		CREATE_OBJECT_PARTICLE(  9.8,  1.8, -29 + z_offset );
		CREATE_OBJECT_PARTICLE( 11.0,  3.0, -29 + z_offset );
		CREATE_OBJECT_PARTICLE( 12.2,  1.8, -29 + z_offset );
		CREATE_OBJECT_PARTICLE( 12.8,  1.2, -29 + z_offset );
		CREATE_OBJECT_PARTICLE( 14.0,  0.0, -29 + z_offset );
		CREATE_OBJECT_PARTICLE( 12.8, -1.2, -29 + z_offset );
		CREATE_OBJECT_PARTICLE( 12.2, -1.8, -29 + z_offset );
	}
}


// fixed offset to hard-coded coordinate specification ------------------------
//
#undef  OBJPARTOFS_X
#undef  OBJPARTOFS_Y
#undef  OBJPARTOFS_Z
#define OBJPARTOFS_X	0.0
#define OBJPARTOFS_Y	0.0
#define OBJPARTOFS_Z	2.2


// attach object particles to hurricane ---------------------------------------
//
PRIVATE
void AttachParticlesHurricane( GenObject *obj )
{
	ASSERT( obj != NULL );
	ASSERT( OBJECT_TYPE_SHIP( obj ) );

	pdef_s				 *pdef	  = NULL;
	genobject_pcluster_s *cluster = NULL;

	// poslights
	pdef = PRT_AcquireParticleDefinition( "poslt03", NULL );
	if ( pdef != NULL ) {

		// extinfo: flare
		CREATE_EXT_INFO( particle_rendermode[ PARTICLE_RENDERMODE_POSLIGHT ] );

		float ref_z = 0.23f * cur_particle_resoscale;
		CREATE_OBJECT_PARTICLE( -17.5, 0, 13.7 );
	}

	pdef = PRT_AcquireParticleDefinition( "poslt05", NULL );
	if ( pdef != NULL ) {

		// extinfo: flare
		CREATE_EXT_INFO( particle_rendermode[ PARTICLE_RENDERMODE_POSLIGHT ] );

		float ref_z = 0.23f * cur_particle_resoscale;
		CREATE_OBJECT_PARTICLE(  17.5, 0, 13.7 );
	}


	// thrust
	pdef = PRT_AcquireParticleDefinition( "thrust1", NULL );
	if ( pdef != NULL ) {

		// extinfo: light
		CREATE_EXT_INFO( particle_rendermode[ PARTICLE_RENDERMODE_THRUST ] );

#ifdef EXTRA_CLUSTER_FOR_THRUST_PARTICLES

		// one cluster for thrust particles only
		cluster = PRT_CreateGenObjectParticleCluster(
					obj, THRUST_CLUSTER_SIZE, NULL, TRUE );
		cluster->type |= CT_HINT_PARTICLES_IDENTICAL;
		cluster->type |= CT_CLUSTER_GLOBAL_EXTINFO;
		cluster->type |= CT_HINT_PARTICLES_HAVE_EXTINFO;
#endif
		float ref_z = 13.0f * cur_particle_resoscale;
		float z_offset = -0.3f; // offset to prevent clipping against ship
		CREATE_OBJECT_PARTICLE( -6.0, -4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE( -5.0, -4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE( -4.0, -4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE( -3.0, -4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE( -2.0, -4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE( -1.0, -4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE(  0.0, -4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE(  1.0, -4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE(  2.0, -4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE(  3.0, -4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE(  4.0, -4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE(  5.0, -4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE(  6.0, -4.5, -33 + z_offset );

		CREATE_OBJECT_PARTICLE( -6.0,  4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE( -5.0,  4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE( -4.0,  4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE( -3.0,  4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE( -2.0,  4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE( -1.0,  4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE(  0.0,  4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE(  1.0,  4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE(  2.0,  4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE(  3.0,  4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE(  4.0,  4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE(  5.0,  4.5, -33 + z_offset );
		CREATE_OBJECT_PARTICLE(  6.0,  4.5, -33 + z_offset );

	}
}


// fixed offset to hard-coded coordinate specification ------------------------
//
#undef  OBJPARTOFS_X
#undef  OBJPARTOFS_Y
#undef  OBJPARTOFS_Z
#define OBJPARTOFS_X	0.0
#define OBJPARTOFS_Y	0.0
#define OBJPARTOFS_Z	0.0


// attach object particles to dumb missile ------------------------------------
//
PRIVATE
void AttachParticlesMissileDumb( GenObject *obj )
{
	ASSERT( obj != NULL );
	ASSERT( OBJECT_TYPE_MISSILE( obj ) );

	pdef_s				 *pdef	  = NULL;
	genobject_pcluster_s *cluster = NULL;

	// thrust
	pdef = PDEF_misglow1();
	if ( pdef != NULL ) {

		// extinfo: light
		CREATE_EXT_INFO( PARTICLE_RENDERMODE_MISSILE );

		//TODO:
		float ref_z = 120.0f * cur_particle_resoscale;
		CREATE_OBJECT_PARTICLE( 0, 0, -8.5 );
	}
}


// fixed offset to hard-coded coordinate specification ------------------------
//
#undef  OBJPARTOFS_X
#undef  OBJPARTOFS_Y
#undef  OBJPARTOFS_Z
#define OBJPARTOFS_X	0.0
#define OBJPARTOFS_Y	0.0
#define OBJPARTOFS_Z	0.0


// attach object particles to guided missile ----------------------------------
//
PRIVATE
void AttachParticlesMissileGuide( GenObject *obj )
{
	ASSERT( obj != NULL );
	ASSERT( OBJECT_TYPE_MISSILE( obj ) );

	pdef_s				 *pdef	  = NULL;
	genobject_pcluster_s *cluster = NULL;

	// thrust
	pdef = PDEF_misglow1();
	if ( pdef != NULL ) {

		// extinfo: light
		CREATE_EXT_INFO( PARTICLE_RENDERMODE_MISSILE );

		//TODO:
		float ref_z = 110.0f * cur_particle_resoscale;
		CREATE_OBJECT_PARTICLE( 0, 0, -8.0 );
	}
}


// fixed offset to hard-coded coordinate specification ------------------------
//
#undef  OBJPARTOFS_X
#undef  OBJPARTOFS_Y
#undef  OBJPARTOFS_Z
#define OBJPARTOFS_X	0.0
#define OBJPARTOFS_Y	0.0
#define OBJPARTOFS_Z	0.0


// particle to object attachment functions ------------------------------------
//
PRIVATE
attach_particles_fpt functab_attach_particles[ MAX_DISTINCT_OBJCLASSES ];


// attach static particles to specified object --------------------------------
//
void OBJ_AttachClassParticles( GenObject *obj )
{
	ASSERT( obj != NULL  );
	ASSERT( obj->ObjectClass < (dword)NumObjClasses );

	//NOTE:
	// this function is called externally on class instantiation (most
	// importantly by OBJ_CTRL::CreateObject() ) to attach particles to the
	// class instance.

	// call attachment function if registered
	if ( functab_attach_particles[ obj->ObjectClass ] != NULL ) {
		(*functab_attach_particles[ obj->ObjectClass ])( obj );
	}
}


// a single registered particle -----------------------------------------------
//
struct attach_part_reg_s {

	pdef_s*		pdef;		// already resolved pdef
	float		refz;		// size
	dword		rendmode;	// rendering mode (already mapped)
	int			isthrust;	// thrust particles are handled differently
	Vector3		position;	// attachment position in object space
};


// table of all particles to attach to a specific object class ----------------
//
struct attach_part_tab_s {

	short				numregs;
	short				maxregs;
	attach_part_reg_s*	regs;
};


// all registered particles for all object classes ----------------------------
//
PRIVATE
attach_part_tab_s attach_part_tab[ MAX_DISTINCT_OBJCLASSES ];


// attach previously registered object particles ------------------------------
//
PRIVATE
void AttachParticlesRegistered( GenObject *obj )
{
	ASSERT( obj != NULL );
	ASSERT( obj->ObjectClass < (dword)NumObjClasses );

	dword objclass = obj->ObjectClass;

	attach_part_reg_s *apreg = attach_part_tab[ objclass ].regs;
	if ( apreg == NULL ) {
		return;
	}

	genobject_pcluster_s *cluster = NULL;

	// first sweep over registered particles
	int acnt = 0;
	for ( acnt = attach_part_tab[ objclass ].numregs; acnt > 0; acnt--, apreg++ ) {

		// thrust particles will be attached in second sweep
		if ( apreg->isthrust ) {
			continue;
		}

		// create extinfo
		pextinfo_s extinfo;
		PRT_InitParticleExtInfo( &extinfo, apreg->pdef, NULL, NULL );

		// attach particle
		int bitmap	  = iter_texrgba | iter_additiveblend | apreg->rendmode;
		float ref_z = apreg->refz * cur_particle_resoscale;

		particle_s particle;
		PRT_InitParticle( particle, bitmap, 0, PRT_NO_SIZEBOUND,
			ref_z, &apreg->position, NULL, INFINITE_LIFETIME, LocalPlayerId, &extinfo );
		cluster = PRT_CreateGenObjectParticle( particle, obj, cluster );
	}

	// do not use previous cluster for thrust particles
	cluster = NULL;

	// second sweep over registered particles
	apreg = attach_part_tab[ objclass ].regs;
	for ( acnt = attach_part_tab[ objclass ].numregs; acnt > 0; acnt--, apreg++ ) {

		// only thrust particles in this sweep
		if ( !apreg->isthrust ) {
			continue;
		}

		// create separate cluster for thrust particles
		if ( cluster == NULL ) {

#ifdef EXTRA_CLUSTER_FOR_THRUST_PARTICLES

			// one cluster for thrust particles only
			cluster = PRT_CreateGenObjectParticleCluster(
				obj, THRUST_CLUSTER_SIZE, NULL, TRUE );
			cluster->type |= CT_HINT_PARTICLES_IDENTICAL;
			cluster->type |= CT_CLUSTER_GLOBAL_EXTINFO;
			cluster->type |= CT_HINT_PARTICLES_HAVE_EXTINFO;
#endif
		}

		// create extinfo with appropriate render mode
		pextinfo_s extinfo;
		PRT_InitParticleExtInfo( &extinfo, apreg->pdef, NULL, NULL );

		// attach particle
		int bitmap	  = iter_texrgba | iter_specularadd | apreg->rendmode;
		float ref_z = apreg->refz * cur_particle_resoscale;

		particle_s particle;
		PRT_InitParticle( particle, bitmap, 0, PRT_NO_SIZEBOUND,
			ref_z, &apreg->position, NULL, INFINITE_LIFETIME, LocalPlayerId, &extinfo );
		cluster = PRT_CreateGenObjectParticle( particle, obj, cluster );
	}
}


// maximum number of particles that may be registered for a single class ------
//
#define MAX_ATTACH_PART		( THRUST_CLUSTER_SIZE * 2 )


// reset registered particles for specified class -----------------------------
//
void OBJ_ResetRegisteredClassParticles( dword objclass )
{
	ASSERT( objclass < (dword)NumObjClasses );

	functab_attach_particles[ objclass ] = NULL;

	if ( attach_part_tab[ objclass ].regs != NULL ) {
		FREEMEM( attach_part_tab[ objclass ].regs );
		attach_part_tab[ objclass ].regs = NULL;
	}
}


// register single particle to be attached on object class instantiation ------
//
int OBJ_RegisterClassParticle( dword objclass, const char *pdefname, float refz, int rendmode, Vector3 *pos )
{
	ASSERT( objclass < (dword)NumObjClasses );
	ASSERT( pdefname != NULL );
	ASSERT( pos != NULL );

	// register general callback for this class if not already done
	if ( functab_attach_particles[ objclass ] == NULL  ) {
		functab_attach_particles[ objclass ] = AttachParticlesRegistered;
	}

	// create new table of particles to attach if not yet there
	if ( attach_part_tab[ objclass ].regs == NULL ) {
		attach_part_tab[ objclass ].regs = (attach_part_reg_s *)
			ALLOCMEM( MAX_ATTACH_PART * sizeof( attach_part_reg_s ) );
		if ( attach_part_tab[ objclass ].regs == NULL )
			OUTOFMEM( "no mem for class particles." );
		attach_part_tab[ objclass ].numregs	= 0;
		attach_part_tab[ objclass ].maxregs	= MAX_ATTACH_PART;
	}

	// guard against overflow
	if ( attach_part_tab[ objclass ].numregs == attach_part_tab[ objclass ].maxregs ) {
		return FALSE;
	}

	// register this particle
	attach_part_reg_s *apreg = &attach_part_tab[ objclass ].regs[ attach_part_tab[ objclass ].numregs ];

	apreg->pdef		= PRT_AcquireParticleDefinition( pdefname, NULL );
	apreg->refz		= refz;
	apreg->rendmode	= particle_rendermode[ rendmode ];
	apreg->isthrust	= ( stricmp( pdefname, "thrust1" ) == 0 );
	apreg->position	= *pos;

	// pdef might not be registered yet
	if ( apreg->pdef == NULL ) {
		return FALSE;
	}

	attach_part_tab[ objclass ].numregs++;

	return TRUE;
}


// register attachment function for a specific named class --------------------
//
PRIVATE
int OBJ_RegisterAttachParticles( const char *classname, attach_particles_fpt atfunc )
{
	ASSERT( classname != NULL );

	// try to resolve name to id
	dword classid = OBJ_FetchObjectClassId( classname );
	if ( classid == CLASS_ID_INVALID ) {
		return FALSE;
	}

	// double registration not allowed
	if ( functab_attach_particles[ classid ] != NULL ) {
		return FALSE;
	}

	// set callback
	functab_attach_particles[ classid ] = atfunc;

	return TRUE;
}


// register attachment functions for all accessible classes -------------------
//
PRIVATE
void RegisterAttachParticlesCallback( int numparams, int *params )
{
	//NOTE:
	// must be idempotent.

	// ships
	OBJ_RegisterAttachParticles(
		OBJCLASSNAME_SHIP_FIREBIRD,  AttachParticlesFirebird );

	OBJ_RegisterAttachParticles(
		OBJCLASSNAME_SHIP_BLUESPIRE, AttachParticlesBluespire );

	OBJ_RegisterAttachParticles(
		OBJCLASSNAME_SHIP_CORMORAN,  AttachParticlesCormoran );

	OBJ_RegisterAttachParticles(
		OBJCLASSNAME_SHIP_STINGRAY,  AttachParticlesStingray );

	OBJ_RegisterAttachParticles(
		OBJCLASSNAME_SHIP_CLAYMORE,	 AttachParticlesClaymore );

	OBJ_RegisterAttachParticles(
		OBJCLASSNAME_SHIP_HURRICANE, AttachParticlesHurricane );

	// missiles
	OBJ_RegisterAttachParticles(
		OBJCLASSNAME_MISSILE_DUMB,   AttachParticlesMissileDumb );

	OBJ_RegisterAttachParticles(
		OBJCLASSNAME_MISSILE_GUIDE,  AttachParticlesMissileGuide );
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( OBJ_PART )
{
	// redundant
	for ( int cid = 0; cid < MAX_DISTINCT_OBJCLASSES; cid++ ) {
		functab_attach_particles[ cid ] = NULL;
	}

	// register on creg
	OBJ_RegisterClassRegistration( RegisterAttachParticlesCallback );
}



