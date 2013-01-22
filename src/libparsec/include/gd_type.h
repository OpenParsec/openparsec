/*
 * PARSEC HEADER: gd_type.h
 */

#ifndef _GD_TYPE_H_
#define _GD_TYPE_H_

#include <stdint.h>


// ----------------------------------------------------------------------------
// BASIC TYPE DEFINITIONS                                                     -
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
//
typedef uint8_t					uint8;
typedef uint16_t				uint16;
typedef uint32_t				uint32;
typedef uint64_t				uint64;

typedef int8_t					int8;
typedef int16_t					int16;
typedef int32_t					int32;
typedef int64_t					int64;


// asm-style types ------------------------------------------------------------
//
typedef uint8_t					byte;
typedef uint16_t				word;
typedef uint32_t				dword;
typedef uint64_t				qword;


// new types ------------------------------------------------------------------
//
typedef bool					bool_t;


// custom types ---------------------------------------------------------------
//
typedef int32					refframe_t;
#define REFFRAME_INVALID		-1


// boolean values -------------------------------------------------------------
//
#ifndef TRUE
	#define TRUE				1
#endif
#ifndef FALSE
	#define FALSE				0
#endif


#endif // _GD_TYPE_H_


