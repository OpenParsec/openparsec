/*
 * PARSEC - Server-side Simulator 
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
#include <math.h>

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
#include "e_defs.h"

// network code config
#include "net_conf.h"

// net game header
#include "net_game_sv.h"

// mathematics header
#include "utl_math.h"

// utility headers
#include "utl_list.h"

// local module header
#include "e_simulator.h"

// proprietary module headers
#include "con_aux_sv.h"
#include "e_colldet.h"
#include "g_extra.h"
//#include "net_csdf.h"
#include "net_packetdriver.h"
#include "net_stream.h"
#include "obj_creg.h"
//#include "e_stats.h"
#include "g_main_sv.h"
#include "e_simplayerinfo.h"
#include "e_simnetinput.h"
#include "e_simnetoutput.h"
#include "e_connmanager.h"
#include "e_gameserver.h"
#include "e_packethandler.h"
#include "e_relist.h"
#include "sys_refframe_sv.h"

// flags ----------------------------------------------------------------------
//
#define SERVER_SLERP_ORIENTATION			1
#define _SLERP_DEBUGGING

#ifdef _SLERP_DEBUGGING

//FIXME: not needed anymore ?
/*#ifdef SYSTEM_WIN32_UNUSED
	#include "windows.h"
	#include "mmsystem.h"
	#pragma comment( lib, "winmm.lib" )
#endif // SYSTEM_WIN32_UNUSED
*/

#endif // _SLERP_DEBUGGING


// reset all data members -----------------------------------------------------
//
void E_SimShipState::Reset()
{
	MakeIdMatrx( m_ObjPosition );

	m_CurSpeed		= 0;
	m_CurYaw		= 0;
	m_CurPitch		= 0;
	m_CurRoll		= 0;
	m_CurSlideHorz	= 0;
	m_CurSlideVert	= 0;
}


// connect a client -----------------------------------------------------------
//
void E_SimClientState::Connect( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	ASSERT( m_States == NULL );

	m_nClientID				= nClientID;
	m_nNumStateSlots		= TheServer->GetSimFrequency();
	ASSERT( m_nNumStateSlots > 0 );
	m_States				= new E_SimShipState[ m_nNumStateSlots ];
	m_ClientMovementMode	= CMM_SIMULATING;
	m_playerlerp.transition = 0;
	m_playerlerp.curalpha   = -1.0f;
}

// check the movement bounds for a client -------------------------------------
//
int E_SimClientState::CheckMovementBounds( dword MessageID, RE_PlayerAndShipStatus* pPAS )
{
	ASSERT( pPAS != NULL );

	//TODO: check whether movement from last to current state is possbile

	return TRUE;
}


// check whether the change from the last to the current state involved a movement
//
int E_SimClientState::_CheckForMovement( dword MessageID, RE_PlayerAndShipStatus* pPAS )
{
	if ( m_InputState.m_CurSpeed		!= pPAS->CurSpeed	  ) { /*MSGOUT( "_CheckForMovement(): CurSpeed" );*/ return TRUE; }
	if ( m_InputState.m_CurYaw			!= pPAS->CurYaw		  ) { /*MSGOUT( "_CheckForMovement(): CurYaw" ); */  return TRUE; }
	if ( m_InputState.m_CurPitch		!= pPAS->CurPitch	  ) { /*MSGOUT( "_CheckForMovement(): CurPitch" ); */return TRUE; }
	if ( m_InputState.m_CurRoll			!= pPAS->CurRoll	  ) { /*MSGOUT( "_CheckForMovement(): CurRoll" ); */ return TRUE; }
	if ( m_InputState.m_CurSlideHorz	!= pPAS->CurSlideHorz ) { /*MSGOUT( "_CheckForMovement(): CurSlideHorz" ); */return TRUE; }
	if ( m_InputState.m_CurSlideVert	!= pPAS->CurSlideVert ) { /*MSGOUT( "_CheckForMovement(): CurSlideVert" ); */return TRUE; }

	//geomv_t* last_pos = (geomv_t*)m_InputState.m_ObjPosition;
	//geomv_t* new_pos  = (geomv_t*)pPAS->ObjPosition;
	//for( int i = 0; i < ( 3 * 4 )/* sizeof( Xmatrx ) / sizeof( geomv_t ) */; i++ )
	//	if ( ( last_pos[ i ] - new_pos[ i ] ) > GEOMV_VANISHING ) return TRUE;

	//FIXME: enable whole matrix check again ?
	//FIXME: relaxed checking of values ( diff must be greater than certain epsilon ) 

	// check position only
	pXmatrx last_pos = m_InputState.m_ObjPosition;
	pXmatrx new_pos  = pPAS->ObjPosition;
	if ( last_pos[ 0 ][ 3 ] != new_pos[ 0 ][ 3 ] ) { /*MSGOUT( "_CheckForMovement(): pos[ 0 ][ 3 ]" ); */return TRUE; }
	if ( last_pos[ 1 ][ 3 ] != new_pos[ 1 ][ 3 ] ) { /*MSGOUT( "_CheckForMovement(): pos[ 1 ][ 3 ]" );*/ return TRUE; }
	if ( last_pos[ 2 ][ 3 ] != new_pos[ 2 ][ 3 ] ) { /*MSGOUT( "_CheckForMovement(): pos[ 2 ][ 3 ]" ); */ return TRUE; }

	return FALSE;
}

