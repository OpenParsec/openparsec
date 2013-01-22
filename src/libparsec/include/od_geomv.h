/*
 * PARSEC HEADER (OBJECT)
 * Types and Conversions for Geometry and Rasterization V1.22
 *
 * Copyright (c) Markus Hadwiger 1997-1999
 * All Rights Reserved.
 */

#ifndef _OD_GEOMV_H_
#define _OD_GEOMV_H_


//NOTE:
// basically, there are two important abstract types:
// *) GEOMV_T: for geometric calculations (view-space)
// *) RASTV_T: for rasterization calculations (screen-space)
// rastv_t should be identical to the primitive data type
// used by the underlying rasterization API.
// otherwise, time-consuming conversions will take place
// rather often.


// fixed and floating point types
typedef int32_t				fixed_t;
typedef float  				psc_float_t;
typedef double				hprec_t;


// type for binary angle measurement
typedef int32_t				bams_t;


// fixed-point <-> floating-point conversions
#define FIXED_TO_FLOAT(x)	( (psc_float_t)( (fixed_t)(x) / 65536.0 ) )
#define FLOAT_TO_FIXED(x)	( (fixed_t)( (hprec_t)(x) * 65536.0 ) )


// define internal conversion macros
#define FIXED2FLOAT(x)		( (psc_float_t)( (fixed_t)(x) / 65536.0 ) )
#define FLOAT2FIXED(x)		( (fixed_t)( (psc_float_t)(x) * 65536.0 ) )
#define FIXED2INT(x)		( ( (fixed_t)(x) + 0x8000 ) >> 16 )
#define INT2FIXED(x)		( (fixed_t)(x) << 16 )
#define FLOAT2INT(x)		( (int)(psc_float_t)(x) )
#define INT2FLOAT(x)		( (psc_float_t)(x) )


// define coordinate conversion macros
#ifdef SCREENVERTEX_SUBPIXEL_ACCURACY
	#define FLOAT2COORD(x)	( (psc_float_t)( (psc_float_t)(x) * D_Value ) )
	#define COORD2FLOAT(x)	((psc_float_t)(x))	// this doesn't account for D.
#else
	#define FLOAT2COORD(x)	( (long)( (psc_float_t)(x) * D_Value ) )
	#define COORD2FLOAT(x)	INT2FLOAT(x)	// this doesn't account for D.
#endif


// pi and multiples
#define HPREC_PI			((hprec_t)3.14159265359)
#define HPREC_TWO_PI		((hprec_t)(2.0 * HPREC_PI))
#define HPREC_HALF_PI		((hprec_t)(0.5 * HPREC_PI))

// some special angle values in bams
#define BAMS_DEG0			0x00000000
#define BAMS_DEG45			0x00002000
#define BAMS_DEG90			0x00004000
#define BAMS_DEG180			0x00008000
#define BAMS_DEG360			0x00010000
#define BAMS_DEG720			0x00020000

// conversions between angle in degrees and angle in bams
#define DEG_TO_BAMS(x)		((bams_t)((65536.0/360.0) * (x)))
#define BAMS_TO_DEG(x)		((hprec_t)((360.0/65536.0) * (x)))

// conversions between angle in radians and angle in bams
#define RAD_TO_BAMS(x)		((bams_t)((65536.0/HPREC_TWO_PI) * (x)))
#define BAMS_TO_RAD(x)		((hprec_t)((HPREC_TWO_PI/65536.0) * (x)))

// conversions between angle in radians and angle in degrees
#define RAD_TO_DEG(x)		((hprec_t)((360.0/HPREC_TWO_PI) * (x)))
#define DEG_TO_RAD(x)		((hprec_t)((HPREC_TWO_PI/360.0) * (x)))


// access 32-bit quantity as dword
#define DW32(x)				(*(dword*)&(x))


// swap 32-bit quantity without intermediate temporary
#define SWAP_VALUES_32(a,b)			{ DW32(a)^=DW32(b); DW32(b)^=DW32(a); DW32(a)^=DW32(b); }

// swap two geomv_t vars (works regardless of actual geomv_t)
#define SWAP_GEOMV(a,b)		SWAP_VALUES_32((a),(b))


// abs() macro for 32-bit quantity (only for lvalue!)
#define ABS32(x)		{ if ((x)<0) (x)=-(x); }


// abs() macros for fixed_t and float_t vars (only for lvalues!)
#define ABS_FIXED(x)		ABS32(x)
#define ABS_FLOAT(x)		{ DW32(x)&=0x7fffffff; }


// comparison functions with zero for geomv_t that
// work for both fixed_t and float_t (only for lvalues!)

// determine if geomv_t is zero
#define GEOMV_ZERO(x)		(DW32(x) == 0)

// determine if geomv_t is non-zero
#define GEOMV_NONZERO(x)	(!GEOMV_ZERO(x))

