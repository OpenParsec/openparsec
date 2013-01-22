/*
 * PARSEC HEADER (OBJECT)
 * Rasterization Types V1.70
 *
 * Copyright (c) Markus Hadwiger 1995-1999
 * All Rights Reserved.
 */

#ifndef _OD_RAST_H_
#define _OD_RAST_H_


// coordinate on actual screen (integral component on the integer lattice) ----
//
typedef dword	ugrid_t;		// unsigned!
typedef int32_t	sgrid_t;		// signed!
typedef rastv_t	fgrid_t;


// 2-D point in unsigned integer coordinates (always >= 0) --------------------
//
struct UPoint {

	ugrid_t		X;				// unsigned!
	ugrid_t		Y;				// unsigned!
};


// 2-D point in screen coordinates (used in vertex list for the edge tracer) --
//
struct SPoint {

	sgrid_t		X;				// signed!
	sgrid_t		Y;				// signed!
};


// extended version of SPoint structure; contains z and color info ------------
//
struct SPointEx {

	sgrid_t		X;				// signed!
	sgrid_t		Y;				// signed!
	depth_t		View_Z; 		// Z coordinate in view space (for buffer fill)
	dword		RGBA;			// RGBA color (8-bit channels)
};


// 2-D point in screen coordinates with fractional part -----------------------
//
struct FPoint {

	fgrid_t		X;				// 8 bytes
	fgrid_t		Y;
};


// defines a polygon that should by rendered and drawn to the screen; ---------
// created after projection of 3-D polygon; scaled to screen coordinates;
// used only by edge tracer (must be filled from the object structure)
//
struct SVertexList {

	dword		VCount; 		// number of vertices in list
	SPoint		Vertices[ 1 ];	// first element of vertex list (array)
};


// list of extended SPoints ---------------------------------------------------
//
struct SVertexExList {

	dword		VCount;			// number of vertices in list
	SPointEx	Vertices[ 1 ];	// first element of vertex list (array)
};


// defines single horizontal span; used for horizontal span list structure ----
// the span starts at XLeft and ends at XRight (inclusively)
//
struct HSpan {

	ugrid_t		XLeft;			// size is 8 bytes
	ugrid_t		XRight;
};


// extended version of HSpan structure; contains z and lighting info ----------
//
struct HSpanEx {

	ugrid_t		XLeft;			// size is 16 bytes
	ugrid_t		XRight;
	word		ZLeft;
	word		ILeft;
	word		ZRight;
	word		IRight;
};


// stores list of horizontal spans; created by polygon edge-tracer; -----------
// used by low level polygon drawing/rendering functions
//
struct HSpanList {

	dword		Height; 		// length of list (height of bounding rectangle)
	ugrid_t		TopY;			// lowest y coordinate of polygon vertices
	HSpan		HSpans[ 1 ];	// first element of horizontal span list (array)
};


// list of extended HSpans ----------------------------------------------------
//
struct HSpanExList {

	dword		Height;			// length of list (height of bounding rectangle)
	ugrid_t		TopY;			// lowest y coordinate of polygon vertices
	HSpanEx 	HSpans[ 1 ];	// first element of horizontal span list (array)
};


#endif // _OD_RAST_H_


