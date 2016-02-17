/*
 * PARSEC - Collision Detection
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:23 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
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
#include "od_props.h"

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "net_defs.h"
#include "sys_defs.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "obj_coll.h"

// proprietary module headers
#include "con_aux.h"
#include "net_prediction.h"
#include "g_supp.h"
#include "g_extra.h"
#include "h_supp.h"
#include "obj_ctrl.h"
#include "obj_expl.h"
#include "obj_game.h"
#include "obj_xtra.h"
#include "g_sfx.h"


// flags ----------------------------------------------------------------------
//
//#define NO_EXTRA_MAINTENANCE
//#define ASSERT_IF_NUMEXTRAS_INVALID
#define CHECK_BOX_BEFORE_BSP_COLLISION
#define NEW_PREDICTION_SYSTEM

// time we keep an extra in zombie state, when we predict a removal -----------
//
#define EXTRA_PREDICT_ZOMBIE_TIME	DEFAULT_REFFRAME_FREQUENCY * 2


// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// kill message strings -------------------------------------------------------
//
static char laser_downed_ship_str[] 	= "laser shot down ship";
static char missile_downed_ship_str[]	= "missile shot down ship";
static char killed_by_str[] 			= "killed by ";


// strings for extra collections ----------------------------------------------
//
/*static char got_dumb_str[]		        = "dumb missiles collected";
static char max_dumb_str[]		        = "dumb missiles maxed out";
static char got_homing_str[]	        = "guided missiles found";
static char max_homing_str[]	        = "guided missiles maxed out";
static char got_mines_str[] 	        = "found proximity mines";
static char max_mines_str[] 	        = "mines maxed out";
static char got_swarm_str[]		        = "swarm missiles found";
static char max_swarm_str[]	    	    = "swarm missiles maxed out";
static char got_helix_str[]				= "got helix cannon";
static char max_helix_str[]				= "already got helix cannon";
static char got_lightning_str[]	        = "got lightning device";
static char max_lightning_str[]	        = "already got lightning device";
static char got_photon_str[]			= "got photon cannon";
static char max_photon_str[]			= "already got photon cannon";
static char unknown_dev_str[]	        = "got unknown device";
static char unknown_extra_str[]	        = "got unknown extra";
static char no_damage_str[]		        = "no damage to repair";
static char damage_repaired[]	        = "damage repaired";*/
static char mine_killed_str[]	        = "killed by mine";
static char mine_hit_str[]		        = "mine hit";
/*static char got_afterburner_str[]       = "got afterburner";
static char max_afterburner_str[]       = "already got afterburner";
static char got_cloak_str[]		        = "got cloaking device";
static char max_cloak_str[]		        = "cloak reactivated";
static char got_megashield_str[]        = "got invulnerability shield";
static char max_megashield_str[]        = "invulnerability reinforced";
static char got_decoy_str[] 	        = "got holo decoy device";
static char max_decoy_str[]     	    = "already got holo decoy device";
static char got_laser_upgrade_1_str[]   = "got laser upgrade 1";
static char max_laser_upgrade_1_str[]   = "already got laser upgrade 1";
static char got_laser_upgrade_2_str[]   = "got laser upgrade 2";
static char max_laser_upgrade_2_str[]   = "already got laser upgrade 2";
static char need_laser_upgrade_1_str[]	= "need laser upgrade 1 first";
static char got_emp_str[]				= "got emp device";
static char max_emp_str[]				= "already got emp device";
static char got_emp_upgrade_1_str[]		= "got emp upgrade 1";
static char max_emp_upgrade_1_str[]		= "already got emp upgrade 1";
static char got_emp_upgrade_2_str[]		= "got emp upgrade 2";
static char max_emp_upgrade_2_str[]		= "already got emp upgrade 2";
static char need_emp_upgrade_1_str[]	= "need emp upgrade 1 first";*/


// collision detection helper variables ---------------------------------------
//
static ShipObject*	cur_ship;
static Vertex3		obj_pos;
static Vertex3		test_pos;
static Vertex3		prev_pos;
static geomv_t		bd_sphere;
static geomv_t		bd_sphere2;
static int			test_shield;

#define SET_SHIELD_TESTING()					{ test_shield = TRUE; }
//#define SET_SHIELD_TESTING()					{ test_shield = FALSE; }

#define CONSERVATIVE_POINTSAMPLE_EXPANSION		16


// globals --------------------------------------------------------------------
//
NET_PredictionManager		ThePredictionManager( &ServerStream );


