/*
 * PARSEC - Data Loading Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:34 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-2001
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
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "gd_heads.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"

// rendering subsystem
#include "r_patch.h"

// local module header
#include "con_load.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux.h"
#include "con_ext.h"
#include "con_main.h"
#include "con_shad.h"
#include "con_std.h"
#include "e_color.h"
#include "img_api.h"
#include "obj_clas.h"
#include "obj_ctrl.h"
#include "obj_cust.h"
#include "obj_odt.h"
#include "obj_type.h"
#include "sys_file.h"
#include "sys_swap.h"



// generic string paste area
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];

// string constants
static char datatype_object[]		= "object";
static char datatype_texture[]		= "texture";
static char datatype_texfont[]		= "texfont";
static char datatype_bitmap[]		= "bitmap";
static char datatype_sample[]		= "sample";
static char datatype_invalid[]		= "unknown data type.";
static char datatype_missing[]		= "data type must be specified.";
static char name_missing[]			= "name must be specified.";
static char name_invalid[]			= "name invalid.";
static char name_already_used[]		= "name must be unique.";
static char filename_missing[]		= "filename missing.";
static char filename_invalid[]		= "filename invalid.";
static char too_many_textures[]		= "too many textures.";
static char too_many_texfonts[]		= "too many texfonts.";
static char too_many_objects[]		= "too many objects.";
static char type_missing[]			= "type missing.";
static char type_invalid[]			= "type invalid.";
static char type_reused[]			= "type invalid. using old type.";
static char no_myship_subst[]		= "cannot substitute player ship object.";
static char too_many_bitmaps[]		= "too many bitmaps.";
static char too_many_samples[]		= "too many samples.";
static char object_not_found[]  	= "object data file not found.";
static char texture_invalid_file[]	= "texture file invalid or not found.";
static char bitmap_invalid_file[]	= "bitmap file invalid or not found.";
static char bitmap_geom_inval[]		= "bitmap geometry invalid.";
static char bitmap_geom_miss[]		= "bitmap geometry missing.";
static char bitmap_geom_over[]		= "geometry in file overrides specified geometry.";
static char bitmap_range_error[]	= "bitmap geometry out of range.";
static char bitmap_out_of_mem[]		= "not enough memory for bitmap.";
static char sample_invalid_file[]	= "sample file invalid or not found.";
static char volume_invalid[]		= "volume invalid.";
static char volume_range_error[]	= "volume out of range.";
static char stdfreq_invalid[]		= "standard frequency invalid.";
static char stereolevel_invalid[]	= "stereo priority level invalid.";
static char odt_invalid_file[]		= "odt file invalid.";
static char odt_invalid_overwrite[]	= "invalid file used to overwrite existing object.";
static char too_many_lod_fnames[]	= "stripping excessive lods.";
static char ignore_lodmags[]		= "ignoring lodmag spec.";
static char ignore_lodmins[]		= "ignoring lodmin spec.";
static char invalid_lodmags[]		= "lodmag spec missing or invalid.";
static char invalid_lodmins[]		= "lodmin spec missing or invalid.";



// set object's name ----------------------------------------------------------
//
INLINE
void SetObjectName( int idx, char *name )
{
	ASSERT( (dword)idx < MAX_DISTINCT_OBJCLASSES );
	ASSERT( name != NULL );

	if ( ObjectInfo[ idx ].name != NULL )
		FREEMEM( ObjectInfo[ idx ].name );
	ObjectInfo[ idx ].name = (char *) ALLOCMEM( strlen( name ) + 1 );
	if ( ObjectInfo[ idx ].name == NULL )
		OUTOFMEM( 0 );
	strcpy( ObjectInfo[ idx ].name, name );
}


// set object's file name -----------------------------------------------------
//
INLINE
void SetObjectFilename( int idx, char *fname )
{
	ASSERT( (dword)idx < MAX_DISTINCT_OBJCLASSES );
	ASSERT( fname != NULL );

	if ( ObjectInfo[ idx ].file != NULL )
		FREEMEM( ObjectInfo[ idx ].file );
	ObjectInfo[ idx ].file = (char *) ALLOCMEM( strlen( fname ) + 1 );
	if ( ObjectInfo[ idx ].file == NULL )
		OUTOFMEM( 0 );
	strcpy( ObjectInfo[ idx ].file, fname );
}


// make error message including the file name that failed ---------------------
//
PRIVATE
char *MakeFileNameMsg( char *msg, char *fname )
{
	ASSERT( msg != NULL );
	ASSERT( fname != NULL );

	strcpy( paste_str, msg );
	strcat( paste_str, " (" );
	strcat( paste_str, fname );
	strcat( paste_str, ")" );

	ProcessExternalLine( paste_str );

	return paste_str;
}


// temporary tables to store info about object lods ---------------------------
//
static int		object_lod_numlods;
static char*	object_lod_fnames[ MAX_OBJECT_LODS ];
static char*	object_lod_fname_storage = NULL;
static float	object_lod_mags[ MAX_OBJECT_LODS ];
static float	object_lod_mins[ MAX_OBJECT_LODS ];


// read object from odt file --------------------------------------------------
//
PRIVATE
int LoadObjectFromODT( dword objtypeid, int insertindex, dword flags, shader_s *shader )
{
	ASSERT( (dword)object_lod_numlods <= MAX_OBJECT_LODS );
	ASSERT( (dword)insertindex < MAX_DISTINCT_OBJCLASSES );
	ASSERT( object_lod_numlods > 0 );

	// set object type
	ObjectInfo[ insertindex ].type = objtypeid;

	// enter (base) filename into table
	SetObjectFilename( insertindex, object_lod_fnames[ 0 ] );

	// free old lod table
	if ( ObjectInfo[ insertindex ].lodinfo != NULL ) {
		FREEMEM( ObjectInfo[ insertindex ].lodinfo );
		ObjectInfo[ insertindex ].lodinfo = NULL;
	}

	if ( object_lod_numlods > 1 ) {

		// alloc and init lodinfo table
		size_t nsize = 0;
		int lod = 0;
		for ( lod = 0; lod < object_lod_numlods; lod++ )
			nsize += strlen( object_lod_fnames[ lod ] ) + 1;
		lodinfo_s *lodinfo = (lodinfo_s *) ALLOCMEM( sizeof( lodinfo_s ) + nsize +
			object_lod_numlods * ( sizeof( char* ) + sizeof( geomv_t ) * 2 ) );
		if ( lodinfo == NULL ) {
			OUTOFMEM( 0 );
		}

		lodinfo->numlods = object_lod_numlods;
		lodinfo->filetab = (char **)   ( (char*)lodinfo + sizeof( lodinfo_s ) );
		lodinfo->lodmags = (geomv_t *) ( (char*)lodinfo->filetab + object_lod_numlods * sizeof( char* ) + nsize );
		lodinfo->lodmins = (geomv_t *) ( (char*)lodinfo->lodmags + object_lod_numlods * sizeof( geomv_t ) );

		char *fnamestore = (char *) ( (char*)lodinfo->filetab + object_lod_numlods * sizeof( char* ) );
		for ( lod = 0; lod < object_lod_numlods; lod++ ) {

			lodinfo->filetab[ lod ] = fnamestore;
			strcpy( fnamestore, object_lod_fnames[ lod ] );
			fnamestore += strlen( object_lod_fnames[ lod ] ) + 1;

			if ( lod < object_lod_numlods - 1 ) {
				lodinfo->lodmags[ lod ] = FLOAT_TO_GEOMV( object_lod_mags[ lod ] );
				lodinfo->lodmins[ lod ] = FLOAT_TO_GEOMV( object_lod_mins[ lod ] );
			} else {
				lodinfo->lodmags[ lod ] = GEOMV_0;
				lodinfo->lodmins[ lod ] = GEOMV_0;
			}
		}

		// attach lodinfo table
		ObjectInfo[ insertindex ].lodinfo = lodinfo;
	}

	// actually load data and create object
	return OBJ_LoadODT( insertindex, flags, shader );
}


// determine lod spec for object, fill tables accordingly ---------------------
//
PRIVATE
int DetermineODTLodSpec( char *fname, key_value_s *lodmags, key_value_s *lodmins )
{
	ASSERT( fname != NULL );
	ASSERT( lodmags != NULL );
	ASSERT( lodmins != NULL );

	// parse filename, determine lods
	int numlods = 0;
	char *fdest = object_lod_fname_storage;
	ASSERT( fdest != NULL );

	for ( char *fscan = fname; ; ) {

		// strip whitespace
		while ( *fscan == ' ' )
			fscan++;
		fname = fscan;
		if ( *fscan == 0 )
			break;
		while ( ( *fscan != ' ' ) && ( *fscan != 0 ) )
			fscan++;

		// avoid overflowing tables
		if ( numlods >= MAX_OBJECT_LODS ) {
			CON_AddLine( too_many_lod_fnames );
			break;
		}

		int last = ( *fscan == 0 );
		*fscan++ = 0;

		// process file name and use current workdir
		fname = SubstCurWorkDir( fname );

		int len = strlen( fname );
		if ( ( len < 5 ) || ( fname[ len - 4 ] != '.' ) ) {
			CON_AddLine( MakeFileNameMsg( filename_invalid, fname ) );
			return FALSE;
		}

		// try to find odt file
		FILE *fp = SYS_fopen( fname, "rb" );
		if ( fp == NULL ) {
			CON_AddLine( MakeFileNameMsg( object_not_found, fname ) );
			return FALSE;
		}
		SYS_fclose( fp );

		//NOTE:
		// this check is necessary to prevent OBJ_LoadODT() from
		// exiting with an error message to the shell later on if
		// the object file is simply missing.

		// store lod file name
		strcpy( fdest, fname );
		object_lod_fnames[ numlods++ ] = fdest;
		fdest += strlen( fname ) + 1;

		if ( last ) {
			break;
		}
	}

	if ( numlods < 1 ) {
		CON_AddLine( filename_missing );
		return FALSE;
	}

	// set temporary global
	object_lod_numlods = numlods;

	if ( numlods == 1 ) {
		if ( lodmags->value != NULL )
			CON_AddLine( ignore_lodmags );
		if ( lodmins->value != NULL )
			CON_AddLine( ignore_lodmins );
		return TRUE;
	}

	// thresholds are between lods
	int numthresh = numlods - 1;

	// parse lod mags and mins
	if ( ScanKeyValueFloatList( lodmags, object_lod_mags, numthresh, numthresh ) != numthresh ) {
		CON_AddLine( invalid_lodmags );
		return FALSE;
	}
	if ( ScanKeyValueFloatList( lodmins, object_lod_mins, numthresh, numthresh ) != numthresh ) {
		CON_AddLine( invalid_lodmins );
		return FALSE;
	}

	return TRUE;
}


// flag map for objload_xx flags ----------------------------------------------
//
flag_map_s objload_flag_map[] = {

	{ "nodefaults",		OBJLOAD_NONE,				0	},
	{ "w_normals",		OBJLOAD_WEDGENORMALS,		0	},
	{ "w_colors",		OBJLOAD_WEDGECOLORS,		0	},
	{ "w_texcoords",	OBJLOAD_WEDGETEXCOORDS,		0	},
	{ "w_lighted",		OBJLOAD_WEDGELIGHTED,		0	},
	{ "w_specular",		OBJLOAD_WEDGESPECULAR,		0	},
	{ "w_fogged",		OBJLOAD_WEDGEFOGGED,		0	},
	{ "f_anims",		OBJLOAD_FACEANIMS,			0	},
	{ "v_anims",		OBJLOAD_VTXANIMS,			0	},
	{ "p_colors",		OBJLOAD_POLYCORNERCOLORS,	0	},
	{ "p_wedges",		OBJLOAD_POLYWEDGEINDEXES,	0	},

	{ NULL,				0,							0	},
};


// parse objload_xx flags for object ------------------------------------------
//
PRIVATE
dword DetermineODTFlags( key_value_s *flagspec )
{
	ASSERT( flagspec != NULL );

	dword objloadflags = OBJLOAD_DEFAULT;

	if ( flagspec->value != NULL ) {

		// parse flag list, set corresponding bits
		objloadflags = OBJLOAD_NONE;
		if ( ScanKeyValueFlagList( flagspec, &objloadflags, objload_flag_map ) == 0 ) {
			objloadflags = OBJLOAD_DEFAULT;
		}
	}

	return objloadflags;
}


// key table for object loading -----------------------------------------------
//
key_value_s object_key_value[] = {

	{ "file",		NULL,	KEYVALFLAG_PARENTHESIZE	},
	{ "type",		NULL,	KEYVALFLAG_NONE			},
	{ "flags",		NULL,	KEYVALFLAG_PARENTHESIZE	},
	{ "lodmag",		NULL,	KEYVALFLAG_PARENTHESIZE	},
	{ "lodmin",		NULL,	KEYVALFLAG_PARENTHESIZE	},
	{ "shader",		NULL,	KEYVALFLAG_PARENTHESIZE	},
	{ "normth",		NULL,	KEYVALFLAG_NONE			},
	{ "maxfanims",	NULL,	KEYVALFLAG_NONE			},
	{ "maxvanims",	NULL,	KEYVALFLAG_NONE			},

	{ NULL,			NULL,	KEYVALFLAG_NONE			},
};

enum {

	KEY_OBJ_FILE,
	KEY_OBJ_TYPE,
	KEY_OBJ_FLAGS,
	KEY_OBJ_LODMAG,
	KEY_OBJ_LODMIN,
	KEY_OBJ_SHADER,
	KEY_OBJ_NORMTH,
	KEY_OBJ_MAXFANIMS,
	KEY_OBJ_MAXVANIMS,
};


// load object data file from console -----------------------------------------
//
PRIVATE
int ConLoadObject( char *name )
{
	ASSERT( name != NULL );

	// check if object slot available
	if ( NumLoadedObjects >= MAX_DISTINCT_OBJCLASSES - 1 ) {
		CON_AddLine( too_many_objects );
		return FALSE;
	}

	// check if object of same name already exists
	unsigned int insertindex = NumLoadedObjects;
	for ( int oid = 0; oid < NumLoadedObjects; oid++ ) {
		if ( stricmp( ObjectInfo[ oid ].name, name ) == 0 ) {
			if ( AUX_ENABLE_OBJECT_OVERLOADING ) {
				insertindex = oid;
				break;
			} else {
				CON_AddLine( name_already_used );
				return FALSE;
			}
		}
	}

	// set flag if new object should be appended
	int newobject = ( insertindex == (unsigned int)NumLoadedObjects );

	// scan out all values to texture keys
	if ( !ScanKeyValuePairs( object_key_value, NULL ) )
		return FALSE;

	// fetch file name/lod file name list
	char *fname = object_key_value[ KEY_OBJ_FILE ].value;
	if ( fname == NULL ) {
		CON_AddLine( filename_missing );
		return FALSE;
	}

	// alloc tempmem for lod object file names
	object_lod_fname_storage = (char *) ALLOCMEM( MAX_OBJECT_LODS * PATH_MAX );
	if ( object_lod_fname_storage == NULL )
		OUTOFMEM( 0 );

	// parse lod spec (filenames, mags, mins)
	if ( !DetermineODTLodSpec( fname,
		&object_key_value[ KEY_OBJ_LODMAG ],
		&object_key_value[ KEY_OBJ_LODMIN ] ) ) {

		return FALSE;
	}

	// parse flags
	dword objloadflags = DetermineODTFlags( &object_key_value[ KEY_OBJ_FLAGS ] );

	// parse shader
	shader_s *shader =DetermineODTShader( &object_key_value[ KEY_OBJ_SHADER ] );

	// determine object's type id and prepare object loading
	dword objtypeid;
	if ( newobject ) {

		// fetch type specifier
		char *typestr = object_key_value[ KEY_OBJ_TYPE ].value;
		if ( typestr == NULL ) {
			CON_AddLine( type_missing );
			return FALSE;
		}

		// translate type specifier into id
		objtypeid = OBJ_FetchTypeIdFromName( typestr );

		// check for invalid type
		if ( objtypeid == TYPE_ID_INVALID ) {
			CON_AddLine( type_invalid );
			return FALSE;
		}

		// enter new object name into table
		SetObjectName( insertindex, name );

	} else {

		// prevent elimination of local player's ship by disallowing
		// object substitution of player's ship class
		if ( MyShip->ObjectClass == insertindex ) {
			// allow it on startup, though
			if ( !TextModeActive ) {
				CON_AddLine( no_myship_subst );
				return FALSE;
			}
		}

		// check for optional type specifier
		char *typestr = object_key_value[ KEY_OBJ_TYPE ].value;
		if ( typestr != NULL ) {

			// translate type specifier into id
			objtypeid = OBJ_FetchTypeIdFromName( typestr );

			// check for invalid type
			if ( objtypeid == TYPE_ID_INVALID ) {
				CON_AddLine( type_reused );
				objtypeid = ObjectInfo[ insertindex ].type;
			}

			// LoadObjectFromODT() also sets this
			ObjectInfo[ insertindex ].type = objtypeid;

		} else {

			// use type of object that is being overwritten
			objtypeid = ObjectInfo[ insertindex ].type;
		}

		// kill all instances of previous object
		int killcount = KillClassInstances( insertindex );
		if ( killcount > 0 ) {
			sprintf( paste_str, "number of removed class instances: %d.", killcount );
			CON_AddLine( paste_str );
		}

		// free previous object's data
		FREEMEM( ObjClasses[ insertindex ] );
		ObjClasses[ insertindex ] = NULL;
	}

	// save current loading params
	odt_loading_params_s oldloadingparams = odt_loading_params;

	// change normal merging threshold if specified
	if ( object_key_value[ KEY_OBJ_NORMTH ].value != NULL ) {
		float fthresh;
		if ( ScanKeyValueFloat( &object_key_value[ KEY_OBJ_NORMTH ], &fthresh ) == 1 ) {
			odt_loading_params.merge_normals_threshold = FLOAT_TO_GEOMV( fthresh );
		}
	}

	// change maximum number of face anim states if specified
	if ( object_key_value[ KEY_OBJ_MAXFANIMS ].value != NULL ) {
		int maxfanims;
		if ( ScanKeyValueInt( &object_key_value[ KEY_OBJ_MAXFANIMS ], &maxfanims ) == 1 ) {
			odt_loading_params.max_face_anim_states = maxfanims;
		}
	}

	// change maximum number of vertex anim states if specified
	if ( object_key_value[ KEY_OBJ_MAXVANIMS ].value != NULL ) {
		int maxvanims;
		if ( ScanKeyValueInt( &object_key_value[ KEY_OBJ_MAXVANIMS ], &maxvanims ) == 1 ) {
			odt_loading_params.max_vtx_anim_states = maxvanims;
		}
	}

	// load data
	int rc = LoadObjectFromODT( objtypeid, insertindex, objloadflags, shader );

	// restore loading params
	odt_loading_params = oldloadingparams;

	if ( rc <= 0 ) {

		//NOTE:
		// reaction to load error depends on whether a new
		// object should have been created or an already
		// existing substituted. if the latter is the case
		// the old object has already been killed and there
		// is no valid one to use. so, a not-so-graceful
		// program exit is performed.

		if ( newobject ) {
			CON_AddLine(
				MakeFileNameMsg( odt_invalid_file, object_lod_fnames[ -rc ] ) );
			return FALSE;
		} else {
			PANIC(
				MakeFileNameMsg( odt_invalid_overwrite, object_lod_fnames[ -rc ] ) );
		}
	}

	// free tempmem for lod file names
	FREEMEM( object_lod_fname_storage );
	object_lod_fname_storage = NULL;

	if ( newobject ) {

		// increment number of loaded objects
		NumLoadedObjects++;

		// display assigned id
		sprintf( paste_str, "assigned object class id: %d.", insertindex );
		CON_AddLine( paste_str );

	} else {

		// display id of overwritten class
		sprintf( paste_str, "overwrote object class id: %d.", insertindex );
		CON_AddLine( paste_str );
	}

	// set global number of object classes
	NumObjClasses = NumLoadedObjects;

	return TRUE;
}


// set texture's name ---------------------------------------------------------
//
INLINE
void SetTextureName( int idx, char *name )
{
	ASSERT( name != NULL );

	if ( TextureInfo[ idx ].name != NULL )
		FREEMEM( TextureInfo[ idx ].name );
	TextureInfo[ idx ].name = (char *) ALLOCMEM( strlen( name ) + 1 );
	if ( TextureInfo[ idx ].name == NULL )
		OUTOFMEM( 0 );
	strcpy( TextureInfo[ idx ].name, name );
}


// set texture's file name ----------------------------------------------------
//
INLINE
void SetTextureFilename( int idx, char *fname )
{
	ASSERT( fname != NULL );

	if ( TextureInfo[ idx ].file != NULL )
		FREEMEM( TextureInfo[ idx ].file );
	TextureInfo[ idx ].file = (char *) ALLOCMEM( strlen( fname ) + 1 );
	if ( TextureInfo[ idx ].file == NULL )
		OUTOFMEM( 0 );
	strcpy( TextureInfo[ idx ].file, fname );
}


// set texture's loading parameters (optional) --------------------------------
//
INLINE
void SetTextureLoaderParams( int idx, char *params )
{
	if ( TextureInfo[ idx ].loaderparams != NULL ) {
		FREEMEM( TextureInfo[ idx ].loaderparams );
		TextureInfo[ idx ].loaderparams = NULL;
	}
	if ( params != NULL ) {
		TextureInfo[ idx ].loaderparams = (char *) ALLOCMEM( strlen( params ) + 1 );
		if ( TextureInfo[ idx ].loaderparams == NULL )
			OUTOFMEM( 0 );
		strcpy( TextureInfo[ idx ].loaderparams, params );
	}
}


// register a new texture (remember loading params for automatic reloading) ---
//
void RegisterTexture( int insertindex, char *name, char *fname, char *loaderparams )
{
	ASSERT( name != NULL );
	ASSERT( fname != NULL );

	// enter file name into table
	SetTextureFilename( insertindex, fname );

	// set texture name for new textures
	if ( insertindex == NumLoadedTextures ) {

		// enter new texture name into table
		SetTextureName( insertindex, name );
		NumLoadedTextures++;

		// texture map structure needs access to the texture name
		TextureMap *texmap = TextureInfo[ insertindex ].texpointer;
		ASSERT( texmap != NULL );
		ASSERT( texmap->TexMapName == NULL );
		texmap->TexMapName = TextureInfo[ insertindex ].name;
	}

	// name must be set (either still the old or the new name)
	ASSERT( TextureInfo[ insertindex ].texpointer->TexMapName != NULL );

	// set loading parameters (may be NULL)
	SetTextureLoaderParams( insertindex, loaderparams );

	// remember whether we potentially read from the package
	TextureInfo[ insertindex ].flags &= ~TEXINFOFLAG_PACKWASDISABLED;
	if ( AUX_DISABLE_PACKAGE_DATA_FILES ) {
		TextureInfo[ insertindex ].flags |= TEXINFOFLAG_PACKWASDISABLED;
	}
}


// key table for texture loading ----------------------------------------------
//
key_value_s texture_key_value[] = {

	{ "file",		NULL,	KEYVALFLAG_NONE			},
	{ "format",		NULL,	KEYVALFLAG_PARENTHESIZE	},
//	{ "width",		NULL,	KEYVALFLAG_NONE			},
//	{ "height",		NULL,	KEYVALFLAG_NONE			},
//	{ "lodmin",		NULL,	KEYVALFLAG_NONE			},
//	{ "lodmax",		NULL,	KEYVALFLAG_NONE			},
//	{ "aspect",		NULL,	KEYVALFLAG_NONE			},
//	{ "flags",		NULL,	KEYVALFLAG_NONE			},

	{ NULL,			NULL,	KEYVALFLAG_NONE			},
};

enum {

	KEY_TEX_FILE,		// file name
	KEY_TEX_FORMAT,		// depends on file format (passed through to loader)
//	KEY_TEX_WIDTH,
//	KEY_TEX_HEIGHT,
//	KEY_TEX_LODMIN,
//	KEY_TEX_LODMAX,
//	KEY_TEX_ASPECT,
//	KEY_TEX_FLAGS,
};


// load texture file from console ---------------------------------------------
//
PRIVATE
int ConLoadTexture( char *name )
{
	ASSERT( name != NULL );

	// check if texture slot available
	if ( NumLoadedTextures >= MAX_TEXTURES - 1 ) {
		CON_AddLine( too_many_textures );
		return FALSE;
	}

	// check if texture of same name already exists
	int insertindex = NumLoadedTextures;
	for ( int tid = 0; tid < NumLoadedTextures; tid++ ) {
		if ( stricmp( TextureInfo[ tid ].name, name ) == 0 ) {
			if ( AUX_ENABLE_TEXTURE_OVERLOADING ) {
				insertindex = tid;
				break;
			} else {
				CON_AddLine( name_already_used );
				return FALSE;
			}
		}
	}

	// scan out all values to texture keys
	if ( !ScanKeyValuePairs( texture_key_value, NULL ) )
		return FALSE;

	// fetch file name
	char *fname = texture_key_value[ KEY_TEX_FILE ].value;
	if ( fname == NULL ) {
		CON_AddLine( filename_missing );
		return FALSE;
	}

	// process file name and use current workdir
	fname = SubstCurWorkDir( fname );

	int len = strlen( fname );
	if ( ( len < 5 ) || ( fname[ len - 4 ] != '.' ) ) {
		CON_AddLine( filename_invalid );
		return FALSE;
	}

	// fetch explicitly supplied format (may be NULL)
	char *formatspec = texture_key_value[ KEY_TEX_FORMAT ].value;

	// load texture from file
	if ( !IMG_LoadTexture( fname, insertindex, formatspec, NULL ) ) {
		CON_AddLine( MakeFileNameMsg( texture_invalid_file, fname ) );
		return FALSE;
	}

	// set texture info
	RegisterTexture( insertindex, name, fname, formatspec );

	return TRUE;
}


// order in which chars must be listed in a charset for a texfont -------------
//
static int		num_charseq = 0;
static byte*	char_sequence = NULL;


// order of special chars that are not directly listed in ascii order ---------
//
static byte special_chars[] = {

//	ü,    ä,    Ä,    ö,    Ö,    Ü
	0x81, 0x84, 0x8e, 0x94, 0x99, 0x9a
};


/* char sequence for alex fonts (obsolete) ------------------------------------
//
//static byte alex_char_sequence[] = {
//	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" \
//	"1234567890  !\"§$%&/()=? <;:- *> .-#+öäü\'[]\\|~{}@"
//};*/


