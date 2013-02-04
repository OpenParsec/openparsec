/*
 * PARSEC - 
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:33 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   1999

  Updated by:
    Copyright (c) 2003 Sivaram Velauthapillai (sivaram33@hotmail.com or koalabear33@yahoo.com) 
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

/*
UPDATES:
August 10, 2003
  + MP3 support dropped; instead of loading mp3 files, game now loads ogg files
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
#include "aud_bkgn.h"

// proprietary module headers
#include "aud_game.h"
#include "aud_supp.h"
#include "con_arg.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_int.h"
#include "con_main.h"
#include "e_callbk.h"
#include "e_events.h"
#include "g_supp.h"
#include "snd_api.h"
#include "sys_file.h"


#define NUM_AUDIO_CD_TRACKS 9
#define NUM_GAMESFX_STREAMS 9


// defined background item types ----------------------------------------------
//
#define BKGN_ITEM_TYPE_SAMPLE		0x0001
#define BKGN_ITEM_TYPE_CDTRACK		0x0002
#define BKGN_ITEM_TYPE_STREAM		0x0004
#define BKGN_ITEM_TYPE_SILENCE		0x0008


// generic background item structure ------------------------------------------
//
struct BKGN_Item_s {
	BKGN_Item_s*	next;
	dword			type;			// type of background item
	dword			id;				// id (sampleid, cdtrack, ...) of background item
	char*			filename;		// filename if item is an audiostream
	int				groupnext;		// don't shuffle after completing this item
	int				fadein_start;	// seconds to start the fadein
	int				fadein_end;		// seconds to end the fadein (full volume)
	int				fadeout_start;	// seconds to start the fadeout
	int				fadeout_end;	// seconds to end the fadeout
	int				_mksiz32;
};


// module local globals -------------------------------------------------------
//
static BKGN_Item_s*		BKGN_items;
static BKGN_Item_s*		BKGN_items_head;
static int				BKGN_nVoice;
static int				BKGN_nNumItems;
static int				BKGN_nCurrentItem;
static int				BKGN_Started;
static int				BKGN_shuffle = TRUE;




// console texts --------------------------------------------------------------
//
static char invalid_command[]          = "invalid command.";
static char invalid_sample_specified[] = "invalid sample specified.";
static char invalid_stream_specified[] = "invalid stream specified.";
static char invalid_parameter[]		   = "invalid parameter.";


// paste string area ----------------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// get an item specified by an index ------------------------------------------
//
BKGN_Item_s* BKGN_GetItem( int nItem )
{
	int nIndex = 0;
	for ( BKGN_Item_s* scan = BKGN_items_head; scan != NULL; nIndex++ ) {
		BKGN_Item_s* next = scan->next;

		if ( nItem == nIndex ) {
			return scan;
		}

		scan = next;
	}

	return NULL;
}

// Initializes background music files
//RAM replaced mp3 with ogg vorbis files
void AUD_BackGroundPlayer_Init( int voice )
{
	BKGN_nVoice			= voice;
	BKGN_nNumItems		= 0;
	BKGN_nCurrentItem 	= -1;
	BKGN_Started        = FALSE;
	BKGN_items			= NULL;
	BKGN_items_head		= NULL;

	if ( !AUX_DISABLE_BACKGROUNDPLAYER_ITEM_FILL ) {

		// add music tracks (either from audio cd or as mp3)
		for ( int tid = 2; tid < NUM_AUDIO_CD_TRACKS + 2; tid++ ) {

				sprintf( paste_str, "track%d.mp3", tid );
				AUD_BackGroundPlayer_AddStream( paste_str );
        
		}

		// add game sfx
		for ( int sid = 1; sid < NUM_GAMESFX_STREAMS + 1; sid++ ) {

			sprintf( paste_str, "gamesfx%d.mp3", sid );
			AUD_BackGroundPlayer_AddStream( paste_str );
      
		}

		// new game sfx in pscdata3.dat
		AUD_BackGroundPlayer_AddStream( "gamefx10.mp3" );
		AUD_BackGroundPlayer_AddStream( "gamefx11.mp3" );
    
    //????RAM not sure what this silence stuff is ????
		// add some silence
		AUD_BackGroundPlayer_AddSilence( 15 );
		AUD_BackGroundPlayer_AddSilence( 15 );
		AUD_BackGroundPlayer_AddSilence( 30 );
	}

}


// kill the background sound player -------------------------------------------
//
void AUD_BackGroundPlayer_Kill()
{
	for ( BKGN_Item_s* scan = BKGN_items_head; scan != NULL; ) {
		BKGN_Item_s* next = scan->next;
		FREEMEM( scan );
		scan = next;
	}

	BKGN_nNumItems		= 0;
	BKGN_nCurrentItem 	= -1;
	BKGN_Started        = FALSE;
	BKGN_items_head		= NULL;
	BKGN_items			= NULL;
}


// remove an item from the background list ------------------------------------
//
PRIVATE
BKGN_Item_s* BKGNm_AddItem( int id, const char *filename, dword type )
{
	// add the background item to the end of the list

  DBGTXT(MSGOUT("Adding Item with id=%i, filename=%s, type=%d\n", id, filename, type););
	BKGN_Item_s* item = (BKGN_Item_s*) ALLOCMEM( sizeof( BKGN_Item_s ) );
	memset( item, 0, sizeof( BKGN_Item_s ) );

	item->type			= type;

	if ( type == BKGN_ITEM_TYPE_STREAM ) {

		item->id		= 0;
		item->filename	= (char *) ALLOCMEM( strlen( filename ) + 1 );
		strcpy( item->filename, filename );

	} else {

		item->id		= id;
		item->filename	= NULL;
	}

	item->groupnext 	= FALSE;
	item->next			= NULL;

	if ( BKGN_items_head == NULL ) {
		BKGN_items_head = item;
	}

	if ( BKGN_items != NULL ) {
		BKGN_items->next = item;
	}

	BKGN_items = item;

	// increase the number of attached items
	BKGN_nNumItems++;

	return item;
}


// remove an item from the background list ------------------------------------
//
PRIVATE
int BKGNm_RemoveItem( dword id, char *filename, dword type )
{
	int nNumItems = BKGN_nNumItems;

	BKGN_Item_s* prev = NULL;
	for ( BKGN_Item_s* scan = BKGN_items_head; scan != NULL; ) {
		BKGN_Item_s* next = scan->next;

		if ( scan->type & type ) {
			if ( ( scan->id == id ) && ( scan->filename == filename ) ) {

				if ( prev != NULL ) {
					prev->next = next;
				} else {
					BKGN_items_head = next;
				}

				if ( scan->filename != NULL )
					FREEMEM( scan->filename );

				FREEMEM( scan );
				scan = NULL;

				BKGN_nNumItems--;
			}
		}

		if ( scan != NULL )
			prev = scan;

		scan = next;
	}

	return ( nNumItems != BKGN_nNumItems );
}


// add a sample to be played by the background player -------------------------
//
int AUD_BackGroundPlayer_AddSample( const char *name )
{
	if ( name != NULL ) {
		// fetch the named sample
		int sampleid;
		sampleinfo_s* pSampleInfo = AUD_FetchSampleByName( name, &sampleid );

		if ( pSampleInfo != NULL ) {
			BKGN_Item_s* item = BKGNm_AddItem( sampleid, NULL, BKGN_ITEM_TYPE_SAMPLE );
		}
		return TRUE;
	}

	return FALSE;
}


// remove a sample from the list of played items ------------------------------
//
int	AUD_BackGroundPlayer_RemoveSample( char *name )
{
	if ( name != NULL ) {

		// fetch the named sample
		int sampleid;
		sampleinfo_s* pSampleInfo = AUD_FetchSampleByName( name, &sampleid );

		// check the whole list and delete the item if needed
		if ( pSampleInfo != NULL ) {

			// stop playing if removed item is the currently played
			BKGN_Item_s* curitem = BKGN_GetItem( BKGN_nCurrentItem );

			if ( ( curitem->id == (dword) sampleid ) && ( curitem->type & BKGN_ITEM_TYPE_SAMPLE ) ) {
				AUD_BackGroundPlayer_Stop();
				EVT_RemoveEventType( EVT_TYPE_BKGN_SAMPLE_END );
				EVT_RemoveEventType( EVT_TYPE_BKGN_CDTRACK_END );
			}

			// remove the item
			int rc = BKGNm_RemoveItem( sampleid, NULL, BKGN_ITEM_TYPE_SAMPLE );

			// and start playing again
			AUD_BackGroundPlayer_Start( FALSE );

			return rc;
		}
	}

	return FALSE;
}


// add a cd-track to be played by the background player -----------------------
//
int	AUD_BackGroundPlayer_AddTrack( int track )
{
	BKGN_Item_s* item = BKGNm_AddItem( track, NULL, BKGN_ITEM_TYPE_CDTRACK );

	return TRUE;
}


// remove a cd-track from the list of played items ----------------------------
//
int	AUD_BackGroundPlayer_RemoveTrack( int track )
{
	// stop playing if removed item is the currently played
	BKGN_Item_s* curitem = BKGN_GetItem( BKGN_nCurrentItem );

	if ( ( curitem->id == (dword) track ) && ( curitem->type & BKGN_ITEM_TYPE_CDTRACK ) ) {
		AUD_BackGroundPlayer_Stop();
		EVT_RemoveEventType( EVT_TYPE_BKGN_SAMPLE_END );
		EVT_RemoveEventType( EVT_TYPE_BKGN_CDTRACK_END );
	}

	// remove the item
	int rc = BKGNm_RemoveItem( track, NULL, BKGN_ITEM_TYPE_CDTRACK );

	// and start playing again
	AUD_BackGroundPlayer_Start( FALSE );

	return rc;
}



// add a sample to be played by the background player -------------------------
//
int AUD_BackGroundPlayer_AddStream( const char *name )
{
	if ( name != NULL ) {

		BKGN_Item_s* item = BKGNm_AddItem( 0, name, BKGN_ITEM_TYPE_STREAM );

		return TRUE;
	}

	return FALSE;
}


// add a silence delay to the background player list --------------------------
//
int AUD_BackGroundPlayer_AddSilence( int seconds )
{
	BKGN_Item_s* item = BKGNm_AddItem( seconds, NULL, BKGN_ITEM_TYPE_SILENCE );

	return TRUE;
}


// remove an item from the list -----------------------------------------------
//
int	AUD_BackGroundPlayer_RemoveItem( int item )
{

	// stop playing if removed item is the currently played
	if ( BKGN_nCurrentItem == item ) {
		AUD_BackGroundPlayer_Stop();
		EVT_RemoveEventType( EVT_TYPE_BKGN_SAMPLE_END );
		EVT_RemoveEventType( EVT_TYPE_BKGN_CDTRACK_END );
	}

	// get the item
	BKGN_Item_s* curitem = BKGN_GetItem( item );

	// remove the item
	int rc = BKGNm_RemoveItem( curitem->id, curitem->filename, curitem->type );

	// and start playing again
	AUD_BackGroundPlayer_Start( FALSE );

	return rc;
}


// choose a new item to play in the background --------------------------------
//
INLINE
int BKGNm_ChooseNewItem()
{
	ASSERT( BKGN_nNumItems > 1 );

	int nNewSample;

	// get current item
	BKGN_Item_s* item = BKGN_GetItem( BKGN_nCurrentItem );

	int group = item ? item->groupnext : FALSE;
	if ( BKGN_shuffle && !group ) {

		nNewSample = ( RAND() % BKGN_nNumItems );

		while ( nNewSample == BKGN_nCurrentItem ) {
			nNewSample = ( RAND() % BKGN_nNumItems );
		}

	} else {

		nNewSample = ( BKGN_nCurrentItem + 1 ) % BKGN_nNumItems;
	}

	return nNewSample;
}

// play the next item ---------------------------------------------------------
//
int BKGN_StartNextItem( void* dummy )
{
	AUD_BackGroundPlayer_Start( TRUE );

	return TRUE;
}


// play a sample --------------------------------------------------------------
//
PRIVATE
void BKGNm_PlaySample( BKGN_Item_s* item )
{
	SoundParams_s soundParams;
	memset( &soundParams, 0, sizeof( SoundParams_s ) );

	if ( BKGN_nNumItems == 1 ) {
		// if only one sample, then we loop that forever
		soundParams.flags = SOUNDPARAMS_LOOP;
		soundParams.start = 0;
	}

	sampleinfo_s* pSampleInfo = AUD_FetchSampleById( item->id );

	soundParams.volume = pSampleInfo->volume;
	soundParams.flags |= SOUNDPARAMS_VOLUME;

	// retrieve the wave from the sampleinfo
	audiowave_t wave = pSampleInfo->samplepointer;
	AUDs_PlayVoiceBuffer(BKGN_nVoice, wave, &soundParams);

	// add an event to start the next item, after the sample play has stopped
	event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );

	event->type				= EVT_TYPE_BKGN_SAMPLE_END;
	event->callback			= BKGN_StartNextItem;
	event->callback_params	= NULL;
	event->refframe_delay	= pSampleInfo->size;
	event->flags			= EVENT_PARAM_ONESHOT |
							  EVENT_PARAM_AUTOFREE |
							  EVENT_PARAM_AUTOTRIGGER;

	EVT_AddEvent( event );
}


// play a cd track ------------------------------------------------------------
//
PRIVATE
void BKGNm_PlayTrack( BKGN_Item_s* item )
{
	/*
	int tracklength = AUDs_CDGetTrackLength( (short)item->id );

	if ( tracklength != -1 ) {

		AUDs_CDPlay( (short)item->id, -1, -1 );

		// add an event to start the next item, after the cdtrack play has stopped
		event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );

		event->type				= EVT_TYPE_BKGN_CDTRACK_END;
		event->callback			= BKGN_StartNextItem;
		event->callback_params	= NULL;

		event->refframe_delay	= FRAME_MEASURE_TIMEBASE * tracklength;
		event->flags			= EVENT_PARAM_ONESHOT |
								  EVENT_PARAM_AUTOFREE |
								  EVENT_PARAM_AUTOTRIGGER;

		EVT_AddEvent( event );

	} else {

		BKGN_StartNextItem( NULL );
	}*/

}


