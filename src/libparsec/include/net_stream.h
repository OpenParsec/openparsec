/*
 * PARSEC HEADER: net_stream.h
 */

#ifndef _NET_STREAM_H_
#define _NET_STREAM_H_

// forward decls --------------------------------------------------------------
//
class E_REList;

// history buffer for received message ids to filter out duplicates -----------
//
#define MSGID_HISTORY_SIZE 64

// packet loss meter ----------------------------------------------------------
//
#define PACKET_LOSS_METER_LENGTH	100						// horizontal size

// flag to indicate that the packet does NOT include a reliable payload -------
//
#define NO_RELIABLE					0

// max. # of reliable RE lists to buffer --------------------------------------
//
#define MAX_NUM_RELIABLE_BACKLOG 256

// sructure of FIFO entries ---------------------------------------------------
//
struct StreamFIFOEntry_s {
	E_REList* m_pREList;
	refframe_t  m_Timeout;
	int			m_nRetransmitCount;

	StreamFIFOEntry_s() :
		m_pREList( NULL )
	{
		Reset();
	}
	
	void Reset();
	void InitEntry( E_REList* pREList );
};

// maintaining of message-ids -------------------------------------------------
//
class NET_Stream {
protected:

	dword		Out_MessageId;				// next out message
	dword		Out_ReliableMessageId;		// next reliable out message

	dword		I_ACK_MessageId;			// message I ACK from YOU
	dword		I_ACK_ReliableMessageId;	// reliable message I ACK from YOU

	dword		YOU_ACK_MessageId;			// message YOU ACK from ME
	dword		YOU_ACK_ReliableMessageId;	// reliable message YOU ACK from ME

	dword		m_MessageId_ReliableWasSent;

	bool_t		m_EnableReliable;
	int			m_nPeerID;
	int			m_nSenderID;

	bool_t		m_bIsConnected;

	// FIXME: STATS
	char		packet_graph_recv[ PACKET_LOSS_METER_LENGTH ];
	char		packet_graph_send[ PACKET_LOSS_METER_LENGTH ];

	int			message_id_history[ MSGID_HISTORY_SIZE ];

	int					m_nFIFO_WritePos;
	int					m_nFIFO_ReadPos;
	StreamFIFOEntry_s	m_FIFOEntries[ MAX_NUM_RELIABLE_BACKLOG ];

	refframe_t  m_ReliableRetransmit_Frames;	// timeout to retransmit reliable packets ( based on RTT ( see Stevens ) )

protected:
	int _FIFO_GetNextWritePos()		{ return ( m_nFIFO_WritePos + 1 ) % MAX_NUM_RELIABLE_BACKLOG; }
	int _FIFO_GetNextReadPos()		{ return ( m_nFIFO_ReadPos  + 1 ) % MAX_NUM_RELIABLE_BACKLOG; }

	// update the timeout for a specific FIFO entry
	void _FIFO_UpdateTimeOut( StreamFIFOEntry_s* pEntry );

public:

	// default ctor
	NET_Stream();

	// set the peer ID this stream is connected to
	void SetPeerID( int nPeerID ) { m_nPeerID = nPeerID; }

	// set the sender ID of this stream
	void SetSenderID( int nSenderID ) { m_nSenderID = nSenderID; }

	// reset the stream to defaults
	void Reset();

	// set the stream to connected mode ( accepts non datagrams )
	void SetConnected();

	// set whether we enable reliable for this stream
	void SetEnableReliable( bool_t enable ) { m_EnableReliable = enable; }

	// check whether a reliable message was already ACK
	bool_t IsReliableACK( dword dwMessage ) { return YOU_ACK_ReliableMessageId >= dwMessage; } 

	// check whether a message was already ACK
	bool_t IsACK( dword dwMessage ) { return YOU_ACK_MessageId >= dwMessage; } 

	// return the last message id we got an ACK for
	dword GetLastACKMessageId() { return YOU_ACK_MessageId; }

	// return the message# of the last packet received
	dword GetLastInPacket() { return I_ACK_MessageId; }

	// check input packet for rejection and maintain ACKs correctly
	int InPacket( NetPacket_GMSV* gamepacket_GMSV );

	// fill in fields in outgoing packet
	void OutPacket( NetPacket_GMSV* gamepacket_GMSV, int reliable = FALSE  );

	// check whether to filter out a duplicate packet
	int FilterPacketDuplicate( int messageid );

	// log packet loss statistics 
	void LogPacketLossStats( int numpackets, int packetok, int incoming );

	// append a RE list to the FIFO 
	int AppendToReliableFIFO( E_REList* pREList );

	// retrieve the next reliable RE list from FIFO to send
	E_REList* GetNextReliableToSend();

	// reset all reliable handling
	void FlushReliableBuffer();

	// return the next outgoing message id
	dword GetNextOutMessageId() { return Out_MessageId; }

	friend class NET_PacketDriver;
};


#endif // _NET_STREAM_H_


