/*
 * PARSEC HEADER: img_3df.h
 */

#ifndef _IMG_3DF_H_
#define _IMG_3DF_H_


// 3df format info

struct format_3df_s {

	const char*	spec;
	dword	format;
	int		texelsize;
	int		_mksiz16;
};


// external functions

format_3df_s*	TDF_DecodeFormatInfo( char *formatspec );
int				TDF_LoadTexture( char *fname, int insertindex, char *loaderparams, texfont_s *texfont );


#endif // _IMG_3DF_H_