// play an audio stream -------------------------------------------------------
//
PRIVATE
void BKGNm_PlayStream( BKGN_Item_s* item )
{
	if ( AUDs_PlayAudioStream( item->filename ) ) {

//		MSGOUT( "playing stream %s", item->filename );

		// specify callback type and flags
//		int callbacktype = CBTYPE_STREAM_ENDED | CBFLAG_REMOVE;

		// ensure no dangling callbacks are left around, they might add up
//		CALLBACK_DeleteCallback( callbacktype, BKGN_callback_id );

		// register callback to play next item, after the stream has ended
//		BKGN_callback_id =
//			CALLBACK_RegisterCallback( callbacktype, BKGN_StartNextItem, (void*) NULL );

	} else {

//		MSGOUT( "Can't play stream %s", item->filename );
		BKGN_StartNextItem( NULL );
	}
}

// play an audio stream -------------------------------------------------------
//
PRIVATE
void BKGNm_PlaySilence( BKGN_Item_s* item )
{
	// add an event to start the next item, after the silence time has elapsed
	event_s* event = (event_s*) ALLOCMEM( sizeof ( event_s ) );

	event->type				= EVT_TYPE_BKGN_SILENCE_END;
	event->callback			= BKGN_StartNextItem;
	event->callback_params	= NULL;
	event->refframe_delay	= FRAME_MEASURE_TIMEBASE * item->id;
	event->flags			= EVENT_PARAM_ONESHOT |
							  EVENT_PARAM_AUTOFREE |
							  EVENT_PARAM_AUTOTRIGGER;

	EVT_AddEvent( event );

}


