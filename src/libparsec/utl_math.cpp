/*
 * PARSEC - Math Code I (ANSI-C)
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:43 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-1999
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1998
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */ 

// C library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// mathematics header
#include "utl_math.h"

// local module header


// flags
//#define USE_SINCOSTABLE


// sine/cosine tables ---------------------------------------------------------
//
float fsin_tab[] =
#include "utl_fsin.h"

float fcos_tab[] =
#include "utl_fcos.h"


// adjoint matrix sub-determinant table ---------------------------------------
//
dword adj_tab[ 4*9 ] = {

#define M_WIDTH 4		// number of row elements

	M_WIDTH*1+1,		// [1][1]
	M_WIDTH*2+3,		// [2][2]
	M_WIDTH*1+3,		// [1][2]
	M_WIDTH*2+1,		// [2][1]

	M_WIDTH*0+3,		// [0][2]
	M_WIDTH*2+1,		// [2][1]
	M_WIDTH*0+1,		// [0][1]
	M_WIDTH*2+3,		// [2][2]

	M_WIDTH*0+1,		// [0][1]
	M_WIDTH*1+3,		// [1][2]
	M_WIDTH*0+3,		// [0][2]
	M_WIDTH*1+1,		// [1][1]

	M_WIDTH*1+3,		// [1][2]
	M_WIDTH*2+0,		// [2][0]
	M_WIDTH*1+0,		// [1][0]
	M_WIDTH*2+3,		// [2][2]

	M_WIDTH*0+0,		// [0][0]
	M_WIDTH*2+3,		// [2][2]
	M_WIDTH*0+3,		// [0][2]
	M_WIDTH*2+0,		// [2][0]

	M_WIDTH*0+3,		// [0][2]
	M_WIDTH*1+0,		// [1][0]
	M_WIDTH*0+0,		// [0][0]
	M_WIDTH*1+3,		// [1][2]

	M_WIDTH*1+0,		// [1][0]
	M_WIDTH*2+1,		// [2][1]
	M_WIDTH*1+1,		// [1][1]
	M_WIDTH*2+0,		// [2][0]

	M_WIDTH*0+1,		// [0][1]
	M_WIDTH*2+0,		// [2][0]
	M_WIDTH*0+0,		// [0][0]
	M_WIDTH*2+1,		// [2][1]

	M_WIDTH*0+0,		// [0][0]
	M_WIDTH*1+1,		// [1][1]
	M_WIDTH*0+1,		// [0][1]
	M_WIDTH*1+0,		// [1][0]
};


// calculate adjoint 3x3 matrix -----------------------------------------------
//
void AdjointMtx( const Xmatrx smatrx, Xmatrx dmatrx )
{
	ASSERT( smatrx != NULL );
	ASSERT( dmatrx != NULL );
	ASSERT( smatrx != dmatrx );

	geomv_t	*pdmatrx = (geomv_t *) dmatrx;

	int tabindx = 0;
	for ( int curdet = 0; curdet < 9; curdet++, tabindx+=4 ) {

		geomv_t A = *( (geomv_t *)smatrx + adj_tab[ tabindx + 0 ] );
		geomv_t B = *( (geomv_t *)smatrx + adj_tab[ tabindx + 2 ] );
		geomv_t C = *( (geomv_t *)smatrx + adj_tab[ tabindx + 3 ] );
		geomv_t D = *( (geomv_t *)smatrx + adj_tab[ tabindx + 1 ] );

		geomv_t d1 = GEOMV_MUL( A, D );
		geomv_t d2 = GEOMV_MUL( B, C );

		pdmatrx[ curdet ] = d1 - d2;
	}
}


