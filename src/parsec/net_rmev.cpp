/*
 * PARSEC - Remote Events
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:40 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001-2002
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
#include "linkinfo.h"
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
#include "net_rmev.h"

// proprietary module headers
#include "con_aux.h"
#include "con_say.h"
#include "h_supp.h"
#include "net_game.h"
#include "obj_creg.h"
#include "obj_ctrl.h"
#include "obj_xtra.h"
#include "g_sfx.h"
#include "g_wfx.h"
#include "g_emp.h"
#include "g_swarm.h"



// string scratchpad
#define PASTE_STR_LEN 1023
static char	paste_str[ PASTE_STR_LEN + 1 ];

// external flag
int name_change_notification = TRUE;



// table of remote event sizes ------------------------------------------------
//
static size_t re_sizes[] = {

	sizeof( RE_Header ),
	sizeof( RE_Header ),
	sizeof( RE_CreateObject ),
	sizeof( RE_CreateLaser ),
	sizeof( RE_CreateMissile ),
	sizeof( RE_CreateExtra ),
    sizeof( RE_KillObject ),
	sizeof( RE_SendText ),
	sizeof( RE_PlayerName ),
	sizeof( RE_ParticleObject ),
	sizeof( RE_PlayerList ),
	sizeof( RE_ConnectQueue ),
	sizeof( RE_WeaponState ),
	sizeof( RE_StateSync ),
	sizeof( RE_CreateSwarm ),
	sizeof( RE_CreateEmp ),
	sizeof( RE_OwnerSection ),
	sizeof( RE_PlayerStatus ),
	sizeof( RE_PlayerAndShipStatus ),
	sizeof( RE_KillStats ),
	sizeof( RE_GameState ),
	sizeof( RE_CommandInfo ),
	sizeof( RE_ClientInfo ),
	sizeof( RE_CreateExtra2 ),
	sizeof( RE_IPv4ServerInfo ),
	sizeof( RE_ServerLinkInfo ),
	sizeof( RE_MapObject ),
	sizeof( RE_Stargate ),
    sizeof( RE_CreateMine ),
};


// check if enough space in RE_List for specified remote event ----------------
//
int NET_RmEvAllowed( int re_type )
{
	if ( NetConnected != NETWORK_GAME_ON )
		return TRUE;

	ASSERT( re_type > RE_DELETED );
	ASSERT( re_type < RE_NUMEVENTS );
	ASSERT( re_type != RE_SENDTEXT );
	ASSERT( re_type != RE_PLAYERLIST );
	ASSERT( re_type != RE_CONNECTQUEUE );
	ASSERT( re_type != RE_OWNERSECTION );

	if ( NetConnected && ( (size_t)RE_List_Avail < re_sizes[ re_type ] ) )
		return FALSE;

	//NOTE:
	// if the connection is down the check need not be performed since
	// all insertion functions will skip the insertion in that case.

	return TRUE;
}


// determine size of remote event ---------------------------------------------
//
size_t NET_RmEvGetSize( RE_Header *relist )
{
	ASSERT( relist != NULL );
	byte retype = relist->RE_Type;

	// use stored size for remote-events of variable size
	if ( ( retype == RE_DELETED ) || ( retype == RE_SENDTEXT ) || ( retype == RE_COMMANDINFO  ) ) {
		ASSERT( relist->RE_BlockSize != RE_BLOCKSIZE_INVALID );
		return relist->RE_BlockSize;
	}

	ASSERT( retype > RE_DELETED );
	ASSERT( retype < RE_NUMEVENTS );

	return re_sizes[ retype ];
}


// insert playername into RE_List ---------------------------------------------
//
int NET_RmEvPlayerName( const char *name )
{
	//NOTE:
	// calling NET_RmEvAllowed() is NOT mandatory
	// before calling this function.

	ASSERT( name != NULL );

	if ( NetConnected == NETWORK_GAME_ON ) {

		// this is no fault condition
		if ( (size_t)RE_List_Avail < sizeof( RE_PlayerName ) )
			return 0;
		
		RE_PlayerName *re_playername = (RE_PlayerName *) RE_List_CurPos;
		re_playername->RE_Type 	     = RE_PLAYERNAME;
		re_playername->RE_BlockSize  = sizeof( RE_PlayerName );
		CopyRemoteName( re_playername->PlayerName, name );
		
		re_playername++;
		re_playername->RE_Type		 = RE_EMPTY;
		
		RE_List_CurPos = (char *) re_playername;
		RE_List_Avail -= sizeof( RE_PlayerName );

		// this has to be done here although it is also done
		// by NET_UTIL::NET_SetPlayerName() later on, because
		// NET_ServerUpdateName() already needs the new name.
		strcpy( Player_Name[ LocalPlayerId ], name );

		// update name on server if playing with a server-based protocol
		if ( !NETs_UpdateName() ) {

			// restore old name if name change on server
			// not possible or forbidden. PlayerName has
			// not been altered by NETs_UpdateName()!
			strcpy( Player_Name[ LocalPlayerId ], LocalPlayerName );
			return 0;
		}
	}

	return 1;
}


// insert text to send into RE_List -------------------------------------------
//
int NET_RmEvSendText( const char *text )
{
	//NOTE:
	// calling NET_RmEvAllowed() is NOT mandatory
	// before calling this function.

	//NOTE:
	// the maximum transmitted text-length is 252.
	// (excluding the terminating zero-byte.)

	ASSERT( text != NULL );
	if(strlen(text) == 0)
		return 1;
	if ( NetConnected == NETWORK_GAME_ON ) {

		// make copy of text
		strncpy( paste_str, LocalPlayerName, MAX_PLAYER_NAME);
        strncat( paste_str," ",1);
        strncat( paste_str, text, (PASTE_STR_LEN - MAX_PLAYER_NAME - 1));
		paste_str[ PASTE_STR_LEN ] = 0;

		int textlen  = strlen( text ) + strlen(LocalPlayerName) + 1; //Name + space(1) + text
		int	blocksiz = sizeof( RE_SendText ) + textlen;
		// terminating zero-byte is included in sizeof( RE_SendText )

        // this is no fault condition
		if ( RE_List_Avail < blocksiz )
			return 0;

        // cut text if blocksize doesn't fit in one byte
		ASSERT( blocksiz > 2 );
		if ( blocksiz > 255 ) {
			blocksiz = 255;
			paste_str[ 255 - sizeof( RE_SendText ) ] = 0;
		}

		RE_SendText *re_sendtext  = (RE_SendText *) RE_List_CurPos;
		re_sendtext->RE_Type 	  = RE_SENDTEXT;
		re_sendtext->RE_BlockSize = blocksiz;
		strcpy( re_sendtext->TextStart, paste_str );

		re_sendtext = (RE_SendText *) ( (char *)re_sendtext + blocksiz );
		re_sendtext->RE_Type	  = RE_EMPTY;
        RE_List_CurPos = (char *) re_sendtext;
		RE_List_Avail -= blocksiz;
	}
    
	return 1;
}


// insert generic object into RE_List -----------------------------------------
//
int NET_RmEvObject( const GenObject *objpo )
{
	//NOTE:
	// calling NET_RmEvAllowed() is mandatory
	// before calling this function.

	ASSERT( objpo != NULL );

	if ( NetConnected == NETWORK_GAME_ON ) {

		if ( (size_t)RE_List_Avail < sizeof( RE_CreateObject ) ) {
			ASSERT( 0 );
			return 0;
		}

		RE_CreateObject *re_createobject = (RE_CreateObject *) RE_List_CurPos;
		re_createobject->RE_Type		 = RE_CREATEOBJECT;
		re_createobject->RE_BlockSize    = sizeof( RE_CreateObject );
		re_createobject->ObjectClass     = objpo->ObjectClass;
		re_createobject->HostObjId       = objpo->HostObjNumber;
		memcpy( &re_createobject->ObjPosition, &objpo->ObjPosition, sizeof( Xmatrx ) );
		re_createobject->Flags           = 0x00000000;

		re_createobject++;
		re_createobject->RE_Type 	     = RE_EMPTY;

		RE_List_CurPos = (char *) re_createobject;
		RE_List_Avail -= sizeof( RE_CreateObject );
	}

	return 1;
}


// insert laser into RE_List --------------------------------------------------
//
int NET_RmEvLaser( const LaserObject *laserpo )
{
	//NOTE:
	// calling NET_RmEvAllowed() is mandatory
	// before calling this function.

	ASSERT( laserpo != NULL );

	if ( NetConnected == NETWORK_GAME_ON ) {

		if ( (size_t)RE_List_Avail < sizeof( RE_CreateLaser ) ) {
			ASSERT( 0 );
			return 0;
		}

		RE_CreateLaser *re_createlaser = (RE_CreateLaser *) RE_List_CurPos;
		re_createlaser->RE_Type		   = RE_CREATELASER;
		re_createlaser->RE_BlockSize   = sizeof( RE_CreateLaser );
		re_createlaser->ObjectClass    = laserpo->ObjectClass;
		re_createlaser->HostObjId	   = laserpo->HostObjNumber;
		memcpy( &re_createlaser->ObjPosition, &laserpo->ObjPosition, sizeof( Xmatrx ) );
		re_createlaser->DirectionVec   = laserpo->DirectionVec;

		re_createlaser++;
		re_createlaser->RE_Type 	   = RE_EMPTY;

		RE_List_CurPos = (char *) re_createlaser;
		RE_List_Avail -= sizeof( RE_CreateLaser );
	}

	return 1;
}


// insert missile into RE_List ------------------------------------------------
//
int NET_RmEvMissile( const MissileObject *missilepo, dword targetobjid )
{
	//NOTE:
	// calling NET_RmEvAllowed() is mandatory
	// before calling this function.

	ASSERT( missilepo != NULL );

	if ( NetConnected == NETWORK_GAME_ON ) {

		if ( (size_t)RE_List_Avail < sizeof( RE_CreateMissile ) ) {
			ASSERT( 0 );
			return 0;
		}

		RE_CreateMissile *re_createmissl = (RE_CreateMissile *) RE_List_CurPos;
		re_createmissl->RE_Type 		 = RE_CREATEMISSILE;
		re_createmissl->RE_BlockSize	 = sizeof( RE_CreateMissile );
		re_createmissl->ObjectClass 	 = missilepo->ObjectClass;
		re_createmissl->HostObjId		 = missilepo->HostObjNumber;
		re_createmissl->TargetHostObjId  = targetobjid;
		memcpy( &re_createmissl->ObjPosition, &missilepo->ObjPosition, sizeof( Xmatrx ) );
		re_createmissl->DirectionVec	 = missilepo->DirectionVec;

		re_createmissl++;
		re_createmissl->RE_Type 		 = RE_EMPTY;

		RE_List_CurPos = (char *) re_createmissl;
		RE_List_Avail -= sizeof( RE_CreateMissile );
	}

	return 1;
}

// insert extra into RE_List --------------------------------------------------
//
int NET_RmEvMine( const ExtraObject *extrapo )
{
	//NOTE:
	// calling NET_RmEvAllowed() is mandatory
	// before calling this function.
    
	ASSERT( extrapo != NULL );
    
    //	DSEXC( return 1 );
    
	if ( NetConnected == NETWORK_GAME_ON ) {
        
		if ( (size_t)RE_List_Avail < sizeof( RE_CreateExtra ) ) {
			ASSERT( 0 );
			return 0;
		}
        
		int extraindex = ObjClassExtraIndex[ extrapo->ObjectClass ];
		ASSERT( extraindex != EXTRAINDEX_NO_EXTRA );
        
		RE_CreateExtra *re_createextra = (RE_CreateExtra *) RE_List_CurPos;
		re_createextra->RE_Type 	   = RE_CREATEMINE;
		re_createextra->RE_BlockSize   = sizeof( RE_CreateExtra );
		re_createextra->ExtraIndex     = extraindex;
		re_createextra->HostObjId	   = extrapo->HostObjNumber;
		memcpy( &re_createextra->ObjPosition, &extrapo->ObjPosition, sizeof( Xmatrx ) );
        
		re_createextra++;
		re_createextra->RE_Type 	   = RE_EMPTY;
        
		RE_List_CurPos = (char *) re_createextra;
		RE_List_Avail -= sizeof( RE_CreateExtra );
	}
    
	return 1;
}

// insert extra into RE_List --------------------------------------------------
//
int NET_RmEvExtra( const ExtraObject *extrapo )
{
	//NOTE:
	// calling NET_RmEvAllowed() is mandatory
	// before calling this function.

	ASSERT( extrapo != NULL );

//	DSEXC( return 1 );

	if ( NetConnected == NETWORK_GAME_ON ) {

		if ( (size_t)RE_List_Avail < sizeof( RE_CreateExtra ) ) {
			ASSERT( 0 );
			return 0;
		}

		int extraindex = ObjClassExtraIndex[ extrapo->ObjectClass ];
		ASSERT( extraindex != EXTRAINDEX_NO_EXTRA );

		RE_CreateExtra *re_createextra = (RE_CreateExtra *) RE_List_CurPos;
		re_createextra->RE_Type 	   = RE_CREATEEXTRA;
		re_createextra->RE_BlockSize   = sizeof( RE_CreateExtra );
		re_createextra->ExtraIndex     = extraindex;
		re_createextra->HostObjId	   = extrapo->HostObjNumber;
		memcpy( &re_createextra->ObjPosition, &extrapo->ObjPosition, sizeof( Xmatrx ) );

		re_createextra++;
		re_createextra->RE_Type 	   = RE_EMPTY;

		RE_List_CurPos = (char *) re_createextra;
		RE_List_Avail -= sizeof( RE_CreateExtra );
	}

	return 1;
}

// insert particle object into RE_List ----------------------------------------
//
int NET_RmEvParticleObject( int type, const Vertex3& origin )
{
	//NOTE:
	// calling NET_RmEvAllowed() is mandatory
	// before calling this function.

//	DSEXC( return 1 );

	if ( NetConnected == NETWORK_GAME_ON ) {

		if ( (size_t)RE_List_Avail < sizeof( RE_ParticleObject ) ) {
			ASSERT( 0 );
			return 0;
		}

		RE_ParticleObject *re_particleobject = (RE_ParticleObject *) RE_List_CurPos;
		re_particleobject->RE_Type 	         = RE_PARTICLEOBJECT;
		re_particleobject->RE_BlockSize      = sizeof( RE_ParticleObject );
		re_particleobject->ObjectType        = type;
		re_particleobject->Origin	   		 = origin;

		re_particleobject++;
		re_particleobject->RE_Type 	         = RE_EMPTY;

		RE_List_CurPos = (char *) re_particleobject;
		RE_List_Avail -= sizeof( RE_ParticleObject );
	}

	return 1;
}


// insert weapon state update into RE_List ------------------------------------
//
int NET_RmEvWeaponState( dword weapon, byte state, int energy, dword specials )
{
	//NOTE:
	// calling NET_RmEvAllowed() is mandatory
	// before calling this function.

	if ( NetConnected == NETWORK_GAME_ON ) {

		if ( (size_t)RE_List_Avail < sizeof( RE_WeaponState ) ) {
			ASSERT( 0 );
			return 0;
		}

		RE_WeaponState *re_weaponstate	= (RE_WeaponState *) RE_List_CurPos;
		re_weaponstate->RE_Type			= RE_WEAPONSTATE;
		re_weaponstate->RE_BlockSize	= sizeof( RE_WeaponState );
		re_weaponstate->State			= state;
		re_weaponstate->WeaponMask		= weapon;
	//	re_weaponstate->Specials		= specials; //CrazySpence changed this, this comment is so he knows to change it back if it bites him in the ass
		re_weaponstate->CurEnergy   	= energy;
		re_weaponstate->SenderId        = LocalPlayerId;
		re_weaponstate++;
		re_weaponstate->RE_Type			= RE_EMPTY;

		RE_List_CurPos = (char *) re_weaponstate;
		RE_List_Avail -= sizeof( RE_WeaponState );
	}

	return 1;
}


// insert state sync into RE_List ---------------------------------------------
//
int NET_RmEvStateSync( dword statekey, dword stateval )
{
	//NOTE:
	// calling NET_RmEvAllowed() is mandatory
	// before calling this function.

	if ( NetConnected == NETWORK_GAME_ON ) {

		if ( (size_t)RE_List_Avail < sizeof( RE_StateSync ) ) {
			ASSERT( 0 );
			return 0;
		}

		RE_StateSync *re_statesync	= (RE_StateSync *) RE_List_CurPos;
		re_statesync->RE_Type		= RE_STATESYNC;
		re_statesync->RE_BlockSize	= sizeof( RE_StateSync );
		re_statesync->StateKey		= (word)statekey;
		re_statesync->StateValue	= stateval;

		re_statesync++;
		re_statesync->RE_Type		= RE_EMPTY;

		RE_List_CurPos = (char *) re_statesync;
		RE_List_Avail -= sizeof( RE_StateSync );
	}

	return 1;
}


// insert swarm into RE_List --------------------------------------------------
//
int NET_RmEvCreateSwarm( Vertex3 *origin, dword targetobjid, dword seed )
{
	//NOTE:
	// calling NET_RmEvAllowed() is mandatory
	// before calling this function.

	if ( NetConnected == NETWORK_GAME_ON ) {

		if ( (size_t)RE_List_Avail < sizeof( RE_CreateSwarm ) ) {
			ASSERT( 0 );
			return 0;
		}

		RE_CreateSwarm *re_createswarm = (RE_CreateSwarm *) RE_List_CurPos;
		re_createswarm->RE_Type 		 = RE_CREATESWARM;
		re_createswarm->RE_BlockSize	 = sizeof( RE_CreateSwarm );
		re_createswarm->Origin		 	 = *origin;
		re_createswarm->TargetHostObjId  = targetobjid;
		re_createswarm->RandSeed  		 = seed;
        re_createswarm->SenderId         = LocalPlayerId;
		re_createswarm++;
		re_createswarm->RE_Type 		 = RE_EMPTY;

		RE_List_CurPos = (char *) re_createswarm;
		RE_List_Avail -= sizeof( RE_CreateSwarm );
	}

	return 1;
}


// insert emp into RE_List ----------------------------------------------------
//
int NET_RmEvCreateEmp( int upgradelevel )
{
	//NOTE:
	// calling NET_RmEvAllowed() is mandatory
	// before calling this function.

	if ( NetConnected == NETWORK_GAME_ON ) {

		if ( (size_t)RE_List_Avail < sizeof( RE_CreateEmp ) ) {
			ASSERT( 0 );
			return 0;
		}

		RE_CreateEmp *re_createemp		= (RE_CreateEmp *) RE_List_CurPos;
		re_createemp->RE_Type 			= RE_CREATEEMP;
		re_createemp->RE_BlockSize		= sizeof( RE_CreateEmp );
		re_createemp->Upgradelevel		= upgradelevel;

		re_createemp++;
		re_createemp->RE_Type 			= RE_EMPTY;

		RE_List_CurPos = (char *) re_createemp;
		RE_List_Avail -= sizeof( RE_CreateEmp );
	}

	return 1;
}


// insert object kill event into RE_List --------------------------------------
//
int NET_RmEvKillObject( dword objid, byte listno )
{
	//NOTE:
	// calling NET_RmEvAllowed() is NOT mandatory
	// before calling this function.

#ifdef NO_PROJECTILE_KILL_MESSAGES

	// for projectiles depend on local client lifetime expiration
	if ( ( listno == LASER_LIST ) || ( listno == MISSL_LIST ) ) {
		return 1;
	}
	ASSERT( listno == EXTRA_LIST );

#endif

#ifdef SEND_KILL_MESSAGES

	if ( NetConnected == NETWORK_GAME_ON ) {

		if ( (size_t)RE_List_Avail < sizeof( RE_KillObject ) )
			return 0;

		RE_KillObject *re_killobject = (RE_KillObject *) RE_List_CurPos;
		re_killobject->RE_Type		 = RE_KILLOBJECT;
		re_killobject->RE_BlockSize  = sizeof( RE_KillObject );
		re_killobject->HostObjId	 = objid;
		re_killobject->ListId		 = listno;

		re_killobject++;
		re_killobject->RE_Type		 = RE_EMPTY;

		RE_List_CurPos = (char *) re_killobject;
		RE_List_Avail -= sizeof( RE_KillObject );
	}

#endif

	return 1;
}

// insert RE_PlayerStatus event into RE_List ----------------------------------
//
RE_PlayerStatus* NET_RmEvPlayerStatus( byte last_unjoin_flag )
{
	//NOTE:
	// calling NET_RmEvAllowed() is mandatory
	// before calling this function.

	if ( NetConnected == NETWORK_GAME_ON ) {

		if ( (size_t)RE_List_Avail < sizeof( RE_PlayerStatus ) ) {
			ASSERT( 0 );
			return NULL;
		}

		RE_PlayerStatus* re_playerstatus = (RE_PlayerStatus*) RE_List_CurPos;
		re_playerstatus->RE_Type		= RE_PLAYERSTATUS;
		re_playerstatus->RE_BlockSize	= sizeof( RE_PlayerStatus );

		re_playerstatus->senderid	 = LocalPlayerId;
		re_playerstatus->objectindex = ObjClassShipIndex[ MyShip->ObjectClass ];

		// set desired player status
		re_playerstatus->player_status = Player_Status[ LocalPlayerId ];

		//FIXME: only send "normal" unjoin to server
		// send last unjoin to server ( include killer )
		if ( Player_Status[ LocalPlayerId ] == PLAYER_CONNECTED ) {

			// store reason for last unjoin
			re_playerstatus->params[ 0 ] = last_unjoin_flag;

			// enable remote player to determine who killed us if ( param1 == SHIP_DOWNED )
			if ( last_unjoin_flag == SHIP_DOWNED ) {
				ASSERT( ( CurKiller >= KILLERID_UNKNOWN ) && ( CurKiller < MAX_NET_PROTO_PLAYERS + KILLERID_BIAS ) );
				ASSERT( ( CurKiller >= 0 ) && ( CurKiller < 128 ) );
				re_playerstatus->params[ 3 ] = CurKiller;
			}
		}

		// store the refframe for serverside movement checks
		re_playerstatus->RefFrame = SYSs_GetRefFrameCount();

		RE_PlayerStatus* re = re_playerstatus;

		re_playerstatus++;
		re_playerstatus->RE_Type = RE_EMPTY;

		RE_List_CurPos = (char *) re_playerstatus;
		RE_List_Avail -= sizeof( RE_PlayerStatus );

		// return the remote event added
		return re;
	}

	return NULL;
}


// insert PlayerAndShipStatus event into RE_List ------------------------------
//
RE_PlayerAndShipStatus* NET_RmEvPlayerAndShipStatus ( byte last_unjoin_flag )
{
	//NOTE:
	// calling NET_RmEvAllowed() is mandatory
	// before calling this function.

	if ( NetConnected == NETWORK_GAME_ON ) {

		if ( (size_t)RE_List_Avail < sizeof( RE_PlayerAndShipStatus ) ) {
			ASSERT( 0 );
			return NULL;
		}

		RE_PlayerAndShipStatus* re_pas_status	= (RE_PlayerAndShipStatus*) RE_List_CurPos;
		re_pas_status->RE_Type					= RE_PLAYERANDSHIPSTATUS;
		re_pas_status->RE_BlockSize				= sizeof( RE_PlayerAndShipStatus );

		re_pas_status->senderid		= LocalPlayerId;
		re_pas_status->objectindex	= ObjClassShipIndex[ MyShip->ObjectClass ];

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
		re_pas_status->player_status = Player_Status[ LocalPlayerId ];

		//FIXME: only send "normal" unjoin to server
		// send last unjoin to server ( include killer )
		if ( Player_Status[ LocalPlayerId ] == PLAYER_CONNECTED ) {

			// store reason for last unjoin
			re_pas_status->params[ 0 ] = last_unjoin_flag;

			// enable remote player to determine who killed us if ( param1 == SHIP_DOWNED )
			if ( last_unjoin_flag == SHIP_DOWNED ) {
				ASSERT( ( CurKiller >= KILLERID_UNKNOWN ) && ( CurKiller < MAX_NET_PROTO_PLAYERS + KILLERID_BIAS ) );
				ASSERT( ( CurKiller >= 0 ) && ( CurKiller < 128 ) );
				re_pas_status->params[ 3 ] = CurKiller;
			}
		}

		// store the refframe for serverside movement checks
		re_pas_status->RefFrame = SYSs_GetRefFrameCount();

		RE_PlayerAndShipStatus* re = re_pas_status;

		re_pas_status++;
		re_pas_status->RE_Type = RE_EMPTY;

		RE_List_CurPos = (char *) re_pas_status;
		RE_List_Avail -= sizeof( RE_PlayerAndShipStatus );

		// return the remote event added
		return re;
	}

	return NULL;
}


// execute received remote event: create object -------------------------------
//
void NET_ExecRmEvCreateObject( RE_Header *rmev, int ownerid )
{
	ASSERT( rmev != NULL );
	ASSERT( rmev->RE_Type == RE_CREATEOBJECT );
	ASSERT( ( ownerid >= 0 ) && ( ownerid < MAX_NET_PROTO_PLAYERS ) );

	RE_CreateObject *re_co = (RE_CreateObject *) rmev;

	GenObject *objpo       = CreateObject( re_co->ObjectClass, re_co->ObjPosition );
	ASSERT( objpo != NULL );
	objpo->HostObjNumber   = re_co->HostObjId;
	//TODO:
	// re_co->Flags

	RMEVTXT( MSGOUT( "--executing RE_CREATEOBJECT: %d (%d).", re_co->ObjectClass, ownerid ); );
}


// execute received remote event: create laser --------------------------------
//
void NET_ExecRmEvCreateLaser( RE_Header *rmev, int ownerid )
{
	// get owner from RE
	if ( NET_ProtocolGMSV() ) {
		ownerid = GetOwnerFromHostOjbNumber( ( (RE_CreateLaser*)rmev )->HostObjId );
	}

	ASSERT( rmev != NULL );
	ASSERT( rmev->RE_Type == RE_CREATELASER );
	ASSERT( ( ownerid >= 0 ) && ( ownerid < MAX_NET_PROTO_PLAYERS ) );
	ASSERT( (dword)ownerid == GetOwnerFromHostOjbNumber( ( (RE_CreateLaser*)rmev )->HostObjId ) );

	RE_CreateLaser *re_cl  = (RE_CreateLaser *) rmev;

	LaserObject *laserpo   = (LaserObject *) CreateObject( re_cl->ObjectClass, re_cl->ObjPosition );
	ASSERT( laserpo != NULL );
	laserpo->HostObjNumber = re_cl->HostObjId;
	laserpo->DirectionVec  = re_cl->DirectionVec;
	laserpo->Owner		   = ownerid;

	AUD_Laser( laserpo );

	RMEVTXT( MSGOUT( "--executing RE_CREATELASER: %d (%d).", re_cl->ObjectClass, ownerid ); );
}


// execute received remote event: create missile ------------------------------
//
void NET_ExecRmEvCreateMissile( RE_Header *rmev, int ownerid )
{
	// get owner from RE
	if ( NET_ProtocolGMSV() ) {
		ownerid = GetOwnerFromHostOjbNumber( ( (RE_CreateMissile*)rmev )->HostObjId );
	}
    
    ASSERT( rmev != NULL );
	ASSERT( rmev->RE_Type == RE_CREATEMISSILE );
	ASSERT( ( ownerid >= 0 ) && ( ownerid < MAX_NET_PROTO_PLAYERS ) );
	
	RE_CreateMissile *re_cm  = (RE_CreateMissile *) rmev;

	MissileObject *missilepo = (MissileObject *) CreateObject( re_cm->ObjectClass, re_cm->ObjPosition );
	ASSERT( missilepo != NULL );
	missilepo->HostObjNumber = re_cm->HostObjId;
	missilepo->DirectionVec  = re_cm->DirectionVec;
	missilepo->Owner		 = ownerid;

	if ( re_cm->ObjectClass == GUIDE_CLASS_1 ) {
		TargetMissileObject *targetmisspo = (TargetMissileObject *) missilepo;
		targetmisspo->TargetObjNumber     = re_cm->TargetHostObjId;
	}

	AUD_Missile( missilepo );

	RMEVTXT( MSGOUT( "--executing RE_CREATEMISSILE: %d (%d).", re_cm->ObjectClass, ownerid ); );
}


// execute received remote event: create particle object ----------------------
//
void NET_ExecRmEvParticleObject( RE_Header *rmev, int ownerid )
{
	ASSERT( rmev != NULL );
	ASSERT( rmev->RE_Type == RE_PARTICLEOBJECT );
	ASSERT( ( ownerid >= 0 ) && ( ownerid < MAX_NET_PROTO_PLAYERS ) );

	RE_ParticleObject *re_po = (RE_ParticleObject *) rmev;

	switch ( re_po->ObjectType ) {

		case POBJ_ENERGYFIELD:
			SFX_CreateEnergyField( re_po->Origin );
			break;

		case POBJ_MEGASHIELD:
			SFX_RemoteEnableInvulnerabilityShield( ownerid );
			break;

		case POBJ_SPREADFIRE:
			WFX_RemoteShootSpreadfire( ownerid );
			break;
	}

	RMEVTXT( MSGOUT( "--executing RE_PARTICLEOBJECT: %d (%d).", re_po->ObjectType, ownerid ); );
}


// execute received remote event: create extra --------------------------------
//
void NET_ExecRmEvCreateExtra( RE_Header *rmev, int ownerid )
{
    // get owner from RE
	if ( NET_ProtocolGMSV() ) {
		ownerid = GetOwnerFromHostOjbNumber( ( (RE_CreateExtra*)rmev )->HostObjId );
	}
	
    ASSERT( rmev != NULL );
	ASSERT( rmev->RE_Type == RE_CREATEEXTRA );
	ASSERT( ( ( ownerid >= 0 ) && ( ownerid < MAX_NET_PROTO_PLAYERS ) ) || ( ownerid == PLAYERID_SERVER ) );

	RE_CreateExtra *re_ce  = (RE_CreateExtra *) rmev;

	// resolve to class id
	ASSERT( re_ce->ExtraIndex < NumExtraClasses );
	dword classid = ExtraClasses[ re_ce->ExtraIndex ];

	// create object if extra available on this client
	if ( classid != CLASS_ID_INVALID ) {

		ExtraObject *extrapo = (ExtraObject *) CreateObject( classid, re_ce->ObjPosition );
		ASSERT( extrapo != NULL );
		extrapo->HostObjNumber = re_ce->HostObjId;
		if ( extrapo->ObjectType == MINE1TYPE ) {
			ASSERT( ownerid != PLAYERID_SERVER );
			((MineObject*)extrapo)->Owner = ownerid;
			AUD_Mine( extrapo );
		} else {
			OBJ_FillExtraMemberVars( extrapo );
		}
	}

	RMEVTXT( MSGOUT( "--executing RE_CREATEEXTRA: %d (%d).", classid, ownerid ); );
}


// execute received remote event: kill object ---------------------------------
//
void NET_ExecRmEvKillObject( RE_Header *rmev )
{
	ASSERT( rmev != NULL );
	ASSERT( rmev->RE_Type == RE_KILLOBJECT );

	RE_KillObject *re_ko = (RE_KillObject *) rmev;

	GenObject *objlistpo = NULL;
	switch ( re_ko->ListId ) {
		case LASER_LIST:
			objlistpo = LaserObjects;
			break;
		case MISSL_LIST:
			objlistpo = MisslObjects;
			break;
		case EXTRA_LIST:
			objlistpo = ExtraObjects;
			break;
		case CUSTM_LIST:
			objlistpo = CustmObjects;
			break;
		default:
			ASSERT( FALSE );
			break;
	}
	if ( objlistpo != NULL ) {
		KillSpecificHostObject( re_ko->HostObjId, objlistpo );
	}

	RMEVTXT( MSGOUT( "--executing RE_KILLOBJECT: obj %d, list %d.", re_ko->HostObjId, re_ko->ListId ); );
}


// execute received remote event: send text -----------------------------------
//
void NET_ExecRmEvSendText( RE_Header *rmev, int ownerid )
{
	
    ASSERT( rmev != NULL );
	ASSERT( rmev->RE_Type == RE_SENDTEXT );
	ASSERT( ( ownerid >= 0 ) && ( ownerid < MAX_NET_PROTO_PLAYERS ) );
		
	RE_SendText *re_st = (RE_SendText *) rmev;

	char *s, *name, *brk;
    s = strtok_r(re_st->TextStart," ",&brk);
    name = s;
    s = strtok_r(NULL,"\0",&brk);
    char *textpo = s;
	size_t textlen  = strlen( textpo );
	if ( textlen <= ( MAX_MESSAGELEN - strlen( name ) - 3 ) ) {
		ASSERT( ( 3 + strlen( name ) + textlen ) <= PASTE_STR_LEN );
		sprintf( paste_str, "-%s- %s",
					name, textpo );
		ShowMessage( paste_str );
	}
	ShowSentText( name, textpo, TALK_PUBLIC );

	RMEVTXT( MSGOUT( "--executing RE_SENDTEXT: \"%s\" (%d).", textpo, ownerid ); );
}


// execute received remote event: player name ---------------------------------
//
void NET_ExecRmEvPlayerName( RE_Header *rmev, int ownerid )
{
	ASSERT( rmev != NULL );
	ASSERT( rmev->RE_Type == RE_PLAYERNAME );
	ASSERT( ( ownerid >= 0 ) && ( ownerid < MAX_NET_PROTO_PLAYERS ) );

	RE_PlayerName *re_pn = (RE_PlayerName *) rmev;

	if ( name_change_notification ) {
		ASSERT( ( 8 + strlen( Player_Name[ ownerid ] ) + strlen( re_pn->PlayerName ) ) <= PASTE_STR_LEN );
		sprintf( paste_str, "%s is now %s",
					Player_Name[ ownerid ], re_pn->PlayerName );
		if ( strlen( paste_str ) <= MAX_MESSAGELEN )
			ShowMessage( paste_str );
	}
	CopyRemoteName( Player_Name[ ownerid ], re_pn->PlayerName );

	RMEVTXT( MSGOUT( "--executing RE_PLAYERNAME: \"%s\" (%d).", re_pn->PlayerName, ownerid ); );
}


// execute received remote event: weapon state --------------------------------
//
void NET_ExecRmEvWeaponState( RE_Header *rmev, int ownerid )
{
	ASSERT( rmev != NULL );
	ASSERT( rmev->RE_Type == RE_WEAPONSTATE );
	//ASSERT( ( ownerid >= 0 ) && ( ownerid < MAX_NET_PROTO_PLAYERS ) );

	//NOTE:
	// if new duration weapons are added this function should be updated.

	RE_WeaponState *re_ws = (RE_WeaponState *) rmev;

	// must only change state of single weapon
	ASSERT( ( re_ws->WeaponMask & ( re_ws->WeaponMask - 1 ) ) == 0 );

	// fetch pointer to remote player's ship
	ShipObject *shippo = NET_FetchOwnersShip( re_ws->SenderId );
	ASSERT( shippo != NULL );
	ASSERT( shippo != MyShip );

	// update energy
	shippo->CurEnergy = re_ws->CurEnergy;
//	shippo->Specials  = re_ws->Specials;

	// do weapon specific stuff
	switch ( re_ws->WeaponMask ) {

		case WPMASK_CANNON_LIGHTNING:
			if ( re_ws->State == WPSTATE_ON )
				WFX_RemoteActivateLightning( re_ws->SenderId );
			else
				WFX_RemoteDeactivateLightning( re_ws->SenderId );
			break;

		case WPMASK_CANNON_HELIX:
			if ( re_ws->State == WPSTATE_ON )
				WFX_RemoteActivateHelix( re_ws->SenderId );
			else
				WFX_RemoteDeactivateHelix( re_ws->SenderId );
			break;

		case WPMASK_CANNON_PHOTON:
			if ( re_ws->State == WPSTATE_ON )
				WFX_RemoteActivatePhoton( re_ws->SenderId );
			else
				WFX_RemoteDeactivatePhoton(re_ws->SenderId );
			break;

		case WPMASK_DEVICE_EMP:
			if ( re_ws->State == WPSTATE_ON )
				WFX_RemoteActivateEmp( re_ws->SenderId );
			else
				WFX_RemoteDeactivateEmp( re_ws->SenderId );
			break;

		default:
			// simply set/reset active flag
			if ( re_ws->State == WPSTATE_ON )
				shippo->WeaponsActive |= re_ws->WeaponMask;
			else
				shippo->WeaponsActive &= ~re_ws->WeaponMask;
			break;
	}

	RMEVTXT( MSGOUT( "--executing RE_WEAPONSTATE: 0x%X, %d, %d (%d).", re_ws->WeaponMask, re_ws->State, re_ws->CurEnergy, re_ws->SenderId ); );
}


// list of syncable remote states ---------------------------------------------
//
static dword *rmev_sync_states[ RMEVSTATE_NUMSTATES ] = {

	(dword*)&AUXDATA_BACKGROUND_NEBULA_ID,
	(dword*)&ExtraProbability,
	(dword*)&ProbHelixCannon,
	(dword*)&ProbLightningDevice,
	(dword*)&ProbPhotonCannon,
	(dword*)&ProbProximityMine,
	(dword*)&ProbRepairExtra,
	(dword*)&ProbAfterburner,
	(dword*)&ProbHoloDecoy,
	(dword*)&ProbInvisibility,
	(dword*)&ProbInvulnerability,
	(dword*)&ProbEnergyField,
	(dword*)&ProbLaserUpgrade,
	(dword*)&ProbLaserUpgrade1,
	(dword*)&ProbLaserUpgrade2,
	(dword*)&ProbMissilePack,
	(dword*)&ProbDumbMissPack,
	(dword*)&ProbHomMissPack,
	(dword*)&ProbSwarmMissPack,
	(dword*)&ProbEmpUpgrade1,
	(dword*)&ProbEmpUpgrade2,
	(dword*)&AUXDATA_SKILL_AMAZING_TIME,
	(dword*)&AUXDATA_SKILL_BRILLIANT_TIME,
	(dword*)&AUX_KILL_LIMIT_FOR_GAME_END,
    (dword*)&EnergyExtraBoost,
    (dword*)&RepairExtraBoost,
    (dword*)&DumbPackNumMissls,
    (dword*)&HomPackNumMissls,
    (dword*)&SwarmPackNumMissls,
    (dword*)&ProxPackNumMines,
};

/*
 int		EnergyExtraBoost	= ENERGY_EXTRA_BOOST;
 int 	RepairExtraBoost	= 0;
 
 int 	DumbPackNumMissls	= DUMB_PACK_NUMMISSLS;
 int 	HomPackNumMissls	= HOM_PACK_NUMMISSLS;
 int 	SwarmPackNumMissls	= SWARM_PACK_NUMMISSLS;
 int 	ProxPackNumMines	= PROX_PACK_NUMMINES;
 */

