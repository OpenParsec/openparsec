/*
 * PARSEC HEADER: net_game_peer.h
 */

#ifndef _NET_GAME_PEER_H_
#define _NET_GAME_PEER_H_


// net_game_peer.c implements the following functions
// --------------------------------------------------
//  size_t	NETs_RmEvList_GetMaxSize();
//  void	NETs_UpdateKillStats( RE_KillStats* killstats );


// external functions ---------------------------------------------------------
//


// declare prototypes when we do not compile for shared lib gamecode support --
//
int		NET_AcquireRemoteSlot			( node_t *node );
int		NET_AddSlotRequest				( node_t *node, char *name, int timetag );
int		NET_DelSlotRequest				( node_t *node );
int		NET_MergeSlotRequests			( NetPacket_PEER* gamepacket, int timetag );
void	NET_ResetSlotReqQueue			();
void	NET_RmEvSinglePlayerTable		( NetPacket_PEER* gamepacket );
void	NET_RmEvSingleConnectQueue		( NetPacket_PEER* gamepacket );
void	NET_SetPacketKillStats			( NetPacket_PEER* gamepacket );
void	NET_Translate_PKTP_JOIN			( NetPacket_PEER* gamepacket, RE_PlayerAndShipStatus* pShipStatus );
void	NET_Translate_PKTP_GAME_STATE	( NetPacket_PEER* gamepacket, RE_PlayerAndShipStatus* pShipStatus, RE_KillStats* pKillStats );
void	NET_Translate_PKTP_UNJOIN		( NetPacket_PEER* gamepacket, RE_PlayerAndShipStatus* pShipStatus, RE_KillStats* pKillStats );
void	NET_Translate_PKTP_NODE_ALIVE	( NetPacket_PEER* gamepacket, RE_PlayerAndShipStatus* pShipStatus, RE_KillStats* pKillStats );


// special slotrequest timetags -----------------------------------------------
//
#define TIMETAG_LOCALHOST		-1
#define TIMETAG_REPLYNOW		-2


// slot request structure (entry in slot request queue) -----------------------
//
struct slotrequest_s {
	
	int			slotid;
	int			timetag;
	node_t		node;
	char		name[ MAX_PLAYER_NAME + 1 ];
};
	

// external variables ---------------------------------------------------------
//
extern int				NumSlotRequests;
extern slotrequest_s	SlotReqQueue[];


#endif // _NET_PEER_H_


