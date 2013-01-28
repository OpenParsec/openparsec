/*
 * PARSEC - Server Access (UDP)
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:40 $
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
#include <ctype.h>
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
#include "net_defs.h"
#include "sys_defs.h"

// mathematics header
#include "utl_math.h"

// network code config
#include "net_conf.h"

// local module header
#include "net_serv.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_main.h"
#include "e_record.h"
#include "e_supp.h"
#include "h_supp.h"
#include "h_strmap.h"
#include "net_csdf.h"
#include "net_game.h"
#include "net_game_gmsv.h"
//#include "net_gmsv.h"
#include "net_rmev_gmsv.h"
#include "net_serv.h"
#include "net_stream.h"
#include "net_udpdf.h"
#include "net_univ.h"
#include "net_wrap.h"
#include "obj_clas.h"
#include "obj_ctrl.h"
#include "obj_cust.h"
#include "g_stgate.h"

// flags ----------------------------------------------------------------------
//

// string constants -----------------------------------------------------------
//
static char invalid_ping[]	 			= "invalid or missing maximum ping value.";
static char invalid_players[] 			= "invalid or missing minimum players value.";
static char no_server[]					= "you are not connected to a gameserver.";
static char no_command_specified[]		= "server command missing.";
static char no_default_masterserver[]	= "no default masterserver set";


// time in seconds after which connect attempt ends ---------------------------
//
#define TIMEOUT_JUMP				10		//


// connect retry timing -------------------------------------------------------
//
#define TIMEOUT_CONNECT				10		// number of connect tries
#define RETRYWAIT_CONNECT			500		// time to wait until next try

// disconnect retry timing ----------------------------------------------------
//
#define TIMEOUT_DISCONNECT			10		// number of disconnect tries
#define RETRYWAIT_DISCONNECT		600		// time to wait until next try

// command retry timing -------------------------------------------------------
//
#define TIMEOUT_COMMAND				10		// number of command send tries
#define RETRYWAIT_COMMAND			200		// time to wait until next try


// connect-loop exit codes ----------------------------------------------------
//
#define CONNECT_IN_GETCHALLENGE_LOOP		0	// stay in getchallenge-loop until timeout
#define CONNECT_IN_CONNECT_LOOP				1	// got challenge -> stay in connect loop
#define CONNECT_EXIT_TO_GAME_LOOP			2	// connection accepted -> exit to game loop
#define CONNECT_EXIT_TO_CONNECT_REFUSED		3	// connection refused  -> return
int GMSV_connect_state;				// current state in connection state machine

// namechange-loop exit codes -------------------------------------------------
//
#define NAMECHANGE_IN_NAMECHANGE_LOOP		0
#define NAMECHANGE_EXIT_TO_ACCEPTED	1
#define NAMECHANGE_EXIT_TO_REFUSED	2
int GMSV_namechange_state;			// current state in namechange state machine

// disconnect loop exit codes -------------------------------------------------
//
#define DISCONNECT_IN_DISCONNECT_LOOP		0	// stay in disconnect loop
#define DISCONNECT_EXIT_TO_ACCEPTED	1	// got disconnect ACK
#define DISCONNECT_EXIT_TO_INVALID		2	// got disconnect NACK
#define DISCONNECT_NOT_ENTERED			3   // disconnect loop not yet entered
int	GMSV_disconnect_state = DISCONNECT_NOT_ENTERED;



// module global variables ----------------------------------------------------
//
//static int			serv_fd;		// the server socket
//static int			j_serv_fd;		// temporary socket during server transition

static char				local_ip[ MAX_IPADDR_LEN + 1 ];			// our local IP (as seen from the server)
static word				local_port;								// our local port (as seen from the server)
static char				j_resolved_name[ MAX_IPADDR_LEN + 1 ];	// tempory servername during transition
static char				server_name[ MAX_SERVER_NAME + 1 ];

static int				num_clients_connected = 0;
client_s				client_list[ MAX_NET_ALLOC_SLOTS ];

static const char		challline1[] = GIVECHALLSTRING1;
static const char		connline2[]	 = CONNSTRING2;
static const char		rmvline1[]	 = RMVSTRING1;

//static char			recvline1[]	 = RECVSTR_ACCEPTED;
//static char			recvline2[]	 = RECVSTR_SERVER_FULL;
//static char			recvline3[]	 = RECVSTR_REQUEST_INVALID;
//static char			recvline4[]	 = RECVSTR_REMOVE_OK;
//static char			recvline5[]	 = RECVSTR_NOT_CONNECTED;
//static char			recvline6[]	 = RECVSTR_SERVER_INCOMP;
//static char			recvline7[]	 = RECVSTR_CLIENT_BANNED;
//static char			recvline8[]	 = RECVSTR_NAME_OK;
//static char			recvline9[]	 = RECVSTR_NAME_INVALID;
//static char			recvline10[] = RECVSTR_LINK_SERVER;

static const char		listline1[]  = LISTSTR_ADDED_IN_SLOT;
static const char		listline2[]  = LISTSTR_REMOVED_FROM_SLOT;
static const char		listline3[]  = LISTSTR_NAME_UPDATED;

static const char		nameline1[]  = NAMESTRING1;


// server (destination) ports -------------------------------------------------
//
extern int			server_udp_port;	// defined in NET_UDP.C

// client udp socket for both sending and receiving ---------------------------
//
extern int			udp_socket;			// defined in NET_UDP.C

// challenge we got from server -----------------------------------------------
//
int GMSV_connect_challenge;				


// print a list of currently active (connected) clients -----------------------
//
PRIVATE
void PrintClientList()
{
	for ( int cid = 0; cid < MAX_NET_PROTO_PLAYERS; cid++ )
		if ( !client_list[ cid ].slot_free ) {
			MSGOUT( "#%d: %s", cid, client_list[ cid ].client_name );
		}
}


// add <hostname> to list of connected clients --------------------------------
//
PRIVATE
int AddClient( int slot, char *name, int local_client )
{
	ASSERT( ( slot >= 0 ) && ( slot < MAX_NET_ALLOC_SLOTS ) );

	// ensure idempotent call
	if ( client_list[ slot ].slot_free == FALSE ) {
		MSGOUT( "repeated call to AddClient(%d, %s)", slot, name );
		return FALSE;
	}

	client_list[ slot ].slot_free = FALSE;
	strncpy( client_list[ slot ].client_name, name, MAX_PLAYER_NAME );
	client_list[ slot ].client_name[ MAX_PLAYER_NAME ] = 0;

	PrintClientList();

	// check for local client
	if ( local_client ) {

		LocalPlayerId = slot;

		// NumRemPlayers has already been set
		// to 1 in NET_GMSV::NETs_Connect()
		ASSERT( NumRemPlayers == 1 );

		// add local player to remote players
		NET_InitLocalPlayer();

		//NOTE:
		// for implementation efficiency and simplicity the
		// local player is added to the table of remote players.
		// to determine if a remote player is actually local
		// (e.g., to prevent sending a packet to oneself)
		// the shipid can be used, which is set to SHIPID_LOCALPLAYER.
		// the REMOTE_PLAYER_ACTIVE() macro determines if a player is
		// both active and actually remote.

	} else {

		// local player must already have been added
		ASSERT( NumRemPlayers > 0 );

		// add and register the new remote player
		NumRemPlayers++;
		NET_RegisterRemotePlayer( slot, NULL, name );
	}

	return TRUE;
}


// remove <slot? from list of connected clients -------------------------------
//
PRIVATE
int RemoveClient( int slot, int local_client )
{
	ASSERT( ( slot >= 0 ) && ( slot < MAX_NET_ALLOC_SLOTS ) );
	ASSERT( slot != LocalPlayerId );

	// ensure idempotent call
	if ( client_list[ slot ].slot_free ) {
		return FALSE;
	}

	ASSERT( NumRemPlayers > 1 );

	client_list[ slot ].slot_free = TRUE;

	// check for local client
	if ( !local_client ) {

		// unjoin player before disconnecting
		if ( Player_Status[ slot ] == PLAYER_JOINED ) {

			DBGTXT( MSGOUT( "NET_SERV::RemoveClient() unjoining still joined player before disconnecting" ); );

			// fill minimal RE for state switch
			RE_PlayerAndShipStatus pas_status;
			pas_status.senderid			= slot;
			pas_status.player_status	= PLAYER_CONNECTED;
			pas_status.params[ 0 ]		= USER_EXIT;

			// set the desired state
			NET_SetDesiredPlayerStatus( &pas_status );
		}

		NET_PlayerDisconnected( slot );
	}

	ASSERT( NumRemPlayers > 0 );

	PrintClientList();

	return TRUE;
}


// update <slot> with new name <name> ------------------------------
//
PRIVATE
void UpdateClientName( int slot, char *name )
{
	ASSERT( ( slot >= 0 ) && ( slot < MAX_NET_ALLOC_SLOTS ) );
	ASSERT( name != NULL );

	if ( client_list[ slot ].slot_free ) {
		MSGOUT( "NET_SERV::UpdateClientName(): invalid client address." );
		return;
	}

	strncpy( client_list[ slot ].client_name, name, MAX_PLAYER_NAME );
	client_list[ slot ].client_name[ MAX_PLAYER_NAME ] = 0;

	strncpy( Player_Name[ slot ], name, MAX_PLAYER_NAME );
	Player_Name[ slot ][ MAX_PLAYER_NAME ] = 0;

	PrintClientList();
}


// do save parsing of challenge request reply ---------------------------------
//
PRIVATE
int ParseChallengeRequestReply( char *recvline, int* challenge )
{
	ASSERT( recvline != NULL );
	ASSERT( challenge != NULL );
	
	// copy to parseline for tokenisation
	char parseline[ NET_UDP_DATA_LENGTH + 1 ];
	strncpy( parseline, recvline, NET_UDP_DATA_LENGTH );
	parseline[ NET_UDP_DATA_LENGTH ] = 0;
	
	// check for string identifier
	char *ident_str = strtok( parseline, " " );
	if ( ident_str == NULL )
		return FALSE;

	size_t len = strlen( ident_str );
	if ( strncmp( ident_str, RECVSTR_CHALLENGE, len ) != 0 )
		return FALSE;
	
	// get challenge number
	char* chall_str = strtok( NULL, "\0" );
	if ( chall_str == NULL )
		return FALSE;
	
	char *errpart;
	*challenge = (int) strtol( chall_str, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}
	
	return TRUE;
}


// send a client command to the server ----------------------------------------
//
PRIVATE
int Send_COMMAND( const char* clientcommand, node_t* node )
{
	char			buffer[ 2 * NET_MAX_NETPACKET_INTERNAL_LEN ];
	NetPacket_GMSV*	gamepacket = (NetPacket_GMSV *) buffer;
	
	// fill game data header
	NETs_StdGameHeader( PKTP_COMMAND, (NetPacket*)gamepacket );
	ASSERT( gamepacket->SendPlayerId == LocalPlayerId );
	
	// insert a single remote event containing the command
	NET_RmEvSingleCommandInfo( gamepacket, clientcommand );

	// send the packet
	NETs_AuxSendPacket( (NetPacket*)gamepacket, node );

	return TRUE;
}

// send a command to a specific node ( in a datagram ) ------------------------
//
int Send_COMMAND_Datagram( const char* commandstring, node_t* node, int anon )
{
	ASSERT( commandstring != NULL );
	ASSERT( node != NULL );

	char			buffer[ NET_MAX_NETPACKET_INTERNAL_LEN ];
	NetPacket_GMSV*	gamepacket = (NetPacket_GMSV *) buffer;

	// clear header and remote event list area
	memset( gamepacket, 0, NET_MAX_NETPACKET_INTERNAL_LEN );
	
	gamepacket->Command					= PKTP_COMMAND;
	gamepacket->SendPlayerId			= anon ? PLAYERID_ANONYMOUS : LocalPlayerId;

	gamepacket->MessageId				= MSGID_DATAGRAM;
	gamepacket->ReliableMessageId		= 0;
	gamepacket->AckMessageId			= 0;
	gamepacket->AckReliableMessageId	= 0;
	
	// RE list size always includes the RE terminator
	((RE_Header* )&gamepacket->RE_List)->RE_Type = RE_EMPTY;
	gamepacket->RE_ListSize	= sizeof( dword );

	// insert a single remote event containing the command
	NET_RmEvSingleCommandInfo( gamepacket, commandstring );

	// send the packet
	NETs_AuxSendPacket( (NetPacket*)gamepacket, node );
	
	return TRUE;
}



// request challenge from server ----------------------------------------------
//
PRIVATE
int RequestChallenge()
{
	ASSERT( LocalPlayerId == PLAYERID_ANONYMOUS );

	return Send_COMMAND_Datagram( challline1, &Server_Node, TRUE );
}

// request connect to server --------------------------------------------------
//
PRIVATE
int RequestConnect()
{
	ASSERT( LocalPlayerId == PLAYERID_ANONYMOUS );

	// connect string contains protocol version, challenge, player name, and operating system
	char command[ 75 ];
	snprintf( command, sizeof( command ),
		connline2,
		CLSV_PROTOCOL_MAJOR, CLSV_PROTOCOL_MINOR,
		GMSV_connect_challenge,
		LocalPlayerName, 
		Packet_Send_Frequency,
		Packet_Recv_Rate,
		"System" );
	
	return Send_COMMAND_Datagram( command, &Server_Node, TRUE );
}

// request disconnect from server ---------------------------------------------
//
PRIVATE
int RequestDisconnect()
{
	ASSERT( LocalPlayerId != PLAYERID_ANONYMOUS );

	char command[ 75 ];
	snprintf( command, sizeof( command ), rmvline1 );
	
	return Send_COMMAND( command, &Server_Node );
}


// request a namechange on the server -----------------------------------------
//
PRIVATE
int RequestNameChange()
{
	ASSERT( LocalPlayerId != PLAYERID_ANONYMOUS );

	char command[ 75 ];
	snprintf( command, sizeof( command ), nameline1, Player_Name[ LocalPlayerId ] );
	
	return Send_COMMAND( command, &Server_Node );
}


// run a request/reply loop, cheking state and timeout ------------------------
//
PRIVATE
int RunRequestReplyLoop( int (*requfunc)(), int timeout, int wait, int initstate, int* statevar )
{
	ASSERT( requfunc != NULL);

	(*statevar) = initstate;
	while ( ( (*statevar) == initstate ) && ( timeout > 0 ) ) {
		
		if ( TextModeActive )
			MSGPUT( "." );
		
		// send request for connection and wait a bit
		requfunc();
		NETs_SleepUntilInput( 1000 * wait / FRAME_MEASURE_TIMEBASE );
		//SYSs_Wait( wait );
		
		// process any received packets
extern void NET_ProcPacketLoop_GMSV( NetPacket* int_gamepacket, int bufid );
		NETs_ProcessPacketChain( NET_ProcPacketLoop_GMSV );
		
		timeout--;
	}
	
	// check for timeout
	if ( ( timeout == 0 ) && ( (*statevar) == initstate ) ) {
		return FALSE;
	}

	return TRUE;
}



// send connect request to server ---------------------------------------------
//
PRIVATE
int ServerConnectRequest()
{
	// invalidate current server ID
	CurServerID = -1;

	// reset stream 
	extern NET_Stream ServerStream;
#ifdef PARSEC_DEBUG
	MSGOUT( "ServerStream.Reset()" );
#endif //PARSEC_DEBUG
	ServerStream.Reset();
	ServerStream.SetPeerID( PLAYERID_SERVER );

	// getchallenge-loop
	if ( RunRequestReplyLoop( &RequestChallenge, TIMEOUT_CONNECT, RETRYWAIT_CONNECT, CONNECT_IN_GETCHALLENGE_LOOP, &GMSV_connect_state ) == FALSE ) {
		MSGOUT( "NET_SERV::ServerConnectRequest(): timeout while waiting for challenge." );
		return FALSE;
	}

	// connect loop
	if ( RunRequestReplyLoop( &RequestConnect, TIMEOUT_CONNECT, RETRYWAIT_CONNECT, CONNECT_IN_CONNECT_LOOP, &GMSV_connect_state ) == FALSE ) {
		MSGOUT( "NET_SERV::ServerConnectRequest(): timeout while waiting for connect reply." );
		return FALSE;
	}

	// we have successfully connected to the server
	if ( GMSV_connect_state == CONNECT_EXIT_TO_GAME_LOOP ) {
		// reset time display
		CurGameTime = GAME_NOTSTARTEDYET;

		// set the correct sender id of the stream
		ServerStream.SetSenderID( LocalPlayerId );
		ServerStream.SetConnected();
		
		// remove server name display
		if ( CurServerName != NULL ) {
			FREEMEM( CurServerName );
			CurServerName = NULL;
		}

		return TRUE;
	} else {
		// server refused connection
		ASSERT( GMSV_connect_state == CONNECT_EXIT_TO_CONNECT_REFUSED );
		return FALSE;
	}
}


// send name update request to server -----------------------------------------
//
int NET_ServerUpdateName()
{
	//NOTE:
	// the function RequestNameChange reads the new player name from Player_Name[ LocalPlayerId ]
	// instead of from PlayerName. this is crucial!! the semantics of
	// NET_RMEV::NET_RmEvPlayerName() which calls this function via
	// NET_SLSV::NETs_UpdateName() depends on this.

	// namechange-loop
	if ( RunRequestReplyLoop( &RequestNameChange, TIMEOUT_COMMAND, RETRYWAIT_COMMAND, NAMECHANGE_IN_NAMECHANGE_LOOP, &GMSV_namechange_state ) == FALSE ) {
		MSGOUT( "NET_SERV::NET_ServerUpdateName(): timeout while waiting namechange ACK." );
		return FALSE;
	}

	return ( GMSV_namechange_state == NAMECHANGE_EXIT_TO_ACCEPTED );
}



// do save parsing of connect request reply -----------------------------------
//
PRIVATE
int ParseConnectRequestReply( char* recvline, char* local_ip, word* local_port, int* slot, int* maxplayers, int* serverid, char* server_name )
{

	ASSERT( recvline != NULL );
	ASSERT( local_ip != NULL );
	ASSERT( slot != NULL );
	ASSERT( maxplayers != NULL );
	ASSERT( server_name != NULL );

	char parseline[ MAXLINE + 1 ];

	strncpy( parseline, recvline, MAXLINE );
	parseline[ MAXLINE ] = 0;

	// check for string identifier
	char *ident_str = strtok( parseline, " " );
	if ( ident_str == NULL )
		return FALSE;

	size_t len = strlen( ident_str );
	if ( strncmp( ident_str, RECVSTR_ACCEPTED, len ) != 0 )
		return FALSE;

	// get local ip string
	char *ip_str = strtok( NULL, ":" );
	if ( ip_str == NULL )
		return FALSE;

	strncpy( local_ip, ip_str, MAX_IPADDR_LEN );
	local_ip[ MAX_IPADDR_LEN ] = 0;

	// get local port
	char *port_str = strtok( NULL, " " );
	if ( port_str == NULL )
		return FALSE;

	char *errpart;
	int value = (int) strtol( port_str, &errpart, 10 );

	if ( *errpart != 0 ) {
		return FALSE;
	}

	*local_port = value;

	// get assigned slot number
	strtok( NULL, " " );
	char *slot_str = strtok( NULL, " " );
	if ( slot_str == NULL )
		return FALSE;

	*slot = (int) strtol( slot_str, &errpart, 10 );

	if ( *errpart != 0 ) {
		return FALSE;
	}

	// get max number of players on server
	strtok( NULL, " " );
	char *max_str = strtok( NULL, " " );
	if ( max_str == NULL )
		return FALSE;
	*maxplayers = (int) strtol( max_str, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}

	// get serverid
	strtok( NULL, " " );
	char* serverid_str = strtok( NULL, " " );
	if ( serverid_str == NULL )
		return FALSE;
	*serverid = (int) strtol( serverid_str, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}

	// get servername
	strtok( NULL, " " );
	char *sn_str = strtok( NULL, " " );
	if ( sn_str == NULL )
		return FALSE;

	strncpy( server_name, sn_str, MAX_SERVER_NAME );
	server_name[ MAX_SERVER_NAME ] = 0;

	return TRUE;
}


// do save parsing of list update string ( LISTSTR_ADDED_IN_SLOT ) ------------
//
PRIVATE
int ParseAddSlot( char* recvline, int* slot, char* name )
{
	ASSERT( recvline != NULL );
	ASSERT( name != NULL );
	ASSERT( slot != NULL );

	SAFE_STR_DUPLICATE( parseline, recvline, MAXLINE );

	// check for string identifier
	char *ident_str = strtok( parseline, " " );
	if ( ident_str == NULL )
		return FALSE;

	size_t len = strlen( ident_str );
	if ( strncmp( ident_str, LISTSTR_ADDED_IN_SLOT, len ) != 0 )
		return FALSE;

	// get slot number
	char *slot_str = strtok( NULL, " " );
	if ( slot_str == NULL )
		return FALSE;

	char* errpart;
	*slot = (int) strtol( slot_str, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}

	//FIXME: remove "nick" from command string
	// eat "nick"
	strtok( NULL, " " );
	//FIXME: we should take the whole rest of the commandling as name, as the 
	//       name could contain spaces
	// get player name
	char* name_str = strtok( NULL, " " );
	if ( name_str == NULL )
		return FALSE;

	strncpy( name, name_str, MAX_PLAYER_NAME );
	name[ MAX_PLAYER_NAME ] = 0;

	return TRUE;
}


// do save parsing of list update string ( LISTSTR_REMOVED_FROM_SLOT ) --------
//
PRIVATE
int ParseRemoveSlot( char* recvline, int* slot )
{
	ASSERT( recvline != NULL );

	SAFE_STR_DUPLICATE( parseline, recvline, MAXLINE );
	//char parseline[ MAXLINE + 1 ];
	//strncpy( parseline, recvline, MAXLINE );
	//parseline[ MAXLINE ] = 0;

	// check for string identifier
	char* ident_str = strtok( parseline, " " );
	if ( ident_str == NULL )
		return FALSE;

	size_t len = strlen( ident_str );
	if ( strncmp( ident_str, LISTSTR_REMOVED_FROM_SLOT, len ) != 0 )
		return FALSE;

	// get slot
	char* slot_str = strtok( NULL, "\n" );
	if ( slot_str == NULL )
		return FALSE;

	char* errpart;
	int value = (int) strtol( slot_str, &errpart, 10 );

	if ( *errpart != 0 ) {
		return FALSE;
	}

	*slot = value;

	return TRUE;
}


// do save parsing of list update string ( LISTSTR_NAME_UPDATED ) -------------
//
PRIVATE
int ParseUpdateSlot( char* recvline, int* slot, char* name )
{
	ASSERT( recvline != NULL );
	ASSERT( slot != NULL );
	ASSERT( name != NULL );

	SAFE_STR_DUPLICATE( parseline, recvline, MAXLINE );

	// check for string identifier
	char *ident_str = strtok( parseline, " " );
	if ( ident_str == NULL )
		return FALSE;

	size_t len = strlen( ident_str );
	if ( strncmp( ident_str, LISTSTR_NAME_UPDATED, len ) != 0 )
		return FALSE;

	// get slot affected
	char *slot_str = strtok( NULL, " " );
	if ( slot_str == NULL )
		return FALSE;

	char *errpart;
	int value = (int) strtol( slot_str, &errpart, 10 );

	if ( *errpart != 0 ) {
		return FALSE;
	}

	*slot = value;

	// get player name
	strtok( NULL, " " );
	char *name_str = strtok( NULL, "\n" );
	if ( name_str == NULL )
		return FALSE;

	strncpy( name, name_str, MAX_PLAYER_NAME );
	name[ MAX_PLAYER_NAME ] = 0;

	return TRUE;
}


// do save parsing of serverlink string ---------------------------------------
//
PRIVATE
int ParseServerLink( char *recvline, char *hostname, word *port, Vector3 *objpos )
{
	//FIXME: server links should be specified by full matrices not just object positions

	ASSERT( recvline != NULL );
	ASSERT( hostname != NULL );
	ASSERT( objpos   != NULL );

	char parseline[ MAXLINE + 1 ];

	strncpy( parseline, recvline, MAXLINE );
	parseline[ MAXLINE ] = 0;

	// check for string identifier
	char *ident_str = strtok( parseline, " " );
	if ( ident_str == NULL )
		return FALSE;

	size_t len = strlen( ident_str );
	if ( strncmp( ident_str, RECVSTR_LINK_SERVER, len ) != 0 )
		return FALSE;

	// get ip string (server hostname)
	char *ip_str = strtok( NULL, ":" );
	if ( ip_str == NULL )
		return FALSE;

	strncpy( hostname, ip_str, MAX_IPADDR_LEN );
	hostname[ MAX_IPADDR_LEN ] = 0;

	// get port of server
	char *port_str = strtok( NULL, " " );
	if ( port_str == NULL )
		return FALSE;

	char *errpart;

	long value = strtol( port_str, &errpart, 10 );

	if ( *errpart != 0 ) {
		return FALSE;
	}

	*port = value;

	// get x coordinate of object
	char *coord_str = strtok( NULL, " " );
	if ( coord_str == NULL )
		return FALSE;

	value = strtol( coord_str, &errpart, 10 );

	if ( *errpart != 0 ) {
		return FALSE;
	}

	objpos->X = (float) value;

	// get y coordinate of object
	coord_str = strtok( NULL, " " );
	if ( coord_str == NULL )
		return FALSE;

	value = strtol( coord_str, &errpart, 10 );

	if ( *errpart != 0 ) {
		return FALSE;
	}

	objpos->Y = (float) value;

	// get z coordinate of object
	coord_str = strtok( NULL, "\n" );
	if ( coord_str == NULL )
		return FALSE;

	value = strtol( coord_str, &errpart, 10 );

	if ( *errpart != 0 ) {
		return FALSE;
	}

	objpos->Z = (float) value;

	return TRUE;
}

// parsing helper function for ParseServerINFO and ParseServerPONG ------------
//
PRIVATE
int _ParsePONG_INFOHelper( char *parseline, int& serverid, refframe_t& sendframe, int& curplayers, int& maxplayers )
{
	ASSERT( parseline != NULL );

	// get serverid
	strtok( NULL, " " );
	char* serverid_str = strtok( NULL, " " );
	if ( serverid_str == NULL ) {
		return FALSE;
	}
	char *errpart;
	serverid = (int) strtol( serverid_str, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}

	// get refframe we send the ping
	strtok( NULL, " " );
	char* sendframe_str = strtok( NULL, " " );
	if ( sendframe_str == NULL ) {
		return FALSE;
	}
	sendframe = (refframe_t) strtol( sendframe_str, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}
	
	// get cur # of players
	strtok( NULL, " " );
	char* curplayers_str = strtok( NULL, "/" );
	if ( curplayers_str == NULL ) {
		return FALSE;
	}
	curplayers = (int) strtol( curplayers_str, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}

	// get max # of players 
	char* maxplayers_str = strtok( NULL, " \n" );
	if ( maxplayers_str == NULL ) {
		return FALSE;
	}
	maxplayers = (int) strtol( maxplayers_str, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}

	return TRUE;
}

// do save parsing of server INFO string --------------------------------------
//
PRIVATE
int ParseServerINFO( char* recvline, int& serverid, refframe_t& sendframe, int& curplayers, 
					 int& maxplayers, int& srv_version_minor, int& srv_version_major, char* srv_name )
{
	ASSERT( recvline != NULL );
	ASSERT( srv_name != NULL );

	// copy parseline for safe processing
	SAFE_STR_DUPLICATE( parseline, recvline, MAXLINE );

	// check for string identifier
	char *ident_str = strtok( parseline, " " );
	if ( ident_str == NULL ) {
		return FALSE;
	}

	size_t len = strlen( ident_str );
	if ( strncmp( ident_str, RECVSTR_INFO_REPLY, len ) != 0 ) {
		return FALSE;
	}

	// get some info
	if ( !_ParsePONG_INFOHelper( parseline, serverid, sendframe, curplayers, maxplayers ) ) {
		return FALSE;
	}

	// get major version
	strtok( NULL, " " );
	char* srv_version_major_str = strtok( NULL, "." );
	if ( srv_version_major_str == NULL ) {
		return FALSE;
	}
	char *errpart;
	srv_version_major = (int) strtol( srv_version_major_str, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}

	// get minor version
	char* srv_version_minor_str = strtok( NULL, " " );
	if ( srv_version_minor_str == NULL ) {
		return FALSE;
	}
	srv_version_minor = (int) strtol( srv_version_minor_str, &errpart, 10 );
	if ( *errpart != 0 ) {
		return FALSE;
	}

	// get servername
	strtok( NULL, " " );
	char* temp_srv_name = strtok( NULL, " " );
	if ( temp_srv_name == NULL ) {
		return FALSE;
	}
	strncpy( srv_name, temp_srv_name, MAX_SERVER_NAME );
	srv_name[ MAX_SERVER_NAME ] = 0;

	// replace underscores with spaces
	for ( char *scan = srv_name; *scan; scan++ ) {
		if ( *scan == '_' )
			*scan = ' ';
	}


	return TRUE;
}

// do save parsing of server PONG string --------------------------------------
//
PRIVATE
int ParseServerPONG( char* recvline, int& serverid, refframe_t& sendframe, int& curplayers, int& maxplayers )
{
	ASSERT( recvline != NULL );

	// copy parseline for safe processing
	SAFE_STR_DUPLICATE( parseline, recvline, MAXLINE );

	// check for string identifier
	char *ident_str = strtok( parseline, " " );
	if ( ident_str == NULL ) {
		return FALSE;
	}

	size_t len = strlen( ident_str );
	if ( strncmp( ident_str, RECVSTR_PING_REPLY, len ) != 0 ) {
		return FALSE;
	}

	return _ParsePONG_INFOHelper( parseline, serverid, sendframe, curplayers, maxplayers );
}



// init all data upon connection accepted -------------------------------------
//
PRIVATE
int _ConnectionAccepted( char* local_ip, word local_port, int slot, int maxplayers, int serverid, char* server_name )
{
	MSGOUT( "server: %s (%d): has accepted our connect request.", server_name, serverid );
	MSGOUT( "our accepted local ip is: %s:%d.", local_ip, local_port );
	MSGOUT( "slot: %d, maxplayers: %d.", slot, maxplayers );

	ASSERT( ( maxplayers > 0 ) && ( maxplayers <= MAX_NET_ALLOC_SLOTS ) );

	// check sanity of values received from server
	if ( ( maxplayers < 1 ) || ( maxplayers > MAX_NET_ALLOC_SLOTS ) ) {
		MSGOUT( "warning: illegal max number of players received from server." );
		return FALSE;
	}

	if ( ( slot < 0 ) || ( slot >= MAX_NET_ALLOC_SLOTS ) ) {
		MSGOUT( "warning: illegal slot number received from server." );
		return FALSE;
	}

	// store serverid
	CurServerID = serverid;

	// store max. player count
	CurMaxPlayers = maxplayers;

	// fixup servername
	strlwr( server_name );
	for ( char *scan = server_name; *scan != 0; scan++ ) {
		if ( *scan == '_' )
			*scan = ' ';
	}

	// make fictitious name of server available globally for display in HUD
	if ( CurServerName != NULL ) {
		FREEMEM( CurServerName );
		CurServerName = NULL;
	}

	CurServerName = (char *) ALLOCMEM( MAX_SERVER_NAME + 1 );
	strcpy( CurServerName, server_name );

	//FIXME: this must be part of the initial update burst received from the server
	//AUXDATA_BACKGROUND_NEBULA_ID = nebula + 2;

	// let's add ourselves...
	ASSERT( num_clients_connected == 0 );
	num_clients_connected = 1;
	int rc = AddClient( slot, LocalPlayerName, TRUE );
	ASSERT( rc );

	// set global flag
	NetConnected = NETWORK_GAME_ON;

	// we got connection ACK, so exit to the game loop
	GMSV_connect_state = CONNECT_EXIT_TO_GAME_LOOP;

	// reset timeout counter
	ServerTimeoutFrames = DEFAULT_TIMEOUT_SERVER_CONNECTION;

	// all objects ( extras, summoned objects etc. are removed upon connection )
	//FIXME: this most likely will move to initial gamestate dump 
	KillAllObjects();

	return TRUE;
}


// parse and handle all server messages ---------------------------------------
//
int NET_ServerParseMessage( char* msg )
{
	ASSERT( msg != NULL );
	ASSERT( *msg != '\0' );
	
	int  	maxplayers = 0;
	int		slot = -1;
	char    hostname[ MAX_HOSTNAME_LEN + 1 ];
	word	port = 0;
	char 	name[ MAX_PLAYER_NAME + 1 ];
	Vector3 objpos;
	int		challenge = 0;
	int		serverid = 0;
	refframe_t sendframe = 0;
	int		curplayers = 0;
	int		srv_version_minor;
	int		srv_version_major;
	char	srv_name[ MAX_SERVER_NAME + 1 ];
	char*	recvline = NULL;
	
	// record server message for demo playback
	if ( NetConnected != NETWORK_GAME_SIMULATED ) {
		
		recvline = msg;
		REC_RecordServerMessage( recvline );
		
	} else {
		
		// add line-feed because it gets stripped during playback
		recvline = (char *) ALLOCMEM( strlen( msg ) + 2 );
		strcpy( recvline, msg );

		//FIXME: CBX ???? WTF ???
		size_t len = strlen( recvline );
		recvline[ len ] = '\n';
		recvline[ len + 1 ] = 0x00;
	}
	
	// challenge request reply
	if ( ParseChallengeRequestReply( recvline, &challenge ) ) {

		// check for relevant state in state machine
		if ( GMSV_connect_state == CONNECT_IN_GETCHALLENGE_LOOP ) {

			// we got the challenge, so enter the connect loop
			GMSV_connect_state = CONNECT_IN_CONNECT_LOOP;
		
			// store the challenge we got from the server
			GMSV_connect_challenge = challenge;
			
			return TRUE;
		} else {
			// ignore reply
			return FALSE;
		}

	// connect request reply
	} else if ( ParseConnectRequestReply( recvline, local_ip, &local_port, &slot, &maxplayers, &serverid, server_name )  ) {

		// check for correct state in state machine 
		if ( GMSV_connect_state == CONNECT_IN_CONNECT_LOOP ) {

			// init state to connected
			return _ConnectionAccepted( local_ip, local_port, slot, maxplayers, serverid, server_name );

		} else {
			// ignore reply
			return FALSE;
		}

	// challenge invalid
	} else if ( strcmp( recvline, RECVSTR_CHALL_INVALID ) == 0 ) {

		// check for relevant state in state machine
		if ( GMSV_connect_state == CONNECT_IN_CONNECT_LOOP ) {
			MSGOUT( "Server got invalid challenge." );
			// connect was refused
			GMSV_connect_state = CONNECT_EXIT_TO_CONNECT_REFUSED;
		} 
		
		return FALSE;
		
	// remove request reply
	} else if ( strcmp( recvline, RECVSTR_REMOVE_OK ) == 0 ) {

		// check for relevant state in state machine ( disconnect requested by client )
		if ( GMSV_disconnect_state == DISCONNECT_IN_DISCONNECT_LOOP ) {
			MSGOUT( "server has accepted our remove request." );
			if ( NetConnected == NETWORK_GAME_SIMULATED ) {
				
				//NOTE:
				// the actual remove is normally done by
				// NETs_Disconnect(), but this function will
				// not be called in simulated net mode.
				
				NET_ServerDisconnectReset();
			}
			
			// server accepted the disconnect request
			ASSERT( GMSV_disconnect_state == DISCONNECT_IN_DISCONNECT_LOOP );
			GMSV_disconnect_state = DISCONNECT_EXIT_TO_ACCEPTED;
			
			return TRUE;

		} else {

			// got forced disconnect from server 
			MSGOUT( "forced disconnect from server" );

			// we must be in the game loop
			ASSERT( GMSV_connect_state == CONNECT_EXIT_TO_GAME_LOOP );

			// clean up 
			NET_ServerDisconnectReset();
			
			return TRUE;
		}

	// add client message
	} else if ( ParseAddSlot( recvline, &slot, name ) ) {
		ASSERT( NetConnected );


		// we must be in the game loop
		if ( GMSV_connect_state != CONNECT_EXIT_TO_GAME_LOOP ) {
			return FALSE;
		}
		
		// check sanity of slot number received
		if ( ( slot < 0 ) || ( slot >= MAX_NET_ALLOC_SLOTS ) ) {
			MSGOUT( "warning: illegal slot number received from server." );
			return FALSE;
		}

		// add client
		if ( num_clients_connected < MAX_NET_ALLOC_SLOTS ) {

			if ( AddClient( slot, name, FALSE ) ) {
				MSGOUT( "receiving client list update from server." );
				MSGOUT( "adding client %d named %s to list.", slot, name );
				num_clients_connected++;
			}
		}
		return TRUE;

	// remove client message
	} else if ( ParseRemoveSlot( recvline, &slot ) ) {
		ASSERT( NetConnected );

		// we must be in the game loop
		if ( GMSV_connect_state != CONNECT_EXIT_TO_GAME_LOOP ) {
			return FALSE;
		}
		
		// remove client
		if ( num_clients_connected > 0 ) {
			if ( RemoveClient( slot, FALSE ) ) {
				MSGOUT( "receiving client list update from server." );
				MSGOUT( "removing client %d from list.", slot );
				num_clients_connected--;
			}
		}
		return TRUE;

	// update name message
	} else if ( ParseUpdateSlot( recvline, &slot, name ) ) {
		ASSERT( NetConnected );

		// we must be in the game loop
		ASSERT( GMSV_connect_state == CONNECT_EXIT_TO_GAME_LOOP );
		
		// update name
		MSGOUT( "receiving client list update from server." );
		MSGOUT( "client %d has new name: %s.", slot, name );
		UpdateClientName( slot, name );
		return TRUE;

	// name change reply
	} else if ( strcmp( recvline, RECVSTR_NAME_OK ) == 0 ) {
		ASSERT( NetConnected );

		// we must be in the game loop
		ASSERT( GMSV_connect_state == CONNECT_EXIT_TO_GAME_LOOP );
		
		if( GMSV_namechange_state == NAMECHANGE_IN_NAMECHANGE_LOOP ) {
			MSGOUT( "server %s has accepted our name update request.", server_name );
			GMSV_namechange_state = NAMECHANGE_EXIT_TO_ACCEPTED;
			return TRUE;
		} else {
			return FALSE;
		}

	// server link
	} else if ( ParseServerLink( recvline, hostname, &port, &objpos ) ) {

		// we must be in the game loop
		ASSERT( GMSV_connect_state == CONNECT_EXIT_TO_GAME_LOOP );

		/*
		DBGTXT( MSGOUT( "adding stargate to %s:%d at (%d %d %d)", hostname, port,
										    		   GEOMV_TO_INT( objpos.X ),
													   GEOMV_TO_INT( objpos.Y ),
													   GEOMV_TO_INT( objpos.Z ) ); );

		dword objclass = OBJ_FetchObjectClassId( "stargate" );

		if ( objclass != CLASS_ID_INVALID ) {

			Xmatrx startm;
			MakeIdMatrx( startm );
			startm[ 0 ][ 3 ] = objpos.X;
			startm[ 1 ][ 3 ] = objpos.Y;
			startm[ 2 ][ 3 ] = objpos.Z;

			Stargate *obj = (Stargate *) SummonObject( objclass, startm );

			if ( obj != NULL ) {

				// set destination ip address of stargate
				strncpy( obj->destination_ip, hostname, STARGATE_MAX_DEST_IP );
				obj->destination_ip[ STARGATE_MAX_DEST_IP ] = 0;

				obj->destination_port = port;

				// set destination system name of stargate
				// (default to ip:port)
				snprintf( obj->destination_name, STARGATE_MAX_DEST_NAME, "%s:%d", hostname, port );

				DBGTXT( MSGOUT( "stargate created." ); );
				// ping the server, the ping reply will activate the stargate
				// i.e. wake it up from dormant state
				int ping = -1;
				NET_ServerPing( hostname, port, &ping, FALSE, TRUE );

				//NOTE:
				// ping result is not valid here, since we didn't wait
				// for the arrival of the ping packet. The ping time will
				// be available once the Net_ProcessPingPacket() function
				// has processed the returned packet
			}
		}

		return TRUE;
	
	// server is full*/
	} else if ( strcmp( recvline, RECVSTR_SERVER_FULL ) == 0 ) {

		if ( GMSV_connect_state == CONNECT_IN_CONNECT_LOOP ) {
			// connect was refused
			MSGOUT( "Server is full, try again later." );
			GMSV_connect_state = CONNECT_EXIT_TO_CONNECT_REFUSED;
		} 

		return FALSE;

	// unknown comamnd sent
	} else if ( strcmp( recvline, RECVSTR_REQUEST_INVALID ) == 0 ) {

		MSGOUT( "Server does not understand this request." );

		if ( GMSV_connect_state == CONNECT_IN_CONNECT_LOOP ) {
			// connect was refused
			GMSV_connect_state = CONNECT_EXIT_TO_CONNECT_REFUSED;
		}
		
		return FALSE;

	// not connected
	} else if ( strcmp( recvline, RECVSTR_NOT_CONNECTED ) == 0 ) {

		MSGOUT( "Server thinks we are not connected." );

		if ( GMSV_disconnect_state == DISCONNECT_IN_DISCONNECT_LOOP ) {
			// server thinks we are not connected
			GMSV_disconnect_state = DISCONNECT_EXIT_TO_INVALID;
		}

		if ( GMSV_namechange_state == NAMECHANGE_IN_NAMECHANGE_LOOP ) {
			// server thinks we are not connected
			GMSV_namechange_state = NAMECHANGE_EXIT_TO_REFUSED;
		}

		return FALSE;

	// name already taken
	} else if ( strcmp( recvline, RECVSTR_NAME_INVALID ) == 0 ) {

		MSGOUT( "This name is already taken, choose a new one !" );
		
		// namechange was refused
		if ( GMSV_namechange_state == NAMECHANGE_IN_NAMECHANGE_LOOP ) {
			GMSV_namechange_state = NAMECHANGE_EXIT_TO_REFUSED;
		}

		// connect was refused
		if ( GMSV_connect_state == CONNECT_IN_CONNECT_LOOP ) {
			GMSV_connect_state = CONNECT_EXIT_TO_CONNECT_REFUSED;
		}
		
		return FALSE;

	// incompatible client version
	} else if ( strcmp( recvline, RECVSTR_SERVER_INCOMP ) == 0 ) {

		if ( GMSV_connect_state == CONNECT_IN_CONNECT_LOOP ) {
			// connect was refused
			MSGOUT( "Server is not compatible with this version of Parsec." );
			GMSV_connect_state = CONNECT_EXIT_TO_CONNECT_REFUSED;
		} 

		return FALSE;

	// banned from server
	} else if ( strcmp( recvline, RECVSTR_CLIENT_BANNED ) == 0 ) {

		if ( GMSV_connect_state == CONNECT_IN_CONNECT_LOOP ) {
			// connect was refused
			MSGOUT( "You are banned on this server." );
			GMSV_connect_state = CONNECT_EXIT_TO_CONNECT_REFUSED;
		} 
		
		return FALSE;

	// parse for PONG
	} else if ( ParseServerPONG( recvline, serverid, sendframe, curplayers, maxplayers ) ) {

		// set the updated information
		NET_ServerList_SetPONG( serverid, 
								SYSs_GetRefFrameCount() - sendframe, 
								curplayers, 
								maxplayers );

	// parse for INFO
	} else if ( ParseServerINFO( recvline, serverid, sendframe, curplayers, maxplayers, srv_version_minor, srv_version_major, srv_name ) ) {

		// set the updated information
		NET_ServerList_SetINFO( serverid, 
								SYSs_GetRefFrameCount() - sendframe, 
								curplayers, 
								maxplayers, 
								srv_version_minor, 
								srv_version_major, 
								srv_name );

	} else {

		//NOTE:
		// we output every server message that we can't handle.
		// this can be abused to send actual text messages to the clients.
		// imagine this example:
		// --- this server will be rebooted in 5 minutes ---
		// ---    PLEASE END YOUR GAME AND DISCONNECT    ---
		// Clever, eh ? ;)

		//TODO:
		// some additional parsing code could go here, so that some
		// special messages are displayed using ShowMessage(), or
		// using the demo text display...

		// ignore empty strings
		if ( *recvline == 0x0a || *recvline == 0x0d )
			return TRUE;

		// strip trailing '\n' because MSGOUT() adds its own
		for ( char *scan = recvline; *scan; scan++ ) {
			if ( *scan == 0x0d || *scan == 0x0a )
				*scan = 0x00;
		}

		MSGOUT( recvline );

		// show it on message display too...
		ShowMessage( recvline );

		return TRUE;
	}

	return FALSE;
}

