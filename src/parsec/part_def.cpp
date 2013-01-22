/*
 * PARSEC - Particle Definitions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:36 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-2000
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

// particle types
#include "parttype.h"

// local module header
#include "part_def.h"

// proprietary module headers
#include "con_arg.h"
#include "con_com.h"
#include "con_main.h"
#include "obj_ctrl.h"
#include "part_api.h"

//FIXME: [CLEMENS] split this into game and engine modules

// generic string paste area
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// names of available particle definitions ------------------------------------
//
struct pdefnames_s {

	const char *pdefname;
	pdef_s	*(*pdeffunction)();
};

pdefnames_s s_pdefnames[] = {

	{ "plight01",	PDEF_plight01	},
	{ "psmoke01",	PDEF_psmoke01	},
	{ "psmoke02",	PDEF_psmoke02	},
	{ "pflare01",	PDEF_pflare01	},
	{ "pflare02",	PDEF_pflare02	},
	{ "pflare03",	PDEF_pflare03	},
	{ "pflare04",	PDEF_pflare04	},
	{ "pflare05",	PDEF_pflare05	},
	{ "pflare06",	PDEF_pflare06	},
	{ "stargate",	PDEF_stargate	},
	{ "explode1",	PDEF_explode1	},
	{ "expwave1",	PDEF_expwave1	},
	{ "misglow1",	PDEF_misglow1	},
	{ "extragen",	PDEF_extragen	},

	{ NULL,			NULL			}
};

// make table substitution possible
pdefnames_s *pdefnames = s_pdefnames;

// macros to retrieve names internally
#define PLIGHT01	pdefnames[0].pdefname
#define PSMOKE01	pdefnames[1].pdefname
#define PSMOKE02	pdefnames[2].pdefname
#define PFLARE01	pdefnames[3].pdefname
#define PFLARE02	pdefnames[4].pdefname
#define PFLARE03	pdefnames[5].pdefname
#define PFLARE04	pdefnames[6].pdefname
#define PFLARE05	pdefnames[7].pdefname
#define PFLARE06	pdefnames[8].pdefname
#define STARGATE	pdefnames[9].pdefname
#define EXPLODE1	pdefnames[10].pdefname
#define EXPWAVE1	pdefnames[11].pdefname
#define MISGLOW1	pdefnames[12].pdefname
#define EXTRAGEN	pdefnames[13].pdefname


// fetch particle definition by name ------------------------------------------
//
pdef_s *PDEF_FetchByName( char *name )
{
	//NOTE:
	// this function should not be confused with
	// PRT_AcquireParticleDefinition(). the API function
	// is only able to return already registered pdefs,
	// whereas this function will register the pdef
	// of the specified name if not already done.

	// look up name of particle definition and
	// call corresponding definition function
	for ( pdefnames_s *scan = pdefnames; scan->pdefname; scan++ )
		if ( strcmp( name, scan->pdefname ) == 0 )
			return (*scan->pdeffunction)();

	// name not found
	return NULL;
}


// particle animation tables for "plight01" -----------------------------------
//
const char *plight01_texnames[] = {

	"imp_001",
	"imp_003",
	"imp_005",
	"imp_007",
	"imp_009",
	"imp_011",
	"imp_013",
	"imp_015",
	"imp_017",
	"imp_019",
	"imp_021",
	"imp_023",
	"imp_025",
//	"imp_027",
//	"imp_029",
};

int plight01_texframes = CALC_NUM_ARRAY_ENTRIES( plight01_texnames );


// fetch particle definition for "plight01" -----------------------------------
//
pdef_s *PDEF_plight01()
{
	pdef_s *pdef;
	static int pdefid = -1;

	if ( pdefid != -1 ) {
		pdef = PRT_AcquireParticleDefinitionById( pdefid );
	} else {
		// try to acquire pdef by name
		pdef = PRT_AcquireParticleDefinition( PLIGHT01, &pdefid );
		if ( pdef == NULL ) {

			// alloc temporary table mem
			ptexreg_s *texinfo = (ptexreg_s *) ALLOCMEM( sizeof( ptexreg_s ) * plight01_texframes );
			if ( texinfo == NULL )
				return NULL;

			// init temporary table mem
			for ( int fid = 0; fid < plight01_texframes; fid++ ) {
				texinfo[ fid ].deltatime = 30;
				texinfo[ fid ].texname   = (char *) plight01_texnames[ fid ];
			}

			// init registration info
			pdefreg_s pdefreg;
			pdefreg.texinfo		= texinfo;
			pdefreg.textabsize	= plight01_texframes;
			pdefreg.texstart	= 0;
			pdefreg.texrep		= 5;
			pdefreg.texend		= plight01_texframes - 1;
			pdefreg.xfoinfo		= NULL;
			pdefreg.xfotabsize	= 0;

			// register particle definition
			pdef = PRT_RegisterParticleDefinition( PLIGHT01, &pdefreg, FALSE );

			// free temporary table mem
			FREEMEM( texinfo );
		}
	}
	return pdef;
}


// particle animation tables for "psmoke01" -----------------------------------
//
const char *psmoke01_texnames[] = {

	"smoke001",
	"smoke003",
	"smoke005",
	"smoke007",
	"smoke009",
	"smoke011",
	"smoke013",
	"smoke015",
	"smoke017",
	"smoke019",
	"smoke021",
	"smoke023",
};

int psmoke01_texframes = CALC_NUM_ARRAY_ENTRIES( psmoke01_texnames );


// fetch particle definition for "psmoke01" -----------------------------------
//
pdef_s *PDEF_psmoke01()
{
	pdef_s *pdef;
	static int pdefid = -1;

	if ( pdefid != -1 ) {
		pdef = PRT_AcquireParticleDefinitionById( pdefid );
	} else {
		// try to acquire pdef by name
		pdef = PRT_AcquireParticleDefinition( PSMOKE01, &pdefid );
		if ( pdef == NULL ) {
			// register particle definition
			pdef = PRT_RegisterOneShotTexParticle(
				PSMOKE01, psmoke01_texframes, psmoke01_texnames, 20, FALSE );
		}
	}
	return pdef;
}


// particle animation tables for "psmoke02" -----------------------------------
//
//char *psmoke02_texnames[] = {
//};

//int psmoke02_texframes = CALC_NUM_ARRAY_ENTRIES( psmoke02_texnames );


// fetch particle definition for "psmoke02" -----------------------------------
//
pdef_s *PDEF_psmoke02()
{
	pdef_s *pdef;
	static int pdefid = -1;

	if ( pdefid != -1 ) {
		pdef = PRT_AcquireParticleDefinitionById( pdefid );
	} else {
		// try to acquire pdef by name
		pdef = PRT_AcquireParticleDefinition( PSMOKE02, &pdefid );
		if ( pdef == NULL ) {
			// register particle definition
			pdef = PRT_RegisterOneShotTexParticle(
				PSMOKE02, psmoke01_texframes, psmoke01_texnames, 80, FALSE );
		}
	}
	return pdef;
}


// particle animation tables for "pflare01" -----------------------------------
//
const char *pflare01_texnames[] = {

	"eff10001",
//	"eff10003",
//	"eff10005",
//	"eff10007",
//	"eff10009",
//	"eff10011",
//	"eff10013",
//	"eff10015",
//	"eff10017",
//	"eff10019",
//	"eff10021",
//	"eff10023",
//	"eff10025",
//	"eff10027",
//	"eff10029",
};

int pflare01_texframes = CALC_NUM_ARRAY_ENTRIES( pflare01_texnames );


// fetch particle definition for "pflare01" -----------------------------------
//
pdef_s *PDEF_pflare01()
{
	pdef_s *pdef;
	static int pdefid = -1;

	if ( pdefid != -1 ) {
		pdef = PRT_AcquireParticleDefinitionById( pdefid );
	} else {
		// try to acquire pdef by name
		pdef = PRT_AcquireParticleDefinition( PFLARE01, &pdefid );
		if ( pdef == NULL ) {
			// register particle definition
			pdef = PRT_RegisterLoopTexParticle(
				PFLARE01, pflare01_texframes, pflare01_texnames, 30, FALSE );
		}
	}
	return pdef;
}


// particle animation tables for "pflare02" -----------------------------------
//
const char *pflare02_texnames[] = {

	"eff20001",
//	"eff20003",
//	"eff20005",
//	"eff20007",
//	"eff20009",
//	"eff20011",
//	"eff20013",
//	"eff20015",
//	"eff20017",
//	"eff20019",
//	"eff20021",
//	"eff20023",
//	"eff20025",
//	"eff20027",
//	"eff20029",
};

int pflare02_texframes = CALC_NUM_ARRAY_ENTRIES( pflare02_texnames );


// fetch particle definition for "pflare02" -----------------------------------
//
pdef_s *PDEF_pflare02()
{
	pdef_s *pdef;
	static int pdefid = -1;

	if ( pdefid != -1 ) {
		pdef = PRT_AcquireParticleDefinitionById( pdefid );
	} else {
		// try to acquire pdef by name
		pdef = PRT_AcquireParticleDefinition( PFLARE02, &pdefid );
		if ( pdef == NULL ) {
			// register particle definition
			pdef = PRT_RegisterLoopTexParticle(
				PFLARE02, pflare02_texframes, pflare02_texnames, 30, FALSE );
		}
	}
	return pdef;
}


// particle animation tables for "pflare03" -----------------------------------
//
const char *pflare03_texnames[] = {

//	"eff30001",
//	"eff30003",
//	"eff30005",
//	"eff30007",
//	"eff30009",
//	"eff30011",
//	"eff30013",
	"eff30015",
//	"eff30017",
	"eff30019",
//	"eff30021",
//	"eff30023",
//	"eff30025",
//	"eff30027",
	"eff30029",
};

int pflare03_texframes = CALC_NUM_ARRAY_ENTRIES( pflare03_texnames );


// fetch particle definition for "pflare03" -----------------------------------
//
pdef_s *PDEF_pflare03()
{
	pdef_s *pdef;
	static int pdefid = -1;

	if ( pdefid != -1 ) {
		pdef = PRT_AcquireParticleDefinitionById( pdefid );
	} else {
		// try to acquire pdef by name
		pdef = PRT_AcquireParticleDefinition( PFLARE03, &pdefid );
		if ( pdef == NULL ) {
			// register particle definition
			pdef = PRT_RegisterLoopTexParticle(
				PFLARE03, pflare03_texframes, pflare03_texnames, 30, FALSE );
		}
	}
	return pdef;
}


// particle animation tables for "pflare04" -----------------------------------
//
const char *pflare04_texnames[] = {

	"eff40001",
//	"eff40003",
//	"eff40005",
//	"eff40007",
//	"eff40009",
//	"eff40011",
//	"eff40013",
//	"eff40015",
//	"eff40017",
//	"eff40019",
//	"eff40021",
//	"eff40023",
//	"eff40025",
//	"eff40027",
//	"eff40029",
};

int pflare04_texframes = CALC_NUM_ARRAY_ENTRIES( pflare04_texnames );


// fetch particle definition for "pflare04" -----------------------------------
//
pdef_s *PDEF_pflare04()
{
	pdef_s *pdef;
	static int pdefid = -1;

	if ( pdefid != -1 ) {
		pdef = PRT_AcquireParticleDefinitionById( pdefid );
	} else {
		// try to acquire pdef by name
		pdef = PRT_AcquireParticleDefinition( PFLARE04, &pdefid );
		if ( pdef == NULL ) {
			// register particle definition
			pdef = PRT_RegisterLoopTexParticle(
				PFLARE04, pflare04_texframes, pflare04_texnames, 30, FALSE );
		}
	}
	return pdef;
}


// particle animation tables for "pflare05" -----------------------------------
//
const char *pflare05_texnames[] = {

	"eff50001",
//	"eff50003",
//	"eff50005",
//	"eff50007",
//	"eff50009",
//	"eff50011",
//	"eff50013",
//	"eff50015",
//	"eff50017",
//	"eff50019",
//	"eff50021",
//	"eff50023",
//	"eff50025",
//	"eff50027",
//	"eff50029",
};

int pflare05_texframes = CALC_NUM_ARRAY_ENTRIES( pflare05_texnames );


// fetch particle definition for "pflare05" -----------------------------------
//
pdef_s *PDEF_pflare05()
{
	pdef_s *pdef;
	static int pdefid = -1;

	if ( pdefid != -1 ) {
		pdef = PRT_AcquireParticleDefinitionById( pdefid );
	} else {
		// try to acquire pdef by name
		pdef = PRT_AcquireParticleDefinition( PFLARE05, &pdefid );
		if ( pdef == NULL ) {
			// register particle definition
			pdef = PRT_RegisterLoopTexParticle(
				PFLARE05, pflare05_texframes, pflare05_texnames, 30, FALSE );
		}
	}
	return pdef;
}


// particle animation tables for "pflare06" -----------------------------------
//
const char *pflare06_texnames[] = {

	"helix001",
};

int pflare06_texframes = CALC_NUM_ARRAY_ENTRIES( pflare06_texnames );


// fetch particle definition for "pflare06" -----------------------------------
//
pdef_s *PDEF_pflare06()
{
	pdef_s *pdef;
	static int pdefid = -1;

	if ( pdefid != -1 ) {
		pdef = PRT_AcquireParticleDefinitionById( pdefid );
	} else {
		// try to acquire pdef by name
		pdef = PRT_AcquireParticleDefinition( PFLARE06, &pdefid );
		if ( pdef == NULL ) {
			// register particle definition
			pdef = PRT_RegisterLoopTexParticle(
				PFLARE06, pflare06_texframes, pflare06_texnames, 30, FALSE );
		}
	}
	return pdef;
}


// particle animation tables for "stargate" -----------------------------------
//
const char *stargate_texnames[] = {

	"gate_001",
	"gate_003",
	"gate_005",
	"gate_007",
	"gate_009",
	"gate_011",
	"gate_013",
	"gate_015",
	"gate_017",
	"gate_019",
	"gate_021",
	"gate_023",
	"gate_025",
	"gate_027",
	"gate_029",
	"gate_031",
	"gate_033",
	"gate_035",
	"gate_037",
	"gate_039",
	"gate_041",
	"gate_043",
	"gate_045",
	"gate_047",
	"gate_049",
	"gate_051",
	"gate_053",
	"gate_055",
	"gate_057",
	"gate_058",
	"gate_059",
	"gate_060",
	"gate_000",
};

int stargate_texframes = CALC_NUM_ARRAY_ENTRIES( stargate_texnames );


// fetch particle definition for "stargate" -----------------------------------
//
pdef_s *PDEF_stargate()
{
	pdef_s *pdef;
	static int pdefid = -1;

	if ( pdefid != -1 ) {
		pdef = PRT_AcquireParticleDefinitionById( pdefid );
	} else {
		// try to acquire pdef by name
		pdef = PRT_AcquireParticleDefinition( STARGATE, &pdefid );
		if ( pdef == NULL ) {
			// register particle definition
			pdef = PRT_RegisterOneShotTexParticle(
				STARGATE, stargate_texframes, stargate_texnames, 40, FALSE );
		}
	}
	return pdef;
}


// particle animation tables for "explode1" -----------------------------------
//
const char *explode1_texnames[] = {

	"expl_001",
	"expl_002",
	"expl_003",
	"expl_004",
	"expl_005",
	"expl_006",
	"expl_007",
	"expl_008",
	"expl_009",
	"expl_010",
	"expl_011",
	"expl_012",
	"expl_013",
	"expl_014",
	"expl_015",
	"expl_016",
	"expl_017",
	"expl_018",
	"expl_019",
	"expl_020",
	"expl_021",
	"expl_022",
	"expl_023",
	"expl_024",
	"expl_025",
	"expl_026",
	"expl_027",
	"expl_028",
	"expl_029",
	"expl_030",
	"expl_031",
};

int explode1_texframes = CALC_NUM_ARRAY_ENTRIES( explode1_texnames );


// fetch particle definition for "explode1" -----------------------------------
//
pdef_s *PDEF_explode1()
{
	pdef_s *pdef;
	static int pdefid = -1;

	if ( pdefid != -1 ) {
		pdef = PRT_AcquireParticleDefinitionById( pdefid );
	} else {
		// try to acquire pdef by name
		pdef = PRT_AcquireParticleDefinition( EXPLODE1, &pdefid );
		if ( pdef == NULL ) {
			// register particle definition
			pdef = PRT_RegisterOneShotTexParticle(
				EXPLODE1, explode1_texframes, explode1_texnames, 10, FALSE );
		}
	}
	return pdef;
}


// particle animation tables for "expwave1" -----------------------------------
//
const char *expwave1_texnames[] = {

	"swv_001",
	"swv_003",
	"swv_005",
	"swv_007",
	"swv_009",
	"swv_011",
	"swv_013",
	"swv_015",
	"swv_017",
	"swv_019",
	"swv_021",
	"swv_023",
	"swv_025",
};

int expwave1_texframes = CALC_NUM_ARRAY_ENTRIES( expwave1_texnames );


// fetch particle definition for "expwave1" -----------------------------------
//
pdef_s *PDEF_expwave1()
{
	pdef_s *pdef;
	static int pdefid = -1;

	if ( pdefid != -1 ) {
		pdef = PRT_AcquireParticleDefinitionById( pdefid );
	} else {
		// try to acquire pdef by name
		pdef = PRT_AcquireParticleDefinition( EXPWAVE1, &pdefid );
		if ( pdef == NULL ) {
			// register particle definition
			pdef = PRT_RegisterLoopTexParticle(
				EXPWAVE1, expwave1_texframes, expwave1_texnames, 30, FALSE );
		}
	}
	return pdef;
}


// particle animation tables for "misglow1" -----------------------------------
//
const char *misglow1_texnames[] = {

	"glow06",
};

int misglow1_texframes = CALC_NUM_ARRAY_ENTRIES( misglow1_texnames );


// fetch particle definition for "misglow1" -----------------------------------
//
pdef_s *PDEF_misglow1()
{
	pdef_s *pdef;
	static int pdefid = -1;

	if ( pdefid != -1 ) {
		pdef = PRT_AcquireParticleDefinitionById( pdefid );
	} else {
		// try to acquire pdef by name
		pdef = PRT_AcquireParticleDefinition( MISGLOW1, &pdefid );
		if ( pdef == NULL ) {
			// register particle definition
			pdef = PRT_RegisterLoopTexParticle(
				MISGLOW1, misglow1_texframes, misglow1_texnames, 30, FALSE );
		}
	}
	return pdef;
}


// particle animation tables for "extragen" -----------------------------------
//
const char *extragen_texnames[] = {

	"eg01_01a",
	"eg01_02a",
	"eg01_03a",
	"eg01_04a",
	"eg01_05a",
	"eg01_06a",
	"eg01_07a",
	"eg01_08a",
	"eg01_09a",
	"eg01_10a",
	"eg01_11a",
	"eg01_12a",
	"eg01_13a",
	"eg01_14a",
	"eg01_15a",
};

int extragen_texframes = CALC_NUM_ARRAY_ENTRIES( extragen_texnames );


// fetch particle definition for "extragen" -----------------------------------
//
pdef_s *PDEF_extragen()
{
	pdef_s *pdef;
	static int pdefid = -1;

	if ( pdefid != -1 ) {
		pdef = PRT_AcquireParticleDefinitionById( pdefid );
	} else {
		// try to acquire pdef by name
		pdef = PRT_AcquireParticleDefinition( EXTRAGEN, &pdefid );
		if ( pdef == NULL ) {
			// register particle definition
			pdef = PRT_RegisterOneShotTexParticle(
				EXTRAGEN, extragen_texframes, extragen_texnames, 40, FALSE );
		}
	}
	return pdef;
}


// key table for pdef registration via console command ------------------------
//
key_value_s pdef_key_value[] = {

	{ "base",		NULL,	KEYVALFLAG_MANDATORY	},
	{ "len",		NULL,	KEYVALFLAG_MANDATORY	},
	{ "dig",		NULL,	KEYVALFLAG_NONE			},
	{ "tab",		NULL,	KEYVALFLAG_PARENTHESIZE	},
	{ "bgn",		NULL,	KEYVALFLAG_NONE			},
	{ "rep",		NULL,	KEYVALFLAG_NONE			},
	{ "end",		NULL,	KEYVALFLAG_NONE			},
//TODO:
	{ "xfolen",		NULL,	KEYVALFLAG_IGNORE		},
	{ "xfotab",		NULL,	KEYVALFLAG_IGNORE | KEYVALFLAG_PARENTHESIZE },
	{ "xfobgn",		NULL,	KEYVALFLAG_IGNORE		},
	{ "xforep",		NULL,	KEYVALFLAG_IGNORE		},
	{ "xfoend",		NULL,	KEYVALFLAG_IGNORE		},

	{ NULL,			NULL,	KEYVALFLAG_NONE			},
};

enum {

	KEY_PDEF_TEXNAME,
	KEY_PDEF_TEXLEN,
	KEY_PDEF_TEXDIG,
	KEY_PDEF_TEXTAB,
	KEY_PDEF_TEXBGN,
	KEY_PDEF_TEXREP,
	KEY_PDEF_TEXEND,

	KEY_PDEF_XFOLEN,
	KEY_PDEF_XFOTAB,
	KEY_PDEF_XFOBGN,
	KEY_PDEF_XFOREP,
	KEY_PDEF_XFOEND,
};

key_value_s pdef_textab_key_value[] = {

	{ "base",		NULL,	KEYVALFLAG_NONE			},
	{ "stride",		NULL,	KEYVALFLAG_NONE			},
	{ "frm",		NULL,	KEYVALFLAG_PARENTHESIZE	},

	{ "t",			NULL,	KEYVALFLAG_NONE			},
	{ "tl",			NULL,	KEYVALFLAG_PARENTHESIZE	},

	{ NULL,			NULL,	KEYVALFLAG_NONE			},
};

enum {

	KEY_PDEF_TEXTAB_FSTART,
	KEY_PDEF_TEXTAB_FSTRIDE,
	KEY_PDEF_TEXTAB_FLIST,

	KEY_PDEF_TEXTAB_TEQUI,
	KEY_PDEF_TEXTAB_TLIST,
};


// console command for registering new particle definition ("pdef") -----------
//
PRIVATE
int Cmd_PDEF( char *pdefstr )
{
	//NOTE:
	//CONCOM:
	// pdef_command ::= 'pdef' <name> <texanispec> [<xfoanispec>]
	// name         ::= "may be parenthesized () to include whitespace"
	// texanispec   ::= <basename> <tablen> [<basedig>] [<textab>] [<textabctrl>]
	// basename     ::= 'base' <name>
	// tablen       ::= 'len' <int>
	// basedig      ::= 'dig' '1' | '2' | '3' | '4'
	// textab		::= 'tab' '(' <texframespec> <textimespec> ')'
	// texframespec	::= ( ['base' <int>] ['stride' <int>] ) | ['frm' '(' <intlist> ')']
	// textimespec	::= ['t' <int>] | ['tl' '(' <intlist> ')']
	// textabctrl	::= ['bgn' <int>] ['rep' <int>] ['end' <int>]
	// xfoanispec   ::= 'xfolen' <int> ['xfotab' <xfotab>] [<xfotabctrl>]
	// xfotab		::= '('  ')'
	// xfotabctrl	::= ['xfobgn' <int>] ['xforep' <int>] ['xfoend' <int>]
	// intlist		::= [<intlist>] <int>

	ASSERT( pdefstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( pdefstr );

	// create pointer to list of parameters (first is always the name)
	char *pdefname = strtok( pdefstr, " " );
	if ( pdefname == NULL ) {
		CON_AddLine( "name of pdef must be specified." );
		return TRUE;
	}

	// allow name to be parenthesized to include whitespace
	pdefname = GetParenthesizedName( pdefname );
	if ( pdefname == NULL ) {
		CON_AddLine( "pdef name invalid." );
		return TRUE;
	}

	// scan out all values to texture keys
	if ( !ScanKeyValuePairs( pdef_key_value, NULL ) )
		return TRUE;

	// defaults
	int texlen = -1;	// missing
	int texbgn = 0;
	int texrep = 0;
	int texend = -1;	// missing

	int invalid;
	invalid  = ( ScanKeyValueInt( &pdef_key_value[ KEY_PDEF_TEXLEN ], &texlen ) < 0 );
	invalid += ( ScanKeyValueInt( &pdef_key_value[ KEY_PDEF_TEXBGN ], &texbgn ) < 0 );
	invalid += ( ScanKeyValueInt( &pdef_key_value[ KEY_PDEF_TEXREP ], &texrep ) < 0 );
	invalid += ( ScanKeyValueInt( &pdef_key_value[ KEY_PDEF_TEXEND ], &texend ) < 0 );

	if ( invalid || ( texlen < 1 ) ) {
		CON_AddLine( "invalid integer parameter." );
		return TRUE;
	}
	if ( ( texbgn < 0 ) || ( texbgn >= texlen ) || ( texend >= texlen ) ||
		 ( texrep < 0 ) || ( texrep >= texlen ) ) {
		CON_AddLine( "tabctrl range error." );
		return TRUE;
	}
	if ( texend < 0 ) {
		// default if missing
		texend = texlen - 1;
	}

	// fetch base name for texture (enumeration will be appended automatically)
	char *basestr = pdef_key_value[ KEY_PDEF_TEXNAME ].value;
	ASSERT( basestr != NULL );
	int baselen = strlen( basestr );

	// fetch number of name enum digits
	int basedig = 3;
	if ( pdef_key_value[ KEY_PDEF_TEXDIG ].value != NULL ) {
		if ( ScanKeyValueInt( &pdef_key_value[ KEY_PDEF_TEXDIG ], &basedig ) < 0 ) {
			CON_AddLine( "digit spec invalid." );
			return TRUE;
		}
		if ( ( basedig < 1 ) || ( basedig > 4 ) ) {
			CON_AddLine( "only 1, 2, 3, or 4 digits allowed." );
			return TRUE;
		}
	}

	// alloc temporary table mem
	char *tempmem = (char *) ALLOCMEM( texlen * sizeof( ptexreg_s ) +
									   texlen * sizeof( int ) * 2 +
									   texlen * ( baselen + basedig + 1 ) );
	if ( tempmem == NULL )
		return FALSE;
	ptexreg_s *	texinfo = (ptexreg_s *) tempmem;
	int *		tilist1 = (int *) ( tempmem + sizeof( ptexreg_s ) * texlen );
	int *		tilist2 = tilist1 + texlen;
	char *		txnames = (char *) ( tilist2 + texlen );

	// default values if no textab
	int fstart  = 0;
	int fstride = 1;
	int tequi   = 30;
	int *enumlist   = NULL;
	int *tdeltalist = NULL;

	// parse textab if supplied
	char *textabstr = pdef_key_value[ KEY_PDEF_TEXTAB ].value;
	if ( textabstr != NULL ) {

		if ( !ScanKeyValuePairs( pdef_textab_key_value, textabstr ) ) {
			FREEMEM( tempmem );
			return TRUE;
		}

		invalid  = ( ScanKeyValueInt( &pdef_textab_key_value[ KEY_PDEF_TEXTAB_FSTART ],  &fstart  ) < 0 );
		invalid += ( ScanKeyValueInt( &pdef_textab_key_value[ KEY_PDEF_TEXTAB_FSTRIDE ], &fstride ) < 0 );
		invalid += ( ScanKeyValueInt( &pdef_textab_key_value[ KEY_PDEF_TEXTAB_TEQUI ],   &tequi   ) < 0 );

		if ( invalid || ( fstart < 0 ) || ( tequi < 1 ) ) {
			CON_AddLine( "invalid integer parameter." );
			FREEMEM( tempmem );
			return TRUE;
		}
		if ( ( fstart + fstride * ( texlen - 1 ) ) < 0 ) {
			CON_AddLine( "stride invalid." );
			FREEMEM( tempmem );
			return TRUE;
		}

		//NOTE:
		// stride is allowed to be negative in order to make
		// playback of textures in reversed order possible.
		// (without listing all frames explicitly, which, of
		// course, is able to achieve the same result.)

		// check for enumeration list
		if ( pdef_textab_key_value[ KEY_PDEF_TEXTAB_FLIST ].value != NULL ) {
			enumlist = tilist1;
			if ( !ScanKeyValueIntList( &pdef_textab_key_value[ KEY_PDEF_TEXTAB_FLIST ], enumlist, texlen, texlen ) ) {
				FREEMEM( tempmem );
				return TRUE;
			}
		}

		// check for timedelta list
		if ( pdef_textab_key_value[ KEY_PDEF_TEXTAB_TLIST ].value != NULL ) {
			tdeltalist = tilist2;
			if ( !ScanKeyValueIntList( &pdef_textab_key_value[ KEY_PDEF_TEXTAB_TLIST ], tdeltalist, texlen, texlen ) ) {
				FREEMEM( tempmem );
				return TRUE;
			}
		}
	}

	// init temporary table mem
	if ( texlen == 1 ) {

		// use base name as is for single frame
		texinfo[ 0 ].texname   = basestr;
		texinfo[ 0 ].deltatime = tdeltalist ? *tdeltalist : tequi;

	} else {

		// automatically append frame numbers to base name
		for ( int fid = 0; fid < texlen; fid++ ) {

			strcpy( txnames, basestr );

			int frameno;
			if ( enumlist != NULL ) {
				frameno = *enumlist;
				enumlist++;
			} else {
				frameno = fstart;
				fstart += fstride;
			}

			for ( int dpos = basedig - 1; dpos >= 0; dpos-- ) {
				txnames[ baselen + dpos ] = ( frameno % 10 ) + '0';
				frameno /= 10;
			}
			txnames[ baselen + basedig ] = 0;

			texinfo[ fid ].texname   = txnames;
			texinfo[ fid ].deltatime = tdeltalist ? *tdeltalist++ : tequi;

			txnames += baselen + basedig + 1;
		}
	}

	// init registration info
	pdefreg_s pdefreg;
	pdefreg.texinfo		= texinfo;
	pdefreg.textabsize	= texlen;
	pdefreg.texstart	= texbgn;
	pdefreg.texrep		= texrep;
	pdefreg.texend		= texend;
	pdefreg.xfoinfo		= NULL;
	pdefreg.xfotabsize	= 0;

	// do registration
	pdef_s *pdef = PRT_RegisterParticleDefinition( pdefname, &pdefreg, TRUE );

	// free temporary table mem
	FREEMEM( tempmem );

	// pdef registration may fail (e.g., texture name not found)
	if ( pdef == NULL ) {
		CON_AddLine( "could not register pdef." );
	}

	return TRUE;
}


// key table for pattach command ----------------------------------------------
//
key_value_s pattach_key_value[] = {

	{ "ship",		NULL,	KEYVALFLAG_MANDATORY							},
	{ "origin",		NULL,	KEYVALFLAG_MANDATORY | KEYVALFLAG_PARENTHESIZE	},
	{ "size",		NULL,	KEYVALFLAG_NONE									},
	{ "type",		NULL,	KEYVALFLAG_NONE									},

	{ NULL,			NULL,	KEYVALFLAG_NONE									},
};

enum {

	KEY_PATTACH_SHIPID,
	KEY_PATTACH_ORIGIN,
	KEY_PATTACH_REFZ,
	KEY_PATTACH_TYPE,
};


// console command for attaching a particle to a ship ("pattach") -------------
//
PRIVATE
int Cmd_PATTACH( char *attachstr )
{
	//NOTE:
	//CONCOM:
	// pattach_command	::= 'pattach' <pdefname> <shipspec> <origin> [<ref_z>] [<type>]
	// shipspec			::= 'ship' 'local' | 'target'
	// origin			::= 'origin' '(' <float> <float> <float> ')'
	// ref_z			::= 'size' <float>
	// type				::= 'normal' | 'flare' | 'light'

	ASSERT( attachstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( attachstr );

	// create pointer to name of pdef to use
	char *pdefname = strtok( attachstr, " " );
	if ( pdefname == NULL ) {
		CON_AddLine( "name of pdef must be specified." );
		return TRUE;
	}

	// allow name to be parenthesized to include whitespace
	pdefname = GetParenthesizedName( pdefname );
	if ( pdefname == NULL ) {
		CON_AddLine( "pdef name invalid." );
		return TRUE;
	}

	// scan out all values to keys
	if ( !ScanKeyValuePairs( pattach_key_value, NULL ) )
		return TRUE;

	// default is local ship
	ShipObject *baseship = MyShip;

	char *shipspec = pattach_key_value[ KEY_PATTACH_SHIPID ].value;
	if ( shipspec != NULL ) {

		if ( strcmp( shipspec, "target" ) == 0 ) {

			if ( TargetObjNumber != TARGETID_NO_TARGET ) {

				// search for target
				GenObject *scan = FetchFirstShip();
				while ( ( scan != NULL ) && ( scan->HostObjNumber != TargetObjNumber ) )
					scan = scan->NextObj;
				if ( scan == NULL ) {
					ASSERT( 0 );
					CON_AddLine( "target not found." );
					return TRUE;
				}
				baseship = (ShipObject *) scan;

			} else {
				CON_AddLine( "no target selected." );
				return TRUE;
			}

		} else if ( strcmp( shipspec, "local" ) != 0 ) {

			//TODO:
			// allow numerical ship ids.

			CON_AddLine( "shipid invalid." );
			return TRUE;
		}
	}

	// determine origin (mandatory)
	Vertex3 origin;
	if ( pattach_key_value[ KEY_PATTACH_ORIGIN ].value != NULL ) {

		float locspec[ 3 ];
		if ( !ScanKeyValueFloatList( &pattach_key_value[ KEY_PATTACH_ORIGIN ], locspec, 3, 3 ) ) {
			CON_AddLine( "origin invalid." );
			return TRUE;
		}

		origin.X = FLOAT_TO_GEOMV( locspec[ 0 ] );
		origin.Y = FLOAT_TO_GEOMV( locspec[ 1 ] );
		origin.Z = FLOAT_TO_GEOMV( locspec[ 2 ] );
	}

	// fetch pdef
	pdef_s *pdef = PRT_AcquireParticleDefinition( pdefname, NULL );
	if ( pdef == NULL ) {
		CON_AddLine( "pdef invalid." );
		return TRUE;
	}

	// determine size
	float ref_z = 10.0f;
	if ( pattach_key_value[ KEY_PATTACH_REFZ ].value != NULL ) {
		if ( ScanKeyValueFloat( &pattach_key_value[ KEY_PATTACH_REFZ ], &ref_z ) < 0 ) {
			CON_AddLine( "refz invalid." );
			return TRUE;
		}
	}

	// determine particle type
	dword rendflags = PART_REND_NONE;
	char *typestr = pattach_key_value[ KEY_PATTACH_TYPE ].value;
	if ( typestr != NULL ) {
		if ( strcmp( typestr, "flare" ) == 0 ) {
			rendflags = PART_REND_POINTVIS | PART_REND_NODEPTHCMP | PART_REND_NODEPTHSCALE;
		} else if ( strcmp( typestr, "light" ) == 0 ) {
			rendflags = PART_REND_POINTVIS | PART_REND_NODEPTHCMP;
		} else if ( strcmp( typestr, "normal" ) != 0 ) {
			CON_AddLine( "type invalid." );
			return TRUE;
		}
	}

	// basic attributes
	int bitmap = ( iter_texrgba | iter_specularadd ) | rendflags;
	int color  = 0;

	// create pextinfo
	pextinfo_s extinfo;
	PRT_InitParticleExtInfo( &extinfo, pdef, NULL, NULL );

	particle_s particle;
	PRT_InitParticle( particle, bitmap, color, PRT_NO_SIZEBOUND,
				  	  ref_z, &origin, NULL, INFINITE_LIFETIME,
					  LocalPlayerId, //TODO: GetObjectOwner( shippo )
					  &extinfo );

	// attach particle to ship
	if ( PRT_CreateGenObjectParticle( particle, baseship, NULL ) == NULL ) {
		CON_AddLine( "could not create or attach particle." );
	}

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( PART_DEF )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "pdef" command
	regcom.command	 = "pdef";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_PDEF;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "pattach" command
	regcom.command	 = "pattach";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_PATTACH;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );
}



