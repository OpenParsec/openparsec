/*
 * PARSEC - Extra management - SERVER
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:45 $
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
//#include "e_defs.h"
#ifdef PARSEC_CLIENT
	#include "aud_defs.h"
#endif // PARSEC_CLIENT

// mathematics header
#include "utl_math.h"

// utility headers
#include "utl_list.h"

// local module header
#include "g_extra.h"

#ifdef PARSEC_CLIENT

	// proprietary module headers
	#include "con_aux.h"
	#include "e_demo.h"
  #include "g_supp.h"
	//#include "net_game.h"
	//#include "net_util.h"
	#include "obj_clas.h"
	#include "obj_creg.h"
	#include "obj_ctrl.h"
	#include "obj_game.h"
	#include "obj_name.h"
	#include "od_props.h"
	#include "od_class.h"
	#include "g_sfx.h"
	//#include "e_connmanager.h"
	//#include "e_simulator.h"

	#include "g_shipobject.h"

#else // !PARSEC_CLIENT

	// proprietary module headers
	#include "con_aux_sv.h"
	#include "g_player.h"
	#include "net_game_sv.h"
	//#include "net_util.h"
	#include "obj_clas.h"
	#include "obj_creg.h"
	#include "obj_name.h"
	#include "od_props.h"
	#include "od_class.h"
	#include "g_main_sv.h"
	#include "e_connmanager.h"
	#include "e_simnetoutput.h"
	#include "e_simulator.h"
	#include "g_shipobject.h"

#endif // !PARSEC_CLIENT


// number of refframes that extras should drift before they stop --------------
//
#define DEFAULT_DRIFT_TIMEOUT	2000

// message strings ------------------------------------------------------------
//
//FIXME: centralize ?
static char got_energy_str[]	        	= "energy boosted";
static char max_energy_str[]	        	= "energy maxed out";

// strings for extra collections ----------------------------------------------
//
//FIXME: centralize ?
static char got_dumb_str[]		        	= "dumb missiles collected";
static char max_dumb_str[]		        	= "dumb missiles maxed out";
static char got_homing_str[]	        	= "guided missiles found";
static char max_homing_str[]	        	= "guided missiles maxed out";
static char got_mines_str[] 	        	= "found proximity mines";
static char max_mines_str[] 	        	= "mines maxed out";
static char got_swarm_str[]		        	= "swarm missiles found";
static char max_swarm_str[]	    	    	= "swarm missiles maxed out";
static char got_helix_str[]					= "got helix cannon";
static char max_helix_str[]					= "already got helix cannon";
static char got_lightning_str[]	        	= "got lightning device";
static char max_lightning_str[]	        	= "already got lightning device";
static char got_photon_str[]				= "got photon cannon";
static char max_photon_str[]				= "already got photon cannon";
static char unknown_dev_str[]	        	= "got unknown device";
static char unknown_extra_str[]	        	= "got unknown extra";
static char no_damage_str[]		        	= "no damage to repair";
static char damage_repaired[]	        	= "damage repaired";
#ifdef PARSEC_CLIENT
static char mine_killed_str[]	        	= "killed by mine";
static char mine_hit_str[]		        	= "mine hit";
#endif
static char got_afterburner_str[]       	= "got afterburner";
static char max_afterburner_str[]       	= "already got afterburner";
static char got_cloak_str[]		        	= "got cloaking device";
static char max_cloak_str[]		        	= "cloak reactivated";
static char got_megashield_str[]        	= "got invulnerability shield";
static char max_megashield_str[]        	= "invulnerability reinforced";
static char got_decoy_str[] 	        	= "got holo decoy device";
static char max_decoy_str[]     	    	= "already got holo decoy device";
static char got_laser_upgrade_1_str[]   	= "got laser upgrade 1";
static char max_laser_upgrade_1_str[]   	= "already got laser upgrade 1";
static char got_laser_upgrade_2_str[]   	= "got laser upgrade 2";
static char max_laser_upgrade_2_str[]   	= "already got laser upgrade 2";
static char need_laser_upgrade_1_str[]		= "need laser upgrade 1 first";
static char got_emp_upgrade_1_str[]			= "got emp upgrade 1";
static char max_emp_upgrade_1_str[]			= "already got emp upgrade 1";
static char got_emp_upgrade_2_str[]			= "got emp upgrade 2";
static char max_emp_upgrade_2_str[]			= "already got emp upgrade 2";
static char need_emp_upgrade_1_str[]		= "need emp upgrade 1 first";


// flags ----------------------------------------------------------------------
//
//#define TEST_EXTRA_CREATION_CODE
//#define TEST_EXTRA_CREATION_CODE_ON_SHIP_DOWNING


// standard ctor --------------------------------------------------------------
//
G_ExtraManager::G_ExtraManager()
{
	ExtraProbability	= EXTRA_PROBABILITY;
	MaxExtraArea		= MAX_EXTRA_AREA;
	MinExtraDist		= MIN_EXTRA_DIST;

	ProbHelixCannon		= PROB_DAZZLE_LASER;
	ProbLightningDevice	= PROB_THIEF_LASER;
	ProbPhotonCannon	= 0;

	ProbProximityMine	= PROB_PROXIMITY_MINE;

	ProbMissilePack		= PROB_MISSILE_PACK;
	ProbDumbMissPack	= PROB_DUMB_MISS_PACK;
	ProbHomMissPack		= PROB_HOM_MISS_PACK;
	ProbSwarmMissPack	= 0;

	ProbRepairExtra		= 0;
	ProbAfterburner		= 0;
	ProbHoloDecoy		= 0;
	ProbInvisibility	= 0;
	ProbInvulnerability	= 0;
	ProbEnergyField		= 0;

	ProbLaserUpgrade 	= 0;
	ProbLaserUpgrade1	= 0;
	ProbLaserUpgrade2	= 0;

	ProbEmpUpgrade1		= 0;
	ProbEmpUpgrade2		= 0;
}

// fill member variables of extras --------------------------------------------
//
void G_ExtraManager::OBJ_FillExtraMemberVars( ExtraObject *extrapo )
{
#ifdef PARSEC_CLIENT
	ASSERT( FALSE );
#else // !PARSEC_CLIENT

	ASSERT( extrapo != NULL );
	ASSERT( OBJECT_TYPE_EXTRA( extrapo ) );

	//NOTE:
	// this function may be called for mines, but it need
	// not be (and is indeed not called when the owner of
	// the mine is set manually).

	// mine is a special extra
	if ( extrapo->ObjectType == MINE1TYPE ) {

		// fallback for owner is the local player
		// (just to be on the safe side, the basic type
		// init sets the owner to the same value)
		((MineObject*)extrapo)->Owner = OWNER_LOCAL_PLAYER;
		return;
	}

	// default: no drifting
	extrapo->DriftTimeout = 0;

	Extra1Obj *extra1po;
	Extra2Obj *extra2po;
	Extra3Obj *extra3po;

	switch ( extrapo->ObjectClass ) {

		case ENERGY_EXTRA_CLASS:
			extra1po = (Extra1Obj *) extrapo;
			extra1po->EnergyBoost = TheGame->EnergyExtraBoost;
			break;

		case REPAIR_EXTRA_CLASS:
			extra1po = (Extra1Obj *) extrapo;
			extra1po->EnergyBoost = TheGame->RepairExtraBoost;
			break;

		case DUMB_PACK_CLASS:
			extra2po = (Extra2Obj *) extrapo;
			extra2po->MissileType = MISSILE1TYPE;
			extra2po->NumMissiles = TheGame->DumbPackNumMissls;
			break;

		case GUIDE_PACK_CLASS:
			extra2po = (Extra2Obj *) extrapo;
			extra2po->MissileType = MISSILE4TYPE;
			extra2po->NumMissiles = TheGame->HomPackNumMissls;
			break;

		case SWARM_PACK_CLASS:
			extra2po = (Extra2Obj *) extrapo;
			extra2po->MissileType = MISSILE5TYPE;
			extra2po->NumMissiles = TheGame->SwarmPackNumMissls;
			break;

		case HELIX_DEVICE_CLASS:
			extra3po = (Extra3Obj *) extrapo;
			extra3po->DeviceType = HELIX_DEVICE;
			break;

		case LIGHTNING_DEVICE_CLASS:
			extra3po = (Extra3Obj *) extrapo;
			extra3po->DeviceType = LIGHTNING_DEVICE;
			break;

		case MINE_PACK_CLASS:
			extra2po = (Extra2Obj *) extrapo;
			extra2po->MissileType = MINE1TYPE;
			extra2po->NumMissiles = TheGame->ProxPackNumMines;
			break;

		case AFTERBURNER_DEVICE_CLASS:
			extra3po = (Extra3Obj *) extrapo;
			extra3po->DeviceType = AFTERBURNER_DEVICE;
			break;

		case INVISIBILITY_CLASS:
			extra3po = (Extra3Obj *) extrapo;
			extra3po->DeviceType = INVISIBILITY_DEVICE;
			break;

		case INVULNERABILITY_CLASS:
			extra3po = (Extra3Obj *) extrapo;
			extra3po->DeviceType = INVULNERABILITY_DEVICE;
			break;

		case PHOTON_DEVICE_CLASS:
			extra3po = (Extra3Obj *) extrapo;
			extra3po->DeviceType = PHOTON_DEVICE;
			break;

		case DECOY_DEVICE_CLASS:
			extra3po = (Extra3Obj *) extrapo;
			extra3po->DeviceType = DECOY_DEVICE;
			break;

		case LASERUPGRADE1_CLASS:
			extra3po = (Extra3Obj *) extrapo;
			extra3po->DeviceType = LASER_UPGRADE_1_DEVICE;
			break;

		case LASERUPGRADE2_CLASS:
			extra3po = (Extra3Obj *) extrapo;
			extra3po->DeviceType = LASER_UPGRADE_2_DEVICE;
			break;

		default:
		
			//FIXME: ??????
			if ( extrapo->ObjectClass == ExtraClasses[ EXTRAINDX_EMPUPGRADE1 ] ) {
			
				extra3po = (Extra3Obj *) extrapo;
				extra3po->DeviceType = EMP_UPGRADE_1_DEVICE;
				
			} else if ( extrapo->ObjectClass == ExtraClasses[ EXTRAINDX_EMPUPGRADE2 ] ) {

				extra3po = (Extra3Obj *) extrapo;
				extra3po->DeviceType = EMP_UPGRADE_2_DEVICE;
			}
			
			break;
		

			MSGOUT( "OBJ_FillExtraMemberVars(): unknown extra class: %d.", extrapo->ObjectClass );
	}
#endif // !PARSEC_CLIENT
}


// kill extra that immediately follows passed node in list --------------------
//
void G_ExtraManager::OBJ_KillExtra( ExtraObject* precnode, int collected )
{
	ASSERT( precnode != NULL );

	ExtraObject *extra = (ExtraObject *) precnode->NextObj;
	ASSERT( extra != NULL );

#ifdef PARSEC_CLIENT

	// create vanishing animation if not collected
	if ( !collected ) {
		SFX_VanishExtra( extra );
	}

	#ifndef NO_EXTRA_KILL_MESSAGES

		// only transmit collection events, not lifetime expiration
		if ( !NET_ConnectedGMSV() ) {
			if ( collected ) {
				NET_RmEvKillObject( extra->HostObjNumber, EXTRA_LIST );
			}
		}

	#endif // NO_EXTRA_KILL_MESSAGES


	// unlink from list
	precnode->NextObj = extra->NextObj;

	// free extra
	FreeObjectMem( extra );

#else // !PARSEC_CLIENT

	// release the E_Distributable ( distribute removal )
	//MSGOUT( "G_ExtraManager::OBJ_KillExtra() calls ReleaseDistributable()" );
	TheSimNetOutput->ReleaseDistributable( extra->pDist );

	// unlink from list
	precnode->NextObj = extra->NextObj;

	// free extra
	TheWorld->FreeObjectMem( extra );

#endif // !PARSEC_CLIENT
}


// animate an extra -----------------------------------------------------------
//
void G_ExtraManager::OBJ_AnimateExtra( ExtraObject *extrapo )
{
#ifdef PARSEC_CLIENT

	ASSERT( FALSE );

#else // !PARSEC_CLIENT

	ASSERT( extrapo != NULL );
	ASSERT( OBJECT_TYPE_EXTRA( extrapo ) );

	// do self rotation
	ObjRotX( extrapo->ObjPosition, extrapo->SelfRotX * TheSimulator->GetThisFrameRefFrames() );
	ObjRotY( extrapo->ObjPosition, extrapo->SelfRotY * TheSimulator->GetThisFrameRefFrames() );
	ObjRotZ( extrapo->ObjPosition, extrapo->SelfRotZ * TheSimulator->GetThisFrameRefFrames() );

	// drift extras away from ship explosion center
	if ( extrapo->DriftTimeout == 0 )
		return;

	extrapo->DriftTimeout -= TheSimulator->GetThisFrameRefFrames();
	if ( extrapo->DriftTimeout < 0 )
		extrapo->DriftTimeout = 0;

	extrapo->ObjPosition[ 0 ][ 3 ] += extrapo->DriftVec.X * TheSimulator->GetThisFrameRefFrames();
	extrapo->ObjPosition[ 1 ][ 3 ] += extrapo->DriftVec.Y * TheSimulator->GetThisFrameRefFrames();
	extrapo->ObjPosition[ 2 ][ 3 ] += extrapo->DriftVec.Z * TheSimulator->GetThisFrameRefFrames();

#endif // !PARSEC_CLIENT
}


// place extras in vicinity of local ship object ------------------------------
//
void G_ExtraManager::OBJ_DoExtraPlacement()
{
#ifdef PARSEC_CLIENT
	ASSERT( FALSE );
#else // !PARSEC_CLIENT

#ifdef PARSEC_SERVER

	// check whether to autocreate extras
	if ( !SV_GAME_EXTRAS_AUTOCREATE ) {
		return;
	}

	// determine upper bound to number of extras
	int maxextras = SV_GAME_EXTRAS_MAXNUM * TheConnManager->GetNumConnected();

#else // !PARSEC_SERVER

	// no extra creation in GMSV mode
	if ( NET_ConnectedGMSV() ) {
		return;
	}

	// don't create extras during demo replay
	if ( !AUX_CREATE_EXTRAS_DURING_REPLAY && DEMO_ReplayActive() )
		return;

	// check if enough space in RE_List
	ASSERT( sizeof( RE_CreateExtra ) >= sizeof( RE_ParticleObject ) );
	if ( !NET_RmEvAllowed( RE_CREATEEXTRA ) ) {
		return;
	}

	// determine upper bound to number of extras
	ASSERT( ( NumRemPlayers >= 0 ) && ( NumRemPlayers <= MAX_NET_PROTO_PLAYERS ) );
	int maxextras = MaxNumExtras;
	if ( NetConnected ) {
		ASSERT( NumRemPlayers > 0 );
		maxextras *= NumRemPlayers;
	}

#endif // !PARSEC_CLIENT

	// create random extra object
	if ( ( TheWorld->m_nCurrentNumExtras + TheWorld->m_nCurrentNumPrtExtras ) < maxextras ) {

		// check whether to create an extra in this turn
		int r = RAND() % 100;
		if ( r > ExtraProbability )
			return;

		Vector3 spawn_point;

#ifdef PARSEC_SERVER

		// spawn position is either somewhere around a randomly selected ship
		// or the origin
		if ( TheGame->GetNumJoined() > 0 ) {

			// get a random ship object
			int nRandomPlayer = RAND() % TheGame->GetNumJoined();

			UTL_List<G_Player*>* pCurJoinedPlayerList = TheGame->GetCurJoinedPlayerList();
			ASSERT( pCurJoinedPlayerList != NULL );

			UTL_listentry_s<G_Player*>* entry = pCurJoinedPlayerList->GetEntryAtIndex( nRandomPlayer );
			ASSERT( entry != NULL );

			G_Player* pPlayer = (G_Player*)entry->m_data;
			ASSERT( pPlayer != NULL );

			ShipObject* pCurShip = pPlayer->GetShipObject();
			ASSERT( pCurShip != NULL );

			FetchTVector( pCurShip->ObjPosition, &spawn_point );
		} else {	

			// spawn around origin
			spawn_point.X = spawn_point.Y = spawn_point.Z = GEOMV_0;
		}

#else // !PARSEC_SERVER

		FetchTVector( MyShip->ObjPosition, &spawn_point );

#endif // !PARSEC_SERVER

		Xmatrx startm;
		MakeIdMatrx( startm );

#ifdef PARSEC_SERVER

		if ( SV_GAME_EXTRAS_TESTPLACE ) {

			static int nNumExtraCreated = 0;
		
			startm[ 0 ][ 3 ] = INT_TO_GEOMV( 0 );
			startm[ 1 ][ 3 ] = INT_TO_GEOMV( 0 );
			startm[ 2 ][ 3 ] = INT_TO_GEOMV( ( ( nNumExtraCreated % 20 ) + 1 ) * 100 );

			nNumExtraCreated++;

		} else {

#endif // PARSEC_SERVER

			// get random position around currently selected ship
			int x = ( RAND() % MaxExtraArea ) - MaxExtraArea / 2;
			int y = ( RAND() % MaxExtraArea ) - MaxExtraArea / 2;
			int z = ( RAND() % MaxExtraArea ) - MaxExtraArea / 2;

			x += ( x < 0 ) ? -MinExtraDist : MinExtraDist;
			y += ( y < 0 ) ? -MinExtraDist : MinExtraDist;
			z += ( z < 0 ) ? -MinExtraDist : MinExtraDist;

			startm[ 0 ][ 3 ] = INT_TO_GEOMV( x ) + spawn_point.X;
			startm[ 1 ][ 3 ] = INT_TO_GEOMV( y ) + spawn_point.Y;
			startm[ 2 ][ 3 ] = INT_TO_GEOMV( z ) + spawn_point.Z;

#ifdef PARSEC_SERVER
		}
#endif // PARSEC_SERVER

		int probsum  = 0;
		int objclass = -1;
		int dice = RAND() % 100;

		// energy field
		if ( dice < ( probsum += ProbEnergyField ) ) {

#ifdef PARSEC_SERVER
			// create energy field particle object
		       Vertex3 origin;
			FetchTVector( startm, &origin );
			TheWorld->SFX_CreateEnergyField( origin );
            
            RE_ParticleObject re_particleobject;
            memset( &re_particleobject , 0, sizeof ( RE_ParticleObject ) );
            re_particleobject.RE_Type       = RE_PARTICLEOBJECT;
            re_particleobject.RE_BlockSize  = sizeof( RE_ParticleObject );
            re_particleobject.ObjectType    = POBJ_ENERGYFIELD;
            re_particleobject.Origin        = origin;

            TheSimNetOutput->BufferForMulticastRE((RE_ParticleObject *) &re_particleobject, PLAYERID_ANONYMOUS, FALSE);
            
 
#else // !PARSEC_SERVER

			// create energy field particle object
			Vertex3 origin;
			FetchTVector( startm, &origin );
			SFX_CreateEnergyField( origin );

			// insert remote event
			NET_RmEvParticleObject( POBJ_ENERGYFIELD, origin );

			// record create event if recording active
			Record_EnergyFieldCreation( origin );

#endif // !PARSEC_SERVER

			return;
		}

		// helix cannon
		if ( dice < ( probsum += ProbHelixCannon ) ) {
			objclass = HELIX_DEVICE_CLASS;

		// lightning device
		} else if ( dice < ( probsum += ProbLightningDevice ) ) {
			objclass = LIGHTNING_DEVICE_CLASS;

		// photon cannon
		} else if ( dice < ( probsum += ProbPhotonCannon ) ) {
			objclass = PHOTON_DEVICE_CLASS;

		// proximity mine pack
		} else if ( dice < ( probsum += ProbProximityMine ) ) {
			objclass = MINE_PACK_CLASS;

		// laser upgrade
		} else if ( dice < ( probsum += ProbLaserUpgrade ) ) {

			probsum = 0;
			int lasertype = RAND() % 100;
			if ( lasertype < ( probsum += ProbLaserUpgrade1 ) ) {
				objclass = LASERUPGRADE1_CLASS;
			} else { // if ( lasertype < ( probsum += ProbLaserUpgrade2 ) )
				objclass = LASERUPGRADE2_CLASS;
			}

		// missile pack
		} else if ( dice < ( probsum += ProbMissilePack ) ) {

			probsum = 0;
			int misstype = RAND() % 100;
			if ( misstype < ( probsum += ProbDumbMissPack ) ) {
				objclass = DUMB_PACK_CLASS;
			} else if ( misstype < ( probsum += ProbHomMissPack ) ) {
				objclass = GUIDE_PACK_CLASS;
			} else { // if ( misstype < ( probsum += ProbSwarmMissPack ) )
				objclass = SWARM_PACK_CLASS;
			}

		// repair extra
		} else if ( dice < ( probsum += ProbRepairExtra ) ) {
			objclass = REPAIR_EXTRA_CLASS;

		// afterburner
		} else if ( dice < ( probsum += ProbAfterburner ) ) {
			objclass = AFTERBURNER_DEVICE_CLASS;

		// holo decoy
		} else if ( dice < ( probsum += ProbHoloDecoy ) ) {
			objclass = DECOY_DEVICE_CLASS;

		// invisibility
		} else if ( dice < ( probsum += ProbInvisibility ) ) {
			objclass = INVISIBILITY_CLASS;

		// invulnerability
		} else if ( dice < ( probsum += ProbInvulnerability ) ) {
			objclass = INVULNERABILITY_CLASS;

		// emp upgrade 1
		} else if ( dice < ( probsum += ProbEmpUpgrade1 ) ) {
		
			if ( ExtraClasses[ EXTRAINDX_EMPUPGRADE1 ] == CLASS_ID_INVALID ) {
				ExtraClasses[ EXTRAINDX_EMPUPGRADE1 ] = OBJ_FetchObjectClassId( OBJCLASSNAME_DEVICE_EMP_UPGRADE_1 );
			}

			objclass = ExtraClasses[ EXTRAINDX_EMPUPGRADE1 ];			
			
		// emp upgrade 2
		} else if ( dice < ( probsum += ProbEmpUpgrade2 ) ) {
		
			if ( ExtraClasses[ EXTRAINDX_EMPUPGRADE2 ] == CLASS_ID_INVALID ) {
				ExtraClasses[ EXTRAINDX_EMPUPGRADE2 ] = OBJ_FetchObjectClassId( OBJCLASSNAME_DEVICE_EMP_UPGRADE_2 );
			}

			objclass = ExtraClasses[ EXTRAINDX_EMPUPGRADE2 ];			

		// energy extra
		} else {
			objclass = ENERGY_EXTRA_CLASS;
		}

#ifdef TEST_EXTRA_CREATION_CODE

		objclass = ENERGY_EXTRA_CLASS;

#endif // !TEST_EXTRA_CREATION_CODE

		// create object
		ExtraObject *extrapo = (ExtraObject *) TheWorld->CreateObject( objclass, startm, PLAYERID_SERVER );
		OBJ_FillExtraMemberVars( extrapo );


#ifdef PARSEC_SERVER

		// attach the created E_Distributable for the engine object
		extrapo->pDist = TheSimNetOutput->CreateDistributable( extrapo );

		// record create event if recording active
		//Record_ExtraCreation( extrapo );


#else // !PARSEC_SERVER

		// insert remote event
		NET_RmEvExtra( extrapo );

		// record create event if recording active
		Record_ExtraCreation( extrapo );

#endif // !PARSEC_SERVER
	}
#endif // !PARSEC_CLIENT
}


// define area in which extras around ship will be placed ---------------------
//
#define EXPL_MAXEXTRAAREA	30
#define EXPL_MINEXTRADIST	5

#define EXPL_DRIFTSPEED		0.0012f


// place extras after ship was shot down --------------------------------------
//
void G_ExtraManager::_PlaceShipExtras( int num, int objclass, ShipObject *shippo )
{
#ifdef PARSEC_CLIENT
	
	ASSERT( FALSE );

#else // !PARSEC_CLIENT

	ASSERT( num >= 0 );
	ASSERT( objclass >= 0 );
	ASSERT( objclass < NumObjClasses );
	ASSERT( shippo != NULL );

	for ( ; num > 0; num-- ) {

#ifdef PARSEC_CLIENT

		if ( !NET_RmEvAllowed( RE_CREATEEXTRA ) )
			return;

#endif // PARSEC_CLIENT

		// do random placement of extras in near vicinity of ship
		int x = ( RAND() % EXPL_MAXEXTRAAREA ) - EXPL_MAXEXTRAAREA / 2;
		int y = ( RAND() % EXPL_MAXEXTRAAREA ) - EXPL_MAXEXTRAAREA / 2;
		int z = ( RAND() % EXPL_MAXEXTRAAREA ) - EXPL_MAXEXTRAAREA / 2;

		x += ( x < 0 ) ? -EXPL_MINEXTRADIST : EXPL_MINEXTRADIST;
		y += ( y < 0 ) ? -EXPL_MINEXTRADIST : EXPL_MINEXTRADIST;
		z += ( z < 0 ) ? -EXPL_MINEXTRADIST : EXPL_MINEXTRADIST;

#ifdef TEST_EXTRA_CREATION_CODE_ON_SHIP_DOWNING

		static int numplaced = 0;

		Xmatrx startm;
		MakeIdMatrx( startm );
		startm[ 0 ][ 3 ] = INT_TO_GEOMV( 0 ) + shippo->ObjPosition[ 0 ][ 3 ];
		startm[ 1 ][ 3 ] = INT_TO_GEOMV( ( numplaced % 10 ) * 20 ) + shippo->ObjPosition[ 1 ][ 3 ];
		startm[ 2 ][ 3 ] = INT_TO_GEOMV( 0 ) + shippo->ObjPosition[ 2 ][ 3 ];

		numplaced++;

#else // !TEST_EXTRA_CREATION_CODE_ON_SHIP_DOWNING
		Xmatrx startm;
		MakeIdMatrx( startm );
		startm[ 0 ][ 3 ] = INT_TO_GEOMV( x ) + shippo->ObjPosition[ 0 ][ 3 ];
		startm[ 1 ][ 3 ] = INT_TO_GEOMV( y ) + shippo->ObjPosition[ 1 ][ 3 ];
		startm[ 2 ][ 3 ] = INT_TO_GEOMV( z ) + shippo->ObjPosition[ 2 ][ 3 ];
#endif // !TEST_EXTRA_CREATION_CODE_ON_SHIP_DOWNING

		// create object ( owner is ship )
		//NOTE: 
		// normal extras have PLAYERID_SERVER as owner, only extras placed after
		// a player was shot down are owned by the player
		int owner = GetOwnerFromHostOjbNumber( shippo->HostObjNumber );
		ExtraObject *extrapo = (ExtraObject *) TheWorld->CreateObject( objclass, startm, owner );
		OBJ_FillExtraMemberVars( extrapo );

		// initialize drift timeout
		extrapo->DriftTimeout = DEFAULT_DRIFT_TIMEOUT;

		// calculate drift direction vector
		extrapo->DriftVec.X = FLOAT_TO_GEOMV( x * EXPL_DRIFTSPEED );
		extrapo->DriftVec.Y = FLOAT_TO_GEOMV( y * EXPL_DRIFTSPEED );
		extrapo->DriftVec.Z = FLOAT_TO_GEOMV( z * EXPL_DRIFTSPEED );

		//TODO:
		// randomly alter self rotation.

		ASSERT( extrapo->ObjectClass != MINE_CLASS_1 );
		ASSERT( extrapo->ObjectType != MINE1TYPE );

#ifdef PARSEC_SERVER

		// attach the created E_Distributable for the engine object
		extrapo->pDist = TheSimNetOutput->CreateDistributable( extrapo, FALSE, TRUE );

		// record create event if recording active
		//Record_ExtraCreation( extrapo );

#else // !PARSEC_SERVER

		// insert remote event
		NET_RmEvExtra( extrapo );

		// record create event if recording active
		Record_ExtraCreation( extrapo );

#endif // !PARSEC_SERVER
	}
#endif // // !PARSEC_CLIENT
}

// create extras after ship was shot down -------------------------------------
//
void G_ExtraManager::OBJ_CreateShipExtras( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

#ifdef PARSEC_CLIENT
	ASSERT( FALSE );
#else // !PARSEC_CLIENT


#ifdef PARSEC_CLIENT
	//NOTE:
	// this is a helper function for functions
	// in OBJ_COLL.C and for R_DrawWorld().

	if ( !AUX_ENABLE_KILLED_SHIP_EXTRAS )
		return;

	// don't create extras during demo replay
	if ( !AUX_CREATE_EXTRAS_DURING_REPLAY && DEMO_ReplayActive() )
		return;

	// in network game only for local ship because
	// others will be received as remote events
	if ( NetConnected && ( shippo != MyShip ) )
		return;

//FIXME:
//	if ( shippo != MyShip )
//		return;

#endif // PARSEC_CLIENT


//#ifdef TEST_EXTRA_CREATION_CODE
//	_PlaceShipExtras( 1, DUMB_PACK_CLASS, shippo );
//	return;
//#endif // TEST_EXTRA_CREATION_CODE


	#define MAX_DUMBPACKS		2
	#define MAX_GUIDEPACKS		2
	#define MAX_MINEPACKS		1

	// create dumb missile packs
	if ( shippo->NumMissls > 0 ) {
        if(TheGame->DumbPackNumMissls > 0) {
		   int num_missilepacks = ( shippo->NumMissls / TheGame->DumbPackNumMissls );
		   if ( num_missilepacks > MAX_DUMBPACKS ) {
              num_missilepacks = MAX_DUMBPACKS;
		   }

		   _PlaceShipExtras( num_missilepacks, DUMB_PACK_CLASS, shippo );
	    }
    }

	// create guided missile packs
	if ( shippo->NumHomMissls > 0 ) {
        if(TheGame->HomPackNumMissls > 0) {
		   int num_guidedmissilepacks = ( shippo->NumHomMissls / TheGame->HomPackNumMissls );
		   if ( num_guidedmissilepacks > MAX_GUIDEPACKS ) {
              num_guidedmissilepacks = MAX_GUIDEPACKS;
		   }

		   _PlaceShipExtras( num_guidedmissilepacks, GUIDE_PACK_CLASS, shippo );
        }
	}

	// create proximity mine packs
	if ( shippo->NumMines > 0 ) {
        if(TheGame->ProxPackNumMines > 0) {
		   int num_minepacks = ( shippo->NumMines / TheGame->ProxPackNumMines );
		   if ( num_minepacks > MAX_MINEPACKS ) {
              num_minepacks = MAX_MINEPACKS;
		   }

		   _PlaceShipExtras( num_minepacks, MINE_PACK_CLASS, shippo );
        }
	}

	// create lightning device
	if ( TheGame->OBJ_DeviceAvailable( shippo, WPMASK_CANNON_LIGHTNING ) ) {

		_PlaceShipExtras( 1, LIGHTNING_DEVICE_CLASS, shippo );
	}

	// create helix cannon
	if ( TheGame->OBJ_DeviceAvailable( shippo, WPMASK_CANNON_HELIX ) ) {

		_PlaceShipExtras( 1, HELIX_DEVICE_CLASS, shippo );
	}

	// create photon cannon
	if ( TheGame->OBJ_DeviceAvailable( shippo, WPMASK_CANNON_PHOTON ) ) {

		_PlaceShipExtras( 1, PHOTON_DEVICE_CLASS, shippo );
	}
	
	// create laser upgrade 1
	if ( shippo->Specials & SPMASK_LASER_UPGRADE_1 ) {
	
		_PlaceShipExtras( 1, LASERUPGRADE1_CLASS, shippo );
	}

	// create laser upgrade 2
	if ( shippo->Specials & SPMASK_LASER_UPGRADE_2 ) {
	
		_PlaceShipExtras( 1, LASERUPGRADE2_CLASS, shippo );
	}

	//FIXME: why do we not place the EMP upgrades ?

/*
	// create emp upgrade 1
	if ( shippo->Specials & SPMASK_EMP_UPGRADE_1 ) {
	
		_PlaceShipExtras( 1, EMPUPGRADE1_CLASS, shippo );
	}

	// create emp upgrade 2
	if ( shippo->Specials & SPMASK_EMP_UPGRADE_2 ) {
	
		_PlaceShipExtras( 1, EMPUPGRADE2_CLASS, shippo );
	}
*/	
	// create energy extra if ship still has enough energy
	if ( shippo->CurEnergy >= TheGame->EnergyExtraBoost ) {
	
		_PlaceShipExtras( 1, ENERGY_EXTRA_CLASS, shippo );
	}
