/*
 * PARSEC - Server-side Simulator OUTPUT
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:46 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001-2002
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <math.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "net_defs.h"

// UNP header
#include "net_wrap.h"

// server defs
//#include "e_defs.h"

// network code config
//#include "net_conf.h"

// net game header
#include "net_game_sv.h"

// mathematics header
//#include "utl_math.h"

// utility headers
#include "utl_list.h"

// local module header
#include "e_simnetoutput.h"

// proprietary module headers
#include "con_aux_sv.h"
//#include "g_colldet.h"
#include "g_extra.h"
//#include "net_csdf.h"
#include "net_packetdriver.h"
//#include "net_stream.h"
//#include "obj_creg.h"
//#include "e_stats.h"
#include "g_main_sv.h"
//#include "e_simplayerinfo.h"
#include "e_simulator.h"
//#include "e_simnetinput.h"
#include "e_connmanager.h"
#include "e_gameserver.h"
#include "e_packethandler.h"
//#include "e_relist.h"
#include "sys_refframe_sv.h"





// max. packets per client per second -----------------------------------------
//
#define MAX_PACKETS_PER_SEC		TheServer->GetSimFrequency()

// standard ctor --------------------------------------------------------------
//
E_SimClientNetOutput::E_SimClientNetOutput()
{
	m_SentPacketInfo		= NULL;
	m_nPacket				= 0;
	m_pReliableBuffer		= NULL;
	m_pUnreliableBuffer		= NULL;
	m_nNumPacketSlots		= 0;
	m_AllDistributables		= NULL;
	m_nDestClientID				= PLAYERID_ANONYMOUS;

	m_ClientIDList			= NULL;
	m_SendReliable			= NULL;
	m_nNumDistsForNextPacket= 0;
	m_nSizeAvail			= NET_UDP_DATA_LENGTH;

	m_bIncludeDestClientState= FALSE;
}


// standard dtor --------------------------------------------------------------
//
E_SimClientNetOutput::~E_SimClientNetOutput()
{
	Disconnect();
}


// reset all data members -----------------------------------------------------
//
void E_SimClientNetOutput::_Reset()
{
	if ( m_pReliableBuffer != NULL ) {
		m_pReliableBuffer->Release();
		m_pReliableBuffer = NULL;
	}

	if ( m_pUnreliableBuffer != NULL ) {
		m_pUnreliableBuffer->Release();
		m_pUnreliableBuffer	= NULL;
	}

	delete []m_SentPacketInfo;
	delete m_AllDistributables;
	m_SentPacketInfo = NULL;
	m_AllDistributables = NULL;

	m_nDestClientID					= PLAYERID_ANONYMOUS;
	m_nPacket					= 0;
	m_LastSendFrame				= 0;
	m_Send_FrameTime    		= 0;
	m_nNumPacketSlots			= 0;
	m_nAveragePacketSize		= NET_UDP_DATA_LENGTH;
	m_Heartbeat_Timeout_Frame	= -1;

	delete []m_ClientIDList;
	delete []m_SendReliable;
	m_ClientIDList = NULL;
	m_SendReliable = NULL;
}


// connect a client -----------------------------------------------------------
//
void E_SimClientNetOutput::Connect( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	ASSERT( m_SentPacketInfo	== NULL );
	ASSERT( m_AllDistributables == NULL );
	ASSERT( m_pReliableBuffer   == NULL );
	ASSERT( m_pUnreliableBuffer == NULL );

	m_nDestClientID	= nClientID;

	m_nNumPacketSlots = MAX_PACKETS_PER_SEC * TheServer->GetPacketAverageSecs();
	m_SentPacketInfo = new sent_packet_info_s[ m_nNumPacketSlots ];
	m_nPacket = 0;
	m_nAveragePacketSize = NET_UDP_DATA_LENGTH;

	RateChanged();
	m_LastSendFrame = 0;

	m_AllDistributables = new UTL_List<E_Distributable*>;

	// --
	m_ClientIDList = new int   [ MAX_NUM_CLIENTS ];
	m_SendReliable = new bool_t[ MAX_NUM_CLIENTS ];
	_ClearUpdates();

	_DoHeartbeat();
}


// disconnect a client --------------------------------------------------------
//
void E_SimClientNetOutput::Disconnect()
{
	_Reset();
}


// choke client update to the rate (byte/sec.) set ----------------------------
// 
int E_SimClientNetOutput::_CanSendPacket()
{
	refframe_t now = SYSs_GetRefFrameCount();

	//printf( "now: %d, m_LastSendFrame: %d, m_Send_FrameTime: %d\n", now, m_LastSendFrame, m_Send_FrameTime );
	
	// check whether frame has already passed
	if ( ( now - m_LastSendFrame ) < m_Send_FrameTime ) {
		return FALSE;
	} else {
		m_LastSendFrame = now;
		return TRUE;
	}
}


// set the new heartbeat timeout value ----------------------------------------
//
void E_SimClientNetOutput::_DoHeartbeat() 
{ 
	//FIXME: change to based timing
	m_Heartbeat_Timeout_Frame = SYSs_GetRefFrameCount() + TheServer->GetClientUpdateHeartbeat();
}


// check whether to send a heartbeat to the client ----------------------------
//
int E_SimClientNetOutput::_ShouldSendHeartbeat() 
{ 
	return ( m_Heartbeat_Timeout_Frame < SYSs_GetRefFrameCount() ); 
}


// schedule a E_Distributable to be sent to the client --------------------------
//
void E_SimClientNetOutput::ScheduleDistributable( E_Distributable* pDist, int check_unique /*= FALSE*/ )
{
	// only allow distributables where destination client is not the owner
	if ( ( GetOwnerFromHostOjbNumber( pDist->GetObjectID() ) != (dword)m_nDestClientID ) || pDist->WillBeSentToOwner() ) {

		// check that the E_Distributable is not already in the list for this client ( STATE & REMOVE in same packet-send-frame )
		if ( check_unique ) {
			if ( m_AllDistributables->Find( pDist ) != NULL ) {
				DBGTXT( MSGOUT( "DIST: E_SimClientNetOutput::ScheduleDistributable():   IGNORING DUPLICATE client %d objectid %x listno %d reliable %d", m_nDestClientID, pDist->GetObjectID(), pDist->GetListNo(), pDist->NeedsReliable() ); );
				return;
			}
		}

		m_AllDistributables->AppendTail( pDist );

		DBGTXT( MSGOUT( "DIST: E_SimClientNetOutput::ScheduleDistributable():   client %d objectid %x listno %d reliable %d", m_nDestClientID, pDist->GetObjectID(), pDist->GetListNo(), pDist->NeedsReliable() ); );
	}
}


