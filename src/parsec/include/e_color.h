/*
 * PARSEC HEADER: e_color.h
 */

#ifndef _E_COLOR_H_
#define _E_COLOR_H_


// external functions

visual_t	ColorIndexToVisual( int colindx );
void		ColorIndexToRGBA( colrgba_s *colrgba, int colindx );
void		VisualToRGBA( colrgba_s *colrgba, visual_t visual );
visual_t	RGBAToVisual( colrgba_s *colrgba );

void		SetupTranslationPalette( char *palette );
void		SetupSingleBitmapColors( int bmid );
void		SetupSingleCharsetColors( int ftid );
void		SetupBitmapColors();

void		InitColorMaps();
void		KillColorMaps();


// color index translation

#define COLINDX_TO_VISUAL(x)	IndexToVisualTab[ (x) & 0xff ]
//#define COLINDX_TO_VISUAL(x)	ColorIndexToVisual( x )

#define COLINDX_TO_RGBA(c,x)	{ *(c) = IndexToRGBATab[ (x) & 0xff ]; }
//#define COLINDX_TO_RGBA(c,x)	ColorIndexToRGBA( (c), (x) )


// translucency table ids

enum {

	TRTAB_PANELBACK,
	TRTAB_PANELTEXT,
	TRTAB_FLAREBASE,

	TRTAB_NUMTABLES		// has to be last!!
};


// color bit-resolution translation tables

extern byte colbits_1_to_8[];
extern byte colbits_2_to_8[];
extern byte colbits_3_to_8[];
extern byte colbits_4_to_8[];
extern byte colbits_5_to_8[];
extern byte colbits_6_to_8[];


// color setup (conversion) flags

#define COLORSETUP_DEFAULT						0x00000000
#define COLORSETUP_32BPP_PAL_SWAP_RGB			0x00000001
#define COLORSETUP_32BPP_PAL_ALPHA_IN_LSB		0x00000002
#define COLORSETUP_32BPP_PAL_SET_ALPHA			0x00000004
#define COLORSETUP_32BPP_BITMAP_YINVERSE		0x00000008
#define COLORSETUP_32BPP_FONT_YINVERSE			0x00000010
#define COLORSETUP_32BPP_ENABLE_TRANSPARENCY	0x00000020
#define COLORSETUP_32BPP_CONVERT_SCALE_BITMAPS	0x00000040
#define COLORSETUP_STANDARD_TO_RGB_565			0x00000100
#define COLORSETUP_STANDARD_TO_RGBA_1555		0x00000200
#define COLORSETUP_STANDARD_TO_RGB_888			0x00000400
#define COLORSETUP_STANDARD_TO_RGBA_8888		0x00000800
#define COLORSETUP_STANDARD_DO_CONVERSION		0x00000f00
#define COLORSETUP_GLIDE3DF_TO_RGB_565			0x00001000
#define COLORSETUP_GLIDE3DF_TO_RGBA_1555		0x00002000
#define COLORSETUP_GLIDE3DF_TO_RGB_888			0x00004000
#define COLORSETUP_GLIDE3DF_TO_RGBA_8888		0x00008000
#define COLORSETUP_GLIDE3DF_DO_CONVERSION		0x0000f000
#define COLORSETUP_FONT_TO_TEXTURE_ALPHA		0x00010000
#define COLORSETUP_FONT_TO_TEXTURE_INTENSITY	0x00020000
#define COLORSETUP_FONT_TO_TEXTURE_LUMINANCE	0x00040000
#define COLORSETUP_FONT_TO_TEXTURE_RGBA			0x00080000
#define COLORSETUP_FONT_DO_CONVERSION			0x000f0000
#define COLORSETUP_ALPHA_8_TO_RGBA_8888			0x00100000
#define COLORSETUP_INTENSITY_8_TO_RGBA_8888		0x00200000
//#define COLORSETUP_GENERAL_TO_GLIDE3DF			0x00400000
#define COLORSETUP_SWAP_BITMAPS_16BPP			0x01000000
#define COLORSETUP_SWAP_BITMAPS_32BPP			0x02000000
#define COLORSETUP_SWAP_FONTS_16BPP				0x04000000
#define COLORSETUP_SWAP_FONTS_32BPP				0x08000000

extern dword ColorSetupFlags;


#endif // _E_COLOR_H_


