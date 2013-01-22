/*
 * PARSEC - Server-side Simulator INPUT
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
#include "e_simnetinput.h"

// proprietary module headers
#include "con_aux_sv.h"
//#include "g_colldet.h"
//#include "g_extra.h"
//#include "net_csdf.h"
#include "net_packetdriver.h"
#include "net_stream.h"
//#include "obj_creg.h"
//#include "e_stats.h"
#include "g_main_sv.h"
#include "e_simplayerinfo.h"
#include "e_simulator.h"
#include "e_simnetoutput.h"
#include "e_connmanager.h"
#include "e_gameserver.h"
#include "e_packethandler.h"
//#include "e_relist.h"
#include "sys_refframe_sv.h"


// default ctor ---------------------------------------------------------------
//
E_SimNetInput::E_SimNetInput() : 
	m_pInputREList( NULL )
{
}


// default dtor ---------------------------------------------------------------
//
E_SimNetInput::~E_SimNetInput()
{
	if ( m_pInputREList != NULL )
		m_pInputREList->Release();
}


// reset all data -------------------------------------------------------------
//
void E_SimNetInput::Reset()
{
	if ( m_pInputREList	!= NULL )  {
		m_pInputREList->Release();
	}
	// initialize input remote event list
	m_pInputREList = E_REList::CreateAndAddRef( (size_t)RE_LIST_MAXAVAIL * ( MAX_NUM_CLIENTS + 1 ) );
}


// walk the input RE list and apply events/states to simulation ---------------
//
void E_SimNetInput::ProcessInputREList()
{
	RE_Header* relist = m_pInputREList->GetData();
       ASSERT( relist != NULL );

	int nClientID = PLAYERID_ANONYMOUS;

	// process remote event list
	while ( relist->RE_Type != RE_EMPTY ) {

		// determine size of the remote event
		size_t size = E_REList::RmEvGetSize( relist );
		
		switch( relist->RE_Type ) {
			case RE_OWNERSECTION:
				// set the sender
				nClientID = ( (RE_OwnerSection*)relist )->owner;
				break;

			case RE_CREATEEXTRA2:
            case RE_CREATEEXTRA:
                 MSGOUT("CREATEEXTRA Event recieved");
                 break;

			case RE_IPV4SERVERINFO:
			case RE_SERVERLINKINFO:
			case RE_MAPOBJECT:
			case RE_STARGATE:
				// invalid/already processed remote events MUST not be in the input remote event l
				ASSERT( FALSE );
				break;

			case RE_CLIENTINFO:
				// adjust client-info ( send/recv frequency )
				TheConnManager->NET_ExecRmEvClientInfo( nClientID, (RE_ClientInfo*)relist );
				break;

			case RE_PLAYERSTATUS:
				{
					ASSERT( nClientID != PLAYERID_ANONYMOUS );
					RE_PlayerStatus* playerstatus = (RE_PlayerStatus*)relist;

					// set the desired status (joined/unjoined)
					TheSimulator->GetSimPlayerInfo( nClientID )->SetDesiredPlayerStatus( playerstatus );
				}
				break;

			case RE_PLAYERANDSHIPSTATUS:
				{
					ASSERT( nClientID != PLAYERID_ANONYMOUS );

					RE_PlayerAndShipStatus* re_pas_status = (RE_PlayerAndShipStatus*)relist;

					//MSGOUT( "_ParseNetInput(): RE_PLAYERANDSHIPSTATUS from %d, state %d", nClientID, re_pas_status->player_status );

					// set the desired status (joined/unjoined)
					E_SimPlayerInfo* pSimPlayerInfo = TheSimulator->GetSimPlayerInfo( nClientID );
					pSimPlayerInfo->SetDesiredPlayerStatus( re_pas_status );

					// only update player state if player is joined
					if( pSimPlayerInfo->IsPlayerJoined() ) {

						E_SimClientState*		pSimClientState = TheSimulator->GetSimClientState( nClientID );
						NET_Stream*	pStream = ThePacketDriver->GetStream( nClientID );
			
						int bAcceptMovement = FALSE;

						dword dwInputMessageID = pStream->GetLastInPacket();

						// check absolute movement bounds
						if ( _CheckAbsoluteMovementBounds( re_pas_status ) ) {

							// check whether the movement from the last received packet to the current packet is possible
							if ( pSimClientState->CheckMovementBounds( dwInputMessageID, re_pas_status ) ) {

								// store the received state as the newest 
								pSimClientState->StoreNewState( dwInputMessageID, re_pas_status );
								
								bAcceptMovement = TRUE;
							}
						}

						// resync client if state has not been accepted
						if ( bAcceptMovement == FALSE ) {

							// this client's state differs too much from the one on the server -> resync with server
							pSimClientState->SetClientResync();
							MSGOUT( "SetClientResync() for client %d", nClientID );
						}
					}
				}
				break;
			case RE_SENDTEXT:

				// multicast RE to all connected clients except sender
				//FIXME: determine, whether to send RE_SENDTEXT reliable ?
				//FIXME: this appends a RE_OWNERSECTION for each RE we buffer 
				//       evt. keep track of whether we already appended a RE_OWNERSECTION
				TheSimNetOutput->BufferForMulticastRE( relist, nClientID/*, append_ownersection*/, FALSE );

				break;

			case RE_CREATELASER:
				{
					ASSERT( nClientID != PLAYERID_ANONYMOUS );
					if ( TheSimulator->IsPlayerJoined( nClientID ) ) {
						//FIXME: we must determine the exact time the client fired the laser ( relative to when the packet was sent )
						TheGameInput->ActivateGun( nClientID, 0 );
					}
				}
				break;
			case RE_CREATEMISSILE:
				{
					ASSERT( nClientID != PLAYERID_ANONYMOUS );
					if ( TheSimulator->IsPlayerJoined( nClientID ) ){
						TheGameInput->LaunchMissile( nClientID, ((RE_CreateMissile *)relist)->TargetHostObjId,
									    ((RE_CreateMissile *)relist)->ObjectClass);
					}
				}
				break;
            case RE_CREATEMINE:
                 {
                      ASSERT( nClientID != PLAYERID_ANONYMOUS );
					  if ( TheSimulator->IsPlayerJoined( nClientID ) ){
                           TheGameInput->LaunchMine( nClientID);     
                      }
                 }
                 break;
            case RE_CREATESWARM:
            {
                ASSERT( nClientID != PLAYERID_ANONYMOUS );
                if ( TheSimulator->IsPlayerJoined( nClientID ) ){
                    TheGameInput->LaunchSwarm( nClientID, ((RE_CreateSwarm *)relist)->TargetHostObjId); //Set up Sim Object
                    TheSimNetOutput->BufferForMulticastRE( relist, nClientID, FALSE ); //Notify Clients of firing
                }
            }
            case RE_WEAPONSTATE:
            {
               switch (((RE_WeaponState *)relist)->WeaponMask ) {
                    case WPMASK_CANNON_LIGHTNING:
                        if (((RE_WeaponState *) relist)->State == WPSTATE_ON )
                            TheGameInput->ActivateGun( nClientID, 2 );
                        else
                            TheGameInput->DeactivateGun( nClientID, 2 );
                        break;
                        
                    case WPMASK_CANNON_HELIX:
                        if (((RE_WeaponState *) relist)->State == WPSTATE_ON )
                            TheGameInput->ActivateGun( nClientID, 1 );
                        else
                            TheGameInput->DeactivateGun( nClientID, 1 );
                        break;
                        
                    case WPMASK_CANNON_PHOTON:
                        if (((RE_WeaponState *) relist)->State == WPSTATE_ON )
                            TheGameInput->ActivateGun( nClientID, 3 );
                        else
                            TheGameInput->DeactivateGun( nClientID, 3 );
                        break;
                
                }
                TheSimNetOutput->BufferForMulticastRE( relist, nClientID, FALSE ); //Duration weapons are not objects, just tell the client someone has fired/stopped
            }
                break;
            case RE_CREATEEMP:
            {
            	// Create the EMP object
                ASSERT( nClientID != PLAYERID_ANONYMOUS );
                if ( TheSimulator->IsPlayerJoined( nClientID ) ){
                    TheGameInput->CreateEMP( nClientID, ((RE_CreateEmp *)relist)->Upgradelevel); //Set up Sim Object
                    TheSimNetOutput->BufferForMulticastRE( relist, nClientID, FALSE ); //Notify Clients of firing
                }


            }
            break;
			default:
				// we must handle all REs in this function
				MSGOUT("UNKNOWN RE: %d received. ", relist->RE_Type);
			//	ASSERT( FALSE ); This is broken but I dont need to kill the game over it
				break;
			
		}
		
		// advance to next event in list
		ASSERT( ( relist->RE_BlockSize == RE_BLOCKSIZE_INVALID ) || ( relist->RE_BlockSize == size ) );
		relist = (RE_Header *) ( (char *) relist + size );
	}

	// clear all the input remote events
	m_pInputREList->Clear();		
}



