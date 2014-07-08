/*
 * PARSEC - Remote Events
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:40 $
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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"
#include "od_class.h"

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "net_defs.h"
#include "sys_defs.h"

// network code config
#include "net_conf.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "net_rmev_gmsv.h"

// proprietary module headers
#include "con_aux.h"
//#include "con_say.h"
//#include "h_supp.h"
#include "net_prediction.h"
#include "net_game.h"
#include "net_game_gmsv.h"
#include "net_rmev.h"
#include "net_serv.h"
#include "net_stream.h"
#include "net_udpdf.h"
#include "net_univ.h"
#include "obj_clas.h"
#include "obj_creg.h"
#include "obj_ctrl.h"
#include "obj_cust.h"
#include "obj_xtra.h"
//#include "g_sfx.h"
//#include "g_wfx.h"
//#include "g_emp.h"
//#include "g_swarm.h"
#include "g_stgate.h"
#include "g_telep.h"
#include "g_extra.h"
#include "g_shipobject.h"


// process entire remote event list (execute all contained events) coming from the server
//
void NET_ProcessRmEvList_GMSV( NetPacket_GMSV* gamepacket )
{
	ASSERT( gamepacket != NULL );
	
	//NOTE:
	// if new remote events are added this function
	// should be updated.
	
	RE_Header *pREList = (RE_Header *) &gamepacket->RE_List;

	// default to invalid owner
	int ownerid = PLAYERID_ANONYMOUS;
	
	// process remote event list
	while ( pREList->RE_Type != RE_EMPTY ) {

		//MSGOUT( "processing RE type %d", pREList->RE_Type );

		switch ( pREList->RE_Type ) {

			case RE_OWNERSECTION:
				// get the ownerid from RE_OWNERSECTION
				ownerid = ( (RE_OwnerSection*)pREList )->owner;
				break;
			
			case RE_DELETED:
				// ignore
				break;
				
			case RE_CREATEOBJECT:
				if ( NetConnected ) {
					NET_ExecRmEvCreateObject( pREList, ownerid );
				}
				break;
				
			case RE_CREATELASER:

				if ( NetConnected ) {
					// ownerid is taken from RE 
					NET_ExecRmEvCreateLaser( pREList, PLAYERID_ANONYMOUS );
				}
				break;
				
			case RE_CREATEMISSILE:

				if ( NetConnected ) {
					NET_ExecRmEvCreateMissile( pREList, ownerid );
				}
				break;
				
			case RE_PARTICLEOBJECT:

				if ( NetConnected ) {
					//FIXME: POBJ_ENERGYFIELD ??? CBX: ask SID/MSH why ???
					NET_ExecRmEvParticleObject( pREList, ownerid );
				}
				break;

			case RE_CREATEEXTRA:
				// not supported anymore
				ASSERT( FALSE );
				break;

			case RE_CREATEEXTRA2:
				
				if ( NetConnected ) {

					int owner = GetOwnerFromHostOjbNumber( ((RE_CreateExtra2*)pREList)->HostObjId );
					NET_ExecRmEvCreateExtra2( pREList, owner );	
				}
				break;

			case RE_KILLOBJECT:

				if ( NetConnected ) {
					NET_ExecRmEvKillObject( pREList );
				}
				break;
				
			case RE_SENDTEXT:

				if ( NetConnected ) {
					NET_ExecRmEvSendText( pREList, ownerid );
				}
				break;
				
			case RE_PLAYERNAME:

				if ( NetConnected ) {
					NET_ExecRmEvPlayerName( pREList, ownerid );
				}
				break;
				
			case RE_WEAPONSTATE:

				if ( NetConnected ) {
					NET_ExecRmEvWeaponState( pREList, ownerid );
				}
				break;
				
			case RE_STATESYNC:

				if ( NetConnected ) {
					NET_ExecRmEvStateSync( pREList, ownerid );
				}
				break;
				
			case RE_CREATESWARM:

				if ( NetConnected ) {
					NET_ExecRmEvCreateSwarm( pREList, ownerid );
				}
				break;
				
			case RE_CREATEEMP:

				if ( NetConnected ) {
					NET_ExecRmEvCreateEmp( pREList, ownerid );
				}
				break;

			case RE_PLAYERSTATUS:
				// should never be received by a client
				DBGTXT( MSGOUT( "WARNING ! received RE_PLAYERSTATUS from server." ); );
				ASSERT( FALSE );
				break;

			case RE_PLAYERANDSHIPSTATUS:

				if ( NetConnected ) {
					RE_PlayerAndShipStatus* pas_status = (RE_PlayerAndShipStatus*)pREList;

					// set the desired playerstate ( PLAYER_INACTIVE, PLAYER_CONNECTED, PLAYER_JOINED ) for remote clients
					if ( pas_status->senderid != LocalPlayerId ) {
						NET_SetDesiredPlayerStatus( pas_status );
					}

					// set the shipstate according to the remote event ( also mainains playerstate for local player )
					NET_ExecRmEvPlayerAndShipStatus( pas_status, gamepacket->MessageId );
				}
				break;
			case RE_KILLSTATS:

				if ( NetConnected ) {
					// update the killstats according to the ones on the server
					NET_ExecRmEvKillStats( (RE_KillStats*)pREList );
				}

				break;

			case RE_GAMESTATE:

				if ( NetConnected ) {
					// execute RE_GameState
					NET_ExecRmEvGameState( (RE_GameState*) pREList );
				}
				break;

			case RE_COMMANDINFO:
				
				// execute the single remote event containing the CommandInfo
				NET_ExecRmEvCommandInfo( (RE_CommandInfo*) pREList );

				// check whether a RE_COMMANDINFO in the stream disconnected us
				// stop executing remote events
				if ( NetConnected == NETWORK_GAME_OFF ) {
					return;
				}

				break;

			case RE_CLIENTINFO:

				// RE_ClientInfo is C->S only
				ASSERT( FALSE );
				break;

			case RE_IPV4SERVERINFO:

				// can be received when connected and when not connected
				NET_ExecRmEvIPv4ServerInfo( (RE_IPv4ServerInfo*) pREList );
				break;

			case RE_SERVERLINKINFO:

				// can be received when connected and when not connected
				NET_ExecRmEvServerLinkInfo( (RE_ServerLinkInfo*) pREList );
				break;

			case RE_MAPOBJECT:
				// can be received when connected and when not connected
				NET_ExecRmEvMapObject( (RE_MapObject*) pREList );
				break;

			case RE_STARGATE:
				// can only be received when connected
				NET_ExecRmEvStargate( (RE_Stargate*) pREList );
				break;
			case RE_TELEPORTER:
				NET_ExecRmEvTeleporter( (RE_Teleporter*)pREList );
				break;
			case RE_GENERIC:

				break;
			default:
				MSGOUT( "ProcessRmEvList_GMSV(): unknown remote event (%d).", pREList->RE_Type );
		}
		
		// advance to next event in list
		ASSERT( ( pREList->RE_BlockSize == RE_BLOCKSIZE_INVALID ) ||
				( pREList->RE_BlockSize == NET_RmEvGetSize( pREList ) ) );
		pREList = (RE_Header *) ( (char *) pREList + NET_RmEvGetSize( pREList ) );
	}
}


// dump the ship remote info to the console -----------------------------------
//
static
void DumpShipRemInfo( ShipRemInfo* pShipRemInfo )
{
	MSGOUT( "CurYaw %d CurPitch %d CurRoll %d CurSlideHorz %d CurSlideVert %d", pShipRemInfo->CurYaw, 
																				pShipRemInfo->CurPitch, 
																				pShipRemInfo->CurRoll,
																				pShipRemInfo->CurSlideHorz,
																				pShipRemInfo->CurSlideVert );
}


// execute received remote event: create extra (2)-----------------------------
//
void NET_ExecRmEvCreateExtra2( RE_Header *rmev, int ownerid )
{
	ASSERT( rmev != NULL );
	ASSERT( rmev->RE_Type == RE_CREATEEXTRA2 );
	ASSERT( ( ( ownerid >= 0 ) && ( ownerid < MAX_NET_PROTO_PLAYERS ) ) || ( ownerid == PLAYERID_SERVER ) );

	RE_CreateExtra2* re_ce2  = (RE_CreateExtra2 *) rmev;

	// resolve to class id
	ASSERT( re_ce2->ExtraIndex < NumExtraClasses );
	dword classid = ExtraClasses[ re_ce2->ExtraIndex ];

	// create object if extra available on this client
	if ( classid != CLASS_ID_INVALID ) {

		ExtraObject *extrapo = (ExtraObject *) CreateObject( classid, re_ce2->ObjPosition );
		ASSERT( extrapo != NULL );
		extrapo->HostObjNumber = re_ce2->HostObjId;
		if ( extrapo->ObjectType == MINE1TYPE ) {
			ASSERT( ownerid != PLAYERID_SERVER );
			((MineObject*)extrapo)->Owner = ownerid;
			AUD_Mine( extrapo );
		} else {
			OBJ_FillExtraMemberVars( extrapo );
		}

		// get drift vector
		extrapo->DriftVec.X = re_ce2->DriftVec.X;
		extrapo->DriftVec.Y = re_ce2->DriftVec.Y;
		extrapo->DriftVec.Z = re_ce2->DriftVec.Z;

		// get drift time
		extrapo->DriftTimeout = re_ce2->DriftTimeout;

		RMEVTXT( MSGOUT( "--executing RE_CREATEEXTRA2: class %d, obj %d.", classid, extrapo->HostObjNumber ); );
	}
}


// execute RE containing a command info ---------------------------------------
//
void NET_ExecRmEvCommandInfo( RE_CommandInfo* commandinfo )
{
	ASSERT( commandinfo->RE_Type == RE_COMMANDINFO );

	// copy to safe buffer
	char recvline[ MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ];
	memset( recvline, 0, ( MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ) * sizeof( char ) );
	ASSERT( sizeof( recvline ) >= sizeof( commandinfo->command ) );
	strncpy( recvline, commandinfo->command, MAX_RE_COMMANDINFO_COMMAND_LEN );
	recvline[ MAX_RE_COMMANDINFO_COMMAND_LEN ] = 0;

	// only process non-empty commands
	if( *recvline != '\0' ) {
		// process the command
		NET_ServerParseMessage( recvline );
	} 

	DBGTXT(MSGOUT( "NET_ExecRmEvCommandInfo: %s", recvline ););
}


// exectue RE containing a serverinfo from the masterserver -------------------
//
void NET_ExecRmEvIPv4ServerInfo( RE_IPv4ServerInfo* pIPv4ServerInfo )
{
	ASSERT( pIPv4ServerInfo != NULL );

	MSGOUT( "got server %d from masterserver: %d.%d.%d.%d:%d at pos %d/%d",	
				pIPv4ServerInfo->serverid,
				pIPv4ServerInfo->node[ 0 ], 
				pIPv4ServerInfo->node[ 1 ], 
				pIPv4ServerInfo->node[ 2 ], 
				pIPv4ServerInfo->node[ 3 ], 
				UDP_GetNodePort( (node_t*)pIPv4ServerInfo->node ),
				pIPv4ServerInfo->xpos,
				pIPv4ServerInfo->ypos
				);

	// try to update server in serverlist ( also appends to empty serverlist, if no full serverlist has yet been requested )
	if ( !NET_ServerList_UpdateServer( pIPv4ServerInfo ) ) {

		// normal add to the serverlist ( no update )
		NET_ServerList_AddServer( pIPv4ServerInfo );
	}
}


// exectue RE containing a server linkinfo from the masterserver --------------
//
void NET_ExecRmEvServerLinkInfo( RE_ServerLinkInfo* pServerLinkInfo )
{
	ASSERT( pServerLinkInfo != NULL );

	/*MSGOUT( "got linkinfo from masterserver: %d to %d, flags: %d",	
				pServerLinkInfo->serverid1, 
				pServerLinkInfo->serverid2,
				pServerLinkInfo->flags );
	*/

	// link the 2 servers
	NET_ServerList_AddLinkInfo( pServerLinkInfo );
}