// (re)calculate the average packet size --------------------------------------
//
void E_SimClientNetOutput::CalculateAveragePacketSize()
{
	size_t nTotalSize = 0;
	for( int nPktSlot = 0; nPktSlot < m_nNumPacketSlots; nPktSlot++ ) {
		nTotalSize += m_SentPacketInfo[ nPktSlot ].m_size;
	}
	int nNewAveragePacketSize = nTotalSize / m_nNumPacketSlots;
	if ( m_nAveragePacketSize != nNewAveragePacketSize ) {
		m_nAveragePacketSize = nNewAveragePacketSize;
		//MSGOUT( "NEW AveragePacketSize for %d is %d", m_nDestClientID, m_nAveragePacketSize );

		// recalc send-frametime
		RateChanged();
	}
}


// the RATE for this client has changed - recalc frame-time -------------------
//
void E_SimClientNetOutput::RateChanged()
{
	// get the frequency
	float fClientSendFreq = (float)TheConnManager->GetClientInfo( m_nDestClientID )->GetRecvRate() / (float)m_nAveragePacketSize;

	//FIXME: is this the correct value to clamp to ????
	if ( fClientSendFreq > (float)TheServer->GetSimFrequency() ) {
		//MSGOUT( "NEW ClientSendFreq for %d clamped to %d", m_nDestClientID, TheServer->GetSimFrequency() );
		fClientSendFreq = (float)TheServer->GetSimFrequency();
	} else {
	
		//FIXME: clamp client send frequency to 0.5 Hz
		//if ( fClientSendFreq < )

		//MSGOUT( "NEW ClientSendFreq for %d is %f", m_nDestClientID, fClientSendFreq );
	}

	// calculate the refframes between 2 packets to this client
	m_Send_FrameTime = (refframe_t)((float)FRAME_MEASURE_TIMEBASE / (float)fClientSendFreq);
}


// buffer an RE for output ----------------------------------------------------
//
void E_SimClientNetOutput::BufferRE( RE_Header* re, int reliable )
{
	ASSERT( re != NULL );

	// reserve 
	size_t resize = E_REList::RmEvGetSize( re );
	size_t size   = E_REList::RmEvGetSizeFromType( RE_OWNERSECTION );
	if ( !_ReserveForOutput( size + resize ) ) {
		DBGTXT( MSGOUT( "E_SimClientNetOutput::BufferRE(): overflow" ); );
		return;
	}

	// create buffers
	_CreateBuffers();

	E_REList* pBufferREList = ( reliable ? m_pReliableBuffer : m_pUnreliableBuffer );
	ASSERT( pBufferREList != NULL );

	// append RE_OWNERSECTION and supplied remote event to buffer RE list
	pBufferREList->NET_Append_RE_OwnerSection( m_nDestClientID );
	pBufferREList->AppendEvent( re, resize );
}