// resolve the CurServerToResolve into Server_Node ----------------------------
//
int NET_ResolveServerNode()
{
	ASSERT( CurServerToResolve != NULL );

	// default is well-known port
	int port = server_udp_port;

	// optional port specification can override
	char *portstr = NULL;
	for ( char *scan = CurServerToResolve; *scan; scan++ ) {
		if ( *scan == ':' ) {
			*scan = 0;
			portstr = scan + 1;
			break;
		}
	}
	if ( portstr != NULL ) {
		char *errpart;
		port = (int) strtol( portstr, &errpart, 10 );
		if ( *errpart != 0 ) {
			return FALSE;
		}
	}

	// try to resolve DNS name to IP address (still as string)
	char resolved_name[ MAX_IPADDR_LEN + 1 ];
	if ( NET_ResolveHostName( CurServerToResolve, resolved_name, NULL ) ) {
		MSGOUT( "servername %s resolved to %s", CurServerToResolve, resolved_name );
	} else {
		// DNS lookup failed
		return FALSE;
	}

	// save server address for later use by NET_GMSV.C
	inet_aton( resolved_name, (in_addr*)&Server_Node );
	UDP_StoreNodePort( &Server_Node, port );

	return TRUE;
}




// establish connection to server ---------------------------------------------
//
int NET_ServerConnect()
{
	MSGOUT( "trying to establish connection..." );

	ASSERT( NumRemPlayers == 1 );


	//NOTE:
	// ServerConnectRequest() will return FALSE if
	// the server refuses the connect request.
	
	// send connect request to server
	return ServerConnectRequest();
}


