/*
 * PARSEC HEADER: net_serv.h
 */

#ifndef _NET_SERV_H_
#define _NET_SERV_H_


//NOTE:
// these functions are only needed by the GMSV protocol
// they are implemented by NET_SERV.C.


// server access functions

int		NET_ResolveServerNode();
int		NET_ServerConnect();
int		NET_ServerDisconnect();
void	NET_ServerDisconnectReset();
int 	NET_ServerJump();
void	NET_ServerMessages();
int 	NET_ServerUpdateName();
int 	NET_ServerParseMessage( char* msg );
void	NET_ServerProcessPingPacket( NetPacket_GMSV* gamepacket );
int		NET_ServerList_Get( char* masterhostname, int serverid = -1 );
int		NET_ServerList_AddServer( RE_IPv4ServerInfo* pServerInfo );
int		NET_ServerList_UpdateServer( RE_IPv4ServerInfo* pServerInfo );
int		NET_ServerList_AddLinkInfo( RE_ServerLinkInfo* pServerLinkInfo );
int		NET_ServerList_AddMapObject( RE_MapObject* pMapObject );
int		NET_ServerList_SetPONG( int serverid, refframe_t sendframe, int curplayers, int maxplayers );
int		NET_ServerList_SetINFO( int serverid, refframe_t framediff, int curplayers, int maxplayers, int srv_version_minor, int srv_version_major, char* srv_name );
int		NET_ServerPing( node_t* node, int anon, int fullinfo = FALSE );

// defaults for "SERVRLIST" command -------------------------------------------
//
#define DEFAULT_SERVERLIST_MIN_PING			0
#define DEFAULT_SERVERLIST_MAX_PING			9999
#define DEFAULT_SERVERLIST_MIN_PLAYERS		0
#define DEFAULT_SERVERLIST_MAX_PLAYERS		MAX_NET_ALLOC_SLOTS
#define DEFAULT_SERVERLIST_LOCALE			1

// all info about a "SERVERLIST" command --------------------------------------
//
struct serverlist_info_s {

	int			m_ListRequested;
	int			m_nMinPing;
	int			m_nMaxPing;
	int			m_nMinPlayers;
	int			m_nMaxPlayers;
	int			m_nLocale;

	word		m_RequestedServerIDs[ MAX_NUM_LINKS ];
	int			m_nNumRequestedServerIDs;

	serverlist_info_s()
	{
		Reset();
	}

	void Reset()
	{
		m_ListRequested = FALSE;
		m_nMinPing		= DEFAULT_SERVERLIST_MIN_PING;
		m_nMaxPing		= DEFAULT_SERVERLIST_MAX_PING;
		m_nMinPlayers	= DEFAULT_SERVERLIST_MIN_PLAYERS;
		m_nMaxPlayers	= DEFAULT_SERVERLIST_MAX_PLAYERS;
		m_nLocale		= DEFAULT_SERVERLIST_LOCALE;
		m_nNumRequestedServerIDs = 0;
	}
};

extern serverlist_info_s serverlist_info;

#endif // _NET_SERV_H_