#endif // !PARSEC_CLIENT
}




// boost extra collected: energy boost ----------------------------------------
//
char* G_ExtraManager::_CollectBoostEnergy( ShipObject* cur_ship, Extra1Obj* extra1po )
{
	ASSERT( extra1po != NULL );
	char* text = NULL;

	G_ShipObject* pShip = (G_ShipObject*)cur_ship;
#ifdef PARSEC_DEBUG
	//MSGOUT( "G_ExtraManager::_CollectBoostEnergy() adds energy of %d", extra1po->EnergyBoost );
#endif // PARSEC_DEBUG
	CLIENT_ONLY( ASSERT( cur_ship == MyShip ); );
	if ( !pShip->BoostEnergy( extra1po->EnergyBoost ) ) {
		CLIENT_ONLY( AUD_MaxedOut( ENERGY_EXTRA_CLASS ); );
		text = max_energy_str;
	} else {
		CLIENT_ONLY( AUD_EnergyBoosted(); );
		CLIENT_ONLY( AUD_JimCommenting( JIM_COMMENT_ENERGY ); );
		text = got_energy_str;
	}

	return text;
}

// boost extra collected: energy boost ----------------------------------------
//
char* G_ExtraManager::_CollectBoostEnergyField( ShipObject* cur_ship, int boost )
{
	char* text = NULL;

	G_ShipObject* pShip = (G_ShipObject*)cur_ship;
#ifdef PARSEC_DEBUG
	//MSGOUT( "G_ExtraManager::_CollectBoostEnergyField() adds energy of %d", boost );
#endif // PARSEC_DEBUG
	CLIENT_ONLY( ASSERT( cur_ship == MyShip ); );
	if ( !pShip->BoostEnergy( boost ) ) {
		CLIENT_ONLY( AUD_MaxedOut( ENERGY_EXTRA_CLASS ); );
		text = max_energy_str;
	} else {
		CLIENT_ONLY( AUD_EnergyBoosted(); );
		CLIENT_ONLY( AUD_JimCommenting( JIM_COMMENT_ENERGY ); );
		text = got_energy_str;
	}

	return text;
}