// send remove request to server ----------------------------------------------
//
int NET_ServerDisconnect()
{
	// we MUST not be in the disconnect loop yet
	ASSERT( GMSV_disconnect_state == DISCONNECT_NOT_ENTERED );

	int disc_success;

extern int disable_disc_command_send_to_server;
	if ( disable_disc_command_send_to_server == FALSE ) {
		// disconnect-loop
		if ( RunRequestReplyLoop( &RequestDisconnect, TIMEOUT_DISCONNECT, RETRYWAIT_DISCONNECT, DISCONNECT_IN_DISCONNECT_LOOP, &GMSV_disconnect_state ) == FALSE ) {
			// request timed out
			MSGOUT( "NET_ServerDisconnect(): disconnect reply timed out." );
			disc_success = FALSE;
		} else {
			// successfully disconnected from the server ?
			disc_success = ( GMSV_disconnect_state == DISCONNECT_EXIT_TO_ACCEPTED );
		}
	} else {
		disc_success = TRUE;
	}
	
	// reset to initial state
	GMSV_disconnect_state = DISCONNECT_NOT_ENTERED;

	return disc_success;
}


// clean up afterclient has disconnected (even if not verified by server) -----
//
void NET_ServerDisconnectReset()
{
	// remove all remote players
	NET_RemoveRemotePlayers();

	// remove local player
	ASSERT( NumRemPlayers == 1 );
	NumRemPlayers = 0;

	// update player status
	Player_Status[ LocalPlayerId ] = PLAYER_INACTIVE;

	// reset global flag
	NetConnected = NETWORK_GAME_OFF;

	// reset local playerid
	LocalPlayerId = PLAYERID_ANONYMOUS;

	// no client list entries
	num_clients_connected = 0;

	// reset time display
	CurGameTime = GAME_NOTSTARTEDYET;

	// remove server name display
	if ( CurServerName != NULL ) {
		FREEMEM( CurServerName );
		CurServerName = NULL;
	}

	// invalidate current server ID
	CurServerID = -1;

	// kill all objects (mostly summoned ones) that might still be around
	KillAllObjects();
}


