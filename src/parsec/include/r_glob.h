/*
 * PARSEC HEADER: r_glob.h
 */

#ifndef _R_GLOB_H_
#define _R_GLOB_H_


// ----------------------------------------------------------------------------
// RENDERING SUBSYSTEM (R) related global declarations and definitions        -
// ----------------------------------------------------------------------------


// rasterization performance statistics

struct rastperfstats_s {

	dword	fieldstate;

	dword	fragments;
	dword	pixels;
	dword	faildepth;
	dword	failalpha;
	dword	failchroma;
};

#define RASTPERF_NONE			0x0000
#define RASTPERF_FRAGMENTS		0x0001
#define RASTPERF_PIXELS			0x0002
#define RASTPERF_FAILDEPTH		0x0004
#define RASTPERF_FAILALPHA		0x0008
#define RASTPERF_FAILCHROMA		0x0010


// geometry performance statistics

struct geomperfstats_s {

	dword	fieldstate;

	dword	vertices_in;
	dword	vertices_out;
	dword	polygons_in;
	dword	polygons_out;
	dword	triangles_in;
	dword	triangles_out;
	dword	edges;
	dword	spans_in;
	dword	spans_out;
};

#define GEOMPERF_NONE			0x0000
#define GEOMPERF_VERTICES_IN	0x0001
#define GEOMPERF_VERTICES_OUT	0x0002
#define GEOMPERF_POLYGONS_IN	0x0004
#define GEOMPERF_POLYGONS_OUT	0x0008
#define GEOMPERF_TRIANGLES_IN	0x0010
#define GEOMPERF_TRIANGLES_OUT	0x0020
#define GEOMPERF_EDGES			0x0040
#define GEOMPERF_SPANS_IN		0x0080
#define GEOMPERF_SPANS_OUT		0x0100


// make definition of pcluster pointers possible

struct pcluster_s;


#endif // _R_GLOB_H_