// boost extra collected: repair damage ---------------------------------------
//
char* G_ExtraManager::_CollectBoostRepair( ShipObject* cur_ship, Extra1Obj* extra1po )
{
	ASSERT( extra1po != NULL );
	char *text = NULL;
#ifdef PARSEC_DEBUG
	//MSGOUT( "G_ExtraManager::_CollectBoostRepair() adds energy of %d", extra1po->EnergyBoost );
#endif // PARSEC_DEBUG

	CLIENT_ONLY( ASSERT( cur_ship == MyShip ); );
	if ( !((G_ShipObject*)cur_ship)->BoostRepair( extra1po->EnergyBoost ) ) {
		CLIENT_ONLY( AUD_MaxedOut( REPAIR_EXTRA_CLASS ); );
		text = no_damage_str;
	} else {
		CLIENT_ONLY( AUD_DamageRepaired(); );
		text = damage_repaired;
	}

	return text;
}


// package extra collected: dumb missiles -------------------------------------
//
char* G_ExtraManager::_CollectPackDumb( ShipObject* cur_ship, Extra2Obj *extra2po )
{
	ASSERT( cur_ship != NULL );
	ASSERT( extra2po != NULL );
	char *text = NULL;

#ifdef PARSEC_DEBUG
	//MSGOUT( "G_ExtraManager::_CollectPackDumb() adds up to %d Missiles", extra2po->NumMissiles );
#endif // PARSEC_DEBUG

	CLIENT_ONLY( ASSERT( cur_ship == MyShip ); );
	if ( !((G_ShipObject*)cur_ship)->BoostMissiles( extra2po->NumMissiles ) ) {
		CLIENT_ONLY( AUD_MaxedOut( MISSILE1TYPE ); );
		text = max_dumb_str;
	} else {
		CLIENT_ONLY( AUD_ExtraCollected( MISSILE1TYPE ); );
		text = got_dumb_str;
	}

	return text;
}