// set texfont's name ---------------------------------------------------------
//
INLINE
void SetTexfontName( int idx, char *name )
{
	ASSERT( name != NULL );

	if ( TexfontInfo[ idx ].name != NULL )
		FREEMEM( TexfontInfo[ idx ].name );
	TexfontInfo[ idx ].name = (char *) ALLOCMEM( strlen( name ) + 1 );
	if ( TexfontInfo[ idx ].name == NULL )
		OUTOFMEM( 0 );
	strcpy( TexfontInfo[ idx ].name, name );
}


// set texfont's file name ----------------------------------------------------
//
INLINE
void SetTexfontFilename( int idx, char *fname )
{
	ASSERT( fname != NULL );

	if ( TexfontInfo[ idx ].file != NULL )
		FREEMEM( TexfontInfo[ idx ].file );
	TexfontInfo[ idx ].file = (char *) ALLOCMEM( strlen( fname ) + 1 );
	if ( TexfontInfo[ idx ].file == NULL )
		OUTOFMEM( 0 );
	strcpy( TexfontInfo[ idx ].file, fname );
}


// register a new texfont -----------------------------------------------------
//
void RegisterTexfont( int insertindex, char *name, char *fname, texfont_s *texfont, int charwidth, int charheight, int spacing )
{
	ASSERT( name != NULL );
	ASSERT( fname != NULL );
	ASSERT( texfont != NULL );

	// enter texfont file name into table
	SetTexfontFilename( insertindex, fname );

	// new texfont
	if ( insertindex == NumLoadedTexfonts ) {

		// enter new texfont name into table
		SetTexfontName( insertindex, name );
		NumLoadedTexfonts++;

		// take basic info from texture
		TexfontInfo[ insertindex ].texfont	= texfont;
		TexfontInfo[ insertindex ].width	= charwidth;
		TexfontInfo[ insertindex ].height	= charheight;
		TexfontInfo[ insertindex ].srcimage	= NULL;

		// convert geometry table
		texchar_s *texchars = (texchar_s *) ALLOCMEM( 256 * sizeof( texchar_s ) );
		if ( texchars == NULL )
			OUTOFMEM( 0 );
		memset( texchars, 0, 256 * sizeof( texchar_s ) );

		for ( int cid = 0; cid < texfont->numtexchars; cid++ ) {
			if ( cid >= num_charseq )
				break;
			int chcode = char_sequence[ cid ];
			if ( chcode == 0 )
				break;
			if ( chcode == 32 )
				continue;
			memcpy( &texchars[ chcode ], &texfont->texchars[ cid ], sizeof( texchar_s ) );
		}

		// set space
		texchars[ 32 ].tex_id    = -1;
		texchars[ 32 ].tex_ustep = spacing * 6;

		FREEMEM( texfont->texchars );
		texfont->texchars = texchars;
		texfont->numtexchars = 256;
	}
}