// multiply two 4x4 matrices --------------------------------------------------
//
void MtxMtxMUL( const Xmatrx matrxb, const Xmatrx matrxa, Xmatrx dmatrx )
{
	ASSERT( matrxb != NULL );
	ASSERT( matrxa != NULL );
	ASSERT( dmatrx != NULL );
	ASSERT( matrxb != dmatrx );
	ASSERT( matrxa != dmatrx );

	//NOTE:
	// D = B * A
	// ---------
	// [ d d d d ]   [ b b b b ]   [ a a a a ]
	// [ d d d d ] = [ b b b b ] * [ a a a a ]
	// [ d d d d ]   [ b b b b ]   [ a a a a ]
	// [ 0 0 0 1 ]   [ 0 0 0 1 ]   [ 0 0 0 1 ]

	dmatrx[0][0] = GEOMV_MUL(matrxb[0][0],matrxa[0][0]) + GEOMV_MUL(matrxb[0][1],matrxa[1][0]) + GEOMV_MUL(matrxb[0][2],matrxa[2][0]);
	dmatrx[0][1] = GEOMV_MUL(matrxb[0][0],matrxa[0][1]) + GEOMV_MUL(matrxb[0][1],matrxa[1][1]) + GEOMV_MUL(matrxb[0][2],matrxa[2][1]);
	dmatrx[0][2] = GEOMV_MUL(matrxb[0][0],matrxa[0][2]) + GEOMV_MUL(matrxb[0][1],matrxa[1][2]) + GEOMV_MUL(matrxb[0][2],matrxa[2][2]);
	dmatrx[0][3] = GEOMV_MUL(matrxb[0][0],matrxa[0][3]) + GEOMV_MUL(matrxb[0][1],matrxa[1][3]) + GEOMV_MUL(matrxb[0][2],matrxa[2][3]) + matrxb[0][3];
	dmatrx[1][0] = GEOMV_MUL(matrxb[1][0],matrxa[0][0]) + GEOMV_MUL(matrxb[1][1],matrxa[1][0]) + GEOMV_MUL(matrxb[1][2],matrxa[2][0]);
	dmatrx[1][1] = GEOMV_MUL(matrxb[1][0],matrxa[0][1]) + GEOMV_MUL(matrxb[1][1],matrxa[1][1]) + GEOMV_MUL(matrxb[1][2],matrxa[2][1]);
	dmatrx[1][2] = GEOMV_MUL(matrxb[1][0],matrxa[0][2]) + GEOMV_MUL(matrxb[1][1],matrxa[1][2]) + GEOMV_MUL(matrxb[1][2],matrxa[2][2]);
	dmatrx[1][3] = GEOMV_MUL(matrxb[1][0],matrxa[0][3]) + GEOMV_MUL(matrxb[1][1],matrxa[1][3]) + GEOMV_MUL(matrxb[1][2],matrxa[2][3]) + matrxb[1][3];
	dmatrx[2][0] = GEOMV_MUL(matrxb[2][0],matrxa[0][0]) + GEOMV_MUL(matrxb[2][1],matrxa[1][0]) + GEOMV_MUL(matrxb[2][2],matrxa[2][0]);
	dmatrx[2][1] = GEOMV_MUL(matrxb[2][0],matrxa[0][1]) + GEOMV_MUL(matrxb[2][1],matrxa[1][1]) + GEOMV_MUL(matrxb[2][2],matrxa[2][1]);
	dmatrx[2][2] = GEOMV_MUL(matrxb[2][0],matrxa[0][2]) + GEOMV_MUL(matrxb[2][1],matrxa[1][2]) + GEOMV_MUL(matrxb[2][2],matrxa[2][2]);
	dmatrx[2][3] = GEOMV_MUL(matrxb[2][0],matrxa[0][3]) + GEOMV_MUL(matrxb[2][1],matrxa[1][3]) + GEOMV_MUL(matrxb[2][2],matrxa[2][3]) + matrxb[2][3];
}


