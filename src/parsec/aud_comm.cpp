/*
 * PARSEC - Audio Console Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:33 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   1999
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

// local module header
#include "aud_comm.h"

// proprietary module headers
#include "aud_supp.h"
#include "con_arg.h"
#include "con_com.h"
#include "con_int.h"
#include "con_main.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// open particular cd drive (for those with more than one drive) --------------
//
PRIVATE
int CD_Open( char *drivestr )
{
	ASSERT( drivestr != NULL );
	HANDLE_COMMAND_DOMAIN( drivestr );

	return TRUE;
}


// key table for cd playing ---------------------------------------------------
//
key_value_s cdplay_key_value[] = {

	{ "from",		NULL,	KEYVALFLAG_NONE	},
	{ "to",			NULL,	KEYVALFLAG_NONE },

	{ NULL,			NULL,	KEYVALFLAG_NONE	},
};

enum {

	KEY_CDPLAY_FROM,
	KEY_CDPLAY_TO,
};


// start playing cd audio track -----------------------------------------------
//
PRIVATE
int CD_Play( char *trackstr )
{
	ASSERT( trackstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( trackstr );
/*
	// create pointer to list of parameters (first is always the track)
	char *tracknumber = strtok( trackstr, " " );
	if ( tracknumber == NULL ) {
		CON_AddLine( "track number must be specified." );
		return TRUE;
	}

	// check track parameter
	char *errpart;
	long track = strtol( tracknumber, &errpart, 10 );
	if ( *errpart != 0 ) {
		CON_AddLine( "invalid track parameter." );
		return TRUE;
	}

	// scan out all values to track keys
	if ( !ScanKeyValuePairs( cdplay_key_value, NULL ) )
		return TRUE;

	// defaults
	int nFrom = -1;
	int nTo = -1;

	if ( ScanKeyValueInt( &cdplay_key_value[ KEY_CDPLAY_FROM ], &nFrom ) < 0 ) {
		CON_AddLine( "invalid from parameter." );
		return TRUE;
	}
	if ( ScanKeyValueInt( &cdplay_key_value[ KEY_CDPLAY_TO ], &nTo ) < 0 ) {
		CON_AddLine( "invalid to parameter." );
		return TRUE;
	}

	// issue the play command
	AUDs_CDPlay( (short)track, nFrom, nTo );
*/
	return TRUE;
}


// stop playing cd audio track ------------------------------------------------
//
PRIVATE
int CD_Stop( char *params )
{
	ASSERT( params != NULL );
	USERCOMMAND_NOPARAM( params );


	return TRUE;
}


// pause playing of cd audio track --------------------------------------------
//
PRIVATE
int CD_Pause( char *params )
{
	ASSERT( params != NULL );
	USERCOMMAND_NOPARAM( params );

	return TRUE;
}


// resume playing of cd audio track -------------------------------------------
//
PRIVATE
int CD_Resume( char *params )
{
	ASSERT( params != NULL );
	USERCOMMAND_NOPARAM( params );


	return TRUE;
}


// eject audio cd -------------------------------------------------------------
//
PRIVATE
int CD_Eject( char *params )
{
	ASSERT( params != NULL );
	USERCOMMAND_NOPARAM( params );


	return TRUE;
}


// set cd volume --------------------------------------------------------------
//
PRIVATE
int CD_Volume( char *volumestr )
{
	ASSERT( volumestr != NULL );
	HANDLE_COMMAND_DOMAIN( volumestr );
/*
	int  nVolume = AUDs_CDGetVolume();

	char *params;

	if ( (params = QueryIntArgumentEx( volumestr, "%d", &nVolume )) ) {

		// determine if delta modification (++/--)
		int delta = 0;
		if ( ( params[ 0 ] == '+' ) && ( params[ 1 ] == '+' ) )
			delta = 1;
		else if ( ( params[ 0 ] == '-' ) && ( params[ 1 ] == '-' )  )
			delta = -1;
		if ( delta != 0 )
			params += 2;

		if ( *params != 0 ) {
			char *errpart;

			long sval = strtol( params, &errpart, 10 );
			if ( *errpart == 0 ) {
				if ( delta != 0 )
					sval = nVolume + sval * delta;
				if ( sval >= AUD_MIN_VOLUME && sval <= AUD_MAX_VOLUME ) {
					nVolume = sval;
					AUDs_CDSetVolume( nVolume );
				} else {
					CON_AddLine( range_error );
				}
			} else {
				CON_AddLine( invalid_arg );
			}
		} else {
			CON_AddLine( invalid_arg );
		}
	}
*/
	return TRUE;
}