// jump to a new server -------------------------------------------------------
//
int NET_ServerJump()
{
	// remember current server node
	node_t _OldServerNode;
	memcpy( &_OldServerNode, &Server_Node, sizeof( node_t ) );
	
	// disconnect from current server
	NETs_Disconnect();

	// ensure correct state
	ASSERT( !NetConnected && !NetJoined );
	ASSERT( NumRemPlayers == 0 );
	ASSERT( CurJumpServerNode != NULL );

	// set new server node to connect to
	memcpy( &Server_Node, CurJumpServerNode, sizeof( node_t ) );

	// (re)init tables
	NET_InitRemotePlayerTables();

	// discard old packets
	NETs_FlushListenBuffers();

	// already count local player
	//FIXME: why ????
	ASSERT( NumRemPlayers == 0 );
	NumRemPlayers = 1;

	// try to connect to new jump server
	if ( !NET_ServerConnect() ) {

		MSGOUT( "jump failed - trying to reconnect to previous server." );

		// try to reconnect to old server
		memcpy( &Server_Node, &_OldServerNode, sizeof( node_t ) );
		if ( !NET_ServerConnect() ) {

			MSGOUT( "connect to previous server failed." );
			return FALSE;
		}
	}

	return TRUE;

	//FIXME: implement with UDP




/*	MSGOUT( "trying to establish new connection..." );

	ASSERT( NetConnected );
	ASSERT( !NetJoined );

	ASSERT( CurServer != NULL );
	ASSERT( CurJumpServer != NULL );

	// default is well-known port
	int port = server_udp_port;

	// optional port specification can override
	char *portstr = NULL;
	for ( char *scan = CurJumpServer; *scan; scan++ ) {
		if ( *scan == ':' ) {
			*scan = 0;
			portstr = scan + 1;
			break;
		}
	}
	if ( portstr != NULL ) {
		char *errpart;
		port = strtol( portstr, &errpart, 10 );
		if ( *errpart != 0 ) {
			return FALSE;
		}
	}

	// try to resolve DNS name to IP address (still as string)
	if ( NET_ResolveHostName( CurJumpServer, j_resolved_name, NULL ) ) {
		MSGOUT( "servername %s resolved to %s", CurJumpServer, j_resolved_name );
	} else {
		return FALSE;
	}

	// open socket for sending messages to the server
	j_serv_fd = Socket( AF_INET, SOCK_STREAM, 0 );

	// make socket non-blocking
	int arg = TRUE;
	Ioctl( j_serv_fd, FIONBIO, (char *)&arg );

	// set target address and port for socket
	sockaddr_in servaddr;
	bzero( &servaddr, sizeof( servaddr ) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons( port );
	inet_aton( j_resolved_name, &servaddr.sin_addr );

	// perform non-blocking connect with a timeout of TIMEOUT_JUMP seconds
	if ( connect_nonb( j_serv_fd, (SA *)&servaddr, sizeof( servaddr ), TIMEOUT_JUMP ) < 0 ) {
		MSGOUT( "NET_ServerJump(): connect timed out." );
		return FALSE;
	}

	// ensure that socket is still non-blocking
	arg = TRUE;
	Ioctl( j_serv_fd, FIONBIO, (char *)&arg );

	// save current server name in case the jump doesn't succeed
	char *oldserver = (char *) ALLOCMEM( strlen ( CurServer ) + 1 );
	strcpy( oldserver, CurServer );

	//NOTE:
	// at this point we still have a connection with the old server and
	// we already have a connection to the new server...
	// we can now disconnect from the old server and connect to the new one...

	// disconnect from old server
	NETs_Disconnect();

	//NOTE:
	// CurServer will have been freed by
	// NET_GMSV::NETs_Disconnect().

	// ensure correct state
	ASSERT( !NetConnected && !NetJoined );
	ASSERT( NumRemPlayers == 0 );
	ASSERT( CurServer == NULL );
	ASSERT( CurJumpServer != NULL );
	ASSERT( Player_Status[ LocalPlayerId ] == PLAYER_INACTIVE );

	// get jump target into global variable
	CurServer = CurJumpServer;
	CurJumpServer = NULL;

	// (re)init tables
	NET_InitRemotePlayerTables();

	// discard old packets
	NETs_FlushListenBuffers();

	// already count local player
	ASSERT( NumRemPlayers == 0 );
	NumRemPlayers = 1;

	// init client list
	for ( int cid = 0; cid < MAX_NET_ALLOC_SLOTS; cid++ ) {
		client_list[ cid ].slot_free = TRUE;
	}

	// send connect request to server via tcp connection at socket
	if ( ServerConnectRequest( j_serv_fd ) == TRUE ) {

		serv_fd = j_serv_fd;

		// save server address for later use by NET_GMSV.C
		inet_aton( j_resolved_name, &Server_Node );
		Server_Node.address[ 4 ] = port >> 8;
		Server_Node.address[ 5 ] = port & 0x00ff;

		ASSERT( !NetConnected );
		ASSERT( NumRemPlayers == 1 );
		ASSERT( Player_Status[ LocalPlayerId ] == PLAYER_CONNECTED );

		// set global flag
		NetConnected = NETWORK_GAME_ON;

		// old server not needed anymore
		ASSERT( oldserver != NULL );
		FREEMEM( oldserver );
		oldserver = NULL;

		return TRUE;

	} else {

		// if new server refused our connect request, we
		// re-connect to the old server if possible
		if ( ServerConnectRequest( serv_fd ) == TRUE ) {

			ASSERT( CurServer != NULL );
			FREEMEM( CurServer );
			CurServer = oldserver;

			ASSERT( !NetConnected );
			ASSERT( NumRemPlayers == 1 );
			ASSERT( Player_Status[ LocalPlayerId ] == PLAYER_CONNECTED );

			// set global flag
			NetConnected = NETWORK_GAME_ON;

			return TRUE;
		}
	}
*/
}


