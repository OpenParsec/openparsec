/*
 * PARSEC HEADER: net_pckt.h
 */

#ifndef _NET_PCKT_H_
#define _NET_PCKT_H_

// net_pckt.c implements the following functions
// ---------------------------------------------


// external functions ---------------------------------------------------------
//


// protocol type --------------------------------------------------------------
//
#define PROTOCOL_PEER2PEER				0x0100
#define PROTOCOL_GAMESERVER				0x0200
#define PROTOCOL_ENCRYPTED				0xF000

// protocol type signature ----------------------------------------------------
//
extern const char net_game_signature[];
extern const char net_cryp_signature[];
extern const char net_packet_signature_gameserver[];
extern const char net_packet_signature_peer2peer[];

// signature lengths ----------------------------------------------------------
//
#define SIGNATRUE_LEN_OLD				PACKET_SIGNATURE_SIZE
#define SIGNATURE_LEN_GAMESERVER		3
#define SIGNATURE_LEN_PEER2PEER			3


#endif // _NET_PCKT_H_


