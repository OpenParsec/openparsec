/*
 * PARSEC - Connect Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:30 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-2000
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
#include "net_defs.h"
#include "vid_defs.h"

// subsystem linkage info
#include "linkinfo.h"

// network code config
#include "net_conf.h"

// local module header
#include "net_conn.h"

// proprietary module headers
#include "con_aux.h"
#include "con_ext.h"
#include "con_main.h"
#include "e_demo.h"
#include "g_supp.h"
#include "sys_bind.h"
#include "vid_init.h"


// maximum id of universe to try if entry has been refused --------------------
//
#define MAX_UNIVERSE				2


// string constants -----------------------------------------------------------
//
static char net_unavailable[]		= "network not available.";
static char replay_in_progress[]	= "demo replay in progress. command not allowed.";
static char stopping_replay[]		= "stopping demo replay.";
static char already_connected[]		= "already connected.";
static char not_connected[]			= "not connected.";
static char forcing_unjoin[]		= "forcing unjoin.";
static char disconnect_ok[]			= "disconnect ok.";
static char disconnect_for_switch[]	= "subsystem change only possible while disconnected.";


// protocol name table --------------------------------------------------------
//
static protocol_s _protocol_table[] = {

#if defined( LINKED_PROTOCOL_PEERTOPEER )
	{ "peer-to-peer",	BT_PROTOCOL_PEERTOPEER	},
#endif

#if defined( LINKED_PROTOCOL_GAMESERVER )
	{ "game-server",	BT_PROTOCOL_GAMESERVER	},
#endif

#if defined( LINKED_PROTOCOL_DEMONULL )
	{ "demo",			BT_PROTOCOL_DEMONULL	},
#endif

	{ NULL,				-1,						}
};

PUBLIC protocol_s *protocol_table = _protocol_table;


// get name of currently selected protocol ------------------------------------
//
const char *NET_GetCurrentProtocolName()
{
	// search for currently selected protocol in table
	for ( int id = 0; protocol_table[ id ].name; id++ )
		if ( protocol_table[ id ].id == sys_BindType_PROTOCOL )
			return protocol_table[ id ].name;

	return "none";
}


// on-the-fly protocol and packet api switching -------------------------------
//
int NET_SwitchNetSubsys( const char *protocol )
{
	ASSERT(protocol != NULL);

	// ensure messages work correctly
	CON_AddLine( con_prompt );
	CON_DisableLineFeed();

	// search for protocol with specified name
	int protid = 0;
	if ( protocol != NULL ) {
		for ( ; protocol_table[ protid ].name; protid++ ) {
			if ( strcmp( protocol, protocol_table[ protid ].name ) == 0 )
				break;
		}
		if ( protocol_table[ protid ].name == NULL )
			return FALSE;
	}

	// only if not connected
	if ( NetConnected ) {
		MSGOUT( disconnect_for_switch );
		return FALSE;
	}

	// ensure no replay in progress while switching
	if ( DEMO_ReplayActive() ) {
		MSGOUT( stopping_replay );
		DEMO_StopReplay();
	}

	// deinit old subsystem
	NETs_KillAPI();

	// rebind protocol
	if ( protocol != NULL ) {
		sys_BindType_PROTOCOL = protocol_table[ protid ].id;
		SYS_Bind_PROTOCOL();
	}

	// init new subsystem
	NETs_InitAPI();

	return TRUE;
}


// display names of players currently playing in universe ---------------------
//
INLINE
void DisplayPlayerNames()
{
	//TODO:
	// this function is only used when there was no remote slot available.
	// the names of all remote players are displayed in textmode.
}


// used to perform automatic connect on game-entry ----------------------------
//
int NET_AutomaticConnect()
{
	//NOTE:
	// this function is used by G_BOOT::HandleEntryMode().

	// start with universe #1
	MyUniverse = 1;

	// find universe with available player slot
	while ( ( MyUniverse <= MAX_UNIVERSE ) && !NETs_Connect() ) {

		// display players occupying universe
		DisplayPlayerNames();

		MSGOUT( "Already four players in universe %d.", MyUniverse );
		if ( MyUniverse < MAX_UNIVERSE )
			MSGOUT( "Trying next universe:" );

		// try to enter next universe
		MyUniverse++;
	}

	// no free slot found in any universe?
	if ( MyUniverse > MAX_UNIVERSE ) {

		if ( TextModeActive ) {
			VID_InitMode();
		}

		return FALSE;
	}

	return TRUE;
}


// rebind networking protocol -------------------------------------------------
//
PRIVATE
void RebindProtocol( int protid )
{
	// check if rebind necessary
	if ( sys_BindType_PROTOCOL != protid ) {

		// set protocol
		sys_BindType_PROTOCOL = protid;

		// deinit old subsystem
		NETs_KillAPI();

		// rebind protocol
		SYS_Bind_PROTOCOL();

		// init new subsystem
		NETs_InitAPI();
	}
}


// user connect peer-to-peer --------------------------------------------------
//
PRIVATE
int PeerConnect()
{

#ifdef LINKED_PROTOCOL_PEERTOPEER

	// rebind protocol if necessary
	RebindProtocol( BT_PROTOCOL_PEERTOPEER );

	// start with universe #1
	MyUniverse = 1;

	// find universe with available player slot
	while ( ( MyUniverse <= MAX_UNIVERSE ) && !NETs_Connect() ) {

		MSGOUT( "connect to universe %d refused.", MyUniverse );

		// try to enter next universe
		MyUniverse++;
	}

	// no free slot found in any universe?
	if ( MyUniverse > MAX_UNIVERSE ) {

		MSGOUT( "unable to establish connection." );
		ASSERT( !EntryMode );

		return FALSE;

	} else {

		// always start network game in entry mode
		InFloatingMenu	= FloatingMenu;
		EntryMode		= TRUE;

		// make sure it's off
		ObjCamOff();

		return TRUE;
	}

#else

	MSGOUT( "peer-to-peer protocol not linked." );
	return FALSE;

#endif

}


// default client-server protocol ---------------------------------------------
//
#ifdef LINKED_PROTOCOL_GAMESERVER
	int default_prot = BT_PROTOCOL_GAMESERVER;
#else
	int default_prot = -1;
#endif


// user connect to server -----------------------------------------------------
//
PRIVATE
int ServerConnect( const char *server )
{

#if defined( LINKED_PROTOCOL_GAMESERVER )

	//NOTE:
	// the default client/server protocol will be bound
	// automatically if peer-to-peer is active.
	// this normally is game-server, provided it is
	// linked. if it is not linked an error message results.

	// ensure client/server protocol is bound
	int protid = sys_BindType_PROTOCOL;
	if ( protid == BT_PROTOCOL_PEERTOPEER )
		protid = default_prot;

	// error if no client-server protocol linked
	if ( protid == -1 ) {
		MSGOUT( "no client-server protocol available." );
		return FALSE;
	}

	// rebind protocol if necessary
	RebindProtocol( protid );

	// always universe 1
	//FIXME: ?? GMSV ??
	MyUniverse = 1;

	// store server name temporarely
	ASSERT( CurServerToResolve == NULL );
	CurServerToResolve = (char *) ALLOCMEM( strlen( server ) + 1 );
	strcpy( CurServerToResolve, server );

	// try to establish connection
	int connect_success = NETs_Connect();

	// delete temp server name
	ASSERT( CurServerToResolve != NULL );
	FREEMEM( CurServerToResolve );
	CurServerToResolve = NULL;
	
	if ( !connect_success ) {

		MSGOUT( "connect to server refused." );
		ASSERT( !EntryMode );
		return FALSE;

	} else {

		// always start network game in entry mode
		InFloatingMenu	= FloatingMenu;
		EntryMode		= TRUE;

		// make sure it's off
		ObjCamOff();

		return TRUE;
	}

#else

	MSGOUT( "client-server protocol not linked." );
	return FALSE;

#endif

}


// user connect ---------------------------------------------------------------
//
int NET_CommandConnect( const char *server )
{
	ASSERT( server != NULL );

	//NOTE:
	// this function may be called by CON_COM::NetConnect(),
	// or M_MAIN::MenuItemSelectConnect().

	// ensure messages work correctly
	CON_AddLine( con_prompt );
	CON_DisableLineFeed();

	// ensure network is available
	if ( NetworkGameMask == NETWORK_GAME_OFF ) {
		MSGOUT( net_unavailable );
		return FALSE;
	}

	// ensure no replay in progress
	if ( DEMO_ReplayActive() ) {

		if ( AUX_STOP_DEMO_REPLAY_ON_CONNECT ) {
			MSGOUT( stopping_replay );
			DEMO_StopReplay();
		} else {
			MSGOUT( replay_in_progress );
			return FALSE;
		}
	}

	// only if not connected
	if ( NetConnected ) {
		MSGOUT( already_connected );
		return FALSE;
	}

	// determine type of connection
	if ( strcmp( server, "peers" ) == 0 ) {

		// try to connect to peers via broadcast
		return PeerConnect();

	} else {

		// try to connect to server
		return ServerConnect( server );
	}
}


// user disconnect ------------------------------------------------------------
//
int NET_CommandDisconnect()
{
	// this function may be called by CON_COM::NetDisconnect(),
	// or M_MAIN::MenuItemSelectDisconnect().

	// ensure messages work correctly
	CON_AddLine( con_prompt );
	CON_DisableLineFeed();

	// ensure network is available
	if ( NetworkGameMask == NETWORK_GAME_OFF ) {
		MSGOUT( net_unavailable );
		return FALSE;
	}

	// only if connected
	if ( !NetConnected ) {
		MSGOUT( not_connected );
		return FALSE;
	}

	// ensure no replay in progress
	if ( DEMO_ReplayActive() ) {
		MSGOUT( replay_in_progress );
		return FALSE;
	}

	// if joined unjoin first
	if ( NetJoined ) {

		MSGOUT( forcing_unjoin );

		ObjCamOff();
		NETs_Unjoin( USER_EXIT );
	}

	// close connection
	NETs_Disconnect();

	// set floating menu and entrymode state
	InFloatingMenu	= FloatingMenu;
	EntryMode		= FALSE;

	// make sure it's off
	ObjCamOff();

	MSGOUT( disconnect_ok );
	return TRUE;
}


