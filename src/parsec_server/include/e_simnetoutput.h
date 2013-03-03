#ifndef E_SIMNETOUTPUT_H_
#define E_SIMNETOUTPUT_H_


// update todos ---------------------------------------------------------------
//
#define UPDATE_TODO_NOTHING					0x0000
#define UPDATE_TODO_SEND					0x0001
#define UPDATE_TODO_MASK					0x0001

// update mode  ---------------------------------------------------------------
//
#define UPDATEMODE_SENDMODE_UNRELIABLE		0x0000
#define UPDATEMODE_SENDMODE_RELIABLE		0x0010
#define UPDATEMODE_SENDMODE_MASK			0x0010

#define UPDATEMODE_SENDWHAT_STATE			0x0000
#define UPDATEMODE_SENDWHAT_REMOVE			0x0020
#define UPDATEMODE_SENDWHAT_MASK			0x0020

#define UPDATEMODE_FLAGS_SEND_TO_OWNER		0x0100
#define UPDATEMODE_FLAGS_MASK				0xFF00


enum {
    
	RMEVSTATE_NEBULAID,
	RMEVSTATE_PROBEXTRA,
	RMEVSTATE_PROBHELIX,
	RMEVSTATE_PROBLIGHTNING,
	RMEVSTATE_PROBPHOTON,
	RMEVSTATE_PROBMINE,
	RMEVSTATE_PROBREPAIR,
	RMEVSTATE_PROBAFTERBURNER,
	RMEVSTATE_PROBHOLODECOY,
	RMEVSTATE_PROBINVISIBILITY,
	RMEVSTATE_PROBINVULNERABILITY,
	RMEVSTATE_PROBENERGYFIELD,
	RMEVSTATE_PROBLASERUPGRADE,
	RMEVSTATE_PROBLASERUPGRADE1,
	RMEVSTATE_PROBLASERUPGRADE2,
	RMEVSTATE_PROBMISSPACK,
	RMEVSTATE_PROBDUMBPACK,
	RMEVSTATE_PROBGUIDEPACK,
	RMEVSTATE_PROBSWARMPACK,
	RMEVSTATE_PROBEMPUPGRADE1,
	RMEVSTATE_PROBEMPUPGRADE2,
	RMEVSTATE_AMAZING,
	RMEVSTATE_BRILLIANT,
	RMEVSTATE_KILLLIMIT,
    RMEVSTATE_ENERGYBOOST,
    RMEVSTATE_REPAIRBOOST,
    RMEVSTATE_DUMBPACK,
    RMEVSTATE_HOMPACK,
    RMEVSTATE_SWARMPACK,
    RMEVSTATE_PROXPACK,
    
	RMEVSTATE_NUMSTATES
};

// info about a E_Distributable -------------------------------------------------
//
class E_Distributable
{
protected:
	dword				m_objectid;
	int					m_listno;
	int					m_update_mode;
	byte*				m_update_to_client;
public:
	E_Distributable( dword objectid, int listno, int reliable, int send_to_owner ) : 
		m_objectid( objectid ),
		m_listno( listno ),
		m_update_mode( UPDATE_TODO_NOTHING ),
		m_update_to_client( NULL )
	{
		// translate to internal format
		if ( reliable ) {
			m_update_mode |= UPDATEMODE_SENDMODE_RELIABLE;
		}
		if ( send_to_owner ) {
			m_update_mode |= UPDATEMODE_FLAGS_SEND_TO_OWNER;
		}

		ResetUpdateInfo();
	}

	~E_Distributable()
	{
		delete []m_update_to_client;
	}

	// reset the update info
	void ResetUpdateInfo();

	// get the object id
	dword GetObjectID() { return m_objectid; }

	// return the list # the object belongs to
	int GetListNo() { return m_listno; }

	// set updatemode for all clients to STATE
	void UpdateMode_STATE();

	// set updatemode for all clients to REMOVE
	void UpdateMode_REMOVE();

	// check whether this E_Distributable is marked as REMOVE
	int IsInRemoving()
	{
		return ( ( m_update_mode & UPDATEMODE_SENDWHAT_MASK ) == UPDATEMODE_SENDWHAT_REMOVE );
	}

	// return whether the E_Distributable will be sent to the owner
	int WillBeSentToOwner()
	{
		return ( ( m_update_mode & UPDATEMODE_FLAGS_SEND_TO_OWNER ) != 0 );
	}

	// check whether the E_Distributable is a ZOMBIE ( client has received the REMOVE update )
	int IsZombie( int nClientID );

	// check whethe the E_Distributable has an update for a specific client
	int HasUpdate( int nClientID );

	// mark, that we sent an update to a client
	void MarkUpdateSent( int nClientID );

	// mark, that we need to send an update to a client
	void MarkForUpdate( int nClientID );

	// check whether the E_Distributable should be sent reliable
	int NeedsReliable()
	{
		return ( ( m_update_mode & UPDATEMODE_SENDMODE_MASK ) == UPDATEMODE_SENDMODE_RELIABLE );
	}

	// write a corresponding RE to the REList
	void DistributeToREList( E_REList* relist );

	// determine the size this E_Distributable is
	size_t DetermineSizeInPacket();
};





// send modi for client updates -----------------------------------------------
//
#define SEND_MODE_NONE			0
#define SEND_MODE_UNRELIABLE	1
#define SEND_MODE_RELIABLE		2

#define MAX_NUM_DISTRIBUTABLES_TO_SEND_PER_PACKET		256

