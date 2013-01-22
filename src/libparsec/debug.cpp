/*
 * PARSEC - Debug Support Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:39 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */ 

// C library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// global compilation options
#include "config.h"

// for cleanup function
#include "gd_type.h"
#ifdef PARSEC_SERVER
	#include "sys_err_sv.h"
#else
	#include "sys_err.h"
	#include "sys_glob.h"
#endif


// local module header
#include "debug.h"


// flags
//#define REGISTER_MCBDUMP_COMMAND
//#define WRITE_MALLOC_FREE_LOG



// log file names -------------------------------------------------------------
//
#ifdef WRITE_MALLOC_FREE_LOG
	static char use_log_name[] = "memuse.log";
#endif

#ifdef REGISTER_MCBDUMP_COMMAND
	static char mcb_log_name[] = "mcbdump.log";
#endif


#ifdef USE_ALIGNED_MEMBLOCKS // -----------------------------------------------


// alignment to enforce for all allocated memory blocks
#define MEMBLOCK_ALIGNMENT_VAL		0x1f		// align to 32 byte boundary
#define MEMBLOCK_ALIGNMENT_MASK		(~MEMBLOCK_ALIGNMENT_VAL)


// malloc wrapper that enforces a guaranteed block alignment ------------------
//
void* _AlignedMalloc( size_t size )
{
	void *temp = HEAP_ALLOC( size + ( MEMBLOCK_ALIGNMENT_VAL + 1 ) * 2 );

	if ( temp == NULL )
		return NULL;

	size_t alignmentpadding = ( ( (size_t)temp + MEMBLOCK_ALIGNMENT_VAL ) & MEMBLOCK_ALIGNMENT_MASK ) - (size_t)temp;

	size_t *alignhead = (size_t *) ( (size_t)temp + alignmentpadding );
	*alignhead = (size_t)temp;

#if defined( WRITE_MALLOC_FREE_LOG ) && !defined( LOG_MEMORY_BLOCKS )

	refframe_t SYSs_GetRefFrameCount();
	refframe_t currefframe = SYSs_GetRefFrameCount();

	FILE *fp = fopen( use_log_name, "a" );
	if ( fp != NULL ) {
		fprintf( fp, "[%03d.%03d.%03d.%03d] A_ALLOC: 0x%08X (%d)\n",
			( ( currefframe >> 24 ) & 0xff ),
			( ( currefframe >> 16 ) & 0xff ),
			( ( currefframe >>  8 ) & 0xff ),
			(   currefframe         & 0xff ),
			(size_t)alignhead + MEMBLOCK_ALIGNMENT_VAL + 1, size );
		fclose( fp );
	}

#endif

	return (void*)( (size_t)alignhead + MEMBLOCK_ALIGNMENT_VAL + 1 );
}


// free wrapper needed if aligned mallocs used --------------------------------
//
void _AlignedFree( void *mem )
{

#if defined( WRITE_MALLOC_FREE_LOG ) && !defined( LOG_MEMORY_BLOCKS )

	refframe_t SYSs_GetRefFrameCount();
	refframe_t currefframe = SYSs_GetRefFrameCount();

	FILE *fp = fopen( use_log_name, "a" );
	if ( fp != NULL ) {
		fprintf( fp, "[%03d.%03d.%03d.%03d] A_FREE : 0x%08X\n",
			( ( currefframe >> 24 ) & 0xff ),
			( ( currefframe >> 16 ) & 0xff ),
			( ( currefframe >>  8 ) & 0xff ),
			(   currefframe         & 0xff ),
			(size_t)mem );
		fclose( fp );
	}

#endif

	HEAP_FREE( (void*) ( *(size_t*)( (size_t)mem - ( MEMBLOCK_ALIGNMENT_VAL + 1 ) ) ) );
}


// redefine heap functions to enforce alignment -------------------------------
//
#undef HEAP_ALLOC
#undef HEAP_FREE

#define HEAP_ALLOC					_AlignedMalloc
#define HEAP_FREE					_AlignedFree


#endif // USE_ALIGNED_MEMBLOCKS -----------------------------------------------



// memory alloc/free/size counters

unsigned int Num_Allocs   = 0;
unsigned int Num_Frees	  = 0;
unsigned int Dyn_Mem_Size = 0;


// include system debugging functions -----------------------------------------
//
#include "debugsys.h"



#ifdef LOG_MEMORY_BLOCKS // ---------------------------------------------------


// structure for logging allocated memory blocks ------------------------------
//
struct MCB_Log {