// exectue RE containing a map object to be displayed in the starmap ----------
//
void NET_ExecRmEvMapObject( RE_MapObject* pMapObject )
{
	ASSERT( pMapObject != NULL );

	// link the 2 servers
	NET_ServerList_AddMapObject( pMapObject );
}

// exectue RE containing a teleporter -------------------------------------------
//
void NET_ExecRmEvTeleporter( RE_Teleporter* pRE_Teleporter )
{
	ASSERT( pRE_Teleporter != NULL );
	ASSERT( NetConnected );

	// found an existing stargate ?
	Teleporter* teleporter = NET_FindTeleporter( pRE_Teleporter->id );

	// create new stargate ?
	if ( teleporter == NULL ) {

		 MSGOUT( "adding teleporter at (%d %d %d)", 
										    			pRE_Teleporter->pos[ 0 ],
														pRE_Teleporter->pos[ 1 ],
														pRE_Teleporter->pos[ 2 ] ); 

		dword objclass = OBJ_FetchObjectClassId( "teleporter" );

		if ( objclass != CLASS_ID_INVALID ) {

			Xmatrx startm;
			MakeIdMatrx( startm );
			startm[ 0 ][ 3 ] = pRE_Teleporter->pos[ 0 ];
			startm[ 1 ][ 3 ] = pRE_Teleporter->pos[ 1 ];
			startm[ 2 ][ 3 ] = pRE_Teleporter->pos[ 2 ];

			startm[ 0 ][ 2 ] = pRE_Teleporter->dir[ 0 ];
			startm[ 1 ][ 2 ] = pRE_Teleporter->dir[ 1 ];
			startm[ 2 ][ 2 ] = pRE_Teleporter->dir[ 2 ];

			// ensure orthogonal matrix
			CrossProduct2( &startm[ 0 ][ 1 ], &startm[ 0 ][ 2 ], &startm[ 0 ][ 0 ] );
			CrossProduct2( &startm[ 0 ][ 0 ], &startm[ 0 ][ 2 ], &startm[ 0 ][ 1 ] );

			teleporter = (Teleporter *) SummonObject( objclass, startm );
			teleporter->id = pRE_Teleporter->id;    
			teleporter->exit_delta_x = pRE_Teleporter->exit_delta_x;
			teleporter->exit_delta_y = pRE_Teleporter->exit_delta_y;
			teleporter->exit_delta_z = pRE_Teleporter->exit_delta_z;

			teleporter->exit_rot_phi = pRE_Teleporter->exit_rot_phi;
			teleporter->exit_rot_theta = pRE_Teleporter->exit_rot_theta;

		} else {
			MSGOUT( "object class Teleporter not found." );
		}
	} else {
		// found the teleporter, let's edit some of the stuff.
 
		// set the start coords
		teleporter->start.X = pRE_Teleporter->pos[ 0 ];
		teleporter->start.Y = pRE_Teleporter->pos[ 1 ];
		teleporter->start.Z = pRE_Teleporter->pos[ 2 ];

		// set the start rotation spherical coords
		float phi_atan = (pRE_Teleporter->dir[ 0 ]) ? (pRE_Teleporter->dir[ 1 ]/pRE_Teleporter->dir[ 0 ]) : 0; 
		teleporter->start_rot_phi = (atan(phi_atan)*180)/M_PI;
		float theta_atan = (pRE_Teleporter->dir[ 2 ]) ? ((sqrt(powf(pRE_Teleporter->dir[ 0 ],2)+powf(pRE_Teleporter->dir[ 1 ],2))/pRE_Teleporter->dir[ 2 ])) : 0;
		teleporter->start_rot_theta = (atan(theta_atan)*180)/M_PI;

		teleporter->exit_delta_x = pRE_Teleporter->exit_delta_x;
		teleporter->exit_delta_y = pRE_Teleporter->exit_delta_y;
		teleporter->exit_delta_z = pRE_Teleporter->exit_delta_z;

		teleporter->exit_rot_phi = pRE_Teleporter->exit_rot_phi;
		teleporter->exit_rot_theta = pRE_Teleporter->exit_rot_theta;
 
 	}

	// notify the teleporter that it changed some properties.
	TeleporterPropsChanged(teleporter);

}

