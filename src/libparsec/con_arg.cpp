/*
 * PARSEC - Argument Parsing
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:44 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
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
#include "con_arg.h"

// proprietary module headers
#ifdef PARSEC_SERVER
	#include "con_int_sv.h"
	#include "con_main_sv.h"
#else // !PARSEC_SERVER
	#include "con_int.h"
	#include "con_main.h"
#endif // !PARSEC_SERVER

#include "obj_clas.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// string constants -----------------------------------------------------------
//
static char value_missing[]			= "value missing.";
static char unknown_key[]			= "unknown key.";
static char invalid_value[]			= "invalid value syntax.";
static char disallowed_key[]		= "key not allowed.";
static char int_list_invalid[]		= "int list invalid.";
static char int_list_too_long[]		= "int list too long.";
static char int_list_too_short[]	= "int list too short.";
static char float_list_invalid[]	= "float list invalid.";
static char float_list_too_long[]	= "float list too long.";
static char float_list_too_short[]	= "float list too short.";
static char flag_value_invalid[]	= "invalid flag specified.";
static char invalid_class[] 		= "invalid object class identifier.";
static char object_spec_needed[]	= "object class must be specified.";
static char too_many_strings[]		= "exactly one string must be supplied.";
static char string_arg_too_long[]	= "string too long.";


// scan out all values to specified keys (key/value pairs) --------------------
//
int ScanKeyValuePairs( key_value_s *table, char *kvstr )
{
	ASSERT( table != NULL );

	//NOTE:
	// kvstr may be NULL to continue parsing with
	// strtok( NULL, " " ).

	// reset all values
	int keyno = 0;
	for ( keyno = 0; table[ keyno ].key; keyno++ )
		table[ keyno ].value = NULL;

	// scan all supplied key/value pairs
	char *key;
	while ( ( key = strtok( kvstr, " " ) ) != NULL ) {
		// continue in same string
		kvstr = NULL;
		// check all keys in table
		for ( keyno = 0; table[ keyno ].key; keyno++ ) {
			if ( strcmp( table[ keyno ].key, key ) == 0 ) {
				if ( table[ keyno ].flags & KEYVALFLAG_DISALLOW ) {
					CON_AddLine( disallowed_key );
					return FALSE;
				}
				char *value = strtok( NULL, " " );
				if ( value == NULL ) {
					CON_AddLine( value_missing );
					return FALSE;
				}
				if ( table[ keyno ].flags & KEYVALFLAG_PARENTHESIZE ) {
					value = GetParenthesizedName( value );
					if ( value == NULL ) {
						CON_AddLine( invalid_value );
						return FALSE;
					}
				}
				if ( table[ keyno ].flags & KEYVALFLAG_IGNORE ) {
					sprintf( paste_str, "key %s ignored.", table[ keyno ].key );
					CON_AddLine( paste_str );
				} else {
					table[ keyno ].value = value;
				}
				break;
			}
		}
		if ( table[ keyno ].key == NULL ) {
			CON_AddLine( unknown_key );
			return FALSE;
		}
	}

	// check for mandatory keys
	for ( keyno = 0; table[ keyno ].key; keyno++ ) {
		if ( table[ keyno ].flags & KEYVALFLAG_MANDATORY ) {
			if ( table[ keyno ].value == NULL ) {
				sprintf( paste_str, "mandatory key %s missing.", table[ keyno ].key );
				CON_AddLine( paste_str );
				return FALSE;
			}
		}
	}

	return TRUE;
}


// convert integer value of key/value pair to int -----------------------------
//
int ScanKeyValueInt( key_value_s *keyval, int *ival )
{
	//NOTE:
	// returns -1 if value was invalid.
	// returns  0 if value was missing.
	// returns  1 if value was ok.

	ASSERT( keyval != NULL );
	ASSERT( ival != NULL );

	// missing value alters nothing
	if ( keyval->value == NULL ) {
		return 0;
	}

	char *errpart;
	int32 iparam = (int32) strtol( keyval->value, &errpart, 10 );

	// invalid value returns error
	if ( *errpart != 0 ) {
		return -1;
	}

	*ival = iparam;

	return 1;
}


// convert integer value list of key/value pair to int array ------------------
//
int ScanKeyValueIntList( key_value_s *keyval, int *ilist, int minlen, int maxlen )
{
	ASSERT( keyval != NULL );
	ASSERT( ilist != NULL );
	ASSERT( minlen > 0 );
	ASSERT( minlen <= maxlen );

	char *liststr = keyval->value;
	if ( liststr == NULL ) {
		return 0;
	}

	char *istr;
	int cnt = 0;
	for ( cnt = 0; (istr = strtok( liststr, " " )); cnt++ ) {

		liststr = NULL;

		if ( cnt >= maxlen ) {
			CON_AddLine( int_list_too_long );
			return 0;
		}

		char *errpart;
		int iparam1 = -1;

		// check for range specification like "50-70"
		char *checkstr = istr;
		while ( ( *checkstr >= '0' ) && ( *checkstr <= '9' ) )
			checkstr++;
		if ( ( *checkstr == '-' ) && ( checkstr != istr ) ) {
			iparam1 = (int) strtol( istr, &errpart, 10 );
			if ( *errpart != '-' ) {
				CON_AddLine( int_list_invalid );
				return 0;
			}
			istr = &checkstr[ 1 ];
		}

		int iparam2 = (int) strtol( istr, &errpart, 10 );
		if ( *errpart != 0 ) {
			CON_AddLine( int_list_invalid );
			return 0;
		}

		if ( iparam1 != -1 ) {

			// store specified range
			if ( iparam1 <= iparam2 ) {

				if ( ( cnt + iparam2 - iparam1 ) >= maxlen ) {
					CON_AddLine( int_list_too_long );
					return 0;
				}
				cnt += iparam2 - iparam1;
				for ( ; iparam1 <= iparam2; iparam1++ ) {
					*ilist++ = iparam1;
				}

			} else {

				if ( ( cnt + iparam1 - iparam2 ) >= maxlen ) {
					CON_AddLine( int_list_too_long );
					return 0;
				}
				cnt += iparam1 - iparam2;
				for ( ; iparam1 >= iparam2; iparam1-- ) {
					*ilist++ = iparam1;
				}
			}

		} else {

			// store single value
			*ilist++ = iparam2;
		}
	}

	if ( cnt < minlen ) {
		CON_AddLine( int_list_too_short );
		return 0;
	}

	return cnt;
}


// convert bounded integer value list of key/value pair to int array ----------
//
int ScanKeyValueIntListBounded( key_value_s *keyval, int *ilist, int minlen, int maxlen, int minval, int maxval )
{
	ASSERT( keyval != NULL );
	ASSERT( ilist != NULL );
	ASSERT( minlen > 0 );
	ASSERT( minlen <= maxlen );
	ASSERT( minval <= maxval );

	char *liststr = keyval->value;
	if ( liststr == NULL ) {
		return 0;
	}

	char *istr;
	int cnt = 0;
	for ( cnt = 0; (istr = strtok( liststr, " " )); cnt++ ) {

		liststr = NULL;

		if ( cnt >= maxlen ) {
			CON_AddLine( int_list_too_long );
			return 0;
		}

		char *errpart;
		int iparam1 = -1;

		// check for range specification like "50-70"
		char *checkstr = istr;
		while ( ( *checkstr >= '0' ) && ( *checkstr <= '9' ) )
			checkstr++;
		if ( ( *checkstr == '-' ) && ( checkstr != istr ) ) {
			iparam1 = (int) strtol( istr, &errpart, 10 );
			if ( *errpart != '-' ) {
				CON_AddLine( int_list_invalid );
				return 0;
			}
			istr = &checkstr[ 1 ];
		}

		int iparam2 = (int) strtol( istr, &errpart, 10 );
		if ( *errpart != 0 ) {
			CON_AddLine( int_list_invalid );
			return 0;
		}

		if ( ( iparam2 < minval ) || ( iparam2 > maxval ) ) {
			CON_AddLine( int_list_invalid );
			return 0;
		}

		if ( iparam1 != -1 ) {

			if ( ( iparam1 < minval ) || ( iparam1 > maxval ) ) {
				CON_AddLine( int_list_invalid );
				return 0;
			}

			// store specified range
			if ( iparam1 <= iparam2 ) {

				if ( ( cnt + iparam2 - iparam1 ) >= maxlen ) {
					CON_AddLine( int_list_too_long );
					return 0;
				}
				cnt += iparam2 - iparam1;
				for ( ; iparam1 <= iparam2; iparam1++ ) {
					*ilist++ = iparam1;
				}

			} else {

				if ( ( cnt + iparam1 - iparam2 ) >= maxlen ) {
					CON_AddLine( int_list_too_long );
					return 0;
				}
				cnt += iparam1 - iparam2;
				for ( ; iparam1 >= iparam2; iparam1-- ) {
					*ilist++ = iparam1;
				}
			}

		} else {

			// store single value
			*ilist++ = iparam2;
		}
	}

	if ( cnt < minlen ) {
		CON_AddLine( int_list_too_short );
		return 0;
	}

	return cnt;
}


// convert float value of key/value pair to float -----------------------------
//
int ScanKeyValueFloat( key_value_s *keyval, float *fval )
{
	//NOTE:
	// returns -1 if value was invalid.
	// returns  0 if value was missing.
	// returns  1 if value was ok.

	ASSERT( keyval != NULL );
	ASSERT( fval != NULL );

	// missing value alters nothing
	if ( keyval->value == NULL ) {
		return 0;
	}

	char *errpart;
	double fparam = strtod( keyval->value, &errpart );

	// invalid value returns error
	if ( *errpart != 0 ) {
		return -1;
	}

	*fval = (float)fparam;

	return 1;
}


// convert float value list of key/value pair to float array ------------------
//
int ScanKeyValueFloatList( key_value_s *keyval, float *flist, int minlen, int maxlen )
{
	ASSERT( keyval != NULL );
	ASSERT( flist != NULL );
	ASSERT( minlen > 0 );
	ASSERT( minlen <= maxlen );

	char *liststr = keyval->value;
	if ( liststr == NULL ) {
		return 0;
	}

	char *fstr;
	int cnt = 0;
	for ( cnt = 0; (fstr = strtok( liststr, " " )); cnt++ ) {

		liststr = NULL;

		if ( cnt >= maxlen ) {
			CON_AddLine( float_list_too_long );
			return 0;
		}

		char *errpart;
		double fparam = strtod( fstr, &errpart );
		if ( *errpart != 0 ) {
			CON_AddLine( float_list_invalid );
			return 0;
		}

		*flist++ = (float)fparam;
	}

	if ( cnt < minlen ) {
		CON_AddLine( float_list_too_short );
		return 0;
	}

	return cnt;
}


// convert bounded float value list of key/value pair to float array ----------
//
int ScanKeyValueFloatListBounded( key_value_s *keyval, float *flist, int minlen, int maxlen, float minval, float maxval )
{
	ASSERT( keyval != NULL );
	ASSERT( flist != NULL );
	ASSERT( minlen > 0 );
	ASSERT( minlen <= maxlen );
	ASSERT( minval <= maxval );

	char *liststr = keyval->value;
	if ( liststr == NULL ) {
		return 0;
	}

	char *fstr;
	int cnt = 0;
	for ( cnt = 0; (fstr = strtok( liststr, " " )); cnt++ ) {

		liststr = NULL;

		if ( cnt >= maxlen ) {
			CON_AddLine( float_list_too_long );
			return 0;
		}

		char *errpart;
		double fparam = strtod( fstr, &errpart );
		if ( *errpart != 0 ) {
			CON_AddLine( float_list_invalid );
			return 0;
		}

		if ( ( fparam < minval ) || ( fparam > maxval ) ) {
			CON_AddLine( float_list_invalid );
			return 0;
		}

		*flist++ = (float)fparam;
	}

	if ( cnt < minlen ) {
		CON_AddLine( float_list_too_short );
		return 0;
	}

	return cnt;
}


// convert flag value list of key/value pair to single flagword ---------------
//
int ScanKeyValueFlagList( key_value_s *keyval, dword *flagword, flag_map_s *flagmap )
{
	ASSERT( keyval != NULL );
	ASSERT( flagword != NULL );
	ASSERT( flagmap != NULL );

	char *liststr = keyval->value;
	if ( liststr == NULL ) {
		return 0;
	}

	char *fstr;
	int cnt = 0;
	for ( cnt = 0; (fstr = strtok( liststr, " " )); cnt++ ) {

		liststr = NULL;

		// look for flag spec match
		flag_map_s *map = NULL;
		for ( map = flagmap; map->name != NULL; map++ ) {
			if ( stricmp( fstr, map->name ) == 0 ) {
				break;
			}
		}
		if ( map->name == NULL ) {
			CON_AddLine( flag_value_invalid );
			return 0;
		}

		// set corresponding bit
		*flagword |= map->value;
	}

	return cnt;
}


// convert class spec (either class name or id) of key/value pair to id -------
//
dword ScanKeyValueObjClass( key_value_s *keyval, int keyclass, int keyid )
{
	ASSERT( keyval != NULL );

	dword objclass = CLASS_ID_INVALID;

	// if name was specified try to get	class id via name lookup
	char *classname = keyval[ keyclass ].value;
	if ( classname != NULL ) {
		objclass = OBJ_FetchObjectClassId( classname );
		if ( objclass == CLASS_ID_INVALID ) {
			CON_AddLine( invalid_class );
			return CLASS_ID_INVALID;
		}
	}

	// if the name was not found, try the id
	if ( objclass == CLASS_ID_INVALID ) {
		if ( ScanKeyValueInt( &keyval[ keyid ], (int*)&objclass ) < 0 ) {
			CON_AddLine( invalid_class );
			return CLASS_ID_INVALID;
		}
	}

	// if neither name nor id was specified print error msg
	if ( objclass == CLASS_ID_INVALID ) {
		CON_AddLine( object_spec_needed );
		return CLASS_ID_INVALID;
	}

	if ( objclass >= ( (dword)NumObjClasses ) ) {
		CON_AddLine( invalid_class );
		return CLASS_ID_INVALID;
	}

	return objclass;
}


// parse name that may be parenthesized to allow inclusion of whitespace ------
//
char *GetParenthesizedName( char *name )
{
	ASSERT( name != NULL );

	if ( *name == '(' ) {

		// skip leftmost parenthesis
		name++;

		int opened = 1;
		int closed = 0;

		// concatenate fields
		char *nextfield = name;
		for ( ;; ) {

			// count enclosed parentheses
			for ( ; *nextfield != 0; nextfield++ ) {
				if ( *nextfield == '(' )
					opened++;
				else if ( *nextfield == ')' )
					closed++;
			}
			// check for rightmost parenthesis
			if ( *( nextfield - 1 ) == ')' ) {
				if ( closed == opened ) {
					// strip rightmost parenthesis
					*( nextfield - 1 ) = 0;
					return name;
				}
			}

			// replace '\0' by ' '
			*nextfield = ' ';

			// fetch next field
			nextfield = strtok( NULL, " " );
			if ( nextfield == NULL )
				return NULL;
		}
	}

	return name;
}


// get int parameter from behind a command string -----------------------------
//
int GetIntBehindCommand( char *cstr, int32 *iarg, int base, int paramoptional )
{
	//NOTE:
	// this function returns TRUE only if the int parameter
	// could be parsed correctly. otherwise an error message
	// will be displayed and FALSE returned. this implicates
	// that quiet syntax error detection is not possible.
	// (exception: missing parameter if it is optional. in
	// this case paramoptional will be returned to the caller
	// who must ensure to react correctly.)

	ASSERT( cstr != NULL );
	ASSERT( iarg != NULL );

	// check if there is a space between command and argument
	if ( ( *cstr != ' ' ) && ( *cstr != 0 ) ) {
		CON_AddLine( unknown_command );
		return FALSE;
	}

	// split off first token (int parameter)
	char *istr = strtok( cstr, " " );

	if ( istr == NULL ) {
		if ( !paramoptional )
			CON_AddLine( arg_missing );
		return paramoptional;
	}

	// check if more than one parameter
	if ( strtok( NULL, " " ) != NULL ) {
		CON_AddLine( too_many_args );
		return FALSE;
	}

	// convert int parameter
	char *errpart;
	int32 iparam = (int32) strtol( istr, &errpart, base );

	if ( *errpart != 0 ) {
		CON_AddLine( invalid_arg );
		return FALSE;
	}

	*iarg = iparam;
	return TRUE;
}


// get argument string that has to be separated with at least one space -------
//
const char *GetStringBehindCommand( char *scan, int paramoptional )
{
	//NOTE:
	// this function returns non-NULL only if the string parameter
	// could be parsed correctly. otherwise an error message
	// will be displayed and NULL returned. this implicates
	// that quiet syntax error detection is not possible.
	// (exception: missing parameter if it is optional.)

	//NOTE:
	// if the parameter is optional this function
	// will return an empty string ("").

	ASSERT( scan != NULL );

	// check if there is a space between command and argument
	if ( ( *scan != 0 ) && ( *scan != ' ' ) ) {
		CON_AddLine( unknown_command );
		return NULL;
	}

	// eat whitespace
	while ( *scan == ' ' )
		scan++;

	// no argument found?
	if ( *scan == 0 ) {
		if ( paramoptional ) {
			// return empty string
			return "";
		} else {
			// return error
			CON_AddLine( arg_missing );
			return NULL;
		}
	}

	// cut off trailing whitespace
	char *start = scan;
	while ( *scan != 0 )
		scan++;
	while ( *--scan == ' ' )
		{}
	*( scan + 1 ) = 0;

	// return isolated part (may contain whitespace in the middle!)
	return start;
}


// generic string set function ------------------------------------------------
//
int SetSingleStringCommand( char *cstr, char *dst, int dstmaxlen )
{
	ASSERT( cstr != NULL );
	ASSERT( dst != NULL );
	ASSERT( dstmaxlen > 0 );

	// check if there is a space between command and argument
	if ( ( *cstr != ' ' ) && ( *cstr != 0 ) ) {
		CON_AddLine( unknown_command );
		return FALSE;
	}

	char *name = strtok( cstr, " " );

	if ( name == NULL ) {
		CON_AddLine( dst );
		return FALSE;
	}

	if ( strtok( NULL, " " ) != NULL ) {
		CON_AddLine( too_many_strings );
		return FALSE;
	}

	if ( ( (int)strlen( name ) ) > dstmaxlen ) {
		CON_AddLine( string_arg_too_long );
		return FALSE;
	}

	strcpy( dst, name );
	return TRUE;
}


// query/retrieve single int argument -----------------------------------------
//
char *QueryIntArgument( const char *query, int *arg )
{
	//NOTE:
	// query may be NULL, indicating that the variable
	// may be queried by not supplying any argument.

	ASSERT( arg != NULL );

	char *scan = strtok( NULL, " " );
	if ( scan != NULL ) {
		if ( strtok( NULL, " " ) == NULL ) {
			return scan;
		} else {
			CON_AddLine( too_many_args );
		}
	} else if ( query != NULL ) {
		sprintf( paste_str, query, *arg );
		CON_AddLine( paste_str );
	} else {
		CON_AddLine( arg_missing );
	}

	return NULL;
}


// query/retrieve single int argument not using strtok( NULL, ... ) -----------
//
char *QueryIntArgumentEx( char *params, const char *query, int *arg )
{
	//NOTE:
	// query may be NULL, indicating that the variable
	// may be queried by not supplying any argument.

	ASSERT( params != NULL );
	ASSERT( arg != NULL );

	char *scan = strtok( params, " " );
	if ( scan != NULL ) {
		if ( strtok( NULL, " " ) == NULL ) {
			return scan;
		} else {
			CON_AddLine( too_many_args );
		}
	} else if ( query != NULL ) {
		sprintf( paste_str, query, *arg );
		CON_AddLine( paste_str );
	} else {
		CON_AddLine( arg_missing );
	}

	return NULL;
}


// query/retrieve single float argument ---------------------------------------
//
char *QueryFltArgument( const char *query, float *arg )
{
	//NOTE:
	// query may be NULL, indicating that the variable
	// may be queried by not supplying any argument.

	ASSERT( arg != NULL );

	char *scan = strtok( NULL, " " );
	if ( scan != NULL ) {
		if ( strtok( NULL, " " ) == NULL ) {
			return scan;
		} else {
			CON_AddLine( too_many_args );
		}
	} else if ( query != NULL ) {
		sprintf( paste_str, query, *arg );
		CON_AddLine( paste_str );
	} else {
		CON_AddLine( arg_missing );
	}

	return NULL;
}


// query/retrieve single float argument not using strtok( NULL, ... ) ---------
//
char *QueryFltArgumentEx( char *params, const char *query, float *arg )
{
	//NOTE:
	// query may be NULL, indicating that the variable
	// may be queried by not supplying any argument.

	ASSERT( params != NULL );
	ASSERT( arg != NULL );

	char *scan = strtok( params, " " );
	if ( scan != NULL ) {
		if ( strtok( NULL, " " ) == NULL ) {
			return scan;
		} else {
			CON_AddLine( too_many_args );
		}
	} else if ( query != NULL ) {
		sprintf( paste_str, query, *arg );
		CON_AddLine( paste_str );
	} else {
		CON_AddLine( arg_missing );
	}

	return NULL;
}


// check command that alters an int value -------------------------------------
//
int CheckSetIntArgument( const char *query, const char *scan, const char *command, int *arg )
{
	//NOTE:
	// query may be NULL, indicating that the variable
	// may be queried by not supplying any argument.

	ASSERT( scan != NULL );
	ASSERT( command != NULL );
	ASSERT( arg != NULL );

	if ( strcmp( scan, command ) == 0 ) {
		if ( (scan = QueryIntArgument( query, arg )) ) {
			char *errpart;
			int sval = (int) strtol( scan, &errpart, int_calc_base );
			if ( *errpart == 0 ) {
				*arg = sval;
			} else {
				CON_AddLine( invalid_arg );
			}
		}
		return TRUE;
	} else {
		return FALSE;
	}
}


// check command that alters a float value ------------------------------------
//
int CheckSetFltArgument( const char *query, const char *scan, const char *command, float *arg )
{
	//NOTE:
	// query may be NULL, indicating that the variable
	// may be queried by not supplying any argument.

	ASSERT( scan != NULL );
	ASSERT( command != NULL );
	ASSERT( arg != NULL );

	if ( strcmp( scan, command ) == 0 ) {
		if ( (scan = QueryFltArgument( query, arg )) ) {
			char *errpart;
			float sval = (float)strtod( scan, &errpart );
			if ( *errpart == 0 ) {
				*arg = sval;
			} else {
				CON_AddLine( invalid_arg );
			}
		}
		return TRUE;
	} else {
		return FALSE;
	}
}


// check command that alters an int value with guaranteed bounds --------------
//
int CheckSetIntArgBounded( const char *query, const char *scan, char *command, int *arg, int bmin, int bmax, void (*func)() )
{
	//NOTE:
	// this function accepts delta arguments. instead of
	// an absolute value ++val or --val may be supplied
	// to specify a value that should be added to or
	// subtracted from the current value, respectively.

	//NOTE:
	// query may be NULL, indicating that the variable
	// may be queried by not supplying any argument.

	ASSERT( scan != NULL );
	ASSERT( command != NULL );
	ASSERT( arg != NULL );
	ASSERT( bmin <= bmax );

	if ( strcmp( scan, command ) == 0 ) {
		if ( (scan = QueryIntArgument( query, arg )) ) {

			// determine if delta modification (++/--)
			int delta = 0;
			if ( ( scan[ 0 ] == '+' ) && ( scan[ 1 ] == '+' ) )
				delta = 1;
			else if ( ( scan[ 0 ] == '-' ) && ( scan[ 1 ] == '-' )  )
				delta = -1;
			if ( delta != 0 )
				scan += 2;

			if ( *scan != 0 ) {
				char *errpart;
				int sval = (int) strtol( scan, &errpart, int_calc_base );
				if ( *errpart == 0 ) {
					if ( delta != 0 )
						sval = *arg + sval * delta;
					if ( sval >= bmin && sval <= bmax ) {
						*arg = sval;
						if ( func != NULL )
							(*func)();
					} else {
						CON_AddLine( range_error );
					}
				} else {
					CON_AddLine( invalid_arg );
				}
			} else {
				CON_AddLine( invalid_arg );
			}
		}
		return TRUE;
	} else {
		return FALSE;
	}
}


// check commands that manipulate int array contents --------------------------
//
int CheckSetIntArray( const char *query, char *scan, char *comstub, int *array, int numelements )
{
	//NOTE:
	// query may be NULL, indicating that the variable
	// may be queried by not supplying any argument.

	ASSERT( scan != NULL );
	ASSERT( comstub != NULL );
	ASSERT( array != NULL );
	ASSERT( numelements > 0 );

	size_t cslen = strlen( comstub );

	if ( strncmp( scan, comstub, cslen ) == 0 ) {

		int indx;
		int ndig = 0;

		if ( ( strlen( scan ) == cslen + 2 ) &&
			 isdigit( scan[ cslen + 0 ] ) &&
			 isdigit( scan[ cslen + 1 ] ) ) {
			ndig = 2;
			indx = ( scan[ cslen ] - '0' ) * 10 + scan[ cslen + 1 ] - '0';
		} else if ( ( strlen( scan ) == cslen + 3 ) &&
			 isdigit( scan[ cslen + 0 ] ) &&
			 isdigit( scan[ cslen + 1 ] ) &&
			 isdigit( scan[ cslen + 2 ] ) ) {
			ndig = 3;
			indx = ( scan[ cslen ] - '0' ) * 100 + ( scan[ cslen + 1 ] - '0' ) * 10 + scan[ cslen + 2 ] - '0';
		}

		if ( ndig > 0 ) {
			if ( indx < numelements ) {
				int *arg = array + indx;
				if ( (scan = QueryIntArgument( query, arg )) ) {
					char *errpart;
					int sval = (int) strtol( scan, &errpart, int_calc_base );
					if ( *errpart == 0 ) {
						*arg = sval;
					} else {
						CON_AddLine( invalid_arg );
					}
				}
				return TRUE;
			} else {
				return FALSE;
			}
		} else {
			return FALSE;
		}
	} else {
		return FALSE;
	}
}


// check argument that alters a single string ---------------------------------
//
int CheckSetStrArgument( int query, char *scan, char *command, char *string, int maxlen )
{
	ASSERT( ( query == FALSE ) || ( query == TRUE ) );
	ASSERT( scan != NULL );
	ASSERT( command != NULL );
	ASSERT( string != NULL );
	ASSERT( maxlen > 0 );

	if ( strcmp( scan, command ) == 0 ) {
		if ( (scan = strtok( NULL, " " )) ) {
			if ( !strtok( NULL, " " ) ) {
				strncpy( string, scan, maxlen );
				string[ maxlen ] = 0;
			} else {
				CON_AddLine( too_many_args );
			}
		} else {
			if ( query ) {
				CON_AddLine( string );
			} else {
				CON_AddLine( arg_missing );
			}
		}
		return TRUE;
	} else {
		return FALSE;
	}
}



