/*
 * PARSEC - Object Explosion/Damage
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:24 $
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
#include "aud_defs.h"
#include "net_defs.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "obj_expl.h"

// proprietary module headers
#include "con_aux.h"
#include "h_supp.h"
#include "obj_ctrl.h"
#include "obj_xtra.h"
#include "part_api.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char	paste_str[ PASTE_STR_LEN + 1 ];


// decrease per refframe for explosion animation counter ----------------------
//
#define EXPLOSION_ANIM_SPEED			1


// kill message strings -------------------------------------------------------
//
static char lightning_downed_ship_str[]	= "ship struck down by lightning";
static char helix_downed_ship_str[]		= "ship struck down by helix";
static char swarm_downed_ship_str[]		= "ship struck down by swarm missiles";
static char photon_downed_ship_str[]    = "ship struck down by photon";
static char emp_downed_ship_str[]		= "ship struck down by emp";
static char killed_by_str[] 			= "killed by ";


// start explosion animation for ship -----------------------------------------
//
void LetShipExplode( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	//NOTE:
	// this function is only used in off-line
	// mode to start the explosion of a ship
	// that is not the player's ship.

	//NOTE:
	// this function is used by this module
	// and by OBJ_COLL.C exclusively.

	ASSERT( !NetConnected );
	ASSERT( shippo != MyShip );
	ASSERT( shippo->ExplosionCount == 0 );

	// this actually starts the explosion
	shippo->ExplosionCount = MAX_EXPLOSION_COUNT;

	//FIXME:
	//NOTE:
	// attached particles must not be freed here, since this
	// function may be called via PART_ANI::PAN_AnimateParticles(),
	// which doesn't work correctly if clusters get freed here.

	// free attached particles
//	PRT_FreeAttachedClusterList( shippo );

/*
	if ( !AUX_USE_SIMPLE_EXPLOSION )
		SFX_ParticleExplosion( shippo );
*/

}


// inflict damage upon ship hit by helix weapon -------------------------------
//
void OBJ_ShipHelixDamage( ShipObject *shippo, int owner )
{
	ASSERT( shippo != NULL );

	//NOTE:
	// this function is used by
	// PART_ANI::LinearParticleCollision().

#define HELIX_HITPOINTS_PER_PARTICLE 1

	// no damage at all if megashield active
	if ( shippo->MegaShieldAbsorption > 0 )
		return;

	if ( shippo == MyShip ) {

		SetScreenBlue = 1;

		if ( MyShip->CurDamage <= MyShip->MaxDamage ) {
			shippo->CurDamage += HELIX_HITPOINTS_PER_PARTICLE;
			if ( MyShip->CurDamage > MyShip->MaxDamage ) {
				if(NET_ConnectedPEER()) {
                    OBJ_CreateShipExtras( MyShip );
				    NET_SetPlayerKillStat( owner, 1 );
				    strcpy( paste_str, killed_by_str );
				    strcat( paste_str, NET_FetchPlayerName( owner ) );
				    ShowMessage( paste_str );
                }
			}
		}

	} else if ( !NetConnected && ( shippo->ExplosionCount == 0 ) ) {

		shippo->CurDamage += HELIX_HITPOINTS_PER_PARTICLE;
		if ( shippo->CurDamage > shippo->MaxDamage ) {

			// schedule explosion
			LetShipExplode( shippo );

			// needed for explosions caused by particles
			shippo->DelayExplosion = 1;

			ShowMessage( helix_downed_ship_str );
			AUD_ShipDestroyed( shippo );
			AUD_JimCommenting( JIM_COMMENT_EATTHIS );
		}
	}
}


