/*
 * PARSEC HEADER: net_defs.h
 */

#ifndef _NET_DEFS_H_
#define _NET_DEFS_H_


// ----------------------------------------------------------------------------
// NETWORKING SUBSYSTEM (NET) related definitions                             -
// ----------------------------------------------------------------------------


// possible values and masks of global NetworkGame ----------------------------
//
#define NETWORK_GAME_OFF		0x00
#define NETWORK_GAME_ON 		0x01
#define NETWORK_GAME_SIMULATED	0x02


// game management constants (special values of global CurGameTime) -----------
//
#define GAME_NOTSTARTEDYET		-1
#define GAME_FINISHED_TIME		-2
#define GAME_FINISHED_KILLS		-3
#define GAME_TERMINATED			-4
#define GAME_PEERTOPEER			-5

#define GAME_RUNNING()			( CurGameTime >= 0 )
#define GAME_OVER()				( ( CurGameTime == GAME_FINISHED_TIME ) || ( CurGameTime == GAME_FINISHED_KILLS ) )
#define GAME_NO_SERVER()		( CurGameTime == GAME_PEERTOPEER )


// identification of killer ---------------------------------------------------
//
#define KILLERID_UNKNOWN		0		// playerid of killer not known
#define KILLERID_BIAS			1		// playerid bias (additive)

// number of allocated network slots (absolute maximum number of players) -----
//
#define MAX_NET_ALLOC_SLOTS		16

// special player ids ---------------------------------------------------------
//
#define PLAYERID_SERVER			128		// packet destination is the server
#define PLAYERID_ANONYMOUS		-2		// packet sender is not connected
#define PLAYERID_MASTERSERVER	-3		// packet sender is masterserver
#define PLAYERID_INVALID		-4		// invalid sender

// maximum number of simultaneous network players in game-server mode ---------
//
#define MAX_NET_GMSV_PLAYERS	MAX_NET_ALLOC_SLOTS

// maximum number of simultaneous network players in peer-to-peer mode --------
//
#define MAX_NET_UDP_PEER_PLAYERS	8
#define MAX_NET_IPX_PEER_PLAYERS	4


// current maximum number of network players, depending on active protocol ----
//
#define MAX_NET_PROTO_PLAYERS	CurMaxPlayers	// declared in NET_GLOB.H


// node address compare results -----------------------------------------------
//
#define NODECMP_LESSTHAN		-1
#define NODECMP_EQUAL			0
#define NODECMP_GREATERTHAN		1


// message id for connectionless datagrams ------------------------------------
//
#define MSGID_DATAGRAM			0xFFFFFFFF

// size of packet signature ---------------------------------------------------
//
#define PACKET_SIGNATURE_SIZE	8


// maximum packet payload for ipx ---------------------------------------------
//
#define NET_IPX_DATA_LENGTH		546


// maximum packet payload for udp ---------------------------------------------
//
#define NET_UDP_DATA_LENGTH		1400


// current maximum packet payload ---------------------------------------------
//
#ifdef PARSEC_CLIENT
	#define NET_MAX_DATA_LENGTH		CurMaxDataLength
#else // !PARSEC_CLIENT
	#define NET_MAX_DATA_LENGTH		NET_UDP_DATA_LENGTH
#endif // !PARSEC_CLIENT


// max. length of internal packets --------------------------------------------
//
#define NET_MAX_NETPACKET_INTERNAL_LEN		2 * NET_UDP_DATA_LENGTH


// size of statically allocated packets ---------------------------------------
//
#define NET_ALLOC_DATA_LENGTH	NET_UDP_DATA_LENGTH


// size of remote packet stored into file (by recording it) -------------------
//
#define RECORD_PACKET_SIZE		NET_ALLOC_DATA_LENGTH


// maximum number of pending slot requests ------------------------------------
//
#define MAX_SLOT_REQUESTS		8


