//-----------------------------------------------------------------------------
//	BSPLIB HEADER: BspLibDefs.h
//
//  Copyright (c) 1996-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _BSPLIBDEFS_H_
#define _BSPLIBDEFS_H_


// standard C library headers -------------------------------------------------
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>


// various build dependent macros ---------------------------------------------
#ifdef _DEBUG

#define PUBLIC
#define PRIVATE static
#define D(x) x
#define CHECK_DEREFERENCING(x) x

#else

#define PUBLIC
#define PRIVATE static
#define D(x)
#define CHECK_DEREFERENCING(x) x

#endif


// define NAMESPACE usage -----------------------------------------------------
#define USE_BSPLIB_NAMESPACE

#ifdef USE_BSPLIB_NAMESPACE

#define BSPLIB_NAMESPACE BspLib
#define BSPLIB_NAMESPACE_BEGIN namespace BSPLIB_NAMESPACE {
#define BSPLIB_NAMESPACE_END }

#else

#define BSPLIB_NAMESPACE
#define BSPLIB_NAMESPACE_BEGIN
#define BSPLIB_NAMESPACE_END

#endif


#ifndef _WIN32
	#define stricmp strcasecmp
	#define strnicmp strncasecmp
#endif


// MS VC++ specific macros ----------------------------------------------------
#ifdef _MSC_VER
#define PATH_MAX				_MAX_PATH
#endif
#define _CRT_SECURE_NO_WARNINGS 1


// boolean values -------------------------------------------------------------
#define TRUE					1
#define FALSE					0

// asm types ------------------------------------------------------------------
typedef uint8_t					byte;
typedef uint16_t				word;
typedef uint32_t				dword;

// data types for coordinates -------------------------------------------------
typedef int32_t					fixed_t;
typedef double					hprec_t;

// conversion macros for 3 and 2 digit values ---------------------------------
#define DIG3_TO_STR( s, a ) (s)[0]=(((a)/100)%10)+'0';\
							(s)[1]=(((a)/10)%10)+'0'; \
							(s)[2]=((a)%10)+'0';      \
							(s)[3]='\0';

#define DIG2_TO_STR( s, a ) (s)[0]=(((a)/10)%10)+'0'; \
							(s)[1]=((a)%10)+'0';      \
							(s)[2]='\0';


// some general structures ----------------------------------------------------
//
BSPLIB_NAMESPACE_BEGIN


// RGB color triplet using bytes as members -----------------------------------
struct ColorRGB {
	byte R;
	byte G;
	byte B;
};

// RGB color triplet plus alpha using bytes as members ------------------------
struct ColorRGBA {
	byte R;
	byte G;
	byte B;
	byte A;
};

// RGB color triplet using floats as members ----------------------------------
struct ColorRGBf {
	float R;
	float G;
	float B;
};

// RGB color triplet plus alpha using floats as members -----------------------
struct ColorRGBAf {
	float R;
	float G;
	float B;
	float A;
};


// epsilon area specifications ------------------------------------------------
//
#define EPS_SCALAR				EpsAreas::eps_scalarproduct
#define EPS_COMP_ZERO			EpsAreas::eps_vanishcomponent
#define EPS_DENOM_ZERO			EpsAreas::eps_vanishdenominator
#define EPS_POINT_ON_PLANE		EpsAreas::eps_planethickness
#define EPS_POINT_ON_LINE		EpsAreas::eps_pointonline
#define EPS_POINT_ON_LINESEG	EpsAreas::eps_pointonlineseg
#define EPS_VERTEX_MERGE		EpsAreas::eps_vertexmergearea

class EpsAreas {

public:
	static double eps_scalarproduct;		// epsilon area for general scalar product comparisons
	static double eps_vanishcomponent;		// epsilon area for vanishing vector components
	static double eps_vanishdenominator;	// epsilon area for vanishing denominators
	static double eps_planethickness;		// epsilon area for width of "infinitely thin" plane
	static double eps_pointonline;			// epsilon area for point on line determination
	static double eps_pointonlineseg;		// epsilon area for point on lineseg parameter (t)
	static double eps_vertexmergearea;		// epsilon area for merging of vertices
};


BSPLIB_NAMESPACE_END


#endif // _BSPLIBDEFS_H_