// package extra collected: guided missiles -----------------------------------
//

char* G_ExtraManager::_CollectPackGuide( ShipObject* cur_ship, Extra2Obj *extra2po )
{
	ASSERT( cur_ship != NULL );
	ASSERT( extra2po != NULL );
	char *text = NULL;

#ifdef PARSEC_DEBUG
//	MSGOUT( "G_ExtraManager::_CollectPackGuide() adds up to %d Missiles", extra2po->NumMissiles );
#endif // PARSEC_DEBUG
	CLIENT_ONLY( ASSERT( cur_ship == MyShip ); );
	if ( !((G_ShipObject*)cur_ship)->BoostHomMissiles( extra2po->NumMissiles ) ) {
		text = max_homing_str;
		CLIENT_ONLY( AUD_MaxedOut( MISSILE4TYPE ); );
	} else {
		text = got_homing_str;
		CLIENT_ONLY( AUD_ExtraCollected( MISSILE4TYPE ); );
	}

	return text;
}


// device extra collected: swarm missiles -------------------------------------
//
char* G_ExtraManager::_CollectPackSwarm( ShipObject* cur_ship, Extra2Obj *extra2po )
{
	ASSERT( cur_ship != NULL );
	ASSERT( extra2po != NULL );

	char *text = NULL;

#ifdef PARSEC_DEBUG
//	MSGOUT( "G_ExtraManager::_CollectPackSwarm() adds up to %d Missiles", extra2po->NumMissiles );
#endif // PARSEC_DEBUG
	CLIENT_ONLY( ASSERT( cur_ship == MyShip ); );
	if ( !((G_ShipObject*)cur_ship)->BoostPartMissiles( extra2po->NumMissiles ) ) {
		text = max_swarm_str;
		CLIENT_ONLY( AUD_MaxedOut( MISSILE5TYPE ); );
	} else {
		text = got_swarm_str;
		CLIENT_ONLY( AUD_ExtraCollected( MISSILE5TYPE ); );
	}

	return text;
}


