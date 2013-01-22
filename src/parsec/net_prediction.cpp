/*
 * PARSEC - 
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:30 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002-2003
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
#include <stdio.h>
#include <string.h>

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

// utility modules
#include "utl_list.h"

// local module header
#include "net_prediction.h"

// proprietary module headers
#include "con_aux.h"
#include "e_record.h"
#include "g_extra.h"
#include "h_supp.h"
#include "net_game.h"
#include "net_stream.h"
#include "g_sfx.h"


// special implementation for a extra collect prediction event ----------------
//
G_PredEventExtraCollect::G_PredEventExtraCollect( ExtraObject* pExtra ) :
	m_pExtra( pExtra )
{
	ASSERT( pExtra != NULL );
}

// do the extra collection ----------------------------------------------------
//
void G_PredEventExtraCollect::Do()
{
	char* pszMessage = TheGameExtraManager->CollectExtra( MyShip, m_pExtra );

	// show message saying what type of extra has been collected
	if ( m_nDoCount == 0 ) {
		if ( pszMessage != NULL ) {
//#ifdef PARSEC_DEBUG
//			WriteExtraCollectMessage( "G_PredEventExtraCollect::Do()" );
//#else // !PARSEC_DEBUG
			WriteExtraCollectMessage( pszMessage );
//#endif // !PARSEC_DEBUG
		} else {
			MSGOUT( "CheckShipExtraCollision(): invalid extra type: %0x.", m_pExtra->ObjectType );
		}
	}

#ifdef PARSEC_DEBUG
	MSGOUT( "G_PredEventExtraCollect::Do()" );
#endif // PARSEC_DEBUG
	NET_PredEvent::Do();
}


//NOTE: stores information about events that were predicted in the game-engine
//      whenever the game-engine modifies the local world, we save a prediction 
//			that includes a checkpoint ( = next outgoing message id )
//		whenever we receive a packet, we clear all predictions inserted before 
//			the ACK checkpoint and we re-apply all pending predictions not yet 
//			ACK ( checkpoint # is synonym for message id )

// standard ctor --------------------------------------------------------------
//
NET_PredictionManager::NET_PredictionManager( NET_Stream* pStream )
{
	ASSERT( pStream != NULL );

	m_predictions = new UTL_List<NET_PredEvent*>;
	m_pStream     = pStream;
}


// standard dtor --------------------------------------------------------------
//
NET_PredictionManager::~NET_PredictionManager()
{
	Reset();
	delete m_predictions;
}


// reset ----------------------------------------------------------------------
//
void NET_PredictionManager::Reset()
{
	// remove all predictions
	for( LE_PredEvt* entry = m_predictions->GetHead(); entry != NULL; entry = entry->m_pNext ) {
		delete entry->m_data;
	}
	m_predictions->Clear();
}


// clear all predictions up to the message# and reapply all predictions after the message#
//
void NET_PredictionManager::DoCheckpoint( dword MessageId )
{
	for( LE_PredEvt* pEntry = m_predictions->GetHead(); pEntry != NULL; ) {
		LE_PredEvt*		pNext = pEntry->m_pNext;
		NET_PredEvent*	pPred = pEntry->m_data;
		if ( pPred->GetMessageId() <= MessageId ) {
#ifdef PARSEC_DEBUG
			MSGOUT( "NET_PredictionManager::DoCheckpoint(): removing ACK prediction ( pred-msgid %d <= msgid %d)", pPred->GetMessageId(), MessageId );
#endif // PARSEC_DEBUG
			delete m_predictions->RemoveEntry( pEntry );
		} else {
#ifdef PARSEC_DEBUG
			int nDoCountBefore = pPred->GetDoCount();
			MSGOUT( "NET_PredictionManager::DoCheckpoint(): reapplying prediction ( pred-msgid %d > msgid %d)", pPred->GetMessageId(), MessageId );
#endif // PARSEC_DEBUG

			pPred->Do();

#ifdef PARSEC_DEBUG
			ASSERT( pPred->GetDoCount() == ( nDoCountBefore + 1 ) );
#endif // PARSEC_DEBUG
		}
		pEntry = pNext;
	}
}


// add a predictionevent ------------------------------------------------------
// ensures that MAX_NUM_PREDICTIONS is not exceeded
void NET_PredictionManager::_AddPrediction( NET_PredEvent* pEvent )
{	
	// check for prediction choke ( server does not send ACKs for checkpoints fast enough )
	if ( m_predictions->GetNumEntries() >= MAX_NUM_PREDICTIONS ) {
		MSGOUT( "NET_PredictionManager::_AddPrediction(): prediction list overflow" );
		return;
	} else {
		m_predictions->AppendTail( pEvent );
		pEvent->SetMessageId( m_pStream->GetNextOutMessageId() );
#ifdef PARSEC_DEBUG
		int nDoCountBefore = pEvent->GetDoCount();

		MSGOUT( "NET_PredictionManager::_AddPrediction() at msgid: %d", m_pStream->GetNextOutMessageId() );
		
#endif // PARSEC_DEBUG

		pEvent->Do();

#ifdef PARSEC_DEBUG
		ASSERT( pEvent->GetDoCount() == ( nDoCountBefore + 1 ) );
#endif // PARSEC_DEBUG
	}
}


// add a prediction event for collecting an extra -----------------------------
//
void NET_PredictionManager::PredictExtraCollect( ExtraObject* pExtra )
{
	ASSERT( pExtra != NULL );
	_AddPrediction( new G_PredEventExtraCollect( pExtra ) );
}

