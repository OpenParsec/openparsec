/*
 * PARSEC HEADER: net_peer.h
 */

#ifndef _NET_PEER_H_
#define _NET_PEER_H_


// net_peer.c implements the following functions
// ---------------------------------------------
//	int		NETs_Connect();
//	int		NETs_Disconnect();
//	int		NETs_Join();
//	int		NETs_Unjoin( byte flag );
//	int		NETs_UpdateName();
//	void	NETs_MaintainNet();


#ifdef DBIND_PROTOCOL

#undef  NETs_Connect
#undef  NETs_Disconnect
#undef  NETs_Join
#undef  NETs_Unjoin
#undef  NETs_UpdateName
#undef  NETs_MaintainNet

#define NETs_Connect					NETs_PEERTOPEER_Connect
#define NETs_Disconnect					NETs_PEERTOPEER_Disconnect
#define NETs_Join						NETs_PEERTOPEER_Join
#define NETs_Unjoin						NETs_PEERTOPEER_Unjoin
#define NETs_UpdateName					NETs_PEERTOPEER_UpdateName
#define NETs_MaintainNet				NETs_PEERTOPEER_MaintainNet

#endif


#endif // _NET_PEER_H_


