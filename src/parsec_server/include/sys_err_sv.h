/*
 * PARSEC HEADER: sys_err_sv.h
 */

#ifndef _SYS_ERR_SV_H_
#define _SYS_ERR_SV_H_


#include "debug.h"


// ----------------------------------------------------------------------------
// SERVER SYSTEM SUBSYSTEM (SYS) ERROR HANDLING                              -
// ----------------------------------------------------------------------------

void SYSs_CriticalCleanUp();
FUNCTION_NORETURN(void SYSs_PanicFunction( const char *msg, const char *file, unsigned line ));
FUNCTION_NORETURN(void SYSs_OutOfMemFunction( const char *msg, const char *file, unsigned line ));

FUNCTION_NORETURN(void SYSs_PError( const char *msg, ... ));

// wrappers

#define PANIC(x)		SYSs_PanicFunction( (x), __FILE__, __LINE__ )
#define OUTOFMEM(x)		SYSs_OutOfMemFunction( (x), __FILE__, __LINE__ )
#define PERROR			SYSs_PError
#define FERROR			SYSs_PError		//FIXME: //SYSs_FError


#endif // _SYS_ERR_SV_H_

