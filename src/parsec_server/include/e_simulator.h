/*
* PARSEC HEADER: e_simulator.h
*/

#ifndef _E_SIMULATOR_H_
#define _E_SIMULATOR_H_

#include "net_util.h"
#include "utl_math.h"			// for playerlerp_s

// forward decls --------------------------------------------------------------
//
class E_Distributable;
class E_SimClientNetOutput;
class E_ClientInfo;
class E_GameServer;
class E_REList;
template <class T> class UTL_List;


//FIXME:
// gamecode stuff is skipped for now
#define GAMECODE( x )		{}

// gamecode2 is executed
#define GAMECODE2( x )      x


// class holding a ship state -------------------------------------------------
//
class E_SimShipState
{
protected:

	Xmatrx		m_ObjPosition;
	fixed_t		m_CurSpeed;			
	bams_t 		m_CurYaw;				
	bams_t 		m_CurPitch;			
	bams_t 		m_CurRoll;			
	geomv_t		m_CurSlideHorz;		
	geomv_t		m_CurSlideVert;

public:	
	// standard ctor
	E_SimShipState()
	{
		Reset();
	}

	// reset al data members
	void Reset();

	E_SimShipState& operator= ( E_SimShipState& _other )
	{
		ASSERT( FALSE );
		return _other;//remove the warning without it
	}

	// copy from other 
	E_SimShipState& CopyFrom( E_SimShipState& _other )
	{
		memcpy( &m_ObjPosition, &_other.m_ObjPosition, sizeof( Xmatrx ) );
		m_CurSpeed		= _other.m_CurSpeed;
		m_CurYaw		= _other.m_CurYaw;
		m_CurPitch		= _other.m_CurPitch;
		m_CurRoll		= _other.m_CurRoll;
		m_CurSlideHorz	= _other.m_CurSlideHorz;
		m_CurSlideVert	= _other.m_CurSlideVert;
		
		return *this;
	}

	// copy information from ShipRemInfo struct
	void CopyFromPlayerAndShipStatus( RE_PlayerAndShipStatus& _other )
	{
		memcpy( &m_ObjPosition, &_other.ObjPosition, sizeof( Xmatrx ) );
		m_CurSpeed		= _other.CurSpeed;
		m_CurYaw		= _other.CurYaw;
		m_CurPitch		= _other.CurPitch;
		m_CurRoll		= _other.CurRoll;
		m_CurSlideHorz	= _other.CurSlideHorz;
		m_CurSlideVert	= _other.CurSlideVert;
	}

	// various get/set functions
	pXmatrx GetObjPosition()				{ return m_ObjPosition; }

	//FIXME: get rid of friend classes
	friend class E_SimClientState;
	friend class E_REList;
};


// class handling the network input for a client -----------------------------
//
class E_SimClientState {
protected:
	E_SimShipState*			m_States;
	int							m_nNumStateSlots;

	E_SimShipState			m_InputState;
	dword						m_nInputStateMessageID;
	dword						m_nLastInputStateMessageId;

	refframe_t					m_LastSwitchToSimRefFrame;

	int							m_nClientID;

	enum ClientMovementMode_t { CMM_SIMULATING, CMM_SMOOTHING };
	ClientMovementMode_t        m_ClientMovementMode;
	playerlerp_s				m_playerlerp;

	bool_t						m_ResyncClient;
    bool_t                      m_ClientHasStateSync;

public:
	E_SimClientState()
	{
		m_States				   = NULL;
		m_nNumStateSlots		   = -1;
		m_nInputStateMessageID	   = 0;
		m_nLastInputStateMessageId = 0;
		m_ClientMovementMode	   = CMM_SIMULATING;
		m_ResyncClient			   = FALSE;
		m_LastSwitchToSimRefFrame  = -1;
        m_ClientHasStateSync       = FALSE;
	}

	~E_SimClientState()
	{
		Reset();
	}

	// reset all data-members
	void Reset()
	{
		if ( m_States != NULL ) {
			delete []m_States;
			m_States = NULL;
		}
		m_nNumStateSlots			= -1;
		m_nInputStateMessageID		= 0;
		m_nLastInputStateMessageId	= 0;
		m_InputState.Reset();
		m_ClientMovementMode		= CMM_SIMULATING;
		m_ResyncClient				= FALSE;
		m_LastSwitchToSimRefFrame	= -1;
        m_ClientHasStateSync       = FALSE;
	}

