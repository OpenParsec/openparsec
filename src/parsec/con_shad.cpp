/*
 * PARSEC - Shader Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:35 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999-2000
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

// local module header
#include "con_shad.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux.h"
#include "con_main.h"
#include "e_colani.h"
#include "e_shader.h"
#include "obj_clas.h"
#include "part_api.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// string constants -----------------------------------------------------------
//
static char invalid_lod[]			= "invalid lod specification.";
static char face_id_invalid[]		= "face id invalid.";
static char shader_name_missing[]	= "name of shader must be specified.";
static char shader_name_invalid[]	= "shader name invalid.";
static char shader_spec_invalid[]	= "shader spec missing or invalid.";
static char iter_base_invalid[]		= "iter_base spec invalid.";
static char iter_compose_invalid[]	= "iter_compose spec invalid.";
static char texanim_invalid[]		= "texanim name invalid.";
static char colanim_invalid[]		= "colanim name invalid.";
static char colanim_flags_invalid[]	= "colanim mode spec invalid.";
static char colanim_flags_ignored[]	= "ignoring colanim mode spec.";
static char facecolor_invalid[]		= "face color invalid.";
static char shader_reg_failed[]		= "shader registration failed.";
static char base_object_empty[]		= "base object is empty.";


// flag map for shader spec ---------------------------------------------------
//
static flag_map_s shader_flag_map[] = {

	{ "iter_constrgb",			iter_constrgb,					0	},
	{ "iter_constrgba",			iter_constrgba,					0	},
	{ "iter_rgb",				iter_rgb,						0	},
	{ "iter_rgba",				iter_rgba,						0	},
	{ "iter_texonly",			iter_texonly,					0	},
	{ "iter_texconsta",			iter_texconsta,					0	},
	{ "iter_texconstrgb",		iter_texconstrgb,				0	},
	{ "iter_texconstrgba",		iter_texconstrgba,				0	},
	{ "iter_texa",				iter_texa,						0	},
	{ "iter_texrgb",			iter_texrgb,					0	},
	{ "iter_texrgba",			iter_texrgba,					0	},
	{ "iter_constrgbtexa",		iter_constrgbtexa,				0	},
	{ "iter_constrgbatexa",		iter_constrgbatexa,				0	},
	{ "iter_rgbtexa",			iter_rgbtexa,					0	},
	{ "iter_rgbatexa",			iter_rgbatexa,					0	},
	{ "iter_texblendrgba",		iter_texblendrgba,				0	},
	{ "iter_texspeca",			iter_texspeca,					0	},
	{ "iter_texspecrgb",		iter_texspecrgb,				0	},
	{ "iter_texconstaspecrgb",	iter_texconstaspecrgb,			0	},
	{ "iter_texconstrgbspeca",	iter_texconstrgbspeca,			0	},
	{ "iter_texaspecrgb",		iter_texaspecrgb,				0	},
	{ "iter_texrgbspeca",		iter_texrgbspeca,				0	},

	{ "iter_overwrite",			iter_overwrite,					1	},
	{ "iter_alphablend",		iter_alphablend,				1	},
	{ "iter_modulate",			iter_modulate,					1	},
	{ "iter_specularadd",		iter_specularadd,				1	},
	{ "iter_premulblend",		iter_premulblend,				1	},
	{ "iter_additiveblend",		iter_additiveblend,				1	},

	{ "flag_texipolate",		FACE_SHADING_TEXIPOLATE<<16,  	2	},
	{ "flag_nodepthcmp",		FACE_SHADING_NODEPTHCMP<<16,  	2	},
	{ "flag_nodepthwrite",		FACE_SHADING_NODEPTHWRITE<<16,	2	},
	{ "flag_draw_sorted",		FACE_SHADING_DRAW_SORTED<<16,	2	},
	{ "flag_draw_last",			FACE_SHADING_DRAW_LAST<<16,   	2	},
	{ "flag_nobackcull",		FACE_SHADING_NOBACKCULLING<<16,	2	},
	{ "flag_back_first",		FACE_SHADING_BACK_FIRST<<16,	2	},

	{ NULL,						0,								0	},
};


// temporary shader for returning a shader specified in immediate mode --------
//
static shader_s result_shader;


// parse global object shading spec -------------------------------------------
//
shader_s *DetermineODTShader( key_value_s *shaderspec )
{
	ASSERT( shaderspec != NULL );

	shader_s *shader = NULL;

	if ( shaderspec->value != NULL ) {

		// try to interpret argument as shader name
		shader = FetchShaderByName( shaderspec->value, NULL );
		if ( shader != NULL )
			return shader;

		// use anonymous (temp) shader
		shader = &result_shader;
		memset( shader, 0, sizeof( shader_s ) );

		// parse list, set corresponding bits
		dword dwshader = 0x00000000;
		if ( ScanKeyValueFlagList( shaderspec, &dwshader, shader_flag_map ) == 0 ) {
			return NULL;
		}

		// check shader for consistency
		dword base = dwshader & iter_base_mask;
		dword comp = dwshader & iter_compose_mask;

		// iter_base may be invalid if multiple specs or'ed together
		if ( base > iter_texrgbspeca ) {
			CON_AddLine( iter_base_invalid );
			shader = NULL;
		}
		// iter_compose may be invalid if multiple specs or'ed together
		if ( comp > iter_premulblend ) {
			CON_AddLine( iter_compose_invalid );
			shader = NULL;
		}

		// set immediate mode fields
		if ( shader != NULL ) {
			shader->iter  = dwshader & 0xffff;
			shader->flags = dwshader >> 16;
		}
	}

	return shader;
}


// get description for shader (iter/flags part only) --------------------------
//
char *GetShaderDescription( dword shaderid )
{
	ASSERT( shaderid < (dword)num_registered_shaders );

	shader_s *shader = &registered_shaders[ shaderid ];

	dword base  = (dword)shader->iter & iter_base_mask;
	dword comp  = (dword)shader->iter & iter_compose_mask;
	dword flags = (dword)shader->flags << 16;

	flag_map_s *basemap = NULL;
	flag_map_s *compmap = NULL;

	// get iter and compose description
	flag_map_s *map = NULL;
	for ( map = shader_flag_map; map->name != NULL; map++ ) {

		switch ( map->group ) {

			case 0:
				if ( base == map->value )
					basemap = map;
				break;

			case 1:
				if ( comp == map->value )
					compmap = map;
				break;
		}
	}

	strcpy( paste_str, ( basemap != NULL ) ? basemap->name : "base_invalid" );
	strcat( paste_str, " " );
	strcat( paste_str, ( compmap != NULL ) ? compmap->name : "comp_invalid" );

	// append flags
	for ( map = shader_flag_map; map->name != NULL; map++ ) {
		if ( ( map->group == 2 ) && ( flags & map->value ) ) {
			strcat( paste_str, " " );
			strcat( paste_str, map->name );
		}
	}

	return paste_str;
}


// key table for defshader specification --------------------------------------
//
key_value_s defshader_key_value[] = {

	{ "shader",		NULL,	KEYVALFLAG_PARENTHESIZE	},
	{ "color",		NULL,	KEYVALFLAG_PARENTHESIZE	},
	{ "texanim",	NULL,	KEYVALFLAG_PARENTHESIZE	},
	{ "colanim",	NULL,	KEYVALFLAG_PARENTHESIZE	},
	{ "colmode",	NULL,	KEYVALFLAG_NONE			},

	{ NULL,			NULL,	KEYVALFLAG_NONE			},
};

enum {

	KEY_DEFSHAD_SHADER,
	KEY_DEFSHAD_COLOR,
	KEY_DEFSHAD_TEXANIM,
	KEY_DEFSHAD_COLANIM,
	KEY_DEFSHAD_COLMODE
};


// possible FaceAnimState::ColFlags modes (FACE_ANIM_BASExx) ------------------
//
static char colanim_mode_baseignore[]	= "nobase";
static char colanim_mode_baseadd[]		= "add";
static char colanim_mode_basemul[]		= "mul";


// command "shader.def" -------------------------------------------------------
//
int Cmd_DefShader( char *command )
{
	//NOTE:
	//CONCOM:
	// defshader_command ::= 'shader.def' <shader-name> [<shader-spec>] [<color>] [<anims>]
	// shader_name       ::= "may be parenthesized to include whitespace"
	// shader_spec       ::= 'shader' <shader-iter> | <shader-name>
	// shader_iter       ::= '(' 'iter_xx'* 'flag_xx'* ')'
	// shader_name       ::= "valid name of shader"
	// color             ::= 'color' '(' <int> <int> <int> [<int>] ')'
	// anims             ::= [<texanim>] [<colanim> [<colanim-mode>]]
	// texanim           ::= 'texanim' "valid name of texanim"
	// colanim           ::= 'colanim' "valid name of colanim"
	// colanim_mode      ::= 'colmode' 'add' | 'mul' | 'nobase'

	ASSERT( command != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( command );

	// this flag is used during internal data reloads
	if ( AUX_ALLOW_ONLY_TEXTURE_LOADING ) {
		return TRUE;
	}

	// create pointer to list of parameters (first is always the name)
	char *shadername = strtok( command, " " );
	if ( shadername == NULL ) {
		CON_AddLine( shader_name_missing );
		return TRUE;
	}

	// allow name to be parenthesized to include whitespace
	shadername = GetParenthesizedName( shadername );
	if ( shadername == NULL ) {
		CON_AddLine( shader_name_invalid );
		return TRUE;
	}

	// scan out all values to keys
	if ( !ScanKeyValuePairs( defshader_key_value, NULL ) )
		return TRUE;

	// parse shader spec
	shader_s *shader = DetermineODTShader( &defshader_key_value[ KEY_DEFSHAD_SHADER ] );
	if ( shader == NULL ) {
		CON_AddLine( shader_spec_invalid );
		return TRUE;
	}

	//NOTE:
	// if ( shader->name != NULL ) here, the shader has not been
	// specified in immediate mode, but with an already existing
	// shader name. this can be used for duplicating shaders.

	word iter  = shader->iter;
	word flags = shader->flags;

	colrgba_s facecolor;
	memset( &facecolor, 0, sizeof( colrgba_s ) );

	// check for constant face color specification
	if ( defshader_key_value[ KEY_DEFSHAD_COLOR ].value != NULL ) {

		int colspec[ 4 ];
		colspec[ 3 ] = 255;

		int numcomps =
			ScanKeyValueIntList( &defshader_key_value[ KEY_DEFSHAD_COLOR ], colspec, 3, 4 );
		if ( numcomps < 3 ) {
			CON_AddLine( facecolor_invalid );
			return TRUE;
		}

		if ( ( (dword)colspec[ 0 ] > 255 ) || ( (dword)colspec[ 1 ] > 255 ) ||
			 ( (dword)colspec[ 2 ] > 255 ) || ( (dword)colspec[ 3 ] > 255 ) ) {
			CON_AddLine( facecolor_invalid );
			return TRUE;
		}

		facecolor.R = colspec[ 0 ];
		facecolor.G = colspec[ 1 ];
		facecolor.B = colspec[ 2 ];
		facecolor.A = colspec[ 3 ];

		// set flag to use face color
		flags |= FACE_SHADING_FACECOLOR;
	}

	// acquire texanim if name specified
	texanim_s *texanim = NULL;
	if ( defshader_key_value[ KEY_DEFSHAD_TEXANIM ].value != NULL ) {

		texanim = PRT_AcquireParticleDefinition(
			defshader_key_value[ KEY_DEFSHAD_TEXANIM ].value, NULL );
		if ( texanim == NULL ) {
			CON_AddLine( texanim_invalid );
			return TRUE;
		}
	}

	// acquire colanim if name specified
	colanim_s *colanim = NULL;
	if ( defshader_key_value[ KEY_DEFSHAD_COLANIM ].value != NULL ) {

		colanimreg_s *colanimreg = FetchColAnim(
			defshader_key_value[ KEY_DEFSHAD_COLANIM ].value, NULL );
		if ( colanimreg == NULL ) {
			CON_AddLine( colanim_invalid );
			return TRUE;
		}
		colanim = &colanimreg->colanim;

		// set flag to use face color
		flags |= FACE_SHADING_FACECOLOR;
	}

	// fetch colanim mode
	dword colmode = FACE_ANIM_BASEIGNORE;
	if ( defshader_key_value[ KEY_DEFSHAD_COLMODE ].value != NULL ) {

		if ( colanim != NULL ) {
			char *modespec = defshader_key_value[ KEY_DEFSHAD_COLMODE ].value;
			if ( strcmp( modespec, colanim_mode_baseadd ) == 0 ) {
				colmode = FACE_ANIM_BASEADD;
			} else if ( strcmp( modespec, colanim_mode_basemul ) == 0 ) {
				colmode = FACE_ANIM_BASEMUL;
			} else if ( strcmp( modespec, colanim_mode_baseignore ) == 0 ) {
				colmode = FACE_ANIM_BASEIGNORE;
			} else {
				CON_AddLine( colanim_flags_invalid );
				return TRUE;
			}
		} else {
			CON_AddLine( colanim_flags_ignored );
		}
	}

	shader_s newshader;
	newshader.iter		= iter;
	newshader.flags		= flags;
	newshader.name		= shadername;
	newshader.facecolor	= facecolor;
	newshader.texanim	= texanim;
	newshader.colanim	= colanim;
	newshader.colflags	= colmode;

	// register shader
	if ( !RegisterShader( &newshader ) ) {
		CON_AddLine( shader_reg_failed );
	}

	return TRUE;
}


// key table for setshader specification --------------------------------------
//
key_value_s setshader_key_value[] = {

	{ "class",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "id",			NULL,	KEYVALFLAG_NONE				},
	{ "shader",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "texture",	NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "faceid",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "lod",		NULL,	KEYVALFLAG_NONE				},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_SETSHAD_CLASS,
	KEY_SETSHAD_ID,
	KEY_SETSHAD_SHADER,
	KEY_SETSHAD_TEXTURE,
	KEY_SETSHAD_FACEID,
	KEY_SETSHAD_LOD
};


// command "shader.set" to overwrite specified (object,face)'s shader spec ----
//
int Cmd_SetShader( char *command )
{
	//NOTE:
	//CONCOM:
	// setshader_command ::= 'shader.set' <class-spec> <shader-spec> [<face-spec>]
	// class_spec        ::= <class-name> | <class-id> [<lod-spec>]
	// class_name        ::= 'class' <name>
	// class_id          ::= 'id' <int>
	// lod_spec          ::= 'lod' <int>
	// shader_spec       ::= 'shader' <shader-iter> | <shader-name>
	// shader_iter       ::= '(' 'iter_xx'* 'flag_xx'* ')'
	// shader_name       ::= "valid name of shader"
	// face_spec		 ::= <tex-spec> | <face-id-spec>
	// tex_spec          ::= 'texture' <texture-name>
	// face_id_spec      ::= 'faceid' <face-id> | <face-id-list>
	// face_id_list      ::= '(' <face-id>+ ')'

	ASSERT( command != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( command );

	// this flag is used during internal data reloads
	if ( AUX_ALLOW_ONLY_TEXTURE_LOADING ) {
		return TRUE;
	}

	// scan out all values to keys
	if ( !ScanKeyValuePairs( setshader_key_value, command ) )
		return TRUE;

	// get object class (either name or id)
	dword objclass = ScanKeyValueObjClass( setshader_key_value, KEY_SETSHAD_CLASS, KEY_SETSHAD_ID );
	if ( objclass == CLASS_ID_INVALID ) {
		return TRUE;
	}

	GenObject *gobj = ObjClasses[ objclass ];

	// guard against empty base objects
	if ( gobj->NumFaces == 0 ) {
		CON_AddLine( base_object_empty );
		return TRUE;
	}

	// highest allowed face id
	dword maxfaceid = gobj->NumFaces - 1;

	// parse (optional) lod spec
	int lod = 0;
	if ( setshader_key_value[ KEY_SETSHAD_LOD ].value != NULL ) {
		if ( ScanKeyValueInt( &setshader_key_value[ KEY_SETSHAD_LOD ], &lod ) < 0 ) {
			CON_AddLine( invalid_lod );
			return TRUE;
		}
		if ( lod != 0 ) {
			if ( ( lod >= gobj->NumLodObjects ) ||
				( -lod >= gobj->NumLodObjects ) ) {
				CON_AddLine( invalid_lod );
				return TRUE;
			}
			// check will be made for
			// each lod individually
			maxfaceid = 100000;
		}
	}

	// parse shader spec
	shader_s *shader = DetermineODTShader( &setshader_key_value[ KEY_SETSHAD_SHADER ] );
	if ( shader == NULL ) {
		CON_AddLine( shader_spec_invalid );
		return TRUE;
	}

	// set shader for single face or list of faces specified via id
	char *faceidstr = setshader_key_value[ KEY_SETSHAD_FACEID ].value;
	if ( faceidstr != NULL ) {

		// try list first
		while ( *faceidstr == ' ' )
			faceidstr++;
		while ( ( *faceidstr != 0 ) && ( *faceidstr != ' ' ) && ( *faceidstr != '-' ) )
			faceidstr++;
		while ( *faceidstr == ' ' )
			faceidstr++;
		if ( *faceidstr != 0 ) {

			// read list of ids
			int *faceids = (int *) ALLOCMEM( 512 * sizeof( int ) );
			if ( faceids == NULL )
				OUTOFMEM( 0 );
			int cnt = ScanKeyValueIntListBounded(&setshader_key_value[ KEY_SETSHAD_FACEID ], faceids, 2, 512, 0, maxfaceid );
			if ( cnt > 0 ) {
				if (override_arg_faceids != NULL)
					FREEMEM(override_arg_faceids);
				
				override_arg_faceids = (dword *) faceids;
				override_arg_count   = cnt;
				OverrideShaderLodFaces( objclass, shader, lod, OVERRIDE_ARG_IDLIST );
			} else {
				CON_AddLine( face_id_invalid );
				FREEMEM(faceids);
			}
			return TRUE;
		}

		// read single id
		int * faceid = (int *) ALLOCMEM(sizeof(int));
		if ( ScanKeyValueInt( &setshader_key_value[ KEY_SETSHAD_FACEID ], faceid ) < 0 ) {
			CON_AddLine( face_id_invalid );
			FREEMEM(faceid);
			return TRUE;
		}
		if ( ( *faceid < 0 ) || ( *faceid > (int)maxfaceid ) ) {
			CON_AddLine( face_id_invalid );
			FREEMEM(faceid);
			return TRUE;
		}
		
		if (override_arg_faceids != NULL)
			FREEMEM(override_arg_faceids);
		
		override_arg_faceids = (dword *) faceid;
		override_arg_count   = 1;
		OverrideShaderLodFaces( objclass, shader, lod, OVERRIDE_ARG_IDLIST );
		return TRUE;
	}

	// set shader for set of faces specified via texture name
	if ( setshader_key_value[ KEY_SETSHAD_TEXTURE ].value != NULL ) {
		override_arg_texname = setshader_key_value[ KEY_SETSHAD_TEXTURE ].value;
		OverrideShaderLodFaces( objclass, shader, lod, OVERRIDE_ARG_TEXTURE );
		return TRUE;
	}

	// set shader for all faces of object
	OverrideShaderLodFaces( objclass, shader, lod, OVERRIDE_ARG_GLOBAL );

	return TRUE;
}



