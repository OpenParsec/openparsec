/*
 * PARSEC MODULE (ENGINE CORE)
 * Command Line Options Support V1.05
 *
 * Copyright (c) Markus Hadwiger 1999-2000
 * All Rights Reserved.
 */

// C library
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// local module header
#include "getopt.h"



// command line options tables ------------------------------------------------
//
#define DEFAULT_MAX_OPTIONS		32

static cli_option_s  cli_options_default[ DEFAULT_MAX_OPTIONS ];
static cli_option_s* cli_options = cli_options_default;
static int			 num_cli_options = 0;
static int			 max_cli_options = DEFAULT_MAX_OPTIONS;


// shortcut for registering a new set command line option ---------------------
//
int OPT_RegisterSetOption( const char *optshort, const char *optlong, int (*func)() )
{
	ASSERT( ( optshort != NULL ) || ( optlong != NULL ) );
	ASSERT( func != NULL );

	cli_option_s clioption;
	memset( &clioption, 0, sizeof( cli_option_s ) );

	clioption.opt_short	= optshort;
	clioption.opt_long	= optlong;
	clioption.exec_set	= func;

	return OPT_RegisterOption( &clioption );
}


// shortcut for registering a new int command line option ---------------------
//
int OPT_RegisterIntOption( const char *optshort, const char *optlong, int (*func)(int*) )
{
	ASSERT( ( optshort != NULL ) || ( optlong != NULL ) );
	ASSERT( func != NULL );

	cli_option_s clioption;
	memset( &clioption, 0, sizeof( cli_option_s ) );

	clioption.opt_short	= optshort;
	clioption.opt_long	= optlong;
	clioption.num_int	= 1;
	clioption.exec_int	= func;

	return OPT_RegisterOption( &clioption );
}


// shortcut for registering a new float command line option -------------------
//
int OPT_RegisterFloatOption( const char *optshort, const char *optlong, int (*func)(float*) )
{
	ASSERT( ( optshort != NULL ) || ( optlong != NULL ) );
	ASSERT( func != NULL );

	cli_option_s clioption;
	memset( &clioption, 0, sizeof( cli_option_s ) );

	clioption.opt_short	 = optshort;
	clioption.opt_long	 = optlong;
	clioption.num_float	 = 1;
	clioption.exec_float = func;

	return OPT_RegisterOption( &clioption );
}


// shortcut for registering a new string command line option ------------------
//
int OPT_RegisterStringOption( const char *optshort, const char *optlong, int (*func)(char**) )
{
	ASSERT( ( optshort != NULL ) || ( optlong != NULL ) );
	ASSERT( func != NULL );

	cli_option_s clioption;
	memset( &clioption, 0, sizeof( cli_option_s ) );

	clioption.opt_short	  = optshort;
	clioption.opt_long	  = optlong;
	clioption.num_string  = 1;
	clioption.exec_string = func;

	return OPT_RegisterOption( &clioption );
}


// register a new command line option -----------------------------------------
//
int OPT_RegisterOption( cli_option_s *regopt )
{
	//NOTE:
	//CAVEAT:
	// the supplied option strings are not copied
	// by this function. thus, the caller MUST ENSURE
	// that these strings are available indefinitely.
	// (e.g., allocated statically.)

	ASSERT( regopt != NULL );
	ASSERT( num_cli_options <= max_cli_options );

	// expand table memory if already used up
	if ( num_cli_options == max_cli_options ) {

		// expand exponentially
		int newtabsize = max_cli_options * 2;

		// alloc new table
		cli_option_s * newlist = new cli_option_s[newtabsize];
		if ( newlist == NULL ) {
			ASSERT( 0 );
			return FALSE;
		}

		// set new size
		max_cli_options = newtabsize;

		// move old table
		memcpy( newlist, cli_options, sizeof( cli_option_s ) * num_cli_options );
		if ( cli_options != cli_options_default )
			delete [] cli_options;
		cli_options = newlist;
	}

	// append new command line option
	ASSERT( num_cli_options < max_cli_options );
	cli_options[ num_cli_options++ ] = *regopt;

	return TRUE;
}


