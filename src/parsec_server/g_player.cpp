/*
 * PARSEC - Player logic -
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:45 $
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

// utility headers
#include "utl_list.h"

// local module header
#include "g_player.h"

// proprietary module headers
#include "con_aux_sv.h"
//#include "g_extra.h"
//#include "net_game_sv.h"
//#include "net_util.h"
//#include "obj_clas.h"
//#include "obj_creg.h"
//#include "obj_name.h"
//#include "od_props.h"
//#include "od_class.h"
#include "e_simplayerinfo.h"
#include "g_main_sv.h"
//#include "g_stgate.h"
//#include "e_connmanager.h"
#include "e_gameserver.h"
#include "e_simulator.h"
//#include "sys_refframe_sv.h"

#include "g_emp.h"

float	spreadfire_ref_z			= 1.0f;

// reset all game variables ---------------------------------------------------
//
void G_Player::ResetGameVars()
{
      m_nKills                = 0;
      m_nDeaths               = 0;
      m_nPoints               = 0;
      m_nLastUnjoinFlag       = USER_EXIT;
      m_nLastKiller           = KILLERID_UNKNOWN;

      m_FireDisableFrames     = 1;
      m_MissileDisableFrames  = 1;

      m_CurGun                = 0;
      m_CurLauncher           = 0;
      m_StateSync             = 0; 
}


// reset all fields to defaults ( not connected ) -----------------------------
//
void G_Player::Reset()
{
      ResetGameVars();
      m_nClientID	= -1;
      m_pSimPlayerInfo	= NULL;
}


// set the player status to connected -----------------------------------------
//
void G_Player::Connect( int nClientID )
{
      Reset();
      m_nClientID	= nClientID;
      m_pSimPlayerInfo = TheSimulator->GetSimPlayerInfo( nClientID );
}


// set the player status to disconnected --------------------------------------
//
void G_Player::Disconnect()
{
      Reset();
}


// user fired laser -----------------------------------------------------------
//
void G_Player::FireLaser()
{
      // check whether laser firing is valid
      if ( m_FireDisableFrames > 0 ) {
       //       MSGOUT( "G_Player::FireLaser(): laser firing disabled: m_FireDisableFrames: %d", m_FireDisableFrames );
              return;
      }
     // MSGOUT( "G_Player::FireLaser(): laser firing OK: m_FireDisableFrames: %d", m_FireDisableFrames );

      // create laser
      _OBJ_ShootLaser();

      ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
      if ( ( m_FireDisableFrames += pShip->FireDisableDelay ) <= 0 ) {
              m_FireDisableFrames = 1;
      }
}

//user fired helix cannon

void G_Player::FireHelix()
{
	ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
    if ( ( pShip->WeaponsActive & WPMASK_CANNON_HELIX ) == 0 ) {
		_WFX_ActivateHelix();
	}
}


// user fired lightning device ------------------------------------------------
//
void G_Player::FireLightning()
{
    ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
	if ( ( pShip->WeaponsActive & WPMASK_CANNON_LIGHTNING ) == 0 ) {
		_WFX_ActivateLightning();
	}
}


// user fired photon cannon ---------------------------------------------------
//
void G_Player::FirePhoton()
{
    ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
	if ( (pShip->WeaponsActive & WPMASK_CANNON_PHOTON ) == 0 ) {
		_WFX_ActivatePhoton();
	}
}

// user launched dumb missle -----------------------------------------------------------
//
void G_Player::LaunchMissile()
{
      // check whether missle launching is valid
      if ( m_MissileDisableFrames > 0 ) {
         //     MSGOUT( "G_Player::LaunchMissle(): missile launching disabled: m_MissileDisableFrames: %d", m_MissileDisableFrames );
              return;
      }
    //  MSGOUT( "G_Player::LaunchMissile(): Missile Launching OK: m_MissileDisableFrames: %d", m_MissileDisableFrames );

      // create missle
      _OBJ_LaunchMissile();

      ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
      if ( ( m_MissileDisableFrames += pShip->MissileDisableDelay ) <= 0 ) {
              m_MissileDisableFrames = 1;
      }
}
// user launched homing missle -----------------------------------------------------------
//
void G_Player::LaunchHomingMissile(dword launcher, dword targetid)
{

      // check whether missle launching is valid
      if ( m_MissileDisableFrames > 0 ) {
      //        MSGOUT( "G_Player::LaunchMissle(): missile launching disabled: m_MissileDisableFrames: %d", m_MissileDisableFrames );
              return;
      }
      //MSGOUT( "G_Player::LaunchHomingMissile(): Missile Launching OK: m_MissileDisableFrames: %d", m_MissileDisableFrames );

      // create missle
      _OBJ_LaunchHomingMissile(launcher, targetid);

      ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
      if ( ( m_MissileDisableFrames += pShip->MissileDisableDelay ) <= 0 ) {
              m_MissileDisableFrames = 1;
      }
}

void G_Player::LaunchMine()
{

	//MSGOUT( "G_Player::LaunchMine(); Mine Launch Ok");

	// create the mine object
	_OBJ_LaunchMine();
}

void G_Player::LaunchSwarm( dword targetid )
{
      // check whether missle launching is valid
      if ( m_MissileDisableFrames > 0 ) {
              //MSGOUT( "G_Player::LaunchSwarm(): Swarm missile launching disabled: m_MissileDisableFrames: %d", m_MissileDisableFrames );
              return;
      }
      //MSGOUT( "G_Player::LaunchSwarm(): Swarm Missile Launching OK: m_MissileDisableFrames: %d", m_MissileDisableFrames );

      // create missle
      _OBJ_LaunchSwarm( targetid );

      ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
      if ( ( m_MissileDisableFrames += pShip->MissileDisableDelay ) <= 0 ) {
              m_MissileDisableFrames = 1;
      }
}

// create laser originating from specified ship -------------------------------
//
void G_Player::_OBJ_ShootLaser()
{
	//NOTE: based on OBJ_GAME::OBJ_ShootLaser

	ASSERT( m_pSimPlayerInfo != NULL );
	ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
	ASSERT( pShip != NULL );

	//NOTE:
	// C: this function is called by INP_USER::User_CheckGunFire().
	// S: this function is called by G_Input::User_FireLaser

	//FIXME: GAMEVAR ( property of ship )
	#define MIN_LASER_ENERGY		20

#ifdef PARSEC_SERVER

	// check for availability
	if ( !TheGame->OBJ_DeviceAvailable( pShip, WPMASK_CANNON_LASER ) ) {
		return;
	}

#else
	// check if enough space in RE_List
	if ( !NET_RmEvAllowed( RE_CREATELASER ) )
		return;

	// check for availability
	if ( !OBJ_DeviceAvailable( pShip, WPMASK_CANNON_LASER ) ) {
		if ( pShip == MyShip ) {
			ShowMessage( no_standard_str );
		}
		return;
	}
#endif // PARSEC_SERVER

	// determine current laser upgrade level
	int curlevel = 0;
	if ( pShip->Specials & SPMASK_LASER_UPGRADE_2 ) {
		curlevel = 2;
	} else if ( pShip->Specials & SPMASK_LASER_UPGRADE_1 ) {
		curlevel = 1;
	}

	// sequence of gun (laser) barrels ----------------------------------------
	//
	#define MAX_GUN_BARRELS		4
	static int gun_barrels_sequence[ MAX_GUN_BARRELS ]	= { 0, 3, 1, 2 };

	int barrel = gun_barrels_sequence[ m_CurGun % MAX_GUN_BARRELS ];
	dword laserclass = pShip->Laser1_Class[ curlevel ][ barrel ];

	if ( !SV_CHEAT_ENERGY_CHECKS ) {

		// check if enough energy to shoot laser
		//FIXME: TheWorld->ObjClasses ??????????????
		int energyneeded = ( (LaserObject*)TheWorld->ObjClasses[ laserclass ] )->EnergyNeeded;
		if ( ( pShip->CurEnergy - energyneeded ) < MIN_LASER_ENERGY ) {

#ifdef PARSEC_SERVER
			//MSGOUT( "G_Player::_OBJ_ShootLaser(): client %d low energy", m_nClientID );
#else // !PARSEC_SERVER
			if ( pShip == MyShip ) {
				ShowMessage( low_energy_str );
				AUD_LowEnergy();
			}
#endif // !PARSEC_SERVER
			
			return;
		}
		pShip->CurEnergy -= energyneeded;
		//MSGOUT( "G_Player::_OBJ_ShootLaser(): client %d energy after shot: %d", m_nClientID, pShip->CurEnergy );
	}

	// create actual laser object(s)
	TheGame->OBJ_CreateLaserObject( pShip, curlevel, barrel, m_nClientID );
	m_CurGun++;

	if ( curlevel == 2 ) {
		barrel = gun_barrels_sequence[ m_CurGun % MAX_GUN_BARRELS ];
		TheGame->OBJ_CreateLaserObject( pShip, curlevel, barrel, m_nClientID );
		m_CurGun++;
	}

#ifdef PARSEC_CLIENT
	// play sound effect
	AUD_Laser( laserpo );
#endif // PARSEC_CLIENT
}

// helix props ----------------------------------------------------------------
//
bams_t  HelixBamsInc			= 0x1500;
int     HelixRefFrames			= 10;                     // create one particle every ... frames
geomv_t HelixRadius				= INT_TO_GEOMV( 10 );


void G_Player::_WFX_ActivateHelix()
{
	ASSERT( m_pSimPlayerInfo != NULL );
	ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
	ASSERT( pShip != NULL );
    
	// check if helix available
	if ( !TheGame->OBJ_DeviceAvailable( pShip, WPMASK_CANNON_HELIX ) ) {
		MSGOUT("G_PLAYER::_WFX_ActivateHelix(): client %d trying to fire Helix cannon illegally",m_nClientID);
		return;
	}
    
	// check if enough energy to shoot helix
    if ( pShip->CurEnergy < MIN_HELIX_ENERGY + HELIX_ENERGY_CONSUMPTION ) {
		//MSGOUT("G_PLAYER::_WFX_ActivateHelix(): Not enough energy to activate Helix cannon");
		return;
	}
    
	// set active flag
	pShip->WeaponsActive |= WPMASK_CANNON_HELIX;
   // MSGOUT("G_PLAYER::_WFX_ActivateHelix(): client %d fired",m_nClientID);
    //Create the object/event somehow    
}

// create helix particles for current frame -----------------------------------
//
int G_Player::_WFX_MaintainHelix( ShipObject *shippo, int playerid )
{
	ASSERT( shippo != NULL );
    
	//int bitmap	  = SPREADFIRE_BM_INDX;
	int color	  = SPREADFIRE_PARTICLE_COLOR;
	float ref_z = spreadfire_ref_z;
	int sizebound = partbitmap_size_bound;
	int lifetime  = HELIX_LIFETIME;
    
	pextinfo_s *pextinfo = NULL;
    
	// calc number of particles according to time passed (remember remainder)
	refframe_t numrefframes		  = TheSimulator->GetThisFrameRefFrames() + shippo->helix_refframes_delta;
	int numparticles			  = numrefframes / HelixRefFrames;
	shippo->helix_refframes_delta = numrefframes % HelixRefFrames;
    Vector3 dirvec;
	fixed_t speed = HELIX_SPEED + shippo->CurSpeed;
	DirVctMUL( shippo->ObjPosition, FIXED_TO_GEOMV( speed ), &dirvec );
    
	for ( int curp = 0; curp < numparticles; curp++ ) {
        
		// check if enough energy to shoot helix
		if ( shippo->CurEnergy < MIN_HELIX_ENERGY + HELIX_ENERGY_CONSUMPTION ) {
            
			_WFX_DeactivateHelix();
            
            return 0;
		}
		shippo->CurEnergy -= HELIX_ENERGY_CONSUMPTION;
        
		sincosval_s resultp;
		GetSinCos( shippo->HelixCurBams, &resultp );
        
		// create oldest particle first
		int		invcurp = numparticles - curp - 1;
		fixed_t timefrm = invcurp * HelixRefFrames + helix_refframes_delta;
		fixed_t timepos = timefrm * shippo->HelixSpeed;
        
		// create one full frame set back because the current frame will
		// be added by the linear particle animation code in the same frame
		timepos -= speed * TheSimulator->GetThisFrameRefFrames();
        
		Vector3 pofsvec;
		pofsvec.X = GEOMV_MUL( HelixRadius, resultp.sinval );
		pofsvec.Y = GEOMV_MUL( HelixRadius, resultp.cosval );
		pofsvec.Z = FIXED_TO_GEOMV( timepos );
        
		// first particle ---
		Vertex3 object_space;
		object_space.X = shippo->Helix_X + pofsvec.X;
		object_space.Y = shippo->Helix_Y + pofsvec.Y;
		object_space.Z = shippo->Helix_Z + pofsvec.Z;
        
		Vertex3 world_space;
		MtxVctMUL( shippo->ObjPosition, &object_space, &world_space );
        
		particle_s particle;
		TheWorld->PRT_InitParticle( particle, color, sizebound,
                         ref_z, &world_space, &dirvec,
                         lifetime, playerid, pextinfo );
		particle.flags |= PARTICLE_COLLISION;
		TheWorld->PRT_CreateLinearParticle( particle );
        
		// second particle ---
		object_space.X = shippo->Helix_X - pofsvec.X;
#ifdef SECOND_HELIX
		object_space.Y = shippo->Helix_Y - pofsvec.Y;
#else
		object_space.Y = shippo->Helix_Y + pofsvec.Y;
#endif
		MtxVctMUL( shippo->ObjPosition, &object_space, &world_space );
        
		TheWorld->PRT_InitParticle( particle, color, sizebound,
                         ref_z, &world_space, &dirvec,
                         lifetime, playerid, pextinfo );
		particle.flags |= PARTICLE_COLLISION;
		particle.flags |= PARTICLE_IS_HELIX;
		TheWorld->PRT_CreateLinearParticle( particle );
        
		shippo->HelixCurBams = ( shippo->HelixCurBams + HelixBamsInc ) & 0xffff;
	}
    
	return 1;
}

void G_Player::_WFX_DeactivateHelix()
{
	ASSERT( m_pSimPlayerInfo != NULL );
	ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
	ASSERT( pShip != NULL );
    
	// make sure helix cannon is inactive
	if ( pShip->WeaponsActive & WPMASK_CANNON_HELIX ) {
        
		// reset activation flag
        pShip->WeaponsActive &= ~WPMASK_CANNON_HELIX;
        //MSGOUT("G_PLAYER::_WFX_DeactivateHelix(): client %d stopped firing",m_nClientID);
        
	}
}

// remote player activated lightning device -----------------------------------
//
void G_Player::_WFX_ActivateLightning()
{
	ASSERT( m_pSimPlayerInfo != NULL );
	ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
	ASSERT( pShip != NULL );
    
    
    if ( !TheGame->OBJ_DeviceAvailable( pShip, WPMASK_CANNON_LIGHTNING ) ) {
		MSGOUT("G_PLAYER::_WFX_ActivateLightning(): client %d trying to fire Lightning cannon illegally",m_nClientID);
		return;
	}
    dword energy_consumption = pShip->CurEnergyFrac + ( TheSimulator->GetThisFrameRefFrames() * LIGHTNING_ENERGY_CONSUMPTION );
    
	// check if enough energy to shoot lightning
	if ( (dword)pShip->CurEnergy < ( MIN_LIGHTNING_ENERGY + ( energy_consumption >> 16 ) ) ) {
       MSGOUT("G_PLAYER::_WFX_ActivateLightning(): client %d low energy",m_nClientID);
       return;
	}
    
    
	// make sure lightning device is active
	if ( ( pShip->WeaponsActive & WPMASK_CANNON_LIGHTNING ) == 0 ) {
        
		if ( TheWorld->CreateLightningParticles( pShip, m_nClientID ) != NULL ) {
			pShip->WeaponsActive |= WPMASK_CANNON_LIGHTNING;
            MSGOUT("G_PLAYER::_WFX_ActivateLightning(): client %d fired",m_nClientID);
		}
    }
}


// remote player deactivated lightning device ---------------------------------
//
void G_Player::_WFX_DeactivateLightning()
{
	ASSERT( m_pSimPlayerInfo != NULL );
	ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
	ASSERT( pShip != NULL );
    
	// make sure lightning device is inactive
	if ( ( pShip->WeaponsActive & WPMASK_CANNON_LIGHTNING ) != 0 ) {
        // remove lightning particles
        TheWorld->PRT_DeleteAttachedClustersOfType( pShip, SAT_LIGHTNING );
        // reset activation flag
        pShip->WeaponsActive &= ~WPMASK_CANNON_LIGHTNING;
        MSGOUT("G_PLAYER::_WFX_DeactivateLightning(): client %d fired",m_nClientID);
	    //Remote event goes here
    }
}

// check whether to turn off lightning due to too little energy ---------------
//
void G_Player::WFX_MaintainLightning( ShipObject *shippo)
{
	ASSERT( shippo != NULL );
	ASSERT( shippo->WeaponsActive & WPMASK_CANNON_LIGHTNING );
    
	dword energy_consumption = shippo->CurEnergyFrac +
    ( TheSimulator->GetThisFrameRefFrames() * LIGHTNING_ENERGY_CONSUMPTION );
    
	// check if enough energy to shoot lightning
	if ( (dword)shippo->CurEnergy < ( MIN_LIGHTNING_ENERGY + ( energy_consumption >> 16 ) ) ) {
		_WFX_DeactivateLightning();
	} else {
        
		shippo->CurEnergyFrac = ( energy_consumption & 0xffff );
		shippo->CurEnergy    -= ( energy_consumption >> 16 );
	}
}

// activate photon cannon of local ship ----------------------------------------
//
void G_Player::_WFX_ActivatePhoton()
{
	ASSERT( m_pSimPlayerInfo != NULL );
	ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
	ASSERT( pShip != NULL );
    
    // check if photon available
    if ( !TheGame->OBJ_DeviceAvailable( pShip, WPMASK_CANNON_PHOTON ) ) {
        //MSGOUT("G_Player::_WFX_ActivatePhoton: client %d illigally tried to fire Photon Cannon\n",m_nClientID);
        return;
	}
    
    // check if enough energy to shoot photon
    if ( pShip->CurEnergy < MIN_PHOTON_ENERGY ) {
		MSGOUT("G_Player::_WFX_ActivatePhoton: client %d low energy",m_nClientID);
		return;
	}
    
    // check if photon cannon still firing
    if ( TheWorld->PRT_ObjectHasAttachedClustersOfType( pShip, SAT_PHOTON ) ) {
        return;
    }
     
    //Not sure yet if I even need to simulate this part below
    // create particle sphere
    if ( !TheWorld->CreatePhotonSphere( pShip ) ) {
        return;
    }
    
    // set active flag
    pShip->WeaponsActive |= WPMASK_CANNON_PHOTON;
    MSGOUT("G_Player::_WFX_ActivatePhoton: client %d charging photon cannon",m_nClientID);
    
}

// deactivate photon cannon of specified ship ----------------------------------
//
void G_Player::_WFX_DeactivatePhoton()
{
    // anim type for photon particles (no "real" sphere animtype)
    #define SAT_PHOTON                          0x00000008

	ASSERT( m_pSimPlayerInfo != NULL );
	ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
    ASSERT( pShip != NULL );
    
	// send remote event to switch photon off
    
	// reset activation flag
    pShip->WeaponsActive &= ~WPMASK_CANNON_PHOTON;
    MSGOUT("G_Player::_WFX_DectivatePhoton: client %d fired photon cannon",m_nClientID);
    
    // start firing
    photon_sphere_pcluster_s* cluster = (photon_sphere_pcluster_s *) TheWorld->PRT_ObjectHasAttachedClustersOfType( pShip, SAT_PHOTON );
    if(cluster != NULL)
       cluster->firing = TRUE;
    
}

// create missile originating from specified position -------------------------
//
void G_Player::_OBJ_LaunchMissile()
{
	ASSERT( m_pSimPlayerInfo != NULL );
	ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
	ASSERT( pShip != NULL );

	//NOTE: based on OBJ_GAME::OBJ_LaunchMissle


	// check ammo
	if ( pShip->NumMissls <= 0 ) {
		return;
	}
	pShip->NumMissls--;
	
	
	#define MAX_MISSILE_BARRELS_MASK 	4
	static int missile_barrel_sequence[ MAX_MISSILE_BARRELS_MASK ]	= { 0, 3, 1, 2 };
	
	// select correct barrel
	int barrel       = missile_barrel_sequence[ m_CurLauncher%MAX_MISSILE_BARRELS_MASK ];
	
	TheGame->OBJ_CreateMissileObject( pShip, barrel, m_nClientID );
	m_CurLauncher++;
#ifdef PARSEC_CLIENT
	// insert remote event
	NET_RmEvMissile( missilepo, 0 );

	// record create event if recording active
	Record_MissileCreation( missilepo );

	// play sound effect
	AUD_Missile( missilepo );
#endif
}


// create homing missile originating from specified position ------------------
//
void G_Player::_OBJ_LaunchHomingMissile( dword launcher, dword targetid )
{
	ASSERT( m_pSimPlayerInfo != NULL );
	ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
	ASSERT( pShip != NULL );


	// check ammo
	if ( pShip->NumHomMissls <= 0 ) {
            return;
	}
	
	
	/* check if any target locked
	if (!targetid ) {
		MSGOUT("NO TARGET Targetid: %d", targetid); //This is server side, to be here a client has already fired at a target
		return;
	}*/ 
	pShip->NumHomMissls--;


	#define MAX_MISSILE_BARRELS_MASK 	4
	static int missile_barrel_sequence[ MAX_MISSILE_BARRELS_MASK ]	= { 0, 3, 1, 2 };
	
	// select correct barrel
	int barrel       = missile_barrel_sequence[ m_CurLauncher%MAX_MISSILE_BARRELS_MASK ];
	
	TheGame->OBJ_CreateHomingMissileObject( pShip, barrel, m_nClientID, targetid );
	m_CurLauncher++;



}


