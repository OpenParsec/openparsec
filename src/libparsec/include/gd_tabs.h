/*
 * PARSEC HEADER: gd_tabs.h
 */

#ifndef _GD_TABS_H_
#define _GD_TABS_H_


// ----------------------------------------------------------------------------
// GLOBAL DATA INFO TABLES                                                    -
// ----------------------------------------------------------------------------


// object lod info ------------------------------------------------------------
//
struct lodinfo_s {

	int			numlods;
	char**		filetab;
	geomv_t*	lodmags;
	geomv_t*	lodmins;
};


// object info table ----------------------------------------------------------
//
struct objectinfo_s {

	dword 		type;						// object type code
	char*		name;						// object name (must be unique!)
	char*		file;						// file object has been read from
	lodinfo_s*	lodinfo;					// optional lod specification
};


// texture info table ---------------------------------------------------------
//
struct textureinfo_s {

	TextureMap*	texpointer;					// pointer to actual texture struct
	dword		flags;						// flagword
	int 		width;						// width in texels
	int 		height;						// height in texels
	char*		name;						// texture name (must be unique!)
	char*		file;						// file texture has been read from
	char*		standardbitmap;				// texture bitmap in TEXFMT_STANDARD
	char*		loaderparams;				// optional loading parameters
};

enum {

	TEXINFOFLAG_NONE			= 0x00000000,
	TEXINFOFLAG_USERPALETTE		= 0x00000001,	// don't touch texture palette
	TEXINFOFLAG_PACKWASDISABLED	= 0x00000100,	// data was not read from pack
};


// texfont info table ---------------------------------------------------------
//
struct texfontinfo_s {

	texfont_s*	texfont;		// pointer to actual texfont structure
	dword		flags;
	int 		width;			// width of single char in texels
	int 		height;			// height of single char in texels
	char*		name;			// texfont name (must be unique!)
	char*		file;			// file font has been read from
	char*		srcimage;		// originally read data
	int			_mksiz32;
};


// bitmap info table ----------------------------------------------------------
//
struct bitmapinfo_s {

	char*		bitmappointer;				// pointer to current bitmap data
	char*		loadeddata;					// pointer to original bitmap data
	int 		width;						// bitmap width
	int 		height;						// bitmap height
	char*		file;						// file bitmap has been read from
	char*		name;						// bitmap name (must be unique!)
	int			_mksiz32[2];
};


// charset info table ---------------------------------------------------------
//
struct charsetinfo_s {

	char*		charsetpointer;				// pointer to current font data
	char*		loadeddata;					// pointer to original font data
	dword*		geompointer;				// pointer to geometry table
	int			datasize;					// size of actual font data in bytes
	int 		width;						// char width
	int 		height;						// char height
	int    		srcwidth;   				// font bitmap width
	dword		flags;						// flagword
	char*		file;						// file font has been read from
	TextureMap*	fonttexture;				// font converted to texture
	int			_mksiz64[6];
};


// sample info table ----------------------------------------------------------
//
struct sampleinfo_s {

	char*		name;
	char*		file;
	char*		samplepointer;
	size_t		size;
	word		flags;
	short		stereolevel;
	int			volume;
	float		samplefreq;
	float		stdfreq;
};


// song info table ------------------------------------------------------------
//
struct songinfo_s {

	char*		songpointer;
	char*		name;
	char*		file;
	int			_mksiz16;
};


#endif // _GD_TABS_H_


