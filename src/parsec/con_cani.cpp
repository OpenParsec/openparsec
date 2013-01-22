/*
 * PARSEC - Color Animation Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:34 $
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

// mathematics header
#include "utl_math.h"

// local module header
#include "con_cani.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux.h"
#include "con_main.h"
#include "e_colani.h"



// string constants -----------------------------------------------------------
//
static char colanim_name_missing[]	= "name of colanim must be specified.";
static char colanim_name_invalid[]	= "name invalid.";
static char colanim_mode_invalid[]	= "mode invalid.";
static char colanim_len0_invalid[]	= "len invalid.";
static char colanim_len1_invalid[]	= "len2 invalid.";
static char colanim_len1_missing[]	= "len2 missing but src2 specified.";
static char colanim_tab1_missing[]	= "src2 missing but len2 specified.";
static char colanim_comp0_invalid[]	= "comp is invalid (src).";
static char colanim_comp1_invalid[]	= "comp is invalid (src2).";
static char colanim_t0_invalid[]	= "tequi is invalid (src).";
static char colanim_t1_invalid[]	= "tequi is invalid (src2).";
static char colanim_tl0_invalid[]	= "tlist is invalid (src).";
static char colanim_tl1_invalid[]	= "tlist is invalid (src2).";
static char colanim_w0_invalid[]	= "invalid wave spec (src).";
static char colanim_w1_invalid[]	= "invalid wave spec (src2).";
static char colanim_min0_invalid[]	= "invalid min value (src).";
static char colanim_min1_invalid[]	= "invalid min value (src2).";
static char colanim_max0_invalid[]	= "invalid max value (src).";
static char colanim_max1_invalid[]	= "invalid max value (src2).";
static char colanim_mask0_invalid[]	= "invalid channel mask (src).";
static char colanim_mask1_invalid[]	= "invalid channel mask (src2).";
static char colanim_reg_failed[]	= "colanim registration failed.";


// key tables for colanim registration via console command --------------------
//
key_value_s colanim_key_value[] = {

	{ "src",		NULL,	KEYVALFLAG_PARENTHESIZE	| KEYVALFLAG_MANDATORY	},
	{ "len",		NULL,	KEYVALFLAG_NONE			| KEYVALFLAG_MANDATORY	},
	{ "src2",		NULL,	KEYVALFLAG_PARENTHESIZE							},
	{ "len2",		NULL,	KEYVALFLAG_NONE									},
	{ "mode",		NULL,	KEYVALFLAG_NONE									},

	{ NULL,			NULL,	KEYVALFLAG_NONE									},
};

enum {

	KEY_COLANI_SRC0,
	KEY_COLANI_LEN0,
	KEY_COLANI_SRC1,
	KEY_COLANI_LEN1,
	KEY_COLANI_MODE,
};


key_value_s colanim_tab_key_value[] = {

	{ "tab",		NULL,	KEYVALFLAG_PARENTHESIZE	| KEYVALFLAG_MANDATORY	},
	{ "comp",		NULL,	KEYVALFLAG_NONE									},
	{ "t",			NULL,	KEYVALFLAG_NONE									},
	{ "tl",			NULL,	KEYVALFLAG_PARENTHESIZE							},
	{ "mask",		NULL,	KEYVALFLAG_NONE									},

	{ NULL,			NULL,	KEYVALFLAG_NONE									},
};

enum {

	KEY_COLANI_TAB_TAB,
	KEY_COLANI_TAB_COMP,
	KEY_COLANI_TAB_TEQUI,
	KEY_COLANI_TAB_TLIST,
	KEY_COLANI_TAB_MASK,
};


key_value_s colanim_wav_key_value[] = {

	{ "wave",		NULL,	KEYVALFLAG_MANDATORY	},
	{ "t",			NULL,	KEYVALFLAG_NONE			},
	{ "max",		NULL,	KEYVALFLAG_NONE			},
	{ "min",		NULL,	KEYVALFLAG_NONE			},
	{ "mask",		NULL,	KEYVALFLAG_NONE			},

	{ NULL,			NULL,	KEYVALFLAG_NONE			},
};

enum {

	KEY_COLANI_WAV_WAVE,
	KEY_COLANI_WAV_TEQUI,
	KEY_COLANI_WAV_MAX,
	KEY_COLANI_WAV_MIN,
	KEY_COLANI_WAV_MASK,
};

static char colanim_wave_separator[]	= "wave ";
static int  colanim_wave_seplen			= strlen( colanim_wave_separator );


// waveform specs -------------------------------------------------------------
//
static char colanim_wave_tri[]	= "tri";
static char colanim_wave_saw[]	= "saw";
static char colanim_wave_ramp[]	= "ramp";
static char colanim_wave_sine[]	= "sine";


// store color and timing info into colanim entry -----------------------------
//
#define SET_COLTAB_ENTRY(p,v,t)	{ \
	coltab[ p ].color.R = ( mask & 0x01 ) ? (byte)(v) : 0x00; \
	coltab[ p ].color.G = ( mask & 0x02 ) ? (byte)(v) : 0x00; \
	coltab[ p ].color.B = ( mask & 0x04 ) ? (byte)(v) : 0x00; \
	coltab[ p ].color.A = ( mask & 0x08 ) ? (byte)(v) : 0x00; \
	coltab[ p ].deltatime = (t); \
}


// check and parse color channel mask -----------------------------------------
//
PRIVATE
int CheckChannelMask( char *maskstr, dword *mask )
{
	ASSERT( mask != NULL );

	if ( maskstr != NULL ) {

		*mask = 0x00;

		if ( maskstr[ 0 ] == 'r' ) {
			*mask |= 0x01;
			maskstr++;
		}
		if ( maskstr[ 0 ] == 'g' ) {
			*mask |= 0x02;
			maskstr++;
		}
		if ( maskstr[ 0 ] == 'b' ) {
			*mask |= 0x04;
			maskstr++;
		}
		if ( maskstr[ 0 ] == 'a' ) {
			*mask |= 0x08;
			maskstr++;
		}
		if ( ( maskstr[ 0 ] != ' ' ) && ( maskstr[ 0 ] != 0 ) ) {
			return FALSE;
		}
	}

	return TRUE;
}


// check for waveform specs ---------------------------------------------------
//
PRIVATE
int CheckWaveSpec( char *tabstr, colfrm_s *coltab, size_t tabsize, int tid )
{
	ASSERT( tabstr != NULL );
	ASSERT( coltab != NULL );
	ASSERT( ( tid == 0 ) || ( tid == 1 ) );

	// scan out all values to colanim_wav keys
	if ( !ScanKeyValuePairs( colanim_wav_key_value, tabstr ) ) {
		return FALSE;
	}

	// timing
	int tequi = 200;
	if ( colanim_wav_key_value[ KEY_COLANI_WAV_TEQUI ].value != NULL ) {
		if ( ScanKeyValueInt( &colanim_wav_key_value[ KEY_COLANI_WAV_TEQUI ], &tequi ) < 0 ) {
			CON_AddLine( tid ? colanim_t1_invalid : colanim_t0_invalid );
			return FALSE;
		}
		if ( tequi < 1 ) {
			CON_AddLine( tid ? colanim_t1_invalid : colanim_t0_invalid );
			return FALSE;
		}
	}

	// lower bound
	int minval = 0;
	if ( colanim_wav_key_value[ KEY_COLANI_WAV_MIN ].value != NULL ) {
		if ( ScanKeyValueInt( &colanim_wav_key_value[ KEY_COLANI_WAV_MIN ], &minval ) < 0 ) {
			CON_AddLine( tid ? colanim_min1_invalid : colanim_min0_invalid );
			return FALSE;
		}
		if ( ( minval < 0 ) || ( minval > 255 ) ) {
			CON_AddLine( tid ? colanim_min1_invalid : colanim_min0_invalid );
			return FALSE;
		}
	}

	// upper bound
	int maxval = 255;
	if ( colanim_wav_key_value[ KEY_COLANI_WAV_MAX ].value != NULL ) {
		if ( ScanKeyValueInt( &colanim_wav_key_value[ KEY_COLANI_WAV_MAX ], &maxval ) < 0 ) {
			CON_AddLine( tid ? colanim_max1_invalid : colanim_max0_invalid );
			return FALSE;
		}
		if ( ( maxval < minval ) || ( maxval > 255 ) ) {
			CON_AddLine( tid ? colanim_max1_invalid : colanim_max0_invalid );
			return FALSE;
		}
	}

	// check for color channel mask
	dword mask = 0x0f;
	if ( !CheckChannelMask( colanim_wav_key_value[ KEY_COLANI_WAV_MASK ].value, &mask ) ) {
		CON_AddLine( tid ? colanim_mask1_invalid : colanim_mask0_invalid );
		return FALSE;
	}

	// generate specified waveform
	char *wave = colanim_wav_key_value[ KEY_COLANI_WAV_WAVE ].value;

	if ( strcmp( wave, colanim_wave_tri ) == 0 ) {

		// triangle: "/\"
		if ( tabsize < 2 ) {
			SET_COLTAB_ENTRY( 0, minval, tequi );
			return TRUE;
		}
		unsigned int     halft  = tabsize / 2;
		float step   = (float)( maxval - minval ) / (float)halft;
		float curval = (float)minval;
		unsigned int tpos = 0;
		for ( tpos = 0; tpos < halft; tpos++ ) {
			SET_COLTAB_ENTRY( tpos, curval, tequi );
			curval += step;
		}
		for ( ; tpos < tabsize; tpos++ ) {
			SET_COLTAB_ENTRY( tpos, curval, tequi );
			curval -= step;
		}

	} else if ( strcmp( wave, colanim_wave_saw ) == 0 ) {

		// sawtooth: "|\"
		if ( tabsize < 2 ) {
			SET_COLTAB_ENTRY( 0, minval, tequi );
			return TRUE;
		}
		float step   = (float)( maxval - minval ) / (float)( tabsize - 1 );
		float curval = (float)maxval;
		for ( unsigned int tpos = 0; tpos < tabsize; tpos++ ) {
			SET_COLTAB_ENTRY( tpos, curval, tequi );
			curval -= step;
		}

	} else if ( strcmp( wave, colanim_wave_ramp ) == 0 ) {

		// ramp: "/|"
		if ( tabsize < 2 ) {
			SET_COLTAB_ENTRY( 0, minval, tequi );
			return TRUE;
		}
		float step   = (float)( maxval - minval ) / (float)( tabsize - 1 );
		float curval = (float)minval;
		for ( unsigned int tpos = 0; tpos < tabsize; tpos++ ) {
			SET_COLTAB_ENTRY( tpos, curval, tequi );
			curval += step;
		}

	} else if ( strcmp( wave, colanim_wave_sine ) == 0 ) {

		// sine
		float step   = (float)BAMS_DEG360 / (float)tabsize;
		float base   = (float)( maxval + minval ) / 2;
		float ampl   = (float)( maxval - minval ) / 2;
		float curval = (float)BAMS_DEG0;
		for ( unsigned int tpos = 0; tpos < tabsize; tpos++ ) {
			sincosval_s sincos;
			GetSinCos( (dword)curval, &sincos );
			float cursig = ampl * GEOMV_TO_FLOAT( sincos.sinval );
			SET_COLTAB_ENTRY( tpos, base + cursig, tequi );
			curval += step;
		}

	} else {

		CON_AddLine( tid ? colanim_w1_invalid : colanim_w0_invalid );
		return FALSE;
	}

	return TRUE;
}


// parse entire color table of 1, 3, or 4 components --------------------------
//
PRIVATE
int ParseColorTable( char *tabstr, colfrm_s *coltab, int *intlist, size_t tabsize, int tid )
{
	ASSERT( tabstr != NULL );
	ASSERT( coltab != NULL );
	ASSERT( intlist != NULL );
	ASSERT( ( tid == 0 ) || ( tid == 1 ) );

	// check for wave spec instead of table
	while ( *tabstr == ' ' )
		tabstr++;
	if ( strncmp( tabstr, colanim_wave_separator, colanim_wave_seplen ) == 0 ) {
		return CheckWaveSpec( tabstr, coltab, tabsize, tid );
	}

	// scan out all values to colanim_tab keys
	if ( !ScanKeyValuePairs( colanim_tab_key_value, tabstr ) ) {
		return FALSE;
	}

	// number of color components
	int comp = 1;
	if ( ScanKeyValueInt( &colanim_tab_key_value[ KEY_COLANI_TAB_COMP ], &comp ) < 0 ) {
		CON_AddLine( tid ? colanim_comp1_invalid : colanim_comp0_invalid );
		return FALSE;
	}
	if ( ( comp != 1 ) && ( comp != 3 ) && ( comp != 4 ) ) {
		CON_AddLine( tid ? colanim_comp1_invalid : colanim_comp0_invalid );
		return FALSE;
	}

	// check for color channel mask
	dword mask = 0x0f;
	if ( !CheckChannelMask( colanim_tab_key_value[ KEY_COLANI_TAB_MASK ].value, &mask ) ) {
		CON_AddLine( tid ? colanim_mask1_invalid : colanim_mask0_invalid );
		return FALSE;
	}

	// timing
	int tequi = 200;
	int tlist = FALSE;
	if ( colanim_tab_key_value[ KEY_COLANI_TAB_TEQUI ].value != NULL ) {

		if ( ScanKeyValueInt( &colanim_tab_key_value[ KEY_COLANI_TAB_TEQUI ], &tequi ) < 0 ) {
			CON_AddLine( tid ? colanim_t1_invalid : colanim_t0_invalid );
			return FALSE;
		}
		if ( tequi < 1 ) {
			CON_AddLine( tid ? colanim_t1_invalid : colanim_t0_invalid );
			return FALSE;
		}

	} else if ( colanim_tab_key_value[ KEY_COLANI_TAB_TLIST ].value != NULL ) {

		if ( ScanKeyValueIntList( &colanim_tab_key_value[ KEY_COLANI_TAB_TLIST ],
			 intlist, tabsize, tabsize ) == 0 ) {
			CON_AddLine( tid ? colanim_tl1_invalid : colanim_tl0_invalid );
			return FALSE;
		}
		tlist = TRUE;
	}
	if ( tlist ) {
		for ( unsigned int eid = 0; eid < tabsize; eid++ ) {
			if ( intlist[ eid ] < 1 ) {
				CON_AddLine( tid ? colanim_t1_invalid : colanim_t0_invalid );
				return FALSE;
			}
			coltab[ eid ].deltatime = intlist[ eid ];
		}
	} else {
		for ( unsigned int eid = 0; eid < tabsize; eid++ ) {
			coltab[ eid ].deltatime = tequi;
		}
	}

	// color components
	size_t complistlen = comp * tabsize;
	if ( ScanKeyValueIntList( &colanim_tab_key_value[ KEY_COLANI_TAB_TAB ],
		 intlist, complistlen, complistlen ) == 0 ) {
		CON_AddLine( tid ? colanim_tl1_invalid : colanim_tl0_invalid );
		return FALSE;
	}
	int *complist = intlist;
	for ( unsigned int eid = 0; eid < tabsize; eid++, complist += comp ) {

		switch ( comp ) {

			case 1:
				coltab[ eid ].color.R = ( mask & 0x01 ) ? complist[ 0 ] : 0x00;
				coltab[ eid ].color.G = ( mask & 0x02 ) ? complist[ 0 ] : 0x00;
				coltab[ eid ].color.B = ( mask & 0x04 ) ? complist[ 0 ] : 0x00;
				coltab[ eid ].color.A = ( mask & 0x08 ) ? complist[ 0 ] : 0x00;
				break;

			case 3:
				coltab[ eid ].color.R = ( mask & 0x01 ) ? complist[ 0 ] : 0x00;
				coltab[ eid ].color.G = ( mask & 0x02 ) ? complist[ 1 ] : 0x00;
				coltab[ eid ].color.B = ( mask & 0x04 ) ? complist[ 2 ] : 0x00;
				coltab[ eid ].color.A = ( mask & 0x08 ) ? 0xff          : 0x00;
				break;

			case 4:
				coltab[ eid ].color.R = ( mask & 0x01 ) ? complist[ 0 ] : 0x00;
				coltab[ eid ].color.G = ( mask & 0x02 ) ? complist[ 1 ] : 0x00;
				coltab[ eid ].color.B = ( mask & 0x04 ) ? complist[ 2 ] : 0x00;
				coltab[ eid ].color.A = ( mask & 0x08 ) ? complist[ 3 ] : 0x00;
				break;
		}
	}

	return TRUE;
}


// possible colanim_s::col_flags modes (COLANIM_SOURCExx) ---------------------
//
static char colanim_mode_sourcenocomb[]	= "nocombine";
static char colanim_mode_sourceadd[]	= "add";
static char colanim_mode_sourcemul[]	= "mul";


// console command for registering new colanim ("colanim") --------------------
//
int Cmd_DefColAnim( char *command )
{
	//NOTE:
	//CONCOM:
	// colanim_command ::= 'colanim' <name> <coltab0> [<coltab1>] [<mode-spec>]
	// name            ::= "may be parenthesized to include whitespace"
	// coltab0         ::= 'src'  <coltab-src> 'len'  <coltab-len>
	// coltab1         ::= 'src2' <coltab-src> 'len2' <coltab-len>
	// coltab_src      ::= '(' <tab-spec> | <wave-spec> ')'
	// coltab_len      ::= <int>
	// mode_spec       ::= 'mode' 'add' | 'mul'
	// tab_spec        ::= 'tab' <col-list> [<comp>] [<mask>] [<t-equi> | <t-list>]
	// col_list        ::= '(' <int>+ ')'
	// comp            ::= 'comp' '1' | '3' | '4'
	// t_equi          ::= 't' <int>
	// t_list          ::= 'tl' '(' <int>+ ')'
	// wave_spec       ::= 'wave' <wave-name> [<t-equi>] [<max>] [<min>] [<mask>]
	// wave_name       ::= 'tri' | 'saw' | 'ramp' | 'sine'
	// max             ::= 'max' <int>
	// min             ::= 'min' <int>
	// mask            ::= 'mask' ['r'] ['g'] ['b'] ['a']

	ASSERT( command != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( command );

	// this flag is used during internal data reloads
	if ( AUX_ALLOW_ONLY_TEXTURE_LOADING ) {
		return TRUE;
	}

	// create pointer to list of parameters (first is always the name)
	char *colanimname = strtok( command, " " );
	if ( colanimname == NULL ) {
		CON_AddLine( colanim_name_missing );
		return TRUE;
	}

	// allow name to be parenthesized to include whitespace
	colanimname = GetParenthesizedName( colanimname );
	if ( colanimname == NULL ) {
		CON_AddLine( colanim_name_invalid );
		return TRUE;
	}

	// scan out all values to colanim keys
	if ( !ScanKeyValuePairs( colanim_key_value, NULL ) )
		return TRUE;

	// fetch length of coltab0
	int len0;
	if ( ScanKeyValueInt( &colanim_key_value[ KEY_COLANI_LEN0 ], &len0 ) < 0 ) {
		CON_AddLine( colanim_len0_invalid );
		return TRUE;
	}
	if ( ( len0 < 1 ) || ( len0 > 256 ) ) {
		CON_AddLine( colanim_len0_invalid );
		return TRUE;
	}

	// fetch length of coltab1
	int len1 = 0;
	if ( colanim_key_value[ KEY_COLANI_LEN1 ].value != NULL ) {
		if ( ScanKeyValueInt( &colanim_key_value[ KEY_COLANI_LEN1 ], &len1 ) < 0 ) {
			CON_AddLine( colanim_len1_invalid );
			return TRUE;
		}
		if ( ( len1 < 1 ) || ( len1 > 256 ) ) {
			CON_AddLine( colanim_len1_invalid );
			return TRUE;
		}
	}

	// check whether both len1 and coltab1 specified or none at all
	if ( colanim_key_value[ KEY_COLANI_SRC1 ].value != NULL ) {
		if ( len1 < 1 ) {
			CON_AddLine( colanim_len1_missing );
			return TRUE;
		}
	} else {
		if ( len1 > 0 ) {
			CON_AddLine( colanim_tab1_missing );
			return TRUE;
		}
	}

	// parse mode if supplied
	dword mode = COLANIM_SOURCEADD;
	if ( colanim_key_value[ KEY_COLANI_MODE ].value != NULL ) {
		char *modestr = colanim_key_value[ KEY_COLANI_MODE ].value;
		if ( strcmp( modestr, colanim_mode_sourceadd ) == 0 ) {
			mode = COLANIM_SOURCEADD;
		} else if ( strcmp( modestr, colanim_mode_sourcemul ) == 0 ) {
			mode = COLANIM_SOURCEMUL;
		} else if ( strcmp( modestr, colanim_mode_sourcenocomb ) == 0 ) {
			mode = COLANIM_SOURCENOCOMBINE;
		} else {
			CON_AddLine( colanim_mode_invalid );
			return TRUE;
		}
	}

	// reserve table memory (both tables in one block)
	size_t tabsize = ( len0 + len1 );
	char *tabmem = (char *) ALLOCMEM( tabsize * sizeof( colfrm_s ) );
	if ( tabmem == NULL )
		OUTOFMEM( 0 );
	colfrm_s *coltab0 = (colfrm_s *) tabmem;
	colfrm_s *coltab1 = ( len1 > 0 ) ? &coltab0[ len0 ] : NULL;

	// need tempmem for color lists (maximum four components per color)
	size_t tempsize = max( len0, len1 ) * 4;
	char *tempmem = (char *) ALLOCMEM( tempsize * sizeof( int ) );
	if ( tempmem == NULL )
		OUTOFMEM( 0 );
	int *intlist = (int *) tempmem;

	// parse coltab0
	char *coltab0str = colanim_key_value[ KEY_COLANI_SRC0 ].value;
	if ( !ParseColorTable( coltab0str, coltab0, intlist, len0, 0 ) ) {
		FREEMEM( tempmem );
		FREEMEM( tabmem );
		return TRUE;
	}

	// parse coltab1 if supplied
	char *coltab1str = colanim_key_value[ KEY_COLANI_SRC1 ].value;
	if ( coltab1str != NULL ) {
		if ( !ParseColorTable( coltab1str, coltab1, intlist, len1, 1 ) ) {
			FREEMEM( tempmem );
			FREEMEM( tabmem );
			return TRUE;
		}
	}

	// init registration info
	colanim_s colanim;
	colanim.col_table0 = coltab0;
	colanim.col_table1 = coltab1;
	colanim.col_end0   = ( len0 - 1 );
	colanim.col_end1   = ( len1 > 0 ) ? ( len1 - 1 ) : 0;
	colanim.col_flags  = mode;

	colanimreg_s colanimreg;
	colanimreg.name    = colanimname;
	colanimreg.colanim = colanim;

	// do registration
	if ( !RegisterColAnim( &colanimreg ) ) {

		CON_AddLine( colanim_reg_failed );
		FREEMEM( tempmem );
		FREEMEM( tabmem );
		return TRUE;
	}

	// free temporary mem
	FREEMEM( tempmem );

	return TRUE;
}