// determine which states must be updated at the destination client -----------
//
int E_SimClientNetOutput::_PrepareClientUpdateInfo()
{
	//NOTE: this method prepares the update to the client. that is, it decides
	//      what to send in the next packet ( game-state, kill-statsl, client
	//      info, destination client info, distributables ).
	//      this information is stored in m_ClientUpdateInfo and is used
	//      in E_SimClientNetOutput::_FillPacketForClient()

	ASSERT( ( m_nDestClientID >= 0 ) && ( m_nDestClientID < MAX_NUM_CLIENTS ) );

	// start with empty update list
	_ClearUpdates();

	//FIXME: here we must add much better heuristic to decide what to send to a 
	//       specific client (e.g. distance based, direction based, joined/unjoined, 
	//       last time info sent etc. )

	E_SimClientState* pSimClientState = TheSimulator->GetSimClientState( m_nDestClientID );

	// include the destination client's state if no resync and if player joined
	m_bIncludeDestClientState = ( pSimClientState->NeedsResync() == FALSE ) && TheSimulator->IsPlayerJoined( m_nDestClientID );

#define LAG_CLIENT_STATE
#ifdef LAG_CLIENT_STATE
	m_bIncludeDestClientState &= _ShouldSendHeartbeat();
#endif // LAG_CLIENT_STATE

	if(!pSimClientState->HasState()) {
        size_t state_size = E_REList::RmEvGetSizeFromType(RE_STATESYNC) * 7; //We current send 7 states to the client
        if(_ReserveForOutput( state_size )) {
            m_bIncludeStateSync = SEND_MODE_UNRELIABLE;
        }
        
    }
    
    // check whether to include the gamestate ( RE_GameState ) in a heartbeat packet
	//FIXME: why only if NeedsResync() == FALSE ?
	if ( !pSimClientState->NeedsResync() && _ShouldSendHeartbeat() ) {

		size_t size = E_REList::RmEvGetSizeFromType( RE_KILLSTATS ) + 
			          E_REList::RmEvGetSizeFromType( RE_GAMESTATE );

		if ( _ReserveForOutput( size ) ) {

			m_bIncludeGameState = SEND_MODE_UNRELIABLE;
			m_bIncludeKillStats = SEND_MODE_UNRELIABLE;

			_DoHeartbeat();
		}
	}

	// iterate over all connected clients ( check for forced resync )
	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
		if( !TheSimulator->IsPlayerDisconnected( nClientID ) ) {

			bool_t bIncludeUnreliable = false;
			bool_t bIncludeReliable   = false;

			// state information of other clients is sent UNRELIABLE
			if ( nClientID != m_nDestClientID ) {
				bIncludeUnreliable = true;
			} else {

				// send info about client itself, if we need a resync
				if ( pSimClientState->NeedsResync() ) {

					// disable client resync
					pSimClientState->ClearClientResync();

					DBGTXT( 
					{	
						dword dwMsg = ThePacketDriver->GetNextOutMessageId( nClientID ); 
						MSGOUT( "NeedsResync() for client %d. including server-state in update packet %d.", m_nDestClientID, dwMsg ); } 
					);

					// forced resync ( state information about the destination client ) is sent RELIABLE
					bIncludeReliable = true;
				} else {

					// include destination clients' state in heartbeat packets
					if ( m_bIncludeDestClientState ) {
						bIncludeUnreliable = true;
					}	
				}
			}

			// include the client in the update ?
			if ( bIncludeUnreliable || bIncludeReliable ) {

				if ( _ReserveForOutput( E_REList::RmEvGetSizeFromType( RE_PLAYERANDSHIPSTATUS ) ) ) {
					_FlagIncludeClient( nClientID, bIncludeReliable );
				}
			} 
		}
	}

	
#ifdef PARSEC_DEBUG

	int nIndex = 0;
	for( UTL_listentry_s<E_Distributable*>* entry1 = m_AllDistributables->GetHead(); entry1 != NULL; ) {
		E_Distributable* pDist = entry1->m_data;
		DBGTXT( MSGOUT( "DIST: DUMP of m_AllDistributables[ %d ]: client %d objectid %x listno %d reliable %d", nIndex, m_nDestClientID, pDist->GetObjectID(), pDist->GetListNo(), pDist->NeedsReliable() ); );
		entry1 = entry1->m_pNext;
		nIndex++;
	}

#endif 


	// iterate through all distributables for this client and prioritize them 
	for( UTL_listentry_s<E_Distributable*>* entry = m_AllDistributables->GetHead(); entry != NULL; ) {

		E_Distributable* pDist = entry->m_data;
		ASSERT( pDist->HasUpdate( m_nDestClientID ) );

		// get next in list
		UTL_listentry_s<E_Distributable*>* nextentry = entry->m_pNext;

		// reserve space in output packet
		if ( _ReserveForOutput( pDist->DetermineSizeInPacket() ) ) {

			ASSERT( _GetSizeAvail() > 0 );

			//FIXME: prioritize distributables
			_AddDistForOutput( pDist );

			DBGTXT( MSGOUT( "DIST: E_SimClientNetOutput::_PrepareClientUpdateInfo(): client %d objectid %x listno %d reliable %d", m_nDestClientID, pDist->GetObjectID(), pDist->GetListNo(), pDist->NeedsReliable() ); );

			// remove from list
			m_AllDistributables->RemoveEntry( entry );
		} else {
			//FIXMEL
			DBGTXT( MSGOUT( "DIST: E_SimClientNetOutput::_PrepareClientUpdateInfo(): SKIPPING due to space constraints. client %d objectid %x listno %d reliable %d", m_nDestClientID, pDist->GetObjectID(), pDist->GetListNo(), pDist->NeedsReliable() ); );
		}

		// step to next entry
		entry = nextentry;
	}


	return _HasUpdates();
}


