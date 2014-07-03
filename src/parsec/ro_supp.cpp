/*
 * PARSEC - Supporting Rendering Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:40 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999-2001
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   1999-2001
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
#include "r_supp.h"

// subsystem headers
#include "aud_defs.h"
#include "inp_defs.h"
#include "net_defs.h"
#include "sys_defs.h"
#include "vid_defs.h"

// local module header
#include "ro_supp.h"

// proprietary module headers
#include "con_aux.h"
#include "con_main.h"
#include "e_color.h"
#include "e_supp.h"
#include "img_api.h"
#include "ro_api.h"
#include "sys_file.h"


// flags
//#define APPEND_TEXTURE_AT_TAIL


// enable opengl depth buffering ----------------------------------------------
//
void RO_EnableDepthBuffer( bool checking, bool writing )
{
	if ( checking ) {
		RO_EnableDepthTest( TRUE );
		RO_DepthFunc( GL_GREATER );
	}

	if ( writing ) {
		RO_DepthMask( GL_TRUE );
	}
}


// disable opengl depth buffering ---------------------------------------------
//
void RO_DisableDepthBuffer( bool checking, bool writing )
{
	if ( checking ) {
		RO_EnableDepthTest( FALSE );
	}

	if ( writing ) {
		RO_DepthMask( GL_FALSE );
	}
}


// restore original depth state -----------------------------------------------
//
void RO_RestoreDepthState( int zcmpstate, int zwritestate )
{
	int zcmp = RO_DepthCmpEnabled();
	if ( zcmp && !zcmpstate )
		RO_DisableDepthBuffer( true, false );
	else if ( !zcmp && zcmpstate )
		RO_EnableDepthBuffer( true, false );

	int zwrite = RO_DepthWriteEnabled();
	if ( zwrite && !zwritestate )
		RO_DisableDepthBuffer( false, true );
	else if ( !zwrite && zwritestate )
		RO_EnableDepthBuffer( false, true );
}


// expand non-power-of-two texture into power-of-two texture ------------------
//
INLINE
void RO_ExpandTexPow2( GLTexInfo *texinfo )
{
	ASSERT( texinfo != NULL );

	int srcw = texinfo->width;
	int srch = texinfo->height;

	ASSERT( ( srcw > 0 ) && ( srch > 0 ) );

	// determine nearest power of two size for width and height
	int wc2 = 0;
	int hc2 = 0;
	for (  wc2 = 1; wc2 < srcw; wc2 <<= 1 )
		{}
	for ( hc2 = 1; hc2 < srch; hc2 <<= 1 )
		{}

	ASSERT( ( wc2 <= 256 ) && ( hc2 <= 256 ) );

	// make texture square
	if ( hc2 > wc2 )
		wc2 = hc2;
	hc2 = wc2;

	int		lod;
	float	csc;

	switch ( wc2 ) {
		
		case 1024:
			lod = TEXLOD_1024;
			csc = 1/1024.0;
			break;
		
		case 512:
			lod = TEXLOD_512;
			csc = 1/512.0;
			break;

		case 256:
			lod = TEXLOD_256;
			csc = 1/256.0;
			break;

		case 128:
			lod = TEXLOD_128;
			csc = 1/128.0;
			break;

		case 64:
			lod = TEXLOD_64;
			csc = 1/64.0;
			break;

		case 32:
			lod = TEXLOD_32;
			csc = 1/32.0;
			break;

		case 16:
			lod = TEXLOD_16;
			csc = 1/16.0;
			break;

		case 8:
			lod = TEXLOD_8;
			csc = 1/8.0;
			break;

		case 4:
			lod = TEXLOD_4;
			csc = 1/4.0;
			break;

		case 2:
			lod = TEXLOD_2;
			csc = 1/2.0;
			break;

		default:
			PANIC( 0 );
	}

	// set texture geometry
	texinfo->width		= wc2;
	texinfo->height		= hc2;
	texinfo->coscale	= csc;
	texinfo->aratio		= 1.0f;
	texinfo->lodsmall	= lod;
	texinfo->lodlarge	= lod;

	ASSERT( ( srcw <= wc2 ) && ( srch <= hc2 ) );
	if ( ( srcw == wc2 ) && ( srch == hc2 ) )
		return;

	// alloc temporary translation block
	char *texmem = (char *) ALLOCMEM( wc2 * hc2 * 4 );
	if ( texmem == NULL )
		OUTOFMEM( 0 );

//FIXME:
	// preclear mem to value of pixel (0,0)
//	memset( texmem, *(char *)texinfo->data, wc2 * hc2 * 4 );
	memset( texmem, 0, wc2 * hc2 * 4 );

	// copy texture over
	dword *dst = (dword *) texmem;
	dword *src = (dword *) texinfo->data;
	for ( int row = srch; row > 0; --row ) {
		dword *wdst = dst;
		for ( dword *beyond = dst + srcw; wdst < beyond; )
			*wdst++ = *src++;
		dst += wc2;
	}

	// simply overwrite old pointer
	// (texinfo is temporary anyway)
	texinfo->data = texmem;
}


// texture object manager variables -------------------------------------------
//
struct texmementry_s {

	texmementry_s*	next;
	void*			texture;		// texture key: &TextureMap or &data
	GLuint			texname;		// OpenGL texture name

	dword			texparams;

	// cache certain GLTexInfo fields
	int				auxvalid;
	float			coscale;

	dword			_pad32[ 2 ];
};

// set these wisely
#define TEX_HASH_TABLE_SIZE			1001
#define HASH_TEXTURE_ADDRESS(x)		( ((size_t)(x)) % TEX_HASH_TABLE_SIZE )

texmementry_s**	ro_TexBlockHashTab	= NULL;		// hash table
texmementry_s**	ro_TexBlockTailTab	= NULL;		// tail table

int			 	ro_TexMemInfoValid	= FALSE;

void*			ro_LastTextureUsed	= NULL;		// MRU texture key
texmementry_s*	ro_LastTextureEntry;			// list element of MRU texture


// invalidate on-board texture memory -----------------------------------------
//
void RO_InvalidateTextureMem()
{
	//NOTE:
	// this function is called on every subsystem init, including
	// the first one (never inited) by RO_PATCH::R_PatchSubSystem().
	// it can also be called via RO_PATCH::R_InvalidateCachedTexture()
	// if the texture to invalidate is specified as NULL. this is
	// used by implementations of VIDs_RestoreDisplay() to invalidate
	// the entire texture cache on video subsystem deinit.

	// no use if manager down
	if ( !ro_TexMemInfoValid ) {
		ASSERT( ro_LastTextureUsed == NULL );
		return;
	}

	// invalidate info
	ro_TexMemInfoValid = FALSE;
	ro_LastTextureUsed = NULL;

	// invalidate all hashed texmem block lists
	if ( ro_TexBlockHashTab != NULL ) {

		for ( dword htid = 0; htid < TEX_HASH_TABLE_SIZE; htid++ ) {
			texmementry_s *scan = ro_TexBlockHashTab[ htid ];
			while ( scan != NULL ) {
				// delete texture object (if not already
				// invalidated previously)
				if ( scan->texture != NULL )
					glDeleteTextures( 1, &scan->texname );
				texmementry_s *temp = scan->next;
				FREEMEM( scan );
				scan = temp;
			}
		}

		// free hash table
		FREEMEM( ro_TexBlockHashTab );
		ro_TexBlockHashTab = NULL;
		ro_TexBlockTailTab = NULL;
	}

	ASSERT( ro_TexBlockHashTab == NULL );
	ASSERT( ro_TexBlockTailTab == NULL );

	AUXDATA_TMM_NEXT_TEXMEM_LOCATION	= 0;
	AUXDATA_TMM_NUM_DOWNLOADED			= 0;
	AUXDATA_TMM_NUM_NONDISCARDABLE		= 0;
	AUXDATA_TMM_NUM_HASHTABLE_ENTRIES	= 0;
	AUXDATA_TMM_CUR_TAILBUBBLE_SIZE		= 0;
	AUXDATA_TMM_CUR_BUBBLE_SIZE			= 0;
}


// init texture memory manager variables --------------------------------------
//
PRIVATE
void RO_InitTexMemManager()
{
	ASSERT( ro_TexMemInfoValid == FALSE );
	ASSERT( ro_TexBlockHashTab == NULL );
	ASSERT( ro_TexBlockTailTab == NULL );

	// alloc hash table
	ro_TexBlockHashTab = (texmementry_s **) ALLOCMEM( TEX_HASH_TABLE_SIZE * 2 * sizeof( texmementry_s* ) );
	if ( ro_TexBlockHashTab == NULL )
		OUTOFMEM( 0 );
	memset( ro_TexBlockHashTab, 0, TEX_HASH_TABLE_SIZE * 2 * sizeof( texmementry_s* ) );
	ro_TexBlockTailTab = &ro_TexBlockHashTab[ TEX_HASH_TABLE_SIZE ];

	// set flag
	ro_TexMemInfoValid = TRUE;
}


// conversion table for texture format to opengl image and internal format ----
//
struct tex_format_opengl_s {

	int		pixsize;			// size in bytes of source pixel
	GLenum	format;
	GLint	internalformat;
	GLenum  datatype;
};

static tex_format_opengl_s tex_format_opengl[] = {

	// We use the base (unsized) internal formats for compatibility with GLES.
	// GL will pick the correct sized internal format based on format + datatype.
	{ 0, (GLenum) 0,   0,            (GLenum) 0                }, // TEXFMT_STANDARD
	{ 2, GL_RGB,       GL_RGB,       GL_UNSIGNED_SHORT_5_6_5   }, // TEXFMT_RGB_565
	{ 2, GL_RGBA,      GL_RGBA,      GL_UNSIGNED_SHORT_5_5_5_1 }, // TEXFMT_RGBA_1555
	{ 3, GL_RGB,       GL_RGB,       GL_UNSIGNED_BYTE          }, // TEXFMT_RGB_888
	{ 4, GL_RGBA,      GL_RGBA,      GL_UNSIGNED_BYTE          }, // TEXFMT_RGBA_8888
	{ 1, GL_ALPHA,     GL_ALPHA,     GL_UNSIGNED_BYTE          }, // TEXFMT_ALPHA_8
	{ 1, GL_INTENSITY, GL_INTENSITY, GL_UNSIGNED_BYTE          }, // TEXFMT_INTENSITY_8
	{ 1, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE          }, // TEXFMT_LUMINANCE_8
};


// texture params -------------------------------------------------------------
//
enum {

	TEXPARAMS_WRAPPING_ON			= 0x01,
	TEXPARAMS_FILTERING_ON			= 0x02,
	TEXPARAMS_TRILINEAR_ON			= 0x04,
	TEXPARAMS_MIPMAPPING_ON			= 0x08,
	TEXPARAMS_MIPMAPPING_ALLOW		= 0x10,

	TEXPARAMS_FILTER_CHANGED_MASK	= TEXPARAMS_FILTERING_ON |
									  TEXPARAMS_TRILINEAR_ON |
									  TEXPARAMS_MIPMAPPING_ON
};


// check whether texture params need to be changed ----------------------------
//
PRIVATE
void RO_CheckTexParams( texmementry_s *texentry )
{
	ASSERT( texentry != NULL );

	dword anisotropy = texentry->texparams >> 8;
	dword texparams  = texentry->texparams & 0xFF;
	
	int mipmappingon = ( texparams & TEXPARAMS_MIPMAPPING_ALLOW ) &&
					   !AUX_DISABLE_POLYGON_MIPMAPPING;

	// determine desired state
	if ( mipmappingon ) {
		texparams |= TEXPARAMS_MIPMAPPING_ON;
		if ( AUX_ENABLE_TRILINEAR_FILTERING ) {
			texparams |= TEXPARAMS_TRILINEAR_ON;
		} else {
			texparams &= ~TEXPARAMS_TRILINEAR_ON;
		}
	} else {
		texparams &= ~TEXPARAMS_MIPMAPPING_ON;
		texparams &= ~TEXPARAMS_TRILINEAR_ON;
	}
	if ( AUX_DISABLE_POLYGON_FILTERING ) {
		texparams &= ~TEXPARAMS_FILTERING_ON;
	} else {
		texparams |= TEXPARAMS_FILTERING_ON;
	}
	if ( AUX_DISABLE_TEXTURE_WRAPPING ) {
		texparams &= ~TEXPARAMS_WRAPPING_ON;
	} else {
		texparams |= TEXPARAMS_WRAPPING_ON;
	}
	
	// anisotropic filtering on mipmapped textures, if supported
	if ( GLEW_EXT_texture_filter_anisotropic && anisotropy != ( mipmappingon ? AUX_ANISOTROPIC_FILTERING : 0 ) ) {
		anisotropy = mipmappingon ? AUX_ANISOTROPIC_FILTERING : 0;
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max((GLfloat) anisotropy, 1.0f) );
	}

	// compare desired state with current state
	dword statecmp = (texentry->texparams & 0xFF) ^ texparams;

	if ( statecmp & TEXPARAMS_FILTER_CHANGED_MASK ) {

		GLint texfiltmag, texfiltmin, mipfilter;
		if ( AUX_DISABLE_POLYGON_FILTERING ) {
			mipfilter = ( texparams & TEXPARAMS_TRILINEAR_ON ) ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;
			texfiltmag = GL_NEAREST;
			texfiltmin = mipmappingon ? mipfilter : texfiltmag;
		} else {
			mipfilter = ( texparams & TEXPARAMS_TRILINEAR_ON ) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST;
			texfiltmag = GL_LINEAR;
			texfiltmin = mipmappingon ? mipfilter : texfiltmag;
		}
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texfiltmag );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texfiltmin );
	}
	
	if ( statecmp & TEXPARAMS_WRAPPING_ON ) {
		GLint texwrap = AUX_DISABLE_TEXTURE_WRAPPING ? GL_CLAMP_TO_EDGE : GL_REPEAT;
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texwrap );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texwrap );
	}
	
	// remember current state
	texentry->texparams = texparams | (anisotropy << 8);
}


// download texture data including all mipmap levels --------------------------
//
PRIVATE
void RO_DownloadTexture( GLTexInfo *texinfo, int mipmappingon )
{
	ASSERT( texinfo != NULL );

	int texfmt = texinfo->format;
	ASSERT( texfmt > TEXFMT_STANDARD );
	ASSERT( texfmt <= TEXFMT_LUMINANCE_8 );

	int lodlarge = texinfo->lodlarge;
	int lodsmall = texinfo->lodsmall;
	ASSERT( lodsmall <= lodlarge );

	byte *data = (byte *) texinfo->data;
	int width  = texinfo->width;
	int height = texinfo->height;

	// force-strip mipmap levels that are too large for gl implementation
	int forcebias = 0;
	int maxside = ( width > height ) ? width : height;
	for ( ; maxside > VidInfo_MaxTextureSize; maxside >>= 1 ) {
		forcebias++;
	}

	// largest mipmap level size
	size_t levelsize = width * height * tex_format_opengl[ texfmt ].pixsize;

	// automatically strip mip levels if lod bias specified
	int lodbias = ( forcebias > AUXDATA_TMM_MIPMAP_LOD_BIAS ) ?
		forcebias : AUXDATA_TMM_MIPMAP_LOD_BIAS;
	for ( ; lodbias > 0; lodbias-- ) {

		if ( lodlarge == lodsmall )
			break;
		lodlarge--;

		data += levelsize;

		if ( width > 1 ) {
			width >>= 1;
			levelsize >>= 1;
		}
		if ( height > 1 ) {
			height >>= 1;
			levelsize >>= 1;
		}
	}

	// check texture compression info. TODO: check TEXFLG_IS_COMPRESSED.
	TextureMap *tmap  = texinfo->texmap;

	if (tmap != NULL && (tmap->Flags & TEXFLG_DO_COMPRESSION)) {
		// We don't support on-the-fly texture compression.
		tmap->CompFormat = 0;
	}

	// determine texture format
	GLenum glformat = tex_format_opengl[ texfmt ].format;
	GLint  glinternalformat	= tex_format_opengl[ texfmt ].internalformat;
	GLenum gltype = tex_format_opengl[ texfmt ].datatype;

	// download most detailed mipmap level (or more if we support it)
	glTexImage2D( GL_TEXTURE_2D, 0, glinternalformat, width, height,
		0, glformat, gltype, data );

	// don't do anything if mipmapping is off
	if ( !mipmappingon ) {
		return;
	}

	// otherwise, download pre-generated mipmap levels
	for ( int level = 1; lodlarge > lodsmall; lodlarge--, level++ ) {

		data += levelsize;

		if ( width > 1 ) {
			width >>= 1;
			levelsize >>= 1;
		}
		if ( height > 1 ) {
			height >>= 1;
			levelsize >>= 1;
		}

		glTexImage2D( GL_TEXTURE_2D, level, glinternalformat, width, height,
			0, glformat, gltype, data );
	}
}


// retrieve/download texture from/to texture memory ---------------------------
//
PRIVATE
texmementry_s *RO_CacheTexture( GLTexInfo *texinfo, int expand )
{
	ASSERT( texinfo != NULL );
	ASSERT( ( expand == TRUE ) || ( expand == FALSE ) );
//	ASSERT( ( expand == FALSE ) || ( texinfo->format == GR_TEXFMT_P_8 ) );

	// key is pointer to struct TextureMap if available, data otherwise
	void *texref = texinfo->texmap ? (void*)texinfo->texmap : texinfo->data;
	ASSERT( texref != NULL );

	// check whether last call was the same
	if ( ro_LastTextureUsed == texref ) {

		ASSERT( ro_LastTextureUsed != NULL );

		//NOTE:
		// texinfo->coscale is not set here!

		// return same entry as on last call
		return ro_LastTextureEntry;

	} else {

		// remember most recently used texture
		ro_LastTextureUsed = texref;
	}

	// acquire texture memory info if not already done
	if ( !ro_TexMemInfoValid ) {
		RO_InitTexMemManager();
	}

	// hash key (texture data address)
	size_t htid = HASH_TEXTURE_ADDRESS( texref );

	// search texmem block list (hash table)
	texmementry_s *scan = ro_TexBlockHashTab[ htid ];
	for ( ; scan != NULL; scan = scan->next ) {

		// texture already downloaded?
		if ( scan->texture == texref ) {

			// retrieve coscale if cached
			if ( scan->auxvalid ) {
				texinfo->coscale = scan->coscale;
			}

			ro_LastTextureEntry = scan;
			return ro_LastTextureEntry;
		}
	}

	// acquire new texture object (name)
	GLuint texname;
	glGenTextures( 1, &texname );

	// create new block
	texmementry_s *newentry = (texmementry_s *) ALLOCMEM( sizeof( texmementry_s ) );
	if ( newentry == NULL )
		OUTOFMEM( 0 );
	newentry->texture	= texref;
	newentry->texname	= texname;
	newentry->auxvalid	= FALSE;	// texinfo cache off

#ifdef APPEND_TEXTURE_AT_TAIL

	// next field of tail is always NULL
	newentry->next = NULL;

	// append block at tail of list (so oldest texture will be at head)
	if ( ro_TexBlockHashTab[ htid ] == NULL ) {
		AUXDATA_TMM_NUM_HASHTABLE_ENTRIES++;
		ro_TexBlockHashTab[ htid ] = newentry;
	} else {
		ASSERT( ro_TexBlockTailTab[ htid ] != NULL );
		ASSERT( ro_TexBlockTailTab[ htid ]->next == NULL );
		ro_TexBlockTailTab[ htid ]->next = newentry;
	}

	// remember previous node
	ro_TexBlockTailTab[ htid ] = newentry;

#else // APPEND_TEXTURE_AT_TAIL

	// prepend at head of list (so oldest texture will be at tail)
	if ( ro_TexBlockHashTab[ htid ] == NULL )
		AUXDATA_TMM_NUM_HASHTABLE_ENTRIES++;
	newentry->next = ro_TexBlockHashTab[ htid ];
	ro_TexBlockHashTab[ htid ] = newentry;

#endif // APPEND_TEXTURE_AT_TAIL

	// check if texture needs to be expanded
	if ( expand ) {

		// expand texture to power of two
		// (create temporary texture)
		RO_ExpandTexPow2( texinfo );

		// cache texinfo
		newentry->auxvalid = TRUE;
		newentry->coscale  = texinfo->coscale;
	}

	// to remember bound params
	dword texparams = 0x00;

	// bind texture and attributes to object
	glBindTexture( GL_TEXTURE_2D, texname );

	if ( AUX_DISABLE_TEXTURE_WRAPPING ) {
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	} else {
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		texparams |= TEXPARAMS_WRAPPING_ON;
	}

	// determine whether mipmapping should be used
	ASSERT( texinfo->lodsmall <= texinfo->lodlarge );
	int mipmappingallow = ( texinfo->lodlarge != texinfo->lodsmall );
	int mipmappingon = mipmappingallow && !AUX_DISABLE_POLYGON_MIPMAPPING;

	if ( mipmappingon && texinfo->lodsmall != TEXLOD_1 ) {

		// We have to tell OpenGL if not all mipmap levels are present.
		int maxlevel = texinfo->lodlarge - texinfo->lodsmall;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, maxlevel);

		// TODO: This isn't supported on GLES, so we should disable mipmapping
		// (or let OpenGL generate the mipmaps) for this texture in that case.
	}

	// set filter params accordingly
	GLint texfiltmag, texfiltmin, mipfilter;
	if ( AUX_DISABLE_POLYGON_FILTERING ) {
		mipfilter = AUX_ENABLE_TRILINEAR_FILTERING ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;
		texfiltmag = GL_NEAREST;
		texfiltmin = mipmappingon ? mipfilter : texfiltmag;
	} else {
		mipfilter = AUX_ENABLE_TRILINEAR_FILTERING ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST;
		texfiltmag = GL_LINEAR;
		texfiltmin = mipmappingon ? mipfilter : texfiltmag;
		texparams |= TEXPARAMS_FILTERING_ON;
	}
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texfiltmag );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texfiltmin );
	
	
	// set anisotropic filtering levels on mipmapped textures if supported
	int anisotropy = 1;
	if ( GLEW_EXT_texture_filter_anisotropic ) {
		anisotropy = mipmappingon ? AUX_ANISOTROPIC_FILTERING : 1;
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max((GLfloat) anisotropy, 1.0f) );
	}

	// download all mipmap levels
	RO_DownloadTexture( texinfo, mipmappingon );

	// remember mipmapping settings
	if ( mipmappingon ) {
		if ( AUX_ENABLE_TRILINEAR_FILTERING )
			texparams |= TEXPARAMS_TRILINEAR_ON;
		texparams |= TEXPARAMS_MIPMAPPING_ON;
		texparams |= TEXPARAMS_MIPMAPPING_ALLOW;
	}
	
	// remember anisotropy setting (store number in upper bits)
	texparams |= (anisotropy << 8);
		

	// remember bound params
	newentry->texparams = texparams;

	// free expand mem
	if ( expand ) {
		FREEMEM( texinfo->data );
		texinfo->data = NULL;
	}

	// user may disable this to avoid the ensuing
	// texture data reloads on vidmode changes
	if ( AUX_ALLOW_CACHE_TEXTURE_EVICT ) {

		// free original data if allowed
		TextureMap *tmap = texinfo->texmap;
		if ( ( tmap != NULL ) && ( tmap->Flags & TEXFLG_CACHE_MAY_FREE ) ) {
			char *block = tmap->TexPalette ? (char*)tmap->TexPalette : tmap->BitMap;
			ASSERT( block != NULL );
			FREEMEM( block );
			tmap->BitMap     = NULL;
			tmap->TexPalette = NULL;
		}
	}

	// count number of downloaded textures
	AUXDATA_TMM_NUM_DOWNLOADED++;

	ro_LastTextureEntry = newentry;
//	return ro_LastTextureEntry;

	// avoids redundant duplicate bind
	return NULL;
}


// last texture supplied to SelectTexelSource functions -----------------------
//
void*		ro_LastData		= NULL;
float		ro_LastCoScale2 = 1.0f;


// invalidate a single texture in cache (in case it is in the cache) ----------
//
void RO_InvalidateCachedTexture( TextureMap *tmap )
{
	ASSERT( tmap != NULL );

	// no use if manager down
	if ( !ro_TexMemInfoValid ) {
		ASSERT( ro_LastTextureUsed == NULL );
		return;
	}

	// key depends on whether it's an actual texture or a fake
	// (temporary) texture struct used to invalidate a bitmap
	void *texref = tmap->TexMapName ? (void*)tmap : (void*)tmap->BitMap;
	ASSERT( texref != NULL );
	ASSERT( ( tmap->TexMapName != NULL ) || ( ( tmap->Width == 0 ) && ( tmap->Height == 0 ) ) );

	// reset early-out
	ro_LastData			= NULL;
	ro_LastTextureUsed	= NULL;

	// hash key (address of texture struct or data)
	dword htid = HASH_TEXTURE_ADDRESS( texref );

	// search texmem block list (hash table)
	texmementry_s *scan = ro_TexBlockHashTab[ htid ];
	if ( scan != NULL ) {

		if ( scan->texture == texref ) {

			// delete texture object
			glDeleteTextures( 1, &scan->texname );
			AUXDATA_TMM_NUM_DOWNLOADED--;

			// unlink head
			if ( scan->next == NULL )
				AUXDATA_TMM_NUM_HASHTABLE_ENTRIES--;
			ro_TexBlockHashTab[ htid ] = scan->next;

			// free old head
			FREEMEM( scan );

		} else {

			// scan list
			texmementry_s *prev = scan;
			for ( scan = scan->next; scan; scan = scan->next ) {
				if ( scan->texture == texref ) {

					// delete texture object
					glDeleteTextures( 1, &scan->texname );
					AUXDATA_TMM_NUM_DOWNLOADED--;

					// unlink from list
					prev->next = scan->next;

					// free block
					FREEMEM( scan );

					break;
				}
				prev = scan;
			}
		}
	}
}


// download texture if not already resident and designate it as texel source --
//
void RO_SelectTexelSource( GLTexInfo *texinfo )
{
	ASSERT( texinfo != NULL );

	//NOTE:
	// the following GLTexInfo fields must be valid:
	// texmap, data, width, height, format, lodsmall, lodlarge.
	// (they are needed by RO_CacheTexture().)

	// key is pointer to struct TextureMap if available, data otherwise
	void *texref = texinfo->texmap ? (void*)texinfo->texmap : texinfo->data;
	ASSERT( texref != NULL );

	// early out if call redundant
	if ( ( texref == ro_LastData ) && ( ro_LastTextureUsed != NULL ) ) {
		return;
	}
	ro_LastData = texref;

	// retrieve/download texture from/to texture memory
	texmementry_s *texentry = RO_CacheTexture( texinfo, FALSE );

	// designate texel source if not already done
	if ( texentry != NULL ) {
		glBindTexture( GL_TEXTURE_2D, texentry->texname );
		RO_CheckTexParams( texentry );
	}
}


// build a texture with power of two dimensions -------------------------------
//
void RO_SelectTexelSource2( GLTexInfo *p2info )
{
	ASSERT( p2info != NULL );

	//NOTE:
	// only the following GLTexInfo fields need
	// be valid: texmap, data, width, height, format.
	// field coscale will be filled in automatically
	// in any case, so it may be used after the call
	// of RO_SelectTexelSource2().
	// fields aratio, lodsmall, and lodlarge will only
	// be filled in if the texture actually needs to
	// be downloaded, so they may or may not be valid
	// after the call of RO_SelectTexelSource2().

	// key is pointer to struct TextureMap if available, data otherwise
	void *texref = p2info->texmap ? (void*)p2info->texmap : p2info->data;
	ASSERT( texref != NULL );

	// early out if call redundant
	if ( ( texref == ro_LastData ) && ( ro_LastTextureUsed != NULL ) ) {
		// coscale must still be correct
		p2info->coscale = ro_LastCoScale2;
		return;
	}
	ro_LastData = texref;

	// retrieve/download texture from/to texture memory
	// and fill in GLTexInfo::coscale
	texmementry_s *texentry = RO_CacheTexture( p2info, TRUE );

	//NOTE:
	// fields coscale, aratio, lodsmall, and lodlarge
	// are filled in by RO_ExpandTexPow2() which is
	// called by RO_CacheTexture() in this case.

	// designate texel source if not already done
	if ( texentry != NULL ) {
		glBindTexture( GL_TEXTURE_2D, texentry->texname );
		RO_CheckTexParams( texentry );
	}

	// for caching
	ro_LastCoScale2 = p2info->coscale;
}


// convert vector of IterVertex2's to vector of GLVertex3's -------------------
//
void RO_IterVertex2GLVertex( GLVertex3 *glvtxs, IterVertex2 *itervtxs, int num, GLTexInfo *texinfo )
{
	ASSERT( glvtxs != NULL );
	ASSERT( itervtxs != NULL );

	if ( texinfo != NULL ) {

		float scale_u = texinfo->coscale;
		float scale_v = texinfo->aratio * scale_u;

		// convert specified number of vertices
		for ( int v = 0; v < num; v++ ) {

			glvtxs[ v ].x = RASTV_TO_FLOAT( itervtxs[ v ].X );
			glvtxs[ v ].y = RASTV_TO_FLOAT( itervtxs[ v ].Y );
			glvtxs[ v ].z = RASTV_TO_FLOAT( itervtxs[ v ].Z ) * OPENGL_DEPTH_RANGE;

			*(dword*)&glvtxs[ v ].r = *(dword*)&itervtxs[ v ].R;

			glvtxs[ v ].s = GEOMV_TO_FLOAT( itervtxs[ v ].U ) * scale_u;
			glvtxs[ v ].t = GEOMV_TO_FLOAT( itervtxs[ v ].V ) * scale_v;
			glvtxs[ v ].p = 0.0f;
			glvtxs[ v ].q = GEOMV_TO_FLOAT( itervtxs[ v ].W );
		}

	} else {

		// convert specified number of vertices
		for ( int v = 0; v < num; v++ ) {

			glvtxs[ v ].x = RASTV_TO_FLOAT( itervtxs[ v ].X );
			glvtxs[ v ].y = RASTV_TO_FLOAT( itervtxs[ v ].Y );
			glvtxs[ v ].z = RASTV_TO_FLOAT( itervtxs[ v ].Z ) * OPENGL_DEPTH_RANGE;

			*(dword*)&glvtxs[ v ].r = *(dword*)&itervtxs[ v ].R;
		}
	}
}


// convert vector of IterVertex2 pointers to vector of GLVertex3's ------------
//
void RO_IterVertex2GLVertexRef( GLVertex3 *glvtxs, IterVertex2 **itervtxs, int num, GLTexInfo *texinfo )
{
	ASSERT( glvtxs != NULL );
	ASSERT( itervtxs != NULL );

	if ( texinfo != NULL ) {

		float scale_u = texinfo->coscale;
		float scale_v = texinfo->aratio * scale_u;

		// convert specified number of vertices
		for ( int v = 0; v < num; v++ ) {

			IterVertex2 *itervtx = itervtxs[ v ];
			ASSERT( itervtx != NULL );

			glvtxs[ v ].x = RASTV_TO_FLOAT( itervtx->X );
			glvtxs[ v ].y = RASTV_TO_FLOAT( itervtx->Y );
			glvtxs[ v ].z = RASTV_TO_FLOAT( itervtx->Z ) * OPENGL_DEPTH_RANGE;

			*(dword*)&glvtxs[ v ].r = *(dword*)&itervtx->R;

			glvtxs[ v ].s = GEOMV_TO_FLOAT( itervtx->U ) * scale_u;
			glvtxs[ v ].t = GEOMV_TO_FLOAT( itervtx->V ) * scale_v;
			glvtxs[ v ].p = 0.0f;
			glvtxs[ v ].q = GEOMV_TO_FLOAT( itervtx->W );
		}

	} else {

		// convert specified number of vertices
		for ( int v = 0; v < num; v++ ) {

			IterVertex2 *itervtx = itervtxs[ v ];
			ASSERT( itervtx != NULL );

			glvtxs[ v ].x = RASTV_TO_FLOAT( itervtx->X );
			glvtxs[ v ].y = RASTV_TO_FLOAT( itervtx->Y );
			glvtxs[ v ].z = RASTV_TO_FLOAT( itervtx->Z ) * OPENGL_DEPTH_RANGE;

			*(dword*)&glvtxs[ v ].r = *(dword*)&itervtx->R;
		}
	}
}


// convert vector of IterVertex3's to vector of GLVertex3's -------------------
//
void RO_IterVertex3GLVertex( GLVertex3 *glvtxs, IterVertex3 *itervtxs, int num, GLTexInfo *texinfo )
{
	ASSERT( glvtxs != NULL );
	ASSERT( itervtxs != NULL );

	if ( texinfo != NULL ) {

		float scale_u = texinfo->coscale;
		float scale_v = texinfo->aratio * scale_u;

		// convert specified number of vertices
		for ( int v = 0; v < num; v++ ) {

			glvtxs[ v ].x = GEOMV_TO_FLOAT( itervtxs[ v ].X );
			glvtxs[ v ].y = GEOMV_TO_FLOAT( itervtxs[ v ].Y );
			glvtxs[ v ].z = GEOMV_TO_FLOAT( itervtxs[ v ].Z ) * OPENGL_DEPTH_RANGE;

			*(dword*)&glvtxs[ v ].r = *(dword*)&itervtxs[ v ].R;

			glvtxs[ v ].s = GEOMV_TO_FLOAT( itervtxs[ v ].U ) * scale_u;
			glvtxs[ v ].t = GEOMV_TO_FLOAT( itervtxs[ v ].V ) * scale_v;
			glvtxs[ v ].p = 0.0f;
			glvtxs[ v ].q = GEOMV_TO_FLOAT( itervtxs[ v ].W );
		}

	} else {

		// convert specified number of vertices
		for ( int v = 0; v < num; v++ ) {

			glvtxs[ v ].x = GEOMV_TO_FLOAT( itervtxs[ v ].X );
			glvtxs[ v ].y = GEOMV_TO_FLOAT( itervtxs[ v ].Y );
			glvtxs[ v ].z = GEOMV_TO_FLOAT( itervtxs[ v ].Z ) * OPENGL_DEPTH_RANGE;

			*(dword*)&glvtxs[ v ].r = *(dword*)&itervtxs[ v ].R;
		}
	}
}


// convert vector of IterVertex3 pointers to vector of GLVertex3's ------------
//
void RO_IterVertex3GLVertexRef( GLVertex3 *glvtxs, IterVertex3 **itervtxs, int num, GLTexInfo *texinfo )
{
	ASSERT( glvtxs != NULL );
	ASSERT( itervtxs != NULL );

	if ( texinfo != NULL ) {

		float scale_u = texinfo->coscale;
		float scale_v = texinfo->aratio * scale_u;

		// convert specified number of vertices
		for ( int v = 0; v < num; v++ ) {

			IterVertex3 *itervtx = itervtxs[ v ];
			ASSERT( itervtx != NULL );

			glvtxs[ v ].x = GEOMV_TO_FLOAT( itervtx->X );
			glvtxs[ v ].y = GEOMV_TO_FLOAT( itervtx->Y );
			glvtxs[ v ].z = GEOMV_TO_FLOAT( itervtx->Z ) * OPENGL_DEPTH_RANGE;

			*(dword*)&glvtxs[ v ].r = *(dword*)&itervtx->R;

			glvtxs[ v ].s = GEOMV_TO_FLOAT( itervtx->U ) * scale_u;
			glvtxs[ v ].t = GEOMV_TO_FLOAT( itervtx->V ) * scale_v;
			glvtxs[ v ].p = 0.0f;
			glvtxs[ v ].q = GEOMV_TO_FLOAT( itervtx->W );
		}

	} else {

		// convert specified number of vertices
		for ( int v = 0; v < num; v++ ) {

			IterVertex3 *itervtx = itervtxs[ v ];
			ASSERT( itervtx != NULL );

			glvtxs[ v ].x = GEOMV_TO_FLOAT( itervtx->X );
			glvtxs[ v ].y = GEOMV_TO_FLOAT( itervtx->Y );
			glvtxs[ v ].z = GEOMV_TO_FLOAT( itervtx->Z ) * OPENGL_DEPTH_RANGE;

			*(dword*)&glvtxs[ v ].r = *(dword*)&itervtx->R;
		}
	}
}


// convert internal aspect ratio spec to numerical ratio ----------------------
//
float gl_tex_aspect[] = {

	1.0f,		// TEXGEO_ASPECT_1x1

	2.0f,		// TEXGEO_ASPECT_2x1
	4.0f,		// TEXGEO_ASPECT_4x1
	8.0f,		// TEXGEO_ASPECT_8x1
	16.0f,		// TEXGEO_ASPECT_16x1
	32.0f,		// TEXGEO_ASPECT_32x1
	64.0f,		// TEXGEO_ASPECT_64x1
	128.0f,		// TEXGEO_ASPECT_128x1
	256.0f,		// TEXGEO_ASPECT_256x1
	512.0f,		// TEXGEO_ASPECT_512x1
	1024.0f,	// TEXGEO_ASPECT_1024x1

	1/2.0f,		// TEXGEO_ASPECT_1x2
	1/4.0f,		// TEXGEO_ASPECT_1x4
	1/8.0f,		// TEXGEO_ASPECT_1x8
	1/16.0f,	// TEXGEO_ASPECT_1x16
	1/32.0f,	// TEXGEO_ASPECT_1x32
	1/64.0f,	// TEXGEO_ASPECT_1x64
	1/128.0f,	// TEXGEO_ASPECT_1x128
	1/256.0f,	// TEXGEO_ASPECT_1x256
	1/512.0f,	// TEXGEO_ASPECT_1x512
	1/1024.0f,	// TEXGEO_ASPECT_1x1024
};


// conversion tables ----------------------------------------------------------
//
float	gl_tgr_aspect[] = { 8.0f, 4.0f, 2.0f, 1.0f, 0.5f, 0.25f, 0.125f, 1.0f };
int		gl_tgr_invert[] = {    0,    0,    0,    0,    2,     4,      8,    1 };
float	gl_tex_scales[] = {

	// small textures (1..256)
	1/256.0, 2/256.0, 4/256.0, 8/256.0, 16/256.0, 32/256.0, 64/256.0, 128/256.0, 256/256.0,

	// large textures (512..1024)
	2/1024.0, 1/1024.0,
};

// texture geometry:	32x32      64x32      64x64      128x64      128x128     256x128
float	gl_csctab[] = { 1/32.0,    1/64.0,    1/64.0,    1/128.0,    1/128.0,    1/256.0    };
float	gl_asptab[] = { 1.0f,      2.0f,      1.0f,      2.0f,       1.0f,       2.0f       };
int		gl_lodtab[] = { TEXLOD_32, TEXLOD_64, TEXLOD_64, TEXLOD_128, TEXLOD_128, TEXLOD_256 };


// convert TextureMap to GLTexInfo and determine coordinate scale factor ------
//
void RO_TextureMap2GLTexInfo( GLTexInfo *texinfo, TextureMap *texmap )
{
	ASSERT( texinfo != NULL );
	ASSERT( texmap != NULL );

	// fill texinfo structure
	texinfo->texmap = texmap;
	texinfo->data	= texmap->BitMap;
	texinfo->width	= 1 << texmap->Width;
	texinfo->height	= 1 << texmap->Height;
	texinfo->format	= texmap->TexelFormat;

	dword geometry = texmap->Geometry;
	if ( texmap->Flags & TEXFLG_EXT_GEOMETRY ) {

		// extract scale factor
		int scaleindx = ( geometry & TEXGEO_SCALE2MASK ) >> TEXGEO_SCALE2SHIFT;
		ASSERT( scaleindx < 11 );
		texinfo->coscale = gl_tex_scales[ scaleindx ];

		// extract aspect ratio
		int aindx = geometry & TEXGEO_ASPECTMASK;
		if ( geometry & TEXGEO_GLIDEASPECT ) {

			// use glide aspect ratio
			ASSERT( aindx <= ( TEXGEO_ASPECT_GR_1x8 & TEXGEO_ASPECTMASK ) );
			texinfo->aratio = gl_tgr_aspect[ aindx ];
			if ( gl_tgr_invert[ aindx ] ) {
				texinfo->coscale *= gl_tgr_invert[ aindx ];
			}

		} else {

			// use abstract aspect ratio
			ASSERT( aindx <= ( TEXGEO_ASPECT_1x1024 & TEXGEO_ASPECTMASK ) );
			texinfo->aratio = gl_tex_aspect[ aindx ];
			if ( aindx > ( TEXGEO_ASPECT_1024x1 & TEXGEO_ASPECTMASK ) ) {
				aindx -= TEXGEO_ASPECT_1024x1 & TEXGEO_ASPECTMASK;
				texinfo->coscale *= gl_tex_aspect[ aindx ];
			}
		}

		if ( texmap->Flags & TEXFLG_LODRANGE_VALID ) {
			texinfo->lodsmall = texmap->LOD_small;
			texinfo->lodlarge = texmap->LOD_large;
		} else {
			PANIC( "TEXFLG_EXT_GEOMETRY without TEXFLG_LODRANGE_VALID." );
		}

	} else {

		ASSERT( TEXGEO_CODE_32x32   == 0 );
		ASSERT( TEXGEO_CODE_256x256 == 6 );

		// determine geometry from geometry code
		if ( geometry < TEXGEO_CODE_256x256 ) {

			texinfo->coscale  = gl_csctab[ geometry ];
			texinfo->lodsmall = gl_lodtab[ geometry ];
			texinfo->lodlarge = gl_lodtab[ geometry ];
			texinfo->aratio   = gl_asptab[ geometry ];

		} else {
			PANIC( 0 );
		}
	}
}


// render two dimensional rectangle -------------------------------------------
//
void RO_Render2DRectangle( sgrid_t putx, sgrid_t puty, float srcw, float srch, dword dstw, dword dsth, dword zvalue, colrgba_s *color )
{
	GLVertex3 glvtxs[ 4 ];

	glvtxs[ 0 ].x = putx;
	glvtxs[ 0 ].y = puty;
	glvtxs[ 0 ].z = ( zvalue & 0xffff ) / 65536.0f;
	glvtxs[ 0 ].r = color ? color->R : 255;
	glvtxs[ 0 ].g = color ? color->G : 255;
	glvtxs[ 0 ].b = color ? color->B : 255;
	glvtxs[ 0 ].a = color ? color->A : 255;
	glvtxs[ 0 ].s = 0;
	glvtxs[ 0 ].t = 0;

	glvtxs[ 1 ].x = putx + dstw;
	glvtxs[ 1 ].y = puty;
	glvtxs[ 1 ].z = ( zvalue & 0xffff ) / 65536.0f;
	glvtxs[ 1 ].r = glvtxs[ 0 ].r;
	glvtxs[ 1 ].g = glvtxs[ 0 ].g;
	glvtxs[ 1 ].b = glvtxs[ 0 ].b;
	glvtxs[ 1 ].a = glvtxs[ 0 ].a;
	glvtxs[ 1 ].s = srcw;
	glvtxs[ 1 ].t = 0;

	glvtxs[ 2 ].x = putx + dstw;
	glvtxs[ 2 ].y = puty + dsth;
	glvtxs[ 2 ].z = ( zvalue & 0xffff ) / 65536.0f;
	glvtxs[ 2 ].r = glvtxs[ 0 ].r;
	glvtxs[ 2 ].g = glvtxs[ 0 ].g;
	glvtxs[ 2 ].b = glvtxs[ 0 ].b;
	glvtxs[ 2 ].a = glvtxs[ 0 ].a;
	glvtxs[ 2 ].s = srcw;
	glvtxs[ 2 ].t = srch;

	glvtxs[ 3 ].x = putx;
	glvtxs[ 3 ].y = puty + dsth;
	glvtxs[ 3 ].z = ( zvalue & 0xffff ) / 65536.0f;
	glvtxs[ 3 ].r = glvtxs[ 0 ].r;
	glvtxs[ 3 ].g = glvtxs[ 0 ].g;
	glvtxs[ 3 ].b = glvtxs[ 0 ].b;
	glvtxs[ 3 ].a = glvtxs[ 0 ].a;
	glvtxs[ 3 ].s = 0;
	glvtxs[ 3 ].t = srch;

	RO_ClientState( VTXARRAY_VERTICES | VTXARRAY_COLORS | VTXARRAY_TEXCOORDS );
	RO_ArrayMakeCurrent( VTXPTRS_NONE, NULL );

	glVertexPointer( 3, GL_FLOAT, sizeof( GLVertex3 ), &glvtxs->x );
	glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( GLVertex3 ), &glvtxs->r );
	glTexCoordPointer( 2, GL_FLOAT, sizeof( GLVertex3 ), &glvtxs->s );

	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

//	RO_ClientState( VTXARRAY_NONE );
}


// pre cache textures ---------------------------------------------------------
//
int R_PrecacheTextures()
{
	if ( !AUX_ENABLE_TEXTURE_PRECACHING )
		return 0;

	//CON_AddMessage( "precaching compressed textures..." );

	int precachecount = 0;

	int old_disable_buffer_clear  = AUX_DISABLE_BUFFER_CLEAR;
	int old_disable_zbuffer_clear = AUX_DISABLE_ZBUFFER_CLEAR;

	AUX_DISABLE_BUFFER_CLEAR = 0;
	AUX_DISABLE_ZBUFFER_CLEAR = 0;

	VIDs_ClearRenderBuffer();

	TextureMap* tex_precache = FetchTextureMap( "precache.3df" );
	
	if ( tex_precache != NULL ) {
		GLTexInfo texinfo;
		
		RO_TextureMap2GLTexInfo( &texinfo, tex_precache );
		
		// configure rasterizer
		dword itertype	= iter_rgbatexa | iter_alphablend;
		dword raststate	= rast_chromakeyoff;
		dword rastmask	= rast_nomask;
		RO_InitRasterizerState( itertype, raststate, rastmask );

		// enforce texel source
		RO_SelectTexelSource( &texinfo );
		
		//FIXME: use iter to draw the texture
		
		int xOffset = ( Screen_Width  - texinfo.width  ) / 2;
		int yOffset = ( Screen_Height - texinfo.height ) / 2;
		
		GLshort vertices[] = {
			xOffset, yOffset,
			xOffset + texinfo.width, yOffset,
			xOffset + texinfo.width, yOffset + texinfo.height,
			xOffset, yOffset + texinfo.height,
		};
		
		GLshort texcoords[] = {0, 0, 1, 0, 1, 1, 0, 1};
		
		RO_ClientState(VTXARRAY_VERTICES | VTXARRAY_TEXCOORDS);
		
		RO_ArrayMakeCurrent(VTXPTRS_NONE, NULL);
		glVertexPointer(2, GL_SHORT, 0, vertices);
		glTexCoordPointer(2, GL_SHORT, 0, texcoords);

		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}
	
	VIDs_CommitRenderBuffer();

	// save old sound setting and disable sound while precaching
	int OldSoundDisabled = SoundDisabled;
	SoundDisabled = TRUE;

	// scan entire table of textures
	for ( int texid = 0; texid < NumLoadedTextures; texid++ ) {
		
		// get pointer to texture map
		TextureMap* texmap = TextureInfo[ texid ].texpointer;
		
		// check whether we want to compress the texture
		if ( texmap->Flags & TEXFLG_DO_COMPRESSION ) {
			// MSGOUT("precaching texture %s\n", texmap->TexMapName);
			
			// convert to OpenGL texture
			GLTexInfo texinfo;
			RO_TextureMap2GLTexInfo( &texinfo, texmap );

			// enforce texel source, cache texture info, and generate mipmaps
			RO_SelectTexelSource( &texinfo );

			precachecount++;
		}
	}

	// restore previous sound setting
	SoundDisabled = OldSoundDisabled;

	/*
	char szBuffer[ 128 ];
	sprintf( szBuffer, "precached %d compressed textures", precachecount );
	CON_AddMessage( szBuffer );
	*/

	AUX_DISABLE_BUFFER_CLEAR  = old_disable_buffer_clear;
	AUX_DISABLE_ZBUFFER_CLEAR = old_disable_zbuffer_clear;

	return precachecount;
}