// exectue RE containing a stargate -------------------------------------------
//
void NET_ExecRmEvStargate( RE_Stargate* pRE_Stargate )
{
	ASSERT( pRE_Stargate != NULL );
	ASSERT( NetConnected );

	// found an existing stargate ?
	Stargate* stargate = NET_FindStargate( pRE_Stargate->serverid );

	// create new stargate ?
	if ( stargate == NULL ) {

		DBGTXT( MSGOUT( "adding stargate to %d at (%d %d %d)", 
														pRE_Stargate->serverid,
										    			pRE_Stargate->pos[ 0 ],
														pRE_Stargate->pos[ 1 ],
														pRE_Stargate->pos[ 2 ] ); );

		dword objclass = OBJ_FetchObjectClassId( "stargate" );

		if ( objclass != CLASS_ID_INVALID ) {

			Xmatrx startm;
			MakeIdMatrx( startm );
			startm[ 0 ][ 3 ] = pRE_Stargate->pos[ 0 ];
			startm[ 1 ][ 3 ] = pRE_Stargate->pos[ 1 ];
			startm[ 2 ][ 3 ] = pRE_Stargate->pos[ 2 ];

			startm[ 0 ][ 2 ] = pRE_Stargate->dir[ 0 ];
			startm[ 1 ][ 2 ] = pRE_Stargate->dir[ 1 ];
			startm[ 2 ][ 2 ] = pRE_Stargate->dir[ 2 ];

			// ensure orthogonal matrix
			CrossProduct2( &startm[ 0 ][ 1 ], &startm[ 0 ][ 2 ], &startm[ 0 ][ 0 ] );
			CrossProduct2( &startm[ 0 ][ 0 ], &startm[ 0 ][ 2 ], &startm[ 0 ][ 1 ] );

			stargate = (Stargate *) SummonObject( objclass, startm );
		} else {
			MSGOUT( "object class stargate not found." );
		}
	}

	// read properties with persistency callback
	if ( stargate != NULL ) {
		if ( stargate->callback_persist != NULL ) {
			stargate->callback_persist( (CustomObject*)stargate, FALSE, (void*)pRE_Stargate );
		}

		// request server information for server
		NET_ServerList_Get( Masters[ 0 ], stargate->serverid );
	}
}
// exectue RE for generic actions and such. -------------------------------------------
//
void NET_ExecRmEvGeneric( RE_Generic* pRE_Generic) 
{
	ASSERT( pRE_Generic != NULL );
	ASSERT( NetConnected );


	// check for afterburner enabled
	if(pRE_Generic->RE_ActionFlags &  (1 << AFTB_ACTIVE)){

	}

	// check for afterburner disabled
	if(pRE_Generic->RE_ActionFlags &  (1 << AFTB_INACTIVE)){

	}

	// check for invulnerability
	if(pRE_Generic->RE_ActionFlags &  (1 << INVUNERABLE)){

		dword ownerid = GetOwnerFromHostOjbNumber(pRE_Generic->HostObjId);			
		// fetch pointer to remote player's ship
		ShipObject *invul_ship = NET_FetchOwnersShip( ownerid );
		((G_ShipObject*)invul_ship)->CollectSpecial( SPMASK_INVULNERABILITY);

	}

	// check for teleporter collision
	if(pRE_Generic->RE_ActionFlags &  (1 << TELEP_COLLIDE)){

	}



}