// fill the RE lists to be sent to the client ---------------------------------
//
int E_SimClientNetOutput::_FillPacketForClient( E_REList* pReliable, E_REList* pUnreliable )
{
	ASSERT( pReliable	!= NULL );
	ASSERT( pUnreliable != NULL );

	//FIXME: we want to send the state of all players as unreliable and not necessarely the whole state in one packet

	//FIXME: we get a serious space problem with our default packet size here, if more 
	//       than 10 (=1000/92) players connected. strip size of RE_PlayerAndShipStatus and increase max. 
	//       packetsize to ~1400 bytes ( should be MTU of Ethernet packet minus PPP/IP/UDP headers )

	// append a RE containing state for each connected player ( all but the destination player )
	for( int nAboutClient = 0; nAboutClient < m_nNumClients; nAboutClient++ ) {

		// get the client-id we want to send information about
		int nClientID = m_ClientIDList[ nAboutClient ];

		// get the result state from last simulation run 
		// NOTE: as the simframe counter is increased at the end of each sim-frame and the network update 
		//       is done after that, we need to get the state from the last-sim-frame here
		E_SimShipState* pPrevState = TheSimulator->GetSimClientState( nClientID )->GetPrevSimFrameStateSlot();

		//DBGTXT( LOGOUT( "SENDING info about %d to %d: phi: %f", nClientID, nDestClientID, RAD_TO_DEG( acos( pPrevState->m_ObjPosition[ 0 ][ 0 ] ) ) ); );

		// determine in which RE list we want to stuff the state
		E_REList* relist = ( m_SendReliable[ nAboutClient ] ? pReliable : pUnreliable );

		bool_t bUpdatePropsOnly = ( ( nClientID == m_nDestClientID ) && m_bIncludeDestClientState );
		if ( !relist->NET_Append_RE_PlayerAndShipStatus( nClientID, 
														 TheSimulator->GetSimPlayerInfo( nClientID ), 
														 pPrevState, 
														 TheSimulator->GetCurSimRefFrame(),
														 bUpdatePropsOnly ) ) {
			DBGTXT( MSGOUT( "E_SimClientNetOutput::_FillPacketForClient(): packet choke" ); );
			return FALSE;
		}
	}

    // include a StateSync ?
    if (m_bIncludeStateSync != SEND_MODE_NONE) {
        E_SimClientState* pSimClientState = TheSimulator->GetSimClientState( m_nDestClientID );
        E_REList* relist = ( m_bIncludeStateSync == SEND_MODE_UNRELIABLE ? pUnreliable : pReliable );
        if(!relist->RmEvStateSync(RMEVSTATE_NEBULAID,TheGame->m_NebulaID)) {
            DBGTXT( MSGOUT( "E_SimClientNetOutput::_FillPacketForClient(): packet choke" ); );
			return FALSE;
        }
        if(!relist->RmEvStateSync(RMEVSTATE_ENERGYBOOST,TheGame->EnergyExtraBoost)) {
            DBGTXT( MSGOUT( "E_SimClientNetOutput::_FillPacketForClient(): packet choke" ); );
			return FALSE;
        }
        if(!relist->RmEvStateSync(RMEVSTATE_REPAIRBOOST,TheGame->RepairExtraBoost)) {
            DBGTXT( MSGOUT( "E_SimClientNetOutput::_FillPacketForClient(): packet choke" ); );
			return FALSE;
        }
        if(!relist->RmEvStateSync(RMEVSTATE_DUMBPACK,TheGame->DumbPackNumMissls)) {
            DBGTXT( MSGOUT( "E_SimClientNetOutput::_FillPacketForClient(): packet choke" ); );
			return FALSE;
        }
        if(!relist->RmEvStateSync(RMEVSTATE_HOMPACK,TheGame->HomPackNumMissls)) {
            DBGTXT( MSGOUT( "E_SimClientNetOutput::_FillPacketForClient(): packet choke" ); );
			return FALSE;
        }
        if(!relist->RmEvStateSync(RMEVSTATE_SWARMPACK,TheGame->SwarmPackNumMissls)) {
            DBGTXT( MSGOUT( "E_SimClientNetOutput::_FillPacketForClient(): packet choke" ); );
			return FALSE;
        }
        if(!relist->RmEvStateSync(RMEVSTATE_PROXPACK,TheGame->ProxPackNumMines)) {
            DBGTXT( MSGOUT( "E_SimClientNetOutput::_FillPacketForClient(): packet choke" ); );
			return FALSE;
        }
        pSimClientState->SetState();
    }
    
    // include a RE_GAMESTATE ?
	if ( m_bIncludeGameState != SEND_MODE_NONE ) {
		E_REList* relist = ( m_bIncludeGameState == SEND_MODE_UNRELIABLE ? pUnreliable : pReliable );
		if ( !relist->NET_Append_RE_GameState() ) {
			DBGTXT( MSGOUT( "E_SimClientNetOutput::_FillPacketForClient(): packet choke" ); );
			return FALSE;
		}
	}

	// include a RE_KILLSTATS ?
	if ( m_bIncludeKillStats != SEND_MODE_NONE ) {
		E_REList* relist = ( m_bIncludeKillStats == SEND_MODE_UNRELIABLE ? pUnreliable : pReliable );
		if ( !relist->NET_Append_RE_KillStats() ) {
			DBGTXT( MSGOUT( "E_SimClientNetOutput::_FillPacketForClient(): packet choke" ); );
			return FALSE;
		}
	}

	// create remote events for all distributables to send in this packet
	for( int nEntry = 0; nEntry < m_nNumDistsForNextPacket; nEntry++ ) {

		E_Distributable* pDist = m_DistsForNextPacket[ nEntry ];
		ASSERT( pDist->HasUpdate( m_nDestClientID ) );

		E_REList* relist = pDist->NeedsReliable() ? pReliable : pUnreliable;

		// write RE to list
		pDist->DistributeToREList( relist );

		// mark that we sent the update
		pDist->MarkUpdateSent( m_nDestClientID );
	}

	return TRUE;
}


// create buffer E_RELists for pass-through multicasts ----------------------
//
void E_SimClientNetOutput::_CreateBuffers()
{
	if ( m_pReliableBuffer == NULL )
		m_pReliableBuffer = E_REList::CreateAndAddRef( RE_LIST_MAXAVAIL );

	if ( m_pUnreliableBuffer == NULL )
		m_pUnreliableBuffer	= E_REList::CreateAndAddRef( RE_LIST_MAXAVAIL );
}