	MCB_Log* next;		// next block in loglist
	size_t   size;		// size of memblock (incl. signature)
	void*    address;	// blockaddress (pointer to head-sig)
	void*    caller;	// address of ALLOCMEM call (owner)
};

MCB_Log *McbLogList = NULL;


#define MCB_SIG_SIZE 32
static char mcb_signature[] = "msh MCB BlockCheckerBoundarySig";


// insert new log-block into list ---------------------------------------------
//
void InsertMcbLog( void *address, size_t size, void *caller )
{
	MCB_Log *temp = (MCB_Log *) HEAP_ALLOC( sizeof( MCB_Log ) );
	ASSERT( temp != NULL );

	temp->address = address;
	temp->caller  = caller;
	temp->size	  = size;
	temp->next	  = McbLogList;
	McbLogList	  = temp;
}


// delete log-block from list -------------------------------------------------
//
void DeleteMcbLog( void *address )
{
	MCB_Log *prev = NULL;

	for ( MCB_Log *temp = McbLogList; temp; temp = temp->next ) {

		if ( temp->address == address ) {
			if ( prev == NULL ) {
				McbLogList = temp->next;
				HEAP_FREE( temp );
				return;
			} else {
				prev->next = temp->next;
				HEAP_FREE( temp );
				return;
			}
		}

		prev = temp;
	}

	ASSERT( 0 );
}


// get pointer to log-block ---------------------------------------------------
//
MCB_Log *GetMcbLog( void *address )
{
	for ( MCB_Log *temp = McbLogList; temp; temp = temp->next ) {

		if ( temp->address == address )
			return temp;
	}

//	  ASSERT( 0 );
	return NULL;
}


#endif	// LOG_MEMORY_BLOCKS --------------------------------------------------



#ifdef EXTENSIVE_MEMORY_CHECKS // ---------------------------------------------


#ifndef LOG_MEMORY_BLOCKS
	#error "inconsistent debugging specs."
#endif


// check if ref is a valid base address of any heap block ---------------------
//
void _CheckHeapBaseRef( void *ref )
{
	ref = (void *) ( (char*)ref - MCB_SIG_SIZE );
	ASSERT( GetMcbLog( ref ) != NULL );
}


// check if ref is a valid pointer into any heap block ------------------------
//
void _CheckHeapRef( void *ref )
{
	for ( MCB_Log *temp = McbLogList; temp; temp = temp->next ) {

		if ( ( (unsigned)temp->address <= (unsigned)ref-MCB_SIG_SIZE ) &&
			 ( ((unsigned)temp->address+temp->size-MCB_SIG_SIZE) > (unsigned)ref ) )
			return;
	}

	ASSERT( 0 );
}


// check integrity of all dynamically allocated memory blocks -----------------
//
void _CheckMemIntegrity()
{
	int nummcbs = 0;
	for ( MCB_Log *temp = McbLogList; temp; temp = temp->next, nummcbs++ ) {

		int corrupt1 = ( strcmp( (char*)temp->address, mcb_signature ) != 0 );
		int corrupt2 = ( strcmp( (char*)temp->address+temp->size-MCB_SIG_SIZE, mcb_signature ) != 0 );

		// make breakpoint setting easy
		if ( corrupt1 || corrupt2 ) {
			ASSERT( 0 );
		}
	}

	ASSERT( nummcbs == ( Num_Allocs - Num_Frees ) );
}


#endif // EXTENSIVE_MEMORY_CHECKS ---------------------------------------------



#ifdef LOG_MEMORY_BLOCKS // ---------------------------------------------------


// allocate memory block ------------------------------------------------------
//
void* _LogAllocMem( size_t size )
{
	// first thing: retrieve caller's address
	void *caller;
	GETCALLER( caller )

	ASSERT( size > 0 );

	size += MCB_SIG_SIZE * 2;

	void *temp = HEAP_ALLOC( size );
	ASSERT( temp != NULL );

	Num_Allocs++;
	Dyn_Mem_Size += size;

	// log mem allocation
	InsertMcbLog( temp, size, caller );

	unsigned *filldw = (unsigned *) temp;
	size_t siz4 = size / 4;
	while ( siz4-- > 0 )
		*filldw++ = 0xCCCCCCCC;

	unsigned char *fillb = (unsigned char *) filldw;
	siz4 = size % 4;
	while ( siz4-- > 0 )
		*fillb++ = 0xCC;

	strcpy( (char*)temp, mcb_signature );
	strcpy( (char*)temp + size - MCB_SIG_SIZE, mcb_signature );

	return (void *) ( (char*)temp + MCB_SIG_SIZE );
}


