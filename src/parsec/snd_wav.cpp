/*
 * PARSEC - WAV Format Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:42 $
 *
 * Orginally written by:
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1998-1999
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-1999
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   1998-1999
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

// sound API header
#include "snd_api.h"

// local module header
#include "snd_wav.h"

// proprietary module headers
#include "sys_file.h"



// string constants -----------------------------------------------------------
//
static char wav_readerror[] = "wav \"%s\" readerror";


// parse file header of already opened wav file -------------------------------
//
void WAV_ParseHeader( FILE *fp, sampleinfo_s *pSampleInfo, sample_s *pSample )
{
	ASSERT( fp != NULL );
	ASSERT( pSampleInfo != NULL );
	ASSERT( pSample != NULL );

	// read header field by field

	char magicword[ 4 ];

	if ( SYS_fread( &magicword, 1, 4, fp ) != 4 )
		FERROR( wav_readerror, pSampleInfo->file );

	if ( magicword[0] != 'R' ||
		 magicword[1] != 'I' ||
		 magicword[2] != 'F' ||
		 magicword[3] != 'F' )
		FERROR( wav_readerror, pSampleInfo->file );

	if ( SYS_fread( &pSample->totalfilelength, 1, 4, fp ) != 4 )
		FERROR( wav_readerror, pSampleInfo->file );

	pSample->totalfilelength = SWAP_32( pSample->totalfilelength );

	if ( SYS_fread( &magicword, 1, 4, fp ) != 4 )
		FERROR( wav_readerror, pSampleInfo->file );

	if ( magicword[0] != 'W' ||
		 magicword[1] != 'A' ||
		 magicword[2] != 'V' ||
		 magicword[3] != 'E' )
		FERROR( wav_readerror, pSampleInfo->file );

	if ( SYS_fread( &magicword, 1, 4, fp ) != 4 )
		FERROR( wav_readerror, pSampleInfo->file );

	if ( magicword[0] != 'f' ||
		 magicword[1] != 'm' ||
		 magicword[2] != 't' ||
		 magicword[3] != ' ' )
		FERROR( wav_readerror, pSampleInfo->file );

	if ( SYS_fread( &pSample->headerlength, 1, 4, fp ) != 4 )
		FERROR( wav_readerror, pSampleInfo->file );

	pSample->headerlength = SWAP_32( pSample->headerlength );
	if (pSample->headerlength != 16)
		FERROR( wav_readerror, pSampleInfo->file );


	if ( SYS_fread( &pSample->datatype, 1, 2, fp ) != 2 )
		FERROR( wav_readerror, pSampleInfo->file );

	pSample->datatype = SWAP_16( pSample->datatype );
	if (pSample->datatype != 1)
		FERROR( wav_readerror, pSampleInfo->file );


	if ( SYS_fread( &pSample->numchannels, 1, 2, fp ) != 2 )
		FERROR( wav_readerror, pSampleInfo->file );

	pSample->numchannels = SWAP_16( pSample->numchannels );
	if ( pSample->numchannels != 1 && pSample->numchannels != 2 )
		FERROR( wav_readerror, pSampleInfo->file );


	if ( SYS_fread( &pSample->samplerate, 1, 4, fp ) != 4 )
		FERROR( wav_readerror, pSampleInfo->file );

	pSample->samplerate = SWAP_32( pSample->samplerate );


	dword ldummy;
	if ( SYS_fread( &ldummy, 1, 4, fp ) != 4 )
		FERROR( wav_readerror, pSampleInfo->file );


	if ( SYS_fread( &pSample->alignment, 1, 2, fp ) != 2 )
		FERROR( wav_readerror, pSampleInfo->file );

	pSample->alignment = SWAP_16( pSample->alignment );


	if ( SYS_fread( &pSample->samplesize, 1, 2, fp ) != 2 )
		FERROR( wav_readerror, pSampleInfo->file );

	pSample->samplesize = SWAP_16( pSample->samplesize );

	if ( SYS_fread( &magicword, 1, 4, fp ) != 4 )
		FERROR( wav_readerror, pSampleInfo->file );

	if ( magicword[0] != 'd' ||
		 magicword[1] != 'a' ||
		 magicword[2] != 't' ||
		 magicword[3] != 'a' ) {
		if ( magicword[0] != 'f' ||
			 magicword[1] != 'a' ||
			 magicword[2] != 'c' ||
			 magicword[3] != 't' ) {
			FERROR( wav_readerror, pSampleInfo->file );
			}
		else {
		// skip 'fact' chunk
			if ( SYS_fread( &ldummy, 1, 4, fp ) != 4 )
				FERROR( wav_readerror, pSampleInfo->file );
			if ( SYS_fread( &ldummy, 1, 4, fp ) != 4 )
				FERROR( wav_readerror, pSampleInfo->file );
			if ( SYS_fread( &ldummy, 1, 4, fp ) != 4 )
				FERROR( wav_readerror, pSampleInfo->file );
		}
	}

	if ( SYS_fread( &pSample->samplebytes, 1, 4, fp ) != 4 )
		FERROR( wav_readerror, pSampleInfo->file );

	pSample->samplebytes = SWAP_32( pSample->samplebytes );
}


// open and read wav file -----------------------------------------------------
//
void WAV_ReadSample( sampleinfo_s *pSampleInfo, sample_s *pSample )
{
	ASSERT( pSampleInfo != NULL );
	ASSERT( pSample != NULL );

	// open wav file
	FILE *fp = SYS_fopen( pSampleInfo->file, "rb" );
	if ( fp == NULL )
		FERROR( wav_readerror, pSampleInfo->file );

	// parse header of wav file and fill pSample structure
	WAV_ParseHeader( fp, pSampleInfo, pSample );

	// alloc sample data buffer
	pSample->samplebuffer = (byte *) ALLOCMEM( pSample->samplebytes );
	for(unsigned int i = 0; i<pSample->samplebytes; i++)
		pSample->samplebuffer[i]=0;
	if ( pSample->samplebuffer == NULL )
		OUTOFMEM( "not enough mem for sample." );

	if ( SYS_fread( pSample->samplebuffer, 1, pSample->samplebytes, fp ) != pSample->samplebytes )
		FERROR( wav_readerror, pSampleInfo->file );

	SYS_fclose( fp );


#ifdef SYSTEM_BIG_ENDIAN

	// numframes are number of individual samples
	dword numframes = pSample->samplebytes;

	if ( pSample->numchannels == 2 )
		numframes = numframes >> 1;
	if ( pSample->samplesize == 16 )
		numframes = numframes >> 1;

	if ( pSample->samplesize == 16 ) {
		if ( pSample->numchannels == 2 ) {
			word *sweep = (word *) pSample->samplebuffer;
			for ( int i = 0; i < numframes * 2; i++, sweep++ )
				*sweep = SWAP_16( *sweep );
		} else {
			word *sweep = (word *) pSample->samplebuffer;
			for ( int i = 0; i < numframes; i++, sweep++ )
				*sweep = SWAP_16( *sweep );
		}

	}
#endif

	//FIXME: SEAL ?
	//ASSERT( pSample->numchannels == 1 );
}


// free a sample previously read with WAV_ReadSample() ------------------------
//
void WAV_FreeSample( sample_s *pSample )
{
	ASSERT( pSample != NULL );

	if ( pSample->samplebuffer != NULL ) {
		FREEMEM( pSample->samplebuffer );
		pSample->samplebuffer = NULL;
	}

	FREEMEM( pSample );
}


// convert a stereo sample to mono --------------------------------------------
//
void WAV_ConvertToMono( sample_s *pSample )
{
	ASSERT( pSample != NULL );

	if ( pSample->numchannels == 2 ) {
		// numframes are number of individual samples
		dword numframes = pSample->samplebytes >> 1;

		if ( pSample->samplesize == 16 ) {

			// handle 16 bit stereo samples
			numframes = numframes >> 1;

			word* source = (word *) pSample->samplebuffer;
			word* dest	 = (word *) ALLOCMEM( pSample->samplebytes >> 1 );
			word* new_samplebuffer = dest;
//			signed long  temp;
//			signed short s1,s2;

			for ( dword i = 0; i < numframes; i++ ) {

//#define AVERAGING
#ifdef AVERAGING

				//FIXME: this does not work yet

				//temp = ( (dword) *source ) + ( (dword) *( source+1 ) );
				s1 = *source;
				s2 = *source+1;

				temp =  (signed long)( s1 ) & 0x0000ffff ;
				temp += (signed long)( s2 ) & 0x0000ffff ;

				*dest = (signed short)( ( temp / 2 ) & 0xffff );

#else

				*dest = ( *source + 1 );

#endif // AVERAGING

				//*dest = ( *source >> 1 ) + ( *( source+1 ) >> 1 );
				//*dest = ( *source ) >> 1;
				//source++;
				//*dest += ( *source ) >> 1;

				source++;
				source++;
				dest++;
			}

			// free the stereo buffer
			FREEMEM( pSample->samplebuffer );

			// assign the mono buffer, and correct samplebytes, numchannels, alignment
			pSample->samplebuffer = (byte*) new_samplebuffer;
			pSample->samplebytes  = pSample->samplebytes >> 1;
			pSample->numchannels  = 1;
			pSample->alignment	  = pSample->alignment >> 1;

		} else {

			// handle 8 bit stereo samples
			byte* source = (byte *) pSample->samplebuffer;
			byte* dest	 = (byte *) ALLOCMEM( pSample->samplebytes >> 1 );
			byte* new_samplebuffer = dest;
//			signed long  temp;
//			signed short s1,s2;

			for ( dword i = 0; i < numframes; i++ ) {

				*dest = ( *source + 1 );

				//*dest = ( *source >> 1 ) + ( *( source+1 ) >> 1 );
				//*dest = ( *source ) >> 1;
				//source++;
				//*dest += ( *source ) >> 1;

				source++;
				source++;
				dest++;
			}

			// free the stereo buffer
			FREEMEM( pSample->samplebuffer );

			// assign the mono buffer, and correct samplebytes, numchannels, alignment
			pSample->samplebuffer = (byte*) new_samplebuffer;
			pSample->samplebytes  = pSample->samplebytes >> 1;
			pSample->numchannels  = 1;
			pSample->alignment	  = pSample->alignment >> 1;
		}
	}
}


// convert a sample to a given samplerate -------------------------------------
//
void WAV_ConvertRate( sample_s *pSample, int samplerate )
{
	ASSERT( pSample != NULL );
	ASSERT( ( samplerate >= 11025 ) && ( samplerate <= 44100 ) );

	int origrate = pSample->samplerate;

	// we don't upsample
	if ( origrate <= samplerate )
		return;

	float resampleratio = ( (float) origrate ) / ( (float) samplerate );

	// numframes are number of individual samples
	dword numbytes = pSample->samplebytes;
	dword numframes = numbytes;

	if ( pSample->numchannels == 2 )
		numframes = numframes >> 1;
	if ( pSample->samplesize == 16 )
		numframes = numframes >> 1;

	dword destframes = FLOAT2INT( numframes / resampleratio );
	dword destbytes = FLOAT2INT( numbytes / resampleratio );

	byte*	newbuffer = NULL;
	int 	destsample;
	float srcpos = 0;

	if ( pSample->samplesize == 16 ) {

		word* source = (word *) pSample->samplebuffer;
		word* dest	 = (word *) ALLOCMEM( destbytes );
		if ( dest == NULL )
			OUTOFMEM( 0 );

		newbuffer = (byte *) dest;

		if ( pSample->numchannels == 2 ) {

			for ( dword sid = 0; sid < destframes*2; sid += 2 ) {
				destsample = source[ FLOAT2INT( srcpos ) ];
				dest[ sid ] = destsample;
				destsample = source[ FLOAT2INT( srcpos + 1 ) ];
				dest[ sid + 1 ] = destsample;

				srcpos += resampleratio * 2;
			}

		} else if ( pSample->numchannels == 1 ) {

			for ( dword sid = 0; sid < destframes; sid++ ) {
				destsample = source[ FLOAT2INT( srcpos ) ];
				dest[ sid ] = destsample;

				srcpos += resampleratio;
			}
		}

	} else if ( pSample->samplesize == 8 ) {

		byte* source = (byte *) pSample->samplebuffer;
		byte* dest	 = (byte *) ALLOCMEM( destbytes );
		if ( dest == NULL )
			OUTOFMEM( 0 );

		newbuffer = dest;

		if ( pSample->numchannels == 2 ) {

			for ( dword sid = 0; sid < destframes*2; sid += 2 ) {
				destsample = source[ FLOAT2INT( srcpos ) ];
				dest[ sid ] = destsample;
				destsample = source[ FLOAT2INT( srcpos + 1 ) ];
				dest[ sid + 1 ] = destsample;

				srcpos += resampleratio * 2;
			}

		} else if ( pSample->numchannels == 1 ) {

			for ( dword sid = 0; sid < destframes; sid++ ) {
				destsample = source[ FLOAT2INT( srcpos ) ];
				dest[ sid ] = destsample;

				srcpos += resampleratio;
			}
		}
	}

	FREEMEM( pSample->samplebuffer );
	pSample->samplebuffer = newbuffer;
	pSample->samplerate   = samplerate;
	pSample->samplebytes  = destbytes;
}



