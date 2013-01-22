/*
 * PARSEC HEADER: con_arg.h
 */

#ifndef _CON_ARG_H_
#define _CON_ARG_H_


// structure for key/value pairs

struct key_value_s {

	const char* key;
	char* 	value;

	dword	flags;
	dword	_padto_16;
};

#define KEYVALFLAG_NONE				0x0000
#define KEYVALFLAG_DISALLOW			0x0001
#define KEYVALFLAG_IGNORE			0x0002
#define KEYVALFLAG_MANDATORY		0x0004
#define KEYVALFLAG_PARENTHESIZE		0x0008


// structure for flag mapping spec

struct flag_map_s {

	const char* name;
	dword 	value;
	int		group;
	dword	_pad16;
};


// external functions

int		ScanKeyValuePairs( key_value_s *table, char *kvstr );
int		ScanKeyValueInt( key_value_s *keyval, int *ival );
int		ScanKeyValueIntList( key_value_s *keyval, int *ilist, int minlen, int maxlen );
int		ScanKeyValueIntListBounded( key_value_s *keyval, int *ilist, int minlen, int maxlen, int minval, int maxval );
int		ScanKeyValueFloat( key_value_s *keyval, float *fval );
int		ScanKeyValueFloatList( key_value_s *keyval, float *flist, int minlen, int maxlen );
int		ScanKeyValueFloatListBounded( key_value_s *keyval, float *flist, int minlen, int maxlen, float minval, float maxval );
int		ScanKeyValueFlagList( key_value_s *keyval, dword *flagword, flag_map_s *flagmap );
dword	ScanKeyValueObjClass( key_value_s *keyval, int keyclass, int keyid );

char*	GetParenthesizedName( char *name );
int		GetIntBehindCommand( char *cstr, int32 *iarg, int base, int paramoptional );
const char*	GetStringBehindCommand( char *scan, int paramoptional );
int		SetSingleStringCommand( char *cstr, char *dst, int dstmaxlen );

char*	QueryIntArgument( const char *query, int *arg );
char*	QueryFltArgument( const char *query, float *arg );
char*	QueryIntArgumentEx( char *params, const char *query, int *arg );
char*	QueryFltArgumentEx( char *params, const char *query, float *arg );
int		CheckSetIntArgument( const char *query, const char *scan, const char *command, int *arg );
int		CheckSetFltArgument( const char *query, const char *scan, const char *command, float *arg );
int		CheckSetIntArgBounded( const char *query, const char *scan, char *command, int *arg, int bmin, int bmax, void (*func)() );
int		CheckSetIntArray( const char *query, char *scan, char *comstub, int *array, int numelements );
int		CheckSetStrArgument( int query, char *scan, char *command, char *string, int maxlen );


#endif // _CON_ARG_H_