// collision detection helper function ----------------------------------------
//
INLINE
int CheckShipProjectileDisjoint()
{
	// extend sphere for point sampling
	geomv_t hugesphere = bd_sphere * CONSERVATIVE_POINTSAMPLE_EXPANSION;

	// vector to center
	Vector3 vectest;
	vectest.X = obj_pos.X - test_pos.X;
	vectest.Y = obj_pos.Y - test_pos.Y;
	vectest.Z = obj_pos.Z - test_pos.Z;

	// check enlarged vicinity for early-out
	if ( vectest.X >= hugesphere )
		return TRUE;
	if ( vectest.Y >= hugesphere )
		return TRUE;
	if ( vectest.Z >= hugesphere )
		return TRUE;

	hugesphere = -hugesphere;
	if ( vectest.X <= hugesphere )
		return TRUE;
	if ( vectest.Y <= hugesphere )
		return TRUE;
	if ( vectest.Z <= hugesphere )
		return TRUE;

	// test shield (bounding sphere)
	if ( test_shield ) {

		// squared distance to current position
		geomv_t vectestl2 = DOT_PRODUCT( &vectest, &vectest );

		// trivial intersect
		if ( vectestl2 < bd_sphere2 )
			return FALSE;

		// squared distance to previous position
		Vector3 vecprev;
		vecprev.X = obj_pos.X - prev_pos.X;
		vecprev.Y = obj_pos.Y - prev_pos.Y;
		vecprev.Z = obj_pos.Z - prev_pos.Z;

		geomv_t vecprevl2 = DOT_PRODUCT( &vecprev, &vecprev );

		// trivial intersect
		if ( vecprevl2 < bd_sphere2 )
			return FALSE;

		// calc nearest point on line to center
		Vector3 vecline;
		vecline.X = test_pos.X - prev_pos.X;
		vecline.Y = test_pos.Y - prev_pos.Y;
		vecline.Z = test_pos.Z - prev_pos.Z;

		geomv_t scaledt = DOT_PRODUCT( &vecline, &vecprev );

		// ray pointing away from sphere?
		if ( GEOMV_NEGATIVE( scaledt ) )
			return TRUE;

		// ray not yet at the sphere?
		geomv_t veclinel2 =	DOT_PRODUCT( &vecline, &vecline );
		if ( scaledt >= veclinel2 )
			return TRUE;
		ASSERT( veclinel2 > GEOMV_VANISHING );

		// calc squared distance of nearest point
		geomv_t dot2  = GEOMV_MUL( scaledt, scaledt );
		geomv_t tlen2 = GEOMV_DIV( dot2, veclinel2 );

		//NOTE:
		// the actual point of impact could now be
		// easily calculated without a sqrt().

		// collision if nearest point inside
		return ( ( vecprevl2 - tlen2 ) >= bd_sphere2 );

	} else {

		// shield is down: test actual object

#ifdef CHECK_BOX_BEFORE_BSP_COLLISION

		// clip lineseg into box

		geomv_t isect;
		geomv_t seglen;
		geomv_t vtx0t = GEOMV_0;
		geomv_t vtx1t = GEOMV_1;

		int collside = -1;

		static geomv_t vanish = FLOAT_TO_GEOMV( 0.00001 );

		// cull/clip against +Z
		geomv_t curax = obj_pos.Z + bd_sphere;
		geomv_t dist0 = prev_pos.Z - curax;
		geomv_t dist1 = test_pos.Z - curax;
		dword outcode = ( ( DW32( dist0 ) & 0x80000000 ) >> 1 ) |
						  ( DW32( dist1 ) & 0x80000000 );
		// both outside?
		if ( outcode == 0x00000000 )
			return TRUE;
		// at least one outside?
		if ( outcode != 0xc0000000 ) {
			if ( outcode == 0x40000000 ) {
				// v0 inside, v1 outside
				ABS_GEOMV( dist0 );
				seglen = dist0 + dist1;
				if ( seglen <= vanish )
					return TRUE;
				vtx1t = GEOMV_DIV( dist0, seglen );
			} else {
				// v0 outside, v1 inside
				ABS_GEOMV( dist1 );
				seglen = dist0 + dist1;
				if ( seglen <= vanish )
					return TRUE;
				vtx0t = GEOMV_DIV( dist0, seglen );
				collside = 0;
			}
		}

#define AXIAL_CLIP_T(s)	{ \
\
	outcode = ( ( DW32( dist0 ) & 0x80000000 ) >> 1 ) | \
				( DW32( dist1 ) & 0x80000000 ); \
	if ( outcode == 0x00000000 ) \
		return TRUE; \
	if ( outcode != 0xc0000000 ) { \
		if ( outcode == 0x40000000 ) { \
			ABS_GEOMV( dist0 ); \
			seglen = dist0 + dist1; \
			if ( seglen <= vanish ) \
				return TRUE; \
			isect = GEOMV_DIV( dist0, seglen ); \
			if ( isect <= vtx0t ) \
				return TRUE; \
			if ( isect < vtx1t ) \
				vtx1t = isect; \
		} else { \
			ABS_GEOMV( dist1 ); \
			seglen = dist0 + dist1; \
			if ( seglen <= vanish ) \
				return TRUE; \
			isect = GEOMV_DIV( dist0, seglen ); \
			if ( isect >= vtx1t ) \
				return TRUE; \
			if ( isect > vtx0t ) { \
				vtx0t = isect; \
				collside = (s); \
			} \
		} \
	} \
}
		// cull/clip against -Z
		curax = obj_pos.Z - bd_sphere;
		dist0 = curax - prev_pos.Z;
		dist1 = curax - test_pos.Z;
		AXIAL_CLIP_T( 1 );

		// cull/clip against +X
		curax = obj_pos.X + bd_sphere;
		dist0 = prev_pos.X - curax;
		dist1 = test_pos.X - curax;
		AXIAL_CLIP_T( 2 );

		// cull/clip against -X
		curax = obj_pos.X - bd_sphere;
		dist0 = curax - prev_pos.X;
		dist1 = curax - test_pos.X;
		AXIAL_CLIP_T( 3 );

		// cull/clip against +Y
		curax = obj_pos.Y + bd_sphere;
		dist0 = prev_pos.Y - curax;
		dist1 = test_pos.Y - curax;
		AXIAL_CLIP_T( 4 );

		// cull/clip against -Y
		curax = obj_pos.Y - bd_sphere;
		dist0 = curax - prev_pos.Y;
		dist1 = curax - test_pos.Y;
		AXIAL_CLIP_T( 5 );

		//NOTE:
		// collside now contains the code of the collider side
		// (-1 means the lineseg starts in the bounding box)

		//TODO:
		// use already clipped segment in BSP test?

#endif // CHECK_BOX_BEFORE_BSP_COLLISION

		// test actual object (polygon accurate)

		// transform line vertices into object-space
		CalcOrthoInverse( cur_ship->ObjPosition, DestXmatrx );
		Vertex3 v0;
		MtxVctMUL( DestXmatrx, &prev_pos, &v0 );
		Vertex3 v1;
		MtxVctMUL( DestXmatrx, &test_pos, &v1 );

		// assume collision if no bsp tree
		CullBSPNode *node = cur_ship->AuxBSPTree;
		if ( node == NULL )
			return FALSE;

		dword	nodeid = 0;
		geomv_t	impact_t;
		int collided = BSP_FindColliderLine( node, &v0, &v1, &nodeid, &impact_t );

		// no collision?
		if ( !collided )
			return TRUE;

		// react only if not started in solid leaf
		if ( nodeid > 0 ) {

			// fetch collider
			node = &cur_ship->AuxBSPTree[ nodeid ];

			// calc collision position (object-space)
			v1.X -= v0.X;
			v1.Y -= v0.Y;
			v1.Z -= v0.Z;
			v1.X  = v0.X + GEOMV_MUL( v1.X, impact_t );
			v1.Y  = v0.Y + GEOMV_MUL( v1.Y, impact_t );
			v1.Z  = v0.Z + GEOMV_MUL( v1.Z, impact_t );

			// create hull impact particles
			SFX_HullImpact( cur_ship, &v1, &node->plane );
		}

		// disable shield
		test_shield = FALSE;
	}

	return FALSE;
}


// local ship collided with laser ---------------------------------------------
//
PRIVATE
void CollisionLaserMyShip( LaserObject *curlaser )
{
	ASSERT( curlaser != NULL );

	int hitpoints = curlaser->HitPoints;
	if ( ( hitpoints -= MyShip->MegaShieldAbsorption ) < 0 ) {
		hitpoints = 0;
	}

	SetScreenBlue = FLASHTIME_LASER_STANDARD;

	if ( MyShip->CurDamage <= MyShip->MaxDamage ) {
        MyShip->CurDamage += hitpoints;
		// object removal only done in PEER mode
		if ( NET_ConnectedPEER() ) {
			if ( MyShip->CurDamage > MyShip->MaxDamage ) {
				KillDurationWeapons( MyShip );
				OBJ_CreateShipExtras( MyShip );
				NET_SetPlayerKillStat( curlaser->Owner, 1 );
				strcpy( paste_str, killed_by_str );
				strcat( paste_str, NET_FetchPlayerName( curlaser->Owner ) );
				ShowMessage( paste_str );
			}
		}
	}

	if ( MyShip->CurDamage <= MyShip->MaxDamage ) {
		OBJ_EventShipImpact( MyShip, test_shield );
	}

	// RE only in PEER mode
	if ( NET_ConnectedPEER() ) {
		NET_RmEvKillObject( curlaser->HostObjNumber, LASER_LIST );
	} 
	KillSpecificObject( curlaser->ObjectNumber, LaserObjects );
}


