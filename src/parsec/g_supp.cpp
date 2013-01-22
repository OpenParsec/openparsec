/*
 * PARSEC - Supporting Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:37 $
 *
 * Orginally written by:
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
#include "inp_defs.h"
#include "net_defs.h"

// drawing subsystem
#include "d_font.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "g_supp.h"

// proprietary module headers
#include "con_aux.h"
#include "con_com.h"
#include "con_main.h"
#include "g_camera.h"
#include "g_stars.h"
#include "h_drwhud.h"
#include "obj_ctrl.h"
#include "obj_game.h"
#include "obj_part.h"
#include "part_api.h"
#include "g_sfx.h"
#include "g_wfx.h"
#include "g_emp.h"


// flags
#define PROPULSION_PARTICLES_POSSIBLE
#define ALLOW_CROSSHAIR_TARGET



// join position control ------------------------------------------------------
//
#define JP_M_S_D 		500				// minimum join position distance
#define JP_RANGE 		1000			// maximum join position range
#define JP_OFS			(JP_RANGE/2)


// reset position control -----------------------------------------------------
//
#define RESET_RANGE 	1000
#define RESET_OFS		(RESET_RANGE/2)


// init exit function keys for info screen ------------------------------------
//
void PrepInfoScreenKeys()
{
	DepressedKeys->key_Escape = 0;
	SELECT_KEYS_RESET();
}


// check if exit function invoked in info screen ------------------------------
//
int CheckInfoScreenExit()
{
	if ( DepressedKeys->key_Escape || SELECT_KEYS_PRESSED() ) {
		DepressedKeys->key_Escape = 0;
		SELECT_KEYS_RESET();

		return TRUE;
	}

	return FALSE;
}


// init structure of local ship (central helper function) ---------------------
//
PRIVATE
void InitMyShipStructure( dword myclassid )
{
	// free attached clusters
	PRT_FreeAttachedClusterList( MyShip );

	//NOTE:
	// if this is called from within a demo it is not entirely sure
	// that the supplied class id is indeed valid. this may happen
	// if a recorded ship is not loaded during replay, for instance.

	// fall back to first ship if class is invalid
	if ( myclassid >= (dword)NumObjClasses ) {
		myclassid = SHIP_CLASS_1;
	}
	ASSERT( ObjClasses[ myclassid ] != NULL );
	if ( !OBJECT_TYPE_SHIP( ObjClasses[ myclassid ] ) ) {
		myclassid = SHIP_CLASS_1;
	}

	// re-init ship object structure
	ShipObject *shippo = (ShipObject *) ObjClasses[ myclassid ];
	ASSERT( shippo->InstanceSize <= MyShipMaxInstanceSize );
	memcpy( MyShip, shippo, shippo->InstanceSize );
	OBJ_CorrectObjectInstance( MyShip, shippo );

	// attach static particles that are
	// part of the object instance
	if ( AUX_ATTACH_OBJECT_PARTICLES ) {
		OBJ_AttachClassParticles( MyShip );
	}
}


// init local player's ship ---------------------------------------------------
//
void InitLocalShipStatus( dword myclassid )
{
	// init local ship object structure
	InitMyShipStructure( myclassid );

	ShipDowned = FALSE;
}


// advance camera in direction of flight --------------------------------------
//
void MoveLocalShip()
{
	if ( AUX_FLAGWORD_SCREENSHOT_HELPERS & 0x01 ) {
		// allow position advance disabling for screenshots
		return;
	}

	fixed_t speedadvance = MyShip->CurSpeed * CurScreenRefFrames;

	if ( speedadvance != 0 ) {

		ShipViewCamera[ 2 ][ 3 ] -= FIXED_TO_GEOMV( speedadvance );

		if ( !ObjCameraActive ) {

			PseudoStarMovement[ 2 ][ 3 ] -= FIXED_TO_GEOMV( speedadvance );
//			  CameraMoved = 1;

		} else {

			Vector3 dirvec;
			DirVctMUL( ObjectCamera, FIXED_TO_GEOMV( speedadvance ), &dirvec );
			PseudoStarMovement[ 0 ][ 3 ] -= dirvec.X;
			PseudoStarMovement[ 1 ][ 3 ] -= dirvec.Y;
			PseudoStarMovement[ 2 ][ 3 ] -= dirvec.Z;
//			  CameraMoved = 1;
		}

#ifdef PROPULSION_PARTICLES_POSSIBLE

		if ( AUX_CREATE_PROPULSION_FUMES ) {

			if ( ( MyShip->FumeCount -= CurScreenRefFrames ) < 0 ) {

				int fcadd = ( MyShip->MaxSpeed - MyShip->CurSpeed ) / 256;
				MyShip->FumeCount = MyShip->FumeFreq + fcadd;

				SFX_CreatePropulsionParticles( MyShip );
			}
		}
#endif

	}
}


// init object camera ---------------------------------------------------------
//
void ObjCamOn()
{
	if ( !ObjCameraActive ) {

		ObjCameraActive = 1;

		extern int objcam_textcount; // defined in H_DRWHUD.C
		objcam_textcount = 0;

		// insert local player's ship into ship objects list
		ASSERT( PShipObjects->NextObj != MyShip );
		ASSERT( MyShip->NextObj == NULL );
		MyShip->NextObj		  = PShipObjects->NextObj;
		PShipObjects->NextObj = MyShip;

		// init start matrix of object camera (euler angles and distance)
		MakeIdMatrx( ObjectCamera );
		CamRotX( ObjectCamera, MyShip->ObjCamStartPitch );
		CamRotY( ObjectCamera, MyShip->ObjCamStartYaw );
		CamRotZ( ObjectCamera, MyShip->ObjCamStartRoll );
		ObjectCamera[ 2 ][ 3 ] += MyShip->ObjCamStartDist;

		// do new random placement of pseudo stars
		NumPseudoStars = 0;
		InitPseudoStars();
	}
}


// kill object camera ---------------------------------------------------------
//
void ObjCamOff()
{
	if ( ObjCameraActive ) {

		ObjCameraActive = 0;

		// unlink local player's ship from ship objects list
		ASSERT( PShipObjects->NextObj == MyShip );
		PShipObjects->NextObj = MyShip->NextObj;
		MyShip->NextObj		  = NULL;

		// do new random placement of pseudo stars
		NumPseudoStars = 0;
		InitPseudoStars();
	}
}


// handle object camera enabling/disabling via keypress -----------------------
//
void ObjectCameraControl()
{
	// toggle object camera
	if ( DepressedKeys->key_ToggleObjCamera ) {
		DepressedKeys->key_ToggleObjCamera = 0;

		// toggling only allowed outside entrymode/floating menu
		if ( GAME_MODE_ACTIVE() ) {

			if ( ObjCameraActive ) {
				ObjCamOff();
			} else {
				ObjCamOn();
			}
		}
	}
}


// init local ship in floating menu -------------------------------------------
//
void InitFloatingMyShip()
{
	// (re-)init local ship object structure
	InitMyShipStructure( MyShip->ObjectClass );

	// init pseudo- and fixed stars
	NumPseudoStars = 0;
	InitPseudoStars();

	// select standard laser and dumb missile
	SelectedLaser 	= 0;
	SelectedMissile = 0;

	// remove target lock
	TargetObjNumber = TARGETID_NO_TARGET;
	TargetVisible	= FALSE;
}


// init ship position for joining network game --------------------------------
//
void InitJoinPosition()
{
	// get pointer to another ship (any other ship will do)
	GenObject *shippo = FetchFirstShip();

	// change position only when in PEER mode. in GMSV join position comes from server
	if ( NET_ConnectedPEER() ) {

		MakeIdMatrx( ShipViewCamera );
		if ( shippo != NULL ) {

			int xdist = ( RAND() % JP_RANGE ) - JP_OFS;
			int ydist = ( RAND() % JP_RANGE ) - JP_OFS;
			int zdist = ( RAND() % JP_RANGE ) - JP_OFS;

			xdist += ( xdist < 0 ) ? -JP_M_S_D : JP_M_S_D;
			ydist += ( ydist < 0 ) ? -JP_M_S_D : JP_M_S_D;
			zdist += ( zdist < 0 ) ? -JP_M_S_D : JP_M_S_D;

			ShipViewCamera[ 0 ][ 3 ] = -shippo->ObjPosition[ 0 ][ 3 ] + INT_TO_GEOMV( xdist );
			ShipViewCamera[ 1 ][ 3 ] = -shippo->ObjPosition[ 1 ][ 3 ] + INT_TO_GEOMV( ydist );
			ShipViewCamera[ 2 ][ 3 ] = -shippo->ObjPosition[ 2 ][ 3 ] + INT_TO_GEOMV( zdist );
		}
	}

	// reset view camera smoothing filter
	CAMERA_ResetFilter();

	// (re-)init local ship object structure
	InitMyShipStructure( MyShip->ObjectClass );

	//NOTE:
	// the call of CalcOrthoInverse() is not really necessary
	// here, since the matrix is translation-only anyway.
	// maybe this will change in the future, but not likely.

	// store ship position in world-space
	CalcOrthoInverse( ShipViewCamera, MyShip->ObjPosition );

	// init pseudo- and fixed stars
	NumPseudoStars = 0;
	InitPseudoStars();

	// select standard laser and dumb missile
	SelectedLaser 	= 0;
	SelectedMissile = 0;

	// remove target lock
	TargetObjNumber = TARGETID_NO_TARGET;
	TargetVisible	= FALSE;
}


// prepare join replayed from recorded network game ---------------------------
//
void InitJoinReplay()
{
	// (re-)init local ship object structure
	InitMyShipStructure( MyShip->ObjectClass );

	MakeIdMatrx( ShipViewCamera );
	MakeIdMatrx( MyShip->ObjPosition );

	// init pseudo- and fixed stars
	NumPseudoStars = 0;
	InitPseudoStars();

	// select standard laser and dumb missile
	SelectedLaser 	= 0;
	SelectedMissile = 0;

	// remove target lock
	TargetObjNumber = TARGETID_NO_TARGET;
	TargetVisible	= FALSE;
}


// set origin of ship to defined location (called by console command) ---------
//
void Cmd_SetShipOrigin()
{
	// init position in proximity of origin (0,0,0)
	MakeIdMatrx( ShipViewCamera );
	geomv_t xdist = INT_TO_GEOMV( ( RAND() % RESET_RANGE ) - RESET_OFS );
	geomv_t ydist = INT_TO_GEOMV( ( RAND() % RESET_RANGE ) - RESET_OFS );
	geomv_t zdist = INT_TO_GEOMV( ( RAND() % RESET_RANGE ) - RESET_OFS );

	ShipViewCamera[ 0 ][ 3 ] = xdist;
	ShipViewCamera[ 1 ][ 3 ] = ydist;
	ShipViewCamera[ 2 ][ 3 ] = zdist;

	// reset view camera smoothing filter
	CAMERA_ResetFilter();

	// (re-)init local ship object structure
	InitMyShipStructure( MyShip->ObjectClass );

	//NOTE:
	// the call of CalcOrthoInverse() is not really necessary
	// here, since the matrix is translation-only anyway.
	// maybe this will change in the future, but not likely.

	// store ship position in world-space
	CalcOrthoInverse( ShipViewCamera, MyShip->ObjPosition );

	// init pseudo- and fixed stars
	NumPseudoStars = 0;
	InitPseudoStars();

	// select standard laser and dumb missile
	SelectedLaser 	= 0;
	SelectedMissile = 0;

	// remove target lock
	TargetObjNumber = TARGETID_NO_TARGET;
	TargetVisible	= FALSE;
}


// select target at crosshair -------------------------------------------------
//
void SelectCrosshairTarget()
{

#ifdef ALLOW_CROSSHAIR_TARGET

	if ( ObjCameraActive )
		return;

	// walk ship list and select first ship encountered
	// as target if in tracking bounding-box
	ShipObject *shippo = FetchFirstShip();

	for ( ; shippo; shippo = (ShipObject *) shippo->NextObj ) {

		// check if ship allowed as target
		if ( ( shippo->ExplosionCount == 0 ) ||
			 ( shippo->ExplosionCount >= BM_EXPLNOTARGFRAME * EXPL_REF_SPEED ) ) {

			Vertex3 Pos, ViewPos;

			// position of ship in viewspace
			FetchTVector( shippo->ObjPosition, &Pos );
			MtxVctMUL( ShipViewCamera, &Pos, &ViewPos );

			// cull objects behind local ship
			if ( ViewPos.Z <= GEOMV_0 )
				continue;

			// calc position of ship in screenspace
			SPoint sloc;
			PROJECT_TO_SCREEN( ViewPos, sloc );

			extern int TrackingLeftBoundX;
			extern int TrackingRightBoundX;
			extern int TrackingLeftBoundY;
			extern int TrackingRightBoundY;

			// check if ship in tracking bounding-box
			if ( ( sloc.X > TrackingLeftBoundX ) &&
				 ( sloc.X < TrackingRightBoundX ) &&
				 ( sloc.Y > TrackingLeftBoundY ) &&
				 ( sloc.Y < TrackingRightBoundY	) ) {

				// select ship as current target
				TargetObjNumber = shippo->HostObjNumber;
				TargetVisible   = TRUE;

				if ( ( shippo->HostObjNumber & 0xffff ) == 0 )
					TargetRemId = GetObjectOwner( shippo );
				else
					TargetRemId = TARGETID_NO_TARGET;

				TargetScreenX = sloc.X;
				TargetScreenY = sloc.Y;

				return;
			}
		}
	}

#endif

}


// fetch next target ----------------------------------------------------------
//
void SelectNextTarget()
{
	dword nexttarget   = TARGETID_NO_TARGET;
	GenObject *objscan = FetchFirstShip();

	// take listhead as default-target
	if ( objscan != NULL ) {
		nexttarget = objscan->HostObjNumber;
	}

	// search for current target
	while ( objscan && ( objscan->HostObjNumber != TargetObjNumber ) )
		objscan = objscan->NextObj;

	// if there is an object behind the current target in list take it
	if ( objscan && objscan->NextObj ) {
		nexttarget = objscan->NextObj->HostObjNumber;
	}

	TargetObjNumber = nexttarget;
}


// do ship intelligence -------------------------------------------------------
//
void ShipIntelligence()
{
	static fixed_t old_speed = 0;

	// ensure old speed gets reset on join
	if ( MyShip->CurSpeed == 0 ) {
		old_speed = 0;
	}

	// (re)start thrust sample on speed change
	if ( old_speed != MyShip->CurSpeed ) {
		old_speed = MyShip->CurSpeed;
		AUD_StartThrust( THRUST_TYPE_NORMAL );
	}

	// (re)start slidethrust sample on slide change
	if ( ( CurSlideHorz != GEOMV_0 ) || ( CurSlideVert != GEOMV_0 ) ) {
		AUD_StartThrust( THRUST_TYPE_SLIDE );
	}

	// perform afterburner energy depletion
	if ( MyShip->afterburner_active ) {

		MyShip->afterburner_energy -= CurScreenRefFrames;
		if ( MyShip->afterburner_energy <= 0 ) {
			OBJ_DisableAfterBurner( MyShip );
		}
	}

	// check autotargeting
	if ( MyShip->Specials & SPMASK_AUTOTARGETING ) {

		// automatically select next target if old target lost
		GenObject *targetpo = FetchHostObject( TargetObjNumber );
		if ( targetpo == NULL ) {

//			MSGOUT( "target lost" );
			SelectNextTarget();
		}
	}
}


// call duration weapons maintenance functions --------------------------------
//
void MaintainDurationWeapons( int playerid )
{
	ASSERT( ( playerid == LocalPlayerId ) || NetConnected );
	ASSERT( ( playerid != LocalPlayerId ) ||
			( NetConnected != NETWORK_GAME_ON ) ||
			( NET_FetchOwnersShip( playerid ) == MyShip ) );

	ShipObject *shippo = ( playerid == LocalPlayerId ) ? MyShip : NET_FetchOwnersShip( playerid );
	ASSERT( shippo != NULL );

	// maintain helix
	if ( shippo->WeaponsActive & WPMASK_CANNON_HELIX ) {
		WFX_MaintainHelix( shippo, playerid );
	}

	// maintain lightning
	if ( shippo->WeaponsActive & WPMASK_CANNON_LIGHTNING ) {
		WFX_MaintainLightning( shippo );
	}

	// maintain photon
	photon_sphere_pcluster_s* cluster = (photon_sphere_pcluster_s *)
		PRT_ObjectHasAttachedClustersOfType( shippo, SAT_PHOTON );
	if ( cluster != NULL ) {
		WFX_CalcPhotonSphereAnimation( cluster );
	}

	// maintain emp
	if ( shippo->WeaponsActive & WPMASK_DEVICE_EMP ) {
		WFX_CreateEmpWaves( shippo );
	}
}


// kill duration weapons (make sure they are properly turned off) -------------
//
void KillDurationWeapons( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	// kill helix
	if ( shippo->WeaponsActive & WPMASK_CANNON_HELIX ) {

		// stop sound
		AUD_HelixOff( shippo );
	}

	// kill lightning
	if ( shippo->WeaponsActive & WPMASK_CANNON_LIGHTNING ) {

		// stop sound
		AUD_LightningOff( shippo );
	}

	// kill photon
	if ( shippo->WeaponsActive & WPMASK_CANNON_PHOTON ) {

		// stop sound
		AUD_PhotonLoadingOff( shippo );
	}

	// kill emp
//	if ( shippo->WeaponsActive & WPMASK_DEVICE_EMP ) {
		// nothing to do
//	}
}


// select crosshair target ----------------------------------------------------
//
PRIVATE
int Cmd_CROSSTARGET( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// crosstarget_command ::= 'crosstarget'

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	if ( strtok( paramstr, " " ) == NULL ) {

		if ( GAME_MODE_ACTIVE() ) {
			SelectCrosshairTarget();
		}

	} else {
		CON_AddLine( too_many_args );
	}

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( G_SUPP )
{

#ifdef ALLOW_CROSSHAIR_TARGET

	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "crosstarget" command
	regcom.command	 = "crosstarget";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_CROSSTARGET;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

#endif // ALLOW_CROSSHAIR_TARGET

}



