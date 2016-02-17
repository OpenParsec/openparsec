/*
 * PARSEC - Data Loading Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:44 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
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
//#include "e_world_trans.h"

// subsystem headers
//#include "aud_defs.h"

// rendering subsystem
//#include "r_patch.h"

// local module header
#include "con_load_sv.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux_sv.h"
#include "con_ext_sv.h"
#include "con_main_sv.h"
//#include "con_shad.h"
#include "con_std_sv.h"
//#include "e_color.h"
#include "net_limits.h"
#include "net_defs.h"
#include "e_gameserver.h"
//#include "img_api.h"
#include "obj_clas.h"
//#include "obj_ctrl.h"
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
static char too_many_objects[]		= "too many objects.";
static char type_missing[]			= "type missing.";
static char type_invalid[]			= "type invalid.";
static char type_reused[]			= "type invalid. using old type.";
static char object_not_found[]  	= "object data file not found.";
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
        int lod;
		// alloc and init lodinfo table
		size_t nsize = 0;
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
	KEY_OBJ_MAXVANIMS
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
	int insertindex = NumLoadedObjects;
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
	int newobject = ( insertindex == NumLoadedObjects );

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
	//shader_s *shader = DetermineODTShader( &object_key_value[ KEY_OBJ_SHADER ] );
	shader_s *shader = NULL;

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

#ifdef PARSEC_CLIENT
		// prevent elimination of local player's ship by disallowing
		// object substitution of player's ship class
		if ( MyShip->ObjectClass == insertindex ) {
			// allow it on startup, though
			if ( !TextModeActive ) {
				CON_AddLine( no_myship_subst );
				return FALSE;
			}
		}
#endif // PARSEC_CLIENT

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
		int killcount = TheWorld->KillClassInstances( insertindex );
		if ( killcount > 0 ) {
			sprintf( paste_str, "number of removed class instances: %d.", killcount );
			CON_AddLine( paste_str );
		}

		// free previous object's data
		FREEMEM( TheWorld->ObjClasses[ insertindex ] );
		TheWorld->ObjClasses[ insertindex ] = NULL;
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
			CON_AddLine( MakeFileNameMsg( odt_invalid_file, object_lod_fnames[ -rc ] ) );
			return FALSE;
		} else {
			PANIC( MakeFileNameMsg( odt_invalid_overwrite, object_lod_fnames[ -rc ] ) );
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






// main entry point for "load" command (load data via console) ----------------
//
int Cmd_ExecLoadCommand( char *command )
{
	if(TheServer->GetServerIsMaster()){
		return false;
	}
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

#ifdef PARSEC_CLIENT
	// this flag is used for internal data reloading
	if ( AUX_ALLOW_ONLY_TEXTURE_LOADING ) {
		// disallow everything but textures
		if ( strcmp( datatype, datatype_texture ) != 0 ) {
			return TRUE;
		}
	}
#endif // PARSEC_CLIENT

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


#ifdef PARSEC_CLIENT

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

#else // !PARSEC_CLIENT

	// dispatch to functions for handling different data types
	if ( strcmp( datatype, datatype_object ) == 0 )
		ConLoadObject( name );
	else if ( strcmp( datatype, datatype_texture ) == 0 )
		CON_AddLine( "ignoring textures." );
	else if ( strcmp( datatype, datatype_texfont ) == 0 )
		CON_AddLine( "ignoring textfonts." );
	else if ( strcmp( datatype, datatype_bitmap ) == 0 )
		CON_AddLine( "ignoring bitmaps." );
	else if ( strcmp( datatype, datatype_sample ) == 0 )
		CON_AddLine( "ignoring samples." );
	else
		CON_AddLine( datatype_invalid );

#endif // PARSEC_CLIENT

	// print dot for every loaded item in text-mode
	// (well, also if one of the above routines fails...)
	//if ( AUX_CMD_WRITE_ACTIVE_IN_TEXTMODE ) {
		MSGPUT( "." );
	//}
	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( CON_LOAD_SV )
{
}