// force a new state  ---------------------------------------------------------
//
void E_SimClientState::ForceNewState( E_SimShipState* pSimShipState )
{
	ASSERT( pSimShipState != NULL );

	// NOTE: this actually is stored in the last-sim-frame as these values get transfered 
	//       to the current-sim-frame in the CalcNewState function
	E_SimShipState* pPrevState = GetPrevSimFrameStateSlot();	

	// copy new state over latest simstate and the input state(s)
	pPrevState->CopyFrom ( *pSimShipState );
	m_InputState.CopyFrom( *pSimShipState );

	// abort any smoothing 
	m_ClientMovementMode	= CMM_SIMULATING;
	m_playerlerp.transition = 0;
	m_playerlerp.curalpha   = -1.0f;
}


// update the message ids the lateste state comes from ( dead reckoning ) -----
//
inline
void E_SimClientState::_UpdateStateMessageID( dword MessageID )
{
	// store the message ID
	m_nLastInputStateMessageId	= m_nInputStateMessageID;
	m_nInputStateMessageID		= MessageID;
}

// store the received state as the newest -------------------------------------
//
void E_SimClientState::StoreNewState( dword MessageID, RE_PlayerAndShipStatus* pPAS )
{
	ASSERT( pPAS != NULL );

	// check whether the change from the last to the current state involved a movement
	bool_t bDidMove = _CheckForMovement( MessageID, pPAS );

	// set the movement parameters for the next simulation step
	// NOTE: this actually is stored in the last-sim-frame as these values get transfered 
	//       to the current-sim-frame in the CalcNewState function
	E_SimShipState* pPrevState = GetPrevSimFrameStateSlot();	
	pPrevState->m_CurSpeed		= pPAS->CurSpeed;
	pPrevState->m_CurYaw		= pPAS->CurYaw;
	pPrevState->m_CurPitch		= pPAS->CurPitch;
	pPrevState->m_CurRoll		= pPAS->CurRoll;
	pPrevState->m_CurSlideHorz	= pPAS->CurSlideHorz;
	pPrevState->m_CurSlideVert	= pPAS->CurSlideVert;

	// store the input state
	m_InputState.CopyFromPlayerAndShipStatus( *pPAS );

	// store the message ID
	_UpdateStateMessageID( MessageID );

	// calculate the newest smoothing target, if the player moved
	if ( bDidMove ) {
		_CalcNewSmoothingTarget();
	}
}


// dump some SimClientState for debugging purposes -------------------------------
//
void E_SimClientState::Dump()
{
	MSGOUT( "SimFrame: %d", TheSimulator->GetSimFrame() );
	for( int i = 0; i < m_nNumStateSlots; ) {
		MSGOUT( "state %02d: Z: %8.3f %8.3f %8.3f %8.3f %8.3f", 
			i, 
			m_States[ i     ].m_ObjPosition[ 0 ][ 3 ],
			m_States[ i + 1 ].m_ObjPosition[ 0 ][ 3 ],
			m_States[ i + 2 ].m_ObjPosition[ 0 ][ 3 ],
			m_States[ i + 3 ].m_ObjPosition[ 0 ][ 3 ],
			m_States[ i + 4 ].m_ObjPosition[ 0 ][ 3 ]
		);
		i += 5;
	}
}