// terminate the multicast data in the E_REList buffers ---------------------
//
void E_SimClientNetOutput::_TerminateBufferMulticast()
{
	// if we have buffered multicasts from other clients, we must append a RE_OWNERSECTION with PLAYERID_SERVER
	if ( m_pReliableBuffer && m_pReliableBuffer->HasEvents() ) {
		bool_t rc = _ReserveForOutput( E_REList::RmEvGetSizeFromType( RE_OWNERSECTION ) );
		//ASSERT( rc );
        if(!rc)
            MSGOUT("CRAZYSPENCE: If an odd issue starts cropping up, line 555 esimnetoutput is where to look");
		m_pReliableBuffer->NET_Append_RE_OwnerSection( PLAYERID_SERVER );
	}
	if ( m_pUnreliableBuffer && m_pUnreliableBuffer->HasEvents() ) {
		bool_t rc = _ReserveForOutput( E_REList::RmEvGetSizeFromType( RE_OWNERSECTION ) );
        if(!rc)
           MSGOUT("CRAZYSPENCE: If an odd issue starts cropping up, line 561 esimnetoutput is where to look");
		m_pUnreliableBuffer->NET_Append_RE_OwnerSection( PLAYERID_SERVER );
	}
}


// sent udpate information to a client if bandwidth available -----------------
//
int E_SimClientNetOutput::SendUpdateToClient()
{
	ASSERT( m_nDestClientID != PLAYERID_ANONYMOUS );

	bool_t rc = false;

	// choke client to its max. rate
	if ( _CanSendPacket() == FALSE ) {

		// clear this packet-slot
		sent_packet_info_s* spi = &m_SentPacketInfo[ m_nPacket % m_nNumPacketSlots ];
		spi->m_size		= 0;
		spi->m_sendtime	= SYSs_GetRefFrameCount();

		//DBGTXT( MSGOUT( "E_SimClientNetOutput::UpdateClient(): RATE_CHOKE" ); );
		//TheStatsManager->IncStats( m_nDestClientID, STAT_RATE_CHOKE, 1 );
		return FALSE;
	}

	// create buffers ( buffers evt. already contain pass-through multicasts from other clients )
	_CreateBuffers();

	// terminate the buffered multicasts by a RE_OWNERSECTION( PLAYERID_SERVER )
	_TerminateBufferMulticast();

	// prepare the client update
	if ( _PrepareClientUpdateInfo() ) {
		
		// fill the RE lists to be sent to the client
		_FillPacketForClient( m_pReliableBuffer, m_pUnreliableBuffer );

		// send the stream-packet
		ssize_t sent_size = ThePacketHandler->Send_STREAM( m_nDestClientID, m_pReliableBuffer, m_pUnreliableBuffer ); 

		// check whether the packet could be sent
		if ( ( sent_size == -1 ) || ( sent_size == -2 ) ){

			//TheStatsManager->IncStats( m_nDestClientID, STAT_PACKET_OVERFLOW, 1 );

		} else {

			//MSGOUT( "SendUpdateToClient(): %d", m_nDestClientID );

			// store information about the packet we just sent for rate estimation
			sent_packet_info_s* spi = &m_SentPacketInfo[ m_nPacket % m_nNumPacketSlots ];
			spi->m_size		= sent_size;
			spi->m_sendtime	= SYSs_GetRefFrameCount();

			//NOTE: wraparound shouldnt be a problem ( ~500 days with 100 packets/sec. :) )
			m_nPacket++;
			//TheStatsManager->IncStats( m_nDestClientID, STAT_PACKET_SENT, 1 );

			rc = true;
		}

	} else {
		//DBGTXT( MSGOUT( "_PrepareClientUpdateInfo() for %d had nothing to send", m_nDestClientID ); );
	}

	// release lists
	m_pReliableBuffer->Release();
	m_pUnreliableBuffer->Release();

	m_pReliableBuffer   = NULL;
	m_pUnreliableBuffer = NULL;

	return rc;
}


// empty the list of things to update -----------------------------------------
//
void E_SimClientNetOutput::_ClearUpdates()
{
	m_nNumClients			= 0;
	m_bIncludeGameState		= SEND_MODE_NONE;
	m_bIncludeKillStats		= SEND_MODE_NONE;
    m_bIncludeStateSync     = SEND_MODE_NONE;
	m_nNumDistsForNextPacket= 0;
	m_nSizeAvail			= RE_LIST_MAXAVAIL;
}


// check whether there are any updates to send to the client ------------------
//
int E_SimClientNetOutput::_HasUpdates()
{
	return ( ( m_nNumDistsForNextPacket > 0 ) || ( m_nNumClients > 0 ) || m_bIncludeGameState || m_bIncludeKillStats );
}


// add a E_Distributable to be sent out with the next packet --------------------
//
void E_SimClientNetOutput::_AddDistForOutput( E_Distributable* pDist )
{
	ASSERT( pDist != NULL );
	ASSERT( m_nNumDistsForNextPacket < MAX_NUM_DISTRIBUTABLES_TO_SEND_PER_PACKET );

	m_DistsForNextPacket[ m_nNumDistsForNextPacket ] = pDist;
	m_nNumDistsForNextPacket++;
}


// include the information about a client in the next update ( reliable or unreliable )
//
void E_SimClientNetOutput::_FlagIncludeClient( int nClientID, bool_t bReliable )
{
	m_ClientIDList[ m_nNumClients ] = nClientID;
	m_SendReliable[ m_nNumClients ] = bReliable;
	m_nNumClients++;
}


