/*
 * PARSEC - Supporting Game Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:35 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
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

// subsystem headers
#include "aud_defs.h"
#include "net_defs.h"
#include "sys_defs.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "obj_game.h"

// proprietary module headers
#include "aud_game.h"
#include "con_arg.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_main.h"
#include "e_record.h"
#include "e_shader.h"
#include "h_supp.h"
#include "obj_ctrl.h"
#include "parttype.h"
#include "part_api.h"
#include "g_sfx.h"
#include "g_swarm.h"


// flags
#define SHIPBOUNDED_MINE_PLACEMENT



// message strings ------------------------------------------------------------
//
static char low_energy_str[]				= "low energy";
static char no_dumb_str[]		        	= "no dumb missiles";
static char no_homing_str[] 	        	= "no guided missiles";
static char no_swarm_str[] 		        	= "no swarm missiles";
static char no_target_str[] 	        	= "no target locked";
static char no_target_sel_str[] 	       	= "no target selected";
static char no_mines_str[]		        	= "no proximity mines";
static char mine_released_str[]         	= "mine released";
static char no_standard_str[]	        	= "no standard laser";
static char got_energy_str[]	        	= "energy boosted";
static char max_energy_str[]	        	= "energy maxed out";
static char no_decoy_str[]					= "no holo decoy device";
static char decoy_activated_str[]			= "holo decoy activated";
static char no_invisibility_str[]			= "no invisibility device";
static char invisibility_activated_str[]	= "invisibility activated";


// check availability of specified device -------------------------------------
//
int OBJ_DeviceAvailable( ShipObject *shippo, int mask )
{
	ASSERT( shippo != NULL );

	if ( AUX_CHEAT_DISABLE_DEVICE_CHECKS )
		return TRUE;

	return ( ( shippo->Weapons & mask ) != 0 );
}


// sequence of gun (laser) barrels --------------------------------------------
//
#define MAX_GUN_BARRELS_MASK		0x0003
static int gun_barrels_sequence[]	= { 0, 3, 1, 2 };


// create actual laser object -------------------------------------------------
//
PRIVATE
LaserObject *OBJm_CreateLaserObject( ShipObject *shippo, int curlevel, int barrel )
{
	ASSERT( shippo != NULL );

	dword laserclass = shippo->Laser1_Class[ curlevel ][ barrel ];

	// create launch matrix
	Xmatrx startm;
	MakeIdMatrx( startm );
	startm[ 0 ][ 3 ] = shippo->Laser1_X[ curlevel ][ barrel ];
	startm[ 1 ][ 3 ] = shippo->Laser1_Y[ curlevel ][ barrel ];
	startm[ 2 ][ 3 ] = shippo->Laser1_Z[ curlevel ][ barrel ];

	// create laser object and calculate position and direction vector
	MtxMtxMUL( shippo->ObjPosition, startm, DestXmatrx );
	LaserObject *laserpo = (LaserObject *) CreateObject( laserclass, DestXmatrx );
	ASSERT( laserpo != NULL );
	laserpo->Speed += shippo->CurSpeed;
	DirVctMUL( laserpo->ObjPosition, FIXED_TO_GEOMV( laserpo->Speed ), &laserpo->DirectionVec );
	laserpo->Owner = OWNER_LOCAL_PLAYER;

	if ( ++NumShots > MaxNumShots ) {
		MaxNumShots = NumShots;
	}

	// insert remote event
	NET_RmEvLaser( laserpo );

	// record create event if recording active
	Record_LaserCreation( laserpo );

	return laserpo;
}


// create laser originating from specified position ---------------------------
//
void OBJ_ShootLaser( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	//NOTE:
	// this function is called by
	// INP_USER::User_CheckGunFire().

	#define MIN_LASER_ENERGY		20

	// check if enough space in RE_List
	if ( !NET_RmEvAllowed( RE_CREATELASER ) )
		return;

	// check for availability
	if ( !OBJ_DeviceAvailable( shippo, WPMASK_CANNON_LASER ) ) {
		if ( shippo == MyShip ) {
			ShowMessage( no_standard_str );
		}
		return;
	}

	// determine current laser upgrade level
	int curlevel = 0;
	if ( shippo->Specials & SPMASK_LASER_UPGRADE_2 ) {
		curlevel = 2;
	} else if ( shippo->Specials & SPMASK_LASER_UPGRADE_1 ) {
		curlevel = 1;
	}

	int barrel = gun_barrels_sequence[ CurGun & MAX_GUN_BARRELS_MASK ];
	dword laserclass = shippo->Laser1_Class[ curlevel ][ barrel ];

	if ( !AUX_CHEAT_DISABLE_ENERGY_CHECKS ) {

		// check if enough energy to shoot laser
		int energyneeded = ((LaserObject*)ObjClasses[ laserclass ])->EnergyNeeded;
		if ( shippo->CurEnergy < MIN_LASER_ENERGY + energyneeded ) {
			if ( shippo == MyShip ) {
				ShowMessage( low_energy_str );
				AUD_LowEnergy();
			}
			return;
		}
		shippo->CurEnergy -= energyneeded;
		//MSGOUT( "G_PlayerInfo::_OBJ_ShootLaser(): energy after shot: %d", shippo->CurEnergy );
	}

	// create actual laser object(s)
	LaserObject *laserpo = OBJm_CreateLaserObject( shippo, curlevel, barrel );
	CurGun++;

	if ( curlevel == 2 ) {
		barrel = gun_barrels_sequence[ CurGun & MAX_GUN_BARRELS_MASK ];
		OBJm_CreateLaserObject( shippo, curlevel, barrel );
		CurGun++;
	}

	// play sound effect
	AUD_Laser( laserpo );
}


// sequence of missile barrels ------------------------------------------------
//
#define MAX_MISSILE_BARRELS_MASK		0x0003
static int missile_barrel_sequence[]	= { 0, 3, 1, 2 };


// create missile originating from specified position -------------------------
//
void OBJ_LaunchMissile( ShipObject *shippo, dword launcher )
{
	ASSERT( shippo != NULL );

	//NOTE:
	// this function is called by
	// INP_USER::User_CheckMissileLaunch().

	// check if enough space in RE_List
	if ( !NET_RmEvAllowed( RE_CREATEMISSILE ) )
		return;

	if ( !AUX_CHEAT_DISABLE_AMMO_CHECKS ) {

		// check ammo
		if ( shippo->NumMissls <= 0 ) {
			if ( shippo == MyShip )
				ShowMessage( no_dumb_str );
			return;
		}
		shippo->NumMissls--;

		// play the voice sample for the missile countdown
		AUD_Countdown( shippo->NumMissls );
	}

	// select correct barrel
	int barrel       = missile_barrel_sequence[ launcher & MAX_MISSILE_BARRELS_MASK ];
	int missileclass = shippo->Missile1_Class[ barrel ];

	// create launch matrix
	Xmatrx startm;
	MakeIdMatrx( startm );
	startm[ 0 ][ 3 ] = shippo->Missile1_X[ barrel ];
	startm[ 1 ][ 3 ] = shippo->Missile1_Y[ barrel ];
	startm[ 2 ][ 3 ] = shippo->Missile1_Z[ barrel ];

	// create missile object and calculate position and direction vector
	MtxMtxMUL( shippo->ObjPosition, startm, DestXmatrx );
	MissileObject *missilepo = (MissileObject *) CreateObject( missileclass, DestXmatrx );
	ASSERT( missilepo != NULL );
	missilepo->Speed += shippo->CurSpeed;
	DirVctMUL( missilepo->ObjPosition, FIXED_TO_GEOMV( missilepo->Speed ), &missilepo->DirectionVec );
	missilepo->Owner = OWNER_LOCAL_PLAYER;

	if ( ++NumMissiles > MaxNumMissiles )
		MaxNumMissiles = NumMissiles;

	// insert remote event
	NET_RmEvMissile( missilepo, 0 );

	// record create event if recording active
	Record_MissileCreation( missilepo );

	// play sound effect
	AUD_Missile( missilepo );
}


// create homing missile originating from specified position ------------------
//
void OBJ_LaunchHomingMissile( ShipObject *shippo, dword launcher, dword targetid )
{
	ASSERT( shippo != NULL );

	//NOTE:
	// this function is called by
	// INP_USER::User_CheckMissileLaunch().

	// check if enough space in RE_List
	if ( !NET_RmEvAllowed( RE_CREATEMISSILE ) )
		return;

	if ( !AUX_CHEAT_DISABLE_AMMO_CHECKS ) {

		// check ammo
		if ( shippo->NumHomMissls <= 0 ) {
			if ( shippo == MyShip )
				ShowMessage( no_homing_str );
			return;
		}
	}

	// check if any target locked
	if ( !TargetLocked && ( shippo == MyShip ) ) {
		ShowMessage( no_target_str );
		return;
	}

	if ( !AUX_CHEAT_DISABLE_AMMO_CHECKS ) {

		shippo->NumHomMissls--;

		// play the voice sample for the missile countdown
		AUD_Countdown( shippo->NumHomMissls );
	}

	// select correct barrel
	int barrel       = missile_barrel_sequence[ launcher & MAX_MISSILE_BARRELS_MASK ];
	int missileclass = shippo->Missile2_Class[ barrel ];

	// create launch matrix
	Xmatrx startm;
	MakeIdMatrx( startm );
	startm[ 0 ][ 3 ] = shippo->Missile2_X[ barrel ];
	startm[ 1 ][ 3 ] = shippo->Missile2_Y[ barrel ];
	startm[ 2 ][ 3 ] = shippo->Missile2_Z[ barrel ];

	// create missile object and calculate position and direction vector
	MtxMtxMUL( shippo->ObjPosition, startm, DestXmatrx );
	TargetMissileObject *missilepo = (TargetMissileObject *) CreateObject( missileclass, DestXmatrx );
	ASSERT( missilepo != NULL );
	missilepo->Speed += shippo->CurSpeed;
	DirVctMUL( missilepo->ObjPosition, FIXED_TO_GEOMV( missilepo->Speed ), &missilepo->DirectionVec );
	missilepo->Owner		   = OWNER_LOCAL_PLAYER;
	missilepo->TargetObjNumber = targetid;

	if ( ++NumMissiles > MaxNumMissiles )
		MaxNumMissiles = NumMissiles;

	if ( NetConnected ) {

		dword targethostobjid = targetid;
		GenObject *targetpo	  = FetchFirstShip();

		// search for target with current TargetObjNumber in list of ships
		while ( targetpo && ( targetpo->HostObjNumber != targetid ) )
			targetpo = targetpo->NextObj;

		// if target doesn't exist anymore, use no target
		if ( targetpo == NULL )
			targethostobjid = TARGETID_NO_TARGET;

		// missile remote event
		NET_RmEvMissile( (MissileObject*)missilepo, targethostobjid );
	}

	// record create event if recording active
	Record_TargetMissileCreation( missilepo );

	// play sound effect
	AUD_Missile( missilepo );
}


// create mine object ---------------------------------------------------------
//
void OBJ_LaunchMine( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	//NOTE:
	// this function is called by
	// INP_USER::User_CheckMissileLaunch().

	// check if enough space in RE_List
	if ( !NET_RmEvAllowed( RE_CREATEMINE ) )
		return;

	if ( !AUX_CHEAT_DISABLE_AMMO_CHECKS ) {

		// check ammo
		if ( shippo->NumMines <= 0 ) {
			if ( shippo == MyShip )
				ShowMessage( no_mines_str );
			return;
		}
		shippo->NumMines--;

		// play the voice sample for the mine countdown
		AUD_Countdown( shippo->NumMines );
	}

	// create launch matrix
	Xmatrx startm;
	MakeIdMatrx( startm );

//#ifdef SHIPBOUNDED_MINE_PLACEMENT
	startm[ 0 ][ 3 ] = GEOMV_0;
	startm[ 1 ][ 3 ] = GEOMV_0;
	startm[ 2 ][ 3 ] = -( shippo->BoundingSphere + GEOMV_1 );
//#else
//	startm[ 0 ][ 3 ] = shippo->Mine1_X;
//	startm[ 1 ][ 3 ] = shippo->Mine1_Y;
//	startm[ 2 ][ 3 ] = shippo->Mine1_Z;
//#endif

	// create mine object
	MtxMtxMUL( shippo->ObjPosition, startm, DestXmatrx );
	MineObject *minepo = (MineObject *) CreateObject( MINE_CLASS_1, DestXmatrx );
        
	ASSERT( minepo != NULL );

        minepo->Owner = OWNER_LOCAL_PLAYER;

	// insert remote event
	NET_RmEvMine( (ExtraObject *) minepo );

	// record create event if recording active
	Record_MineCreation( minepo );

	// play sound effect
	AUD_Mine( (GenObject *) shippo );
	if ( shippo == MyShip ) {
		ShowMessage( mine_released_str );
	}
}


// create swarm missiles ------------------------------------------------------
//
void OBJ_LaunchSwarmMissiles( ShipObject *shippo, dword targetid )
{
	ASSERT( shippo != NULL );

	//NOTE:
	// this function is called by
	// INP_USER::User_CheckMissileLaunch().

	// check if enough space in RE_List
	if ( !NET_RmEvAllowed( RE_CREATESWARM ) )
		return;

	if ( !AUX_CHEAT_DISABLE_AMMO_CHECKS ) {

		// check ammo
		if ( shippo->NumPartMissls <= 0 ) {
			if ( shippo == MyShip )
				ShowMessage( no_swarm_str );
			return;
		}
	}

	// check if any target locked
	if ( !TargetLocked && ( shippo == MyShip ) ) {
		ShowMessage( no_target_str );
		return;
	}

	// fetch target from ship list
	ShipObject *targetpo = (ShipObject *) FetchFirstShip();
	while ( targetpo && ( targetpo->HostObjNumber != TargetObjNumber ) )
		targetpo = (ShipObject *) targetpo->NextObj;

	if ( targetpo == NULL ) {
		ShowMessage( no_target_sel_str );
		return;
	}

	if ( !AUX_CHEAT_DISABLE_AMMO_CHECKS ) {

		shippo->NumPartMissls--;

		// play the voice sample for the missile countdown
		AUD_Countdown( shippo->NumPartMissls );
	}

	dword randseed = SYSs_GetRefFrameCount();

	Vertex3 origin;
	origin.X = shippo->ObjPosition[ 0 ][ 3 ];
	origin.Y = shippo->ObjPosition[ 1 ][ 3 ];
	origin.Z = shippo->ObjPosition[ 2 ][ 3 ];

	GenObject *dummyobj = SWARM_Init( LocalPlayerId, &origin, targetpo, randseed );

	if ( NetConnected ) {

		dword targethostobjid = targetid;
		GenObject *targetpo	  = FetchFirstShip();

		// search for target with current TargetObjNumber in list of ships
		while ( targetpo && ( targetpo->HostObjNumber != targetid ) )
			targetpo = targetpo->NextObj;

		// if target doesn't exist anymore, use no target
		if ( targetpo == NULL )
			targethostobjid = TARGETID_NO_TARGET;

		// swarm remote event
		NET_RmEvCreateSwarm( &origin, targethostobjid, randseed );
	}

	//TODO:
	//	Record_SwarmMissilesCreation( ... );

	AUD_SwarmMissiles( &origin, dummyobj );
}


// enable invisibility function -----------------------------------------------
//
void OBJ_EnableInvisibility( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	//NOTE:
	// this function is used by
	// OBJ_COLL::CheckShipExtraCollision().

	// set active flag in ship structure
	shippo->Specials |= SPMASK_INVISIBILITY;

	//TODO:
	// well, nobody got around to implementing this :)
}


// enable invulnerability function --------------------------------------------
//
void OBJ_EnableInvulnerability( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	//NOTE:
	// this function is used by
	// OBJ_COLL::CheckShipExtraCollision().

	// create particle cluster visualizing invulnerability shield
	if ( SFX_EnableInvulnerabilityShield( shippo ) ) {

		// set active flag in ship structure
		shippo->Specials |= SPMASK_INVULNERABILITY;

		// set shield strength
		shippo->MegaShieldAbsorption = MegaShieldStrength;
	}
}


// enable decoy device --------------------------------------------------------
//
void OBJ_EnableDecoy( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	// set active flag in ship structure
	shippo->Specials |= SPMASK_DECOY;
}


// enable laser upgrade 1  ----------------------------------------------------
//
void OBJ_EnableLaserUpgrade1( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	// set active flag in ship structure
	shippo->Specials |= SPMASK_LASER_UPGRADE_1;
}


// enable laser upgrade 2  ----------------------------------------------------
//
void OBJ_EnableLaserUpgrade2( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	// set active flag in ship structure
	shippo->Specials |= SPMASK_LASER_UPGRADE_2;
}


// enable emp upgrade 1  ------------------------------------------------------
//
void OBJ_EnableEmpUpgrade1( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	// set active flag in ship structure
	shippo->Specials |= SPMASK_EMP_UPGRADE_1;
}


// enable emp upgrade 2  ------------------------------------------------------
//
void OBJ_EnableEmpUpgrade2( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	// set active flag in ship structure
	shippo->Specials |= SPMASK_EMP_UPGRADE_2;
}


// boost a ship's energy ------------------------------------------------------
//
char *OBJ_BoostEnergy( ShipObject *shippo, int boost )
{
	ASSERT( shippo != NULL );
	//ASSERT( !NET_ConnectedGMSV() );

	//NOTE:
	// this function is used by
	// PART_SYS::CheckEnergyField() and
	// OBJ_COLL::CheckShipExtraCollision().

	if ( shippo->CurEnergy == shippo->MaxEnergy ) {

		if ( shippo == MyShip ) {
			AUD_MaxedOut( ENERGY_EXTRA_CLASS );
		}

		return max_energy_str;

	} else {

		shippo->CurEnergy += boost;
		if ( shippo->CurEnergy > shippo->MaxEnergy )
			shippo->CurEnergy = shippo->MaxEnergy;

		if ( shippo == MyShip ) {
			AUD_EnergyBoosted();
			AUD_JimCommenting( JIM_COMMENT_ENERGY );
		}

		return got_energy_str;
	}
}


// ----------------------------------------------------------------------------
//
/*static int afterburner_previous_speed	= 0;
PUBLIC int afterburner_active 			= FALSE;
PUBLIC int afterburner_energy			= AFTERBURNER_ENERGY;
*/

