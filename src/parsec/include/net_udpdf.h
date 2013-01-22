/*
 * PARSEC HEADER: net_udpdf.h
 */

#ifndef _NET_UDPDF_H_
#define _NET_UDPDF_H_

#include "net_ports.h"

// default timeout value for connection to server -----------------------------
//
#define DEFAULT_TIMEOUT_SERVER_CONNECTION		10 * FRAME_MEASURE_TIMEBASE


// external functions ---------------------------------------------------------
//
void	UDP_StoreNodePort( node_t *node, word port );
word	UDP_GetNodePort( node_t *node );
int		NET_ResolveHostName( const char* hostname, char* ipaddress, node_t* node );


// UDP packet platform specific functions -------------------------------------
//
int		UDPs_GetLocalIP();
void	UDPs_GetLocalBroadcast();
int		UDPs_InitOSNetAPI();
void	UDPs_KillOSNetAPI();

#endif // _NET_UDPDF_H_