// calculate the newest smooothing target -------------------------------------
//
void E_SimClientState::_CalcNewSmoothingTarget()
{
	m_playerlerp.curspeed		= m_InputState.m_CurSpeed;
	m_playerlerp.curyaw			= m_InputState.m_CurYaw;
	m_playerlerp.curpitch		= m_InputState.m_CurPitch;
	m_playerlerp.curroll		= m_InputState.m_CurRoll;
	m_playerlerp.curslidehorz	= m_InputState.m_CurSlideHorz;
	m_playerlerp.curslidevert	= m_InputState.m_CurSlideVert;

//NOTE: this is the smoothing code, that should prevent the "running ahead" of the server
//#define NEW_SMOOTHING_CODE
#ifdef NEW_SMOOTHING_CODE

	// do nothing on first input message
	if ( m_nLastInputStateMessageId == 0 ) {
		return;
	}

	// determine # of packets between updates
	int	nNumPackets = m_nInputStateMessageID - m_nLastInputStateMessageId;
	ASSERT( nNumPackets != 0 );

#define NUM_PACKETS_LOST_BEFORE_RESYNC	5
	ASSERT( nNumPackets < NUM_PACKETS_LOST_BEFORE_RESYNC );

	// determine smoothing time
	int			sendfreq		= TheConnManager->GetClientInfo( m_nClientID )->GetSendFreq();
	refframe_t	smoothing_time  = FRAME_MEASURE_TIMEBASE / sendfreq;

	refframe_t  nSimRefFrames;
	if ( ( m_ClientMovementMode == CMM_SIMULATING ) && ( m_LastSwitchToSimRefFrame != -1 ) ) {
		// account for the time we already spent simulating
		nSimRefFrames = SYSs_GetRefFrameCount() - m_LastSwitchToSimRefFrame;
	} else {
		nSimRefFrames = 0;
	}

	// calculate the time to smooth to the newly supplied position
	m_playerlerp.transition = ( smoothing_time * nNumPackets ) - nSimRefFrames;

	// no smoothing necessary ( input movement is still taken for simulating )
	if ( m_playerlerp.transition <= 0 ) {
		m_playerlerp.transition = 0;
		return;
	}

	// destination position is current input-position
	Vector3 vDestPos;
	FetchTVector( m_InputState.m_ObjPosition, &vDestPos );
	//FIXME: shouldnt we include the predicted movement from the input position 
	//       in m_playerlerp.transition refframes

#else // !NEW_SMOOTHING_CODE

#define DEFAULT_PLAYERLERP_TRANSITION_TIME	200

	//FIXME: which values are best here ? ( <= or >= client-send-frame-time ??? )
	m_playerlerp.transition = DEFAULT_PLAYERLERP_TRANSITION_TIME;


	// calculate the movement
	Vector3 vMovementOffset;
	CalcMovement( &vMovementOffset, 
				m_InputState.m_ObjPosition, 
				m_InputState.m_CurSpeed, 
				m_InputState.m_CurSlideHorz, 
				m_InputState.m_CurSlideVert,
				(refframe_t)m_playerlerp.transition);

	// destination position as contained in last state information from client
	Vector3 vDestPos;
	FetchTVector( m_InputState.m_ObjPosition, &vDestPos );

	// predict where destination position will be after transition time
	vDestPos.X += vMovementOffset.X;
	vDestPos.Y += vMovementOffset.Y;
	vDestPos.Z += vMovementOffset.Z;

#endif // !NEW_SMOOTHING_CODE

	// store corrected (predicted) destination
	m_playerlerp.dstposition[ 0 ][ 3 ] = vDestPos.X;
	m_playerlerp.dstposition[ 1 ][ 3 ] = vDestPos.Y;
	m_playerlerp.dstposition[ 2 ][ 3 ] = vDestPos.Z;

	// position in last (prior to this) sim-frame
	E_SimShipState* pPrevState = GetPrevSimFrameStateSlot();
	Vector3 vSimPos;
	FetchTVector( pPrevState->m_ObjPosition, &vSimPos );

	// determine one step on path from current to destination position
	float normfac = 1.0f / m_playerlerp.transition;

	//NOTE: the time the server needs for the transition must also include the 
	//       time-difference between the last received packet.
	//       when the difference exceeds a certain treshold, we must resync the client anyway

	m_playerlerp.transvec_x = FLOAT_TO_GEOMV( GEOMV_TO_FLOAT( vDestPos.X - vSimPos.X ) * normfac );
	m_playerlerp.transvec_y = FLOAT_TO_GEOMV( GEOMV_TO_FLOAT( vDestPos.Y - vSimPos.Y ) * normfac );
	m_playerlerp.transvec_z = FLOAT_TO_GEOMV( GEOMV_TO_FLOAT( vDestPos.Z - vSimPos.Z ) * normfac );

	DBGTXT( MSGOUT( "new PLAYERLERP: to: %f/%f/%f, from: %f/%f/%f, transvec: (%f/%f/%f)", 
					vDestPos.X, 
					vDestPos.Y, 
					vDestPos.Z, 
					vSimPos.X, 
					vSimPos.Y, 
					vSimPos.Z, 
					m_playerlerp.transvec_x, 
					m_playerlerp.transvec_y, 
					m_playerlerp.transvec_z ); );

	if ( SERVER_SLERP_ORIENTATION )	{

		// orientation in previous frame
		QuaternionFromMatrx( &m_playerlerp.srcquat, pPrevState->m_ObjPosition );
		QuaternionMakeUnit( &m_playerlerp.srcquat );

		// orientation state from packet
		QuaternionFromMatrx( &m_playerlerp.dstquat, m_InputState.m_ObjPosition );
		QuaternionMakeUnit( &m_playerlerp.dstquat );

		// stepsize currently bound to poslerp speed
		//FIXME: how did we come up with this constant ?
		//float stepsize = normfac * 1.2f;
		float stepsize = normfac;

		m_playerlerp.incalpha = stepsize;
		m_playerlerp.curalpha = 0.0f;

		// start with identity for (yaw,pitch,roll) accumulation
		m_playerlerp.dstposition[ 0 ][ 0 ] = GEOMV_1;
		m_playerlerp.dstposition[ 0 ][ 1 ] = GEOMV_0;
		m_playerlerp.dstposition[ 0 ][ 2 ] = GEOMV_0;
		m_playerlerp.dstposition[ 1 ][ 0 ] = GEOMV_0;
		m_playerlerp.dstposition[ 1 ][ 1 ] = GEOMV_1;
		m_playerlerp.dstposition[ 1 ][ 2 ] = GEOMV_0;
		m_playerlerp.dstposition[ 2 ][ 0 ] = GEOMV_0;
		m_playerlerp.dstposition[ 2 ][ 1 ] = GEOMV_0;
		m_playerlerp.dstposition[ 2 ][ 2 ] = GEOMV_1;

	} else {

		// simply assume new orientation
		pXmatrx dest = pPrevState->m_ObjPosition;
		pXmatrx src  = m_InputState.m_ObjPosition;
		dest[ 0 ][ 0 ] = src[ 0 ][ 0 ];
		dest[ 0 ][ 1 ] = src[ 0 ][ 1 ];
		dest[ 0 ][ 2 ] = src[ 0 ][ 2 ];
		dest[ 1 ][ 0 ] = src[ 1 ][ 0 ];
		dest[ 1 ][ 1 ] = src[ 1 ][ 1 ];
		dest[ 1 ][ 2 ] = src[ 1 ][ 2 ];
		dest[ 2 ][ 0 ] = src[ 2 ][ 0 ];
		dest[ 2 ][ 1 ] = src[ 2 ][ 1 ];
		dest[ 2 ][ 2 ] = src[ 2 ][ 2 ];
	}

	// set movement sate to smoothing
	m_ClientMovementMode = CMM_SMOOTHING;
}