// key table for texfont loading ----------------------------------------------
//
key_value_s texfont_key_value[] = {

	{ "file",		NULL,	KEYVALFLAG_NONE			},
	{ "format",		NULL,	KEYVALFLAG_PARENTHESIZE	},
	{ "width",		NULL,	KEYVALFLAG_NONE			},
	{ "height",		NULL,	KEYVALFLAG_NONE			},
	{ "spacing",	NULL,	KEYVALFLAG_NONE			},
	{ "flags",		NULL,	KEYVALFLAG_PARENTHESIZE	},

	{ NULL,			NULL,	KEYVALFLAG_NONE			},
};

enum {

	KEY_FONT_FILE,		// file name
	KEY_FONT_FORMAT,	// depends on file format (passed through to loader)
	KEY_FONT_WIDTH,		// tile width
	KEY_FONT_HEIGHT,	// tile height
	KEY_FONT_SPACING,	// spacing to the left and right of each char
	KEY_FONT_FLAGS,
};


// load font from console and create texfont ----------------------------------
//
PRIVATE
int ConLoadTexfont( char *name )
{
	ASSERT( name != NULL );

	// check if texfont slot available
	if ( NumLoadedTexfonts >= MAX_TEXFONTS - 1 ) {
		CON_AddLine( too_many_texfonts );
		return FALSE;
	}

	// need also texture slot
	if ( NumLoadedTextures >= MAX_TEXTURES - 1 ) {
		CON_AddLine( too_many_textures );
		return FALSE;
	}

	// check if texfont of same name already exists
	int texfontindex = NumLoadedTexfonts;
	int tid = 0;
	for ( tid = 0; tid < NumLoadedTexfonts; tid++ ) {
		if ( stricmp( TexfontInfo[ tid ].name, name ) == 0 ) {
			texfontindex = tid;
			break;
		}
	}

	// check if texture of same name already exists
	int texindex = NumLoadedTextures;
	for ( tid = 0; tid < NumLoadedTextures; tid++ ) {
		if ( stricmp( TextureInfo[ tid ].name, name ) == 0 ) {
			if ( AUX_ENABLE_TEXTURE_OVERLOADING ) {
				texindex = tid;
				break;
			} else {
				CON_AddLine( name_already_used );
				return FALSE;
			}
		}
	}

	// scan out all values to texfont keys
	if ( !ScanKeyValuePairs( texfont_key_value, NULL ) )
		return FALSE;

	// fetch file name
	char *fname = texfont_key_value[ KEY_FONT_FILE ].value;
	if ( fname == NULL ) {
		CON_AddLine( filename_missing );
		return FALSE;
	}

	// process file name and use current workdir
	fname = SubstCurWorkDir( fname );

	size_t len = strlen( fname );
	if ( ( len < 5 ) || ( fname[ len - 4 ] != '.' ) ) {
		CON_AddLine( filename_invalid );
		return FALSE;
	}

	// let user override automatically determined geometry
	int charwidth  = 0;
	int charheight = 0;
	int spacing    = 2;
	if ( ScanKeyValueInt( &texfont_key_value[ KEY_FONT_WIDTH ], &charwidth ) < 0 ) {
		return FALSE;
	}
	if ( ScanKeyValueInt( &texfont_key_value[ KEY_FONT_HEIGHT ], &charheight ) < 0 ) {
		return FALSE;
	}
	if ( ScanKeyValueInt( &texfont_key_value[ KEY_FONT_SPACING ], &spacing ) < 0 ) {
		return FALSE;
	}

	// destination texture geometry
	int texwidth  = 512;
	int texheight = 256;

	// fetch explicitly supplied format (may be NULL)
	char *formatspec = texfont_key_value[ KEY_FONT_FORMAT ].value;

	// append destination texture and retiling geometry
	sprintf( paste_str, "map_width %d map_height %d retile_width %d retile_height %d spacing %d ",
		texwidth, texheight, charwidth, charheight, spacing );
	if ( formatspec != NULL ) {
		strcat( paste_str, formatspec );
	}

	// append optional flags
	if ( texfont_key_value[ KEY_FONT_FLAGS ].value != NULL ) {
		strcat( paste_str, " flags " );
		strcat( paste_str, texfont_key_value[ KEY_FONT_FLAGS ].value );
	}

	// allocate new texfont if not already existing
	texfont_s *texfont = NULL;
	if ( texfontindex == NumLoadedTexfonts ) {
		texfont = (texfont_s *) ALLOCMEM( sizeof( texfont_s ) );
		if ( texfont == NULL )
			OUTOFMEM( 0 );
		memset( texfont, 0, sizeof( texfont_s ) );
	}

	// load font from file and convert into texture
	if ( !IMG_LoadTexture( fname, texindex, paste_str, texfont ) ) {
		CON_AddLine( MakeFileNameMsg( texture_invalid_file, fname ) );
		return FALSE;
	}

	// deduce from texture geometry if not overridden
	if ( charwidth == 0 ) {
		charwidth = TextureInfo[ texindex ].width;
	}
	if ( charheight == 0 ) {
		// intentionally not height of texture
		charheight = TextureInfo[ texindex ].width;
	}

	// set texture info
	RegisterTexture( texindex, name, fname, paste_str );

	// set texfont info
	RegisterTexfont( texfontindex, name, fname, texfont, charwidth, charheight, spacing );

	return TRUE;
}


