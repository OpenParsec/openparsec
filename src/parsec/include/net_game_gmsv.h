/*
 * PARSEC HEADER: net_game_gmsv.h
 */

#ifndef _NET_GAME_GMSV_H_
#define _NET_GAME_GMSV_H_


// net_game_gmsv.c implements the following functions
// --------------------------------------------------
//  size_t	NETs_RmEvList_GetMaxSize();
//  void	NETs_UpdateKillStats( RE_KillStats* killstats );



// external functions ---------------------------------------------------------
//
void	NET_DisconnectNoConnection();
void	NET_ResyncLocalPlayer( RE_PlayerAndShipStatus* pas_status );

// declare prototypes when we do not compile for shared lib gamecode support --
//
void	NET_PingCurrentServer();
void	NET_ProcessPingPacket( NetPacket_GMSV* gamepacket );
void	NET_Handle_STREAM ( NetPacket_GMSV* gamepacket );


#endif // _NET_PEER_H_