// package extra collected: proximity mines -----------------------------------
//

char* G_ExtraManager::_CollectPackMine( ShipObject* cur_ship, Extra2Obj *extra2po )
{
	ASSERT( cur_ship != NULL );
	ASSERT( extra2po != NULL );
	char *text = NULL;

#ifdef PARSEC_DEBUG
//	MSGOUT( "G_ExtraManager::_CollectPackMine() adds up to %d Mines", extra2po->NumMissiles );
#endif // PARSEC_DEBUG
	CLIENT_ONLY( ASSERT( cur_ship == MyShip ); );
	if ( !((G_ShipObject*)cur_ship)->BoostMines( extra2po->NumMissiles ) ) {
		CLIENT_ONLY( AUD_MaxedOut( MINE1TYPE ); );
		text = max_mines_str;
	} else {
		CLIENT_ONLY( AUD_ExtraCollected( MINE1TYPE ); );
		text = got_mines_str;
	}

	return text;
}

// helper struct for defining paramters when collecting devices/specials ------
//
struct collect_info_s {
	int		isDevice;
	int		mask;
	char*	success_string;
	char*	failure_string;
};


collect_info_s collect_info[] =
{
	{ FALSE,	0,						NULL,					NULL },
	{ TRUE,		WPMASK_CANNON_HELIX,		got_helix_str,			max_helix_str },
	{ TRUE,		WPMASK_CANNON_LIGHTNING,	got_lightning_str,		max_lightning_str },
	{ FALSE,	SPMASK_AFTERBURNER,			got_afterburner_str,	max_afterburner_str },
	{ FALSE,	SPMASK_INVISIBILITY,		got_cloak_str,			max_cloak_str },
	{ TRUE,		WPMASK_CANNON_PHOTON,		got_photon_str,			max_photon_str },
	{ FALSE,	SPMASK_DECOY,				got_decoy_str,			max_decoy_str },		
	{ FALSE,	SPMASK_INVULNERABILITY,		got_megashield_str,		max_megashield_str },
	/*LASER_UPGRADE_1_DEVICE,			// 8
	LASER_UPGRADE_2_DEVICE,			// 9
	EMP_UPGRADE_1_DEVICE,			// 10
	EMP_UPGRADE_2_DEVICE,			// 11*/
};


