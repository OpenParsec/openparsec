/*
* PARSEC HEADER: e_gameserver.h
*/

#ifndef _E_GAMESERVER_H_
#define _E_GAMESERVER_H_

// constants ------------------------------------------------------------------
//
#define MAX_MASTERSERVER_NAME					MAX_HOSTNAME_LEN
#define MAX_NUM_CLIENTS							TheServer->GetMaxNumClients()

#include <string.h>

// class holding the server configuration -------------------------------------
//
class ServerConfig
{
private:
	int					m_SimFrequency;						// frequency to run the sim
protected:

	// data that can be modified via commandline/cfg file
	//char				m_szSelectedInterfaceIP[ MAX_STRLEN_IPADDR ];	// string containing the selected interface IP ( NULL if default )
	word				m_nInterface;						// zero based interface index
	int					m_MaintainFrequency;				// frequency to run maintainance
	int					m_nPacketAverageSecs;				// time to sample the average packetsize for optimal client update rate

	int					m_nMaxNumClients;					// current max. # of players
	char				m_szServername[ MAX_SERVER_NAME + 1 ];// the servername

	refframe_t			m_ClientUpdateHeartbeat;

	int					m_ServerIsMaster;

/*	char	location[ MAX_CFG_LINE + 1 ]		= "not specified";
	char	admin_name[ MAX_CFG_LINE + 1 ]		= "not specified";
	char	admin_email[ MAX_CFG_LINE + 1 ]		= "not specified";
	char	server_info[ MAX_CFG_LINE + 1 ]		= "";
	char	server_url[ MAX_CFG_LINE + 1 ]		= "";
	char	messageline1[ MAX_CFG_LINE + 1 ]	= "";
	char	messageline2[ MAX_CFG_LINE + 1 ]	= "";
	char	messageline3[ MAX_CFG_LINE + 1 ]	= "";
	char	messageline4[ MAX_CFG_LINE + 1 ]	= "";
	
	int		time_limit = 0;
	int		time_startrefframes = 0;
	int		time_counter = GAME_NOTSTARTEDYET;
	int		counter_active = FALSE;
	int		wait_startrefframes = 0;
	
	int		kill_limit = 0;
	
	char	*banlist[ MAX_BANNED_CLIENTS ];
	int		num_banned = 0;
*/	

public:
	// standard ctor
	ServerConfig();

	// return the refframes to wait between client update heartbeats
	refframe_t GetClientUpdateHeartbeat() { return m_ClientUpdateHeartbeat; }

	// functions to set/get whether this server instance is a master server.
	void SetServerIsMaster(bool isMaster);
	bool GetServerIsMaster();

	// set the servername
	void SetServername( const char* pName )
	{ 
		strncpy( m_szServername, pName, MAX_SERVER_NAME );
		m_szServername[ MAX_SERVER_NAME ] = 0;
	}

	// get the server name 
	const char* GetServerName()	{ return m_szServername; }

	// set the # of max players
	bool_t SetMaxNumClients( int nMaxNumClients );

	// return the max # of players
	int GetMaxNumClients();

	// set the simulation frequency
	bool_t SetSimFrequency( int nSimFrequency );

	// return the simulation frequency
	int	GetSimFrequency();
};

// class for holding a server link --------------------------------------------
//
class E_ServerLinkInfo
{
public:
	int			m_serverid;
	Vector3		m_pos;
	Vector3		m_dir;

	E_ServerLinkInfo()
	{
		Reset();
	}

	void Reset()
	{
		m_serverid = 0;
		memset( &m_pos, 0, sizeof( Vector3 ) );
		memset( &m_dir, 0, sizeof( Vector3 ) );
	}
};



// main gameserver class ------------------------------------------------------
//
class E_GameServer : public ServerConfig
{
protected: 	// data

	// internal data
	refframe_t			m_Maintain_FrameTime;
	refframe_t			m_SimTick_FrameTime;				// duration in refframes of one simulation tick
	int					m_Select_Timeout;					// timout for select (microseconds)
	int					m_ServerIdleTime_msec;				// idle time ( select timeout ) of server

	int					m_nServerFrame;						// the current server frame

	refframe_t			m_MaintainFrameBase;
	refframe_t			m_MasterServerFrameBase;
	refframe_t			m_PacketFrameBase;
	refframe_t			m_CurPacketRefFrames;

	refframe_t			m_CurServerRefFrame;
	refframe_t			m_LastServerRefFrame;

	refframe_t			m_MasterServer_FrameTime;			// frametime for master server maintenance
	char				m_MasterServer_Hostname[ MAX_MASTERSERVER_NAME + 1];
	int					m_MasterServer_Challenge;
	bool_t				m_bMasterServer_NodeValid;
	node_t				m_MasterServer_Node;

	E_ServerLinkInfo	m_ServerLinks[ MAX_NUM_LINKS ];
	int					m_nNumServerLinks;

	bool_t				m_bQuit;
protected: 	// methods

	// init pre/post running of console script
	int			_InitPostConsoleScript();
	int			_InitPreConsoleScript();

	// ------------------------------------------------------------------------
	// main methods called in the server frame
	// ------------------------------------------------------------------------
	void		_MaintainSimulation();						// maintain the simulation
	int			_MaintainHousekeeping();					// maintain clients ( client timeouts etc. )
	int			_MaintainMasterServer();					// maintain communication to master server

	E_GameServer();										// standard ctor
	~E_GameServer();										// standard dtor

public: 	// methods

	// SINGLETON pattern
	static E_GameServer* GetGameServer()
	{
		static E_GameServer _TheGameServer;
		return &_TheGameServer;
	}

	int			Init();										// init the server components
	int			Kill();										// kill the server components
	
	int			ParseCommandLine( int argc, char** argv );	// parse the command line

	int			PrintCopyRight();							// print copyright
	int			PrintUsage();								// print program usage
	
	int			MainLoop();									// server mainloop
	refframe_t	ServerFrame();								// server Frame

	// set the new masterserver info
	void		SetMasterServerInfo( char* pName, refframe_t _MasterServer_FrameTime );

	// check whether the supplied node matches the stored masterserver node
	int			IsMasterServerNode( node_t* node );

	// set a new masterserver challenge
	void		SetMasterServerChallenge( int nMASVChallenge );

	// add a serverlink
	int			AddServerLink( int serverid, Vector3* pos_spec, Vector3* dir_spec );

	// accessor methods
	refframe_t	GetSimTickFrameTime()		{ return m_SimTick_FrameTime; }
	int			GetPacketAverageSecs()		{ return m_nPacketAverageSecs; }
	refframe_t  GetMasterServerFrameTime()  { return m_MasterServer_FrameTime; }

	void		SetQuitFlag()				{ m_bQuit = TRUE; }
};

#endif // _E_GAMESERVER_H_