// multiply two 4x4 matrices (neglects translation part of matrix a) ----------
//
void MtxMtxMULt( const Xmatrx matrxb, const Xmatrx matrxa, Xmatrx dmatrx )
{
	ASSERT( matrxb != NULL );
	ASSERT( matrxa != NULL );
	ASSERT( dmatrx != NULL );
	ASSERT( matrxb != dmatrx );
	ASSERT( matrxa != dmatrx );

	//NOTE:
	// D = B * A
	// ---------
	// [ d d d d ]   [ b b b b ]   [ a a a 0 ]
	// [ d d d d ] = [ b b b b ] * [ a a a 0 ]
	// [ d d d d ]   [ b b b b ]   [ a a a 0 ]
	// [ 0 0 0 1 ]   [ 0 0 0 1 ]   [ 0 0 0 1 ]

	dmatrx[0][0] = GEOMV_MUL(matrxb[0][0],matrxa[0][0]) + GEOMV_MUL(matrxb[0][1],matrxa[1][0]) + GEOMV_MUL(matrxb[0][2],matrxa[2][0]);
	dmatrx[0][1] = GEOMV_MUL(matrxb[0][0],matrxa[0][1]) + GEOMV_MUL(matrxb[0][1],matrxa[1][1]) + GEOMV_MUL(matrxb[0][2],matrxa[2][1]);
	dmatrx[0][2] = GEOMV_MUL(matrxb[0][0],matrxa[0][2]) + GEOMV_MUL(matrxb[0][1],matrxa[1][2]) + GEOMV_MUL(matrxb[0][2],matrxa[2][2]);
	dmatrx[0][3] = matrxb[0][3];
	dmatrx[1][0] = GEOMV_MUL(matrxb[1][0],matrxa[0][0]) + GEOMV_MUL(matrxb[1][1],matrxa[1][0]) + GEOMV_MUL(matrxb[1][2],matrxa[2][0]);
	dmatrx[1][1] = GEOMV_MUL(matrxb[1][0],matrxa[0][1]) + GEOMV_MUL(matrxb[1][1],matrxa[1][1]) + GEOMV_MUL(matrxb[1][2],matrxa[2][1]);
	dmatrx[1][2] = GEOMV_MUL(matrxb[1][0],matrxa[0][2]) + GEOMV_MUL(matrxb[1][1],matrxa[1][2]) + GEOMV_MUL(matrxb[1][2],matrxa[2][2]);
	dmatrx[1][3] = matrxb[1][3];
	dmatrx[2][0] = GEOMV_MUL(matrxb[2][0],matrxa[0][0]) + GEOMV_MUL(matrxb[2][1],matrxa[1][0]) + GEOMV_MUL(matrxb[2][2],matrxa[2][0]);
	dmatrx[2][1] = GEOMV_MUL(matrxb[2][0],matrxa[0][1]) + GEOMV_MUL(matrxb[2][1],matrxa[1][1]) + GEOMV_MUL(matrxb[2][2],matrxa[2][1]);
	dmatrx[2][2] = GEOMV_MUL(matrxb[2][0],matrxa[0][2]) + GEOMV_MUL(matrxb[2][1],matrxa[1][2]) + GEOMV_MUL(matrxb[2][2],matrxa[2][2]);
	dmatrx[2][3] = matrxb[2][3];
}


// multiply 4x4 matrix by 4x1 matrix (column vector) --------------------------
//
void MtxVctMUL( const Xmatrx matrx, const Vector3 *svect, Vector3 *dvect )
{
	ASSERT( matrx != NULL );
	ASSERT( svect != NULL );
	ASSERT( dvect != NULL );
	ASSERT( svect != dvect );

	dvect->X = GEOMV_MUL(matrx[0][0],svect->X) + GEOMV_MUL(matrx[0][1],svect->Y) + GEOMV_MUL(matrx[0][2],svect->Z) + matrx[0][3];
	dvect->Y = GEOMV_MUL(matrx[1][0],svect->X) + GEOMV_MUL(matrx[1][1],svect->Y) + GEOMV_MUL(matrx[1][2],svect->Z) + matrx[1][3];
	dvect->Z = GEOMV_MUL(matrx[2][0],svect->X) + GEOMV_MUL(matrx[2][1],svect->Y) + GEOMV_MUL(matrx[2][2],svect->Z) + matrx[2][3];
}