// start playing the background sound by randomly picking one of the samples --
//
int AUD_BackGroundPlayer_Start( int fromevent )
{
	if ( DISABLE_MUSIC_FUNCTIONS )
		return FALSE;

	if ( BKGN_nNumItems > 0 ) {
		if(fromevent!=42){
		if ( BKGN_Started ) {
			AUD_BackGroundPlayer_Stop();
			if ( !fromevent ) {
				EVT_RemoveEventType( EVT_TYPE_BKGN_SAMPLE_END );
				EVT_RemoveEventType( EVT_TYPE_BKGN_CDTRACK_END );
			}
		}
		}

		if ( BKGN_nNumItems == 1 ) {
			BKGN_nCurrentItem = 0;
		} else {
			BKGN_nCurrentItem = BKGNm_ChooseNewItem();
		}

		BKGN_Item_s* item = BKGN_GetItem( BKGN_nCurrentItem );

		if ( item != NULL ) {
			switch ( item->type ) {

				case BKGN_ITEM_TYPE_SAMPLE:
					BKGNm_PlaySample( item );
					break;

				case BKGN_ITEM_TYPE_CDTRACK:
					BKGNm_PlayTrack( item );
					break;

				case BKGN_ITEM_TYPE_STREAM:
						BKGNm_PlayStream( item );
					break;

				case BKGN_ITEM_TYPE_SILENCE:
					BKGNm_PlaySilence( item );
					break;

				default:
					break;
			}

			BKGN_Started = TRUE;

			return TRUE;
		}
	}

	return FALSE;

}