// execute RE holding newest player & ship state  -----------------------------
//
void NET_ExecRmEvPlayerAndShipStatus( RE_PlayerAndShipStatus* pas_state, dword messageid )
{
	ASSERT( pas_state != NULL );
	ASSERT( pas_state->RE_Type == RE_PLAYERANDSHIPSTATUS );

	// get the sender
	int nClientID = pas_state->senderid;

	UPDTXT(	MSGOUT( "updating game state for client %d (msg=%d).", nClientID, messageid ); );

	// keep player from being kicked out ( only done in PEER )
	Player_AliveCounter[ nClientID ] = MAX_ALIVE_COUNTER;

	// set up-to-date flag (i.e., no interpolation need be performed since real data is available).
	Player_UpToDate[ nClientID ] = TRUE;

	// ship status will only be updated on newer messages
	if ( messageid > (dword)Player_LastUpdateGameStateMsgId[ nClientID ] ) {
		if ( nClientID == LocalPlayerId ) {
			
			// set the properties/speeds/position of the local player
			NET_ResyncLocalPlayer( pas_state );

			// clear all predictions made obsolete with this update from the server
			// and re-apply the ones not yet ACK
			ThePredictionManager.DoCheckpoint( ServerStream.GetLastACKMessageId() );

		} else {

			// player must be joined in order to send GAME_STATE
			if ( Player_Status[ nClientID ] != PLAYER_JOINED ) {
				DBGTXT( MSGOUT( "filtering GAME_STATE for player who is not joined: %d.", nClientID ); );
				return;
			}

			// ensure correct player state
			ASSERT( Player_Status[ nClientID ] == pas_state->player_status );

			ShipRemInfo _ShipRemInfo;
			memcpy( &_ShipRemInfo.ObjPosition, pas_state->ObjPosition, sizeof( Xmatrx ) );
			_ShipRemInfo.CurDamage		= pas_state->CurDamage;
			_ShipRemInfo.CurShield		= pas_state->CurShield;			
			_ShipRemInfo.CurSpeed		= pas_state->CurSpeed;			
			_ShipRemInfo.CurYaw			= pas_state->CurYaw;				
			_ShipRemInfo.CurPitch		= pas_state->CurPitch;			
			_ShipRemInfo.CurRoll		= pas_state->CurRoll;			
			_ShipRemInfo.CurSlideHorz	= pas_state->CurSlideHorz;		
			_ShipRemInfo.CurSlideVert	= pas_state->CurSlideVert;		
			_ShipRemInfo.NumMissls      = pas_state->NumMissls;
			_ShipRemInfo.NumHomMissls   = pas_state->NumHomMissls;
			_ShipRemInfo.NumMines       = pas_state->NumMines;
            _ShipRemInfo.NumPartMissls  = pas_state->NumPartMissls;
            
			// fetch pointer to remote player's local ship-object
			ShipObject *playerobj = (ShipObject *) Player_Ship[ nClientID ];
			ASSERT( playerobj != NULL );

			//FIXME: we must change this to work with RE_PlayerShipAndStatus
			NET_ExecShipRemInfo( &_ShipRemInfo, playerobj, nClientID );
		}
	} else {
		DBGTXT( MSGOUT( "NET_ExecRmEvShipStatus(): supressed update from packet %d ( already had %d )", messageid, Player_LastUpdateGameStateMsgId[ nClientID ] ); );
	}

	// ensure message ids are monotonically increasing
	Player_LastUpdateGameStateMsgId[ nClientID ] = max( (dword)Player_LastUpdateGameStateMsgId[ nClientID ], messageid );
}


