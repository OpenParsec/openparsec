/*
 * PARSEC - Geometry Generation Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:23 $
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

// local module header
#include "con_geom.h"

// proprietary module headers
#include "con_arg.h"
#include "con_com.h"
#include "con_main.h"
#include "obj_iter.h"
#include "obj_part.h"



// string constants -----------------------------------------------------------
//
static char reset_iter[]		= "resetting class iter polys for this class.";
static char reset_part[]		= "resetting class particles for this class.";
static char invalid_command[]	= "invalid command.";
static char trafo_invalid[]		= "transformation state invalid.";
static char trafo_set[]			= "setting transformation state.";
static char params_ignored[]	= "other params ignored.";
static char prim_type_invalid[]	= "invalid primitive type.";
static char apex_info_invalid[]	= "apex info invalid.";
static char vtx_list_invalid[]	= "vertex list invalid.";
static char vtx_list_missing[]	= "vertex list missing.";
static char position_invalid[]	= "attach position invalid.";
static char position_missing[]	= "attach position missing.";
static char pdef_missing[]		= "particle definition name not specified.";
static char refz_invalid[]		= "particle size invalid.";
static char rendmode_invalid[]	= "invalid particle render mode.";
static char iter_reg_failed[]	= "iter poly could not be registered.";
static char part_reg_failed[]	= "particle could not be registered.";


// trafo (offset plus scale) to apply to all iter polygons of a class ---------
//
PRIVATE
Point3h_f global_classiter_trafo[ MAX_DISTINCT_OBJCLASSES ];


// key table for classiter command --------------------------------------------
//
key_value_s classiter_key_value[] = {

	{ "class",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "id",			NULL,	KEYVALFLAG_NONE				},
	{ "cmd",		NULL,	KEYVALFLAG_NONE				},
	{ "trafo",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "apex",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "prim",		NULL,	KEYVALFLAG_NONE				},
	{ "vtxs",		NULL,	KEYVALFLAG_PARENTHESIZE		},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_CLASSITER_CLASS,
	KEY_CLASSITER_ID,
	KEY_CLASSITER_CMD,
	KEY_CLASSITER_TRAFO,
	KEY_CLASSITER_APEX,
	KEY_CLASSITER_PRIM,
	KEY_CLASSITER_VTXS
};


// console command for object iter registration ("classiter") -----------------
//
PRIVATE
int Cmd_CLASSITER( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// classiter_command ::= 'classiter' <class_spec> ( [<command>] [<trafo>] ) |
	//						  ( [<apex_info>] [<prim_type>] <vertex_list> )
	// class_spec		 ::= 'class' <classname> | 'id' <classid>
	// command			 ::= 'cmd' 'reset'
	// trafo			 ::= 'trafo' '(' <vertex_spec> [<float>] ')'
	// apex_info		 ::= 'apex' '(' <float> [<float> [<float>]] ')'
	// prim_type		 ::= 'prim' 'tri' | 'quad' | 'pent'
	// vertex_list		 ::= 'vtxs' '(' <vertex_spec>+ ')'
	// vertex_spec		 ::= <float> <float> <float>

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( classiter_key_value, paramstr ) )
		return TRUE;

	// get object class (either name or id)
	dword objclass = ScanKeyValueObjClass( classiter_key_value, KEY_CLASSITER_CLASS, KEY_CLASSITER_ID );
	if ( objclass == CLASS_ID_INVALID ) {
		return TRUE;
	}

	int configset = FALSE;

	// check for special commands
	if ( classiter_key_value[ KEY_CLASSITER_CMD ].value != NULL ) {

		if ( stricmp( classiter_key_value[ KEY_CLASSITER_CMD ].value, "reset" ) == 0 ) {

			CON_AddLine( reset_iter );
			OBJ_ResetRegisteredClassIterPolys( objclass );

			global_classiter_trafo[ objclass ].X = 0.0f;
			global_classiter_trafo[ objclass ].Y = 0.0f;
			global_classiter_trafo[ objclass ].Z = 0.0f;
			global_classiter_trafo[ objclass ].W = 1.0f;

		} else {
			CON_AddLine( invalid_command );
			return TRUE;
		}

		configset = TRUE;
	}

	// a global trafo state (offset plus optional scale) may be set
	if ( classiter_key_value[ KEY_CLASSITER_TRAFO ].value != NULL ) {

		float trafospec[ 4 ];
		int vecsiz = ScanKeyValueFloatList( &classiter_key_value[ KEY_CLASSITER_TRAFO ], trafospec, 3, 4 );
		if ( vecsiz == 0 ) {
			CON_AddLine( trafo_invalid );
			return TRUE;
		}

		CON_AddLine( trafo_set );
		global_classiter_trafo[ objclass ].X = trafospec[ 0 ];
		global_classiter_trafo[ objclass ].Y = trafospec[ 1 ];
		global_classiter_trafo[ objclass ].Z = trafospec[ 2 ];
		if ( vecsiz == 4 ) {
			global_classiter_trafo[ objclass ].W = trafospec[ 3 ];
		}

		configset = TRUE;
	}

	// early-out if only config set
	if ( configset ) {
		CON_AddLine( params_ignored );
		return TRUE;
	}

	// determine number of vertices per primitive from primitive type (optional)
	int numprimvtxs = 3;
	if ( classiter_key_value[ KEY_CLASSITER_PRIM ].value != NULL ) {

		if ( stricmp( classiter_key_value[ KEY_CLASSITER_PRIM ].value, "tri" ) == 0 ) {
			numprimvtxs = 3;
		} else if ( stricmp( classiter_key_value[ KEY_CLASSITER_PRIM ].value, "quad" ) == 0 ) {
			numprimvtxs = 4;
		} else if ( stricmp( classiter_key_value[ KEY_CLASSITER_PRIM ].value, "pent" ) == 0 ) {
			numprimvtxs = 5;
		} else {
			CON_AddLine( prim_type_invalid );
			return TRUE;
		}
	}

	// determine apex info (optional)
	int apexid   = 0;
	int apexvtx  = 1;
	int apexcomp = 2;

	if ( classiter_key_value[ KEY_CLASSITER_APEX ].value != NULL ) {

		int apexspec[ 3 ];
		int numspec = ScanKeyValueIntList(
			&classiter_key_value[ KEY_CLASSITER_APEX ], apexspec, 1, 3 );
		if ( numspec == 0 ) {
			CON_AddLine( apex_info_invalid );
			return TRUE;
		}

		// for clustering primitives to a single apex
		apexid = apexspec[ 0 ];

		// vertex index that is the apex (in a quad the second one must be successive)
		if ( numspec > 1 ) {
			apexvtx = apexspec[ 1 ];
		}

		// component to modulate with random value (usually z)
		if ( numspec > 2 ) {
			apexcomp = apexspec[ 2 ];
		}
	}

	// read vertex coordinates (mandatory)
	int numvtxs = 0;
	float *vtxlist = NULL;

	if ( classiter_key_value[ KEY_CLASSITER_VTXS ].value != NULL ) {

		vtxlist = (float *) ALLOCMEM( 10 * numprimvtxs * 3 * sizeof( float ) );
		if ( vtxlist == NULL ) {
			OUTOFMEM( 0 );
		}

		int numcoords = ScanKeyValueFloatList( &classiter_key_value[ KEY_CLASSITER_VTXS ],
			vtxlist, numprimvtxs, 10 * numprimvtxs * 3 );
		if ( numcoords == 0 ) {
			CON_AddLine( vtx_list_invalid );
			return TRUE;
		}

		// full vertices must be specified (3 floats)
		numvtxs = numcoords / 3;
		if ( ( numvtxs * 3 ) != numcoords ) {
			CON_AddLine( vtx_list_invalid );
			return TRUE;
		}

		// full primitives must be specified
		int numprims = numvtxs / numprimvtxs;
		if ( ( numprims * numprimvtxs ) != numvtxs ) {
			CON_AddLine( vtx_list_invalid );
			return TRUE;
		}

	} else {
		CON_AddLine( vtx_list_missing );
		return TRUE;
	}

	// register iter polys
	if ( !OBJ_RegisterClassIterPolys( objclass,
		apexid, apexvtx, apexcomp, numprimvtxs, numvtxs, vtxlist ) ) {
		CON_AddLine( iter_reg_failed );
	}

	// free vertex list (has been copied by registration function)
	if ( vtxlist != NULL ) {
		FREEMEM( vtxlist );
		vtxlist = NULL;
	}

	return TRUE;
}


// trafo (offset plus scale) to apply to all class particles of a class -------
//
PRIVATE
Point3h_f global_classpart_trafo[ MAX_DISTINCT_OBJCLASSES ];


// key table for classpart command --------------------------------------------
//
key_value_s classpart_key_value[] = {

	{ "class",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "id",			NULL,	KEYVALFLAG_NONE				},
	{ "cmd",		NULL,	KEYVALFLAG_NONE				},
	{ "trafo",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "pdef",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "pos",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "size",		NULL,	KEYVALFLAG_NONE				},
	{ "render",		NULL,	KEYVALFLAG_NONE				},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_CLASSPART_CLASS,
	KEY_CLASSPART_ID,
	KEY_CLASSPART_CMD,
	KEY_CLASSPART_TRAFO,
	KEY_CLASSPART_PDEF,
	KEY_CLASSPART_POS,
	KEY_CLASSPART_SIZE,
	KEY_CLASSPART_RENDER
};


// console command for object particle registration ("classpart") -----------
//
PRIVATE
int Cmd_CLASSPART( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// classpart_command ::= 'classpart' <class_spec> ( [<command>] [<trafo>] ) |
	//						  ( <pdef> <position> [<size>] [<render_mode>] )
	// class_spec		 ::= 'class' <classname> | 'id' <classid>
	// command			 ::= 'cmd' 'reset'
	// trafo			 ::= 'trafo' '(' <vertex_spec> [<float>] ')'
	// pdef				 ::= 'pdef' <pdef_name>
	// position			 ::= 'pos' '(' <vertex_spec> ')'
	// size				 ::= 'size' <float>
	// render_mode		 ::= 'render' 'poslight' | 'thrust' | 'missile'
	// vertex_spec		 ::= <float> <float> <float>

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( classpart_key_value, paramstr ) )
		return TRUE;

	// get object class (either name or id)
	dword objclass = ScanKeyValueObjClass( classpart_key_value, KEY_CLASSPART_CLASS, KEY_CLASSPART_ID );
	if ( objclass == CLASS_ID_INVALID ) {
		return TRUE;
	}

	int configset = FALSE;

	// check for special commands
	if ( classpart_key_value[ KEY_CLASSPART_CMD ].value != NULL ) {

		if ( stricmp( classpart_key_value[ KEY_CLASSPART_CMD ].value, "reset" ) == 0 ) {

			CON_AddLine( reset_part );
			OBJ_ResetRegisteredClassParticles( objclass );

			global_classpart_trafo[ objclass ].X = 0.0f;
			global_classpart_trafo[ objclass ].Y = 0.0f;
			global_classpart_trafo[ objclass ].Z = 0.0f;
			global_classpart_trafo[ objclass ].W = 1.0f;

		} else {
			CON_AddLine( invalid_command );
			return TRUE;
		}

		configset = TRUE;
	}

	// a global trafo state (offset plus optional scale) may be set
	if ( classpart_key_value[ KEY_CLASSPART_TRAFO ].value != NULL ) {

		float trafospec[ 4 ];
		int vecsiz = ScanKeyValueFloatList( &classpart_key_value[ KEY_CLASSPART_TRAFO ], trafospec, 3, 4 );
		if ( vecsiz == 0 ) {
			CON_AddLine( trafo_invalid );
			return TRUE;
		}

		CON_AddLine( trafo_set );
		global_classpart_trafo[ objclass ].X = trafospec[ 0 ];
		global_classpart_trafo[ objclass ].Y = trafospec[ 1 ];
		global_classpart_trafo[ objclass ].Z = trafospec[ 2 ];
		if ( vecsiz == 4 ) {
			global_classpart_trafo[ objclass ].W = trafospec[ 3 ];
		}

		configset = TRUE;
	}

	// early-out if only config set
	if ( configset ) {
		CON_AddLine( params_ignored );
		return TRUE;
	}

	// determine attach position (mandatory)
	Vertex3 position;
	if ( classpart_key_value[ KEY_CLASSPART_POS ].value != NULL ) {

		float posspec[ 3 ];
		if ( !ScanKeyValueFloatList( &classpart_key_value[ KEY_CLASSPART_POS ], posspec, 3, 3 ) ) {
			CON_AddLine( position_invalid );
			return TRUE;
		}

		posspec[ 0 ] += global_classpart_trafo[ objclass ].X;
		posspec[ 1 ] += global_classpart_trafo[ objclass ].Y;
		posspec[ 2 ] += global_classpart_trafo[ objclass ].Z;

		posspec[ 0 ] *= global_classpart_trafo[ objclass ].W;
		posspec[ 1 ] *= global_classpart_trafo[ objclass ].W;
		posspec[ 2 ] *= global_classpart_trafo[ objclass ].W;

		position.X = FLOAT_TO_GEOMV( posspec[ 0 ] );
		position.Y = FLOAT_TO_GEOMV( posspec[ 1 ] );
		position.Z = FLOAT_TO_GEOMV( posspec[ 2 ] );

	} else {

		CON_AddLine( position_missing );
		return TRUE;
	}

	// get name of pdef to use (mandatory)
	char *pdefname = classpart_key_value[ KEY_CLASSPART_PDEF ].value;
	if ( pdefname == NULL ) {
		CON_AddLine( pdef_missing );
		return TRUE;
	}

	// fetch optional size (reference z)
	float refz = 10.0f;
	if ( ScanKeyValueFloat( &classpart_key_value[ KEY_CLASSPART_SIZE ], &refz ) < 0 ) {
		CON_AddLine( refz_invalid );
		return TRUE;
	}

	// determine render mode
	int attmode = PARTICLE_RENDERMODE_THRUST;
	char *rendermode = classpart_key_value[ KEY_CLASSPART_RENDER ].value;
	if ( rendermode != NULL ) {

		if ( stricmp( rendermode, "poslight" ) == 0 ) {
			attmode = PARTICLE_RENDERMODE_POSLIGHT;
		} else if ( stricmp( rendermode, "thrust" ) == 0 ) {
			attmode = PARTICLE_RENDERMODE_THRUST;
		} else if ( stricmp( rendermode, "missile" ) == 0 ) {
			attmode = PARTICLE_RENDERMODE_MISSILE;
		} else {
			CON_AddLine( rendmode_invalid );
			return TRUE;
		}
	}

	// register particle
	if ( !OBJ_RegisterClassParticle( objclass, pdefname, refz, attmode, &position ) ) {
		CON_AddLine( part_reg_failed );
	}

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( CON_GEOM )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "classiter" command
	regcom.command	 = "classiter";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_CLASSITER;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "classpart" command
	regcom.command	 = "classpart";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_CLASSPART;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// reset offset and scale for class iter polys and particles
	for ( int objclass = 0; objclass < MAX_DISTINCT_OBJCLASSES; objclass++ ) {

		global_classiter_trafo[ objclass ].X = 0.0f;
		global_classiter_trafo[ objclass ].Y = 0.0f;
		global_classiter_trafo[ objclass ].Z = 0.0f;
		global_classiter_trafo[ objclass ].W = 1.0f;

		global_classpart_trafo[ objclass ].X = 0.0f;
		global_classpart_trafo[ objclass ].Y = 0.0f;
		global_classpart_trafo[ objclass ].Z = 0.0f;
		global_classpart_trafo[ objclass ].W = 1.0f;
	}
}