// helper function when collecting devices ------------------------------------
// 
char* G_ExtraManager::_CollectDevice( int nDevice, ShipObject* cur_ship )
{
	ASSERT( nDevice >  UNKNOWN_DEVICE );
	ASSERT( nDevice <= EMP_UPGRADE_2_DEVICE );
	ASSERT( cur_ship != NULL );
	char *text = NULL;

	CLIENT_ONLY( ASSERT( cur_ship == MyShip ); );
	if ( ((G_ShipObject*)cur_ship)->CollectDevice( collect_info[ nDevice ].mask ) ) {
		text = collect_info[ nDevice ].success_string;
		CLIENT_ONLY( AUD_ExtraCollected( nDevice ); );
	} else {
		text = collect_info[ nDevice ].failure_string;
		CLIENT_ONLY( AUD_MaxedOut( nDevice ); );
	}

	return text;
}

// helper function when collecting specials -----------------------------------
// 
char* G_ExtraManager::_CollectSpecial( int nSpecial, ShipObject* cur_ship )
{
	ASSERT( nSpecial >  UNKNOWN_DEVICE );
	ASSERT( nSpecial <= EMP_UPGRADE_2_DEVICE );
	ASSERT( cur_ship != NULL );
	char *text = NULL;
	
	CLIENT_ONLY( ASSERT( cur_ship == MyShip ); );
	
#ifdef PARSEC_SERVER
	// for the server, create and send an RE_Generic saying that we picked up 
	// the invulnerability device.
	if(nSpecial == INVULNERABILITY_DEVICE) {
		RE_Generic *re_gen = new RE_Generic;
		re_gen->RE_ActionFlags |= 1 << INVUNERABLE;
		re_gen->HostObjId = cur_ship->HostObjNumber;
		re_gen->TargetId = 0;
		re_gen->Padding = 0;
		re_gen->RE_BlockSize = sizeof(RE_Generic);
		re_gen->RE_Type = RE_GENERIC;

		TheSimNetOutput->BufferForMulticastRE(re_gen, PLAYERID_SERVER, true);
		delete re_gen;
	}
#endif
	if ( ((G_ShipObject*)cur_ship)->CollectSpecial( collect_info[ nSpecial ].mask ) ) {
		text = collect_info[ nSpecial ].success_string;
	} else {
		text = collect_info[ nSpecial ].failure_string;
	}

	// special is collected in any case
	CLIENT_ONLY( AUD_ExtraCollected( nSpecial ); );

	return text;
}