// store commandinfo as RE_List head ------------------------------------------
//
RE_CommandInfo* NET_RmEvSingleCommandInfo( NetPacket_GMSV* gamepacket, const char* commandstring )
{
	ASSERT( gamepacket != NULL );
	ASSERT( gamepacket->Command == PKTP_COMMAND );
	RE_CommandInfo* re_commandinfo = (RE_CommandInfo*)&gamepacket->RE_List;

	int len = strlen( commandstring );
	if ( len > MAX_RE_COMMANDINFO_COMMAND_LEN ) {
		len = MAX_RE_COMMANDINFO_COMMAND_LEN;
	}

	re_commandinfo->RE_Type = RE_COMMANDINFO;
	re_commandinfo->RE_BlockSize = len + 1/*string terminator '\0'*/ + 2/*header*/ + 1/*code*/;
	strncpy(re_commandinfo->command, commandstring, MAX_RE_COMMANDINFO_COMMAND_LEN );
	re_commandinfo->command[ MAX_RE_COMMANDINFO_COMMAND_LEN ] = 0;

	char* pNext = (char*)re_commandinfo;
	pNext += re_commandinfo->RE_BlockSize;
	((RE_Header*)pNext)->RE_Type = RE_EMPTY;

	return re_commandinfo;
}


// scan a remote event list for a specific type -------------------------------
//
RE_Header* FindInREList( RE_Header *relist, byte type )
{
	ASSERT( relist != NULL );

	// scan remote event list for already inserted type
	while ( relist->RE_Type != RE_EMPTY ) {

		if ( relist->RE_Type == type ) {
			return relist;
		}

		// advance to next event
		ASSERT( ( relist->RE_BlockSize == RE_BLOCKSIZE_INVALID ) ||
				( relist->RE_BlockSize == NET_RmEvGetSize( relist ) ) );
		relist = (RE_Header *) ( (char *) relist + NET_RmEvGetSize( relist ) );
	}

	return NULL;
}


