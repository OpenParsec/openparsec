/*
 * PARSEC - 
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:36 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
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
#include <stdarg.h>
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
//#include "aud_defs.h"
#include "inp_defs.h"
#include "net_defs.h"
#include "sys_defs.h"
//#include "vid_defs.h"

// mathematics header
#include "utl_math.h"

// proprietary headers
#include "con_arg.h"
#include "con_int.h"
#include "con_com.h"
#include "con_main.h"
#include "e_callbk.h"
#include "inp_user.h"
#include "m_main.h"
#include "net_conn.h"
#include "obj_ctrl.h"
#include "utl_logfile.h"
#include "g_bot_cl.h"
#include "obj_game.h"
#include "net_glob.h"
#include "od_class.h"
#include "g_emp.h"

static int botwait = 0;

// do the desired object control ----------------------------------------------
//
int	OCT_DoControl( object_control_s* objctl )
{
	ASSERT( objctl != NULL );
	ASSERT( ( objctl->rot_x >= -1.0f ) && ( objctl->rot_x <= 1.0f ) );
	ASSERT( ( objctl->rot_y >= -1.0f ) && ( objctl->rot_y <= 1.0f ) );
	ASSERT( ( objctl->rot_z >= -1.0f ) && ( objctl->rot_z <= 1.0f ) );
	ASSERT( ( objctl->accel >= -1.0f ) && ( objctl->accel <= 1.0f ) );

	// check whether we have a valid object to control
	if ( objctl->pShip == NULL ) {
		CON_AddMessage( "OCT: not object to control is set." );
		return FALSE;
	}

	//FIXME: for now we only support ship objects
	if ( !OBJECT_TYPE_SHIP( objctl->pShip ) ) {
		CON_AddMessage( "OCT: controlled object is not a ship." );
		return FALSE;
	}
	
	int ship_controlled = FALSE;
	
	// check for rotation around x axis - PITCH - ( -1 = pullup, +1 = divedown )
	if ( objctl->rot_x != 0.0f ) {
		bams_t _angle = (bams_t)(objctl->rot_x * objctl->pShip->PitchPerRefFrame * CurScreenRefFrames);
		//ObjRotX( objctl->pShip->ObjPosition, _angle );
		INP_UserRotX( _angle );
		ship_controlled = TRUE;
	}
	
	// check for rotation around y axis - YAW - ( -1 = right, +1 = left )
	if ( objctl->rot_y != 0.0f ) {
		bams_t _angle = (bams_t)(objctl->rot_y * objctl->pShip->YawPerRefFrame * CurScreenRefFrames);
		//ObjRotY( objctl->pShip->ObjPosition, _angle );
		INP_UserRotY( _angle );
		ship_controlled = TRUE;
	}
	
	// check for rotation around z axis - ROLL - ( -1 = right, +1 = left )
	if ( objctl->rot_z != 0.0f ) {
		bams_t _angle = (bams_t)(objctl->rot_z * objctl->pShip->RollPerRefFrame * CurScreenRefFrames);
		//ObjRotZ( objctl->pShip->ObjPosition, _angle );
		INP_UserRotZ( _angle );
		ship_controlled = TRUE;
	}
	
	// check for accel control ( +1 accelerate, -1 decelerate )
	if ( objctl->accel != 0.0f ) {
		//fixed_t _accel = FLOAT_TO_FIXED( objctl->accel * (float)(objctl->pShip->SpeedIncPerRefFrame) * CurScreenRefFrames );
		fixed_t _accel = (fixed_t)(objctl->accel * objctl->pShip->SpeedIncPerRefFrame * CurScreenRefFrames);
		INP_UserAcceleration( _accel );
		
/*
		// alter accel
		objctl->pShip->CurSpeed += c_speed;
		
		// check against speed constraints
		if ( objctl->pShip->CurSpeed > objctl->pShip->MaxSpeed ) {
			objctl->pShip->CurSpeed = objctl->pShip->MaxSpeed;
		} else if ( objctl->pShip->CurSpeed < 0 ) {
			objctl->pShip->CurSpeed = 0;
		}
*/
		
		ship_controlled = TRUE;
	}
	
	//// we always move the controlled ship according to the speed
	//if ( objctl->pShip->CurSpeed != 0 ) {
	//	
	//	// actually move the ship
	//	Vector3 dirvec;
	//	fixed_t speed = objctl->pShip->CurSpeed * CurScreenRefFrames;
	//	DirVctMUL( objctl->pShip->ObjPosition, FIXED_TO_GEOMV( speed ), &dirvec );
	//	objctl->pShip->ObjPosition[ 0 ][ 3 ] += dirvec.X;
	//	objctl->pShip->ObjPosition[ 1 ][ 3 ] += dirvec.Y;
	//	objctl->pShip->ObjPosition[ 2 ][ 3 ] += dirvec.Z;
	//}

	return ship_controlled;
}

