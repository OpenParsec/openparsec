//-----------------------------------------------------------------------------
//	BSPLIB HEADER: Transform2.h
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _TRANSFORM2_H_
#define _TRANSFORM2_H_

// bsplib header files
#include "BspLibDefs.h"
#include "Vertex.h"


BSPLIB_NAMESPACE_BEGIN


// general 2-D homogeneous transformation -------------------------------------
//
class Transform2 {

	friend Transform2 operator *( const Transform2& trafo1, const Transform2& trafo2 );

public:
	Transform2();
	Transform2( const float trafo[3][3] );
	Transform2( const double trafo[3][3] );
	~Transform2() { }

	// transform single vector by current transformation
	Vector2		TransformVector2( const Vector2& vec ) const;

	double		Determinant();						// calc matrix's determinant
	int			Inverse( Transform2& inverse );		// calc matrix's inverse

	// concatenate two transformations (natural and reversed order)
	Transform2&	Concat( const Transform2& cattrafo );
	Transform2&	ConcatR( const Transform2& cattrafo );

	// create elementary matrices
	Transform2&	LoadIdentity();
	Transform2&	LoadRotation( double angle );
	Transform2&	LoadScale( double x, double y );
	Transform2&	LoadTranslation( double x, double y );

	// elementary transformations in natural order
	Transform2&	Rotate( double angle );
	Transform2&	Scale( double x, double y );
	Transform2&	Translate( double x, double y );

	// elementary transformations in reversed order
	Transform2&	RotateR( double angle );
	Transform2&	ScaleR( double x, double y );
	Transform2&	TranslateR( double x, double y );

	// fetch translation part (optionally set it null afterwards)
	Vector2		FetchTranslation() const;
	Vector2		ExtractTranslation();

	// fetch pointer to matrix
	double *	LinMatrixAccess() { return (double *) m_matrix; }

private:
	// 3x3 matrix for 2-D homogeneous transformation (row vectors!)
	double	m_matrix[3][3];
};

// load identity matrix -------------------------------------------------------
inline Transform2& Transform2::LoadIdentity()
{
	memset( m_matrix, 0, sizeof( m_matrix ) );

	m_matrix[ 0 ][ 0 ] = 1.0;
	m_matrix[ 1 ][ 1 ] = 1.0;
	m_matrix[ 2 ][ 2 ] = 1.0;

	return *this;
}

// construct a Transform2 -----------------------------------------------------
inline Transform2::Transform2()
{
//	LoadIdentity();
}

// construct a Transform2 directly from 3x3 matrix ----------------------------
inline Transform2::Transform2( const double trafo[3][3] )
{
	//CAVEAT:
	// the outer dimension of the matrix is not enforced!!
	// trafo is actually of type double(*)[3]
	memcpy( m_matrix, trafo, sizeof( m_matrix ) );
}

// construct a Transform2 directly from 3x3 matrix ----------------------------
inline Transform2::Transform2( const float trafo[3][3] )
{
	//CAVEAT:
	// the outer dimension of the matrix is not enforced!!
	// trafo is actually of type float(*)[3]
	for ( int i = 0; i < 9; i++ )
		((double *)m_matrix)[ i ] = (double) ((float *)trafo)[ i ];
}


BSPLIB_NAMESPACE_END


#endif // _TRANSFORM2_H_