// insert ClientInfo event into RE_List ---------------------------------------
//
RE_ClientInfo* NET_RmEvClientInfo( int nSendFreq, int nRecvRate )
{
	//FIXME: this remote event must be transfered RELIABLE

	if ( NetConnected == NETWORK_GAME_ON ) {

		// adjust to bounds
		nSendFreq = max( CLIENT_SEND_FREQUENCY_MIN, nSendFreq );
		nSendFreq = min( CLIENT_SEND_FREQUENCY_MAX, nSendFreq );
		nRecvRate = max( CLIENT_RECV_RATE_MIN, nRecvRate );
		nRecvRate = min( CLIENT_RECV_RATE_MAX, nRecvRate );

		// scan remote event list for already inserted RE_ClientInfo
		RE_ClientInfo* re_clientinfo = (RE_ClientInfo*)FindInREList( (RE_Header *) REListMem, RE_CLIENTINFO );

		// reuse already inserted RE 
		if ( re_clientinfo != NULL ) {

			// fill in info
			re_clientinfo->client_sendfreq = (byte)nSendFreq;
			re_clientinfo->server_sendrate = PACK_SERVERRATE( nRecvRate );

		} else {

			if ( (size_t)RE_List_Avail < sizeof( RE_ClientInfo ) ) {
				ASSERT( 0 );
				return 0;
			}

			// append RE
			re_clientinfo = (RE_ClientInfo*) RE_List_CurPos;
			re_clientinfo->RE_Type		= RE_CLIENTINFO;
			re_clientinfo->RE_BlockSize	= sizeof( RE_ClientInfo );

			// fill in info
			re_clientinfo->client_sendfreq = (byte)nSendFreq;
			re_clientinfo->server_sendrate = PACK_SERVERRATE( nRecvRate );

			// maintain list
			re_clientinfo++;
			re_clientinfo->RE_Type = RE_EMPTY;

			RE_List_CurPos = (char *) re_clientinfo;
			RE_List_Avail -= sizeof( RE_ClientInfo );
		}

		return re_clientinfo;
	} else {
		return NULL;
	}
}