// device extra collected: helix cannon ---------------------------------------
//
char* G_ExtraManager::_CollectDeviceHelix( ShipObject* cur_ship )
{
	return _CollectDevice( HELIX_DEVICE, cur_ship );
}


// device extra collected: lightning device -----------------------------------
//
char* G_ExtraManager::_CollectDeviceLightning( ShipObject* cur_ship )
{
	return _CollectDevice( LIGHTNING_DEVICE, cur_ship );
}


// device extra collected: photon cannon --------------------------------------
//

char* G_ExtraManager::_CollectDevicePhoton( ShipObject* cur_ship )
{
	return _CollectDevice( PHOTON_DEVICE, cur_ship );
}


// device extra collected: afterburner ----------------------------------------
//
char* G_ExtraManager::_CollectDeviceAfterburner( ShipObject* cur_ship )
{
#ifdef PARSEC_DEBUG
//	MSGOUT( "G_ExtraManager::_CollectDeviceAfterburner() Item collected");
#endif // PARSEC_DEBUG
   return _CollectSpecial( AFTERBURNER_DEVICE, cur_ship );
}


// device extra collected: invisibility ---------------------------------------
//
char* G_ExtraManager::_CollectDeviceInvisibility( ShipObject* cur_ship )
{
#ifdef PARSEC_DEBUG
//	MSGOUT( "G_ExtraManager::_CollectDeviceInvisibility() Item collected");
#endif // PARSEC_DEBUG
	return _CollectSpecial( INVISIBILITY_DEVICE, cur_ship );
}


// device extra collected: invulnerability ------------------------------------
//
char* G_ExtraManager::_CollectDeviceInvulnerability( ShipObject* cur_ship )
{
#ifdef PARSEC_DEBUG
//	MSGOUT( "G_ExtraManager::_CollectDeviceinvunerability() Item collected");
#endif // PARSEC_DEBUG	
   return _CollectSpecial( INVULNERABILITY_DEVICE, cur_ship );
}


// device extra collected: decoy ----------------------------------------------
//
char* G_ExtraManager::_CollectDeviceDecoy( ShipObject* cur_ship )
{
	return _CollectSpecial( DECOY_DEVICE, cur_ship );
}


// device extra collected: laser upgrade 1 ------------------------------------
//
char* G_ExtraManager::_CollectDeviceLaserUpgrade1( ShipObject* cur_ship )
{
	ASSERT( cur_ship != NULL );

	char *text = NULL;
#ifdef PARSEC_DEBUG
//	MSGOUT( "G_ExtraManager::_CollectDeviceLaserUpgrade1() Item collected");
#endif // PARSEC_DEBUG
	CLIENT_ONLY( ASSERT( cur_ship == MyShip ); );
	if ( cur_ship->Specials & SPMASK_LASER_UPGRADE_2 ) {
		text = max_laser_upgrade_2_str;
		CLIENT_ONLY( AUD_MaxedOut( LASER_UPGRADE_1_DEVICE ); );
	} else {
		if ( ( cur_ship->Specials & SPMASK_LASER_UPGRADE_1 ) == 0 ) {

			text = got_laser_upgrade_1_str;
			CLIENT_ONLY( AUD_ExtraCollected( LASER_UPGRADE_1_DEVICE ); );

			((G_ShipObject*)cur_ship)->EnableLaserUpgrade1();

		} else {

			text = max_laser_upgrade_1_str;
			CLIENT_ONLY( AUD_MaxedOut( LASER_UPGRADE_1_DEVICE ); );
		}
	}

	return text;
}


// device extra collected: laser upgrade 2 ------------------------------------
//

char* G_ExtraManager::_CollectDeviceLaserUpgrade2( ShipObject* cur_ship )
{
	ASSERT( cur_ship != NULL );

	char *text = NULL;

#ifdef PARSEC_DEBUG
//	MSGOUT( "G_ExtraManager::_CollectDeviceLaserUpgrade2() Item collected");
#endif // PARSEC_DEBUG
	CLIENT_ONLY( ASSERT( cur_ship == MyShip ); );
	if ( ( cur_ship->Specials & SPMASK_LASER_UPGRADE_1 ) == 0 ) {
		text = need_laser_upgrade_1_str;
		//FIXME: no audio feedback here ?
	} else {
		if ( ( cur_ship->Specials & SPMASK_LASER_UPGRADE_2 ) == 0 ) {

			text = got_laser_upgrade_2_str;
			CLIENT_ONLY( AUD_ExtraCollected( LASER_UPGRADE_2_DEVICE ); );

			((G_ShipObject*)cur_ship)->EnableLaserUpgrade2();

		} else {

			text = max_laser_upgrade_2_str;
			CLIENT_ONLY( AUD_MaxedOut( LASER_UPGRADE_2_DEVICE ); );
		}
	}

	return text;
}


// device extra collected: emp upgrade 1 --------------------------------------
//

char* G_ExtraManager::_CollectDeviceEmpUpgrade1( ShipObject* cur_ship )
{
	ASSERT( cur_ship != NULL );

	char *text = NULL;

	CLIENT_ONLY( ASSERT( cur_ship == MyShip ); );
	// no emp1 upgrade if already got emp2
	if ( cur_ship->Specials & SPMASK_EMP_UPGRADE_2 ) {
		text = max_emp_upgrade_2_str;
		CLIENT_ONLY( AUD_MaxedOut( EMP_UPGRADE_1_DEVICE ); );
	} else {
		if ( ( cur_ship->Specials & SPMASK_EMP_UPGRADE_1 ) == 0 ) {
			text = got_emp_upgrade_1_str;
			CLIENT_ONLY( AUD_ExtraCollected( EMP_UPGRADE_1_DEVICE ); );

			((G_ShipObject*)cur_ship)->EnableEmpUpgrade1();

		} else {
			text = max_emp_upgrade_1_str;
			CLIENT_ONLY( AUD_MaxedOut( EMP_UPGRADE_1_DEVICE ); );
		}
	}

	return text;
}


// device extra collected: emp upgrade 2 --------------------------------------
//

