/*
 * PARSEC HEADER: gd_const.h
 */

#ifndef _GD_CONST_H_
#define _GD_CONST_H_


// ----------------------------------------------------------------------------
// GLOBAL CONSTANTS                                                           -
// ----------------------------------------------------------------------------


// frequency in hertz for ref-frames (number of ref-frames per second) --------
//
#define DEFAULT_REFFRAME_FREQUENCY		600

#ifdef VARIABLE_REFFRAME_FREQUENCY
	#define FRAME_MEASURE_TIMEBASE		RefFrameFrequency
#else
	#define FRAME_MEASURE_TIMEBASE		DEFAULT_REFFRAME_FREQUENCY
#endif


// size of padding area appended to render segment ----------------------------
//
#define RSEG_PADDING_SIZE				(640*20)


// packet recv rates (bytes/sec) ----------------------------------------------
//
#define CLIENT_RECV_RATE_MAX			100000		//FIXME: reasonable ?
#define CLIENT_RECV_RATE_1				8000
#define CLIENT_RECV_RATE_2				4000
#define CLIENT_RECV_RATE_3				2000
#define CLIENT_RECV_RATE_MIN			10			//FIXME: reasonable ?

// default rate (bytes/sec) for S->C traffic ----------------------------------
//
#define DEFAULT_CLIENT_RECV_RATE		CLIENT_RECV_RATE_1

// packet send frequencies ----------------------------------------------------
//
#define CLIENT_SEND_FREQUENCY_MAX		100			//FIXME: reasonable ?
#define CLIENT_SEND_FREQUENCY_1			20
#define CLIENT_SEND_FREQUENCY_2 		15
#define CLIENT_SEND_FREQUENCY_3 		10
#define CLIENT_SEND_FREQUENCY_MIN		1

// default frequency (packets/sec) for C->S traffic ---------------------------
//
#define DEFAULT_CLIENT_SEND_FREQUENCY	CLIENT_SEND_FREQUENCY_1

// detail codes ---------------------------------------------------------------
//
#define DETAIL_LOW						0
#define DETAIL_MEDIUM					1
#define DETAIL_HIGH 					2


#endif // _GD_CONST_H_