// set bitmap's name ----------------------------------------------------------
//
INLINE
void SetBitmapName( int idx, char *name )
{
	ASSERT( name != NULL );

	if ( BitmapInfo[ idx ].name != NULL )
		FREEMEM( BitmapInfo[ idx ].name );
	BitmapInfo[ idx ].name = (char *) ALLOCMEM( strlen( name ) + 1 );
	if ( BitmapInfo[ idx ].name == NULL )
		OUTOFMEM( 0 );
	strcpy( BitmapInfo[ idx ].name, name );
}


// set bitmap's file name -----------------------------------------------------
//
INLINE
void SetBitmapFilename( int idx, char *fname )
{
	ASSERT( fname != NULL );

	if ( BitmapInfo[ idx ].file != NULL )
		FREEMEM( BitmapInfo[ idx ].file );
	BitmapInfo[ idx ].file = (char *) ALLOCMEM( strlen( fname ) + 1 );
	if ( BitmapInfo[ idx ].file == NULL )
		OUTOFMEM( 0 );
	strcpy( BitmapInfo[ idx ].file, fname );
}


// key table for bitmap loading -----------------------------------------------
//
key_value_s bitmap_key_value[] = {

	{ "file",		NULL,	KEYVALFLAG_NONE	},
	{ "width",		NULL,	KEYVALFLAG_NONE	},
	{ "height",		NULL,	KEYVALFLAG_NONE	},

	{ NULL,			NULL,	KEYVALFLAG_NONE	},
};

