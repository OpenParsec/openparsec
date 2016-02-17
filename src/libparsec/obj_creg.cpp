/*
 * PARSEC - Object Class Registration
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:44 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999
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
#include "od_class.h"

// global externals
#include "globals.h"
#ifdef PARSEC_SERVER
	#include "e_world_trans.h"
#endif // PARSEC_SERVER

// local module header
#include "obj_creg.h"

// proprietary module headers
#include "con_arg.h"
#ifdef PARSEC_SERVER
	#include "con_com_sv.h"
	#include "con_main_sv.h"
#else // !PARSEC_SERVER
	#include "con_com.h"
	#include "con_main.h"
	#include "e_supp.h"
#endif // !PARSEC_SERVER
#include "obj_clas.h"



// string constants -----------------------------------------------------------
//
static char intlist_invalid[]		= "intlist invalid.";
static char no_ship_class[]			= "object class is no ship.";
static char too_many_ships[]		= "no more ships allowed.";
static char no_extra_class[]		= "object class is no extra.";
static char too_many_extras[]		= "no more extras allowed.";
#ifdef PARSEC_CLIENT
static char invalid_texture[]		= "invalid texture specified.";
#endif

// class registration entry ---------------------------------------------------
//
struct classregfunc_s {

	classreg_fpt	regfunc;
	classregfunc_s	*next;
};

classregfunc_s *classregfunc_list = NULL;


// register class-registration callback ---------------------------------------
//
int OBJ_RegisterClassRegistration( classreg_fpt regfunc )
{
	ASSERT( regfunc != NULL );

	//NOTE:
	// registration callback functions must be idempotent.
	// (they may be called multiple times, but semantics
	// equivalent to call-once must be guaranteed.)

	classregfunc_s *newregfunc = (classregfunc_s *) ALLOCMEM( sizeof( classregfunc_s ) );
	if ( newregfunc == NULL ) {
		ASSERT( 0 );
		return FALSE;
	}

	// prepend to list
	newregfunc->regfunc	= regfunc;
	newregfunc->next	= classregfunc_list;
	classregfunc_list	= newregfunc;

	return TRUE;
}


// key table for do_creg command ----------------------------------------------
//
key_value_s do_creg_key_value[] = {

	{ "intlist",	NULL,	KEYVALFLAG_PARENTHESIZE		},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_DO_CREG_INTLIST
};


// console command for registering all pending classes ("do_creg") ------------
//
PRIVATE
int Cmd_DO_CREG( char *intliststr )
{
	//NOTE:
	//CONCOM:
	// do_creg_command	::= 'do_creg' [<int_list>]
	// int_list			::= <int> [<int_list>]

	ASSERT( intliststr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( intliststr );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( do_creg_key_value, intliststr ) )
		return TRUE;

	#define MAX_INTLIST_LENGTH 16
	int intlist[ MAX_INTLIST_LENGTH ];
	int numints	= 0;

	// try to read optional int list
	if ( do_creg_key_value[ KEY_DO_CREG_INTLIST ].value != NULL ) {
		numints = ScanKeyValueIntList( &do_creg_key_value[ KEY_DO_CREG_INTLIST ], intlist, 1, MAX_INTLIST_LENGTH );
		if ( numints < 1 ) {
			CON_AddLine( intlist_invalid );
			return TRUE;
		}
	}

	// call all registered functions
	for ( classregfunc_s *scan = classregfunc_list; scan; scan = scan->next ) {
		ASSERT( scan->regfunc != NULL );
		(*scan->regfunc)( numints, intlist );
	}

	return TRUE;
}


// global ship class table ----------------------------------------------------
//
dword ShipClasses[ MAX_SHIP_CLASSES ] = {

	SHIP_CLASS_1,	// shipclassid 00: firebird
	SHIP_CLASS_3,	// shipclassid 01: bluespire
	SHIP_CLASS_2,	// shipclassid 02: cormoran
};

int NumShipClasses = 3; //CALC_NUM_ARRAY_ENTRIES( ShipClasses );


// reverse lookup ship class table --------------------------------------------
//
int ObjClassShipIndex[ MAX_DISTINCT_OBJCLASSES ];


// register a new ship class --------------------------------------------------
//
int OBJ_RegisterShipClass( dword classid )
{
	ASSERT( classid < (dword)NumObjClasses );
	ASSERT( OBJECT_TYPE_SHIP( ObjClasses[ classid ] ) );

	if ( NumShipClasses >= MAX_SHIP_CLASSES ) {
		return FALSE;
	}

	// turn specified class into a ship class
	ShipClasses[ NumShipClasses ]	= classid;
	ObjClassShipIndex[ classid ]	= NumShipClasses;

	NumShipClasses++;

	return TRUE;
}


// key table for shipclass command --------------------------------------------
//
key_value_s shipclass_key_value[] = {

	{ "class",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "id",			NULL,	KEYVALFLAG_NONE				},
	{ "texture",	NULL,	KEYVALFLAG_PARENTHESIZE		},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_SHIPCLASS_CLASS,
	KEY_SHIPCLASS_ID,
	KEY_SHIPCLASS_TEXTURE
};


// console command for registering a new ship class ("shipclass") -------------
//
PRIVATE
int Cmd_SHIPCLASS( char *classstr )
{
	//NOTE:
	//CONCOM:
	// shipclass_command	::= 'shipclass' <class_spec> [<monitor_texture_spec>]
	// class_spec			::= 'class' <classname> | 'id' <classid>
	// monitor_texture_spec ::= 'texture' <texturename>

	ASSERT( classstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( classstr );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( shipclass_key_value, classstr ) )
		return TRUE;

	// get object class (either name or id)
	dword objclass = ScanKeyValueObjClass( shipclass_key_value, KEY_SHIPCLASS_CLASS, KEY_SHIPCLASS_ID );
	if ( objclass == CLASS_ID_INVALID ) {
		return TRUE;
	}

	if ( !OBJECT_TYPE_SHIP( ObjClasses[ objclass ] ) ) {
		CON_AddLine( no_ship_class );
		return TRUE;
	}

	if ( NumShipClasses >= MAX_SHIP_CLASSES ) {
		CON_AddLine( too_many_ships );
		return TRUE;
	}

	// check if class is already registered
	int firstreg = TRUE;
	char *classname = shipclass_key_value[ KEY_SHIPCLASS_CLASS ].value;
	ASSERT( ( classname == NULL ) || ( stricmp( classname, ObjectInfo[ objclass ].name ) == 0 ) );
	classname = ObjectInfo[ objclass ].name;

	for ( int sid = 0; sid < NumShipClasses; sid++ ) {
		if ( strcmp( classname, ObjectInfo[ ShipClasses[ sid ] ].name ) == 0 ) {
			firstreg = FALSE;
			break;
		}
	}

	// turn specified class into a ship class
	if ( firstreg ) {
		ShipClasses[ NumShipClasses ] = objclass;
		ObjClassShipIndex[ objclass ] = NumShipClasses;
		NumShipClasses++;
	}

#ifdef PARSEC_CLIENT

	// check for shipmonitor texture name and save
	// texturemap pointer for use in H_COCKPT.C
	char *texturename = shipclass_key_value[ KEY_SHIPCLASS_TEXTURE ].value;
	if ( texturename != NULL ) {
		TextureMap *texmap = FetchTextureMap( texturename );
		if ( texmap != NULL ) {
			extern TextureMap *MonitorTextures[];
			MonitorTextures[ ObjClassShipIndex[ objclass ] ] = texmap;
		} else {
			CON_AddLine( invalid_texture );
			return TRUE;
		}
	}

#endif // PARSEC_CLIENT

	return TRUE;
}


// global extra class table ---------------------------------------------------
//
dword ExtraClasses[ MAX_EXTRA_CLASSES ] = {

	ENERGY_EXTRA_CLASS,			// extra classindex 00: energy boost
	DUMB_PACK_CLASS,			// extra classindex 01: missile pack (dumb missiles)
	GUIDE_PACK_CLASS,			// extra classindex 02: missile pack (guided missiles)
	HELIX_DEVICE_CLASS,			// extra classindex 03: dazzling laser/helix cannon
	LIGHTNING_DEVICE_CLASS,		// extra classindex 04: thief laser/lightning device
	MINE_PACK_CLASS,			// extra classindex 05: mine pack (proximity mines)
	REPAIR_EXTRA_CLASS,			// extra classindex 06: repair damage
	AFTERBURNER_DEVICE_CLASS,	// extra classindex 07: afterburner
	SWARM_PACK_CLASS,			// extra classindex 08: missile pack (swarm missiles)
	INVISIBILITY_CLASS,			// extra classindex 09: invisibility (not used yet)
	PHOTON_DEVICE_CLASS,		// extra classindex 10: photon cannon
	DECOY_DEVICE_CLASS,			// extra classindex 11: decoy device
	LASERUPGRADE1_CLASS,		// extra classindex 12: laser upgrade 1
	LASERUPGRADE2_CLASS,		// extra classindex 13: laser upgrade 2
	INVULNERABILITY_CLASS,		// extra classindex 14: invulnerability
	MINE_CLASS_1,				// extra classindex 15: mine
	CLASS_ID_INVALID,			// extra classindex 16: emp upgrade 1
	CLASS_ID_INVALID,			// extra classindex 17: emp upgrade 2
};

int NumExtraClasses = 16; //CALC_NUM_ARRAY_ENTRIES( ExtraClasses );


// reverse lookup extra class table -------------------------------------------
//
int ObjClassExtraIndex[ MAX_DISTINCT_OBJCLASSES ];


// register a new extra class -------------------------------------------------
//
int OBJ_RegisterExtraClass( int index, dword classid )
{
	ASSERT( classid < (dword)NumObjClasses );
	ASSERT( OBJECT_TYPE_EXTRA( ObjClasses[ classid ] ) );

	if ( index < NumExtraClasses ) {

	} else {

		if ( NumExtraClasses >= MAX_EXTRA_CLASSES ) {
			return FALSE;
		}

		// turn specified class into an extra class
		ExtraClasses[ NumExtraClasses ]	= classid;
		ObjClassExtraIndex[ classid ]   = NumExtraClasses;

		NumExtraClasses++;
	}

	return TRUE;
}


// key table for extraclass command -------------------------------------------
//
key_value_s extraclass_key_value[] = {

	{ "class",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "id",			NULL,	KEYVALFLAG_NONE				},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_EXTRACLASS_CLASS,
	KEY_EXTRACLASS_ID
};


// console command for registering a new extra class ("extraclass") -----------
//
PRIVATE
int Cmd_EXTRACLASS( char *classstr )
{
	//NOTE:
	//CONCOM:
	// extraclass_command	::= 'extraclass' <class_spec>
	// class_spec			::= 'class' <classname> | 'id' <classid>

	ASSERT( classstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( classstr );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( extraclass_key_value, classstr ) )
		return TRUE;

	// get object class (either name or id)
	dword objclass = ScanKeyValueObjClass( extraclass_key_value, KEY_EXTRACLASS_CLASS, KEY_EXTRACLASS_ID );
	if ( objclass == CLASS_ID_INVALID ) {
		return TRUE;
	}

	if ( !OBJECT_TYPE_EXTRA( ObjClasses[ objclass ] ) ) {
		CON_AddLine( no_extra_class );
		return TRUE;
	}

	if ( NumExtraClasses >= MAX_EXTRA_CLASSES ) {
		CON_AddLine( too_many_extras );
		return TRUE;
	}

	// turn specified class into an extra class
	OBJ_RegisterExtraClass( NumExtraClasses, objclass );

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( OBJ_CREG )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "do_creg" command
	regcom.command	 = "do_creg";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_DO_CREG;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "shipclass" command
	regcom.command	 = "shipclass";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_SHIPCLASS;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// init reverse lookup of ship classes
	unsigned int cid = 0;
	for ( cid = 0; cid < MAX_DISTINCT_OBJCLASSES; cid++ ) {
		ObjClassShipIndex[ cid ] = SHIPINDEX_NO_SHIP;
		for ( int scid = 0; scid < NumShipClasses; scid++ ) {
			if ( ShipClasses[ scid ] == cid ) {
				ObjClassShipIndex[ cid ] =  scid;
				break;
			}
		}
	}

	// register "extraclass" command
	regcom.command	 = "extraclass";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_EXTRACLASS;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// init reverse lookup of extra classes
	for ( cid = 0; cid < MAX_DISTINCT_OBJCLASSES; cid++ ) {
		ObjClassExtraIndex[ cid ] = EXTRAINDEX_NO_EXTRA;
		for ( unsigned int ecid = 0; ecid < (dword)NumExtraClasses; ecid++ ) {
			if ( ExtraClasses[ ecid ] == cid ) {
				ObjClassExtraIndex[ cid ] =  ecid;
				break;
			}
		}
	}
}



