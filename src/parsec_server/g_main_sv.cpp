/*
 * PARSEC - Game logic - SERVER
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
#include "g_main_sv.h"

// proprietary module headers
#include "con_aux_sv.h"
#include "g_extra.h"
#include "g_player.h"
#include "net_game_sv.h"
//#include "net_util.h"
#include "obj_clas.h"
#include "obj_creg.h"
#include "obj_name.h"
#include "od_props.h"
#include "od_class.h"
#include "g_stgate.h"
#include "e_connmanager.h"
#include "e_gameserver.h"
#include "e_simnetoutput.h"
#include "e_simulator.h"
#include "sys_refframe_sv.h"
#include "g_emp.h"

#undef PShipObjects		
#undef LaserObjects		
#undef MisslObjects		
#undef ExtraObjects		
#undef CustmObjects		

#undef CreateObject		
#undef FreeObjList			
#undef KillAllObjects		
#undef FetchObject			
#undef FetchFirstShip		
#undef FetchFirstLaser		
#undef FetchFirstMissile	
#undef FetchFirstExtra		
#undef FetchFirstCustom	
#undef KillClassInstances	

#undef ObjClasses			


// ----------------------------------------------------------------------------
// G_TimeManagement methods 
// ----------------------------------------------------------------------------

// standard ctor --------------------------------------------------------------
//
G_TimeManagement::G_TimeManagement()
{
	Reset();

	// for an endless game, no timelimit
	// no kill limit.  Just a FFA game :-)
	// GameIsEndless=0;
}


// set the time limits --------------------------------------------------------
//
void G_TimeManagement::RealizeVariables()
{
	//FIXME: kill limit ?
	m_nSecGameTimeLimit		= SV_GAME_TIMELIMIT;
	m_nSecRestartTimeLimit	= SV_GAME_RESTART_TIMEOUT;
	m_GameEndRefFrames		= m_nSecGameTimeLimit * FRAME_MEASURE_TIMEBASE;
}


// reset the game time management ---------------------------------------------
//
void G_TimeManagement::Reset()
{
	m_GameRefFrames	= GAME_NOTSTARTEDYET;
	m_RefFrameBase	= 0;
}


// start a game ---------------------------------------------------------------
//
void G_TimeManagement::StartGame()
{
	m_GameRefFrames	= 0;
	m_RefFrameBase	= SYSs_GetRefFrameCount();
}


// stop the game ( time limit hit ) -------------------------------------------
//
void G_TimeManagement::StopGame_TimeLimit()
{
	m_GameRefFrames		= GAME_FINISHED_TIME;
	m_RefFrameBase		= SYSs_GetRefFrameCount();
	m_RestartRefFrames  = m_nSecRestartTimeLimit * FRAME_MEASURE_TIMEBASE;
}


// stop the game ( kill limit hit ) -------------------------------------------
//
void G_TimeManagement::StopGame_KillLimit()
{
	m_GameRefFrames		= GAME_FINISHED_KILLS;
	m_RefFrameBase		= SYSs_GetRefFrameCount();
	m_RestartRefFrames  = m_nSecRestartTimeLimit * FRAME_MEASURE_TIMEBASE;
}


// return whether the game is not yet started ---------------------------------
//
int G_TimeManagement::IsNotYetStarted() 
{ 
	return ( m_GameRefFrames == GAME_NOTSTARTEDYET ); 
}


// return whether currently a game is running ---------------------------------
//
int G_TimeManagement::IsGameRunning() 
{ 
	return ( m_GameRefFrames >= 0 ); 
}


// return whether the game is finished ----------------------------------------
//
int G_TimeManagement::IsGameFinished()
{
	return ( ( m_GameRefFrames == GAME_FINISHED_TIME  ) || ( m_GameRefFrames == GAME_FINISHED_KILLS ) );
}


// check whether the restart timeout is over ----------------------------------
//
int G_TimeManagement::IsRestartTimeoutOver()
{
	// check whether game is finished at all
	if ( !IsGameFinished() ) {
		return FALSE;
	}

	// maintain timeout
	refframe_t diff = SYSs_GetRefFrameCount() - m_RefFrameBase;
	m_RefFrameBase = SYSs_GetRefFrameCount();
	m_RestartRefFrames -= diff;

	// return whether timeout is over
	return ( m_RestartRefFrames <= 0 );
}


// check whether the game time limit is hit -----------------------------------
//
int G_TimeManagement::IsGameTimeLimitHit()
{
	if ( !IsGameRunning() ) {
		return FALSE;
	}

	// maintain timeout
	refframe_t diff = SYSs_GetRefFrameCount() - m_RefFrameBase;
	m_RefFrameBase = SYSs_GetRefFrameCount();
	m_GameRefFrames += diff;

	// check whether the game time is longer than the timelimit
	return ( m_GameRefFrames >= m_GameEndRefFrames );
}


// return the current gametime in secs or special gametime codes --------------
// this is sent to the clients
int G_TimeManagement::GetCurGameTime()
{
	// NOTE: the conversion to secs. is needed for distribution to the clients
	if ( IsGameRunning() ) {

		refframe_t timeleft = ( m_GameEndRefFrames - m_GameRefFrames );
		if ( timeleft < 0 ) {
			timeleft = 0;
		}

		return ( timeleft / FRAME_MEASURE_TIMEBASE );

	} else {
		ASSERT( ( m_GameRefFrames == GAME_NOTSTARTEDYET ) ||
				( m_GameRefFrames == GAME_FINISHED_TIME ) ||
				( m_GameRefFrames == GAME_FINISHED_KILLS ) );
			
		return m_GameRefFrames;
	}
}


// ----------------------------------------------------------------------------
// G_Main methods 
// ----------------------------------------------------------------------------

// default ctor ---------------------------------------------------------------
//
G_Main::G_Main() :
	m_Players				( NULL ),
	m_CurConnectedPlayerList( NULL ),
	m_CurJoinedPlayerList	( NULL )
{
}


// default dtor ---------------------------------------------------------------
//
G_Main::~G_Main()
{
	delete m_CurJoinedPlayerList;
	delete m_CurConnectedPlayerList;
	delete []m_Players;
}


// realize game vars from console vars ----------------------------------------
//
void G_Main::RealizeVariables()
{
	m_TimeManager.RealizeVariables();
	m_nKillLimit = SV_GAME_KILLLIMIT;
}


// create a stargate for a specific server at a position, with a direction ----
//
void G_Main::CreateStargate( int serverid, Vector3* pos_spec, Vector3* dir_spec )
{
	ASSERT( pos_spec != NULL );
	ASSERT( dir_spec != NULL );

	// create corresponding stargate objects
	dword objclass = OBJ_FetchObjectClassId( "stargate" );
	if ( objclass != CLASS_ID_INVALID ) {

		Xmatrx startm;
		MakeIdMatrx( startm );
		startm[ 0 ][ 3 ] = pos_spec->X;
		startm[ 1 ][ 3 ] = pos_spec->Y;
		startm[ 2 ][ 3 ] = pos_spec->Z;

		startm[ 0 ][ 2 ] = dir_spec->X;
		startm[ 1 ][ 2 ] = dir_spec->Y;
		startm[ 2 ][ 2 ] = dir_spec->Z;

		// ensure orthogonal matrix
		CrossProduct2( &startm[ 0 ][ 1 ], &startm[ 0 ][ 2 ], &startm[ 0 ][ 0 ] );
		CrossProduct2( &startm[ 0 ][ 0 ], &startm[ 0 ][ 2 ], &startm[ 0 ][ 1 ] );

		// create the object
		Stargate* stargate = (Stargate*)TheWorld->CreateObject( objclass, startm, PLAYERID_SERVER );

		// store serverid
		stargate->serverid = serverid;

		// attach the created E_Distributable for the engine object
		// stargates are to be delivered reliable
		stargate->pDist = TheSimNetOutput->CreateDistributable( stargate, TRUE );

	} else {
		MSGOUT( "object class stargate could not be found.\n" );
	}
}



// init all game vars ---------------------------------------------------------
//
void G_Main::Init()
{
	ASSERT( m_Players == NULL );
	ASSERT( m_CurConnectedPlayerList == NULL );
	ASSERT( m_CurJoinedPlayerList == NULL );

	m_Players					= new G_Player[ MAX_NUM_CLIENTS ];
	m_CurConnectedPlayerList	= new UTL_List<G_Player*>;
	m_CurJoinedPlayerList		= new UTL_List<G_Player*>;

	EnergyExtraBoost			= EnergyExtraBoost;
	RepairExtraBoost			= RepairExtraBoost;

	DumbPackNumMissls			= DumbPackNumMissls;
	HomPackNumMissls			= HomPackNumMissls;
	SwarmPackNumMissls			= SwarmPackNumMissls;
	ProxPackNumMines			= ProxPackNumMines;

	MegaShieldStrength 			= MEGASHIELD_STRENGTH * FRAME_MEASURE_TIMEBASE;

	m_NebulaID					= m_NebulaID;

	m_nKillLimit				= DEFAULT_KILL_LIMIT;
    
    if(m_NebulaID == 0) //Wasn't set with nebula.id command
        m_NebulaID = 3; //Default Red system
}


// join a player ( init join position ) ---------------------------------------
//
void G_Main::JoinPlayer( int nClientID, E_SimShipState* pSimShipState )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	ASSERT( pSimShipState != NULL );

    G_Player* pPlayer = &m_Players[ nClientID ];
    
    if(!pPlayer->GotSentState()) {
        TheSimNetOutput->statesync( nClientID,TheGame->m_NebulaID,RMEVSTATE_NEBULAID);
        TheSimNetOutput->statesync( nClientID,TheGame->EnergyExtraBoost,RMEVSTATE_ENERGYBOOST);
        TheSimNetOutput->statesync( nClientID,TheGame->RepairExtraBoost,RMEVSTATE_REPAIRBOOST);
        TheSimNetOutput->statesync( nClientID,TheGame->DumbPackNumMissls,RMEVSTATE_DUMBPACK);
        TheSimNetOutput->statesync( nClientID,TheGame->HomPackNumMissls,RMEVSTATE_HOMPACK);
        TheSimNetOutput->statesync( nClientID,TheGame->SwarmPackNumMissls,RMEVSTATE_SWARMPACK);
        TheSimNetOutput->statesync( nClientID,TheGame->ProxPackNumMines,RMEVSTATE_PROXPACK);
        pPlayer->SetSentState();
    }
	// reset to initial state
	pSimShipState->Reset();

	pXmatrx ObjPosition = pSimShipState->GetObjPosition();

	int xdist = ( RAND() % JP_RANGE ) - JP_OFS;
	int ydist = ( RAND() % JP_RANGE ) - JP_OFS;
	int zdist = ( RAND() % JP_RANGE ) - JP_OFS;

	xdist += ( xdist < 0 ) ? -JP_M_S_D : JP_M_S_D;
	ydist += ( ydist < 0 ) ? -JP_M_S_D : JP_M_S_D;
	zdist += ( zdist < 0 ) ? -JP_M_S_D : JP_M_S_D;
	
	ObjPosition[ 0 ][ 3 ] = INT_TO_GEOMV( xdist );
	ObjPosition[ 1 ][ 3 ] = INT_TO_GEOMV( ydist );
	ObjPosition[ 2 ][ 3 ] = INT_TO_GEOMV( zdist );

	//ObjPosition[ 0 ][ 3 ] = INT_TO_GEOMV( 0 );
	//ObjPosition[ 1 ][ 3 ] = INT_TO_GEOMV( 0 );
	//ObjPosition[ 2 ][ 3 ] = INT_TO_GEOMV( nClientID * 200 );

	m_CurJoinedPlayerList->AppendTail( &m_Players[ nClientID ] );
}


// unjoin a player ------------------------------------------------------------
//
void G_Main::UnjoinPlayer( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	G_Player* pPlayer = &m_Players[ nClientID ];
	int rc = m_CurJoinedPlayerList->Remove( pPlayer );
	ASSERT( rc );
}


// return the # of joined players ---------------------------------------------
//
int G_Main::GetNumJoined()
{
	return m_CurJoinedPlayerList->GetNumEntries();
}


// connect a player -----------------------------------------------------------
//
void G_Main::ConnectPlayer( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	G_Player* pPlayer = &m_Players[ nClientID ];
	pPlayer->Connect( nClientID );
	m_CurConnectedPlayerList->AppendTail( pPlayer );

	// start the game, if not yet started
	if ( m_TimeManager.IsNotYetStarted() ) {
		m_TimeManager.StartGame();
	}
}


// disconnect a player --------------------------------------------------------
//
void G_Main::DisconnectPlayer( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	G_Player* pPlayer = &m_Players[ nClientID ];
	pPlayer->Disconnect();
	int rc = m_CurConnectedPlayerList->Remove( pPlayer );
	ASSERT( rc );

	// if the last player disconnects, reset the game 
	if ( TheConnManager->GetNumConnected() == 0 ) {

		// reset the player game vars
		_ResetPlayerVars();

		m_TimeManager.Reset();
	}
}


// retrieve the # of kills by this player -------------------------------------
//
int G_Main::GetPlayerKills( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	return m_Players[ nClientID ].GetKills();
}


// retrieve the last unjoin flag of the player --------------------------------
//
int G_Main::GetPlayerLastUnjoinFlag( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	return m_Players[ nClientID ].GetLastUnjoinFlag();
}


// retrieve the last killer of the player -------------------------------------
//
int G_Main::GetPlayerLastKiller( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	return m_Players[ nClientID ].GetLastKiller();
}


// record a kill --------------------------------------------------------------
//
void G_Main::RecordKill( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	m_Players[ nClientID ].RecordKill();
}


// record a death -------------------------------------------------------------
//
void G_Main::RecordDeath( int nClientID, int nClientID_Killer )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	m_Players[ nClientID ].RecordDeath( nClientID_Killer );
}


// reset the death info of the client -----------------------------------------
//
void G_Main::ResetDeathInfo( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	m_Players[ nClientID ].ResetDeathInfo();
}


// get the player -------------------------------------------------------------
//
G_Player* G_Main::GetPlayer( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	return &m_Players[ nClientID ];
}


// maintain the game ----------------------------------------------------------
//
void G_Main::MaintainGame()
{
	if ( m_TimeManager.IsGameRunning() ) {

		// check whether the game time-limit is hit
		if ( m_TimeManager.IsGameTimeLimitHit() ) {

			m_TimeManager.StopGame_TimeLimit();

			//FIXME: unjoin all joined players

		} else {

			// check whether the kill-limit is hit
			int nMaxKills = 0;
			for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
				G_Player* pPlayer = &m_Players[ nClientID ];
				if ( pPlayer->GetKills() > nMaxKills ) {
					nMaxKills = pPlayer->GetKills();
				}
			}

			// stop the game if kill limit hit
			if ( nMaxKills >= m_nKillLimit ) {
				m_TimeManager.StopGame_KillLimit();
			}
		}
	} else {

		// check whether restart timeout is over
		if ( m_TimeManager.IsRestartTimeoutOver() ) {

			// reset the player game vars
			_ResetPlayerVars();

			// if there are still players connected, restart the game
			if ( TheConnManager->GetNumConnected() > 0 ) {
				m_TimeManager.StartGame();
			} else {
				// set to GAME_NOTSTARTEDYET mode
				m_TimeManager.Reset();
			}
		}
		return;
	}
}


// reset all player game vars -------------------------------------------------
// 
void G_Main::_ResetPlayerVars()
{
	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
		m_Players[ nClientID ].ResetGameVars();
	}
}


// maintain weapon firing delays ----------------------------------------------
//
void G_Main::MaintainWeaponDelays()
{
	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
		m_Players[ nClientID ].MaintainWeaponDelays();
	}
}


// check availability of specified device -------------------------------------
//
int G_Main::OBJ_DeviceAvailable( ShipObject* pShip, int mask )
{
	ASSERT( pShip != NULL );

	if ( SV_CHEAT_DEVICE_CHECKS )
		return TRUE;

	return ( ( pShip->Weapons & mask ) != 0 );
}


// animate projectile objects (lasers and missiles) ---------------------------
//
void G_Main::OBJ_AnimateProjectiles()
{
	//NOTE:
	// this function gets called once per frame by the game loop ( E_Simulator::DoSim() )

	_WalkLaserObjects();
	_WalkMissileObjects();
    FireDurationWeapons();
    TheWorld->PAN_AnimateParticles();
}


// animate non-projectile objects (extras, mines) -----------------------------
//
void G_Main::OBJ_AnimateNonProjectiles()
{
	//NOTE:
	// this function gets called once per frame by the game loop ( E_Simulator::DoSim() )

	_WalkExtraObjects();
}

void G_Main::MaintainSpecialsCounters(  ) {

	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
        if( TheSimulator->IsPlayerJoined( nClientID ) ) {
           ShipObject *pShip = m_Players[nClientID].GetShipObject();

           // decrement the MegaShieldAbsorption counter
           if(pShip->MegaShieldAbsorption > 0){
        	  pShip->MegaShieldAbsorption -= TheSimulator->GetThisFrameRefFrames();
        	  MSGOUT("Client: %d, MegaShield: %d, RefFrameDec: %d",
        			  nClientID,
        			  pShip->MegaShieldAbsorption,
        			  TheSimulator->GetThisFrameRefFrames()
        			  );
           }
        }
    }
	// walk custom objects and decrement any class life identifiers that need decrementing.
	_WalkCustomObjects();
}

void G_Main::FireDurationWeapons()
{
	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
        if( TheSimulator->IsPlayerJoined( nClientID ) ) {
            MaintainDurationWeapons( nClientID );
        }
    }
}

void G_Main::MaintainDurationWeapons( int playerid )
{
	
    ShipObject* pShip = m_Players[playerid].GetShipObject();
	ASSERT( pShip != NULL );
    
	// maintain helix
	if ( pShip->WeaponsActive & WPMASK_CANNON_HELIX ) {
		m_Players[playerid]._WFX_MaintainHelix( pShip, playerid );
	}
    
	// maintain lightning
	if ( pShip->WeaponsActive & WPMASK_CANNON_LIGHTNING ) {
		m_Players[playerid].WFX_MaintainLightning( pShip );
	}
    
	// maintain photon
	photon_sphere_pcluster_s* cluster = (photon_sphere_pcluster_s *)
    TheWorld->PRT_ObjectHasAttachedClustersOfType( pShip, SAT_PHOTON );
	if ( cluster != NULL ) {
		TheWorld->CalcPhotonSphereAnimation( cluster );
	}
    /*
	// maintain emp
	if ( shippo->WeaponsActive & WPMASK_DEVICE_EMP ) {
		WFX_CreateEmpWaves( shippo );
	}
    */
}

