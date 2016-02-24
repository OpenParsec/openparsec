/*
 * PARSEC - Game Server Protocol
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:30 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001-2002
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1998-2000
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
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"
#include "m_main.h" //CrazySpence

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "net_defs.h"
#include "sys_defs.h"

// network code config
#include "net_conf.h"
#include "net_conn.h" //CrazySpence

// mathematics header
#include "utl_math.h"

// local module header
#include "net_gmsv.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_ext.h"
#include "con_main.h"
#include "e_demo.h"
#include "net_prediction.h"
#include "e_record.h"
#include "g_stars.h"
#include "g_supp.h"
#include "net_game.h"
#include "net_game_gmsv.h"
#include "net_rmev.h"
#include "net_rmev_gmsv.h"
#include "net_serv.h"
#include "net_stream.h"
#include "net_udpdf.h"
#include "obj_ctrl.h"
#include "obj_part.h"
#include "g_sfx.h"
#include "net_csdf.h"
#include "net_univ.h"
#include "net_wrap.h"

// string scratchpad
#define PASTE_STR_LEN 255
static char	paste_str[ PASTE_STR_LEN + 1 ];


extern int			server_udp_port;


// buffer that can be used to prepare NetPacket for sending --------------------
//
static char				GamePacketBuffer[ NET_MAX_NETPACKET_INTERNAL_LEN ];
static NetPacket_GMSV*	GamePacket = (NetPacket_GMSV*) GamePacketBuffer;


// previous game state used for dead-reckoning --------------------------------
//
static RE_PlayerAndShipStatus	prev_shipinfo;			// ship activity
static refframe_t				prev_refframecount;		// timetag


// default masterserver hostname ----------------------------------------------
//
static char default_master[] = "master.openparsec.com";


// disable sending the disc command to the server when disconnecting ----------
//
int disable_disc_command_send_to_server = FALSE;

// time after which a packet will be sent in any case (reset dead reckoning) --
//
#define DEAD_TIME_LIMIT			300


// avoid nameclashes in debug version if multiple protocols linked ------------
//
#define SendGameState			GMSV_SendGameState
#define PlayersNotUptodate		GMSV_PlayersNotUptodate
#define InterpolatePlayers		GMSV_InterpolatePlayers
#define FireDurationWeapons		GMSV_FireDurationWeapons


// string constants -----------------------------------------------------------
//
static char invalid_in_peermode[]	= "command invalid in peer-to-peer mode.";

// 
//
int g_dwLastStatusChangeMsg = 0;

// try to connect to server ---------------------------------------------------
//
int NETs_Connect()
{
	ASSERT( !NetConnected && !NetJoined );
	//ASSERT( Player_Status[ LocalPlayerId ] == PLAYER_INACTIVE );
	ASSERT( CurServerToResolve != NULL );

	// set maximum number of players according to protocol
	CurMaxPlayers = MAX_NET_GMSV_PLAYERS;

	// (re)init tables
	NET_InitRemotePlayerTables();

	// discard old packets
	NETs_FlushListenBuffers();

	// already count local player
	//FIXME: why ????
	ASSERT( NumRemPlayers == 0 );
	NumRemPlayers = 1;

	// init the msg# of the last status change msg
	g_dwLastStatusChangeMsg = 0;

	// resolve the server name into a node
	if ( NET_ResolveServerNode() ) {

		// try to establish connection to server
		if ( NET_ServerConnect() ) {

			ASSERT( NetConnected );
			ASSERT( NumRemPlayers >= 1 );
			ASSERT( Player_Status[ LocalPlayerId ] == PLAYER_CONNECTED );

			return TRUE;

		}
	}

	//NOTE:
	// possible reasons for NET_ServerConnect() to fail:
	// - the DNS lookup failed.
	// - sending UDP packets failed.
	// - the server refused the connect request.

	ASSERT( !NetConnected );
	ASSERT( NumRemPlayers == 1 );

	// reset player count
	NumRemPlayers = 0;

	return FALSE;
}


// disconnect from server -----------------------------------------------------
//
int NETs_Disconnect()
{
	ASSERT( ( NetConnected == NETWORK_GAME_ON ) && !NetJoined );
	ASSERT( Player_Status[ LocalPlayerId ] != PLAYER_INACTIVE );

	// send remove request to server
	int rc = NET_ServerDisconnect();

	// clean up regardless of disconnect success at server
	NET_ServerDisconnectReset();

	return rc;
}


// join game (enter game from entry mode in already allocated slot) -----------
//
int NETs_Join()
{
	ASSERT( NetConnected && !NetJoined );
	ASSERT( LocalPlayerId >= 0 && LocalPlayerId < MAX_NET_PROTO_PLAYERS );
	ASSERT( Player_Status[ LocalPlayerId ] == PLAYER_CONNECTED );

	{
		// update player status
		Player_Status[ LocalPlayerId ] = PLAYER_JOINED;
		Player_Ship  [ LocalPlayerId ] = MyShip;
		Player_ShipId[ LocalPlayerId ] = SHIPID_LOCALPLAYER;

		// set global flags
		NetJoined = TRUE;
		HaveFullPlayerState = FALSE;
		
		// open stargate
		if ( !AUX_DISABLE_LOCAL_STARGATE ) {
			SFX_CreateStargate( MyShip );
		}

		// record join
		Record_Join();
	}

	// store the message # that contains this state change
	g_dwLastStatusChangeMsg = ServerStream.GetNextOutMessageId(); 

	// send join message to server
	DBGTXT( MSGOUT( "sending join message to server." ); );
	NETs_StdGameHeader( PKTP_STREAM, (NetPacket*)GamePacket );
	NET_RmEvSinglePlayerStatus( (RE_Header*)&GamePacket->RE_List, PLAYER_JOINED, -1, -1 );
	NETs_AuxSendPacket( (NetPacket*)GamePacket, &Server_Node );

	return TRUE;
}


// notify remote players of unjoin (exit to entry-mode) -----------------------
//
int NETs_Unjoin( byte flag )
{
	ASSERT( NetConnected && NetJoined );
	ASSERT( LocalPlayerId >= 0 && LocalPlayerId < MAX_NET_PROTO_PLAYERS );
	ASSERT( Player_Status[ LocalPlayerId ] == PLAYER_JOINED );

	// SHIP_DOWNED is not sent from the client anymore
	ASSERT( flag == USER_EXIT );

	{
		// update player status
		Player_Status[ LocalPlayerId ] = PLAYER_CONNECTED;
		Player_Ship  [ LocalPlayerId ] = NULL;
		Player_ShipId[ LocalPlayerId ] = SHIPID_NOSHIP;

		// reset global flag
		NetJoined = FALSE;
		HaveFullPlayerState = FALSE;

		// open stargate
		if ( !AUX_DISABLE_LOCAL_STARGATE ) {
			SFX_CreateStargate( MyShip );
		}

		// record unjoin
		Record_Unjoin( USER_EXIT );
	}

	// store the message # that contains this state change
	g_dwLastStatusChangeMsg = ServerStream.GetNextOutMessageId(); 

	// send unjoin message to server
	DBGTXT( MSGOUT( "sending unjoin message to server." ); );
	NETs_StdGameHeader( PKTP_STREAM, (NetPacket*)GamePacket );
	//FIXME: NET_RmEvSinglePlayerStatus sufficient ?
	NET_RmEvSinglePlayerAndShipStatus( (RE_Header*)&GamePacket->RE_List, PLAYER_CONNECTED, USER_EXIT, 0 );
	NETs_AuxSendPacket( (NetPacket*)GamePacket, &Server_Node );

	return TRUE;
}


// update player name on server -----------------------------------------------
//
int NETs_UpdateName()
{
	return NET_ServerUpdateName();
}


// process the content of a packet received in game-loop ( not connected to a server )
//
void NET_ProcPacketLoop_Disconnected_GMSV( NetPacket* int_gamepacket, int bufid )
{
	ASSERT( int_gamepacket != NULL );

	NetPacket_GMSV* gamepacket = (NetPacket_GMSV*)int_gamepacket;

	// check for PKTP_STREAM packets from the masterserver
	if ( gamepacket->Command == PKTP_STREAM ) {
		if ( ( gamepacket->SendPlayerId == PLAYERID_MASTERSERVER ) || ( gamepacket->SendPlayerId == PLAYERID_SERVER ) ) {

			ASSERT( gamepacket->MessageId == MSGID_DATAGRAM );

			// regular stream packets from server
			NET_Handle_STREAM( gamepacket );
		}
	} 
}



// process the content of a packet received in game-loop ----------------------
//
void NET_ProcPacketLoop_GMSV( NetPacket* int_gamepacket, int bufid )
{
	ASSERT( int_gamepacket != NULL );

	NetPacket_GMSV* gamepacket = (NetPacket_GMSV*)int_gamepacket;

	// perform action according to packet type
	switch( gamepacket->Command ) {

		// these must not be received
		case PKTP_CONNECT:
		case PKTP_CONNECT_REPLY:
		case PKTP_DISCONNECT:
		case PKTP_SLOT_REQUEST:
		case PKTP_SUBDUE_SLAVE:

			//NOTE:
			// these should be ignored transparently since somebody
			// might try to connect peer-to-peer on the same
			// segment and would kick us out.

//			ASSERT( 0 );
			break;

		case PKTP_JOIN:
		case PKTP_UNJOIN:
		case PKTP_NODE_ALIVE:
		case PKTP_GAME_STATE:
		//case PKTP_PING:
			//NOTE: legacy packet = removed in new GameServer ( >= build 0198 )
			break;

		case PKTP_STREAM:

			if ( NetConnected  ) {
				// regular stream packets from server
				NET_Handle_STREAM( gamepacket );
			} else {
				DBGTXT( MSGOUT( "NET_ProcPacketLoop_GMSV(): received PKTP_STREAM when disconnected!"); );
			}

			break;

		case PKTP_COMMAND:

			// execute the single remote event containing the CommandInfo
			//FIXME: 
			NET_ExecRmEvCommandInfo( (RE_CommandInfo*) &gamepacket->RE_List );
			break;

		default:
			MSGOUT( "gameloop: received packet of unknown type: %d.", gamepacket->Command );
			ASSERT( 0 );
	}
}


// check if dead-reckoning can be done ----------------------------------------
//
PRIVATE
int DeadReckoning( RE_PlayerAndShipStatus* re_pas_status )
{
	ASSERT( re_pas_status != NULL );

	//NOTE:
	// cheap dead-reckoning.

	if ( AUX_DISABLE_DEAD_RECKONING )
		return FALSE;

	// remote-event list has to be empty
	RE_Header *relist = (RE_Header *) REListMem;
	if ( relist->RE_Type != RE_EMPTY )
		return FALSE;


	// don't dead-reckon if the ship is sliding, because sliding
	// doesn't look good with low packet send frequencies
	if ( ( re_pas_status->CurSlideHorz != 0 ) || ( re_pas_status->CurSlideVert != 0 ) ) {
		return FALSE;
	}

	int noactivity =
		( re_pas_status->CurSpeed		== prev_shipinfo.CurSpeed		) &&
		( re_pas_status->CurYaw			== prev_shipinfo.CurYaw			) &&
		( re_pas_status->CurPitch		== prev_shipinfo.CurPitch	  	) &&
		( re_pas_status->CurRoll		== prev_shipinfo.CurRoll		) &&
		( re_pas_status->CurSlideHorz	== prev_shipinfo.CurSlideHorz	) &&
		( re_pas_status->CurSlideVert	== prev_shipinfo.CurSlideVert	);

	// test for forced send interval
	refframe_t refframe = SYSs_GetRefFrameCount();
	int resetit  = ( ( refframe - prev_refframecount ) > DEAD_TIME_LIMIT );

	return ( noactivity && !resetit );
}



// send current player state to server ----------------------------------------
//
PRIVATE
void SendGameState()
{
	// fill game data header
	NETs_StdGameHeader( PKTP_STREAM, (NetPacket*)GamePacket );

	// update locations in global RE list
	NET_RmEvListUpdateLocations();


	RE_PlayerAndShipStatus* re_pas_status = NULL;

	// only send shipstate once we have got the initial state from the server
	if ( HaveFullPlayerState ) {
		re_pas_status = NET_RmEvPlayerAndShipStatus( USER_EXIT );

		//MSGPUT( "SEND   " );
		//DumpMatrix( MyShip->ObjPosition );

	} else {
		NET_RmEvPlayerStatus( USER_EXIT );
	}

	if ( re_pas_status == NULL ) {

		//UPDTXT( MSGOUT( "updating EVENTS-ONLY to server." ); );

		// send only global RE list packet to the server 
		NETs_SendPacket( (NetPacket*)GamePacket, &Server_Node );

	} else {

		UPDTXT3( MSGOUT( "updating player state & event to server." ); );

		// check if dead reckoning can be done
		if ( DeadReckoning( re_pas_status ) == FALSE ) {

			// send the packet to the server ( include global RE list )
			NETs_SendPacket( (NetPacket*)GamePacket, &Server_Node );

			// copy ShipInfo and store refframe-counter for DeadReckoning test
			prev_shipinfo		= *re_pas_status;
			prev_refframecount	= SYSs_GetRefFrameCount();

		} else {

			UPDTXT( MSGOUT( "no game update sent due to dead-reckoning." ); );
		}
	}
}

// assume all remote players are not up-to-date -------------------------------
//
PRIVATE
void PlayersNotUptodate()
{
	// reset up-to-date flags for all remote players
	for ( int id = 0; id < MAX_NET_PROTO_PLAYERS; id++ )
		Player_UpToDate[ id ] = FALSE;
}


// interpolate remote player actions ------------------------------------------
//
PRIVATE
void InterpolatePlayers()
{
#ifdef INTERPOLATE_PLAYER_ACTIONS

	// interpolate actions of active players that are not up-to-date
	for ( int id = 0; id < MAX_NET_PROTO_PLAYERS; id++ )
		if ( REMOTE_PLAYER_ACTIVE( id )	&&
			 !Player_UpToDate[ id ] && Player_Ship[ id ] ) {

			NET_InterpolatePlayer( id );
		}
#endif
}


// fire duration weapons ------------------------------------------------------
//
PRIVATE
void FireDurationWeapons()
{
	for ( int id = 0; id < MAX_NET_PROTO_PLAYERS; id++ )
		if ( REMOTE_PLAYER_JOINED( id ) ) {
			MaintainDurationWeapons( id );
		}
}


// perform jump to globally specified destination server ----------------------
//
PRIVATE
void JumpToCurJumpServer()
{
	ASSERT( CurJumpServerNode != NULL );

	// remember if we were joined before the transition
	int oldjoinedstate = NetJoined;

	// ensure that we are not joined during server transition
	if ( NetJoined ) {
		NETs_Unjoin( USER_EXIT );
	}

	// for now, let's re-init the ship on jump
	InitFloatingMyShip();

	// unlink local player's ship from ship objects list if object camera is
	// active. otherwise the killing of objects during the jump will complain.
	if ( ObjCameraActive ) {
		ASSERT( PShipObjects->NextObj == MyShip );
		PShipObjects->NextObj = MyShip->NextObj;
		MyShip->NextObj		  = NULL;
	}

	// try to jump to server
	if(NET_ServerJump()) {

		// if the jump succeeded the jump target has already been deleted.
		// if not, we delete it here to avoid retrying the jump endlessly.
		if ( CurJumpServerNode != NULL ) {
			FREEMEM( CurJumpServerNode );
			CurJumpServerNode = NULL;
		}

		if ( NetConnected ) {

			// auto-join game if we were joined before jump
			if ( oldjoinedstate ) {
				NETs_Join();
			}

			// attach static particles that are part of the object instance
			// since they have been killed during the disconnect
			//FIXME: ????????
			if ( AUX_ATTACH_OBJECT_PARTICLES ) {
				OBJ_AttachClassParticles( MyShip );
			}
		}

		// insert local player's ship into ship objects list if object camera is
		// active, since it had to be removed beforehand.
		if ( ObjCameraActive ) {
			ASSERT( PShipObjects->NextObj != MyShip );
			ASSERT( MyShip->NextObj == NULL );
			MyShip->NextObj		  = PShipObjects->NextObj;
			PShipObjects->NextObj = MyShip;
		}

		// do new random placement of pseudo stars
		//FIXME: ????????
		NumPseudoStars = 0;
		InitPseudoStars();
	} else {
		//This lets you open the starmap again instead of being stuck in menu limbo if Jump fails
		NetConnected = NETWORK_GAME_OFF;
		ExitGameLoop = 2; //pop up the menu so the user knows something happened
	}
}


// number of frames to display before a jump actually occurs ------------------
//
#define SERVER_JUMP_FRAMES_TO_DISPLAY	5
static int server_jump_frame_display_count = SERVER_JUMP_FRAMES_TO_DISPLAY;


// execute a server jump scheduled by a stargate due to transition ------------
//
PRIVATE
void ExecuteScheduledServerJump()
{
	// jump scheduling is global
	if ( CurJumpServerNode == NULL ) {
		return;
	}

	// allow for display of several frames before actual jump
	if ( server_jump_frame_display_count > 0 ) {
		server_jump_frame_display_count--;
		return;
	}

	// perform jump
	JumpToCurJumpServer();

	// reset frame display counter for next jump
	server_jump_frame_display_count = SERVER_JUMP_FRAMES_TO_DISPLAY;
}


// refframes of regular ping interval -----------------------------------------
//
#define PING_WAIT_MAX 	(5 * 600)
static refframe_t server_ping_interval = 0;


// maintain network game play -------------------------------------------------
//
void NETs_MaintainNet()
{
	CHECKMEMINTEGRITY();

	// make NumRemPlayers accessible from console
	AUXDATA_NUMREMPLAYERS_COPY = NumRemPlayers;

	// exit if not connected
	if ( !NetConnected ) {

		// should not stick
		if ( CurJumpServerNode != NULL ) {
			FREEMEM( CurJumpServerNode );
			CurJumpServerNode = NULL;
		}

		// process eventually received masterserver packets
		if ( serverlist_info.m_ListRequested ) {
			NETs_ProcessPacketChain( NET_ProcPacketLoop_Disconnected_GMSV );
		}

		return;
	}

	// perform server jump if scheduled
	ExecuteScheduledServerJump();

	// count down timeout and check disconnect server connection on timeout
	ServerTimeoutFrames -= CurScreenRefFrames;
	if ( ServerTimeoutFrames <= 0 ) {
		NET_DisconnectNoConnection();
		return;
	}

	// send packet if necessary
	if ( CurPacketRefFrames >= Packet_Send_Frametime ) {

		// advance base for frame measurement
		PacketFrameBase += CurPacketRefFrames;

		// perform regular packet sending (send remote event list)
		SendGameState();
		
		// reset remote event list
		NET_RmEvListReset();
	}
	
	// reset up-to-date flags
	PlayersNotUptodate();
	
	// process all received packets
	NETs_ProcessPacketChain( NET_ProcPacketLoop_GMSV );
	
	//NOTE:
	// a PKTP_COMMAND packet might have performed a disconnect, so
	// in this case we must not continue with this function !
	if ( !NetConnected ) {
		return;
	}
	
	// interpolate remote player actions
	InterpolatePlayers();
	
	// fire active duration weapons of remote ships
	FireDurationWeapons();
	
	// send regular server ping packet
	if ( AUX_DRAW_SERVER_PING ) {
		if ( ( server_ping_interval += CurScreenRefFrames ) > PING_WAIT_MAX ) {
			
			server_ping_interval = 0;
			NET_PingCurrentServer();
		}
	}
}


// change packet send frequency -----------------------------------------------
//
PRIVATE
int Cmd_CLIENTTRATE( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// clientrate_command ::= 'net.clientrate' [<frequency>]

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	int32 frequency;
	int syntaxok = GetIntBehindCommand( paramstr, &frequency, 10, -1 );
	if ( syntaxok ) {
		if ( syntaxok == -1 ) {
			sprintf( paste_str, "current clientrate: %d Hz", Packet_Send_Frequency );
			CON_AddLine( paste_str );
		} else if ( frequency < CLIENT_SEND_FREQUENCY_MIN || frequency > CLIENT_SEND_FREQUENCY_MAX ) {
			CON_AddLine( range_error );
		} else {
			Packet_Send_Frequency = frequency;
			Packet_Send_Frametime = FRAME_MEASURE_TIMEBASE / Packet_Send_Frequency;

			if ( ( NetConnected == NETWORK_GAME_ON ) && NET_ProtocolGMSV() ) {
				// insert a remote event with the new clientinfo
				NET_RmEvClientInfo( Packet_Send_Frequency, Packet_Recv_Rate );
			}

			DBGTXT( MSGOUT( "NET_GMSV::Cmd_CLIENTTRATE(): set send frequency to %d.", frequency ); );
		}
	}

	return TRUE;
}

// change packet recv rate ----------------------------------------------------
//
PRIVATE
int Cmd_SERVERRATE( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// serverrate_command ::= 'net.serverrate' [<frequency>]

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	// command invalid in PEER mode
	if ( NET_ProtocolPEER() ) {
		CON_AddLine( invalid_in_peermode );
		return TRUE;
	}

	int32 rate;
	int syntaxok = GetIntBehindCommand( paramstr, &rate, 10, -1 );
	if ( syntaxok ) {
		if ( syntaxok == -1 ) {
			sprintf( paste_str, "current serverrate: %d byte/sec", Packet_Recv_Rate );
			CON_AddLine( paste_str );
		} else if ( rate < CLIENT_RECV_RATE_MIN || rate > CLIENT_RECV_RATE_MAX ) {
			CON_AddLine( range_error );
		} else {
			Packet_Recv_Rate = rate;

			if ( NetConnected == NETWORK_GAME_ON ) {
				// insert a remote event with the new clientinfo
				NET_RmEvClientInfo( Packet_Send_Frequency, Packet_Recv_Rate );
			}

			DBGTXT( MSGOUT( "NET_GMSV::Cmd_CLIENTTRATE(): set send rate to %d.", rate ); );
		}
	}

	return TRUE;
}

// jump to a new server -------------------------------------------------------
//
PRIVATE
int Cmd_JUMP( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// jump_command		::= 'jump' <server>
	// server			::= valid hostname or IP address of gameserver

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	// command invalid in PEER mode
	if ( NET_ProtocolPEER() ) {
		CON_AddLine( invalid_in_peermode );
		return TRUE;
	}

        if ( !NetConnected ) {
	        CON_AddLine( "you must be connected in order to jump." );
                return TRUE;
	}

	// might happen due to delayed jumps
	if ( CurJumpServerNode != NULL ) {
		CON_AddLine( "jump already in progress." );
		return TRUE;
	}

	// either name of host or ip address
	const char *gs_hostname = GetStringBehindCommand( paramstr, FALSE );
	if ( gs_hostname == NULL ) {
		return TRUE;
	}

	// ensure messages work correctly
	CON_AddLine( con_prompt );
	CON_DisableLineFeed();
		
	// store temporary server name globally
	CurJumpServerNode = (node_t *) ALLOCMEM( sizeof( node_t ) );
	if ( CurJumpServerNode == NULL )
		OUTOFMEM( 0 );


	// convert from hostname to node_t
	//FIXME: we only support the default gameserver port for the jump command
	NET_ResolveHostName( gs_hostname, NULL, CurJumpServerNode );

	// perform jump (frees server name!)
        JumpToCurJumpServer();

	return TRUE;
}

// key table for net.master command -------------------------------------------
//
key_value_s net_master_key_value[] = {

	{ "num",	NULL,	KEYVALFLAG_NONE			},
	{ "addr",	NULL,	KEYVALFLAG_NONE			},

	{ NULL,		NULL,	KEYVALFLAG_NONE			},
};

enum {

	KEY_NET_MASTER_NUM,
	KEY_MET_MASTER_ADDR
};


// specify masterservers to use -----------------------------------------------
//
PRIVATE
int Cmd_NET_MASTER( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// net_master_command	::= 'net.master' <num_spec> [<addr_spec>]
	// num_spec				::= 'num' <int>
	// addr_spec			::= 'addr' valid hostname or IP address of masterserver

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	// command invalid in PEER mode
	if ( NET_ProtocolPEER() ) {
		CON_AddLine( invalid_in_peermode );
		return TRUE;
	}

	// scan out all values to keys
	if ( !ScanKeyValuePairs( net_master_key_value, paramstr ) )
		return TRUE;

	// print all know masterservers
	if ( net_master_key_value[ KEY_NET_MASTER_NUM ].value == NULL ) {
		for( int nMaster = 0; nMaster < MAX_MASTERSERVERS; nMaster++ ) {
			if ( Masters[ nMaster ] != NULL ) {
				sprintf( paste_str, "masterserver #%d: %s", nMaster + 1, Masters[ nMaster ] );
			} else {
				sprintf( paste_str, "masterserver #%d: not specified", nMaster + 1 );
			}
			CON_AddLine( paste_str );
			return TRUE;
		}
	}

	// get masterserver id to retrieve/change
	int num = 0;
	if ( ScanKeyValueInt( &net_master_key_value[ KEY_NET_MASTER_NUM ], &num ) < 0 ) {
		CON_AddLine( invalid_arg );
		return TRUE;
	}
	if ( num < 1 || num > MAX_MASTERSERVERS ) {
		CON_AddLine( range_error );
		return TRUE;
	}

	// zero based
	num--;

	// if masterserver is omitted, show current masterserver in this slot
	char *master = net_master_key_value[ KEY_MET_MASTER_ADDR ].value;
	if ( master == NULL ) {

		if ( Masters[ num ] != NULL ) {
			sprintf( paste_str, "masterserver #%d: %s", num + 1, Masters[ num ] );
		} else {
			sprintf( paste_str, "masterserver #%d: not specified", num + 1 );
		}
		CON_AddLine( paste_str );
		return TRUE;
	}

	// otherwise store new masterserver addr in slot
	if ( Masters[ num ] != NULL )
		FREEMEM( Masters[ num ] );

	Masters[ num ] = (char *) ALLOCMEM( strlen( master ) + 1 );
	if ( Masters[ num ] == NULL ) {
		OUTOFMEM( 0 );
	}
	strcpy( Masters[ num ], master );

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( NET_GMSV )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "net.clientrate" command
	regcom.command	 = "net.clientrate";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_CLIENTTRATE;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "net.serverrate" command
	regcom.command	 = "net.serverrate";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_SERVERRATE;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "jump" command
	regcom.command	 = "net.jump";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_JUMP;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );
    
    // register "net.master" command
	regcom.command	 = "net.master";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_NET_MASTER;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	Masters[ 0 ] = (char *) ALLOCMEM( strlen( default_master ) + 1 );
	if ( Masters[ 0 ] == NULL ) {
		OUTOFMEM( 0 );
	}
	strcpy( Masters[ 0 ], default_master );

}



