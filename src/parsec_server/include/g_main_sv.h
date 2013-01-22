/** PARSEC HEADER: g_main_sv.h
*/

#ifndef _G_MAIN_H_
#define _G_MAIN_H_


// join position control ------------------------------------------------------
//
#define JP_M_S_D 		500				// minimum join position distance
#define JP_RANGE 		1000			// maximum join position range
#define JP_OFS			(JP_RANGE/2)


// afterburner constants ------------------------------------------------------
//
#define AFTERBURNER_SPEED	150000
#define AFTERBURNER_ENERGY	2400


// forward decls --------------------------------------------------------------
//
template <class T> class UTL_List;
class G_Player;
class E_SimShipState;

// default gametime limits ----------------------------------------------------
//
#define DEFAULT_GAME_TIMELIMIT_SEC		600
#define DEFAULT_RESTART_TIMELIMIT_SEC	15
#define DEFAULT_KILL_LIMIT				10

// class handling the gametime management -------------------------------------
//
class G_TimeManagement
{
protected:

	int GameIsEndless;
	refframe_t	m_GameEndRefFrames;			// refframes for timelimit
	refframe_t  m_GameRefFrames;			// refframes of current game
	refframe_t	m_RestartRefFrames;			// refframes of current restart timeout

	refframe_t	m_RefFrameBase;

	int			m_nSecGameTimeLimit;		// timelimit for the game in sec.
	int			m_nSecRestartTimeLimit;		// time to wait between restarts in sec.
public:
	G_TimeManagement();

	// realize console variables
	void RealizeVariables();

	// reset the game time management
	void Reset();

	// start a game
	void StartGame();

	// stop the game ( time limit hit )
	void StopGame_TimeLimit();

	// stop the game ( kill limit hit )
	void StopGame_KillLimit();

	// return whether the game is not yet started
	int IsNotYetStarted();

	// return whether currently a game is running
	int IsGameRunning();

	// return whether the game is finished
	int IsGameFinished();

	// check whether the game time limit is hit
	int IsGameTimeLimitHit();

	// check whether the restart timeout is over
	int IsRestartTimeoutOver();

	// return the current gametime in secs or special gametime codes
	int GetCurGameTime();
};


// class holding the parsec game code -----------------------------------------
//
class G_Main
{
public:

	int						EnergyExtraBoost;
	int 					RepairExtraBoost;
	int 					DumbPackNumMissls;
	int 					HomPackNumMissls;
	int 					SwarmPackNumMissls;
	int 					ProxPackNumMines;

	int 					MegaShieldStrength;

	int						m_NebulaID;

	G_Player*				m_Players;
	UTL_List<G_Player*>*	m_CurConnectedPlayerList;		// linked list of all currently connected players
	UTL_List<G_Player*>*	m_CurJoinedPlayerList;			// linked list of all currently joined players

	G_TimeManagement		m_TimeManager;
	int						m_nKillLimit;

protected:
	// default ctor/dtor
	G_Main();
	~G_Main();

	// walk list of extra objects and advance them ( also handle timeout )
	void _WalkExtraObjects();

	// walk list of laser objects and advance them ( also handle timeout )
	void _WalkLaserObjects();
    
    //walk list of missile objects and advance them
    void _WalkMissileObjects();
    
    //Do particle weapon maintenence
    void MaintainDurationWeapons( int playerid );
    
public:

	// SINGLETON pattern
	static G_Main* GetGame()
	{
		static G_Main _TheGame;
		return &_TheGame;
	}

	void Init();

	// connect a player
	void ConnectPlayer( int nClientID );

	// disconnect a player
	void DisconnectPlayer( int nClientID );

	// join a player ( init join position )
	void JoinPlayer( int nClientID, E_SimShipState* pSimShipState );

	// unjoin a player 
	void UnjoinPlayer( int nClientID );

	// return the # of joined players
	int GetNumJoined();

	// retrieve the # of kills by this player
	int GetPlayerKills( int nClientID );

	// retrieve the last unjoin flag of the player
	int GetPlayerLastUnjoinFlag( int nClientID );

	// retrieve the last killer of the player
	int GetPlayerLastKiller( int nClientID );

	// record a kill of another client
	void RecordKill( int nClientID );

	// record a death of the client
	void RecordDeath( int nClientID, int nClientID_Killer );

	// reset the death info of the client
	void ResetDeathInfo( int nClientID );

	// get the playerinfo ( GAME )
	G_Player* GetPlayer( int nClientID );

	// return a list of currently connected players
	UTL_List<G_Player*>* GetCurConnectedPlayerList() { return m_CurConnectedPlayerList; }

	// return a list of currently joined players
	UTL_List<G_Player*>* GetCurJoinedPlayerList() { return m_CurJoinedPlayerList; }

	// maintain weapon firing delays
	void MaintainWeaponDelays();

	// maintain the game
	void MaintainGame();

	// animate projectile objects (lasers and missiles)
	void OBJ_AnimateProjectiles();

	// animate non-projectile objects (extras, mines)
	void OBJ_AnimateNonProjectiles();

	// for maintaining countes on special powerups for players
	void MaintainSpecialsCounters(  );

    //Duration weapons
    void FireDurationWeapons();
    
	// create actual laser object 
	LaserObject* OBJ_CreateLaserObject( ShipObject *shippo, int curlevel, int barrel, int nClientID );

	// create actual dumb missile object
	MissileObject* OBJ_CreateMissileObject( ShipObject *pShip,  int barrel, int nClientID );
	
	// create actual homing missile object
	MissileObject* OBJ_CreateHomingMissileObject( ShipObject *pShip, int barrel, int nClientID, dword targetid );

	// create actual mine object
	MineObject* OBJ_CreateMineObject( ShipObject *pShip, int nClientID );

	// create actual swarm object
	GenObject* OBJ_CreateSwarm( ShipObject *pShip, int nClientID, dword targetid );

	GenObject* G_Main::OBJ_CreateEmp( ShipObject *pShip, int nClientID, byte Upgradelevel );

	// check availability of specified device
	int OBJ_DeviceAvailable( ShipObject* pShip, int mask );

	// return the current gametime in secs or special gametime codes
	int GetCurGameTime() { return m_TimeManager.GetCurGameTime(); }

	// return whether a game 
	int IsGameRunning() { return m_TimeManager.IsGameRunning(); }

	// realize game vars from console vars
	void RealizeVariables();

	// create a stargate for a specific server at a position, with a direction
	void CreateStargate( int serverid, Vector3* pos_spec, Vector3* dir_spec );

protected:

	// reset all player game vars
	void _ResetPlayerVars();
};

// class for handling game input ----------------------------------------------
//
class G_Input
{
protected:
	G_Input()	{} 
	~G_Input() {}

public:
	// SINGLETON pattern
	static G_Input* GetGameInput()
	{
		static G_Input _TheGameInput;
		return &_TheGameInput;
	}

	// activate the selected gun for a client
	void ActivateGun( int nClientID, int SelectedGun );

	// deactivate the selected gun for a client
	void DeactivateGun( int nClientID, int SelectedGun );

	// launch a dumb or homing missle
	void LaunchMissile( int nClientID,  dword targetid, int missileclass );

	// launch a mine
	void LaunchMine( int nClientID );
    
    //Swarms
    void LaunchSwarm(int nClientID, dword targetid);

    // EMP
    void CreateEMP(int nClientID, byte UpgradeLevel)

};


#endif // _G_MAIN_H_

