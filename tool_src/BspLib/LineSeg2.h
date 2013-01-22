//-----------------------------------------------------------------------------
//	BSPLIB HEADER: LineSeg2.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _LINESEG2_H_
#define _LINESEG2_H_

// bsplib header files
#include "BspLibDefs.h"
#include "Vector2.h"


BSPLIB_NAMESPACE_BEGIN


// line segment in 2-space ----------------------------------------------------
//
class LineSeg2 {

public:
	LineSeg2( const Vertex2& basevtx, const Vector2& dirvec );
	~LineSeg2() { }

	int PointOnLineSeg( const Vertex2& vertex ) const;

private:
	Vertex2	m_basevtx;
	Vector2 m_dirvec;
};

// construct a LineSeg2 using a basevertex and a direction vector -------------
inline LineSeg2::LineSeg2( const Vertex2& basevtx, const Vector2& dirvec )
{
	m_basevtx = basevtx;
	m_dirvec  = dirvec;
}


BSPLIB_NAMESPACE_END


#endif // _LINESEG2_H_