char* G_ExtraManager::_CollectDeviceEmpUpgrade2( ShipObject* cur_ship )
{
	ASSERT( cur_ship != NULL );
	char *text = NULL;

	CLIENT_ONLY( ASSERT( cur_ship == MyShip ); );
	if ( ( cur_ship->Specials & SPMASK_EMP_UPGRADE_1 ) == 0 ) {
		text = need_emp_upgrade_1_str;
		//FIXME: no audio feedback here ?
	} else {
		if ( ( cur_ship->Specials & SPMASK_EMP_UPGRADE_2 ) == 0 ) {

			text = got_emp_upgrade_2_str;
			CLIENT_ONLY( AUD_ExtraCollected( EMP_UPGRADE_2_DEVICE ); );

			((G_ShipObject*)cur_ship)->EnableEmpUpgrade2();

		} else {

			text = max_emp_upgrade_2_str;
			CLIENT_ONLY( AUD_MaxedOut( EMP_UPGRADE_2_DEVICE ); );
		}
	}

	return text;
}


// local ship collided with proximity mine ------------------------------------
//
char *G_ExtraManager::_CollisionProximityMine( Mine1Obj *minepo )
{
	ASSERT( minepo != NULL );

	//FIXME: implement server side.... 

	char *text = NULL;

#ifdef PARSEC_CLIENT
	int hitpoints = minepo->HitPoints;
	if ( ( hitpoints -= MyShip->MegaShieldAbsorption ) < 0 ) {
		hitpoints = 0;
	}

	if ( MyShip->CurDamage <= MyShip->MaxDamage ) {

		MyShip->CurDamage += hitpoints;
		if ( MyShip->CurDamage > MyShip->MaxDamage ) {

			KillDurationWeapons( MyShip );
			OBJ_CreateShipExtras( MyShip );

			// if mine is owned by remote player update kill state
			if ( minepo->Owner != OWNER_LOCAL_PLAYER ) {
				NET_SetPlayerKillStat( minepo->Owner, 1 );
			}

			text = mine_killed_str;
			AUD_PlayerKilled();

		} else {

			// detect hull impact

			OBJ_EventShipImpact( MyShip, TRUE );

			text = mine_hit_str;
			AUD_MineCollision();
		}

	} else {

		// occurs on mine hit when already killed
		text = mine_hit_str;
	}
#endif
	return text;
}



// perform action according to extra type and determine message to show -------
//
char* G_ExtraManager::CollectExtra( ShipObject* cur_ship, ExtraObject* curextra )
{
	ASSERT( cur_ship != NULL );
	ASSERT( curextra != NULL );

	const char *text = NULL;

	/*MSGOUT( "G_ExtraManager::CollectExtra() %d at %.2f/%.2f/%.2f - ship at %.2f/%.2f/%.2f",
		curextra->HostObjNumber,

		curextra->ObjPosition[ 0 ][ 3 ],
		curextra->ObjPosition[ 1 ][ 3 ],
		curextra->ObjPosition[ 2 ][ 3 ],

		cur_ship->ObjPosition[ 0 ][ 3 ],
		cur_ship->ObjPosition[ 1 ][ 3 ],
		cur_ship->ObjPosition[ 2 ][ 3 ]
		);
*/
	// extras of type 1 (boost extras)
	if ( curextra->ObjectType == EXTRA1TYPE ) {

		Extra1Obj *extra1po = (Extra1Obj *) curextra;

		switch ( extra1po->ObjectClass ) {

			// energy boost extra
			case ENERGY_EXTRA_CLASS:
				text = _CollectBoostEnergy( cur_ship, extra1po );
				break;

			// damage repair extra
			case REPAIR_EXTRA_CLASS:
				text = _CollectBoostRepair( cur_ship, extra1po );
				break;

			// unrecognized class for this type
			default:
				MSGOUT( "G_ExtraManager::CollectExtra(): unknown boost extra: class1 %d.", extra1po->ObjectClass );
				text = unknown_extra_str;
		}

	// extras of type 2 (package extras)
	} else if ( curextra->ObjectType == EXTRA2TYPE ) {

		Extra2Obj *extra2po = (Extra2Obj *) curextra;

		switch ( extra2po->MissileType ) {

			// dumb missile package
			case MISSILE1TYPE:
				text = _CollectPackDumb( cur_ship, extra2po );
				break;

			// guide missile package
			case MISSILE4TYPE:
				text = _CollectPackGuide( cur_ship, extra2po );
				break;

			// swarm missiles package
			case MISSILE5TYPE:
				text = _CollectPackSwarm( cur_ship, extra2po );
				break;

			// proximity mine package
			case MINE1TYPE:
				text = _CollectPackMine( cur_ship, extra2po );
				break;
           
            // laser upgrade 1
			case LASER_UPGRADE_1_DEVICE:
				text = _CollectDeviceLaserUpgrade1( cur_ship );
				break;

			// laser upgrade 2
			case LASER_UPGRADE_2_DEVICE:
				text = _CollectDeviceLaserUpgrade2( cur_ship );
				break;
			// unrecognized class for this type
			default:
				MSGOUT( "G_ExtraManager::CollectExtra(): unknown package extra: class2 %d.", extra2po->ObjectClass );
				text = unknown_extra_str;
		}

	// extras of type 3 (device extras)
	} else if ( curextra->ObjectType == EXTRA3TYPE ) {

		Extra3Obj *extra3po = (Extra3Obj *) curextra;

		switch ( extra3po->DeviceType ) {

			// helix cannon
			case HELIX_DEVICE:
				text = _CollectDeviceHelix( cur_ship );
				break;

			// lightning device
			case LIGHTNING_DEVICE:
				text = _CollectDeviceLightning( cur_ship );
				break;

			// afterburner device
			case AFTERBURNER_DEVICE:
				text = _CollectDeviceAfterburner( cur_ship );
				break;

			// cloaking (invisibility) device
			case INVISIBILITY_DEVICE:
				text = _CollectDeviceInvisibility( cur_ship );
				break;

			// photon cannon
			case PHOTON_DEVICE:
				text = _CollectDevicePhoton( cur_ship );
				break;

			// invulnerability device
			case INVULNERABILITY_DEVICE:
				text = _CollectDeviceInvulnerability( cur_ship );
				break;

			// decoy device
			case DECOY_DEVICE:
				text = _CollectDeviceDecoy( cur_ship );
				break;

			// laser upgrade 1
			case LASER_UPGRADE_1_DEVICE:
				text = _CollectDeviceLaserUpgrade1( cur_ship );
				break;

			// laser upgrade 2
			case LASER_UPGRADE_2_DEVICE:
				text = _CollectDeviceLaserUpgrade2( cur_ship );
				break;

			// emp upgrade 1
			case EMP_UPGRADE_1_DEVICE:
				text = _CollectDeviceEmpUpgrade1( cur_ship );
				break;

			// emp upgrade 2
			case EMP_UPGRADE_2_DEVICE:
				text = _CollectDeviceEmpUpgrade2( cur_ship );
				break;

			// unrecognized device type found
			default:
				MSGOUT( "G_ExtraManager::CollectExtra(): unknown device extra: class3 %d.", extra3po->ObjectClass );
				text = unknown_dev_str;
		}

	// collision with proximity mine
	} else if ( curextra->ObjectType == MINE1TYPE ) {
        //MSGOUT( "G_ExtraManager::CollectExtra(): Mine Collision");
		text = "mine";
	}

	return (char *) text;
}




