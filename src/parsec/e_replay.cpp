/*
 * PARSEC - Demo Script Replay
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:22 $
 *
 * Orginally written by:
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

// mathematics header
#include "utl_math.h"

// local module header
#include "e_replay.h"

// proprietary module headers
#include "con_aux.h"
#include "con_ext.h"
#include "e_demo.h"
#include "g_supp.h"



// used to temporarily store inverse of object camera -------------------------
//
static Camera objcam_inverse;


// perform automatic pitch ----------------------------------------------------
//
INLINE
void PerformAutomaticPitch()
{
	if ( AutomaticPitch != 0 ) {

		if ( !ObjCameraActive ) {

			if ( ReplayObjCamActive )
				return;

			bams_t angle = AutomaticPitch * CurScreenRefFrames;

			CamRotX( ShipViewCamera, angle );
			CamRotX( PseudoStarMovement, angle );

			CurPitch += -AutomaticPitch;
			RecPitch += -AutomaticPitch;

		} else {

			bams_t angle = AutomaticPitch * CurScreenRefFrames;

			CalcOrthoInverse( ObjectCamera, objcam_inverse );
			MtxMtxMUL( objcam_inverse, PseudoStarMovement, DestXmatrx );
			CamRotX( DestXmatrx, -angle * 4 );
			MtxMtxMUL( ObjectCamera, DestXmatrx, PseudoStarMovement );

			if ( ReplayObjCamActive ) {
				ObjRotX( ObjectCamera, -angle * 4 );
			} else {
				CamRotX( ShipViewCamera, angle );
				RecPitch += -AutomaticPitch;
			}
		}
	}
}


// perform automatic yaw ------------------------------------------------------
//
INLINE
void PerformAutomaticYaw()
{
	if ( AutomaticYaw != 0 ) {

		if ( !ObjCameraActive ) {

			if ( ReplayObjCamActive )
				return;

			bams_t angle = AutomaticYaw * CurScreenRefFrames;

			CamRotY( ShipViewCamera, angle );
			CamRotY( PseudoStarMovement, angle );

			CurYaw += -AutomaticYaw;
			RecYaw += -AutomaticYaw;

		} else {

			bams_t angle = AutomaticYaw * CurScreenRefFrames;

			CalcOrthoInverse( ObjectCamera, objcam_inverse );
			MtxMtxMUL( objcam_inverse, PseudoStarMovement, DestXmatrx );
			CamRotY( DestXmatrx, -angle * 4 );
			MtxMtxMUL( ObjectCamera, DestXmatrx, PseudoStarMovement );

			if ( ReplayObjCamActive ) {
				ObjRotY( ObjectCamera, -angle * 4 );
			} else {
				CamRotY( ShipViewCamera, angle );
				RecYaw += -AutomaticPitch;
			}
		}
	}
}


// perform automatic roll -----------------------------------------------------
//
INLINE
void PerformAutomaticRoll()
{
	if ( AutomaticRoll != 0 ) {

		if ( !ObjCameraActive ) {

			if ( ReplayObjCamActive )
				return;

			bams_t angle = AutomaticRoll * CurScreenRefFrames;

			CamRotZ( ShipViewCamera, angle );
			CamRotZ( PseudoStarMovement, angle );

			CurRoll += -AutomaticRoll;
			RecRoll += -AutomaticRoll;

		} else {

			bams_t angle = AutomaticRoll * CurScreenRefFrames;

			CalcOrthoInverse( ObjectCamera, objcam_inverse );
			MtxMtxMUL( objcam_inverse, PseudoStarMovement, DestXmatrx );
			CamRotZ( DestXmatrx, -angle * 2 );
			MtxMtxMUL( ObjectCamera, DestXmatrx, PseudoStarMovement );

			if ( ReplayObjCamActive ) {
				ObjRotZ( ObjectCamera, -angle * 2 );
			} else {
				CamRotZ( ShipViewCamera, angle );
				RecRoll += -AutomaticRoll;
			}
		}
	}
}


// perform automatic horizontal sliding ---------------------------------------
//
INLINE
void PerformAutomaticSlideHorz()
{
	if ( AutomaticSlideHorz != 0 ) {

		if ( !ObjCameraActive ) {

			geomv_t slideval = AutomaticSlideHorz * CurScreenRefFrames;

			ShipViewCamera[ 0 ][ 3 ]	 += slideval;
			PseudoStarMovement[ 0 ][ 3 ] += slideval;

			RecSlideHorz -= AutomaticSlideHorz;
			CurSlideHorz -= AutomaticSlideHorz;
		}
	}
}


// perform automatic vertical sliding -----------------------------------------
//
INLINE
void PerformAutomaticSlideVert()
{
	if ( AutomaticSlideVert != 0 ) {

		if ( !ObjCameraActive ) {

			geomv_t slideval = AutomaticSlideVert * CurScreenRefFrames;

			ShipViewCamera[ 1 ][ 3 ]	 += slideval;
			PseudoStarMovement[ 1 ][ 3 ] += slideval;

			RecSlideVert -= AutomaticSlideVert;
			CurSlideVert -= AutomaticSlideVert;
		}
	}
}


// perform automatic movement -------------------------------------------------
//
INLINE
void PerformAutomaticMovement()
{
	if ( AutomaticMovement != 0 ) {
		MoveLocalShip();
	}
}


// do necessary actions for discrete interval of time -------------------------
//
INLINE
void DoActions()
{
	if ( CurScreenRefFrames == 0 )
		return;

	PerformAutomaticPitch();
	PerformAutomaticYaw();
	PerformAutomaticRoll();
	PerformAutomaticSlideHorz();
	PerformAutomaticSlideVert();
	PerformAutomaticMovement();
}


// execute next batch of action commands --------------------------------------
//
INLINE
void FetchNewActions()
{
	// reset wait counter
	CurActionWait = 0;

	// binary replay has precedence
	if ( DEMO_BinaryReplayActive() ) {

		// advance in binary demo
		DEMO_BinaryExecCommands( FALSE );

	} else if ( ScriptReplayActive() ) {

		// restart script and advance
		RestartExternalCommand( FALSE );
	}
}


// immediately stop all automatic actions -------------------------------------
//
void REPLAY_StopAutomaticActions()
{
//	ASSERT( CurActionWait == 0 );
//	ASSERT( !DEMO_ReplayActive() );

	AutomaticPitch		= BAMS_DEG0;
	AutomaticYaw		= BAMS_DEG0;
	AutomaticRoll		= BAMS_DEG0;
	AutomaticSlideHorz	= GEOMV_0;
	AutomaticSlideVert	= GEOMV_0;
	AutomaticMovement	= 0;
	ReplayObjCamActive	= FALSE;
}


// perform automatic actions --------------------------------------------------
//
void REPLAY_PerformAutomaticActions()
{
	//NOTE:
	// this function gets called once per frame
	// from the game loop (G_MAIN.C) to allow
	// automatic script/demo replay.

	// perform actions if no idlewait active
	if ( CurActionWait == 0 ) {

		DoActions();

	// check if wait timed out during last frame interval
	} else if ( CurActionWait <= CurScreenRefFrames ) {

		// count the elapsed wait interval
		DEMO_CountDemoFrame();

		// save original CurScreenRefFrames
		refframe_t csrf = CurScreenRefFrames;

		// first interval
		CurScreenRefFrames = CurActionWait;
		DoActions();

		// calc second interval
		CurScreenRefFrames = csrf - CurActionWait;

		// execute next commands
		FetchNewActions();

		// do remaining intervals
		while ( CurScreenRefFrames > 0 ) {

			// exit if no wait
			if ( CurActionWait == 0 ) {
				DoActions();
				break;
			}

			// exit if wait doesn't terminate immediately
			if ( CurActionWait > CurScreenRefFrames ) {
				CurActionWait -= CurScreenRefFrames;
				DoActions();
				break;
			}

			// count the elapsed wait interval
			DEMO_CountDemoFrame();

			// save last CurScreenRefFrames
			int lastcsrf = CurScreenRefFrames;

			// next interval
			CurScreenRefFrames = CurActionWait;
			DoActions();

			// calc remaining interval
			CurScreenRefFrames = lastcsrf - CurActionWait;

			// execute next commands
			FetchNewActions();
		}

		// restore CurScreenRefFrames
		CurScreenRefFrames = csrf;

	} else {

		// wait interval still in progress
		CurActionWait -= CurScreenRefFrames;
		DoActions();
	}
}