// walk list of extra objects and advance them ( also handle timeout ) --------
//
void G_Main::_WalkExtraObjects()
{
	ASSERT( TheWorld->m_ExtraObjects != NULL );

	// walk list of extras
	ExtraObject *precnode = TheWorld->m_ExtraObjects;
	while ( precnode->NextObj != NULL ) {

		ASSERT( OBJECT_TYPE_EXTRA( precnode->NextObj ) );

		// get pointer to current extra
		ExtraObject *curextra = (ExtraObject *) precnode->NextObj;
		ASSERT( curextra != NULL );

		// check if lifetime of extra is spent
		curextra->LifeTimeCount -= TheSimulator->GetThisFrameRefFrames();
		//MSGOUT( "curextra: %x, curextra->LifeTimeCount: %d", curextra, curextra->LifeTimeCount );
		if ( curextra->LifeTimeCount < 0 ) {
			TheGameExtraManager->OBJ_KillExtra( precnode, FALSE );
			continue;
		}

		// animate this extra
		TheGameExtraManager->OBJ_AnimateExtra( curextra );

		// advance in list
		precnode = curextra;
	}
}


// walk list of laser objects and advance them ( also handle timeout ) --------
//
void G_Main::_WalkMissileObjects()
{
	ASSERT( TheWorld->m_MisslObjects != NULL );

	GenObject *precnode  = TheWorld->m_MisslObjects;
	GenObject *walkshots = TheWorld->m_MisslObjects->NextObj;

	// walk all Missiles
	while ( walkshots != NULL ) {

		ASSERT( OBJECT_TYPE_MISSILE( walkshots ) );

		MissileObject *missilepo	= (MissileObject *) walkshots;
		missilepo->LifeTimeCount -= TheSimulator->GetThisFrameRefFrames();

		if ( missilepo->LifeTimeCount <= 0 ) {

#ifndef DONT_RESET_SHOTCOUNTER
			TheWorld->DecreaseShotCounter();
#endif // DONT_RESET_SHOTCOUNTER

			// release the E_Distributable ( distribute removal )
			//MSGOUT( "G_Main::_WalkMissileObjects() calls ReleaseDistributable()" );
			TheSimNetOutput->ReleaseDistributable( missilepo->pDist );
			ASSERT( walkshots != NULL );
			precnode->NextObj = walkshots->NextObj;
			TheWorld->FreeObjectMem( walkshots );
			walkshots = precnode->NextObj;

			continue;

		} else {

			//FIXME: move to OBJ_AnimateMissile()
			Vector3 tempspeed;
			switch ( missilepo->ObjectType & TYPECONTROLMASK ) {
                 case TYPEMISSILEISSTANDARD:
		         {
           	          tempspeed.X = missilepo->DirectionVec.X * TheSimulator->GetThisFrameRefFrames();
			          tempspeed.Y = missilepo->DirectionVec.Y * TheSimulator->GetThisFrameRefFrames();
			          tempspeed.Z = missilepo->DirectionVec.Z * TheSimulator->GetThisFrameRefFrames();

                	  missilepo->PrevPosition.X = missilepo->ObjPosition[ 0 ][ 3 ];
			          missilepo->PrevPosition.Y = missilepo->ObjPosition[ 1 ][ 3 ];
			          missilepo->PrevPosition.Z = missilepo->ObjPosition[ 2 ][ 3 ];

			          missilepo->ObjPosition[ 0 ][ 3 ] += tempspeed.X;
			          missilepo->ObjPosition[ 1 ][ 3 ] += tempspeed.Y;
		              missilepo->ObjPosition[ 2 ][ 3 ] += tempspeed.Z;
                      break;
                  } 
                  case TYPEMISSILEISHOMING:
                  {
                       tempspeed.X = missilepo->DirectionVec.X * TheSimulator->GetThisFrameRefFrames();
	                   tempspeed.Y = missilepo->DirectionVec.Y * TheSimulator->GetThisFrameRefFrames();
	                   tempspeed.Z = missilepo->DirectionVec.Z * TheSimulator->GetThisFrameRefFrames();

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

                      } else {

		                    // search for target in shiplist
		                    targetpo = TheWorld->FetchFirstShip();
		                    while ( ( targetpo != NULL ) && ( targetpo->HostObjNumber != tmissilepo->TargetObjNumber ) ) {
			                     targetpo = targetpo->NextObj;
		                    }
		                    if ( targetpo == NULL ) {
			                     // missile loses target once object not found
                                
                                 MSGOUT("G_Main::_WalkMissileObjects(): Target lost: No Object found");
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
                                 MSGOUT("G_Main::_WalkMissileObjects(): Target lost: Out of position");
                                

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

                       break;
                  }  
             }
		}

		precnode  = walkshots;
		walkshots = walkshots->NextObj;
	}
}

// walk the list of custom objects and handle timeout for the type specified.
void G_Main::_WalkCustomObjects()
{
	ASSERT( TheWorld->m_CustmObjects != NULL );

	CustomObject *precnode  = TheWorld->m_CustmObjects;
	CustomObject *walkobjs = (CustomObject *)TheWorld->m_CustmObjects->NextObj;

	// walk all lasers
	while ( walkobjs != NULL ) {
		if ((walkobjs->ObjectType == emp_type_id[ 0 ]) ||
			(walkobjs->ObjectType == emp_type_id[ 1 ]) ||
			(walkobjs->ObjectType == emp_type_id[ 2 ])) {

				Emp *tmpemp = (Emp *)walkobjs;
				if(!EmpAnimate(tmpemp)) {
					// delete the EMP object
					MSGOUT("Deleting EMP OBJ");
					precnode->NextObj = (GenObject *)walkobjs->NextObj;
					TheWorld->FreeObjectMem( walkobjs );
					walkobjs = (CustomObject *)precnode->NextObj;
				}
		}

		precnode  = walkobjs;
		walkobjs = (CustomObject *)walkobjs->NextObj;
	}
}
// walk list of laser objects and advance them ( also handle timeout ) --------
//
void G_Main::_WalkLaserObjects()
{
	ASSERT( TheWorld->m_LaserObjects != NULL );

	GenObject *precnode  = TheWorld->m_LaserObjects;
	GenObject *walkshots = TheWorld->m_LaserObjects->NextObj;

	// walk all lasers
	while ( walkshots != NULL ) {

		ASSERT( OBJECT_TYPE_LASER( walkshots ) );

		LaserObject *laserpo	= (LaserObject *) walkshots;
		laserpo->LifeTimeCount -= TheSimulator->GetThisFrameRefFrames();

		if ( laserpo->LifeTimeCount <= 0 ) {

#ifndef DONT_RESET_SHOTCOUNTER
			TheWorld->DecreaseShotCounter();
#endif // DONT_RESET_SHOTCOUNTER

			// release the E_Distributable ( distribute removal )
		//	MSGOUT( "G_Main::_WalkLaserObjects() calls ReleaseDistributable()" );
			TheSimNetOutput->ReleaseDistributable( laserpo->pDist );

			ASSERT( walkshots != NULL );
			precnode->NextObj = walkshots->NextObj;
			TheWorld->FreeObjectMem( walkshots );
			walkshots = precnode->NextObj;

			continue;

		} else {

			//FIXME: move to OBJ_AnimateLaser()
			Vector3 tempspeed;
			
			tempspeed.X = laserpo->DirectionVec.X * TheSimulator->GetThisFrameRefFrames();
			tempspeed.Y = laserpo->DirectionVec.Y * TheSimulator->GetThisFrameRefFrames();
			tempspeed.Z = laserpo->DirectionVec.Z * TheSimulator->GetThisFrameRefFrames();

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

// create actual laser object -------------------------------------------------
//
LaserObject* G_Main::OBJ_CreateLaserObject( ShipObject *pShip, int curlevel, int barrel, int nClientID )
{
	ASSERT( pShip != NULL );
	ASSERT( ( curlevel >= 0 ) && ( curlevel < 4 ) );
	ASSERT( ( barrel >= 0   ) && ( barrel   < 4 ) );
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );

	dword laserclass = pShip->Laser1_Class[ curlevel ][ barrel ];

	// create launch matrix
	Xmatrx startm;
	MakeIdMatrx( startm );
	startm[ 0 ][ 3 ] = pShip->Laser1_X[ curlevel ][ barrel ];
	startm[ 1 ][ 3 ] = pShip->Laser1_Y[ curlevel ][ barrel ];
	startm[ 2 ][ 3 ] = pShip->Laser1_Z[ curlevel ][ barrel ];

	// create laser object and calculate position and direction vector
	MtxMtxMUL( pShip->ObjPosition, startm, DestXmatrx );
	LaserObject *laserpo = (LaserObject *) TheWorld->CreateObject( laserclass, DestXmatrx, nClientID );
	ASSERT( laserpo != NULL );
	laserpo->Speed += pShip->CurSpeed;
	DirVctMUL( laserpo->ObjPosition, FIXED_TO_GEOMV( laserpo->Speed ), &laserpo->DirectionVec );
	laserpo->Owner = nClientID;

	TheWorld->IncreaseShotCounter();

	//FIXME: should the creation/deletion of distributables move into E_World::CreateObject() ?
	// attach the created E_Distributable for the engine object
	laserpo->pDist = TheSimNetOutput->CreateDistributable( laserpo );

	// record create event if recording active
	//Record_LaserCreation( laserpo );

	return laserpo;
}
MissileObject* G_Main::OBJ_CreateMissileObject( ShipObject *pShip,  int barrel, int nClientID )
{
	ASSERT( pShip != NULL );
	
	ASSERT( ( barrel >= 0   ) && ( barrel   < 4 ) );
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );

	int missileclass = pShip->Missile1_Class[ barrel ];

	// create launch matrix
	Xmatrx startm;
	MakeIdMatrx( startm );
	startm[ 0 ][ 3 ] = pShip->Missile1_X[ barrel ];
	startm[ 1 ][ 3 ] = pShip->Missile1_Y[ barrel ];
	startm[ 2 ][ 3 ] = pShip->Missile1_Z[ barrel ];

	// create missile object and calculate position and direction vector
	MtxMtxMUL( pShip->ObjPosition, startm, DestXmatrx );
	MissileObject *missilepo = (MissileObject *) TheWorld->CreateObject( missileclass, DestXmatrx, nClientID );
	ASSERT( missilepo != NULL );
	missilepo->Speed += pShip->CurSpeed;
	DirVctMUL( missilepo->ObjPosition, FIXED_TO_GEOMV( missilepo->Speed ), &missilepo->DirectionVec );
	missilepo->Owner = nClientID;

	//FIXME: should the creation/deletion of distributables move into E_World::CreateObject() ?
	// attach the created E_Distributable for the engine object
	missilepo->pDist = TheSimNetOutput->CreateDistributable( missilepo );

	return missilepo;
}

MissileObject* G_Main::OBJ_CreateHomingMissileObject( ShipObject *pShip,  int barrel, int nClientID, dword targetid )
{

	ASSERT( pShip != NULL );
	
	ASSERT( ( barrel >= 0   ) && ( barrel   < 4 ) );
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );

	// create launch matrix
	Xmatrx startm;
	MakeIdMatrx( startm );
	startm[ 0 ][ 3 ] = pShip->Missile2_X[ barrel ];
	startm[ 1 ][ 3 ] = pShip->Missile2_Y[ barrel ];
	startm[ 2 ][ 3 ] = pShip->Missile2_Z[ barrel ];

	// create missile object and calculate position and direction vector
	MtxMtxMUL( pShip->ObjPosition, startm, DestXmatrx );
	TargetMissileObject *missilepo = (TargetMissileObject *) TheWorld->CreateObject( GUIDE_CLASS_1, DestXmatrx, nClientID );
	ASSERT( missilepo != NULL );
	missilepo->Speed += pShip->CurSpeed;
	DirVctMUL( missilepo->ObjPosition, FIXED_TO_GEOMV( missilepo->Speed ), &missilepo->DirectionVec );
	missilepo->Owner		   = nClientID;
	missilepo->TargetObjNumber = targetid;
	
	//FIXME: should the creation/deletion of distributables move into E_World::CreateObject() ?
	// attach the created E_Distributable for the engine object
	missilepo->pDist = TheSimNetOutput->CreateDistributable( missilepo );

	return missilepo;
}

MineObject * G_Main::OBJ_CreateMineObject( ShipObject *pShip, int nClientID )
{
	// create launch matrix
	Xmatrx startm;
	MakeIdMatrx( startm );

//#ifdef SHIPBOUNDED_MINE_PLACEMENT
	startm[ 0 ][ 3 ] = GEOMV_0;
	startm[ 1 ][ 3 ] = GEOMV_0;
	startm[ 2 ][ 3 ] = -( pShip->BoundingSphere + GEOMV_1 );
//#else
//	startm[ 0 ][ 3 ] = pShip->Mine1_X;
//	startm[ 1 ][ 3 ] = pShip->Mine1_Y;
//	startm[ 2 ][ 3 ] = pShip->Mine1_Z;
//#endif

	// create mine object
	MtxMtxMUL( pShip->ObjPosition, startm, DestXmatrx );
	MineObject *minepo = (MineObject *) TheWorld->CreateObject( MINE_CLASS_1, DestXmatrx, nClientID );
	ASSERT( minepo != NULL );
	minepo->Owner = nClientID;

	minepo->pDist = TheSimNetOutput->CreateDistributable( minepo);

	return minepo;

}

GenObject* G_Main::OBJ_CreateSwarm( ShipObject *pShip, int nClientID, dword targetid )
{
	dword randseed = SYSs_GetRefFrameCount();
    GenObject *dummyobj = NULL;
	Vertex3 origin;
	origin.X = pShip->ObjPosition[ 0 ][ 3 ];
	origin.Y = pShip->ObjPosition[ 1 ][ 3 ];
	origin.Z = pShip->ObjPosition[ 2 ][ 3 ];

    // get pointer to ship of target
	ShipObject *targetpo = TheWorld->FetchFirstShip();
    while ( targetpo != NULL && ( targetpo->HostObjNumber != targetid ) ) {
		targetpo = (ShipObject *) targetpo->NextObj;
	}
    
    if(targetpo == NULL) {
        MSGOUT("G_Main::OBJ_CreateSwarm(): Target %d not found for swarm creation",targetid);
        return dummyobj;
    }
    dummyobj = (GenObject *) TheWorld->SWARM_Init( nClientID, &origin, targetpo, randseed );
   // MSGOUT("G_Main::OBJ_CreateSwarm(): %d fired swarm missiles",nClientID);
    
    return dummyobj;
}


// ----------------------------------------------------------------------------
// G_Input methods 
// ----------------------------------------------------------------------------

// activate the selected gun for a client -------------------------------------
//
void G_Input::ActivateGun( int nClientID, int SelectedGun )
{
	G_Player*  pPlayer = TheGame->GetPlayer( nClientID );

	// fire selected gun
	switch ( SelectedGun ) {

		// laser
		case 0:
			pPlayer->FireLaser();
			break;

		// helix cannon
		case 1:
			pPlayer->FireHelix();
			break;

		// lightning device
		case 2:
			pPlayer->FireLightning();
			break;

		// photon cannon
		case 3:
			pPlayer->FirePhoton();
			break;

		// emp device
		case 4:
			//User_FireEmp();
			break;

		default:
			ASSERT( 0 );
	}
}

void G_Input::LaunchMissile( int nClientID, dword targetid, int missileclass )
{

	// launch a missle, dumb or homing
	G_Player*  pPlayer = TheGame->GetPlayer( nClientID );

	if ( missileclass == GUIDE_CLASS_1 ) {
		pPlayer->LaunchHomingMissile(nClientID, targetid );
	}
	else{
		pPlayer->LaunchMissile();
	}
}

void G_Input::LaunchMine( int nClientID )
{
	G_Player* pPlayer = TheGame->GetPlayer( nClientID );
	pPlayer->LaunchMine();
}

void G_Input::LaunchSwarm(int nClientID, dword targetid)
{
    G_Player*  pPlayer = TheGame->GetPlayer( nClientID );
    pPlayer->LaunchSwarm(targetid);
}

void G_Input::CreateEMP(int nClientID, byte UpgradeLevel)
{
	G_Player* pPlayer = TheGame->GetPlayer( nClientID );
	pPlayer->FireEMP(UpgradeLevel);

}

// activate the selected gun for a client -------------------------------------
//
void G_Input::DeactivateGun( int nClientID, int SelectedGun )
{
   G_Player*  pPlayer = TheGame->GetPlayer( nClientID );
    
	// Deactivate selected gun
	switch ( SelectedGun ) {
            
            // laser
		case 0:
			break;
            // helix cannon
		case 1:
			pPlayer->_WFX_DeactivateHelix();
			break;
            
            // lightning device
		case 2:
			pPlayer->_WFX_DeactivateLightning();
			break;
            
            // photon cannon
		case 3:
			pPlayer->_WFX_DeactivatePhoton();
			break;
            
            // emp device
		case 4:
			//User_FireEmp();
			break;
            
		default:
			ASSERT( 0 );
	}
}
