/*
 * PARSEC HEADER: aud_supp.h
 */

#ifndef _AUD_SUPP_H_
#define _AUD_SUPP_H_


// sample-channel relation (holds id once sample is fetched by name) ----------
//
struct sample_channel_info_s {

	const char*	samplename;
	const char*	fallbacksample;
	int		sampleid;
	int		channel;
	int		interruptold;
};


// macro to fetch a wave for sample x -----------------------------------------
//
#define AUD_FETCHSAMPLE(x)	\
\
	audiowave_t fetch_wave = NULL; \
	sampleinfo_s* fetch_pSampleInfo; \
	SoundParams_s fetch_params; \
	memset( &fetch_params, 0, sizeof( fetch_params ) ); \
	static int sampleid = -1; \
	if ( sampleid == -1 ) { \
		fetch_pSampleInfo = AUD_FetchSampleByName((x),&sampleid); \
	} else { \
		fetch_pSampleInfo = AUD_FetchSampleById(sampleid); \
	} \
	if(fetch_pSampleInfo != NULL) { \
		fetch_wave = (audiowave_t)fetch_pSampleInfo->samplepointer; \
		fetch_params.volume = ( fetch_pSampleInfo->volume == -1 ) ? \
								AUD_MAX_VOLUME : fetch_pSampleInfo->volume; \
		fetch_params.flags = SOUNDPARAMS_VOLUME; \
	}


// macro to fetch a wave for sample x store id in y ---------------------------
//
#define AUD_FETCHSAMPLE_EX(x,y)	\
\
	audiowave_t fetch_wave = NULL; \
	sampleinfo_s* fetch_pSampleInfo; \
	SoundParams_s fetch_params; \
	memset( &fetch_params, 0, sizeof( fetch_params ) ); \
	if ( (y) == -1 ) { \
		fetch_pSampleInfo = AUD_FetchSampleByName((x),&(y)); \
	} else { \
		fetch_pSampleInfo = AUD_FetchSampleById(y); \
	} \
	if(fetch_pSampleInfo != NULL) { \
		fetch_wave = (audiowave_t)fetch_pSampleInfo->samplepointer; \
		fetch_params.volume = ( fetch_pSampleInfo->volume == -1 ) ? \
								AUD_MAX_VOLUME : fetch_pSampleInfo->volume; \
		fetch_params.flags = SOUNDPARAMS_VOLUME; \
	}


// macro to fetch a wave for sample x, fallback to sample y -------------------
//
#define AUD_FETCHSAMPLE_FALLBACK(x,y)	\
\
	audiowave_t fetch_wave = NULL; \
	sampleinfo_s* fetch_pSampleInfo; \
	SoundParams_s fetch_params; \
	memset( &fetch_params, 0, sizeof( fetch_params ) ); \
	static int sampleid = -1; \
	if ( sampleid == -1 ) { \
		fetch_pSampleInfo = AUD_FetchSampleByName( (x) ,&sampleid); \
		if(fetch_pSampleInfo == NULL) { \
			fetch_pSampleInfo = AUD_FetchSampleByName( (y) ,&sampleid); \
		} \
	} else { \
		fetch_pSampleInfo = AUD_FetchSampleById(sampleid); \
	} \
	if(fetch_pSampleInfo != NULL) { \
		fetch_wave = (audiowave_t)fetch_pSampleInfo->samplepointer; \
		fetch_params.volume = ( fetch_pSampleInfo->volume == -1 ) ? \
								AUD_MAX_VOLUME : fetch_pSampleInfo->volume; \
		fetch_params.flags = SOUNDPARAMS_VOLUME; \
	}


// macro to fetch a wave for sample x, fallback to sample z, store id in y ----
//
#define AUD_FETCHSAMPLE_FALLBACK_EX(x,y,z)	\
\
	audiowave_t fetch_wave = NULL; \
	sampleinfo_s* fetch_pSampleInfo; \
	SoundParams_s fetch_params; \
	memset( &fetch_params, 0, sizeof( fetch_params ) ); \
	if ( (y) == -1 ) { \
		fetch_pSampleInfo = AUD_FetchSampleByName( (x) ,&(y)); \
		if(fetch_pSampleInfo == NULL) { \
			fetch_pSampleInfo = AUD_FetchSampleByName( (z) ,&(y)); \
		} \
	} else { \
		fetch_pSampleInfo = AUD_FetchSampleById((y)); \
	} \
	if(fetch_pSampleInfo != NULL) { \
		fetch_wave = (audiowave_t)fetch_pSampleInfo->samplepointer; \
		fetch_params.volume = ( fetch_pSampleInfo->volume == -1 ) ? \
								AUD_MAX_VOLUME : fetch_pSampleInfo->volume; \
		fetch_params.flags = SOUNDPARAMS_VOLUME; \
	}


// encapsulate global audio disabling -----------------------------------------
//
#define DISABLE_AUDIO_FUNCTIONS		(!Op_SoundEffects)
#define DISABLE_MUSIC_FUNCTIONS		(!Op_Music)


// external functions ---------------------------------------------------------
//
sampleinfo_s*	AUD_FetchSampleByName( const char *name, int *id );
sampleinfo_s*	AUD_FetchSampleById( int id );
int				AUD_SetSampleVolume( int id, int volume );
int				AUD_PlaySampleOnChannel( sample_channel_info_s *info );


#endif // _AUD_SUPP_H_