// application name option tables ---------------------------------------------
//
#define DEFAULT_MAX_APPLICATIONS	8

static app_option_s  app_options_default[ DEFAULT_MAX_APPLICATIONS ];
static app_option_s* app_options = app_options_default;
static int			 num_app_options = 0;
static int			 max_app_options = DEFAULT_MAX_APPLICATIONS;


// register a special command line option for application invocation ----------
//
int OPT_RegisterApplication( char *appname, void (*appmain)(int,char**) )
{
	ASSERT( appname != NULL );
	ASSERT( appmain != NULL );

	//NOTE:
	//CAVEAT:
	// the supplied name string is not copied
	// by this function. thus, the caller MUST ENSURE
	// that this string is available indefinitely.
	// (e.g., allocated statically.)

	ASSERT( num_app_options <= max_app_options );

	// expand table memory if already used up
	if ( num_app_options == max_app_options ) {

		// expand exponentially
		int newtabsize = max_app_options * 2;

		// alloc new table
		app_option_s * newlist = new app_option_s[newtabsize];
		if ( newlist == NULL ) {
			ASSERT( 0 );
			return FALSE;
		}

		// set new size
		max_app_options = newtabsize;

		// move old table
		memcpy( newlist, app_options, sizeof( app_option_s ) * num_app_options );
		if ( app_options != app_options_default )
			delete [] app_options;
		app_options = newlist;
	}

	// append new command line option
	ASSERT( num_app_options < max_app_options );
	app_options[ num_app_options ].appname = appname;
	app_options[ num_app_options ].appmain = appmain;
	num_app_options++;

	return TRUE;
}


// remove all registered options ----------------------------------------------
//
void OPT_ClearOptions()
{
	// free expanded table memory if present
	if ( cli_options != cli_options_default ) {
		delete [] cli_options;
		cli_options = cli_options_default;
	}

	num_cli_options = 0;
	max_cli_options = DEFAULT_MAX_OPTIONS;

	// free expanded table memory if present
	if ( app_options != app_options_default ) {
		delete [] app_options;
		app_options = app_options_default;
	}

	num_app_options = 0;
	max_app_options = DEFAULT_MAX_APPLICATIONS;
}


// options parsing globals ----------------------------------------------------
//
#define MAX_INT_PARAMS		8
#define MAX_FLOAT_PARAMS	8
#define MAX_STRING_PARAMS	4

static int		cur_opt_indx;
static int		cur_int_indx;
static int		cur_float_indx;
static int		cur_string_indx;

static int		want_params_int		= 0;
static int		want_params_float	= 0;
static int		want_params_string	= 0;

static int		params_int[ MAX_INT_PARAMS ];
static float	params_float[ MAX_FLOAT_PARAMS ];
static char*	params_string[ MAX_STRING_PARAMS ];


// exec set option immediately/schedule others for parameter parsing ----------
//
PRIVATE
int ScheduleOption( int indx )
{
	cli_option_s *refopt = &cli_options[ indx ];

	// remember option we found
	cur_opt_indx = indx;

	if ( cli_options[ indx ].exec_set ) {

		want_params_int		= 0;
		want_params_float	= 0;
		want_params_string	= 0;

		// set options will be called immediately
		return (*cli_options[ indx ].exec_set)();

	} else {

		cur_int_indx		= 0;
		cur_float_indx		= 0;
		cur_string_indx		= 0;

		want_params_int		= refopt->num_int;
		want_params_float	= refopt->num_float;
		want_params_string	= refopt->num_string;

		// options with parameters will be called after
		// parameter parsing has completed
		return TRUE;
	}
}