// return the current simulation-state-slot -----------------------------------
//
E_SimShipState* E_SimClientState::GetCurSimFrameStateSlot()
{
	return &m_States[ TheSimulator->GetSimFrame() % m_nNumStateSlots ];
}

// return the last simulation-state-slot --------------------------------------
//
E_SimShipState* E_SimClientState::GetPrevSimFrameStateSlot()
{
	ASSERT( TheSimulator->GetSimFrame() >= 1 );
	return &m_States[ ( TheSimulator->GetSimFrame() - 1 ) % m_nNumStateSlots ];
}

// update the shipstate -------------------------------------------------------
//
void E_SimClientState::CalcNewState( refframe_t CurSimRefFrames )
{
	E_SimShipState* pPrevState = GetPrevSimFrameStateSlot();
	E_SimShipState* pCurState  = GetCurSimFrameStateSlot();

	// transfer all movements from last-sim-frame-slot to the current
	memcpy( pCurState, pPrevState, sizeof( E_SimShipState ) );

	if ( m_ClientMovementMode == CMM_SIMULATING ) {

		ASSERT( m_ClientMovementMode == CMM_SIMULATING );
		ASSERT( m_playerlerp.transition == 0 );

		// interpolate yaw/pitch/roll
		ObjRotY( pCurState->m_ObjPosition, pCurState->m_CurYaw   * CurSimRefFrames );
		ObjRotX( pCurState->m_ObjPosition, pCurState->m_CurPitch * CurSimRefFrames );
		ObjRotZ( pCurState->m_ObjPosition, pCurState->m_CurRoll  * CurSimRefFrames );

		// calculate the movement
		Vector3 vMovementOffset;
		CalcMovement( &vMovementOffset, 
					pCurState->m_ObjPosition, 
					pCurState->m_CurSpeed, 
					pCurState->m_CurSlideHorz, 
					pCurState->m_CurSlideVert,
					CurSimRefFrames );

		// apply movement
		pCurState->m_ObjPosition[ 0 ][ 3 ] += vMovementOffset.X;
		pCurState->m_ObjPosition[ 1 ][ 3 ] += vMovementOffset.Y;
		pCurState->m_ObjPosition[ 2 ][ 3 ] += vMovementOffset.Z;

		//LOGOUT(( "(%2d), simulating, simframe: %d, refframes: %d, curspeed: %.4f, shippos: %.2f/%.2f/%.2f", 
		/*LOGOUT(( "DUMPSTATE: client,smoothing,simframe,refframes,curspeed,shippos %d, 0, %d, %d, %.4f, %.2f,%.2f,%.2f", 
							m_nClientID,
							TheSimulator->GetSimFrame(),
							CurSimRefFrames,
							FIXED_TO_FLOAT( pCurState->m_CurSpeed ),
							pCurState->m_ObjPosition[ 0 ][ 3 ],
							pCurState->m_ObjPosition[ 1 ][ 3 ],
							pCurState->m_ObjPosition[ 2 ][ 3 ] ));
		*/
		//LOGOUT(( "%.4f", pCurState->m_ObjPosition[ 2 ][ 3 ] ));

		//MSGOUT( "SimFrame: %6d SIMULATING %d: phi: %f", TheSimulator->GetSimFrame(), m_nClientID, RAD_TO_DEG( acos( pCurState->m_ObjPosition[ 0 ][ 0 ] ) ) );
		//DBGTXT( Dump(); );

	} else {

		ASSERT( m_ClientMovementMode == CMM_SMOOTHING );
		ASSERT( m_playerlerp.transition > 0 );

		// check whether slerp in progress
		int slerpactive = SERVER_SLERP_ORIENTATION && ( m_playerlerp.curalpha < 1.0f );

		// determine matrix for (yaw,pitch,roll) accumulation
		pXmatrx activematrix = slerpactive ? m_playerlerp.dstposition : pCurState->m_ObjPosition;

		// interpolate yaw/pitch/roll
		ObjRotY( activematrix, m_playerlerp.curyaw   * CurSimRefFrames );
		ObjRotX( activematrix, m_playerlerp.curpitch * CurSimRefFrames );
		ObjRotZ( activematrix, m_playerlerp.curroll  * CurSimRefFrames );

		// slerp base orientation
		if ( slerpactive ) {

			// determine whether slerp info is valid
			if ( m_playerlerp.curalpha >= 0.0f ) {

				// advance slerp alpha
				m_playerlerp.curalpha += m_playerlerp.incalpha * CurSimRefFrames;
				if ( m_playerlerp.curalpha > 1.0f ) {
					m_playerlerp.curalpha = 1.0f;
				}

				DBGTXT( MSGOUT( "SLERP: %d, curalpha: %f", m_nClientID, m_playerlerp.curalpha ); );

				// calculate current frame of SLERP
				Xmatrx curbasematrix;
				CalcSlerpedMatrix( curbasematrix, &m_playerlerp );

				// concat base with accumulated (yaw,pitch,roll)
				MtxMtxMULt( curbasematrix, m_playerlerp.dstposition, DestXmatrx );

				// set current orientation
				pCurState->m_ObjPosition[ 0 ][ 0 ] = DestXmatrx[ 0 ][ 0 ];
				pCurState->m_ObjPosition[ 0 ][ 1 ] = DestXmatrx[ 0 ][ 1 ];
				pCurState->m_ObjPosition[ 0 ][ 2 ] = DestXmatrx[ 0 ][ 2 ];
				pCurState->m_ObjPosition[ 1 ][ 0 ] = DestXmatrx[ 1 ][ 0 ];
				pCurState->m_ObjPosition[ 1 ][ 1 ] = DestXmatrx[ 1 ][ 1 ];
				pCurState->m_ObjPosition[ 1 ][ 2 ] = DestXmatrx[ 1 ][ 2 ];
				pCurState->m_ObjPosition[ 2 ][ 0 ] = DestXmatrx[ 2 ][ 0 ];
				pCurState->m_ObjPosition[ 2 ][ 1 ] = DestXmatrx[ 2 ][ 1 ];
				pCurState->m_ObjPosition[ 2 ][ 2 ] = DestXmatrx[ 2 ][ 2 ];

				//MSGOUT( "SimFrame: %6d SMOOTHING %d: phi: %f", TheSimulator->GetSimFrame(), m_nClientID, RAD_TO_DEG( acos( pCurState->m_ObjPosition[ 0 ][ 0 ] ) ) );
			}
		}

		// account for this frames time
		m_playerlerp.transition -= CurSimRefFrames;

		// switch from smoothing to simulating ?
		if ( m_playerlerp.transition <= 0 ) {

			// after transition assume destination position
			pCurState->m_ObjPosition[ 0 ][ 3 ] = m_playerlerp.dstposition[ 0 ][ 3 ];
			pCurState->m_ObjPosition[ 1 ][ 3 ] = m_playerlerp.dstposition[ 1 ][ 3 ];
			pCurState->m_ObjPosition[ 2 ][ 3 ] = m_playerlerp.dstposition[ 2 ][ 3 ];

			// remaining frametime: advance in direction of flight
			int remainder = -m_playerlerp.transition;

			// calculate the movement
			Vector3 vMovementOffset;
			CalcMovement( &vMovementOffset, 
						pCurState->m_ObjPosition, 
						m_playerlerp.curspeed, 
						m_playerlerp.curslidehorz, 
						m_playerlerp.curslidevert, 
						remainder );

			pCurState->m_ObjPosition[ 0 ][ 3 ] += vMovementOffset.X;
			pCurState->m_ObjPosition[ 1 ][ 3 ] += vMovementOffset.Y;
			pCurState->m_ObjPosition[ 2 ][ 3 ] += vMovementOffset.Z;

			// turn transition off
			m_playerlerp.transition = 0;

			// set movement state to simulating
			m_ClientMovementMode = CMM_SIMULATING;

			// record the time we switched to simulating to account for this, when calculating the new smoothing targe
			m_LastSwitchToSimRefFrame = SYSs_GetRefFrameCount();

			//DBGTXT( MSGOUT( "remaining" ); );
			//DBGTXT( Dump(); );

		} else {

			// interpolate position transition (along transition vector)
			pCurState->m_ObjPosition[ 0 ][ 3 ] += m_playerlerp.transvec_x * CurSimRefFrames;
			pCurState->m_ObjPosition[ 1 ][ 3 ] += m_playerlerp.transvec_y * CurSimRefFrames;
			pCurState->m_ObjPosition[ 2 ][ 3 ] += m_playerlerp.transvec_z * CurSimRefFrames;

			//DBGTXT( MSGOUT( "CMM_SMOOTHING: %d", m_nClientID ); );
			//DBGTXT( Dump(); );
		}

		//LOGOUT(( "(%2d), smoothing,  simframe: %d, refframes: %d, shippos: %.2f/%.2f/%.2f", 
		/*LOGOUT(( "DUMPSTATE: client,smoothing,simframe,refframes,curspeed,shippos %d, 1, %d, %d, 0, %.2f,%.2f,%.2f", 
							m_nClientID,
							TheSimulator->GetSimFrame(),
							CurSimRefFrames,
							pCurState->m_ObjPosition[ 0 ][ 3 ],
							pCurState->m_ObjPosition[ 1 ][ 3 ],
							pCurState->m_ObjPosition[ 2 ][ 3 ] ));*/

		//LOGOUT(( "%.4f", pCurState->m_ObjPosition[ 2 ][ 3 ] ));
	}

}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

