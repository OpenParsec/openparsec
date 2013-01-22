/*
 * PARSEC HEADER: snd_api.h
 */

#ifndef _SND_API_H_
#define _SND_API_H_


// recognized sample formats

enum sample_formats {

	SAMPLEFORMAT_INVALID,

	SAMPLEFORMAT_WAV,
	SAMPLEFORMAT_VOC,
	SAMPLEFORMAT_AU,
	SAMPLEFORMAT_SAM,
};


// generic sample info

struct sample_s {

	int		format;
	dword	totalfilelength;
	dword	headerlength;
	word	datatype;
	word	numchannels;
	dword	samplerate;
	word	alignment;
	word	samplesize;
	dword	samplebytes;
	byte*	samplebuffer;
};


// external functions

sample_s *	SND_ReadSample( sampleinfo_s *pSampleInfo );
void		SND_FreeSample( sample_s *pSample );
void		SND_ConvertToMono( sample_s *pSample );
void 		SND_ConvertRate( sample_s *pSample, int samplerate );


#endif // _SND_API_H_