enum {

	KEY_BITMAP_FILE,
	KEY_BITMAP_WIDTH,
	KEY_BITMAP_HEIGHT,
};


// load bitmap file from console ----------------------------------------------
//
PRIVATE
int ConLoadBitmap( char *name )
{
	ASSERT( name != NULL );

	// check if bitmap slot available
	if ( NumLoadedBitmaps >= MAX_BITMAPS - 1 ) {
		CON_AddLine( too_many_bitmaps );
		return FALSE;
	}

	// check if bitmap of same name already exists
	int insertindex = NumLoadedBitmaps;
	for ( int bid = 0; bid < NumLoadedBitmaps; bid++ ) {
		if ( stricmp( BitmapInfo[ bid ].name, name ) == 0 ) {
			if ( AUX_ENABLE_BITMAP_OVERLOADING ) {
				insertindex = bid;
				break;
			} else {
				CON_AddLine( name_already_used );
				return FALSE;
			}
		}
	}

	// scan out all values to bitmap keys
	if ( !ScanKeyValuePairs( bitmap_key_value, NULL ) )
		return FALSE;

	// fetch file name
	char *fname = bitmap_key_value[ KEY_BITMAP_FILE ].value;
	if ( fname == NULL ) {
		CON_AddLine( filename_missing );
		return FALSE;
	}

	// process file name and use current workdir
	fname = SubstCurWorkDir( fname );

	size_t len = strlen( fname );
	if ( ( len < 5 ) || ( fname[ len - 4 ] != '.' ) ) {
		CON_AddLine( filename_invalid );
		return FALSE;
	}

	// fetch width parameter
	int width = -1;
	if ( ScanKeyValueInt( &bitmap_key_value[ KEY_BITMAP_WIDTH ], &width ) < 0 ) {
		CON_AddLine( bitmap_geom_inval );
		return FALSE;
	}
	if ( ( width != -1 ) && ( ( width < 1 ) || ( width > 640 ) ) ) {
		CON_AddLine( bitmap_range_error );
		return FALSE;
	}
	int newwidth = width;

	// fetch height parameter
	int height = -1;
	if ( ScanKeyValueInt( &bitmap_key_value[ KEY_BITMAP_HEIGHT ], &height ) < 0 ) {
		CON_AddLine( bitmap_geom_inval );
		return FALSE;
	}
	if ( ( height != -1 ) && ( ( height < 1 ) || ( height > 480 ) ) ) {
		CON_AddLine( bitmap_range_error );
		return FALSE;
	}
	int newheight = height;

	// get file size
	ssize_t fsiz = SYS_GetFileLength( fname );
	if ( fsiz == -1 ) {
		CON_AddLine( MakeFileNameMsg( bitmap_invalid_file, fname ) );
		return FALSE;
	}

	// alloc mem for single bitmap
	char *bitmapbuffer = (char *) ALLOCMEM( fsiz );
	if ( bitmapbuffer == NULL ) {
		CON_AddLine( bitmap_out_of_mem );
		return FALSE;
	}

	// open bitmap file
	FILE *fp = SYS_fopen( fname, "rb" );
	if ( fp == NULL ) {
		CON_AddLine( MakeFileNameMsg( bitmap_invalid_file, fname ) );
		return FALSE;
	}

	// read header
	BdtHeader *header = (BdtHeader *) bitmapbuffer;
	ssize_t headerbytes = SYS_fread( header, 1, sizeof( BdtHeader ), fp );
	int headervalid = ( headerbytes == sizeof( BdtHeader ) );
	ASSERT( fsiz > headerbytes );
	fsiz -= headerbytes;

	if ( headervalid ) {

		if ( stricmp( header->signature, BDT_SIGNATURE ) != 0 )
			headervalid = FALSE;
		if ( header->version < REQUIRED_BDT_VERSION )
			headervalid = FALSE;

		if ( headervalid ) {

			SYS_SwapBdtHeader( header );

			// use width and height in header
			if ( ( newwidth != -1 ) || ( newheight != -1 ) ) {
				CON_AddLine( bitmap_geom_over );
			}
			newwidth  = header->width;
			newheight = header->height;

			// display geometry as contained in header
			sprintf( paste_str, "bitmap geometry: width=%d height=%d.",
					 newwidth, newheight );
			CON_AddLine( paste_str );

		} else {

			if ( ( newwidth  == -1 ) || ( newheight == -1 ) ) {
				SYS_fclose( fp );
				CON_AddLine( bitmap_geom_miss );
				return FALSE;
			}
		}
	}

	// check bitmap geometry
	ssize_t geosiz = newwidth * newheight;
	if ( headervalid ) {
		if ( fsiz != geosiz ) {
			SYS_fclose( fp );
			CON_AddLine( bitmap_geom_inval );
			return FALSE;
		}
	} else {
		if ( ( fsiz + headerbytes ) != geosiz ) {
			CON_AddLine( bitmap_geom_inval );
			SYS_fclose( fp );
			return FALSE;
		}
	}

	// load bitmap data (overwrite header if any)
	char *datadest = bitmapbuffer;
	if ( !headervalid )
		datadest += sizeof( BdtHeader );
	ssize_t bytesread = SYS_fread( datadest, 1, fsiz, fp );
	if ( bytesread != fsiz ) {
		SYS_fclose( fp );
		CON_AddLine( MakeFileNameMsg( bitmap_invalid_file, fname ) );
		return FALSE;
	}

	// close bitmap data
	SYS_fclose( fp );

	if ( insertindex < NumLoadedBitmaps ) {

		//NOTE:
		// if the bitmap has been converted to a texture
		// it may be resident in the texture cache.
		// therefore we need to invalidate this bitmap in
		// the texture cache.

		if ( !TextModeActive ) {
			TextureMap tmap;
			memset( &tmap, 0, sizeof( tmap ) );
			tmap.BitMap = BitmapInfo[ insertindex ].bitmappointer;
			ASSERT( tmap.BitMap != NULL );
			R_InvalidateCachedTexture( &tmap );
		}

		// free previous conversion data
		if ( BitmapInfo[ insertindex ].bitmappointer !=
			 BitmapInfo[ insertindex ].loadeddata ) {

			ASSERT( BitmapInfo[ insertindex ].bitmappointer != NULL );
			FREEMEM( BitmapInfo[ insertindex ].bitmappointer );
			BitmapInfo[ insertindex ].bitmappointer = NULL;
		}

		// free old source bitmap if not part of initial block
		if ( BitmapInfo[ insertindex ].loadeddata != BitmapMem ) {

			ASSERT( BitmapInfo[ insertindex ].loadeddata != NULL );
			FREEMEM( BitmapInfo[ insertindex ].loadeddata );
			BitmapInfo[ insertindex ].loadeddata = NULL;
		}
	}

	// set new data pointers
	BitmapInfo[ insertindex ].loadeddata	= bitmapbuffer;
	BitmapInfo[ insertindex ].bitmappointer	= bitmapbuffer;

	// set new geometry
	BitmapInfo[ insertindex ].width			= newwidth;
	BitmapInfo[ insertindex ].height		= newheight;

	// enter filename into table
	SetBitmapFilename( insertindex, fname );

	if ( insertindex == NumLoadedBitmaps ) {

		// enter name into table
		SetBitmapName( insertindex, name );

		NumLoadedBitmaps++;
	}

	// convert to current color format
	SetupSingleBitmapColors( insertindex );

	return TRUE;
}