// dump an object control to the console --------------------------------------
//
void OCT_Dump(object_control_s* objctl )
{
	ASSERT( objctl != NULL );
	ASSERT( objctl->pShip != NULL );

	char szBuffer[ 512 ];
	sprintf( szBuffer, "OCT: ObjID: %03u rotx: %02u roty: %02u rotz: %02u accel: %02u\n", 
		(unsigned int)objctl->pShip->ObjectNumber, 
		(unsigned int)objctl->rot_x, 
		(unsigned int)objctl->rot_y, 
		(unsigned int)objctl->rot_z, 
		(unsigned int)objctl->accel );

	CON_AddMessage( szBuffer );
}

// ****************************************************************************
// ****************************************************************************
// ****************************************************************************

// bot logfile ----------------------------------------------------------------
//
#ifdef BOT_LOGFILES

static UTL_LogFile g_BotLog( "bot.log" );
static UTL_LogFile g_BotPosLog( "bot_pos.log" );
static UTL_LogFile g_BotXDirLog( "bot_xdir.log" );
static UTL_LogFile g_BotZDirLog( "bot_zdir.log" );
static UTL_LogFile g_BotAccelModeLog( "bot_accel.log" );

#define BOT_MsgOut				g_BotLog.printf
#define BOT_Pos_MsgOut			g_BotPosLog.printf
#define BOT_XDir_MsgOut			g_BotXDirLog.printf
#define BOT_ZDir_MsgOut			g_BotZDirLog.printf
#define BOT_AccelMode_MsgOut	g_BotAccelModeLog.printf

#endif // !BOT_LOGFILES

// ----------------------------------------------------------------------------
//
void UTL_LocomotionController::ControlOjbect( object_control_s* pObjctl, Vector3* pDesiredVelocity, fixed_t _DesiredSpeed )
{
	ASSERT( pObjctl != NULL );
	ASSERT( pDesiredVelocity != NULL );

	Vector3 xDir, yDir, zDir;
	FetchXVector( pObjctl->pShip->ObjPosition, &xDir );
	FetchYVector( pObjctl->pShip->ObjPosition, &yDir );
	FetchZVector( pObjctl->pShip->ObjPosition, &zDir );

	geomv_t len = VctLenX( pDesiredVelocity );

	// stop control if desired velocity is zero 
	if ( len <= GEOMV_VANISHING ) {
		pObjctl->rot_x = 0;
		pObjctl->rot_y = 0;
		pObjctl->accel = -0.82;

#ifdef BOT_LOGFILES
		BOT_MsgOut( "ControlOjbect() got dimishing desired velocity" );
#endif // BOT_LOGFILES

		return;
	} else {
		Vector3 DesVelNorm;
		DesVelNorm.X = FLOAT_TO_GEOMV( pDesiredVelocity->X / len );
		DesVelNorm.Y = FLOAT_TO_GEOMV( pDesiredVelocity->Y / len );
		DesVelNorm.Z = FLOAT_TO_GEOMV( pDesiredVelocity->Z / len );
		
		geomv_t yaw_dot		= DOT_PRODUCT( &DesVelNorm, &xDir );
		geomv_t pitch_dot	= DOT_PRODUCT( &DesVelNorm, &yDir );
		geomv_t heading_dot = DOT_PRODUCT( &DesVelNorm, &zDir );

		float fDesiredSpeed = FIXED_TO_FLOAT( _DesiredSpeed );
		float fCurSpeed	  = FIXED_TO_FLOAT( pObjctl->pShip->CurSpeed );

		float pitch = 0;
		float yaw   = 0;

		// target is behind us
		sincosval_s fullturn;
		GetSinCos( DEG_TO_BAMS( 2 * m_nRelaxedHeadingAngle ), &fullturn );
		//if ( heading_dot < 0.0f ) {
		if ( heading_dot < -fullturn.cosval ) {

			// we must initiate a turn, if not already in a turn
			if ( !pObjctl->IsYaw() || !pObjctl->IsPitch() ) {

				// default to random
				yaw   = (float)( RAND() % 3 ) - 1;
				pitch = (float)( RAND() % 3 ) - 1;

				// steer towards goal
				if ( yaw_dot < -GEOMV_VANISHING ) {
					yaw = OCT_YAW_LEFT;
				} else if ( yaw_dot > GEOMV_VANISHING ) {
					yaw = OCT_YAW_RIGHT;
				}
				if ( pitch_dot < -GEOMV_VANISHING ) {
					pitch = OCT_PITCH_UP;
				} else if ( pitch_dot > GEOMV_VANISHING ) {
					pitch = OCT_PITCH_DOWN;
				}

			} else {
				// reuse prev. turn information
				pitch = pObjctl->rot_x;
				yaw   = pObjctl->rot_y;
			}

			// slow down, until in direction of target
		       // pObjctl->accel = heading_dot;
			//pObjctl->accel = OCT_DECELERATE;

			// also maintain min. speed during turns
			if ( fCurSpeed > m_fMinSpeedTurn ) {
				pObjctl->accel = -0.42;
			}

		} else {

			// determine accel
			if ( fDesiredSpeed > fCurSpeed ) {
				// accelerate towards target
				pObjctl->accel = OCT_ACCELERATE;
			} else if ( fDesiredSpeed < fCurSpeed ) {
				// decelerate towards target
				pObjctl->accel = OCT_DECELERATE;
			} else {
				// no accel
				pObjctl->accel = 0;
			}

			// heading must be inside of 5 degrees cone angle
			sincosval_s sincosv;
			GetSinCos( DEG_TO_BAMS( m_nRelaxedHeadingAngle ), &sincosv );
			if ( yaw_dot < -sincosv.sinval ) {
				yaw = OCT_YAW_LEFT;
			} else if ( yaw_dot > sincosv.sinval ) {
				yaw = OCT_YAW_RIGHT;
			}
			if ( pitch_dot < -sincosv.sinval ) {
				pitch = OCT_PITCH_UP;
			} else if ( pitch_dot > sincosv.sinval ) {
				pitch = OCT_PITCH_DOWN;
			}

			// if heading outside of 30 degrees cone angle, we decelerate
			GetSinCos( DEG_TO_BAMS( m_nFullSpeedHeading ), &sincosv );
			bool_t bYawOutsideHeading	= ( ( yaw_dot   < -sincosv.sinval ) || ( yaw_dot   > sincosv.sinval ) );
			bool_t bPitchOutsideHeading = ( ( pitch_dot < -sincosv.sinval ) || ( pitch_dot > sincosv.sinval ) );
			if ( bYawOutsideHeading || bPitchOutsideHeading ) {

				// also maintain min. speed during turns
				if ( fCurSpeed > min( m_fMinSpeedTurn, fDesiredSpeed ) ) {
					// decelerate towards target
					pObjctl->accel = OCT_DECELERATE;
				}
			}
		}

#ifdef BOT_LOGFILES
		BOT_MsgOut( "heading_dot: %f, yaw_dot: %f, pitch_dot: %f", heading_dot, yaw_dot, pitch_dot );
#endif // BOT_LOGFILES

		//FIXME: oct must also handle slide horiz./vert.

		pObjctl->rot_x = pitch;
		pObjctl->rot_y = yaw;
	}
}

