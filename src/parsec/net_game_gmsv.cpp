/*
 * PARSEC - Network Game Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:30 $
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

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "net_defs.h"
#include "sys_defs.h"

// mathematics header
#include "utl_math.h"

// network code config
#include "net_conf.h"

// subsystem linkage info
#include "linkinfo.h"

// local module header
#include "net_game_gmsv.h"


// proprietary module headers
#include "aud_game.h"
#include "con_aux.h"
#include "g_camera.h"
#include "g_stars.h"
#include "g_supp.h"
#include "h_supp.h"
#include "m_main.h"
#include "net_conn.h"
#include "net_csdf.h"
#include "net_game.h"
#include "net_rmev_gmsv.h"
#include "net_serv.h"
#include "net_stream.h"
#include "net_udpdf.h"
#include "net_wrap.h"
#include "obj_ctrl.h"
#include "obj_xtra.h"
#include "g_sfx.h"

// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];

// game header 
//FIXME: move all the inline functions in G_MAIN_CL::G_Main to implementation file
#include "g_main_cl.h"

// set correct function names for dynamic binding------------------------------
//
#ifdef DBIND_PROTOCOL
	#undef	NETs_RmEvList_GetMaxSize
	#undef  NETs_UpdateKillStats

	#define NETs_RmEvList_GetMaxSize		NETs_GAMESERVER_RmEvList_GetMaxSize
	#define NETs_UpdateKillStats			NETs_GAMESERVER_UpdateKillStats
#endif


// return the max. size of the RE list that can fit in one external packet ----
//
size_t NETs_RmEvList_GetMaxSize()
{
	return ( NET_UDP_DATA_LENGTH - sizeof( NetPacketExternal_GMSV ) );
}


// do complete kill stats update from received remote event -------------------
//
void NETs_UpdateKillStats( RE_KillStats* killstats )
{
	ASSERT( NetConnected );
	ASSERT( killstats != NULL );
	
	//NOTE:
	// this function always sets the current kill stat the ones received from the server
	
	// update entire kill stats list
	for ( int nPlayer = 0; nPlayer < MAX_NET_ALLOC_SLOTS; nPlayer++ ) {
		if ( Player_Status[ nPlayer ] ) {
			Player_KillStat[ nPlayer ] = killstats->PlayerKills[ nPlayer ];
		}
	}
}


// measure ping time to specified server --------------------------------------
//
void NET_PingCurrentServer()
{
	// send server ping packet, don't send anonymously, no full info
	NET_ServerPing( &Server_Node, FALSE, FALSE );
}


// handle a PKTP_STREAM packet from the server --------------------------------
//
void NET_Handle_STREAM( NetPacket_GMSV* gamepacket )
{
	ASSERT( gamepacket != NULL );

	ASSERT( gamepacket->Command == PKTP_STREAM );

	// STREAM comes from the server ( process only remote events )
	ASSERT( ( gamepacket->SendPlayerId == PLAYERID_SERVER ) || 
		    ( ( gamepacket->SendPlayerId == PLAYERID_MASTERSERVER ) && ( gamepacket->MessageId == MSGID_DATAGRAM ) ) );

	// process remote event list ( server specific )
	NET_ProcessRmEvList_GMSV( gamepacket );

	return;
}

// disconnect the client as there is no connection to the server anymore ------
//
void NET_DisconnectNoConnection()
{
	MSGOUT( "lost connection to server" );

extern int disable_disc_command_send_to_server;					
	disable_disc_command_send_to_server = TRUE;

	// disconnect and notify menu
	if ( NET_CommandDisconnect() ) {
		MenuNotifyDisconnect();
	}

	disable_disc_command_send_to_server = FALSE;
}


// resync the local player with information from the server -------------------
//
void NET_ResyncLocalPlayer( RE_PlayerAndShipStatus* pas_status )
{
	ASSERT( pas_status != NULL );
	ASSERT( pas_status->senderid == LocalPlayerId );
	ASSERT( NET_ConnectedGMSV() );

	//FIXME: we need to change RE_PlayerAndShipStatus to a flexible structure
	//       and specify exactly whats inside via the flags, in order to not
	//       waste space

	// got new position from server ?
	if ( pas_status->UpdateFlags & UF_RESYNCPOS ) {
		DBGTXT( MSGOUT( "Resync local player with server" ); );

#ifdef PARSEC_DEBUG
		MSGPUT( "RESYNC " );
		DumpMatrix( pas_status->ObjPosition );
#endif // PARSEC_DEBUG

		// override position/orientation
		memcpy( MyShip->ObjPosition, pas_status->ObjPosition, sizeof( Xmatrx ) );

		// transform world->view to view->world
		CalcOrthoInverse( MyShip->ObjPosition, ShipViewCamera );

		// reset view camera smoothing filter
		CAMERA_ResetFilter();

		// init pseudo- and fixed stars
		NumPseudoStars = 0;
		InitPseudoStars();

		// signal that we got a full playerstate from the server
		HaveFullPlayerState = TRUE;
	}

	// got new properties from server ?
	if ( HaveFullPlayerState && ( pas_status->UpdateFlags & UF_PROPERTIES ) ) {

#ifdef PARSEC_DEBUG
		if ( MyShip->CurEnergy != pas_status->CurEnergy )
			MSGOUT("new CurEnergy from server: %d ( was %d )", pas_status->CurEnergy, MyShip->CurEnergy );
#endif // PARSEC_DEBUG

		MyShip->CurEnergy			= pas_status->CurEnergy;
		MyShip->CurDamage			= pas_status->CurDamage;
		MyShip->CurShield			= pas_status->CurShield;
		MyShip->NumMissls           = pas_status->NumMissls;
		MyShip->NumHomMissls        = pas_status->NumHomMissls;
		MyShip->NumMines            = pas_status->NumMines;
	}

	// got new speeds from server ?
	if ( HaveFullPlayerState && ( pas_status->UpdateFlags & UF_SPEEDS ) ) {
		MyShip->CurSpeed			= pas_status->CurSpeed;
		RecYaw	 = CurYaw			= pas_status->CurYaw;
		RecPitch = CurPitch			= pas_status->CurPitch;
		RecRoll  = CurRoll			= pas_status->CurRoll;
		RecSlideHorz = CurSlideHorz	= pas_status->CurSlideHorz;
		RecSlideVert = CurSlideVert = pas_status->CurSlideVert;
	}

	// got new status from server ?
	if ( pas_status->UpdateFlags & UF_STATUS ) {
		ASSERT( ( pas_status->player_status == PLAYER_CONNECTED ) || ( pas_status->player_status == PLAYER_JOINED ) );

		if ( Player_Status[ LocalPlayerId ] != pas_status->player_status ) {
			
			// if the last state change ( NETs_Join/NETs_Unjoin ) is ACK by the server, 
			// we override the local state with the one from the server, otherwise we ignore it
			extern int g_dwLastStatusChangeMsg;
			if ( ServerStream.IsACK( g_dwLastStatusChangeMsg ) ) {
				if ( pas_status->player_status == PLAYER_CONNECTED ) {

					Player_Status[ LocalPlayerId ] = PLAYER_CONNECTED;
					Player_Ship  [ LocalPlayerId ] = NULL;
					Player_ShipId[ LocalPlayerId ] = SHIPID_NOSHIP;

					// reset global flag
					NetJoined = FALSE;
					HaveFullPlayerState = FALSE;

					// downing of ship starts explosion
					ASSERT( pas_status->params[ 0 ] == SHIP_DOWNED );
						
					// exec gamecode when local player ship is downed
					TheGameExtraction.GC_LocalPlayerKill( pas_status->params[ 2 ] - KILLERID_BIAS, pas_status->params[ 3 ] ) ;
				
				} else {
					// no support of changing the status from CONNECTED to JOINED from the server
					ASSERT( FALSE );
				}
			}
		}
	}
}
