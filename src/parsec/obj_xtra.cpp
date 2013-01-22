/*
 * PARSEC - Extra Objects
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:24 $
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
#include "od_class.h"

// global externals
#include "globals.h"

// subsystem headers
#include "net_defs.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "obj_xtra.h"

// proprietary module headers
#include "con_aux.h"
#include "con_ext.h"
#include "e_demo.h"
#include "e_record.h"
#include "obj_clas.h"
#include "obj_creg.h"
#include "obj_ctrl.h"
#include "obj_game.h"
#include "obj_name.h"
#include "g_sfx.h"


// flags
//#define NO_EXTRA_MAINTENANCE
//#define NO_EXTRA_KILL_MESSAGES



// number of refframes that extras should drift before they stop --------------
//
#define DEFAULT_DRIFT_TIMEOUT	2000


// fill member variables of extras --------------------------------------------
//
void OBJ_FillExtraMemberVars( ExtraObject *extrapo )
{
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
			extra1po->EnergyBoost = EnergyExtraBoost;
			break;

		case REPAIR_EXTRA_CLASS:
			extra1po = (Extra1Obj *) extrapo;
			extra1po->EnergyBoost = RepairExtraBoost;
			break;

		case DUMB_PACK_CLASS:
			extra2po = (Extra2Obj *) extrapo;
			extra2po->MissileType = MISSILE1TYPE;
			extra2po->NumMissiles = DumbPackNumMissls;
			break;

		case GUIDE_PACK_CLASS:
			extra2po = (Extra2Obj *) extrapo;
			extra2po->MissileType = MISSILE4TYPE;
			extra2po->NumMissiles = HomPackNumMissls;
			break;

		case SWARM_PACK_CLASS:
			extra2po = (Extra2Obj *) extrapo;
			extra2po->MissileType = MISSILE5TYPE;
			extra2po->NumMissiles = SwarmPackNumMissls;
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
			extra2po->NumMissiles = ProxPackNumMines;
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
}


// kill extra that immediately follows passed node in list --------------------
//
void OBJ_KillExtra( ExtraObject *precnode, int collected )
{
	ASSERT( precnode != NULL );

	ExtraObject *extra = (ExtraObject *) precnode->NextObj;
	ASSERT( extra != NULL );

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

#endif

	// unlink from list
	precnode->NextObj = extra->NextObj;

	// free extra
	FreeObjectMem( extra );
}


// animate an extra -----------------------------------------------------------
//
void OBJ_AnimateExtra( ExtraObject *extrapo )
{
	ASSERT( extrapo != NULL );
	ASSERT( OBJECT_TYPE_EXTRA( extrapo ) );

	// do self rotation
	ObjRotX( extrapo->ObjPosition, extrapo->SelfRotX * CurScreenRefFrames );
	ObjRotY( extrapo->ObjPosition, extrapo->SelfRotY * CurScreenRefFrames );
	ObjRotZ( extrapo->ObjPosition, extrapo->SelfRotZ * CurScreenRefFrames );

	// drift extras away from ship explosion center
	if ( extrapo->DriftTimeout == 0 )
		return;

	extrapo->DriftTimeout -= CurScreenRefFrames;
	if ( extrapo->DriftTimeout < 0 )
		extrapo->DriftTimeout = 0;

	extrapo->ObjPosition[ 0 ][ 3 ] += extrapo->DriftVec.X * CurScreenRefFrames;
	extrapo->ObjPosition[ 1 ][ 3 ] += extrapo->DriftVec.Y * CurScreenRefFrames;
	extrapo->ObjPosition[ 2 ][ 3 ] += extrapo->DriftVec.Z * CurScreenRefFrames;
}


// place extras in vicinity of local ship object ------------------------------
//
void OBJ_DoExtraPlacement()
{
	//NOTE:
	// this function gets called once per frame
	// from within the gameloop to maintain the
	// regular creation of extras (G_MAIN.C).

#ifndef NO_EXTRA_MAINTENANCE

	// no extra creation in GMSV mode
	//FIXME: this still created extras until we are connected...
	if ( NET_ConnectedGMSV() ) {
		return;
	}

	// don't create extras during demo replay
	if ( !AUX_CREATE_EXTRAS_DURING_REPLAY && DEMO_ReplayActive() )
		return;

	// check if enough space in RE_List
	ASSERT( sizeof( RE_CreateExtra ) >= sizeof( RE_ParticleObject ) );
	if ( !NET_RmEvAllowed( RE_CREATEEXTRA ) )
		return;

	// determine upper bound to number of extras
	ASSERT( ( NumRemPlayers >= 0 ) && ( NumRemPlayers <= MAX_NET_PROTO_PLAYERS ) );
	int maxextras = MaxNumExtras;
	if ( NetConnected ) {
		ASSERT( NumRemPlayers > 0 );
		maxextras *= NumRemPlayers;
	}

	// create random extra object
	if ( ( CurrentNumExtras + CurrentNumPrtExtras ) < maxextras ) {

		int r = RAND() % 100;
		if ( r > ExtraProbability )
			return;

		int x = ( RAND() % MaxExtraArea ) - MaxExtraArea / 2;
		int y = ( RAND() % MaxExtraArea ) - MaxExtraArea / 2;
		int z = ( RAND() % MaxExtraArea ) - MaxExtraArea / 2;

		x += ( x < 0 ) ? -MinExtraDist : MinExtraDist;
		y += ( y < 0 ) ? -MinExtraDist : MinExtraDist;
		z += ( z < 0 ) ? -MinExtraDist : MinExtraDist;

		Xmatrx startm;
		MakeIdMatrx( startm );
		startm[ 0 ][ 3 ] = INT_TO_GEOMV( x ) + MyShip->ObjPosition[ 0 ][ 3 ];
		startm[ 1 ][ 3 ] = INT_TO_GEOMV( y ) + MyShip->ObjPosition[ 1 ][ 3 ];
		startm[ 2 ][ 3 ] = INT_TO_GEOMV( z ) + MyShip->ObjPosition[ 2 ][ 3 ];

		int probsum  = 0;
		int objclass = RAND() % 100;

		// energy field
		if ( objclass < ( probsum += ProbEnergyField ) ) {

			// create energy field particle object
			Vertex3 origin;
			FetchTVector( startm, &origin );
			SFX_CreateEnergyField( origin );

			// insert remote event
			NET_RmEvParticleObject( POBJ_ENERGYFIELD, origin );

			// record create event if recording active
			Record_EnergyFieldCreation( origin );

			return;
		}

		// helix cannon
		if ( objclass < ( probsum += ProbHelixCannon ) ) {
			objclass = HELIX_DEVICE_CLASS;

		// lightning device
		} else if ( objclass < ( probsum += ProbLightningDevice ) ) {
			objclass = LIGHTNING_DEVICE_CLASS;

		// photon cannon
		} else if ( objclass < ( probsum += ProbPhotonCannon ) ) {
			objclass = PHOTON_DEVICE_CLASS;

		// proximity mine pack
		} else if ( objclass < ( probsum += ProbProximityMine ) ) {
			objclass = MINE_PACK_CLASS;

		// laser upgrade
		} else if ( objclass < ( probsum += ProbLaserUpgrade ) ) {

			probsum = 0;
			int lasertype = RAND() % 100;
			if ( lasertype < ( probsum += ProbLaserUpgrade1 ) )
				objclass = LASERUPGRADE1_CLASS;
			else // if ( lasertype < ( probsum += ProbLaserUpgrade2 ) )
				objclass = LASERUPGRADE2_CLASS;

		// missile pack
		} else if ( objclass < ( probsum += ProbMissilePack ) ) {

			probsum = 0;
			int misstype = RAND() % 100;
			if ( misstype < ( probsum += ProbDumbMissPack ) )
				objclass = DUMB_PACK_CLASS;
			else if ( misstype < ( probsum += ProbHomMissPack ) )
				objclass = GUIDE_PACK_CLASS;
			else // if ( misstype < ( probsum += ProbSwarmMissPack ) )
				objclass = SWARM_PACK_CLASS;

		// repair extra
		} else if ( objclass < ( probsum += ProbRepairExtra ) ) {
			objclass = REPAIR_EXTRA_CLASS;

		// afterburner
		} else if ( objclass < ( probsum += ProbAfterburner ) ) {
			objclass = AFTERBURNER_DEVICE_CLASS;

		// holo decoy
		} else if ( objclass < ( probsum += ProbHoloDecoy ) ) {
			objclass = DECOY_DEVICE_CLASS;

		// invisibility
		} else if ( objclass < ( probsum += ProbInvisibility ) ) {
			objclass = INVISIBILITY_CLASS;

		// invulnerability
		} else if ( objclass < ( probsum += ProbInvulnerability ) ) {
			objclass = INVULNERABILITY_CLASS;

		// emp upgrade 1
		} else if ( objclass < ( probsum += ProbEmpUpgrade1 ) ) {
		
			if ( ExtraClasses[ EXTRAINDX_EMPUPGRADE1 ] == CLASS_ID_INVALID ) {
				ExtraClasses[ EXTRAINDX_EMPUPGRADE1 ] = OBJ_FetchObjectClassId( OBJCLASSNAME_DEVICE_EMP_UPGRADE_1 );
			}

			objclass = ExtraClasses[ EXTRAINDX_EMPUPGRADE1 ];			
			
		// emp upgrade 2
		} else if ( objclass < ( probsum += ProbEmpUpgrade2 ) ) {
		
			if ( ExtraClasses[ EXTRAINDX_EMPUPGRADE2 ] == CLASS_ID_INVALID ) {
				ExtraClasses[ EXTRAINDX_EMPUPGRADE2 ] = OBJ_FetchObjectClassId( OBJCLASSNAME_DEVICE_EMP_UPGRADE_2 );
			}

			objclass = ExtraClasses[ EXTRAINDX_EMPUPGRADE2 ];			

		// energy extra
		} else {
			objclass = ENERGY_EXTRA_CLASS;
		}

		// create object
		ExtraObject *extrapo = (ExtraObject *) CreateObject( objclass, startm );
		OBJ_FillExtraMemberVars( extrapo );

		// insert remote event
		NET_RmEvExtra( extrapo );

		// record create event if recording active
		Record_ExtraCreation( extrapo );
	}

#endif

}


// define area in which extras around ship will be placed ---------------------
//
#define expl_MaxExtraArea	30
#define expl_MinExtraDist	5

#define expl_DriftSpeed		0.0012f


// place extras after ship was shot down --------------------------------------
//
PRIVATE
void PlaceShipExtras( int num, int objclass, ShipObject *shippo )
{
	ASSERT( num >= 0 );
	ASSERT( objclass >= 0 );
	ASSERT( objclass < NumObjClasses );
	ASSERT( shippo != NULL );

	for ( ; num > 0; num-- ) {

		if ( !NET_RmEvAllowed( RE_CREATEEXTRA ) )
			return;

		// do random placement of extras in near vicinity of ship
		int x = ( RAND() % expl_MaxExtraArea ) - expl_MaxExtraArea / 2;
		int y = ( RAND() % expl_MaxExtraArea ) - expl_MaxExtraArea / 2;
		int z = ( RAND() % expl_MaxExtraArea ) - expl_MaxExtraArea / 2;

		x += ( x < 0 ) ? -expl_MinExtraDist : expl_MinExtraDist;
		y += ( y < 0 ) ? -expl_MinExtraDist : expl_MinExtraDist;
		z += ( z < 0 ) ? -expl_MinExtraDist : expl_MinExtraDist;

		Xmatrx startm;
		MakeIdMatrx( startm );
		startm[ 0 ][ 3 ] = INT_TO_GEOMV( x ) + shippo->ObjPosition[ 0 ][ 3 ];
		startm[ 1 ][ 3 ] = INT_TO_GEOMV( y ) + shippo->ObjPosition[ 1 ][ 3 ];
		startm[ 2 ][ 3 ] = INT_TO_GEOMV( z ) + shippo->ObjPosition[ 2 ][ 3 ];

		// create object
		ExtraObject *extrapo = (ExtraObject *) CreateObject( objclass, startm );
		OBJ_FillExtraMemberVars( extrapo );

		// initialize drift timeout
		extrapo->DriftTimeout = DEFAULT_DRIFT_TIMEOUT;

		// calculate drift direction vector
		extrapo->DriftVec.X = FLOAT_TO_GEOMV( x * expl_DriftSpeed );
		extrapo->DriftVec.Y = FLOAT_TO_GEOMV( y * expl_DriftSpeed );
		extrapo->DriftVec.Z = FLOAT_TO_GEOMV( z * expl_DriftSpeed );

		//TODO:
		// randomly alter self rotation.

		ASSERT( extrapo->ObjectClass != MINE_CLASS_1 );
		ASSERT( extrapo->ObjectType != MINE1TYPE );

		// insert remote event
		NET_RmEvExtra( extrapo );

		// record create event if recording active
		Record_ExtraCreation( extrapo );
	}
}


// create extras after ship was shot down -------------------------------------
//
void OBJ_CreateShipExtras( ShipObject *shippo )
{
	// done on server in GMSV mode
	if( !NET_ConnectedPEER() ) {
		return;
	}

	ASSERT( shippo != NULL );

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

	#define MAX_DUMBPACKS		2
	#define MAX_GUIDEPACKS		2
	#define MAX_MINEPACKS		1

	// create dumb missile packs
	if ( shippo->NumMissls > 0 ) {

		int num_missilepacks = ( shippo->NumMissls / DumbPackNumMissls );
		if ( num_missilepacks > MAX_DUMBPACKS )
			num_missilepacks = MAX_DUMBPACKS;

		PlaceShipExtras( num_missilepacks, DUMB_PACK_CLASS, shippo );
	}

	// create guided missile packs
	if ( shippo->NumHomMissls > 0 ) {

		int num_guidedmissilepacks = ( shippo->NumHomMissls / HomPackNumMissls );
		if ( num_guidedmissilepacks > MAX_GUIDEPACKS )
			num_guidedmissilepacks = MAX_GUIDEPACKS;

		PlaceShipExtras( num_guidedmissilepacks, GUIDE_PACK_CLASS, shippo );
	}

	// create proximity mine packs
	if ( shippo->NumMines > 0 ) {

		int num_minepacks = ( shippo->NumMines / ProxPackNumMines );
		if ( num_minepacks > MAX_MINEPACKS )
			num_minepacks = MAX_MINEPACKS;

		PlaceShipExtras( num_minepacks, MINE_PACK_CLASS, shippo );
	}

	// create lightning device
	if ( OBJ_DeviceAvailable( shippo, WPMASK_CANNON_LIGHTNING ) ) {

		PlaceShipExtras( 1, LIGHTNING_DEVICE_CLASS, shippo );
	}

	// create helix cannon
	if ( OBJ_DeviceAvailable( shippo, WPMASK_CANNON_HELIX ) ) {

		PlaceShipExtras( 1, HELIX_DEVICE_CLASS, shippo );
	}

	// create photon cannon
	if ( OBJ_DeviceAvailable( shippo, WPMASK_CANNON_PHOTON ) ) {

		PlaceShipExtras( 1, PHOTON_DEVICE_CLASS, shippo );
	}
	
	// create laser upgrade 1
	if ( shippo->Specials & SPMASK_LASER_UPGRADE_1 ) {
	
		PlaceShipExtras( 1, LASERUPGRADE1_CLASS, shippo );
	}

	// create laser upgrade 2
	if ( shippo->Specials & SPMASK_LASER_UPGRADE_2 ) {
	
		PlaceShipExtras( 1, LASERUPGRADE2_CLASS, shippo );
	}
/*
	// create emp upgrade 1
	if ( shippo->Specials & SPMASK_EMP_UPGRADE_1 ) {
	
		PlaceShipExtras( 1, EMPUPGRADE1_CLASS, shippo );
	}

	// create emp upgrade 2
	if ( shippo->Specials & SPMASK_EMP_UPGRADE_2 ) {
	
		PlaceShipExtras( 1, EMPUPGRADE2_CLASS, shippo );
	}
*/	
	// create energy extra if ship still has enough energy
	if ( shippo->CurEnergy >= EnergyExtraBoost ) {
	
		PlaceShipExtras( 1, ENERGY_EXTRA_CLASS, shippo );
	}

}



