/*
 * PARSEC HEADER: aud_defs.h
 */

#ifndef _AUD_DEFS_H_
#define _AUD_DEFS_H_


// ----------------------------------------------------------------------------
// AUDIO SUBSYSTEM (AUD) related definitions                                  -
// ----------------------------------------------------------------------------


typedef void *audiowave_t;


extern int	SELECT_VOICE;
extern int	STREAM_VOICE_L;
extern int	STREAM_VOICE_R;

extern char console_audio_stream_path[];
extern char intro_stream_name[];
extern char menu_stream_name[];

// global volume between 0 and 255
extern int  GlobalVolume;

extern int	aud_sample_quality;

extern int  aud_do_open_menu_sound_on_aud_conf;


// audio subsystem constraints ------------------------------------------------
//
#define AUD_MAX_VOLUME				255
#define AUD_MIN_VOLUME				0
#define AUD_RANGE_VOLUME			( AUD_MAX_VOLUME - AUD_MIN_VOLUME + 1 )

#define NUMCHANNELS					32


// audio subsystem return codes -----------------------------------------------
//
#define AUD_RETURN_SUCCESS			0
#define AUD_RETURN_FAILURE			1


// flags for looping and selection of sound -----------------------------------
//
enum {

	SOUNDPARAMS_NONE			= 0x0000,
	SOUNDPARAMS_SELECTION		= 0x0001,	// play only a selection of the wave
	SOUNDPARAMS_LOOP			= 0x0002,	// loop whole wave, only selection or extended loop
	SOUNDPARAMS_LOOP_COUNT		= 0x0010,	// loop the sound numLoops times
	SOUNDPARAMS_VOLUME			= 0x0100,	// set the specified volume
	SOUNDPARAMS_STREAMWAVE		= 0x0200,	// play streamed wave
};


// parameters when playing a sound --------------------------------------------
//
struct SoundParams_s {

	long		start;				// sample to start playing
	long		end;				// sample to end playing
	long		startLoop;			// sample to rewind when looping
    long		endLoop;			// sample to end looping in all but the last loop
	long		numLoops;			// number of loops to play the sound
	int			volume;				// volume this sound has to be played with
	dword		flags;				// indicate which fields are set
};


// struct for handling distance relative volume changes -----------------------
//
struct DistVolumeInfo_s {

	DistVolumeInfo_s*	next;
	int					nChannel;
	int					nMaxVolume;
	int					nMinVolume;
	geomv_t				fMaxVolumeDist;			// distance for max. volume
	geomv_t				fMinVolumeDist;			// distance for min. volume
	float				fRolloffscale;			// scale for the rolloff inside the sound sphere
	GenObject*			pListener;
	GenObject*			pOriginator;
};


// ids for different types of jim comments ------------------------------------
//
enum {

	JIM_COMMENT_ANGRY,			// 0
	JIM_COMMENT_WEAPON,			// 1
	JIM_COMMENT_KILLED,			// 2
	JIM_COMMENT_EATTHIS,		// 3
	JIM_COMMENT_ENERGY,			// 4
	JIM_COMMENT_BORED,			// 5

	JIM_COMMENT_NUM_COMMENTS,
};


// ids for different types of thrust sounds -----------------------------------
//
enum {

	THRUST_TYPE_NORMAL,			// 0
	THRUST_TYPE_SLIDE,			// 1
};


// include system-specific subsystem prototypes -------------------------------
//
#include "aud_subh.h"


#endif // _AUD_DEFS_H_