// multiply 4x4 matrix by 4x1 matrix (column vector); skip translation --------
//
void MtxVctMULt( const Xmatrx matrx, const Vector3 *svect, Vector3 *dvect )
{
	ASSERT( matrx != NULL );
	ASSERT( svect != NULL );
	ASSERT( dvect != NULL );
	ASSERT( svect != dvect );

	//NOTE:
	// actually 3x3 matrix times 3x1 matrix since all
	// homogeneous components (including translation!)
	// are neglected (assumed to be zero/one).

	dvect->X = GEOMV_MUL(matrx[0][0],svect->X) + GEOMV_MUL(matrx[0][1],svect->Y) + GEOMV_MUL(matrx[0][2],svect->Z);
	dvect->Y = GEOMV_MUL(matrx[1][0],svect->X) + GEOMV_MUL(matrx[1][1],svect->Y) + GEOMV_MUL(matrx[1][2],svect->Z);
	dvect->Z = GEOMV_MUL(matrx[2][0],svect->X) + GEOMV_MUL(matrx[2][1],svect->Y) + GEOMV_MUL(matrx[2][2],svect->Z);
}


// multiply basis vector 3 by scalar (generate direction vector) --------------
//
void DirVctMUL( const Xmatrx matrx, geomv_t scalar, Vector3 *dvect )
{
	ASSERT( matrx != NULL );
	ASSERT( dvect != NULL );

	dvect->X = GEOMV_MUL( matrx[ 0 ][ 2 ], scalar );
	dvect->Y = GEOMV_MUL( matrx[ 1 ][ 2 ], scalar );
	dvect->Z = GEOMV_MUL( matrx[ 2 ][ 2 ], scalar );
}

// multiply basis vector 3 by scalar (generate horizontal slide vector) -------
//
void RightVctMUL( const Xmatrx matrx, geomv_t scalar, Vector3 *dvect )
{
	ASSERT( matrx != NULL );
	ASSERT( dvect != NULL );

	dvect->X = GEOMV_MUL( matrx[ 0 ][ 0 ], scalar );
	dvect->Y = GEOMV_MUL( matrx[ 1 ][ 0 ], scalar );
	dvect->Z = GEOMV_MUL( matrx[ 2 ][ 0 ], scalar );
}

// multiply basis vector 3 by scalar (generate vertical slide vector) ---------
//
void UpVctMUL( const Xmatrx matrx, geomv_t scalar, Vector3 *dvect )
{
	ASSERT( matrx != NULL );
	ASSERT( dvect != NULL );

	dvect->X = GEOMV_MUL( matrx[ 0 ][ 1 ], scalar );
	dvect->Y = GEOMV_MUL( matrx[ 1 ][ 1 ], scalar );
	dvect->Z = GEOMV_MUL( matrx[ 2 ][ 1 ], scalar );
}

// Reflect incident vector (ivec) on surface normal to produce reflection (destvec)
// 'normal' needs to be normalized, ivec doesn't
//
void VctReflect( const Vector3 *ivec, const Vector3 *normal, Vector3 *destvec )
{
	ASSERT(ivec != NULL);
	ASSERT(destvec != NULL);
	ASSERT(normal != NULL);
	
	float dot = DotProduct(ivec, normal);
	
	destvec->X = ivec->X - 2.0f * dot * normal->X;
	destvec->Y = ivec->Y - 2.0f * dot * normal->Y;
	destvec->Z = ivec->Z - 2.0f * dot * normal->Z;
}

// calculate new position from forward/horizontal/vertical movements ----------
//
void CalcMovement( Vector3* movement, const Xmatrx frame, fixed_t forward, geomv_t horiz, geomv_t vert, refframe_t refframes )
{
	ASSERT( movement != NULL );
	ASSERT( frame != NULL );

	// horizontal-slide
	Vector3 rightvec;
	geomv_t horiz_slide = horiz * refframes;
	RightVctMUL( frame, horiz_slide, &rightvec );
	movement->X = rightvec.X;
	movement->Y = rightvec.Y;
	movement->Z = rightvec.Z;

	// vertical-slide
	Vector3 upvec;
	geomv_t vert_slide = vert * refframes;
	UpVctMUL( frame, vert_slide, &upvec );
	movement->X += upvec.X;
	movement->Y += upvec.Y;
	movement->Z += upvec.Z;

	// forward movement
	Vector3 dirvec;
	fixed_t forward_movement = forward * refframes;
	DirVctMUL( frame, FIXED_TO_GEOMV( forward_movement ), &dirvec );
	movement->X += dirvec.X;
	movement->Y += dirvec.Y;
	movement->Z += dirvec.Z;
}

