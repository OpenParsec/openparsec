/*
 * PARSEC HEADER: sys_err.h
 */

#ifndef _SYS_ERR_H_
#define _SYS_ERR_H_


#include "debug.h"


// ----------------------------------------------------------------------------
// SYSTEM SUBSYSTEM (SYS) ERROR HANDLING                                      -
// ----------------------------------------------------------------------------

void	SYSs_InitErrorHandling();

void	SYSs_CriticalCleanUp();

FUNCTION_NORETURN(void SYSs_PanicFunction( const char *msg, const char *file, unsigned line ));
FUNCTION_NORETURN(void SYSs_OutOfMemFunction( const char *msg, const char *file, unsigned line ));

FUNCTION_NORETURN(void SYSs_PError( const char *msg, ... ));
FUNCTION_NORETURN(void SYSs_SError( const char *msg ));
FUNCTION_NORETURN(void SYSs_FError( const char *msg, const char *filename ));


// wrappers

#define PANIC(x)		SYSs_PanicFunction( (x), __FILE__, __LINE__ )
#define OUTOFMEM(x)		SYSs_OutOfMemFunction( (x), __FILE__, __LINE__ )

#define PERROR			SYSs_PError
#define SERROR			SYSs_SError
#define FERROR			SYSs_FError


#endif // _SYS_ERR_H_


