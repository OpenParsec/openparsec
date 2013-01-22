/*
 * PARSEC - Global Variables
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:30 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
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

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// local module header
#include "net_glob.h"

// subsystem headers
#include "net_defs.h"

// engine globals
#include "e_global.h"

//NOTE:
// the inclusion order of NET_GLOB.H and NET_DEFS.H
// has to be this way, because one inline function
// in NET_DEFS.H references PlayerId and NET_GLOB.C
// won't compile if the prototype in NET_GLOB.H is
// not included beforehand.

// proprietary module headers
#include "net_udpdf.h"


// net id of local player -----------------------------------------------------
//
int 			LocalPlayerId		= PLAYERID_ANONYMOUS;


// name of local player -------------------------------------------------------
//
char			LocalPlayerName[ MAX_PLAYER_NAME + 1 ];


// miscellaneous network game globals -----------------------------------------
//
int 			NetAvailable			= FALSE;
int 			NetworkGameMask			= NETWORK_GAME_OFF;
int				NetConnected			= NETWORK_GAME_OFF;		// connect status
int				NetJoined				= FALSE;				// join status
int 			EntryMode				= FALSE;
int 			ShipDowned				= FALSE;
int				Packet_Send_Frequency	= DEFAULT_CLIENT_SEND_FREQUENCY; /* 20Hz */
int 			Packet_Send_Frametime	= FRAME_MEASURE_TIMEBASE / Packet_Send_Frequency;
int				Packet_Recv_Rate		= DEFAULT_CLIENT_RECV_RATE;

// indicates whether we have received a full playerstate from the server ------
//
int				HaveFullPlayerState     = FALSE;

// # of refframes the server times out ----------------------------------------
//
refframe_t		ServerTimeoutFrames		= DEFAULT_TIMEOUT_SERVER_CONNECTION;


// allows to select a specific interface on a multi-homed host ----------------
//
int				NetInterfaceSelect	= 0;

// name of server to resolve ( used by NET_ServerConnect() implementation) ----
//
char*			CurServerToResolve  = NULL;

// node of server to jump to (used by NET_ServerJump() implementation) --------
//
node_t*			CurJumpServerNode	= NULL;


// current maximum number of players (in peer-to-peer or on server) -----------
//
int 			CurMaxPlayers		= MAX_NET_ALLOC_SLOTS;


// server ID of current server ------------------------------------------------
//
int				CurServerID         = -1;


// current maximum size of the packet payload ---------------------------------
//
int				CurMaxDataLength	= NET_UDP_DATA_LENGTH;


// id of player whose killstat will be sent next ------------------------------
//
int 			CurKillUpdate		= 0;


// id of last player who killed us --------------------------------------------
//
int 			CurKiller			= KILLERID_UNKNOWN;


// current universe (game area on same server/connection) ---------------------
//
int 			MyUniverse			= 0;


// number of active remote players (including local player!) ------------------
//
int				NumRemPlayers		= 0;


// id that will be appended to next message for sequencing purposes -----------
//
int				CurSendMessageId_PEER	= 1;


// current game time ----------------------------------------------------------
//
int 			CurGameTime			= GAME_PEERTOPEER;


// fictitious name of current server (not related to the actual host name) ----
//
char*			CurServerName		= NULL;


// current ping to current server (for display purposes) ----------------------
//
int 			CurServerPing		= -1;


// ping time to server (gets set on ping reply processing) --------------------
//
int				ServerPingTime		= -1;


// global remote event list administration ------------------------------------
//
int 			RE_List_Avail;
char*			RE_List_CurPos;


// info needed about each remote player ---------------------------------------
//
int 			Player_Status[ MAX_NET_ALLOC_SLOTS ];
GenObject *		Player_Ship[ MAX_NET_ALLOC_SLOTS ];
int 			Player_ShipId[ MAX_NET_ALLOC_SLOTS ];
int 			Player_AliveCounter[ MAX_NET_ALLOC_SLOTS ];
int 			Player_UpToDate[ MAX_NET_ALLOC_SLOTS ];
int 			Player_KillStat[ MAX_NET_ALLOC_SLOTS ];
int 			Player_LastMsgId[ MAX_NET_ALLOC_SLOTS ];
int				Player_LastUpdateGameStateMsgId[ MAX_NET_ALLOC_SLOTS ];
char	  		Player_Name[ MAX_NET_ALLOC_SLOTS ][ MAX_PLAYER_NAME + 1 ];


// list of masterservers to query ---------------------------------------------
//
char*			Masters[ MAX_MASTERSERVERS ] = { NULL, NULL, NULL };


// jump table for networking subsystem ----------------------------------------
//
net_subsys_jtab_s	net_subsys_jtab;


// module registration function -----------------------------------------------
//
REGISTER_MODULE( NET_GLOB )
{

}



