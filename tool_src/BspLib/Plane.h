//-----------------------------------------------------------------------------
//	BSPLIB HEADER: Plane.h
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _PLANE_H_
#define _PLANE_H_

// bsplib header files
#include "BspLibDefs.h"
#include "Vertex.h"


BSPLIB_NAMESPACE_BEGIN


// plane in 3-space; represented by normal and distance to origin -------------
//
class Plane {

	enum {
		NORMAL_VALID	= 0x0001,
		OFFSET_VALID	= 0x0002,
		PLANE_VALID		= NORMAL_VALID | OFFSET_VALID,
	};

public:
	Plane() : m_valid( 0 ) { }
	Plane( const Vector3& pnormal );
	Plane( const Vector3& pnormal, double poffset );
	Plane( const Vertex3& v1, const Vertex3& v2, const Vertex3& v3 );
	~Plane() { }

	int			InitPlane( const Vertex3& v1, const Vertex3& v2, const Vertex3& v3 );
	int			CalcPlaneOffset( const Vertex3& vtx );

	Vector3		getPlaneNormal() const { return m_normal; }
	void		setPlaneNormal( const Vector3& pnormal ) { m_normal = pnormal; }

	double		getPlaneOffset() const { return m_offset; }
	void		setPlaneOffset( double poffset ) { m_offset = poffset; }

	int			NormalValid() const { return ( ( m_valid & NORMAL_VALID ) == NORMAL_VALID ); }
	int			PlaneValid() const { return ( ( m_valid & PLANE_VALID ) == PLANE_VALID ); }

	void		ApplyScaleFactor( double sfac );
	void		EliminateDirectionality();

	int			PointContained( const Vertex3& point ) const;
	int			PointInPositiveHalfspace( const Vertex3& point ) const;
	int			PointInNegativeHalfspace( const Vertex3& point ) const;

private:
	Vector3		m_normal;	// normal of plane
	double		m_offset;	// distance of plane to origin
	int			m_valid;	// flag if plane specification indeed valid
};

// construct a plane from plane normal only: leave offset invalid -------------
inline Plane::Plane( const Vector3& pnormal )
{
	m_normal = pnormal;
	m_offset = 0.0;
	m_valid  = NORMAL_VALID;
}

// construct a plane from plane normal and distance to origin -----------------
inline Plane::Plane( const Vector3& pnormal, double poffset )
{
	m_normal = pnormal;
	m_offset = poffset;
	m_valid	 = PLANE_VALID;
}

// construct a plane from three vertices (affine combination) -----------------
inline Plane::Plane( const Vertex3& v1, const Vertex3& v2, const Vertex3& v3 )
{
	// normal is calculated so that the three vertices comprise a
	// triangle with the front face in clockwise order
	m_normal.CrossProduct( v3 - v1, v2 - v1 );

	// m_valid will be set to 0 if the three vertices are collinear
	m_valid = m_normal.Normalize() ? PLANE_VALID : 0;

	// compute offset via dot product
	m_offset = m_normal.DotProduct( v1 );
}

// init plane from three points (after construction!) -------------------------
inline int Plane::InitPlane( const Vertex3& v1, const Vertex3& v2, const Vertex3& v3 )
{
	*this = Plane( v1, v2, v3 );
	return m_valid;
}

// calculate plane's distance to origin using already valid normal ------------
inline int Plane::CalcPlaneOffset( const Vertex3& vtx )
{
	// compute offset via dot product
	m_offset = m_normal.DotProduct( vtx );
	return ( m_valid |= OFFSET_VALID );
}

// apply scale factor to plane's distance to origin ---------------------------
inline void Plane::ApplyScaleFactor( double sfac )
{
	// this will be needed if an object containing
	// explicit plane information is scaled.
	m_offset *= sfac;
}

// eliminate frontfacing/backfacing property; force positive offset -----------
inline void Plane::EliminateDirectionality()
{
	if ( m_offset < 0.0 ) {
		m_normal *= -1.0;
		m_offset  = -m_offset;
	}
}

// determine if a point is contained in the plane -----------------------------
inline int Plane::PointContained( const Vertex3& point ) const
{
	return ( fabs( m_normal.DotProduct( point ) - m_offset ) < EPS_POINT_ON_PLANE );
}

// determine if a point is contained in the positive open halfspace -----------
inline int Plane::PointInPositiveHalfspace( const Vertex3& point ) const
{
	return ( m_normal.DotProduct( point ) - m_offset >= EPS_POINT_ON_PLANE );
}

// determine if a point is contained in the negative open halfspace -----------
inline int Plane::PointInNegativeHalfspace( const Vertex3& point ) const
{
	return ( m_normal.DotProduct( point ) - m_offset <= -EPS_POINT_ON_PLANE );
}


BSPLIB_NAMESPACE_END


#endif // _PLANE_H_

