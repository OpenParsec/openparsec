/*
 * PARSEC HEADER: img_conv.h
 */

#ifndef _IMG_CONV_H_
#define _IMG_CONV_H_



// retiling info to create a texfont texture

struct retileinfo_s {

	int			srcwidth;
	int			srcheight;

	int			tilewidth;
	int			tileheight;

	int			spacing;
	texfont_s*	texfont;
};


// forward declaration
struct format_3df_s;


// image to texture conversion info

struct imgtotexdata_s {

	char*			texbitmap;
	int				width;
	int				height;
	int				lodloaded;
	int				lodlarge;
	int				lodsmall;
	dword			format; // TEXFMT
	format_3df_s*	texfmtgr;
	retileinfo_s*	retileinfo;
};


// external functions

char *IMG_ConvertTextureImageData( imgtotexdata_s *imgtotexdata, int swaprgba );


// external variables

extern int		image_mirror_vertically;
extern int		image_invert_pixels;
extern int		image_compress_data;

extern int		image_lods[];


#endif // _IMG_CONV_H_