// ----------------------------------------------------------------------------
//
extern int FloatingMenuStatusWindowActive();


// maintain background sound playing ------------------------------------------
//
int AUD_BackGroundPlayer_Maintain()
{
	if ( DISABLE_AUDIO_FUNCTIONS || AUX_DISABLE_SOUNDEFFECTS ) {
		// stop playing the background sound if currently playing
		if ( BKGN_Started ) {
			AUD_BackGroundPlayer_Stop();
			EVT_RemoveEventType( EVT_TYPE_BKGN_SAMPLE_END );

			EVT_RemoveEventType( EVT_TYPE_BKGN_CDTRACK_END );
		}

		return FALSE;
	}

	// maintain the background sound
	if ( AUX_AUTOSTART_BACKGROUNDPLAYER ) {

		if ( GAME_MODE_ACTIVE() && InGameLoop ) {

			// start background sound, if in game loop, and not yet started
			if ( !AUD_BackGroundPlayer_IsPlaying() ) {
				AUD_BackGroundPlayer_Start( FALSE );
				return TRUE;
			}

		} else if ( !FloatingMenuStatusWindowActive() ) {

			// stop background sound, if out of game loop
			if ( AUD_BackGroundPlayer_IsPlaying() ) {
				AUD_BackGroundPlayer_Stop();
				EVT_RemoveEventType( EVT_TYPE_BKGN_SAMPLE_END );
				EVT_RemoveEventType( EVT_TYPE_BKGN_CDTRACK_END );
				return TRUE;
			}
		}
	}

	return TRUE;
}


