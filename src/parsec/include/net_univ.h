/*
 * PARSEC HEADER: net_univ.h
 */

#ifndef _NET_UNIV_H_
#define _NET_UNIV_H_


// for server_s ---------------------------------------------------------------
// 
#include "net_csdf.h"

// max. # of entries in the linklist ------------------------------------------
//
#define MAX_LINKLIST_SIZE		MAX_SERVERS * MAX_NUM_LINKS / 2

// forward decls --------------------------------------------------------------
//
struct Stargate;

// external variables ---------------------------------------------------------
//
extern server_s		server_list[];
extern link_s		link_list[];
extern mapobj_s*	map_objs[];

extern int			num_servers_joined;
extern int			g_nNumLinks;
extern int			num_map_objs;


// external functions ---------------------------------------------------------
//
int			NET_GetGameServerList();
Stargate*	NET_FindStargate( word serverid );


#endif // _NET_UNIV_H_


