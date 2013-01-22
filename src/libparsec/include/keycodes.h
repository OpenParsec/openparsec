/*
 * PARSEC HEADER: keycodes.h
 */

#ifndef _KEYCODES_H_
#define _KEYCODES_H_


//NOTE:
// this header is now just a wrapper for the
// file corresponding to the actual system

//NOTE:
// only on the mac the actual keycodes are adapted to the system.
// all other systems (e.g., Linux, IRIX) map native key codes to
// parsec key codes in their keyboard handlers. that is, from
// there on, the same key codes as on a pc are used.

#ifdef PARSEC_SERVER
#include "keys_server.h"
#else
#include "keys_sdl.h"
#endif


#endif // _KEYCODES_H_


