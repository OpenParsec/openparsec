//-----------------------------------------------------------------------------
//	BSPLIB HEADER: LineSeg3.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _LINESEG3_H_
#define _LINESEG3_H_

// bsplib header files
#include "BspLibDefs.h"
#include "Vector3.h"


BSPLIB_NAMESPACE_BEGIN


// line segment in 3-space ----------------------------------------------------
//
class LineSeg3 {

public:
	LineSeg3( const Vertex3& basevtx, const Vector3& dirvec );
	~LineSeg3() { }

	int PointOnLineSeg( const Vertex3& vertex ) const;

private:
	Vertex3	m_basevtx;
	Vector3 m_dirvec;
};

// construct a LineSeg3 using a basevertex and a direction vector -------------
inline LineSeg3::LineSeg3( const Vertex3& basevtx, const Vector3& dirvec )
{
	m_basevtx = basevtx;
	m_dirvec  = dirvec;
}


BSPLIB_NAMESPACE_END


#endif // _LINESEG3_H_

