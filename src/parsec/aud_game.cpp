/*
 * PARSEC - High Level Audio Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:21 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1997-2000
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
#include "od_class.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "sys_subh.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "aud_game.h"

// proprietary module headers
#include "aud_supp.h"
#include "con_aux.h"
#include "e_events.h"
#include "obj_game.h"


// flags
//#define DISABLE_SELECT_SOUNDS
#define DIST_VOLUME
#define USE_SOUNDSOURCE_LIST
#define REALISTIC_VOLUME_DECREASE



// critical voice numbers -----------------------------------------------------
//
int SELECT_VOICE   = 2;
int STREAM_VOICE_L = 0;
int STREAM_VOICE_R = 1;


// default audio file names and paths -----------------------------------------
//
char menu_stream_name[ PATH_MAX + 1 ]		   = "menu.ogg";
char intro_stream_name[ PATH_MAX + 1 ]		   = "jingle.ogg";
char console_audio_stream_path[ PATH_MAX + 1 ] = ".\\music\\";


// module local globals -------------------------------------------------------
//
int 						g_nLightningsActive = 0;
struct DistVolumeInfo_s*	g_pDistVolumeInfo[  NUMCHANNELS ];	// distance volume infos for each channel


// global volume --------------------------------------------------------------
//
int GlobalVolume = AUD_MAX_VOLUME;


// ----------------------------------------------------------------------------
//
#define JIM_COMMENT_VOICE				GUN_SELECT_VOICE


// sample-channel relations for general ingame sounds -------------------------
//
static sample_channel_info_s sample_channel_infos[] = {

	{ SND_NAME_DROPMINE,					NULL,					-1,		STD_VOICE,				TRUE	},
	{ SND_NAME_DAZZLING_COLLECTED,			SND_NAME_ENERGYEXTRA,	-1,		STD_VOICE,				TRUE	},
	{ SND_NAME_THIEF_COLLECTED,				SND_NAME_ENERGYEXTRA,	-1,		STD_VOICE,				TRUE	},
	{ SND_NAME_AFTERBURNER_COLLECTED,		SND_NAME_ENERGYEXTRA,	-1,		STD_VOICE,				TRUE	},
	{ SND_NAME_INVISIBILITY_COLLECTED,		SND_NAME_ENERGYEXTRA,	-1,		STD_VOICE,				TRUE	},
	{ SND_NAME_INVUL_COLLECTED,				SND_NAME_ENERGYEXTRA,	-1,		STD_VOICE,				TRUE	},
	{ SND_NAME_DUMB_MISSILE_COLLECTED,		SND_NAME_ENERGYEXTRA,	-1,		STD_VOICE,				TRUE	},
	{ SND_NAME_GUIDE_MISSILE_COLLECTED,		SND_NAME_ENERGYEXTRA,	-1,		STD_VOICE,				TRUE	},
	{ SND_NAME_MINE_COLLECTED,				SND_NAME_ENERGYEXTRA,	-1,		STD_VOICE,				TRUE	},
	{ SND_NAME_ENERGYEXTRA,					NULL,					-1,		ENERGYBOOSTED_VOICE,	FALSE	},
	{ SND_NAME_ENERGYEXTRA,					NULL,					-1,		STD_VOICE,				TRUE	},
	{ SND_NAME_DAMAGE_REPAIRED,				NULL,					-1,		STD_VOICE,				TRUE	},
	{ SND_NAME_MAXED_OUT,					NULL,					-1,		ENERGYBOOSTED_VOICE,	TRUE	},
	{ SND_NAME_MAXED_OUT,					NULL,					-1,		STD_VOICE,				TRUE	},
	{ SND_NAME_SELECT1,						NULL,					-1,		SELECT_VOICE,			TRUE	},
	{ SND_NAME_SELECT2,						NULL,					-1,		SELECT_VOICE, 			TRUE	},
	{ SND_NAME_SHIPDESTROYED,				NULL,					-1,		DESTROYED_VOICE,		TRUE	},
	{ SND_NAME_EXPLOSION,					NULL,					-1,		EXPLOSION_VOICE,		TRUE	},
	{ SND_NAME_LOWENERGY,					NULL,					-1,		LOWENERGY_VOICE,		FALSE	},
	{ SND_NAME_TARGETLOCKED,				NULL,					-1,		LOCKED_VOICE,			TRUE	},
	{ SND_NAME_BUTTONSLIDED,				NULL,					-1,		STD_VOICE,				FALSE	},
	{ SND_NAME_SCV_SLIDE_IN,				NULL,					-1,		GUN_SELECT_VOICE,		TRUE	},
	{ SND_NAME_SCV_SLIDE_OUT,				NULL,					-1,		GUN_SELECT_VOICE,		TRUE	},
	{ SND_NAME_MINEHIT,						NULL,					-1,		EXPLOSION_VOICE,		TRUE	},
	{ SND_NAME_LASER1,						NULL,					-1,		LASER1_VOICE,			TRUE	},
	{ SND_NAME_LASER2,						NULL,					-1,		LASER1_VOICE,			TRUE	},
	{ SND_NAME_LASER3,						NULL,					-1,		LASER1_VOICE,			TRUE	},
	{ SND_NAME_MISSILE1,					NULL,					-1,		MISSILE1_VOICE,			TRUE	},
	{ SND_NAME_MISSILE2,					NULL,					-1,		MISSILE2_VOICE,			TRUE	},
	{ SND_NAME_HELIX,						NULL,					-1,		HELIX_VOICE,			TRUE	},
	{ SND_NAME_TRACKING,					NULL,					-1,		TRACKING_VOICE,			TRUE	},
	{ SND_NAME_TRACKINGBEEP,				NULL,					-1,		TRACKBEEP_VOICE,		TRUE	},
	{ SND_NAME_NEWPLAYER,					NULL,					-1,		NEWPLAYERJOINED_VOICE,	TRUE	},
	{ SND_NAME_OPTIONS_SLIDE_IN,			NULL,					-1,		GUN_SELECT_VOICE,		TRUE	},
	{ SND_NAME_OPTIONS_SLIDE_OUT,			NULL,					-1,		GUN_SELECT_VOICE,		TRUE	},
	{ SND_NAME_SELECT_LASER1,				NULL,					-1,		GUN_SELECT_VOICE,		TRUE	},
	{ SND_NAME_SELECT_LASER2,				NULL,					-1,		GUN_SELECT_VOICE,		TRUE	},
	{ SND_NAME_SELECT_LASER3,				NULL,					-1,		GUN_SELECT_VOICE,		TRUE	},
	{ SND_NAME_SELECT_LASER4,				NULL,					-1,		GUN_SELECT_VOICE,		TRUE	},
	{ SND_NAME_SELECT_LASER5,				NULL,					-1,		GUN_SELECT_VOICE,		TRUE	},
	{ SND_NAME_SELECT_MISSILE1,				NULL,					-1,		MISS_SELECT_VOICE,		TRUE	},
	{ SND_NAME_SELECT_MISSILE2,				NULL,					-1,		MISS_SELECT_VOICE,		TRUE	},
	{ SND_NAME_SELECT_MISSILE3,				NULL,					-1,		MISS_SELECT_VOICE,		TRUE	},
	{ SND_NAME_SELECT_MISSILE4,				NULL,					-1,		MISS_SELECT_VOICE,		TRUE	},
	{ SND_NAME_COUNTDOWN1,					NULL,					-1,		COUNTDOWN_VOICE,		FALSE	},
	{ SND_NAME_COUNTDOWN2,					NULL,					-1,		COUNTDOWN_VOICE,		FALSE	},
	{ SND_NAME_COUNTDOWN3,					NULL,					-1,		COUNTDOWN_VOICE,		FALSE	},
	{ SND_NAME_COUNTDOWN4,					NULL,					-1,		COUNTDOWN_VOICE,		FALSE	},
	{ SND_NAME_COUNTDOWN5,					NULL,					-1,		COUNTDOWN_VOICE,		FALSE	},
	{ SND_NAME_SWITCH_GUN,					NULL,					-1,		SELECT_VOICE,			TRUE	},
	{ SND_NAME_SWITCH_MISSILE,				NULL,					-1,		SELECT_VOICE, 			TRUE	},
	{ SND_NAME_THRUST,						NULL,					-1,		THRUST_VOICE,			FALSE	},
	{ SND_NAME_SLIDETHRUST,					NULL,					-1,		SLIDETHRUST_VOICE,		FALSE	},
	{ SND_NAME_INCOMING,					NULL,					-1,		TRACKBEEP_VOICE,		FALSE	},
	{ SND_NAME_BOOTING,						NULL,					-1,		STD_VOICE,				FALSE	},
	{ SND_NAME_LASERBEAM,					NULL,					-1,		LASER1_VOICE,			TRUE	},
	{ SND_NAME_ENEMYSHIELD_HIT,				NULL,					-1,		ENEMY_HIT_VOICE,		TRUE	},
	{ SND_NAME_ENEMYHULL_HIT,				NULL,					-1,		ENEMY_HIT_VOICE,		TRUE	},
	{ SND_NAME_PLAYERSHIELD_HIT,			NULL,					-1,		ENEMY_HIT_VOICE,		TRUE	},
	{ SND_NAME_PLAYERHULL_HIT,				NULL,					-1,		ENEMY_HIT_VOICE,		TRUE	},
	{ SND_NAME_LOW_SHIELDS,					NULL,					-1,		LOWENERGY_VOICE,		TRUE	},
	{ SND_NAME_ONE_KILL_LEFT,				NULL,					-1,		ANNOUNCE_VOICE,			TRUE	},
	{ SND_NAME_TWO_KILLS_LEFT,				NULL,					-1,		ANNOUNCE_VOICE,			TRUE	},
	{ SND_NAME_THREE_KILLS_LEFT,			NULL,					-1,		ANNOUNCE_VOICE,			TRUE	},
	{ SND_NAME_JIM_COMMENT_AFRAID_1,		NULL,					-1,		JIM_COMMENT_VOICE,		FALSE	},
	{ SND_NAME_JIM_COMMENT_AFRAID_2,		NULL,					-1,		JIM_COMMENT_VOICE,		FALSE	},
	{ SND_NAME_JIM_COMMENT_ANGRY_1,			NULL,					-1,		JIM_COMMENT_VOICE,		FALSE	},
	{ SND_NAME_JIM_COMMENT_ANGRY_2,			NULL,					-1,		JIM_COMMENT_VOICE,		FALSE	},
	{ SND_NAME_JIM_COMMENT_WEAPON_1,		NULL,					-1,		JIM_COMMENT_VOICE,		FALSE	},
	{ SND_NAME_JIM_COMMENT_WEAPON_2,		NULL,					-1,		JIM_COMMENT_VOICE,		FALSE	},
	{ SND_NAME_JIM_COMMENT_WEAPON_3,		NULL,					-1,		JIM_COMMENT_VOICE,		FALSE	},
	{ SND_NAME_JIM_COMMENT_WEAPON_4,		NULL,					-1,		JIM_COMMENT_VOICE,		FALSE	},
	{ SND_NAME_JIM_COMMENT_KILLED_1,		NULL,					-1,		JIM_COMMENT_VOICE,		FALSE	},
	{ SND_NAME_JIM_COMMENT_KILLED_2,		NULL,					-1,		JIM_COMMENT_VOICE,		FALSE	},
	{ SND_NAME_JIM_COMMENT_EAT_THIS_1,		NULL,					-1,		JIM_COMMENT_VOICE,		FALSE	},
	{ SND_NAME_JIM_COMMENT_EAT_THIS_2,		NULL,					-1,		JIM_COMMENT_VOICE,		FALSE	},
	{ SND_NAME_JIM_COMMENT_ENERGY,			NULL,					-1,		JIM_COMMENT_VOICE,		FALSE	},
	{ SND_NAME_JIM_COMMENT_BORED_1,			NULL,					-1,		JIM_COMMENT_VOICE,		FALSE	},
	{ SND_NAME_JIM_COMMENT_BORED_2,			NULL,					-1,		JIM_COMMENT_VOICE,		FALSE	},
	{ SND_NAME_DECOY_COLLECTED,				NULL,					-1,		MISS_SELECT_VOICE,		TRUE	},
	{ SND_NAME_LASER_UPGRADE_COLLECTED,		NULL,					-1,		GUN_SELECT_VOICE,		TRUE	},
	{ SND_NAME_PHOTON_CANNON_COLLECTED,		NULL,					-1,		GUN_SELECT_VOICE,		TRUE	},
	{ SND_NAME_SWARM_MISSILES_COLLECTED,	NULL,					-1,		MISS_SELECT_VOICE,		TRUE	},
	{ SND_NAME_SKILL_AMAZING,				NULL,					-1,		JIM_COMMENT_VOICE,		TRUE	},
	{ SND_NAME_SKILL_BRILLIANT,				NULL,					-1,		JIM_COMMENT_VOICE,		TRUE	},
	{ SND_NAME_AFTERBURNER_ACTIVATE,		NULL,					-1,		AFTERBURNER_VOICE,		TRUE	},
	{ SND_NAME_AFTERBURNER_DEACTIVATE,		NULL,					-1,		AFTERBURNER_VOICE,		TRUE	},
	{ SND_NAME_PHOTON_LOADING,				NULL,					-1,		PHOTON_CANNON_VOICE,	TRUE	},
	{ SND_NAME_PHOTON_FIRING,				NULL,					-1,		PHOTON_CANNON_VOICE,	TRUE	},
	{ SND_NAME_SWARM_MISSILES,				NULL,					-1,		SWARM_MISSILE_VOICE,	TRUE	},
	{ SND_NAME_YOU_DID_IT,					NULL,					-1,		ANNOUNCE_VOICE,			TRUE	},
	{ SND_NAME_YOU_HAVE_LOST,				NULL,					-1,		ANNOUNCE_VOICE,			TRUE	},
	{ SND_NAME_MINE_DETECTOR,				NULL,					-1,		TRACKBEEP_VOICE,		TRUE	},
	{ SND_NAME_EMP_1,	 	 				NULL,					-1,		EMP_VOICE,				TRUE	},
	{ SND_NAME_EMP_2,	 	 				NULL,					-1,		EMP_VOICE,				TRUE	},
	{ SND_NAME_EMP_3,	 	 				NULL,					-1,		EMP_VOICE,				TRUE	},
	{ SND_NAME_TELEPORTER,					NULL,					-1,		TRACKBEEP_VOICE,		TRUE	},


};

enum {

	SCI_DROPMINE,
	SCI_DAZZLING_COLLECTED,
	SCI_THIEF_COLLECTED,
	SCI_AFTERBURNER_COLLECTED,
	SCI_INVISIBILITY_COLLECTED,
	SCI_INVUL_COLLECTED,
	SCI_DUMB_MISSILE_COLLECTED,
	SCI_GUIDE_MISSILE_COLLECTED,
	SCI_MINE_COLLECTED,
	SCI_ENERGYEXTRA,
	SCI_ENERGYEXTRA1,
	SCI_DAMAGE_REPAIRED,
	SCI_ENERGY_MAXED_OUT,
	SCI_MAXED_OUT,
	SCI_SELECT1,
	SCI_SELECT2,
	SCI_SHIPDESTROYED,
	SCI_EXPLOSION,
	SCI_LOWENERGY,
	SCI_TARGETLOCKED,
	SCI_BUTTONSLIDED,
	SCI_SCV_SLIDE_IN,
	SCI_SCV_SLIDE_OUT,
	SCI_MINEHIT,
	SCI_LASER1,
	SCI_LASER2,
	SCI_LASER3,
	SCI_MISSILE1,
	SCI_MISSILE2,
	SCI_HELIX,
	SCI_TRACKING,
	SCI_TRACKINGBEEP,
	SCI_NEWPLAYER,
	SCI_OPTIONS_SLIDE_IN,
	SCI_OPTIONS_SLIDE_OUT,
	SCI_SELECT_LASER1,
	SCI_SELECT_LASER2,
	SCI_SELECT_LASER3,
	SCI_SELECT_LASER4,
	SCI_SELECT_LASER5,
	SCI_SELECT_MISSILE1,
	SCI_SELECT_MISSILE2,
	SCI_SELECT_MISSILE3,
	SCI_SELECT_MISSILE4,
	SCI_COUNTDOWN1,
	SCI_COUNTDOWN2,
	SCI_COUNTDOWN3,
	SCI_COUNTDOWN4,
	SCI_COUNTDOWN5,
	SCI_SWITCH_GUN,
	SCI_SWITCH_MISSILE,
	SCI_THRUST,
	SCI_SLIDETHRUST,
	SCI_INCOMING,
	SCI_BOOTING,
	SCI_LASERBEAM,
	SCI_ENEMYSHIELD_HIT,
	SCI_ENEMYHULL_HIT,
	SCI_PLAYERSHIELD_HIT,
	SCI_PLAYERHULL_HIT,
	SCI_LOW_SHIELDS,
	SCI_ONE_KILL_LEFT,
	SCI_TWO_KILLS_LEFT,
	SCI_THREE_KILLS_LEFT,
	SCI_JIM_COMMENT_AFRAID_1,
	SCI_JIM_COMMENT_AFRAID_2,
	SCI_JIM_COMMENT_ANGRY_1,
	SCI_JIM_COMMENT_ANGRY_2,
	SCI_JIM_COMMENT_WEAPON_1,
	SCI_JIM_COMMENT_WEAPON_2,
	SCI_JIM_COMMENT_WEAPON_3,
	SCI_JIM_COMMENT_WEAPON_4,
	SCI_JIM_COMMENT_KILLED_1,
	SCI_JIM_COMMENT_KILLED_2,
	SCI_JIM_COMMENT_EAT_THIS_1,
	SCI_JIM_COMMENT_EAT_THIS_2,
	SCI_JIM_COMMENT_ENERGY,
	SCI_JIM_COMMENT_BORED_1,
	SCI_JIM_COMMENT_BORED_2,
	SCI_DECOY_COLLECTED,
	SCI_LASER_UPGRADE_COLLECTED,
	SCI_PHOTON_CANNON_COLLECTED,
	SCI_SWARM_MISSILES_COLLECTED,
	SCI_SKILL_AMAZING,
	SCI_SKILL_BRILLIANT,
	SCI_AFTERBURNER_ACTIVATE,
	SCI_AFTERBURNER_DEACTIVATE,
	SCI_PHOTON_LOADING,
	SCI_PHOTON_FIRING,
	SCI_SWARM_MISSILES,
	SCI_YOU_DID_IT,
	SCI_YOU_HAVE_LOST,
	SCI_MINE_DETECTOR,
	SCI_EMP1,
	SCI_EMP2,
	SCI_EMP3,
	SCI_TELEPORTER,

	SCI_NUM_SAMPLE_CHANNEL_INFOS
};

int NumSampleChannelInfos = SCI_NUM_SAMPLE_CHANNEL_INFOS;


// refframe delays for playing the specified sound ----------------------------
//
#define MISSILE_SELECTED_SOUND_DELAY		300
#define LASER_SELECTED_SOUND_DELAY			300
#define COUNTDOWN_SOUND_DELAY				300
#define SHIPDESTROYED_SOUND_DELAY			800
#define EXPLOSION_SOUND_DELAY				400
#define KILLSLEFT_SOUND_DELAY				500
#define THRUST_SOUND_LIFETIME				60
#define THRUST_FADEOUT_FRAMES				300
#define DISTANCE_UPDATE_FRAMES				( FRAME_MEASURE_TIMEBASE / 10 )
#define MAX_DISTANCE_VISIBLE				4096
#define TARGET_LOCKED_REPEAT_RATE			FRAME_MEASURE_TIMEBASE * 3


// module local functions -----------------------------------------------------
//
PRIVATE		int AUDm_GetVolumeDistBased( DistVolumeInfo_s* pDistVolumeInfo );
PRIVATE		int AUDm_AddChannelSoundSource( int nChannel, DistVolumeInfo_s* pDistVolumeInfo );
PRIVATE		int AUDm_RemoveChannelSoundSource( int nChannel, GenObject* originator  );
PRIVATE		int AUDm_WalkChannelSoundSources(  void *param );


// play sound effect "helix device" -------------------------------------------
//
PRIVATE
void AUDm_HelixDevice()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_DAZZLING_COLLECTED ] );
}


// play sound effect "lightning device" ---------------------------------------
//
PRIVATE
void AUDm_LightningDevice()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_THIEF_COLLECTED ] );
}


// play sound effect "photon device" ------------------------------------------
//
PRIVATE
void AUDm_PhotonDevice()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_PHOTON_CANNON_COLLECTED ] );
}


// play sound effect "afterburner device" -------------------------------------
//
PRIVATE
void AUDm_AfterBurnerDevice()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_AFTERBURNER_COLLECTED ] );
}


// play sound effect "invisibility device" ------------------------------------
//
PRIVATE
void AUDm_InvisibilityDevice()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_INVISIBILITY_COLLECTED ] );
}


// play sound effect "invulnerability device" ---------------------------------
//
PRIVATE
void AUDm_InvulnerabilityDevice()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_INVUL_COLLECTED ] );
}


// play sound effect "holo decoy device" --------------------------------------
//
PRIVATE
void AUDm_HoloDecoyDevice()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_DECOY_COLLECTED ] );
}


// play sound effect "laser upgrade" ------------------------------------------
//
PRIVATE
void AUDm_LaserUpgradeDevice()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_LASER_UPGRADE_COLLECTED ] );
}


// play sound effect "dumb missiles" ------------------------------------------
//
PRIVATE
void AUDm_DumbMissiles()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_DUMB_MISSILE_COLLECTED ] );
}


// play sound effect "guided missiles" ----------------------------------------
//
PRIVATE
void AUDm_GuidedMissiles()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_GUIDE_MISSILE_COLLECTED ] );
}


// play sound effect "proximity mines" ----------------------------------------
//
PRIVATE
void AUDm_ProximityMine()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_MINE_COLLECTED ] );
}


// play sound effect "swarm missiles" -----------------------------------------
//
PRIVATE
void AUDm_SwarmMissiles()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_SWARM_MISSILES_COLLECTED ] );
}


// play sound effect "extra collected" ----------------------------------------
//
int AUD_ExtraCollected( int type )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	switch ( type ) {

		// helix cannon
		case HELIX_DEVICE:
			AUDm_HelixDevice();
			AUD_JimCommenting( JIM_COMMENT_WEAPON );
			break;

		// lightning device
		case LIGHTNING_DEVICE:
			AUDm_LightningDevice();
			AUD_JimCommenting( JIM_COMMENT_WEAPON );
			break;

		// afterburner device
		case AFTERBURNER_DEVICE:
			AUDm_AfterBurnerDevice();
			break;

		// cloak (invisibility) device
		case INVISIBILITY_DEVICE:
			AUDm_InvisibilityDevice();
			break;

		// photon cannon
		case PHOTON_DEVICE:
			AUDm_PhotonDevice();
			AUD_JimCommenting( JIM_COMMENT_WEAPON );
			break;

		// invulnerability device
		case INVULNERABILITY_DEVICE:
			AUDm_InvulnerabilityDevice();
			break;

		// holo decoy device
		case DECOY_DEVICE:
			AUDm_HoloDecoyDevice();
			break;

		// laser upgrade devices
		case LASER_UPGRADE_1_DEVICE:
		case LASER_UPGRADE_2_DEVICE:
			AUDm_LaserUpgradeDevice();
			break;

		// dumb missile package
		case MISSILE1TYPE:
			AUDm_DumbMissiles();
			break;

		// guide missile package
		case MISSILE4TYPE:
			AUDm_GuidedMissiles();
			break;

		// swarm missiles package
		case MISSILE5TYPE:
			AUDm_SwarmMissiles();
			break;

		// proximity mine package
		case MINE1TYPE:
			AUDm_ProximityMine();
			break;

		default:
			break;
	}

	return TRUE;
}


// play sound effect "energy boosted" -----------------------------------------
//
int AUD_EnergyBoosted()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_ENERGYEXTRA ] );
	return TRUE;
}


// play sound effect "damage repaired" ----------------------------------------
//
int AUD_DamageRepaired()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_DAMAGE_REPAIRED ] );
	return TRUE;
}


// play sound effect "energy maxed out" ---------------------------------------
//
PRIVATE
void AUDm_EnergyMaxedOut()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_ENERGY_MAXED_OUT ] );
}


// play sound effect "something maxed out" ------------------------------------
//
int AUD_MaxedOut( int type )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	switch( type ) {

		case ENERGY_EXTRA_CLASS:
			AUDm_EnergyMaxedOut();
			return TRUE;

		default:
			AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_MAXED_OUT ] );
			break;
	}

	return TRUE;
}


// play sound effect "select click 1" -----------------------------------------
//
int AUD_Select1()
{

#ifndef DISABLE_SELECT_SOUNDS

	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_SELECT1 ] );

#endif

	return TRUE;
}


// play sound effect "select click 2" -----------------------------------------
//
int AUD_Select2()
{

#ifndef DISABLE_SELECT_SOUNDS

	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_SELECT2 ] );

#endif

	return TRUE;
}


// schedule event for delayed explosion ---------------------------------------
//
PRIVATE
int AUD_Explosion_Event( void* param )
{
	int nVolume = (long) param;
	ASSERT( ( nVolume >= AUD_MIN_VOLUME ) && ( nVolume <= AUD_MAX_VOLUME ) );

	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	AUD_FETCHSAMPLE( sample_channel_infos[ SCI_EXPLOSION ].samplename );

	if ( fetch_wave != NULL ) {

		// set the volume
		fetch_params.volume = nVolume;
		fetch_params.flags |= SOUNDPARAMS_VOLUME;

		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_PlayVoiceBuffer( sample_channel_infos[ SCI_EXPLOSION ].channel,
				fetch_wave, &fetch_params );

		return TRUE;

	} else {

		return FALSE;
	}
}


// play sound effect "ship destroyed" -----------------------------------------
//
int AUD_ShipDestroyed( GenObject* objectpo )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

#define DISTANCE_BASED_EXPLOSION

#ifdef DISTANCE_BASED_EXPLOSION

	AUD_FETCHSAMPLE( sample_channel_infos[ SCI_EXPLOSION ].samplename );

	int nVolume;

	// check whether proper originator is set
	if ( objectpo != NULL ) {

		// first we calculate the volume of the explosion based on the distance to the local ship
		DistVolumeInfo_s DVI;
		DVI.nMaxVolume		= ( fetch_params.flags & SOUNDPARAMS_VOLUME ) ? fetch_params.volume : AUD_MAX_VOLUME;
		DVI.nMinVolume		= AUD_MIN_VOLUME;
		DVI.fMaxVolumeDist	= FLOAT_TO_GEOMV( 700  );       			// distance for max. volume
		DVI.fMinVolumeDist	= FLOAT_TO_GEOMV( MAX_DISTANCE_VISIBLE );	// distance for min. volume
		DVI.fRolloffscale	= 1.0f;
		DVI.pListener		= MyShip;
		DVI.pOriginator		= objectpo;

		// get the volume
		nVolume = AUDm_GetVolumeDistBased( &DVI );

	} else {

		nVolume = ( fetch_params.flags & SOUNDPARAMS_VOLUME ) ?
			fetch_params.volume : AUD_MAX_VOLUME;
	}

	// now we schedule the event for playing the explosion sample
	event_s* eventExplosion = (event_s*) ALLOCMEM( sizeof ( event_s ) );

	eventExplosion->type			= EVT_TYPE_SND_EXPLOSION_DELAY;
	eventExplosion->callback		= (event_callback_t) AUD_Explosion_Event;
	eventExplosion->callback_params	= (void*) nVolume;
	eventExplosion->refframe_delay	= EXPLOSION_SOUND_DELAY;
	eventExplosion->flags			= EVENT_PARAM_AUTOFREE    |
							  	      EVENT_PARAM_AUTOTRIGGER |
							  		  EVENT_PARAM_ONE_PER_TYPE;
	EVT_AddEvent( eventExplosion );

#else // DISTANCE_BASED_EXPLOSION

	//FIXME: the explosion sample should be played by an event
	//AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_EXPLOSION ] );

	event_s* eventExplosion = (event_s*) ALLOCMEM( sizeof ( event_s ) );

	eventExplosion->type			= EVT_TYPE_SND_EXPLOSION_DELAY;
	eventExplosion->callback		= (event_callback_t) AUD_PlaySampleOnChannel;
	eventExplosion->callback_params	= &sample_channel_infos[ SCI_EXPLOSION ];
	eventExplosion->refframe_delay	= EXPLOSION_SOUND_DELAY;
	eventExplosion->flags			= EVENT_PARAM_AUTOFREE    |
							  	      EVENT_PARAM_AUTOTRIGGER |
							  		  EVENT_PARAM_ONE_PER_TYPE;
	EVT_AddEvent( eventExplosion );

#endif // DISTANCE_BASED_EXPLOSION

	event_s* eventShipDestroyed = (event_s*) ALLOCMEM( sizeof ( event_s ) );

	eventShipDestroyed->type			= EVT_TYPE_SND_COUNTDOWN;
	eventShipDestroyed->callback		= (event_callback_t) AUD_PlaySampleOnChannel;
	eventShipDestroyed->callback_params	= &sample_channel_infos[ SCI_SHIPDESTROYED ];
	eventShipDestroyed->refframe_delay	= SHIPDESTROYED_SOUND_DELAY;
	eventShipDestroyed->flags			= EVENT_PARAM_AUTOFREE    |
							  			  EVENT_PARAM_AUTOTRIGGER |
							  			  EVENT_PARAM_ONE_PER_TYPE;
	EVT_AddEvent( eventShipDestroyed );

	return TRUE;
}


// play sound effect "low energy" ---------------------------------------------
//
int AUD_LowEnergy()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_LOWENERGY ] );
	return TRUE;
}


// refframe of last target lock -----------------------------------------------
//
static refframe_t last_target_lock_ref = REFFRAME_INVALID;


// play sound effect "target locked" ------------------------------------------
//
int AUD_Locked()
{
	refframe_t curref = SYSs_GetRefFrameCount();

	// only play the "target locked" sample when enough time passed between play requests
	if ( last_target_lock_ref != REFFRAME_INVALID ) {
		if ( ( curref - last_target_lock_ref ) < TARGET_LOCKED_REPEAT_RATE ) {
			return TRUE;
		}
	}

	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_TARGETLOCKED ] );
//	MSGOUT( "play target locked at %d ( diff: %d )", curref, curref - last_target_lock_ref );
	last_target_lock_ref = curref;

	return TRUE;
}


// play sound effect "menu buttin slided in" ----------------------------------
//
int AUD_ButtonSlided()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_BUTTONSLIDED ] );
	return TRUE;
}


// play sound effect "spacecraft viewer slided in" ----------------------------
//
int AUD_SpaceCraftViewerIn()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_SCV_SLIDE_IN ] );
	return TRUE;
}


// play sound effect "spacecraft viewer slided out" ----------------------------
//
int AUD_SpaceCraftViewerOut()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_SCV_SLIDE_OUT ] );
	return TRUE;
}


// play sound effect "mine hit" -----------------------------------------------
//
int	AUD_MineCollision()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_MINEHIT ] );
	return TRUE;
}


// play sound effect "player has been killed" ---------------------------------
//
int AUD_PlayerKilled()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_MINEHIT ] );
	return TRUE;
}


// play sound effect "laser fired" --------------------------------------------
//
int AUD_Laser( GenObject* objectpo )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	// determine actual sound
	int sci = SCI_LASER1;
	dword objtype = objectpo ? objectpo->ObjectType : LASER1TYPE;

	audiowave_t   fetchwave = NULL;
	SoundParams_s fetchparams;

	switch ( objtype ) {

		case LASER1TYPE:
			{
			sci = SCI_LASER1;
			AUD_FETCHSAMPLE( sample_channel_infos[ sci ].samplename );
			fetchwave	= fetch_wave;
			fetchparams	= fetch_params;
			}
			break;

		case LASER2TYPE:
			{
			sci = SCI_LASER2;
			AUD_FETCHSAMPLE( sample_channel_infos[ sci ].samplename );
			fetchwave	= fetch_wave;
			fetchparams	= fetch_params;
			}
			break;

		case LASER3TYPE:
			{
			sci = SCI_LASER3;
			AUD_FETCHSAMPLE( sample_channel_infos[ sci ].samplename );
			fetchwave	= fetch_wave;
			fetchparams	= fetch_params;
			}
			break;
	}

#ifdef DIST_VOLUME

	if ( fetchwave != NULL ) {

		if ( ( objectpo != NULL ) && ( objectpo != MyShip ) )  {

			// set the info for changing the volume based on the distance
			DistVolumeInfo_s DVI;
			DVI.nMaxVolume		= ( fetchparams.flags & SOUNDPARAMS_VOLUME ) ? fetchparams.volume : AUD_MAX_VOLUME;
			DVI.nMinVolume		= AUD_MIN_VOLUME;
			DVI.fMaxVolumeDist	= FLOAT_TO_GEOMV( 300  );				   // distance for max. volume
			DVI.fMinVolumeDist	= FLOAT_TO_GEOMV( MAX_DISTANCE_VISIBLE );  // distance for min. volume
			DVI.fRolloffscale	= 1.0f;
			DVI.pListener		= MyShip;
			DVI.pOriginator		= objectpo;

			// get the volume
			fetchparams.volume = AUDm_GetVolumeDistBased( &DVI );
			fetchparams.flags |= SOUNDPARAMS_VOLUME;
		}

		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_PlayVoiceBuffer( sample_channel_infos[ sci ].channel, fetchwave, &fetchparams );
	}

	return TRUE;

#else

	AUD_PlaySampleOnChannel( &sample_channel_infos[ sci ] );
	return TRUE;

#endif // DIST_VOLUME

}


// play sound effect "missile launched" ---------------------------------------
//
int AUD_Missile( GenObject *objectpo )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	// determine actual sound
	int sci = SCI_MISSILE1;
	dword objtype = objectpo ? objectpo->ObjectType : MISSILE1TYPE;

	audiowave_t   fetchwave = NULL;
	SoundParams_s fetchparams;

	switch ( objtype ) {

		case MISSILE1TYPE:
			{
			sci = SCI_MISSILE1;
			AUD_FETCHSAMPLE( sample_channel_infos[ sci ].samplename );
			fetchwave	= fetch_wave;
			fetchparams	= fetch_params;
			}
			break;

		case MISSILE4TYPE:
			{
			sci = SCI_MISSILE2;
			AUD_FETCHSAMPLE( sample_channel_infos[ sci ].samplename );
			fetchwave	= fetch_wave;
			fetchparams	= fetch_params;
			}
			break;
	}

#ifdef DIST_VOLUME

	if ( fetchwave != NULL ) {

		if ( ( objectpo != NULL ) && ( objectpo != MyShip ) )  {

			// set the info for changing the volume based on the distance
			DistVolumeInfo_s DVI;
			DVI.nMaxVolume		= ( fetchparams.flags & SOUNDPARAMS_VOLUME ) ? fetchparams.volume : AUD_MAX_VOLUME;
			DVI.nMinVolume		= AUD_MIN_VOLUME;
			DVI.fMaxVolumeDist	= FLOAT_TO_GEOMV( 300  );			      // distance for max. volume
			DVI.fMinVolumeDist	= FLOAT_TO_GEOMV( MAX_DISTANCE_VISIBLE ); // distance for min. volume
			DVI.fRolloffscale	= 1.0f;
			DVI.pListener		= MyShip;
			DVI.pOriginator		= objectpo;

			// get the volume
			fetchparams.volume = AUDm_GetVolumeDistBased( &DVI );
			fetchparams.flags |= SOUNDPARAMS_VOLUME;
		}

		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_PlayVoiceBuffer( sample_channel_infos[ sci ].channel, fetchwave, &fetchparams );
	}

	return TRUE;

#else

	AUD_PlaySampleOnChannel( &sample_channel_infos[ sci ] );
	return TRUE;

#endif // DIST_VOLUME

}


// play sound effect "drop mine" ----------------------------------------------
//
int AUD_Mine( GenObject *objectpo )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	int sci = SCI_DROPMINE;

#ifdef DIST_VOLUME

	if ( ( objectpo != NULL ) && ( objectpo != MyShip ) ) {

		AUD_FETCHSAMPLE( sample_channel_infos[ sci ].samplename );

		if ( fetch_wave != NULL ) {

			// set the info for changing the volume based on the distance
			DistVolumeInfo_s DVI;
			DVI.nMaxVolume		= ( fetch_params.flags & SOUNDPARAMS_VOLUME ) ? fetch_params.volume : AUD_MAX_VOLUME;
			DVI.nMinVolume		= AUD_MIN_VOLUME;
			DVI.fMaxVolumeDist	= FLOAT_TO_GEOMV( 300  );				   // distance for max. volume
			DVI.fMinVolumeDist	= FLOAT_TO_GEOMV( MAX_DISTANCE_VISIBLE );  // distance for min. volume
			DVI.fRolloffscale	= 1.0f;
			DVI.pListener		= MyShip;
			DVI.pOriginator		= objectpo;

			// get the volume
			fetch_params.volume = AUDm_GetVolumeDistBased( &DVI );
			fetch_params.flags |= SOUNDPARAMS_VOLUME;
		}

		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_PlayVoiceBuffer( sample_channel_infos[ sci ].channel, fetch_wave, &fetch_params );

	} else {
		AUD_PlaySampleOnChannel( &sample_channel_infos[ sci ] );
	}

	return TRUE;

#else

	AUD_PlaySampleOnChannel( &sample_channel_infos[ sci ] );
	return TRUE;

#endif // DIST_VOLUME

}


// play sound effect "energy extra" -------------------------------------------
//
int AUD_EnergyExtra()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_ENERGYEXTRA1 ] );
	return TRUE;
}


#ifdef DIST_VOLUME

// calculate the volume based on the distance of two object -------------------
PRIVATE
int AUDm_GetVolumeDistBased( DistVolumeInfo_s* pDistVolumeInfo )
{
	// trivial case, when listener is the originator
	if ( pDistVolumeInfo->pListener == pDistVolumeInfo->pOriginator ) {
		return pDistVolumeInfo->nMaxVolume;
	}

	Vector3 posListener, posOriginator;
	Vector3 dist;

	ASSERT( pDistVolumeInfo != NULL );
	ASSERT( pDistVolumeInfo->pOriginator != NULL );

	FetchTVector( pDistVolumeInfo->pListener->ObjPosition,   &posListener   );
	FetchTVector( pDistVolumeInfo->pOriginator->ObjPosition, &posOriginator );

	VECSUB( &dist, &posOriginator, &posListener );

	geomv_t len = VctLenX( &dist );
	geomv_t minvoldist = pDistVolumeInfo->fMinVolumeDist;
	geomv_t maxvoldist = pDistVolumeInfo->fMaxVolumeDist;

	// default to max. volume
	int nVolume = AUD_MAX_VOLUME;

	// check whether inside of sound sphere
	if ( len <= minvoldist ) {

		// check trivial case: inside of max. volume sphere
		if ( len <= maxvoldist ) {
			nVolume = pDistVolumeInfo->nMaxVolume;

		} else {

#ifdef REALISTIC_VOLUME_DECREASE

			// NOTE: this formula comes from the QMDX SDK documentation
			//       Applied_Volume = Volume / ( 1 + ( ( Range / minDistance ) - 1 ) * scale )

			// calculate fraction inside sound sphere
			geomv_t frac = GEOMV_1 + GEOMV_MUL( ( GEOMV_DIV ( len, maxvoldist ) - GEOMV_1 ), FLOAT_TO_GEOMV( pDistVolumeInfo->fRolloffscale ) );

			nVolume = GEOMV_TO_INT( GEOMV_DIV( INT_TO_GEOMV( pDistVolumeInfo->nMaxVolume ), frac ) );
#else

			// calculate fraction inside sound sphere
			geomv_t frac = GEOMV_DIV( ( len - maxvoldist ), ( minvoldist - maxvoldist ) );

			// calculate volume
			nVolume = pDistVolumeInfo->nMaxVolume - GEOMV_TO_INT( GEOMV_MUL( INT_TO_GEOMV( pDistVolumeInfo->nMaxVolume - pDistVolumeInfo->nMinVolume ), frac ) );

#endif // REALISTIC_VOLUME_DECREASE

		}

	} else {

		nVolume = pDistVolumeInfo->nMinVolume;

	}

	return nVolume;
}


// callback for setting the distance-based volume of a channel ----------------
//
PRIVATE
int AUDm_DistVolumeVoice( void* param )
{
	DistVolumeInfo_s* pDistVolumeInfo = (DistVolumeInfo_s*) param;
	ASSERT( pDistVolumeInfo != NULL );

	// get the volume
	int nVolume = AUDm_GetVolumeDistBased( pDistVolumeInfo );

	// and set it on the channel
	AUDs_SetVoiceVolume( pDistVolumeInfo->nChannel, nVolume );

	// indicate event repetition
	return TRUE;
}


// add a distance-volume info to list for the specified channel ---------------
//
PRIVATE
int AUDm_AddChannelSoundSource( int nChannel, DistVolumeInfo_s* pDistVolumeInfo )
{
	ASSERT( ( nChannel >= 0 ) && ( nChannel < NUMCHANNELS ) );
	ASSERT( pDistVolumeInfo != NULL );

	pDistVolumeInfo->next 			= g_pDistVolumeInfo[ nChannel ];
	g_pDistVolumeInfo[ nChannel ] 	= pDistVolumeInfo;

	return TRUE;
}

// remove a distance-volume info from list for the specified channel ----------
//
PRIVATE
int AUDm_RemoveChannelSoundSource( int nChannel, GenObject* originator  )
{
	ASSERT( ( nChannel >= 0 ) && ( nChannel < NUMCHANNELS ) );

	DistVolumeInfo_s* pCurrent = g_pDistVolumeInfo[ nChannel ];
	DistVolumeInfo_s* pLast    = NULL;

	for( ; pCurrent != NULL; ) {

		// we assume only one DistVolumeInfo_s per object per channel
		if( pCurrent->pOriginator == originator ) {
			if ( pLast != NULL ) {
				pLast->next = pCurrent->next;
			} else {
				// remove head element
				g_pDistVolumeInfo[ nChannel ] = pCurrent->next;
			}
			FREEMEM( pCurrent );

			return TRUE;
		}

		pLast    = pCurrent;
		pCurrent = pCurrent->next;
	}

	return FALSE;
}

// walk all sound sources for a specific channel and set the volume to the loudest one
PRIVATE
int AUDm_WalkChannelSoundSources( void *param )
{
	int nChannel = (long)param;

	DistVolumeInfo_s* pCurrent = g_pDistVolumeInfo[ nChannel ];
	ASSERT( pCurrent != NULL );
	ASSERT( pCurrent->nChannel == nChannel );

	int nVolume 	= 0;

	for( ; pCurrent != NULL; ) {
		// get the volume
		int nObjectVolume = AUDm_GetVolumeDistBased( pCurrent );
		nVolume = max( nVolume, nObjectVolume );

		pCurrent = pCurrent->next;
	}

	// and set it on the channel
	AUDs_SetVoiceVolume( nChannel, nVolume );

	return TRUE;
}

#endif // DIST_VOLUME


// play sound effect "teleporter" ---------------------------------------------
//
int AUD_Teleporter( GenObject* teleporter )
{
	// check whether we already play the sample
	int stopped;
	AUDs_GetVoiceStatus( TRACKBEEP_VOICE, &stopped );
	
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS ) {
		if ( !stopped ) {
			AUXDATA_LAST_SOUNDSYS_RETURNCODE =
				AUDs_StopVoice( TRACKBEEP_VOICE, FALSE );
		}
		return FALSE;
	}
		
	AUD_FETCHSAMPLE( SND_NAME_TELEPORTER );
	
	if ( stopped == 1 ) {
		if ( fetch_wave != NULL ) {
			
			// set the info for changing the volume based on the distance
			DistVolumeInfo_s* pDVI = (DistVolumeInfo_s*) ALLOCMEM( sizeof ( DistVolumeInfo_s ) );
			pDVI->nChannel			= TRACKBEEP_VOICE;
			pDVI->nMaxVolume		= ( fetch_params.flags & SOUNDPARAMS_VOLUME ) ? fetch_params.volume : AUD_MAX_VOLUME;
			pDVI->nMinVolume		= AUD_MIN_VOLUME;
			pDVI->fMaxVolumeDist	= FLOAT_TO_GEOMV( 50  );							// distance for max. volume
			pDVI->fMinVolumeDist	= FLOAT_TO_GEOMV( MAX_VOLUME_DISTANCE_TELEPORTER );	// distance for min. volume
			pDVI->fRolloffscale		= 1.0f;
			pDVI->pListener			= MyShip;
			pDVI->pOriginator		= teleporter;
			
			// get the volume for starting the loop
			fetch_params.volume = AUDm_GetVolumeDistBased( pDVI );
			fetch_params.flags |= SOUNDPARAMS_VOLUME;
			
			// check whether there is already a event for this
			if ( g_pDistVolumeInfo[ TRACKBEEP_VOICE ] == NULL )	{
				
				// add the distance volume info to the list for the lightning channel
				AUDm_AddChannelSoundSource( TRACKBEEP_VOICE, pDVI );
				
				// add the event for changing the volume based on the distance
				event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );
				
				event->type				= EVT_TYPE_SND_TELEPORTER_VOLUME;
				event->callback			= (event_callback_t) AUDm_WalkChannelSoundSources;
				event->callback_params	= (void*) TRACKBEEP_VOICE;
				event->refframe_delay	= DISTANCE_UPDATE_FRAMES;
				event->flags			= EVENT_PARAM_AUTOFREE			|
  											EVENT_PARAM_AUTOTRIGGER		|
											EVENT_PARAM_LOOP_INDEFINITE	|
											EVENT_PARAM_ONE_PER_TYPE;
				EVT_AddEvent( event );
				
			} else {
				
				// add the distance volume info to the list for the lightning channel
				AUDm_AddChannelSoundSource( TRACKBEEP_VOICE, pDVI );
				
			}

			fetch_params.flags |= SOUNDPARAMS_LOOP;
			fetch_params.start = 0;
			
			AUXDATA_LAST_SOUNDSYS_RETURNCODE =
				AUDs_PlayVoiceBuffer( TRACKBEEP_VOICE, fetch_wave, &fetch_params );
			
		}
	}

	return TRUE;
}

// stop "teleporter" effect ---------------------------------------------------
//
int AUD_TeleporterOff( GenObject* teleporter )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;
	
	AUDm_RemoveChannelSoundSource( TRACKBEEP_VOICE, teleporter );
	
	// check whether this was the last soundsource playing on the channel
	if ( g_pDistVolumeInfo[ TRACKBEEP_VOICE ] == NULL ) {
		
		// remove the event for setting the volume
		EVT_RemoveEventType( EVT_TYPE_SND_TELEPORTER_VOLUME );
		
		// and stop playing, as no other soundsource is available anymore
		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_StopVoice( TRACKBEEP_VOICE, FALSE );
	}
	
	return TRUE;
}


// play sound effect "lightning" ----------------------------------------------
//
int AUD_Lightning( ShipObject *shippo )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	AUD_FETCHSAMPLE( SND_NAME_LIGHTNING );

	if ( fetch_wave != NULL ) {

#ifdef DIST_VOLUME

		ASSERT( shippo != NULL );

#ifdef USE_SOUNDSOURCE_LIST

		// set the info for changing the volume based on the distance
		DistVolumeInfo_s* pDVI = (DistVolumeInfo_s*) ALLOCMEM( sizeof ( DistVolumeInfo_s ) );
		pDVI->nChannel			= LIGHTNING_VOICE;
		pDVI->nMaxVolume		= ( fetch_params.flags & SOUNDPARAMS_VOLUME ) ? fetch_params.volume : AUD_MAX_VOLUME;
		pDVI->nMinVolume		= AUD_MIN_VOLUME;
		pDVI->fMaxVolumeDist	= FLOAT_TO_GEOMV( 300  );	              // distance for max. volume
		pDVI->fMinVolumeDist	= FLOAT_TO_GEOMV( MAX_DISTANCE_VISIBLE ); // distance for min. volume
		pDVI->fRolloffscale		= 1.0f;
		pDVI->pListener			= MyShip;
		pDVI->pOriginator		= shippo;

		// get the volume for starting the loop
		fetch_params.volume = AUDm_GetVolumeDistBased( pDVI );
		fetch_params.flags |= SOUNDPARAMS_VOLUME;

		// check whether there is already a event for this
		if ( g_pDistVolumeInfo[ LIGHTNING_VOICE ] == NULL )	{

			// add the distance volume info to the list for the lightning channel
			AUDm_AddChannelSoundSource( LIGHTNING_VOICE, pDVI );

			// add the event for changing the volume based on the distance
			event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );

			event->type				= EVT_TYPE_SND_LIGHTNING_VOLUME;
			event->callback			= (event_callback_t) AUDm_WalkChannelSoundSources;
			event->callback_params	= (void*) LIGHTNING_VOICE;
			event->refframe_delay	= DISTANCE_UPDATE_FRAMES;
			event->flags			= EVENT_PARAM_AUTOFREE			|
									  EVENT_PARAM_AUTOTRIGGER		|
									  EVENT_PARAM_LOOP_INDEFINITE	|
									  EVENT_PARAM_ONE_PER_TYPE;
			EVT_AddEvent( event );

		} else {

			// add the distance volume info to the list for the lightning channel
			AUDm_AddChannelSoundSource( LIGHTNING_VOICE, pDVI );

		}
#else
		if ( shippo != MyShip ) {

			// set the info for changing the volume based on the distance
			DistVolumeInfo_s* pDVI = (DistVolumeInfo_s*) ALLOCMEM( sizeof ( DistVolumeInfo_s ) );
			pDVI->nChannel			= LIGHTNING_VOICE;
			pDVI->nMaxVolume		= ( fetch_params.flags & SOUNDPARAMS_VOLUME ) ? fetch_params.volume : AUD_MAX_VOLUME;
			pDVI->nMinVolume		= AUD_MIN_VOLUME;
			pDVI->fMaxVolumeDist	= FLOAT_TO_GEOMV( 300  );	              // distance for max. volume
			pDVI->fMinVolumeDist	= FLOAT_TO_GEOMV( MAX_DISTANCE_VISIBLE ); // distance for min. volume
			pDVI->fRolloffscale		= 1.0f;
			pDVI->pListener			= MyShip;
			pDVI->pOriginator		= shippo;

			// get the volume for starting the loop
			fetch_params.volume = AUDm_GetVolumeDistBased( pDVI );
			fetch_params.flags |= SOUNDPARAMS_VOLUME;

			// add the event for changing the volume based on the distance
			event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );

			event->type				= EVT_TYPE_SND_LIGHTNING_VOLUME;
			event->callback			= (event_callback_t) AUDm_DistVolumeVoice;
			event->callback_params	= (void*) pDVI;
			event->refframe_delay	= DISTANCE_UPDATE_FRAMES;
			event->flags			= EVENT_PARAM_AUTOFREE			|
									  EVENT_PARAM_AUTOFREE_CALLBACK |
									  EVENT_PARAM_AUTOTRIGGER		|
									  EVENT_PARAM_LOOP_INDEFINITE	|
									  EVENT_PARAM_ONE_PER_TYPE;
			EVT_AddEvent( event );
		}

#endif // USE_SOUNDSOURCE_LIST

#endif //DIST_VOLUME

		fetch_params.flags |= SOUNDPARAMS_LOOP;
		//FIXME: we need to set this to a reasonable value
		fetch_params.start = 3648 * 2;

		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_PlayVoiceBuffer( LIGHTNING_VOICE, fetch_wave, &fetch_params );
	}

	return TRUE;
}


// stop "lightning" effect ----------------------------------------------------
//
int AUD_LightningOff( ShipObject* shippo )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

#ifdef DIST_VOLUME

#ifdef USE_SOUNDSOURCE_LIST

	AUDm_RemoveChannelSoundSource( LIGHTNING_VOICE, shippo );

	// check whether this was the last soundsource playing on the channel
	if ( g_pDistVolumeInfo[ LIGHTNING_VOICE ] == NULL ) {

		// remove the event for setting the volume
		EVT_RemoveEventType( EVT_TYPE_SND_LIGHTNING_VOLUME );

		// and stop playing, as no other soundsource is available anymore
		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_StopVoice( LIGHTNING_VOICE, FALSE );
	}

#else // USE_SOUNDSOURCE_LIST

	// remove the event for setting the volume
	EVT_RemoveEventType( EVT_TYPE_SND_LIGHTNING_VOLUME );

	AUXDATA_LAST_SOUNDSYS_RETURNCODE =
		AUDs_StopVoice( LIGHTNING_VOICE, FALSE );

#endif // USE_SOUNDSOURCE_LIST

#else // DIST_VOLUME

	AUXDATA_LAST_SOUNDSYS_RETURNCODE =
		AUDs_StopVoice( LIGHTNING_VOICE, FALSE );

#endif // DIST_VOLUME

	return TRUE;
}


// play sound effect "helix" --------------------------------------------------
//
int AUD_Helix( ShipObject *shippo )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	AUD_FETCHSAMPLE( sample_channel_infos[ SCI_HELIX ].samplename );

	if ( fetch_wave != NULL ) {

#ifdef DIST_VOLUME

		ASSERT( shippo != NULL );

#ifdef USE_SOUNDSOURCE_LIST

		// set the info for changing the volume based on the distance
		DistVolumeInfo_s* pDVI = (DistVolumeInfo_s*) ALLOCMEM( sizeof ( DistVolumeInfo_s ) );
		pDVI->nChannel			= HELIX_VOICE;
		pDVI->nMaxVolume		= ( fetch_params.flags & SOUNDPARAMS_VOLUME ) ? fetch_params.volume : AUD_MAX_VOLUME;
		pDVI->nMinVolume		= AUD_MIN_VOLUME;
		pDVI->fMaxVolumeDist	= FLOAT_TO_GEOMV( 300  );			      // distance for max. volume
		pDVI->fMinVolumeDist	= FLOAT_TO_GEOMV( MAX_DISTANCE_VISIBLE ); // distance for min. volume
		pDVI->fRolloffscale		= 1.0f;
		pDVI->pListener			= MyShip;
		pDVI->pOriginator		= shippo;

		// get the volume for starting the loop
		fetch_params.volume = AUDm_GetVolumeDistBased( pDVI );
		fetch_params.flags |= SOUNDPARAMS_VOLUME;

		// check whether there is already a event for this
		if ( g_pDistVolumeInfo[ HELIX_VOICE ] == NULL )	{

			// add the distance volume info to the list for the lightning channel
			AUDm_AddChannelSoundSource( HELIX_VOICE, pDVI );

			// add the event for changing the volume based on the distance
			event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );

			event->type				= EVT_TYPE_SND_HELIX_VOLUME;
			event->callback			= (event_callback_t) AUDm_WalkChannelSoundSources;
			event->callback_params	= (void*) HELIX_VOICE;
			event->refframe_delay	= DISTANCE_UPDATE_FRAMES;
			event->flags			= EVENT_PARAM_AUTOFREE			|
									  EVENT_PARAM_AUTOTRIGGER		|
									  EVENT_PARAM_LOOP_INDEFINITE	|
									  EVENT_PARAM_ONE_PER_TYPE;
			EVT_AddEvent( event );

		} else {

			// add the distance volume info to the list for the lightning channel
			AUDm_AddChannelSoundSource( HELIX_VOICE, pDVI );

		}

#else // USE_SOUNDSOURCE_LIST

		if ( shippo != MyShip ) {

			// set the info for changing the volume based on the distance
			DistVolumeInfo_s* pDVI = (DistVolumeInfo_s*) ALLOCMEM( sizeof ( DistVolumeInfo_s ) );
			pDVI->nChannel			= HELIX_VOICE;
			pDVI->nMaxVolume		= ( fetch_params.flags & SOUNDPARAMS_VOLUME ) ? fetch_params.volume : AUD_MAX_VOLUME;
			pDVI->nMinVolume		= AUD_MIN_VOLUME;
			pDVI->fMaxVolumeDist	= FLOAT_TO_GEOMV( 300  );			      // distance for max. volume
			pDVI->fMinVolumeDist	= FLOAT_TO_GEOMV( MAX_DISTANCE_VISIBLE ); // distance for min. volume
			pDVI->fRolloffscale		= 1.0f;
			pDVI->pListener			= MyShip;
			pDVI->pOriginator		= shippo;

			// immediately set the volume for this channel
			AUDm_DistVolumeVoice( pDVI );

			// add the event for changing the volume based on the distance
			event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );

			event->type				= EVT_TYPE_SND_HELIX_VOLUME;
			event->callback			= (event_callback_t) AUDm_DistVolumeVoice;
			event->callback_params	= (void*) pDVI;
			event->refframe_delay	= DISTANCE_UPDATE_FRAMES;
			event->flags			= EVENT_PARAM_AUTOFREE			|
									  EVENT_PARAM_AUTOFREE_CALLBACK |
									  EVENT_PARAM_AUTOTRIGGER		|
									  EVENT_PARAM_LOOP_INDEFINITE	|
									  EVENT_PARAM_ONE_PER_TYPE;
			EVT_AddEvent( event );
		}

#endif // USE_SOUNDSOURCE_LIST

#endif //DIST_VOLUME

		fetch_params.flags |= SOUNDPARAMS_LOOP;
		fetch_params.start = 0;

		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_PlayVoiceBuffer( HELIX_VOICE, fetch_wave, &fetch_params );

	}

	return TRUE;
}


// stop sound effect "helix" --------------------------------------------------
//
int AUD_HelixOff( ShipObject* shippo )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

#ifdef DIST_VOLUME

#ifdef USE_SOUNDSOURCE_LIST

	AUDm_RemoveChannelSoundSource( HELIX_VOICE, shippo );

	// check whether this was the last soundsource playing on the channel
	if ( g_pDistVolumeInfo[ HELIX_VOICE ] == NULL ) {

		// remove the event for setting the volume
		EVT_RemoveEventType( EVT_TYPE_SND_HELIX_VOLUME );

		// and stop playing, as no other soundsource is available anymore
		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_StopVoice( HELIX_VOICE, FALSE );
	}

#else // USE_SOUNDSOURCE_LIST

	// remove the event for setting the volume
	EVT_RemoveEventType( EVT_TYPE_SND_HELIX_VOLUME );

	AUXDATA_LAST_SOUNDSYS_RETURNCODE =
		AUDs_StopVoice( HELIX_VOICE, FALSE );

#endif // USE_SOUNDSOURCE_LIST

#else // DIST_VOLUME

	AUXDATA_LAST_SOUNDSYS_RETURNCODE =
		AUDs_StopVoice( HELIX_VOICE, TRUE );


#endif // DIST_VOLUME

	return TRUE;
}



// play sound effect "tracking" -----------------------------------------------
//
int AUD_Tracking()
{
	//FIXME:
	// currently not in use, therefore save time
	// by not looking for it.

//	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_TRACKING ] );

	return TRUE;
}


// play sound effect "tracking beep" ------------------------------------------
//
int AUD_TrackingBeep()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_TRACKINGBEEP ] );
	return TRUE;
}


// play sound effect "player joined" ------------------------------------------
//
int	AUD_NewPlayerJoined()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_NEWPLAYER ] );
	return TRUE;
}


// weapons index to weapons mask table ----------------------------------------
//
//FIXME: gamecode !!!
int index_weapons_mask[ 5 ] = {

	WPMASK_CANNON_LASER,
	WPMASK_CANNON_HELIX,
	WPMASK_CANNON_LIGHTNING,
	WPMASK_CANNON_PHOTON,
	WPMASK_DEVICE_EMP,
};


// play sound effect "laser changed" ------------------------------------------
//
int	AUD_SelectLaser( int sellaser )
{
	ASSERT( ( sellaser >= 0 ) && ( sellaser < 5 ) );

	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS ) {
		return FALSE;
	}

	// check if selected laser is available
	if ( OBJ_DeviceAvailable( MyShip, index_weapons_mask[ sellaser ] ) ) {

		event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );

		event->type				= EVT_TYPE_SND_GUN_SELECT;
		event->callback			= (event_callback_t) AUD_PlaySampleOnChannel;
		event->callback_params	= &sample_channel_infos[ SCI_SELECT_LASER1 + sellaser ];
		event->refframe_delay	= LASER_SELECTED_SOUND_DELAY;
		event->flags			= EVENT_PARAM_AUTOFREE |
								  EVENT_PARAM_AUTOTRIGGER |
								  EVENT_PARAM_ONE_PER_TYPE;
		EVT_AddEvent( event );

	} else {

		// remove any previously issued event
		EVT_RemoveEventType( EVT_TYPE_SND_GUN_SELECT );
	}

	// play the gun switching sample
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_SWITCH_GUN ] );

	return TRUE;
}


// play sound effect "missile changed" ----------------------------------------
//
int	AUD_SelectMissile( int selmissile )
{
	ASSERT( ( selmissile >= 0 ) && ( selmissile < 4 ) );

	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	if	( ( ( selmissile == 0 ) && ( MyShip->NumMissls     > 0 ) ) ||
		  ( ( selmissile == 1 ) && ( MyShip->NumHomMissls  > 0 ) ) ||
		  ( ( selmissile == 2 ) && ( MyShip->NumMines      > 0 ) ) ||
		  ( ( selmissile == 3 ) && ( MyShip->NumPartMissls > 0 ) )
		  ) {

		event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );

		event->type				= EVT_TYPE_SND_MISSILE_SELECT;
		event->callback			= (event_callback_t) AUD_PlaySampleOnChannel;
		event->callback_params	= &sample_channel_infos[ SCI_SELECT_MISSILE1 + selmissile ];
		event->refframe_delay	= MISSILE_SELECTED_SOUND_DELAY;
		event->flags			= EVENT_PARAM_AUTOFREE |
								  EVENT_PARAM_AUTOTRIGGER |
								  EVENT_PARAM_ONE_PER_TYPE;
		EVT_AddEvent( event );

	} else {

		// remove any previously issued event
		EVT_RemoveEventType( EVT_TYPE_SND_MISSILE_SELECT );
	}

	// play the gun switching sample
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_SWITCH_MISSILE ] );

	return TRUE;
}


// play sound effect for sliding in -------------------------------------------
//
int	AUD_SlideIn( int type )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	switch( type ) {

		case 0:
			AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_OPTIONS_SLIDE_IN ] );
			break;

		default:
			break;
	}

	return TRUE;
}


// play sound effect for sliding out ------------------------------------------
//
int	AUD_SlideOut( int type )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	switch( type ) {

		case 0:
			AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_OPTIONS_SLIDE_OUT ] );
			break;

		default:
			break;
	}

	return TRUE;
}


// play voice sample for missile countdown ------------------------------------
//
int	AUD_Countdown( int num )
{
	if ( num < 1 || num > 5 )
		return FALSE;

	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

//	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_COUNTDOWN1 + num - 1 ] );

	event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );

	event->type				= EVT_TYPE_SND_COUNTDOWN;
	event->callback			= (event_callback_t) AUD_PlaySampleOnChannel;
	event->callback_params	= &sample_channel_infos[ SCI_COUNTDOWN1 + num - 1 ];
	event->refframe_delay	= COUNTDOWN_SOUND_DELAY;
	event->flags			= EVENT_PARAM_AUTOFREE |
							  EVENT_PARAM_AUTOTRIGGER |
							  EVENT_PARAM_ONE_PER_TYPE;
	EVT_AddEvent( event );

	return TRUE;
}


// struct for handling volume change on a specific channel --------------------
//
struct ChannelVolumeChange_s {

	int nChannel;
	int nVolume;
	int nVolumeDelta;
};


// callback function for fading out a specific voice --------------------------
//
PRIVATE
int AUDm_FadeOutVoice( void* param )
{
	ChannelVolumeChange_s* pChannelVolumeChange = (ChannelVolumeChange_s*)param;
	ASSERT( pChannelVolumeChange != NULL );

	// decrease the volume
	pChannelVolumeChange->nVolume -= abs( pChannelVolumeChange->nVolumeDelta );

	// and set the volume on the channel
	if ( pChannelVolumeChange->nVolume > 0 ) {

		AUDs_SetVoiceVolume( pChannelVolumeChange->nChannel, pChannelVolumeChange->nVolume );

		// indicate event repetition
		return TRUE;

	} else {

		// stop playing the voice
		AUDs_StopVoice( pChannelVolumeChange->nChannel, FALSE );

		// indicate removal of the event
		return FALSE;
	}
}


// start playing the thrust sample --------------------------------------------
//
int AUD_StartThrust( int type )
{
    if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	int sci;
	dword evttype;

	switch( type ) {

		case THRUST_TYPE_NORMAL:
			sci 	= SCI_THRUST;
			evttype = EVT_TYPE_SND_THRUST_FADEOUT;
			break;

		case THRUST_TYPE_SLIDE:
			sci 	= SCI_SLIDETHRUST;
			evttype = EVT_TYPE_SND_SLIDETHRUST_FADEOUT;
			break;

		default:
			ASSERT( 0 );

	}

	// fetch the sample
	AUD_FETCHSAMPLE_EX( sample_channel_infos[ sci ].samplename,
						sample_channel_infos[ sci ].sampleid );

	// return, if not available
	if ( fetch_pSampleInfo == NULL )
		return FALSE;

	// set the info for fading the volume on a channel
	ChannelVolumeChange_s *pCVC = (ChannelVolumeChange_s*)
		ALLOCMEM( sizeof ( ChannelVolumeChange_s ) );
	pCVC->nChannel		= sample_channel_infos[ sci ].channel;
	pCVC->nVolume		= fetch_pSampleInfo->volume;
	pCVC->nVolumeDelta	= pCVC->nVolume / 5;

	// add the event for stopping the Thrust, this removes any previous event
	event_s* event			= (event_s*) ALLOCMEM( sizeof ( event_s ) );
	event->type				= evttype;
	event->callback			= (event_callback_t) AUDm_FadeOutVoice;
	event->callback_params	= (void*) pCVC;
	event->refframe_delay	= THRUST_SOUND_LIFETIME;
	event->flags			= EVENT_PARAM_AUTOFREE          |
							  EVENT_PARAM_AUTOFREE_CALLBACK |
							  EVENT_PARAM_AUTOTRIGGER       |
							  EVENT_PARAM_LOOP_INDEFINITE	|
							  EVENT_PARAM_ONE_PER_TYPE;
	EVT_AddEvent( event );

	// check whether we already play the sample
	int stopped;
	AUDs_GetVoiceStatus( sample_channel_infos[ sci ].channel, &stopped );

	if ( stopped == 0 ) {
		// set the volume to max
		AUDs_SetVoiceVolume( sample_channel_infos[ sci ].channel, fetch_params.volume );
		return TRUE;
	}


	// enable looping of sample
	fetch_params.flags |= SOUNDPARAMS_LOOP;

	if ( fetch_wave != NULL ) {
		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_PlayVoiceBuffer( sample_channel_infos[ sci ].channel,
				fetch_wave, &fetch_params );
	}

	return TRUE;
}


// callback function to stop a specific voice ---------------------------------
//
PRIVATE
int AUDm_EventStopVoice( void* param )
{
	uintptr_t channel = (uintptr_t) param;
	AUDs_StopVoice( (int) channel, FALSE );

	return TRUE;
}


// play sample "incoming missile" ---------------------------------------------
//
int AUD_IncomingMissile( int looptime )
{
    if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_INCOMING ] );

	event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );

	event->type				= EVT_TYPE_SND_INCOMING_STOP;
	event->callback			= (event_callback_t) AUDm_EventStopVoice;
	event->callback_params	= (void *) sample_channel_infos[ SCI_INCOMING ].channel;
	event->refframe_delay	= looptime;
	event->flags			= EVENT_PARAM_AUTOFREE |
							  EVENT_PARAM_AUTOTRIGGER |
							  EVENT_PARAM_ONE_PER_TYPE;
	EVT_AddEvent( event );

	return TRUE;
}


// flag whether sample "booting" is playing -----------------------------------
//
static int booting_sample_playing = FALSE;


// play sample "booting" ------------------------------------------------------
//
int AUD_BootOnBoardComputer()
{
	booting_sample_playing =
		AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_BOOTING ] );

	return TRUE;
}


// make sure "booting" sample is not playing ----------------------------------
//
int AUD_StopOnBoardComputer()
{
	if ( !booting_sample_playing )
		return FALSE;

	// stop playing the voice
	AUDs_StopVoice( sample_channel_infos[ SCI_BOOTING ].channel, FALSE );
	booting_sample_playing = FALSE;

	return TRUE;
}


// start sample "laserbeam" ---------------------------------------------------
//
int AUD_StartLaserBeam()
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	// fetch the sample
	AUD_FETCHSAMPLE_EX( sample_channel_infos[ SCI_LASERBEAM ].samplename, sample_channel_infos[ SCI_LASERBEAM ].sampleid );

	// return, if not available
	if ( fetch_pSampleInfo == NULL )
		return FALSE;

	// enable looping of sample
	fetch_params.flags |= SOUNDPARAMS_LOOP;

	if ( fetch_wave != NULL ) {
		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_PlayVoiceBuffer( sample_channel_infos[ SCI_LASERBEAM ].channel, fetch_wave, &fetch_params );
	}

	return TRUE;
}


// stop sample "laserbeam" ----------------------------------------------------
//
int AUD_StopLaserBeam()
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	AUDs_StopVoice( sample_channel_infos[ SCI_LASERBEAM ].channel, FALSE );

	return TRUE;
}


// play sound effect "enemy hit" ----------------------------------------------
//
int	AUD_EnemyShieldHit()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_ENEMYSHIELD_HIT ] );
	return TRUE;
}


// play sound effect "enemyhull hit" ------------------------------------------
//
int	AUD_EnemyHullHit()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_ENEMYHULL_HIT ] );
	return TRUE;
}


// play sound effect "playershield hit" ---------------------------------------
//
int	AUD_PlayerShieldHit()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_PLAYERSHIELD_HIT ] );
	return TRUE;
}


// play sound effect "playerhull hit" -----------------------------------------
//
int	AUD_PlayerHullHit()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_PLAYERHULL_HIT ] );
	return TRUE;
}


// play sound effect "mine detector" ------------------------------------------
//
int	AUD_MineDetector()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_MINE_DETECTOR ] );
	return TRUE;
}


// play sound effect "low shields" --------------------------------------------
//
int AUD_LowShields()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_LOW_SHIELDS ] );
	return TRUE;
}


// play sound effect "x kills left" -------------------------------------------
//
int AUD_KillsLeft( int killsleft )
{
	if ( ( killsleft < 1 ) || ( killsleft > 3 ) )
		return FALSE;

	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );

	event->type				= EVT_TYPE_SND_KILLSLEFT;
	event->callback			= (event_callback_t) AUD_PlaySampleOnChannel;
	event->callback_params	= &sample_channel_infos[ SCI_ONE_KILL_LEFT + killsleft - 1 ];
	event->refframe_delay	= KILLSLEFT_SOUND_DELAY;
	event->flags			= EVENT_PARAM_AUTOFREE |
							  EVENT_PARAM_AUTOTRIGGER |
							  EVENT_PARAM_ONE_PER_TYPE;
	EVT_AddEvent( event );

	return TRUE;
}


// play afterburner activation sample -----------------------------------------
//
int AUD_ActivateAfterBurner()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_AFTERBURNER_ACTIVATE ] );
	return TRUE;
}


// play afterburner deactivation sample ---------------------------------------
//
int AUD_DeactivateAfterBurner()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_AFTERBURNER_DEACTIVATE ] );
	return TRUE;
}


// play swarm missiles sample -------------------------------------------------
//
int AUD_SwarmMissiles( Vertex3 *origin, GenObject *dummyobj )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	AUD_FETCHSAMPLE( sample_channel_infos[ SCI_SWARM_MISSILES ].samplename );

	if ( fetch_wave != NULL ) {

#ifdef DIST_VOLUME

#ifdef USE_SOUNDSOURCE_LIST

		// set the info for changing the volume based on the distance
		DistVolumeInfo_s* pDVI = (DistVolumeInfo_s*) ALLOCMEM( sizeof ( DistVolumeInfo_s ) );
		pDVI->nChannel			= SWARM_MISSILE_VOICE;
		pDVI->nMaxVolume		= ( fetch_params.flags & SOUNDPARAMS_VOLUME ) ? fetch_params.volume : AUD_MAX_VOLUME;
		pDVI->nMinVolume		= AUD_MIN_VOLUME;
		pDVI->fMaxVolumeDist	= FLOAT_TO_GEOMV( 300  );			      // distance for max. volume
		pDVI->fMinVolumeDist	= FLOAT_TO_GEOMV( MAX_DISTANCE_VISIBLE ); // distance for min. volume
		pDVI->fRolloffscale		= 1.0f;
		pDVI->pListener			= MyShip;
		pDVI->pOriginator		= dummyobj;

		// get the volume for starting the loop
		fetch_params.volume = AUDm_GetVolumeDistBased( pDVI );
		fetch_params.flags |= SOUNDPARAMS_VOLUME;

		// check whether there is already a event for this
		if ( g_pDistVolumeInfo[ SWARM_MISSILE_VOICE ] == NULL )	{

			// add the distance volume info to the list for the lightning channel
			AUDm_AddChannelSoundSource( SWARM_MISSILE_VOICE, pDVI );

			// add the event for changing the volume based on the distance
			event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );

			event->type				= EVT_TYPE_SND_SWARM_VOLUME;
			event->callback			= (event_callback_t) AUDm_WalkChannelSoundSources;
			event->callback_params	= (void*) SWARM_MISSILE_VOICE;
			event->refframe_delay	= DISTANCE_UPDATE_FRAMES;
			event->flags			= EVENT_PARAM_AUTOFREE			|
									  EVENT_PARAM_AUTOTRIGGER		|
									  EVENT_PARAM_LOOP_INDEFINITE	|
									  EVENT_PARAM_ONE_PER_TYPE;
			EVT_AddEvent( event );

		} else {

			// add the distance volume info to the list for the lightning channel
			AUDm_AddChannelSoundSource( SWARM_MISSILE_VOICE, pDVI );

		}

#else // USE_SOUNDSOURCE_LIST

		if ( shippo != MyShip ) {

			// set the info for changing the volume based on the distance
			DistVolumeInfo_s* pDVI = (DistVolumeInfo_s*) ALLOCMEM( sizeof ( DistVolumeInfo_s ) );
			pDVI->nChannel			= SWARM_MISSILE_VOICE;
			pDVI->nMaxVolume		= ( fetch_params.flags & SOUNDPARAMS_VOLUME ) ? fetch_params.volume : AUD_MAX_VOLUME;
			pDVI->nMinVolume		= AUD_MIN_VOLUME;
			pDVI->fMaxVolumeDist	= FLOAT_TO_GEOMV( 300  );			      // distance for max. volume
			pDVI->fMinVolumeDist	= FLOAT_TO_GEOMV( MAX_DISTANCE_VISIBLE ); // distance for min. volume
			pDVI->fRolloffscale		= 1.0f;
			pDVI->pListener			= MyShip;
			pDVI->pOriginator		= dummyobj;

			// immediately set the volume for this channel
			AUDm_DistVolumeVoice( pDVI );

			// add the event for changing the volume based on the distance
			event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );

			event->type				= EVT_TYPE_SND_SWARM_VOLUME;
			event->callback			= (event_callback_t) AUDm_DistVolumeVoice;
			event->callback_params	= (void*) pDVI;
			event->refframe_delay	= DISTANCE_UPDATE_FRAMES;
			event->flags			= EVENT_PARAM_AUTOFREE			|
									  EVENT_PARAM_AUTOFREE_CALLBACK |
									  EVENT_PARAM_AUTOTRIGGER		|
									  EVENT_PARAM_LOOP_INDEFINITE	|
									  EVENT_PARAM_ONE_PER_TYPE;
			EVT_AddEvent( event );
		}

#endif // USE_SOUNDSOURCE_LIST

#endif //DIST_VOLUME

		fetch_params.flags |= SOUNDPARAMS_LOOP;
		fetch_params.start = 0;

		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_PlayVoiceBuffer( SWARM_MISSILE_VOICE, fetch_wave, &fetch_params );
	}

	return TRUE;
}


// stop swarm missiles sound effect -------------------------------------------
//
int AUD_SwarmMissilesOff( GenObject *dummypo )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

#ifdef DIST_VOLUME

#ifdef USE_SOUNDSOURCE_LIST

	AUDm_RemoveChannelSoundSource( SWARM_MISSILE_VOICE, dummypo );

	// check whether this was the last soundsource playing on the channel
	if ( g_pDistVolumeInfo[ SWARM_MISSILE_VOICE ] == NULL ) {

		// remove the event for setting the volume
		EVT_RemoveEventType( EVT_TYPE_SND_SWARM_VOLUME );

		// and stop playing, as no other soundsource is available anymore
		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_StopVoice( SWARM_MISSILE_VOICE, FALSE );
	}

#else // USE_SOUNDSOURCE_LIST

	// remove the event for setting the volume
	EVT_RemoveEventType( EVT_TYPE_SND_SWARM_VOLUME );

	AUXDATA_LAST_SOUNDSYS_RETURNCODE =
		AUDs_StopVoice( SWARM_MISSILE_VOICE, FALSE );

#endif // USE_SOUNDSOURCE_LIST

#else // DIST_VOLUME

	AUXDATA_LAST_SOUNDSYS_RETURNCODE =
		AUDs_StopVoice( SWARM_MISSILE_VOICE, TRUE );


#endif // DIST_VOLUME

	return TRUE;
}


// play photon cannon loading sound effect ------------------------------------
//
int AUD_PhotonLoading( ShipObject *objectpo )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	AUD_FETCHSAMPLE( sample_channel_infos[ SCI_PHOTON_LOADING ].samplename );

	if ( fetch_wave != NULL ) {

#ifdef DIST_VOLUME

#ifdef USE_SOUNDSOURCE_LIST

		// set the info for changing the volume based on the distance
		DistVolumeInfo_s* pDVI = (DistVolumeInfo_s*) ALLOCMEM( sizeof ( DistVolumeInfo_s ) );
		pDVI->nChannel			= PHOTON_CANNON_VOICE;
		pDVI->nMaxVolume		= ( fetch_params.flags & SOUNDPARAMS_VOLUME ) ? fetch_params.volume : AUD_MAX_VOLUME;
		pDVI->nMinVolume		= AUD_MIN_VOLUME;
		pDVI->fMaxVolumeDist	= FLOAT_TO_GEOMV( 300  );			      // distance for max. volume
		pDVI->fMinVolumeDist	= FLOAT_TO_GEOMV( MAX_DISTANCE_VISIBLE ); // distance for min. volume
		pDVI->fRolloffscale		= 1.0f;
		pDVI->pListener			= MyShip;
		pDVI->pOriginator		= objectpo;

		// get the volume for starting the loop
		fetch_params.volume = AUDm_GetVolumeDistBased( pDVI );
		fetch_params.flags |= SOUNDPARAMS_VOLUME;

		// check whether there is already a event for this
		if ( g_pDistVolumeInfo[ PHOTON_CANNON_VOICE ] == NULL )	{

			// add the distance volume info to the list for the lightning channel
			AUDm_AddChannelSoundSource( PHOTON_CANNON_VOICE, pDVI );

			// add the event for changing the volume based on the distance
			event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );

			event->type				= EVT_TYPE_SND_PHOTON_VOLUME;
			event->callback			= (event_callback_t) AUDm_WalkChannelSoundSources;
			event->callback_params	= (void*) PHOTON_CANNON_VOICE;
			event->refframe_delay	= DISTANCE_UPDATE_FRAMES;
			event->flags			= EVENT_PARAM_AUTOFREE			|
									  EVENT_PARAM_AUTOTRIGGER		|
									  EVENT_PARAM_LOOP_INDEFINITE	|
									  EVENT_PARAM_ONE_PER_TYPE;
			EVT_AddEvent( event );

		} else {

			// add the distance volume info to the list for the lightning channel
			AUDm_AddChannelSoundSource( PHOTON_CANNON_VOICE, pDVI );

		}

#else // USE_SOUNDSOURCE_LIST

		if ( shippo != MyShip ) {

			// set the info for changing the volume based on the distance
			DistVolumeInfo_s* pDVI = (DistVolumeInfo_s*) ALLOCMEM( sizeof ( DistVolumeInfo_s ) );
			pDVI->nChannel			= PHOTON_CANNON_VOICE;
			pDVI->nMaxVolume		= ( fetch_params.flags & SOUNDPARAMS_VOLUME ) ? fetch_params.volume : AUD_MAX_VOLUME;
			pDVI->nMinVolume		= AUD_MIN_VOLUME;
			pDVI->fMaxVolumeDist	= FLOAT_TO_GEOMV( 300  );			      // distance for max. volume
			pDVI->fMinVolumeDist	= FLOAT_TO_GEOMV( MAX_DISTANCE_VISIBLE ); // distance for min. volume
			pDVI->fRolloffscale		= 1.0f;
			pDVI->pListener			= MyShip;
			pDVI->pOriginator		= objectpo;

			// immediately set the volume for this channel
			AUDm_DistVolumeVoice( pDVI );

			// add the event for changing the volume based on the distance
			event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );

			event->type				= EVT_TYPE_SND_PHOTON_VOLUME;
			event->callback			= (event_callback_t) AUDm_DistVolumeVoice;
			event->callback_params	= (void*) pDVI;
			event->refframe_delay	= DISTANCE_UPDATE_FRAMES;
			event->flags			= EVENT_PARAM_AUTOFREE			|
									  EVENT_PARAM_AUTOFREE_CALLBACK |
									  EVENT_PARAM_AUTOTRIGGER		|
									  EVENT_PARAM_LOOP_INDEFINITE	|
									  EVENT_PARAM_ONE_PER_TYPE;
			EVT_AddEvent( event );
		}

#endif // USE_SOUNDSOURCE_LIST

#endif //DIST_VOLUME

		fetch_params.flags |= SOUNDPARAMS_LOOP;
		fetch_params.start = 0;

		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_PlayVoiceBuffer( PHOTON_CANNON_VOICE, fetch_wave, &fetch_params );
	}

	return TRUE;


}


// stop photon cannon loading sound effect ------------------------------------
//
int AUD_PhotonLoadingOff( ShipObject *objectpo )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

#ifdef DIST_VOLUME

#ifdef USE_SOUNDSOURCE_LIST

	AUDm_RemoveChannelSoundSource( PHOTON_CANNON_VOICE, objectpo );

	// check whether this was the last soundsource playing on the channel
	if ( g_pDistVolumeInfo[ PHOTON_CANNON_VOICE ] == NULL ) {

		// remove the event for setting the volume
		EVT_RemoveEventType( EVT_TYPE_SND_PHOTON_VOLUME );

		// and stop playing, as no other soundsource is available anymore
		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_StopVoice( PHOTON_CANNON_VOICE, FALSE );
	}

#else // USE_SOUNDSOURCE_LIST

	// remove the event for setting the volume
	EVT_RemoveEventType( EVT_TYPE_SND_PHOTON_VOLUME );

	AUXDATA_LAST_SOUNDSYS_RETURNCODE =
		AUDs_StopVoice( PHOTON_CANNON_VOICE, FALSE );

#endif // USE_SOUNDSOURCE_LIST

#else // DIST_VOLUME

	AUXDATA_LAST_SOUNDSYS_RETURNCODE =
		AUDs_StopVoice( PHOTON_CANNON_VOICE, TRUE );


#endif // DIST_VOLUME

	return TRUE;
}


// play photon firing sample --------------------------------------------------
//
int AUD_PhotonFiring( GenObject* objectpo )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;


	AUD_FETCHSAMPLE( sample_channel_infos[ SCI_PHOTON_FIRING ].samplename );

#ifdef DIST_VOLUME

	if ( fetch_wave != NULL ) {

		if ( ( objectpo != NULL ) && ( objectpo != MyShip ) )  {

			// set the info for changing the volume based on the distance
			DistVolumeInfo_s DVI;
			DVI.nMaxVolume		= ( fetch_params.flags & SOUNDPARAMS_VOLUME ) ? fetch_params.volume : AUD_MAX_VOLUME;
			DVI.nMinVolume		= AUD_MIN_VOLUME;
			DVI.fMaxVolumeDist	= FLOAT_TO_GEOMV( 300  );				   // distance for max. volume
			DVI.fMinVolumeDist	= FLOAT_TO_GEOMV( MAX_DISTANCE_VISIBLE );  // distance for min. volume
			DVI.fRolloffscale	= 1.0f;
			DVI.pListener		= MyShip;
			DVI.pOriginator		= objectpo;

			// get the volume
			fetch_params.volume = AUDm_GetVolumeDistBased( &DVI );
			fetch_params.flags |= SOUNDPARAMS_VOLUME;
		}

		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_PlayVoiceBuffer( sample_channel_infos[ SCI_PHOTON_FIRING ].channel, fetch_wave, &fetch_params );
	}

	return TRUE;

#else

	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_PHOTON_FIRING ] );
	return TRUE;

#endif // DIST_VOLUME

}


// play emp sample ------------------------------------------------------------
//
int AUD_EmpBlast( ShipObject *shippo, int level )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return FALSE;

	int sci_id;

	switch ( level ) {
	
		case 0:
			sci_id = SCI_EMP1;
			break;

		case 1:
			sci_id = SCI_EMP2;
			break;

		case 2:
			sci_id = SCI_EMP3;
			break;
		default:
			return FALSE;
	}

	AUD_FETCHSAMPLE( sample_channel_infos[ sci_id ].samplename );

#ifdef DIST_VOLUME

	if ( fetch_wave != NULL ) {

		if ( ( shippo != NULL ) && ( shippo != MyShip ) )  {

			// set the info for changing the volume based on the distance
			DistVolumeInfo_s DVI;
			DVI.nMaxVolume		= ( fetch_params.flags & SOUNDPARAMS_VOLUME ) ? fetch_params.volume : AUD_MAX_VOLUME;
			DVI.nMinVolume		= AUD_MIN_VOLUME;
			DVI.fMaxVolumeDist	= FLOAT_TO_GEOMV( 300  );				   // distance for max. volume
			DVI.fMinVolumeDist	= FLOAT_TO_GEOMV( MAX_DISTANCE_VISIBLE );  // distance for min. volume
			DVI.fRolloffscale	= 1.0f;
			DVI.pListener		= MyShip;
			DVI.pOriginator		= shippo;

			// get the volume
			fetch_params.volume = AUDm_GetVolumeDistBased( &DVI );
			fetch_params.flags |= SOUNDPARAMS_VOLUME;
		}

		AUXDATA_LAST_SOUNDSYS_RETURNCODE =
			AUDs_PlayVoiceBuffer( sample_channel_infos[ sci_id ].channel, fetch_wave, &fetch_params );
	}

	return TRUE;

#else

	AUD_PlaySampleOnChannel( &sample_channel_infos[ sci_id ] );
	return TRUE;

#endif // DIST_VOLUME

}


// play "You Did It" sample ---------------------------------------------------
//
int AUD_YouDidIt()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_YOU_DID_IT ] );
	return TRUE;
}


// play "You Have Lost" sample ------------------------------------------------
//
int AUD_YouHaveLost()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_YOU_HAVE_LOST ] );
	return TRUE;
}


// play li comment sample -----------------------------------------------------
//
int AUD_LiCommenting( int lcid )
{
	if ( !AUX_ENABLE_FUNKY_AUDIO_COMMENTS )
		return TRUE;

	//TODO:

	return TRUE;
}


// play li comment sample -----------------------------------------------------
//
int AUD_LiBabbling( int lbid )
{
	if ( !AUX_ENABLE_FUNKY_AUDIO_COMMENTS )
		return TRUE;

	//TODO:

	return TRUE;
}


// tables for different types of jim comment samples --------------------------
//
struct jim_comment_type_s {

	int		sci;
	int		probability;
	int		delay;
	int		_mksiz16;
};

#define NUM_JIM_ANGRY		4
jim_comment_type_s jim_angry[ NUM_JIM_ANGRY ] = {
	{ SCI_JIM_COMMENT_AFRAID_1,		25,		0		},
	{ SCI_JIM_COMMENT_AFRAID_2,		25,		0		},
	{ SCI_JIM_COMMENT_ANGRY_1,		25,		0		},
	{ SCI_JIM_COMMENT_ANGRY_2,		25,		0		},
};

#define NUM_JIM_WEAPON		2
jim_comment_type_s jim_weapon[ NUM_JIM_WEAPON ] = {
	{ SCI_JIM_COMMENT_WEAPON_1,		50,		1000	},
	{ SCI_JIM_COMMENT_WEAPON_2,		50,		1000	},
};

#define NUM_JIM_KILLED		2
jim_comment_type_s jim_killed[ NUM_JIM_KILLED ] = {
	{ SCI_JIM_COMMENT_KILLED_1,		50,		300		},
	{ SCI_JIM_COMMENT_KILLED_2,		50,		300		},
};

#define NUM_JIM_EATTHIS		2
jim_comment_type_s jim_eatthis[ NUM_JIM_EATTHIS ] = {
	{ SCI_JIM_COMMENT_EAT_THIS_1,	50,		600		},
	{ SCI_JIM_COMMENT_EAT_THIS_2,	50,		600		},
};

#define NUM_JIM_ENERGY		1
jim_comment_type_s jim_energy[ NUM_JIM_ENERGY ] = {
	{ SCI_JIM_COMMENT_ENERGY,		100,	800		},
};

#define NUM_JIM_BORED		2
jim_comment_type_s jim_bored[ NUM_JIM_BORED ] = {
	{ SCI_JIM_COMMENT_BORED_1,		50,		0		},
	{ SCI_JIM_COMMENT_BORED_2,		50,		0		},
};


// standard random comment selection function ---------------------------------
//
PRIVATE
jim_comment_type_s *SelectCommentRandomly( jim_comment_type_s *tab, int tablen )
{
	int selrand = RAND() % 100;
	int sumrand = 0;

	for ( int sid = 0; sid < tablen; sid++ ) {
		if ( selrand < ( tab[ sid ].probability + sumrand ) )
			return &tab[ sid ];
		sumrand += tab[ sid ].probability;
	}

	return NULL;
}


// select angry comment depending on local damage -----------------------------
//
PRIVATE
jim_comment_type_s *SelectCommentAngry( jim_comment_type_s *tab, int tablen )
{
	ASSERT( NUM_JIM_ANGRY == 4 );

	int selrand   = RAND() % 100;
	int threshold = (int)( (float)MyShip->MaxDamage * 0.80f );

	if ( MyShip->CurDamage < threshold ) {
		return ( selrand < 50 ) ? &jim_angry[ 0 ] : &jim_angry[ 1 ];
	} else {
		return ( selrand < 50 ) ? &jim_angry[ 2 ] : &jim_angry[ 3 ];
	}
}


// table for jim comment samples ----------------------------------------------
//
struct jim_comment_s {

	int					probability;
	int					numentries;
	jim_comment_type_s	*table;
	jim_comment_type_s	*(*selfunc)( jim_comment_type_s *tab, int tablen );
};

jim_comment_s jim_comments[] = {

	{ 40,	NUM_JIM_ANGRY,		jim_angry,		SelectCommentAngry		},
	{ 100,	NUM_JIM_WEAPON,		jim_weapon,		SelectCommentRandomly	},
	{ 100,	NUM_JIM_KILLED,		jim_killed,		SelectCommentRandomly	},
	{ 100,	NUM_JIM_EATTHIS,	jim_eatthis,	SelectCommentRandomly	},
	{ 100,	NUM_JIM_ENERGY,		jim_energy,		SelectCommentRandomly	},
	{ 60,	NUM_JIM_BORED,		jim_bored,		SelectCommentRandomly	},
};

#define NUM_JIM_COMMENTS	CALC_NUM_ARRAY_ENTRIES( jim_comments )


// play jim comment sample ----------------------------------------------------
//
int AUD_JimCommenting( int jcid )
{
	ASSERT( ( jcid >= 0 ) && ( jcid < JIM_COMMENT_NUM_COMMENTS ) );
	ASSERT( NUM_JIM_COMMENTS == JIM_COMMENT_NUM_COMMENTS );

	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return TRUE;

	if ( !AUX_ENABLE_FUNKY_AUDIO_COMMENTS )
		return TRUE;

	// probability that any comment will be played
	int decisionrand = RAND() % 100;
	if ( decisionrand >= jim_comments[ jcid ].probability )
		return TRUE;

	// let callback select the comment
	jim_comment_type_s *dstcom =
		(*jim_comments[ jcid ].selfunc)( jim_comments[ jcid ].table, jim_comments[ jcid ].numentries );
	if ( dstcom == NULL )
		return TRUE;

	// play the comment sample
	if ( dstcom->delay == 0 ) {

		AUD_PlaySampleOnChannel( &sample_channel_infos[ dstcom->sci ] );

	} else {

		event_s *event = (event_s *) ALLOCMEM( sizeof ( event_s ) );

		event->type				= EVT_TYPE_SND_COUNTDOWN;
		event->callback			= (event_callback_t) AUD_PlaySampleOnChannel;
		event->callback_params	= &sample_channel_infos[ dstcom->sci ];
		event->refframe_delay	= dstcom->delay;
		event->flags			= EVENT_PARAM_AUTOFREE |
								  EVENT_PARAM_AUTOTRIGGER |
								  EVENT_PARAM_ONE_PER_TYPE;
		EVT_AddEvent( event );
	}

	return TRUE;
}


// play jim babble sample -----------------------------------------------------
//
int AUD_JimBabbling( int jbid )
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS )
		return TRUE;

	if ( !AUX_ENABLE_FUNKY_AUDIO_COMMENTS )
		return TRUE;

	//TODO:

	return TRUE;
}


// play "amazing" sample ------------------------------------------------------
//
int AUD_SkillAmazing()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_SKILL_AMAZING ] );
	return TRUE;
}


// play "brilliant" sample ----------------------------------------------------
//
int AUD_SkillBrilliant()
{
	AUD_PlaySampleOnChannel( &sample_channel_infos[ SCI_SKILL_BRILLIANT ] );
	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE_INIT( AUD_GAME )
{

#ifdef USE_SOUNDSOURCE_LIST

	for( int i = 0; i < NUMCHANNELS; i++ ) {
		g_pDistVolumeInfo[ i ] = NULL;
	}

#endif // USE_SOUNDSOURCE_LIST

}


// module deregistration function ---------------------------------------------
//
REGISTER_MODULE_KILL( AUD_GAME )
{

#ifdef USE_SOUNDSOURCE_LIST

	// remove all remaining DistVolumeInfos
	for ( int i = 0; i < NUMCHANNELS; i++ ) {
		DistVolumeInfo_s* pCurrent = g_pDistVolumeInfo[ i ];

		for( ; pCurrent != NULL; ) {

			DistVolumeInfo_s* pNext = pCurrent->next;
			FREEMEM( pCurrent );
			pCurrent = pNext;

		}
		g_pDistVolumeInfo[ i ] = NULL;
	}

#endif // USE_SOUNDSOURCE_LIST

}