//----------------------------------------------------------------------------

static int clbot_char_plan_freq  = 1;
static int clbot_char_goal_freq  = 10;
static int clbot_char_input_freq = 10;
 
PRIVATE
void RealizeBotChar()
{
	TheBot->GetCharacter()->m_fPlanInterval			= 1.0f / (float)clbot_char_plan_freq;
	TheBot->GetCharacter()->m_fGoalCheckInterval	= 1.0f / (float)clbot_char_goal_freq;
	TheBot->GetCharacter()->m_fInputChangeInterval	= 1.0f / (float)clbot_char_input_freq;
}

// ----------------------------------------------------------------------------
PRIVATE int FetchBotChar_PlanFreq()  { return (int)( ( 1.0f / TheBot->GetCharacter()->GetPlanInterval_sec() ) + 0.5f ); }
PRIVATE int FetchBotChar_GoalFreq()  { return (int)( ( 1.0f / TheBot->GetCharacter()->GetGoalCheckInterval_sec() ) +  0.5f ); }
PRIVATE int FetchBotChar_InputFreq() { return (int)( ( 1.0f / TheBot->GetCharacter()->GetInputChangeInterval_sec() )  + 0.5f ); }

// registration table for bot character variables -----------------------------
//
int_command_s botchar_int_commands[] = {
	{ 0x00,	"clbot.char.plan_freq",		1, 100,	&clbot_char_plan_freq,	RealizeBotChar,	FetchBotChar_PlanFreq,  0 },
	{ 0x00,	"clbot.char.goal_freq",		1, 100,	&clbot_char_goal_freq,	RealizeBotChar, FetchBotChar_GoalFreq,  0 },
	{ 0x00,	"clbot.char.input_freq",	1, 100,	&clbot_char_input_freq,	RealizeBotChar, FetchBotChar_InputFreq, 0 },
	
	{ 0x00, NULL, 0,0,NULL, NULL, NULL, 0 },
};

#define BOT_REPAIR_LEVEL 0.9F
#define BOT_ENERGY_LEVEL 0.1F
#define BOT_GMISSL_LEVEL 0.1F

// ctor -----------------------------------------------------------------------
//
BOT_Character::BOT_Character()
{
	Reset();

	// register commands
	for( int_command_s* cmd = botchar_int_commands; cmd->command != NULL; cmd++  ) {
		CON_RegisterIntCommand( cmd );
	}
}

// reset to defaults ----------------------------------------------------------
//
void BOT_Character::Reset()
{
	m_fPlanInterval			= 100000.f;		
	m_fGoalCheckInterval	= 0.1f;		// goal checking at 10Hz 
	m_fInputChangeInterval	= 0.1f;		// input at 10Hz
	m_emp_delay				= 0.0f;
	m_fire_delay			= 0.0f;
	m_missile_delay			= 0.0f;
}

