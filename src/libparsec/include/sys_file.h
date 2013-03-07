/*
 * PARSEC HEADER: sys_file.h
 */

#ifndef _SYS_FILE_H_
#define _SYS_FILE_H_


// external variables

extern int		num_data_packages;
extern char*	package_filename[];

#define PSCDATA_VERSION	"011" // version to match in the embedded file to verify we are good to go.


// external functions

int		SYS_OverridePackage( const char* oldpackname, const char* newpackname );
int		SYS_RegisterPackage( const char *packname, size_t baseofs, char *prefix );
int		SYS_AcquirePackageScripts( int comtype );
int		SYS_AcquirePackageDemos();
int		SYS_CheckDataVersion();

// signature compatible system file function replacements

long int	SYS_GetFileLength( const char *filename );
FILE*   	SYS_fopen( const char *filename, const char *mode );
int     	SYS_fclose( FILE *fp );
size_t  	SYS_fread( void *buf, size_t elsize, size_t nelem, FILE *fp );
int     	SYS_fseek( FILE *fp, long int offset, int whence );
long int	SYS_ftell( FILE *fp );
int     	SYS_feof( FILE *fp );
char*     SYS_fgets( char *string, int n, FILE *fp );

int     	SYS_open( const char *path, int access );
int     	SYS_close( int handle );
int     	SYS_read( int handle, char *buffer, int len );
long    	SYS_filelength( int handle );


#endif // _SYS_FILE_H_