// maximum number of bytes in a remote address --------------------------------
// (regardless of protocol; accommodates IPv6)
#define MAX_NODE_ADDRESS_BYTES	16


// maximum number of specified masterservers ----------------------------------
//
#define MAX_MASTERSERVERS		3


// maximum number of servers (for serverlist and starmap) ---------------------
//
#define MAX_SERVERS				256


// maximum # of links from one server -----------------------------------------
//
#define MAX_NUM_LINKS			16

// maximum # of teleporters from one server
#define MAX_NUM_TELEP			32


// maximum # of map objects ---------------------------------------------------
//
#define MAX_MAP_OBJECTS			128

// max. length of map object names --------------------------------------------
//
#define MAX_MAP_OBJ_NAME 		31


// various size constants -----------------------------------------------------
//
#include "net_limits.h"


// encapsulate node address in portable manner --------------------------------
//
struct node_t {
	byte		address[ MAX_NODE_ADDRESS_BYTES ];
};


// status of local ship that is transmitted to remote players -----------------
//
struct ShipRemInfo {

	Xmatrx 		ObjPosition;		// 12 * 4 = 48
	word   		CurDamage;			// 2
	word   		CurShield;			// 2
	fixed_t		CurSpeed;			// 4
	bams_t 		CurYaw;				// 4
	bams_t 		CurPitch;			// 4
	bams_t 		CurRoll;			// 4
	geomv_t		CurSlideHorz;		// 4
	geomv_t		CurSlideVert;		// 4
	int         NumMissls;          // 4
	int         NumHomMissls;       // 4
	int         NumMines;           // 4
    int         NumPartMissls;      // 4
}; //92



// info about ship that should be created on remote host ----------------------
//
struct ShipCreateInfo {

	dword		ShipIndex;
	Xmatrx 		ObjPosition;
};


// server properties that are returned in ping packets ------------------------
//
struct ServerInfo {

	char 		name[ 32 ];
	byte		portlo;
	byte		porthi;
	byte		_padding[ 42 ];
};

//NOTE:
// sizeof( ServerInfo ) must be <= sizeof( ShipRemInfo )



// ----------------------------------------------------------------------------
// PEER protocol revisions:
// ----------------------------------------------------------------------------
//
// 0.1	build 0190		NetGameData
// 0.2  build 0196		( maxplayers set to 8 ) OldNetGameData = NetGameData from 0190
// 0.3  build 0198		complete new packet handling ( NetPacket{External} )
// 0.4  build 0198		CRC stored in packets

// parsec peer-to-peer protocol version number --------------------------------
//
#define P2P_PROTOCOL_MAJOR			0
#define P2P_PROTOCOL_MINOR			4


//NOTE: internal NetPackets are those that are used by all network/game functions
//      except the ones actually sending and receiving packets over the wire


//NOTE: As we want to have only some fields in the structure inherited, we
//      can not use standard C++ inheritance here. Instead we define the
//      structures such, that they are compatible in the (shared) sections.
//		The main reason for this is the variable sized remote event list

//NOTE: if you change anything in the "shared" sections of NetPacket, be sure
//		to do the same in NetPacket_GMSV and NetPacket_PEER

// common base structure for internal ( in-memory ) packet structure ( protocol independent )
//
struct NetPacket {

	//------------------------------------------------------------------------
	// header	( shared )									// = 128
	//------------------------------------------------------------------------
	int			SendPlayerId;								// 4
	dword		MessageId;									// 4

	int			Command;									// 4
	int			params[ 29 ];								// 116

	//------------------------------------------------------------------------
	// payload	( differs )									// = 128
	//------------------------------------------------------------------------
	
	byte		payload_pad[ 128 ];							// 128

	//------------------------------------------------------------------------
	// remote event list ( shared )							/// NET_MAX_DATA_LENGTH ( 1400 or 546 ) - 256 = 1144 or 290
	//------------------------------------------------------------------------
	//size_t		RE_ListSize;								// 4
	//dword		RE_List;									// 4 ( RE_Header + 2 )
	size_t test1;
	dword  test2;
};

