/*
 * PARSEC HEADER: e_getopt.h
 */

#ifndef _E_GETOPT_H_
#define _E_GETOPT_H_

#include <stdint.h>



// ----------------------------------------------------------------------------
//
#define ASSERT		assert
#define PRIVATE		static
#define PUBLIC
#define MSGOUT		printf

typedef uint8_t						byte;
typedef uint16_t					word;
typedef uint32_t					dword;

#ifndef TRUE
	#define TRUE					1
#endif
#ifndef FALSE
	#define FALSE					0
#endif




// command line options table entry

struct cli_option_s {

	const char*	opt_short;
	const char*	opt_long;

	dword	_mksiz32;

	byte	_dummy;
	byte	num_int;
	byte	num_float;
	byte	num_string;

	int		(*exec_set)();
	int		(*exec_int)(int*);
	int		(*exec_float)(float*);
	int		(*exec_string)(char**);
};


// application name option table entry

struct app_option_s {

	char*	appname;
	void	(*appmain)(int,char**);
};


// external functions

int		OPT_RegisterSetOption( const char *optshort, const char *optlong, int (*func)() );
int		OPT_RegisterIntOption( char *optshort, const char *optlong, int (*func)(int*) );
int		OPT_RegisterFloatOption( const char *optshort, const char *optlong, int (*func)(float*) );
int		OPT_RegisterStringOption( const char *optshort, const char *optlong, int (*func)(char**) );
int		OPT_RegisterOption( cli_option_s *regopt );
int		OPT_RegisterApplication( char *appname, void (*appmain)(int,char**) );

void	OPT_ClearOptions();

int		OPT_ExecRegisteredOptions( int argc, char **argv );


#endif // _E_GETOPT_H_

