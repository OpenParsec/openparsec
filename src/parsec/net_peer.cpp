/*
 * PARSEC - Peer-to-Peer Protocol
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:39 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-1999
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
#include "od_class.h"

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "net_defs.h"
#include "sys_defs.h"

// network code config
#include "net_conf.h"

// local module header
#include "net_peer.h"

// proprietary module headers
#include "con_aux.h"
#include "con_ext.h"
#include "e_demo.h"
#include "e_record.h"
#include "g_supp.h"
#include "net_conn.h"
#include "net_game.h"
#include "net_game_peer.h"
#include "net_rmev.h"
#include "net_rmev_peer.h"
#include "obj_creg.h"
#include "obj_ctrl.h"
#include "g_sfx.h"
#include "sys_bind.h"


// connect timing

#define TIMEOUT_CONNECT			10		// number of connect tries
#define RETRYWAIT_CONNECT		200		// time to wait until next try


// slot request timing

#define TIMEOUT_SLOTREQUEST		20		// number of slot-request tries
#define RETRYWAIT_SLOTREQUEST	400		// time to wait until next try


// quantum for master timeoffset

#define TIMEOFFSET_QUANTUM		100		// time until next node acts as master


// connect-loop exit codes

#define EXIT_TO_CONNECT_LOOP	0		// stay in connect-loop until timeout
#define EXIT_TO_GAME_LOOP		1		// master found -> protocol done
#define EXIT_TO_SLOTREQ_LOOP	2		// host is slave -> request slot
#define EXIT_TO_CONNECT_REFUSED	3		// connection refused



// address of temporary master
node_t master_node;

// exit code of connect-loop
int connect_loop_exit;


// buffer that can be used to prepare NetPacket for sending --------------------
//
static char				GamePacketBuffer[ NET_MAX_NETPACKET_INTERNAL_LEN ];
static NetPacket_PEER*	GamePacket = (NetPacket_PEER*) GamePacketBuffer;


// identification of slave for DetermineSlave() -------------------------------
//
#define SLAVE_LOCALNODE		0
#define SLAVE_REMOTENODE	1


// determine node's slave property depending on ipx address -------------------
//
PRIVATE
int DetermineSlave( node_t *localplayer, node_t *remoteplayer )
{
	ASSERT( localplayer != NULL );
	ASSERT( remoteplayer != NULL );

	//NOTE:
	// this function determines for two nodes who is
	// a slave in any case. this does not necessarily
	// mean the other node is master; this is indeed
	// not the case if there is another node with still
	// higher priority. then both nodes will be slaves.

	// compare node addresses
	int cmpcode = NETs_CompareNodes( localplayer, remoteplayer );

	// enslave player with lower ethernet node address
	if ( cmpcode == NODECMP_LESSTHAN )
		return SLAVE_LOCALNODE;
	if ( cmpcode == NODECMP_GREATERTHAN )
		return SLAVE_REMOTENODE;

	MSGOUT( "two identical node addresses encountered." );
	ASSERT(0);

	return SLAVE_REMOTENODE;
}


// create player in already allocated slot ------------------------------------
//
PRIVATE
void CreateRemotePlayer( RE_PlayerList *re_playerlist, int id )
{
	ASSERT( re_playerlist != NULL );
	ASSERT( re_playerlist->RE_Type == RE_PLAYERLIST );
	ASSERT( ( id >= 0 ) && ( id < CurMaxPlayers ) );

	int listnum = id >> 2;

	// skip empty slots
	if ( re_playerlist[ listnum ].Status[ id % 4 ] == PLAYER_INACTIVE ) {
		return;
	}

	// prevent creation of local player
	if ( id == LocalPlayerId ) {
		return;
	}

	// make function idempotent by creating each player only once
	if ( Player_Status[ id ] != PLAYER_INACTIVE ) {
		DBGTXT( MSGOUT( "preempting redundant player creation: %d.", id ); );
		return;
	}

	// store status and init alive counter
	Player_Status[ id ]		  = re_playerlist[ listnum ].Status[ id % 4 ];
	Player_AliveCounter[ id ] = MAX_ALIVE_COUNTER;

	// reset up-to-date flag and killstats for newly assigned slot
	Player_UpToDate[ id ] = FALSE;
	Player_KillStat[ id ] = 0;

	// reset msg ids for newly assigned slot
	Player_LastMsgId[ id ] = 0;
	Player_LastUpdateGameStateMsgId[ id ] = 0;

	// store address and name
	NETs_ResolveNode( &Player_Node[ id ], &re_playerlist[ listnum ].AddressTable[ id % 4 ] );
	CopyRemoteName( Player_Name[ id ], re_playerlist[ listnum ].NameTable[ id % 4 ] );

	// if player is joined store ship info and create ship
	if ( Player_Status[ id ] == PLAYER_JOINED ) {

		// try to resolve ship index
		dword shipindex = re_playerlist[ listnum ].ShipInfoTable[ id % 4 ].ShipIndex;
		dword shipclass = CLASS_ID_INVALID;
		if ( shipindex < (dword)NumShipClasses ) {
			shipclass = ShipClasses[ shipindex ];
		}

		// take default ship if the one remotely sent not available
		if ( shipclass == CLASS_ID_INVALID ) {
			shipclass = SHIP_CLASS_1;
		}

		pXmatrx objposition = re_playerlist[ listnum ].ShipInfoTable[ id % 4 ].ObjPosition;
		GenObject *objectpo = CreateObject( shipclass, objposition );
		ASSERT( objectpo != NULL );
		objectpo->HostObjNumber = ShipHostObjId( id );
		Player_Ship[ id ]   = objectpo;
		Player_ShipId[ id ] = objectpo->ObjectNumber;
	}


	DBGTXT( MSGOUT( "created player: id %d, status %d.", id, Player_Status[ id ] ); );
	ADXTXT( NETs_PrintNode( &Player_Node[ id ] ); );
}


// broadcast connect request --------------------------------------------------
//
PRIVATE
void ConnectBroadcast()
{
	node_t node;
	NETs_SetBroadcastAddress( &node );

	// fill game data header
	NETs_StdGameHeader( PKTP_CONNECT, (NetPacket*)GamePacket );
	ASSERT( LocalPlayerId == PLAYERID_ANONYMOUS );

	// connect packets should always be sent anonymous
	GamePacket->SendPlayerId = PLAYERID_ANONYMOUS;

	// create single remote event: slot request queue
	NET_RmEvSingleConnectQueue( GamePacket );

	DBGTXT( MSGOUT( "broadcasting connect for universe %d (pid=%d).", MyUniverse, GamePacket->MessageId ); );
	NETs_AuxSendPacket( (NetPacket*)GamePacket, &node );
}


// store info about real destination into snoop packet ------------------------
//
INLINE
void StoreRealDestination( NetPacket_PEER* gamepacket, int qpos, int snooper )
{
	ASSERT( gamepacket != NULL );
	ASSERT( ( qpos >= 0 ) && ( qpos < NumSlotRequests ) );
	ASSERT( ( snooper >= 0 ) && ( snooper < CurMaxPlayers ) );

	RE_PlayerList *re_playerlist = (RE_PlayerList *) &gamepacket->RE_List;

	ASSERT(	re_playerlist->RE_Type == RE_PLAYERLIST );

	//NOTE:
	// the address and name of the original player are
	// stored into the entries for the snooping player.
	// they are the only ones where it is certain that
	// the snooper doesn't need them. if a player is
	// refused a connection, there is no other place
	// where his node address could be stored in order
	// to be transmitted to the snooper.

	CopyRemoteNode( re_playerlist[ snooper >> 2 ].AddressTable[ snooper % 4 ],
					SlotReqQueue[ qpos ].node );
	CopyRemoteName( re_playerlist[ snooper >> 2 ].NameTable[ snooper % 4 ],
					SlotReqQueue[ qpos ].name );
}


// send connect reply to remote player (may be negative) ----------------------
//
PRIVATE
void SendConnectReply( int qpos, int slotid )
{
	ASSERT( ( qpos >= 0 ) && ( qpos < NumSlotRequests ) );
	ASSERT( slotid < CurMaxPlayers );

	// filter out special slot requests
	if ( slotid < SLOTID_CONNECT_REFUSED )
		return;

	DBGTXT( MSGOUT( "sending connect reply to: %s (id %d).", SlotReqQueue[ qpos ].name, slotid ); );
	DBGTXT( NETs_PrintNode( &SlotReqQueue[ qpos ].node ); );

	// store id remote player got assigned
	GamePacket->DestPlayerId = slotid;

	// send connect reply to new remote player
	NETs_AuxSendPacket( (NetPacket*)GamePacket, &SlotReqQueue[ qpos ].node );

	// send also to all other remote players
	// so that they can snoop the reply.
	for ( int id = 0; id < CurMaxPlayers; id++ ) {
		if ( ( id != slotid ) && REMOTE_PLAYER_ACTIVE( id ) ) {

			DBGTXT( MSGOUT( "sending snoop-cc to id %d.", id ); );

			//NOTE:
			// the DestPlayerId field in the packet is not
			// changed here. that is, it contains always
			// the value of the original destination.
			// this denotes snooped packets and allows for
			// easy determination of the actual receiver id.

			StoreRealDestination( GamePacket, qpos, id );
			NETs_AuxSendPacket( (NetPacket*)GamePacket, &Player_Node[ id ] );
		}
	}
}


// send connect reply to all slot request queue members -----------------------
//
PRIVATE
void ConnectQueue()
{
	//NOTE:
	// connect replies are sent to all queue members.
	// to ensure that packet-drop has no significant
	// impact on the behavior, queue entries are not
	// deleted as long as it has not been verified
	// that the connect reply has been received by
	// the remote player. that is, connect replies
	// may be sent more than once to the same player.

	int dosend[ MAX_SLOT_REQUESTS ] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
	ASSERT( NumSlotRequests <= MAX_SLOT_REQUESTS );

	// process all queued requests (try to allocate
	// slots and schedule replies if necessary)
	int qpos = 0;
	for ( qpos = 0; qpos < NumSlotRequests; qpos++ ) {

		// assume no reply necessary
		dosend[ qpos ] = FALSE;

		int slotid = SlotReqQueue[ qpos ].slotid;

		// skip special slots
		if ( ( slotid == SLOTID_DELETED ) || ( slotid == SLOTID_LOCALHOST ) )
			continue;

		//NOTE:
		// the timetag is used to determine the time when
		// a reply should be sent by the local host. this
		// is done for temporary master election purposes.
		// the timetag may also be used to contain additional
		// flags when the actual time is not needed.

		int timetag = SlotReqQueue[ qpos ].timetag;
		ASSERT( timetag != TIMETAG_LOCALHOST );

		// check timetag
		refframe_t curtime = SYSs_GetRefFrameCount();
		if ( ( timetag != TIMETAG_REPLYNOW ) && ( curtime < timetag ) ) {
			DBGTXT( MSGOUT( "refraining from acting as master (id %d).", slotid ); );
			continue;
		}

		// only register remote players for which
		// this has not already been done
		if ( slotid == SLOTID_NOT_ASSIGNED ) {

			node_t node;
			NETs_ResolveNode( &node, &SlotReqQueue[ qpos ].node );
			char *name = SlotReqQueue[ qpos ].name;

			// try to register remote player
			slotid = NET_AcquireRemoteSlot( &node );
			NET_RegisterRemotePlayer( slotid, &node, name );
			SlotReqQueue[ qpos ].slotid = slotid;

			DBGTXT( MSGOUT( "registered %s with id %d.", name, slotid ); );

		} else {

			DBGTXT( MSGOUT( "skipping registration (id %d).", slotid ); );
		}

		// schedule reply
		dosend[ qpos ] = TRUE;
	}

	// avoid increasing messageid if nothing will be sent
	int pending = FALSE;
	for ( qpos = 0; qpos < NumSlotRequests; qpos++ ) {
		if ( dosend[ qpos ] ) {
			pending = TRUE;
		}
	}
	if ( !pending ) {
		return;
	}

	// fill game data header
	NETs_StdGameHeader( PKTP_CONNECT_REPLY, (NetPacket*)GamePacket );

	// create single remote event: player table
	NET_RmEvSinglePlayerTable( GamePacket );

	// send replies to scheduled nodes
	for ( qpos = 0; qpos < NumSlotRequests; qpos++ ) {
		if ( dosend[ qpos ] ) {
			SendConnectReply( qpos, SlotReqQueue[ qpos ].slotid );
		}
	}
}


// sent slot request to temporary master --------------------------------------
//
PRIVATE
void SendSlotRequest( node_t *node )
{
	ASSERT( node != NULL );

	DBGTXT( MSGOUT( "sending slot_request to host:" ); );
	DBGTXT( NETs_PrintNode( node ); );

	// fill game data header
	NETs_StdGameHeader( PKTP_SLOT_REQUEST, (NetPacket*)GamePacket );
	ASSERT( LocalPlayerId == PLAYERID_ANONYMOUS );

	// create single remote event: slot request queue
	NET_RmEvSingleConnectQueue( GamePacket );

	NETs_AuxSendPacket( (NetPacket*)GamePacket, node );
}


// subdue slave ---------------------------------------------------------------
//
PRIVATE
void SendSubdueCommand( node_t *node )
{
	ASSERT( node != NULL );

	DBGTXT( MSGOUT( "subduing slave:" ); );
	DBGTXT( NETs_PrintNode( node ); );

	// fill game data header
	NETs_StdGameHeader( PKTP_SUBDUE_SLAVE, (NetPacket*)GamePacket );
	ASSERT( LocalPlayerId == PLAYERID_ANONYMOUS );

	NETs_AuxSendPacket( (NetPacket*)GamePacket, node );
}


// process connect request received in connect-loop ---------------------------
//
PRIVATE
void ProcessConnect( NetPacket_PEER* gamepacket, int bufid )
{
	ASSERT( gamepacket != NULL );
	ASSERT( gamepacket->Command == PKTP_CONNECT );

	node_t *node = NETs_GetSender( bufid );

	DBGTXT( MSGOUT( "processing connect request:" ); );
	DBGTXT( NETs_PrintNode( node ); );

	if ( DetermineSlave( &LocalNode, node ) == SLAVE_REMOTENODE ) {

		// local host is now temporary master

		NET_MergeSlotRequests( gamepacket, TIMETAG_REPLYNOW );
		SendSubdueCommand( node );

	} else {

		// local host is now slave

		// remember address of sender (who is temporary master for local host)
		CopyRemoteNode( master_node, *node );

		// exit connect-loop to slotreq-loop
		connect_loop_exit = EXIT_TO_SLOTREQ_LOOP;
	}
}


// process reply to connect request -------------------------------------------
//
PRIVATE
int ProcessConnectReply( NetPacket_PEER* gamepacket, int bufid )
{
	//NOTE:
	// this function is called to process a PKTP_CONNECT_REPLY
	// received in either the connect-loop or the slotreq-loop.

	ASSERT( gamepacket != NULL );
	ASSERT( gamepacket->Command == PKTP_CONNECT_REPLY );

	// retrieve sender's id
	int senderid = gamepacket->SendPlayerId;
	ASSERT( ( senderid >= 0 ) && ( senderid < CurMaxPlayers ) );

	// filter duplicates
	if ( Player_Status[ senderid ] != PLAYER_INACTIVE ) {
		DBGTXT( MSGOUT( "got redundant connect reply from id %d (startup).", senderid ); );
		return TRUE;
	}

	DBGTXT( MSGOUT( "processing connect reply from id %d (pid=%d).", senderid, gamepacket->MessageId ); );
	ADXTXT( NETs_PrintNode( NETs_GetSender( bufid ) ); );

	// fetch player id and number of remote players (local player included)
	int playerid  = gamepacket->DestPlayerId;
	NumRemPlayers = gamepacket->NumPlayers;
	ASSERT( NumRemPlayers > 1 );

	// check whether connect accepted (slot available)
	if ( playerid != SLOTID_CONNECT_REFUSED ) {

		// set local id
		LocalPlayerId = gamepacket->DestPlayerId;

		DBGTXT( MSGOUT( "connection accepted: got id %d.", LocalPlayerId ); );

		RE_PlayerList *re_playerlist = (RE_PlayerList *) &gamepacket->RE_List;

		ASSERT( re_playerlist->RE_Type == RE_PLAYERLIST );

		// sync critical values
		dword killlimit	= re_playerlist->SyncValKillLimit;
		dword nebulaid	= re_playerlist->SyncValNebulaId;

		if ( ( nebulaid >= 2 ) && ( nebulaid <= 5 ) ) {
			AUX_KILL_LIMIT_FOR_GAME_END  = killlimit;
			AUXDATA_BACKGROUND_NEBULA_ID = nebulaid;
		}

		// process player table
		for ( int id = 0; id < CurMaxPlayers; id++ ) {
			re_playerlist = (RE_PlayerList *) &gamepacket->RE_List;
			CreateRemotePlayer( re_playerlist, id );
		}

	} else {

		DBGTXT( MSGOUT( "connection refused." ); );

		connect_loop_exit = EXIT_TO_CONNECT_REFUSED;
		return FALSE;
	}

	// exit connect-loop to game-loop (this also
	// causes slotreq-loop to exit correctly)
	connect_loop_exit = EXIT_TO_GAME_LOOP;

	return TRUE;
}


// process slot request received in either connect- or slotreq-loop -----------
//
PRIVATE
void ProcessSlotRequest( NetPacket_PEER* gamepacket, int bufid )
{
	ASSERT( gamepacket != NULL );
	ASSERT( gamepacket->Command == PKTP_SLOT_REQUEST );

	DBGTXT( MSGOUT( "got slot_request:" ); );
	DBGTXT( NETs_PrintNode( NETs_GetSender( bufid ) ); );

	// merge slot request queue of remote host with local queue
	NET_MergeSlotRequests( gamepacket, TIMETAG_REPLYNOW );
}


// process subdue slave received in connect-loop ------------------------------
//
PRIVATE
void ProcessSubdueSlave( NetPacket_PEER* gamepacket, int bufid )
{
	ASSERT( gamepacket != NULL );
	ASSERT( gamepacket->Command == PKTP_SUBDUE_SLAVE );

	node_t *node = NETs_GetSender( bufid );

	DBGTXT( MSGOUT( "got subdue_slave:" ); );
	DBGTXT( NETs_PrintNode( node ); );

	// remember address of sender (who is temporary master)
	CopyRemoteNode( master_node, *node );

	// exit connect-loop to slotreq-loop
	connect_loop_exit = EXIT_TO_SLOTREQ_LOOP;
}


// process the content of a packet received in connect-loop -------------------
//
PRIVATE
void ProcPacketConnectLoop( NetPacket* pIntPkt, int bufid )
{
	ASSERT( pIntPkt != NULL );

	NetPacket_PEER* gamepacket = (NetPacket_PEER*)pIntPkt;

	// perform action according to packet type
	switch( gamepacket->Command ) {

		case PKTP_CONNECT:
			ProcessConnect( gamepacket, bufid );
			break;

		case PKTP_CONNECT_REPLY:
			ProcessConnectReply( gamepacket, bufid );
			break;

//		case PKTP_DISCONNECT:

		case PKTP_SLOT_REQUEST:
			ProcessSlotRequest( gamepacket, bufid );
			break;

		case PKTP_SUBDUE_SLAVE:
			ProcessSubdueSlave( gamepacket, bufid );
			break;

//		case PKTP_JOIN:
//		case PKTP_UNJOIN:

		case PKTP_GAME_STATE:
		case PKTP_NODE_ALIVE:
			// these packets may be received here if a
			// remote node has already accepted us but
			// we have not exited the connect-loop yet.
			break;

//		case PKTP_PING:

		default:
			MSGOUT( "connectloop: received packet of invalid type: %d.", gamepacket->Command );
			//FIXME: check which packet type causes this
			//ASSERT( 0 );
	}
}


// process the content of a packet received in slotreq-loop -------------------
//
PRIVATE
void ProcPacketSlotReqLoop( NetPacket* pIntPkt, int bufid )
{
	ASSERT( pIntPkt != NULL );

	NetPacket_PEER* gamepacket = (NetPacket_PEER*)pIntPkt;

	// perform action according to packet type
	switch( gamepacket->Command ) {

		case PKTP_CONNECT:
			// filter this packet type
			DBGTXT( MSGOUT( "slotreqloop: filtering connect_request." ); );
			break;

		case PKTP_CONNECT_REPLY:
			ProcessConnectReply( gamepacket, bufid );
			break;

//		case PKTP_DISCONNECT:

		case PKTP_SLOT_REQUEST:
			ProcessSlotRequest( gamepacket, bufid );
			break;

		case PKTP_SUBDUE_SLAVE:
			// filter this packet type
			DBGTXT( MSGOUT( "slotreqloop: filtering subdue_slave." ); );
			break;

//		case PKTP_JOIN:
//		case PKTP_UNJOIN:

		case PKTP_GAME_STATE:
		case PKTP_NODE_ALIVE:
			// these packets may be received here if a
			// remote node has already accepted us but
			// we have not exited the slotreq-loop yet.
			break;

//		case PKTP_PING:

		default:
			MSGOUT( "slotreqloop: received packet of invalid type: %d.", gamepacket->Command );
			ASSERT( 0 );
	}
}


// avoid nameclashes in debug version if multiple protocols linked ------------
//
#define ProcPacketGameLoop		PEER_ProcPacketGameLoop
#define SendGameState			PEER_SendGameState
#define SendNodeAlive			PEER_SendNodeAlive
#define PlayersNotUptodate		PEER_PlayersNotUptodate
#define InterpolatePlayers		PEER_InterpolatePlayers
#define CheckAliveStatus		PEER_CheckAliveStatus
#define FireDurationWeapons		PEER_FireDurationWeapons


// try to connect to peers (establish temporary master and connection) --------
//
int NETs_Connect()
{
	ASSERT( !NetConnected && !NetJoined );

	// turn off game time
	CurGameTime = GAME_PEERTOPEER;

	MSGOUT( "max players: %d", CurMaxPlayers );

	// (re)init tables
	NET_InitRemotePlayerTables();

	MSGOUT( "trying to establish connection..." );
	MSGPUT( "searching for other players in universe %d.", MyUniverse );

	// discard old packets
	NETs_FlushListenBuffers();

	// add slot request for local host
	NET_AddSlotRequest( &LocalNode, LocalPlayerName, TIMETAG_LOCALHOST );

	// connect-loop
	int timeout       = TIMEOUT_CONNECT;
	connect_loop_exit = EXIT_TO_CONNECT_LOOP;
	while ( ( connect_loop_exit == EXIT_TO_CONNECT_LOOP ) && ( timeout > 0 ) ) {

		if ( TextModeActive )
			MSGPUT( "." );

		// send request for connection and wait a bit
		ConnectBroadcast();
		SYSs_Wait( RETRYWAIT_CONNECT );

		// process any received packets
		NETs_ProcessPacketChain( ProcPacketConnectLoop );

		timeout--;
	}

	if ( TextModeActive )
		MSGOUT( "\n" );

	// check if slot request should be sent (local host is slave)
	if ( connect_loop_exit == EXIT_TO_SLOTREQ_LOOP ) {

		DBGTXT( MSGOUT( "other players found: this host is slave." ); );

		// slotreq-loop
		timeout = TIMEOUT_SLOTREQUEST;
		while ( ( connect_loop_exit == EXIT_TO_SLOTREQ_LOOP ) && ( timeout > 0 ) ) {

			if ( TextModeActive )
				MSGPUT( "." );

			// send slot request and wait a bit
			SendSlotRequest( &master_node );
			SYSs_Wait( RETRYWAIT_SLOTREQUEST );

			// process any received packets
			NETs_ProcessPacketChain( ProcPacketSlotReqLoop );

			timeout--;
		}

		if ( TextModeActive )
			MSGOUT( "\n" );

		if ( ( connect_loop_exit == EXIT_TO_SLOTREQ_LOOP ) && ( timeout == 0 ) ) {

			MSGOUT( "lost connection to temporary master." );
			NET_ResetSlotReqQueue();
			return FALSE;
		}
	}

	// check if connection has been refused
	if ( connect_loop_exit == EXIT_TO_CONNECT_REFUSED ) {
		NET_ResetSlotReqQueue();
		return FALSE;
	}

	// determine whether local host is master
	int IsMaster = ( ( connect_loop_exit == EXIT_TO_CONNECT_LOOP ) && ( timeout == 0 ) );

	// if this client is the master, we must set the playerid to 0 
	if ( IsMaster ) {
		ASSERT( LocalPlayerId == PLAYERID_ANONYMOUS );
		LocalPlayerId = 0;
	}

	// add local player to remote players 
	NET_InitLocalPlayer();

	if ( IsMaster ) {

		MSGOUT( "connection established: local host is master." );
		DBGTXT( MSGOUT( "sending connect reply to slaves." ); );

		// remove local host from queue
		NET_DelSlotRequest( &LocalNode );

		// count local player
		ASSERT( NumRemPlayers == 0 );
		NumRemPlayers = 1;

		// send replies to queued slaves
		ConnectQueue();

	} else {

		MSGOUT( "connection established: master found." );

		// delete slot requests
		NET_ResetSlotReqQueue();
	}

	//NOTE:
	// for implementation efficiency and simplicity the
	// local player is added to the table of remote players.
	// to determine if a remote player is actually local
	// (e.g., to prevent sending a packet to oneself)
	// the shipid can be used, which is set to SHIPID_LOCALPLAYER.
	// the REMOTE_PLAYER_ACTIVE() macro determines if a player is
	// both active and actually remote.

	// list all remote players currently connected
	int otherplayers = 0;
	for ( int id = 0; id < CurMaxPlayers; id++ )
		if ( REMOTE_PLAYER_ACTIVE( id ) )
			otherplayers++;
	if ( otherplayers > 0 ) {
		MSGOUT( "the following remote players are in universe %d:", MyUniverse );
		for ( int id = 0; id < CurMaxPlayers; id++ )
			if ( REMOTE_PLAYER_ACTIVE( id ) )
				MSGOUT( "id %d: %s", id, Player_Name[ id ] );
	} else {
		MSGOUT( "there are no other players in universe %d.", MyUniverse );
	}

	// set global flag
	NetConnected = NETWORK_GAME_ON;

	return TRUE;
}


// notify remote players of disconnect (exit entry-mode) ----------------------
//
int NETs_Disconnect()
{
	ASSERT( ( NetConnected == NETWORK_GAME_ON ) && !NetJoined );
	ASSERT( Player_Status[ LocalPlayerId ] == PLAYER_CONNECTED );

	// send disconnect message to all active remote players
	for ( int id = 0; id < CurMaxPlayers; id++ ) {
		if ( REMOTE_PLAYER_ACTIVE( id ) ) {

			DBGTXT( MSGOUT( "sending disconnect message to id %d.", id ); );
			ADXTXT( NETs_PrintNode( &Player_Node[ id ] ); );

			// fill game data header
			NETs_StdGameHeader( PKTP_DISCONNECT, (NetPacket*)GamePacket );
			GamePacket->DestPlayerId = id;

			NETs_AuxSendPacket( (NetPacket*)GamePacket, &Player_Node[ id ] );
		}
	}

	// delete slot requests
	NET_ResetSlotReqQueue();

	// remove all remote players
	NET_RemoveRemotePlayers();

	// remove local player
	ASSERT( NumRemPlayers == 1 );
	NumRemPlayers = 0;

	// update player status
	Player_Status[ LocalPlayerId ] = PLAYER_INACTIVE;

	// reset global flag
	NetConnected = NETWORK_GAME_OFF;

	return TRUE;
}


// join game (enter game from entry mode in already allocated slot) -----------
//
int NETs_Join()
{
	ASSERT( NetConnected && !NetJoined );
	ASSERT( Player_Status[ LocalPlayerId ] == PLAYER_CONNECTED );

	// update player status
	Player_Status[ LocalPlayerId ] = PLAYER_JOINED;

	// set global flag
	NetJoined = TRUE;

	// send join message to all active remote players
	for ( int id = 0; id < CurMaxPlayers; id++ )
		if ( REMOTE_PLAYER_ACTIVE( id ) ) {

			DBGTXT( MSGOUT( "sending join message to id %d.", id ); );
			ADXTXT( NETs_PrintNode( &Player_Node[ id ] ); );

			// fill game data header
			NETs_StdGameHeader( PKTP_JOIN, (NetPacket*)GamePacket );
			GamePacket->DestPlayerId	= id;
			GamePacket->params[ 3 ]		= MyShip->ObjectClass;

			GamePacket->ShipInfo.CurSpeed = MyShip->CurSpeed;
			memcpy( &GamePacket->ShipInfo.ObjPosition, MyShip->ObjPosition, sizeof( Xmatrx ) );

			// create single remote event: player name
			NET_RmEvSinglePlayerName( (RE_Header*)&GamePacket->RE_List );

			NETs_AuxSendPacket( (NetPacket*)GamePacket, &Player_Node[ id ] );
		}

	//FIXME: [1/27/2002] ??? GAMECODE() ???

	// open stargate
	if ( !AUX_DISABLE_LOCAL_STARGATE ) {
		SFX_CreateStargate( MyShip );
	}

	// record join
	Record_Join();

	return TRUE;
}


// last unjoin flag (allows delayed unjoins to compensate for packet drop) ----
//
static byte last_unjoin_flag = USER_EXIT;


// notify remote players of unjoin (exit to entry-mode) -----------------------
//
int NETs_Unjoin( byte flag )
{
	ASSERT( NetConnected && NetJoined );
	ASSERT( Player_Status[ LocalPlayerId ] == PLAYER_JOINED );

	// update player status
	Player_Status[ LocalPlayerId ] = PLAYER_CONNECTED;

	// reset global flag
	NetJoined = FALSE;

	// remember unjoin flag
	last_unjoin_flag = flag;

	// send unjoin message to all active remote players
	for ( int id = 0; id < CurMaxPlayers; id++ )
		if ( REMOTE_PLAYER_ACTIVE( id ) ) {

			DBGTXT( MSGOUT( "sending unjoin message to id %d.", id ); );
			ADXTXT( NETs_PrintNode( &Player_Node[ id ] ); );

			// fill game data header
			NETs_StdGameHeader( PKTP_UNJOIN, (NetPacket*)GamePacket );
			GamePacket->DestPlayerId	= id;
			GamePacket->params[ 0 ]		= flag;
			NET_SetPacketKillStats( GamePacket );

			// insert id of player who downed us
			if ( flag == SHIP_DOWNED ) {
				ASSERT( ( CurKiller >= KILLERID_UNKNOWN ) && ( CurKiller < MAX_NET_PROTO_PLAYERS + KILLERID_BIAS ) );
				ASSERT( ( CurKiller >= 0 ) && ( CurKiller < 128 ) );
				GamePacket->params[ 2 ] = CurKiller;
			}

			NETs_AuxSendPacket( (NetPacket*)GamePacket, &Player_Node[ id ] );
		}

	//FIXME: [1/27/2002] ??? GAMECODE() ???

	// open stargate
	if ( !AUX_DISABLE_LOCAL_STARGATE && ( flag == USER_EXIT ) )
		SFX_CreateStargate( MyShip );

	// record unjoin
	Record_Unjoin( flag );

	return TRUE;
}


// update player name on server -----------------------------------------------
//
int NETs_UpdateName()
{
	//NOTE:
	// not needed since there is no server.

	return TRUE;
}


// calculate time offset to act as master and reply to connect request --------
//
PRIVATE
int CalcReplyTimeOffset()
{
	// assume local host is first to reply
	int localpos = 0;

	node_t localnode;
	NETs_MakeNodeRaw( &localnode, &Player_Node[ LocalPlayerId ] );

	for ( int id = 0; id < CurMaxPlayers; id++ )
		if ( REMOTE_PLAYER_ACTIVE( id ) ) {

			node_t remotenode;
			NETs_MakeNodeRaw( &remotenode, &Player_Node[ id ] );

			// use node address to establish ordering
			int cmpcode = NETs_CompareNodes( &localnode, &remotenode );

			// nodes with greater address have higher priority
			if ( cmpcode == NODECMP_GREATERTHAN )
				localpos++;
		}

	return ( localpos * TIMEOFFSET_QUANTUM );
}


// schedule reply to connect request received in game-loop --------------------
//
PRIVATE
void ScheduleConnect( NetPacket_PEER* gamepacket, int bufid )
{
	ASSERT( gamepacket != NULL );
	ASSERT( ( gamepacket->Command == PKTP_CONNECT ) ||
			( gamepacket->Command == PKTP_SLOT_REQUEST ) );

	DBGTXT( MSGOUT( "scheduling connect request:" ); );
	DBGTXT( NETs_PrintNode( NETs_GetSender( bufid ) ); );

	// calc timetag
	int timetag = SYSs_GetRefFrameCount() + CalcReplyTimeOffset();

	// merge in remote slot request(s)
	NET_MergeSlotRequests( gamepacket, timetag );
}


// snoop connect reply sent from an acting master to a new remote player ------
//
PRIVATE
void SnoopConnect( NetPacket_PEER* gamepacket, int bufid )
{
	ASSERT( gamepacket != NULL );
	ASSERT( gamepacket->Command == PKTP_CONNECT_REPLY );

	int srcid  = gamepacket->SendPlayerId;
	int destid = gamepacket->DestPlayerId;

	ASSERT( ( srcid >= 0 ) && ( srcid < MAX_NET_PROTO_PLAYERS ) );
	ASSERT( ( destid >= SLOTID_CONNECT_REFUSED ) && ( destid < MAX_NET_PROTO_PLAYERS ) );

	// retrieve contained player list
	RE_PlayerList *re_playerlist = (RE_PlayerList *) &gamepacket->RE_List;

	ASSERT( re_playerlist->RE_Type == RE_PLAYERLIST );

	// remove request from queue
	NET_DelSlotRequest( &re_playerlist[ LocalPlayerId >> 2 ].AddressTable[ LocalPlayerId % 4 ] );

	// filter packets actually addressed to local host
	if ( destid == LocalPlayerId ) {
		DBGTXT( MSGOUT( "got redundant CONNECT_REPLY from id %d.", srcid ); );
		return;
	}

	DBGTXT( MSGOUT( "snooping connect reply from id %d to id %d.", srcid, destid ); );
	DBGTXT( NETs_PrintNode( NETs_GetSender( bufid ) ); );
	DBGTXT( NETs_PrintNode( &re_playerlist[ LocalPlayerId >> 2 ].AddressTable[ LocalPlayerId % 4 ] ); );

	// check for duplicate replies
	if ( Player_Status[ destid ] != PLAYER_INACTIVE ) {
		DBGTXT( MSGOUT( "snooped CONNECT_REPLY was redundant." ); );
		return;
	}

	// check whether connect accepted (slot available)
	if ( destid != SLOTID_CONNECT_REFUSED ) {

		// sync critical values
		dword killlimit	= re_playerlist->SyncValKillLimit;
		dword nebulaid	= re_playerlist->SyncValNebulaId;

		if ( ( nebulaid >= 2 ) && ( nebulaid <= 5 ) ) {
			AUX_KILL_LIMIT_FOR_GAME_END  = killlimit;
			AUXDATA_BACKGROUND_NEBULA_ID = nebulaid;
		}

		CreateRemotePlayer( re_playerlist, destid );

		NumRemPlayers++;
		ASSERT( NumRemPlayers == gamepacket->NumPlayers );

		DBGTXT( MSGOUT( "creating new remote player: %s (id %d).", re_playerlist[ LocalPlayerId >> 2 ].NameTable[ LocalPlayerId % 4 ], destid ); );

	} else {

		DBGTXT( MSGOUT( "snooped connect refusal for player %s.", re_playerlist[ LocalPlayerId >> 2 ].NameTable[ LocalPlayerId % 4 ] ); );
	}
}


// called to remove pending slot request when connection established ----------
//
PRIVATE
void ConnectionEstablished( int bufid )
{
	//NOTE:
	// this function is called when a node alive or
	// game update message is received. at that time
	// it is absolutely certain that the connect reply
	// has been received, so the slot request can be
	// removed from the queue.

	// remove request from queue if contained
	if ( NET_DelSlotRequest( NETs_GetSender( bufid ) ) ) {

		DBGTXT( MSGOUT( "slot request deleted." ); );
		DBGTXT( NETs_PrintNode( NETs_GetSender( bufid ) ); );
	}
}


// process the content of a packet received in game-loop ----------------------
//
PRIVATE
void ProcPacketGameLoop( NetPacket* pIntPkt, int bufid )
{
	ASSERT( pIntPkt != NULL );

	NetPacket_PEER* gamepacket = (NetPacket_PEER*)pIntPkt;

	// perform action according to packet type
	switch( gamepacket->Command ) {

		case PKTP_CONNECT:
			ScheduleConnect( gamepacket, bufid );
			break;

		case PKTP_CONNECT_REPLY:
			SnoopConnect( gamepacket, bufid );
			break;

		case PKTP_DISCONNECT:
			NET_PlayerDisconnected( gamepacket->SendPlayerId );
			break;

		case PKTP_SLOT_REQUEST:
			DBGTXT( MSGOUT( "warning: SLOT_REQUEST received in gameloop." ); );
			ScheduleConnect( gamepacket, bufid );
			break;

		case PKTP_SUBDUE_SLAVE:
			DBGTXT( MSGOUT( "warning: SUBDUE_SLAVE received in gameloop." ); );
			break;

		case PKTP_JOIN:
			{
				// translate from packet to remote event 
				RE_PlayerAndShipStatus pas_status;
				NET_Translate_PKTP_JOIN( gamepacket, &pas_status );

				// perform necessary steps to ensure desired player status ( was NET_PlayerJoined )
				NET_SetDesiredPlayerStatus( &pas_status );
			}
			break;

		case PKTP_UNJOIN:
			{
				// translate from packet to remote event 
				RE_PlayerAndShipStatus pas_status;
				RE_KillStats  killstats;
				NET_Translate_PKTP_UNJOIN( gamepacket, &pas_status, &killstats );

				// do last killstat update
				NETs_UpdateKillStats( &killstats );
				
				// perform necessary steps to ensure desired player status ( was NET_PlayerUnjoined )
				NET_SetDesiredPlayerStatus( &pas_status );
			}
			break;

		case PKTP_GAME_STATE:
			{
				ConnectionEstablished( bufid );

				// translate from packet to remote event 
				RE_PlayerAndShipStatus pas_status;
				RE_KillStats  killstats;
				NET_Translate_PKTP_GAME_STATE( gamepacket, &pas_status, &killstats );

				// perform necessary steps to ensure desired player status ( was NET_EnsureJoined )
				NET_SetDesiredPlayerStatus( &pas_status );

				// update player kill stats
				NETs_UpdateKillStats( &killstats );
				
				// keep player in game, update player ship ( ignore older states ), and process remote event list
				NET_UpdateGameState_PEER( gamepacket );
			}
			break;

		case PKTP_NODE_ALIVE:
			{
				ConnectionEstablished( bufid );

				// translate from packet to remote event 
				RE_PlayerAndShipStatus	pas_status;
				RE_KillStats			killstats;
				NET_Translate_PKTP_NODE_ALIVE( gamepacket, &pas_status, &killstats );

				ASSERT( pas_status.player_status == PLAYER_CONNECTED );

				// perform necessary steps to ensure desired player status ( was NET_EnsureUnjoined )
				NET_SetDesiredPlayerStatus( &pas_status );

				// update player kill stats
				NETs_UpdateKillStats( &killstats );

				// keep player in game and process remote event list ( can only be namechange as player is not joined )
				NET_ProcessNodeAlive_PEER( gamepacket );
			}
			break;

		default:
			MSGOUT( "gameloop: received packet of unknown type: %d.", gamepacket->Command );
			ASSERT( 0 );
	}
}


// update game state of active remote players ---------------------------------
//
PRIVATE
void SendGameState()
{
	// fill game data header
	NETs_StdGameHeader( PKTP_GAME_STATE, (NetPacket*)GamePacket );
	NET_SetPacketKillStats( GamePacket );
	NET_FillShipRemInfo( &GamePacket->ShipInfo, MyShip, LocalPlayerId );
	NET_RmEvListUpdateLocations();

	// send state packet to all active remote players
	for ( int id = 0; id < CurMaxPlayers; id++ )
		if ( REMOTE_PLAYER_ACTIVE( id ) ) {

			UPDTXT( MSGOUT( "updating id %d (pid=%d).", id, GamePacket->MessageId ); );
			UPDTXT( ADXTXT( NETs_PrintNode( &Player_Node[ id ] ); ); );

			// enable remote player to determine whether
			// join status is consistent
			GamePacket->params[ 1 ] = Player_Status[ id ];

			// enable remote player to perform delayed join
			// (to compensate for dropped join packets)
			GamePacket->params[ 3 ] = MyShip->ObjectClass;

			// store remote player's id and send packet
			GamePacket->DestPlayerId = id;
			//FIXME: we only need to copy the global REList into the packet once
			NETs_SendPacket( (NetPacket*)GamePacket, &Player_Node[ id ] );
		}
}


// send node alive packets to prevent being kicked out while in entry-mode ----
//
PRIVATE
void SendNodeAlive()
{
	// fill game data header
	NETs_StdGameHeader( PKTP_NODE_ALIVE, (NetPacket*)GamePacket );

	//NOTE:
	// node alive packets contain killstats in order
	// for the server to receive all kills
	NET_SetPacketKillStats( GamePacket );

	NET_RmEvListUpdateLocations();

	// send node alive message to all active remote players
	for ( int id = 0; id < CurMaxPlayers; id++ )
		if ( REMOTE_PLAYER_ACTIVE( id ) ) {

			UPDTXT( MSGOUT( "sending node_alive to id %d (pid=%d).", id, GamePacket->MessageId ); );
			UPDTXT( ADXTXT( NETs_PrintNode( &Player_Node[ id ] ); ); );

			// enable remote player to perform delayed unjoin
			// (to compensate for dropped unjoin packets)
			GamePacket->params[ 0 ] = last_unjoin_flag;

			// enable remote player to determine whether
			// join status is consistent
			GamePacket->params[ 1 ] = Player_Status[ id ];

			// enable remote player to determine who
			// killed us if ( param1 == SHIP_DOWNED )
			if ( last_unjoin_flag == SHIP_DOWNED ) {
				ASSERT( ( CurKiller >= KILLERID_UNKNOWN ) && ( CurKiller < MAX_NET_PROTO_PLAYERS + KILLERID_BIAS ) );
				ASSERT( ( CurKiller >= 0 ) && ( CurKiller < 128 ) );
				GamePacket->params[ 2 ] = CurKiller;
			}

			// store remote player's id and send packet
			GamePacket->DestPlayerId = id;
			NETs_SendPacket( (NetPacket*)GamePacket, &Player_Node[ id ] );
		}
}


// assume all remote players are not up-to-date -------------------------------
//
PRIVATE
void PlayersNotUptodate()
{
	// reset up-to-date flags for all remote players
	for ( int id = 0; id < CurMaxPlayers; id++ )
		Player_UpToDate[ id ] = FALSE;
}


// interpolate remote player actions ------------------------------------------
//
PRIVATE
void InterpolatePlayers()
{
#ifdef INTERPOLATE_PLAYER_ACTIONS

	// interpolate actions of active players that are not up-to-date
	for ( int id = 0; id < CurMaxPlayers; id++ )
		if ( REMOTE_PLAYER_ACTIVE( id )	&&
			 !Player_UpToDate[ id ] && Player_Ship[ id ] ) {

			NET_InterpolatePlayer( id );
		}
#endif
}


// check if all active remote players are still alive -------------------------
//
PRIVATE
void CheckAliveStatus()
{
	for ( int id = 0; id < CurMaxPlayers; id++ )
		if ( REMOTE_PLAYER_ACTIVE( id ) ) {

			// decrease counter
			Player_AliveCounter[ id ] -= CurScreenRefFrames;

			// check for counter underrun
			if ( Player_AliveCounter[ id ] < 0 )
				NET_RemovePlayer( id );
		}
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


// maintain network game play -------------------------------------------------
//
void NETs_MaintainNet()
{
	CHECKMEMINTEGRITY();

	// make NumRemPlayers accessible from console
	AUXDATA_NUMREMPLAYERS_COPY = NumRemPlayers;

	// exit if not connected
	if ( !NetConnected )
		return;

	// send packet if necessary
	if ( CurPacketRefFrames >= Packet_Send_Frametime ) {

		// advance base for frame measurement
		PacketFrameBase += CurPacketRefFrames;

		// reply to received connect requests
		ConnectQueue();

		// perform regular packet sending
		if ( NetJoined ) {

			// send remote event list
			SendGameState();

			// check downing of local ship (only if live, not replay)
//FIXME:	if ( NetConnected == NETWORK_GAME_ON ) {
			if ( !DEMO_ReplayActive() ) {
				if ( MyShip->CurDamage > MyShip->MaxDamage ) {
					ShipDowned = TRUE;
					AUD_PlayerKilled();
				}
			}

		} else {

			// keep node alive
			SendNodeAlive();
		}

		// reset remote event list
		NET_RmEvListReset();

//		UPDTXT( MSGOUT( "__current message id: %d__", CurSendMessageId_PEER ); );
	}

	// reset up-to-date flags
	PlayersNotUptodate();

	// process all received packets
	NETs_ProcessPacketChain( ProcPacketGameLoop );

	// check if all active remote players are still alive
	CheckAliveStatus();

	// interpolate remote player actions
	InterpolatePlayers();

	// fire active duration weapons of remote ships
	FireDurationWeapons();
}