// key table for sample loading ----------------------------------------------
//
key_value_s sample_key_value[] = {

	{ "file",		NULL,	KEYVALFLAG_NONE	},
	{ "volume",		NULL,	KEYVALFLAG_NONE },
	{ "stdfreq",	NULL,	KEYVALFLAG_NONE },
	{ "stereo",		NULL,	KEYVALFLAG_NONE },

	{ NULL,			NULL,	KEYVALFLAG_NONE	},
};

enum {

	KEY_SAMPLE_FILE,
	KEY_SAMPLE_VOLUME,
	KEY_SAMPLE_STDFREQ,
	KEY_SAMPLE_STEREO,
};


// set sample's name ----------------------------------------------------------
//
INLINE
void SetSampleName( int idx, char *name )
{
	ASSERT( name != NULL );

	if ( SampleInfo[ idx ].name != NULL )
		FREEMEM( SampleInfo[ idx ].name );
	SampleInfo[ idx ].name = (char *) ALLOCMEM( strlen( name ) + 1 );
	if ( SampleInfo[ idx ].name == NULL )
		OUTOFMEM( 0 );
	strcpy( SampleInfo[ idx ].name, name );
}


// set sample's file name -----------------------------------------------------
//
INLINE
void SetSampleFilename( int idx, char *fname )
{
	ASSERT( fname != NULL );

	if ( SampleInfo[ idx ].file != NULL )
		FREEMEM( SampleInfo[ idx ].file );
	SampleInfo[ idx ].file = (char *) ALLOCMEM( strlen( fname ) + 1 );
	if ( SampleInfo[ idx ].file == NULL )
		OUTOFMEM( 0 );
	strcpy( SampleInfo[ idx ].file, fname );
}