// list all samples added to the list of background samples -------------------
//
void AUD_BackGroundPlayer_List()
{
	int nItem = 0;
	char current;

	for ( BKGN_Item_s* scan = BKGN_items_head; scan != NULL; nItem++) {
		BKGN_Item_s* next = scan->next;

		if ( ( nItem == BKGN_nCurrentItem ) && BKGN_Started ) {

			current = '*';

		} else if ( scan->groupnext ) {

			current = '|';

		} else {

			current = ' ';
		}

		switch( scan->type ) {

			case BKGN_ITEM_TYPE_SAMPLE:
				{
					sampleinfo_s* pSampleInfo = AUD_FetchSampleById( scan->id );
					sprintf( paste_str, "%c [%02d] sample id %u name (%s) volume %u",
							 current, nItem, (unsigned int)scan->id,
							 pSampleInfo->name, (unsigned int)pSampleInfo->volume );
					CON_AddLine( paste_str );
				}

				break;

			/*case BKGN_ITEM_TYPE_CDTRACK:
				{
					int tracklength = AUDs_CDGetTrackLength( (short)scan->id );
					if ( tracklength != -1 ) {
						int mins = tracklength / 60;
						int secs = tracklength - ( mins * 60 );

						sprintf( paste_str, "%c [%02d] track %02d length %02d:%02d", current, nItem,
								 scan->id, mins, secs );
					} else {

						sprintf( paste_str, "%c [%02d] track %02d (no cd available)", current, nItem,
								 scan->id );

					}
					CON_AddLine( paste_str );
				}
				break;
			*/
			case BKGN_ITEM_TYPE_STREAM:
				{
					sprintf( paste_str, "%c [%02d] stream %s", current, nItem, scan->filename );
					CON_AddLine( paste_str );
				}
				break;

			case BKGN_ITEM_TYPE_SILENCE:
				{
					sprintf( paste_str, "%c [%02d] silence %u seconds", current, nItem, (unsigned int)scan->id );
					CON_AddLine( paste_str );
				}
				break;

			default:
				break;
		}

		scan = next;
	}
}