// remote ship collided with laser --------------------------------------------
//
PRIVATE
void CollisionLaserRemoteShip( LaserObject *curlaser )
{
	ASSERT( curlaser != NULL );

	int hitpoints = curlaser->HitPoints;

	// RE only in PEER mode
	if ( NET_ConnectedPEER() ) {
		NET_RmEvKillObject( curlaser->HostObjNumber, LASER_LIST );
	} 
	KillSpecificObject( curlaser->ObjectNumber, LaserObjects );

	if ( cur_ship->ExplosionCount == 0 ) {

		OBJ_EventShipImpact( cur_ship, test_shield );
		cur_ship->CurDamage += hitpoints;
		if ( !NetConnected ) {

			//NOTE:
			// this path is only used in off-line mode 

			if ( cur_ship->CurDamage > cur_ship->MaxDamage ) {
				LetShipExplode( cur_ship );
				ShowMessage( laser_downed_ship_str );
				AUD_ShipDestroyed( cur_ship );
				AUD_JimCommenting( JIM_COMMENT_EATTHIS );
			}
		}
	}
}


// check whether any ship is hit by laser beam --------------------------------
//
PRIVATE
void CheckShipLaserCollision()
{
	// check whether local player is hit by laser
	if ( NetConnected ) {

		cur_ship   = MyShip;
		obj_pos.X  = MyShip->ObjPosition[ 0 ][ 3 ];
		obj_pos.Y  = MyShip->ObjPosition[ 1 ][ 3 ];
		obj_pos.Z  = MyShip->ObjPosition[ 2 ][ 3 ];
		bd_sphere  = MyShip->BoundingSphere;
		bd_sphere2 = MyShip->BoundingSphere2;

		// walk all laser projectiles
		LaserObject *walklasers = FetchFirstLaser();
		while ( walklasers != NULL ) {

			LaserObject *curlaser = walklasers;
			walklasers = (LaserObject *) walklasers->NextObj;

			// can't be hit by own laser projectile
			if ( curlaser->Owner == OWNER_LOCAL_PLAYER ) {
				continue;
			}

			test_pos.X = curlaser->ObjPosition[ 0 ][ 3 ];
			test_pos.Y = curlaser->ObjPosition[ 1 ][ 3 ];
			test_pos.Z = curlaser->ObjPosition[ 2 ][ 3 ];

			prev_pos.X = curlaser->PrevPosition.X;
			prev_pos.Y = curlaser->PrevPosition.Y;
			prev_pos.Z = curlaser->PrevPosition.Z;

			// shield or actual object?
			SET_SHIELD_TESTING();

			// actual collision detection
			if ( CheckShipProjectileDisjoint() ) {
				continue;
			}

			// collision response
			CollisionLaserMyShip( curlaser );
		}
	}

	// check whether any other shipobject is hit by laser
	cur_ship = FetchFirstShip();
	while ( cur_ship != NULL ) {

		ShipObject *nextship = (ShipObject *) cur_ship->NextObj;

		obj_pos.X  = cur_ship->ObjPosition[ 0 ][ 3 ];
		obj_pos.Y  = cur_ship->ObjPosition[ 1 ][ 3 ];
		obj_pos.Z  = cur_ship->ObjPosition[ 2 ][ 3 ];
		bd_sphere  = cur_ship->BoundingSphere;
		bd_sphere2 = cur_ship->BoundingSphere2;

		// walk all laser projectiles
		LaserObject *walklasers = FetchFirstLaser();
		while ( walklasers != NULL ) {

			// get current laser and step in list ( curlaser is removed upon collision )
			LaserObject *curlaser = walklasers;
			walklasers = (LaserObject *) walklasers->NextObj;

			// owner can't be hit by own laser projectile
			if ( cur_ship->HostObjNumber == ShipHostObjId( curlaser->Owner ) ) {
				continue;
			}

			test_pos.X = curlaser->ObjPosition[ 0 ][ 3 ];
			test_pos.Y = curlaser->ObjPosition[ 1 ][ 3 ];
			test_pos.Z = curlaser->ObjPosition[ 2 ][ 3 ];

			prev_pos.X = curlaser->PrevPosition.X;
			prev_pos.Y = curlaser->PrevPosition.Y;
			prev_pos.Z = curlaser->PrevPosition.Z;

			// shield or actual object?
			SET_SHIELD_TESTING();

			// actual collision detection
			if ( CheckShipProjectileDisjoint() ) {
				continue;
			}

			// collision response
			CollisionLaserRemoteShip( curlaser );
		}

		cur_ship = nextship;
	}
}


// local ship collided with missile -------------------------------------------
//
PRIVATE
void CollisionMissileMyShip( MissileObject *curmissile )
{
	ASSERT( curmissile != NULL );

	int hitpoints = curmissile->HitPoints;
	if ( ( hitpoints -= MyShip->MegaShieldAbsorption ) < 0 ) {
		hitpoints = 0;
	}

	SetScreenBlue = FLASHTIME_MISSILE;

	if ( MyShip->CurDamage <= MyShip->MaxDamage ) {
		MyShip->CurDamage += hitpoints;

		if ( MyShip->CurDamage > MyShip->MaxDamage ) {
            // object removal only done in PEER mode
		    if ( NET_ConnectedPEER() ) {
			    KillDurationWeapons( MyShip );
			    OBJ_CreateShipExtras( MyShip );
			    NET_SetPlayerKillStat( curmissile->Owner, 1 );
			    strcpy( paste_str, killed_by_str );
			    strcat( paste_str, NET_FetchPlayerName( curmissile->Owner ) );
			    ShowMessage( paste_str );
			}
		} else {
			OBJ_EventShipImpact( MyShip, test_shield );
		}
	}
	// RE only in PEER mode
	if ( NET_ConnectedPEER() ) {
	   NET_RmEvKillObject( curmissile->HostObjNumber, MISSL_LIST );
	}
	KillSpecificObject( curmissile->ObjectNumber, MisslObjects );
}


