/*
 * PARSEC HEADER: net_csdf.h
 */

#ifndef _NET_CSDF_H_
#define _NET_CSDF_H_


// parsec client/server protocol version number -------------------------------
//
#define CLSV_PROTOCOL_MAJOR			0
#define CLSV_PROTOCOL_MINOR			23
extern int clsv_protocol_minor_internal;

#ifdef PARSEC_CLIENT
	#include "net_udpdf.h"
#endif // PARSEC_CLIENT

// client -> gameserver
//FIXME: use const char here and get rid of vars defined in sv_glob.h
#define GIVECHALLSTRING1			"givechallenge"
//#define CONNSTRING1					"connreq %d.%d n %s os %s"
#define CONNSTRING2					"c2 %02d.%02d ch %d n %s cr %d sr %d os %s"
#define	RMVSTRING1					"rmvreq"
#define	NAMESTRING1					"namereq %s"
#define PINGSTRING1					"p %d"
#define INFOSTRING1					"reqinfo %d"
#define ACKJUMP						"ackjump"  // ACK a server jump

//FIXME: define these and use in RE_CommandInfo::code
/*#define PROT_GIVECHALLENGE_CODE		0
#define PROT_CONNECTREQUEST_CODE	0
#define PROT_GIVECHALLENGE_CODE		0
#define PROT_GIVECHALLENGE_CODE		0
#define PROT_GIVECHALLENGE_CODE		0
*/

// gameserver -> client
#define RECVSTR_CHALLENGE			"chall %d"
#define RECVSTR_ACCEPTED			"acchost %s:%d slot %d max %d svid %d srv %s"
#define RECVSTR_SERVER_FULL			"srvfull"
#define RECVSTR_REQUEST_INVALID		"reqinval"
#define RECVSTR_REMOVE_OK			"rmvok"
#define RECVSTR_NOT_CONNECTED		"nconn"
#define RECVSTR_SERVER_INCOMP		"srvincomp"
#define RECVSTR_CLIENT_BANNED		"banned"
#define RECVSTR_NAME_OK				"nameok"
#define RECVSTR_NAME_INVALID		"nameinval"
#define RECVSTR_LINK_SERVER			"linkserv %s:%d %d %d %d"
#define RECVSTR_CHALL_INVALID		"challinv"
#define RECVSTR_PING_REPLY			"p s %d r %d p %d/%d"
#define RECVSTR_INFO_REPLY			"info s %d r %d p %d/%d v %d.%d n %s"
#define LISTSTR_ADDED_IN_SLOT		"addslot %d nick %s"
#define LISTSTR_REMOVED_FROM_SLOT	"rmvslot %d"
#define LISTSTR_NAME_UPDATED		"updslot %d nick %s"
#define START_JUMP					"jump %s" // jump <server_ip>  A server telling the client it should jump
#define JUMP_OK						"jumpok"
#define JUMP_FAIL					"jumpfail"


// gameserver -> masterserver
#define MASV_CHALLSTRING			"c2 %02d.%02d ch %d n %s p %d/%d s %d os %s pt %d"
#define VERIFY_TRANS_PASS			"transpass %s uuid %s" // expects an ACKTRANS with the provided UUID

// gameserver -> gameserver
#define NEW_CLIENT					"newclient" // new player transfer from server to server, arg is master pass for the fed id
#define ACK_NEW_CLIENT				"acknewclient"


// masterserver -> gameserver
#define MASV_RESPONSE_NEW_CHALL		RECVSTR_CHALLENGE
#define ACKTRANS					"acktrans %s" // ACK a trans pass request.
#define NACKTRANS					"nacktrans" // Decline a tranfer request

// client -> masterserver
#define MASV_LIST					"list"
#define MASV_INFO					"info %d"


// ----------------------------------------------------------------------------
//
struct ipport_s {

	char ip[ MAX_IPADDR_LEN + 1 ];	// ip address
	byte portlo;					// low 8 bits of port
	byte porthi;					// high 8 bits of port
	word _pad;
};


// ----------------------------------------------------------------------------
//
struct client_s {

	// data
	int		slot_free;
	char	client_name[ MAX_PLAYER_NAME + 1 ];	// the nickname of the player

	// methods
	void Reset()
	{
		slot_free = TRUE;
		memset( client_name, 0, ( MAX_PLAYER_NAME + 1 ) * sizeof( char ) );
	}
};


// ----------------------------------------------------------------------------
//
struct server_s {

	// data
	char	slot_free;
	node_t	node;
	char	server_name[ MAX_SERVER_NAME + 1 ];			// the name of the server
	char	server_os[ MAX_OSNAME_LEN + 1 ];			// the server's operating system
	int		number_of_players;							// Number of clients joined at server
	int		max_players;								// how many players the server can handle
	char	server_url[ MAX_CFG_LINE + 1 ];
	int		xpos;										// x position of server in starmap
	int		ypos;										// y position of server in starmap
	int		serverid;									// global universe id of server
	int		major_version;
	int		minor_version;
	int		ping_in_ms;									// ping in ms
};


// struct defining a link between 2 servers -----------------------------------
//
struct link_s {

	// data
	int		serverid1;
	int		serverid2;
	int		flags;			// see SERVERLINKINFO_BOTH etc.
};


// struct defining a map object in the starmap --------------------------------
//
struct mapobj_s {

	char		name[ MAX_MAP_OBJ_NAME + 1 ];
	int  		xpos;
	int  		ypos;
	int	 		w;
	int  		h;
	char* 		texname;
	TextureMap* texmap;
};


#endif // _NET_CSDF_H_