// stop playing the backgroundplayer ------------------------------------------
//
int AUD_BackGroundPlayer_Stop()
{
	BKGN_Item_s* item = BKGN_GetItem( BKGN_nCurrentItem );

	if ( item != NULL ) {
		switch( item->type ) {

			case BKGN_ITEM_TYPE_SAMPLE:
				AUDs_StopVoice( BKGN_nVoice, FALSE );
				break;

			/*case BKGN_ITEM_TYPE_CDTRACK:
				AUDs_CDStop();
				break;
			*/
			case BKGN_ITEM_TYPE_STREAM:
				AUDs_StopAudioStream();
				break;

			case BKGN_ITEM_TYPE_SILENCE:
				// nothing needs to be done to stop silence
				break;

			default:
				break;
		}

		BKGN_Started = FALSE;

		return TRUE;
	}

	return FALSE;
}


// check whether a sample is playing ------------------------------------------
//
int AUD_BackGroundPlayer_IsPlaying()
{
	return BKGN_Started;
}


// console command for starting the background sample playing -----------------
//
PRIVATE
int Cmd_BKGN_START( char *bkgnstr )
{
	ASSERT( bkgnstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( bkgnstr );

	// start playing the background sound
	AUD_BackGroundPlayer_Start( FALSE );

	return TRUE;
}


// console command for stopping the background sample playing -----------------
//
PRIVATE
int Cmd_BKGN_STOP( char *bkgnstr )
{
	ASSERT( bkgnstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( bkgnstr );

	// stop playing the background sound
	AUD_BackGroundPlayer_Stop();
	EVT_RemoveEventType( EVT_TYPE_BKGN_SAMPLE_END );
	EVT_RemoveEventType( EVT_TYPE_BKGN_CDTRACK_END );

	return TRUE;
}


// key table for background sound loading -------------------------------------
//
static key_value_s bkgn_add_key_value[] = {

	{ "sample",		NULL,	KEYVALFLAG_NONE | KEYVALFLAG_PARENTHESIZE },
	{ "track",		NULL,   KEYVALFLAG_NONE },
	{ "stream",		NULL,   KEYVALFLAG_NONE },
	{ "silence",	NULL,   KEYVALFLAG_NONE },



	{ NULL,			NULL,	KEYVALFLAG_NONE	},
};

enum {

	KEY_BKGN_ADD_SAMPLE,
	KEY_BKGN_ADD_TRACK,
	KEY_BKGN_ADD_STREAM,
	KEY_BKGN_ADD_SILENCE
};


// console command for adding a background sample -----------------------------
//
PRIVATE
int Cmd_BKGN_ADD( char *bkgnstr )
{
	//NOTE:
	//CONCOM:
	// bkgn_add_command		::= 'bkgn.add' <itemspec>
	// itemspec				::= <samplespec> | <trackspec> |
	//							<streamspec> | <silencespec>
	// samplespec			::= 'sample' <paranthesized name of sample>
	// trackspec			::= 'track' <trackno>
	// streamspec			::= 'stream' <filename>
	// silencespec			::= 'silence' <seconds>

	ASSERT( bkgnstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( bkgnstr );

	// scan out all values to bkgn keys
	if ( !ScanKeyValuePairs( bkgn_add_key_value, bkgnstr ) )
		return TRUE;

	char *samplename = bkgn_add_key_value[ KEY_BKGN_ADD_SAMPLE ].value;

	char *streamname = bkgn_add_key_value[ KEY_BKGN_ADD_STREAM ].value;

	int track = -1;

	if( bkgn_add_key_value[ KEY_BKGN_ADD_TRACK ].value != NULL ) {
		ScanKeyValueInt( &bkgn_add_key_value[ KEY_BKGN_ADD_TRACK ], &track );
	}

	int silenceseconds = -1;

	if( bkgn_add_key_value[ KEY_BKGN_ADD_SILENCE ].value != NULL ) {
		ScanKeyValueInt( &bkgn_add_key_value[ KEY_BKGN_ADD_SILENCE ], &silenceseconds );

	}

	//FIXME: need to split this in bkgn.add.sample and bkgn.add.track
	if( track != -1 ) {

		// add a track to the list of background items
		if( AUD_BackGroundPlayer_AddTrack( track ) == FALSE ) {
			CON_AddLine( invalid_parameter );
			return TRUE;
		}

	} else {

		if ( samplename != NULL ) {

			// add a sample to the list of background items
			if( AUD_BackGroundPlayer_AddSample( samplename ) == FALSE ) {
				CON_AddLine( invalid_sample_specified );
				return TRUE;
			}

		} else if ( streamname != NULL ) {

			// add a stream to the list of background items
			if( AUD_BackGroundPlayer_AddStream( streamname ) == FALSE ) {
				CON_AddLine( invalid_stream_specified );
				return TRUE;
			}

		} else if ( silenceseconds != -1 ) {

			// add silence to the list of background items
			if( AUD_BackGroundPlayer_AddSilence( silenceseconds ) == FALSE ) {
				CON_AddLine( invalid_parameter );
				return TRUE;
			}
		}
	}

	return TRUE;
}


// key table for background sound loading -------------------------------------
//
static key_value_s bkgn_rmv_key_value[] = {

	{ "sample",		NULL,	KEYVALFLAG_NONE | KEYVALFLAG_PARENTHESIZE },
	{ "track",		NULL,   KEYVALFLAG_NONE },
	{ "item",		NULL,   KEYVALFLAG_NONE },

	{ NULL,			NULL,	KEYVALFLAG_NONE	},
};

enum {

	KEY_BKGN_RMV_SAMPLE,
	KEY_BKGN_RMV_TRACK,
	KEY_BKGN_RMV_ITEM
};


// console command for adding a background sample -----------------------------
//
PRIVATE
int Cmd_BKGN_REMOVE( char *bkgnstr )
{
	//NOTE:
	//CONCOM:
	// bkgn_remove_command	::= 'bkgn.remove' <itemspec>
	// itemspec				::= <idspec> | <samplespec> | <trackspec>
	// idspec				::= 'id' <id of item>
	// samplespec			::= 'sample' <paranthesized name of sample>
	// trackspec			::= 'track' <trackno>

	ASSERT( bkgnstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( bkgnstr );

	// scan out all values to bkgn keys
	if ( !ScanKeyValuePairs( bkgn_rmv_key_value, bkgnstr ) )
		return TRUE;

	char *samplename = bkgn_rmv_key_value[ KEY_BKGN_RMV_SAMPLE ].value;
	int track = -1;

	if ( bkgn_rmv_key_value[ KEY_BKGN_RMV_TRACK ].value != NULL ) {
		ScanKeyValueInt( &bkgn_rmv_key_value[ KEY_BKGN_RMV_TRACK ], &track);
	}

	int item = -1;
	if ( bkgn_rmv_key_value[ KEY_BKGN_RMV_ITEM ].value != NULL ) {
		ScanKeyValueInt( &bkgn_rmv_key_value[ KEY_BKGN_RMV_ITEM ], &item );
	}

	if ( item != -1 ) {

		// remove a track from the list of background items
		if ( !AUD_BackGroundPlayer_RemoveItem( item ) ) {
			CON_AddLine( invalid_parameter );
			return TRUE;

		}

	} else {

		//NOTE:
		// these are only here for legacy support

		if ( track != -1 ) {

			// remove a track from the list of background items
			if ( !AUD_BackGroundPlayer_RemoveTrack( track ) ) {
				CON_AddLine( invalid_parameter );
				return TRUE;
			}

		} else {

			// remove a sample from the list of background samples
			if ( !AUD_BackGroundPlayer_RemoveSample( samplename ) ) {
				CON_AddLine( invalid_sample_specified );
				return TRUE;
			}
		}

	}

	return TRUE;

}


// console command for listing all the background samples ---------------------
//
PRIVATE
int Cmd_BKGN_LIST( char *bkgnstr )
{
	//NOTE:
	//CONCOM:
	// bkgn_list_command ::= 'bkgn.list'

	ASSERT( bkgnstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( bkgnstr );

	// list all added samples
	AUD_BackGroundPlayer_List();

	return TRUE;
}


// key table for item grouping ------------------------------------------------
//
static key_value_s bkgn_group_key_value[] = {


	{ "item",		NULL,   KEYVALFLAG_MANDATORY },

	{ NULL,			NULL,	KEYVALFLAG_NONE	},
};

enum {

	KEY_BKGN_ITEM
};


// console command for grouping two tracks ------------------------------------
//
PRIVATE
int Cmd_BKGN_GROUP( char *bkgnstr )
{
	//NOTE:
	//CONCOM:
	// bkgn_group_command	::= 'bkgn.group' <itemspec>
	// itemspec				::= 'item' <id of item>

	ASSERT( bkgnstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( bkgnstr );

	// scan out all values to bkgn keys
	if ( !ScanKeyValuePairs( bkgn_group_key_value, bkgnstr ) )
		return TRUE;

	int item = -1;

	if( bkgn_group_key_value[ KEY_BKGN_ITEM ].value != NULL ) {
		ScanKeyValueInt( &bkgn_group_key_value[ KEY_BKGN_ITEM ], &item);
	}

	// group specified item with next in list
	if ( item != -1 ) {

		// get item
		BKGN_Item_s* listitem = BKGN_GetItem( item );
		
		ASSERT(listitem != NULL);

		if ( listitem->groupnext ) {

			listitem->groupnext = FALSE;

		} else {

			listitem->groupnext = TRUE;
		}

	}

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( AUD_BKGN )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "bkgn.start" command
	regcom.command	 = "bkgn.start";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_BKGN_START;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "bkgn.stop" command
	regcom.command	 = "bkgn.stop";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_BKGN_STOP;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "bkgn.add" command
	regcom.command	 = "bkgn.add";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_BKGN_ADD;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "bkgn.remove" command
	regcom.command	 = "bkgn.remove";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_BKGN_REMOVE;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "bkgn.list" command
	regcom.command	 = "bkgn.list";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_BKGN_LIST;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "bkgn.group" command
	regcom.command	 = "bkgn.group";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_BKGN_GROUP;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "bkgn.shuffle" command
	int_command_s intregcom;
	memset( &intregcom, 0, sizeof( int_command_s ) );

	intregcom.command	= "bkgn.shuffle";
	intregcom.bmin	 	= 0;
	intregcom.bmax	 	= 1;
	intregcom.intref	= &BKGN_shuffle;
	intregcom.realize 	= NULL;
	intregcom.fetch 	= NULL;
	CON_RegisterIntCommand( &intregcom );
}

