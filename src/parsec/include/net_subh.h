/*
 * PARSEC HEADER: net_subh.h
 */

#ifndef _NET_SUBH_H_
#define _NET_SUBH_H_


// ----------------------------------------------------------------------------
// NETWORKING SUBSYSTEM (NET) system-dependent function declarations          -
// ----------------------------------------------------------------------------


// remote event functions (RMEV)

int							NET_RmEvAllowed( int re_type );
size_t						NET_RmEvGetSize( RE_Header *relist );
int							NET_RmEvPlayerName( const char *name );
int							NET_RmEvSendText( const char *text );
int 						NET_RmEvObject( const GenObject *objpo );
int							NET_RmEvLaser( const LaserObject *laserpo );
int  						NET_RmEvMissile( const MissileObject *missilepo, dword targetobjid );
int  						NET_RmEvExtra( const ExtraObject *extrapo );
	int                                             NET_RmEvMine( const ExtraObject *extrapo );
int  						NET_RmEvParticleObject( int type, const Vertex3& origin );
int  						NET_RmEvKillObject( dword objid, byte listno );
int							NET_RmEvWeaponState( dword weapon, byte state, int energy, dword specials );
int							NET_RmEvStateSync( dword statekey, dword stateval );
int 						NET_RmEvCreateSwarm( Vector3 *origin, dword targetobjid, dword seed );
int 						NET_RmEvCreateEmp( int upgradelevel );
RE_PlayerStatus*			NET_RmEvPlayerStatus( byte last_unjoin_flag );
RE_PlayerAndShipStatus*		NET_RmEvPlayerAndShipStatus( byte last_unjoin_flag );
RE_ClientInfo*				NET_RmEvClientInfo( int nSendFreq, int nRecvRate );

void		NET_ExecRmEvCreateObject	( RE_Header *rmev, int ownerid );
void		NET_ExecRmEvCreateLaser		( RE_Header *rmev, int ownerid );
void		NET_ExecRmEvCreateMissile	( RE_Header *rmev, int ownerid );
void		NET_ExecRmEvParticleObject	( RE_Header *rmev, int ownerid );
void		NET_ExecRmEvCreateExtra		( RE_Header *rmev, int ownerid );
void		NET_ExecRmEvKillObject		( RE_Header *rmev );
void		NET_ExecRmEvSendText		( RE_Header *rmev, int ownerid );
void		NET_ExecRmEvPlayerName		( RE_Header *rmev, int ownerid );
void		NET_ExecRmEvWeaponState		( RE_Header *rmev, int ownerid );
void		NET_ExecRmEvStateSync		( RE_Header *rmev, int ownerid );
void		NET_ExecRmEvCreateSwarm		( RE_Header *rmev, int ownerid );
void		NET_ExecRmEvCreateEmp		( RE_Header *rmev, int ownerid );

void		NET_ExecRmEvPlayerAndShipStatus	( RE_PlayerAndShipStatus* pas_status, dword messageid );
void		NET_ExecRmEvGameState			( RE_GameState* gamestate );
void		NET_ExecRmEvKillStats			( RE_KillStats* killstats );
void		NET_ExecRmEvCommandInfo			( RE_CommandInfo* commandinfo );
void		NET_ExecRmEvCreateExtra2		( RE_Header *rmev, int ownerid );
void		NET_ExecRmEvIPv4ServerInfo		( RE_IPv4ServerInfo* pIPv4ServerInfo );
void		NET_ExecRmEvServerLinkInfo		( RE_ServerLinkInfo* pServerLinkInfo );
void		NET_ExecRmEvMapObject			( RE_MapObject* pMapObject );
void		NET_ExecRmEvStargate			( RE_Stargate* pRE_Stargate );
void		NET_ExecRmEvTeleporter			( RE_Teleporter* pRE_Teleporter );
	
// utility functions (UTIL)

int			NET_ProtocolPEER();
int			NET_ProtocolGMSV();

int			NET_ConnectedPEER();
int			NET_ConnectedGMSV();

char*		NET_FetchPlayerName( int playerid );
int			NET_SetPlayerName( const char *name );
ShipObject*	NET_FetchOwnersShip( int ownerid );
void 		NET_SetPlayerKillStat( int playerid, int amount );

int			NET_KillStatLimitReached( int limit );
void		NET_KillStatForceIdleZero();