// check whether we can reserve size for output, and do so if space avail. ----
// 
bool_t E_SimClientNetOutput::_ReserveForOutput( int size )
{
	ASSERT( size >= 0 );

	//FIXME: check whether we really need 4 trailing bytes for sentinel RE_EMPTY
	int size_after = m_nSizeAvail - size;
	if ( size_after >= 4 ) {
		m_nSizeAvail -= size;
		return true;
	} else {
        //this may print more often than we'd like but see what it is when e_simnetout ASSERTS line 560
        return false;
	}

	return false;
}







// default ctor ---------------------------------------------------------------
//
E_SimNetOutput::E_SimNetOutput() : 
	m_SimClientNetOutput( NULL ),
	m_Distributables( NULL )
{
}


// default dtor ---------------------------------------------------------------
//
E_SimNetOutput::~E_SimNetOutput()
{
	delete []m_SimClientNetOutput;
	_DestroyDistributables();
}


// reset all data -------------------------------------------------------------
//
void E_SimNetOutput::Reset()
{
	delete []m_SimClientNetOutput;
	m_SimClientNetOutput = new E_SimClientNetOutput[ MAX_NUM_CLIENTS ];

	// delete all existing distributables
	_DestroyDistributables();

	m_Distributables = new UTL_List<E_Distributable*>;
}

// connect the player in a specific slot --------------------------------------
//
int E_SimNetOutput::ConnectPlayer( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) ); 
	m_SimClientNetOutput[ nClientID ].Connect( nClientID );

    // schedule all distributables in the world database for delivery to the newly connected client
	for( UTL_listentry_s<E_Distributable*>* entry = m_Distributables->GetHead(); entry != NULL; entry = entry->m_pNext ) {
		E_Distributable* pDist = entry->m_data;

		// invalidate any previous sent update, as we are newly connected
		pDist->MarkForUpdate( nClientID );

		m_SimClientNetOutput[ nClientID ].ScheduleDistributable( pDist );
	}

    return TRUE;
}

// disconnect the player in a specific slot -----------------------------------
//
int E_SimNetOutput::DisconnectPlayer( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) ); 
	m_SimClientNetOutput[ nClientID ].Disconnect();
	return TRUE;
}


// update all clients ---------------------------------------------------------
//
void E_SimNetOutput::DoClientUpdates()
{
	// append a remote event for each connected player
	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
		if( !TheSimulator->IsPlayerDisconnected( nClientID ) ) {
			m_SimClientNetOutput[ nClientID ].SendUpdateToClient();
		}
	}
}


// update NET.SERVERRATE dependend stuff of E_SimClientNetOutput ---------------
//
void E_SimNetOutput::RateChangedForClient( int nClientID )
{	
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	m_SimClientNetOutput[ nClientID ].RateChanged();
}


// recalulate the average packet sizes for all connected clients --------------
//
void E_SimNetOutput::RecalcAveragePacketSizes()
{
	// for each connected player recalculate the average packetsize
	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
		if( !TheSimulator->IsPlayerDisconnected( nClientID ) ) {
			m_SimClientNetOutput[ nClientID ].CalculateAveragePacketSize();
		}
	}
}


// buffer the RE for sending to all clients except sender --------------------------------------
//
void E_SimNetOutput::BufferForMulticastRE( RE_Header* relist, int nSenderClientID, int reliable )
{
	ASSERT( relist != NULL );
	ASSERT( (( nSenderClientID >= 0 ) && ( nSenderClientID < MAX_NUM_CLIENTS )) || nSenderClientID == PLAYERID_ANONYMOUS );
	//ASSERT( !TheSimulator->IsPlayerDisconnected( nSenderClientID ) || nSenderClientID == PLAYERID_ANONYMOUS );

	// buffer RE to be distributed to all other clients
	for( int nDistClient = 0; nDistClient < MAX_NUM_CLIENTS; nDistClient++ ) {
		if( !TheSimulator->IsPlayerDisconnected( nDistClient ) && ( nDistClient != nSenderClientID ) ) {

			DBGTXT( MSGOUT( "distributing RE type %d from %d to %d", relist->RE_Type, nSenderClientID, nDistClient ); );

			m_SimClientNetOutput[ nDistClient ].BufferRE( relist, reliable );
		}
	}
}

//Send specific RE to specific client, planned for use with state sync on newly connected clients
void E_SimNetOutput::BufferForDirectRE( RE_Header* relist, int nClientID, int reliable )
{
	ASSERT( relist != NULL );
	ASSERT(( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ));
	ASSERT( !TheSimulator->IsPlayerDisconnected( nClientID ) || nClientID == PLAYERID_ANONYMOUS );
    
	// buffer RE to be distributed to specific clients
	if( !TheSimulator->IsPlayerDisconnected( nClientID )) {
            
			DBGTXT( MSGOUT( "distributing RE type %d to %d", relist->RE_Type, nClientID); );
            
			m_SimClientNetOutput[ nClientID ].BufferRE( relist, reliable );
		
	}
}

//
// reset the update info ------------------------------------------------------
//
void E_Distributable::ResetUpdateInfo() 
{
	if ( m_update_to_client != NULL )
		delete []m_update_to_client;

	m_update_to_client = new byte[ MAX_NUM_CLIENTS ];
	UpdateMode_STATE();
}


