/*
 * PARSEC HEADER: snd_wav.h
 */

#ifndef _SND_WAV_H_
#define _SND_WAV_H_


// external functions

void 	WAV_ParseHeader( FILE *fp, sampleinfo_s *pSampleInfo, sample_s *pSample );
void	WAV_ReadSample( sampleinfo_s *pSampleInfo, sample_s *pSample );
void    WAV_FreeSample( sample_s *pSample );
void	WAV_ConvertToMono( sample_s *pSample );
void 	WAV_ConvertRate( sample_s *pSample, int samplerate );


#endif // _SND_WAV_H_


