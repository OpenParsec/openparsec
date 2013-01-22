/*
 * PARSEC HEADER: net_subh_sv.h
 */

#ifndef _NET_SUBH_SV_H_
#define _NET_SUBH_SV_H_


// ----------------------------------------------------------------------------
// NETWORKING SUBSYSTEM (NET) system-dependent function declarations          -
// ----------------------------------------------------------------------------
//#define NULL 0
#include "e_relist.h"

//// remote event functions (RMEV)
//
#define NET_RmEvAllowed				TheSimulator->GetInputREList()->RmEvAllowed
#define NET_RmEvGetSize				E_REList::RmEvGetSize
#define NETs_RmEvList_GetMaxSize	E_REList::GetMaxSizeInPacket
//int				NET_RmEvPlayerName( char *name );
//int				NET_RmEvSendText( const char *text );
//int 			NET_RmEvObject( const GenObject *objpo );
//int				NET_RmEvLaser( const LaserObject *laserpo );
//int  			NET_RmEvMissile( const MissileObject *missilepo, dword targetobjid );
//int  			NET_RmEvExtra( const ExtraObject *extrapo );
//int  			NET_RmEvParticleObject( int type, const Vertex3& origin );
//int  			NET_RmEvKillObject( dword objid, byte listno );
//int				NET_RmEvWeaponState( dword weapon, byte state, int energy, dword specials );
#define NET_RmEvStateSync	TheSimulator->GetInputREList()->RmEvStateSync
//int 			NET_RmEvCreateSwarm( Vector3 *origin, dword targetobjid, dword seed );
//int 			NET_RmEvCreateEmp( int upgradelevel );
//RE_PlayerAndShipStatus*  NET_RmEvShipStatus( byte last_unjoin_flag );
//
//
//void		NET_ExecRmEvCreateObject	( RE_Header *rmev, int senderid );
//void		NET_ExecRmEvCreateLaser		( RE_Header *rmev, int senderid );
//void		NET_ExecRmEvCreateMissile	( RE_Header *rmev, int senderid );
//void		NET_ExecRmEvParticleObject	( RE_Header *rmev, int senderid );
//void		NET_ExecRmEvCreateExtra		( RE_Header *rmev, int senderid );
//void		NET_ExecRmEvKillObject		( RE_Header *rmev );
//void		NET_ExecRmEvSendText		( RE_Header *rmev, int senderid );
//void		NET_ExecRmEvPlayerName		( RE_Header *rmev, int senderid );
//void		NET_ExecRmEvWeaponState		( RE_Header *rmev, int senderid );
//void		NET_ExecRmEvStateSync		( RE_Header *rmev, int senderid );
//void		NET_ExecRmEvCreateSwarm		( RE_Header *rmev, int senderid );
//void		NET_ExecRmEvCreateEmp		( RE_Header *rmev, int senderid );
//
//void		NET_ExecRmEvGameState		( RE_GameState* gamestate );
//void		NET_ExecRmEvKillStats		( RE_KillStats* killstats );
//void		NET_ExecRmEvCommandInfo		( RE_CommandInfo* commandinfo );
//	
//// utility functions (UTIL)
//
//int			NET_ProtocolPEER();
//int			NET_ProtocolGMSV();
//
//int			NET_ConnectedPEER();
//int			NET_ConnectedGMSV();
//
//char*		NET_FetchPlayerName( int playerid );
//int			NET_SetPlayerName( char *name );
//ShipObject*	NET_FetchOwnersShip( int ownerid );
//void 		NET_SetPlayerKillStat( int playerid, int amount );
//
//int			NET_KillStatLimitReached( int limit );
//void		NET_KillStatForceIdleZero();
//
//void		NET_DrawEntryModeText();
//
//
//// protocol api functions
//
//int			NETs_Connect();
//int			NETs_Disconnect();
//int			NETs_Join();
//int			NETs_Unjoin( byte flag );
//
//int			NETs_UpdateName();
//
//void		NETs_MaintainNet();
//
//// protocol api - game functions
//
//size_t		NETs_RmEvList_GetMaxSize();
//void		NETs_UpdateKillStats( RE_KillStats* killstats );
//
// protocol api - packet handling functions 

size_t		NETs_HandleOutPacket		( const NetPacket*			int_gamepacket, NetPacketExternal* ext_gamepacket );
//size_t		NETs_HandleOutPacket_DEMO	( const NetPacket*			int_gamepacket, NetPacketExternal* ext_gamepacket );
int			NETs_HandleInPacket			( const NetPacketExternal*	ext_gamepacket, const int ext_pktsize, NetPacket* int_gamepacket );
//int			NETs_HandleInPacket_DEMO	( const NetPacketExternal*	ext_gamepacket, NetPacket* int_gamepacket, size_t* psize_external );
//size_t		NETs_NetPacketExternal_DEMO_GetSize( NetPacketExternal* ext_gamepacket );
//void		NETs_StdGameHeader( byte command, NetPacket* gamepacket );
//void		NETs_WritePacketInfo( FILE *fp, NetPacketExternal* ext_gamepacket );
//
//// packet api functions
//
//int			NETs_CompareNodes( node_t *node1, node_t *node2 );
//int			NETs_VirtualNode( node_t *node );
//void		NETs_SetVirtualAddress( node_t *node );
//void		NETs_SetBroadcastAddress( node_t *node );
//
//void		NETs_ResolveNode( node_t* node_dst, node_t* node_src );
//void		NETs_MakeNodeRaw( node_t* node_dst, node_t* node_src );
//node_t*	NETs_GetSender( int bufid );
//void		NETs_ResolveSender( node_t *node, int bufid );
//
//void		NETs_PrintNode( node_t *node );
//
//void		NETs_FlushListenBuffers();
//
//int			NETs_InsertVirtualPacket( NetPacket* gamepacket );
//int			NETs_DeterminePacketLoss( char **info, int *isiz, int incoming );
//
//void		NETs_SendPacket			( NetPacket* gamepacket, node_t* node );
//void		NETs_AuxSendPacket		( NetPacket* gamepacket, node_t* node );
//
//void		NETs_ProcessPacketChain	( void (*procfunc)(NetPacket* gamepacket,int bufid) );
//
//int			NETs_ResetAPI();
//int			NETs_InitAPI();
//int			NETs_KillAPI();


#endif // _NET_SUBH_SV_H_