// set updatemode for all clients to STATE ------------------------------------
//
void E_Distributable::UpdateMode_STATE()
{
	m_update_mode &= ~UPDATEMODE_SENDWHAT_MASK;
	m_update_mode |=  UPDATEMODE_SENDWHAT_STATE;
	memset( m_update_to_client, UPDATE_TODO_SEND, MAX_NUM_CLIENTS * sizeof( byte ) );
}


// set updatemode for all clients to REMOVE -----------------------------------
//
void E_Distributable::UpdateMode_REMOVE()
{
	m_update_mode &= ~UPDATEMODE_SENDWHAT_MASK;
	m_update_mode |=  UPDATEMODE_SENDWHAT_REMOVE;
	memset( m_update_to_client, UPDATE_TODO_SEND, MAX_NUM_CLIENTS * sizeof( byte ) );
}


// check whether the E_Distributable is a ZOMBIE ( client has received the REMOVE update )
//
int E_Distributable::IsZombie( int nClientID )
{
	ASSERT( ( nClientID >=0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	if ( m_update_mode & UPDATEMODE_SENDWHAT_REMOVE ) {

		// owners always have ZOMBIE state, as they never get the removal sent ( unless flag is set )
		if ( GetOwnerFromHostOjbNumber( m_objectid ) == (dword)nClientID ) {
			if ( WillBeSentToOwner() == FALSE ) {
				return TRUE;
			}
		}

		return ( m_update_to_client[ nClientID ] == UPDATE_TODO_NOTHING );
	} else {
		return FALSE;
	}
}

// check whethe the E_Distributable has an update for a specific client ---------
//
int E_Distributable::HasUpdate( int nClientID )
{
	ASSERT( ( nClientID >=0 ) && ( nClientID < MAX_NUM_CLIENTS ) );

	// never send an update to the owner of the object, unless the flag is set accordingly
	if ( GetOwnerFromHostOjbNumber( m_objectid ) == (dword)nClientID ) {
		if ( WillBeSentToOwner() == FALSE ) {
			return FALSE;
		}
	}

	return ( m_update_to_client[ nClientID ] == UPDATE_TODO_SEND );
}


// mark, that we sent an update to a client -----------------------------------
//
void E_Distributable::MarkUpdateSent( int nClientID )
{
	ASSERT( ( nClientID >=0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	m_update_to_client[ nClientID ] = UPDATE_TODO_NOTHING;
}


// mark, that we need to send an update to a client ---------------------------
//
void E_Distributable::MarkForUpdate( int nClientID )
{
	ASSERT( ( nClientID >=0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	m_update_to_client[ nClientID ] = UPDATE_TODO_SEND;
}


// determine the size this E_Distributable is in the packet ---------------------
//
size_t E_Distributable::DetermineSizeInPacket()
{
	if ( IsInRemoving() == FALSE ) {
		switch( m_listno ) {
			case SHIP_LIST:
				return E_REList::RmEvGetSizeFromType( RE_PLAYERANDSHIPSTATUS );
			case LASER_LIST:
				return E_REList::RmEvGetSizeFromType( RE_CREATELASER );
			case MISSL_LIST:
				return E_REList::RmEvGetSizeFromType( RE_CREATEMISSILE );
			case EXTRA_LIST:
				return E_REList::RmEvGetSizeFromType( RE_CREATEEXTRA2 );
				break;
			case CUSTM_LIST:
				{
					GenObject* pObject = TheWorld->FetchSpecificObject( m_objectid, TheWorld->m_CustmObjects );
					ASSERT( OBJECT_TYPE_CUSTOM( pObject ) );

					// call persistence function with NULL destination pointer to get the size 
					// of the information to be stored
					CustomObject *custompo = (CustomObject *) pObject;
					if ( custompo->callback_persist != NULL ) {
						return (*custompo->callback_persist)( custompo, TRUE, NULL );
					}
				}
				break;
			default:
				ASSERT(FALSE);
		}

	} else {
		return E_REList::RmEvGetSizeFromType( RE_KILLOBJECT );
	}

	ASSERT( FALSE );
	return 0;
}


// write a corresponding RE to the REList -------------------------------------
//
void E_Distributable::DistributeToREList( E_REList* relist )
{
	// send creation RE
	if ( IsInRemoving() == FALSE ) {

		GenObject* pObject = NULL;
		switch( m_listno ) {
			case SHIP_LIST:
				//FIXME: implement
				//pObject = TheWorld->FetchSpecificObject( objectid, TheWorld->m_PShipObjects );
				break;
			case LASER_LIST:
				pObject = TheWorld->FetchSpecificObject( m_objectid, TheWorld->m_LaserObjects );
				relist->NET_Append_RE_CreateLaser( (LaserObject*) pObject );
				break;
			case MISSL_LIST:
				//FIXME: implement
				pObject = TheWorld->FetchSpecificObject( m_objectid, TheWorld->m_MisslObjects );
				//ASSERT(FALSE);
                relist->NET_Append_RE_CreateMissile( (MissileObject*) pObject, ((TargetMissileObject *)pObject)->TargetObjNumber); //CrazySpence trying to fix Missiles
				break;
			case EXTRA_LIST:
				pObject = TheWorld->FetchSpecificObject( m_objectid, TheWorld->m_ExtraObjects );
				relist->NET_Append_RE_CreateExtra2( (ExtraObject*) pObject );
				break;
			case CUSTM_LIST:
				{
					pObject = TheWorld->FetchSpecificObject( m_objectid, TheWorld->m_CustmObjects );
					ASSERT( OBJECT_TYPE_CUSTOM( pObject ) );

					// call persistence function
					CustomObject *custompo = (CustomObject *) pObject;
					if ( custompo->callback_persist != NULL ) {
						(*custompo->callback_persist)( custompo, TRUE, relist );
					}
				}
				break;
			default:
				ASSERT(FALSE);
		}

	} else {
		// send deletion RE
		relist->NET_Append_RE_KillObject( m_objectid, m_listno );
	}
}


// create a new E_Distributable for all connected clients -----------------------
//
E_Distributable* E_SimNetOutput::CreateDistributable( GenObject* objectpo, int reliable /*= FALSE*/, int send_to_owner)
{
	ASSERT( objectpo != NULL );

	int listno   = -1;

	//FIXME: this must be done by a method
	// determine list #
	switch ( objectpo->ObjectType & TYPELISTMASK ) {
		case PSHIP_LIST_NO:
			listno = SHIP_LIST;  break;
		case LASER_LIST_NO:
			listno = LASER_LIST; break;
		case MISSL_LIST_NO:
			listno = MISSL_LIST; break;
		case EXTRA_LIST_NO:
			listno = EXTRA_LIST; break;
		case CUSTM_LIST_NO:
			listno = CUSTM_LIST; break;
		default:
			ASSERT( FALSE );
			return NULL;
	}

	ASSERT( ( listno >= SHIP_LIST ) && ( listno <= CUSTM_LIST ) );
	DBGTXT(MSGOUT( "DIST: E_Simulator::CreateDistributable():                     objectid %x listno %d reliable %d", objectpo->HostObjNumber, listno, reliable ););
	// create a new E_Distributable ( implicitely calls UpdateMode_STATE() )
	E_Distributable* pDist = new E_Distributable( objectpo->HostObjNumber, listno, reliable, send_to_owner );
	m_Distributables->AppendHead( pDist );

	// schedule the E_Distributable to all connected clients
	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
		if( TheSimulator->IsPlayerDisconnected( nClientID ) == FALSE ) {
			m_SimClientNetOutput[ nClientID ].ScheduleDistributable( pDist );
		}
	}

	return pDist;
}

// release a E_Distributable ----------------------------------------------------
//
void E_SimNetOutput::ReleaseDistributable( E_Distributable* pDist )
{
	ASSERT( pDist != NULL );

	// find entry in list and set the correct mode
	UTL_listentry_s<E_Distributable*>* entry = m_Distributables->Find( pDist );
 	if ( entry == NULL ) {
		ASSERT( FALSE );
		return;
	}

	ASSERT( pDist == entry->m_data);

	DBGTXT( MSGOUT( "DIST: E_Simulator::ReleaseDistributable():                    objectid %x listno %d reliable %d", pDist->GetObjectID(), pDist->GetListNo(), pDist->NeedsReliable() ); );

	pDist->UpdateMode_REMOVE();

	// schedule the E_Distributable to all connected clients
	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
		if( TheSimulator->IsPlayerDisconnected( nClientID ) == FALSE ) {
			m_SimClientNetOutput[ nClientID ].ScheduleDistributable( pDist, TRUE );
		}
	}
}

// cleanup all distributables, that are complete zombies ----------------------
//
void E_SimNetOutput::CleanupZombieDistributables()
{
	// cache connected information
	//FIXME: should become a member function of the connection manager to 
	//       retrieve a int array indicating whether a client is connected
	int* connected = new int[ MAX_NUM_CLIENTS ];
	memset( connected, 0, MAX_NUM_CLIENTS * sizeof( int ) );
	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
		connected[ nClientID ] = ( TheSimulator->IsPlayerDisconnected( nClientID ) == FALSE );
	}

	// check all distributables, that are in REMOVE mode, whether the mode for all
	// connected clients is already ZOMBIE. delete from list, if so
	for( UTL_listentry_s<E_Distributable*>* entry = m_Distributables->GetHead(); entry != NULL; ) {

		E_Distributable*						pDist		= entry->m_data;
		UTL_listentry_s<E_Distributable*>*	nextentry	= entry->m_pNext;

		if ( pDist->IsInRemoving() ) {

			// detect whether in ZOMBIE state for all clients
			int allzombies = TRUE;
			for( int nClientID = 0; ( nClientID < MAX_NUM_CLIENTS ) && allzombies; nClientID++ ) {
				if ( connected[ nClientID ] && ( pDist->IsZombie( nClientID ) == FALSE ) ) {
					allzombies = FALSE;
				}
			}

			// remove if in ZOMBIE state for all clients
			if ( allzombies ) {

				DBGTXT( MSGOUT( "DIST: E_Simulator::CleanupZombieDistributables():             objectid %x listno %d reliable %d", pDist->GetObjectID(), pDist->GetListNo(), pDist->NeedsReliable() ); );

				m_Distributables->RemoveEntry( entry );
				delete pDist;
			}
		} 
		entry = nextentry;
	}

	delete []connected;
}


// destroy m_Distributables ---------------------------------------------------
//
void E_SimNetOutput::_DestroyDistributables()
{
	if ( m_Distributables == NULL )
		return;

	// delete all remaining distributables
	for( UTL_listentry_s<E_Distributable*>* entry = m_Distributables->GetHead(); entry != NULL; entry = entry->m_pNext ) {
		delete entry->m_data;
	}
	delete m_Distributables;
	m_Distributables = NULL;
}