// try to find a match for the whole short option string ----------------------
//
PRIVATE
int CheckShortStringMatch( char *opt )
{
	ASSERT( opt != NULL );
	int indx = 0;
	// try first to match the entire string
	for (  indx = 0; indx < num_cli_options; indx++ ) {

		cli_option_s *refopt = &cli_options[ indx ];
		if ( refopt->opt_short != NULL ) {
			if ( strcmp( opt, refopt->opt_short ) == 0 ) {

				// exec immediately/schedule parameter parsing
				if ( !ScheduleOption( indx ) ) {
					return FALSE;
				}
				break;
			}
		}
	}

	// return whether full-string match found
	return ( indx < num_cli_options );
}


// extract multiple options from a single short option string -----------------
//
PRIVATE
int CheckShortCharContained( char *opt )
{
	ASSERT( opt != NULL );

	// make absolutely sure the supplied string is writable
	char temp[ 20 + 1 ];
	strncpy( temp, opt, 20 );
	temp[ 20 ] = 0;
	opt = temp;

	// scan all registered short (-) options
	for ( int indx = 0; indx < num_cli_options; indx++ ) {
		if ( cli_options[ indx ].opt_short != NULL ) {

			if ( strlen( opt ) < 2 ) {
				if ( *opt == *cli_options[ indx ].opt_short ) {
					// exec immediately/schedule parameter parsing
					if ( !ScheduleOption( indx ) ) {
						return FALSE;
					}
				}
			} else {
				// search for current option in multiple options string
				for ( char *optchar = opt; *optchar != 0; optchar++ ) {
					if ( *optchar == *cli_options[ indx ].opt_short ) {
						// only set options allowed
						if ( cli_options[ indx ].exec_set ) {
							if ( !(*cli_options[ indx ].exec_set)() ) {
								return FALSE;
							}
						} else {
							return FALSE;
						}
						// remove as recognized
						*optchar = ' ';
					}
				}
			}
		}
	}

	// check whether all options recognized
	for ( char *optchar = opt; *optchar != 0; optchar++ ) {
		if ( *optchar != ' ' ) {
			return FALSE;
		}
	}

	return TRUE;
}


// try to match the supplied string with one or more short options ------------
//
PRIVATE
int CheckShortOption( char *opt )
{
	ASSERT( opt != NULL );

	// try first to match the entire string
	if ( CheckShortStringMatch( opt ) )
		return TRUE;

	// if no string-match use string as set of options
	return CheckShortCharContained( opt );
}


// try to match the supplied string with a long option ------------------------
//
PRIVATE
int CheckLongOption( char *opt )
{
	ASSERT( opt != NULL );
	int indx = 0;
	// compare with all registered long (--) options
	for (  indx = 0; indx < num_cli_options; indx++ ) {

		cli_option_s *refopt = &cli_options[ indx ];
		if ( refopt->opt_long != NULL ) {
			if ( strcmp( opt, refopt->opt_long ) == 0 ) {

				// exec immediately/schedule parameter parsing
				if ( !ScheduleOption( indx ) ) {
					return FALSE;
				}
				break;
			}
		}
	}

	// return whether match found
	return ( indx < num_cli_options );
}


// get int parameter for command line option ----------------------------------
//
PRIVATE
int GetOptionIntParam( char *paramstr )
{
	ASSERT( paramstr != NULL );

	char *errpart;
	long iparam = strtol( paramstr, &errpart, 10 );

	if ( *errpart != 0 ) {
		return FALSE;
	}

	// store int into table
	params_int[ cur_int_indx++ ] = iparam;

	// submit completed table to registered function
	if ( --want_params_int == 0 ) {
		if ( !(*cli_options[ cur_opt_indx ].exec_int)( params_int ) ) {
			return FALSE;
		}
	}

	return TRUE;
}


// get float parameter for command line option --------------------------------
//
PRIVATE
int GetOptionFloatParam( char *paramstr )
{
	ASSERT( paramstr != NULL );

	char *errpart;
	double fparam = strtod( paramstr, &errpart );

	if ( *errpart != 0 ) {
		return FALSE;
	}

	// store float into table
	params_float[ cur_float_indx++ ] = fparam;

	// submit completed table to registered function
	if ( --want_params_float == 0 ) {
		if ( !(*cli_options[ cur_opt_indx ].exec_float)( params_float ) ) {
			return FALSE;
		}
	}

	return TRUE;
}


