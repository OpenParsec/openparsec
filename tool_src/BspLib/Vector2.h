//-----------------------------------------------------------------------------
//	BSPLIB HEADER: Vector2.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _VECTOR2_H_
#define _VECTOR2_H_

// bsplib header files
#include "BspLibDefs.h"
#include "Vertex2.h"


BSPLIB_NAMESPACE_BEGIN


// vector times scalar (post-multiply) ----------------------------------------
inline Vector2 operator *( const Vector2 v1, double t )
{
	return Vector2( v1.X * t, v1.Y * t, 1.0 );
}

// scalar times vector (pre-multiply) -----------------------------------------
inline Vector2 operator *( double t, const Vector2 v1 )
{
	return Vector2( v1.X * t, v1.Y * t, 1.0 );
}

// operator '+=' does memberwise addition for (X,Y) ---------------------------
inline Vector2& Vector2::operator +=( const Vector2 v )
{
	X += v.X;
	Y += v.Y;
	return *this;
}

// operator '-=' does memberwise subtraction for (X,Y) ------------------------
inline Vector2& Vector2::operator -=( const Vector2 v )
{
	X -= v.X;
	Y -= v.Y;
	return *this;
}

// operator '*=' does multiplication with a scalar ----------------------------
inline Vector2& Vector2::operator *=( double t )
{
	X *= t;
	Y *= t;
	return *this;
}

// normalize vector (homogeneous coordinate will be set to one) ---------------
inline int Vector2::Normalize()
{
	if ( IsNullVector() )
		return FALSE;

	double oonorm = 1 / VecLength();
	X = X * oonorm;
	Y = Y * oonorm;
	W = 1.0;
	return TRUE;
}

// homogenize vector (divide by homogeneous coordinate) -----------------------
inline int Vector2::Homogenize()
{
	if ( fabs( W ) < EPS_DENOM_ZERO )
		return FALSE;

	double oow = 1 / W;
	X = X * oow;
	Y = Y * oow;
	W = 1.0;
	return TRUE;
}

// check if vector has length zero (employs an epsilon area) ------------------
inline int Vector2::IsNullVector() const
{
	// the epsilon area is employed componentwise, not for the length itself!
	return ( ( fabs( X ) < EPS_COMP_ZERO ) && ( fabs( Y ) < EPS_COMP_ZERO ) );
}

// calculate length of two dimensional vector ---------------------------------
inline hprec_t Vector2::VecLength() const
{
	return sqrt( X * X + Y * Y );
}

// calculate floating point scalar-product ------------------------------------
inline hprec_t Vector2::DotProduct( const Vector2& vect ) const
{
	return ( vect.X * X ) + ( vect.Y * Y );
}

// calc vector directed from second vertex to first vertex --------------------
inline void Vector2::CreateDirVec( const Vertex2& vertex1, const Vertex2& vertex2 )
{
	X = vertex2.X - vertex1.X;
	Y = vertex2.Y - vertex1.Y;
	W = 1.0;
}


BSPLIB_NAMESPACE_END


#endif // _VECTOR2_H_