// remote ship collided with missile ------------------------------------------
//
PRIVATE
void CollisionMissileRemoteShip( MissileObject *curmissile )
{
	ASSERT( curmissile != NULL );

	int hitpoints = curmissile->HitPoints;

	// RE only in PEER mode
	if ( NET_ConnectedPEER() ) {
	   NET_RmEvKillObject( curmissile->HostObjNumber, MISSL_LIST );
	}
	KillSpecificObject( curmissile->ObjectNumber, MisslObjects );

	if ( cur_ship->ExplosionCount == 0 ) {

		OBJ_EventShipImpact( cur_ship, test_shield );
		cur_ship->CurDamage += hitpoints;
		if ( !NetConnected ) {
			if ( cur_ship->CurDamage > cur_ship->MaxDamage ) {
				LetShipExplode( cur_ship );
				ShowMessage( missile_downed_ship_str );
				AUD_ShipDestroyed( cur_ship );
				AUD_JimCommenting( JIM_COMMENT_EATTHIS );
			}
		}
	}
}


// check whether any ship is hit by missile -----------------------------------
//
PRIVATE
void CheckShipMissileCollision()
{
	// check whether local player is hit by missile
	if ( NetConnected ) {

		cur_ship   = MyShip;
		obj_pos.X  = MyShip->ObjPosition[ 0 ][ 3 ];
		obj_pos.Y  = MyShip->ObjPosition[ 1 ][ 3 ];
		obj_pos.Z  = MyShip->ObjPosition[ 2 ][ 3 ];
		bd_sphere  = MyShip->BoundingSphere;
		bd_sphere2 = MyShip->BoundingSphere2;

		// walk all missiles
		MissileObject *walkmissiles = FetchFirstMissile();
		while ( walkmissiles != NULL ) {

			MissileObject *curmissile = walkmissiles;
			walkmissiles = (MissileObject *) walkmissiles->NextObj;

			// can't be hit by own missile
			if ( curmissile->Owner == OWNER_LOCAL_PLAYER ) {
				continue;
			}

			test_pos.X = curmissile->ObjPosition[ 0 ][ 3 ];
			test_pos.Y = curmissile->ObjPosition[ 1 ][ 3 ];
			test_pos.Z = curmissile->ObjPosition[ 2 ][ 3 ];

			prev_pos.X = curmissile->PrevPosition.X;
			prev_pos.Y = curmissile->PrevPosition.Y;
			prev_pos.Z = curmissile->PrevPosition.Z;

			// shield or actual object?
			SET_SHIELD_TESTING();

			// actual collision detection
			if ( CheckShipProjectileDisjoint() ) {
				continue;
			}

			// collision response
			CollisionMissileMyShip( curmissile );
		}
	}

	// check whether any other ship object is hit by missile
	cur_ship = FetchFirstShip();
	while ( cur_ship != NULL ) {

		ShipObject *nextship = (ShipObject*) cur_ship->NextObj;

		obj_pos.X  = cur_ship->ObjPosition[ 0 ][ 3 ];
		obj_pos.Y  = cur_ship->ObjPosition[ 1 ][ 3 ];
		obj_pos.Z  = cur_ship->ObjPosition[ 2 ][ 3 ];
		bd_sphere  = cur_ship->BoundingSphere;
		bd_sphere2 = cur_ship->BoundingSphere2;

		// walk all missiles
		MissileObject *walkmissiles = FetchFirstMissile();
		while ( walkmissiles != NULL ) {

			MissileObject *curmissile = walkmissiles;
			walkmissiles = (MissileObject *) walkmissiles->NextObj;

			// owner can't be hit by own missile
			if ( cur_ship->HostObjNumber == ShipHostObjId( curmissile->Owner ) ) {
				continue;
			}

			test_pos.X = curmissile->ObjPosition[ 0 ][ 3 ];
			test_pos.Y = curmissile->ObjPosition[ 1 ][ 3 ];
			test_pos.Z = curmissile->ObjPosition[ 2 ][ 3 ];

			prev_pos.X = curmissile->PrevPosition.X;
			prev_pos.Y = curmissile->PrevPosition.Y;
			prev_pos.Z = curmissile->PrevPosition.Z;

			// shield or actual object?
			SET_SHIELD_TESTING();

			// actual collision detection
			if ( CheckShipProjectileDisjoint() ) {
				continue;
			}

			// collision response
			CollisionMissileRemoteShip( curmissile );
		}

		cur_ship = nextship;
	}
}

#ifndef NEW_PREDICTION_SYSTEM

// boost extra collected: energy boost ----------------------------------------
//
PRIVATE
char *CollectBoostEnergy( Extra1Obj *extra1po )
{
	ASSERT( extra1po != NULL );

	return OBJ_BoostEnergy( MyShip, extra1po->EnergyBoost );
}


// boost extra collected: repair damage ---------------------------------------
//
PRIVATE
char *CollectBoostRepair( Extra1Obj *extra1po )
{
	ASSERT( extra1po != NULL );

	char *text = NULL;

	if ( MyShip->CurDamage == 0 ) {

		text = no_damage_str;
		AUD_MaxedOut( REPAIR_EXTRA_CLASS );

	} else {

		MyShip->CurDamage -= extra1po->EnergyBoost;
		if ( MyShip->CurDamage < 0 )
			MyShip->CurDamage = 0;
		text = damage_repaired;
		AUD_DamageRepaired();
	}

	return text;
}


// package extra collected: dumb missiles -------------------------------------
//
PRIVATE
char *CollectPackDumb( Extra2Obj *extra2po )
{
	ASSERT( extra2po != NULL );

	char *text = NULL;

	if ( MyShip->NumMissls == MyShip->MaxNumMissls ) {

		text = max_dumb_str;
		AUD_MaxedOut( MISSILE1TYPE );

	} else {

		MyShip->NumMissls += extra2po->NumMissiles;
		if ( MyShip->NumMissls > MyShip->MaxNumMissls )
			MyShip->NumMissls = MyShip->MaxNumMissls;
		text = got_dumb_str;
		AUD_ExtraCollected( MISSILE1TYPE );
	}

	return text;
}


// package extra collected: guided missiles -----------------------------------
//
PRIVATE
char *CollectPackGuide( Extra2Obj *extra2po )
{
	ASSERT( extra2po != NULL );

	char *text = NULL;

	if ( MyShip->NumHomMissls == MyShip->MaxNumHomMissls ) {

		text = max_homing_str;
		AUD_MaxedOut( MISSILE4TYPE );

	} else {

		MyShip->NumHomMissls += extra2po->NumMissiles;
		if ( MyShip->NumHomMissls > MyShip->MaxNumHomMissls )
			MyShip->NumHomMissls = MyShip->MaxNumHomMissls;
		text = got_homing_str;
		AUD_ExtraCollected( MISSILE4TYPE );
	}

	return text;
}