// get string parameter for command line option -------------------------------
//
PRIVATE
int GetOptionStringParam( char *paramstr )
{
	ASSERT( paramstr != NULL );

	// store pointer into table
	params_string[ cur_string_indx++ ] = paramstr;

	// submit completed table to registered function
	if ( --want_params_string == 0 ) {
		if ( !(*cli_options[ cur_opt_indx ].exec_string)( params_string ) ) {
			return FALSE;
		}
	}

	return TRUE;
}


// determine whether there are still parameters missing for previous option ---
//
PRIVATE
int OptionParamsPending()
{
	int pendingparams = want_params_int + want_params_float + want_params_string;

	if ( pendingparams > 0 ) {
		MSGOUT( "Parameters missing.\n" );
		return TRUE;
	}

	return FALSE;
}


// parse a command line option specifier --------------------------------------
//
PRIVATE
int ParseOptionSpecifier( char **argv, int curopt )
{
	ASSERT( argv != NULL );
	ASSERT( *argv != NULL );

	// make sure there are no parameters missing for previous option
	if ( OptionParamsPending() ) {
		return FALSE;
	}

	int optvalid = TRUE;

	// distinguish short and long options via second '-'
	if ( argv[ curopt ][ 1 ] != '-' ) {
		optvalid = CheckShortOption( &argv[ curopt ][ 1 ] );
	} else {
		optvalid = CheckLongOption( &argv[ curopt ][ 2 ] );
	}

	return optvalid;
}


// parse a command line option parameter --------------------------------------
//
PRIVATE
int ParseOptionParameter( char **argv, int curopt )
{
	ASSERT( argv != NULL );
	ASSERT( *argv != NULL );

	int optvalid = TRUE;

	// check for parameters
	if ( want_params_int > 0 ) {
		optvalid = GetOptionIntParam( argv[ curopt ] );
	} else if ( want_params_float > 0 ) {
		optvalid = GetOptionFloatParam( argv[ curopt ] );
	} else if ( want_params_string > 0 ) {
		optvalid = GetOptionStringParam( argv[ curopt ] );
	}

	return optvalid;
}


// parse a command line option that specifies an application ------------------
//
PRIVATE
void CheckApplicationOption( int argc, char **argv )
{
	if ( argc < 2 )
		return;

	// compare with all registered application names
	for ( int indx = 0; indx < num_app_options; indx++ ) {

		ASSERT( app_options[ indx ].appname != NULL );
		ASSERT( app_options[ indx ].appmain != NULL );

		if ( strcmp( argv[ 1 ], app_options[ indx ].appname ) == 0 ) {

			// remove appname option from command line and hand over
			argv[ 1 ] = argv[ 0 ];
			(*app_options[ indx ].appmain)( argc - 1, argv + 1 );
		}
	}
}


// execute all registered command line options (parse command line) -----------
//
int OPT_ExecRegisteredOptions( int argc, char **argv )
{
	ASSERT( argc > 0 );
	ASSERT( argv != NULL );

	// if an application option is found this never returns
	CheckApplicationOption( argc, argv );

	// parse conventional command line options
	for ( int curopt = 1; curopt < argc; curopt++ ) {

		// check for beginning of new option
		if ( argv[ curopt ][ 0 ] == '-' ) {

			if ( !ParseOptionSpecifier( argv, curopt ) ) {
				MSGOUT( "Invalid option supplied: %s.\n", argv[ curopt ] );
				return FALSE;
			}

		} else {

			if ( !ParseOptionParameter( argv, curopt ) ) {
				MSGOUT( "Invalid parameter supplied: %s.\n", argv[ curopt ] );
				return FALSE;
			}
		}
	}

	// make sure there are no parameters missing for last option
	if ( OptionParamsPending() ) {
		return FALSE;
	}

	return TRUE;
}


