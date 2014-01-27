/*
* PARSEC HEADER: NET_PacketDriver.h
*/

#ifndef _NET_PACKETDRIVER_H_
#define _NET_PACKETDRIVER_H_

#include "net_csdf.h"

// forward decls --------------------------------------------------------------
//
struct NetPacket_GMSV;
class NET_UDPDriver;
class NET_Stream;
class E_GameServer;
class E_PacketHandler;


// number of listen buffers in listen buffers array ---------------------------
//
#define NUM_LISTEN_BUFFERS		( MAX_NET_ALLOC_SLOTS * 2 )

// high level packet chain (preprocessed to meet ordering requirements) -------
//
struct PacketChainBlock {
	
	int 				bufferno;
	NetPacket_GMSV*		gamepacket;
	int 				messageid;
	PacketChainBlock*	nextblock;
};

struct PacketChainHead : PacketChainBlock {
	int chainlength;
};


// general packet handling function -------------------------------------------
//
class NET_PacketDriver
{
protected:

	NetPacketExternal_GMSV* SendNetPacketExternal;
	NetPacketExternal_GMSV* RecvNetPacketExternal;
	int						ListenStatus		[ NUM_LISTEN_BUFFERS ];
	sockaddr_in				ListenAddress		[ NUM_LISTEN_BUFFERS ];
	node_t					ListenSenderStorage	[ NUM_LISTEN_BUFFERS ];
	NetPacket_GMSV*			NetPacketsInternal	[ NUM_LISTEN_BUFFERS ];
	
	PacketChainHead*		ReceivedPacketsChain;

	NET_Stream*				m_Streams;			// add one player for PLAYERID_ANONYMOUS

protected:
	// retrieve next packet from received packets chain 
	PacketChainBlock* _FetchPacketFromChain();

	// allocate received packets chain (head node)
	int _AllocPacketsChain();
	
	// free all blocks in received packets chain 
	void _FreePacketsChain();

	// release block containing pointer and info of packet 
	void _ReleasePacketFromChain( PacketChainBlock* block );
	
	// pre-process received packets 
	void _BuildReceivedPacketsChain();
	
	// insert received packet into chain
	int _ChainPacket( NetPacket_GMSV* gamepacket, int bufid );
	
	// check regular player packets for duplicates 
	int _FilterPacketDuplicate( int senderid, int messageid );
	
	// process packet before inserting it into packet chain 
	void _PreprocessInPacket( NetPacket_GMSV* gamepacket );

	// log packet loss statistics 
	void _LogPacketLossStats( int numpackets, int packetok, int incoming );
	
	// fetch a packet into a listen buffer
	int _UDP_FetchPacket( int bufid );

	// send the protocol incompatible message to the client, using the right protocol ids so the client doesn't drop it.
	int _SendClientIncompatible(node_t* node, int nClientID, byte clientProtoMajor = CLSV_PROTOCOL_MAJOR, byte clientProtoMinor = CLSV_PROTOCOL_MINOR);

	// check that an incompatible packet is or isn't a challenge request.
	int _CheckIncompatibleChallenge(node_t *node);

	// determine if packet should be artificially dropped
	int _CheckForcePacketDrop();

	// record a packet
	void _RecordPacket( int bufid );

	NET_PacketDriver();
	virtual ~NET_PacketDriver();

public:

	// SINGLETON pattern
	static NET_PacketDriver* GetPacketDriver()
	{
		static NET_PacketDriver _ThePacketDriver;
		return &_ThePacketDriver;
	}

	// invoke processing function for all packets currently in chain 
	void NET_ProcessPacketChain();

	// retrieve sender's node address from listen header
	node_t* GetPktSender( int bufid );

	// send a packet to a node
	int SendPacket( NetPacket_GMSV* gamepacket, node_t* node, int nClientID, E_REList* pReliable, E_REList* pUnreliable );

	// send a datagram ( unreliable, unnumbered packet ) to a node
	int SendDatagram( NetPacket_GMSV* gamepacket, node_t* node, int nClientID, E_REList* pUnreliable, byte clientProtoMajor = CLSV_PROTOCOL_MAJOR, byte clientProtoMinor = CLSV_PROTOCOL_MINOR );

	// fill standard fields in gamepacket header
	void FillStdGameHeader( byte command, NetPacket_GMSV* gamepacket );

	// reset all data members
	void Reset();

	// reset the stream of a specific client
	void ResetStream( int nClientID );

	// connect the stream of a client
	void ConnectStream( int nClientID );

	// flush the reliable backbuffer of a stream
	void FlushReliableBuffer( int nClientID );

	// return the message id used in the next packet that is sent
	int GetNextOutMessageId( int nClientID );
		
	// get the stream message status
	NET_Stream* GetStream( int nClientID );

	friend class E_GameServer;
};

#endif // _NET_PACKETDRIVER_H_
