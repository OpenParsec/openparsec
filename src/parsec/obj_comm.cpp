/*
 * PARSEC - Object Console Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:35 $
 *
 * Orginally written by:
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1999-2000
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

// subsystem headers
#include "net_defs.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "obj_comm.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_main.h"
#include "h_supp.h"
#include "obj_clas.h"
#include "obj_ctrl.h"
#include "obj_cust.h"
#include "obj_xtra.h"
#include "g_sfx.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// string constants -----------------------------------------------------------
//
static char invalid_class[] 		= "invalid object class identifier.";
static char object_spec_needed[]	= "object class must be specified.";
static char sfx_spec_invalid[]		= "invalid sfx name specified.";
static char origin_invalid[]		= "origin invalid.";
static char space_invalid[]			= "transformation space spec invalid.";
static char speed_only_for_ship[]	= "speed only valid for ship object.";
static char invalid_count[]			= "invalid number of objects specified.";
static char offs_invalid[]			= "invalid offset vector specified.";
static char scatter_invalid[]		= "invalid scatter volume specified.";
static char repl_missing[]			= "no replication rule specified.";
static char disabled_in_netgame[]	= "command disabled while connected.";
static char invalid_type[]          = "invalid object type identifier";
static char frame_invalid[]			= "invalid objectframe specified";


// key table for summon command -----------------------------------------------
//
key_value_s summon_key_value[] = {

	{ "class",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "id",			NULL,	KEYVALFLAG_NONE				},
	{ "sfx",		NULL,	KEYVALFLAG_NONE				},
	{ "vtype",		NULL,	KEYVALFLAG_NONE				},
	{ "origin",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "space",		NULL,	KEYVALFLAG_NONE				},
	{ "speed",		NULL,	KEYVALFLAG_NONE				},
	{ "frame",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "count",		NULL,	KEYVALFLAG_NONE				},
	{ "offs",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "scatter",	NULL,	KEYVALFLAG_PARENTHESIZE		},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_SUMMON_CLASS,
	KEY_SUMMON_ID,
	KEY_SUMMON_SFX,
	KEY_SUMMON_VTYPE,
	KEY_SUMMON_ORIGIN,
	KEY_SUMMON_SPACE,
	KEY_SUMMON_SPEED,
	KEY_SUMMON_FRAME,
	KEY_SUMMON_COUNT,
	KEY_SUMMON_OFFS,
	KEY_SUMMON_SCATTER
};


// maximum number of objects that can be summoned at once ---------------------
//
#define MAX_SUMMON_COUNT	256


// summon an object -----------------------------------------------------------
//
dword last_summoned_objectid = 0;


// summon an object -----------------------------------------------------------
//
PRIVATE
int Cmd_SUMMON( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// summon_command	::= 'summon' <object_spec> [<init_spec>] [<multi_spec>]
	// object_spec		::= <classname_spec> | <classid_spec> | <sfx_spec> | <vtype_spec>
	// classname_spec	::= 'class' <classname>
	// classid_spec		::= 'id' <classid>
	// sfx_spec			::= 'sfx' <sfxname>
	// vtype_spec       ::= 'vtype' <typename>
	// init_spec		::= [<origin_spec> [<space_spec>]] [<speed_spec>] [<frame_spec>]
	// origin_spec		::= 'origin' '(' <float> <float> <float> ')'
	// space_spec		::= 'space' 'vv' | 'vw' | 'wv' | 'ww'
	// speed_spec		::= 'speed' <int>
	// frame_spec       ::= 'frame' '(' <float> <float> <float> <float> <float> <float> <float> <float> <float> ')'
	// multi_spec		::= [<count_spec> [<repl_spec>] [<space_spec>]]
	// repl_spec		::= <offset_spec> | <scatter_spec>
	// count_spec 		::= 'count' <int>
	// offset_spec		::=	'offs' '(' <float> <float> <float> ')'
	// scatter_spec		::=	'scatter' '(' <float> <float> <float> ')'
	// classname		::= "may be parenthesized () to include whitespace"
	// classid			::= <int>
	// sfxname			::= 'energyfield'

	//NOTE:
	// - specifying a class name overrides a possibly specified class id.
	// - both a class name or id override a possibly specified sfx name.
	// - if <origin_spec> is omitted, the object will be generated right in
	//   front of the local ship if the origin is view space-relative, or
	//   at the actual world space origin otherwise.
	// - the default for <space_spec> is so that both origin and replication
	//   coordinates are interpreted view space-relative.
	// - the default for <count_spec> is 1.

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	// disable in netgame for cheat protection
	if ( NetConnected == NETWORK_GAME_ON ) {
		CON_AddLine( disabled_in_netgame );
		return TRUE;
	}

	// scan out all values to keys
	if ( !ScanKeyValuePairs( summon_key_value, paramstr ) )
		return TRUE;

	dword objclass = CLASS_ID_INVALID;
	dword objtype  = TYPE_ID_INVALID;

	// if name was specified try to get	class id via name lookup
	char *classname = summon_key_value[ KEY_SUMMON_CLASS ].value;
	if ( classname != NULL ) {
		objclass = OBJ_FetchObjectClassId( classname );
		if ( objclass == CLASS_ID_INVALID ) {
			CON_AddLine( invalid_class );
			return TRUE;
		}
	}

	// if the name was not found, try the id
	if ( objclass == CLASS_ID_INVALID ) {
		if ( ScanKeyValueInt( &summon_key_value[ KEY_SUMMON_ID ], (int*)&objclass ) < 0 ) {
			CON_AddLine( invalid_class );
			return TRUE;
		}
	}

	int sfx_energyfield = FALSE;

	// if neither name nor id found try sfx spec
	if ( objclass == CLASS_ID_INVALID ) {

		char *sfxname = summon_key_value[ KEY_SUMMON_SFX ].value;
		if ( sfxname != NULL ) {

			if ( strcmp( sfxname, "energyfield" ) == 0 ) {
				sfx_energyfield = TRUE;
			} else {
				CON_AddLine( sfx_spec_invalid );
				return TRUE;
			}

		} else {

			// try whether its a virtual type
			char *vtypename = summon_key_value[ KEY_SUMMON_VTYPE ].value;
			if ( vtypename != NULL ) {
				objtype = OBJ_FetchCustomTypeId( vtypename );
				if ( objtype == TYPE_ID_INVALID ) {
					CON_AddLine( invalid_type );
					return TRUE;
				}
				
			} else {
				// if no object spec at all was specified print error msg and exit
				CON_AddLine( object_spec_needed );
				return TRUE;
			}
		}

	} else {

		if ( objclass >= (dword)NumObjClasses ) {
			CON_AddLine( invalid_class );
			return TRUE;
		}
	}

	// default: both origin and replication in view space
	int orgviewrelative = TRUE;
	int repviewrelative = TRUE;

	// get the specified transformation space config (overwrites the default)
	char *spaceconfig = summon_key_value[ KEY_SUMMON_SPACE ].value;
	if ( spaceconfig != NULL ) {

		if ( strcmp( spaceconfig, "vv" ) == 0 ) {
			orgviewrelative = TRUE;
			repviewrelative = TRUE;
		} else if ( strcmp( spaceconfig, "vw" ) == 0 ) {
			orgviewrelative = TRUE;
			repviewrelative = FALSE;
		} else if ( strcmp( spaceconfig, "wv" ) == 0 ) {
			orgviewrelative = FALSE;
			repviewrelative = TRUE;
		} else if ( strcmp( spaceconfig, "ww" ) == 0 ) {
			orgviewrelative = FALSE;
			repviewrelative = FALSE;
		} else {
			CON_AddLine( space_invalid );
			return TRUE;
		}
	}

	// specification space for supplied coordinates
	Xmatrx specspace;
	MakeIdMatrx( specspace );

	// origin for summoned object(s)
	Vertex3 origin;
	if ( orgviewrelative ) {
		// default origin: in front of MyShip
		origin.X = INT_TO_GEOMV( 0 );
		origin.Y = INT_TO_GEOMV( 0 );
		origin.Z = INT_TO_GEOMV( 200 );
	} else {
		// default origin: absolute origin
		origin.X = GEOMV_0;
		origin.Y = GEOMV_0;
		origin.Z = GEOMV_0;
	}

	// get the specified origin (overwrites the default)
	char *originspec = summon_key_value[ KEY_SUMMON_ORIGIN ].value;
	if ( originspec != NULL ) {

		float locspec[ 3 ];
		if ( !ScanKeyValueFloatList( &summon_key_value[ KEY_SUMMON_ORIGIN ], locspec, 3, 3 ) ) {
			CON_AddLine( origin_invalid );
			return TRUE;
		}

		// set specified origin
		origin.X = FLOAT_TO_GEOMV( locspec[ 0 ] );
		origin.Y = FLOAT_TO_GEOMV( locspec[ 1 ] );
		origin.Z = FLOAT_TO_GEOMV( locspec[ 2 ] );
	}

	// check for replication
	int count = 1;
	if ( ScanKeyValueInt( &summon_key_value[ KEY_SUMMON_COUNT ], &count ) == 1 ) {
		if ( count < 1 || count > MAX_SUMMON_COUNT ) {
			CON_AddLine( invalid_count );
			return TRUE;
		}
	}

	// assume linear replication instead of scattering randomly
	int scatter = FALSE;

	Vector3 offset_vec;
	MakeIdVector( offset_vec );

	Vector3 scatter_siz;
	MakeIdVector( scatter_siz );
	
	// get replication specifier if needed
	if ( count > 1 ) {
		
		//NOTE:
		// if the key 'offs' is specified, a linear line of objects will be summoned
		// with the float list value used as offset vector. The summon origin
		// will be the start of this line.
		//
		// if the key 'scatter' is specified, the float list will be used to
		// specify X, Y, and Z size of the volume that will be filled with
		// randomly placed objects. The summon origin will be the center of
		// this volume.
		
		if ( summon_key_value[ KEY_SUMMON_OFFS ].value != NULL ) {
			
			float offs_spec[ 3 ];
			if ( !ScanKeyValueFloatList( &summon_key_value[ KEY_SUMMON_OFFS ], offs_spec, 3, 3 ) ) {
				CON_AddLine( offs_invalid );
				return TRUE;
			}
			
			offset_vec.X = FLOAT_TO_GEOMV( offs_spec[ 0 ] );
			offset_vec.Y = FLOAT_TO_GEOMV( offs_spec[ 1 ] );
			offset_vec.Z = FLOAT_TO_GEOMV( offs_spec[ 2 ] );
			
		} else if ( summon_key_value[ KEY_SUMMON_SCATTER ].value != NULL ) {
			
			float scatter_spec[ 3 ];
			if ( !ScanKeyValueFloatList( &summon_key_value[ KEY_SUMMON_SCATTER ], scatter_spec, 3, 3 ) ) {
				CON_AddLine( scatter_invalid );
				return TRUE;
			}
			
			// make sure negative sizes don't confuse us
			ABS_FLOAT( scatter_spec[ 0 ] );
			ABS_FLOAT( scatter_spec[ 1 ] );
			ABS_FLOAT( scatter_spec[ 2 ] );
			
			scatter_siz.X = FLOAT_TO_GEOMV( scatter_spec[ 0 ] );
			scatter_siz.Y = FLOAT_TO_GEOMV( scatter_spec[ 1 ] );
			scatter_siz.Z = FLOAT_TO_GEOMV( scatter_spec[ 2 ] );
			
			scatter = TRUE;
			
		} else {
			
			CON_AddLine( repl_missing );
			return TRUE;
		}
	}
	
	// determine specification space to world space transform
	Xmatrx spectoworld;
	
	if ( repviewrelative ) {
		memcpy( spectoworld, MyShip->ObjPosition, sizeof( Xmatrx ) );
	} else {
		MakeIdMatrx( spectoworld );
	}
	
	if ( orgviewrelative ) {
		Vector3 torigin;
		MtxVctMUL( MyShip->ObjPosition, &origin, &torigin );
		StoreTVector( spectoworld, &torigin );
	} else {
		StoreTVector( spectoworld, &origin );
	}
	
	// check whether we should consider the object-frame
	if ( summon_key_value[ KEY_SUMMON_FRAME ].value != NULL ) {
		float frame_spec[ 9 ];
		if ( !ScanKeyValueFloatList( &summon_key_value[ KEY_SUMMON_FRAME ], frame_spec, 9, 9 ) ) {
			CON_AddLine( frame_invalid );
			return TRUE;
		}

		pXmatrx mod_matrx = orgviewrelative ? specspace : spectoworld;
		
		// store the x/y/z frame vectors into the matrix
		for( int comp = 0; comp < 9; comp++ ) {
			mod_matrx[ comp % 3 ][ comp / 3 ] = frame_spec[ comp ];
		}
	}

	// summon specified number of objects
	for ( int objnum = 0; objnum < count; objnum++ ) {

		if ( scatter ) {

			int randofsx = 0;
			int randofsy = 0;
			int randofsz = 0;

			if ( scatter_siz.X != 0 )
				randofsx = RAND() % GEOMV_TO_INT( scatter_siz.X );

			if ( scatter_siz.Y != 0 )
				randofsy = RAND() % GEOMV_TO_INT( scatter_siz.Y );

			if ( scatter_siz.Z != 0 )
				randofsz = RAND() % GEOMV_TO_INT( scatter_siz.Z );

			specspace[ 0 ][ 3 ] = randofsx - scatter_siz.X / 2;
			specspace[ 1 ][ 3 ] = randofsy - scatter_siz.Y / 2;
			specspace[ 2 ][ 3 ] = randofsz - scatter_siz.Z / 2;
		}

		// determine summon position in world space
		MtxMtxMUL( spectoworld, specspace, DestXmatrx );

		// summon energyfield or object
		if ( sfx_energyfield ) {

			Vector3 sfxorigin;
			FetchTVector( DestXmatrx, &sfxorigin );
			SFX_CreateEnergyField( sfxorigin );

		} else {

			GenObject *obj = NULL;
			
			if ( objclass != CLASS_ID_INVALID ) {

				// summon a object from a class ( do not show stargate )
				obj = SummonObject( objclass, DestXmatrx );
				if ( obj == NULL ) {
					CON_AddLine( invalid_class );
					return TRUE;
				}
			} else if ( objtype != TYPE_ID_INVALID ) {

				// summon a object from a type ( do not show stargate )
				obj = SummonObjectFromType( objtype, DestXmatrx );
				if ( obj == NULL ) {
					CON_AddLine( invalid_class );
					return TRUE;
				}

			} else {
				ASSERT( FALSE );
			}


			// remember id
			last_summoned_objectid = obj->ObjectNumber;

			// display object id
			if ( !AUX_DISABLE_LEVEL_CONSOLE_MESSAGES ) {
				sprintf( paste_str, "summoned object has instance id %u.", (unsigned int)last_summoned_objectid );
				CON_AddLine( paste_str );
			}

			// check for speed spec
			int speed = -1;
			if ( ScanKeyValueInt( &summon_key_value[ KEY_SUMMON_SPEED ], &speed ) == 1 ) {
				if ( speed > 0 ) {
					if ( OBJECT_TYPE_SHIP( obj ) ) {
						ShipObject *ship = (ShipObject *) obj;
						ship->CurSpeed = speed;
					} else {
						CON_AddLine( speed_only_for_ship );
					}
				}
			}

			// init extra-specific fields
			if ( OBJECT_TYPE_EXTRA( obj ) ) {
				OBJ_FillExtraMemberVars( (ExtraObject *) obj );
			}
		}

		// add offset vector for next object
		if ( !scatter ) {
			specspace[ 0 ][ 3 ] += offset_vec.X;
			specspace[ 1 ][ 3 ] += offset_vec.Y;
			specspace[ 2 ][ 3 ] += offset_vec.Z;
		}
	}

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( OBJ_COMM )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "summon" command
	regcom.command	 = "summon";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_SUMMON;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );
}