// handle the network input from a client ( filter out invalid events, append valid ones to the input RE list ) 
//
int E_SimNetInput::HandleOneClient( int nClientID, RE_Header* relist )
{
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	ASSERT( relist != NULL );

	// do not handle empty remote event lists
	if ( relist->RE_Type == RE_EMPTY ) {
		DBGTXT( MSGOUT( "E_Simulator::HandleOneClient(): ignoring - no input" ); );
		return FALSE;
	}

	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NUM_CLIENTS ) );
	ASSERT( relist != NULL );

	int nNumEventsAppended = 0;

	// process remote event list
	while ( relist->RE_Type != RE_EMPTY ) {

		// determine size of the remote event
		size_t size = E_REList::RmEvGetSize( relist );

		// check whether RE is valid
		if ( !E_REList::ValidateRE( relist, size ) ) {
			DBGTXT( MSGOUT( "E_Simulator::_ParseNetInput(): ignoring invalid RE %d", relist->RE_Type ); );
			continue;
		}
		
		switch( relist->RE_Type ) {
			case RE_OWNERSECTION:
				// RE_OWNERSECTION packets are invalid from client
				DBGTXT( MSGOUT( "WARNING: client packet from %d contained RE_OWNERSECTION", nClientID ); );
				break;

			case RE_CREATEEXTRA:
			case RE_CREATEEXTRA2:
                {
				     MSGOUT( "WARNING: client packet from %d contained RE_CREATEEXTRA()", nClientID );				}
                break;

			case RE_COMMANDINFO:

				//FIXME: handle RE_COMMANDINFO in normal PKTP_STREAM packets ( _Handle_COMMAND() )
				ASSERT( FALSE );
				break;

			default:
				{
					// append a RE_OWNERSECTION to sender in input RE list
					if ( nNumEventsAppended == 0 ) {
						if ( !m_pInputREList->NET_Append_RE_OwnerSection( nClientID ) ) {
							ASSERT( FALSE );
							DBGTXT( MSGOUT( "E_Simulator::_ParseNetInput(): NET_Append_RE_OwnerSection failed" ); );
						}
					}

					// append the remote event to the input RE list
					if ( (size_t)m_pInputREList->AppendEvent( relist, size ) != size ) {
						ASSERT( FALSE );
						DBGTXT( MSGOUT( "E_Simulator::_ParseNetInput(): AppendEvent failed" ); );
					}

					// increase event counter
					nNumEventsAppended++;
				}
				break;
			
		}
	
		// advance to next event in list
		ASSERT( ( relist->RE_BlockSize == RE_BLOCKSIZE_INVALID ) || ( relist->RE_BlockSize == size ) );
		relist = (RE_Header *) ( (char *) relist + size );
	}

	// dump the input RE list
	UPDTXT2( m_pInputREList->Dump(); ); 

	return TRUE;
}

// check absolute movement bounds ---------------------------------------------
//
int E_SimNetInput::_CheckAbsoluteMovementBounds( RE_PlayerAndShipStatus* pPAS )
{
	ASSERT( pPAS != NULL );
	//FIXME: implement
	return TRUE;
}

