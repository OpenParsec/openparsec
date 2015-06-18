/*
 * PARSEC - stream class - shared
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:45 $
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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

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
#include "sys_defs.h"

// network code config
#include "net_conf.h"

// local module header
#include "net_stream.h"

// proprietary module headers
#ifdef PARSEC_SERVER
	#include "con_aux_sv.h"
#else
	#include "con_aux.h"
#endif 
#include "e_relist.h"


// default retransmit timeout is 200ms ----------------------------------------
//
#define DEFAULT_RETRANSMIT_TIMEOUT	FRAME_MEASURE_TIMEBASE / ( 1000 / 200 )


// flags ----------------------------------------------------------------------
//
#define _FORCED_RELIABLE_DROPPING 
//#define _IDEMPOTENCY_TESTING			// enable this to ALWAYS resent a packet ONCE



// reset the FIFO entry -------------------------------------------------------
//
void StreamFIFOEntry_s::Reset()
{
	// decrease reference counter
	if ( m_pREList != NULL ) {
		m_pREList->Release();
	}

	m_pREList = NULL;
	m_Timeout = -1;
	m_nRetransmitCount = 0;
}


// init the FIFO entry --------------------------------------------------------
//
void StreamFIFOEntry_s::InitEntry( E_REList* pREList )
{
	m_pREList			= pREList; 
	m_Timeout			= -1;
	m_nRetransmitCount	= 0;

	// increase reference counter
	m_pREList->AddRef();
}




// default ctor ---------------------------------------------------------------
//
NET_Stream::NET_Stream()
{
	Reset();
	m_ReliableRetransmit_Frames = DEFAULT_RETRANSMIT_TIMEOUT;
}

// reset the stream to defaults -----------------------------------------------
//
void NET_Stream::Reset()
{
	//MSGOUT("NET_Stream::Reset()");

	// NOTE: 
	// no message with # ( normal/reliable ) of 0 is sent over the wire
	//
	Out_MessageId				= 1;		// first message sent is 1
	Out_ReliableMessageId		= 1;

	I_ACK_MessageId				= 0;
	I_ACK_ReliableMessageId		= 0;

	YOU_ACK_MessageId			= 0;
	YOU_ACK_ReliableMessageId	= 0;

	for( int hid = 0; hid < MSGID_HISTORY_SIZE; hid++ ) {
		message_id_history[ hid ] = 0;
	}

	FlushReliableBuffer();

	// default to support reliable transfer
	m_EnableReliable = true;

	// set ids of the partners for this stream
	m_nPeerID	= PLAYERID_ANONYMOUS;
	m_nSenderID	= PLAYERID_INVALID;

	m_bIsConnected = false;
}


// set the stream to connected mode ( accepts non datagrams ) -----------------
void NET_Stream::SetConnected()
{
	m_bIsConnected = true;
}


// append a RE list to the FIFO  ----------------------------------------------
//
int NET_Stream::AppendToReliableFIFO( E_REList* pREList )
{
	ASSERT( pREList != NULL );

	//MSGOUT( "AppendToReliableFIFO(): " );
	//pREList->Dump();
	if ( pREList->GetSize() == 0 ) {
		ASSERT( FALSE );
		return FALSE;
	}

	if ( !m_EnableReliable ) { 
		ASSERT( FALSE );
		return FALSE;
	}

	// check whether the new append would cause the FIFO to overflow
	int nNextWritePos = _FIFO_GetNextWritePos();
	StreamFIFOEntry_s* pEntry = &m_FIFOEntries[ nNextWritePos ];
	if ( pEntry->m_pREList != NULL ) {
		DBGTXT( 
			MSGOUT("NET_Stream::AppendToReliableFIFO(): FIFO overflow." );
			for( int nEntry = 0; nEntry < MAX_NUM_RELIABLE_BACKLOG; nEntry++ ) {
				MSGOUT( "================================\n" );
				MSGOUT( "FIFO: %d\n", nEntry );
				m_FIFOEntries[ nEntry ].m_pREList->Dump();
				MSGOUT( "================================\n" );
			}
		 );

		return FALSE;
	}

	// otherwise append the RE list to the end of the FIFO 
	pEntry->InitEntry( pREList );

	m_nFIFO_WritePos = nNextWritePos;

	return TRUE;
}


// update the timeout for a specific FIFO slot --------------------------------
//
void NET_Stream::_FIFO_UpdateTimeOut( StreamFIFOEntry_s* pEntry )
{
	ASSERT( pEntry != NULL );
	pEntry->m_Timeout = ( SYSs_GetRefFrameCount() + m_ReliableRetransmit_Frames );
}


// reset all reliable handling ------------------------------------------------
//
void NET_Stream::FlushReliableBuffer()
{
	m_MessageId_ReliableWasSent = 0;

	// reset FIFO
	m_nFIFO_WritePos	= -1;
	m_nFIFO_ReadPos		= 0;
	for( int nEntry = 0; nEntry < MAX_NUM_RELIABLE_BACKLOG; nEntry++ ) {
		m_FIFOEntries[ nEntry ].Reset();
	}
}


// retrieve the next reliable RE list from FIFO to send -----------------------
//
E_REList* NET_Stream::GetNextReliableToSend()
{
	// sanety checks
	if( YOU_ACK_MessageId >= Out_MessageId )
        return NULL;
    if( YOU_ACK_ReliableMessageId >= Out_ReliableMessageId )
        return NULL;

	StreamFIFOEntry_s*	pEntry  = &m_FIFOEntries[ m_nFIFO_ReadPos ];
	E_REList*			pREList = pEntry->m_pREList;

	// nothing in FIFO
	if ( pREList == NULL ) {
		//DBGOUT( "NET_Stream::GetNextReliableToSend(): %d nothing in FIFO", m_nPeerID );
		return NULL;
	}

	// check for first transmit of top entry in FIFO
	if ( pEntry->m_Timeout == -1 ) {
		
		DBGTXT(DBGOUT( "NET_Stream::GetNextReliableToSend(): %d first transmit FIFO entry: %d", m_nPeerID, m_nFIFO_ReadPos  ););

		// update timeout value & first-transmit 
		_FIFO_UpdateTimeOut( pEntry );
		return pREList;
	}
	
	// if last sent reliable is not yet ACK 
	// retransmit if timeout, do nothing otherwise
	if ( YOU_ACK_MessageId < m_MessageId_ReliableWasSent ) {

		ASSERT( pEntry->m_Timeout != -1 );

		// timer expired ?
		if ( pEntry->m_Timeout < SYSs_GetRefFrameCount() ) {

			DBGTXT(DBGOUT( "NET_Stream::GetNextReliableToSend(): %d timer expired - retransmit #%d - FIFO entry: %d", m_nPeerID, pEntry->m_nRetransmitCount + 1, m_nFIFO_ReadPos ););

			// update timeout value & return for retransmit
			pEntry->m_nRetransmitCount++;
			_FIFO_UpdateTimeOut( pEntry );
			return pREList;
		}

		DBGTXT(DBGOUT( "NET_Stream::GetNextReliableToSend(): %d entry %d not yet ACK", m_nPeerID, m_nFIFO_ReadPos  ););

		return NULL;
	}

	ASSERT( YOU_ACK_MessageId >= m_MessageId_ReliableWasSent );

	// if last sent reliable is ACK 
	// remove top Pos from FIFO AND transmit next reliable in FIFO
	if ( YOU_ACK_ReliableMessageId == ( Out_ReliableMessageId - 1 ) ) {

		// remove message if still in backbuffer
		if ( pREList != NULL ) {
#ifdef _IDEMPOTENCY_TESTING
			if ( pEntry->m_nRetransmitCount == 0 ) {
				pEntry->m_nRetransmitCount++;
				DBGOUT( "NET_Stream::GetNextReliableToSend(): %d IDEMPOTENCY TESTING entry %d", m_nPeerID, m_nFIFO_ReadPos  );

				// update timeout value & return for retransmit
				_FIFO_UpdateTimeOut( pEntry );
				return pREList;

			} else {
				DBGOUT( "NET_Stream::GetNextReliableToSend(): %d removing from FIFO due to ACK : %d", m_nPeerID, m_nFIFO_ReadPos  );
				pEntry->Reset();
			}
#else
			DBGTXT(DBGOUT( "NET_Stream::GetNextReliableToSend(): %d removing from FIFO due to ACK : %d", m_nPeerID, m_nFIFO_ReadPos  ););
			pEntry->Reset();
#endif // _IDEMPOTENCY_TESTING
		}

		// step to next entry
		m_nFIFO_ReadPos = _FIFO_GetNextReadPos();
		pEntry  = &m_FIFOEntries[ m_nFIFO_ReadPos ];
		pREList = pEntry->m_pREList;

		if ( pREList != NULL ) {

			ASSERT( pEntry->m_Timeout == -1 );
			DBGTXT(DBGOUT( "NET_Stream::GetNextReliableToSend(): %d first transmit FIFO entry: %d", m_nPeerID, m_nFIFO_ReadPos  ););

			// update timeout value 
			_FIFO_UpdateTimeOut( pEntry );

			return pREList;
		} else {

			//DBGOUT( "NET_Stream::GetNextReliableToSend(): %d nothing in FIFO", m_nPeerID );			
			return NULL;
		}
	
	} else {

		// if last sent reliable is NACK 
		// retransmit top of FIFO
		ASSERT( YOU_ACK_ReliableMessageId < ( Out_ReliableMessageId - 1 ) );

		DBGTXT(DBGOUT( "NET_Stream::GetNextReliableToSend(): %d NACK detected - retransmit - FIFO entry: %d", m_nPeerID, m_nFIFO_ReadPos  ););

		// update timeout value & retransmit
		pEntry->m_nRetransmitCount++;
		_FIFO_UpdateTimeOut( pEntry );
		return pREList;
	}
}


// check input packet for rejection and maintain ACKs correctly -----------
//
int NET_Stream::InPacket( NetPacket_GMSV* gamepacket_GMSV )
{
	ASSERT( gamepacket_GMSV != NULL );

	// datagrams are ignored
	if ( gamepacket_GMSV->MessageId == MSGID_DATAGRAM ) {
		return TRUE;
	}

	// nothing todo until connected
	if ( m_bIsConnected == false ) {
		DBGTXT( MSGOUT( "NET_Stream::InPacket(): ignoring non-datagram packets on disconnected stream" ); );
		return FALSE;
	}

#ifdef _FORCED_RELIABLE_DROPPING 
#ifdef PARSEC_CLIENT
	
	if ( AUX_NETCODE_FLAGS & 32 ) {
		return FALSE;
	}

	if ( ( AUX_NETCODE_FLAGS & 64 ) && ( gamepacket_GMSV->ReliableMessageId != NO_RELIABLE ) ) {
		static int numreliable = -1;

		numreliable++;

		// only accept every 5th reliable packet
		if ( ( numreliable % 5 ) != 4 ) {
			return FALSE;
		}
	}

#endif // PARSEC_CLIENT
#endif // _FORCED_RELIABLE_DROPPING

	// debug output 
#ifdef PARSEC_DEBUG
	if ( AUX_DEBUG_NETSTREAM_DUMP & 1 ) {
		LOGOUT(( "-------------------------------------------------------------------------------" ));
		LOGOUT(( "(%2d), InPacket from %2d, MessageId %5d, ReliableMessageId %5d, AckMessageId %5d, AckReliableMessageId %5d", 
			m_nSenderID,
			m_nPeerID,
			gamepacket_GMSV->MessageId,
			gamepacket_GMSV->ReliableMessageId,
			gamepacket_GMSV->AckMessageId,
			gamepacket_GMSV->AckReliableMessageId
		));

		E_REList* relist = E_REList::CreateAndAddRef( RE_LIST_MAXAVAIL );
		relist->AppendList( (RE_Header*)&gamepacket_GMSV->RE_List );
		relist->Dump();
		relist->Release();

		LOGOUT(( "-------------------------------------------------------------------------------" ));
	}
#endif // INTERNAL_VERSION

	// do not process older messages ( I already sent an ACK )
	//FIXME: what do we do with reliable remote events here ? -> SOLUTION: we remove them as well, as they must already have been retransmitted in later packets
	if ( I_ACK_MessageId >= gamepacket_GMSV->MessageId ) {
#ifdef PARSEC_DEBUG
		MSGOUT( "NET_Stream::InPacket(): ignoring msgid %d ( already got %d )", gamepacket_GMSV->MessageId, I_ACK_MessageId );
#endif
		return FALSE;
	}

	// ACK that we received this message
	I_ACK_MessageId = max( I_ACK_MessageId,	gamepacket_GMSV->MessageId );

	// get the ACK from partner
	YOU_ACK_MessageId = max( YOU_ACK_MessageId,	gamepacket_GMSV->AckMessageId );

	// if we receive a ACK for a message we didnt yet send, we discard the packet
	if ( YOU_ACK_MessageId >= Out_MessageId ) {
		//ASSERT( FALSE );
		DBGTXT( MSGOUT( "NET_Stream::InPacket(): receiving ACK for unsent packet %d ( next outgoing is %d )", YOU_ACK_MessageId, Out_MessageId - 1 ); );
		return FALSE;
	}

	// handle reliable versions ?
	if ( m_EnableReliable ) {

		if ( gamepacket_GMSV->ReliableMessageId != NO_RELIABLE ) {
			// ACK that we received this message
			I_ACK_ReliableMessageId	= max( I_ACK_ReliableMessageId,	gamepacket_GMSV->ReliableMessageId );
		}
		// get the ACK from partner
		YOU_ACK_ReliableMessageId = max( YOU_ACK_ReliableMessageId, gamepacket_GMSV->AckReliableMessageId	);

		// be sure the partner doesnt spoof us ( ACK for msg that was not yet sent ! )
		//ASSERT( YOU_ACK_ReliableMessageId < Out_ReliableMessageId );
		YOU_ACK_ReliableMessageId = min( YOU_ACK_ReliableMessageId, Out_ReliableMessageId	);

	} else {

		// check whether we got a reliable message 
		if ( gamepacket_GMSV->ReliableMessageId != NO_RELIABLE ) {
			ASSERT( FALSE );
#ifdef PARSEC_DEBUG
			MSGOUT( "NET_Stream::InPacket(): received reliable message on UNRELIABLE stream !" );
#endif
			return FALSE;
		}
	}

	return TRUE;
}


// fill in fields in outgoing packet --------------------------------------
//
void NET_Stream::OutPacket( NetPacket_GMSV* gamepacket_GMSV, int reliable /*= FALSE*/  )
{
	ASSERT( gamepacket_GMSV != NULL );

	gamepacket_GMSV->MessageId				= Out_MessageId;
	gamepacket_GMSV->ReliableMessageId		= NO_RELIABLE;
	gamepacket_GMSV->AckMessageId			= I_ACK_MessageId;
	gamepacket_GMSV->AckReliableMessageId   = NO_RELIABLE;

	// handle reliable message #
	if( reliable ) {
	
		// check whether reliable transfer is disabled
		if ( !m_EnableReliable ) {
			ASSERT( FALSE );
			MSGOUT( "NET_Stream::OutPacket(): trying to send reliable message on UNRELIABLE stream !" );
		} else {
			gamepacket_GMSV->ReliableMessageId = Out_ReliableMessageId;

			// remember the message id that carried the reliable
			m_MessageId_ReliableWasSent = Out_MessageId;

			// increase reliable message counter ( wrap-around )
			Out_ReliableMessageId	= ( Out_ReliableMessageId == UINT_MAX ) ? 1 : Out_ReliableMessageId + 1;
		}
	}

	// ACK reliable
	if ( m_EnableReliable ) {
		gamepacket_GMSV->AckReliableMessageId = I_ACK_ReliableMessageId;
	}

	// increase message counter ( wrap-around, check for MSGID_DATAGRAM )
	Out_MessageId = ( Out_MessageId == ( MSGID_DATAGRAM - 1 ) ) ? 1 : Out_MessageId + 1;

#ifdef PARSEC_DEBUG
	// debug output 
	if ( AUX_DEBUG_NETSTREAM_DUMP & 2 ) {

		LOGOUT(( "-------------------------------------------------------------------------------" ));
		LOGOUT(( "(%2d), OutPacket to %2d, MessageId %5d, ReliableMessageId %5d, AckMessageId %5d, AckReliableMessageId %5d", 
			m_nSenderID,
			m_nPeerID,
			gamepacket_GMSV->MessageId,
			gamepacket_GMSV->ReliableMessageId,
			gamepacket_GMSV->AckMessageId,
			gamepacket_GMSV->AckReliableMessageId
		));

		E_REList* relist = E_REList::CreateAndAddRef( RE_LIST_MAXAVAIL );
		relist->AppendList( (RE_Header*)&gamepacket_GMSV->RE_List );
		relist->Dump();
		relist->Release();
		LOGOUT(( "------------------------------------------" ));
	}
#endif // INTERNAL_VERSION
}