// execute received remote event: state sync ----------------------------------
//
void NET_ExecRmEvStateSync( RE_Header *rmev, int ownerid )
{
	ASSERT( rmev != NULL );
	ASSERT( rmev->RE_Type == RE_STATESYNC );
	ASSERT( ( ownerid >= 0 ) && ( ownerid < MAX_NET_PROTO_PLAYERS ) );

	RE_StateSync *re_ss = (RE_StateSync *) rmev;

	// sync state without checking the value
	if ( re_ss->StateKey < RMEVSTATE_NUMSTATES ) {

		*rmev_sync_states[ re_ss->StateKey ] = re_ss->StateValue;

		// always display a sync message
		MSGOUT( "state sync: %d=%d.", re_ss->StateKey, re_ss->StateValue );
	}

	RMEVTXT( MSGOUT( "--executing RE_STATESYNC: %d=%d (%d).", re_ss->StateKey, re_ss->StateValue, ownerid ); );
}


// execute received remote event: swarm missile -------------------------------
//
void NET_ExecRmEvCreateSwarm( RE_Header *rmev, int ownerid )
{
	ASSERT( rmev != NULL );
	ASSERT( rmev->RE_Type == RE_CREATESWARM );
	ASSERT( ( ownerid >= 0 ) && ( ownerid < MAX_NET_PROTO_PLAYERS ) );

	RE_CreateSwarm *re_cs  = (RE_CreateSwarm *) rmev;

	// determine target ship via id
	ShipObject *targetpo = MyShip;
	if ( re_cs->TargetHostObjId != ShipHostObjId( LocalPlayerId ) ) {
		// same as ?:
//		targetpo = FetchSpecificHostObject( re_cs->TargetHostObjID, PShipObjects );
//		if ( targetpo == NULL ) return;
		targetpo = (ShipObject *) FetchFirstShip();
		while ( targetpo && ( re_cs->TargetHostObjId != targetpo->HostObjNumber ) )
			targetpo = (ShipObject *) targetpo->NextObj;
		if ( targetpo == NULL ) {
			return;
		}
	}
	
	GenObject *dummyobj = SWARM_Init( re_cs->SenderId, &re_cs->Origin, targetpo, re_cs->RandSeed );
	
	AUD_SwarmMissiles( &re_cs->Origin, dummyobj );
	
	RMEVTXT( MSGOUT( "--executing RE_CREATESWARM: %d (%d).", re_cs->RandSeed, ownerid ); );
}


