/*
 * PARSEC HEADER: con_kmap.h
 */

#ifndef _CON_KMAP_H_
#define _CON_KMAP_H_


// command mapped to key ------------------------------------------------------
//
struct keycom_s {

	char*	command;	// console command string
	int		echo;		// echo flag (display, add to history)
};


// textual description of key code --------------------------------------------
//
struct textkeymap_s {

	dword		code;		// key code
	const char*	text;	// name of corresponding key
};


// external variables

extern keycom_s				key_com_mappings[];
extern textkeymap_s			make_codes[];
extern textkeymap_s			key_com_names[];
extern const char *const	functional_keys[];


// external functions

char *	GetAKCDescription( int num );
char *	GetMKCDescription( int num );
char *	GetGameFuncDescription( int num );

int		CheckKeyMappingEcho( char *scan );
int		CheckKeyMappingSilent( char *scan );

void	ExecBoundKeyCommands();


#endif // _CON_KMAP_H_