// interal ( in-memory ) packet structure when using the gameserver protocol --
//
struct NetPacket_GMSV  {

	//------------------------------------------------------------------------
	// header	( shared )									// = 128
	//------------------------------------------------------------------------
	int			SendPlayerId;								// 4
	dword		MessageId;									// 4
	
	int			Command;									// 4		
//	int			removeme_params[ 29 ];						// 116		//FIXME: not needed in GMSV protocol
	
	//------------------------------------------------------------------------
	// payload	( differs )									// = 128
	//------------------------------------------------------------------------
	dword		ReliableMessageId;							// 4
	dword		AckMessageId;								// 4
	dword		AckReliableMessageId;						// 4

//	byte		payload_pad[ 128 - 3 * sizeof( dword ) ];	// 112		//FIXME: get rid of this 

	//------------------------------------------------------------------------
	// remote event list ( shared )
	//------------------------------------------------------------------------
	size_t		RE_ListSize;								// 4
	dword		RE_List;									// 4 ( RE_Header + 2 )
};


// interal ( in-memory ) packet structure when using the peer-to-peer protocol
//
struct NetPacket_PEER {

	//------------------------------------------------------------------------
	// header	( shared )									// = 128
	//------------------------------------------------------------------------
	int			SendPlayerId;								// 4
	dword		MessageId;									// 4
	
	int			Command;									// 4
	int			params[ 29 ];								// 116
	
	//------------------------------------------------------------------------
	// payload	( differs )									// = 128
	//------------------------------------------------------------------------
	int			DestPlayerId;								// 4
	
	byte		Universe;									// 1
	byte		NumPlayers;									// 1
	byte		_padding1[ 2 ];								// 2
	byte		PlayerKills[ MAX_NET_ALLOC_SLOTS ];			// 16
	
	ShipRemInfo ShipInfo;									// 76 
	
	int			GameTime;									// 4

	byte		payload_pad[ 128 - 104 ];					// 24

	//------------------------------------------------------------------------
	// remote event list ( shared )							/// NET_MAX_DATA_LENGTH ( 1400 or 546 ) - 256 = 1144 or 290
	//------------------------------------------------------------------------
	size_t		RE_ListSize;								// 4
	dword		RE_List;									// 4 ( RE_Header + 2 )
};

//NOTE: external NetPackets are those that are actually sent over the wire or
//      stored in recorded demos

// common base structure for external network packets -------------------------
//
struct NetPacketExternal {

	// header -------------------------
	char		Signature[ 1 ];
	
};

// external packet format for gameserver protocol -----------------------------
//
struct NetPacketExternal_GMSV : NetPacketExternal {

	// header -------------------------
	char		Signature2[ 3 ];
	word		Protocol;
	byte		MajorVersion;
	byte		MinorVersion;

	dword		crc32;
	//FIXME: padding ?????
	
	// payload ------------------------
	int			SendPlayerId;

	dword		MessageId;
	dword		ReliableMessageId;
	dword		AckMessageId;
	dword		AckReliableMessageId;

	byte		Command;
//	signed char removeme_param1; // do we need these in gameserver protocol??
//	signed char removeme_param2;
//	signed char removeme_param3;
//	int 		removeme_param4;

	dword		RE_List;
};


// external DEMO packet format for gameserver protocol ------------------------
//
typedef struct NetPacketExternal_GMSV NetPacketExternal_DEMO_GMSV;


// external packet format for peer-to-peer protocol ---------------------------
//
struct NetPacketExternal_PEER : NetPacketExternal {

	// header -------------------------
	char		Signature2[ 3 ];
	word		Protocol;
	byte		MajorVersion;
	byte		MinorVersion;

	dword		crc32;
	//FIXME: padding ?????

	// payload ------------------------
	dword		MessageId;