// enable afterburner function ------------------------------------------------
//
void OBJ_EnableAfterBurner( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	// set active flag in ship structure
	shippo->Specials |= SPMASK_AFTERBURNER;

	shippo->afterburner_energy = AFTERBURNER_ENERGY;
}


// disable afterburner function -----------------------------------------------
//
void OBJ_DisableAfterBurner( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	OBJ_DeactivateAfterBurner( shippo );

	// reset active flag in ship structure
	shippo->Specials &= ~SPMASK_AFTERBURNER;

	shippo->afterburner_energy = 0;
}


// activate afterburner -------------------------------------------------------
//
void OBJ_ActivateAfterBurner( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	if ( !shippo->afterburner_active && ( shippo->Specials & SPMASK_AFTERBURNER ) ) {
		shippo->afterburner_active = TRUE;

		//FIXME: use variable per object
		shippo->afterburner_previous_speed = shippo->CurSpeed;
		shippo->CurSpeed = AFTERBURNER_SPEED;
		AUD_ActivateAfterBurner();
	}
	
}


// deactivate afterburner -----------------------------------------------------
//
void OBJ_DeactivateAfterBurner( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	if ( shippo->afterburner_active ) {
		shippo->afterburner_active = FALSE;
		//FIXME: use variable per object
		shippo->CurSpeed = shippo->afterburner_previous_speed;
		shippo->afterburner_previous_speed = 0;
		AUD_DeactivateAfterBurner();
	}
	
}


