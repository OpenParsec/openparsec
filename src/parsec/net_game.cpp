/*
 * PARSEC - Network Game Functions
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
#include "net_game.h"

// proprietary module headers
#include "aud_game.h"
#include "con_aux.h"
#include "e_record.h"
#include "g_camera.h"
#include "g_stars.h"
#include "g_supp.h"
#include "h_supp.h"
#ifdef LINKED_PROTOCOL_GAMESERVER
#	include "net_csdf.h"
#endif // LINKED_PROTOCOL_GAMESERVER
#include "obj_creg.h"
#include "obj_ctrl.h"
#include "obj_xtra.h"
#include "g_sfx.h"


// flags
//#define INCREMENTAL_KILLSTATS
#define _SLERP_DEBUGGING

#ifdef _SLERP_DEBUGGING

#ifdef SYSTEM_TARGET_WINDOWS
	#include "windows.h"
	#include "mmsystem.h"
	#pragma comment( lib, "winmm.lib" )
#endif // SYSTEM_TARGET_WINDOWS

#endif // _SLERP_DEBUGGINGs


// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// local node address (has to be set on networking subsystem startup) ---------
//
node_t			LocalNode;


// local broadcast address (has to be set on networking subsystem startup) ----
//
node_t			LocalBroadcast;


// server node address (only valid for client/server protocols) ---------------
//
node_t			Server_Node;


// remote player node address table -------------------------------------------
//
node_t			Player_Node[ MAX_NET_ALLOC_SLOTS ];


// list where object control code inserts remote events (global rmev list) ----
//
char			REListMem[ RE_LIST_ALLOC_SIZE ];

// ----------------------------------------------------------------------------
//
#include "g_main_cl.h"
G_Extracted TheGameExtraction;

// force clean disconnect -----------------------------------------------------
//
void NET_ForceDisconnect()
{
	//NOTE:
	// this function is used by NETs_KillAPI() to disconnect
	// cleanly before killing the networking code.

	// if joined on server unjoin first
	if ( NetJoined ) {
		ObjCamOff();
		NETs_Unjoin( USER_EXIT );
	}

	// if connected perform disconnect
	if ( NetConnected == NETWORK_GAME_ON ) {
		NETs_Disconnect();
	}
}


// add local player to remote player table ------------------------------------
//
void NET_InitLocalPlayer()
{
	//NOTE:
	// for implementation efficiency and simplicity the
	// local player is added to the table of remote players.
	// to determine if a remote player is actually local
	// (e.g., to prevent sending a packet to oneself)
	// the shipid can be used, which is set to SHIPID_LOCALPLAYER.
	// the REMOTE_PLAYER_ACTIVE() macro determines if a player is
	// both active and actually remote.

	Player_Status[ LocalPlayerId ] = PLAYER_CONNECTED;

	Player_Ship[ LocalPlayerId ]   = MyShip;
	Player_ShipId[ LocalPlayerId ] = SHIPID_LOCALPLAYER;

	NETs_ResolveNode( &Player_Node[ LocalPlayerId ], &LocalNode );

	CopyRemoteName( Player_Name[ LocalPlayerId ], LocalPlayerName );
}

// init tables used to store info about remote players ------------------------
//
void NET_InitRemotePlayerTables()
{
	// start with assumption that no other players present
	LocalPlayerId = PLAYERID_ANONYMOUS;
	NumRemPlayers = 0;

	//NOTE:
	// just to be on the safe side, initialize all
	// MAX_NET_ALLOC_SLOTS slots, not just MAX_NET_PROTO_PLAYERS.

	// do initializations
	for ( int id = 0; id < MAX_NET_ALLOC_SLOTS; id++ ) {

		Player_Status[ id ]		  = PLAYER_INACTIVE;
		Player_Ship[ id ]		  = NULL;
		Player_ShipId[ id ]		  = SHIPID_NOSHIP;
		Player_AliveCounter[ id ] = 0;
		Player_UpToDate[ id ]	  = FALSE;
		Player_KillStat[ id ]	  = 0;
		Player_LastMsgId[ id ]	  = 0;
		Player_LastUpdateGameStateMsgId[ id ] = 0;
		Player_Name[ id ][ 0 ]	  = '\0';
		memset( &Player_Node[ id ], 0, sizeof( node_t ) );

#ifdef LINKED_PROTOCOL_GAMESERVER		
		if ( NET_ProtocolGMSV() ) {
			extern client_s	client_list[ MAX_NET_ALLOC_SLOTS ];
			client_list[ id ].Reset();
		}
#endif // LINKED_PROTOCOL_GAMESERVER

	}
}


// remove all remote players --------------------------------------------------
//
void NET_RemoveRemotePlayers()
{
	ASSERT( NetConnected );

	// process table
	for ( int id = 0; id < MAX_NET_PROTO_PLAYERS; id++ ) {

		// player must be active in order to be removed
		if ( Player_Status[ id ] == PLAYER_INACTIVE )
			continue;

		// don't remove local player
		if ( id == LocalPlayerId )
			continue;

		DBGTXT( MSGOUT( "NET_RemoveRemotePlayers(): removing player: %s (id %d).", Player_Name[ id ], id ); );
		DBGTXT( NETs_PrintNode( &Player_Node[ id ] ); );

		// if player is joined ship must be destroyed
		if ( Player_Status[ id ] == PLAYER_JOINED ) {

			DBGTXT( MSGOUT( "NET_RemoveRemotePlayers(): removing remote ship." ); );

			ASSERT( Player_ShipId[ id ] != SHIPID_LOCALPLAYER );
			ASSERT( Player_ShipId[ id ] != SHIPID_NOSHIP );
			KillSpecificObject( Player_ShipId[ id ], PShipObjects );

		} else {

			ASSERT( Player_Ship[ id ] == NULL );
			ASSERT( Player_ShipId[ id ] == SHIPID_NOSHIP );
		}

		// remove player from table
		Player_Status[ id ] = PLAYER_INACTIVE;
		Player_Ship[ id ]   = NULL;
		Player_ShipId[ id ] = SHIPID_NOSHIP;

//FIXME: DEMO_TEMP
// this assertion fired in
// one of the recorded demos.

		// decrease global number of remote players
//		ASSERT( NumRemPlayers > 1 );
		NumRemPlayers--;
	}
}


// materialize remote player with specified state -----------------------------
//
void NET_SetRemotePlayerState( int id, int status, int objclass, int killstat )
{
	ASSERT( NetConnected );

	//NOTE:
	// this function is used by CON_ACT::AcRemotePlayer() to
	// create a remote player in exactly the state he was in
	// when the "ac.remoteplayer" command had been saved.

	ASSERT( ( id >= 0 ) && ( id < MAX_NET_PROTO_PLAYERS ) );
	ASSERT( ( status == PLAYER_INACTIVE )  ||
			( status == PLAYER_CONNECTED ) ||
			( status == PLAYER_JOINED ) );
	ASSERT( objclass < NumObjClasses );
	// ( objclass < 0 ) explicitly allowed!
	ASSERT( killstat >= 0 );

	// check if active player is being overwritten
	if ( Player_Status[ id ] != PLAYER_INACTIVE ) {

		DBGTXT( MSGOUT( "NET_SetRemotePlayerState(): player %d already active.", id ); );

		if ( Player_ShipId[ id ] == SHIPID_LOCALPLAYER ) {

			DBGTXT( MSGOUT( "NET_SetRemotePlayerState(): active player %d was local.", id ); );

		} else if ( Player_Status[ id ] == PLAYER_JOINED ) {

			DBGTXT( MSGOUT( "NET_SetRemotePlayerState(): removing ship of player %d.", id ); );

			// kill ship object
			ShipObject *shippo = (ShipObject *) Player_Ship[ id ];
			ASSERT( shippo != NULL );
			if ( shippo != NULL )
				KillSpecificObject( Player_ShipId[ id ], PShipObjects );

		} else {

			ASSERT( Player_Ship[ id ] == NULL );
			ASSERT( Player_ShipId[ id ] == SHIPID_NOSHIP );
		}
	}

	// negative objclass denotes local player
	if ( objclass < 0 ) {

		DBGTXT( MSGOUT( "NET_SetRemotePlayerState(): changing local player id from %d to %d.", LocalPlayerId, id ); );
		DBGTXT( MSGOUT( "NET_SetRemotePlayerState(): local player status is %d.", status ); );

		LocalPlayerId		 = id;
		LocalPlayerName[ 0 ] = '\0';
		NumRemPlayers		 = -objclass;
		ASSERT( NumRemPlayers > 0 );

		// set player state
		Player_Status[ id ]    = status;
		Player_Ship[ id ]      = MyShip;
		Player_ShipId[ id ]    = SHIPID_LOCALPLAYER;
		Player_KillStat[ id ]  = killstat;
		Player_Name[ id ][ 0 ] = '\0';
		NETs_ResolveNode( &Player_Node[ id ], &LocalNode );

		// we also must reset the msg id's for the local player if LocalPlayerId == 0, because 
		// PKTP_CONNECT and PKTP_CONNECT_REPLY packets contain SendPlayerId 0.

		Player_LastMsgId[ id ]	  = 0;
		Player_LastUpdateGameStateMsgId[ id ] = 0;

		// restore join state
		NetJoined = ( status == PLAYER_JOINED );

		// restore entry-mode state
		int emstate = ( status == PLAYER_JOINED ) ? FALSE : TRUE;
		DBGTXT(
			if ( emstate != EntryMode )
				MSGOUT( "NET_SetRemotePlayerState(): changing entry-mode state to %d.", emstate );
		);
		EntryMode		= emstate;
		InFloatingMenu	= EntryMode ? FloatingMenu : FALSE;

	} else {

		// set player state
		Player_Status[ id ]		  = status;
		Player_Ship[ id ]		  = NULL;
		Player_ShipId[ id ]		  = SHIPID_NOSHIP;
		Player_AliveCounter[ id ] = MAX_ALIVE_COUNTER;
		Player_UpToDate[ id ]	  = FALSE;
		Player_KillStat[ id ]	  = killstat;
		Player_LastMsgId[ id ]	  = 0;
		Player_LastUpdateGameStateMsgId[ id ] = 0;
		Player_Name[ id ][ 0 ]	  = '\0';
		NETs_SetVirtualAddress( &Player_Node[ id ] );

		if ( status == PLAYER_JOINED ) {

			// default position (no other info available)
			Xmatrx objposition;
			MakeIdMatrx( objposition );

			// create ship object for player
			GenObject *objectpo = CreateObject( objclass, objposition );
			ASSERT( objectpo != NULL );
			objectpo->HostObjNumber = ShipHostObjId( id );
			Player_Ship[ id ]   = objectpo;
			Player_ShipId[ id ] = objectpo->ObjectNumber;

			DBGTXT( MSGOUT( "NET_SetRemotePlayerState(): materializing joined player %d.", id ); );

		} else if ( status == PLAYER_CONNECTED ) {

			DBGTXT( MSGOUT( "NET_SetRemotePlayerState(): materializing connected player %d.", id ); );
		}
	}
}


// set a remote player's position matrix --------------------------------------
//
void NET_SetRemotePlayerMatrix( int id, const Xmatrx matrix  )
{
	ASSERT( NetConnected );

	//NOTE:
	// this function is used by CON_ACT::AcRemoteMatrix() to set
	// the matrix of a remote player already created by
	// CON_ACT::AcRemotePlayer() (via NET_SetRemotePlayerState()).

	ASSERT( Player_Ship[ id ] != NULL );

	//NOTE:
	// this also works for the local player and is actually used thusly.

	// copy matrix into object
	if ( Player_Ship[ id ] != NULL ) {

		// copy over matrix without any further checks
		memcpy( Player_Ship[ id ]->ObjPosition, matrix, sizeof( Xmatrx ) );
	}
}


// set a remote player's name -------------------------------------------------
//
void NET_SetRemotePlayerName( int id, const char *name )
{
	ASSERT( NetConnected );

	//NOTE:
	// this function is used by CON_ACT::AcRemoteName() to set
	// the name of a remote player already created by
	// CON_ACT::AcRemotePlayer() (via NET_SetRemotePlayerState()).

	// store name in table
	CopyRemoteName( Player_Name[ id ], name );

	// additionally store name of local player in LocalPlayerName
	if ( Player_ShipId[ id ] == SHIPID_LOCALPLAYER )
		CopyRemoteName( LocalPlayerName, name );
}




// register prospective remote player (may fail if no slot available) ---------
//
int NET_RegisterRemotePlayer( int slotid, node_t *node, char *name )
{
	ASSERT( ( node != NULL ) || NET_ProtocolGMSV() );
	ASSERT( name != NULL );

	//NOTE:
	// NumRemPlayers must already include the player that
	// is being registered. it will not be updated here.

	ASSERT( NumRemPlayers > 1 );

	// register in specified slot
	if ( slotid != SLOTID_CONNECT_REFUSED ) {
	
		ASSERT( Player_Status[ slotid ] == PLAYER_INACTIVE );
		ASSERT( Player_Ship[ slotid ] == NULL );

		// accept connection
		DBGTXT( MSGOUT( "NET_RegisterRemotePlayer(): registering player %s with id %d.", name, slotid ); );
		if ( node != NULL )
			ADXTXT( NETs_PrintNode( node ); );

		// init remote player table entries
		Player_Status[ slotid ]		  = PLAYER_CONNECTED;
		if ( node != NULL ) {
			Player_Node[ slotid ] = *node;
		} else {
			memset( &Player_Node[ slotid ], 0, sizeof(node_t) );
		}

		Player_Ship[ slotid ]		  = NULL;
		Player_ShipId[ slotid ]		  = SHIPID_NOSHIP;
		Player_AliveCounter[ slotid ] = MAX_ALIVE_COUNTER;
		CopyRemoteName( Player_Name[ slotid ], name );

		// init up-to-date flag and killstats
		Player_UpToDate[ slotid ] = FALSE;
		Player_KillStat[ slotid ] = 0;

		// init msg ids 
		Player_LastMsgId[ slotid ]					= 0;
		Player_LastUpdateGameStateMsgId[ slotid ]	= 0;

		// give UI feedback that a player has connected
		TheGameExtraction.UI_PlayerConnectedFeedback( slotid ) ;

	} else {

		// refuse connection
		DBGTXT( MSGOUT( "NET_RegisterRemotePlayer(): rejecting player %s (no slot available).", name ); );
		ADXTXT( NETs_PrintNode( node ); );
	}

	return slotid;
}




// reset global remote event list ---------------------------------------------
//
void NET_RmEvListReset()
{
	ASSERT( RE_LIST_ALLOC_SIZE >= RE_LIST_MAXAVAIL );

	RE_List_Avail  = RE_LIST_MAXAVAIL;
	RE_List_CurPos = REListMem;
	( (RE_Header *) RE_List_CurPos )->RE_Type = RE_EMPTY;
}


// update global remote event list entries for lasers and missiles ------------
//
void NET_RmEvListUpdateLocations()
{
	ASSERT( NetConnected );

	// scan remote event list and update positions of lasers and missiles
	RE_Header *relist = (RE_Header *) REListMem;
	while ( relist->RE_Type != RE_EMPTY ) {

		if ( ( relist->RE_Type == RE_CREATELASER ) ||
			 ( relist->RE_Type == RE_CREATEMISSILE ) ) {

			UPDTXT(
				if ( relist->RE_Type == RE_CREATELASER ) {
					MSGOUT( "NET_RmEvListUpdateLocations(): updating laser position." );
				} else {
					MSGOUT( "NET_RmEvListUpdateLocations(): updating missile position." );
				}
			);

			GenObject *objectpo = ( relist->RE_Type == RE_CREATELASER ) ? (GenObject*)FetchFirstLaser() : (GenObject*)FetchFirstMissile();

			RE_CreateLaser *re_entry = (RE_CreateLaser *) relist;
			dword objnum = re_entry->HostObjId;
			while ( ( objectpo != NULL ) && ( objectpo->HostObjNumber != objnum ) ) {
				objectpo = objectpo->NextObj;
			}

			if ( objectpo == NULL ) {
				//NOTE:
				// in GMSV mode, laser-creations are always sent to the server.
				// a predicted collision, thus removal of the laser, is only visible to the local client
				if ( NET_ConnectedPEER() ) {
					re_entry->RE_Type = RE_DELETED;
				}
			} else {
				memcpy( &re_entry->ObjPosition, &objectpo->ObjPosition, sizeof( Xmatrx ) );
			}

		}

		byte relist_sz = NET_RmEvGetSize( relist );

		// advance to next event
		ASSERT( ( relist->RE_BlockSize == RE_BLOCKSIZE_INVALID ) ||
				( relist->RE_BlockSize == relist_sz ) );
		relist = (RE_Header *) ( (char *) relist + NET_RmEvGetSize( relist ) );
	}
}


// clear remote event list in packet ------------------------------------------
//
void NET_RmEvSingleClear( RE_Header* relist )
{
	ASSERT( relist != NULL );
	relist->RE_Type = RE_EMPTY;
}




// store local playername as RE_List head -------------------------------------
//
void NET_RmEvSinglePlayerName( RE_Header* relist )
{
	ASSERT( relist != NULL );

	RE_PlayerName *re_playername = (RE_PlayerName *) relist;
	re_playername->RE_Type		= RE_PLAYERNAME;
	re_playername->RE_BlockSize	= sizeof( RE_PlayerName );

	ASSERT( sizeof( RE_PlayerName ) <= RE_LIST_MAXAVAIL );

	// fill in local player's name
	CopyRemoteName( re_playername->PlayerName, LocalPlayerName );

	re_playername++;
	re_playername->RE_Type = RE_EMPTY;
}


// store local player and ship state as RE_List head --------------------------
//
void NET_RmEvSinglePlayerAndShipStatus( RE_Header* relist, word desired_player_state, int unjoin_flag, int killer )
{
	ASSERT( relist != NULL );

	RE_PlayerAndShipStatus* re_pas_status	= (RE_PlayerAndShipStatus*) relist;
	re_pas_status->RE_Type					= RE_PLAYERANDSHIPSTATUS;
	re_pas_status->RE_BlockSize				= sizeof( RE_PlayerAndShipStatus );

	re_pas_status->senderid		= LocalPlayerId;
	re_pas_status->objectindex  = ObjClassShipIndex[ MyShip->ObjectClass ];

	// set remote player's damage and speed from ship-object structure
	re_pas_status->CurDamage	= MyShip->CurDamage;
	re_pas_status->CurShield	= MyShip->CurShield;
	re_pas_status->CurSpeed		= MyShip->CurSpeed;

	// store current position of player
	memcpy( &re_pas_status->ObjPosition, &MyShip->ObjPosition, sizeof( Xmatrx ) );

	// store speeds for interpolation
	re_pas_status->CurYaw 		= CurYaw;
	re_pas_status->CurPitch		= CurPitch;
	re_pas_status->CurRoll		= CurRoll;
	re_pas_status->CurSlideHorz	= CurSlideHorz;
	re_pas_status->CurSlideVert	= CurSlideVert;

	// set desired player status
	re_pas_status->player_status = desired_player_state;

	// insert unjoin flag and killer
	re_pas_status->params[ 0 ] = unjoin_flag;
	//FIXME: do we really need the killer information in GMSV mode ( server should know this anyway )
	re_pas_status->params[ 2 ] = killer;

	// store the refframe for serverside movement checks
	re_pas_status->RefFrame = SYSs_GetRefFrameCount();

	re_pas_status++;
	re_pas_status->RE_Type = RE_EMPTY;
}

// store local player and ship state as RE_List head --------------------------
//
void NET_RmEvSinglePlayerStatus( RE_Header* relist, word desired_player_state, int unjoin_flag, int killer )
{
	ASSERT( relist != NULL );

	RE_PlayerStatus* re_playerstatus = (RE_PlayerStatus*) relist;
	re_playerstatus->RE_Type		 = RE_PLAYERSTATUS;
	re_playerstatus->RE_BlockSize	 = sizeof( RE_PlayerStatus );

	re_playerstatus->senderid	 = LocalPlayerId;
	re_playerstatus->objectindex = ObjClassShipIndex[ MyShip->ObjectClass ];

	// set desired player status
	re_playerstatus->player_status = desired_player_state;

	// insert unjoin flag and killer
	re_playerstatus->params[ 0 ] = unjoin_flag;
	//FIXME: do we really need the killer information in GMSV mode ( server should know this anyway )
	re_playerstatus->params[ 2 ] = killer;

	// store the refframe for serverside movement checks
	re_playerstatus->RefFrame = SYSs_GetRefFrameCount();

	re_playerstatus++;
	re_playerstatus->RE_Type = RE_EMPTY;
}



// remote player action tracking for interpolation ----------------------------
//
playerlerp_s	Player_Interpolation[ MAX_NET_ALLOC_SLOTS ];


// bit mask for determining whether orientation should be interpolated --------
//
#define SLERP_ORIENTATION	( AUX_ADVANCED_PLAYERINTERPOLATION & 0x02 )


// calc rotation matrix as result of slerp between two quaternions ------------
//
INLINE
void CalcSlerpedOrientation( GenObject *playership, playerlerp_s *playerlerp )
{
	ASSERT( playership != NULL );
	ASSERT( playerlerp != NULL );

	// determine whether slerp info is valid
	if ( playerlerp->curalpha == 0.0f )
		return;

	// advance slerp alpha
	playerlerp->curalpha += playerlerp->incalpha * CurScreenRefFrames;
	if ( playerlerp->curalpha > 1.0f ) {
		playerlerp->curalpha = 1.0f;
	}

	Xmatrx curbasematrix;
	CalcSlerpedMatrix( curbasematrix, playerlerp );

	// concat base with accumulated (yaw,pitch,roll)
	MtxMtxMULt( curbasematrix, playerlerp->dstposition, DestXmatrx );

	// set current orientation
	playership->ObjPosition[ 0 ][ 0 ] = DestXmatrx[ 0 ][ 0 ];
	playership->ObjPosition[ 0 ][ 1 ] = DestXmatrx[ 0 ][ 1 ];
	playership->ObjPosition[ 0 ][ 2 ] = DestXmatrx[ 0 ][ 2 ];
	playership->ObjPosition[ 1 ][ 0 ] = DestXmatrx[ 1 ][ 0 ];
	playership->ObjPosition[ 1 ][ 1 ] = DestXmatrx[ 1 ][ 1 ];
	playership->ObjPosition[ 1 ][ 2 ] = DestXmatrx[ 1 ][ 2 ];
	playership->ObjPosition[ 2 ][ 0 ] = DestXmatrx[ 2 ][ 0 ];
	playership->ObjPosition[ 2 ][ 1 ] = DestXmatrx[ 2 ][ 1 ];
	playership->ObjPosition[ 2 ][ 2 ] = DestXmatrx[ 2 ][ 2 ];
}

// interpolate remote player orientation --------------------------------------
//
PRIVATE
void InterpolateOrientation( GenObject *playership, playerlerp_s *playerlerp )
{
	ASSERT( playership != NULL );
	ASSERT( playerlerp != NULL );

	// check whether slerp in progress
	int slerpactive = SLERP_ORIENTATION && ( playerlerp->curalpha < 1.0f );

	// determine matrix for (yaw,pitch,roll) accumulation
	pXmatrx activematrix = slerpactive ? playerlerp->dstposition : playership->ObjPosition;

	// interpolate yaw
	bams_t yaw = playerlerp->curyaw * CurScreenRefFrames;
	ObjRotY( activematrix, yaw );

	// interpolate pitch
	bams_t pitch = playerlerp->curpitch * CurScreenRefFrames;
	ObjRotX( activematrix, pitch );

	// interpolate roll
	bams_t roll = playerlerp->curroll * CurScreenRefFrames;
	ObjRotZ( activematrix, roll );

	// slerp base orientation
	if ( slerpactive ) {
		CalcSlerpedOrientation( playership, playerlerp );
	}

#ifdef _SLERP_DEBUGGING
	//if ( playerlerp->curroll != BAMS_DEG0 ) {
		//DBGTXT( MSGOUT( "InterpolateOrientation(): %d", slerpactive ); );
		//DBGTXT( DumpMatrix( activematrix ); );
		//DBGTXT( MSGOUT( "%d: PREDICTING: phi: %f", timeGetTime(), RAD_TO_DEG( acos( activematrix[ 0 ][ 0 ] ) ) ); );
	//}
#endif // _SLERP_DEBUGGING
}


// interpolate actions of remote player (motion) ------------------------------
//
void NET_InterpolatePlayer( int playerid )
{
	ASSERT( NetConnected );
	ASSERT( ( playerid >= 0 ) && ( playerid < MAX_NET_PROTO_PLAYERS ) );
	ASSERT( playerid != LocalPlayerId );

	ASSERT( Player_Status[ playerid ] == PLAYER_JOINED );
	ASSERT( Player_ShipId[ playerid ] != SHIPID_NOSHIP );
	ASSERT( Player_ShipId[ playerid ] != SHIPID_LOCALPLAYER );

	// disable interpolation
//	if ( !AUX_ADVANCED_PLAYERINTERPOLATION )
//		return;

	GenObject *playership = Player_Ship[ playerid ];
	ASSERT( playership != NULL );
	playerlerp_s *playerlerp = &Player_Interpolation[ playerid ];

	// interpolate orientation
	InterpolateOrientation( playership, playerlerp );

	// check whether transition in progress
	if ( playerlerp->transition == 0 ) {

		// calculate the movement
		Vector3 movement_offset;
		CalcMovement( &movement_offset, 
					playership->ObjPosition, 
					playerlerp->curspeed,
					playerlerp->curslidehorz, 
					playerlerp->curslidevert,
					CurScreenRefFrames);

		// apply movement
		playership->ObjPosition[ 0 ][ 3 ] += movement_offset.X;
		playership->ObjPosition[ 1 ][ 3 ] += movement_offset.Y;
		playership->ObjPosition[ 2 ][ 3 ] += movement_offset.Z;

	} else {

		ASSERT( playerlerp->transition > 0 );

		playerlerp->transition -= CurScreenRefFrames;
		if ( playerlerp->transition <= 0 ) {

			// after transition assume destination position
			playership->ObjPosition[ 0 ][ 3 ] = playerlerp->dstposition[ 0 ][ 3 ];
			playership->ObjPosition[ 1 ][ 3 ] = playerlerp->dstposition[ 1 ][ 3 ];
			playership->ObjPosition[ 2 ][ 3 ] = playerlerp->dstposition[ 2 ][ 3 ];

			// remaining frametime: advance in direction of flight
			int remainder = -playerlerp->transition;

			// calculate the movement
			Vector3 movement_offset;
			CalcMovement( &movement_offset, 
						playership->ObjPosition, 
						playerlerp->curspeed, 
						playerlerp->curslidehorz, 
						playerlerp->curslidevert, 
						remainder );

			playership->ObjPosition[ 0 ][ 3 ] += movement_offset.X;
			playership->ObjPosition[ 1 ][ 3 ] += movement_offset.Y;
			playership->ObjPosition[ 2 ][ 3 ] += movement_offset.Z;

			// turn transition off
			playerlerp->transition = 0;

		} else {

			// interpolate position transition (along transition vector)
			playership->ObjPosition[ 0 ][ 3 ] += playerlerp->transvec_x * CurScreenRefFrames;
			playership->ObjPosition[ 1 ][ 3 ] += playerlerp->transvec_y * CurScreenRefFrames;
			playership->ObjPosition[ 2 ][ 3 ] += playerlerp->transvec_z * CurScreenRefFrames;
		}
	}
}


// execute remote info about ship (as contained in GAME_STATE packet) ---------
//
void NET_ExecShipRemInfo( ShipRemInfo *info, ShipObject *shippo, int senderid )
{
	ASSERT( NetConnected );
	ASSERT( info != NULL );
	ASSERT( shippo != NULL );
	ASSERT( ( senderid >= 0 ) && ( senderid < MAX_NET_PROTO_PLAYERS ) );
	ASSERT( senderid != LocalPlayerId );

	// update remote player's damage and speed in ship-object structure
	shippo->CurDamage     = info->CurDamage;
	shippo->CurShield     = info->CurShield;
	shippo->CurSpeed      = info->CurSpeed;
	shippo->NumMissls     = info->NumMissls;
	shippo->NumHomMissls  = info->NumHomMissls;
	shippo->NumMines      = info->NumMines;
    shippo->NumPartMissls = info->NumPartMissls;
	// remember speeds (linear/angular) for interpolation
	playerlerp_s *playerlerp = &Player_Interpolation[ senderid ];
	playerlerp->curspeed	 = info->CurSpeed;
	playerlerp->curyaw		 = info->CurYaw;
	playerlerp->curpitch	 = info->CurPitch;
	playerlerp->curroll		 = info->CurRoll;
	playerlerp->curslidehorz = info->CurSlideHorz;
	playerlerp->curslidevert = info->CurSlideVert;

#ifdef _SLERP_DEBUGGING
	//if ( playerlerp->curroll != BAMS_DEG0 ) {
		//DBGTXT( MSGOUT( "%d: NEW_REMOTE_STATE: phi: %f", timeGetTime(), RAD_TO_DEG( acos( info->ObjPosition[ 0 ][ 0 ] ) ) ); );
	//}
#endif // _SLERP_DEBUGGING

	// set new position of remote player
	if ( AUX_ADVANCED_PLAYERINTERPOLATION ) {

		#define DEFAULT_PLAYERLERP_TRANSITION_TIME	200

		if ( AUXDATA_PLAYERLERP_TRANSITION_TIME == 0 ) {
			playerlerp->transition = DEFAULT_PLAYERLERP_TRANSITION_TIME;
		} else {
			playerlerp->transition = AUXDATA_PLAYERLERP_TRANSITION_TIME;
		}

		// remember destination position
		memcpy( playerlerp->dstposition, info->ObjPosition, sizeof( Xmatrx ) );

		// position in last (prior to this) frame
		Vector3 oldpos;
		FetchTVector( shippo->ObjPosition, &oldpos );

		// destination (current) position as contained in packet
		Vector3 newpos;
		FetchTVector( info->ObjPosition, &newpos );

		// calculate the movement
		Vector3 movement_offset;
		CalcMovement( &movement_offset, 
					info->ObjPosition, 
					info->CurSpeed, 
					info->CurSlideHorz, 
					info->CurSlideVert,
					(refframe_t)playerlerp->transition);

		// predict where destination position will be after transition time
		newpos.X += movement_offset.X;
		newpos.Y += movement_offset.Y;
		newpos.Z += movement_offset.Z;

		// store corrected (predicted) destination
		playerlerp->dstposition[ 0 ][ 3 ] = newpos.X;
		playerlerp->dstposition[ 1 ][ 3 ] = newpos.Y;
		playerlerp->dstposition[ 2 ][ 3 ] = newpos.Z;

		// account for this frame's step
		playerlerp->transition += CurScreenRefFrames;

		// determine one step on path from current to destination position
		float normfac = 1.0f / playerlerp->transition;
		playerlerp->transvec_x = FLOAT_TO_GEOMV( GEOMV_TO_FLOAT( newpos.X - oldpos.X ) * normfac );
		playerlerp->transvec_y = FLOAT_TO_GEOMV( GEOMV_TO_FLOAT( newpos.Y - oldpos.Y ) * normfac );
		playerlerp->transvec_z = FLOAT_TO_GEOMV( GEOMV_TO_FLOAT( newpos.Z - oldpos.Z ) * normfac );

		playerlerp->transition -= CurScreenRefFrames;

		if ( playerlerp->transition <= 0 ) {

			playerlerp->transition = 0;
			memcpy( shippo->ObjPosition, info->ObjPosition, sizeof( Xmatrx ) );

		} else {

			if ( SLERP_ORIENTATION ) {

				// orientation in previous frame
				Quaternion srcquat;
				QuaternionFromMatrx( &srcquat, shippo->ObjPosition );
				QuaternionMakeUnit( &srcquat );

				// orientation state from packet
				Quaternion dstquat;
				QuaternionFromMatrx( &dstquat, info->ObjPosition );
				QuaternionMakeUnit( &dstquat );

				// stepsize currently bound to poslerp speed
				//FIXME: where does this constant come from 
				float stepsize = normfac; //* 1.2f;

				playerlerp->srcquat  = srcquat;
				playerlerp->dstquat  = dstquat;
				playerlerp->incalpha = stepsize;
				playerlerp->curalpha = stepsize * CurScreenRefFrames;

				if ( playerlerp->curalpha > 1.0f ) {
					playerlerp->curalpha = 1.0f;
				}

				// fill in first slerped orientation
				CalcSlerpedMatrix( shippo->ObjPosition, playerlerp );

				// start with identity for (yaw,pitch,roll) accumulation
				playerlerp->dstposition[ 0 ][ 0 ] = GEOMV_1;
				playerlerp->dstposition[ 0 ][ 1 ] = GEOMV_0;
				playerlerp->dstposition[ 0 ][ 2 ] = GEOMV_0;
				playerlerp->dstposition[ 1 ][ 0 ] = GEOMV_0;
				playerlerp->dstposition[ 1 ][ 1 ] = GEOMV_1;
				playerlerp->dstposition[ 1 ][ 2 ] = GEOMV_0;
				playerlerp->dstposition[ 2 ][ 0 ] = GEOMV_0;
				playerlerp->dstposition[ 2 ][ 1 ] = GEOMV_0;
				playerlerp->dstposition[ 2 ][ 2 ] = GEOMV_1;

			} else {

				// simply assume new orientation
				shippo->ObjPosition[ 0 ][ 0 ] = info->ObjPosition[ 0 ][ 0 ];
				shippo->ObjPosition[ 0 ][ 1 ] = info->ObjPosition[ 0 ][ 1 ];
				shippo->ObjPosition[ 0 ][ 2 ] = info->ObjPosition[ 0 ][ 2 ];
				shippo->ObjPosition[ 1 ][ 0 ] = info->ObjPosition[ 1 ][ 0 ];
				shippo->ObjPosition[ 1 ][ 1 ] = info->ObjPosition[ 1 ][ 1 ];
				shippo->ObjPosition[ 1 ][ 2 ] = info->ObjPosition[ 1 ][ 2 ];
				shippo->ObjPosition[ 2 ][ 0 ] = info->ObjPosition[ 2 ][ 0 ];
				shippo->ObjPosition[ 2 ][ 1 ] = info->ObjPosition[ 2 ][ 1 ];
				shippo->ObjPosition[ 2 ][ 2 ] = info->ObjPosition[ 2 ][ 2 ];
			}

			// advance first step in position
			shippo->ObjPosition[ 0 ][ 3 ] += playerlerp->transvec_x * CurScreenRefFrames;
			shippo->ObjPosition[ 1 ][ 3 ] += playerlerp->transvec_y * CurScreenRefFrames;
			shippo->ObjPosition[ 2 ][ 3 ] += playerlerp->transvec_z * CurScreenRefFrames;
		}

	} else {

		// simply set ship to new position (no smooth transition)
		playerlerp->transition = 0;
		memcpy( shippo->ObjPosition, info->ObjPosition, sizeof( Xmatrx ) );
	}
}


// fill ShipRemInfo structure for specified ship ------------------------------
//
void NET_FillShipRemInfo( ShipRemInfo *info, ShipObject *shippo, int playerid )
{
	ASSERT( NetConnected );
	ASSERT( info != NULL );
	ASSERT( shippo != NULL );
	ASSERT( ( playerid >= 0 ) && ( playerid < MAX_NET_PROTO_PLAYERS ) );

	// set remote player's damage and speed from ship-object structure
	info->CurDamage	= shippo->CurDamage;
	info->CurShield	= shippo->CurShield;
	info->CurSpeed	= shippo->CurSpeed;

	// store current position of player
	memcpy( &info->ObjPosition, &shippo->ObjPosition, sizeof( Xmatrx ) );

	// store speeds for interpolation
	if ( shippo == MyShip ) {
		info->CurYaw 		= CurYaw;
		info->CurPitch		= CurPitch;
		info->CurRoll		= CurRoll;
		info->CurSlideHorz	= CurSlideHorz;
		info->CurSlideVert	= CurSlideVert;
	} else {
		info->CurYaw 		= Player_Interpolation[ playerid ].curyaw;
		info->CurPitch		= Player_Interpolation[ playerid ].curpitch;
		info->CurRoll		= Player_Interpolation[ playerid ].curroll;
		info->CurSlideHorz	= Player_Interpolation[ playerid ].curslidehorz;
		info->CurSlideVert	= Player_Interpolation[ playerid ].curslidevert;
	}
}


// if this is enabled the kill stats will not be updated from packets ---------
//
int kill_stats_frozen = FALSE;


// freeze kill stats in order to allow clean reset everywhere -----------------
//
void NET_FreezeKillStats( int freeze )
{
	kill_stats_frozen = freeze;
}





// update tables and create ship for newly joined player ----------------------
//
PRIVATE
void PerformJoin( RE_PlayerAndShipStatus* pas_status )
{
	ASSERT( NetConnected );
	ASSERT( pas_status != NULL );
	ASSERT( pas_status->player_status == PLAYER_JOINED );

	int nClientID = pas_status->senderid;
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NET_PROTO_PLAYERS ) );
	ASSERT( nClientID != LocalPlayerId );

	// there must not be a ship for this player
	ASSERT( Player_Ship  [ nClientID ] == NULL );
	ASSERT( Player_ShipId[ nClientID ] == SHIPID_NOSHIP );

	// update player status
	Player_Status[ nClientID ] = PLAYER_JOINED;

	// create ship object for new player
	dword objclass = ShipClasses[ pas_status->objectindex ];
	ASSERT( objclass != CLASS_ID_INVALID );

	GenObject *objectpo = CreateObject( objclass, pas_status->ObjPosition );
	ASSERT( objectpo != NULL );

	objectpo->HostObjNumber   = ShipHostObjId( nClientID );
	Player_Ship  [ nClientID ] = objectpo;
	Player_ShipId[ nClientID ] = objectpo->ObjectNumber;
	
	// reset target if locked onto newly joined player
	if ( TargetObjNumber == objectpo->HostObjNumber ) {
		
		TargetObjNumber = TARGETID_NO_TARGET;
		TargetVisible	= FALSE;
		
		//NOTE:
		// this caused a very subtle bug when a player rejoined
		// before the explosion animation of his old ship reached
		// the stage where the target would be removed. the target
		// would then jump from the old ship to the new ship, possibly
		// also causing a problem with target caret drawing when the
		// new ship joined behind the local player.
	}
	
	// display object id of remote player's ship
	if ( AUX_DISPLAY_REMOTE_JOIN_OBJECT_ID ) {
		MSGOUT( "player %s joined and got object id %d.", Player_Name[ nClientID ], Player_ShipId[ nClientID ] );
	}
	
	// give UI feedback when player has joined
	TheGameExtraction.UI_PlayerJoinedFeedback( (RE_PlayerStatus*)pas_status ) ;

}

// update tables and delete ship of player who unjoined -----------------------
//
PRIVATE
void PerformUnjoin( RE_PlayerAndShipStatus* pas_status )
{
	ASSERT( NetConnected );
	ASSERT( pas_status != NULL );
	
	int nClientID = pas_status->senderid;
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NET_PROTO_PLAYERS ) );
	ASSERT( nClientID != LocalPlayerId );

	// check desired player status
	ASSERT( pas_status->player_status == PLAYER_CONNECTED );
	
	// there has to be a ship for this player
	ShipObject *shippo = (ShipObject *) Player_Ship[ nClientID ];
	ASSERT( shippo != NULL );
	
	ASSERT( Player_ShipId[ nClientID ] != SHIPID_NOSHIP );
	ASSERT( Player_ShipId[ nClientID ] != SHIPID_LOCALPLAYER );
	
	if ( shippo != NULL ) {
		
		// downing of ship starts explosion
		if ( pas_status->params[ 0 ] == SHIP_DOWNED ) {
			
			// exec gamecode when player ship is downed
			TheGameExtraction.GC_UnjoinPlayer_ShipDowned(	pas_status->senderid,
													pas_status->params[ 2 ] - KILLERID_BIAS,
													pas_status->params[ 3 ] ) ;
			
			// user exit opens stargate and deletes ship
		} else if ( pas_status->params[ 0 ] == USER_EXIT ) {
			
			// exec gamecode when player exits to menu
			TheGameExtraction.GC_UnjoinPlayer_UserExit( (RE_PlayerStatus*)pas_status ) ;
		}
	}
	
	// signify that player is only connected, but not joined.
	Player_Status[ nClientID ] = PLAYER_CONNECTED;
	Player_Ship  [ nClientID ] = NULL;
	Player_ShipId[ nClientID ] = SHIPID_NOSHIP;
	
	// give UI feedback when player has unjoined
	TheGameExtraction.UI_PlayerUnjoinedFeedback( (RE_PlayerStatus*)pas_status ) ;
}

// set the desired player status ( inactive/joined/unjoined ) -----------------
//
void NET_SetDesiredPlayerStatus( RE_PlayerAndShipStatus* pas_status )
{
	ASSERT( NetConnected );
	ASSERT( pas_status != NULL );
	
	// identify sender
	int nClientID = pas_status->senderid;
	ASSERT( ( nClientID >= 0 ) && ( nClientID < MAX_NET_PROTO_PLAYERS ) );
	ASSERT( nClientID != LocalPlayerId );

	switch( pas_status->player_status ) {
		// only join/unjoin allowed
		case PLAYER_INACTIVE:
			{
				ASSERT( FALSE );
			}
			break;
		// desired player_status is CONNECTED ( unjoin/ensure unjoined )
		case PLAYER_CONNECTED:
			{
				// player must be joined in order to unjoin
				if ( Player_Status[ nClientID ] == PLAYER_CONNECTED ) {
					return;
				}
				
				// player must be connected in order to unjoin
				if ( Player_Status[ nClientID ] == PLAYER_INACTIVE ) {
					DBGTXT( MSGOUT( "NET_SetDesiredPlayerStatus(): filtering unjoin for inactive player: %d.", nClientID ); );
					return;
				}

				// player status must be JOINED
				ASSERT( Player_Status[ nClientID ] == PLAYER_JOINED );

				// update tables and delete ship
				PerformUnjoin( pas_status );

				// player status must now be CONNECTED
				ASSERT( Player_Status[ nClientID ] == PLAYER_CONNECTED );
				
				DBGTXT( MSGOUT( "NET_SetDesiredPlayerStatus(): unjoined player %d.", nClientID ); );
				
			}
			break;
		// desired player_status is JOINED ( join/ensure joined )
		case PLAYER_JOINED:
			{
				// player must not be joined already in order to join
				if ( Player_Status[ nClientID ] == PLAYER_JOINED ) {
					return;
				}
				
				// player must be connected in order to join
				if ( Player_Status[ nClientID ] == PLAYER_INACTIVE ) {
					DBGTXT( MSGOUT( "NET_SetDesiredPlayerStatus	(): filtering join for inactive player: %d.", nClientID ); );
					return;
				}

				// player status must be CONNECTED
				ASSERT( Player_Status[ nClientID ] == PLAYER_CONNECTED );

				// update tables and create ship
				PerformJoin( pas_status );

				// player status must now be JOINED 
				ASSERT( Player_Status[ nClientID ] == PLAYER_JOINED );

				DBGTXT( MSGOUT( "NET_SetDesiredPlayerStatus(): joined player %d.", nClientID ); );
			}
			break;
		default:
			ASSERT( FALSE );
			break;
	}
}


// remote player has disconnected (left universe) -----------------------------
//
void NET_PlayerDisconnected( int playerid )
{
	ASSERT( NetConnected );
	ASSERT( ( playerid >= 0 ) && ( playerid < MAX_NET_PROTO_PLAYERS ) );

	// player must be connected in order to disconnect
	if ( Player_Status[ playerid ] == PLAYER_INACTIVE ) {
		DBGTXT( MSGOUT( "NET_PlayerDisconnected(): filtering disconnect for inactive player: %d.", playerid ); );
		return;
	}

	// player must not be joined in order to disconnect
	if ( Player_Status[ playerid ] != PLAYER_CONNECTED ) {
		DBGTXT( MSGOUT( "NET_PlayerDisconnected(): filtering disconnect for joined player: %d.", playerid ); );
		return;
	}

	// set player inactive
	Player_Status[ playerid ] = PLAYER_INACTIVE;
	ASSERT( Player_Ship[ playerid ] == NULL );
	ASSERT( Player_ShipId[ playerid ] == SHIPID_NOSHIP );

	// decrease global number of remote players
	NumRemPlayers--;

	// give UI feedback when player disconnects
	TheGameExtraction.UI_PlayerDisconnectedFeedback( playerid ) ;

	DBGTXT( MSGOUT( "NET_PlayerDisconnected(): player with id %d has disconnected.", playerid ); );
}


// remove player if he didn't send a packet in too long a time ----------------
//
void NET_RemovePlayer( int playerid )
{
	ASSERT( NetConnected );
	ASSERT( ( playerid >= 0 ) && ( playerid < MAX_NET_PROTO_PLAYERS ) );

	// player must be active in order to be removed
	if ( Player_Status[ playerid ] == PLAYER_INACTIVE ) {
		DBGTXT( MSGOUT( "NET_RemovePlayer(): filtering redundant removement of inactive player: %d.", playerid ); );
		return;
	}

	ASSERT( REMOTE_PLAYER_ACTIVE( playerid ) );
	if ( playerid == LocalPlayerId ) {
		DBGTXT( MSGOUT( "NET_RemovePlayer(): filtering removement of local player: %d.", playerid ); );
		return;
	}

	DBGTXT( MSGOUT( "NET_RemovePlayer(): removing player: %s (id %d).", Player_Name[ playerid ], playerid ); );
	DBGTXT( NETs_PrintNode( &Player_Node[ playerid ] ); );

	// if player is joined ship must be destroyed
	if ( Player_Status[ playerid ] == PLAYER_JOINED ) {

		DBGTXT( MSGOUT( "NET_RemovePlayer(): removing remote ship." ); );

		ASSERT( Player_ShipId[ playerid ] != SHIPID_LOCALPLAYER );
		ASSERT( Player_ShipId[ playerid ] != SHIPID_NOSHIP );
		KillSpecificObject( Player_ShipId[ playerid ], PShipObjects );

	} else {

		ASSERT( Player_Ship[ playerid ] == NULL );
		ASSERT( Player_ShipId[ playerid ] == SHIPID_NOSHIP );
	}

	// remove player from table
	Player_Status[ playerid ] = PLAYER_INACTIVE;
	Player_Ship[ playerid ]   = NULL;
	Player_ShipId[ playerid ] = SHIPID_NOSHIP;

	// decrease global number of remote players
	ASSERT( NumRemPlayers > 1 );
	NumRemPlayers--;

	// give UI feedback when player is removed
	TheGameExtraction.UI_PlayerWasRemovedFeedback( playerid ) ;
}