// determine if geomv_t is negative
#define GEOMV_NEGATIVE(x)	( (DW32(x) & 0x80000000) != 0 )
#define GEOMV_LTZERO(x)		GEOMV_NEGATIVE(x)

// determine if geomv_t is less than or equal zero
#define GEOMV_LEZERO(x)		(GEOMV_ZERO(x) || GEOMV_NEGATIVE(x))

// determine if geomv_t is positive (in this context: non-negative)
#define GEOMV_POSITIVE(x)	(!GEOMV_NEGATIVE(x))
#define GEOMV_GEZERO(x)		GEOMV_POSITIVE(x)

// determine if geomv_t is greater than zero
#define GEOMV_GTZERO(x)		(GEOMV_NONZERO(x) && GEOMV_POSITIVE(x))



// ---------------------------------------------------------
// define conversion macros depending on
// actual geomv_t and rastv_t types
// ---------------------------------------------------------

// abs() macro for geomv_t
#define ABS_GEOMV(x)		ABS_FLOAT(x)


// types for geometric values and rasterization values
typedef psc_float_t			geomv_t;
typedef psc_float_t			rastv_t;


// special values for geometric value type
#define GEOMV_0				((geomv_t)0.0)
#define GEOMV_0_5			((geomv_t)0.5)
#define GEOMV_1				((geomv_t)1.0)
#define GEOMV_VANISHING		((geomv_t)1e-7)


// special values for rasterization value type
#define RASTV_0				((rastv_t)0.0)
#define RASTV_1				((rastv_t)1.0)
#define RASTV_VANISHING		((rastv_t)1e-7)


// geometric value conversion macros
#define GEOMV_TO_FIXED(x)	FLOAT2FIXED		(x)
#define FIXED_TO_GEOMV(x)	FIXED2FLOAT		(x)
#define GEOMV_TO_FLOAT(x)	((psc_float_t)	(x))
#define FLOAT_TO_GEOMV(x)	((geomv_t)		(x))
#define GEOMV_TO_COORD(x)	FLOAT2COORD		(x)
#define COORD_TO_GEOMV(x)	COORD2FLOAT		(x)
#define GEOMV_TO_INT(x)		FLOAT2INT		(x)
#define INT_TO_GEOMV(x)		INT2FLOAT		(x)
#define GEOMV_TO_RASTV(x)	((rastv_t)		(x))


// rasterization value conversion macros
#define RASTV_TO_FIXED(x)	FLOAT2FIXED		(x)
#define FIXED_TO_RASTV(x)	FIXED2FLOAT		(x)
#define RASTV_TO_FLOAT(x)	((psc_float_t)	(x))
#define FLOAT_TO_RASTV(x)	((rastv_t)		(x))
#define RASTV_TO_COORD(x)	FLOAT2COORD		(x)
#define COORD_TO_RASTV(x)	COORD2FLOAT		(x)
#define RASTV_TO_INT(x)		FLOAT2INT		(x)
#define INT_TO_RASTV(x)		INT2FLOAT		(x)
#define RASTV_TO_GEOMV(x)	((geomv_t)		(x))



// ------------------------------------------------------
// select type for depth values (integer or fractional)
// ------------------------------------------------------

#ifdef FRACTIONAL_DEPTH_VALUES

typedef geomv_t				depth_t;
#define DEPTH_TO_INTEGER(x)	((int32_t)GEOMV_TO_FIXED(x))
#define DEPTH_TO_GEOMV(x)	((depth_t)(x))
#define DEPTH_TO_RASTV(x)	GEOMV_TO_RASTV(x)
#define DEPTH_TO_FLOAT(x)	GEOMV_TO_FLOAT(x)
#define FIXED_TO_DEPTH(x)	FIXED_TO_GEOMV(x)
#define FLOAT_TO_DEPTH(x)	FLOAT_TO_GEOMV(x)

#else

typedef int32_t				depth_t;
#define DEPTH_TO_INTEGER(x)	((depth_t)(x))
#define DEPTH_TO_GEOMV(x)	INT_TO_GEOMV(x)
#define DEPTH_TO_RASTV(x)	INT_TO_RASTV(x)
#define DEPTH_TO_FLOAT(x)	INT2FLOAT(x)
#define FIXED_TO_DEPTH(x)	((depth_t)(x))
#define FLOAT_TO_DEPTH(x)	FLOAT2FIXED(x)

#endif

//NOTE:
// there are TWO major differences between fractional and integer
// depth values:
// 1. trivially, integer depth values contain no fractional
//    part whereas fractional depth values do. (regardless of
//    whether they are of type float_t or fixed_t.)
// 2. the range of integer depth values is from 0 to 65535,
//    whereas the range of fractional depth values is from 0.0 to 1.0.


#endif // _OD_GEOMV_H_


