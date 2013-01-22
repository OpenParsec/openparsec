/*
* PARSEC HEADER: e_packethandler.h
*/

#ifndef _E_PACKETHANDLER_H_
#define _E_PACKETHANDLER_H_


// forward decls --------------------------------------------------------------
//
struct PacketChainBlock;
class NET_PacketDriver;
class E_ClientChallengeInfo;
class E_ClientConnectInfo;
class E_REList;

// needed includes ------------------------------------------------------------
//
#include "e_connmanager.h"
#include "MasterServerItem.h"

#ifndef _NET_GAME_H_
	//FIXME: we MUST not depend on client module NET_GAME
	//#include "net_game_sv.h"
#endif // _NET_GAME_H_


// Server packet handler class ------------------------------------------------
//
class E_PacketHandler
{
protected:

	E_PacketHandler()
	{
	}

public:

	// SINGLETON pattern
	static E_PacketHandler* GetPacketHandler()
	{
		static E_PacketHandler _ThePacketHandler;
		return &_ThePacketHandler;
	}
	
	// main packet handling function
	void HandlePacket( PacketChainBlock* block );

	// send a challenge response back to a client
	void SendChallengeResponse( E_ClientChallengeInfo* pChallengeInfo, node_t* clientnode );

	// send a connect response back to a client
	int SendConnectResponse( E_ConnManager::ConnResults rc, E_ClientConnectInfo* pClientConnectInfo );

	// send a disconnect response back to a client
	int SendDisconnectResponse( E_ConnManager::DisconnResults rc, node_t* clientnode, int nClientID );

	// send a namechange response back to the client
	int SendNameChangeRepsponse( E_ConnManager::NameChangeResults rc, node_t* clientnode, int nClientID );

	// send a PING response back to the client
	int SendPingResponse( refframe_t client_ping_send_frame, node_t* clientnode, int nClientID );

	// send a INFO response back to the client
	int SendInfoResponse( refframe_t client_ping_send_frame, node_t* clientnode, int nClientID );

	// send IPV4 info to the client after a master server list command
	int SendIPV4Response( node_t* clientnode, int nClientID, int serverid );

	// notify a client that another client is connected
	void SendNotifyConnected( int nDestSlot, int nClientSlot );
	
	// notify a client that another client is disconnected
	void SendNotifyDisconnected( int nDestSlot, int nClientSlot );
	
	// notify a client that another client's name changed
	void SendNotifyNameChange( int nDestSlot, int nClientSlot );


	// send a STREAM packet to a client
	size_t Send_STREAM( int nClientID, E_REList* pReliable, E_REList* pUnreliable );

	// send a server command to a node
	int Send_COMMAND( char* clientcommand, node_t* node, int nClientID, int reliable );

	// send a datagram to a node
	int Send_STREAM_Datagram( E_REList* relist, node_t* node, int nClientID );

	// send a datagram command to a specific client node
	int Send_COMMAND_Datagram( const char* clientcommand, node_t* node, int nClientID );

	// multicast a packet to all connected clients
	//void Multicast_Packet( NetPacket_GMSV* gamepacket, int nSenderClientID );
	
protected:

	// handle a command packet
	void _Handle_COMMAND( NetPacket_GMSV* gamepacket, int bufid );

	// handle a packet containing a command from the masterserver
	void _Handle_COMMAND_MASV( NetPacket_GMSV* gamepacket, int bufid );

	// handle a packet containing a command FOR the master server.
	void _Handle_COMMAND_MASTER( NetPacket_GMSV* gamepacket, int bufid );

	// handle a stream packet
	void _Handle_STREAM( NetPacket_GMSV* gamepacket );

	// handle a stream packet for the master server
	void _Handle_STREAM_MASTER( NetPacket_GMSV* gamepacket, int bufid );

	void _Send_STREAM();

	// do safe parsing of challenge request
	int _ParseChallengeRequest( char* recvline );

	// do safe parsing of connect request
	int _ParseConnectRequest( char* recvline, E_ClientConnectInfo* pClientConnectInfo );

	// do safe parsing of disconnect request
	int _ParseDisconnectRequest( char* recvline );

	// do safe parsing of namechange request
	int _ParseNameChangeRequest( char* recvline, char* playername );

	// do safe parsing of PING request
	int _ParsePingRequest( char* recvline, refframe_t* client_ping_send_frame );

	// do safe parsing of INFO request
	int _ParseInfoRequest( char* recvline, refframe_t* client_ping_send_frame );

	// do safe parsing of LIST requests for Master Server Mode.
	int _ParseListRequest_MASTER( char* recvline, int *serverid);

	// do safe parsing of heartbeat packets for the Master Server.
	int _ParseHBPacket_MASTER(char* recvline);

	// check whether the sender node matches the node for the senderid
	int _IsLegitSender( NetPacket_GMSV* gamepacket, int bufid );

	// do safe parsing of new challenge from MASV
	int _ParseMASVChallenge( char* recvline, int* pChallenge );
};


#endif // _E_PACKETHANDLER_H_
