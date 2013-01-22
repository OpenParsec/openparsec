//-----------------------------------------------------------------------------
//	BSPLIB HEADER: Vertex.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _VERTEX_H_
#define _VERTEX_H_

// bsplib header files
#include "BspLibDefs.h"


BSPLIB_NAMESPACE_BEGIN


// single 3-D vertex ----------------------------------------------------------
//
class Vertex3 {

	friend class Vector3;
	friend class LineSeg3;

	friend int operator ==( const Vertex3 v1, const Vertex3 v2 );
	friend int operator !=( const Vertex3 v1, const Vertex3 v2 );

	friend class Vector3 operator -( const Vertex3 v1, const Vertex3 v2 );

	friend Vertex3 operator +( const Vertex3 v1, const class ector3 v2 );
	friend Vertex3 operator +( const class Vector3 v1, const Vertex3 v2 );
	friend Vertex3 operator -( const Vertex3 v1, const class Vector3 v2 );

public:
	Vertex3( hprec_t x = 0, hprec_t y = 0, hprec_t z = 0, hprec_t w = 1.0 );
	~Vertex3() { }

	Vertex3& operator +=( const Vector3 v );
	Vertex3& operator -=( const Vector3 v );

	int  IsInVicinity( const Vertex3& vertex ) const;
	void ChangeAxes( const char *xchangecmd, const Vertex3& ads );

	hprec_t getX() const { return X; }
	hprec_t getY() const { return Y; }
	hprec_t getZ() const { return Z; }
 	hprec_t getW() const { return W; }

	void setX( hprec_t x ) { X = x; }
	void setY( hprec_t y ) { Y = y; }
	void setZ( hprec_t z ) { Z = z; }
	void setW( hprec_t w ) { W = w; }
	hprec_t X;
	hprec_t Y;
	hprec_t Z;
	hprec_t W;

private:
	void ChangeAxis( const char xchar, const hprec_t src );

protected: 
};


// single 3-D vector ----------------------------------------------------------
//
class Vector3 : public Vertex3 {

	friend class LineSeg3;

	friend Vector3 operator -( const Vertex3 v1, const Vertex3 v2 );

	friend Vertex3 operator +( const Vertex3 v1, const Vector3 v2 );
	friend Vertex3 operator +( const Vector3 v1, const Vertex3 v2 );
	friend Vertex3 operator -( const Vertex3 v1, const Vector3 v2 );

	friend Vector3 operator +( const Vector3 v1, const Vector3 v2 );
	friend Vector3 operator -( const Vector3 v1, const Vector3 v2 );

	friend Vector3 operator *( const Vector3 v1, double t );
	friend Vector3 operator *( double t, const Vector3 v1 );

public:
	Vector3( hprec_t x = 0, hprec_t y = 0, hprec_t z = 0, hprec_t w = 1.0 )
		: Vertex3( x, y, z, w ) { }
	// initialize as direction vector
	Vector3( const Vertex3& v1, const Vertex3& v2 )
		: Vertex3( v2.X - v1.X, v2.Y - v1.Y, v2.Z - v1.Z ) { }
	// initialize as cross product
	Vector3( const Vector3& v1, const Vector3& v2 );
	// conversion from Vertex3
	Vector3( const Vertex3& vtx ) { *(Vertex3*)this = vtx; }
	~Vector3() { }

	Vector3& operator +=( const Vector3 v );
	Vector3& operator -=( const Vector3 v );

	Vector3& operator *=( double t );

	int			Normalize();
	int			Homogenize();
	int			IsNullVector() const;
	hprec_t		VecLength() const;
	hprec_t		DotProduct( const Vector3& vect ) const;
	void		CrossProduct( const Vector3& vect1, const Vector3& vect2 );
	void		CreateDirVec( const Vertex3& vertex1, const Vertex3& vertex2 );
};


// single 2-D vertex ----------------------------------------------------------
//
class Vertex2 {

	friend class Vector2;
	friend class LineSeg2;

	friend int operator ==( const Vertex2 v1, const Vertex2 v2 );
	friend int operator !=( const Vertex2 v1, const Vertex2 v2 );

	friend class Vector2 operator -( const Vertex2 v1, const Vertex2 v2 );

	friend Vertex2 operator +( const Vertex2 v1, const class Vector2 v2 );
	friend Vertex2 operator +( const class Vector2 v1, const Vertex2 v2 );
	friend Vertex2 operator -( const Vertex2 v1, const class Vector2 v2 );

public:
	Vertex2( hprec_t x = 0, hprec_t y = 0, hprec_t w = 1.0 ) { X = x; Y = y; W = w; }
	// conversion Vector2 -> Vertex2
	Vertex2( const Vector2& vect );
	~Vertex2() { }

	Vertex2& operator +=( const Vector2 v );
	Vertex2& operator -=( const Vector2 v );

	int  IsInVicinity( const Vertex2& vertex ) const;
	void InitFromVertex3( const Vertex3& vertex );

	hprec_t getX() const { return X; }
	hprec_t getY() const { return Y; }
 	hprec_t getW() const { return W; }

	void setX( hprec_t x ) { X = x; }
	void setY( hprec_t y ) { Y = y; }
	void setW( hprec_t w ) { W = w; }

protected:
	hprec_t X;
	hprec_t Y;
	hprec_t W;
};


// single 2-D vector ----------------------------------------------------------
//
class Vector2 : public Vertex2 {

	friend class LineSeg2;

	friend Vector2 operator -( const Vertex2 v1, const Vertex2 v2 );

	friend Vertex2 operator +( const Vertex2 v1, const Vector2 v2 );
	friend Vertex2 operator +( const Vector2 v1, const Vertex2 v2 );
	friend Vertex2 operator -( const Vertex2 v1, const Vector2 v2 );

	friend Vector2 operator +( const Vector2 v1, const Vector2 v2 );
	friend Vector2 operator -( const Vector2 v1, const Vector2 v2 );

	friend Vector2 operator *( const Vector2 v1, double t );
	friend Vector2 operator *( double t, const Vector2 v1 );

public:
	Vector2( hprec_t x = 0, hprec_t y = 0, hprec_t w = 1.0 )
		: Vertex2( x, y, w ) { }
	// initialize as direction vector
	Vector2( const Vertex2& v1, const Vertex2& v2 )
		: Vertex2( v2.X - v1.X, v2.Y - v1.Y ) { }
	// conversion from Vertex2
	Vector2( const Vertex2& vtx ) { *(Vertex2*)this = vtx; }
	~Vector2() { }

	Vector2& operator +=( const Vector2 v );
	Vector2& operator -=( const Vector2 v );

	Vector2& operator *=( double t );

	int			Normalize();
	int			Homogenize();
	int			IsNullVector() const;
	hprec_t		VecLength() const;
	hprec_t		DotProduct( const Vector2& vect ) const;
	void		CreateDirVec( const Vertex2& vertex, const Vertex2& dirvec );
};


BSPLIB_NAMESPACE_END


// include vector and lineseg capability, and chunks of vertices
#include "Vector.h"
#include "LineSeg2.h"
#include "LineSeg3.h"
#include "VertexChunk.h"


#endif // _VERTEX_H_

