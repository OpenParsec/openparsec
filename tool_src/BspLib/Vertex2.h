//-----------------------------------------------------------------------------
//	BSPLIB HEADER: Vertex2.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _VERTEX2_H_
#define _VERTEX2_H_

// bsplib header files
#include "BspLibDefs.h"


BSPLIB_NAMESPACE_BEGIN


// conversion from Vector2 to Vertex2 -----------------------------------------
inline Vertex2::Vertex2( const Vector2& vect )
{
	X = vect.X;
	Y = vect.Y;
	W = vect.W;
}

// operator '==' does memberwise comparison for (X,Y,W) -----------------------
inline int operator ==( const Vertex2 v1, const Vertex2 v2 )
{
	return ( ( v1.X == v2.X ) && ( v1.Y == v2.Y ) && ( v1.W == v2.W ) );
}

// operator '!=' does memberwise comparison for (X,Y,W) -----------------------
inline int operator !=( const Vertex2 v1, const Vertex2 v2 )
{
	return ( ( v1.X != v2.X ) || ( v1.Y != v2.Y ) || ( v1.W != v2.W ) );
}

// operator '+=' does memberwise addition for (X,Y) ---------------------------
inline Vertex2& Vertex2::operator +=( const Vector2 v )
{
	X += v.X;
	Y += v.Y;

	return *this;
}

// operator '-=' does memberwise subtraction for (X,Y) ------------------------
inline Vertex2& Vertex2::operator -=( const Vector2 v )
{
	X -= v.X;
	Y -= v.Y;

	return *this;
}

// check if specified vertex is in vicinity of local vertex -------------------
inline int Vertex2::IsInVicinity( const Vertex2& vertex ) const
{
	return ( ( fabs( X - vertex.X ) < EPS_VERTEX_MERGE ) &&
			 ( fabs( Y - vertex.Y ) < EPS_VERTEX_MERGE ) );
}

// copy three coordinates into two plus w -------------------------------------
inline void Vertex2::InitFromVertex3( const Vertex3& vertex )
{
	X = vertex.getX();
	Y = vertex.getY();
	W = vertex.getZ();
}


BSPLIB_NAMESPACE_END


#endif // _VERTEX2_H_