// handle any pending tcp messages from server --------------------------------
//
void NET_ServerMessages()
{
/*	if ( NetConnected == NETWORK_GAME_SIMULATED )
		return;

	// try to read string from server stream
	char recvline[ MAXLINE + 1 ];
	ssize_t n = readline( serv_fd, recvline, MAXLINE );

	if ( n < 0 ) {

		FETCH_ERRNO();
		if ( ERRNO_EWOULDBLOCK )
			return;

		MSGOUT( "NET_ServerMessages(): tcp stream error: %d.", ERRNO );
		return;
	}

	// tcp connection closed by other end?
	if ( n == 0 ) {
		MSGOUT( "NET_ServerMessages(): server dropped connection." );

		//NOTE:
		// server propably has quit somehow, atleast we have to assume this.
		// if we just force the disconnect, Winsock will report WSAENETDOWN
		// errors after a new connection has been established...
		// Only closing and re-opening of the UDP sockets helps.
		// Therefore we completely re-initialize the networking subsystem...
		// (only God and Bill Gates know why...)

		NETs_ResetAPI();

		// kill all objects (mostly summoned ones) that might still be around
		// i.e. stargates!
		KillAllObjects();

		// reset time display
		CurGameTime = GAME_NOTSTARTEDYET;

		// remove server name display
		if ( CurServerName != NULL ) {
			FREEMEM( CurServerName );
			CurServerName = NULL;
		}

		return;
	}

	NET_ServerParseMessage( recvline );
*/
}


