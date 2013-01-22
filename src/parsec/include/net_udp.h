/*
 * PARSEC HEADER: net_udp.h
 */

#ifndef _NET_UDP_H_
#define _NET_UDP_H_


// net_udp.c implements the following functions
// -------------------------------------------
//	int			NETs_CompareNodes( node_t *node1, node_t *node2 );
//	int			NETs_VirtualNode( node_t *node );
//	void		NETs_SetVirtualAddress( node_t *node );
//	void		NETs_SetBroadcastAddress( node_t *node );
//	void		NETs_ResolveNode( node_t *node, node_t *noderaw );
//	void		NETs_MakeNodeRaw( node_t *noderaw, node_t *node );
//	node_t*		NETs_GetSender( int bufid );
//	void		NETs_ResolveSender( node_t *node, int bufid );
//	void		NETs_PrintNode( node_t *node );
//	void		NETs_FlushListenBuffers();
//	void		NETs_ProcessPacketChain( void (*procfunc)(NetPacket* gamepacket,int bufid) );
//	int			NETs_InsertVirtualPacket( NetPacket* gamepacket );
//	int			NETs_DeterminePacketLoss( char **info, int *isiz, int incoming );
//	void		NETs_SendPacket( NetPacket* gamepacket, node_t* node );
//	void		NETs_AuxSendPacket( NetPacket* gamepacket, node_t* node );
//	int			NETs_ResetAPI();
//	int			NETs_InitAPI();
//	int			NETs_KillAPI();


#endif // _NET_UDP_H_


