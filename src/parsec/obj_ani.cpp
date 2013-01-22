/*
 * PARSEC - Object Animation
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:35 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-1999
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

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "net_defs.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "obj_ani.h"

// proprietary module headers
#include "con_aux.h"
#include "g_supp.h"
#include "obj_ctrl.h"
#include "obj_xtra.h"
#include "g_sfx.h"


// flags
//#define DONT_RESET_SHOTCOUNTER



// walk list of laser objects and advance them --------------------------------
//
PRIVATE
void WalkLaserObjects()
{
	ASSERT( LaserObjects != NULL );

	GenObject *precnode  = LaserObjects;
	GenObject *walkshots = LaserObjects->NextObj;

	// walk all lasers
	while ( walkshots != NULL ) {

		ASSERT( OBJECT_TYPE_LASER( walkshots ) );

		LaserObject *laserpo	= (LaserObject *) walkshots;
		laserpo->LifeTimeCount -= CurScreenRefFrames;

		if ( laserpo->LifeTimeCount <= 0 ) {

#ifndef DONT_RESET_SHOTCOUNTER
			NumShots--;
#endif
			NET_RmEvKillObject( laserpo->HostObjNumber, LASER_LIST );

#ifdef LINKED_PROTOCOL_GAMESERVER

#ifdef INTERNAL_VERSION
			if ( NET_ConnectedGMSV() ) {
				ASSERT( FALSE );
				MSGOUT( "OBJ_Ani::WalkLaserObjects(): predicted freeing of laser object" );
			}
#endif // INTERNAL_VERSION

#endif // LINKED_PROTOCOL_GAMESERVER
			ASSERT( walkshots != NULL );
			precnode->NextObj = walkshots->NextObj;
			FreeObjectMem( walkshots );
			walkshots = precnode->NextObj;
			continue;

		} else {

			Vector3 tempspeed;
			tempspeed.X = laserpo->DirectionVec.X * CurScreenRefFrames;
			tempspeed.Y = laserpo->DirectionVec.Y * CurScreenRefFrames;
			tempspeed.Z = laserpo->DirectionVec.Z * CurScreenRefFrames;

			laserpo->PrevPosition.X = laserpo->ObjPosition[ 0 ][ 3 ];
			laserpo->PrevPosition.Y = laserpo->ObjPosition[ 1 ][ 3 ];
			laserpo->PrevPosition.Z = laserpo->ObjPosition[ 2 ][ 3 ];

			laserpo->ObjPosition[ 0 ][ 3 ] += tempspeed.X;
			laserpo->ObjPosition[ 1 ][ 3 ] += tempspeed.Y;
			laserpo->ObjPosition[ 2 ][ 3 ] += tempspeed.Z;
		}

		precnode  = walkshots;
		walkshots = walkshots->NextObj;
	}
}


// perform animation for dumb missiles ----------------------------------------
//
PRIVATE
void AnimateStandardMissile( MissileObject *missilepo )
{
	ASSERT( missilepo != NULL );

	Vector3 tempspeed;
	tempspeed.X = missilepo->DirectionVec.X * CurScreenRefFrames;
	tempspeed.Y = missilepo->DirectionVec.Y * CurScreenRefFrames;
	tempspeed.Z = missilepo->DirectionVec.Z * CurScreenRefFrames;

	missilepo->PrevPosition.X = missilepo->ObjPosition[ 0 ][ 3 ];
	missilepo->PrevPosition.Y = missilepo->ObjPosition[ 1 ][ 3 ];
	missilepo->PrevPosition.Z = missilepo->ObjPosition[ 2 ][ 3 ];

	missilepo->ObjPosition[ 0 ][ 3 ] += tempspeed.X;
	missilepo->ObjPosition[ 1 ][ 3 ] += tempspeed.Y;
	missilepo->ObjPosition[ 2 ][ 3 ] += tempspeed.Z;
}


// perform animation for homing missiles -------------------------------------
//
PRIVATE
void AnimateHomingMissile( MissileObject *missilepo )
{
	ASSERT( missilepo != NULL );

	Vector3 tempspeed;
	tempspeed.X = missilepo->DirectionVec.X * CurScreenRefFrames;
	tempspeed.Y = missilepo->DirectionVec.Y * CurScreenRefFrames;
	tempspeed.Z = missilepo->DirectionVec.Z * CurScreenRefFrames;

	missilepo->PrevPosition.X = missilepo->ObjPosition[ 0 ][ 3 ];
	missilepo->PrevPosition.Y = missilepo->ObjPosition[ 1 ][ 3 ];
	missilepo->PrevPosition.Z = missilepo->ObjPosition[ 2 ][ 3 ];

	missilepo->ObjPosition[ 0 ][ 3 ] += tempspeed.X;
	missilepo->ObjPosition[ 1 ][ 3 ] += tempspeed.Y;
	missilepo->ObjPosition[ 2 ][ 3 ] += tempspeed.Z;

	TargetMissileObject	*tmissilepo	= (TargetMissileObject *) missilepo;
	GenObject			*targetpo	= NULL;

	if ( tmissilepo->TargetObjNumber == TARGETID_NO_TARGET ) {

		// no target (already lost)
		targetpo = NULL;

	} else if ( tmissilepo->TargetObjNumber == ShipHostObjId( LocalPlayerId ) ) {

		// local player is target
		targetpo = MyShip;

		// announce incoming missile
		#define INCOMING_SHOWTIME	600
		IncomingMissile = INCOMING_SHOWTIME;

		// start incoming sample looping
		AUD_IncomingMissile( INCOMING_SHOWTIME );

	} else {

		// search for target in shiplist
		targetpo = FetchFirstShip();
		while ( ( targetpo != NULL ) &&
				( targetpo->HostObjNumber != tmissilepo->TargetObjNumber ) ) {
			targetpo = targetpo->NextObj;
		}
		if ( targetpo == NULL ) {
			// missile loses target once object not found
			tmissilepo->TargetObjNumber = TARGETID_NO_TARGET;
		}
	}

	if ( ( targetpo != NULL ) ) {

		Vector3 dirvec;
		dirvec.X = targetpo->ObjPosition[ 0 ][ 3 ] - missilepo->ObjPosition[ 0 ][ 3 ];
		dirvec.Y = targetpo->ObjPosition[ 1 ][ 3 ] - missilepo->ObjPosition[ 1 ][ 3 ];
		dirvec.Z = targetpo->ObjPosition[ 2 ][ 3 ] - missilepo->ObjPosition[ 2 ][ 3 ];

		Vector3 normvec;
		normvec.X = missilepo->ObjPosition[ 0 ][ 2 ];
		normvec.Y = missilepo->ObjPosition[ 1 ][ 2 ];
		normvec.Z = missilepo->ObjPosition[ 2 ][ 2 ];

		if ( DOT_PRODUCT( &dirvec, &normvec ) < 0 ) {

			// lock lost due to position
			tmissilepo->TargetObjNumber = TARGETID_NO_TARGET;

		} else {

			normvec.X = missilepo->ObjPosition[ 0 ][ 1 ];
			normvec.Y = missilepo->ObjPosition[ 1 ][ 1 ];
			normvec.Z = missilepo->ObjPosition[ 2 ][ 1 ];
			if ( DOT_PRODUCT( &dirvec, &normvec ) <= 0 ) {
				ObjRotX( missilepo->ObjPosition, tmissilepo->MaxRotation );
			} else {
				ObjRotX( missilepo->ObjPosition, -tmissilepo->MaxRotation );
			}

			normvec.X = missilepo->ObjPosition[ 0 ][ 0 ];
			normvec.Y = missilepo->ObjPosition[ 1 ][ 0 ];
			normvec.Z = missilepo->ObjPosition[ 2 ][ 0 ];
			if ( DOT_PRODUCT( &dirvec, &normvec ) <= 0 ) {
				ObjRotY( missilepo->ObjPosition, -tmissilepo->MaxRotation );
			} else {
				ObjRotY( missilepo->ObjPosition, tmissilepo->MaxRotation );
			}
		}

		DirVctMUL( missilepo->ObjPosition, FIXED_TO_GEOMV( missilepo->Speed ), &missilepo->DirectionVec );
	}
}


// walk list of missile objects and advance them ------------------------------
//
PRIVATE
void WalkMissileObjects()
{
	ASSERT( MisslObjects != NULL );

	GenObject *precnode  = MisslObjects;
	GenObject *walkshots = MisslObjects->NextObj;

	// walk entire list
	while ( walkshots != NULL ) {

		ASSERT( OBJECT_TYPE_MISSILE( walkshots ) );

		MissileObject *missilepo  = (MissileObject *) walkshots;
		missilepo->LifeTimeCount -= CurScreenRefFrames;

		if ( missilepo->LifeTimeCount <= 0 ) {

			// delete missile
			NumMissiles--;
			NET_RmEvKillObject( missilepo->HostObjNumber, MISSL_LIST );

			ASSERT( walkshots != NULL );
			precnode->NextObj = walkshots->NextObj;
			FreeObjectMem( walkshots );
			walkshots = precnode->NextObj;
			continue;

		} else {

			// switch on animation type
			switch ( missilepo->ObjectType & TYPECONTROLMASK ) {

				case TYPEMISSILEISSTANDARD:
					// create propulsion particles
					if ( AUX_ENABLE_MISSILE_PROPULSION_FUMES & 0x01 )
						SFX_MissilePropulsion( missilepo );
					// advance missile position
					AnimateStandardMissile( missilepo );
					break;

				case TYPEMISSILEISHOMING:
					// create propulsion particles
					if ( AUX_ENABLE_MISSILE_PROPULSION_FUMES & 0x02 )
						SFX_MissilePropulsion( missilepo );
					// advance missile position
					AnimateHomingMissile( missilepo );
					break;
			}
		}

		precnode  = walkshots;
		walkshots = walkshots->NextObj;
	}
}


// animate projectile objects (lasers and missiles) ---------------------------
//
void OBJ_AnimateProjectiles()
{
	//NOTE:
	// this function gets called once per
	// frame by the game loop (G_MAIN.C).

	WalkLaserObjects();
	WalkMissileObjects();
}


// animate custom objects by calling their animation callback -----------------
//
void OBJ_AnimateCustomObjects()
{
	//NOTE:
	// this function gets called once per
	// frame by the game loop (G_MAIN.C).

	ASSERT( CustmObjects != NULL );

	GenObject *precnode   = CustmObjects;
	GenObject *walkcustom = CustmObjects->NextObj;

	// walk entire list
	while ( walkcustom != NULL ) {

		ASSERT( OBJECT_TYPE_CUSTOM( walkcustom ) );

		CustomObject *custompo = (CustomObject *) walkcustom;
		if ( custompo->callback_animate != NULL ) {

			// call animation function
			if ( (*custompo->callback_animate)( custompo ) == FALSE ) {

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


// animate extra objects (only needed in entry mode/floating menu) ------------
//
void OBJ_BackgroundAnimateExtras()
{
	//NOTE:
	// this function is only used in entry mode and floating menu
	// to animate the extras when no other animations are performed.
	// in game mode these tasks are performed by
	// OBJ_COLL::CheckShipExtraCollision() to scan the extras list
	// only once.

	ASSERT( EntryMode || InFloatingMenu );
	ASSERT( ExtraObjects != NULL );

	ExtraObject *precnode = ExtraObjects;
	while ( precnode->NextObj != NULL ) {

		ASSERT( OBJECT_TYPE_EXTRA( precnode->NextObj ) );

		// get pointer to current extra
		ExtraObject *curextra = (ExtraObject *) precnode->NextObj;
		ASSERT( curextra != NULL );

		// check if lifetime of extra is spent
		curextra->LifeTimeCount -= CurScreenRefFrames;
		if ( curextra->LifeTimeCount < 0 ) {

#ifdef INTERNAL_VERSION
			if ( NET_ConnectedGMSV() ) {
				//ASSERT( FALSE );
				MSGOUT( "OBJ_ANI::OBJ_BackgroundAnimateExtras(): predicted freeing of extra object" );
			}
#endif // INTERNAL_VERSION

			OBJ_KillExtra( precnode, FALSE );
			continue;
		}

		// animate this extra
		OBJ_AnimateExtra( curextra );

		// advance to next extra in list
		precnode = curextra;
	}
}


// perform self running ship actions ------------------------------------------
//
void OBJ_PerformShipActions()
{
	//NOTE:
	// this function gets called once per
	// frame by the game loop (G_MAIN.C).

	ASSERT( GAME_MODE_ACTIVE() );

	// walk all ship objects
	ShipObject *scanpo = FetchFirstShip();
	for ( ; scanpo; scanpo = (ShipObject *) scanpo->NextObj ) {

		ASSERT( OBJECT_TYPE_SHIP( scanpo ) );

		// perform bouncing for non-local ship
		if ( scanpo->BounceCount > 0 ) {
			scanpo->ObjPosition[ 0 ][ 3 ] += scanpo->BounceVec.X * CurScreenRefFrames;
			scanpo->ObjPosition[ 1 ][ 3 ] += scanpo->BounceVec.Y * CurScreenRefFrames;
			scanpo->ObjPosition[ 2 ][ 3 ] += scanpo->BounceVec.Z * CurScreenRefFrames;
			scanpo->BounceCount -= CurScreenRefFrames;
		}
	}

	// perform bouncing for local ship
	if ( MyShip->BounceCount > 0 ) {

		Vector3 mybounce;
		mybounce.X = MyShip->BounceVec.X * CurScreenRefFrames;
		mybounce.Y = MyShip->BounceVec.Y * CurScreenRefFrames;
		mybounce.Z = MyShip->BounceVec.Z * CurScreenRefFrames;

		ShipViewCamera[ 0 ][ 3 ] -= mybounce.X;
		ShipViewCamera[ 1 ][ 3 ] -= mybounce.Y;
		ShipViewCamera[ 2 ][ 3 ] -= mybounce.Z;

		if ( !ObjCameraActive ) {

			PseudoStarMovement[ 0 ][ 3 ] -= mybounce.X;
			PseudoStarMovement[ 1 ][ 3 ] -= mybounce.Y;
			PseudoStarMovement[ 2 ][ 3 ] -= mybounce.Z;

		} else {

			Vector3 dirvec;
			MtxVctMULt( ObjectCamera, &mybounce, &dirvec );
			PseudoStarMovement[ 0 ][ 3 ] -= dirvec.X;
			PseudoStarMovement[ 1 ][ 3 ] -= dirvec.Y;
			PseudoStarMovement[ 2 ][ 3 ] -= dirvec.Z;
		}

		MyShip->BounceCount -= CurScreenRefFrames;
	}
}