	byte		Command;
	signed char param1;
	signed char param2;
	signed char param3;
	int 		param4;

	byte		Universe;
	byte		NumPlayers;
	signed char SendPlayerId;
	signed char DestPlayerId;
	
	byte		PlayerKills[ MAX_NET_ALLOC_SLOTS ];

	ShipRemInfo ShipInfo;

	int			GameTime;

	dword		RE_List;
};

// external DEMO packet format for peer-to-peer protocol ----------------------
//
struct NetPacketExternal_DEMO_PEER : NetPacketExternal {

	// header -------------------------
	char		Signature2[ 3 ];
	word		Protocol;
	byte		MajorVersion;
	byte		MinorVersion;

	// additional header for DEMO -----
	size_t		pktsize;

	dword		crc32;						// CRC over the whole packet
	dword       crc_header;					// CRC over the header ( not including this field )
	
	// payload ------------------------
	dword		MessageId;
	
	byte		Command;
	signed char param1;
	signed char param2;
	signed char param3;
	int 		param4;
	
	byte		Universe;
	byte		NumPlayers;
	signed char SendPlayerId;
	signed char DestPlayerId;
	
	byte		PlayerKills[ MAX_NET_ALLOC_SLOTS ];
	
	ShipRemInfo ShipInfo;
	
	int			GameTime;
	
	dword		RE_List;
};

// external packet format for legacy recorded demos ( stored in demos <= 0190 ) -------
//
struct NetPacketExternal_DEMO_PEER_0190 : NetPacketExternal {

	// header -------------------------
	char		Signature2[ PACKET_SIGNATURE_SIZE - 1 ];

	// payload ------------------------
	dword		MessageId;

	byte		Command;
	signed char param1;
	signed char param2;
	signed char param3;
	int 		param4;

	byte		Universe;
	byte		NumPlayers;
	signed char SendPlayerId;
	signed char DestPlayerId;

	byte		PlayerKills[ MAX_NET_IPX_PEER_PLAYERS ];

	ShipRemInfo ShipInfo;

	int			GameTime;

	dword		RE_List;
};

//#define NETGAMEDATA_SIZE		NET_MAX_DATA_LENGTH

/*
#ifndef _NEW_NETPACKET

// game data for network transfer ---------------------------------------------
//
struct NetGameData {

	char		Signature[ PACKET_SIGNATURE_SIZE ];

	dword		MessageId;

	byte		Command;
	signed char param1;
	signed char param2;
	signed char param3;
	int 		param4;

	byte		Universe;
	byte		NumPlayers;
	signed char SendPlayerId;
	signed char DestPlayerId;

	byte		PlayerKills[ MAX_NET_ALLOC_SLOTS ];

	ShipRemInfo ShipInfo;

	int			GameTime;

	dword		RE_List;
};

// OLD game data for network transfer -----------------------------------------
//
struct OldNetGameData {

	char		Signature[ PACKET_SIGNATURE_SIZE ];

	dword		MessageId;

	byte		Command;
	signed char param1;
	signed char param2;
	signed char param3;
	int 		param4;

	byte		Universe;
	byte		NumPlayers;
	signed char SendPlayerId;
	signed char DestPlayerId;

	byte		PlayerKills[ MAX_NET_IPX_PEER_PLAYERS ];

	ShipRemInfo ShipInfo;

	int			GameTime;

	dword		RE_List;
};

#endif // !_NEW_NETPACKET
*/


// maximum available space for remote event-list ------------------------------
//
#define RE_LIST_MAXAVAIL		( NETs_RmEvList_GetMaxSize() )
#define RE_LIST_ALLOC_SIZE		( NET_ALLOC_DATA_LENGTH + sizeof( RE_Header ) )


