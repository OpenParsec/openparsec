/*
 * PARSEC HEADER: net_swap.h
 */

#ifndef _NET_SWAP_H_
#define _NET_SWAP_H_


// external functions ---------------------------------------------------------
//
void NET_RmEvList_Swap( RE_Header* relist, int incoming );
void SWAP_ShipRemInfo_in( ShipRemInfo *reminfo );
void SWAP_ShipRemInfo_out( ShipRemInfo *reminfo );

// define low-level network byte-order swapping functions
#define NET_SWAP_16		SWAP_16
#define NET_SWAP_32		SWAP_32

#endif


