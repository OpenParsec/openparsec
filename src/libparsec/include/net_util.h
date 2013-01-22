/*
 * PARSEC HEADER: net_util.h
 */

#ifndef _NET_UTIL_H_
#define _NET_UTIL_H_


// net_util.c implements the following functions
// ---------------------------------------------


// flag whether to include packet info writing code
#define PACKETINFO_WRITING_AVAILABLE


// external functions ---------------------------------------------------------
//
#ifdef PACKETINFO_WRITING_AVAILABLE
	int			NET_PacketInfo( const char *bstr, ... );
	void		NET_RmEvList_WriteInfo( FILE* fp, RE_Header* relist );
#endif // PACKETINFO_WRITING_AVAILABLE

dword	NET_CalcCRC( void* buf, size_t len );
void	NET_EncryptData( void* packet, size_t psize );
void	NET_DecryptData( void* packet, size_t psize );


// include proper server/client specific header -------------------------------
//
#ifdef PARSEC_CLIENT
	#include "net_util_cl.h"
#else // !PARSEC_CLIENT
	#include "net_util_sv.h"
#endif // !PARSEC_CLIENT


#endif // _NET_UTIL_H_


