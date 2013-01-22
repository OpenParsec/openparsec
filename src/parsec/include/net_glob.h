/*
 * PARSEC HEADER: net_glob.h
 */

#ifndef _NET_GLOB_H_
#define _NET_GLOB_H_


// ----------------------------------------------------------------------------
// NETWORKING SUBSYSTEM (NET) related global declarations and definitions     -
// ----------------------------------------------------------------------------


// forward declarations -------------------------------------------------------
//
struct node_t;
struct RE_Header;
struct RE_KillStats;
struct NetPacket;
struct NetPacketExternal;
struct ShipRemInfo;
class NET_Stream;


extern int			LocalPlayerId;
extern char 		LocalPlayerName[];

extern int			NetAvailable;
extern int 			NetworkGameMask;
extern int			NetConnected;
extern int			NetJoined;
extern int			EntryMode;
extern int			ShipDowned;
extern int			Packet_Send_Frametime;
extern int			Packet_Send_Frequency;
extern int			Packet_Recv_Rate;
extern int			HaveFullPlayerState;		// indicates whether we have received a full playerstate from the server

extern refframe_t	ServerTimeoutFrames;		// # of refframes the server times out

extern int			NetInterfaceSelect;

extern char*		CurServerToResolve;
extern int			CurServerID;
extern char*		CurServerName;
extern int 			CurServerPing;
extern node_t*		CurJumpServerNode;
extern int 			CurMaxPlayers;
extern int			CurMaxDataLength;
extern int 			CurKillUpdate;
extern int 			CurKiller;
extern int			MyUniverse;
extern int			NumRemPlayers;
extern int			CurSendMessageId_PEER;
extern int 			CurGameTime;
extern int			ServerPingTime;

extern int			RE_List_Avail;
extern char*		RE_List_CurPos;

extern int			Player_Status[];
extern GenObject*	Player_Ship[];
extern int 			Player_ShipId[];
extern int 			Player_AliveCounter[];
extern int 			Player_UpToDate[];
extern int			Player_KillStat[];
extern int 			Player_LastMsgId[];
extern int			Player_LastUpdateGameStateMsgId[];
extern char			Player_Name[][ MAX_PLAYER_NAME + 1 ];

extern char*		Masters[];


// NETWORKING subsystem jump table for dynamic function binding ---------------
//
struct net_subsys_jtab_s {
	
	// PROTOCOL API
	
	int			(* Connect )		();
	int			(* Disconnect )		();
	int			(* Join )			();
	int			(* Unjoin )			( byte flag );
	
	int			(* UpdateName )		();
	
	void		(* MaintainNet )	();
	
	// PROTOCOL API - GAME
	
	size_t		(* RmEvList_GetMaxSize) ();
	void		(* UpdateKillStats) ( RE_KillStats* killstats );
	
	// PROTOCOL API - PACKET HANDLING
	
	size_t		(* HandleOutPacket)					( const NetPacket*			int_gamepacket,	NetPacketExternal*	ext_gamepacket );
	size_t		(* HandleOutPacket_DEMO )			( const NetPacket*			int_gamepacket,	NetPacketExternal*	ext_gamepacket );
	int			(* HandleInPacket)					( const NetPacketExternal*	ext_gamepacket,	const int ext_pktsize, NetPacket*			int_gamepacket );
	int			(* HandleInPacket_DEMO )			( const NetPacketExternal*  ext_gamepacket,	NetPacket*			int_gamepacket, size_t* psize_external );
	void		(* StdGameHeader)					( byte command,	NetPacket* gampacket );
	size_t		(* NetPacketExternal_DEMO_GetSize ) ( const NetPacketExternal*  ext_gamepacket );
	void		(* WritePacketInfo )				( FILE *fp, NetPacketExternal* ext_gamepacket );
	
	// PACKET API

	int			(* CompareNodes )( node_t *node1, node_t *node2 );
	int			(* VirtualNode )( node_t *node );
	void		(* SetVirtualAddress )( node_t *node );
	void		(* SetBroadcastAddress )( node_t *node );

	void		(* ResolveNode )( node_t* node_dst, node_t* node_src );
	void		(* MakeNodeRaw )( node_t* node_dst, node_t* node_src );
	node_t*		(* GetSender )( int bufid );
	void		(* ResolveSender )( node_t *node, int bufid );

	void		(* PrintNode )( node_t *node );

	void		(* ProcessPacketChain )	( void (*procfunc)( NetPacket* gamepacket, int bufid ) );
	void		(* FlushListenBuffers )();
	int			(* InsertVirtualPacket )( NetPacket* gamepacket );
	int			(* DeterminePacketLoss )( char **info, int *isiz, int incoming );

	int			(* SleepUntilInput) ( int timeout_msec );
	
	void		(* SendPacket )			( NetPacket *gamepacket, node_t *node );
	void		(* AuxSendPacket )		( NetPacket *gamepacket, node_t *node );

	int			(* ResetAPI )();
	int			(* InitAPI )();
	int			(* KillAPI )();

};

// exported by NET_GLOB.C -----------------------------------------------------
//
extern net_subsys_jtab_s net_subsys_jtab;

// exported by NET_UDP.C ------------------------------------------------------
//
extern NET_Stream ServerStream;

#endif // _NET_GLOB_H_


