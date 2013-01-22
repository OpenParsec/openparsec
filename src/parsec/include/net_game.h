/*
 * PARSEC HEADER: net_game.h
 */

#ifndef _NET_GAME_H_
#define _NET_GAME_H_

// value alive counter is set to immediately
// after a packet has been received
#define MAX_ALIVE_COUNTER		12000	// 20 seconds

// bufferno for virtual buffer
#define VIRTUAL_BUFFER_ID		-1		// packet is virtual (packet replay)

// ping packet type
#define PING_REQUEST			1		// request packet
#define PING_REPLY				0		// reply packet

// special ship ids
#define SHIPID_LOCALPLAYER		0		// ship belongs to local player
#define SHIPID_NOSHIP			-1		// there is no ship (player not joined)

// special slot ids
#define SLOTID_CONNECT_REFUSED	-1		// connection refused (all slots used)
#define SLOTID_DELETED			-2		// skip request in slot-request queue
#define SLOTID_LOCALHOST		-3		// slot id flag for local player
#define SLOTID_NOT_ASSIGNED		-4		// slot id has not been assigned yet

// connect/join status macros
#define REMOTE_PLAYER_ACTIVE(x)	( ( Player_Status[ x ] != PLAYER_INACTIVE ) && ( Player_ShipId[ x ] != SHIPID_LOCALPLAYER ) )
#define REMOTE_PLAYER_JOINED(x)	( ( Player_Status[ x ] == PLAYER_JOINED )   && ( Player_ShipId[ x ] != SHIPID_LOCALPLAYER ) )


// inline functions

inline
void CopyRemoteNode( node_t& dest, node_t& src )
{
	dest = src;
}

inline
void CopyRemoteName( char *dest, const char *src )
{
	strncpy( dest, src, MAX_PLAYER_NAME );
	dest[ MAX_PLAYER_NAME ] = 0;
}


// external functions

void	NET_ForceDisconnect();

void	NET_InitLocalPlayer();
void	NET_InitRemotePlayerTables();
void	NET_RemoveRemotePlayers();

void	NET_SetRemotePlayerState( int id, int status, int objclass, int killstat );
void	NET_SetRemotePlayerMatrix( int id, const Xmatrx matrix  );
void	NET_SetRemotePlayerName( int id, const char *name );

int		NET_RegisterRemotePlayer( int slotid, node_t *node, char *name );

void	NET_RmEvListReset();
void	NET_RmEvListUpdateLocations();

void	NET_RmEvSingleClear              ( RE_Header* relist );
void	NET_RmEvSinglePlayerName         ( RE_Header* relist );
void	NET_RmEvSinglePlayerStatus       ( RE_Header* relist, word desired_player_state, int unjoin_flag, int killer );
void	NET_RmEvSinglePlayerAndShipStatus( RE_Header* relist, word desired_player_state, int unjoin_flag, int killer );

void	NET_InterpolatePlayer( int playerid );
void	NET_FillShipRemInfo( ShipRemInfo *info, ShipObject *shippo, int playerid );
void	NET_ExecShipRemInfo( ShipRemInfo *info, ShipObject *shippo, int senderid );
void	NET_FreezeKillStats( int freeze );

void	NET_SetDesiredPlayerStatus( RE_PlayerAndShipStatus* pas_status );
void	NET_PlayerDisconnected( int playerid );
void	NET_RemovePlayer( int playerid );


// external variables

extern node_t			LocalNode;
extern node_t			LocalBroadcast;
extern node_t			Server_Node;
extern node_t			Player_Node[];
extern char				REListMem[];

#endif // _NET_GAME_H_


