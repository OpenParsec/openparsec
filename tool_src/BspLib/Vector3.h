//-----------------------------------------------------------------------------
//	BSPLIB HEADER: Vector3.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _VECTOR3_H_
#define _VECTOR3_H_

// bsplib header files
#include "BspLibDefs.h"
#include "Vertex3.h"


BSPLIB_NAMESPACE_BEGIN


// vector times scalar (post-multiply) ----------------------------------------
inline Vector3 operator *( const Vector3 v1, double t )
{
	return Vector3( v1.X * t, v1.Y * t, v1.Z * t, 1.0 );
}

// scalar times vector (pre-multiply) -----------------------------------------
inline Vector3 operator *( double t, const Vector3 v1 )
{
	return Vector3( v1.X * t, v1.Y * t, v1.Z * t, 1.0 );
}

// construct Vector3 as cross product of two other vectors --------------------
inline Vector3::Vector3( const Vector3& v1, const Vector3& v2 )
{
	CrossProduct( v1, v2 );
}

// operator '+=' does memberwise addition for (X,Y,Z) -------------------------
inline Vector3& Vector3::operator +=( const Vector3 v )
{
	X += v.X;
	Y += v.Y;
	Z += v.Z;
	return *this;
}

// operator '-=' does memberwise subtraction for (X,Y,Z) ----------------------
inline Vector3& Vector3::operator -=( const Vector3 v )
{
	X -= v.X;
	Y -= v.Y;
	Z -= v.Z;
	return *this;
}

// operator '*=' does multiplication with a scalar ----------------------------
inline Vector3& Vector3::operator *=( double t )
{
	X *= t;
	Y *= t;
	Z *= t;
	return *this;
}

// normalize vector (homogeneous coordinate will be set to one) ---------------
inline int Vector3::Normalize()
{
	if ( IsNullVector() )
		return FALSE;

	double oonorm = 1 / VecLength();
	X = X * oonorm;
	Y = Y * oonorm;
	Z = Z * oonorm;
	W = 1.0;
	return TRUE;
}

// homogenize vector (divide by homogeneous coordinate) -----------------------
inline int Vector3::Homogenize()
{
	if ( fabs( W ) < EPS_DENOM_ZERO )
		return FALSE;

	double oow = 1 / W;
	X = X * oow;
	Y = Y * oow;
	Z = Z * oow;
	W = 1.0;
	return TRUE;
}

// check if vector has length zero (employs an epsilon area) ------------------
inline int Vector3::IsNullVector() const
{
	// the epsilon area is employed componentwise, not for the length itself!
	return ( ( fabs( X ) < EPS_COMP_ZERO ) && ( fabs( Y ) < EPS_COMP_ZERO ) && ( fabs( Z ) < EPS_COMP_ZERO ) );
}

// calculate length of three dimensional vector -------------------------------
inline hprec_t Vector3::VecLength() const
{
	return sqrt( X * X + Y * Y + Z * Z );
}

// calculate floating point scalar-product ------------------------------------
inline hprec_t Vector3::DotProduct( const Vector3& vect ) const
{
	return ( vect.X * X ) + ( vect.Y * Y ) + ( vect.Z * Z );
}

// calculate floating point cross-product -------------------------------------
inline void Vector3::CrossProduct( const Vector3& vect1, const Vector3& vect2 )
{
	X = ( vect1.Y * vect2.Z ) - ( vect1.Z * vect2.Y );
	Y = ( vect1.Z * vect2.X ) - ( vect1.X * vect2.Z );
	Z = ( vect1.X * vect2.Y ) - ( vect1.Y * vect2.X );
	W = 1.0;
}

// calc vector directed from second vertex to first vertex --------------------
inline void Vector3::CreateDirVec( const Vertex3& vertex1, const Vertex3& vertex2 )
{
	X = vertex2.X - vertex1.X;
	Y = vertex2.Y - vertex1.Y;
	Z = vertex2.Z - vertex1.Z;
	W = 1.0;
}


BSPLIB_NAMESPACE_END


#endif // _VECTOR3_H_

