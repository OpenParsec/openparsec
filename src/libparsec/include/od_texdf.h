/*
 * PARSEC HEADER (OBJECT)
 * Texture Format Specifications V1.15
 *
 * Copyright (c) Markus Hadwiger 1998-2000
 * All Rights Reserved.
 */

#ifndef _OD_TEXDF_H_
#define _OD_TEXDF_H_


// texture flags

#define TEXFLG_NONE					0x0000  // no special flags
#define TEXFLG_EXT_GEOMETRY			0x0001	// extended geometry specification
#define TEXFLG_LODRANGE_VALID		0x0002	// lod range (small to large) valid
#define TEXFLG_GLIDE_TEXTURE		0x0004	// directly usable as glide texture
#define TEXFLG_IS_COMPRESSED		0x0010	// texture data is compressed
#define TEXFLG_DO_COMPRESSION		0x0020	// texture data should be compressed
#define TEXFLG_CACHE_MAY_FREE		0x0100  // cache may free texture data


// texture map formats

#define TEXFMT_FORMATMASK		0x0000ffff	// mask to extract actual format
#define TEXFMT_FLAGSMASK		0xffff0000	// mask to extract additional flags
#define	TEXFMT_PALETTEDTEXTURE	0x00010000	// format needs texture palette
#define	TEXFMT_GLIDEFORMAT		0x00020000	// format spec identical to glide

#define TEXFMT_STANDARD				0x0000	// 8-bit indexes into global palette
#define TEXFMT_RGB_565				0x0001	// 16-bit direct color
#define TEXFMT_RGBA_1555			0x0002	// 16-bit direct color with alpha
#define TEXFMT_RGB_888				0x0003	// 24-bit direct color
#define TEXFMT_RGBA_8888			0x0004	// 32-bit direct color with alpha
#define TEXFMT_ALPHA_8				0x0005	// 8-bit alpha only
#define TEXFMT_INTENSITY_8			0x0006	// 8-bit intensity only
#define TEXFMT_LUMINANCE_8			0x0007	// 8-bit luminance only

#define TEXFMT_GR_8BIT					( 0x0000 | TEXFMT_GLIDEFORMAT )
#define TEXFMT_GR_RGB_332				TEXFMT_GR_8BIT
#define TEXFMT_GR_YIQ_422				( 0x0001 | TEXFMT_GLIDEFORMAT )
#define TEXFMT_GR_ALPHA_8				( 0x0002 | TEXFMT_GLIDEFORMAT )
#define TEXFMT_GR_INTENSITY_8			( 0x0003 | TEXFMT_GLIDEFORMAT )
#define TEXFMT_GR_ALPHA_INTENSITY_44	( 0x0004 | TEXFMT_GLIDEFORMAT )
#define TEXFMT_GR_P_8					( 0x0005 | TEXFMT_GLIDEFORMAT | TEXFMT_PALETTEDTEXTURE )
#define TEXFMT_GR_RSVD0					( 0x0006 | TEXFMT_GLIDEFORMAT )
#define TEXFMT_GR_RSVD1					( 0x0007 | TEXFMT_GLIDEFORMAT )
#define TEXFMT_GR_16BIT					( 0x0008 | TEXFMT_GLIDEFORMAT )
#define TEXFMT_GR_ARGB_8332				TEXFMT_GR_16BIT
#define TEXFMT_GR_AYIQ_8422				( 0x0009 | TEXFMT_GLIDEFORMAT )
#define TEXFMT_GR_RGB_565				( 0x000a | TEXFMT_GLIDEFORMAT )
#define TEXFMT_GR_ARGB_1555				( 0x000b | TEXFMT_GLIDEFORMAT )
#define TEXFMT_GR_ARGB_4444				( 0x000c | TEXFMT_GLIDEFORMAT )
#define TEXFMT_GR_ALPHA_INTENSITY_88	( 0x000d | TEXFMT_GLIDEFORMAT )
#define TEXFMT_GR_AP_88					( 0x000e | TEXFMT_GLIDEFORMAT | TEXFMT_PALETTEDTEXTURE )
#define	TEXFMT_GR_RSVD2					( 0x000f | TEXFMT_GLIDEFORMAT )


// texture lod specification

