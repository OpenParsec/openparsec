/*
 * PARSEC - Input Console Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:27 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999-2000
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
#include "aud_defs.h"
#include "inp_defs.h"

// local module header
#include "inp_comm.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_main.h"
#include "g_supp.h"
#include "inp_user.h"



// mapping table for missile selection ids ------------------------------------
//
static int missile_select_map[ NUMBER_OF_SELECTABLE_MISSILETYPES ] = {
	0, 1, 3, 2,
};


// set selected weapon directly using id --------------------------------------
//
PRIVATE
int Cmd_INP_WEAPONSEL( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// inp_weaponsel_command ::= 'inp.weaponsel' <number>

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	int32 selectedweapon = 0;

	if ( GetIntBehindCommand( paramstr, &selectedweapon, 10, FALSE ) ) {

		if ( ( selectedweapon >= 0 ) && ( selectedweapon < 8 ) ) {

			if ( !GAME_MODE_ACTIVE() )
				return TRUE;

			if ( selectedweapon < NUMBER_OF_SELECTABLE_GUNTYPES ) {

				// disallow gun selection when one is currently active
				if ( MyShip->WeaponsActive != 0x0000 ) {
					return TRUE;
				}

				SelectedLaser = selectedweapon;

				// play sample for new selected gun
				AUD_SelectLaser( SelectedLaser );

				return TRUE;
			}
			selectedweapon -= NUMBER_OF_SELECTABLE_GUNTYPES;

			if ( selectedweapon < NUMBER_OF_SELECTABLE_MISSILETYPES ) {
				SelectedMissile = missile_select_map[ selectedweapon ];

				// play sample for new selected missile
				AUD_SelectMissile( SelectedMissile );

				return TRUE;
			}
			selectedweapon -= NUMBER_OF_SELECTABLE_MISSILETYPES;

		} else {
			CON_AddLine( range_error );
		}
	}

	return TRUE;
}


// cycle guns -----------------------------------------------------------------
//
PRIVATE
int Cmd_INP_CYCLEGUNS( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// inp_cycleguns_command ::= 'inp.cycleguns'

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	if ( strtok( paramstr, " " ) == NULL ) {

		if ( GAME_MODE_ACTIVE() ) {
			INP_UserCycleGuns();
		}

	} else {
		CON_AddLine( too_many_args );
	}

	return TRUE;
}


// cycle missiles -------------------------------------------------------------
//
PRIVATE
int Cmd_INP_CYCLEMISSILES( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// inp_cyclemissiles_command ::= 'inp.cyclemissiles'

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	if ( strtok( paramstr, " " ) == NULL ) {

		if ( GAME_MODE_ACTIVE() ) {
			INP_UserCycleMissiles();
		}

	} else {
		CON_AddLine( too_many_args );
	}

	return TRUE;
}


// cycle targets --------------------------------------------------------------
//
PRIVATE
int Cmd_INP_CYCLETARGETS( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// inp_cycletargets_command ::= 'inp.cycletargets'

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	if ( strtok( paramstr, " " ) == NULL ) {

		if ( GAME_MODE_ACTIVE() ) {
			INP_UserCycleTargets();
		}

	} else {
		CON_AddLine( too_many_args );
	}

	return TRUE;
}


// set local ship speed to zero -----------------------------------------------
//
PRIVATE
int Cmd_INP_ZEROSPEED( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// inp_zerospeed_command ::= 'inp.cycletargets'

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	if ( strtok( paramstr, " " ) == NULL ) {

		if ( GAME_MODE_ACTIVE() ) {
			INP_UserZeroSpeed();
		}

	} else {
		CON_AddLine( too_many_args );
	}

	return TRUE;
}


// set local ship speed to target's speed -------------------------------------
//
PRIVATE
int Cmd_INP_TARGETSPEED( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// inp_targetspeed_command ::= 'inp.targetspeed'

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	if ( strtok( paramstr, " " ) == NULL ) {

		if ( GAME_MODE_ACTIVE() ) {
			INP_UserTrackTargetSpeed();
		}

	} else {
		CON_AddLine( too_many_args );
	}

	return TRUE;
}


// cycle panel display --------------------------------------------------------
//
PRIVATE
int Cmd_INP_CYCLEPANEL( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// inp_cyclepanel_command ::= 'inp.cyclepanel' <number>

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	int32 panelid = 0;

	if ( GetIntBehindCommand( paramstr, &panelid, 10, FALSE ) ) {

		// display toggling (panels)
		#define PANEL_3_NUM_OF_STATES		4
		#define PANEL_4_NUM_OF_STATES		3

		if ( panelid == 3 ) {

			AUX_HUD_PANEL_3_CONTROL++;
			if ( AUX_HUD_PANEL_3_CONTROL >= PANEL_3_NUM_OF_STATES ) {
				AUX_HUD_PANEL_3_CONTROL = 0;
			}

		} else if ( panelid == 4 ) {

			AUX_HUD_PANEL_4_CONTROL++;
			if ( AUX_HUD_PANEL_4_CONTROL >= PANEL_4_NUM_OF_STATES ) {
				AUX_HUD_PANEL_4_CONTROL = 0;
			}

		} else {

			CON_AddLine( range_error );
		}
	}

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( INP_COMM )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "inp.weaponsel" command
	regcom.command	 = "inp.weaponsel";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_INP_WEAPONSEL;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "inp.cycleguns" command
	regcom.command	 = "inp.cycleguns";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_INP_CYCLEGUNS;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "inp.cyclemissiles" command
	regcom.command	 = "inp.cyclemissiles";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_INP_CYCLEMISSILES;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "inp.cycletargets" command
	regcom.command	 = "inp.cycletargets";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_INP_CYCLETARGETS;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "inp.zerospeed" command
	regcom.command	 = "inp.zerospeed";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_INP_ZEROSPEED;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "inp.targetspeed" command
	regcom.command	 = "inp.targetspeed";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_INP_TARGETSPEED;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "inp.cyclepanel" command
	regcom.command	 = "inp.cyclepanel";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_INP_CYCLEPANEL;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );
}



