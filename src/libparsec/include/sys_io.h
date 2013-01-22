/*
 * PARSEC HEADER: sys_io.h
 */

#ifndef _SYS_IO_H_
#define _SYS_IO_H_


// include header containing I/O system calls ---------------------------------

#if defined(SYSTEM_COMPILER_GCC) || defined(SYSTEM_COMPILER_CLANG) || defined(SYSTEM_COMPILER_LLVM_GCC)

	//NOTE:
	// io.h need not be included (doesn't exist and
	// prototypes are declared elsewhere).

	//NOTE:
	// filelength() doesn't exist;
	// SYS_FILE::SYS_filelength() must not be used.

	#define filelength(x)	(PANIC(0),0)

#else // SYSTEM_COMPILER_GCC

	// simply include correct header
	#include <io.h>

	#ifndef SYSTEM_COMPILER_MSVC
		#include <direct.h>
	#endif

#endif // SYSTEM_COMPILER_GCC


#endif // _SYS_IO_H_


