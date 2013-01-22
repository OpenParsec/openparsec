/*
 * PARSEC - Action Commands Recording
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:34 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1997-1999
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
#include <math.h>
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
#include "inp_defs.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "e_record.h"

// proprietary module headers
#include "con_act.h"
#include "con_aux.h"
#include "con_ext.h"
#include "con_main.h"
#include "net_game.h"
#include "net_util.h"
#include "obj_creg.h"
#include "obj_ctrl.h"
#include "obj_cust.h"


// flags
//#define SAVE_OBJNUM_ON_SHIPLISTSAVE
#define STRIP_BINARY_PACKETS



// file name constants for packet recording session files ---------------------
//
static char rpacket_extension[]	= ".prp";
static char rpacket_basename[]	= "rpack";


// here camera information is stored at the right time to be used later on ----
//
static Camera RecordShipViewCamera;


// flag whether absolute matrices should be recorded --------------------------
//
//static int RecordAbsolutePosition = TRUE;
#define RecordAbsolutePosition		(!AUX_DISABLE_ABSOLUTE_RECORDING)


// flag whether wait command has already been inserted in the current frame ---
//
static int idlewait_inserted = FALSE;


// save wait command using the current idle duration --------------------------
//
INLINE
void SaveWaitCommand()
{
	//NOTE:
	// this function is always called before recording
	// a command that depends on correct timing. it
	// ensures that the command will be played back
	// at the original relative time.

	// write idle wait only if not already done
	if ( !idlewait_inserted ) {

		// write idle wait
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_IDLEWAIT ), IdleDuration );

		// reset duration timer
		IdleDuration = 0;

		// set flag
		idlewait_inserted = TRUE;
	}
}


// write list of commands able to recreate the current state ------------------
//
void Save_StateInfo( FILE *fp )
{
	ASSERT( fp != NULL );

	Save_PrepRestore( fp );
	Save_Camera( fp );
	Save_PseudoStars( fp );
	Save_ObjListShips( fp );
	Save_ObjListLasers( fp );
	Save_ObjListMissls( fp );
	Save_ObjListExtras( fp );
	Save_ObjListCustom( fp );
	Save_MyShipState( fp );
	Save_RemoteState( fp );
}


// write prepare restore command ----------------------------------------------
//
void Save_PrepRestore( FILE *fp )
{
	ASSERT( fp != NULL );

	fprintf( fp, "%s %d\n", ACMSTR( ACM_PREPRESTORE ), MyShip->ObjectClass );
}


// save state of local ship (MyShip) ------------------------------------------
//
void Save_MyShipState( FILE *fp )
{
	//NOTE:
	// this function saves all commands restoring
	// the properties of the local ship.

	ASSERT( fp != NULL );

	//NOTE:
	// it is crucial that "ac.weaponsactive" is executed after
	// the available weapons and energy have already been set,
	// since they will be checked on whether the weapons activation
	// is actually possible. (see CON_ACT::AcMyShip_WeaponsActive())

	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_CURDAMAGE		),	MyShip->CurDamage		);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_MAXDAMAGE		),	MyShip->MaxDamage		);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_CURENERGY		),	MyShip->CurEnergy		);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_MAXENERGY		),	MyShip->MaxEnergy		);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_CURSPEED			),	MyShip->CurSpeed		);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_MAXSPEED			),	MyShip->MaxSpeed		);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_NUMMISSLS		),	MyShip->NumMissls		);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_MAXNUMMISSLS		),	MyShip->MaxNumMissls	);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_NUMHOMMISSLS		),	MyShip->NumHomMissls	);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_MAXNUMHOMMISSLS	),	MyShip->MaxNumHomMissls	);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_NUMPARTMISSLS	),	MyShip->NumPartMissls	);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_MAXNUMPARTMISSLS	),	MyShip->MaxNumPartMissls);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_NUMMINES			),	MyShip->NumMines		);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_MAXNUMMINES		),	MyShip->MaxNumMines		);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_WEAPONS			),	MyShip->Weapons			);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_WEAPONSACTIVE	),	MyShip->WeaponsActive	);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_SPECIALS			),	MyShip->Specials		);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_GL_SELECTEDLASER	),	SelectedLaser			);
	fprintf( fp, "%s %d\n", ACMSTR( ACM_GL_SELECTEDMISSILE	),	SelectedMissile         );
	fprintf( fp, "%s %d\n", ACMSTR( ACM_GL_CURGUN			),	CurGun                  );
	fprintf( fp, "%s %d\n", ACMSTR( ACM_GL_CURLAUNCHER		),	CurLauncher             );
	fprintf( fp, "%s %d\n", ACMSTR( ACM_GL_PANEL3CONTROL	),	AUX_HUD_PANEL_3_CONTROL );
	fprintf( fp, "%s %d\n", ACMSTR( ACM_GL_PANEL4CONTROL	),	AUX_HUD_PANEL_4_CONTROL );
	fprintf( fp, "%s %d\n", ACMSTR( ACM_GL_TARGETOBJNUMBER	),	TargetObjNumber         );
}


// save global maximum number of remote players (NET_GLOB::CurMaxPlayers) -----
//
PRIVATE
void SaveCurMaxPlayers( FILE *fp )
{
	ASSERT( fp != NULL );

	fprintf( fp, "%s %d\n", ACMSTR( ACM_REMOTEMAXPLAYERS ), CurMaxPlayers );
}


// save remote player's state -------------------------------------------------
//
PRIVATE
void SavePlayerState( FILE *fp, int id, int status, int objclass, int killstat )
{
	ASSERT( fp != NULL );

	fprintf( fp, "%s %d %d %d %d\n", ACMSTR( ACM_REMOTEPLAYER ),
			 id, status, objclass, killstat );
}


// save remote player's matrix ------------------------------------------------
//
PRIVATE
void SavePlayerMatrix( FILE *fp, ShipObject *obj )
{
	ASSERT( fp != NULL );
	ASSERT( obj != NULL );

	// entire matrix in one command
	fprintf( fp, "%s %f %f %f %f %f %f %f %f %f %f %f %f\n",
			 ACMSTR( ACM_REMOTEMATRIX ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 0 ][ 0 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 0 ][ 1 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 0 ][ 2 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 0 ][ 3 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 1 ][ 0 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 1 ][ 1 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 1 ][ 2 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 1 ][ 3 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 2 ][ 0 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 2 ][ 1 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 2 ][ 2 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 2 ][ 3 ] ) );
}


// save remote player's weapons state -----------------------------------------
//
PRIVATE
void SavePlayerWeaponsActive( FILE *fp, ShipObject *obj )
{
	ASSERT( fp != NULL );
	ASSERT( obj != NULL );

	fprintf( fp, "%s %d\n", ACMSTR( ACM_REMOTEWEAPONSACTIVE ),
			 obj->WeaponsActive );
}


// save remote player's name --------------------------------------------------
//
PRIVATE
void SavePlayerName( FILE *fp, char *name )
{
	ASSERT( fp != NULL );
	ASSERT( name != NULL );

	// save only non-empty names
	if ( name[ 0 ] != '\0' ) {
		fprintf( fp, "%s %s\n", ACMSTR( ACM_REMOTENAME ), name );
	}
}


// save state of all remote players -------------------------------------------
//
void Save_RemoteState( FILE *fp )
{
	ASSERT( fp != NULL );

	// prevent remote state access when
	// network game not active
	if ( !NetConnected )
		return;

	// save maximum number of players to allow
	// restoration/checking on replay
	SaveCurMaxPlayers( fp );

	// process all players
	for ( int id = 0; id < MAX_NET_PROTO_PLAYERS; id++ ) {

		ShipObject *obj = (ShipObject *) Player_Ship[ id ];
		ASSERT( ( obj == NULL ) || OBJECT_TYPE_SHIP( obj ) );

		//NOTE:
		// if ( obj == MyShip ) information about
		// the local player will be stored. the
		// meaning of this information is different
		// from the normal meaning of the fields.

		// determine player ship object class
		int objclass = ( obj != NULL ) ? obj->ObjectClass : 0;

		// if local player use <objclass> field as <num-of-remote-players>
	   	if ( obj == MyShip ) {
			ASSERT( NumRemPlayers > 0 );
			// negative objclass denotes local player
			objclass = -NumRemPlayers;
		}

		// save player state
		SavePlayerState( fp, id, Player_Status[ id ],
						 objclass, Player_KillStat[ id ] );

		// save player matrix and weapons state (only for remote players)
		if ( ( obj != NULL ) && ( obj != MyShip ) ) {
			SavePlayerMatrix( fp, obj );
			SavePlayerWeaponsActive( fp, obj );
		}

		// save player name
		SavePlayerName( fp, Player_Name[ id ] );
	}
}


// record state of local player (invoked after state change) ------------------
//
void Record_LocalPlayerState()
{
	ASSERT( NetConnected );

	if ( RecordingActive && NetConnected ) {

		ASSERT( RecordingFp != NULL );
		ASSERT( NumRemPlayers > 0 );

		ASSERT( Player_Ship[ LocalPlayerId ] == MyShip );
		ASSERT( Player_ShipId[ LocalPlayerId ] == SHIPID_LOCALPLAYER );

		// save player state
		SaveWaitCommand();
		SavePlayerState( RecordingFp, LocalPlayerId, Player_Status[ LocalPlayerId ],
						 -NumRemPlayers, Player_KillStat[ LocalPlayerId ] );

		// save player matrix on join
		if ( Player_Status[ LocalPlayerId ] == PLAYER_JOINED )
			SavePlayerMatrix( RecordingFp, MyShip );

		// save player name
		SavePlayerName( RecordingFp, Player_Name[ LocalPlayerId ] );
	}
}


// record local join ----------------------------------------------------------
//
void Record_Join()
{
	ASSERT( NetConnected );
	ASSERT( NetJoined );
	ASSERT( Player_Status[ LocalPlayerId ] == PLAYER_JOINED );

	if ( RecordingActive && NetConnected ) {

		ASSERT( RecordingFp != NULL );
		ASSERT( NumRemPlayers > 0 );

		ASSERT( Player_Ship[ LocalPlayerId ] == MyShip );
		ASSERT( Player_ShipId[ LocalPlayerId ] == SHIPID_LOCALPLAYER );

		// save join command, matrix, and name
		SaveWaitCommand();
		fprintf( RecordingFp, "%s %d %d %d\n", ACMSTR( ACM_REMOTEJOIN ),
				 LocalPlayerId, 0, NumRemPlayers );
		SavePlayerMatrix( RecordingFp, MyShip );
		SavePlayerName( RecordingFp, Player_Name[ LocalPlayerId ] );
	}
}


// record local unjoin --------------------------------------------------------
//
void Record_Unjoin( byte flag )
{
	ASSERT( NetConnected );
	ASSERT( !NetJoined );
	ASSERT( Player_Status[ LocalPlayerId ] == PLAYER_CONNECTED );

	if ( RecordingActive && NetConnected ) {

		ASSERT( RecordingFp != NULL );
		ASSERT( NumRemPlayers > 0 );

		ASSERT( Player_Ship[ LocalPlayerId ] == MyShip );
		ASSERT( Player_ShipId[ LocalPlayerId ] == SHIPID_LOCALPLAYER );

		// save unjoin command
		SaveWaitCommand();
		fprintf( RecordingFp, "%s %d %d %d\n", ACMSTR( ACM_REMOTEUNJOIN ),
				 LocalPlayerId, flag, NumRemPlayers );
	}
}


// record state changes of specific myship variables (used by CON_INT.C) ------
//
void Record_MyShipStateWeapons()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MS_WEAPONS ), MyShip->Weapons );
	}
}

void Record_MyShipStateSpecials()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MS_SPECIALS ), MyShip->Specials );
	}
}

void Record_MyShipStateCurDamage()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MS_CURDAMAGE ), MyShip->CurDamage );
	}
}

void Record_MyShipStateCurEnergy()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MS_CURENERGY ), MyShip->CurEnergy );
	}
}

void Record_MyShipStateCurSpeed()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MS_CURSPEED ), MyShip->CurSpeed );
	}
}

void Record_MyShipStateNumMissls()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MS_NUMMISSLS ), MyShip->NumMissls );
	}
}

void Record_MyShipStateNumHomMissls()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MS_NUMHOMMISSLS ), MyShip->NumHomMissls );
	}
}

void Record_MyShipStateNumPartMissls()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MS_NUMPARTMISSLS ), MyShip->NumPartMissls );
	}
}

void Record_MyShipStateNumMines()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MS_NUMMINES ), MyShip->NumMines );
	}
}

void Record_MyShipStateMaxDamage()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MS_MAXDAMAGE ), MyShip->MaxDamage );
	}
}

void Record_MyShipStateMaxEnergy()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MS_MAXENERGY ), MyShip->MaxEnergy );
	}
}

void Record_MyShipStateMaxSpeed()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MS_MAXSPEED ), MyShip->MaxSpeed );
	}
}

void Record_MyShipStateMaxNumMissls()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MS_MAXNUMMISSLS ), MyShip->MaxNumMissls );
	}
}

void Record_MyShipStateMaxNumHomMissls()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MS_MAXNUMHOMMISSLS ), MyShip->MaxNumHomMissls );
	}
}

void Record_MyShipStateMaxNumPartMissls()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MS_MAXNUMPARTMISSLS ), MyShip->MaxNumPartMissls );
	}
}

void Record_MyShipStateMaxNumMines()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MS_MAXNUMMINES ), MyShip->MaxNumMines );
	}
}


// save position matrix of object ---------------------------------------------
//
PRIVATE
void SaveObjectMatrix( FILE *fp, GenObject *obj )
{
	//NOTE:
	// between an "ac.creatobj" command and the "ac.objmatrxcol<x>"
	// commands generated by this function no commands except
	// "ac.setobjid" and "ac.setobjhostid" are allowed. this is
	// crucial for delayed object creation. furthermore, there
	// MUST NOT be any "ac.creatobj" commands that are not followed
	// by these four "ac.objmatrxcol<x>" commands.
	// (see CON_ACT::AcCreateObject() etc.)

	//NOTE:
	// the output of this function could look like this:
	//
	// ac.objmatrxcol1 768 0.571397 -0.623530 0.533586
	// ac.objmatrxcol2 768 0.769223 0.633509 -0.083435
	// ac.objmatrxcol3 768 -0.286007 0.458122 0.841620
	// ac.objmatrxcol4 768 8.604858 305.986023 -0.425659
	//
	// 768 (0x0300 == EXTRA_LIST_NO) identifies the object-list
	// where the position matrix should be modified on execution.
	// (the object at the head of the list will be modified.)

	ASSERT( fp != NULL );
	ASSERT( obj != NULL );

	fprintf( fp, "%s %d %f %f %f\n", ACMSTR( ACM_OBJMATRXCOL1 ),
			 obj->ObjectType & TYPELISTMASK,
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 0 ][ 0 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 1 ][ 0 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 2 ][ 0 ] ) );
	fprintf( fp, "%s %d %f %f %f\n", ACMSTR( ACM_OBJMATRXCOL2 ),
			 obj->ObjectType & TYPELISTMASK,
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 0 ][ 1 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 1 ][ 1 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 2 ][ 1 ] ) );
	fprintf( fp, "%s %d %f %f %f\n", ACMSTR( ACM_OBJMATRXCOL3 ),
			 obj->ObjectType & TYPELISTMASK,
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 0 ][ 2 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 1 ][ 2 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 2 ][ 2 ] ) );
	fprintf( fp, "%s %d %f %f %f\n", ACMSTR( ACM_OBJMATRXCOL4 ),
			 obj->ObjectType & TYPELISTMASK,
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 0 ][ 3 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 1 ][ 3 ] ),
			 GEOMV_TO_FLOAT( obj->ObjPosition[ 2 ][ 3 ] ) );
}


// write create event for laser object ----------------------------------------
//
PRIVATE
void SaveLaserCreation( LaserObject *laserpo, FILE *fp )
{
	ASSERT( laserpo != NULL );
	ASSERT( fp != NULL );

	fprintf( fp, "%s %d\n", ACMSTR( ACM_CREATEOBJECT ),
			 laserpo->ObjectClass );
	SaveObjectMatrix( fp, laserpo );
	fprintf( fp, "%s %d\n", ACMSTR( ACM_LASER_SPEED ),
			 laserpo->Speed );
	fprintf( fp, "%s %f %f %f\n", ACMSTR( ACM_LASER_DIRVEC ),
			 GEOMV_TO_FLOAT( laserpo->DirectionVec.X ),
			 GEOMV_TO_FLOAT( laserpo->DirectionVec.Y ),
			 GEOMV_TO_FLOAT( laserpo->DirectionVec.Z ) );
	fprintf( fp, "%s %d\n", ACMSTR( ACM_LASER_OWNER ),
			 laserpo->Owner );
}


// save current energy value --------------------------------------------------
//
PRIVATE
void SaveCurEnergy( FILE *fp )
{
	ASSERT( fp != NULL );

	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_CURENERGY ), MyShip->CurEnergy );
}


// record create event for laser object if recording active -------------------
//
void Record_LaserCreation( LaserObject *laserpo )
{
	ASSERT( laserpo != NULL );

	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
		SaveLaserCreation( laserpo, RecordingFp );
		SaveCurEnergy( RecordingFp );
	}
}


// write create event for missile object --------------------------------------
//
PRIVATE
void SaveMissileCreation( MissileObject *missilepo, FILE *fp )
{
	ASSERT( missilepo != NULL );
	ASSERT( fp != NULL );

	fprintf( fp, "%s %d\n", ACMSTR( ACM_CREATEOBJECT ),
			 missilepo->ObjectClass );
	SaveObjectMatrix( fp, missilepo );
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MISSL_SPEED ),
			 missilepo->Speed );
	fprintf( fp, "%s %f %f %f\n", ACMSTR( ACM_MISSL_DIRVEC ),
			 GEOMV_TO_FLOAT( missilepo->DirectionVec.X ),
			 GEOMV_TO_FLOAT( missilepo->DirectionVec.Y ),
			 GEOMV_TO_FLOAT( missilepo->DirectionVec.Z ) );
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MISSL_OWNER ),
			 missilepo->Owner );
}


// save current number of missiles --------------------------------------------
//
PRIVATE
void SaveNumMissls( FILE *fp )
{
	ASSERT( fp != NULL );

	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_NUMMISSLS ), MyShip->NumMissls );
}


// record create event for missile object if recording active -----------------
//
void Record_MissileCreation( MissileObject *missilepo )
{
	ASSERT( missilepo != NULL );

	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
		SaveMissileCreation( missilepo, RecordingFp );
		SaveNumMissls( RecordingFp );
	}
}


// save current number of homing missiles -------------------------------------
//
PRIVATE
void SaveNumHomMissls( FILE *fp )
{
	ASSERT( fp != NULL );

	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_NUMHOMMISSLS ), MyShip->NumHomMissls );
}


// record create event for missile object with target if recording active -----
//
void Record_TargetMissileCreation( TargetMissileObject *missilepo )
{
	ASSERT( missilepo != NULL );

	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
		SaveMissileCreation( missilepo, RecordingFp );
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MISSL_TARGET ),
				 missilepo->TargetObjNumber );
		SaveNumHomMissls( RecordingFp );
	}
}


// write create event for mine object -----------------------------------------
//
PRIVATE
void SaveMineCreation( MineObject *minepo, FILE *fp )
{
	ASSERT( minepo != NULL );
	ASSERT( fp != NULL );

	fprintf( fp, "%s %d\n", ACMSTR( ACM_CREATEOBJECT ),
			 minepo->ObjectClass );
	SaveObjectMatrix( fp, minepo );
	fprintf( fp, "%s %d\n", ACMSTR( ACM_MINE_OWNER ),
			 minepo->Owner );
}


// save current number of mines -----------------------------------------------
//
PRIVATE
void SaveNumMines( FILE *fp )
{
	ASSERT( fp != NULL );

	fprintf( fp, "%s %d\n", ACMSTR( ACM_MS_NUMMINES ), MyShip->NumMines );
}


// record create event for mine object if recording active --------------------
//
void Record_MineCreation( MineObject *minepo )
{
	ASSERT( minepo != NULL );

	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
		SaveMineCreation( minepo, RecordingFp );
		SaveNumMines( RecordingFp );
	}
}


// write create event for extra object ----------------------------------------
//
PRIVATE
void SaveExtraCreation( ExtraObject *extrapo, FILE *fp )
{
	ASSERT( extrapo != NULL );
	ASSERT( fp != NULL );
	
	fprintf( fp, "%s %d\n", ACMSTR( ACM_CREATEEXTRA ),
		extrapo->ObjectClass );
	SaveObjectMatrix( fp, extrapo );
}


// record create event for extra object if recording active -------------------
//
void Record_ExtraCreation( ExtraObject *extrapo )
{
	ASSERT( extrapo != NULL );
	
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
		SaveExtraCreation( extrapo, RecordingFp );
	}
}


// write create event for custom object ---------------------------------------
//
PRIVATE
void SaveCustomCreation( CustomObject *custompo, FILE *fp )
{
	ASSERT( custompo != NULL );
	ASSERT( fp != NULL );
	
	// virtual objects must not be saved here
	if ( custompo->ObjectClass != CLASS_ID_INVALID ) {
		
		// check whether this custom type is persistent
		int custtype_flags = OBJ_GetCustomTypeFlags( custompo->ObjectType );
		if ( !( custtype_flags & CUSTOM_TYPE_NOT_PERSISTANT ) ) {
			fprintf( fp, "%s %d\n", ACMSTR( ACM_CREATEOBJECT ), custompo->ObjectClass );
			SaveObjectMatrix( fp, custompo );
		}
	}
}


// record create event for custom object if recording active ------------------
//
void Record_CustomCreation( CustomObject *custompo )
{
	ASSERT( custompo != NULL );
	
	if ( RecordingActive ) {
		
		// check whether this custom type is persistent
		int custtype_flags = OBJ_GetCustomTypeFlags( custompo->ObjectType );
		if ( !( custtype_flags & CUSTOM_TYPE_NOT_PERSISTANT ) ) {
			ASSERT( RecordingFp != NULL );
			SaveWaitCommand();
			SaveCustomCreation( custompo, RecordingFp );
		}
	}
}


// record create event for energy field if recording active -------------------
//
void Record_EnergyFieldCreation( Vertex3& origin )
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
		fprintf( RecordingFp, "%s %f %f %f\n", ACMSTR( ACM_ENERGYFIELD ),
				 GEOMV_TO_FLOAT( origin.X ),
				 GEOMV_TO_FLOAT( origin.Y ),
				 GEOMV_TO_FLOAT( origin.Z ) );
	}
}


// record create event for spreadfire particles if recording active -----------
//
void Record_SpreadFireFiring()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
		fprintf( RecordingFp, "%s\n", ACMSTR( ACM_SPREADFIRE ) );
	}
}


// record activation event of lightning beams if recording active -------------
//
void Record_LightningActivation()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
		fprintf( RecordingFp, "%s\n", ACMSTR( ACM_LIGHTNINGON ) );
	}
}


// record deactivation event of lightning beams if recording active -----------
//
void Record_LightningDeactivation()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
		fprintf( RecordingFp, "%s\n", ACMSTR( ACM_LIGHTNINGOFF ) );
	}
}


// record activation event of helix cannon if recording active ----------------
//
void Record_HelixActivation()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
		fprintf( RecordingFp, "%s\n", ACMSTR( ACM_HELIXON ) );
	}
}


// record deactivation event of helix cannon if recording active --------------
//
void Record_HelixDeactivation()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
		fprintf( RecordingFp, "%s\n", ACMSTR( ACM_HELIXOFF ) );
	}
}


// record activation event of photon cannon if recording active ---------------
//
void Record_PhotonActivation()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
        fprintf( RecordingFp, "%s\n", ACMSTR( ACM_PHOTONON ) );
	}
}


// record deactivation event of photon cannon if recording active -------------
//
void Record_PhotonDeactivation()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
        fprintf( RecordingFp, "%s\n", ACMSTR( ACM_PHOTONOFF ) );
	}
}


// record activation event of emp if recording active -------------------------
//
void Record_EmpActivation()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
        fprintf( RecordingFp, "%s\n", ACMSTR( ACM_EMPON ) );
	}
}


// record deactivation event of emp if recording active -------------
//
void Record_EmpDeactivation()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
        fprintf( RecordingFp, "%s\n", ACMSTR( ACM_EMPOFF ) );
	}
}


// record emp creation if recording active ------------------------------------
//
void Record_EmpCreation()
{
	if ( RecordingActive ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
        fprintf( RecordingFp, "%s\n", ACMSTR( ACM_EMP ) );
	}
}


// save shiplist in reversed order --------------------------------------------
//
PRIVATE
void RecursiveShipListSave( FILE *fp, ShipObject *scanpo )
{
	ASSERT( fp != NULL );

	if ( scanpo == NULL )
		return;

	RecursiveShipListSave( fp, (ShipObject *) scanpo->NextObj );

	// in network game ships of remote players
	// should not be saved here
	if ( NetConnected ) {

		//NOTE:
		// inactive players and the local player need
		// not be explicitly excluded here.

		// check all players
		for ( int id = 0; id < MAX_NET_PROTO_PLAYERS; id++ )
			if ( Player_Ship[ id ] == scanpo )
				return;
	}

	fprintf( fp, "%s %d\n", ACMSTR( ACM_CREATEOBJECT ),
				scanpo->ObjectClass );

#ifdef SAVE_OBJNUM_ON_SHIPLISTSAVE

	fprintf( fp, "%s %d\n", ACMSTR( ACM_SETOBJECTID ),
				scanpo->ObjectNumber );
#endif

	fprintf( fp, "%s %d\n", ACMSTR( ACM_SETOBJECTHOSTID ),
				scanpo->HostObjNumber );

	SaveObjectMatrix( fp, scanpo );
}


// save all objects contained in ship objects list ----------------------------
//
void Save_ObjListShips( FILE *fp )
{
	ASSERT( fp != NULL );

	// save recursively to achieve same ordering on
	// restoration as in the original
	RecursiveShipListSave( fp, FetchFirstShip() );
}


// save all objects contained in laser objects list ----------------------------
//
void Save_ObjListLasers( FILE *fp )
{
	ASSERT( fp != NULL );

	LaserObject *scanpo = FetchFirstLaser();
	for ( ; scanpo; scanpo = (LaserObject *) scanpo->NextObj ) {
		SaveLaserCreation( scanpo, fp );
	}
}


// save all objects contained in missile objects list -------------------------
//
void Save_ObjListMissls( FILE *fp )
{
	ASSERT( fp != NULL );

	MissileObject *scanpo = FetchFirstMissile();
	for ( ; scanpo; scanpo = (MissileObject *) scanpo->NextObj ) {
		SaveMissileCreation( scanpo, fp );
	}
}


// save all objects contained in extra objects list ---------------------------
//
void Save_ObjListExtras( FILE *fp )
{
	ASSERT( fp != NULL );

	ExtraObject *scanpo = FetchFirstExtra();
	for ( ; scanpo; scanpo = (ExtraObject *) scanpo->NextObj ) {
		if ( scanpo->ObjectType == MINE1TYPE )
			SaveMineCreation( (MineObject *) scanpo, fp );
		else
			SaveExtraCreation( scanpo, fp );
	}
}


// save all objects contained in custom objects list --------------------------
//
void Save_ObjListCustom( FILE *fp )
{
	ASSERT( fp != NULL );

	CustomObject *scanpo = FetchFirstCustom();
	for ( ; scanpo; scanpo = (CustomObject *) scanpo->NextObj ) {
		SaveCustomCreation( scanpo, fp );
	}
}


// save command to simulate depression of key ---------------------------------
//
PRIVATE
void SaveKeyPress( int indx )
{
	ASSERT( ( indx >= 0 ) && ( (dword)indx < NUM_GAMEFUNC_KEYS ) );

	if ( ( indx >= 0 ) && ( (dword)indx < NUM_GAMEFUNC_KEYS ) ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_SIMULKEYPRESS ), indx );
	}
}


// save command to simulate depression of key ---------------------------------
//
PRIVATE
void SaveKeyRelease( int indx )
{
	ASSERT( ( indx >= 0 ) && ( (dword)indx < NUM_GAMEFUNC_KEYS ) );

	if ( ( indx >= 0 ) && ( (dword)indx < NUM_GAMEFUNC_KEYS ) ) {
		ASSERT( RecordingFp != NULL );
		SaveWaitCommand();
		fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_SIMULKEYRELEASE ), indx );
	}
}


// record firing of weapons if recording active -------------------------------
//
void REC_RecordFiring()
{
	//NOTE:
	// this function is called by INP_USER::INP_UserCheckFiring()
	// to record firing of weapons literally (not just by recording
	// the resulting object creations).
	//
	// NOT USED AT THE MOMENT.

	if ( RecordingActive ) {
	}
}


// record key depressions of specific keys if recording active ----------------
//
void REC_RecordKeys()
{
	//NOTE:
	// this function is called by INP_USER::INP_UserProcessInput()
	// to record special keystrokes literally (that is, where not
	// just their results are recorded).

	if ( RecordingActive ) {

		//NOTE:
		// if the object camera is deactivated by pressing
		// <ESC> instead of the activation key, this will
		// also be recorded here, because this keydown event
		// will be translated by G_MAIN::SpecialKeyFunctions().

		// activate/deactive object camera
		if ( DepressedKeys->key_ToggleObjCamera ) {
			SaveKeyPress( offsetof( keyfunc_s, key_ToggleObjCamera ) );
		}

		// cycle weapons
		if ( DepressedKeys->key_NextWeapon ) {
			SaveKeyPress( offsetof( keyfunc_s, key_NextWeapon ) );
		}

		// cycle missiles
		if ( DepressedKeys->key_NextMissile ) {
			SaveKeyPress( offsetof( keyfunc_s, key_NextMissile ) );
		}

		// cycle targets
		if ( DepressedKeys->key_NextTarget ) {
			SaveKeyPress( offsetof( keyfunc_s, key_NextTarget ) );
		}

		//NOTE:
		// if the object camera is active, zooming in and out
		// have to be handled separately, since they are not
		// tied to the local ship's speed.

		if ( ObjCameraActive ) {

static int acc_active = FALSE;
			if ( DepressedKeys->key_Accelerate ) {
				SaveKeyPress( offsetof( keyfunc_s, key_Accelerate ) );
				acc_active = TRUE;
			} else if ( acc_active ) {
				SaveKeyRelease( offsetof( keyfunc_s, key_Accelerate ) );
				acc_active = FALSE;
			}

static int dec_active = FALSE;
			if ( DepressedKeys->key_Decelerate ) {
				SaveKeyPress( offsetof( keyfunc_s, key_Decelerate ) );
				dec_active = TRUE;
			} else if ( dec_active ) {
				SaveKeyRelease( offsetof( keyfunc_s, key_Decelerate ) );
				dec_active = FALSE;
			}
		}

	}
}

// variable to holde the stride in packets ( either 546 or 1000 bytes ) ------- 
//
static int		prev_session = -1;
static size_t	packet_read_stride = RECORD_PACKET_SIZE;

// create filename for remote packet data -------------------------------------
//
PRIVATE
void CreatePacketName( char *packetname, int session, int packet, int write )
{
	ASSERT( packetname != NULL );
	ASSERT( session >= 0 );
	ASSERT( packet >= 0 );

	if ( AUX_RECORD_PACKETS_INTO_SESSION ) {

		//NOTE:
		// each recording session is stored into its own file.
		// the filename for session 5 would be "RPACK005.PRP".
		// this function also returns the offset in this file
		// at which the packet may be found/should be stored.
		// (packets are simply stored as array in the file.)

		strcpy( packetname, rpacket_basename );
		DIG3_TO_STR( packetname + strlen( rpacket_basename ), session );
		packetname[ strlen( rpacket_basename ) + 3 ] = 0;
		strcat( packetname, rpacket_extension );

	} else {

		//NOTE:
		// all packets of a recording session are stored into
		// a file named like the generated console script.
		// so, if the console script's name is "FLIGHT.CON",
		// the corresponding packets will be stored in
		// "FLIGHT.PRP".

		// just use the current recording's name
		strcpy( packetname, write ? packet_record_name : packet_replay_name );
		strcat( packetname, rpacket_extension );
	}
}

// load previously saved remote packet ----------------------------------------
//
PRIVATE
int LoadPacket( NetPacketExternal* ext_gamepacket, int session, int packet )
{
	ASSERT( ext_gamepacket != NULL );
	ASSERT( session >= 0 );
	ASSERT( packet >= 0 );

	// create name of packet file
	char packetname[ PATH_MAX + 1 ];
	CreatePacketName( packetname, session, packet, FALSE );

	// search paths
	FILE *fp = fopen( packetname, "rb" );
	if ( fp == NULL ) {
		char pathname[ PATH_MAX + 1 ];
		strcpy( pathname, REFCON_COMMANDS_DIR );
		strcat( pathname, packetname );
		fp = fopen( pathname, "rb" );
		if ( fp == NULL ) {
			strcpy( pathname, STDCON_COMMANDS_DIR );
			strcat( pathname, packetname );
			fp = fopen( pathname, "rb" );
			if ( fp == NULL ) {
				strcpy( pathname, RECORD_COMMANDS_DIR );
				strcat( pathname, packetname );
				fp = fopen( pathname, "rb" );
			}
		}
	}

	// load packet data
	if ( fp != NULL ) {

		// check whether this session contains packets with IPX length
		if ( prev_session != session ) {
			ASSERT( packet  == 0 );

			// only read signature
			fseek( fp, 0, SEEK_SET );
			size_t bytesread = fread( ext_gamepacket, 1, PACKET_SIGNATURE_SIZE, fp );
			if ( bytesread != PACKET_SIGNATURE_SIZE ) {
				return FALSE;
			}

			// check for < 0198 signature
			extern const char net_game_signature[];
			if ( strncmp( ext_gamepacket->Signature, net_game_signature, PACKET_SIGNATURE_SIZE ) == 0 ) {
				// set packet read stride to IPX length, this was used in all demos prior to 0198
				packet_read_stride = NET_IPX_DATA_LENGTH;
			} else {
				packet_read_stride = RECORD_PACKET_SIZE;
			}
			
			prev_session = session;
		}

		// calculate seek position based on packet# and stride
		long seekpos = ( packet * packet_read_stride );
		
		fseek( fp, seekpos, SEEK_SET );
		size_t bytesread = fread( ext_gamepacket, 1, packet_read_stride, fp );
		fclose( fp );
		int readok = ( bytesread == packet_read_stride );
		if ( !readok )
			MSGOUT( "E_RECORD::LoadPacket(): read error." );

		return readok;
	}

	return FALSE;
}


// save entire remote packet as binary file -----------------------------------
//
PRIVATE
int SavePacket( NetPacketExternal* ext_gamepacket, int session, int packet )
{
	ASSERT( ext_gamepacket != NULL );
	ASSERT( session >= 0 );
	ASSERT( packet >= 0 );

	// create name of packet file
	char packetname[ PATH_MAX + 1 ];
	CreatePacketName( packetname, session, packet, TRUE );

	// recorded packets always have a stride of RECORD_PACKET_SIZE
	long seekpos = ( packet * RECORD_PACKET_SIZE );
	
	// save packet data
	FILE *fp = fopen( packetname, ( seekpos > 0 ) ? "ab" : "wb" );
	if ( fp != NULL ) {
		fseek( fp, seekpos, SEEK_SET );
		size_t byteswritten = fwrite( ext_gamepacket, 1, RECORD_PACKET_SIZE, fp );
		fclose( fp );
		int writeok = ( byteswritten == RECORD_PACKET_SIZE );
		if ( !writeok )
			MSGOUT( "SavePacket(): write error." );
		return writeok;
	}

	return FALSE;
}

// return whether we are currently recording packets --------------------------
//
int REC_IsRecordingPackets()
{
	return ( RecordingActive && RecordRemotePackets );
}


// this gets called whenever a valid remote packet is received ----------------
//
void REC_RecordRemotePacket( NetPacketExternal* ext_gamepacket )
{
	//NOTE:
	// this function is called by the low-level network code
	// whenever a packet is inserted into the list of valid
	// received packets. for example NET_UDP::ChainPacket().

	ASSERT( ext_gamepacket != NULL );

	if ( RecordingActive && RecordRemotePackets ) {

		ASSERT( RecordingFp != NULL );

		// save packet data
		if ( !SavePacket( ext_gamepacket, RemoteRecSessionId, RemoteRecPacketId ) ) {
			MSGOUT( "remote packet %d.%d not saved.", RemoteRecSessionId, RemoteRecPacketId );
			fprintf( RecordingFp, ";*** packet %d.%d not saved.\n", RemoteRecSessionId, RemoteRecPacketId );
			return;
		}

		// save batch command
		SaveWaitCommand();
		fprintf( RecordingFp, "%s %d %d\n", ACMSTR( ACM_PACKET ), RemoteRecSessionId, RemoteRecPacketId );

#ifdef PACKETINFO_WRITING_AVAILABLE

		if ( AUX_RECORD_VERBOSE_PACKETS )
			NETs_WritePacketInfo( RecordingFp, ext_gamepacket );
#endif
		// increment packet number
		RemoteRecPacketId++;
	}
}


// read and replay remote packet from session file ----------------------------
//
void REC_PlayRemotePacket( int session, int packetno )
{
	//NOTE:
	// this function is called by CON_ACT::AcPlayRemotePacket()
	// to play back a remote packet identified by its session
	// and packet id. the packet is read from the corresponding
	// session file by this function.

	ASSERT( session >= 0 );
	ASSERT( packetno >= 0 );

	//NOTE:
	// simulation will not be enabled for script replay if actual
	// network not available. (in contrast to binary replay.)

	if ( !NetConnected ) {
		MSGOUT( "REC_PlayRemotePacket(): no net (packet %d.%d).", session, packetno );
		return;
	}
	ASSERT( NetConnected == NETWORK_GAME_ON );

	NetPacketExternal* ext_gamepacket = (NetPacketExternal*) ALLOCMEM( RECORD_PACKET_SIZE );

	if ( ext_gamepacket != NULL ) {
		if ( LoadPacket( ext_gamepacket, session, packetno ) ) {

			// reuse binary playback
			REC_PlayRemotePacketBin( ext_gamepacket );

		} else {
			MSGOUT( "remote packet %d.%d not found (replay).", session, packetno );
		}
	}

	//NOTE:
	// free the external packet, as the NETs_HandleInPacket_DEMO in REC_PlayRemotePacketBin already
	// makes a copy of the packet
	FREEMEM( ext_gamepacket );
}


// determines whether packet signature should also be stripped ----------------
//
#define STRIP_HEAD_SIZE		PACKET_SIGNATURE_SIZE 	// strip it
//#define STRIP_HEAD_SIZE	0						// don't strip


// replay remote packet from compiled demo ------------------------------------
//
size_t REC_PlayRemotePacketBin( NetPacketExternal* ext_gamepacket )
{
	//NOTE:
	// this function is called by CON_ACT::ExecBinCommands() to
	// play back a remote packet coming from a binary demo file.
	// Also reused by E_RECORD::REC_PlayRemotePacket

	ASSERT( NetConnected );
	ASSERT( ext_gamepacket != NULL );

	NetPacket* gamepacket = (NetPacket*)ALLOCMEM( NET_MAX_NETPACKET_INTERNAL_LEN );

	size_t psize = 0;

	// translate from external DEMO to internal, return size of external packet
	if ( NETs_HandleInPacket_DEMO( ext_gamepacket, gamepacket, &psize ) ) {

		// chain gamepacket as received
		NETs_InsertVirtualPacket( gamepacket );
	}

	//NOTE:
	// gamepacket MUST not be freed here, as this is done by e.g. NET_UDP::ReleasePacketFromChain
	// NO FREEMEM( gamepacket );

	ASSERT( psize > 0 );
	return psize;
}


// fetch remote packet according to session and packet ids --------------------
//
size_t REC_FetchRemotePacket( NetPacketExternal* ext_gamepacket, int session, int packetno )
{
	//NOTE:
	// this function is called by CON_ACT::WriteActionCommand() to
	// fetch a remote packet for insertion into a binary demo file.

	ASSERT( ext_gamepacket != NULL );
	ASSERT( session >= 0 );
	ASSERT( packetno >= 0 );

	if ( ext_gamepacket != NULL ) {
		if ( LoadPacket( ext_gamepacket, session, packetno ) ) {

#ifdef STRIP_BINARY_PACKETS

			//FIXME: DEMO packet header compression should be done when converting 
			//       from normal packets to DEMO packets in NETs_HandleOutPacket_DEMO

			// return only used part of packet ( strip excess zero bytes )
			size_t psize = NETs_NetPacketExternal_DEMO_GetSize( ext_gamepacket );
			return psize;
#else
			// return entire packet
			return RECORD_PACKET_SIZE;
#endif
		} else {
			MSGOUT( "remote packet %d.%d not found (compile).", session, packetno );
		}
	}

	return 0;
}


// this gets called whenever a tcp message from the gameserver is received ----
//
void REC_RecordServerMessage( char *message )
{
	ASSERT( message != NULL );

	if ( RecordingActive ) {

		ASSERT( RecordingFp != NULL );

		// save batch command
		if ( message[ 0 ] != '\0' ) {
			fprintf( RecordingFp, "%s %s\n", ACMSTR( ACM_SERVERMSG ), message );
		}
	}
}


// save matrices to record later on -------------------------------------------
//
void REC_InitMatrices()
{
	//NOTE:
	// this function is used to remember certain matrices that
	// have to be saved in this state but will already have
	// been modified when they are actually saved to file.

//FIXME:
//need this also for demo cutting
//	if ( RecordingActive ) {
		memcpy( &RecordShipViewCamera, &ShipViewCamera, sizeof( Camera ) );
//	}
}


// write command to reestablish current rotation of camera --------------------
//
PRIVATE
void WriteCameraRotation( FILE *fp )
{
	//NOTE:
	// this function is also used by REC_RecordActions()
	// if absolute recording is turned on.

	ASSERT( fp != NULL );

	fprintf( fp, "%s %f %f %f\n", ACMSTR( ACM_CAMROT1 ),
			 GEOMV_TO_FLOAT( RecordShipViewCamera[ 0 ][ 0 ] ),
			 GEOMV_TO_FLOAT( RecordShipViewCamera[ 1 ][ 0 ] ),
			 GEOMV_TO_FLOAT( RecordShipViewCamera[ 2 ][ 0 ] ) );
	fprintf( fp, "%s %f %f %f\n", ACMSTR( ACM_CAMROT2 ),
			 GEOMV_TO_FLOAT( RecordShipViewCamera[ 0 ][ 1 ] ),
			 GEOMV_TO_FLOAT( RecordShipViewCamera[ 1 ][ 1 ] ),
			 GEOMV_TO_FLOAT( RecordShipViewCamera[ 2 ][ 1 ] ) );
	fprintf( fp, "%s %f %f %f\n", ACMSTR( ACM_CAMROT3 ),
			 GEOMV_TO_FLOAT( RecordShipViewCamera[ 0 ][ 2 ] ),
			 GEOMV_TO_FLOAT( RecordShipViewCamera[ 1 ][ 2 ] ),
			 GEOMV_TO_FLOAT( RecordShipViewCamera[ 2 ][ 2 ] ) );
}


// write command to reestablish current origin of camera ----------------------
//
PRIVATE
void WriteCameraOrigin( FILE *fp )
{
	//NOTE:
	// this function is also used by REC_RecordActions()
	// if absolute recording is turned on.

	ASSERT( fp != NULL );

	fprintf( fp, "%s %f %f %f\n", ACMSTR( ACM_CAMORIGIN ),
			 GEOMV_TO_FLOAT( RecordShipViewCamera[ 0 ][ 3 ] ),
			 GEOMV_TO_FLOAT( RecordShipViewCamera[ 1 ][ 3 ] ),
			 GEOMV_TO_FLOAT( RecordShipViewCamera[ 2 ][ 3 ] ) );
}


// save camera position and orientation ---------------------------------------
//
void Save_Camera( FILE *fp )
{
	ASSERT( fp != NULL );

	WriteCameraRotation( fp );
	WriteCameraOrigin( fp );
}


// save pseudo star positions -------------------------------------------------
//
void Save_PseudoStars( FILE *fp )
{
	ASSERT( fp != NULL );

	for ( int sid = 0; sid < NumPseudoStars; sid++ ) {

		fprintf( fp, "%s %f %f %f\n", ACMSTR( ACM_ADDPSTAR ),
				 GEOMV_TO_FLOAT( PseudoStars[ sid ].X ),
				 GEOMV_TO_FLOAT( PseudoStars[ sid ].Y ),
				 GEOMV_TO_FLOAT( PseudoStars[ sid ].Z ) );
	}
}


// use same commands for camera and object camera -----------------------------
//
#define ACM_OBJCAMPITCH		ACM_PITCH
#define ACM_OBJCAMYAW		ACM_YAW
#define ACM_OBJCAMROLL		ACM_ROLL


// write action command if pitch changed --------------------------------------
//
INLINE
int CheckPitch( int write )
{
	int command = ObjCameraActive ? ACM_OBJCAMPITCH : ACM_PITCH;

	if ( LastPitch != RecPitch ) {
		if ( write ) {
			LastPitch = RecPitch;
			fprintf( RecordingFp, "%s %d\n", ACMSTR( command ),
					 -LastPitch );
		}
		return 1;
	}

	return 0;
}


// write action command if yaw changed ----------------------------------------
//
INLINE
int CheckYaw( int write )
{
	int command = ObjCameraActive ? ACM_OBJCAMYAW : ACM_YAW;

	if ( LastYaw != RecYaw ) {
		if ( write ) {
			LastYaw = RecYaw;
			fprintf( RecordingFp, "%s %d\n", ACMSTR( command ),
					 -LastYaw );
		}
		return 1;
	}

	return 0;
}


// write action command if roll changed ---------------------------------------
//
INLINE
int CheckRoll( int write )
{
	int command = ObjCameraActive ? ACM_OBJCAMROLL : ACM_ROLL;

	if ( LastRoll != RecRoll ) {
		if ( write ) {
			LastRoll = RecRoll;
			fprintf( RecordingFp, "%s %d\n", ACMSTR( command ),
					 -LastRoll );
		}
		return 1;
	}

	return 0;
}


// write action command if horizontal slide changed ---------------------------
//
INLINE
int CheckSlideHorz( int write )
{
	if ( !ObjCameraActive ) {

		if ( LastSlideHorz != RecSlideHorz ) {
			if ( write ) {
				LastSlideHorz = RecSlideHorz;
				fprintf( RecordingFp, "%s %f\n", ACMSTR( ACM_SLIDEX ),
						 GEOMV_TO_FLOAT( LastSlideHorz ) );
			}
			return 1;
		}
	}

	return 0;
}


// write action command if horizontal slide changed ---------------------------
//
INLINE
int CheckSlideVert( int write )
{
	if ( !ObjCameraActive ) {

		if ( LastSlideVert != RecSlideVert ) {
			if ( write ) {
				LastSlideVert = RecSlideVert;
				fprintf( RecordingFp, "%s %f\n", ACMSTR( ACM_SLIDEY ),
						 GEOMV_TO_FLOAT( LastSlideVert ) );
			}
			return 1;
		}
	}

	return 0;
}


// write action command if speed changed --------------------------------------
//
INLINE
int CheckSpeed( int write )
{
	if ( LastSpeed != MyShip->CurSpeed ) {
		if ( write ) {
			LastSpeed = MyShip->CurSpeed;
			fprintf( RecordingFp, "%s %d\n", ACMSTR( ACM_MOVE ),
					 LastSpeed );
		}
		return 1;
	}

	return 0;
}


// record changes into batch file if recording active -------------------------
//
void REC_RecordActions()
{
	//NOTE:
	// this function is called from within the gameloop
	// (G_MAIN::GameLoop()) to record the user's actions
	// (motion) since the last frame.

	if ( !RecordingActive )
		return;

	ASSERT( RecordingFp != NULL );

	int rotchange = 0;

	// check if pitch/yaw/roll have changed
	// since they were last saved (just check)
	rotchange += CheckPitch( FALSE );
	rotchange += CheckYaw( FALSE );
	rotchange += CheckRoll( FALSE );

	// check if absolute values should be recorded regardless of change
	if ( RecordAbsolutePosition && !ObjCameraActive ) {
		// if nothing at all going on still don't record anything
		if ( ( RecPitch != 0 ) || ( RecYaw != 0 ) || ( RecRoll != 0 ) )
			rotchange += 1;
	}

	int poschange = 0;

	// check if position has changed
	// since it was last saved
	poschange += CheckSlideHorz( FALSE );
	poschange += CheckSlideVert( FALSE );
	poschange += CheckSpeed( FALSE );

	if ( rotchange || poschange ) {

		// write idle wait if not already done
		SaveWaitCommand();

		// save pitch/yaw/roll to file if they have
		// changed and then remember them internally
		CheckPitch( TRUE );
		CheckYaw( TRUE );
		CheckRoll( TRUE );

		// save absolute values to file if desired
		if ( RecordAbsolutePosition && !ObjCameraActive ) {
			WriteCameraRotation( RecordingFp );
			WriteCameraOrigin( RecordingFp );
		}

		// save position changes to file
		CheckSlideHorz( TRUE );
		CheckSlideVert( TRUE );
		CheckSpeed( TRUE );
	}

	// clear flag
	idlewait_inserted = FALSE;

	// increase duration count
	IdleDuration += CurScreenRefFrames;
}