// list the contents of the cd ------------------------------------------------
//
PRIVATE
int CD_List( char *params )
{
	ASSERT( params != NULL );
	USERCOMMAND_NOPARAM( params );


	return TRUE;
}


// register cd audio console commands -----------------------------------------
//
void AUD_RegisterCDCommands()
{
	static int registration_done = FALSE;

	if ( !registration_done ) {

		registration_done = TRUE;

		user_command_s regcom;
		memset( &regcom, 0, sizeof( user_command_s ) );

		regcom.command	 = "cd.open";
		regcom.numparams = 1;
		regcom.execute	 = CD_Open;
		CON_RegisterUserCommand( &regcom );

		regcom.command	 = "cd.play";
		regcom.numparams = 1;
		regcom.execute	 = CD_Play;
		CON_RegisterUserCommand( &regcom );

		regcom.command	 = "cd.stop";
		regcom.numparams = 0;
		regcom.execute	 = CD_Stop;
		CON_RegisterUserCommand( &regcom );

		regcom.command	 = "cd.pause";
		regcom.numparams = 0;
		regcom.execute	 = CD_Pause;
		CON_RegisterUserCommand( &regcom );

		regcom.command	 = "cd.resume";
		regcom.numparams = 0;
		regcom.execute	 = CD_Resume;
		CON_RegisterUserCommand( &regcom );

		regcom.command	 = "cd.eject";
		regcom.numparams = 0;
		regcom.execute	 = CD_Eject;
		CON_RegisterUserCommand( &regcom );

		regcom.command	 = "cd.volume";
		regcom.numparams = 1;
		regcom.execute	 = CD_Volume;
		CON_RegisterUserCommand( &regcom );

		regcom.command	 = "cd.list";
		regcom.numparams = 0;
		regcom.execute	 = CD_List;
		CON_RegisterUserCommand( &regcom );
	}
}

// set global volume via console command --------------------------------------
//
PRIVATE
int Cmd_AUD_VOLUME( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// aud_volume_command	::= 'aud.volume' [<volume>]

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );
	// create pointer to list of parameters (first is always the name)
	char *volume = strtok( paramstr, " " );
	if ( volume == NULL ) {
		sprintf( paste_str, "%d", GlobalVolume );
		CON_AddLine( paste_str );
		return TRUE;
	} else {
		char *errpart;
		long lVolume = strtol( volume, &errpart, 10 );
		if ( *errpart != 0 ) {
			CON_AddLine( "invalid argument." );
			return TRUE;
		}

		if ( ( lVolume < AUD_MIN_VOLUME ) || ( lVolume > AUD_MAX_VOLUME) ) {
			CON_AddLine( "range error." );
			return TRUE;
		}

		GlobalVolume = lVolume;
	}
	return TRUE;
}