// process ping packets replied from server -----------------------------------
//
void NET_ServerProcessPingPacket( NetPacket_GMSV* gamepacket )
{
/*	refframe_t roundtriptime = SYSs_GetRefFrameCount() - gamepacket->params[ 3 ];

	int senderid = gamepacket->SendPlayerId;

	// calculate ping time in milliseconds
	int pingtime = ( ( (float)roundtriptime ) / ( 2 * FRAME_MEASURE_TIMEBASE ) ) * 1000;

	// check if server replied to an anonymous packet
	if ( senderid == PLAYERID_ANONYMOUS ) {

		ServerPingTime = pingtime;

		// get node address from returned ping packet
		//FIXME: this should go into one of the params
		dword servernode = gamepacket->GameTime;

		//FIXME: this should go into a single remote event
		ServerInfo *info = (ServerInfo *) &gamepacket->ShipInfo;

		int port = ( info->porthi << 8 ) | info->portlo;

		DBGTXT( MSGOUT( "ping reply from node: 0x%x", servernode, port ); );

		// try to wakeup the stargate associated with this servernode
		char serverip[ MAX_IPADDR_LEN + 1 ];

		inet_ntop( AF_INET, &servernode, serverip, MAX_IPADDR_LEN + 1 );

		DBGTXT( MSGOUT( "Waking stargate of %s", serverip ); );

		//FIXME: this is GAMECODE !!!!!

		GenObject *walkobjs = FetchFirstCustom();
		for ( ; walkobjs; walkobjs = walkobjs->NextObj ) {

			// get type id of the custom stargate
			static dword stargate_typeid = TYPE_ID_INVALID;

			if ( stargate_typeid == TYPE_ID_INVALID ) {
				stargate_typeid = OBJ_FetchCustomTypeId( "stargate" );
			}

			// we only want to walk stargates
			if ( walkobjs->ObjectType != stargate_typeid ) {
				continue;
			}

			Stargate *stargate = (Stargate *) walkobjs;

			if ( strcmp( stargate->destination_ip, serverip ) == 0 &&
				 stargate->destination_port == port ) {

				stargate->dormant	 = FALSE;
				stargate->ping		 = pingtime;
				stargate->lastpinged = SYSs_GetRefFrameCount();

				// copy servername from ServerInfo of ping packet
				//ServerInfo *info = (ServerInfo *) &gamedata->ShipInfo;

				// replace underscores with spaces
				for ( char *scan = info->name; *scan; scan++ ) {

					if ( *scan == '_' )
						*scan = ' ';
				}

				strncpy( stargate->destination_name, info->name, MAX_SERVER_NAME );
				stargate->destination_name[ MAX_SERVER_NAME ] = 0;

				return;
			}
		}

	} else {

		CurServerPing = pingtime;

		MSGOUT( "ping reply from server: %dms", pingtime );
		DBGTXT( MSGOUT( "sid=%d pid=%d.", senderid, gamepacket->MessageId ); );
	}*/
}


