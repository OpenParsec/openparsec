/*
 * PARSEC - SDL Mixer Functions
 *
 * $Authors: mrsid $ - $Date: 2004/10/04 12:06:04 $
 *           Gold-Reaver $ - $Date: 2004/10/04 12:06:04 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-2000
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
#include "config.h"
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
//#include "al_null.h"
#include "snd_api.h"
#include "snd_wav.h"
#include "aud_bkgn.h"
#include "aud_supp.h"
#include "sys_file.h"
#include "sys_path.h"

#ifdef SYSTEM_TARGET_LINUX
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_mixer.h>
#else
	#include <SDL.h>
	#include <SDL_mixer.h>
#endif
// flags

static int snd_init_done = FALSE;
static int in_game_mode = FALSE;
Mix_Chunk *snd_chunks[MAX_SAMPLES];
Mix_Music *music;
char *tmp_music_buffer = NULL;

// play the specified sound on channel voice ----------------------------------
//
int AUDs_PlayVoice( int voice, int sound )
{
  if (SoundDisabled)
      return AUD_RETURN_SUCCESS;
	
    Mix_PlayChannel(voice, snd_chunks[sound], 0);
	
    return AUD_RETURN_SUCCESS;
}


// play the specified wave on channel voice with normal stereo ----------------
//
int AUDs_PlayVoiceBuffer( int voice, audiowave_t wave, SoundParams_s *params )
{
	if (SoundDisabled)
		return AUD_RETURN_SUCCESS;
	
	if (params != NULL) {

		if ( !(params->flags & SOUNDPARAMS_LOOP) ) {
			params->numLoops = 0;
		} else {
			if( !(params->flags & SOUNDPARAMS_LOOP_COUNT))
				params->numLoops = -1;
		}

		Mix_PlayChannel(voice, snd_chunks[(long)wave], params->numLoops );
		
	} else {
		
		Mix_PlayChannel(voice, snd_chunks[(long)wave], 0);
		return AUD_RETURN_SUCCESS;
		
	}


	if ( params->flags & SOUNDPARAMS_VOLUME) {
		if (params->volume > 128)
			params->volume = 128;

		Mix_Volume(voice, params->volume);
	}
	
	return AUD_RETURN_SUCCESS;
}


// set the volume for the specified channel -----------------------------------
//
int AUDs_SetVoiceVolume( int voice, int volume )
{
	if (SoundDisabled)
		return AUD_RETURN_SUCCESS;
	
	if (volume > 128)
		volume = 128;
	
    Mix_Volume(voice, volume);

    return AUD_RETURN_SUCCESS;
}


// load wave file into memory -------------------------------------------------
//
PUBLIC int aud_sample_quality = 44100;

int AUDs_LoadWaveFile( int num )
{
	if (SoundDisabled)
		return AUD_RETURN_SUCCESS;
	
    ASSERT( ( num >= 0 ) && ( num < MAX_SAMPLES ) );

	if ( !SoundAvailable )
		return AUD_RETURN_SUCCESS;

	// read the sample and assign the list entry
	sample_s* pSample = SND_ReadSample( &SampleInfo[ num ] );

	SND_ConvertRate( pSample, aud_sample_quality );

	// because SDL_RWFromFP doesn't work cross platform, we need
	// to get a little bit kinky

	// set up a buffer for the file
	char *tmp_sample_buffer = NULL;
	size_t tmp_sample_size = 0;

	// get the size of this sample
	tmp_sample_size = SYS_GetFileLength(SampleInfo[num].file);
	if (tmp_sample_size <= 0) {
		return FALSE; // return on error
	}

	// alloc space for the buffer
	tmp_sample_buffer = (char *)ALLOCMEM(tmp_sample_size + 1);

	// open the sample file
	FILE *wave = SYS_fopen(SampleInfo[num].file, "rb");

	if (wave == NULL)
		return FALSE;

	// read the file into the temp_sample_buffer
	int read_rc = SYS_fread((void *)tmp_sample_buffer, 1, tmp_sample_size, wave);

	if (read_rc <= 0) {
		MSGOUT("ERROR: Error reading sample %s.", SampleInfo[num].file);
		return FALSE;
	}

	SDL_RWops *waverwops = SDL_RWFromMem((void *)tmp_sample_buffer, tmp_sample_size);
	snd_chunks[num] = Mix_LoadWAV_RW(waverwops, 0);
	SYS_fclose(wave);

	FREEMEM( tmp_sample_buffer );
	FREEMEM( pSample->samplebuffer );
	pSample->samplebuffer = NULL;
        

#ifdef SHOW_SAMPLE_LOADING_INFO

	MSGOUT("loaded %-12s to slot %03d: rate: %05d, numch: %02d, type: %s, bytes: %07d",
		SampleInfo[ num ].file, num, pSample->samplerate, pSample->numchannels,
		( pSample->samplesize == 8 ) ? "8" : "16" , pSample->samplebytes );

#endif // SHOW_SAMPLE_LOADING_INFO

	// store the slot submitted to the play functions
	SampleInfo[ num ].samplepointer = (char *)num;

	//CAVEAT: for now we store the number of refframes in size
	SampleInfo[ num ].size = FLOAT2INT( (float) FRAME_MEASURE_TIMEBASE *
		(float) pSample->samplebytes / (float) ( pSample->samplerate * pSample->alignment ) + 0.5f );
	
	FREEMEM( pSample );

	return AUD_RETURN_SUCCESS;
}


// free resources associated with wave file -----------------------------------
//
int AUDs_FreeWaveFile( int num )
{
	if (SoundDisabled)
		return AUD_RETURN_SUCCESS;
	
	ASSERT( ( num >= 0 ) && ( num < MAX_SAMPLES ) );

	if ( !SoundAvailable )
		return AUD_RETURN_SUCCESS;
	
    Mix_FreeChunk(snd_chunks[num]);
	
    return AUD_RETURN_SUCCESS;
}


// set panning of channel voice -----------------------------------------------
//
int AUDs_SetVoicePanning( int voice, int panning )
{
	if (SoundDisabled)
		return AUD_RETURN_SUCCESS;
	
    Mix_SetPanning(voice, panning, panning);

    return AUD_RETURN_SUCCESS;
}


// set frequency of channel voice ---------------------------------------------
//
int AUDs_SetVoiceFrequency( int voice, int freq )
{
    return AUD_RETURN_SUCCESS;
}


// get replay status of channel voice -----------------------------------------
//
int AUDs_GetVoiceStatus( int voice, int *stopped )
{
	ASSERT( stopped != NULL );
	if (SoundDisabled)
		return AUD_RETURN_SUCCESS;
	
	*stopped = !Mix_Playing(voice);
	return AUD_RETURN_SUCCESS;
}


// stop replay on channel voice -----------------------------------------------
//
int AUDs_StopVoice( int voice, int finishloop )
{
    if (SoundDisabled)
		return AUD_RETURN_SUCCESS;
	
    Mix_HaltChannel(voice);
	return AUD_RETURN_SUCCESS;
}


// speak text string ----------------------------------------------------------
//
int AUDs_SpeakString( char *speakstr )
{
	return 0;
}


// init sound stuff for game mode ---------------------------------------------
//
int AUDs_OpenGameSound()
{

	return 1;
}


// kill sound stuff for game mode ---------------------------------------------
//
int AUDs_CloseGameSound()
{
	return 1;
}


// play specified stream file -------------------------------------------------
//
int AUDs_PlayAudioStream( char *fname )
{

    if (SoundDisabled)
		return 1;
    
    ASSERT(fname!=NULL);
    
    if (Mix_PlayingMusic())
		Mix_HaltMusic();
	
    if (music != NULL) { 
         Mix_FreeMusic(music);
         FREEMEM(tmp_music_buffer);
         tmp_music_buffer = NULL;
         music = NULL;
    }
    // because SDL_RWFromFP doesn't work cross platform, we need
	// to get a little bit kinky
    
	// set up a buffer for the file
	
	size_t tmp_music_size = 0;
    
	// get the size of this sample
	tmp_music_size = SYS_GetFileLength(fname);
	if (tmp_music_size <= 0) {
		return 1; // return on error
	}
    
	// alloc space for the buffer
    if (tmp_music_buffer != NULL) {
        printf("ERROR: Shouldn't be this far without being cleaned up previously\n");
        FREEMEM(tmp_music_buffer);
        tmp_music_buffer = NULL;
    }
	
	tmp_music_buffer = (char *)ALLOCMEM(tmp_music_size + 1);
    
	// open the sample file
	FILE *musicfp = SYS_fopen(fname, "rb");
    if (musicfp == NULL)
		return 1;
    
	// read the file into the temp_sample_buffer
	int read_rc = SYS_fread((void *)tmp_music_buffer, 1, tmp_music_size, musicfp);
    
	if(read_rc <= 0) {
		MSGOUT("ERROR: Error reading music %s.", fname);
		return FALSE;
	}

	SDL_RWops *musicrwops = SDL_RWFromMem((void *)tmp_music_buffer, tmp_music_size);

    music = Mix_LoadMUS_RW(musicrwops, 1);
    //MSGOUT("      PlayAudioStream() with %s       ", fname);
    if (music == NULL) {
		printf("Mix_LoadMUS(\"%s\"): %s\n", fname, Mix_GetError());
		// this might be a critical error...
		FREEMEM(tmp_music_buffer);
		tmp_music_buffer = NULL;
		return 1;
    }
    
    Mix_VolumeMusic(128);
	
    if (music != NULL) {
    	Mix_PlayMusic(music, 0);
    }
	
    //Pick up your your toys when you finish
    SYS_fclose(musicfp);
   
    return 1;
}


// stop replay of stream file -------------------------------------------------
//
int AUDs_StopAudioStream()
{
//    Mix_HaltChannel(1);
    if (SoundDisabled)
		return 1;
	
    if (Mix_PlayingMusic())
		Mix_HaltMusic();
    
    if (music != NULL) {
		Mix_FreeMusic(music);
		music = NULL;
	}

	if (tmp_music_buffer != NULL) {
		FREEMEM(tmp_music_buffer);
		tmp_music_buffer = NULL;
	}

    return 1;
}


// precache sound effects specified in control file ---------------------------
//
void AUDs_LoadAllSoundEffects()
{
    if (SoundDisabled)
		return;
	
    for (int i = 0; i < NumLoadedSamples; i++)
        AUDs_LoadWaveFile(i);
}


// free sound effects that have been precached --------------------------------
//
void AUDs_FreeAllSoundEffects()
{
    if (SoundDisabled)
		return;
	
    for (int sid = 0; sid < NumLoadedSamples; sid++) {
		AUDs_FreeWaveFile( sid );
	}
}


// init sound stuff for game menu ---------------------------------------------
//
int AUDs_OpenMenuSound()
{
    if (SoundDisabled)
		return 1;
	if(!Op_Music) 
		return 1;
    in_game_mode = FALSE;
	
    if (Mix_PlayingMusic())
		Mix_HaltMusic();
	
    AUDs_PlayAudioStream( menu_stream_name );
	
    return 1;
}


// kill sound stuff for game menu ---------------------------------------------
//
int AUDs_CloseMenuSound()
{
    if (SoundDisabled)
      return 1;
	
    AUDs_StopAudioStream();
	
    return 1;
}


// init sound stuff for intro animation ---------------------------------------
//
int AUDs_OpenIntroSound()
{
	return 1;
}


// kill sound stuff for intro animation ---------------------------------------
//
int AUDs_CloseIntroSound()
{
	return 1;
}


// init sound stuff for credits scroller --------------------------------------
//
int AUDs_OpenCreditsSound()
{
	return 1;
}


// kill sound stuff for credits scroller --------------------------------------
//
int AUDs_CloseCreditsSound()
{
	return 1;
}


// set music/stream volume to maximum -----------------------------------------
//
void AUDs_MaxMusicVolume()
{
    Mix_VolumeMusic(MIX_MAX_VOLUME);
}


// set music/stream volume to zero --------------------------------------------
//
void AUDs_MinMusicVolume()
{
    Mix_VolumeMusic(0);
}


// decrease music/stream volume (no continuous fading!) -----------------------
//
void AUDs_FadeMusicVolume()
{
}


// maintain sound by keeping the output buffer filled -------------------------
//
void AUDs_MaintainSound()
{
	if ( SoundDisabled )
		return;

	if (!Mix_PlayingMusic() && in_game_mode == FALSE) {
		AUD_BackGroundPlayer_Start(FALSE);
		in_game_mode = TRUE;
	}
	
	if (!Mix_PlayingMusic())
		AUD_BackGroundPlayer_Start(FALSE);
}

void AUDs_MusicFinished()
{
	if (in_game_mode) {
		// just in case, not that it would matter.
		Mix_HaltMusic();
		
		if (music != NULL) {
			Mix_FreeMusic(music);
			music = NULL;
		}

		if (tmp_music_buffer != NULL) {
			FREEMEM(tmp_music_buffer);
			tmp_music_buffer = NULL;
		}
		
		// signal the BackGround Player we're ready for the next song.
		AUD_BackGroundPlayer_Start(FALSE);
	}
}

// init sound system and sound hardware ---------------------------------------
//
int AUDs_InitSoundSys()
{
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		MSGOUT("Failed to open audio.\n");
		SoundDisabled = TRUE;
		return FALSE;
	}
	
	if (Mix_OpenAudio(44100, AUDIO_S16, 2, 4096) == -1) {
		MSGOUT("Could not open mixer.\n%s\n",Mix_GetError());
		SoundDisabled = TRUE;
		return FALSE;
	}
	
	Mix_AllocateChannels(32);
	Mix_Volume(-1, MIX_MAX_VOLUME);

	AUDs_LoadAllSoundEffects();
	AUD_BackGroundPlayer_Init(1);
	
	//Mix_HookMusicFinished(AUDs_MusicFinished);
	
	MSGOUT("Sound Init Done\n");
	
	snd_init_done = TRUE;
	return TRUE;
}


// cancel sound system and sound hardware -------------------------------------
//
int AUDs_KillSoundSys()
{
	AUDs_FreeAllSoundEffects();
	AUD_BackGroundPlayer_Kill();
	
	Mix_CloseAudio();
	SDL_Quit();
	
	return TRUE;
}


