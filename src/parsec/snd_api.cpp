/*
 * PARSEC - Interface/Utility Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:42 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   1998
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

// local module header
#include "snd_api.h"

// proprietary module headers
#include "snd_wav.h"
#include "sys_path.h"



// sound file extensions ------------------------------------------------------
//
static char snd_ext_wav[]	= ".wav";


// read sample after determining its format -----------------------------------
//
sample_s *SND_ReadSample( sampleinfo_s *pSampleInfo )
{
	ASSERT( pSampleInfo != NULL );

	// determine sound type from extension
	char *fext = SYSs_ScanToExtension( pSampleInfo->file );

	sample_s *pSample = (sample_s *) ALLOCMEM(sizeof(sample_s));

	pSample->format = SAMPLEFORMAT_INVALID;
//	pSample->samplebuffer;

	if ( stricmp( fext, snd_ext_wav ) == 0 ) {
		pSample->format = SAMPLEFORMAT_WAV;
		WAV_ReadSample(pSampleInfo,pSample);
	}

	return pSample;
}


// free sample according to its format ----------------------------------------
//
void SND_FreeSample( sample_s *pSample )
{
	ASSERT( pSample != NULL );

	switch ( pSample->format ) {

		case SAMPLEFORMAT_WAV:
			WAV_FreeSample( pSample );
			break;

		case SAMPLEFORMAT_INVALID:
			FREEMEM( pSample );
			break;

		default:
			ASSERT( 0 );
	}
}


// convert sample to mono if in stereo ----------------------------------------
//
void SND_ConvertToMono( sample_s *pSample )
{
	ASSERT( pSample != NULL );

	switch ( pSample->format ) {

		case SAMPLEFORMAT_WAV:
			WAV_ConvertToMono( pSample );
			break;

		case SAMPLEFORMAT_INVALID:
			FREEMEM( pSample );
			break;

		default:
			ASSERT( 0 );
	}
}


// convert a sample to a given samplerate -------------------------------------
//
void SND_ConvertRate( sample_s *pSample, int samplerate )
{
	ASSERT( pSample != NULL );

	switch ( pSample->format ) {

		case SAMPLEFORMAT_WAV:
			WAV_ConvertRate( pSample, samplerate );
			break;

		case SAMPLEFORMAT_INVALID:
			FREEMEM( pSample );
			break;

		default:
			ASSERT( 0 );
	}
}



