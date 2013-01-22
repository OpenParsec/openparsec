/*
 * PARSEC HEADER: net_util_sv.h
 */

#ifndef _NET_UTIL_SV_H_
#define _NET_UTIL_SV_H_

// net_util_sv.c implements the following functions
// ------------------------------------------------

// external functions ---------------------------------------------------------
//
void		NODE_Copy( node_t* dst, node_t* src );
void		NODE_StorePort( node_t *node, word port );
word		NODE_GetPort( node_t *node );
int			NODE_Compare( node_t *node1, node_t *node2 );
int			NODE_AreSame( node_t *node1, node_t *node2 );
char*		NODE_Print( node_t* node );

// external variables ---------------------------------------------------------
//

#endif // _NET_UTIL_SV_H_