// standard ctor --------------------------------------------------------------
//
E_Simulator::E_Simulator() :
	m_SimPlayerInfos( NULL ),
	m_SimClientState( NULL )
{
	Reset();
}

// standard dtor --------------------------------------------------------------
//
E_Simulator::~E_Simulator()
{
	delete []m_SimClientState;
	delete []m_SimPlayerInfos;
}


// reset all data -------------------------------------------------------------
//
void E_Simulator::Reset()
{
	TheSimNetInput ->Reset();
	TheSimNetOutput->Reset();

	delete []m_SimPlayerInfos;
	delete []m_SimClientState;

	m_SimPlayerInfos		= new E_SimPlayerInfo	[ MAX_NUM_CLIENTS ];
	m_SimClientState		= new E_SimClientState[ MAX_NUM_CLIENTS ];

	m_CurSimRefFrames		= 0;
	m_LastSimRefFrame		= REFFRAME_INVALID;
	m_nSimFrame				= 0;
}

// connect the player in a specific slot --------------------------------------
//
int E_Simulator::ConnectPlayer( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) ); 
	m_SimPlayerInfos[ nClientID ].Connect( nClientID );
	m_SimClientState[ nClientID ].Connect( nClientID );
	
	TheSimNetOutput->ConnectPlayer( nClientID );

	DBGOUT( "E_Simulator::ConnectPlayer( %d )", nClientID );

	return TRUE;
}

