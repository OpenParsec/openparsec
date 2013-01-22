#ifndef E_SIMPLAYERINFO_H_
#define E_SIMPLAYERINFO_H_


// class holding all simulation specific player information -------------------
//
class E_SimPlayerInfo
{
protected:
	int						m_Status;				// status of the player (inactive,connected,joined)
	int						m_objclass;				// the objectclass selected as ship when joined

	ShipObject*				m_pShip;
	int						m_nShipID;
	int						m_nClientID;

	int						m_IgnoreJoinUntilUnjoinFromClient;
public:
	E_SimPlayerInfo()
	{
		Reset();
	}

	// reset all player fields to defaults ( not connected )
	void Reset();
	
	// check whether a player is disconnected
	bool_t IsPlayerDisconnected() { return ( m_Status == PLAYER_INACTIVE ); }

	// check whether a player is connected
	bool_t IsPlayerConnected() { return ( m_Status == PLAYER_CONNECTED ); }
	
	// check whether a player is joined in the game
	bool_t IsPlayerJoined() { return ( m_Status == PLAYER_JOINED ); }

	// set the desired player status ( inactive/joined/unjoined )
	void SetDesiredPlayerStatus( RE_PlayerStatus* playerstatus );

	// update tables and create ship for newly joined player
	void PerformJoin( RE_PlayerStatus* pas_status );
	
	// update tables and delete ship of player who unjoined
	void PerformUnjoin( RE_PlayerStatus* playerstatus );

	// set the player status to connected
	void Connect( int nClientID );

	// set the player status to disconnected
	void Disconnect();

	// set flag to ignore all joins until we get an unjoin from the client
	void IgnoreJoinUntilUnjoinFromClient()
	{
		m_IgnoreJoinUntilUnjoinFromClient = TRUE;
	}

	// get a pointer to the ship object
	ShipObject* GetShipObject() { return m_pShip; }

	// get the object class of the ship
	int GetShipObjectClass() { return m_objclass; }

	// return the status of the player
	int GetStatus() { return m_Status; }
};

#endif // E_SIMPLAYERINFO_H_
