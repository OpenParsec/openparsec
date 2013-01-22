//-----------------------------------------------------------------------------
//	BSPLIB HEADER: Vector.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _VECTOR_H_
#define _VECTOR_H_

// bsplib header files
#include "Vector3.h"
#include "Vector2.h"


BSPLIB_NAMESPACE_BEGIN


// vertex plus vector yields vertex -------------------------------------------
inline Vertex3 operator +( const Vertex3 v1, const Vector3 v2 )
{
	return Vertex3( v1.X + v2.X, v1.Y + v2.Y, v1.Z + v2.Z, 1.0 );
}

// vector plus vertex yields vertex -------------------------------------------
inline Vertex3 operator +( const Vector3 v1, const Vertex3 v2 )
{
	return Vertex3( v1.X + v2.X, v1.Y + v2.Y, v1.Z + v2.Z, 1.0 );
}

// vertex minus vertex yields vector ------------------------------------------
inline Vector3 operator -( const Vertex3 v1, const Vertex3 v2 )
{
	return Vector3( v1.X - v2.X, v1.Y - v2.Y, v1.Z - v2.Z, 1.0 );
}

// vertex minus vector yields vertex ------------------------------------------
inline Vertex3 operator -( const Vertex3 v1, const Vector3 v2 )
{
	return Vertex3( v1.X - v2.X, v1.Y - v2.Y, v1.Z - v2.Z, 1.0 );
}

// vector plus vector yields vector -------------------------------------------
inline Vector3 operator +( const Vector3 v1, const Vector3 v2 )
{
	return Vector3( v1.X + v2.X, v1.Y + v2.Y, v1.Z + v2.Z, 1.0 );
}

// vector minus vector yields vector ------------------------------------------
inline Vector3 operator -( const Vector3 v1, const Vector3 v2 )
{
	return Vector3( v1.X - v2.X, v1.Y - v2.Y, v1.Z - v2.Z, 1.0 );
}


//-----------------------------------------------------------------------------


// vertex plus vector yields vertex -------------------------------------------
inline Vertex2 operator +( const Vertex2 v1, const Vector2 v2 )
{
	return Vertex2( v1.X + v2.X, v1.Y + v2.Y, 1.0 );
}

// vector plus vertex yields vertex -------------------------------------------
inline Vertex2 operator +( const Vector2 v1, const Vertex2 v2 )
{
	return Vertex2( v1.X + v2.X, v1.Y + v2.Y, 1.0 );
}

// vertex minus vertex yields vector ------------------------------------------
inline Vector2 operator -( const Vertex2 v1, const Vertex2 v2 )
{
	return Vector2( v1.X - v2.X, v1.Y - v2.Y, 1.0 );
}

// vertex minus vector yields vertex ------------------------------------------
inline Vertex2 operator -( const Vertex2 v1, const Vector2 v2 )
{
	return Vertex2( v1.X - v2.X, v1.Y - v2.Y, 1.0 );
}

// vector plus vector yields vector -------------------------------------------
inline Vector2 operator +( const Vector2 v1, const Vector2 v2 )
{
	return Vector2( v1.X + v2.X, v1.Y + v2.Y, 1.0 );
}

// vector minus vector yields vector ------------------------------------------
inline Vector2 operator -( const Vector2 v1, const Vector2 v2 )
{
	return Vector2( v1.X - v2.X, v1.Y - v2.Y, 1.0 );
}


BSPLIB_NAMESPACE_END


#endif // _VECTOR_H_