// load sample file from console ----------------------------------------------
//
PRIVATE
int ConLoadSample( char *name )
{
	ASSERT( name != NULL );

	// check if sample slot available
	if ( NumLoadedSamples >= MAX_SAMPLES - 1 ) {
		CON_AddLine( too_many_samples );
		return FALSE;
	}

	// check if sample of same name already exists
	int insertindex = NumLoadedSamples;
	for ( int sid = 0; sid < NumLoadedSamples; sid++ ) {
		if ( stricmp( SampleInfo[ sid ].name, name ) == 0 ) {
			if ( AUX_ENABLE_SAMPLE_OVERLOADING ) {
				insertindex = sid;
				break;
			} else {
				CON_AddLine( name_already_used );
				return FALSE;
			}
		}
	}

	// scan out all values to sample keys
	if ( !ScanKeyValuePairs( sample_key_value, NULL ) )
		return FALSE;

	// fetch file name
	char *fname = sample_key_value[ KEY_SAMPLE_FILE ].value;
	if ( fname == NULL ) {
		CON_AddLine( filename_missing );
		return FALSE;
	}

	// process file name and use current workdir
	fname = SubstCurWorkDir( fname );

	int len = strlen( fname );
	if ( ( len < 5 ) || ( fname[ len - 4 ] != '.' ) ) {
		CON_AddLine( filename_invalid );
		return FALSE;
	}

	// fetch (optional) volume parameter
	int nVolume = AUD_MAX_VOLUME;
	if ( ScanKeyValueInt( &sample_key_value[ KEY_SAMPLE_VOLUME ], &nVolume ) < 0 ) {
		CON_AddLine( volume_invalid );
		return FALSE;
	}
	if ( ( nVolume < 0 ) || ( nVolume > AUD_MAX_VOLUME ) ) {
		CON_AddLine( volume_range_error );
		return FALSE;
	}

	// fetch (optional) standard frequency parameter
	float stdfreq = 44100.0f;
	if ( ScanKeyValueFloat( &sample_key_value[ KEY_SAMPLE_STDFREQ ], &stdfreq ) < 0 ) {
		CON_AddLine( stdfreq_invalid );
		return FALSE;
	}
	if ( ( stdfreq < 5.0f ) || ( stdfreq > 44100.0f ) ) {
		CON_AddLine( stdfreq_invalid );
		return FALSE;
	}

	// fetch (optional) stereo priority level parameter
	int stereolevel = 0;
	if ( ScanKeyValueInt( &sample_key_value[ KEY_SAMPLE_STEREO ], &stereolevel ) < 0 ) {
		CON_AddLine( stereolevel_invalid );
		return FALSE;
	}
	if ( ( stereolevel < 0 ) || ( stereolevel > 5 ) ) {
		CON_AddLine( stereolevel_invalid );
		return FALSE;
	}

	// make sure file is at least readable (failure later on
	// might perform an immediate program exit).
	FILE *fp = SYS_fopen( fname, "rb" );
	if ( fp == NULL ) {
		CON_AddLine( MakeFileNameMsg( sample_invalid_file, fname ) );
		return FALSE;
	}
	SYS_fclose( fp );

	// free previous wave if we overload an already existing entry
	if ( insertindex < NumLoadedSamples ) {

		//NOTE:
		// if no audio subsystem is linked nothing will actually
		// be freed here, although AUDs_FreeWaveFile() will return
		// AUD_RETURN_SUCCESS. in this case the sample will either
		// not have been loaded anyway or simply stay in memory.
		// this prevents an error message.

		if ( AUDs_FreeWaveFile( insertindex ) != AUD_RETURN_SUCCESS ) {
			CON_AddLine( sample_invalid_file );
			return FALSE;
		}
	}

	// enter name into table
	SetSampleName( insertindex, name );

	// enter filename into table
	SetSampleFilename( insertindex, fname );

	//NOTE:
	// if no audio subsystem is linked the sample will not
	// actually be loaded, although AUDs_LoadWaveFile() will
	// return AUD_RETURN_SUCCESS. this prevents an error message.

	// load sample from file
	if ( AUDs_LoadWaveFile( insertindex ) != AUD_RETURN_SUCCESS ) {
		CON_AddLine( MakeFileNameMsg( sample_invalid_file, fname ) );
		return FALSE;
	}

	// store additional info in table
	SampleInfo[ insertindex ].volume	  = nVolume;
	SampleInfo[ insertindex ].stdfreq	  = stdfreq;
	SampleInfo[ insertindex ].stereolevel = stereolevel;

	// this will not happen if an error occurs earlier on!
	if ( insertindex == NumLoadedSamples ) {
		NumLoadedSamples++;
	}

	return TRUE;
}