// execute received remote event: create emp ----------------------------------
//
void NET_ExecRmEvCreateEmp( RE_Header *rmev, int ownerid )
{
	ASSERT( rmev != NULL );
	ASSERT( rmev->RE_Type == RE_CREATEEMP );
	ASSERT( ( ownerid >= 0 ) && ( ownerid < MAX_NET_PROTO_PLAYERS ) );
	// get owner from RE
	if ( NET_ProtocolGMSV() ) {
		ownerid = GetOwnerFromHostOjbNumber( ( (RE_CreateLaser*)rmev )->HostObjId );
	}
	RE_CreateEmp *re_ce  = (RE_CreateEmp *) rmev;
	
	// fetch pointer to remote player's ship
	ShipObject *shippo = NET_FetchOwnersShip( ownerid );
	//	ASSERT( shippo != NULL );
	//ASSERT( shippo != MyShip );
	// well,this happens sometimes...
	if ( shippo == NULL ) {
		return;
	}
	
	WFX_RemoteEmpBlast( shippo, re_ce->Upgradelevel );
	
	RMEVTXT( MSGOUT( "--executing RE_CREATEEMP: %d (%d).", re_ce->Upgradelevel, ownerid ); );
}

// execute RE that describes the current gamestate ( time, game running, etc. )
//
void NET_ExecRmEvGameState( RE_GameState* gamestate )
{
	ASSERT( gamestate != NULL );
	ASSERT( gamestate->RE_Type == RE_GAMESTATE );
	ASSERT( CurGameTime != GAME_PEERTOPEER );

	// server sends RE_GameState to transmit current game state/time
	CurGameTime = gamestate->GameTime;
}

