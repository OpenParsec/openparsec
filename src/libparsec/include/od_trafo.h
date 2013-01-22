/*
 * PARSEC HEADER (OBJECT)
 * Transformation Types V1.68
 *
 * Copyright (c) Markus Hadwiger 1995-1998
 * All Rights Reserved.
 */

#ifndef _OD_TRAFO_H_
#define _OD_TRAFO_H_


// generic transformation matrices --------------------------------------------
//
typedef geomv_t	Xmatrx[ 3 ][ 4 ];		// [0 0 0 1] (line 4) omitted!
typedef geomv_t	dXmatrx[ 16 ];			// can be used as destination (ofs+4!!)
typedef geomv_t (*pXmatrx)[ 4 ];		// pointer to Xmatrx type
typedef geomv_t	Camera[ 3 ][ 4 ];		// position and orientation of camera
typedef geomv_t (*pCamera)[ 4 ];		// pointer to Camera type

typedef float	fMatrx2h[ 3 ][ 3 ];		// float matrix for 2-D (homogeneous)
typedef float	fMatrx3h[ 4 ][ 4 ];		// float matrix for 3-D (homogeneous)


// easily allocate destination matrix -----------------------------------------
//
#define ALLOC_DESTXMATRX(x) \
	dXmatrx _##x; pXmatrx x = (pXmatrx) ( (char*)_##x + 4 )

//NOTE:
// must not be used like if (.) ALLOC_DESTXMATRX(x). this would make no sense
// and also doesn't work because the macro is not a single statement.


// image transformation -------------------------------------------------------
//
struct imgtrafo_s {

	geomv_t	A;
	geomv_t	B;
	geomv_t	C;
	geomv_t	D;
	geomv_t	E;
	geomv_t	F;
	geomv_t	G;
	geomv_t	H;
	geomv_t	I;
};


// structure containing both the sine and cosine of a given angle -------------
//
struct sincosval_s {

	geomv_t	sinval;
	geomv_t	cosval;
};


#endif // _OD_TRAFO_H_