// packet types (commands) ----------------------------------------------------
//
#define PKTP_CONNECT			0x00	// initial request for connection	(peer-to-peer only)
#define PKTP_CONNECT_REPLY		0x01	// reply to connect request			(peer-to-peer only)
#define PKTP_DISCONNECT			0x02	// disconnect entirely				(peer-to-peer only)
#define PKTP_SLOT_REQUEST		0x03	// slave requests slot				(peer-to-peer only)
#define PKTP_SUBDUE_SLAVE		0x04	// make sure slave subjects himself	(peer-to-peer only)

#define PKTP_JOIN				0x05	// join game (connection already up)		(peer-to-peer only)
#define PKTP_UNJOIN				0x06	// unjoin game (exit to entry-mode)			(peer-to-peer only)
#define PKTP_GAME_STATE			0x07	// current game state update ( joined )		(peer-to-peer only)
#define PKTP_NODE_ALIVE			0x08	// prevent being kicked out  ( unjoined )	(peer-to-peer only)
//#define PKTP_PING				0x09	// ping packet								(peer-to-peer only)

#define PKTP_COMMAND			0x20	// client/server command            ( C <-> S ) 
#define PKTP_STREAM				0x21    // client/server stream				( C <-> S )


// object list identifiers ----------------------------------------------------
//
#define SHIP_LIST				0x00
#define LASER_LIST				0x01
#define MISSL_LIST				0x02
#define EXTRA_LIST				0x03
#define CUSTM_LIST				0x04


// particle object types ------------------------------------------------------
//
#define POBJ_ENERGYFIELD		0x01
#define POBJ_MEGASHIELD			0x02
#define POBJ_SPREADFIRE			0x03


// weapon status flags --------------------------------------------------------
//
#define WPSTATE_OFF				0x00
#define WPSTATE_ON				0x01


// player connect/join status -------------------------------------------------
//
#define PLAYER_INACTIVE			0x00
#define PLAYER_CONNECTED		0x01
#define PLAYER_JOINED			0x03


// cause of return to entry-mode ----------------------------------------------
//
#define USER_EXIT				0x00
#define SHIP_DOWNED 			0x01


// flags for RE_Generic things.
enum re_generic_flags {
	AFTB_ACTIVE = 1,
	AFTB_INACTIVE,
	INVUNERABLE,
	TELEP_COLLIDE,
};


// remote event control blocks ------------------------------------------------
//
enum re_events {

	RE_EMPTY,			// 0x00
	RE_DELETED,			// 0x01
	RE_CREATEOBJECT, 		// 0x02
	RE_CREATELASER,		// 0x03
	RE_CREATEMISSILE,		// 0x04
	RE_CREATEEXTRA,		// 0x05
	RE_KILLOBJECT,		// 0x06
	RE_SENDTEXT, 			// 0x07
	RE_PLAYERNAME,		// 0x08
	RE_PARTICLEOBJECT,		// 0x09
	RE_PLAYERLIST,		// 0x0a
	RE_CONNECTQUEUE,		// 0x0b
	RE_WEAPONSTATE,		// 0x0c
	RE_STATESYNC,			// 0x0d
	RE_CREATESWARM,		// 0x0e
	RE_CREATEEMP,			// 0x0f
	RE_OWNERSECTION,		// 0x10		// 16
	RE_PLAYERSTATUS,		// 0x11		// 17
	RE_PLAYERANDSHIPSTATUS,	// 0x12		// 18
	RE_KILLSTATS,			// 0x13		// 19
	RE_GAMESTATE,			// 0x14		// 20
	RE_COMMANDINFO,		// 0x15		// 21
	RE_CLIENTINFO,		// 0x16		// 22
	RE_CREATEEXTRA2,		// 0x17		// 23 
	RE_IPV4SERVERINFO,		// 0x18		// 24
	RE_SERVERLINKINFO,		// 0x19		// 25
	RE_MAPOBJECT,			// 0x1a		// 26
	RE_STARGATE,			// 0x1b		// 27
    RE_CREATEMINE,
    RE_TELEPORTER,
    RE_GENERIC,
	RE_NUMEVENTS
};
	
