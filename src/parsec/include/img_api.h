/*
 * PARSEC HEADER: img_api.h
 */

#ifndef _IMG_API_H_
#define _IMG_API_H_


// buffer formats

enum buffer_formats {

	BUFFERFORMAT_INVALID		= 0x0000,

	BUFFERFORMAT_INDEXED		= 0x0001,
	BUFFERFORMAT_RGB_332		= 0x0002,
	BUFFERFORMAT_ALPHA_8		= 0x0003,
	BUFFERFORMAT_INTENSITY_8	= 0x0004,
	BUFFERFORMAT_RGB_555		= 0x0005,
	BUFFERFORMAT_RGB_565		= 0x0006,
	BUFFERFORMAT_ARGB_1555		= 0x0007,
	BUFFERFORMAT_ARGB_4444		= 0x0008,
	BUFFERFORMAT_AP_88			= 0x0009,
	BUFFERFORMAT_AI_88			= 0x000a,
	BUFFERFORMAT_RGB_888		= 0x000b,
	BUFFERFORMAT_ARGB_8888		= 0x000c,

	BUFFERFORMAT_BASE_MASK		= 0x00ff,

	BUFFERFORMAT_VGA_PAL		= 0x0100,
	BUFFERFORMAT_RGB_PAL		= 0x0200,
	BUFFERFORMAT_RGBA_PAL		= 0x0400,

	BUFFERFORMAT_PREFIX_PAL		= 0x1000,
	BUFFERFORMAT_POSTFIX_PAL	= 0x2000
};


// recognized image formats

enum image_formats {

	IMAGEFORMAT_INVALID,

	IMAGEFORMAT_RAW,
	IMAGEFORMAT_TGA,
	IMAGEFORMAT_3DF,
	IMAGEFORMAT_PCX,
	IMAGEFORMAT_BMP,
	IMAGEFORMAT_JPG,
	IMAGEFORMAT_PNG
};


// generic image info

struct img_info_s {

	int		format;
	int		width;
	int		height;
	size_t	buffsize;
	int		userinfo[ 4 ];
};


// external functions

int		IMG_DetermineFormat( char *fname );
int		IMG_BufferPixelSize( int format );
int		IMG_CopyConvertBuffer( char *dstbuff, int dstfmt, char *srcbuff, int srcfmt, size_t bsiz );
int		IMG_SaveBuffer( const char *filename, char *buff, img_info_s *info );
int		IMG_LoadTexture( char *fname, int insertindex, char *loaderparams, texfont_s *texfont );


#endif // _IMG_API_H_