// device extra collected: swarm missiles -------------------------------------
//
PRIVATE
char *CollectPackSwarm( Extra2Obj *extra2po )
{
	ASSERT( extra2po != NULL );

	char *text = NULL;

	if ( MyShip->NumPartMissls == MyShip->MaxNumPartMissls ) {

		text = max_swarm_str;
		AUD_MaxedOut( MISSILE5TYPE );

	} else {

		MyShip->NumPartMissls += extra2po->NumMissiles;
		if ( MyShip->NumPartMissls > MyShip->MaxNumPartMissls )
			MyShip->NumPartMissls = MyShip->MaxNumPartMissls;
		text = got_swarm_str;
		AUD_ExtraCollected( MISSILE5TYPE );
	}

	return text;
}


// package extra collected: proximity mines -----------------------------------
//
PRIVATE
char *CollectPackMine( Extra2Obj *extra2po )
{
	ASSERT( extra2po != NULL );

	char *text = NULL;

	if ( MyShip->NumMines == MyShip->MaxNumMines ) {

		text = max_mines_str;
		AUD_MaxedOut( MINE1TYPE );

	} else {

		MyShip->NumMines += extra2po->NumMissiles;
		if ( MyShip->NumMines > MyShip->MaxNumMines )
			MyShip->NumMines = MyShip->MaxNumMines;
		text = got_mines_str;
		AUD_ExtraCollected( MINE1TYPE );
	}

	return text;
}


// device extra collected: helix cannon ---------------------------------------
//
PRIVATE
char *CollectDeviceHelix()
{
	char *text = NULL;

	if ( ( MyShip->Weapons & WPMASK_CANNON_HELIX ) == 0 ) {

		MyShip->Weapons |= WPMASK_CANNON_HELIX;
		text = got_helix_str;
		AUD_ExtraCollected( HELIX_DEVICE );

	} else {

		text = max_helix_str;
		AUD_MaxedOut( HELIX_DEVICE );
	}

	return text;
}


// device extra collected: lightning device -----------------------------------
//
PRIVATE
char *CollectDeviceLightning()
{
	char *text = NULL;

	if ( ( MyShip->Weapons & WPMASK_CANNON_LIGHTNING ) == 0 ) {

		MyShip->Weapons |= WPMASK_CANNON_LIGHTNING;
		text = got_lightning_str;
		AUD_ExtraCollected( LIGHTNING_DEVICE );

	} else {

		text = max_lightning_str;
		AUD_MaxedOut( LIGHTNING_DEVICE );
	}

	return text;
}


// device extra collected: photon cannon --------------------------------------
//
PRIVATE
char *CollectDevicePhoton()
{
	char *text = NULL;

	if ( ( MyShip->Weapons & WPMASK_CANNON_PHOTON ) == 0 ) {

		MyShip->Weapons |= WPMASK_CANNON_PHOTON;
		text = got_photon_str;
		AUD_ExtraCollected( PHOTON_DEVICE );

	} else {

		text = max_photon_str;
		AUD_MaxedOut( PHOTON_DEVICE );
	}

	return text;
}


// device extra collected: afterburner ----------------------------------------
//
PRIVATE
char *CollectDeviceAfterburner()
{
	char *text = NULL;

	if ( ( MyShip->Specials & SPMASK_AFTERBURNER ) == 0 ) {

		text = got_afterburner_str;
		AUD_ExtraCollected( AFTERBURNER_DEVICE );

		OBJ_EnableAfterBurner( MyShip );

	} else {

		text = max_afterburner_str;
		AUD_ExtraCollected( AFTERBURNER_DEVICE );

		OBJ_EnableAfterBurner( MyShip );
	}

	return text;
}


// device extra collected: invisibility ---------------------------------------
//
PRIVATE
char *CollectDeviceInvisibility()
{
	char *text = NULL;

	if ( ( MyShip->Specials & SPMASK_INVISIBILITY ) == 0 ) {

		text = got_cloak_str;
		AUD_ExtraCollected( INVISIBILITY_DEVICE );

		OBJ_EnableInvisibility( MyShip );

	} else {

		text = max_cloak_str;
		AUD_ExtraCollected( INVISIBILITY_DEVICE );

		OBJ_EnableInvisibility( MyShip );
	}

	return text;
}


// device extra collected: invulnerability ------------------------------------
//
PRIVATE
char *CollectDeviceInvulnerability()
{
	char *text = NULL;

	if ( ( MyShip->Specials & SPMASK_INVULNERABILITY ) == 0 ) {

		text = got_megashield_str;
		AUD_ExtraCollected( INVULNERABILITY_DEVICE );

		OBJ_EnableInvulnerability( MyShip );

	} else {

		text = max_megashield_str;
		AUD_ExtraCollected( INVULNERABILITY_DEVICE );

		OBJ_EnableInvulnerability( MyShip );
	}

	return text;
}


// device extra collected: decoy ----------------------------------------------
//
PRIVATE
char *CollectDeviceDecoy()
{
	char *text = NULL;

	text = got_decoy_str;
	AUD_ExtraCollected( DECOY_DEVICE );

	OBJ_EnableDecoy( MyShip );

	return text;
}


// device extra collected: laser upgrade 1 ------------------------------------
//
PRIVATE
char *CollectDeviceLaserUpgrade1()
{
	char *text = NULL;

	if ( MyShip->Specials & SPMASK_LASER_UPGRADE_2 ) {

		text = max_laser_upgrade_2_str;
		AUD_MaxedOut( LASER_UPGRADE_1_DEVICE );

	} else {

		if ( ( MyShip->Specials & SPMASK_LASER_UPGRADE_1 ) == 0 ) {

			text = got_laser_upgrade_1_str;
			AUD_ExtraCollected( LASER_UPGRADE_1_DEVICE );

			OBJ_EnableLaserUpgrade1( MyShip );

		} else {

			text = max_laser_upgrade_1_str;
			AUD_MaxedOut( LASER_UPGRADE_1_DEVICE );
		}
	}

	return text;
}


// device extra collected: laser upgrade 2 ------------------------------------
//
PRIVATE
char *CollectDeviceLaserUpgrade2()
{
	char *text = NULL;

	if ( ( MyShip->Specials & SPMASK_LASER_UPGRADE_1 ) == 0 ) {

		text = need_laser_upgrade_1_str;

	} else {

		if ( ( MyShip->Specials & SPMASK_LASER_UPGRADE_2 ) == 0 ) {

			text = got_laser_upgrade_2_str;
			AUD_ExtraCollected( LASER_UPGRADE_2_DEVICE );

			OBJ_EnableLaserUpgrade2( MyShip );

		} else {

			text = max_laser_upgrade_2_str;
			AUD_MaxedOut( LASER_UPGRADE_2_DEVICE );
		}
	}

	return text;
}


