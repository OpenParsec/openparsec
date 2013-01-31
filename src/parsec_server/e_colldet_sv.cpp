/*
 * PARSEC - Game collision detection - SERVER
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:44 $
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
#include "e_defs.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "e_colldet.h"

// proprietary module headers
#include "g_extra.h"
#include "net_game_sv.h"
#include "g_main_sv.h"
#include "e_simplayerinfo.h"
#include "e_connmanager.h"
#include "e_simnetoutput.h"
#include "e_simulator.h"
#include "parttype.h"

#define SET_SHIELD_TESTING()					{ test_shield = TRUE; }
//#define SET_SHIELD_TESTING()					{ test_shield = FALSE; }

#define CONSERVATIVE_POINTSAMPLE_EXPANSION		16

// collision detection helper function ----------------------------------------
//
int G_CollDet::_CheckShipProjectileDisjoint()
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

		ASSERT( FALSE );
/*
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
*/
	}

	return FALSE;
}

//Check if any ship is hit by missile
void G_CollDet::_CheckShipMissileCollision()
{
	// check whether any other ship object is hit by missile
	for ( cur_ship = TheWorld->FetchFirstShip(); cur_ship != NULL; ) {

		ShipObject *nextship = (ShipObject*) cur_ship->NextObj;

		obj_pos.X  = cur_ship->ObjPosition[ 0 ][ 3 ];
		obj_pos.Y  = cur_ship->ObjPosition[ 1 ][ 3 ];
		obj_pos.Z  = cur_ship->ObjPosition[ 2 ][ 3 ];
		bd_sphere  = cur_ship->BoundingSphere;
		bd_sphere2 = cur_ship->BoundingSphere2;

		
        int cur_ship_owner = GetOwnerFromHostOjbNumber( cur_ship->HostObjNumber );

        // walk all missiles
		MissileObject *walkmissiles = TheWorld->FetchFirstMissile();
		while ( walkmissiles != NULL ) {

			MissileObject *curmissile = walkmissiles;
			walkmissiles = (MissileObject *) walkmissiles->NextObj;

			// owner can't be hit by own missile
			if ( curmissile->Owner == cur_ship_owner ) {
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
			if ( _CheckShipProjectileDisjoint() ) {
				continue;
			}

			// collision response
			_CollisionResponse_MissileShip( curmissile );
		}

		cur_ship = nextship;
	}

}

// check whether any ship is hit by laser beam --------------------------------
//
void G_CollDet::_CheckShipLaserCollision()
{
#ifdef PARSEC_CLIENT
	// check whether local player is hit by laser
	if ( NetConnected ) {

		cur_ship   = MyShip;
		obj_pos.X  = MyShip->ObjPosition[ 0 ][ 3 ];
		obj_pos.Y  = MyShip->ObjPosition[ 1 ][ 3 ];
		obj_pos.Z  = MyShip->ObjPosition[ 2 ][ 3 ];
		bd_sphere  = MyShip->BoundingSphere;
		bd_sphere2 = MyShip->BoundingSphere2;

		// walk all laser projectiles
		LaserObject *walklasers = TheWorld->FetchFirstLaser();
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
#endif // PARSEC_CLIENT

	// check whether any (other) shipobject is hit by laser
	for ( cur_ship = TheWorld->FetchFirstShip(); cur_ship != NULL; ) {

		// get pointer to next ship in list, as the current might get removed upon collision
		ShipObject* nextship = (ShipObject *) cur_ship->NextObj;

		obj_pos.X  = cur_ship->ObjPosition[ 0 ][ 3 ];
		obj_pos.Y  = cur_ship->ObjPosition[ 1 ][ 3 ];
		obj_pos.Z  = cur_ship->ObjPosition[ 2 ][ 3 ];
		bd_sphere  = cur_ship->BoundingSphere;
		bd_sphere2 = cur_ship->BoundingSphere2;

		int cur_ship_owner = GetOwnerFromHostOjbNumber( cur_ship->HostObjNumber );

		// walk all laser projectiles
		for ( LaserObject *walklasers = TheWorld->FetchFirstLaser(); walklasers != NULL; ) {

			LaserObject* curlaser = walklasers;
			walklasers = (LaserObject *) walklasers->NextObj;

			// owner can't be hit by own laser projectile
			if ( curlaser->Owner == cur_ship_owner ) {
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
			if ( _CheckShipProjectileDisjoint() ) {
				continue;
			}

			// collision response
			_CollisionResponse_LaserShip( curlaser );
		}

		cur_ship = nextship;
	}
}

// emp collision detection ----------------------------------------------------
//
void G_CollDet::_CheckShipEmpCollision()
{
	// walk the list of EMP objects
	ASSERT( TheWorld->m_CustmObjects != NULL );

	CustomObject *precnode  = TheWorld->m_CustmObjects;
	CustomObject *walkobjs = (CustomObject *)TheWorld->m_CustmObjects->NextObj;

	// walk all lasers
	while ( walkobjs != NULL ) {
		if ((walkobjs->ObjectType == emp_type_id[ 0 ]) ||
			(walkobjs->ObjectType == emp_type_id[ 1 ]) ||
			(walkobjs->ObjectType == emp_type_id[ 2 ])) {

				Emp *tmpemp = (Emp *)walkobjs;
				// got an emp object, now let's walk the ship objects
				// and check for collisions.

				for ( cur_ship = TheWorld->FetchFirstShip(); cur_ship != NULL; ) {
					// current ship's owner
					int cur_ship_owner = GetOwnerFromHostOjbNumber( cur_ship->HostObjNumber );

					// get pointer to next ship in list, as the current might get removed upon collision
					ShipObject* nextship = (ShipObject *) cur_ship->NextObj;

					// cant collide with the owner
					if(tmpemp->Owner != cur_ship_owner){
						// do the actual check
						if(EmpShipCollision(tmpemp, cur_ship)){
							// HIT!  Record damage and send out packets.
							_CollisionResponse_EmpShip(tmpemp);
						}
					}
					cur_ship = nextship;
				}
		}

		precnode  = walkobjs;
		walkobjs = (CustomObject *)walkobjs->NextObj;
	}


}
// ship collided with EMP ---------------------------------------------------
//
void G_CollDet::_CollisionResponse_EmpShip( Emp *curemp )
{
	ASSERT( curemp != NULL );
	ASSERT( cur_ship != NULL );

	int hitpoints = curemp->damage;

	if ( ( hitpoints -= cur_ship->MegaShieldAbsorption ) < 0 ) {
		hitpoints = 0;
	}

	// apply damage
	if ( cur_ship->CurDamage <= cur_ship->MaxDamage ) {
		MSGOUT("Hitpoints deduction is %d", hitpoints);
		MSGOUT("Damage Calc Ref Frames is %d",TheSimulator->GetThisFrameRefFrames()*2);
		cur_ship->CurDamageFrac +=  (TheSimulator->GetThisFrameRefFrames()*2) * hitpoints;
		MSGOUT("Damage Fractional is %d",cur_ship->CurDamageFrac);
		cur_ship->CurDamage += cur_ship->CurDamageFrac >> 16;
		MSGOUT("Doing %d damage to ship", cur_ship->CurDamageFrac >> 16);
		cur_ship->CurDamageFrac &= 0xffff;


		// new less confusing shit...
		//cur_ship->CurDamage += hitpoints;
	}

	// check whether the ship should explode
	if ( cur_ship->CurDamage > cur_ship->MaxDamage ) {
		int nClientID_Attacker = curemp->Owner;
		int nClientID_Downed   = GetOwnerFromHostOjbNumber( cur_ship->HostObjNumber );

		// create the extras this client left
		TheGameExtraManager->OBJ_CreateShipExtras( cur_ship );

		E_SimPlayerInfo* pSimPlayerInfo = TheSimulator->GetSimPlayerInfo( nClientID_Downed );

		// maintain stats
		TheGame->RecordKill ( nClientID_Attacker );
		TheGame->RecordDeath( nClientID_Downed, nClientID_Attacker );

		// fill rudimentary RE
		RE_PlayerStatus ps;
		memset( &ps, 0, sizeof ( RE_PlayerStatus ) );
		ps.player_status	= PLAYER_CONNECTED;
		ps.senderid         = nClientID_Downed;
		ps.params[ 0 ]		= SHIP_DOWNED;
		pSimPlayerInfo->PerformUnjoin( &ps );

		// ignore joins from client, until he himself sent an unjoin
		//FIXME: redesign this !!
		pSimPlayerInfo->IgnoreJoinUntilUnjoinFromClient();

		// force a client resync of the downed client
		TheSimulator->GetSimClientState( nClientID_Downed )->SetClientResync();

		//FIXME:
		//cur_ship->pDist->UpdateMODE_REFRESH();

		//FIXME: this should be called GAME_LOGOUT or so to indicate that these messages
		//       are the most important ones for processing user stats
		MSGOUT( "%s was shot down by %s's EMP", TheConnManager->GetClientName( nClientID_Downed ), TheConnManager->GetClientName( nClientID_Attacker ) );

	}
}


// ship collided with Missile ---------------------------------------------------
//
void G_CollDet::_CollisionResponse_MissileShip( MissileObject *curmissile )
{
	ASSERT( curmissile != NULL );
	ASSERT( cur_ship != NULL );

	int hitpoints = curmissile->HitPoints;
	if ( ( hitpoints -= cur_ship->MegaShieldAbsorption ) < 0 ) {
		hitpoints = 0;
	}

	// apply damage
	if ( cur_ship->CurDamage <= cur_ship->MaxDamage ) {
		cur_ship->CurDamage += hitpoints;
	}

	// check whehter the ship should explode
	if ( cur_ship->CurDamage > cur_ship->MaxDamage ) {


		int nClientID_Attacker = curmissile->Owner;
		int nClientID_Downed   = GetOwnerFromHostOjbNumber( cur_ship->HostObjNumber );

		// create the extras this client left
		TheGameExtraManager->OBJ_CreateShipExtras( cur_ship );

		E_SimPlayerInfo* pSimPlayerInfo = TheSimulator->GetSimPlayerInfo( nClientID_Downed ); 

		// maintain stats
		TheGame->RecordKill ( nClientID_Attacker );
		TheGame->RecordDeath( nClientID_Downed, nClientID_Attacker );

		// fill rudimentary RE
		RE_PlayerStatus ps;
		memset( &ps, 0, sizeof ( RE_PlayerStatus ) );
		ps.player_status	= PLAYER_CONNECTED;
		ps.senderid         = nClientID_Downed;
		ps.params[ 0 ]		= SHIP_DOWNED;
		pSimPlayerInfo->PerformUnjoin( &ps );

		// ignore joins from client, until he himself sent an unjoin
		//FIXME: redesign this !!
		pSimPlayerInfo->IgnoreJoinUntilUnjoinFromClient();

		// force a client resync of the downed client
		TheSimulator->GetSimClientState( nClientID_Downed )->SetClientResync();

		//FIXME:
		//cur_ship->pDist->UpdateMODE_REFRESH();

		//FIXME: this should be called GAME_LOGOUT or so to indicate that these messages
		//       are the most important ones for processing user stats
		MSGOUT( "%s was shot down by %s's Missile", TheConnManager->GetClientName( nClientID_Downed ), TheConnManager->GetClientName( nClientID_Attacker ) );
	}

	// release the E_Distributable ( distribute removal )
	//MSGOUT( "G_CollDet::_CollisionResponse_MissileShip() calls ReleaseDistributable()" );
	TheSimNetOutput->ReleaseDistributable( curmissile->pDist );

	TheWorld->KillSpecificObject( curmissile->ObjectNumber, TheWorld->m_MisslObjects );

}

// ship collided with laser ---------------------------------------------------
//
void G_CollDet::_CollisionResponse_LaserShip( LaserObject *curlaser )
{
	ASSERT( curlaser != NULL );
	ASSERT( cur_ship != NULL );

	int hitpoints = curlaser->HitPoints;
	if ( ( hitpoints -= cur_ship->MegaShieldAbsorption ) < 0 ) {
		hitpoints = 0;
	}

	// apply damage
	if ( cur_ship->CurDamage <= cur_ship->MaxDamage ) {
		cur_ship->CurDamage += hitpoints;
	}

	// check whehter the ship should explode
	if ( cur_ship->CurDamage > cur_ship->MaxDamage ) {

#ifdef PARSEC_CLIENT
		KillDurationWeapons( MyShip );
		OBJ_CreateShipExtras( MyShip );
		NET_SetPlayerKillStat( curlaser->Owner, 1 );
		strcpy( paste_str, killed_by_str );
		strcat( paste_str, NET_FetchPlayerName( curlaser->Owner ) );
		ShowMessage( paste_str );
#else // !PARSEC_CLIENT

		int nClientID_Attacker = curlaser->Owner;
		int nClientID_Downed   = GetOwnerFromHostOjbNumber( cur_ship->HostObjNumber );

		// create the extras this client left
		TheGameExtraManager->OBJ_CreateShipExtras( cur_ship );

		E_SimPlayerInfo* pSimPlayerInfo = TheSimulator->GetSimPlayerInfo( nClientID_Downed ); 

		// maintain stats
		TheGame->RecordKill ( nClientID_Attacker );
		TheGame->RecordDeath( nClientID_Downed, nClientID_Attacker );

		// fill rudimentary RE
		RE_PlayerStatus ps;
		memset( &ps, 0, sizeof ( RE_PlayerStatus ) );
		ps.player_status	= PLAYER_CONNECTED;
		ps.senderid         = nClientID_Downed;
		ps.params[ 0 ]		= SHIP_DOWNED;
		pSimPlayerInfo->PerformUnjoin( &ps );

		// ignore joins from client, until he himself sent an unjoin
		//FIXME: redesign this !!
		pSimPlayerInfo->IgnoreJoinUntilUnjoinFromClient();

		// force a client resync of the downed client
		TheSimulator->GetSimClientState( nClientID_Downed )->SetClientResync();

		//FIXME:
		//cur_ship->pDist->UpdateMODE_REFRESH();

		//FIXME: this should be called GAME_LOGOUT or so to indicate that these messages
		//       are the most important ones for processing user stats
		MSGOUT( "%s was shot down by %s's laser", TheConnManager->GetClientName( nClientID_Downed ),
													TheConnManager->GetClientName( nClientID_Attacker ) );

#endif // !PARSEC_CLIENT
	}


#ifdef PARSEC_CLIENT

	//if ( cur_ship->CurDamage <= cur_ship->MaxDamage ) {
	//	OBJ_EventShipImpact( cur_ship, test_shield );
	//}

	NET_RmEvKillObject( curlaser->HostObjNumber, LASER_LIST );

#else // !PARSEC_CLIENT

	// release the E_Distributable ( distribute removal )
	//MSGOUT( "G_CollDet::_CollisionResponse_LaserShip() calls ReleaseDistributable()" );
	TheSimNetOutput->ReleaseDistributable( curlaser->pDist );
#endif // !PARSEC_CLIENT

	TheWorld->KillSpecificObject( curlaser->ObjectNumber, TheWorld->m_LaserObjects );

/*	if ( cur_ship->ExplosionCount == 0 ) {

#ifdef PARSEC_CLIENT

		//FIXME: pure client gamecode
		OBJ_EventShipImpact( cur_ship, test_shield );

#endif // PARSEC_CLIENT

		if ( !NetConnected ) {
			cur_ship->CurDamage += hitpoints;
			if ( cur_ship->CurDamage > cur_ship->MaxDamage ) {
				LetShipExplode( cur_ship );
				ShowMessage( laser_downed_ship_str );
				AUD_ShipDestroyed( cur_ship );
				AUD_JimCommenting( JIM_COMMENT_EATTHIS );
			}
		}
	}
*/
}
// local ship collided with proximity mine ------------------------------------
//
void G_CollDet::_CollisionResponse_MineShip( Mine1Obj *curmine )
{
	ASSERT( curmine != NULL );

	int cur_ship_owner = GetOwnerFromHostOjbNumber( cur_ship->HostObjNumber );

    
    int hitpoints = curmine->HitPoints;
	if ( ( hitpoints -= cur_ship->MegaShieldAbsorption ) < 0 ) {
		hitpoints = 0;
	}

	if ( cur_ship->CurDamage <= cur_ship->MaxDamage ) {

		cur_ship->CurDamage += hitpoints;
		if ( cur_ship->CurDamage > cur_ship->MaxDamage ) {
             int nClientID_Attacker = curmine->Owner;
		     int nClientID_Downed   = GetOwnerFromHostOjbNumber( cur_ship->HostObjNumber );

		     // create the extras this client left
		     TheGameExtraManager->OBJ_CreateShipExtras( cur_ship );

		     E_SimPlayerInfo* pSimPlayerInfo = TheSimulator->GetSimPlayerInfo( nClientID_Downed ); 

		     if(cur_ship_owner != curmine->Owner) {
                  // maintain stats
		          TheGame->RecordKill ( nClientID_Attacker );
		          TheGame->RecordDeath( nClientID_Downed, nClientID_Attacker );
             }
             TheGame->RecordDeath( nClientID_Downed, nClientID_Attacker );

		     // fill rudimentary RE
		     RE_PlayerStatus ps;
		     memset( &ps, 0, sizeof ( RE_PlayerStatus ) );
		     ps.player_status	= PLAYER_CONNECTED;
		     ps.senderid         = nClientID_Downed;
		     ps.params[ 0 ]		= SHIP_DOWNED;
		     pSimPlayerInfo->PerformUnjoin( &ps );

		     // ignore joins from client, until he himself sent an unjoin
		     //FIXME: redesign this !!
		     pSimPlayerInfo->IgnoreJoinUntilUnjoinFromClient();

		     // force a client resync of the downed client
		     TheSimulator->GetSimClientState( nClientID_Downed )->SetClientResync();

		     //FIXME:
		     //cur_ship->pDist->UpdateMODE_REFRESH();

		     //FIXME: this should be called GAME_LOGOUT or so to indicate that these messages
		     //       are the most important ones for processing user stats
		     MSGOUT( "%s was destroyed by %s's mine", TheConnManager->GetClientName( nClientID_Downed ),
													         TheConnManager->GetClientName( nClientID_Attacker ) );
       
       } 
	} 
    // release the E_Distributable ( distribute removal )
   // MSGOUT( "G_CollDet::_CollisionResponse_MineShip() calls ReleaseDistributable()" );
    curmine->pDist->WillBeSentToOwner();
    TheSimNetOutput->ReleaseDistributable( curmine->pDist );
    TheWorld->KillSpecificObject( curmine->ObjectNumber, TheWorld->m_ExtraObjects );

	return;
}

void G_CollDet::OBJ_ShipHelixDamage( ShipObject *shippo, int owner )
{
	ASSERT( shippo != NULL );
    
	//NOTE:
	// this function is used by
	// PART_ANI::LinearParticleCollision().
    
#define HELIX_HITPOINTS_PER_PARTICLE 1
    
	// no damage at all if megashield active
	if ( shippo->MegaShieldAbsorption > 0 )
		return;
    
    if (shippo->ExplosionCount == 0 ) {
        
		shippo->CurDamage += HELIX_HITPOINTS_PER_PARTICLE;
		if ( shippo->CurDamage > shippo->MaxDamage ) {
            // needed for explosions caused by particles
            shippo->DelayExplosion = 1;
            
            int nClientID_Attacker = owner;
            int nClientID_Downed   = GetOwnerFromHostOjbNumber( shippo->HostObjNumber );
            
            // create the extras this client left
            TheGameExtraManager->OBJ_CreateShipExtras( shippo );
            
            E_SimPlayerInfo* pSimPlayerInfo = TheSimulator->GetSimPlayerInfo( nClientID_Downed ); 
            
            // maintain stats
            TheGame->RecordKill ( nClientID_Attacker );
            TheGame->RecordDeath( nClientID_Downed, nClientID_Attacker );
            
            // fill rudimentary RE
            RE_PlayerStatus ps;
            memset( &ps, 0, sizeof ( RE_PlayerStatus ) );
            ps.player_status	= PLAYER_CONNECTED;
            ps.senderid         = nClientID_Downed;
            ps.params[ 0 ]		= SHIP_DOWNED;
            pSimPlayerInfo->PerformUnjoin( &ps );
            
            // ignore joins from client, until he himself sent an unjoin
            //FIXME: redesign this !!
            pSimPlayerInfo->IgnoreJoinUntilUnjoinFromClient();
            
            // force a client resync of the downed client
            TheSimulator->GetSimClientState( nClientID_Downed )->SetClientResync();
            MSGOUT( "%s was shot down by %s's Helix Cannon", TheConnManager->GetClientName( nClientID_Downed ),TheConnManager->GetClientName( nClientID_Attacker ) );
        }
	}
}

// inflict damage upon ship hit by swarm missiles -----------------------------
//
void G_CollDet::OBJ_ShipSwarmDamage( ShipObject *shippo, int owner )
{
	ASSERT( shippo != NULL );
    
	//NOTE:
	// this function is used by
	// S_SWARM::SWARM_Animate().
    
#define SWARM_HITPOINTS_PER_PARTICLE 1
    
	// no damage at all if megashield active
	if ( shippo->MegaShieldAbsorption > 0 )
		return;
    
	if ( shippo->ExplosionCount == 0  ) {
        
		shippo->CurDamage += SWARM_HITPOINTS_PER_PARTICLE;
		if ( shippo->CurDamage > shippo->MaxDamage ) {
            
			// schedule explosion
            
			// needed for explosions caused by particles
			shippo->DelayExplosion = 1;

            int nClientID_Attacker = owner;
            int nClientID_Downed   = GetOwnerFromHostOjbNumber( shippo->HostObjNumber );
            
            // create the extras this client left
            TheGameExtraManager->OBJ_CreateShipExtras( shippo );
            
            E_SimPlayerInfo* pSimPlayerInfo = TheSimulator->GetSimPlayerInfo( nClientID_Downed );
            
            // maintain stats
            TheGame->RecordKill ( nClientID_Attacker );
            TheGame->RecordDeath( nClientID_Downed, nClientID_Attacker );
            
            // fill rudimentary RE
            RE_PlayerStatus ps;
            memset( &ps, 0, sizeof ( RE_PlayerStatus ) );
            ps.player_status	= PLAYER_CONNECTED;
            ps.senderid         = nClientID_Downed;
            ps.params[ 0 ]		= SHIP_DOWNED;
            pSimPlayerInfo->PerformUnjoin( &ps );
            
            // ignore joins from client, until he himself sent an unjoin
            //FIXME: redesign this !!
            pSimPlayerInfo->IgnoreJoinUntilUnjoinFromClient();
            
            // force a client resync of the downed client
            TheSimulator->GetSimClientState( nClientID_Downed )->SetClientResync();
            MSGOUT( "%s was destroyed by %s's Swarm missiles", TheConnManager->GetClientName( nClientID_Downed ), TheConnManager->GetClientName( nClientID_Attacker ) );
        }
	}
}

// inflict damage upon ship hit by lightning beam -----------------------------
//
void G_CollDet::OBJ_ShipLightningDamage( ShipObject *shippo, int owner )
{
	ASSERT( shippo != NULL );
    
	//NOTE:
	// this function is used by
	// PART_ANI::CheckLightningParticleShipCollision().
    
#define LIGHTNING_HITPOINTS_PER_REFFRAME 6000
    
	// no damage at all if megashield active
	if ( shippo->MegaShieldAbsorption > 0 )
		return;
    
	if (  shippo->ExplosionCount == 0  ) {
        
		shippo->CurDamageFrac += TheSimulator->GetThisFrameRefFrames() * LIGHTNING_HITPOINTS_PER_REFFRAME;
		shippo->CurDamage += shippo->CurDamageFrac >> 16;
		shippo->CurDamageFrac &= 0xffff;
        
		if ( shippo->CurDamage > shippo->MaxDamage ) {
            // needed for explosions caused by particles
            shippo->DelayExplosion = 1;
            
            int nClientID_Attacker = owner;
            int nClientID_Downed   = GetOwnerFromHostOjbNumber( shippo->HostObjNumber );
            
            // create the extras this client left
            TheGameExtraManager->OBJ_CreateShipExtras( shippo );
            
            E_SimPlayerInfo* pSimPlayerInfo = TheSimulator->GetSimPlayerInfo( nClientID_Downed );
            
            // maintain stats
            TheGame->RecordKill ( nClientID_Attacker );
            TheGame->RecordDeath( nClientID_Downed, nClientID_Attacker );
            
            // fill rudimentary RE
            RE_PlayerStatus ps;
            memset( &ps, 0, sizeof ( RE_PlayerStatus ) );
            ps.player_status	= PLAYER_CONNECTED;
            ps.senderid         = nClientID_Downed;
            ps.params[ 0 ]		= SHIP_DOWNED;
            pSimPlayerInfo->PerformUnjoin( &ps );
            
            // ignore joins from client, until he himself sent an unjoin
            //FIXME: redesign this !!
            pSimPlayerInfo->IgnoreJoinUntilUnjoinFromClient();
            
            // force a client resync of the downed client
            TheSimulator->GetSimClientState( nClientID_Downed )->SetClientResync();
            MSGOUT( "%s was cooked %s's Lightning Cannon", TheConnManager->GetClientName( nClientID_Downed ),TheConnManager->GetClientName( nClientID_Attacker ) );
		}
	}
}

// check if ship collected extra ----------------------------------------------
//
void G_CollDet::_CheckShipExtraCollision()
{
	// check whether any (other) shipobject is hit by laser
	for ( cur_ship = TheWorld->FetchFirstShip(); cur_ship != NULL; ) {

		// fetch location of local ship
		geomv_t	objX = cur_ship->ObjPosition[ 0 ][ 3 ];
		geomv_t	objY = cur_ship->ObjPosition[ 1 ][ 3 ];
		geomv_t	objZ = cur_ship->ObjPosition[ 2 ][ 3 ];

		//MSGOUT( "G_CollDet::_CheckShipExtraCollision(): ship %d is at %.2f/%.2f/%.2f", cur_ship->HostObjNumber, objX, objY, objZ );

		// fetch radius of bounding sphere of local ship
		geomv_t	bsphere = cur_ship->BoundingSphere;

		ASSERT( TheWorld->m_ExtraObjects != NULL );

		// walk list of extras
		ExtraObject *precnode = TheWorld->m_ExtraObjects;
		while ( precnode->NextObj != NULL ) {

			ASSERT( OBJECT_TYPE_EXTRA( precnode->NextObj ) );

			// get pointer to current extra
			ExtraObject *curextra = (ExtraObject *) precnode->NextObj;
			ASSERT( curextra != NULL );

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
			if ( dist2 > cur_ship->BoundingSphere2 ) {
				precnode = curextra;
				continue;
			}

			// perform action according to extra type and determine message to show
			char *text = TheGameExtraManager->CollectExtra( cur_ship, curextra );

			// show message saying what type of extra has been collected
			if (( text != NULL ) || ( curextra->ObjectType == MINE1TYPE )) {
                
				//FIXME: implement server->client reliable "got ..." message
				//fgfg

			} else {
                
				MSGOUT( "CheckShipExtraCollision(): invalid extra type: %0x.", curextra->ObjectType );
			}

			// remove extra object
			if( curextra->ObjectType == MINE1TYPE ) //Mine collision
                 _CollisionResponse_MineShip((Mine1Obj *) curextra);     
            else
                 TheGameExtraManager->OBJ_KillExtra( precnode, TRUE );
		}
	
		cur_ship = (ShipObject*)cur_ship->NextObj;
	}
}

// check if point is contained in ship's bounding sphere ----------------------
//
int G_CollDet::PRT_ParticleInBoundingSphere( ShipObject *shippo, Vertex3& point )
{
	ASSERT( shippo != NULL );

	geomv_t objX = shippo->ObjPosition[ 0 ][ 3 ];
	geomv_t objY = shippo->ObjPosition[ 1 ][ 3 ];
	geomv_t objZ = shippo->ObjPosition[ 2 ][ 3 ];
    
	geomv_t bdsphere  = shippo->BoundingSphere;
	geomv_t bdsphere2 = shippo->BoundingSphere2;
    
	if ( point.X < ( objX - bdsphere ) ) return FALSE;
	if ( point.X > ( objX + bdsphere ) ) return FALSE;
	if ( point.Y < ( objY - bdsphere ) ) return FALSE;
	if ( point.Y > ( objY + bdsphere ) ) return FALSE;
	if ( point.Z < ( objZ - bdsphere ) ) return FALSE;
	if ( point.Z > ( objZ + bdsphere ) ) return FALSE;
    
	Vector3 dvec;
	dvec.X = point.X - objX;
	dvec.Y = point.Y - objY;
	dvec.Z = point.Z - objZ;
    
	return ( DOT_PRODUCT( &dvec, &dvec ) < bdsphere2 );
}

// brute force collision detection of linear particle with all ships ----------
//
void G_CollDet::LinearParticleCollision( linear_pcluster_s *cluster, int pid )
{
	ASSERT( cluster != NULL );
	ASSERT( ( pid >= 0 ) && ( pid < cluster->numel ) );
	ASSERT( cluster->rep[ pid ].flags & PARTICLE_COLLISION );
    
	//NOTE:
	// this is set as callback by PART_WFX::MaintainHelix()
	// to detect collisions of helix particles.
    
	int owner = cluster->rep[ pid ].owner;
    
	// check shiplist
	ShipObject *walkships = TheWorld->FetchFirstShip();
	for ( ; walkships; walkships = (ShipObject*) walkships->NextObj ) {
        
		// prevent collision with owner of particle
		if ( ( GetObjectOwner( walkships ) == (dword)owner ) )
			continue;
        
		if ( !PRT_ParticleInBoundingSphere( walkships, cluster->rep[ pid ].position ) )
			continue;
        
		// disable particle
		cluster->rep[ pid ].flags &= ~PARTICLE_ACTIVE;
    	    
              switch ( cluster->rep[ pid ].flags & PARTICLE_IS_MASK ) {
                
                 case PARTICLE_IS_HELIX :
                 //   MSGOUT("G_CollDet::LinearParticleCollision(): Helix particle collision with player %d",GetObjectOwner( walkships ));
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

// inflict damage upon ship hit by photon weapon ------------------------------
//
void G_CollDet::OBJ_ShipPhotonDamage( ShipObject *shippo, int owner )
{
	ASSERT( shippo != NULL );
    
	//NOTE:
	// this function is used by
	// PART_ANI::LinearParticleCollision().
    
#define PHOTON_HITPOINTS_PER_PARTICLE 3
    
	// no damage at all if megashield active
	if ( shippo->MegaShieldAbsorption > 0 )
		return;
    if (shippo->ExplosionCount == 0 ) {
        
		shippo->CurDamage += PHOTON_HITPOINTS_PER_PARTICLE;
		if ( shippo->CurDamage > shippo->MaxDamage ) {
            // needed for explosions caused by particles
            shippo->DelayExplosion = 1;
            
            int nClientID_Attacker = owner;
            int nClientID_Downed   = GetOwnerFromHostOjbNumber( shippo->HostObjNumber );
            
            // create the extras this client left
            TheGameExtraManager->OBJ_CreateShipExtras( shippo );
            
            E_SimPlayerInfo* pSimPlayerInfo = TheSimulator->GetSimPlayerInfo( nClientID_Downed );
            
            // maintain stats
            TheGame->RecordKill ( nClientID_Attacker );
            TheGame->RecordDeath( nClientID_Downed, nClientID_Attacker );
            
            // fill rudimentary RE
            RE_PlayerStatus ps;
            memset( &ps, 0, sizeof ( RE_PlayerStatus ) );
            ps.player_status	= PLAYER_CONNECTED;
            ps.senderid         = nClientID_Downed;
            ps.params[ 0 ]		= SHIP_DOWNED;
            pSimPlayerInfo->PerformUnjoin( &ps );
            
            // ignore joins from client, until he himself sent an unjoin
            //FIXME: redesign this !!
            pSimPlayerInfo->IgnoreJoinUntilUnjoinFromClient();
            
            // force a client resync of the downed client
            TheSimulator->GetSimClientState( nClientID_Downed )->SetClientResync();
            MSGOUT( "%s was mowed down by %s's Photon Cannon", TheConnManager->GetClientName( nClientID_Downed ),TheConnManager->GetClientName( nClientID_Attacker ) );
        }
	}
}


// check if ship entered energy field -----------------------------------------
//
void G_CollDet::CheckEnergyField( pcluster_s *cluster )
{
	if ( ( cluster->type & CT_TYPEMASK ) == CT_PARTICLE_SPHERE ) {
 	     sphereobj_pcluster_s *objcluster = (sphereobj_pcluster_s *) cluster;
	     if ( objcluster->animtype == SAT_ENERGYFIELD_SPHERE ) {
               // check shiplist
               ShipObject *walkships = TheWorld->FetchFirstShip();
               for ( ; walkships; walkships = (ShipObject*) walkships->NextObj ) {
		    if ( PRT_ParticleInBoundingSphere( walkships, objcluster->origin ) ) {
                     TheGameExtraManager->_CollectBoostEnergyField(walkships, TheSimulator->GetThisFrameRefFrames() * 2 );
	           }
               }
	     }
	}
}

int G_CollDet::CheckLightningParticleShipCollision( Vertex3& particlepos, int owner )
{
	// check shiplist
	ShipObject *walkships = TheWorld->FetchFirstShip();
	for ( ; walkships; walkships = (ShipObject*) walkships->NextObj ) {
        
		// prevent collision with owner of particle
		if ( ( GetObjectOwner( walkships ) == (dword)owner ) )
			continue;
        
		if ( !PRT_ParticleInBoundingSphere( walkships, particlepos ) )
			continue;
        
		OBJ_ShipLightningDamage( walkships, owner );
        
		return TRUE;
	}
    
	return FALSE;
}

// perform collision detection tests ------------------------------------------
//
void G_CollDet::OBJ_CheckCollisions()
{
	//NOTE:
	// this function gets called once per frame by the
	// game loop (G_MAIN.C).

	// check if laser beam hits ship
	_CheckShipLaserCollision();

	// check if missile hits ship
	_CheckShipMissileCollision();

	// check if two ships collide
	//CheckShipShipCollision();

	// check if ship collected extra
	_CheckShipExtraCollision();

	// check collisions for custom objects
	//CheckCustomCollisions();

	// check for emp collisions
	_CheckShipEmpCollision();
}

