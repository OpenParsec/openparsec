/*
 * PARSEC - Utility Functions - client
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:40 $
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

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

// drawing subsystem
#include "d_bmap.h"
#include "d_font.h"

// network code config
#include "net_conf.h"

// local module header
#include "net_util.h"

// proprietary module headers
#include "e_color.h"
#include "net_swap.h"
#include "sys_bind.h"


// string constants -----------------------------------------------------------
//
static char unknown_str[]		= "unknown";
static char logged_in_str[]		= "players currently logged in";


//-----------------------------------------------------------------------------
// UTILITY FUNCTIONS FOR GAME CODE                                            -
//-----------------------------------------------------------------------------


// determine whether current protocol is peer to peer -------------------------
//
int NET_ProtocolPEER()
{
	return ( sys_BindType_PROTOCOL == BT_PROTOCOL_PEERTOPEER );
}


// determine whether current protocol is game server --------------------------
//
int NET_ProtocolGMSV()
{
	return ( sys_BindType_PROTOCOL == BT_PROTOCOL_GAMESERVER );
}


// determine whether currently connected in peer to peer game -----------------
//
int NET_ConnectedPEER()
{
	return ( NetConnected && NET_ProtocolPEER() );
}


// determine whether currently connected to game server -----------------------
//
int NET_ConnectedGMSV()
{
	return ( NetConnected && NET_ProtocolGMSV() );
}


// fetch pointer to name of remote player -------------------------------------
//
char *NET_FetchPlayerName( int playerid )
{
	//NOTE:
	// this function is declared in NET_SUBH.H

//	ASSERT( NetConnected );
	ASSERT( playerid >= 0 );
	ASSERT( playerid < MAX_NET_PROTO_PLAYERS );

	if ( ( (dword)playerid < (dword)MAX_NET_PROTO_PLAYERS ) && ( Player_Status[ playerid ] ) )
		return Player_Name[ playerid ];
	else
		return unknown_str;
}


// set name of local player ---------------------------------------------------
//
int NET_SetPlayerName( const char *name )
{
	ASSERT( name != NULL );

	//NOTE:
	// this function is declared in NET_SUBH.H and
	// used by CON_COM::CheckPlayerName().

	int rc = TRUE;

	// generate remote event that updates the other players to altered name
	if ( NetConnected )
		rc = NET_RmEvPlayerName( name );

	if ( rc ) {

		// store new name globally
		strcpy( LocalPlayerName, name );

		// copy local player name into list of network player names
		strcpy( Player_Name[ LocalPlayerId ], LocalPlayerName );
	}

	return rc;
}


// fetch pointer to ship of remote player -------------------------------------
//
ShipObject *NET_FetchOwnersShip( int ownerid )
{
	//NOTE:
	// this function is declared in NET_SUBH.H

	ASSERT( NetConnected );
	ASSERT( ownerid >= 0 );
	ASSERT( ownerid < MAX_NET_PROTO_PLAYERS );

	return (ShipObject *) Player_Ship[ ownerid ];
}


// update kill stat for single player (called by game code) -------------------
//
void NET_SetPlayerKillStat( int playerid, int amount )
{
	//NOTE:
	// this function is declared in NET_SUBH.H

	ASSERT( NetConnected );
	ASSERT( playerid >= 0 );
	ASSERT( playerid < MAX_NET_PROTO_PLAYERS );

	if ( ( (dword)playerid < (dword)MAX_NET_PROTO_PLAYERS ) && ( Player_Status[ playerid ] ) )
		Player_KillStat[ playerid ] += amount;

	// save biased id of player who killed us
	CurKiller = playerid + KILLERID_BIAS;

	// ensure that next packet contains killstat update
	// for the player that was killed right now
	CurKillUpdate = playerid;
}


// check whether the specified kill limit has already been reached ------------
//
int NET_KillStatLimitReached( int limit )
{
	ASSERT( NetConnected );

	//NOTE:
	// this function is only used by
	// G_MAIN::Gm_HandleGameOver().

	for ( int id = 0; id < MAX_NET_PROTO_PLAYERS; id++ ) {
		if ( Player_Status[ id ] != PLAYER_INACTIVE ) {
			if ( Player_KillStat[ id ] >= limit ) {
				return TRUE;
			}
		}
	}

	return FALSE;
}


// force all killstats to zero if no player joined ----------------------------
//
void NET_KillStatForceIdleZero()
{
	ASSERT( NetConnected );

	//NOTE:
	// this function is only used by
	// G_MAIN::Gm_HandleGameOver().

	int gameidle = TRUE;
	int id = 0;
	for ( id = 0; id < MAX_NET_PROTO_PLAYERS; id++ ) {
		if ( Player_Status[ id ] == PLAYER_JOINED ) {
			gameidle = FALSE;
		}
	}

	if ( gameidle ) {
		for ( id = 0; id < MAX_NET_PROTO_PLAYERS; id++ ) {
			Player_KillStat[ id ] = 0;
		}
	}
}


// old remote player-list with big opaque font --------------------------------
//
PRIVATE
void DrawRemotePlayerListScreen()
{
//	if ( !NetConnected )
//		return;

	D_SetWStrContext( CharsetInfo[ MSG_CHARSETNO ].charsetpointer,
					  CharsetInfo[ MSG_CHARSETNO ].geompointer,
					  NULL,
					  CharsetInfo[ MSG_CHARSETNO ].width,
					  CharsetInfo[ MSG_CHARSETNO ].height );

	int EM_Title_Y  = 100;
	int EM_PName_Y  = 150;
	int EM_LineDist = 12;

	D_WriteString( logged_in_str,
				 ( Screen_Width - strlen( logged_in_str ) * CharsetInfo[ MSG_CHARSETNO ].width ) / 2, EM_Title_Y );

	const char *voidstr = "- available -";
	int stry = EM_PName_Y;

	for ( int i = 0; i < MAX_NET_PROTO_PLAYERS; i++ ) {

		const char *strp = Player_Status[ i ] ? Player_Name[ i ] : voidstr;

		if ( i == LocalPlayerId )
			strp = LocalPlayerName; // so local name can be changed on the fly

		int strx = ( Screen_Width - strlen( strp ) * CharsetInfo[ MSG_CHARSETNO ].width ) / 2;
		D_WriteString( strp, strx, stry );
		stry += CharsetInfo[ MSG_CHARSETNO ].height + EM_LineDist;
	}
}


// draw text for entry mode (names of all players currently logged in) --------
//
void NET_DrawEntryModeText()
{
	//NOTE:
	// this function is declared in NET_SUBH.H

	ASSERT( EntryMode );
	ASSERT( NetConnected && !NetJoined );
	ASSERT( !FloatingMenu && !InFloatingMenu );

	DrawRemotePlayerListScreen();
}