#define TEXLOD_1					0x0000	// lowest level: 1x1
#define TEXLOD_2					0x0001
#define TEXLOD_4					0x0002
#define TEXLOD_8					0x0003
#define TEXLOD_16					0x0004
#define TEXLOD_32					0x0005
#define TEXLOD_64					0x0006
#define TEXLOD_128					0x0007
#define TEXLOD_256					0x0008
#define TEXLOD_512					0x0009
#define TEXLOD_1024					0x000a
#define TEXLOD_2048					0x000b	// highest level: 2048x2048


// texture geometry codes (legacy geometry spec)

#define TEXGEO_CODE_32x32		0x00000000
#define TEXGEO_CODE_64x32		0x00000001
#define TEXGEO_CODE_64x64		0x00000002
#define TEXGEO_CODE_128x64		0x00000003
#define TEXGEO_CODE_128x128		0x00000004
#define TEXGEO_CODE_256x128		0x00000005
#define TEXGEO_CODE_256x256		0x00000006	// currently not supported!


// texture extended geometry codes (scale, aspect ratio, flags)

#define TEXGEO_NOTSPECIFIED		0x00000000

#define TEXGEO_GLIDEASPECT		0x00000100	// aspect ratio spec for glide
#define TEXGEO_EXTASPECT		0x00000200	// non-standard aspect ratio spec
#define TEXGEO_INVERSESCALE		0x00000400	// inverse scale factor stored
#define TEXGEO_ASPECTMASK		0x000000ff	// mask to extract aspect ratio
#define TEXGEO_SCALEMASK		0x0fff0000	// mask to extract coordinate scale
#define TEXGEO_SCALESHIFT		16			// shift to get actual scale value
#define TEXGEO_SCALE2MASK		0xf0000000	// mask to extract log2 scale
#define TEXGEO_SCALE2SHIFT		28			// shift to get actual log2 scale
#define TEXGEO_FLAGSMASK		0x0000ff00	// mask to extract additional flags

#define TEXGEO_SCALE_1			0x00010000	// scale factors by which texture
#define TEXGEO_SCALE_2			0x10020000	// coordinates need to be scaled
#define TEXGEO_SCALE_4			0x20040000	// up before they can be used for
#define TEXGEO_SCALE_8			0x30080000	// actual rendering (| their log2).
#define TEXGEO_SCALE_16 		0x40100000	// this facility is used if texture
#define TEXGEO_SCALE_32 		0x50200000	// coordinates have to be normalized
#define TEXGEO_SCALE_64 		0x60400000	// to a reference texture size
#define TEXGEO_SCALE_128		0x70800000	// instead of the actual size of
#define TEXGEO_SCALE_256		0x81000000	// the texture.
#define TEXGEO_SCALE_512		0x92000000
#define TEXGEO_SCALE_1024		0xa4000000

#define TEXGEO_ASPECT_1x1			  0x00	// supported everywhere
#define TEXGEO_ASPECT_2x1			  0x01	// supported everywhere
#define TEXGEO_ASPECT_4x1			( 0x02 | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_8x1			( 0x03 | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_16x1			( 0x04 | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_32x1			( 0x05 | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_64x1			( 0x06 | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_128x1			( 0x07 | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_256x1			( 0x08 | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_512x1			( 0x09 | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_1024x1		( 0x0a | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_1x2			( 0x0b | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_1x4			( 0x0c | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_1x8			( 0x0d | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_1x16			( 0x0e | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_1x32			( 0x0f | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_1x64			( 0x10 | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_1x128			( 0x11 | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_1x256			( 0x12 | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_1x512			( 0x13 | TEXGEO_EXTASPECT )
#define TEXGEO_ASPECT_1x1024		( 0x14 | TEXGEO_EXTASPECT )

#define TEXGEO_ASPECT_GR_8x1		( 0x00 | TEXGEO_GLIDEASPECT )
#define TEXGEO_ASPECT_GR_4x1		( 0x01 | TEXGEO_GLIDEASPECT )
#define TEXGEO_ASPECT_GR_2x1		( 0x02 | TEXGEO_GLIDEASPECT )
#define TEXGEO_ASPECT_GR_1x1		( 0x03 | TEXGEO_GLIDEASPECT )
#define TEXGEO_ASPECT_GR_1x2		( 0x04 | TEXGEO_GLIDEASPECT )
#define TEXGEO_ASPECT_GR_1x4		( 0x05 | TEXGEO_GLIDEASPECT )
#define TEXGEO_ASPECT_GR_1x8		( 0x06 | TEXGEO_GLIDEASPECT )


#endif // _OD_TEXDF_H_