// device extra collected: emp upgrade 1 --------------------------------------
//
PRIVATE
char *CollectDeviceEmpUpgrade1()
{
	char *text = NULL;

	if ( MyShip->Specials & SPMASK_EMP_UPGRADE_2 ) {

		text = max_emp_upgrade_2_str;
		AUD_MaxedOut( EMP_UPGRADE_1_DEVICE );

	} else {

		if ( ( MyShip->Specials & SPMASK_EMP_UPGRADE_1 ) == 0 ) {

			text = got_emp_upgrade_1_str;
			AUD_ExtraCollected( EMP_UPGRADE_1_DEVICE );

			OBJ_EnableEmpUpgrade1( MyShip );

		} else {

			text = max_emp_upgrade_1_str;
			AUD_MaxedOut( EMP_UPGRADE_1_DEVICE );
		}
	}

	return text;
}


// device extra collected: emp upgrade 2 --------------------------------------
//
PRIVATE
char *CollectDeviceEmpUpgrade2()
{
	char *text = NULL;

	if ( ( MyShip->Specials & SPMASK_EMP_UPGRADE_1 ) == 0 ) {

		text = need_emp_upgrade_1_str;

	} else {

		if ( ( MyShip->Specials & SPMASK_EMP_UPGRADE_2 ) == 0 ) {

			text = got_emp_upgrade_2_str;
			AUD_ExtraCollected( EMP_UPGRADE_2_DEVICE );

			OBJ_EnableEmpUpgrade2( MyShip );

		} else {

			text = max_emp_upgrade_2_str;
			AUD_MaxedOut( EMP_UPGRADE_2_DEVICE );
		}
	}

	return text;
}

#endif // NEW_PREDICTION_SYSTEM

// local ship collided with proximity mine ------------------------------------
//
PRIVATE
char *CollisionProximityMine( Mine1Obj *minepo )
{
	ASSERT( minepo != NULL );

	char *text = NULL;

	if ( NetConnected ) {
        // RE only in PEER mode
	    if ( NET_ConnectedPEER() ) {
		    NET_RmEvKillObject( minepo->HostObjNumber, EXTRA_LIST );
		}
		KillSpecificObject( minepo->ObjectNumber, ExtraObjects );
        
		int hitpoints = minepo->HitPoints;
		if ( ( hitpoints -= MyShip->MegaShieldAbsorption ) < 0 ) {
			hitpoints = 0;
		}

		if ( MyShip->CurDamage <= MyShip->MaxDamage ) {

			MyShip->CurDamage += hitpoints;
			
			if ( MyShip->CurDamage > MyShip->MaxDamage ) {
				// Object removal only in Peer
	            if ( NET_ConnectedPEER() ) {

				    KillDurationWeapons( MyShip );
				    OBJ_CreateShipExtras( MyShip );

				    // if mine is owned by remote player update kill state
				    if ( minepo->Owner != OWNER_LOCAL_PLAYER ) {
					    NET_SetPlayerKillStat( minepo->Owner, 1 );
				    }

				    text = mine_killed_str;
				    AUD_PlayerKilled();
				}
			} else {

				//TODO:
				// detect hull impact

				OBJ_EventShipImpact( MyShip, TRUE );

				text = mine_hit_str;
				AUD_MineCollision();
			}

		} else {

			// occurs on mine hit when already killed
			text = mine_hit_str;
		}

	} else {

		//NOTE:
		// in standalone mode no damage is caused by mines!

		//TODO:
		// detect hull impact
        KillSpecificObject( minepo->ObjectNumber, ExtraObjects );
		OBJ_EventShipImpact( MyShip, TRUE );

		text = mine_hit_str;
		AUD_MineCollision();
	}

	return text;
}

#ifndef NEW_PREDICTION_SYSTEM

// perform action according to extra type and determine message to show -------
//
PRIVATE
char *CollectExtra( ExtraObject *curextra )
{
	ASSERT( curextra != NULL );

	char *text = NULL;

	// extras of type 1 (boost extras)
	if ( curextra->ObjectType == EXTRA1TYPE ) {

		Extra1Obj *extra1po = (Extra1Obj *) curextra;

		switch ( extra1po->ObjectClass ) {

			// energy boost extra
			case ENERGY_EXTRA_CLASS:
				text = CollectBoostEnergy( extra1po );
				break;

			// damage repair extra
			case REPAIR_EXTRA_CLASS:
				text = CollectBoostRepair( extra1po );
				break;

			// unrecognized class for this type
			default:
				MSGOUT( "OBJ_COLL::CollectExtra(): unknown boost extra: class %d.", extra1po->ObjectClass );
				text = unknown_extra_str;
		}

	// extras of type 2 (package extras)
	} else if ( curextra->ObjectType == EXTRA2TYPE ) {

		Extra2Obj *extra2po = (Extra2Obj *) curextra;

		switch ( extra2po->MissileType ) {

			// dumb missile package
			case MISSILE1TYPE:
				text = CollectPackDumb( extra2po );
				break;

			// guide missile package
			case MISSILE4TYPE:
				text = CollectPackGuide( extra2po );
				break;

			// swarm missiles package
			case MISSILE5TYPE:
				text = CollectPackSwarm( extra2po );
				break;

			// proximity mine package
			case MINE1TYPE:
				text = CollectPackMine( extra2po );
				break;

			// unrecognized class for this type
			default:
				MSGOUT( "OBJ_COLL::CollectExtra(): unknown package extra: class %d.", extra2po->ObjectClass );
				text = unknown_extra_str;
		}

	// extras of type 3 (device extras)
	} else if ( curextra->ObjectType == EXTRA3TYPE ) {

		Extra3Obj *extra3po = (Extra3Obj *) curextra;

		switch ( extra3po->DeviceType ) {

			// helix cannon
			case HELIX_DEVICE:
				text = CollectDeviceHelix();
				break;

			// lightning device
			case LIGHTNING_DEVICE:
				text = CollectDeviceLightning();
				break;

			// afterburner device
			case AFTERBURNER_DEVICE:
				text = CollectDeviceAfterburner();
				break;

			// cloaking (invisibility) device
			case INVISIBILITY_DEVICE:
				text = CollectDeviceInvisibility();
				break;

			// photon cannon
			case PHOTON_DEVICE:
				text = CollectDevicePhoton();
				break;

			// invulnerability device
			case INVULNERABILITY_DEVICE:
				text = CollectDeviceInvulnerability();
				break;

			// decoy device
			case DECOY_DEVICE:
				text = CollectDeviceDecoy();
				break;

			// laser upgrade 1
			case LASER_UPGRADE_1_DEVICE:
				text = CollectDeviceLaserUpgrade1();
				break;

			// laser upgrade 2
			case LASER_UPGRADE_2_DEVICE:
				text = CollectDeviceLaserUpgrade2();
				break;

			// emp upgrade 1
			case EMP_UPGRADE_1_DEVICE:
				text = CollectDeviceEmpUpgrade1();
				break;

			// emp upgrade 2
			case EMP_UPGRADE_2_DEVICE:
				text = CollectDeviceEmpUpgrade2();
				break;

			// unrecognized device type found
			default:
				MSGOUT( "OBJ_COLL::CollectExtra(): unknown device extra: class %d.", extra3po->ObjectClass );
				text = unknown_dev_str;
		}

	// collision with proximity mine
	} else if ( curextra->ObjectType == MINE1TYPE ) {

		text = CollisionProximityMine( (Mine1Obj *) curextra );
	}

	return text;
}