// fetch sine/cosine value for given angle from table -------------------------
//
void GetSinCos( dword angle, sincosval_s *resultp )
{
	ASSERT( resultp != NULL );

#ifdef USE_SINCOSTABLE

	//NOTE:
	// uses 32K table (4K entries for sin and cos each)
	// angles are in BAMS

	int tabindx = ( angle & 0xffff ) >> 4;
	resultp->sinval = FLOAT_TO_GEOMV( fsin_tab[ tabindx ] );
	resultp->cosval = FLOAT_TO_GEOMV( fcos_tab[ tabindx ] );

#else

	resultp->sinval = FLOAT_TO_GEOMV( sin( BAMS_TO_RAD( angle ) ) );
	resultp->cosval = FLOAT_TO_GEOMV( cos( BAMS_TO_RAD( angle ) ) );

#endif

}


// rotate vector space around x axis (multiply from the right) ----------------
//
void ObjRotX( Xmatrx matrix, bams_t pitch )
{
	ASSERT( matrix != NULL );

	sincosval_s sincosv;
	GetSinCos( pitch, &sincosv );

	Xmatrx rotmtx;
	rotmtx[0][0] = GEOMV_1;
	rotmtx[0][1] = GEOMV_0;
	rotmtx[0][2] = GEOMV_0;
//	rotmtx[0][3] = GEOMV_0;
	rotmtx[1][0] = GEOMV_0;
	rotmtx[1][1] = sincosv.cosval;
	rotmtx[1][2] = -sincosv.sinval;
//	rotmtx[1][3] = GEOMV_0;
	rotmtx[2][0] = GEOMV_0;
	rotmtx[2][1] = sincosv.sinval;
	rotmtx[2][2] = sincosv.cosval;
//	rotmtx[2][3] = GEOMV_0;

	Xmatrx destmtx;
	MtxMtxMULt( matrix, rotmtx, destmtx );
	memcpy( matrix, destmtx, sizeof( Xmatrx ) );
}


// rotate vector space around y axis (multiply from the right) ----------------
//
void ObjRotY( Xmatrx matrix, bams_t yaw )
{
	ASSERT( matrix != NULL );

	sincosval_s sincosv;
	GetSinCos( yaw, &sincosv );

	Xmatrx rotmtx;
	rotmtx[0][0] = sincosv.cosval;
	rotmtx[0][1] = GEOMV_0;
	rotmtx[0][2] = sincosv.sinval;
//	rotmtx[0][3] = GEOMV_0;
	rotmtx[1][0] = GEOMV_0;
	rotmtx[1][1] = GEOMV_1;
	rotmtx[1][2] = GEOMV_0;
//	rotmtx[1][3] = GEOMV_0;
	rotmtx[2][0] = -sincosv.sinval;
	rotmtx[2][1] = GEOMV_0;
	rotmtx[2][2] = sincosv.cosval;
//	rotmtx[2][3] = GEOMV_0;

	Xmatrx destmtx;
	MtxMtxMULt( matrix, rotmtx, destmtx );
	memcpy( matrix, destmtx, sizeof( Xmatrx ) );
}