// used if remote event size > 255
#define RE_BLOCKSIZE_INVALID	0x00

// header every block starts with
struct RE_Header {

	byte		RE_Type;      //1
	byte		RE_BlockSize; //1
}; //Size: 2

// generic object creation
struct RE_CreateObject : RE_Header { //2

	word		ObjectClass; //2
	dword		HostObjId; //4
	dword		Flags; //4
	Xmatrx		ObjPosition; //48
}; //Size: 60

// laser object creation
struct RE_CreateLaser : RE_Header { //2 

	word		ObjectClass; //2
	dword		HostObjId; //4
	Vertex3 	DirectionVec; //16
	Xmatrx		ObjPosition; //48
}; //72

// missile object creation
struct RE_CreateMissile : RE_Header { //2

	word		ObjectClass; //2
	dword		HostObjId; //4
	dword		TargetHostObjId; //4	
	Vertex3 	DirectionVec; //16
	Xmatrx		ObjPosition; //48
}; //76

// extra object creation
struct RE_CreateExtra : RE_Header { //2

	word		ExtraIndex; //2
	dword		HostObjId; //4
	Xmatrx		ObjPosition; //28
}; //36

// extra object creation
struct RE_CreateExtra2 : RE_Header { //2

	word		ExtraIndex; //2
	dword		HostObjId; //4
	int			DriftTimeout; //4
	Vector3     DriftVec; //16
	Xmatrx		ObjPosition; //48
}; //76

// Mine object creation
struct RE_CreateMine : RE_Header { //2
    word            ExtraIndex; //2
    dword           HostObjId; //4
    Xmatrx          ObjPosition; //48
}; //56

// object destruction 
struct RE_KillObject : RE_Header { //2

	byte		ListId; //1
	byte		Flags; //1
	dword		HostObjId; //4
}; //8

// arbitrary text
struct RE_SendText : RE_Header { //2

	char		TextStart[2];
}; //4 - 257

// player name
struct RE_PlayerName : RE_Header { //2

	char		PlayerName[ MAX_PLAYER_NAME + 1 ]; //32
}; //34

// particle object creation
struct RE_ParticleObject : RE_Header { //2

	word		ObjectType; //2
	Vertex3 	Origin; //16
}; //20

// list of all remote players
struct RE_PlayerList : RE_Header { //2

	byte			SyncValKillLimit;
	byte			SyncValNebulaId;
	byte			Status[ MAX_NET_IPX_PEER_PLAYERS ];
	node_t			AddressTable[ MAX_NET_IPX_PEER_PLAYERS ];
	ShipCreateInfo	ShipInfoTable[ MAX_NET_IPX_PEER_PLAYERS ];
	char			NameTable[MAX_NET_IPX_PEER_PLAYERS][MAX_PLAYER_NAME + 1];
};

// list of remote players trying to connect
struct RE_ConnectQueue : RE_Header {

	short		NumRequests;
	node_t		AddressTable[ MAX_SLOT_REQUESTS ];
	char		NameTable[ MAX_SLOT_REQUESTS ][ MAX_PLAYER_NAME + 1 ];
};

// weapon firing
struct RE_WeaponState : RE_Header { //2

	byte		State; //1
	byte		_dummy; //1
	dword		WeaponMask; //4
//	dword		Specials;		// would break demos
	int 		CurEnergy; //4
    int         SenderId; //4
}; //16

// state synchroniziation
struct RE_StateSync : RE_Header { //2

	byte		StateKey; //1
	byte		StateValue; //1
}; //4 (was 8)

// swarm missile creation
struct RE_CreateSwarm : RE_Header { //2
	dword		TargetHostObjId; //4
	dword		RandSeed; //4
    byte        SenderId; //4
	Vertex3		Origin; //16
}; //30

// emp creation
struct RE_CreateEmp : RE_Header { //2
	byte 			Upgradelevel; //1
	int			    SenderId; // 4
    char            pad[1]; //1
}; //8

