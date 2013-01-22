//-----------------------------------------------------------------------------
//	BSPLIB HEADER: Vertex3.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _VERTEX3_H_
#define _VERTEX3_H_

// bsplib header files
#include "BspLibDefs.h"


BSPLIB_NAMESPACE_BEGIN


// construct Vertex3 by initializing member coordinates -----------------------
inline Vertex3::Vertex3( hprec_t x, hprec_t y, hprec_t z, hprec_t w )
{
	X = x;
	Y = y;
	Z = z;
	W = w;
}

// operator '==' does memberwise comparison for (X,Y,Z,W) ---------------------
inline int operator ==( const Vertex3 v1, const Vertex3 v2 )
{
	return ( ( v1.X == v2.X ) && ( v1.Y == v2.Y ) && ( v1.Z == v2.Z ) && ( v1.W == v2.W ) );
}

// operator '!=' does memberwise comparison for (X,Y,Z,W) ---------------------
inline int operator !=( const Vertex3 v1, const Vertex3 v2 )
{
	return ( ( v1.X != v2.X ) || ( v1.Y != v2.Y ) || ( v1.Z != v2.Z ) || ( v1.W != v2.W ) );
}

// operator '+=' does memberwise addition for (X,Y,Z) -------------------------
inline Vertex3& Vertex3::operator +=( const Vector3 v )
{
	X += v.X;
	Y += v.Y;
	Z += v.Z;

	return *this;
}

// operator '-=' does memberwise subtraction for (X,Y,Z) ----------------------
inline Vertex3& Vertex3::operator -=( const Vector3 v )
{
	X -= v.X;
	Y -= v.Y;
	Z -= v.Z;

	return *this;
}

// check if specified vertex is in vicinity of local vertex -------------------
inline int Vertex3::IsInVicinity( const Vertex3& vertex ) const
{
	return ( ( fabs( X - vertex.X ) < EPS_VERTEX_MERGE ) &&
			 ( fabs( Y - vertex.Y ) < EPS_VERTEX_MERGE ) &&
			 ( fabs( Z - vertex.Z ) < EPS_VERTEX_MERGE ) );
}


BSPLIB_NAMESPACE_END


#endif // _VERTEX3_H_