// inflict damage upon ship hit by lightning beam -----------------------------
//
void OBJ_ShipLightningDamage( ShipObject *shippo, int owner )
{
	ASSERT( shippo != NULL );

	//NOTE:
	// this function is used by
	// PART_ANI::CheckLightningParticleShipCollision().

#define LIGHTNING_HITPOINTS_PER_REFFRAME 6000

	// no damage at all if megashield active
	if ( shippo->MegaShieldAbsorption > 0 )
		return;

	if ( shippo == MyShip ) {

		if ( MyShip->CurDamage <= MyShip->MaxDamage ) {

			MyShip->CurDamageFrac += CurScreenRefFrames * LIGHTNING_HITPOINTS_PER_REFFRAME;
			MyShip->CurDamage     += MyShip->CurDamageFrac >> 16;
			MyShip->CurDamageFrac &= 0xffff;

			if ( MyShip->CurDamage > MyShip->MaxDamage ) {
				OBJ_CreateShipExtras( MyShip );
				NET_SetPlayerKillStat( owner, 1 );
				strcpy( paste_str, killed_by_str );
				strcat( paste_str, NET_FetchPlayerName( owner ) );
				ShowMessage( paste_str );
			}
		}

	} else if ( !NetConnected && ( shippo->ExplosionCount == 0 ) ) {

		shippo->CurDamageFrac += CurScreenRefFrames * LIGHTNING_HITPOINTS_PER_REFFRAME;
		shippo->CurDamage += shippo->CurDamageFrac >> 16;
		shippo->CurDamageFrac &= 0xffff;

		if ( shippo->CurDamage > shippo->MaxDamage ) {

			// schedule explosion
			LetShipExplode( shippo );

			// needed for explosions caused by particles
			shippo->DelayExplosion = 1;

			ShowMessage( lightning_downed_ship_str );
			AUD_ShipDestroyed( shippo );
			AUD_JimCommenting( JIM_COMMENT_EATTHIS );
		}
	}
}


// inflict damage upon ship hit by photon weapon ------------------------------
//
void OBJ_ShipPhotonDamage( ShipObject *shippo, int owner )
{
	ASSERT( shippo != NULL );

	//NOTE:
	// this function is used by
	// PART_ANI::LinearParticleCollision().

#define PHOTON_HITPOINTS_PER_PARTICLE 3

	// no damage at all if megashield active
	if ( shippo->MegaShieldAbsorption > 0 )
		return;

	if ( shippo == MyShip ) {

		SetScreenBlue = 1;

		if ( MyShip->CurDamage <= MyShip->MaxDamage ) {
            shippo->CurDamage += PHOTON_HITPOINTS_PER_PARTICLE;
			if ( MyShip->CurDamage > MyShip->MaxDamage ) {
				OBJ_CreateShipExtras( MyShip );
				NET_SetPlayerKillStat( owner, 1 );
				strcpy( paste_str, killed_by_str );
				strcat( paste_str, NET_FetchPlayerName( owner ) );
				ShowMessage( paste_str );
			}
		}

	} else if ( !NetConnected && ( shippo->ExplosionCount == 0 ) ) {

        shippo->CurDamage += PHOTON_HITPOINTS_PER_PARTICLE;
		if ( shippo->CurDamage > shippo->MaxDamage ) {

			// schedule explosion
			LetShipExplode( shippo );

			// needed for explosions caused by particles
			shippo->DelayExplosion = 1;

            ShowMessage( photon_downed_ship_str );
			AUD_ShipDestroyed( shippo );
			AUD_JimCommenting( JIM_COMMENT_EATTHIS );
		}
	}
}


// inflict damage upon ship hit by swarm missiles -----------------------------
//
void OBJ_ShipSwarmDamage( ShipObject *shippo, int owner )
{
	ASSERT( shippo != NULL );

	//NOTE:
	// this function is used by
	// S_SWARM::SWARM_Animate().

#define SWARM_HITPOINTS_PER_PARTICLE 1

	// no damage at all if megashield active
	if ( shippo->MegaShieldAbsorption > 0 )
		return;

	if ( shippo == MyShip ) {

		SetScreenBlue = 1;

		if ( MyShip->CurDamage <= MyShip->MaxDamage ) {
			shippo->CurDamage += SWARM_HITPOINTS_PER_PARTICLE;
			if ( MyShip->CurDamage > MyShip->MaxDamage ) {
				if(NET_ConnectedPEER()) {
                    OBJ_CreateShipExtras( MyShip );
				    NET_SetPlayerKillStat( owner, 1 );
				    strcpy( paste_str, killed_by_str );
				    strcat( paste_str, NET_FetchPlayerName( owner ) );
				    ShowMessage( paste_str );
			    }
            }
		}

	} else if ( !NetConnected && ( shippo->ExplosionCount == 0 ) ) {

		shippo->CurDamage += SWARM_HITPOINTS_PER_PARTICLE;
		if ( shippo->CurDamage > shippo->MaxDamage ) {
            
			// schedule explosion
			LetShipExplode( shippo );

			// needed for explosions caused by particles
			shippo->DelayExplosion = 1;

			ShowMessage( swarm_downed_ship_str );
			AUD_ShipDestroyed( shippo );
			AUD_JimCommenting( JIM_COMMENT_EATTHIS );
		}
	}
}