// owner section
struct RE_OwnerSection : RE_Header { //2

	byte		owner; //1
    char        pad[1]; //1
}; //4

// playerstate
struct RE_PlayerStatus : RE_Header { //2
	byte		senderid; //1
	word		player_status;	//2
	signed char	params[ 4 ]; //4
	int		    objectindex; //4
	refframe_t	RefFrame; //4
    char        pad[1];
	// sizeof( RE_PlayerStatus ) 18
};	


#define UF_PROPERTIES		0x0001
#define UF_SPEEDS			0x0002
#define UF_RESYNCPOS		0x0004
#define UF_STATUS			0x0008
#define UF_ALL				0xFFFF

// full ship & player state 
struct RE_PlayerAndShipStatus : RE_PlayerStatus //18
{
	char        pad[1];         // 1
	byte 		NumMissls;      // 1
	byte        NumHomMissls;   // 1
	byte        NumMines;       // 1
	byte        NumPartMissls;  // 1
	byte		UpdateFlags;	// 1
	word   		CurDamage;		// 2
	word   		CurShield;		// 2
	fixed_t		CurSpeed;		// 4
	bams_t 		CurYaw;		    // 4
	bams_t 		CurPitch;		// 4
	bams_t 		CurRoll;		// 4
	geomv_t		CurSlideHorz;	// 4
	geomv_t		CurSlideVert;	// 4
	int			CurEnergy;		// 4
	Xmatrx 		ObjPosition;	// 48
	// sizeof( RE_PlayerAndShipStatus ) = (18) + 86 = 104
};


// killstats 
struct RE_KillStats : RE_Header { //2
	byte	PlayerKills[ MAX_NET_ALLOC_SLOTS ]; //16
	// 18
};

// gamestate
struct RE_GameState : RE_Header { //2
	int		GameTime; //4
	// 6
}; //12

// max. length of a command ---------------------------------------------------
#define MAX_RE_COMMANDINFO_COMMAND_LEN		254 - 1 - 2 /*header*/ - 1 /*code*/

// command ( GMSV mode only )
struct RE_CommandInfo : RE_Header { //2
	byte code; //1
	//char command[ MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ];
	char command[1]; //?
}; //4 to 250

#define PACK_SERVERRATE( r )				( (byte)(r / 100) )
#define UNPACK_SERVERRATE( r )				( (r) * 100 )

// clientinfo ( GMSV mode only )
struct RE_ClientInfo : RE_Header {
	byte client_sendfreq;		// packets per sec.
	byte server_sendrate;		// bytes per sec.
	// 4
};


// serverinfo about a IP4 server 
struct RE_IPv4ServerInfo : RE_Header {
	byte	node[ 6 ];
	word	flags;
	word	serverid;
	int		xpos;
	int		ypos;

	// sizeof( RE_IPv4ServerInfo ) = (2) + 6 + 2 + 2 + 4 + 4 = 20
};

// serverlink info flags ------------------------------------------------------
//
#define SERVERLINKINFO_1_TO_2	0x01		// link is  1 --> 2
#define SERVERLINKINFO_2_TO_1	0x02		// link is  1 <-- 2
#define SERVERLINKINFO_BOTH		0x03		// link is  1 <-> 2

// linkinfo between 2 servers -------------------------------------------------
//
struct RE_ServerLinkInfo : RE_Header {
	word	flags;
	word	serverid1;
	word	serverid2;

	// sizeof( RE_ServerLinkInfo ) = (2) + 2 + 2 + 2 = 8
};


// map object ----------------------------------------------------------------- 
//
struct RE_MapObject : RE_Header {
	word		map_objectid;
	char		name[ MAX_MAP_OBJ_NAME + 1 ];
	int  		xpos;
	int  		ypos;
	int	 		w;
	int  		h;
	char 		texname[ MAX_TEXNAME + 1 ];