// select the attack target ---------------------------------------------------
//
ShipObject* BOT_Character::SelectAttackTarget( ShipObject* pAttacker )
{
	ShipObject* pCurTarget = NULL;
	int nCurTargetDamageRemain = 0;

	for ( ShipObject* pShip = /*TheWorld->*/FetchFirstShip(); pShip != NULL; ) {

		// do not select self as target
		if ( pAttacker == pCurTarget )
			continue;

		if ( pCurTarget == NULL ) {
			pCurTarget				= pShip;
			nCurTargetDamageRemain	= ( pCurTarget->MaxDamage - pCurTarget->CurDamage );
		} else {
			// select weakest ship as target 
			int nDamageRemain = ( pShip->MaxDamage - pShip->CurDamage );

			if ( nDamageRemain > nCurTargetDamageRemain ) {
				pCurTarget = pShip;
				nCurTargetDamageRemain = nDamageRemain;
			}
		}
		
		pShip = (ShipObject *) pShip->NextObj;
	}
	if(pCurTarget != NULL) {

		// set the target tracking to be the host object we just aquired.
		TargetObjNumber = pCurTarget->HostObjNumber;
	}
	return pCurTarget;
}


// main think function, called every render or sim frame ( client or server )
//
void BOT_AI::DoThink()
{
	ASSERT( NetJoined );
        // count down fire disable delay
	// get the agent position for this frame
	FetchTVector( m_pShip->ObjPosition, &m_AgentPos );


	// do planning ?
	if ( m_PlanTimeout.IsTimeout() ) {
		_DoPlan();
	}

	// do goal checking ?
	if ( m_GoalCheckTimeout.IsTimeout() ) {
		// check goals for completion and define new goals
		switch ( m_nAgentMode ) {
			case AGENTMODE_RETREAT:
				_GoalCheck_AgentMode_Retreat();
				break;
			case AGENTMODE_ATTACK:
				_GoalCheck_AgentMode_Attack();
				break;
			case AGENTMODE_IDLE:
				_GoalCheck_AgentMode_Idle();
				break;
			case AGENTMODE_POWERUP:
				_GoalCheck_AgentMode_Powerup();
				break;
			default:
				ASSERT( FALSE );
				return;
		}
	}

	// change steering ?
	if ( m_InputTimeout.IsTimeout() ) {

		// modify ObjectControl to steer to position in current goal
		_SteerToPosition( m_State.GetCurGoal()->GetGoalPosition(),  m_State.GetObjectControl() );
	}

	// maintain counters
	m_Character.SetEMPDelay(m_Character.GetEMPDelay() - .03);
	m_Character.SetFireDelay(m_Character.GetFireDelay() - .03);
	m_Character.SetMissileDelay(m_Character.GetMissleDelay() - .03);

	// actually control the object
	OCT_DoControl( m_State.GetObjectControl() );
}


// determine in which agentmode we should be ----------------------------------
// 
void BOT_AI::_DoPlan()
{
	MSGOUT("BOT_AI::_DoPlan() Execute New Plan, MODE: %i\n", m_nAgentMode);

    ShipObject*      pTargetObject = NULL;
    BOT_Goal* pGoal	= m_State.GetCurGoal();
	if(m_pShip->CurDamage > (m_pShip->MaxDamage * BOT_REPAIR_LEVEL)) { //90% damage
		m_nAgentMode = AGENTMODE_RETREAT;
		MSGOUT("Want Repair in DoPlan");
	    return;
	}
	
	if(m_pShip->NumHomMissls < 1) {
		m_nAgentMode = AGENTMODE_RETREAT;
		MSGOUT("Want Missles in DoPlan");
		return;
	}

	if(m_pShip->CurEnergy < (m_pShip->MaxEnergy * BOT_ENERGY_LEVEL)) { //10% energy left
		m_nAgentMode = AGENTMODE_RETREAT;
		MSGOUT("Want Energy in DoPlan");
		return;
	}
	if(NumRemPlayers > 1){
		pTargetObject = m_Character.SelectAttackTarget( m_pShip ); //Check for target
		if(pTargetObject != NULL) {
			m_nAgentMode = AGENTMODE_ATTACK;
			return;
		}	
	} else {

		if ( pTargetObject == NULL ) {
			ExtraObject* pObject = FetchFirstExtra(); //Check for powerups
			if(pObject != NULL) {
				m_nAgentMode = AGENTMODE_POWERUP;
           
			} else {
				m_nAgentMode = AGENTMODE_IDLE;
			}
		} 
	}
}

// ----------------------------------------------------------------------------
//
void BOT_AI::_GoalCheck_AgentMode_Idle()
{



#ifdef BOT_LOGFILES
		BOT_MsgOut( "new goal position: %f %f %f", pGoalPos->X, pGoalPos->Y, pGoalPos->Z );
#endif // BOT_LOGFILES
		// Don't want to sit idle, so...
		// get a new plan
	   m_PlanTimeout.Reset();
	   _DoPlan();
//	}
}


