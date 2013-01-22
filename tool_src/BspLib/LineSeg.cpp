//-----------------------------------------------------------------------------
//	BSPLIB MODULE: LineSeg.cpp
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib headers
#include "Vertex.h"


BSPLIB_NAMESPACE_BEGIN


// check if a single vertex lies on a line segment ----------------------------
//
int LineSeg2::PointOnLineSeg( const Vertex2& vertex ) const
{
	if ( m_dirvec.IsNullVector() )
		return FALSE;

	Vector2 vertexvec( vertex - m_basevtx );
	Vector2 crossprod( vertexvec, m_dirvec );

	if ( ( crossprod.VecLength() / m_dirvec.VecLength() ) > EPS_POINT_ON_LINE )
		return FALSE;

	double t;
	if ( fabs( m_dirvec.X ) >= EPS_DENOM_ZERO )
		t = vertexvec.X / m_dirvec.X;
	else if ( fabs( m_dirvec.Y ) >= EPS_DENOM_ZERO )
		t = vertexvec.Y / m_dirvec.Y;
	else
		return FALSE;

	return ( ( t >= EPS_POINT_ON_LINESEG ) && ( t <= 1.0 - EPS_POINT_ON_LINESEG ) );
}


// check if a single vertex lies on a line segment ----------------------------
//
int LineSeg3::PointOnLineSeg( const Vertex3& vertex ) const
{
	if ( m_dirvec.IsNullVector() )
		return FALSE;

	Vector3 vertexvec( vertex - m_basevtx );
	Vector3 crossprod( vertexvec, m_dirvec );

	// check if point lies on line
	if ( ( crossprod.VecLength() / m_dirvec.VecLength() ) > EPS_POINT_ON_LINE )
		return FALSE;

	// check if it lies between start- and endvertex of lineseg
	double t;
	if ( fabs( m_dirvec.X ) >= EPS_DENOM_ZERO )
		t = vertexvec.X / m_dirvec.X;
	else if ( fabs( m_dirvec.Y ) >= EPS_DENOM_ZERO )
		t = vertexvec.Y / m_dirvec.Y;
	else if ( fabs( m_dirvec.Z ) >= EPS_DENOM_ZERO )
		t = vertexvec.Z / m_dirvec.Z;
	else
		return FALSE;

	return ( ( t >= EPS_POINT_ON_LINESEG ) && ( t <= 1.0 - EPS_POINT_ON_LINESEG ) );
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