// disconnect the player in a specific slot -----------------------------------
//
int E_Simulator::DisconnectPlayer( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) ); 

	m_SimPlayerInfos[ nClientID ].Disconnect();
	m_SimClientState[ nClientID ].Disconnect();
	
	TheSimNetOutput->DisconnectPlayer( nClientID );

	DBGOUT( "E_Simulator::DisconnectPlayer( %d )", nClientID );

	return TRUE;
}


// check whether a player in a slot is joined ---------------------------------
//
int E_Simulator::IsPlayerJoined( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) ); 
	return m_SimPlayerInfos[ nClientID ].IsPlayerJoined();
}


// check whether a player in a slot is connected ------------------------------
//
int E_Simulator::IsPlayerDisconnected( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) ); 
	return m_SimPlayerInfos[ nClientID ].IsPlayerDisconnected();
}


// get the PlayerInfo for a client --------------------------------------------
//
E_SimPlayerInfo* E_Simulator::GetSimPlayerInfo( int nClientID ) 
{ 
	ASSERT( nClientID >= 0 ); return &m_SimPlayerInfos[ nClientID ]; 
}


// get the SimClientState for a specific client -------------------------------
//
E_SimClientState* E_Simulator::GetSimClientState( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	return &m_SimClientState[ nClientID ];
}