// create mine object ---------------------------------------------------------
//
void G_Player::_OBJ_LaunchMine()
{
	ASSERT( m_pSimPlayerInfo != NULL );
	ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
	ASSERT( pShip != NULL );

	// check ammo
	if ( pShip->NumMines <= 0 ) {
		return;
	}
	pShip->NumMines--;
	TheGame->OBJ_CreateMineObject( pShip, m_nClientID );
	

}


// create swarm missiles ------------------------------------------------------
//
void G_Player::_OBJ_LaunchSwarm( dword targetid )
{

	ASSERT( m_pSimPlayerInfo != NULL );
	ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
	ASSERT( pShip != NULL );

	// check ammo
	if ( pShip->NumPartMissls <= 0 ) {
		return;
	}

	TheGame->OBJ_CreateSwarm(pShip, m_nClientID, targetid);
    
   	pShip->NumPartMissls--;

}

void G_Player::FireEMP(byte Upgradelevel) {
	ASSERT( m_pSimPlayerInfo != NULL );
	ShipObject* pShip = m_pSimPlayerInfo->GetShipObject();
	ASSERT( pShip != NULL );
	int curdelay = 0;
	int energy_consumption = emp_energy[ Upgradelevel ] * emp_waves[ Upgradelevel ];

	// check if enough energy to shoot emp
	if ( pShip->CurEnergy >= energy_consumption  ) {

		pShip->CurEnergy -= energy_consumption;

		for ( int i = 0; i < emp_waves[ Upgradelevel ]; i++ ) {
			CreateEmp( pShip, curdelay, 0, Upgradelevel, m_nClientID );
			curdelay += emp_delay[ Upgradelevel ];
		}
	}
}