	// sizeof( RE_MapObject ) = (2) + 2 + 32 + 4 + 4 + 4 + 4 + 32 = 84
};

// stargate properties --------------------------------------------------------
//
struct RE_Stargate : RE_Header {
	word	serverid;			// 2
	float	pos[ 3 ];			// 12
	float	dir[ 3 ];			// 12
	bams_t	rotspeed;			// 4
	float radius;				// 4
	float	actdistance;		// 4
	float	partvel;			// 4
	float	modulrad1;			// 4
	float	modulrad2;			// 4
	char	flare_name		[ MAX_TEXNAME     + 1 ]; // 32
	char	interior_name	[ MAX_TEXNAME	  + 1 ]; // 32
	word	numpartactive;		// 2
	word	actcyllen;			// 2
	word	modulspeed;			// 2
	byte	acttime;			// 1
	byte	autoactivate;		// 1
	byte	dormant;			// 1
	byte	active;				// 1
	byte	_mksz128[ 2 ];		// 2

	// sizeof( RE_Stargate ) = 128
};

// teleporter properties --------------------------------------------------------
//
struct RE_Teleporter : RE_Header {
	word id;			// 2
	float	pos[ 3 ];			// 12
	float	dir[ 3 ];			// 12
	geomv_t	exit_delta_x; // 4
	geomv_t 	exit_delta_y; // 4
	geomv_t	exit_delta_z; // 4
	float exit_rot_phi; // 4
	float exit_rot_theta; // 4

	// sizeof( RE_Teleporter ) = 40
};

struct RE_Generic: RE_Header{ //2
    word        RE_ActionFlags; //2
    dword       HostObjId; //4  - the object spawning the event
//  Xmatrx      ObjPos; //48 - probably don't need this, we'll see.
    dword       TargetId; //4 - can be used for targetting events
    dword       Padding; //4 - can be used for anything you desire.
}; //Size: 16 bytes


//NOTE:
// new remote event structures must ensure proper alignment
// after the two leading header bytes, which usually means
// using two single bytes or a word as first field.


// make object id unique in network game by appending local player's id -------
//
#ifndef PARSEC_MASTER

	#ifdef PARSEC_SERVER

		inline dword CreateGlobalObjId( dword localobjid, dword dwOwner )
		{
			ASSERT( ( ( dwOwner >= 0 ) && ( dwOwner < MAX_NET_ALLOC_SLOTS ) ) || ( dwOwner == PLAYERID_SERVER ) );
			return ( ( dwOwner << 16 ) | ( localobjid & 0xffff ) );
		}

	#else

		inline dword CreateGlobalObjId( dword localobjid )
		{
			ASSERT( !NetConnected || ( ( LocalPlayerId >= 0 ) && ( LocalPlayerId < MAX_NET_ALLOC_SLOTS ) ) );
			return ( ( LocalPlayerId << 16 ) | ( localobjid & 0xffff ) );
		}

	#endif // PARSEC_SERVER

#endif // PARSEC_MASTER


// calc host object-id for ship of remote player (ship object id is 0) --------
//
//#define ShipHostObjId( playerid )			( playerid << 16 )
inline dword ShipHostObjId( dword playerid ) 
{ 
	return ( playerid << 16 ); 
}


// retrieve owner from HostOjbNumber ------------------------------------------
// 
inline dword GetOwnerFromHostOjbNumber( dword _HostObjNumber )
{
	return _HostObjNumber >> 16;
}

// determine owner of object (remote player id) -------------------------------
//
inline dword GetObjectOwner( const GenObject *obj )
{
	return GetOwnerFromHostOjbNumber( obj->HostObjNumber );
}

#ifdef PARSEC_CLIENT

	// include system-specific subsystem prototypes
	#include "net_subh.h"

#else // !PARSEC_CLIENT

	// include system-specific subsystem prototypes
	#include "net_subh_sv.h"

#endif // !PARSEC_CLIENT


#endif // _NET_DEFS_H_