#endif // NEW_PREDICTION_SYSTEM










// check if ship collected extra ----------------------------------------------
//
PRIVATE
void CheckShipExtraCollision()
{

#ifndef NO_EXTRA_MAINTENANCE

	// fetch location of local ship
	geomv_t	objX = MyShip->ObjPosition[ 0 ][ 3 ];
	geomv_t	objY = MyShip->ObjPosition[ 1 ][ 3 ];
	geomv_t	objZ = MyShip->ObjPosition[ 2 ][ 3 ];

	//MSGOUT( "CheckShipExtraCollision(): ship is at %.2f/%.2f/%.2f", objX, objY, objZ );

	// fetch radius of bounding sphere of local ship
	geomv_t	bsphere = MyShip->BoundingSphere;

	ASSERT( ExtraObjects != NULL );

#ifdef ASSERT_IF_NUMEXTRAS_INVALID
	ASSERT( CurrentNumExtras >= 0 );
	ASSERT( CurrentNumExtras + CurrentNumPrtExtras <= MaxNumExtras * MAX_NET_PROTO_PLAYERS );
#endif

	// walk list of extras
	ExtraObject *precnode = ExtraObjects;
	while ( precnode->NextObj != NULL ) {

		ASSERT( OBJECT_TYPE_EXTRA( precnode->NextObj ) );

		// get pointer to current extra
		ExtraObject *curextra = (ExtraObject *) precnode->NextObj;
		ASSERT( curextra != NULL );

		// check if lifetime of extra is spent
		curextra->LifeTimeCount -= CurScreenRefFrames;

		if ( curextra->LifeTimeCount < 0 ) {

#ifdef PARSEC_DEBUG
			if ( NET_ConnectedGMSV() ) {
				//ASSERT( FALSE );
				MSGOUT( "OBJ_COLL::CheckShipExtraCollision(): predicted freeing of extra object" );
			}
#endif // INTERNAL_VERSION

			OBJ_KillExtra( precnode, FALSE );
			continue;
		}

		// reset VisibleFrame of extra ( reactivate zombie extra ? )
		if ( ( curextra->VisibleFrame == VISFRAME_NEVER ) && ( curextra->VisibleFrame_Reset_Frames >= 0 ) ) {
			curextra->VisibleFrame_Reset_Frames -= CurScreenRefFrames;

			//FIXME: 
#ifdef PARSEC_DEBUG
			MSGOUT( "OBJ_COLL::CheckShipExtraCollision(): checkin curextra->VisibleFrame_Reset_Frames %d ( type %d ) ", curextra->VisibleFrame_Reset_Frames, curextra->ObjectType );
#endif
			if ( curextra->VisibleFrame_Reset_Frames <= 0 ) {

				//FIXME: 
#ifdef PARSEC_DEBUG
				MSGOUT( "OBJ_COLL::CheckShipExtraCollision(): RESET predicted collision with extra object" );
#endif
				curextra->VisibleFrame = CurVisibleFrame;
				//FIXME: better audio sample
				AUD_SkillAmazing();
			}
		}
		

		// animate this extra
		OBJ_AnimateExtra( curextra );

		// avoid collecting the extras that have been created on the start of the ship explosion
		// or that are predicted to be removed
		if ( ( MyShip->CurDamage > MyShip->MaxDamage ) || ( curextra->VisibleFrame == VISFRAME_NEVER ) ) {
			precnode = curextra;
			continue;
		}

		// fetch position of extra
		geomv_t	extraX = curextra->ObjPosition[ 0 ][ 3 ];
		geomv_t	extraY = curextra->ObjPosition[ 1 ][ 3 ];
		geomv_t	extraZ = curextra->ObjPosition[ 2 ][ 3 ];

		// do simple collision test (bounding box)
		if ( extraX < ( objX - bsphere ) ) { precnode = curextra; continue; }
		if ( extraX > ( objX + bsphere ) ) { precnode = curextra; continue; }
		if ( extraY < ( objY - bsphere ) ) { precnode = curextra; continue; }
		if ( extraY > ( objY + bsphere ) ) { precnode = curextra; continue; }
		if ( extraZ < ( objZ - bsphere ) ) { precnode = curextra; continue; }
		if ( extraZ > ( objZ + bsphere ) ) { precnode = curextra; continue; }

		// do actual bounding sphere collision test
		Vector3 vecdist;
		vecdist.X = extraX - objX;
		vecdist.Y = extraY - objY;
		vecdist.Z = extraZ - objZ;

		geomv_t dist2 = DOT_PRODUCT( &vecdist, &vecdist );
		if ( dist2 > MyShip->BoundingSphere2 ) {
			precnode = curextra;
			continue;
		}

#ifdef NEW_PREDICTION_SYSTEM

		// add a prediction event for the extra collection
		ThePredictionManager.PredictExtraCollect( curextra );

#else
		// perform action according to extra type and determine message to show
		char *text = CollectExtra( curextra );

		// show message saying what type of extra has been collected
		if ( text != NULL ) {
			WriteExtraCollectMessage( text );
		} else {
			MSGOUT( "CheckShipExtraCollision(): invalid extra type: %0x.", curextra->ObjectType );
		}
#endif

		
		//NOTE: in GMSV mode, the client never kills an extra, only predicts the removal, 
		//      by making it a zombie for a certain time.
		if ( NET_ConnectedGMSV() ) {

			// set the timeout this extra should become a zombie ( invisible )
			curextra->VisibleFrame_Reset_Frames = EXTRA_PREDICT_ZOMBIE_TIME;
			curextra->VisibleFrame = VISFRAME_NEVER;

			//FIXME:
#ifdef PARSEC_DEBUG
			MSGOUT( "OBJ_COLL::CheckShipExtraCollision(): predict collision with extra object" );
#endif
			if ( ((MineObject * )curextra)->Owner == OWNER_LOCAL_PLAYER ) { //Players own objects are not send by the RE list for cleanup
                  OBJ_KillExtra( precnode, TRUE );     
            }
		} else { 
			// remove extra object
			OBJ_KillExtra( precnode, TRUE );
		}
	}

#endif // NO_EXTRA_MAINTENANCE

}