// get the latest state for a client ------------------------------------------
//
E_SimShipState* E_Simulator::GetLatestSimShipState( int nClientID )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	return m_SimClientState[ nClientID ].GetPrevSimFrameStateSlot();
}


// calculate the refframes we want to apply to this simulation step -----------
//
int E_Simulator::_CalcSimRefFrames()
{
	// do not simulate on first sim frame
	if ( m_LastSimRefFrame == REFFRAME_INVALID ) {
		m_LastSimRefFrame = m_CurRefFrame;
		return FALSE;
	}
	
	// calc current sim-frametime
	m_CurSimRefFrames = m_CurRefFrame - m_LastSimRefFrame;

	// ensure we run with the desired simulation frequency
	ASSERT( m_CurSimRefFrames == TheServer->GetSimTickFrameTime() );
		
	// save last sim frame
	m_LastSimRefFrame = m_CurRefFrame;

	return TRUE;
}


// simulate ship movements ----------------------------------------------------
//
void E_Simulator::_SimulateShips()
{
	//FIXME: use the joined player list
	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
		if( IsPlayerJoined( nClientID ) ) {
			m_SimClientState[ nClientID ].CalcNewState( m_CurSimRefFrames );
		}
	}
}


// apply the current sim-state to the engine state ( E_SimShipState -> ShipObject ) 
//
void E_Simulator::_ApplySimToEngineState()
{
	//FIXME: use the joined player list
	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
		if( IsPlayerJoined( nClientID ) ) {

			ShipObject*			pShipObject = m_SimPlayerInfos[ nClientID ].GetShipObject();
			E_SimShipState*	pCurState   = m_SimClientState[ nClientID ].GetCurSimFrameStateSlot();

			// apply the state in the simulation to engine object
			memcpy( pShipObject->ObjPosition, pCurState->GetObjPosition(), sizeof( Xmatrx ) );
		}
	}
}


