/*
 * PARSEC HEADER: nw_udp.h
 */

#ifndef _NW_UDP_H_
#define _NW_UDP_H_


// nw_udp.c implements the following UDP platform specific functions
// -----------------------------------------------------------------
//	int			UDPs_GetLocalIP();
//	void		UDPs_GetLocalBroadcast();
//	int			UDPs_InitOSNetAPI();
//	void		UDPs_KillOSNetAPI();


// link winsock library
#ifdef NEED_WINSOCK2
	#pragma comment( lib, "ws2_32" )
#else
	#pragma comment( lib, "wsock32" )
#endif


#endif // _NW_UDP_H_