// ----------------------------------------------------------------------------
//
void BOT_AI::_GoalCheck_AgentMode_Powerup()
{
    BOT_Goal* pGoal	= m_State.GetCurGoal();
	Vector3* pGoalPos = pGoal->GetGoalPosition();
	
	ASSERT( pGoal != NULL );
    if(pGoal->GetTargetObject() == NULL) {
        ExtraObject* pObject = FetchFirstExtra();
        if(pObject == NULL) {
            m_nAgentMode = AGENTMODE_IDLE;
            return;
         }
         pGoal->SetTargetObject(pObject);
         FetchTVector( pObject->ObjPosition, pGoalPos );
#ifdef BOT_LOGFILES
         BOT_MsgOut("Targetting Extra");
#endif
	}
	
	pGoalPos = pGoal->GetGoalPosition();

	// get vector to target
	Vector3 vec2Target;	
	VECSUB( &vec2Target, pGoalPos, &m_AgentPos );

	float len = VctLenX( &vec2Target );
#ifdef BOT_LOGFILES
	BOT_MsgOut( "BOT: distance to goal: %f", len );
#endif    
	if ( len < 100.0f )  {
#ifdef BOT_LOGFILES
	   BOT_MsgOut("Clearing Goal");
	   
#endif	   
	   pGoal->SetTargetObject(NULL);
	   
	   // get a new plan
	   m_PlanTimeout.Reset();
	   _DoPlan();
	}
  
}

// check whether the target is in range for weapons ------------------------

int BOT_AI::_TargetInRange( ShipObject *ship, ShipObject *target, geomv_t range )
{

	//XXX: Taken from other collision detection need to check that it works right.

	ASSERT( target != NULL );
	ASSERT( ship != NULL );

	

	Vector3 tgtnormal;
	FetchZVector( target->ObjPosition, &tgtnormal );

	Vertex3 tgtpos;
	FetchTVector( target->ObjPosition, &tgtpos );

	Vertex3 shippos;
	FetchTVector( ship->ObjPosition, &shippos );

	geomv_t shipdot  = -DOT_PRODUCT( &tgtnormal, &shippos );
	geomv_t tgtdot  = -DOT_PRODUCT( &tgtnormal, &tgtpos );
	geomv_t distance = shipdot - tgtdot;

	// not in range if ship in wrong halfspace ( target behind us )
	if ( ! GEOMV_NEGATIVE( distance ) ) {
		return FALSE;
	}



	// inside the activation distance/range ?
	if ( distance < range ) {

		return TRUE;
	}

	return FALSE;
}




// set the currently used goal position ---------------------------------------
//
void BOT_AI::_GoalCheck_AgentMode_Attack()
{
	//FIXME: push current goal onto goal stack
	BOT_Goal* pGoal	= m_State.GetCurGoal();
	ASSERT( pGoal != NULL );

	//FIXME: here we should also evaluate whether we pick another target


	/*
	// check our stats and get a new plan if we are low on energy, damage, or missles.
	if(m_pShip->NumHomMissls < 1) {
		m_nAgentMode = AGENTMODE_RETREAT;
		MSGOUT("Want Missles in Attack");
		return;
	}
	if(m_pShip->CurDamage > (m_pShip->MaxDamage * BOT_REPAIR_LEVEL)) { //90% damage
	     m_nAgentMode = AGENTMODE_RETREAT;
		 MSGOUT("Want repair in Attack");
	     return;
	}
	if(m_pShip->CurEnergy < (m_pShip->MaxEnergy * BOT_ENERGY_LEVEL)) { //10% energy left
	    m_nAgentMode = AGENTMODE_RETREAT;
		MSGOUT("Want Energy in Attack");
		return;
	}

	*/
	// select a target, if none, or no ship selected as target 
	GenObject*	pTargetObject = pGoal->GetTargetObject();
	Vector3*	pGoalPos		 = pGoal->GetGoalPosition();
	if( ( pTargetObject == NULL ) || ( OBJECT_TYPE_SHIP( pTargetObject) == FALSE ) ){

		// let the character select a target
		pTargetObject = m_Character.SelectAttackTarget( m_pShip );

		// no target
		if ( pTargetObject == NULL ) {
			//FIXME: transition function
			m_nAgentMode = AGENTMODE_IDLE;
			pGoal->SetTargetObject(NULL);

#ifdef BOT_LOGFILES
			BOT_MsgOut( "switching from ATTACK to IDLE" );
#endif // BOT_LOGFILES

			return;
		}

		// set the target object
		pGoal->SetTargetObject( pTargetObject );

		// set the goal position to where the target is
		FetchTVector( pTargetObject->ObjPosition, pGoalPos );

#ifdef BOT_LOGFILES
		BOT_MsgOut( "found new ATTACK target %d", pTargetObject->HostObjNumber );
#endif // BOT_LOGFILES

	}

	Vector3	TargetPos;
	FetchTVector( pTargetObject->ObjPosition, &TargetPos );

	// get vector to target
	Vector3 vec2Target;	
	VECSUB( &vec2Target, &TargetPos, &m_AgentPos );

	float len = VctLenX( &vec2Target );
#ifdef BOT_LOGFILES
	BOT_MsgOut( "BOT: distance to target: %f", len );
#endif
#define MIN_DISTANCE_TO_TARGET 100.0f

	// TODO: Check to see if we are facing the target.
	if(_TargetInRange(MyShip, (ShipObject *)pTargetObject, 1000.0F)) {
		if (len < 600.0F) {


			if (m_Character.GetFireDelay() < 0.0f) {
				// create laser
				OBJ_ShootLaser(m_pShip);
				m_Character.SetFireDelay(1.0f);
			}
		
		}

		// give bots the ability to fire homing missiles?  Evil.
		if (len > 500.0F ) {
			SelectedMissile = 1;
			// check to see if we locked on the target
			if(TargetLocked ) {
				// we are locked on, attempt to fire a homing missile.
				OBJ_LaunchHomingMissile(m_pShip, CurLauncher, TargetObjNumber);
				m_Character.SetMissileDelay(2.0f); //Missile delay is longer than other weaps
			}
		}
		if(len < 100.0F ) {
			//: Fire EMP 
			if ( m_Character.GetEMPDelay() < 0.0f) {
				WFX_EmpBlast(m_pShip);
				m_Character.SetEMPDelay(1.0f);
			} 
		}
	}
	
	if ( len < MIN_DISTANCE_TO_TARGET )  {
                		
		// if nearby goal, we stay where we are
		memcpy( pGoalPos, &m_AgentPos, sizeof( Vector3 ) );
               
        } else {

		// set the goal position to the position of the target
		ASSERT( pTargetObject != NULL );
		FetchTVector( pTargetObject->ObjPosition, pGoalPos );
	}

#ifdef BOT_LOGFILES
	BOT_MsgOut( "ATTACK goal is at %f %f %f", pGoalPos->X, pGoalPos->Y, pGoalPos->Z );
#endif // BOT_LOGFILES
}