// apply the current engine state to the sim-state ( ShipObject -> E_SimShipState ) 
//
void E_Simulator::_ApplyEngineToSimState()
{
	//NOTE: this function must be called after properties of ShipObject have been altered
	//      that need to be transferred to the other connected clients

	//FIXME: use the joined player list
	for( int nClientID = 0; nClientID < MAX_NUM_CLIENTS; nClientID++ ) {
		if( IsPlayerJoined( nClientID ) ) {

			ShipObject*			pShipObject = m_SimPlayerInfos[ nClientID ].GetShipObject();
			E_SimShipState*	pCurState   = m_SimClientState[ nClientID ].GetCurSimFrameStateSlot();

			// get pos/orientation from engine to simulator
			//FIXME: upon collision ship/ship, we must get the newest pos/orientation into pCurState
			//memcpy( pCurState->ObjPosition, pShipObject->m_ObjPosition, sizeof( Xmatrx ) );
		}
	}
}


// main simulation method -----------------------------------------------------
//
int E_Simulator::DoSim( refframe_t CurSimRefFrame )
{
	// store the current refframe
	m_CurRefFrame = CurSimRefFrame;

	if ( !_CalcSimRefFrames() ) {
		return FALSE;
	}

	// maintain the game ( time/kill-limit )
	GAMECODE2( TheGame->MaintainGame(); );

	// maintain the weapon delays
	GAMECODE2( TheGame->MaintainWeaponDelays(); );

	// animate projectile weapons
	GAMECODE2( TheGame->OBJ_AnimateProjectiles(); );

	// animate non-projectile objects
	GAMECODE2( TheGame->OBJ_AnimateNonProjectiles(); );

	TheGame->MaintainSpecialsCounters();

	// simulate ship movements
	//FIXME: GAMECODE2 ?????? ( how do we merge G_Simluator and G_Main ? )
	_SimulateShips();

	// copy sim-states to engine-states
	_ApplySimToEngineState();

	// object collision detection
	GAMECODE2( TheGameCollDet->OBJ_CheckCollisions(); );

	// place extras around the level
	GAMECODE2( TheGameExtraManager->OBJ_DoExtraPlacement(); );

	// perform ship actions like bouncing
	//GAMECODE2( TheGame->OBJ_PerformShipActions(); );

	// copy engine-states to sim-states
	_ApplyEngineToSimState();

	// increase the simulation-frame number
	m_nSimFrame++;

	return TRUE;
}