// rotate vector space around z axis (multiply from the right) ----------------
//
void ObjRotZ( Xmatrx matrix, bams_t roll )
{
	ASSERT( matrix != NULL );

	sincosval_s sincosv;
	GetSinCos( roll, &sincosv );

	Xmatrx rotmtx;
	rotmtx[0][0] = sincosv.cosval;
	rotmtx[0][1] = -sincosv.sinval;
	rotmtx[0][2] = GEOMV_0;
//	rotmtx[0][3] = GEOMV_0;
	rotmtx[1][0] = sincosv.sinval;
	rotmtx[1][1] = sincosv.cosval;
	rotmtx[1][2] = GEOMV_0;
//	rotmtx[1][3] = GEOMV_0;
	rotmtx[2][0] = GEOMV_0;
	rotmtx[2][1] = GEOMV_0;
	rotmtx[2][2] = GEOMV_1;
//	rotmtx[2][3] = GEOMV_0;

	Xmatrx destmtx;
	MtxMtxMULt( matrix, rotmtx, destmtx );
	memcpy( matrix, destmtx, sizeof( Xmatrx ) );
}


// rotate vector space around x axis (multiply from the left) -----------------
//
void CamRotX( Xmatrx matrix, bams_t pitch )
{
	ASSERT( matrix != NULL );

	sincosval_s sincosv;
	GetSinCos( pitch, &sincosv );

	Xmatrx rotmtx;
	rotmtx[0][0] = GEOMV_1;
	rotmtx[0][1] = GEOMV_0;
	rotmtx[0][2] = GEOMV_0;
	rotmtx[0][3] = GEOMV_0;
	rotmtx[1][0] = GEOMV_0;
	rotmtx[1][1] = sincosv.cosval;
	rotmtx[1][2] = -sincosv.sinval;
	rotmtx[1][3] = GEOMV_0;
	rotmtx[2][0] = GEOMV_0;
	rotmtx[2][1] = sincosv.sinval;
	rotmtx[2][2] = sincosv.cosval;
	rotmtx[2][3] = GEOMV_0;

	Xmatrx destmtx;
	MtxMtxMUL( rotmtx, matrix, destmtx );
	memcpy( matrix, destmtx, sizeof( Xmatrx ) );
}


// rotate vector space around y axis (multiply from the left) -----------------
//
void CamRotY( Xmatrx matrix, bams_t yaw )
{
	ASSERT( matrix != NULL );

	sincosval_s sincosv;
	GetSinCos( yaw, &sincosv );

	Xmatrx rotmtx;
	rotmtx[0][0] = sincosv.cosval;
	rotmtx[0][1] = GEOMV_0;
	rotmtx[0][2] = sincosv.sinval;
	rotmtx[0][3] = GEOMV_0;
	rotmtx[1][0] = GEOMV_0;
	rotmtx[1][1] = GEOMV_1;
	rotmtx[1][2] = GEOMV_0;
	rotmtx[1][3] = GEOMV_0;
	rotmtx[2][0] = -sincosv.sinval;
	rotmtx[2][1] = GEOMV_0;
	rotmtx[2][2] = sincosv.cosval;
	rotmtx[2][3] = GEOMV_0;

	Xmatrx destmtx;
	MtxMtxMUL( rotmtx, matrix, destmtx );
	memcpy( matrix, destmtx, sizeof( Xmatrx ) );
}


// rotate vector space around z axis (multiply from the left) -----------------
//
void CamRotZ( Xmatrx matrix, bams_t roll )
{
	ASSERT( matrix != NULL );

	sincosval_s sincosv;
	GetSinCos( roll, &sincosv );

	Xmatrx rotmtx;
	rotmtx[0][0] = sincosv.cosval;
	rotmtx[0][1] = -sincosv.sinval;
	rotmtx[0][2] = GEOMV_0;
	rotmtx[0][3] = GEOMV_0;
	rotmtx[1][0] = sincosv.sinval;
	rotmtx[1][1] = sincosv.cosval;
	rotmtx[1][2] = GEOMV_0;
	rotmtx[1][3] = GEOMV_0;
	rotmtx[2][0] = GEOMV_0;
	rotmtx[2][1] = GEOMV_0;
	rotmtx[2][2] = GEOMV_1;
	rotmtx[2][3] = GEOMV_0;

	Xmatrx destmtx;
	MtxMtxMUL( rotmtx, matrix, destmtx );
	memcpy( matrix, destmtx, sizeof( Xmatrx ) );
}