// search for the repair module ---------------------------------------------------
//
ExtraObject* BOT_Character::SelectRepairObject()
{
	ExtraObject* pCurTarget = NULL;
	
	for ( ExtraObject* pSearch = FetchFirstExtra(); pSearch != NULL; ) {

		if ( pCurTarget == NULL ) { //Save what we find first, if it matches great if not, search on
			if(pSearch->ObjectType == EXTRA1TYPE) {
			   Extra1Obj *extra1po = (Extra1Obj *) pSearch;
			   if(extra1po != NULL) {
					if( extra1po->ObjectClass == REPAIR_EXTRA_CLASS){
						pCurTarget = pSearch;
						return pCurTarget; //found it first try!
					}
			   }
			}
		}
		pSearch = (ExtraObject *) pSearch->NextObj;
	}
	return pCurTarget; //Return Original or NULL search item as we couldnt find the rep module
}

// search for the energy module ---------------------------------------------------
//
ExtraObject* BOT_Character::SelectEnergyObject()
{
	ExtraObject* pCurTarget = NULL;
	
	for ( ExtraObject* pSearch = FetchFirstExtra(); pSearch != NULL; ) {

		if ( pCurTarget == NULL ) { //Save what we find first, if it matches great if not, search on
			
			if(pSearch->ObjectType == EXTRA1TYPE) {
			   Extra1Obj *extra1po = (Extra1Obj *) pSearch;
			   if(extra1po != NULL) {
					if( extra1po->ObjectClass == ENERGY_EXTRA_CLASS){
						pCurTarget = pSearch;
						return pCurTarget; //found it first try!
					}
			   }
			}
		} 
		pSearch = (ExtraObject *) pSearch->NextObj;
	}
	return pCurTarget; //Return Original or NULL search item as we couldnt find the energy module
}


// search for the energy module ---------------------------------------------------
//
ExtraObject* BOT_Character::SelectHomingMissileObject()
{
	ExtraObject* pCurTarget = NULL;
	
	for ( ExtraObject* pSearch = FetchFirstExtra(); pSearch != NULL; ) {

		if ( pCurTarget == NULL ) { //Save what we find first, if it matches great if not, search on
			
			if(pSearch->ObjectType == EXTRA2TYPE) {
			   Extra2Obj *extra2po = (Extra2Obj *) pSearch;
			   if(extra2po != NULL) {
					if( extra2po->ObjectClass == GUIDE_PACK_CLASS){
						pCurTarget = pSearch;
						return pCurTarget; //found it first try!
					}
			   }
			}
		} 
		pSearch = (ExtraObject *) pSearch->NextObj;
	}
	return pCurTarget; //Return Original or NULL search item as we couldnt find the energy module
}