// inflict damage upon ship hit by emp ----------------------------------------
//
void OBJ_ShipEmpDamage( ShipObject *shippo, int owner, int hitpoints )
{
	ASSERT( shippo != NULL );

	//NOTE:
	// this function is used by
	// S_EMP::CheckEmpCollision().

	// no damage at all if megashield active
	if ( shippo->MegaShieldAbsorption > 0 )
		return;

	if ( shippo == MyShip ) {

		if ( MyShip->CurDamage <= MyShip->MaxDamage ) {

			MyShip->CurDamageFrac += CurScreenRefFrames * hitpoints;
			MyShip->CurDamage     += MyShip->CurDamageFrac >> 16;
			MyShip->CurDamageFrac &= 0xffff;

			if ( MyShip->CurDamage > MyShip->MaxDamage ) {
				OBJ_CreateShipExtras( MyShip );
				NET_SetPlayerKillStat( owner, 1 );
				strcpy( paste_str, killed_by_str );
				strcat( paste_str, NET_FetchPlayerName( owner ) );
				ShowMessage( paste_str );
			}
		}

	} else if ( !NetConnected && ( shippo->ExplosionCount == 0 ) ) {

		shippo->CurDamageFrac += CurScreenRefFrames * hitpoints;
		shippo->CurDamage += shippo->CurDamageFrac >> 16;
		shippo->CurDamageFrac &= 0xffff;

		if ( shippo->CurDamage > shippo->MaxDamage ) {

			// schedule explosion
			LetShipExplode( shippo );

			// needed for explosions caused by particles
			shippo->DelayExplosion = 1;

			ShowMessage( emp_downed_ship_str );
			AUD_ShipDestroyed( shippo );
			AUD_JimCommenting( JIM_COMMENT_EATTHIS );
		}
	}
}


// check if there's anything exploding right now ------------------------------
//
void OBJ_CheckExplosions()
{
	//NOTE:
	// this function gets called once per
	// frame by the game loop (G_MAIN.C).

	ShipObject *nextship;
	for ( ShipObject *shippo = FetchFirstShip(); shippo; shippo = nextship ) {

		nextship = (ShipObject *) shippo->NextObj;

		if ( shippo->ExplosionCount > 0 ) {

			// used for correct intra-frame timing of
			// explosions caused by particles
			if ( shippo->DelayExplosion > 0 ) {
				shippo->DelayExplosion--;
				continue;
			}

			// let ship fly along
			if ( !AUX_DISABLE_DYING_SHIP_MOVEMENT ) {

				Vector3 dirvec;
				fixed_t speed = shippo->CurSpeed * CurScreenRefFrames;
				DirVctMUL( shippo->ObjPosition, FIXED_TO_GEOMV( speed ), &dirvec );
				shippo->ObjPosition[ 0 ][ 3 ] += dirvec.X;
				shippo->ObjPosition[ 1 ][ 3 ] += dirvec.Y;
				shippo->ObjPosition[ 2 ][ 3 ] += dirvec.Z;
			}

			// animate explosion
			shippo->ExplosionCount -= EXPLOSION_ANIM_SPEED * CurScreenRefFrames;

			// check for end of explosion
			if ( shippo->ExplosionCount <= 0 ) {
				KillSpecificObject( shippo->ObjectNumber, PShipObjects );
				if ( NetConnected ) {
					//TODO: acknowledge kill to all remote players
				}
			}
		}
	}
}