// main entry point for "load" command (load data via console) ----------------
//
int Cmd_ExecLoadCommand( char *command )
{
	//NOTE:
	//CONCOM:
	// load_command ::= 'load' <data_type> <name> [<key> <value>]*
	// data_type    ::= 'object' | 'texture' | 'texfont' | 'bitmap' | 'sample'
	// name         ::= "may be parenthesized () to include whitespace"
	// key          ::= "valid keys depend on data type"

	ASSERT( command != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( command );

	// split off first token (data type specifier)
	char *datatype = strtok( command, " " );
	if ( datatype == NULL ) {
		CON_AddLine( datatype_missing );
		return TRUE;
	}

	// this flag is used for internal data reloading
	if ( AUX_ALLOW_ONLY_TEXTURE_LOADING ) {
		// disallow everything but textures
		if ( strcmp( datatype, datatype_texture ) != 0 ) {
			return TRUE;
		}
	}

	// create pointer to list of parameters (first is always the name)
	char *name = strtok( NULL, " " );
	if ( name == NULL ) {
		CON_AddLine( name_missing );
		return TRUE;
	}

	// allow name to be parenthesized to include whitespace
	name = GetParenthesizedName( name );
	if ( name == NULL ) {
		CON_AddLine( name_invalid );
		return TRUE;
	}

	// dispatch to functions for handling different data types
	if ( strcmp( datatype, datatype_object ) == 0 )
		ConLoadObject( name );
	else if ( strcmp( datatype, datatype_texture ) == 0 )
		ConLoadTexture( name );
	else if ( strcmp( datatype, datatype_texfont ) == 0 )
		ConLoadTexfont( name );
	else if ( strcmp( datatype, datatype_bitmap ) == 0 )
		ConLoadBitmap( name );
	else if ( strcmp( datatype, datatype_sample ) == 0 )
		ConLoadSample( name );
	else
		CON_AddLine( datatype_invalid );

	// print dot for every loaded item in text-mode
	// (well, also if one of the above routines fails...)
	if ( TextModeActive && AUX_CMD_WRITE_ACTIVE_IN_TEXTMODE ) {
//		static int dot_count = 0x00;
//		if ( ( dot_count++ & 0x01 ) == 0x00 )
			MSGPUT( "." );
	}

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( CON_LOAD )
{
	// file char sequence table
	int numbasechars = ( 0x7f - 0x21 );
	int numspecialchars = sizeof( special_chars );
	char_sequence = (byte *) ALLOCMEM( numbasechars + numspecialchars );
	if ( char_sequence == NULL ) {
		OUTOFMEM( 0 );
	}

	int storepos = 0;
	for ( int ccode = 0x21; ccode < 0x7f; ccode++ ) {
		char_sequence[ storepos++ ] = (byte)ccode;
	}
	for ( int sindx = 0; sindx < numspecialchars; sindx++ ) {
		char_sequence[ storepos++ ] = special_chars[ sindx ];
	}

	ASSERT( storepos == ( numbasechars + numspecialchars ) );
	num_charseq = storepos;
}