// ship impact event has occurred ---------------------------------------------
//
void OBJ_EventShipImpact( ShipObject *shippo, int shieldhit )
{
	ASSERT( shippo != NULL );
/*
	// exit if ship is already exploding
	if ( shippo->ExplosionCount > 0 ) {
		return;
	}
*/
	#define TARGET_IMPACT_DISPLAY_DURATION	5

	// for cockpit display of target impact
	if ( shippo != MyShip ) {
		if ( shippo->HostObjNumber == TargetObjNumber ) {
			HitCurTarget = TARGET_IMPACT_DISPLAY_DURATION;
		}
	}

	if ( shieldhit ) {

		if ( shippo == MyShip ) {

			// play "playershield hit" sample
			AUD_PlayerShieldHit();
			AUD_JimCommenting( JIM_COMMENT_ANGRY );

		} else {

			// play "enemyshield hit" sample
			AUD_EnemyShieldHit();
		}

		SFX_FlashProtectiveShield( shippo );

	} else {

		if ( shippo == MyShip ) {

			// play "playerhull hit" sample
			AUD_PlayerHullHit();
			AUD_JimCommenting( JIM_COMMENT_ANGRY );

		} else {

			// play "enemyhull hit" sample
			AUD_EnemyHullHit();
		}
	}
	
	// play "low shields" sample
	if ( ( shippo == MyShip ) && 
		 ( ( MyShip->MaxDamage - MyShip->CurDamage ) <= 8 ) &&
		 !PRT_ObjectHasAttachedClustersOfType( shippo, SAT_MEGASHIELD_SPHERE ) ) {
		 
		AUD_LowShields();
	}
}


