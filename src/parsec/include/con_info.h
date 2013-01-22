/*
 * PARSEC HEADER: con_info.h
 */

#ifndef _CON_INFO_H_
#define _CON_INFO_H_


// callback function type
typedef int (*notify_callback_fpt)( GenObject *base );


// access information for single object property (type/class field)

struct proplist_s {

	const char*			propname;		// name property can be accessed by
	size_t				propoffset;		// offset of property in obj structure
	int 				bmin;			// minimum value of property
	int 				bmax;			// maximum value of property
	int					fieldtype;		// field's data type
	notify_callback_fpt notify_callback; // callback on field value changes
};

//NOTE:
// bmin and bmax (value bounds) for all types other than
// PROPTYPE_INT have to be specified in fixed-point!

//NOTE:
// for PROPTYPE_STRING and PROPTYPE_CHARPTR bmin and bmax specify
// minimum and maximum number of chars (excluding the '\0').

#define PROPTYPE_INT		0
#define PROPTYPE_FLOAT		1
#define PROPTYPE_GEOMV		2
#define PROPTYPE_FIXED		3
#define PROPTYPE_STRING		4
#define PROPTYPE_CHARPTR	5


// external functions

void	CON_RegisterCustomType( dword objtypeid, proplist_s *proplist );

char*	BuildTypeInfo( int typeindex );
char*	BuildClassInfo( int classindex );

int		CheckSetObjTypeProperty( const char *query, const char *scan );
int 	CheckSetObjClassProperty( const char *query, const char *scan );
int		CheckSetObjInstanceProperty( const char *query, const char *scan );

int 	CheckTypeInfo( const char *scan );
int 	CheckClassInfo( const char *scan );
int		CheckInstanceInfo( const char *scan );

int		WritePropList( FILE* fp, dword objid );

int		Cmd_FaceInfo( char *command );


#endif // _CON_INFO_H_


