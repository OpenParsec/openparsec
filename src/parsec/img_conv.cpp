/*
 * PARSEC - Image To Texture Conversion Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:35 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   2001
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
#include "img_conv.h"

// proprietary module headers
#include "e_color.h"
#include "img_3df.h"
#include "img_supp.h"

// input image mirroring flag (usage depends on type of image file) -----------
//
PUBLIC int image_mirror_vertically = TRUE;


// input image pixel inversion flag -------------------------------------------
//
PUBLIC int image_invert_pixels = FALSE;


// input image texture compression flag ---------------------------------------
//
PUBLIC int image_compress_data = FALSE;


// possible lods read from image file -----------------------------------------
//
PUBLIC int image_lods[] = {

	TEXLOD_1,
	TEXLOD_2,
	TEXLOD_4,
	TEXLOD_8,
	TEXLOD_16,
	TEXLOD_32,
	TEXLOD_64,
	TEXLOD_128,
	TEXLOD_256,
	TEXLOD_512,
	TEXLOD_1024,
	TEXLOD_2048,
};


// build (append) mipmaps in already allocated memblock -----------------------
//
PRIVATE
void IMG_BuildMipmaps( char *baselod, int texw, int texh, int pixsiz, int lodlarge, int lodsmall )
{
	ASSERT( baselod != NULL );
	ASSERT( lodlarge >= lodsmall );

	char *dst = baselod + texw * texh * pixsiz;
	char *src = baselod;

	// create levels behind base lod
	for ( int lev = lodlarge - 1; lev > lodsmall; lev-- ) {

		int prew = texw;
		int preh = texh;

		if ( texw > 1 )
			texw >>= 1;
		if ( texh > 1 )
			texh >>= 1;

		// create a single mipmap level
		if ( pixsiz == 4 ) {

			// rgba map
			for ( int cury = 0; cury < texh; cury++ ) {
				for ( int curx = 0; curx < texw; curx++ ) {

					// simply average four texels
					colrgba_s *t0 = &((colrgba_s *)src)[ 0 ];
					colrgba_s *t1 = &((colrgba_s *)src)[ 1 ];
					colrgba_s *t2 = &((colrgba_s *)src)[ prew + 0 ];
					colrgba_s *t3 = &((colrgba_s *)src)[ prew + 1 ];

					colrgba_s *dt = &((colrgba_s *)dst)[ curx ];

					// rectangular maps only average two texels
					// after smaller side hits one
					if ( prew == 1 ) {
						t1 = t0;
						t3 = t2;
					} else if ( preh == 1 ) {
						t2 = t0;
						t3 = t1;
					}

					int r = t0->R + t1->R + t2->R + t3->R;
					dt->R = ( r + 0x02 ) >> 2;

					int g = t0->G + t1->G + t2->G + t3->G;
					dt->G = ( g + 0x02 ) >> 2;

					int b = t0->B + t1->B + t2->B + t3->B;
					dt->B = ( b + 0x02 ) >> 2;

					int a = t0->A + t1->A + t2->A + t3->A;
					dt->A = ( a + 0x02 ) >> 2;

					src += ( prew > 1 ) ? pixsiz * 2 : pixsiz;
				}

				src += ( preh > 1 ) ? prew * pixsiz : 0;
				dst += texw * pixsiz;
			}

		} else if ( pixsiz == 3 ) {

			// rgb map
			for ( int cury = 0; cury < texh; cury++ ) {
				for ( int curx = 0; curx < texw; curx++ ) {

					// simply average four texels
					colrgb_s *t0 = &((colrgb_s *)src)[ 0 ];
					colrgb_s *t1 = &((colrgb_s *)src)[ 1 ];
					colrgb_s *t2 = &((colrgb_s *)src)[ prew + 0 ];
					colrgb_s *t3 = &((colrgb_s *)src)[ prew + 1 ];

					colrgb_s *dt = &((colrgb_s *)dst)[ curx ];

					// rectangular maps only average two texels
					// after smaller side hits one
					if ( prew == 1 ) {
						t1 = t0;
						t3 = t2;
					} else if ( preh == 1 ) {
						t2 = t0;
						t3 = t1;
					}

					int r = t0->R + t1->R + t2->R + t3->R;
					dt->R = ( r + 0x02 ) >> 2;

					int g = t0->G + t1->G + t2->G + t3->G;
					dt->G = ( g + 0x02 ) >> 2;

					int b = t0->B + t1->B + t2->B + t3->B;
					dt->B = ( b + 0x02 ) >> 2;

					src += ( prew > 1 ) ? pixsiz * 2 : pixsiz;
				}

				src += ( preh > 1 ) ? prew * pixsiz : 0;
				dst += texw * pixsiz;
			}

		} else {

			// only rgba or rgb
			PANIC( 0 );
		}
	}
}


// strip mipmap levels from most detailed to desired upper boundary -----------
//
PRIVATE
char *IMG_StripMipmaps( char *texture, int width, int height, int lodloaded, int lodlarge, int *numtexels, int pixsiz )
{
	ASSERT( texture != NULL );
	ASSERT( lodloaded >= lodlarge );
	ASSERT( numtexels != NULL );

	if ( lodloaded == lodlarge ) {
		return texture;
	}

	// determine number of texels to strip
	int striptexels = ( lodloaded > lodlarge ) ?
		IMG_CountMipmapTexels( width, height, lodloaded, lodlarge + 1 ) : 0;

	// new number of texels
	*numtexels -= striptexels;

	// allocate new buffer
	size_t strippedsize = *numtexels * pixsiz;
	char *strippedbuff = (char *) ALLOCMEM( strippedsize );
	if ( strippedbuff == NULL ) {
		FREEMEM( texture );
		return NULL;
	}

	// copy into new buffer without leading mipmap levels
	memcpy( strippedbuff, texture + striptexels * pixsiz, strippedsize );
	FREEMEM( texture );

	return strippedbuff;
}


// pixel sizes of non-glide texture formats -----------------------------------
//
static int texfmt_to_pixsiz[] = {

	1,			// TEXFMT_STANDARD
	2,			// TEXFMT_RGB_565
	2,			// TEXFMT_RGBA_1555
	3,			// TEXFMT_RGB_888
	4,			// TEXFMT_RGBA_8888
	1,			// TEXFMT_ALPHA_8
	1,			// TEXFMT_INTENSITY_8
	1,			// TEXFMT_LUMINANCE_8
};

// check format of texture; convert if it cannot be used as is ----------------
//
PRIVATE
char *IMG_CheckTextureFormat( char *texture, int numtexels, format_3df_s *texfmtgr )
{
	ASSERT( texture != NULL );

	return texture;
}


// communication info for retiling --------------------------------------------
//
struct tileinfo_s {

	char*		tilesrc;
	char*		tiledst;
	int			pixsiz;
	int			stride;
	int			tilewidth;
	int			tileheight;
	int			tileadv;

	int			u;
	int			v;
	int			ulead;
	int			ustep;

	retileinfo_s* retileinfo;
};


// copy a single tile from a source into a destination buffer -----------------
//
PRIVATE
void IMG_CopyTile( tileinfo_s *tileinfo )
{
	ASSERT( tileinfo != NULL );

	tileinfo->ulead = tileinfo->retileinfo->tilewidth;
	tileinfo->ustep = 0;
	int row = 0;
	int pos = 0;
	for ( row = tileinfo->tileheight; row > 0; row-- ) {

		if ( tileinfo->pixsiz == 4 ) {

			colrgba_s *tsrc = (colrgba_s *) tileinfo->tilesrc;
			colrgba_s *tdst = (colrgba_s *) tileinfo->tiledst;
			dword srcpix = 0;	

			for (  pos = 0; pos < tileinfo->retileinfo->tilewidth; pos++ ) {

				srcpix = *(long*)&tsrc[ pos ];
				if ( image_invert_pixels )
					srcpix ^= 0xffffffff;
				int opaque = ( ( srcpix & SWAP_32( 0x00ffffff ) ) != 0x00000000 );
				if ( opaque ) {
					if ( tileinfo->ulead > pos )
						tileinfo->ulead = pos;
					if ( tileinfo->ustep < pos )
						tileinfo->ustep = pos;
					dword newalpha = ( SWAP_32( srcpix ) & 0x000000ff ) << 24;
					srcpix  = SWAP_32( 0x00ffffff );
					srcpix |= SWAP_32( newalpha );
				} else {
					srcpix &= SWAP_32( 0x00ffffff );
				}
				*(dword*)&tdst[ pos ] = srcpix;
 			}	

		} else if ( tileinfo->pixsiz == 3 ) {

			colrgb_s *tsrc = (colrgb_s *) tileinfo->tilesrc;
			colrgb_s *tdst = (colrgb_s *) tileinfo->tiledst;

			for (  pos = 0; pos < tileinfo->retileinfo->tilewidth; pos++ ) {

				if ( image_invert_pixels ) {
					tsrc[ pos ].R ^= 0xff;
					tsrc[ pos ].G ^= 0xff;
					tsrc[ pos ].B ^= 0xff;
				}
				int opaque = (
					( tsrc[ pos ].R != 0x00 ) ||
					( tsrc[ pos ].G != 0x00 ) ||
					( tsrc[ pos ].B != 0x00 ) );
				if ( opaque ) {
					if ( tileinfo->ulead > pos )
						tileinfo->ulead = pos;
					if ( tileinfo->ustep < pos )
						tileinfo->ustep = pos;
				}
				tdst[ pos ].R = tsrc[ pos ].R;
				tdst[ pos ].G = tsrc[ pos ].G;
				tdst[ pos ].B = tsrc[ pos ].B;
			}

		} else {

			PANIC( 0 );
		}

		tileinfo->tilesrc += tileinfo->tileadv;
		tileinfo->tiledst += tileinfo->stride;
	}
}


// store geometry of a single texfont character -------------------------------
//
PRIVATE
void IMG_StoreTexchar( texchar_s *texchar, tileinfo_s *tileinfo )
{
	ASSERT( texchar != NULL );
	ASSERT( tileinfo != NULL );

	// fill texfont geometry table
	tileinfo->ulead -= tileinfo->retileinfo->spacing;
	if ( tileinfo->ulead < 0 ) {
		tileinfo->ulead = 0;
	}

	tileinfo->ustep -= tileinfo->ulead;

	tileinfo->ustep += tileinfo->retileinfo->spacing;
	if ( tileinfo->ustep > tileinfo->retileinfo->tilewidth - 1 ) {
		tileinfo->ustep = tileinfo->retileinfo->tilewidth - 1;
	}

	texchar->tex_u     = tileinfo->u;
	texchar->tex_v     = tileinfo->v;
	texchar->tex_id    = 0;
	texchar->tex_ulead = tileinfo->ulead;
	texchar->tex_ustep = tileinfo->ustep + 1;
}


// retile a source texture into a destination texture -------------------------
//
PRIVATE
void IMG_FillTextureBufferRetiled( char *dstbuff, char *texbitmap, int pixsiz, int texwidth, int texheight, retileinfo_s *retileinfo )
{
	ASSERT( dstbuff != NULL );
	ASSERT( texbitmap != NULL );
	ASSERT( retileinfo != NULL );

	// create texfont if desired
	texfont_s *texfont  = retileinfo->texfont;
	texchar_s *texchars = NULL;
	if ( texfont != NULL ) {

		texfont->height = retileinfo->tileheight;
		texfont->numtextures = 1;
		texfont->numtexchars = 0;
		texchars = (texchar_s *) ALLOCMEM( 256 * sizeof( texchar_s ) );
		if ( texchars == NULL )
			OUTOFMEM( 0 );
		texfont->texchars = texchars;
	}

	texwidth *= pixsiz;

	int srcwidth   = retileinfo->srcwidth * pixsiz;
	int srcheight  = retileinfo->srcheight;
	int tilewidth  = retileinfo->tilewidth * pixsiz;
	int tileheight = retileinfo->tileheight;

	char *dst = dstbuff;
	char *src = texbitmap;

	int tileadv = srcwidth;
	int rowadv  = tileheight * srcwidth;

	if ( image_mirror_vertically ) {
		src += ( srcheight - 1 ) * srcwidth;
		tileadv = -tileadv;
		rowadv  = -rowadv;
	}

	int charcount = 0;

	int curdstx = 0;
	int curdsty = 0;

	char *rowdst = dst;
	for ( int cursrcy = 0; ( cursrcy += tileheight ) <= srcheight; ) {

		char *rowsrc = src;
		for ( int cursrcx = 0; ( cursrcx += tilewidth ) <= srcwidth; ) {

			if ( ( curdstx += tilewidth ) > texwidth ) {
				curdstx  = tilewidth;
				curdsty += tileheight;
				// to break out
				if ( curdsty >= texheight )
					cursrcy = srcheight;
				dst += tileheight * texwidth;
				rowdst = dst;
			}

			tileinfo_s tileinfo;
			tileinfo.tilesrc	= rowsrc;
			tileinfo.tiledst	= rowdst;
			tileinfo.pixsiz		= pixsiz;
			tileinfo.stride		= texwidth;
			tileinfo.tilewidth	= tilewidth;
			tileinfo.tileheight	= tileheight;
			tileinfo.tileadv	= tileadv;
			tileinfo.retileinfo	= retileinfo;

			IMG_CopyTile( &tileinfo );

			if ( texfont != NULL ) {
				tileinfo.u = ( curdstx - tilewidth ) / pixsiz;
				tileinfo.v = curdsty;
				IMG_StoreTexchar( &texchars[ charcount++ ], &tileinfo );
			}

			rowsrc += tilewidth;
			rowdst += tilewidth;
		}

		src += rowadv;
	}

	if ( texfont != NULL ) {
		texfont->numtexchars = charcount;
	}
}


// copy a source texture into a destination texture (mirror if necessary) -----
//
PRIVATE
void IMG_FillTextureBuffer( char *dstbuff, char *texbitmap, int pixsiz, int width, int height )
{
	ASSERT( dstbuff != NULL );
	ASSERT( texbitmap != NULL );

	// copy image into destination (mirrored if required)
	if ( image_mirror_vertically ) {

		// mirror texture vertically (src->dst)
		char *dst = dstbuff;
		char *src = texbitmap + ( height - 1 ) * width * pixsiz;
		for ( int vpos = 0; vpos < height; vpos++ ) {
			memcpy( dst, src, width * pixsiz );
			dst += width * pixsiz;
			src -= width * pixsiz;
		}

	} else {

		// copy as is
		memcpy( dstbuff, texbitmap, width * height * pixsiz );
	}
}


// convert texture image data as contained in file to internal format ---------
//
char *IMG_ConvertTextureImageData( imgtotexdata_s *imgtotexdata, int swaprgba )
{
	ASSERT( imgtotexdata != NULL );

	ASSERT( imgtotexdata->texbitmap != NULL );
	ASSERT( imgtotexdata->lodloaded >= imgtotexdata->lodlarge );
	ASSERT( imgtotexdata->lodlarge >= imgtotexdata->lodsmall );

	ASSERT( ( imgtotexdata->format == TEXFMT_RGB_888 ) ||
			( imgtotexdata->format == TEXFMT_RGBA_8888 ) );
	int pixsiz = texfmt_to_pixsiz[ imgtotexdata->format ];

	// determine size including mipmaps (if any)
	int numtexels = IMG_CountMipmapTexels(
		imgtotexdata->width, imgtotexdata->height,
		imgtotexdata->lodloaded, imgtotexdata->lodsmall );

	// alloc destination image buffer
	size_t datasize = numtexels * pixsiz;
	char *dstbuff = (char *) ALLOCMEM( datasize );
	if ( dstbuff == NULL ) {
		FREEMEM( imgtotexdata->texbitmap );
		return NULL;
	}

	// transfer image into destination image buffer
	if ( imgtotexdata->retileinfo != NULL ) {
		IMG_FillTextureBufferRetiled( dstbuff,
			imgtotexdata->texbitmap, pixsiz,
			imgtotexdata->width, imgtotexdata->height,
			imgtotexdata->retileinfo );
	} else {
		IMG_FillTextureBuffer( dstbuff,
			imgtotexdata->texbitmap, pixsiz,
			imgtotexdata->width, imgtotexdata->height );
	}

	// free loaded data
	FREEMEM( imgtotexdata->texbitmap );
	imgtotexdata->texbitmap = NULL;

	// perform other necessary format conversions
	if ( pixsiz == 4 ) {

		// convert from (B,G,R,A) to (R,G,B,A)
		colrgba_s *scan = (colrgba_s *) dstbuff;
		int ict = imgtotexdata->width * imgtotexdata->height;
		for ( ; ict > 0; ict--, scan++ ) {
			colrgba_s col = *scan;

			if ( swaprgba ) {
				scan->R = col.B;
				scan->G = col.G;
				scan->B = col.R;
				scan->A = col.A;
			} else {  
				scan->R = col.R;
				scan->G = col.G;
				scan->B = col.B;
				scan->A = col.A;
			}
		}

	} else if ( pixsiz == 3 ) {

		// convert from (B,G,R) to (R,G,B)
		colrgb_s *scan = (colrgb_s *) dstbuff;
		int ict = imgtotexdata->width * imgtotexdata->height;
		for ( ; ict > 0; ict--, scan++ ) {
			colrgb_s col = *scan;

			if ( swaprgba ) {
				scan->R = col.B;
				scan->G = col.G;
				scan->B = col.R;
			} else {
				scan->R = col.R;
				scan->G = col.G;
				scan->B = col.B;
			}
		}
	}

	// create mipmaps in already reserved memory
	IMG_BuildMipmaps( dstbuff,
		imgtotexdata->width, imgtotexdata->height, pixsiz,
		imgtotexdata->lodloaded, imgtotexdata->lodsmall );

	// strip mipmap levels that are too large
	dstbuff = IMG_StripMipmaps( dstbuff,
		imgtotexdata->width, imgtotexdata->height,
		imgtotexdata->lodloaded, imgtotexdata->lodlarge,
		&numtexels, pixsiz );
	if ( dstbuff == NULL ) {
		return NULL;
	}

	// convert texture format if necessary for current renderer
	dstbuff = IMG_CheckTextureFormat(
		dstbuff, numtexels, imgtotexdata->texfmtgr );
	if ( dstbuff == NULL ) {
		return NULL;
	}

	return dstbuff;
}



