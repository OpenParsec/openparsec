/*
 * PARSEC HEADER: inps_defs.h
 */

#ifndef _INPS_DEFS_H_
#define _INPS_DEFS_H_


// ----------------------------------------------------------------------------
// INPUT SUBSYSTEM (INP) related definitions                                  -
// ----------------------------------------------------------------------------

// header of keyboard buffer
struct keybbuffer_s {

	dword	ReadPos;
	dword	WritePos;
	dword	Data;
};

#define KEYB_BUFF_SIZ				16


// global pointer to the keyboard buffer ( for console input ) ----------------
//
extern keybbuffer_s* KeybBuffer;

#endif // _INPS_DEFS_H_


