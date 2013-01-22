/*
* PARSEC HEADER: e_connmanager.h
*/

#ifndef _E_CONNMANAGER_H_
#define _E_CONNMANAGER_H_

// forward decls --------------------------------------------------------------
//
class E_GameServer;
class E_PacketHandler;
class E_SimPlayerInfo;


// class holding the challenges sent to requesting clients --------------------
//
class E_ClientChallengeInfo
{
public:
	int			m_challenge;
	node_t		m_node;
	refframe_t  m_frame_generated;
public:
	E_ClientChallengeInfo() : 
	  m_challenge( -1 ),
		  m_frame_generated( 0 )
	  {
		  memset( &m_node, 0, sizeof( node_t ) );
	  }
};

// class holding basic client information -------------------------------------
//
class E_BasicClientInfo 
{
public:
	int		m_nVersionMajor;
	int		m_nVersionMinor;
	char	m_szName	[ MAX_PLAYER_NAME  + 1 ];
	char	m_szOSLine  [ MAX_OSNAME_LEN   + 1 ];	
	char	m_szHostName[ MAX_HOSTNAME_LEN + 1 ];
	node_t	m_node;
	int		m_challenge;
	int		m_nRecvRate;				// byte/sec. this client wants data
	int		m_nSendFreq;				// packets/sec. this client sends data
	
public:
	E_BasicClientInfo ()
	{
		Reset();
	}

	void Reset()
	{
		m_nVersionMajor = -1;
		m_nVersionMinor = -1;
		memset( m_szName, 0, MAX_PLAYER_NAME  + 1 );
		memset( m_szOSLine,     0, MAX_OSNAME_LEN   + 1 );
		memset( m_szHostName,	0, MAX_HOSTNAME_LEN + 1 );
		memset( &m_node,		0, sizeof( node_t ) );
		m_challenge = -1;
		m_nRecvRate = DEFAULT_CLIENT_RECV_RATE;
		m_nSendFreq = DEFAULT_CLIENT_SEND_FREQUENCY;
	}

	// return the NET.SERVERRATE for this client
	int GetRecvRate() { return m_nRecvRate; }

	// return the NET.CLIENTRATE for this client
	int GetSendFreq() { return m_nSendFreq; }
};


// class holding client information ( when connecting ) -----------------------
//
class E_ClientConnectInfo : public E_BasicClientInfo
{
public:
	int			m_selected_slot;
public:
	E_ClientConnectInfo() : 
		E_BasicClientInfo(),
		m_selected_slot(-1)
	  {
	  }
};


// class holding client information ( when connected ) ------------------------
//
class E_ClientInfo : public E_BasicClientInfo
{
protected:
	int					m_slotfree;	
	int					m_nAliveCounter;
	E_SimPlayerInfo*	m_pSimPlayerInfo;

public:
	E_ClientInfo() : 
	  m_slotfree( TRUE ),
	  m_nAliveCounter( MAX_ALIVE_COUNTER ),
	  m_pSimPlayerInfo( NULL )
	{
	}

	void CopyClientConnectInfo( E_ClientConnectInfo* pClientConnectInfo );

	void Reset()
	{
		E_BasicClientInfo::Reset();

		m_slotfree		= TRUE;
		m_nAliveCounter = MAX_ALIVE_COUNTER;
	}

	// mark the client alive
	void MarkAlive();

	// check whether the client is alive
	int IsAlive();

	// check whether the slot is free
	int IsSlotFree() { return m_slotfree; }

	// set the status of slot 
	void SetSlotFree( int slotfree ) { m_slotfree = slotfree; }
};


// connection manager class ---------------------------------------------------
//
class E_ConnManager
{
protected:

	E_ClientChallengeInfo*		m_ChallengInfos;
	int						m_nMaxNumChallengeInfos;
	int						m_nCurChallengeInfo;

	E_ClientInfo*			m_ClientInfos;
	int						m_nNumConnected;

	int						m_inDisconnectClient;

	E_ConnManager();
	~E_ConnManager();

public:

	// SINGLETON pattern
	static E_ConnManager* GetConnManager()
	{
		static E_ConnManager _TheConnManager;
		return &_TheConnManager;
	}

	// request a challenge for a client
	int RequestChallenge( node_t* clientnode );

	enum ConnResults {
		CONN_CHALLENGE_INVALID   = 0,
		CONN_CLIENT_INCOMAPTIBLE = 1,
		CONN_CLIENT_BANNED       = 2,
		CONN_SERVER_FULL		 = 3,
		CONN_NAME_TAKEN			 = 4,
		CONN_OK					 = 5,
	};

	enum DisconnResults {
		DISC_NOT_CONNECTED		 = 0,
		DISC_OK					 = 1,
	};

	enum NameChangeResults {
		NAMECHANGE_NOT_CONNECTED = 0,
		NAMECHANGE_ALREADY_TAKEN = 1,
		NAMECHANGE_OK            = 2,
	};
	
	// check whether the client connection is valid
	int CheckClientConnect( E_ClientConnectInfo* pClientConnectInfo );

	// check for a client disconnect
	int CheckClientDisconnect( node_t* clientnode );

	// check for a client namechange
	int CheckNameChange( node_t* clientnode, char* newplayername );

	// get the client information for a specific slot
	E_ClientInfo* GetClientInfo( int nSlot );

	// check the alive counter of all connected clients and disconnect any timed out clients
	int CheckAliveStatus();

	// check node of client with node used at connection
	int CheckNodesMatch( int nClientID, node_t* node );

	// return the # of connected clients
	int GetNumConnected() { return m_nNumConnected; }

	// set some client-info fields according to RE
	void NET_ExecRmEvClientInfo( int nClientID, RE_ClientInfo* re_clientinfo );

	// disconnect a client
	int DisconnectClient( int nClientID );

	// return the name of the client
	char* GetClientName( int nClientID );

protected:

	// check if client version is compatible
	int _IsClientCompatible( E_ClientConnectInfo* pClientConnectInfo );

	// check if client is banned
	int _IsClientBanned( E_ClientConnectInfo* pClientConnectInfo );

	// check if challenge is correct
	int _IsChallengeCorrect( E_ClientConnectInfo* pClientConnectInfo, E_ClientChallengeInfo** pChallengeInfo );

	// ensure a client is not connected
	int _EnsureClientIsDisconnected( E_ClientConnectInfo* pClientConnectInfo );

	// connect a client 
	int _ConnectClient( E_ClientConnectInfo* pClientConnectInfo );
	
	// change the name of a client
	int _ChangeClientName( int nSlot, char* newplayername );

	// send client connected notifications to the other clients
	int _NotifyClientConnected( int nSlotConnected );

	// send client disconnected notifications to the other clients
	int _NotifyClientDisconnected( int nSlotDisconnected );

	// send client namechange notifications to the other clients
	int _NotifyClientNameChange( int nSlotNamechanged );

	// find the assigned slot for a specific node
	int _FindSlotWithNode( node_t* clientnode );

	// find the slot of a connected client by its name
	int _FindClientName( char* name );
};


#endif // _E_CONNMANAGER_H_