// execute RE containing latest killstats -------------------------------------
//
void NET_ExecRmEvKillStats( RE_KillStats* killstats )
{
	ASSERT( killstats != NULL );
	ASSERT( killstats->RE_Type == RE_KILLSTATS );

	// handled different in PEER/GMSV mode
	NETs_UpdateKillStats( killstats );
}



// determine total size of remote event list (including termination dword ) ---
//
size_t NET_RmEvList_GetSize( RE_Header* relist )
{
	ASSERT( relist != NULL );
	
	size_t relistsize = 0;
	
	// process remote event list
	while ( relist->RE_Type != RE_EMPTY ) {
		
		// sum up size of all remote events
		size_t resize = NET_RmEvGetSize( relist );
		relistsize += resize;
		
		// advance to next event
		ASSERT( ( relist->RE_BlockSize == RE_BLOCKSIZE_INVALID ) || ( relist->RE_BlockSize == resize ) );
		relist = (RE_Header *) ( (char *) relist + resize );
	}
	
	// include size of remote event list termination
	relistsize += sizeof( dword );
	
	return relistsize;
}

// check integrity of remote event list ( known types, type checks, length check )
//
int NET_RmEvList_IsWellFormed( RE_Header* relist )
{
	ASSERT( relist != NULL );
	
	//FIXME: [1/30/2002] implement
	//FIXME: check SVG_REList::IsWellFormed
	
	//RE_LIST_MAXAVAIL
	
	//ASSERT( FALSE );
	return TRUE;
}

