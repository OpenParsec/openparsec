/*
 * PARSEC HEADER: net_limits.h
 */

#ifndef _NET_LIMITS_H_
#define _NET_LIMITS_H_

// various size constants -----------------------------------------------------
//
#define MAX_IPADDR_LEN					15		// xxx.xxx.xxx.xxx = 15
#define MAX_HOSTNAME_LEN				1024
#define MAX_OSNAME_LEN					31
#define MAX_FILENAME_LEN				255
#define MAX_SERVER_NAME					31
#define MAX_CFG_LINE					127
#define IP_ADR_LENGTH 					4		// four octet ip address
#define NODE_ADR_LENGTH					6		// ip address plus port number
#define MAX_STRLEN_IPADDR				MAX_IPADDR_LEN

#endif // _NET_LIMITS_H_