	// connect a client
	void Connect( int nClientID );

	// disconnect a client
	void Disconnect()
	{
		Reset();
	}

	// check the movement bounds between the last received state
	int CheckMovementBounds( dword MessageID, RE_PlayerAndShipStatus* pPAS );

	// store the received state as the newest 
	void StoreNewState( dword MessageID, RE_PlayerAndShipStatus* pPAS );

	// reset the sim state
	void ForceNewState( E_SimShipState* pSimShipState );
	
	// either apply simulation or smoothing to get newest client-state
	void CalcNewState( refframe_t CurSimRefFrames );

	// dump some SimClientState for debugging purposes
	void Dump();

	// return the current/last simulation-state-slot
	E_SimShipState* GetCurSimFrameStateSlot();
	E_SimShipState* GetPrevSimFrameStateSlot();

	// set the client resync flag ( RE_PLAYERANDSHIPSTATUS for this client sent from server to client )
	void SetClientResync() { m_ResyncClient = TRUE; }

	// clear the client resync flag
	void ClearClientResync() { m_ResyncClient = FALSE; }

	// check whether we must resync this client
	int NeedsResync() { return m_ResyncClient; }
    
    // Check whether state variables (nebula id, ammo pack sizes) have been sent
    int HasState() { return m_ClientHasStateSync; }
    
    void SetState() { m_ClientHasStateSync = TRUE; }

protected:
	// calculate the newest smooothing target
	void _CalcNewSmoothingTarget();

	// check whether the change from the last to the current state involved a movement
	int _CheckForMovement( dword MessageID, RE_PlayerAndShipStatus* pPAS );

	// update the message ids the lateste state comes from ( dead reckoning )
	void _UpdateStateMessageID( dword MessageID );
};


// class for handling the overall simulation as well as all the states --------
//
class E_Simulator {
protected:

	refframe_t					m_CurRefFrame;				// the current RefFrame
	refframe_t					m_CurSimRefFrames;			// refframes to advance in current sim step
	refframe_t					m_LastSimRefFrame;			// last sim refframe
	
	E_SimPlayerInfo*			m_SimPlayerInfos;
	E_SimClientState*			m_SimClientState;

	int							m_nSimFrame;

	// ctor/dtor
	E_Simulator();
	~E_Simulator();

public:
	// SINGLETON access
	static E_Simulator* GetSimulator()
	{
		static E_Simulator _TheSimulator;
		return &_TheSimulator;
	}

	// main simulation method
	int DoSim( refframe_t CurSimRefFrame );

	// get the PlayerInfo for a client
	E_SimPlayerInfo* GetSimPlayerInfo( int nClientID );

	// return the current simulation refframe
	refframe_t GetCurSimRefFrame() { return m_CurRefFrame; }

	// return the refframes the simulation runs in this sim-frame
	refframe_t GetThisFrameRefFrames() { return m_CurSimRefFrames; }

	// get the SimClientState for a specific client
	E_SimClientState* GetSimClientState( int nClientID );

	// get the latest valid player
	E_SimShipState* GetLatestSimShipState( int nClientID );

	// reset all data
	void Reset();

	// connect the player in a specific slot
	int ConnectPlayer( int nClientID );

	// disconnect the player in a specific slot
	int DisconnectPlayer( int nClientID );

	// check whether a player in a slot is joined
	int IsPlayerJoined( int nClientID );

	// check whether a player in a slot is connected
	int IsPlayerDisconnected( int nClientID );

	// return the current simulation frame
	int	GetSimFrame() { return m_nSimFrame; }

protected:

	// update the shipstate ( according to the current sim refframes )
	void _UpdateShipState( int nClientID );

	// simulate ship movements
	void _SimulateShips();

	// calculate the refframes we want to apply to this simulation step
	int _CalcSimRefFrames();

	// apply the current sim-state to the engine state ( E_SimShipState -> ShipObject ) 
	void _ApplySimToEngineState();

	// apply the current engine state to the sim-state ( ShipObject -> E_SimShipState ) 
	void _ApplyEngineToSimState();
};


#endif // _E_SIMULATOR_H_
