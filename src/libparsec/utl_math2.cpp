/*
 * PARSEC - Math Code II (ANSI-C)
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:43 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-1999
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001
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
#include <stddef.h>
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


// to prevent warnings when converting doubles to floats
#ifdef SYSTEM_COMPILER_MSVC
	#pragma warning ( disable : 4244 )
#endif // SYSTEM_COMPILER_MSVC

// global matrix that can be used as destination by assembly math code --------
//
ALLOC_DESTXMATRX( DestXmatrx );


// calculate vector length ----------------------------------------------------
//
geomv_t VctLenX( Vector3 *vec )
{
	ASSERT( vec != NULL );

	float x = GEOMV_TO_FLOAT( vec->X );
	float y = GEOMV_TO_FLOAT( vec->Y );
	float z = GEOMV_TO_FLOAT( vec->Z );

	float norm = sqrtf( x*x + y*y + z*z );

	return FLOAT_TO_GEOMV( norm );
}


// calculate vector length (approximated version) -----------------------------
//
geomv_t VctLen( Vector3 *vec )
{
	ASSERT( vec != NULL );

	//NOTE:
	// this version approximates a vector's length
	// by using the following formula:
	//   len = vmax + (11/32)vmed + (1/4)vmin
	// [this gives a result that is accurate within 8%]

	geomv_t vmax = vec->X;
	geomv_t vmed = vec->Y;
	geomv_t vmin = vec->Z;

	ABS_GEOMV( vmax );
	ABS_GEOMV( vmed );
	ABS_GEOMV( vmin );

	if ( DW32( vmax ) < DW32( vmed ) )
		SWAP_GEOMV( vmax, vmed );
	if ( DW32( vmax ) < DW32( vmin ) )
		SWAP_GEOMV( vmax, vmin );
	if ( DW32( vmed ) < DW32( vmin ) )
		SWAP_GEOMV( vmed, vmin );

	geomv_t norm = vmax + ( vmed * ( 11.0f / 8.0f ) + vmin ) / 4.0f;

	return norm;
}


// normalize 3-D vector -------------------------------------------------------
//
void NormVctX( Vector3 *vec )
{
	//NOTE:
	// this function doesn't handle null
	// vectors transparently. there is only
	// an assertion. otherwise NaN's will be
	// the result.

	ASSERT( vec != NULL );

	float x = GEOMV_TO_FLOAT( vec->X );
	float y = GEOMV_TO_FLOAT( vec->Y );
	float z = GEOMV_TO_FLOAT( vec->Z );

	float norm = sqrtf( x*x + y*y + z*z );

	ASSERT( norm > 1e-7 );
//	if ( norm <= 1e-7 ) {
//		return;
//	}

	vec->X = FLOAT_TO_GEOMV( x / norm );
	vec->Y = FLOAT_TO_GEOMV( y / norm );
	vec->Z = FLOAT_TO_GEOMV( z / norm );
}


// normalize 3-D vector (approximated version) --------------------------------
//
void NormVct( Vector3 *vec )
{
	//NOTE:
	// this function doesn't handle null
	// vectors transparently. there is only
	// an assertion. otherwise NaN's will be
	// the result.

	ASSERT( vec != NULL );

	geomv_t vmax = vec->X;
	geomv_t vmed = vec->Y;
	geomv_t vmin = vec->Z;

	ABS_GEOMV( vmax );
	ABS_GEOMV( vmed );
	ABS_GEOMV( vmin );

	if ( DW32( vmax ) < DW32( vmed ) )
		SWAP_GEOMV( vmax, vmed );
	if ( DW32( vmax ) < DW32( vmin ) )
		SWAP_GEOMV( vmax, vmin );
	if ( DW32( vmed ) < DW32( vmin ) )
		SWAP_GEOMV( vmed, vmin );

	geomv_t norm = vmax + ( vmed * ( 11.0f / 8.0f ) + vmin ) / 4.0f;

	ASSERT( norm > GEOMV_VANISHING );
//	if ( norm <= GEOMV_VANISHING ) {
//		return;
//	}

	vec->X = GEOMV_DIV( vec->X, norm );
	vec->Y = GEOMV_DIV( vec->Y, norm );
	vec->Z = GEOMV_DIV( vec->Z, norm );
}


// normalize vectors of upper left 3x3 submatrix ------------------------------
//
void NormMtx( Xmatrx matrix )
{
	// normalize matrix column vectors
	for ( int i = 0; i < 3; i++ ) {

		float x = GEOMV_TO_FLOAT( matrix[ 0 ][ i ] );
		float y = GEOMV_TO_FLOAT( matrix[ 1 ][ i ] );
		float z = GEOMV_TO_FLOAT( matrix[ 2 ][ i ] );

		float norm = sqrtf( x*x + y*y + z*z );

		matrix[ 0 ][ i ] = FLOAT_TO_GEOMV( x / norm );
		matrix[ 1 ][ i ] = FLOAT_TO_GEOMV( y / norm );
		matrix[ 2 ][ i ] = FLOAT_TO_GEOMV( z / norm );
	}
}


// re-orthonormalize upper left 3x3 submatrix ---------------------------------
//
void ReOrthoNormMtx( Xmatrx matrix )
{
	ReOrthoMtx( matrix );
	NormMtx( matrix );
}


// create identity matrix -----------------------------------------------------
//
void MakeIdMatrx( Xmatrx matrx )
{
	matrx[ 0 ][ 0 ] = GEOMV_1;
	matrx[ 0 ][ 1 ] = GEOMV_0;
	matrx[ 0 ][ 2 ] = GEOMV_0;
	matrx[ 0 ][ 3 ] = GEOMV_0;
	matrx[ 1 ][ 0 ] = GEOMV_0;
	matrx[ 1 ][ 1 ] = GEOMV_1;
	matrx[ 1 ][ 2 ] = GEOMV_0;
	matrx[ 1 ][ 3 ] = GEOMV_0;
	matrx[ 2 ][ 0 ] = GEOMV_0;
	matrx[ 2 ][ 1 ] = GEOMV_0;
	matrx[ 2 ][ 2 ] = GEOMV_1;
	matrx[ 2 ][ 3 ] = GEOMV_0;
}


// create matrix with translation vector (0,0,0) ------------------------------
//
void MakeNonTranslationMatrx( const Xmatrx matrx, Xmatrx dmatrx )
{
	dmatrx[ 0 ][ 0 ] = matrx[ 0 ][ 0 ];
	dmatrx[ 0 ][ 1 ] = matrx[ 0 ][ 1 ];
	dmatrx[ 0 ][ 2 ] = matrx[ 0 ][ 2 ];
	dmatrx[ 0 ][ 3 ] = GEOMV_0;
	dmatrx[ 1 ][ 0 ] = matrx[ 1 ][ 0 ];
	dmatrx[ 1 ][ 1 ] = matrx[ 1 ][ 1 ];
	dmatrx[ 1 ][ 2 ] = matrx[ 1 ][ 2 ];
	dmatrx[ 1 ][ 3 ] = GEOMV_0;
	dmatrx[ 2 ][ 0 ] = matrx[ 2 ][ 0 ];
	dmatrx[ 2 ][ 1 ] = matrx[ 2 ][ 1 ];
	dmatrx[ 2 ][ 2 ] = matrx[ 2 ][ 2 ];
	dmatrx[ 2 ][ 3 ] = GEOMV_0;
}


// calculate the inverse of an orthogonal matrix ------------------------------
//
void CalcOrthoInverse( const Xmatrx matrx, Xmatrx dmatrx )
{
	//NOTE:
	// assumes matrx is an T*R matrix, where
	// T is translation only and R is orthogonal.

	Vector3 tvec;
	Vector3 dvec;

	// transpose 3x3 submatrix
	dmatrx[ 0 ][ 0 ] = matrx[ 0 ][ 0 ];
	dmatrx[ 0 ][ 1 ] = matrx[ 1 ][ 0 ];
	dmatrx[ 0 ][ 2 ] = matrx[ 2 ][ 0 ];
//	dmatrx[ 0 ][ 3 ] = GEOMV_0;			// implicit
	dmatrx[ 1 ][ 0 ] = matrx[ 0 ][ 1 ];
	dmatrx[ 1 ][ 1 ] = matrx[ 1 ][ 1 ];
	dmatrx[ 1 ][ 2 ] = matrx[ 2 ][ 1 ];
//	dmatrx[ 1 ][ 3 ] = GEOMV_0;			// implicit
	dmatrx[ 2 ][ 0 ] = matrx[ 0 ][ 2 ];
	dmatrx[ 2 ][ 1 ] = matrx[ 1 ][ 2 ];
	dmatrx[ 2 ][ 2 ] = matrx[ 2 ][ 2 ];
//	dmatrx[ 2 ][ 3 ] = GEOMV_0;			// implicit

	// invert translation vector
	tvec.X = -matrx[ 0 ][ 3 ];
	tvec.Y = -matrx[ 1 ][ 3 ];
	tvec.Z = -matrx[ 2 ][ 3 ];

	// apply transformation to translation vector
	MtxVctMULt( dmatrx, &tvec, &dvec );

	// store result (new translation vector)
	dmatrx[ 0 ][ 3 ] = dvec.X;
	dmatrx[ 1 ][ 3 ] = dvec.Y;
	dmatrx[ 2 ][ 3 ] = dvec.Z;
}


// calculate camera origin in object space as defined by CurrentXmatrx --------
//
void CalcObjSpaceCamera( const GenObject *objectp, Vector3 *cameravec )
{
	ASSERT( objectp != NULL );
	ASSERT( cameravec != NULL );

	//NOTE:
	// assumes CurrentXmatrx is an T*R matrix, where
	// T is translation only and R is orthogonal.

	//NOTE:
	// the calculation of the inverse CurrentXmatrx is sped up by
	// calculating R^-1 and T^-1 separately, multiplying them afterwards

	Xmatrx inv;
	inv[ 0 ][ 0 ] = objectp->CurrentXmatrx[ 0 ][ 0 ];
	inv[ 0 ][ 1 ] = objectp->CurrentXmatrx[ 1 ][ 0 ];
	inv[ 0 ][ 2 ] = objectp->CurrentXmatrx[ 2 ][ 0 ];
//	inv[ 0 ][ 3 ] = GEOMV_0;							// implicit
	inv[ 1 ][ 0 ] = objectp->CurrentXmatrx[ 0 ][ 1 ];
	inv[ 1 ][ 1 ] = objectp->CurrentXmatrx[ 1 ][ 1 ];
	inv[ 1 ][ 2 ] = objectp->CurrentXmatrx[ 2 ][ 1 ];
//	inv[ 1 ][ 3 ] = GEOMV_0;							// implicit
	inv[ 2 ][ 0 ] = objectp->CurrentXmatrx[ 0 ][ 2 ];
	inv[ 2 ][ 1 ] = objectp->CurrentXmatrx[ 1 ][ 2 ];
	inv[ 2 ][ 2 ] = objectp->CurrentXmatrx[ 2 ][ 2 ];
//	inv[ 2 ][ 3 ] = GEOMV_0;							// implicit

	Vertex3 camobjloc;
	camobjloc.X = -objectp->CurrentXmatrx[ 0 ][ 3 ];
	camobjloc.Y = -objectp->CurrentXmatrx[ 1 ][ 3 ];
	camobjloc.Z = -objectp->CurrentXmatrx[ 2 ][ 3 ];

	MtxVctMULt( inv, &camobjloc, cameravec );
}


// transform volume with transformation matrix --------------------------------
//
void TransformVolume( const Xmatrx matrx, Plane3 *vol_in, Plane3 *vol_out, dword cullmask )
{
	ASSERT( vol_in != NULL );
	ASSERT( vol_out != NULL );

	//NOTE:
	// assumes matrx is an T*R matrix, where
	// T is translation only and R is orthogonal.

	Vector3 tvec;
	tvec.X = matrx[ 0 ][ 3 ];
	tvec.Y = matrx[ 1 ][ 3 ];
	tvec.Z = matrx[ 2 ][ 3 ];

	// transform all non-masked planes
	for ( ; cullmask != 0x00; cullmask >>= 1, vol_in++, vol_out++ ) {

		if ( cullmask & 0x01 ) {

			// transform normal
			MtxVctMULt( matrx, PLANE_NORMAL( vol_in ), PLANE_NORMAL( vol_out ) );

			// transform distance
			PLANE_OFFSET( vol_out ) = PLANE_OFFSET( vol_in ) +
									  DOT_PRODUCT( &tvec, PLANE_NORMAL( vol_out ) );
		}
	}
}


// transform volume with inverse of transformation matrix ---------------------
//
void BackTransformVolume( const Xmatrx matrx, Plane3 *vol_in, Plane3 *vol_out, dword cullmask )
{
	ASSERT( vol_in != NULL );
	ASSERT( vol_out != NULL );

	//NOTE:
	// assumes matrx is an T*R matrix, where
	// T is translation only and R is orthogonal.

	// transpose R (yields R^-1)
	Xmatrx rinv;
	rinv[ 0 ][ 0 ] = matrx[ 0 ][ 0 ];
	rinv[ 0 ][ 1 ] = matrx[ 1 ][ 0 ];
	rinv[ 0 ][ 2 ] = matrx[ 2 ][ 0 ];
//	rinv[ 0 ][ 3 ] = GEOMV_0;			// implicit
	rinv[ 1 ][ 0 ] = matrx[ 0 ][ 1 ];
	rinv[ 1 ][ 1 ] = matrx[ 1 ][ 1 ];
	rinv[ 1 ][ 2 ] = matrx[ 2 ][ 1 ];
//	rinv[ 1 ][ 3 ] = GEOMV_0;			// implicit
	rinv[ 2 ][ 0 ] = matrx[ 0 ][ 2 ];
	rinv[ 2 ][ 1 ] = matrx[ 1 ][ 2 ];
	rinv[ 2 ][ 2 ] = matrx[ 2 ][ 2 ];
//	rinv[ 2 ][ 3 ] = GEOMV_0;			// implicit

	// invert T
	Vector3 tvec;
	tvec.X = -matrx[ 0 ][ 3 ];
	tvec.Y = -matrx[ 1 ][ 3 ];
	tvec.Z = -matrx[ 2 ][ 3 ];

	// transform all non-masked planes
	for ( ; cullmask != 0x00; cullmask >>= 1, vol_in++, vol_out++ ) {

		if ( cullmask & 0x01 ) {

			// transform distance
			PLANE_OFFSET( vol_out ) = PLANE_OFFSET( vol_in ) +
									  DOT_PRODUCT( &tvec, PLANE_NORMAL( vol_in ) );
			// transform normal
			MtxVctMULt( rinv, PLANE_NORMAL( vol_in ), PLANE_NORMAL( vol_out ) );
		}
	}
}


// check whether general quaternion is unit quaternion ------------------------
//
int QuaternionIsUnit( const Quaternion *quat )
{
	ASSERT( quat != NULL );

	float qlen2 = GEOMV_TO_FLOAT( quat->X ) * GEOMV_TO_FLOAT( quat->X ) +
					GEOMV_TO_FLOAT( quat->Y ) * GEOMV_TO_FLOAT( quat->Y ) +
					GEOMV_TO_FLOAT( quat->Z ) * GEOMV_TO_FLOAT( quat->Z ) +
					GEOMV_TO_FLOAT( quat->W ) * GEOMV_TO_FLOAT( quat->W );

	return ( ( qlen2 > ( 1.0f - 1e-5f ) ) && ( qlen2 < ( 1.0f + 1e-5f ) ) );
}

int QuaternionIsUnit_f( const Quaternion_f *quat )
{
	ASSERT( quat != NULL );

	float qlen2 = quat->X * quat->X + quat->Y * quat->Y +
					quat->Z * quat->Z + quat->W * quat->W;

	return ( ( qlen2 > ( 1.0f - 1e-5f ) ) && ( qlen2 < ( 1.0f + 1e-5f ) ) );
}


// convert general quaternion into unit quaternion ----------------------------
//
void QuaternionMakeUnit( Quaternion *quat )
{
	ASSERT( quat != NULL );

	float W = GEOMV_TO_FLOAT( quat->W );
	float X = GEOMV_TO_FLOAT( quat->X );
	float Y = GEOMV_TO_FLOAT( quat->Y );
	float Z = GEOMV_TO_FLOAT( quat->Z );

	float invnorm = 1.0f / ( W*W + X*X + Y*Y + Z*Z );
	invnorm = sqrtf( invnorm );

	quat->W = FLOAT_TO_GEOMV( W * invnorm );
	quat->X = FLOAT_TO_GEOMV( X * invnorm );
	quat->Y = FLOAT_TO_GEOMV( Y * invnorm );
	quat->Z = FLOAT_TO_GEOMV( Z * invnorm );
}

void QuaternionMakeUnit_f( Quaternion_f *quat )
{
	ASSERT( quat != NULL );

	float W = quat->W;
	float X = quat->X;
	float Y = quat->Y;
	float Z = quat->Z;

	float invnorm = 1.0f / ( W*W + X*X + Y*Y + Z*Z );
	invnorm = sqrtf( invnorm );

	quat->W = W * invnorm;
	quat->X = X * invnorm;
	quat->Y = Y * invnorm;
	quat->Z = Z * invnorm;
}


// invert a unit quaternion ---------------------------------------------------
//
void QuaternionInvertUnit( Quaternion *quat )
{
	ASSERT( quat != NULL );

	quat->X = -quat->X;
	quat->Y = -quat->Y;
	quat->Z = -quat->Z;
}

void QuaternionInvertUnit_f( Quaternion_f *quat )
{
	ASSERT( quat != NULL );

	quat->X = -quat->X;
	quat->Y = -quat->Y;
	quat->Z = -quat->Z;
}


// invert a general quaternion ------------------------------------------------
//
void QuaternionInvertGeneral( Quaternion *quat )
{
	ASSERT( quat != NULL );

	float W = GEOMV_TO_FLOAT( quat->W );
	float X = GEOMV_TO_FLOAT( quat->X );
	float Y = GEOMV_TO_FLOAT( quat->Y );
	float Z = GEOMV_TO_FLOAT( quat->Z );

	float invmag = 1.0f / ( W*W + X*X + Y*Y + Z*Z );

	quat->W = FLOAT_TO_GEOMV( -W * invmag );
	quat->X = FLOAT_TO_GEOMV( -X * invmag );
	quat->Y = FLOAT_TO_GEOMV( -Y * invmag );
	quat->Z = FLOAT_TO_GEOMV( -Z * invmag );
}

void QuaternionInvertGeneral_f( Quaternion_f *quat )
{
	ASSERT( quat != NULL );

	float W = quat->W;
	float X = quat->X;
	float Y = quat->Y;
	float Z = quat->Z;

	float invmag = 1.0f / ( W*W + X*X + Y*Y + Z*Z );

	quat->W = -W * invmag;
	quat->X = -X * invmag;
	quat->Y = -Y * invmag;
	quat->Z = -Z * invmag;
}


// diagonal forward cycling ---------------------------------------------------
//
static int nxt_wrp[ 3 ] = { 1, 2, 0 };


// create a unit quaternion from a rotation matrix ----------------------------
//
void QuaternionFromMatrx( Quaternion *quat, const Xmatrx matrix )
{
	ASSERT( quat != NULL );
	ASSERT( matrix != NULL );

	float fdiag[ 3 ];
	fdiag[ 0 ] = GEOMV_TO_FLOAT( matrix[ 0 ][ 0 ] );
	fdiag[ 1 ] = GEOMV_TO_FLOAT( matrix[ 1 ][ 1 ] );
	fdiag[ 2 ] = GEOMV_TO_FLOAT( matrix[ 2 ][ 2 ] );

	// temp result
	float qt[4] = {1.0f, 0.0f, 0.0f, 0.0f};	// (W,X,Y,Z)

	float trace = fdiag[ 0 ] + fdiag[ 1 ] + fdiag[ 2 ];
	if ( trace > 0.0 ) {

		float scale = sqrtf( trace + 1.0f );	// 2W=sqrt(4W^2)
		qt[ 0 ] = scale * 0.5f;					// W
		scale = 0.5f / scale;					// 1/(4W)

		qt[ 1 ] = ( GEOMV_TO_FLOAT( matrix[ 2 ][ 1 ] ) - GEOMV_TO_FLOAT( matrix[ 1 ][ 2 ] ) ) * scale;
		qt[ 2 ] = ( GEOMV_TO_FLOAT( matrix[ 0 ][ 2 ] ) - GEOMV_TO_FLOAT( matrix[ 2 ][ 0 ] ) ) * scale;
		qt[ 3 ] = ( GEOMV_TO_FLOAT( matrix[ 1 ][ 0 ] ) - GEOMV_TO_FLOAT( matrix[ 0 ][ 1 ] ) ) * scale;
		
	} else {

		int mxi = 0;
		if ( fdiag[ 1 ] > fdiag[ 0 ] )
			mxi = 1;
		if ( fdiag[ 2 ] > fdiag[ mxi ] )
			mxi = 2;

		int nx1 = nxt_wrp[ mxi ];
		int nx2 = nxt_wrp[ nx1 ];

		float scale = sqrtf( fdiag[ mxi ] - ( fdiag[ nx1 ] + fdiag[ nx2 ] ) + 1.0f );
		qt[ mxi+1 ] = scale * 0.5f;
		scale = 0.5f / scale;

		qt[   0   ] = ( GEOMV_TO_FLOAT( matrix[ nx2 ][ nx1 ] ) - GEOMV_TO_FLOAT( matrix[ nx1 ][ nx2 ] ) ) * scale;
		qt[ nx1+1 ] = ( GEOMV_TO_FLOAT( matrix[ nx1 ][ mxi ] ) + GEOMV_TO_FLOAT( matrix[ mxi ][ nx1 ] ) ) * scale;
		qt[ nx2+1 ] = ( GEOMV_TO_FLOAT( matrix[ nx2 ][ mxi ] ) + GEOMV_TO_FLOAT( matrix[ mxi ][ nx2 ] ) ) * scale;
	}

	quat->W = FLOAT_TO_GEOMV( qt[ 0 ] );
	quat->X = FLOAT_TO_GEOMV( qt[ 1 ] );
	quat->Y = FLOAT_TO_GEOMV( qt[ 2 ] );
	quat->Z = FLOAT_TO_GEOMV( qt[ 3 ] );
}

void QuaternionFromMatrx_f( Quaternion_f *quat, const Xmatrx matrix )
{
	ASSERT( quat != NULL );
	ASSERT( matrix != NULL );

	float fdiag[ 3 ];
	fdiag[ 0 ] = GEOMV_TO_FLOAT( matrix[ 0 ][ 0 ] );
	fdiag[ 1 ] = GEOMV_TO_FLOAT( matrix[ 1 ][ 1 ] );
	fdiag[ 2 ] = GEOMV_TO_FLOAT( matrix[ 2 ][ 2 ] );

	// direct result
	float *qt = (float *) quat;	// (W,X,Y,Z)

	float trace = fdiag[ 0 ] + fdiag[ 1 ] + fdiag[ 2 ];
	if ( trace > 0.0 ) {

		float scale = sqrtf( trace + 1.0f );	// 2W=sqrt(4W^2)
		qt[ 0 ] = scale * 0.5f;					// W
		scale = 0.5f / scale;					// 1/(4W)

		qt[ 1 ] = ( GEOMV_TO_FLOAT( matrix[ 2 ][ 1 ] ) - GEOMV_TO_FLOAT( matrix[ 1 ][ 2 ] ) ) * scale;
		qt[ 2 ] = ( GEOMV_TO_FLOAT( matrix[ 0 ][ 2 ] ) - GEOMV_TO_FLOAT( matrix[ 2 ][ 0 ] ) ) * scale;
		qt[ 3 ] = ( GEOMV_TO_FLOAT( matrix[ 1 ][ 0 ] ) - GEOMV_TO_FLOAT( matrix[ 0 ][ 1 ] ) ) * scale;
		
	} else {

		int mxi = 0;
		if ( fdiag[ 1 ] > fdiag[ 0 ] )
			mxi = 1;
		if ( fdiag[ 2 ] > fdiag[ mxi ] )
			mxi = 2;

		int nx1 = nxt_wrp[ mxi ];
		int nx2 = nxt_wrp[ nx1 ];

		float scale = sqrtf( fdiag[ mxi ] - ( fdiag[ nx1 ] + fdiag[ nx2 ] ) + 1.0f );
		qt[ mxi+1 ] = scale * 0.5f;
		scale = 0.5f / scale;

		qt[   0   ] = ( GEOMV_TO_FLOAT( matrix[ nx2 ][ nx1 ] ) - GEOMV_TO_FLOAT( matrix[ nx1 ][ nx2 ] ) ) * scale;
		qt[ nx1+1 ] = ( GEOMV_TO_FLOAT( matrix[ nx1 ][ mxi ] ) + GEOMV_TO_FLOAT( matrix[ mxi ][ nx1 ] ) ) * scale;
		qt[ nx2+1 ] = ( GEOMV_TO_FLOAT( matrix[ nx2 ][ mxi ] ) + GEOMV_TO_FLOAT( matrix[ mxi ][ nx2 ] ) ) * scale;
	}
}


// create a rotation matrix from a unit quaternion ----------------------------
//
void MatrxFromQuaternion( Xmatrx matrix, const Quaternion *quat )
{
	ASSERT( matrix != NULL );
	ASSERT( quat != NULL );

	//NOTE:
	// the translation in the matrix will be
	// left untouched.

	//NOTE:
	// unit quaternion check is only assertion.

	ASSERT( QuaternionIsUnit( quat ) );

	// calculate intermediate terms
	geomv_t X2 = quat->X * 2;
	geomv_t Y2 = quat->Y * 2;
	geomv_t Z2 = quat->Z * 2;

	geomv_t XX = GEOMV_MUL( quat->X, X2 );
	geomv_t YY = GEOMV_MUL( quat->Y, Y2 );
	geomv_t ZZ = GEOMV_MUL( quat->Z, Z2 );
	geomv_t XY = GEOMV_MUL( quat->X, Y2 );
	geomv_t XZ = GEOMV_MUL( quat->X, Z2 );
	geomv_t YZ = GEOMV_MUL( quat->Y, Z2 );
	geomv_t WX = GEOMV_MUL( quat->W, X2 );
	geomv_t WY = GEOMV_MUL( quat->W, Y2 );
	geomv_t WZ = GEOMV_MUL( quat->W, Z2 );

	// convert quaternion into rotation matrix
	matrix[ 0 ][ 0 ] = GEOMV_1 - ( YY + ZZ );
	matrix[ 0 ][ 1 ] =           ( XY - WZ );
	matrix[ 0 ][ 2 ] =           ( XZ + WY );
//	matrix[ 0 ][ 3 ] = GEOMV_0;
	matrix[ 1 ][ 0 ] =           ( XY + WZ );
	matrix[ 1 ][ 1 ] = GEOMV_1 - ( XX + ZZ );
	matrix[ 1 ][ 2 ] =           ( YZ - WX );
//	matrix[ 1 ][ 3 ] = GEOMV_0;
	matrix[ 2 ][ 0 ] =           ( XZ - WY );
	matrix[ 2 ][ 1 ] =           ( YZ + WX );
	matrix[ 2 ][ 2 ] = GEOMV_1 - ( XX + YY );
//	matrix[ 2 ][ 3 ] = GEOMV_0;
}

void MatrxFromQuaternion_f( Xmatrx matrix, const Quaternion_f *quat )
{
	ASSERT( matrix != NULL );
	ASSERT( quat != NULL );

	//NOTE:
	// the translation in the matrix will be
	// left untouched.

	//NOTE:
	// unit quaternion check is only assertion.

	ASSERT( QuaternionIsUnit_f( quat ) );

	// calculate intermediate terms
	float X2 = quat->X * 2;
	float Y2 = quat->Y * 2;
	float Z2 = quat->Z * 2;

	float XX = quat->X * X2;
	float YY = quat->Y * Y2;
	float ZZ = quat->Z * Z2;
	float XY = quat->X * Y2;
	float XZ = quat->X * Z2;
	float YZ = quat->Y * Z2;
	float WX = quat->W * X2;
	float WY = quat->W * Y2;
	float WZ = quat->W * Z2;

	// convert quaternion into rotation matrix
	matrix[ 0 ][ 0 ] = FLOAT_TO_GEOMV( 1 - ( YY + ZZ ) );
	matrix[ 0 ][ 1 ] = FLOAT_TO_GEOMV(     ( XY - WZ ) );
	matrix[ 0 ][ 2 ] = FLOAT_TO_GEOMV(     ( XZ + WY ) );
//	matrix[ 0 ][ 3 ] = GEOMV_0;
	matrix[ 1 ][ 0 ] = FLOAT_TO_GEOMV(     ( XY + WZ ) );
	matrix[ 1 ][ 1 ] = FLOAT_TO_GEOMV( 1 - ( XX + ZZ ) );
	matrix[ 1 ][ 2 ] = FLOAT_TO_GEOMV(     ( YZ - WX ) );
//	matrix[ 1 ][ 3 ] = GEOMV_0;
	matrix[ 2 ][ 0 ] = FLOAT_TO_GEOMV(     ( XZ - WY ) );
	matrix[ 2 ][ 1 ] = FLOAT_TO_GEOMV(     ( YZ + WX ) );
	matrix[ 2 ][ 2 ] = FLOAT_TO_GEOMV( 1 - ( XX + YY ) );
//	matrix[ 2 ][ 3 ] = GEOMV_0;
}


// create a rotation matrix from an angular displacement (angle, axis) --------
//
void MatrxFromAngularDisplacement( Xmatrx matrix, bams_t angle, Vertex3 *axis )
{
	ASSERT( matrix != NULL );
	ASSERT( axis != NULL );

	//NOTE:
	// NULL axes are not handled transparently.

	// normalize axis of rotation
	NormVctX( axis );

	// calculate unit quaternion corresponding to rotation
	float phi2	= -BAMS_TO_RAD( angle ) / 2;
	float sinphi2	= sinf( phi2 );

	Quaternion_f quat;
	quat.W = cosf( phi2 );
	quat.X = sinphi2 * GEOMV_TO_FLOAT( axis->X );
	quat.Y = sinphi2 * GEOMV_TO_FLOAT( axis->Y );
	quat.Z = sinphi2 * GEOMV_TO_FLOAT( axis->Z );

	// calculate intermediate terms
	float X2 = quat.X * quat.X;
	float Y2 = quat.Y * quat.Y;
	float Z2 = quat.Z * quat.Z;
	float XY = quat.X * quat.Y;
	float XZ = quat.X * quat.Z;
	float YZ = quat.Y * quat.Z;
	float WX = quat.W * quat.X;
	float WY = quat.W * quat.Y;
	float WZ = quat.W * quat.Z;

	// convert quaternion into rotation matrix
	matrix[ 0 ][ 0 ] = FLOAT_TO_GEOMV( 1 - ( Y2 + Z2 ) * 2 );
	matrix[ 0 ][ 1 ] = FLOAT_TO_GEOMV(     ( XY - WZ ) * 2 );
	matrix[ 0 ][ 2 ] = FLOAT_TO_GEOMV(     ( XZ + WY ) * 2 );
	matrix[ 0 ][ 3 ] = GEOMV_0;
	matrix[ 1 ][ 0 ] = FLOAT_TO_GEOMV(     ( XY + WZ ) * 2 );
	matrix[ 1 ][ 1 ] = FLOAT_TO_GEOMV( 1 - ( X2 + Z2 ) * 2 );
	matrix[ 1 ][ 2 ] = FLOAT_TO_GEOMV(     ( YZ - WX ) * 2 );
	matrix[ 1 ][ 3 ] = GEOMV_0;
	matrix[ 2 ][ 0 ] = FLOAT_TO_GEOMV(     ( XZ - WY ) * 2 );
	matrix[ 2 ][ 1 ] = FLOAT_TO_GEOMV(     ( YZ + WX ) * 2 );
	matrix[ 2 ][ 2 ] = FLOAT_TO_GEOMV( 1 - ( X2 + Y2 ) * 2 );
	matrix[ 2 ][ 3 ] = GEOMV_0;
}


// multiply two general quaternions -------------------------------------------
//
void QuaternionMUL( Quaternion *qd, const Quaternion *qb, const Quaternion *qa )
{
	ASSERT( qd != NULL );
	ASSERT( qb != NULL );
	ASSERT( qa != NULL );

	//NOTE:
	// QD = QB*QA;

	float BW = GEOMV_TO_FLOAT( qb->W ), AW = GEOMV_TO_FLOAT( qa->W );
	float BX = GEOMV_TO_FLOAT( qb->X ), AX = GEOMV_TO_FLOAT( qa->X );
	float BY = GEOMV_TO_FLOAT( qb->Y ), AY = GEOMV_TO_FLOAT( qa->Y );
	float BZ = GEOMV_TO_FLOAT( qb->Z ), AZ = GEOMV_TO_FLOAT( qa->Z );

	qd->W = FLOAT_TO_GEOMV( BW*AW - BX*AX - BY*AY - BZ*AZ );
	qd->X = FLOAT_TO_GEOMV( BY*AZ - AY*BZ + BW*AX + AW*BX );
	qd->Y = FLOAT_TO_GEOMV( BZ*AX - AZ*BX + BW*AY + AW*BY );
	qd->Z = FLOAT_TO_GEOMV( BX*AY - AX*BY + BW*AZ + AW*BZ );
}

void QuaternionMUL_f( Quaternion_f *qd, const Quaternion_f *qb, const Quaternion_f *qa )
{
	ASSERT( qd != NULL );
	ASSERT( qb != NULL );
	ASSERT( qa != NULL );

	//NOTE:
	// QD = QB*QA;

	float BW = qb->W, AW = qa->W;
	float BX = qb->X, AX = qa->X;
	float BY = qb->Y, AY = qa->Y;
	float BZ = qb->Z, AZ = qa->Z;

	qd->W = BW*AW - BX*AX - BY*AY - BZ*AZ;
	qd->X = BY*AZ - AY*BZ + BW*AX + AW*BX;
	qd->Y = BZ*AX - AZ*BX + BW*AY + AW*BY;
	qd->Z = BX*AY - AX*BY + BW*AZ + AW*BZ;
}


// flags and bounds for quaternion slerp --------------------------------------
//
#define SLERP_ALWAYS_SHORTEST

#define EPS_COSOM		1e-5


// interpolate spherically between two unit quaternions -----------------------
//
void QuaternionSlerp( Quaternion *qd, const Quaternion *qa, const Quaternion *qb, float alpha )
{
	ASSERT( qd != NULL );
	ASSERT( qa != NULL );
	ASSERT( qb != NULL );

	//NOTE:
	// QD = SLERP(QA,QB,ALPHA);
	// (  = QA->[ALPHA*ANGLE]->QB ) )

	float scalea;
	float scaleb;

	float AW = GEOMV_TO_FLOAT( qa->W ), BW = GEOMV_TO_FLOAT( qb->W );
	float AX = GEOMV_TO_FLOAT( qa->X ), BX = GEOMV_TO_FLOAT( qb->X );
	float AY = GEOMV_TO_FLOAT( qa->Y ), BY = GEOMV_TO_FLOAT( qb->Y );
	float AZ = GEOMV_TO_FLOAT( qa->Z ), BZ = GEOMV_TO_FLOAT( qb->Z );

	float cosom = AX * BX + AY * BY + AZ * BZ + AW * BW;

#ifdef SLERP_ALWAYS_SHORTEST

	// resolve ambiguity
	if ( cosom < 0.0f ) {
		cosom = -cosom;
		BW    = -BW;
		BX    = -BX;
		BY    = -BY;
		BZ    = -BZ;
	}

	if ( ( 1.0f - cosom ) > EPS_COSOM ) {

		// full slerp
		float omega = acosf( cosom );
		float sinom = 1.0f / sinf( omega );

		alpha *= omega;
		scalea = sinf( omega - alpha ) * sinom;
		scaleb = sinf(         alpha ) * sinom;

	} else {

		// use lerp if cos(omega) about 1.0
		scalea = 1.0f - alpha;
		scaleb = alpha;
	}

	qd->W = FLOAT_TO_GEOMV( scalea * AW + scaleb * BW );
	qd->X = FLOAT_TO_GEOMV( scalea * AX + scaleb * BX );
	qd->Y = FLOAT_TO_GEOMV( scalea * AY + scaleb * BY );
	qd->Z = FLOAT_TO_GEOMV( scalea * AZ + scaleb * BZ );

#else // SLERP_ALWAYS_SHORTEST

	if ( ( 1.0f + cosom ) > EPS_COSOM ) {

		if ( ( 1.0f - cosom ) > EPS_COSOM ) {

			// full slerp
			float omega = acos( cosom );
			float sinom = 1.0f / sin( omega );

			alpha *= omega;
			scalea = sin( omega - alpha ) * sinom;
			scaleb = sin(         alpha ) * sinom;

		} else {

			// use lerp if cos(omega) about 1.0
			scalea = 1.0f - alpha;
			scaleb = alpha;
		}

		qd->W = FLOAT_TO_GEOMV( scalea * AW + scaleb * BW );
		qd->X = FLOAT_TO_GEOMV( scalea * AX + scaleb * BX );
		qd->Y = FLOAT_TO_GEOMV( scalea * AY + scaleb * BY );
		qd->Z = FLOAT_TO_GEOMV( scalea * AZ + scaleb * BZ );

	} else {

		// handle diametrically opposite quaternions
		qd->W =  qa->Z;
		qd->X = -qa->Y;
		qd->Y =  qa->X;
		qd->Z = -qa->W;

		alpha *= HPREC_HALF_PI;
		scalea = sin( HPREC_HALF_PI - alpha );
		scaleb = sin(                 alpha );

		qd->X = FLOAT_TO_GEOMV( scalea * AX + scaleb * GEOMV_TO_FLOAT( qd->X ) );
		qd->Y = FLOAT_TO_GEOMV( scalea * AY + scaleb * GEOMV_TO_FLOAT( qd->Y ) );
		qd->Z = FLOAT_TO_GEOMV( scalea * AZ + scaleb * GEOMV_TO_FLOAT( qd->Z ) );
	}

#endif // SLERP_ALWAYS_SHORTEST

}

void QuaternionSlerp_f( Quaternion_f *qd, const Quaternion_f *qa, const Quaternion_f *qb, float alpha )
{
	ASSERT( qd != NULL );
	ASSERT( qa != NULL );
	ASSERT( qb != NULL );

	//NOTE:
	// QD = SLERP(QA,QB,ALPHA);
	// (  = QA->[ALPHA*ANGLE]->QB ) )

	float scalea;
	float scaleb;

	float BW = qb->W;
	float BX = qb->X;
	float BY = qb->Y;
	float BZ = qb->Z;

	float cosom = qa->X * BX + qa->Y * BY + qa->Z * BZ + qa->W * BW;

#ifdef SLERP_ALWAYS_SHORTEST

	// resolve ambiguity
	if ( cosom < 0.0f ) {
		cosom = -cosom;
		BW    = -BW;
		BX    = -BX;
		BY    = -BY;
		BZ    = -BZ;
	}

	if ( ( 1.0f - cosom ) > EPS_COSOM ) {

		// full slerp
		float omega = acosf( cosom );
		float sinom = 1.0f / sinf( omega );

		alpha *= omega;
		scalea = sinf( omega - alpha ) * sinom;
		scaleb = sinf(         alpha ) * sinom;

	} else {

		// use lerp if cos(omega) about 1.0
		scalea = 1.0f - alpha;
		scaleb = alpha;
	}

	qd->W = scalea * qa->W + scaleb * BW;
	qd->X = scalea * qa->X + scaleb * BX;
	qd->Y = scalea * qa->Y + scaleb * BY;
	qd->Z = scalea * qa->Z + scaleb * BZ;

#else // SLERP_ALWAYS_SHORTEST

	if ( ( 1.0f + cosom ) > EPS_COSOM ) {

		if ( ( 1.0f - cosom ) > EPS_COSOM ) {

			// full slerp
			float omega = acos( cosom );
			float sinom = 1.0f / sin( omega );

			alpha *= omega;
			scalea = sin( omega - alpha ) * sinom;
			scaleb = sin(         alpha ) * sinom;

		} else {

			// use lerp if cos(omega) about 1.0
			scalea = 1.0f - alpha;
			scaleb = alpha;
		}

		qd->W = scalea * qa->W + scaleb * BW;
		qd->X = scalea * qa->X + scaleb * BX;
		qd->Y = scalea * qa->Y + scaleb * BY;
		qd->Z = scalea * qa->Z + scaleb * BZ;
		
	} else {
		
		// handle diametrically opposite quaternions
		qd->W =  qa->Z;
		qd->X = -qa->Y;
		qd->Y =  qa->X;
		qd->Z = -qa->W;
		
		alpha *= HPREC_HALF_PI;
		scalea = sin( HPREC_HALF_PI - alpha );
		scaleb = sin(                 alpha );
		
		qd->X = scalea * qa->X + scaleb * qd->X;
		qd->Y = scalea * qa->Y + scaleb * qd->Y;
		qd->Z = scalea * qa->Z + scaleb * qd->Z;
	}
	
#endif // SLERP_ALWAYS_SHORTEST
	
}

// do a quaternion slerp from frame to frame ----------------------------------
//
void QuaternionSlerpFrames( Xmatrx slerp_frame, Xmatrx start_frame, Xmatrx end_frame, float t )
{
	// get source orientation quaternion
	Quaternion srcquat;
	QuaternionFromMatrx( &srcquat, start_frame );
	QuaternionMakeUnit( &srcquat );
	
	// get destination orientation quaternion
	Quaternion dstquat;
	QuaternionFromMatrx( &dstquat, end_frame );
	QuaternionMakeUnit( &dstquat );
	
	// do slerp from src to dst (filter output this frame)
	Quaternion slerpquat;
	QuaternionSlerp( &slerpquat, &srcquat, &dstquat, t );
	QuaternionMakeUnit( &slerpquat );
	
	// fill R part of view camera matrix (filtered orientation this frame)
	MatrxFromQuaternion( slerp_frame, &slerpquat );
}

// calc rotation matrix as result of slerp between two quaternions ------------
//
void CalcSlerpedMatrix( Xmatrx matrix, const playerlerp_s *playerlerp )
{
	ASSERT( matrix != NULL );
	ASSERT( playerlerp != NULL );

	// do slerp from src to dst
	Quaternion slerpquat;
	QuaternionSlerp( &slerpquat, &playerlerp->srcquat, &playerlerp->dstquat, playerlerp->curalpha );
	QuaternionMakeUnit( &slerpquat );

	// fill R part of matrix
	MatrxFromQuaternion( matrix, &slerpquat );
}


// interpolate using the Hermite Interpolation --------------------------------
//
void Hermite_Interpolate( Vector3* dest, float t, const Vector3* start, const Vector3* end, 
						 const Vector3* start_tan, const Vector3* end_tan )
{
	// do a Hermite interpolation for the position
	float s  = t;
	float s2 = s  * s;
	float s3 = s2 * s;
	
	// calculate the basis functions
	float h1 =  2 * s3 - 3 * s2 + 1;
	float h2 = -2 * s3 + 3 * s2;
	float h3 =      s3 - 2 * s2 + s;
	float h4 =      s3 - s2;
	
	Vector3 h1P1, h2P2, h3T1, h4T2;
	
	VECMULS( &h1P1, start,		(geomv_t)h1 );
	VECMULS( &h2P2, end,		(geomv_t)h2 );
	VECMULS( &h3T1, start_tan,	(geomv_t)h3 );
	VECMULS( &h4T2, end_tan,	(geomv_t)h4 );
	
	Vector3 hermite_pos;
	VECADD( &hermite_pos, &h1P1, &h2P2 );
	VECADD( &hermite_pos, &h3T1, &hermite_pos );
	VECADD( &hermite_pos, &h4T2, &hermite_pos );
	
	memcpy( dest, &hermite_pos, sizeof( Vector3 ) );	
}


// init a the Hermite ArcLen interpolation struct -----------------------------
//
Hermite_ArcLen* Hermite_ArcLen_InitData( int num_steps, const Vector3* start, const Vector3* end, 
										 const Vector3* start_tan, const Vector3* end_tan )
{
	Hermite_ArcLen* hermite_data = (Hermite_ArcLen*)ALLOCMEM( sizeof( Hermite_ArcLen ) );

	// init the arrays
	size_t allocsize1	= (size_t)num_steps * sizeof( float );
	size_t allocsize2	= (size_t)num_steps * sizeof( Vector3 );
	hermite_data->table_u		= (float*)ALLOCMEM( allocsize1 );
	hermite_data->table_s		= (float*)ALLOCMEM( allocsize1 );
	hermite_data->table_Vecs	= (Vector3*)ALLOCMEM( allocsize2 );
	
	memset( hermite_data->table_u,		0, allocsize1 );
	memset( hermite_data->table_s,		0, allocsize1 );
	memset( hermite_data->table_Vecs,	0, allocsize2 );

	//NOTE: u is original parameter space, s is arclen parameter space

	// store the # of steps for the interpolation 
	hermite_data->num_steps = num_steps;
	
	// store u and corresponding value
	int step = 0;
	for(step = 0; step < num_steps; step++ ) {

		// normalize u
		float u = (float)step / (float)( num_steps - 1 );
	
		hermite_data->table_u[ step ] = u;

		Hermite_Interpolate( &hermite_data->table_Vecs[ step ], u, start, end, start_tan, end_tan );
	}

	// calculate total arclength
	hermite_data->total_arc_len = 0.0f;

	// store arclen for interpolation step s == 0
	hermite_data->table_s[ 0 ]	= 0.0f;

	Vector3 diff;
	for( step = 1; step < num_steps; step++ ) {

		// get the (approximated) arclen of vector ( step - 1 ) -> ( step )
		VECSUB( &diff, &hermite_data->table_Vecs[ step - 1 ], &hermite_data->table_Vecs[ step ] );
		float arc_len = GEOMV_TO_FLOAT( VctLenX( &diff ) );
		hermite_data->total_arc_len += arc_len;

		// store accumulated arclen in table_s
		hermite_data->table_s[ step  ] = hermite_data->total_arc_len;
	}
	
	// normalize arc lens
	for( step = 0; step < num_steps; step++ ) {
		hermite_data->table_s[ step ] = hermite_data->table_s[ step ] / hermite_data->total_arc_len;
	}
	
	return hermite_data;
}

// kill data needed for the Hermite ArcLen interpolation ----------------------
//
void Hermite_ArcLen_KillData( Hermite_ArcLen* hermite_data )
{
	ASSERT( hermite_data			 != NULL );
	ASSERT( hermite_data->table_u	 != NULL );
	ASSERT( hermite_data->table_s    != NULL );
	ASSERT( hermite_data->table_Vecs != NULL );
	
	FREEMEM( hermite_data->table_u );
	FREEMEM( hermite_data->table_s );
	FREEMEM( hermite_data->table_Vecs );
}

// do arclen interpolation of the supplied hermite arclen data ----------------
//
void Hermite_ArcLen_Interpolate( const Hermite_ArcLen* hermite_data, float s, Vector3* interp )
{
	ASSERT( hermite_data != NULL );
	ASSERT( ( s >= 0 ) && ( s <= 1.0f ) );
	ASSERT( interp != NULL );
	
	//NOTE: given a parameter s in arclen [0,1] we find an u in original parameter space
	
	ASSERT( hermite_data->num_steps >= 2 );
	
	int left  = 0;
	int right = ( hermite_data->num_steps - 1 );
	
	float* table_s = hermite_data->table_s;
	
	int start = 0, end = 0;
	
	for( ; right >= left; ) {
		int x = ( left + right ) / 2;
		
		if( table_s[ x ] == s ) {
			memcpy( interp, &hermite_data->table_Vecs[ x ], sizeof( Vector3 ) );
			return;
		} else if ( s < table_s[ x ] ) {
			if ( s > table_s[ x - 1 ] ) {
				start = x - 1;
				end   = x;
				break;
			} else {
				right = x - 1;
			}
		} else {
			if ( s < table_s[ x + 1 ]  ) {
				start = x;
				end   = x + 1;
				break;
			} else {
				left  = x + 1;
			}
		}
	}
	
	Vector3* start_vec = &hermite_data->table_Vecs[ start ];
	Vector3* end_vec   = &hermite_data->table_Vecs[ end   ];
	
	float alpha = ( s - table_s[ start ] ) / ( table_s[ end ] - table_s[ start ] );
	
	interp->X = start_vec->X + GEOMV_TO_FLOAT( end_vec->X - start_vec->X ) * FLOAT_TO_GEOMV( alpha );
	interp->Y = start_vec->Y + GEOMV_TO_FLOAT( end_vec->Y - start_vec->Y ) * FLOAT_TO_GEOMV( alpha );
	interp->Z = start_vec->Z + GEOMV_TO_FLOAT( end_vec->Z - start_vec->Z ) * FLOAT_TO_GEOMV( alpha );
}


// dump the contents of a matrix ----------------------------------------------
//
void DumpMatrix( Xmatrx mat )
{
	MSGOUT( "%10.4f %10.4f %10.4f %10.4f | %10.4f %10.4f %10.4f %10.4f | %10.4f %10.4f %10.4f %10.4f", 
		mat[ 0 ][ 0 ], mat[ 0 ][ 1 ], mat[ 0 ][ 2 ], mat[ 0 ][ 3 ], 
		mat[ 1 ][ 0 ], mat[ 1 ][ 1 ], mat[ 1 ][ 2 ], mat[ 1 ][ 3 ],
		mat[ 2 ][ 0 ], mat[ 2 ][ 1 ], mat[ 2 ][ 2 ], mat[ 2 ][ 3 ] );
}