// information about packets sent out ( time and size ) -----------------------
//
struct sent_packet_info_s {

	sent_packet_info_s()
	{
		Reset();
	}

	void Reset()
	{
		m_size = 0;
		m_sendtime = 0;
	}

	size_t		m_size;
	refframe_t	m_sendtime;
};


// class handling the network output for a client -----------------------------
// class with all necessary information on what to send to the destination client
class E_SimClientNetOutput
{
protected:

	int							m_nDestClientID;

	E_REList*					m_pReliableBuffer;
	E_REList*					m_pUnreliableBuffer;

	sent_packet_info_s*			m_SentPacketInfo;
	unsigned int				m_nPacket;

	refframe_t					m_LastSendFrame;
	refframe_t					m_Send_FrameTime;
	
	refframe_t					m_Heartbeat_Timeout_Frame;

	int							m_nNumPacketSlots;
	int							m_nAveragePacketSize;

	// list of all distributables for the client
	UTL_List<E_Distributable*>*	m_AllDistributables;

protected:

	// list of client ids the client is to be updated in the next packet
	int*			m_ClientIDList;		
	bool_t*			m_SendReliable;
	int				m_nNumClients;

	// inidicate that we should send a the state of the client itself to the destination client ( energy, damage, shields )
	bool_t			m_bIncludeDestClientState;

	// indicate whether to send the gamestate to the client
	bool_t			m_bIncludeGameState;

	// indicate whether to send the killstats to the client
	bool_t			m_bIncludeKillStats;

    //Indicate whether to send state variables to client
    bool_t          m_bIncludeStateSync;
    
	// array of distributables for next packet
	E_Distributable*	m_DistsForNextPacket[ MAX_NUM_DISTRIBUTABLES_TO_SEND_PER_PACKET ];
	int				m_nNumDistsForNextPacket;

	// # of byte available for the update
	int				m_nSizeAvail;

public:

	// default ctor/dtor
	E_SimClientNetOutput();
	~E_SimClientNetOutput();

	// connect a client
	void Connect( int nClientID );

	// disconnect a client
	void Disconnect();

	// sent udpate information to a client if bandwidth available
	int SendUpdateToClient();

	// the NET.SERVERRATE for this client has changed - recalc frame-time
	void RateChanged();

	// (re)calculate the average packet size
	void CalculateAveragePacketSize();

	// schedule a E_Distributable to be sent to the client
	void ScheduleDistributable( E_Distributable* pDist, int check_unique = FALSE );

	// buffer an RE for output
	void BufferRE( RE_Header* re, int reliable );

protected:

	// reset all data members
	void _Reset();

	// clear the list of things to update 
	void _ClearUpdates();

	// check whether there are any updates to send to the client
	int _HasUpdates();

	// add a E_Distributable to be sent out with the next packet
	void _AddDistForOutput( E_Distributable* pDist );

	// include the information about a client in the next update ( reliable or unreliable )
	void _FlagIncludeClient( int nClientID, bool_t bReliable );

	// check whether we can reserve size for output, and do so if space avail.
	bool_t _ReserveForOutput( int size );

	// fill the RE lists to be sent to the client
	int _FillPacketForClient( E_REList* pReliable, E_REList* pUnreliable );

	// getters/setters
	size_t _GetSizeAvail()	{ return m_nSizeAvail; }

	// check whether we should send a packet to this client
	int _CanSendPacket();

	// create buffer E_RELists for pass-through multicasts
	void _CreateBuffers();

	// terminate the multicast data in the E_REList buffers
	void _TerminateBufferMulticast();

	// determine which states must be updated for this client
	int _PrepareClientUpdateInfo();

	// check whether to send a heartbeat to the client
	int _ShouldSendHeartbeat();

	// set the new heartbeat timeout value
	void _DoHeartbeat();
};



// class for handling the network output from the simulation ------------------
//
class E_SimNetOutput {
protected:
	E_SimClientNetOutput*		m_SimClientNetOutput;
	UTL_List<E_Distributable*>*	m_Distributables;

protected:
	E_SimNetOutput();
	~E_SimNetOutput();

public:
	// SINGLETON access
	static E_SimNetOutput* GetSimNetOutput()
	{
		static E_SimNetOutput _TheSimNetOutput;
		return &_TheSimNetOutput;
	}

	// reset all data
	void Reset();

	// connect the player in a specific slot 
	int ConnectPlayer( int nClientID );

	// disconnect the player in a specific slot
	int DisconnectPlayer( int nClientID );

	// update all clients
	void DoClientUpdates();

	// buffer a RE for sending to all clients minus sender
	void BufferForMulticastRE( RE_Header* relist, int nSenderClientID, int reliable );
    
    // buffer a RE for sending to a specific client
    void BufferForDirectRE( RE_Header* relist, int nClientID, int reliable );
    
	// update NET.SERVERRATE dependent stuff of E_SimClientNetOutput
	void RateChangedForClient( int nClientID );

	// recalculate the average packet sizes for all connected clients
	void RecalcAveragePacketSizes();

	// create a new E_Distributable for all connected clients
	E_Distributable* CreateDistributable( GenObject* object, int reliable = FALSE, int send_to_owner = FALSE );

	// release a E_Distributable
	void ReleaseDistributable( E_Distributable* pDist );

	// cleanup all distributables, that are complete zombies
	void CleanupZombieDistributables();
   
protected:
	// destroy m_Distributables
	void _DestroyDistributables();

};


#endif // !E_SIMNETOUTPUT_H_
