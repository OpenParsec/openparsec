/*
 * PARSEC - Supporting Audio Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:21 $
 *
 * Orginally written by:
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
#include "aud_supp.h"

// proprietary module headers
#include "con_aux.h"



// find a sample specified by a name ------------------------------------------
//
sampleinfo_s* AUD_FetchSampleByName( const char *name, int *id )
{
	ASSERT( name != NULL );
	ASSERT( id != NULL );

	// scan entire table of samples
	for ( int sampleid = 0; sampleid < NumLoadedSamples; sampleid++ ) {
		if ( stricmp( SampleInfo[ sampleid ].name, name ) == 0 ) {
			*id = sampleid;
			return &SampleInfo [ sampleid ];
		}
	}

	// no sample of this name found
	return NULL;
}


// find a sample specified by its id ------------------------------------------
//
sampleinfo_s* AUD_FetchSampleById( int id )
{
	//NOTE:
	// this function may be invoked for invalid
	// sample ids. in that case NULL will be
	// returned.

	// ensure id is valid
	if ( ( id < 0 ) || ( id >= NumLoadedSamples ) )
		return (sampleinfo_s*)NULL;

	return &SampleInfo [ id ];
}


// change the volume of the specified sample ----------------------------------
//
int AUD_SetSampleVolume( int id, int volume )
{
	//NOTE:
	// this function may be invoked for invalid
	// sample ids. in that case FALSE will be
	// returned.

	if ( ( id >= 0 ) && ( id < NumLoadedSamples ) ) {

		SampleInfo [ id ].volume = volume;
		return TRUE;
	}

	return FALSE;
}


// play a sample on a channel -------------------------------------------------
//
int AUD_PlaySampleOnChannel( sample_channel_info_s *info )
{
	ASSERT( info != NULL );

	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	if ( info->interruptold == FALSE ) {

		// check whether any previous playing has already stopped
		int stopped;
		AUDs_GetVoiceStatus( info->channel, &stopped );

		if ( stopped == 0 )
			return FALSE;
	}

	if ( info->fallbacksample ) {

		AUD_FETCHSAMPLE_FALLBACK_EX( info->samplename, info->sampleid,
									 info->fallbacksample );
		if ( fetch_wave != NULL ) {
			AUXDATA_LAST_SOUNDSYS_RETURNCODE =
				AUDs_PlayVoiceBuffer( info->channel, fetch_wave, &fetch_params );
		}

	} else {

		AUD_FETCHSAMPLE_EX( info->samplename, info->sampleid );

		if ( fetch_wave != NULL ) {
			AUXDATA_LAST_SOUNDSYS_RETURNCODE =
				AUDs_PlayVoiceBuffer( info->channel, fetch_wave, &fetch_params );
		}
	}

	return TRUE;
}



