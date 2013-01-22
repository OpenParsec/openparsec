// class holding all GAME specific player information -------------------------
// 
// helix energy properties
#define HELIX_LIFETIME                          1500
#define HELIX_SPEED                             0x10000
#define MIN_HELIX_ENERGY                        10
#define HELIX_ENERGY_CONSUMPTION			2
// properties of spreadfire particles       Also move this someday
#define SPREADFIRE_PARTICLE_COLOR	              174
#define SPREADFIRE_BM_INDX	                     BM_FIREBALL1
#define SPREADFIRE_REF_Z                         20.0f //400.0f
// lightning energy properties
#define MIN_LIGHTNING_ENERGY				10
#define LIGHTNING_ENERGY_CONSUMPTION		7000          // 65536 = 1.0

class G_Player
{
	//NOTE: this class mirrors all information/actions of a single client ( in the client code )

protected:

	int			m_nKills;				// # of ships this player killed
	int			m_nDeaths;				// # of deaths for this player
	int			m_nPoints;				// # of points for this player
	int			m_nLastUnjoinFlag;		// the last unjoin flag
	int			m_nLastKiller;			// the playerid of last killer

	refframe_t	m_FireDisableFrames;	// refframes gun-fire is disabled                  ( = G_GLOBAL::FireDisable	in old CLIENT code )
	refframe_t  m_MissileDisableFrames;	// refframes missile-fire is disabled              ( = G_GLOBAL::MissileDisable	in old CLIENT code )
    refframe_t	helix_refframes_delta;
	int 		m_CurGun;				// currently selected gun outlet
	int 		m_CurLauncher;			// currently selected missile outlet

	int			m_nClientID;
    int         m_StateSync;            //Set to 1 after the server tells the client what the Nebula ID , ammo packs, energy/health values are

	E_SimPlayerInfo* m_pSimPlayerInfo;
protected:

	// create laser originating from player ship
	void _OBJ_ShootLaser();

    //Activate Helix Cannon
    void _WFX_ActivateHelix();
    	
    //Activate lightning gun
    void _WFX_ActivateLightning();
    
    //Activate Photon cannon
    void _WFX_ActivatePhoton();
    
    // create a dumb missle originating from player ship
	void _OBJ_LaunchMissile();
	
	// create a homing missle originating from player ship
	void _OBJ_LaunchHomingMissile( dword launcher, dword targetid );

	void _OBJ_LaunchMine();

	void _OBJ_LaunchSwarm( dword targetid );


public:
	G_Player()
	{
		Reset();
	}

	// reset all fields to defaults ( not connected )
	void Reset();

	// set the player status to connected
	void Connect( int nClientID );

	// set the player status to disconnected
	void Disconnect();

	// user fired laser 
	void FireLaser();
    
    // user fired Helix cannon
    void FireHelix();
    int  _WFX_MaintainHelix( ShipObject *shippo, int playerid );
    
    //Deactivate Helix Cannon
    void _WFX_DeactivateHelix();
    
    // user fired Lightning cannon
    void FireLightning();
    void WFX_MaintainLightning( ShipObject *shippo);
    
    //Deactivate lightning gun
    void _WFX_DeactivateLightning();
    
    // user fired Photon cannon
    void FirePhoton();
    
    //Deactivate Photon Cannon
    void _WFX_DeactivatePhoton();

	// user launched a dumb missle
	void LaunchMissile();
	
	// user launched a homping missle
	void LaunchHomingMissile(dword launcher, dword targetid);

	// user launched a mine
	void LaunchMine();

	// user launched a swarm
	void LaunchSwarm(dword targetid);

	// user fired emp
	void FireEMP(byte Upgradelevel);

	// record a kill
	void RecordKill();

	// record a death
	void RecordDeath( int nClientID_Killer );

	// reset any death info
	void ResetDeathInfo();

	// reset all game variables
	void ResetGameVars();

	// maintain weapon firing delays
	void MaintainWeaponDelays();

	// return the ship object assigned to the player
	ShipObject* GetShipObject();

	// return # of kills for the player
	int GetKills() { return m_nKills; }

	// return the last unjoin flag
	int GetLastUnjoinFlag() { return m_nLastUnjoinFlag; }

	// return the player id that last killed this player
	int GetLastKiller() { return m_nLastKiller; }
    
    //Send client ammo pack sizes, system ID
    int GotSentState() { return m_StateSync; }
    void SetSentState() { m_StateSync = 1; }
};