// console command for activating the holo decoy object -----------------------
//
PRIVATE
int Cmd_DECOY( char *argstr )
{
	//NOTE:
	//CONCOM:
	// decoy_command	::= 'decoy'

	ASSERT( argstr != NULL );
	HANDLE_COMMAND_DOMAIN( argstr );

	// check if enough space in RE_List
	if ( !NET_RmEvAllowed( RE_CREATEOBJECT ) )
		return TRUE;

	if ( ( MyShip->Specials & SPMASK_DECOY ) == 0 ) {
		ShowMessage( no_decoy_str );
		return TRUE;
	}

	Xmatrx startm;
	MakeIdMatrx( startm );

	// initial position of holo decoy object
	startm[ 0 ][ 3 ] = GEOMV_0;
	startm[ 1 ][ 3 ] = GEOMV_0;
	startm[ 2 ][ 3 ] = -( 2 * ( MyShip->BoundingSphere ) + GEOMV_1 );

	// create holo decoy object
	MtxMtxMUL( MyShip->ObjPosition, startm, DestXmatrx );
	GenObject *obj = SummonObject( MyShip->ObjectClass, DestXmatrx );
	ASSERT( obj != NULL );

	// attach dynamically generated shader to object
	// ...

	// insert remote event
	NET_RmEvObject( obj );

	// schedule event for holo decoy destruction
	// ...

	ShowMessage( decoy_activated_str );

	// lose device after it has been activated
	MyShip->Specials &= ~SPMASK_DECOY;

	return TRUE;
}


// console command for activating the invisibility (cloaking) device) ---------
//
PRIVATE
int Cmd_CLOAK( char *argstr )
{
	//NOTE:
	//CONCOM:
	// cloak_command	::= 'cloak'

	ASSERT( argstr != NULL );
	HANDLE_COMMAND_DOMAIN( argstr );

	if ( ( MyShip->Specials & SPMASK_INVISIBILITY ) == 0 ) {
		ShowMessage( no_invisibility_str );
		return TRUE;
	}

	shader_s *shader = NULL;
	static dword shaderid = (dword)-1;
	if ( shaderid == (dword)-1 ) {
		shader = FetchShaderByName( "ex_invis_anim", &shaderid );
	} else {
		shader = FetchShaderById( shaderid );
	}

	OverrideShaderFaceGlobalInstanced( MyShip, shader );

	ShowMessage( invisibility_activated_str );

	// lose device after it has been activated
//	MyShip->Specials &= ~SPMASK_INVISIBILITY;

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( OBJ_GAME )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "decoy" command
	regcom.command	 = "decoy";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_DECOY;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "cloak" command
	regcom.command	 = "cloak";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_CLOAK;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );
}



