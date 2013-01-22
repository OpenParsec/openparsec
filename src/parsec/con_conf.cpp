/*
 * PARSEC - Configuration Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:34 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-1998
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
#include "vid_defs.h"

// local module header
#include "con_conf.h"

// proprietary module headers
#include "con_arg.h"
#include "con_main.h"
#include "con_std.h"
#include "e_color.h"
#include "net_conn.h"
#include "vid_plug.h"



// generic string paste area
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// string constants
static char netsubsys_syntax[]	= "syntax: net.subsys [<protocol>]";



// commands to get/set an rgba color value ------------------------------------
//
colcomm_s col_comms[] = {

	{ "panelback.i",	&PanelBackColor,		NULL				},
	{ "paneltext.i",	&PanelTextColor,		NULL				},
	{ "flarebase.i",	&FlareBaseColor,		NULL				},

	{ "ambient.i",		&LightColorAmbient,		NULL				},
	{ "diffuse.i",		&LightColorDiffuse,		NULL				},
	{ "specular.i",		&LightColorSpecular, 	NULL				},

	{ NULL,				NULL,					NULL				}
};

int num_col_commands = CALC_NUM_ARRAY_ENTRIES( col_comms ) - 1;


PRIVATE
int ExecColorCommand(int cmdid)
{
	if ( col_comms[cmdid].func != NULL )
		(*col_comms[cmdid].func)();
	
	return TRUE;
}


// check single command for color configuration (rgba) ------------------------
//
INLINE
int CheckColorCommand( int cmdid, char *scan )
{
	ASSERT( cmdid >= 0 );
	ASSERT( col_comms[ cmdid ].cmd && col_comms[ cmdid ].col );
	ASSERT( scan != NULL );

	int cmdlen = strlen( col_comms[ cmdid ].cmd );
	ASSERT( ( cmdlen > 1 ) && ( cmdlen < 32 ) );

	// early exit if wrong prefix
	if ( strncmp( scan, col_comms[ cmdid ].cmd, cmdlen - 2 ) != 0 )
		return FALSE;

	// make copy of command to avoid manipulating BSS string
	char cmdstr[ 32 ];
	strcpy( cmdstr, col_comms[ cmdid ].cmd );

	cmdstr[ cmdlen - 2 ] = '\0';
	if ( strcmp( scan, cmdstr ) == 0 ) {

		if ( strtok( NULL, " " ) != NULL ) {
			// no arguments must be specified!
			return FALSE;
		}

		// color command without field specifier displays (R,G,B,A)
		sprintf( paste_str, "%s currently is (rgba):", cmdstr );
		CON_AddLine( paste_str );
		sprintf( paste_str, "(%03d,%03d,%03d,%03d):",
							col_comms[ cmdid ].col->R, col_comms[ cmdid ].col->G,
							col_comms[ cmdid ].col->B, col_comms[ cmdid ].col->A );
		CON_AddLine( paste_str );

		return TRUE;
	}
	cmdstr[ cmdlen - 2 ] = '.';

	int byteconv = col_comms[ cmdid ].col->A;
	cmdstr[ cmdlen - 1 ] = 'a';
	if ( CheckSetIntArgBounded( "%d", scan, cmdstr, &byteconv, 0, COLRGBA_FULLYOPAQUE, NULL ) ) {
		col_comms[ cmdid ].col->A = byteconv;
		return ExecColorCommand(cmdid);
	}

	byteconv = col_comms[ cmdid ].col->R;
	cmdstr[ cmdlen - 1 ] = 'r';
	if ( CheckSetIntArgBounded( "%d", scan, cmdstr, &byteconv, 0, COLRGBA_FULLBRIGHT, NULL ) ) {
		col_comms[ cmdid ].col->R = byteconv;
		return ExecColorCommand(cmdid);
	}

	byteconv = col_comms[ cmdid ].col->G;
	cmdstr[ cmdlen - 1 ] = 'g';
	if ( CheckSetIntArgBounded( "%d", scan, cmdstr, &byteconv, 0, COLRGBA_FULLBRIGHT, NULL ) ) {
		col_comms[ cmdid ].col->G = byteconv;
		return ExecColorCommand(cmdid);
	}

	byteconv = col_comms[ cmdid ].col->B;
	cmdstr[ cmdlen - 1 ] = 'b';
	if ( CheckSetIntArgBounded( "%d", scan, cmdstr, &byteconv, 0, COLRGBA_FULLBRIGHT, NULL ) ) {
		col_comms[ cmdid ].col->B = byteconv;
		return ExecColorCommand(cmdid);
	}

	// luminance=(r,g,b)
	byteconv = col_comms[ cmdid ].col->G;
	cmdstr[ cmdlen - 1 ] = 'l';
	if ( CheckSetIntArgBounded( "%d", scan, cmdstr, &byteconv, 0, COLRGBA_FULLBRIGHT, NULL ) ) {
		col_comms[ cmdid ].col->R = byteconv;
		col_comms[ cmdid ].col->G = byteconv;
		col_comms[ cmdid ].col->B = byteconv;
		return ExecColorCommand(cmdid);
	}

	return FALSE;
}



// check all commands for color configuration (rgba) --------------------------
//
int CheckColorConfig( char *scan )
{
	ASSERT( scan != NULL );

	// scan list of color commands
	for ( int curcmd = 0; col_comms[ curcmd ].cmd; curcmd++ ) {

		if ( CheckColorCommand( curcmd, scan ) )
			return TRUE;
	}

	return FALSE;
}


// switch to alternate video configuration ------------------------------------
//
PRIVATE
void SetVideoConfiguration()
{
	// display active video subsystem if no parameter supplied
	char *vsys;
	if ( ( vsys = strtok( NULL, " " ) ) == NULL ) {
		// removed
		return;
	}

	// check for more than one parameter
	if ( strtok( NULL, " " ) != NULL ) {
		CON_AddLine( "specify video subsystem with a single word." );
		return;
	}

	// string must be copied because it will be destroyed
	// if a console script is executed on subsystem switch
	strcpy( paste_str, vsys );

	// try to change to specified video subsystem
	if ( VID_PlugInSubsys( vsys ) ) {
		strcat( paste_str, " selected as video subsystem." );
		CON_AddLine( paste_str );
	} else {
		strcat( paste_str, " video subsystem not available or invalid." );
		CON_AddLine( paste_str );
	}
}


// display name of currently active protocol ---------------------------------
//
void DisplayCurrentNetSubsys()
{
	// fetch string describing currently active protocol
	const char *prot = NET_GetCurrentProtocolName();

	// display
	sprintf( paste_str, "current networking subsystem: %s", prot);
	CON_AddLine( paste_str );
}


// switch to alternate network configuration (protocol/packet api) ------------
//
PRIVATE
void SetNetworkConfiguration()
{
	//NOTE:
	//CONCOM:
	// net_config ::= 'net.subsys' [<protocol>]
	// protocol   ::= 'peer-to-peer' | 'slot-server' | 'game-server'

	//NOTE:
	// displays currently active networking subsystem
	// if no parameter supplied (protocol/packet-api).

	// fetch protocol
	const char *prot = strtok( NULL, " " );
	if ( prot == NULL ) {
		DisplayCurrentNetSubsys();
		return;
	}

	// check for more than one parameter
	if ( strtok( NULL, " " ) != NULL ) {
		CON_AddLine( netsubsys_syntax );
//		return;
	}

	//FIXME: at startup the init messages are displayed twice due to net.conf in parsecrc.con

	// check for idempotent network configuration change
	const char* cur_prot = NET_GetCurrentProtocolName();
	if ( strncmp( cur_prot, prot, strlen( prot ) ) != 0 ) {

		// try to change to specified networking subsystem
		if ( NET_SwitchNetSubsys( prot ) ) {
			prot = NET_GetCurrentProtocolName();
			MSGOUT( "%s selected.", prot );
		} else {
			if ( prot == NULL )
				prot = NET_GetCurrentProtocolName();
			MSGOUT( "%s not available or invalid.", prot );
	//FIXME:		CON_AddLine( netsubsys_syntax );
		}
	}
}


// set audio configuration (music, sfx ) -------------------------------------
//
PRIVATE
void SetAudioConfiguration()
{
	//NOTE:
	//CONCOM:
	// audio_config ::= 'aud.conf' [conf_spec] [conf_spec] 
	// conf_spec    ::= 'off' | 'music' | 'sfx'

	//NOTE:
	// displays currently audio configuration
	// if no parameter supplied.

        if ( SoundDisabled ) {
	    return;
        }

	// fetch first option
	char *option1 = strtok( NULL, " " );
	if ( option1 == NULL ) {

		int cursetting = 0x00;
		if ( Op_SoundEffects )
			cursetting |= 0x01;
		if ( Op_Music )
			cursetting |= 0x02;

		switch ( cursetting ) {
		
			case 0x00:
				sprintf( paste_str, "current audio configuration: off" );
				break;

			case 0x01:
				sprintf( paste_str, "current audio configuration: sound effects only" );
				break;

			case 0x02:
				sprintf( paste_str, "current audio configuration: music only" );
				break;
		
			case 0x03:
				sprintf( paste_str, "current audio configuration: music and sound effects" );
				break;
		
		}
		
		CON_AddLine( paste_str );
		return;
	}

	int cursetting = 0x00;

	if ( stricmp( option1, "music" ) == 0 ) {
	
		cursetting |= 0x02;
		
	} else if ( stricmp( option1, "sfx" ) == 0 ) {
	
		cursetting |= 0x01;
		
	} else if ( stricmp( option1, "off" ) == 0 ) {

		cursetting |= 0x00;
	} 

	// fetch second option
	char *option2 = strtok( NULL, " " );

	if ( option2 != NULL ) {
		if ( stricmp( option2, "music" ) == 0 ) {
		
			cursetting |= 0x02;
			
		} else if ( stricmp( option2, "sfx" ) == 0 ) {
		
			cursetting |= 0x01;
			
		} else if ( stricmp( option2, "off" ) == 0 ) {
	
			cursetting |= 0x00;
		} 
	}
	
	// determine new setting
	Op_SoundEffects	= ( ( cursetting & 0x01 ) != 0x00 );
	Op_Music		= ( ( cursetting & 0x02 ) != 0x00 );
	
	// start/stop according to setting
	switch ( cursetting ) {

		//NOTE:
		// the sequence of AUDs_CloseMenuSound() and AUDs_StopAudioStream()
		// must not be changed, or the stream active flag will wrongly stick.

		case 0x00:
			//AUDs_CDStop();
			AUDs_CloseMenuSound();
			AUDs_StopAudioStream();
			break;

		case 0x01:
			//AUDs_CDStop();
			AUDs_CloseMenuSound();
			AUDs_StopAudioStream();
			break;

		case 0x02:
			if ( aud_do_open_menu_sound_on_aud_conf )
				AUDs_OpenMenuSound();
			break;

		case 0x03:
			if ( aud_do_open_menu_sound_on_aud_conf )
				AUDs_OpenMenuSound();
			break;

		default:
			ASSERT( 0 );
	}	
}


// set input configuration (mouse, joy) --------------------------------------
//
PRIVATE
void SetInputConfiguration()
{
	//NOTE:
	//CONCOM:
	// input_config ::= 'inp.conf' [conf_spec] [conf_spec] 
	// conf_spec    ::= 'off' | 'joystick' | 'mouse'

	//NOTE:
	// displays currently input configuration
	// if no parameter supplied.

	// fetch first option
	char *option = strtok( NULL, " " );
	if ( option == NULL ) {

		int cursetting = 0x00;
		if ( Op_Joystick )
			cursetting |= 0x01;
		if ( Op_Mouse )
			cursetting |= 0x02;

		switch ( cursetting ) {
		
			case 0x00:
				sprintf( paste_str, "current input configuration: off (keyboard only)" );
				break;

			case 0x01:
				sprintf( paste_str, "current input configuration: joystick and keyboard" );
				break;

			case 0x02:
				sprintf( paste_str, "current input configuration: mouse and keyboard" );
				break;
		}
		
		CON_AddLine( paste_str );
		return;
	}

	int cursetting = 0x00;

	if ( stricmp( option, "mouse" ) == 0 ) {
	
		cursetting |= 0x02;
		
	} else if ( stricmp( option, "joystick" ) == 0 ) {
	
		cursetting |= 0x01;
		
	} else if ( stricmp( option, "off" ) == 0 ) {

		cursetting |= 0x00;
	} 

	// ensure joystick is available
	if ( ( cursetting & 0x01 ) && JoystickDisabled ) {
		cursetting &= ~0x01;
	}

	// determine new setting
	Op_Joystick	= ( ( cursetting & 0x01 ) != 0x00 );
	Op_Mouse	= ( ( cursetting & 0x02 ) != 0x00 );
}


// commands to do on-the-fly configuration ------------------------------------
//
struct configcomm_s {

	int		cmdid;
	void	(*func)();
};

configcomm_s config_comms[] = {

	{ CM_VID_SUBSYS,	SetVideoConfiguration		},
	{ CM_NET_SUBSYS,	SetNetworkConfiguration		},
	{ CM_AUD_CONF,		SetAudioConfiguration		},
	{ CM_INP_CONF,		SetInputConfiguration		},

	{ 0,				NULL			}
};


// check single command for on-the-fly configuration --------------------------
//
INLINE
int CheckConfigCommand( int cmdid, char *scan )
{
	ASSERT( cmdid >= 0 );
	ASSERT( config_comms[ cmdid ].func );
	ASSERT( scan != NULL );

	if ( strcmp( scan, CMSTR( config_comms[ cmdid ].cmdid ) ) == 0 ) {

		if ( config_comms[ cmdid ].func != NULL )
			(*config_comms[ cmdid ].func)();

		return TRUE;
	}

	return FALSE;
}


// check configuration command ------------------------------------------------
//
int CheckConfigCommand( char *scan )
{
	ASSERT( scan != NULL );

	for ( int curcmd = 0; config_comms[ curcmd ].func; curcmd++ ) {

		if ( CheckConfigCommand( curcmd, scan ) )
			return TRUE;
	}

	return FALSE;
}