// ping timing constants ------------------------------------------------------
//
#define TIMEOUT_PING		300	// number of tries to receive a ping packet
#define RETRYWAIT_PING		2	// time to wait for a ping reply

// measure ping time to specified server ( gameserver ) -------------------
//
int NET_ServerPing( node_t* node, int anon, int fullinfo /*= FALSE*/ )
{
	ASSERT( node != NULL );

	// send request in datagram
	char szBuffer[ MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ];
	if ( fullinfo ) {
		snprintf( szBuffer, MAX_RE_COMMANDINFO_COMMAND_LEN, INFOSTRING1, (int)SYSs_GetRefFrameCount() );
	} else {
		snprintf( szBuffer, MAX_RE_COMMANDINFO_COMMAND_LEN, PINGSTRING1, (int)SYSs_GetRefFrameCount() );
	}

	return Send_COMMAND_Datagram( szBuffer, node, anon );
}


// all info about a "SERVERLIST" command --------------------------------------
//
PUBLIC serverlist_info_s serverlist_info;


// get list of running gameservers from masterserver --------------------------
//
int NET_ServerList_Get( char* masterhostname, int serverid /*= -1*/ )
{
	ASSERT( masterhostname != NULL );

	char	ms_resolved[ MAX_IPADDR_LEN + 1 ];
	node_t	_MasterServerNode;

	// try to resolve DNS name to IP address (still as string)
	if ( NET_ResolveHostName( masterhostname, ms_resolved, &_MasterServerNode ) ) {
		UDP_StoreNodePort( &_MasterServerNode, DEFAULT_MASTERSERVER_UDP_PORT );
		MSGOUT( "masterserver %s resolved to: %s", masterhostname, ms_resolved );
	} else {
		return FALSE;
	}
	MSGOUT("Getting List for %d", serverid);
	// get whole list
	if ( serverid == -1 ) {

		// send request in datagram
		Send_COMMAND_Datagram( MASV_LIST, &_MasterServerNode, FALSE );

		// reset information that we are about to receive
		for ( int sid = 0; sid < MAX_SERVERS; sid++ ) {
			server_list[ sid ].slot_free = TRUE;
		}
		//FIXME: 
		for ( int oid = 0; oid < num_map_objs; oid++ ) {
			mapobj_s *tmp = map_objs[ oid ];
			if ( tmp != NULL ) {
				if ( tmp->texname != NULL ) {
					FREEMEM( tmp->texname );
				}
				FREEMEM( tmp );
			}
		}
		num_servers_joined	= 0;
		g_nNumLinks			= 0;
		num_map_objs		= 0;

		// set flag that we have started a serverlist command, and remember startpoint to
		serverlist_info.m_ListRequested = TRUE;

	} else {

		// check whether request already in list
		int nIndex = 0;
		for( nIndex = 0; nIndex < serverlist_info.m_nNumRequestedServerIDs; nIndex++ ) {
			if ( serverid == serverlist_info.m_RequestedServerIDs[ nIndex ] ) {
				MSGOUT("Request already in queue for %d?",serverid);
				break;
			}
		}
		// append to list of requests
		if ( nIndex == serverlist_info.m_nNumRequestedServerIDs ) {
			ASSERT( serverlist_info.m_nNumRequestedServerIDs < MAX_NUM_LINKS );
			serverlist_info.m_RequestedServerIDs[ nIndex ] = serverid;
			serverlist_info.m_nNumRequestedServerIDs++;
		}

		// send request in datagram
		char szBuffer[ MAX_RE_COMMANDINFO_COMMAND_LEN ];
		sprintf( szBuffer, MASV_INFO, serverid );
		Send_COMMAND_Datagram( szBuffer, &_MasterServerNode, FALSE );

	}

	return TRUE;
}


// set the information we get from a PONG packet in the serverlist ------------
//
int NET_ServerList_SetINFO( int serverid, refframe_t framediff, int curplayers, int maxplayers, int srv_version_minor, int srv_version_major, char* srv_name )
{
	// calculate ping in ms
	int ping_in_ms = ( 1000 * framediff ) / FRAME_MEASURE_TIMEBASE;

	// check whether to update the information for the current server
	if ( CurServerID == serverid ) {
		CurServerPing = ping_in_ms;
	}

	// scan for serverid
	for( int nServerIndex = 0; nServerIndex < MAX_SERVERS; nServerIndex++ ) {
		if ( server_list[ nServerIndex ].serverid == serverid ) {
			server_s& server = server_list[ nServerIndex ];

			server.major_version		= srv_version_major;
			server.minor_version		= srv_version_minor;
			strncpy( server.server_name, srv_name, MAX_SERVER_NAME + 1 );
			server.server_name[ MAX_SERVER_NAME ] = 0;
			server.number_of_players	= curplayers;
			server.max_players			= maxplayers;
			server.ping_in_ms			= ping_in_ms;

			if ( NetConnected ) {
				// find stargate for this server 
				Stargate* stargate = NET_FindStargate( serverid );
				if ( stargate != NULL ) {
					stargate->dormant	 = FALSE;
					stargate->ping		 = ping_in_ms;
					stargate->lastpinged = SYSs_GetRefFrameCount();

					strncpy( stargate->destination_name, server.server_name, MAX_SERVER_NAME );
					stargate->destination_name[ MAX_SERVER_NAME ] = 0;

					memcpy( &stargate->destination_node, &server.node, sizeof( node_t ) );
				}
			}

			return TRUE;
		}
	}

	return FALSE;
}


// set the information we get from a PONG packet in the serverlist ------------
//
int NET_ServerList_SetPONG( int serverid, refframe_t framediff, int curplayers, int maxplayers )
{
	// calculate ping in ms
	int ping_in_ms = ( 1000 * framediff ) / FRAME_MEASURE_TIMEBASE;

	// check whether to update the information for the current server
	if ( CurServerID == serverid ) {
		CurServerPing = ping_in_ms;
	}

	// scan for serverid
	for( int nServerIndex = 0; nServerIndex < MAX_SERVERS; nServerIndex++ ) {
		if ( server_list[ nServerIndex ].serverid == serverid ) {
			server_s& server = server_list[ nServerIndex ];

			server.number_of_players	= curplayers;
			server.max_players			= maxplayers;
			server.ping_in_ms			= ping_in_ms;

			if ( NetConnected ) {
				// find stargate for this server 
				Stargate* stargate = NET_FindStargate( serverid );
				if ( stargate != NULL ) {
					stargate->dormant	 = FALSE;
					stargate->ping		 = ping_in_ms;
					stargate->lastpinged = SYSs_GetRefFrameCount();

					strncpy( stargate->destination_name, server.server_name, MAX_SERVER_NAME );
					stargate->destination_name[ MAX_SERVER_NAME ] = 0;

					memcpy( &stargate->destination_node, &server.node, sizeof( node_t ) );
				}
			}

			return TRUE;
		}
	}

	return FALSE;
}