// check if ship collides with another ship -----------------------------------
//
PRIVATE
void CheckShipShipCollision()
{
	#define BOUNCE_STRENGTH 	2
	#define BOUNCE_COUNT		48

	geomv_t objX = MyShip->ObjPosition[ 0 ][ 3 ];
	geomv_t objY = MyShip->ObjPosition[ 1 ][ 3 ];
	geomv_t objZ = MyShip->ObjPosition[ 2 ][ 3 ];

	geomv_t bsphere  = MyShip->BoundingSphere * 2;
	geomv_t bsphere2 = MyShip->BoundingSphere2;

	ShipObject *walkships  = FetchFirstShip();
	ShipObject *walkothers = walkships;

	// check local ship against all others
	while ( walkothers != NULL ) {

		geomv_t otherX = walkothers->ObjPosition[ 0 ][ 3 ];
		geomv_t otherY = walkothers->ObjPosition[ 1 ][ 3 ];
		geomv_t otherZ = walkothers->ObjPosition[ 2 ][ 3 ];

		ShipObject *curother = walkothers;
		walkothers = (ShipObject*) walkothers->NextObj;

		if ( otherX < ( objX - bsphere ) ) continue;
		if ( otherX > ( objX + bsphere ) ) continue;
		if ( otherY < ( objY - bsphere ) ) continue;
		if ( otherY > ( objY + bsphere ) ) continue;
		if ( otherZ < ( objZ - bsphere ) ) continue;
		if ( otherZ > ( objZ + bsphere ) ) continue;

		Vector3 bouncevec;
		bouncevec.X = otherX - objX;
		bouncevec.Y = otherY - objY;
		bouncevec.Z = otherZ - objZ;

		geomv_t veclen = DOT_PRODUCT( &bouncevec, &bouncevec );
		if ( veclen > ( bsphere2 * 4 ) )
			continue;

		if ( veclen > GEOMV_VANISHING )
			NormVctX( &bouncevec );

		curother->BounceVec.X = bouncevec.X / BOUNCE_STRENGTH;
		curother->BounceVec.Y = bouncevec.Y / BOUNCE_STRENGTH;
		curother->BounceVec.Z = bouncevec.Z / BOUNCE_STRENGTH;
		curother->BounceCount = BOUNCE_COUNT;

		MyShip->BounceVec.X = -bouncevec.X / BOUNCE_STRENGTH;
		MyShip->BounceVec.Y = -bouncevec.Y / BOUNCE_STRENGTH;
		MyShip->BounceVec.Z = -bouncevec.Z / BOUNCE_STRENGTH;
		MyShip->BounceCount = BOUNCE_COUNT;
	}

	// check non-local ships against each other
	while ( walkships != NULL ) {

		ShipObject *nextship = (ShipObject*) walkships->NextObj;

		objX	 = walkships->ObjPosition[ 0 ][ 3 ];
		objY	 = walkships->ObjPosition[ 1 ][ 3 ];
		objZ	 = walkships->ObjPosition[ 2 ][ 3 ];
		bsphere  = walkships->BoundingSphere * 2;
		bsphere2 = walkships->BoundingSphere2;

		walkothers = nextship;
		while ( walkothers != NULL ) {

			geomv_t otherX = walkothers->ObjPosition[ 0 ][ 3 ];
			geomv_t otherY = walkothers->ObjPosition[ 1 ][ 3 ];
			geomv_t otherZ = walkothers->ObjPosition[ 2 ][ 3 ];

			ShipObject *curother = walkothers;
			walkothers = (ShipObject*) walkothers->NextObj;

			if ( otherX < ( objX - bsphere ) ) continue;
			if ( otherX > ( objX + bsphere ) ) continue;
			if ( otherY < ( objY - bsphere ) ) continue;
			if ( otherY > ( objY + bsphere ) ) continue;
			if ( otherZ < ( objZ - bsphere ) ) continue;
			if ( otherZ > ( objZ + bsphere ) ) continue;

			Vector3 bouncevec;
			bouncevec.X = otherX - objX;
			bouncevec.Y = otherY - objY;
			bouncevec.Z = otherZ - objZ;

			geomv_t veclen = DOT_PRODUCT( &bouncevec, &bouncevec );
			if ( veclen > ( bsphere2 * 4 ) )
				continue;

			if ( veclen > GEOMV_VANISHING )
				NormVctX( &bouncevec );

			curother->BounceVec.X  = bouncevec.X / BOUNCE_STRENGTH;
			curother->BounceVec.Y  = bouncevec.Y / BOUNCE_STRENGTH;
			curother->BounceVec.Z  = bouncevec.Z / BOUNCE_STRENGTH;
			curother->BounceCount  = BOUNCE_COUNT;
			walkships->BounceVec.X = -bouncevec.X / BOUNCE_STRENGTH;
			walkships->BounceVec.Y = -bouncevec.Y / BOUNCE_STRENGTH;
			walkships->BounceVec.Z = -bouncevec.Z / BOUNCE_STRENGTH;
			walkships->BounceCount = BOUNCE_COUNT;
		}

		walkships = nextship;
	}
}


// invoke collision callbacks of custom objects -------------------------------
//
PRIVATE
void CheckCustomCollisions()
{
	ASSERT( CustmObjects != NULL );

	GenObject *precnode   = CustmObjects;
	GenObject *walkcustom = CustmObjects->NextObj;

	// walk entire list
	while ( walkcustom != NULL ) {

		CustomObject *custompo = (CustomObject *) walkcustom;
		if ( custompo->callback_collide != NULL ) {

			// call collision function
			if ( (*custompo->callback_collide)( custompo ) == FALSE ) {

				// delete object
				ASSERT( walkcustom != NULL );
				precnode->NextObj = walkcustom->NextObj;
				FreeObjectMem( walkcustom );
				walkcustom = precnode->NextObj;
				continue;
			}
		}

		precnode   = walkcustom;
		walkcustom = walkcustom->NextObj;
	}
}


// perform collision detection tests ------------------------------------------
//
void OBJ_CheckCollisions()
{
	//NOTE:
	// this function gets called once per frame by the
	// game loop (G_MAIN.C).

	// check if laser beam hits ship
	CheckShipLaserCollision();

	// check if missile hits ship
	CheckShipMissileCollision();

	// check if two ships collide
	CheckShipShipCollision();

	// check if ship collected extra
	CheckShipExtraCollision();

	// check collisions for custom objects
	CheckCustomCollisions();
}



