/*
 * PARSEC HEADER: net_game_sv.h
 */

#ifndef _NET_GAME_SV_H_
#define _NET_GAME_SV_H_


// value alive counter is set to immediately
// after a packet has been received	(default 20s)
#define MAX_ALIVE_COUNTER				60 * 600

// bufferno for virtual buffer
#define VIRTUAL_BUFFER_ID		-1		// packet is virtual (packet replay)

// ping packet type
//#define PING_REQUEST			1		// request packet
//#define PING_REPLY				0		// reply packet

// special player ids
//#define PLAYERID_SERVER			-1		// packet destination is the server
//#define PLAYERID_ANONYMOUS		-2		// packet sender is not connected

// special ship ids
#define SHIPID_NOSHIP			-1		// there is no ship (player not joined)


#endif // _NET_GAME_SV_H_