// ----------------------------------------------------------------------------
//
void BOT_AI::_GoalCheck_AgentMode_Retreat()
{
    BOT_Goal* pGoal	= m_State.GetCurGoal();
	Vector3* pGoalPos = pGoal->GetGoalPosition();
	ExtraObject* pObject = NULL;
	ASSERT( pGoal != NULL );
    
	if(pGoal->GetTargetObject() == NULL) {
        
		// third is to get some guided missles.
		if(m_pShip->NumHomMissls < (m_pShip->MaxNumHomMissls * BOT_GMISSL_LEVEL)) {
			pObject = m_Character.SelectHomingMissileObject();
			m_nAgentMode = AGENTMODE_RETREAT;
			MSGOUT("BOT: Want Guided Missles.");
		}


		// second highest priority is to re-energize
		if(m_pShip->CurEnergy < (m_pShip->MaxEnergy * BOT_ENERGY_LEVEL)) {
			pObject = m_Character.SelectEnergyObject();
			m_nAgentMode = AGENTMODE_RETREAT;
			MSGOUT("BOT: Want Energy.");
	    }

		// highest priority is to repair ourself
		if(m_pShip->CurDamage > (m_pShip->MaxDamage * BOT_REPAIR_LEVEL)){
	       pObject = m_Character.SelectRepairObject();
		   m_nAgentMode = AGENTMODE_RETREAT;
		   MSGOUT("BOT: Want Repair.");
	    }
		if(pObject == NULL) {
            m_nAgentMode = AGENTMODE_RETREAT;
			MSGOUT("BOT: Didn't get a powerup.");
			//TODO: Perhaps force it to do something else for a while.
			m_PlanTimeout.Reset();
			_DoPlan();
            return;
         }
		
		Vector3 vec2Target_chk;
		Vector3 GoalPos_chk;
		FetchTVector( pObject->ObjPosition, &GoalPos_chk );
		// if the target is greater than 15000 away, get something else.
		/*if((VctLenX(&GoalPos_chk) - VctLenX(&m_AgentPos))> 15000.0F){
			pObject=NULL;
			//m_nAgentMode = AGENTMODE_IDLE;
            return;
		}*/

		pGoal->SetTargetObject(pObject);
        FetchTVector( pObject->ObjPosition, pGoalPos );
#ifdef BOT_LOGFILES
		 BOT_MsgOut("Retreat: Targetting Extra");
#endif
	}
	
	pGoalPos = pGoal->GetGoalPosition();

	// get vector to target
	Vector3 vec2Target;	
	VECSUB( &vec2Target, pGoalPos, &m_AgentPos );

	float len = VctLenX( &vec2Target );
#ifdef BOT_LOGFILES
	BOT_MsgOut( "BOT: distance to goal: %f", len );
#endif        
	if ( len < 100.0f )  {
#ifdef BOT_LOGFILES
	   BOT_MsgOut("Clearing Goal");
#endif
	   pGoal->SetTargetObject(NULL);
	   m_PlanTimeout.Reset();
	   _DoPlan();
	}

}


// ----------------------------------------------------------------------------
//
void BOT_AI::_SteerToPosition( Vector3* targetPos, object_control_s* pObjctl )
{
	// get vector to target
	Vector3 vec2Target;	
	VECSUB( &vec2Target, targetPos, &m_AgentPos );

	float oldaccel = pObjctl->accel;

	UTL_LocomotionController _LomoCtrl;
	_LomoCtrl.ControlOjbect( pObjctl, &vec2Target, pObjctl->pShip->MaxSpeed );

	if ( pObjctl->accel != oldaccel ) {
#ifdef BOT_LOGFILES
		BOT_AccelMode_MsgOut( "%f %f %f", m_AgentPos.X, m_AgentPos.Y, m_AgentPos.Z );
#endif // BOT_LOGFILES
	}

	Vector3 xDir, yDir, zDir;
	FetchXVector( m_pShip->ObjPosition, &xDir );
	FetchYVector( m_pShip->ObjPosition, &yDir );
	FetchZVector( m_pShip->ObjPosition, &zDir );

#ifdef BOT_LOGFILES
	BOT_MsgOut( "pos: %7.2f/%7.2f/%7.2f vec2target: %7.2f/%7.2f/%7.2f xDir: %7.2f/%7.2f/%7.2f yDir: %7.2f/%7.2f/%7.2f rot_x: %7.2f rot_y: %7.2f accel: %7.2f",
		m_AgentPos.X, m_AgentPos.Y, m_AgentPos.Z,
				vec2Target.X, vec2Target.Y, vec2Target.Z,
				xDir.X, xDir.Y, xDir.Z,
				yDir.X, yDir.Y, yDir.Z,
				pObjctl->rot_x, pObjctl->rot_y, pObjctl->accel );

	BOT_Pos_MsgOut ( "%f %f %f", m_AgentPos.X, m_AgentPos.Y, m_AgentPos.Z );
	BOT_XDir_MsgOut( "%f %f %f", xDir.X, xDir.Y, xDir.Z );
	BOT_ZDir_MsgOut( "%f %f %f", zDir.X, zDir.Y, zDir.Z );
#endif // BOT_LOGFILES

}



// check whether current status (connected/joined etc. ) matches the desired one
//
void BOT_ClientSide::_CheckStatus()
{
    BOT_Goal* pGoal	= m_State.GetCurGoal();
	// check whether to connect
	if ( ( NetConnected == NETWORK_GAME_OFF ) && m_bWantToBeConnected )	{
               pGoal->SetTargetObject(NULL);
		if ( strlen( m_szServer ) > 0 )  {
			// connect
			NET_CommandConnect( m_szServer );

			// set the timeout for the next connection attempt 
			if ( !NetConnected ) {
				m_NextStatusCheckRefFrame = SYSs_GetRefFrameCount() + ( 5 * BOT_STATUS_CHECK_TIMEOUT );
			}

			return;
		} else {

#ifdef BOT_LOGFILES
			BOT_MsgOut( "Error: BOT_ClientSide::_CheckStatus(): missing servername for connect !" );
#endif // BOT_LOGFILES

		}

	// check whether to disconnect
	} else if ( ( NetConnected == NETWORK_GAME_ON ) && !m_bWantToBeConnected ) {

		// disconnect
		NET_CommandDisconnect();
		return;
	}

	if ( NetConnected ) {
		// check for join
		if ( !NetJoined && m_bWantToBeJoined ) {
			if(botwait == 1) {
			   botwait = 0;
			} else {
			   botwait = 1;
			}
		    // join
			pGoal->SetTargetObject(NULL);
			if(botwait == 0) {
			   MENU_FloatingMenuEnterGame();
			} 
			
			if(!NetJoined) {
			   	m_NextStatusCheckRefFrame = SYSs_GetRefFrameCount() + ( 5 * BOT_STATUS_CHECK_TIMEOUT );
			}
			return;

		// check for unjoin
		} else if ( NetJoined && !m_bWantToBeJoined ) {

			// unjoin
			pGoal->SetTargetObject(NULL);
			ExitGameLoop = 1;
		}
	}
}