// update a server in the serverlist ------------------------------------------
//
int NET_ServerList_UpdateServer( RE_IPv4ServerInfo* pServerInfo )
{
	// check whether request in list
	word serverid = pServerInfo->serverid;
	for( int nReqIndex = 0; nReqIndex < serverlist_info.m_nNumRequestedServerIDs; nReqIndex++ ) {

		if ( serverid == serverlist_info.m_RequestedServerIDs[ nReqIndex ] ) {

			// find server in global list
			for( int nServerIndex = 0; nServerIndex < num_servers_joined; nServerIndex++ ) {
				server_s& server = server_list[ nServerIndex ];
				if ( server.serverid == serverid ) {
					memcpy( &server.node, &pServerInfo->node, sizeof( node_t ) );
					server.xpos	= pServerInfo->xpos;
					server.ypos	= pServerInfo->ypos;

					// request full information about the server
					if ( AUX_SHOW_PING_IN_SERVERLIST ) {
						NET_ServerPing( &server.node, TRUE, TRUE );
					}
					return TRUE;
				}
			}

			// if not in list, append to list
			int old_requested = serverlist_info.m_ListRequested;
			serverlist_info.m_ListRequested = TRUE;
			NET_ServerList_AddServer( pServerInfo );
			serverlist_info.m_ListRequested = old_requested;
			
			return TRUE;
		}
	}

	return FALSE;
}


// add a server to the serverlist ---------------------------------------------
//
int NET_ServerList_AddServer( RE_IPv4ServerInfo* pServerInfo )
{
	ASSERT( pServerInfo != NULL );

	// we do not accept any servers, when we didnt request a serverlist
	if ( serverlist_info.m_ListRequested == FALSE ) {
		return FALSE;
	}

	// fill in all info we get out of the RE_IPv4ServerInfo
	int nIndex = num_servers_joined;

	if ( nIndex < MAX_SERVERS ) {
		server_s& server = server_list[ nIndex ];

		//server_list[ nIndex ].server_name[ 0 ]		= 0;
		strcpy( server.server_name, "TestServer" );
		memcpy( &server.node, &pServerInfo->node, sizeof( node_t ) );
		server.server_os[ 0 ]		= 0;
		server.number_of_players	= 0;
		server.max_players			= 0;
		server.xpos					= pServerInfo->xpos;
		server.ypos					= pServerInfo->ypos;
		server.serverid				= pServerInfo->serverid;
		server.ping_in_ms            = -1;
		
		server.slot_free				= FALSE;

		// request full information about the server
		if ( AUX_SHOW_PING_IN_SERVERLIST ) {
			NET_ServerPing( &server.node, TRUE, TRUE );
		}

		num_servers_joined++;
	}

	return TRUE;
}


// link 2 servers in the serverlist -------------------------------------------
//
int NET_ServerList_AddLinkInfo( RE_ServerLinkInfo* pServerLinkInfo )
{
	ASSERT( pServerLinkInfo != NULL );

	// check for upper bound
	if ( g_nNumLinks < MAX_LINKLIST_SIZE ) {

		// simply store the information we get from the masterserver
		link_list[ g_nNumLinks ].serverid1 = pServerLinkInfo->serverid1;
		link_list[ g_nNumLinks ].serverid2 = pServerLinkInfo->serverid2;
		link_list[ g_nNumLinks ].flags = pServerLinkInfo->flags;

		// increase the # of links
		g_nNumLinks++;

		return TRUE;

	} else {
		return FALSE;
	}
}


// add a map object to be displayed in the starmap ----------------------------
//
int NET_ServerList_AddMapObject( RE_MapObject* pMapObject )
{
	ASSERT( pMapObject != NULL );

	// check against upper bound
	if ( num_map_objs < MAX_MAP_OBJECTS ) {

		// allocate memory
		mapobj_s *tmp = (mapobj_s *) ALLOCMEM( sizeof( mapobj_s ) );
		if ( tmp == NULL ) {
			OUTOFMEM( 0 );
		}

		// fill in values
		tmp->xpos 	= pMapObject->xpos;
		tmp->ypos 	= pMapObject->ypos;
		tmp->w 		= pMapObject->w;
		tmp->h		= pMapObject->h;
		tmp->texmap = NULL;

		tmp->texname = (char *) ALLOCMEM( strlen( pMapObject->texname ) + 1 );
		strcpy( tmp->texname, pMapObject->texname );

		map_objs[ num_map_objs ] = tmp;
		num_map_objs++;

		return TRUE;
	} else {
		return FALSE;
	}
}

// print current serverlist on console ----------------------------------------
//
PRIVATE
void NET_ServerList_Print()
{
	// no list requested yet
	if ( serverlist_info.m_ListRequested == FALSE ) {
		CON_AddLine( "no serverlist requested yet" );
		return;
	}

	char output_line[ MAXLINE ];
	int  showline = TRUE;

	// print server_list on console
	for ( int nIndex = 0; nIndex < num_servers_joined; nIndex++ ) {

		server_s *server = &server_list[ nIndex ];
		/*
		if ( serverlist_info.m_nMinPlayers != -1 ) {
			if ( server->number_of_players < serverlist_info.m_nMinPlayers ) {
				showline = FALSE;
			}
		}*/

		if ( AUX_SHOW_PING_IN_SERVERLIST ) {
			/*
			if ( serverlist_info.m_nMaxPing != -1 ) {
				if ( server->ping_in_ms > serverlist_info.m_nMaxPing ) {
					showline = FALSE;
				} else if ( server->ping_in_ms < serverlist_info.m_nMinPing ) {
					showline = FALSE;
				}
			}*/

			sprintf( output_line, "%s : %u.%u.%u.%u:%d : %d/%d : %dms", server->server_name,
															server->node.address[ 0 ], 
															server->node.address[ 1 ], 
															server->node.address[ 2 ], 
															server->node.address[ 3 ],
															UDP_GetNodePort( &server->node ),
															server->number_of_players,
					 										server->max_players, 
															server->ping_in_ms );
		} else {

			sprintf( output_line, "%s : %u.%u.%u.%u:%d : %d/%d", server->server_name,
															server->node.address[ 0 ], 
															server->node.address[ 1 ], 
															server->node.address[ 2 ], 
															server->node.address[ 3 ],
															UDP_GetNodePort( &server->node ),
															server->number_of_players,
					 										server->max_players );
		}

		if ( showline ) {
			CON_AddLine( output_line );
		}
	}
}


// key table for serverlist command -------------------------------------------
//
key_value_s serverlist_key_value[] = {

	{ "ping",		NULL,	KEYVALFLAG_NONE				},
	{ "players",	NULL,	KEYVALFLAG_NONE				},
	{ "master",		NULL,	KEYVALFLAG_NONE				},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_SERVERLIST_PING,
	KEY_SERVERLIST_PLAYERS,
	KEY_SERVERLIST_MASTER
};


// get list of running gameservers from masterserver --------------------------
//
PRIVATE
int Cmd_SERVERLIST_REQUEST( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// serverlist_request_command	::= 'serverlist.request' [<max_ping>] [<min_players>]
	// max_ping						::= 'ping' <int>
	// min_players					::= 'players' <int>

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( serverlist_key_value, paramstr ) )
		return TRUE;

	int max_ping = -1;

	if ( ScanKeyValueInt( &serverlist_key_value[ KEY_SERVERLIST_PING ], &max_ping ) < 0 ) {
		CON_AddLine( invalid_ping );
		return TRUE;
	}

	int min_players = -1;

	if ( ScanKeyValueInt( &serverlist_key_value[ KEY_SERVERLIST_PLAYERS ], &min_players ) < 0 ) {
		CON_AddLine( invalid_players );
		return TRUE;
	}

	// get server_list from masterserver
	//FIXME: how do we switch to different masterservers, once we discover, that the current is not active
	if ( !NET_ServerList_Get( Masters[ 0 ] ) ) {
		return TRUE;
	}

	return TRUE;
}


// get list of running gameservers from masterserver --------------------------
//
PRIVATE
int Cmd_SERVERLIST_SHOW( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// serverlist_show_command	::= 'serverlist.show' 

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	// print the serverlist
	NET_ServerList_Print();

	return TRUE;
}

// initiate control connection to server --------------------------------------
//
PRIVATE
int Cmd_SRVCMD( char *paramstr )
{
/*	//NOTE:
	//CONCOM:
	// srvcmd_command	::= 'srvcmd' <command_spec>
	// command_spec		::= <valid server control command>

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	if ( NetConnected != NETWORK_GAME_ON ) {
		CON_AddLine( no_server );
		return TRUE;
	}

	char *command = GetStringBehindCommand( paramstr, FALSE );
	if ( command == NULL ) {
		CON_AddLine( no_command_specified );
		return TRUE;
	}

	int cmd_len = strlen( command );
	char *sendline = NULL;
	sendline = (char *) ALLOCMEM( cmd_len + 3 );
	if ( sendline == NULL )
		OUTOFMEM( 0 );

	strcpy( sendline, command );

	// add end of line characters
	sendline[ cmd_len ] = 0x0d;
	sendline[ cmd_len + 1 ] = 0x0a;
	sendline[ cmd_len + 2 ] = 0x00;

retry:
	int rc = writen( serv_fd, sendline, strlen( sendline ) );
	if ( rc < 0 ) {

		FETCH_ERRNO();
		if ( ERRNO_EWOULDBLOCK )
			goto retry;

		MSGOUT( "Cmd_SRVCMD(): cannot send command to server." );

		FREEMEM( sendline );
		return TRUE;
	}

	FREEMEM( sendline );*/
	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( NET_SERV )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "serverlist.request" command
	regcom.command	 = "serverlist.request";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_SERVERLIST_REQUEST;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "serverlist.show" command
	regcom.command	 = "serverlist.show";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_SERVERLIST_SHOW;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "srvcmd" command
	regcom.command	 = "srvcmd";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_SRVCMD;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	serverlist_info.Reset();
}