// record a kill --------------------------------------------------------------
//
void G_Player::RecordKill() 
{
	if ( m_pSimPlayerInfo->IsPlayerJoined() ) {
		m_nKills++;
	}
}


// record a death -------------------------------------------------------------
//
void G_Player::RecordDeath( int nClientID_Killer ) 
{
	ASSERT( ( nClientID_Killer >= 0 ) && ( nClientID_Killer < MAX_NUM_CLIENTS ) );
	if ( m_pSimPlayerInfo->IsPlayerJoined() ) {
		m_nDeaths++;
		m_nLastUnjoinFlag	= SHIP_DOWNED;
		m_nLastKiller		= nClientID_Killer;
	}
}


// reset any death info for the player ----------------------------------------
//
void G_Player::ResetDeathInfo()
{
	m_nLastUnjoinFlag		= USER_EXIT;
	m_nLastKiller			= KILLERID_UNKNOWN;
}


// return the ship object assigned to the player ------------------------------
//
ShipObject* G_Player::GetShipObject() 
{ 
	ASSERT( m_pSimPlayerInfo != NULL );
	return m_pSimPlayerInfo->GetShipObject(); 
}

// maintain weapon firing delays ----------------------------------------------
//
void G_Player::MaintainWeaponDelays()
{
	// count down fire disable delay
	if ( m_FireDisableFrames > 0 ) {
		m_FireDisableFrames -= TheSimulator->GetThisFrameRefFrames();
	}
	if ( m_MissileDisableFrames > 0 ) {
		m_MissileDisableFrames -= TheSimulator->GetThisFrameRefFrames();
	}
}


