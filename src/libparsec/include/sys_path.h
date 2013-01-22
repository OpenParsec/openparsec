/*
 * PARSEC HEADER: sys_path.h
 */

#ifndef _SYS_PATH_H_
#define _SYS_PATH_H_


// ----------------------------------------------------------------------------
// SYSTEM SUBSYSTEM (SYS) PATH PROCESSING                                     -
// ----------------------------------------------------------------------------

char*	SYSs_ProcessPathString( char *path );
char*	SYSs_ScanToExtension( char *fname );
char*	SYSs_StripPath( char *fname );
int		SYSs_AcquireScriptPath( char *path, int comtype, char *prefix );
int		SYSs_AcquireDemoPath( char *path, char *prefix );


#endif // _SYS_PATH_H_


