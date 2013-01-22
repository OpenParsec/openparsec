/*
 * PARSEC HEADER: net_rmev_peer.h
 */

#ifndef _NET_RMEV_PEER_H_
#define _NET_RMEV_PEER_H_


// net_rmev_peer.c implements the following functions
// --------------------------------------------------


// external functions

void	NET_ProcessNodeAlive_PEER( NetPacket_PEER* gamepacket );
void	NET_UpdateGameState_PEER ( NetPacket_PEER* gamepacket );


#endif // _NET_RMEV_PEER_H_