// free memory block ----------------------------------------------------------
//
void _LogFreeMem( void *mem )
{
	ASSERT( mem != NULL );

#ifdef CHECK_MEM_INTEGRITY_ON_FREE

#ifndef EXTENSIVE_MEMORY_CHECKS
	#error "inconsistent debugging specs."
#endif

	CHECKMEMINTEGRITY();

#endif

	mem = (void *) ( (char*)mem - MCB_SIG_SIZE );

	MCB_Log *log = GetMcbLog( mem );
	ASSERT( log != NULL );

	size_t size = log->size;

	DeleteMcbLog( mem );

	unsigned *filldw = (unsigned *) mem;
	size_t siz4 = size / 4;
	while ( siz4-- > 0 )
		*filldw++ = 0xCCCCCCCC;

	unsigned char *fillb = (unsigned char *) filldw;
	siz4 = size % 4;
	while ( siz4-- > 0 )
		*fillb++ = 0xCC;

	HEAP_FREE( mem );

	Num_Frees++;
	Dyn_Mem_Size -= size;
}


#endif // LOG_MEMORY_BLOCKS ---------------------------------------------------



#if defined( REGISTER_MCBDUMP_COMMAND ) && defined( LOG_MEMORY_BLOCKS ) // ----


// user command registration
#include "con_com.h"
#include "gd_help.h"
#include "gd_type.h"


// command "mcbdump" for dumping the entire mcb list to a log file ------------
//
PRIVATE
int Cmd_MCBDUMP( char *params )
{
	ASSERT( params != NULL );
	void CON_AddLine( char *text );
	USERCOMMAND_NOPARAM( params );

	refframe_t SYSs_GetRefFrameCount();
	refframe_t currefframe = SYSs_GetRefFrameCount();

	FILE *fp = fopen( mcb_log_name, "a" );
	if ( fp != NULL ) {

		fprintf( fp, "\n[%03d.%03d.%03d.%03d] --------------\n",
			( ( currefframe >> 24 ) & 0xff ),
			( ( currefframe >> 16 ) & 0xff ),
			( ( currefframe >>  8 ) & 0xff ),
			(   currefframe         & 0xff ) );

		int    nummcbs = 0;
		size_t mcbsum  = 0;

		for ( MCB_Log *temp = McbLogList; temp; temp = temp->next, nummcbs++ ) {

			int corrupt1 = ( strcmp( (char*)temp->address, mcb_signature ) != 0 );
			int corrupt2 = ( strcmp( (char*)temp->address+temp->size-MCB_SIG_SIZE, mcb_signature ) != 0 );

			mcbsum += temp->size;

			fprintf( fp, "[%d] adx=0x%08X own=0x%08X siz=%d", nummcbs, (size_t)temp->address, (size_t)temp->caller, temp->size );
			if ( corrupt1 )
				fprintf( fp, " **START CORRUPTED**" );
			if ( corrupt2 )
				fprintf( fp, " **END CORRUPTED**" );
			fprintf( fp, "\n" );
		}
		fprintf( fp, "\n%d bytes in %d entries.\n", mcbsum, nummcbs );

		fclose( fp );
	}

	return TRUE;
}


// register mcb dumping command -----------------------------------------------
//
REGISTER_MODULE( DEBUG )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	regcom.command	 = "mcbdump";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_MCBDUMP;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );
}


#endif // REGISTER_MCBDUMP_COMMAND && LOG_MEMORY_BLOCKS -----------------------



// check assertion, abort if assertion failed ---------------------------------
//
#ifdef SYSTEM_LINUX
	#define SYSABORT()		exit( EXIT_FAILURE )	// abort() crashes
#else
	#define SYSABORT()		abort()
#endif


// check assertion, abort if assertion failed ---------------------------------
//
void _SysAssert( const char *file, unsigned line )
{
	static int inSysAssert = FALSE;

	if ( !inSysAssert ) {
		inSysAssert = TRUE;

#if defined ( _WIN32 ) && !defined ( __MINGW32__ )
		__debugbreak();
#endif // SYSTEM_WIN32

		SYSs_CriticalCleanUp();

		fflush( NULL );
		fprintf( stderr, "\nAssertion failed: %s, line %u\n", file, line );
		fflush( stderr );

		// system-dependent handling
		SysAssertCallback( file, line );

		// abort if callback returned
		SYSABORT();
	}
}



