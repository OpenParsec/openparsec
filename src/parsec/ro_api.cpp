/*
 * PARSEC - Rasterizer API Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:40 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   1999
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

// local module header
#include "ro_api.h"

// opengl header
#include "r_gl.h"

// proprietary module headers
#include "con_aux.h"


// current opengl state as relevant to rasterization --------------------------
//
struct openglstate_s {

	// glEnable state
	bool
	texturing,						// GL_TEXTURE_2D
	alphatest,						// GL_ALPHA_TEST
	blending,						// GL_BLEND
	depthtest,						// GL_DEPTH_TEST
	linesmooth,						// GL_LINE_SMOOTH
	pointsmooth;					// GL_POINT_SMOOTH

	// depth buffer config
	GLenum		depthfunc;			// glDepthFunc()
	GLboolean	depthmask;			// glDepthMask()

	// depth range [glDepthRange()]
	GLclampd	depthrange_zNear;
	GLclampd	depthrange_zFar;

	// blend function [glBlendFunc()]
	GLenum		blendfunc_sfactor;
	GLenum		blendfunc_dfactor;

	// alpha test function [glAlphaFunc()]
	GLenum		alphatest_func;
	GLclampf	alphatest_ref;

	// point size [glPointSize()]
	GLfloat		pointsize;

	// vertex arrays (client states and pointers)
	dword		clientstate_bits;		// glEnableClientState()
	int			clientstate_user;		// user id
	void*		clientstate_base;		// gl{Vertex|Color|TexCoord}Pointer()
};

openglstate_s DefaultOpenGLState = {
	false,
	false,
	false,
	false,
	false,
	false,

	GL_LESS,
	GL_TRUE,

	0.0,
	1.0,

	GL_ONE,
	GL_ZERO,

	GL_ALWAYS,
	0.0f,

	1.0f,

	VTXARRAY_NONE,
	VTXPTRS_NONE,
	0,
};

openglstate_s OpenGLState = DefaultOpenGLState;


// Initialize tracked state values based on current GL state ------------------
//
void RO_InitializeState()
{
	openglstate_s &state = OpenGLState;

	// glEnable state
	state.texturing = glIsEnabled(GL_TEXTURE_2D) == GL_TRUE;
	state.alphatest = glIsEnabled(GL_ALPHA_TEST) == GL_TRUE;
	state.blending = glIsEnabled(GL_BLEND) == GL_TRUE;
	state.depthtest = glIsEnabled(GL_DEPTH_TEST) == GL_TRUE;
	state.linesmooth = glIsEnabled(GL_LINE_SMOOTH) == GL_TRUE;
	state.pointsmooth = glIsEnabled(GL_POINT_SMOOTH) == GL_TRUE;

	// depth buffer
	glGetIntegerv(GL_DEPTH_FUNC, (GLint *) &state.depthfunc);
	glGetBooleanv(GL_DEPTH_WRITEMASK, &state.depthmask);

	// depth range
	GLfloat depthrange[] = {0.0f, 1.0f};
	glGetFloatv(GL_DEPTH_RANGE, depthrange);
	state.depthrange_zNear = depthrange[0];
	state.depthrange_zFar = depthrange[1];

	// blending
	glGetIntegerv(GL_BLEND_SRC_RGB, (GLint *) &state.blendfunc_sfactor);
	glGetIntegerv(GL_BLEND_DST_RGB, (GLint *) &state.blendfunc_dfactor);

	// alpha testing
	glGetIntegerv(GL_ALPHA_TEST_FUNC, (GLint *) &state.alphatest_func);
	glGetFloatv(GL_ALPHA_TEST_REF, &state.alphatest_ref);

	// point size
	glGetFloatv(GL_POINT_SIZE, &state.pointsize);

	// vertex array crap
	state.clientstate_user = VTXPTRS_NONE;
	state.clientstate_base = NULL;

	// disable all vertex array attributes
	state.clientstate_bits = 0;
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}


// set GL server-side capabilities --------------------------------------------
//
void RO_SetCapability(GLenum cap, bool enable)
{
	bool * curstate = NULL;

	switch (cap) {
		case GL_TEXTURE_2D:
			curstate = &OpenGLState.texturing;
			break;
		case GL_ALPHA_TEST:
			curstate = &OpenGLState.alphatest;
			break;
		case GL_BLEND:
			curstate = &OpenGLState.blending;
			break;
		case GL_DEPTH_TEST:
			curstate = &OpenGLState.depthtest;
			break;
		case GL_LINE_SMOOTH:
			curstate = &OpenGLState.linesmooth;
			break;
		case GL_POINT_SMOOTH:
			curstate = &OpenGLState.pointsmooth;
			break;
	}

	if (curstate == NULL || *curstate != enable) {
		if (enable) {
			glEnable(cap);
		} else {
			glDisable(cap);
		}
		if (curstate != NULL) {
			*curstate = enable;
		}
	}
}


// determine if depth compare is enabled --------------------------------------
//
int RO_DepthCmpEnabled()
{
	return OpenGLState.depthtest;
}


// determine if depth write is enabled ----------------------------------------
//
int RO_DepthWriteEnabled()
{
	return OpenGLState.depthmask;
//	return ( OpenGLState.depthtest && OpenGLState.depthmask );
}


// wrapper for GL_DEPTH_TEST state --------------------------------------------
//
void RO_EnableDepthTest( int enable )
{
	RO_SetCapability(GL_DEPTH_TEST, enable);
}


// wrapper for glDepthFunc() --------------------------------------------------
//
void RO_DepthFunc( GLenum func )
{
	if ( func != OpenGLState.depthfunc )
		glDepthFunc( func );
	
	OpenGLState.depthfunc = func;
}


// wrapper for glDepthMask() --------------------------------------------------
//
void RO_DepthMask( GLboolean flag )
{
	if ( flag != OpenGLState.depthmask )
		glDepthMask( flag );
	
	OpenGLState.depthmask = flag;
}


// wrapper for glBlendFunc() --------------------------------------------------
//
void RO_BlendFunc( GLenum sfactor, GLenum dfactor )
{
	if ( sfactor != OpenGLState.blendfunc_sfactor || dfactor != OpenGLState.blendfunc_dfactor )
		glBlendFunc( sfactor, dfactor );
	
	OpenGLState.blendfunc_sfactor = sfactor;
	OpenGLState.blendfunc_dfactor = dfactor;
}


// wrapper for glAlphaFunc() --------------------------------------------------
//
void RO_AlphaFunc( GLenum func, GLclampf ref )
{
	if ( func != OpenGLState.alphatest_func || ref != OpenGLState.alphatest_ref )
		glAlphaFunc( func, ref );
	
	OpenGLState.alphatest_func = func;
	OpenGLState.alphatest_ref  = ref;
}


// wrapper for glEnable/glDisable( GL_ALPHA_TEST ) ----------------------------
//
void RO_AlphaTest( int enable )
{
	RO_SetCapability(GL_ALPHA_TEST, enable);
}


// wrapper for glEnable/glDisable( GL_POINT_SMOOTH ) --------------------------
//
void RO_PointSmooth( int enable )
{
	RO_SetCapability(GL_POINT_SMOOTH, enable);
}


// wrapper for glEnable/glDisable( GL_LINE_SMOOTH ) ---------------------------
//
void RO_LineSmooth( int enable )
{
	RO_SetCapability(GL_LINE_SMOOTH, enable);
}


// wrapper for glPointSize() --------------------------------------------------
//
void RO_PointSize( GLfloat size )
{
	if ( OpenGLState.pointsize != size ) {
		glPointSize( size );
	}
	
	OpenGLState.pointsize = size;
}


// enable/disable opengl client state -----------------------------------------
//
PRIVATE
inline void RO_CheckClientState(dword statebits, int state, GLenum glstate)
{
	if (statebits & state) {
		if ((OpenGLState.clientstate_bits & state) == 0) {
			OpenGLState.clientstate_bits |= state;
			glEnableClientState(glstate);
		}
	} else {
		if (OpenGLState.clientstate_bits & state) {
			OpenGLState.clientstate_bits &= ~state;
			glDisableClientState(glstate);
		}
	}
}


// enable/disable client state vector -----------------------------------------
//
void RO_ClientState( dword statebits )
{
	RO_CheckClientState( statebits, VTXARRAY_VERTICES, GL_VERTEX_ARRAY );
	RO_CheckClientState( statebits, VTXARRAY_COLORS, GL_COLOR_ARRAY );
	RO_CheckClientState( statebits, VTXARRAY_TEXCOORDS, GL_TEXTURE_COORD_ARRAY );
//	RO_CheckClientState( statebits, VTXARRAY_EDGEFLAGS, GL_EDGE_FLAG_ARRAY );
	RO_CheckClientState( statebits, VTXARRAY_NORMALS, GL_NORMAL_ARRAY );
}


// check whether vertex pointers must be made current, remember as current ----
//
int RO_ArrayMakeCurrent( int userid, void *base )
{
	// user id and base pointer must match
	if ( ( OpenGLState.clientstate_user == userid ) &&
		 ( OpenGLState.clientstate_base == base ) )
		return FALSE;

	OpenGLState.clientstate_user = userid;
	OpenGLState.clientstate_base = base;

	return TRUE;
}


// possible states of texture environment -------------------------------------
//
struct configs_textureenvironment_s {

	int		texturingon;
	GLint	texenvmode;
	GLenum	shademodel;

} Configs_TextureEnvironment[] = {

	// [ITER_CONSTRGB] = [FLAT SHADING / DONT_CARE]
	// color_out = color_constant;
	// alpha_out = undefined;
	{
		FALSE,
		0,
		GL_FLAT
	},

	// [ITER_CONSTRGBA] = [FLAT SHADING / CONST_ALPHA]
	// color_out = color_constant;
	// alpha_out = alpha_constant;
	{
		FALSE,
		0,
		GL_FLAT
	},

	// [ITER_RGB] = [GOURAUD SHADING / DONT_CARE]
	// color_out = color_iterated;
	// alpha_out = undefined;
	{
		FALSE,
		0,
		GL_SMOOTH
	},

	// [ITER_RGBA] = [GOURAUD SHADING / ITERATED_ALPHA]
	// color_out = color_iterated;
	// alpha_out = alpha_iterated;
	{
		FALSE,
		0,
		GL_SMOOTH
	},

	// [ITER_TEXONLY] = [UNSHADED TEXTURE / TEXTURE_ALPHA]
	// color_out = color_texture;
	// alpha_out = alpha_texture;
	{
		TRUE,
		GL_REPLACE,
		GL_FLAT,
	},

	// [ITER_TEXCONSTA] = [UNSHADED TEXTURE / SCALED_TEXTURE_ALPHA]
	// color_out = color_texture;
	// alpha_out = alpha_texture * alpha_constant;
	{
		TRUE,
		GL_REPLACE,
		GL_FLAT,
	},

	// [ITER_TEXCONSTRGB] = [FLAT-SHADED TEXTURE / TEXTURE_ALPHA]
	// color_out = color_texture * color_constant;
	// alpha_out = alpha_texture;
	{
		TRUE,
		GL_MODULATE,
		GL_FLAT
	},

	// [ITER_TEXCONSTRGBA] = [FLAT-SHADED TEXTURE / SCALED_TEXTURE_ALPHA]
	// color_out = color_texture * color_constant;
	// alpha_out = alpha_texture * alpha_constant;
	{
		TRUE,
		GL_MODULATE,
		GL_FLAT
	},

	// [ITER_TEXA] = [UNSHADED TEXTURE / MODULATED_TEXTURE_ALPHA]
	// color_out = color_texture;
	// alpha_out = alpha_texture * alpha_iterated;
	{
		TRUE,
		GL_MODULATE,
		GL_SMOOTH,
	},

	// [ITER_TEXRGB] = [GOURAUD-SHADED (DIFFUSELY LIT) TEXTURE / TEXTURE_ALPHA]
	// color_out = color_texture * color_iterated;
	// alpha_out = alpha_texture;
	{
		TRUE,			// must be GL_RGBA
		GL_MODULATE,
		GL_SMOOTH		// alpha of glColor() must be 1.0
	},

	// [ITER_TEXRGBA] = [GOURAUD-SHADED (DIFFUSELY LIT) TEXTURE / MODULATED_TEXTURE_ALPHA]
	// color_out = color_texture * color_iterated;
	// alpha_out = alpha_texture * alpha_iterated;
	{
		TRUE,			// must be GL_RGBA
		GL_MODULATE,
		GL_SMOOTH
	},

	// [CONSTRGBTEXA] = [FLAT SHADING / TEXTURE_ALPHA]
	// color_out = color_constant;
	// alpha_out = alpha_texture;
	{
		TRUE,			// must be GL_ALPHA
		GL_REPLACE,
		GL_FLAT
	},

	// [CONSTRGBATEXA] = [FLAT SHADING / SCALED_TEXTURE_ALPHA]
	// color_out = color_constant;
	// alpha_out = alpha_texture * alpha_constant;
	{
		TRUE,			// must be GL_ALPHA
		GL_MODULATE,
		GL_FLAT
	},

	// [ITER_RGBTEXA] = [GOURAUD SHADING / TEXTURE_ALPHA]
	// color_out = color_iterated;
	// alpha_out = alpha_texture;
	{
		TRUE,			// must be GL_ALPHA
		GL_REPLACE,
		GL_SMOOTH
	},

	// [ITER_RGBATEXA] = [GOURAUD SHADING / MODULATED_TEXTURE_ALPHA]
	// color_out = color_iterated;
	// alpha_out = alpha_texture * alpha_iterated;
	{
		TRUE,			// must be GL_ALPHA
		GL_MODULATE,
		GL_SMOOTH
	},

};

#define CONFIG_TEXTUREENVIRONMENT_CONSTRGB					0
#define CONFIG_TEXTUREENVIRONMENT_CONSTRGBA					1
#define CONFIG_TEXTUREENVIRONMENT_RGB						2
#define CONFIG_TEXTUREENVIRONMENT_RGBA						3
#define CONFIG_TEXTUREENVIRONMENT_TEXONLY					4
#define CONFIG_TEXTUREENVIRONMENT_TEXCONSTA					5
#define CONFIG_TEXTUREENVIRONMENT_TEXCONSTRGB				6
#define CONFIG_TEXTUREENVIRONMENT_TEXCONSTRGBA				7
#define CONFIG_TEXTUREENVIRONMENT_TEXA						8
#define CONFIG_TEXTUREENVIRONMENT_TEXRGB					9
#define CONFIG_TEXTUREENVIRONMENT_TEXRGBA					10
#define CONFIG_TEXTUREENVIRONMENT_CONSTRGBTEXA				11
#define CONFIG_TEXTUREENVIRONMENT_CONSTRGBATEXA				12
#define CONFIG_TEXTUREENVIRONMENT_RGBTEXA					13
#define CONFIG_TEXTUREENVIRONMENT_RGBATEXA					14

#define CONFIG_TEXTUREENVIRONMENT_NUMCONFIGS				15
#define CONFIG_TEXTUREENVIRONMENT_NUMCONFIGS_BASE			15

#define CONFIG_TEXTUREENVIRONMENT_UNDEFINED					-1

int Iter_TextureEnvironment[] = {

	CONFIG_TEXTUREENVIRONMENT_CONSTRGB,     			// iter_constrgb
	CONFIG_TEXTUREENVIRONMENT_CONSTRGBA,                // iter_constrgba
	CONFIG_TEXTUREENVIRONMENT_RGB,                      // iter_rgb
	CONFIG_TEXTUREENVIRONMENT_RGBA,                     // iter_rgba
	CONFIG_TEXTUREENVIRONMENT_TEXONLY,                  // iter_texonly
	CONFIG_TEXTUREENVIRONMENT_TEXCONSTA,                // iter_texconsta
	CONFIG_TEXTUREENVIRONMENT_TEXCONSTRGB,              // iter_texconstrgb
	CONFIG_TEXTUREENVIRONMENT_TEXCONSTRGBA,             // iter_texconstrgba
	CONFIG_TEXTUREENVIRONMENT_TEXA,                     // iter_texa
	CONFIG_TEXTUREENVIRONMENT_TEXRGB,                   // iter_texrgb
	CONFIG_TEXTUREENVIRONMENT_TEXRGBA,                  // iter_texrgba
	CONFIG_TEXTUREENVIRONMENT_CONSTRGBTEXA,             // iter_constrgbtexa
	CONFIG_TEXTUREENVIRONMENT_CONSTRGBATEXA,            // iter_constrgbatexa
	CONFIG_TEXTUREENVIRONMENT_RGBTEXA,                  // iter_rgbtexa
	CONFIG_TEXTUREENVIRONMENT_RGBATEXA,                 // iter_rgbatexa
};

#define CONFIG_TEXTUREENVIRONMENT_NUMTYPES	CALC_NUM_ARRAY_ENTRIES( Iter_TextureEnvironment )


// possible states of blend function ------------------------------------------
//
struct configs_blendfunction_s {

	int			active;
	GLenum		blendfunc_sfactor;
	GLenum		blendfunc_dfactor;

} Configs_BlendFunction[] = {

	// [OVERWRITE]
	// color_out = color_src;
	// alpha_out = alpha_src;
	{
		FALSE,
		GL_ONE,
		GL_ZERO
	},

	// [WEIGHTED_BLEND]
	// color_out = color_src * alpha + color_dst * ( 1 - alpha );
	// alpha_out = alpha_src;
	{
		TRUE,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA
	},

	// [MODULATE]
	// color_out = color_src * color_dst;
	// alpha_out = alpha_src;
	{
		TRUE,
		GL_DST_COLOR,
		GL_ZERO
	},

	// [SPECULAR_ADD]
	// color_out = color_src + color_dst;
	// alpha_out = alpha_src;
	{
		TRUE,
		GL_ONE,
		GL_ONE
	},

	// [PREMULTIPLIED_BLEND]
	// color_out = color_src + color_dst * ( 1 - alpha );
	// alpha_out = alpha_src;
	{
		TRUE,
		GL_ONE,
		GL_ONE_MINUS_SRC_ALPHA
	},
	
	// [ADDITIVE_BLEND]
	// color_out = color_src * color_alpha + color_dst;
	// alpha_out = alpha_src;
	{
		TRUE,
		GL_SRC_ALPHA,
		GL_ONE
	},
};

#define CONFIG_BLENDFUNCTION_OVERWRITE					0
#define CONFIG_BLENDFUNCTION_WEIGHTED_BLEND				1
#define CONFIG_BLENDFUNCTION_MODULATE					2
#define CONFIG_BLENDFUNCTION_SPECULAR_ADD				3
#define CONFIG_BLENDFUNCTION_PREMULTIPLIED_BLEND		4
#define CONFIG_BLENDFUNCTION_ADDITIVE_BLEND				5

#define CONFIG_BLENDFUNCTION_NUMCONFIGS					6
#define CONFIG_BLENDFUNCTION_UNDEFINED					-1

int Iter_BlendFunction[] = {

	CONFIG_BLENDFUNCTION_OVERWRITE,						// iter_overwrite
	CONFIG_BLENDFUNCTION_WEIGHTED_BLEND,				// iter_alphablend
	CONFIG_BLENDFUNCTION_MODULATE,						// iter_modulate
	CONFIG_BLENDFUNCTION_SPECULAR_ADD,					// iter_specularadd
	CONFIG_BLENDFUNCTION_PREMULTIPLIED_BLEND,			// iter_premulblend
	CONFIG_BLENDFUNCTION_ADDITIVE_BLEND,				// iter_additiveblend
};

#define CONFIG_BLENDFUNCTION_NUMTYPES	CALC_NUM_ARRAY_ENTRIES( Iter_BlendFunction )


// currently active configurations --------------------------------------------
//
int last_texenv_config		= CONFIG_TEXTUREENVIRONMENT_UNDEFINED;
int last_blendfunc_config	= CONFIG_BLENDFUNCTION_UNDEFINED;


// invalidate rasterizer state ------------------------------------------------
//
void RO_InvalidateRasterizerState()
{
	last_texenv_config		= CONFIG_TEXTUREENVIRONMENT_UNDEFINED;
	last_blendfunc_config	= CONFIG_BLENDFUNCTION_UNDEFINED;

	OpenGLState = DefaultOpenGLState;
}


// init rasterizer to specified state -----------------------------------------
//
void RO_InitRasterizerState( dword itertype, dword raststate, dword rastmask )
{
	//NOTE:
	// the following things cannot be set here due
	// to the inner workings of OpenGL texture mapping:
	// - texture clamp mode
	// - mip-map mode
	// - filtering mode
	// (the corresponding rast_xx flags will be ignored.)

	// isolate basic iteration type
	dword iterbase = itertype & iter_base_mask;

	// fetch desired config
	ASSERT( iterbase < CONFIG_TEXTUREENVIRONMENT_NUMTYPES );
	int texenv_config = Iter_TextureEnvironment[ iterbase ];
	ASSERT( texenv_config >= 0 );
	ASSERT( texenv_config < CONFIG_TEXTUREENVIRONMENT_NUMCONFIGS );

	// check if texture environment configuration needs to be changed
	if ( texenv_config != last_texenv_config ) {

		// remember config
		last_texenv_config = texenv_config;

		// configure shading model
		glShadeModel( Configs_TextureEnvironment[ texenv_config ].shademodel );

		bool texturingon = Configs_TextureEnvironment[ texenv_config ].texturingon;

		RO_SetCapability(GL_TEXTURE_2D, texturingon);

		if ( texturingon ) {
			// configure texture environment
			glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
				Configs_TextureEnvironment[ texenv_config ].texenvmode );
		}
	}

	// isolate composition type
	dword comptype = ( itertype & iter_compose_mask ) >> iter_compose_shift;

	// fetch desired config
	ASSERT( comptype < CONFIG_BLENDFUNCTION_NUMTYPES );
	int blendfunc_config = Iter_BlendFunction[ comptype ];
	ASSERT( blendfunc_config >= 0 );
	ASSERT( blendfunc_config < CONFIG_BLENDFUNCTION_NUMCONFIGS );

	// check if blend function configuration needs to be changed
	if ( blendfunc_config != last_blendfunc_config ) {

		// remember config
		last_blendfunc_config = blendfunc_config;

		bool blendactive = Configs_BlendFunction[ blendfunc_config ].active;

		RO_SetCapability(GL_BLEND, blendactive);

		// configure blend function
		if ( blendactive ) {
			glBlendFunc(
				Configs_BlendFunction[ blendfunc_config ].blendfunc_sfactor,
				Configs_BlendFunction[ blendfunc_config ].blendfunc_dfactor );
		}
	}

	// depth buffer write
	if ( ( rastmask & rast_mask_zwrite ) == 0 ) {

		int depthmask = ( raststate & rast_zwrite ) ? GL_TRUE : GL_FALSE;
		RO_DepthMask(depthmask);
	}

	// depth buffer compare
	if ( ( rastmask & rast_mask_zcompare ) == 0 ) {

		bool depthtest = ( raststate & rast_zcompare ) != 0;

		RO_SetCapability(GL_DEPTH_TEST, depthtest);

		if ( depthtest ) {
			RO_DepthFunc(GL_GREATER);
		}
	}
}


// set combine state of texture units (mostly for multipass mapping) ----------
//
void RO_TextureCombineState( dword texcomb )
{
	//TODO:
}


// set rasterizer state to default --------------------------------------------
//
#ifndef NO_DEFAULT_RASTERIZER_STATE
void RO_DefaultRasterizerState()
{
	if ( AUX_NO_DEFAULT_RASTERIZER_STATE )
		return;

	dword itertype	= iter_rgb;
	dword raststate	= rast_chromakeyoff;
	dword rastmask	= rast_mask_zbuffer | rast_mask_texclamp |
					  rast_mask_mipmap  | rast_mask_texfilter;

	RO_InitRasterizerState( itertype, raststate, rastmask );
	RO_TextureCombineState( texcomb_decal );
}
#endif



