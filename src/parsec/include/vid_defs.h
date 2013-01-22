/*
 * PARSEC HEADER: vid_defs.h
 */

#ifndef _VID_DEFS_H_
#define _VID_DEFS_H_


// ----------------------------------------------------------------------------
// VIDEO SUBSYSTEM (VID) related definitions                                  -
// ----------------------------------------------------------------------------


#define MODE_COL_16BPP	16	// depth (in bits per pixel)
#define MODE_COL_32BPP	32	


// color mode ids irrespective of bit depth -----------------------------------
//
#define COLMODE_HICOLOR 	1
#define COLMODE_TRUECOLOR	2


// color mode dependent constants ---------------------------------------------
//
#define VGA_PAL_NUMCOLORS	256		// number of distinct colors in vga palette
#define VGA_PAL_SIZE		768		// size of vga palette in bytes
#define VGA_PAL_COLBITS		6		// number of bits per color in vga palette
#define VGA_PAL_FULLBRIGHT	63		// maximum value for vga color

#define HICOLOR_NUMCOLORS	65536L	// number of distinct colors in hicolor mode


// global palette identifiers -------------------------------------------------
//
#define PAL_GAME			( VGA_PAL_SIZE * 0 )
#define PAL_MENU			( VGA_PAL_SIZE * 1 )
#define PAL_BEYOND			( VGA_PAL_SIZE * 2 )	// has to be last!!

#define PAL_STANDARD		PAL_GAME
#define PAL_SHIP_VIEWER		PAL_GAME
#define PAL_OBJECT_VIEWER	PAL_GAME
#define PAL_CREDITS_SCROLL	PAL_MENU


// special color identifiers --------------------------------------------------
//
#define COL_BACKGROUND_BLACK	-2
#define COL_BACKGROUND_BLUE		-3


// special values for RGB color triplet (6 bits per color) --------------------
//
#define COLRGB_CHANNELBITS	6
#define COLRGB_FULLBRIGHT	0x3f
#define COLRGB_MUTECHANNEL	0x00
#define COLRGB_BLACK		{ COLRGB_MUTECHANNEL, COLRGB_MUTECHANNEL, COLRGB_MUTECHANNEL }
#define COLRGB_WHITE		{ COLRGB_FULLBRIGHT, COLRGB_FULLBRIGHT, COLRGB_FULLBRIGHT }


// special values for RGBA color (8 bits per color) ---------------------------
//
#define COLRGBA_CHANNELBITS	8
#define COLRGBA_FULLBRIGHT	0xff
#define COLRGBA_MUTECHANNEL	0x00
#define COLRGBA_FULLYOPAQUE	0xff
#define COLRGBA_TRANSPARENT 0x00
#define COLRGBA_BLACK		{ COLRGBA_MUTECHANNEL, COLRGBA_MUTECHANNEL, COLRGBA_MUTECHANNEL, COLRGBA_FULLYOPAQUE }
#define COLRGBA_WHITE		{ COLRGBA_FULLBRIGHT, COLRGBA_FULLBRIGHT, COLRGBA_FULLBRIGHT, COLRGBA_FULLYOPAQUE }


// wait for start of vertical retrace -----------------------------------------
//
#define AwaitVertSync()	{}


// include system-specific subsystem prototypes -------------------------------
//
#include "vid_subh.h"


#endif // _VID_DEFS_H_


