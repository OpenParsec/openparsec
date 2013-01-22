/*
 * PARSEC - Texture Loading Helper Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:35 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999-2001
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

// rendering subsystem
#include "r_patch.h"

// local module header
#include "img_load.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux.h"
#include "e_supp.h"
#include "img_3df.h"
#include "img_conv.h"
#include "img_supp.h"
#include "img_api.h"

// image file loader
#include <stb_image/stb_image.h>



// string constants -----------------------------------------------------------
//
static char no_tex_head_mem[]		= "not enough mem for texture header.";
static char invalid_map_width[]		= "invalid map_width.";
static char invalid_map_height[]	= "invalid map_height.";
static char invalid_lod_large[]		= "invalid lod_large.";
static char invalid_lod_small[]		= "invalid lod_small.";
static char invalid_retile_width[]	= "invalid retile_width.";
static char invalid_retile_height[]	= "invalid retile_height.";
static char invalid_texfmt_gr[]		= "invalid texfmt_gr (glide texture format).";
static char invalid_spacing[]		= "invalid font spacing.";


// flag map for texture loading -----------------------------------------------
//
enum {

	IMAGE_INVERT_PIXELS	= 0x00000001,
	IMAGE_COMPRESS_DATA	= 0x00000002,
};

static flag_map_s loaderparams_flag_map[] = {

	{ "invert",		IMAGE_INVERT_PIXELS,	0	},
	{ "compress",	IMAGE_COMPRESS_DATA,	0	},

	{ NULL,			0,						0	},
};


// possible keys for optional loading params ----------------------------------
//
static key_value_s loadparams_key_value[] = {

	{ "map_width",		NULL,	KEYVALFLAG_NONE			},
	{ "map_height",		NULL,	KEYVALFLAG_NONE			},
	{ "lod_large",		NULL,	KEYVALFLAG_NONE			},
	{ "lod_small",		NULL,	KEYVALFLAG_NONE			},
	{ "texfmt_gr",		NULL,	KEYVALFLAG_NONE			},
	{ "retile_width",	NULL,	KEYVALFLAG_NONE			},
	{ "retile_height",	NULL,	KEYVALFLAG_NONE			},
	{ "spacing",		NULL,	KEYVALFLAG_NONE			},
	{ "flags",			NULL,	KEYVALFLAG_PARENTHESIZE	},

	{ NULL,				NULL,	KEYVALFLAG_NONE			},
};

enum {

	KEY_PARA_MAP_WIDTH,
	KEY_PARA_MAP_HEIGHT,
	KEY_PARA_LOD_LARGE,
	KEY_PARA_LOD_SMALL,
	KEY_PARA_TEXFMT_GR,
	KEY_PARA_RETILE_WIDTH,
	KEY_PARA_RETILE_HEIGHT,
	KEY_PARA_SPACING,
	KEY_PARA_FLAGS,
};


// parse optional texture loading parameters into supplied structure ----------
//
PRIVATE
void IMG_DefaultLoadingParameters( imgreadinfo_s *imgreadinfo )
{
	imgreadinfo->mapwidth		= -1;
	imgreadinfo->mapheight		= -1;

	imgreadinfo->lodlarge		= -1;
	imgreadinfo->lodsmall		= -1;

	imgreadinfo->retilewidth	= -1;
	imgreadinfo->retileheight	= -1;

	// default proportional font spacing
	imgreadinfo->spacing		= 3;

	// glide format destination spec
	imgreadinfo->texfmtgr		= NULL;

	// default: no input mirroring, no pixel-wise image inversion, no texture compression
	image_mirror_vertically	= FALSE;
	image_invert_pixels		= FALSE;
	image_compress_data		= FALSE;
}


// parse optional texture loading parameters into supplied structure ----------
//
int IMG_ParseLoadingParameters( const char *loaderparams, imgreadinfo_s *imgreadinfo )
{
//	ASSERT( loaderparams != NULL );
	ASSERT( imgreadinfo != NULL );

	// store default parameters
	IMG_DefaultLoadingParameters( imgreadinfo );

	// keep defaults if no parameters specified
	if ( loaderparams == NULL ) {
		return TRUE;
	}

	// parsing must not destroy original string since it will be
	// reused on texture restoration
	char *lparams = (char *) ALLOCMEM( strlen( loaderparams ) + 1 );
	if ( lparams == NULL )
		OUTOFMEM( 0 );
	strcpy( lparams, loaderparams );

	if ( !ScanKeyValuePairs( loadparams_key_value, lparams ) ) {
		FREEMEM( lparams );
		return FALSE;
	}

	//NOTE:
	// the alternate mapping geometry specified with "map_width" and
	// "map_height" can be used to specify the scale of the texture
	// coordinates used for the texture being loaded in objects that
	// use it, if it is not identical to the actual texture dimensions.

	// check for alternate mapping geometry
	if ( ScanKeyValueInt( &loadparams_key_value[ KEY_PARA_MAP_WIDTH ], &imgreadinfo->mapwidth ) < 0 ) {
		MSGOUT( invalid_map_width );
		FREEMEM( lparams );
		return FALSE;
	}
	if ( ScanKeyValueInt( &loadparams_key_value[ KEY_PARA_MAP_HEIGHT ], &imgreadinfo->mapheight ) < 0 ) {
		MSGOUT( invalid_map_height );
		FREEMEM( lparams );
		return FALSE;
	}

	// check for forced lod boundaries
	if ( ScanKeyValueInt( &loadparams_key_value[ KEY_PARA_LOD_LARGE ], &imgreadinfo->lodlarge ) < 0 ) {
		MSGOUT( invalid_lod_large );
		FREEMEM( lparams );
		return FALSE;
	}
	if ( ScanKeyValueInt( &loadparams_key_value[ KEY_PARA_LOD_SMALL ], &imgreadinfo->lodsmall ) < 0 ) {
		MSGOUT( invalid_lod_small );
		FREEMEM( lparams );
		return FALSE;
	}

	// check for retiling geometry (usually only for texfonts)
	if ( ScanKeyValueInt( &loadparams_key_value[ KEY_PARA_RETILE_WIDTH ], &imgreadinfo->retilewidth ) < 0 ) {
		MSGOUT( invalid_retile_width );
		FREEMEM( lparams );
		return FALSE;
	}
	if ( ScanKeyValueInt( &loadparams_key_value[ KEY_PARA_RETILE_HEIGHT ], &imgreadinfo->retileheight ) < 0 ) {
		MSGOUT( invalid_retile_height );
		FREEMEM( lparams );
		return FALSE;
	}

	// check for glide texture format conversion
	if ( loadparams_key_value[ KEY_PARA_TEXFMT_GR ].value != NULL ) {

		imgreadinfo->texfmtgr = TDF_DecodeFormatInfo( loadparams_key_value[ KEY_PARA_TEXFMT_GR ].value );
		if ( imgreadinfo->texfmtgr == NULL ) {
			MSGOUT( invalid_texfmt_gr );
			FREEMEM( lparams );
			return FALSE;
		}
	}

	// check for optional spacing
	if ( ScanKeyValueInt( &loadparams_key_value[ KEY_PARA_SPACING ], &imgreadinfo->spacing ) < 0 ) {
		MSGOUT( invalid_spacing );
		FREEMEM( lparams );
		return FALSE;
	}

	// check for optional flags
	if ( loadparams_key_value[ KEY_PARA_FLAGS ].value != NULL ) {

		// parse flag list
		dword flags = 0;
		if ( ScanKeyValueFlagList( &loadparams_key_value[ KEY_PARA_FLAGS ], &flags, loaderparams_flag_map ) == 0 ) {
			MSGOUT( "invalid flag specified." );
			FREEMEM( lparams );
			return FALSE;
		}

		if ( flags & IMAGE_INVERT_PIXELS ) {
			image_invert_pixels = TRUE;
		}
		if ( flags & IMAGE_COMPRESS_DATA ) {
			image_compress_data = TRUE;
		}
	}

	FREEMEM( lparams );
	lparams = NULL;

	return TRUE;
}


// check optional mapping geometry (mapping differs from file geometry) -------
//
int IMG_CheckMapGeometry( imgreadinfo_s *imgreadinfo, int width, int height )
{
	ASSERT( imgreadinfo != NULL );

	// take geometry of file if no override
	if ( imgreadinfo->mapwidth < 1 ) {

		imgreadinfo->mapwidth = width;

	} else {

		// validate override
		if ( ( imgreadinfo->mapwidth & ( imgreadinfo->mapwidth - 1 ) ) != 0 ) {
			MSGOUT( "map_width must be power of two." );
			imgreadinfo->mapwidth = width;
		} else if ( imgreadinfo->mapwidth > width ) {
			MSGOUT( "map_width is larger than texture width." );
			imgreadinfo->mapwidth = width;
		}
	}

	if ( imgreadinfo->mapheight < 1 ) {

		imgreadinfo->mapheight = height;

	} else {

		// validate override
		if ( ( imgreadinfo->mapheight & ( imgreadinfo->mapheight - 1 ) ) != 0 ) {
			MSGOUT( "map_height must be power of two." );
			imgreadinfo->mapheight = height;
		} else if ( imgreadinfo->mapheight > height ) {
			MSGOUT( "map_height is larger than texture height." );
			imgreadinfo->mapheight = height;
		}
	}

	// determine largest side of map geometry
	int lgmap = ( imgreadinfo->mapwidth > imgreadinfo->mapheight ) ?
		imgreadinfo->mapwidth : imgreadinfo->mapheight;

	return lgmap;
}


// check optional lod specs (if restricted lod bounds have been specified) ----
//
void IMG_CheckLodSpecs( imgreadinfo_s *imgreadinfo, int lgside )
{
	ASSERT( imgreadinfo != NULL );

	// from lod of file to one if no override
	if ( imgreadinfo->lodlarge < 1 ) {

		imgreadinfo->lodlarge = lgside;

	} else {

		// validate override
		if ( ( imgreadinfo->lodlarge & ( imgreadinfo->lodlarge - 1 ) ) != 0 ) {
			MSGOUT( "lod_large must be power of two." );
			imgreadinfo->lodlarge = lgside;
		} else if ( imgreadinfo->lodlarge > lgside ) {
			MSGOUT( "lod_large is larger than texture." );
			imgreadinfo->lodlarge = lgside;
		}
	}

	if ( imgreadinfo->lodsmall < 1 ) {

		imgreadinfo->lodsmall = 1;

	} else {

		// validate override
		if ( ( imgreadinfo->lodsmall & ( imgreadinfo->lodsmall - 1 ) ) != 0 ) {
			MSGOUT( "lod_small must be power of two." );
			imgreadinfo->lodsmall = 1;
		} else if ( imgreadinfo->lodsmall > imgreadinfo->lodlarge ) {
			MSGOUT( "lod_small is larger than lod_large." );
			imgreadinfo->lodsmall = 1;
		}
	}
}


// create texture from image --------------------------------------------------
//
void IMG_CreateTexture( int insertindex, imgtotexdata_s *imgtotexdata, dword texgeometry, int inputwidth, int inputheight, texfont_s *texfont )
{
	ASSERT( imgtotexdata != NULL );
	//	ASSERT( texfont != NULL );

	// create texture map description
	TextureMap *tmap = NULL;

	if ( insertindex == NumLoadedTextures ) {

		// create new texture structure
		tmap = (TextureMap *) ALLOCMEM( sizeof( TextureMap ) );
		if ( tmap == NULL )
			OUTOFMEM( no_tex_head_mem );
		memset( tmap, 0, sizeof( TextureMap ) );

	} else {

		// keep old texture structure
		tmap = TextureInfo[ insertindex ].texpointer;

		//NOTE:
		// AUX_DONT_OVERLOAD_TEXTURE_GEOMETRY is disregarded here.
		// (in contrast to textures in 3df format, which do not
		// use this helper function)

		// free texture if not already done.
		// (the texture may have already been freed by the cache)
		char *block = tmap->TexPalette ? (char*)tmap->TexPalette : tmap->BitMap;
		if ( block != NULL ) {
			FREEMEM( block );
		}

		// invalidate texture in cache to prevent
		// texture from failing to download
		if ( !TextModeActive ) {
			R_InvalidateCachedTexture( tmap );
		}

		// reset data pointers
		tmap->BitMap	 = NULL;
		tmap->TexPalette = NULL;
	}
	ASSERT( tmap != NULL );

	// init fields of texture map structure
	tmap->Flags			= TEXFLG_EXT_GEOMETRY;
	tmap->Flags		   |= TEXFLG_LODRANGE_VALID;
	tmap->Flags		   |= TEXFLG_CACHE_MAY_FREE;
	tmap->Flags		   |= image_compress_data ? TEXFLG_DO_COMPRESSION : 0;
	tmap->Geometry		= texgeometry;
	tmap->Width			= CeilPow2Exp( imgtotexdata->width );
	tmap->Height		= CeilPow2Exp( imgtotexdata->height );
	tmap->LOD_small		= imgtotexdata->lodsmall;
	tmap->LOD_large		= imgtotexdata->lodlarge;
	tmap->BitMap		= imgtotexdata->texbitmap;
	tmap->TexPalette	= NULL;
	tmap->TexelFormat	= imgtotexdata->format;

	// if texture is used by a texfont it must know its address
	if ( texfont != NULL ) {
		texfont->texmap = tmap;
	}

	//NOTE:
	// the texture name (TextureMap::TexMapName) is either still
	// valid from the old texture, or it has been reset to NULL
	// and must be set by the caller of this function.
	// e.g. CON_LOAD::ConLoadTexture().

	TextureInfo[ insertindex ].texpointer	  = tmap;
	TextureInfo[ insertindex ].standardbitmap = NULL;
	TextureInfo[ insertindex ].flags		  = TEXINFOFLAG_USERPALETTE;

	if ( ( insertindex == NumLoadedTextures ) || !AUX_DONT_OVERLOAD_TEXTURE_GEOMETRY ) {

		if ( imgtotexdata->retileinfo != NULL ) {
			TextureInfo[ insertindex ].width  = inputwidth;
			TextureInfo[ insertindex ].height = inputheight;
		} else {
			TextureInfo[ insertindex ].width  = imgtotexdata->width;
			TextureInfo[ insertindex ].height = imgtotexdata->height;
		}
	}
}


// read image from file using stb_image --------------------------------------------------
//
int IMG_LoadGenericTexture(const char *fname, int insertindex, char *loaderparams, texfont_s *texfont)
{
	ASSERT( fname != NULL );
	ASSERT( insertindex >= 0 );
	//	ASSERT( loaderparams != NULL );
	//	ASSERT( texfont != NULL );

	// parse optional loading parameters
	imgreadinfo_s imgreadinfo;
	if ( !IMG_ParseLoadingParameters( loaderparams, &imgreadinfo ) ) {
		return FALSE;
	}

	int width = 0, height = 0;
	int components = 0;

	// load image file
	unsigned char *imagedata = stbi_load(fname, &width, &height, &components, 0);

	if (imagedata == NULL) {
		MSGOUT("Error loading file %s: %s\n", fname, stbi_failure_reason());
		return FALSE;
	}

	int format;
	switch (components) {
		case STBI_grey:
			format = BUFFERFORMAT_INTENSITY_8;
			break;
		case STBI_grey_alpha:
			format = BUFFERFORMAT_AI_88;
			break;
		case STBI_rgb:
			format = BUFFERFORMAT_RGB_888;
			break;
		case STBI_rgb_alpha:
			format = BUFFERFORMAT_ARGB_8888;
			break;
		default:
			MSGOUT("image %s has an unknown format (unexpected number of components: %d)\n", fname, components);
			stbi_image_free(imagedata);
			return FALSE;
	}

	size_t imagesize = width * height * components;
	char * buffer = (char *) ALLOCMEM(imagesize);

	if (buffer == NULL) {
		stbi_image_free(imagedata);
		return FALSE;
	}

	// copy stb data to our own buffer (stb might have used a different memory allocator)
	memcpy(buffer, imagedata, imagesize);
	stbi_image_free(imagedata);

	// for later reference if width/height has been overwritten ( retileinfo != NULL ).
	int inputwidth  = width;
	int inputheight = height;

	// check whether to retile source image to create texture
	retileinfo_s _retileinfo;
	retileinfo_s *retileinfo = NULL;
	if ( ( imgreadinfo.retilewidth != -1 ) && ( imgreadinfo.retileheight != -1 ) ) {

		retileinfo = &_retileinfo;

		// destination texture must be specified
		if ( ( imgreadinfo.mapwidth == -1 ) || ( imgreadinfo.mapheight == -1 ) ) {
			FREEMEM(buffer);
			return FALSE;
		}

		// deduce retiling geometry from input width
		// if not specified explicitly
		if ( imgreadinfo.retilewidth == 0 ) {
			imgreadinfo.retilewidth = width;
		}
		if ( imgreadinfo.retileheight == 0 ) {
			imgreadinfo.retileheight = width;
		}

		// store retiling info
		retileinfo->srcwidth   = width;
		retileinfo->srcheight  = height;
		retileinfo->tilewidth  = imgreadinfo.retilewidth;
		retileinfo->tileheight = imgreadinfo.retileheight;
		retileinfo->texfont    = texfont;
		retileinfo->spacing    = imgreadinfo.spacing;

		width  = imgreadinfo.mapwidth;
		height = imgreadinfo.mapheight;
	}

	// determine largest side of map geometry
	int lgmap = IMG_CheckMapGeometry( &imgreadinfo, width, height );

	// determine largest side and check lod specs
	int lgside = ( width > height ) ? width : height;
	IMG_CheckLodSpecs( &imgreadinfo, lgside );

	// determine aspect ratio
	int aspect = ( VidInfo_MaxTextureLod > TEXLOD_256 ) ?
	IMG_DetermineTexGeoAspectExt( imgreadinfo.mapwidth, imgreadinfo.mapheight ) :
	IMG_DetermineTexGeoAspect( imgreadinfo.mapwidth, imgreadinfo.mapheight );
	if ( aspect == -1 ) {
		MSGOUT( "invalid aspect ratio." );
		FREEMEM(buffer);
		return FALSE;
	}

	// map to internal lod spec
	int lodspecsmall = image_lods[ CeilPow2Exp( imgreadinfo.lodsmall ) ];
	int lodspeclarge = image_lods[ CeilPow2Exp( imgreadinfo.lodlarge ) ];
	int lodloaded    = image_lods[ CeilPow2Exp( lgside ) ];

	// clamp to maximum lod
	int maxlod = VidInfo_MaxTextureLod;

	if ( lodspeclarge > maxlod ) {
		lodspeclarge = maxlod;
	}
	if ( lodspecsmall > maxlod ) {
		lodspecsmall = maxlod;
	}

	imgtotexdata_s imgtotexdata;
	imgtotexdata.texbitmap	= buffer;
	imgtotexdata.width		= width;
	imgtotexdata.height		= height;
	imgtotexdata.lodloaded	= lodloaded;
	imgtotexdata.lodlarge	= lodspeclarge;
	imgtotexdata.lodsmall	= lodspecsmall;
	imgtotexdata.texfmtgr	= imgreadinfo.texfmtgr;
	imgtotexdata.retileinfo	= retileinfo;

	// convert format spec
	switch ( format ) {
		case BUFFERFORMAT_RGB_888:
			imgtotexdata.format = TEXFMT_RGB_888;
			break;
		case BUFFERFORMAT_ARGB_8888:
			imgtotexdata.format = TEXFMT_RGBA_8888;
			break;
		default:
			FREEMEM(buffer);
			return FALSE;
	}

	// convert image data (build mipmaps on-the-fly)
	imgtotexdata.texbitmap = IMG_ConvertTextureImageData( &imgtotexdata, FALSE );
	if ( imgtotexdata.texbitmap == NULL ) {
		return FALSE;
	}

	// adjust geometry if lods have been stripped
	for ( int lcnt = lodloaded - lodspeclarge; lcnt > 0; lcnt-- ) {
		if ( width > 1 ) {
			width >>= 1;
		}
		if ( height > 1 ) {
			height >>= 1;
		}
	}
	imgtotexdata.width  = width;
	imgtotexdata.height = height;

	dword scalecode   = IMG_DetermineTexGeoScale( lgmap );
	dword texgeometry = ( aspect | scalecode );

	// create texture from image
	IMG_CreateTexture( insertindex, &imgtotexdata, texgeometry, inputwidth, inputheight, texfont );
	
	return TRUE;
}