// set sample volume (without reloading) via console command ------------------
//
PRIVATE
int Cmd_AUD_SAMPLEVOLUME( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// aud_samplevolume_command	::= 'aud.samplevolume' <samplename> [<volume>]

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	// create pointer to list of parameters (first is always the name)
	char *samplename = strtok( paramstr, " " );
	if ( samplename == NULL ) {
		CON_AddLine( "name of sample must be specified." );
		return TRUE;
	}

	// allow name to be parenthesized to include whitespace
	samplename = GetParenthesizedName( samplename );
	if ( samplename == NULL ) {
		CON_AddLine( "samplename name invalid." );
		return TRUE;
	}

	// lookup the sample
	int nSampleID = -1;
	sampleinfo_s* pSampleInfo;

	pSampleInfo = AUD_FetchSampleByName( samplename, &nSampleID );
	if( pSampleInfo == NULL ) {
		CON_AddLine( "sample not found." );
		return TRUE;
	}

	// advance to next token
	paramstr = strtok( NULL, " " );

	int syntaxok = -1;

	// check for specified volume and either set it or print the current volume
	long lVolume;
	if( paramstr != NULL) {

		char *errpart;
		lVolume = strtol( paramstr, &errpart, 10 );
		if ( *errpart != 0 ) {
			CON_AddLine( "invalid argument." );
			return TRUE;
		}

		syntaxok = 1;
	}

	if ( syntaxok ) {
		if ( syntaxok == -1 ) {
			sprintf( paste_str, "%d", pSampleInfo->volume );
			CON_AddLine( paste_str );
		} else if ( lVolume < AUD_MIN_VOLUME || lVolume > AUD_MAX_VOLUME) {
			CON_AddLine( "range error." );
		} else {
			if( AUD_SetSampleVolume( nSampleID, lVolume ) == FALSE ) {
				CON_AddLine( "setting volume failed." );
			}
		}
	}

	return TRUE;
}


// key table for sample playing ---------------------------------------------------
//
key_value_s sampleplay_key_value[] = {

	{ "name",		NULL,	KEYVALFLAG_PARENTHESIZE	},
	{ "id",			NULL,	KEYVALFLAG_NONE 		},

	{ NULL,			NULL,	KEYVALFLAG_NONE			},
};

enum {

	KEY_SAMPLEPLAY_NAME,
	KEY_SAMPLEPLAY_ID,
};


// play sample via console command --------------------------------------------
//
PRIVATE
int Cmd_AUD_PLAYSAMPLE( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// aud_playsample_command	::= 'aud.playsample' <samplespec>
	// samplespec               ::= <samplename> | <sampleid>
	// samplename				::= 'name' <name of sample>
	// sampleid					::= 'id' <id of sample>

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	// defaults
	char *samplename = NULL;
	int nSampleID = -1;
	sampleinfo_s* pSampleInfo = NULL;

	// scan out all values to sampleplay keys
	if ( !ScanKeyValuePairs( sampleplay_key_value, paramstr ) )
		return TRUE;

	// check whether id or name was specified
	if ( sampleplay_key_value[ KEY_SAMPLEPLAY_NAME ].value != NULL ) {

		samplename = sampleplay_key_value[ KEY_SAMPLEPLAY_NAME ].value;

		// lookup the sample
		pSampleInfo = AUD_FetchSampleByName( samplename, &nSampleID );

	} else if ( ScanKeyValueInt( &sampleplay_key_value[ KEY_SAMPLEPLAY_ID ], &nSampleID ) >= 0 ) {

		// lookup the sample
		pSampleInfo = AUD_FetchSampleById( nSampleID );

	} else {
		CON_AddLine( "invalid parameter." );
		return TRUE;
	}

	if( pSampleInfo == NULL ) {
		CON_AddLine( "sample not found." );
		return TRUE;
	}

	// get the pointer to the wave data
	audiowave_t wave = pSampleInfo->samplepointer;

	// retrieve the volume for this sample
	SoundParams_s sound_params;
	memset( &sound_params, 0, sizeof( sound_params ) );
	sound_params.volume = ( pSampleInfo->volume == -1 ) ? AUD_MAX_VOLUME : pSampleInfo->volume;
	sound_params.flags = SOUNDPARAMS_VOLUME;

	// and play the sample
	AUDs_PlayVoiceBuffer( /*STD_VOICE*/2, wave, &sound_params );

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( AUD_COMM )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "aud.samplevolume" command
	regcom.command	 = "aud.samplevolume";
	regcom.numparams = 2;
	regcom.execute	 = Cmd_AUD_SAMPLEVOLUME;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "aud.volume" command
	regcom.command	 = "aud.volume";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_AUD_VOLUME;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "aud.playsample" command
	regcom.command	 = "aud.playsample";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_AUD_PLAYSAMPLE;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );
}