void		NET_DrawEntryModeText();


#ifdef DBIND_PROTOCOL

	// protocol api functions

	#define NETs_Connect				(*net_subsys_jtab.Connect)
	#define NETs_Disconnect				(*net_subsys_jtab.Disconnect)
	#define NETs_Join					(*net_subsys_jtab.Join)
	#define NETs_Unjoin					(*net_subsys_jtab.Unjoin)

	#define NETs_UpdateName				(*net_subsys_jtab.UpdateName)

	#define NETs_MaintainNet			(*net_subsys_jtab.MaintainNet)

	// protocol api - game functions

	#define NETs_RmEvList_GetMaxSize	(*net_subsys_jtab.RmEvList_GetMaxSize)
	#define NETs_UpdateKillStats		(*net_subsys_jtab.UpdateKillStats)

	// protocol api - packet handling functions 

	#define NETs_HandleOutPacket				(*net_subsys_jtab.HandleOutPacket)
	#define NETs_HandleOutPacket_DEMO			(*net_subsys_jtab.HandleOutPacket_DEMO)
	#define NETs_HandleInPacket					(*net_subsys_jtab.HandleInPacket)
	#define NETs_HandleInPacket_DEMO			(*net_subsys_jtab.HandleInPacket_DEMO)
	#define NETs_NetPacketExternal_DEMO_GetSize (*net_subsys_jtab.NetPacketExternal_DEMO_GetSize)
	#define NETs_StdGameHeader					(*net_subsys_jtab.StdGameHeader)
	#define NETs_WritePacketInfo				(*net_subsys_jtab.WritePacketInfo)
	
#else // DBIND_PROTOCOL

	// protocol api functions

	int			NETs_Connect();
	int			NETs_Disconnect();
	int			NETs_Join();
	int			NETs_Unjoin( byte flag );

	int			NETs_UpdateName();

	void		NETs_MaintainNet();

	// protocol api - game functions
	
	size_t		NETs_RmEvList_GetMaxSize();
	void		NETs_UpdateKillStats( RE_KillStats* killstats );

	// protocol api - packet handling functions 
	
	size_t		NETs_HandleOutPacket		( const NetPacket*			int_gamepacket, NetPacketExternal* ext_gamepacket );
	size_t		NETs_HandleOutPacket_DEMO	( const NetPacket*			int_gamepacket, NetPacketExternal* ext_gamepacket );
	int			NETs_HandleInPacket			( const NetPacketExternal*	ext_gamepacket, const int ext_pktsize, NetPacket* int_gamepacket );
	int			NETs_HandleInPacket_DEMO	( const NetPacketExternal*	ext_gamepacket, NetPacket* int_gamepacket, size_t* psize_external );
	size_t		NETs_NetPacketExternal_DEMO_GetSize( NetPacketExternal* ext_gamepacket );
	void		NETs_StdGameHeader( byte command, NetPacket* gamepacket );
	void		NETs_WritePacketInfo( FILE *fp, NetPacketExternal* ext_gamepacket );
	
#endif // DBIND_PROTOCOL


// packet api functions

int			NETs_CompareNodes( node_t *node1, node_t *node2 );
int			NETs_VirtualNode( node_t *node );
void		NETs_SetVirtualAddress( node_t *node );
void		NETs_SetBroadcastAddress( node_t *node );

void		NETs_ResolveNode( node_t* node_dst, node_t* node_src );
void		NETs_MakeNodeRaw( node_t* node_dst, node_t* node_src );
node_t*		NETs_GetSender( int bufid );
void		NETs_ResolveSender( node_t *node, int bufid );

void		NETs_PrintNode( node_t *node );

void		NETs_FlushListenBuffers();
void		NETs_ProcessPacketChain	( void (*procfunc)(NetPacket* gamepacket,int bufid) );
int			NETs_InsertVirtualPacket( NetPacket* gamepacket );
int			NETs_DeterminePacketLoss( char **info, int *isiz, int incoming );

int			NETs_SleepUntilInput( int timeout_msec );

void		NETs_SendPacket			( NetPacket* gamepacket, node_t* node );
void		NETs_AuxSendPacket		( NetPacket* gamepacket, node_t* node );

int			NETs_ResetAPI();
int			NETs_InitAPI();
int			NETs_KillAPI();


#endif // _NET_SUBH_H_


