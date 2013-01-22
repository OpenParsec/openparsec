/*
 * PARSEC HEADER: net_rmev_gmsv.h
 */

#ifndef _NET_RMEV_GMSV_H_
#define _NET_RMEV_GMSV_H_


// net_rmev_gmsv.c implements the following functions
// --------------------------------------------------
// void	NET_ExecRmEvCommandInfo( RE_CommandInfo* commandinfo );

// external functions ---------------------------------------------------------
//
void			NET_ProcessRmEvList_GMSV( NetPacket_GMSV* gamepacket );
RE_CommandInfo* NET_RmEvSingleCommandInfo( NetPacket_GMSV* gamepacket, const char* commandstring );

#endif // _NET_RMEV_GMSV_H_