// check whether to filter out a duplicate packet -----------------------------
//
int NET_Stream::FilterPacketDuplicate( int messageid )
{
	// init message id for last packet with sane value
	// if this is the first packet from this player
	if ( I_ACK_MessageId == 0 ) {
		//FIXME: cbx - 2002/02/28 - we must init message_id_history to 0 if 
		//       this is the FIRST packet not the SECOND. This caused a very
		//       subtle bug, preventing recorded demos from playing a second time
		//       in one session, mainly because PKTP_CONNECT packets are droppped 
		//       as they have a SendPlayerId of 0, which did not have a 
		//       cleared message_id_history at the first packet !
		if ( messageid >= 1 ) {
			int previd = messageid - 1;
			I_ACK_MessageId = previd;
			for ( int hid = 0; hid < MSGID_HISTORY_SIZE; hid++ ) {
				message_id_history[ hid ] = previd;
			}
		}
	}
	
	// discard duplicate packets remembered in history buffer
	int hid =0;
	for ( hid = MSGID_HISTORY_SIZE - 1; hid >= 0; hid-- ) {
		if ( message_id_history[ hid ] == messageid ) {
			// filter out packet
			return TRUE;
		}
	}
	
	// update history buffer
	for ( hid = 1; hid < MSGID_HISTORY_SIZE; hid++ ) {
		message_id_history[ hid - 1 ] = message_id_history[ hid ];
	}
	message_id_history[ MSGID_HISTORY_SIZE - 1 ] = messageid;
	
	// calculate packet loss using message id comparisons
	int previd  = I_ACK_MessageId;
	int numlost = messageid - ( previd + 1 );
	
	// log missing packets
	LogPacketLossStats( numlost, FALSE, TRUE );
	
	// got this packet
	LogPacketLossStats( 1, TRUE, TRUE );
	
	// take this packet
	return FALSE;
}


// log packet loss statistics -------------------------------------------------
//
void NET_Stream::LogPacketLossStats( int numpackets, int packetok, int incoming )
{
	//FIXME: STATS()

	// negative values might happen if we used
	// mismatched packet ids to calc numpackets
	if ( numpackets < 1 ) {
		return;
	}
	
	// guard against more lost packets than we can display
	if ( numpackets > PACKET_LOSS_METER_LENGTH ) {
		numpackets = PACKET_LOSS_METER_LENGTH;
	}
	
	// select receive/transmit graph
	char *graph = incoming ? packet_graph_recv : packet_graph_send;
	
	// scroll packet graph leftward
	int pos = 0;
	for ( pos = 0; pos < PACKET_LOSS_METER_LENGTH - numpackets; pos++ ) {
		graph[ pos ] = graph[ pos + numpackets ];
	}
	
	// insert new values at the right
	for ( ; pos < PACKET_LOSS_METER_LENGTH; pos++ ) {
		graph[ pos ] = ( packetok ? 0 : 1 );
	}
}