// overwritten think method to allow special client-side things ---------------
//
void BOT_ClientSide::DoThink()
{
	// do status checking ?
	if ( SYSs_GetRefFrameCount() >= m_NextStatusCheckRefFrame ) {
		m_NextStatusCheckRefFrame = SYSs_GetRefFrameCount() + BOT_STATUS_CHECK_TIMEOUT;
		_CheckStatus();
	}

	// AI only active if connected & joined & we have the full state received from the server
	if ( NetConnected && NetJoined & HaveFullPlayerState ) {
		
		BOT_AI::DoThink();
	}
}

// start the bot --------------------------------------------------------------
//
void BOT_ClientSide::Start()
{
	if ( !m_Started ) {
		CALLBACK_RegisterCallback( CBTYPE_USER_INPUT | CBFLAG_PERSISTENT, BOT_CallThink, (void *) this );
		m_Started = TRUE;

#ifdef BOT_LOGFILES
		BOT_MsgOut( "STARTED" );
#endif // BOT_LOGFILES
	}
}

// stop the bot ---------------------------------------------------------------
//
void BOT_ClientSide::Stop()
{
	if ( m_Started ) {
		int numremoved = CALLBACK_DestroyCallback( CBTYPE_USER_INPUT | CBFLAG_PERSISTENT, (void *) this );
		ASSERT( numremoved <= 1 );
		m_Started = FALSE;

#ifdef BOT_LOGFILES
		BOT_MsgOut( "STOPPED" );
#endif // BOT_LOGFILES
	}
}


// callback function to call the bot DoThink() function -----------------------
//
PRIVATE
int BOT_CallThink( void* param )
{
	ASSERT( param != NULL );
	BOT_ClientSide* pBot = (BOT_ClientSide*) param;
	
	pBot->DoThink();

	return TRUE;
}


// console command for starting the client-side bot ---------------------------
//
PRIVATE
int Cmd_CLBOT_START( char* dummystr )
{
	ASSERT( dummystr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( dummystr );

	TheBot->SetShipObject( MyShip );
	TheBot->Start();

	return TRUE;
}

// console command for stopping the client-side bot ---------------------------
//
PRIVATE
int Cmd_CLBOT_STOP( char* dummystr )
{
	ASSERT( dummystr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( dummystr );

	TheBot->Stop();
	TheBot->SetShipObject( NULL );

	return TRUE;
}


// console command for setting/getting the server we want to connect to -------
//
PRIVATE
int Cmd_CLBOT_SERVER( char* pszServername )
{
	// trim left
	char *p = NULL;
	for( p = pszServername; *p == ' '; p++ );

	if ( *p == '\0' ) {
		const char* pszSetServer = TheBot->GetConnectServer();
		if ( pszSetServer == NULL || pszSetServer[0] == '\0' ) {
			CON_AddLine( "no server set." );
		} else {
			CON_AddLine( pszSetServer );
		}
	} else {
		TheBot->SetConnectServer( pszServername );
	}

	return TRUE;
}

// console command to set/get the desired game status (connected/joined etc ) -
//
PRIVATE
int Cmd_CLBOT_GAMESTATUS( char* pszStatus )
{
	ASSERT( pszStatus != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( pszStatus );

	int status = ( TheBot->GetDesiredConnStatus() << 1 ) | TheBot->GetDesiredJoinStatus();
	char *scan;

	if ( (scan = QueryIntArgumentEx( pszStatus, "%d", &status )) ) {
		char *errpart;
		long sval = strtol( scan, &errpart, 10 );
		if ( *errpart == 0 ) {
			
			TheBot->SetDesiredConnStatus( ( sval & 0x2 ) >> 1 );
			TheBot->SetDesiredJoinStatus( sval & 0x1 );
		}
	}

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( G_BOT_CL )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "clbot.start" command
	regcom.command	 = "clbot.start";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_CLBOT_START;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "clbot.start" command
	regcom.command	 = "clbot.stop";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_CLBOT_STOP;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "clbot.setsever" command
	regcom.command	 = "clbot.server";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_CLBOT_SERVER;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "clbot.gamestatus" command
	regcom.command	 = "clbot.gamestatus";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_CLBOT_GAMESTATUS;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );
}