// calc dot-product of two vectors (*v1 * *v2) --------------------------------
//
geomv_t DotProduct( const Vector3 *vect1, const Vector3 *vect2 )
{
	ASSERT( vect1 != NULL );
	ASSERT( vect2 != NULL );

	return GEOMV_MUL( vect1->X, vect2->X ) + GEOMV_MUL( vect1->Y, vect2->Y ) + GEOMV_MUL( vect1->Z, vect2->Z );
}


// calc cross product of two vectors (*cproduct = *v1 x *v2) ------------------
//
void CrossProduct( const Vector3 *vect1, const Vector3 *vect2, Vector3 *cproduct )
{
	ASSERT( vect1 != NULL );
	ASSERT( vect2 != NULL );
	ASSERT( cproduct != NULL );
	ASSERT( vect1 != cproduct );
	ASSERT( vect2 != cproduct );

	cproduct->X = GEOMV_MUL( vect1->Y, vect2->Z ) - GEOMV_MUL( vect1->Z, vect2->Y );
	cproduct->Y = GEOMV_MUL( vect1->Z, vect2->X ) - GEOMV_MUL( vect1->X, vect2->Z );
	cproduct->Z = GEOMV_MUL( vect1->X, vect2->Y ) - GEOMV_MUL( vect1->Y, vect2->X );
}


// calc cross product of two vectors imbedded in matrix -----------------------
//
void CrossProduct2( const geomv_t *vect1, const geomv_t *vect2, geomv_t *cproduct )
{
	ASSERT( vect1 != NULL );
	ASSERT( vect2 != NULL );
	ASSERT( cproduct != NULL );
	ASSERT( vect1 != cproduct );
	ASSERT( vect2 != cproduct );

	cproduct[0] = GEOMV_MUL( vect1[4], vect2[8] ) - GEOMV_MUL( vect1[8], vect2[4] );
	cproduct[4] = GEOMV_MUL( vect1[8], vect2[0] ) - GEOMV_MUL( vect1[0], vect2[8] );
	cproduct[8] = GEOMV_MUL( vect1[0], vect2[4] ) - GEOMV_MUL( vect1[4], vect2[0] );
}


// re-orthogonalize matrix column vectors -------------------------------------
//
void ReOrthoMtx( Xmatrx matrix )
{
	ASSERT( matrix != NULL );

	//TODO:
	// replace with numerically more
	// sane version.

	CrossProduct2( &matrix[ 0 ][ 0 ], &matrix[ 0 ][ 1 ], &matrix[ 0 ][ 2 ] );
	CrossProduct2( &matrix[ 0 ][ 1 ], &matrix[ 0 ][ 2 ], &matrix[ 0 ][ 0 ] );
}

#ifdef PARSEC_CLIENT

// process entire object (transforms and projects all vertices) ---------------
//
void ProcessObject( GenObject *object )
{
	ASSERT( object != NULL );

	// leave normals alone
	int base = object->NumNormals;

	Vertex3	*vtxlist   = &object->VertexList[ base ];
	Vertex3	*x_vtxlist = &object->X_VertexList[ base ];
	SPoint	*s_vtxlist = &object->S_VertexList[ base ];

	for ( int numvtxs = object->NumPolyVerts; numvtxs > 0; numvtxs--, vtxlist++ ) {

		// only transform and project actually visible vertices
		if ( vtxlist->VisibleFrame == CurVisibleFrame ) {

			// transform vertex into view-space
			MtxVctMUL( object->CurrentXmatrx, vtxlist, x_vtxlist );

			// project vertex onto screen (but do not add origin offset!!)
			s_vtxlist->X = GEOMV_TO_COORD( GEOMV_DIV( x_vtxlist->X, x_vtxlist->Z ) );
			s_vtxlist->Y = GEOMV_TO_COORD( GEOMV_DIV( x_vtxlist->Y, x_vtxlist->Z ) );
		}

		x_vtxlist++;
		s_vtxlist++;
	}
}


#endif // PARSEC_CLIENT



