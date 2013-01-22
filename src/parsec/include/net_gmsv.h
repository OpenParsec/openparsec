/*
 * PARSEC HEADER: net_gmsv.h
 */

#ifndef _NET_GMSV_H_
#define _NET_GMSV_H_


// net_gmsv.c implements the following functions
// ---------------------------------------------
//	int		NETs_Connect( char *server );
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

	#define NETs_Connect					NETs_GAMESERVER_Connect
	#define NETs_Disconnect					NETs_GAMESERVER_Disconnect
	#define NETs_Join						NETs_GAMESERVER_Join
	#define NETs_Unjoin						NETs_GAMESERVER_Unjoin
	#define NETs_UpdateName					NETs_GAMESERVER_UpdateName
	#define NETs_MaintainNet				NETs_GAMESERVER_MaintainNet

#endif // DBIND_PROTOCOL


// external functions ---------------------------------------------------------
//
void NET_ProcPacketLoop_GMSV( NetPacket* int_gamepacket, int bufid );
void NET_ProcPacketLoop_Disconnected_GMSV( NetPacket* int_gamepacket, int bufid );


#endif // _NET_GMSV_H_


