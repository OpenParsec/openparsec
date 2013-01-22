//-----------------------------------------------------------------------------
//	BSPLIB HEADER: Transform3.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _TRANSFORM3_H_
#define _TRANSFORM3_H_

// bsplib header files
#include "BspLibDefs.h"
#include "Vertex.h"


BSPLIB_NAMESPACE_BEGIN


// general 3-D homogeneous transformation -------------------------------------
//
class Transform3 {

	friend Transform3 operator *( const Transform3& trafo1, const Transform3& trafo2 );

public:
	Transform3();
	Transform3( const float trafo[4][4] );
	Transform3( const double trafo[4][4] );
	~Transform3() { }

	// transform single vector by current transformation
	Vector3		TransformVector3( const Vector3& vec ) const;

	// concatenate two transformations (natural and reversed order)
	Transform3&	Concat( const Transform3& cattrafo );
	Transform3&	ConcatR( const Transform3& cattrafo );

	// create elementary matrices
	Transform3&	LoadIdentity();
	Transform3&	LoadRotation( double angle, double x, double y, double z );
	Transform3&	LoadScale( double x, double y, double z );
	Transform3&	LoadTranslation( double x, double y, double z );

	// elementary transformations in natural order
	Transform3&	Rotate( double angle, double x, double y, double z );
	Transform3&	Scale( double x, double y, double z );
	Transform3&	Translate( double x, double y, double z );

	// elementary transformations in reversed order
	Transform3&	RotateR( double angle, double x, double y, double z );
	Transform3&	ScaleR( double x, double y, double z );
	Transform3&	TranslateR( double x, double y, double z );

	// fetch translation part (optionally set it null afterwards)
	Vector3		FetchTranslation() const;
	Vector3		ExtractTranslation();

private:
	// 4x4 matrix for 3-D homogeneous transformation (row vectors!)
	double	m_matrix[4][4];
};

// load identity matrix -------------------------------------------------------
inline Transform3& Transform3::LoadIdentity()
{
	memset( m_matrix, 0, sizeof( m_matrix ) );

	m_matrix[ 0 ][ 0 ] = 1.0;
	m_matrix[ 1 ][ 1 ] = 1.0;
	m_matrix[ 2 ][ 2 ] = 1.0;
	m_matrix[ 3 ][ 3 ] = 1.0;

	return *this;
}

// construct a transform3 -----------------------------------------------------
inline Transform3::Transform3()
{
//	LoadIdentity();
}

// construct a transform3 directly from 4x4 matrix ----------------------------
inline Transform3::Transform3( const double trafo[4][4] )
{
	//CAVEAT:
	// the outer dimension of the matrix is not enforced!!
	// trafo is actually of type double(*)[4]
	memcpy( m_matrix, trafo, sizeof( m_matrix ) );
}

// construct a transform3 directly from 4x4 matrix ----------------------------
inline Transform3::Transform3( const float trafo[4][4] )
{
	//CAVEAT:
	// the outer dimension of the matrix is not enforced!!
	// trafo is actually of type float(*)[4]
	for ( int i = 0; i < 16; i++ )
		((double *)m_matrix)[ i ] = (double) ((float *)trafo)[ i ];
}


BSPLIB_NAMESPACE_END


#endif // _TRANSFORM3_H_

