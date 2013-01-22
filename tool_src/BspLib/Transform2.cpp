//-----------------------------------------------------------------------------
//	BSPLIB MODULE: Transform2.cpp
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib headers
#include "Transform2.h"


BSPLIB_NAMESPACE_BEGIN


// multiply two 3x3 matrices --------------------------------------------------
//
PRIVATE
void mat3x3_mul( double dest_matrix[][3], const double matrix1[][3], const double matrix2[][3] )
{
	//NOTE:
	// full homogeneous multiplication is performed. (that is, all 9
	// elements are calculated.) normally this isn't necessary as
	// most matrices are affine. but since this is no real-time
	// application more flexibility seems better.
/*
	for ( int i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 3; j++ ) {
			dest_matrix[ i ][ j ] = 0.0;
			for ( int k = 0; k < 3; k++ )
				dest_matrix[ i ][ j ] += matrix1[ i ][ k ] * matrix2[ k ][ j ];
		}
	}
*/
	dest_matrix[ 0 ][ 0 ] = matrix1[ 0 ][ 0 ] * matrix2[ 0 ][ 0 ] +
							matrix1[ 0 ][ 1 ] * matrix2[ 1 ][ 0 ] +
							matrix1[ 0 ][ 2 ] * matrix2[ 2 ][ 0 ];
	dest_matrix[ 0 ][ 1 ] = matrix1[ 0 ][ 0 ] * matrix2[ 0 ][ 1 ] +
							matrix1[ 0 ][ 1 ] * matrix2[ 1 ][ 1 ] +
							matrix1[ 0 ][ 2 ] * matrix2[ 2 ][ 1 ];
	dest_matrix[ 0 ][ 2 ] = matrix1[ 0 ][ 0 ] * matrix2[ 0 ][ 2 ] +
							matrix1[ 0 ][ 1 ] * matrix2[ 1 ][ 2 ] +
							matrix1[ 0 ][ 2 ] * matrix2[ 2 ][ 2 ];

	dest_matrix[ 1 ][ 0 ] = matrix1[ 1 ][ 0 ] * matrix2[ 0 ][ 0 ] +
							matrix1[ 1 ][ 1 ] * matrix2[ 1 ][ 0 ] +
							matrix1[ 1 ][ 2 ] * matrix2[ 2 ][ 0 ];
	dest_matrix[ 1 ][ 1 ] = matrix1[ 1 ][ 0 ] * matrix2[ 0 ][ 1 ] +
							matrix1[ 1 ][ 1 ] * matrix2[ 1 ][ 1 ] +
							matrix1[ 1 ][ 2 ] * matrix2[ 2 ][ 1 ];
	dest_matrix[ 1 ][ 2 ] = matrix1[ 1 ][ 0 ] * matrix2[ 0 ][ 2 ] +
							matrix1[ 1 ][ 1 ] * matrix2[ 1 ][ 2 ] +
							matrix1[ 1 ][ 2 ] * matrix2[ 2 ][ 2 ];

	dest_matrix[ 2 ][ 0 ] = matrix1[ 2 ][ 0 ] * matrix2[ 0 ][ 0 ] +
							matrix1[ 2 ][ 1 ] * matrix2[ 1 ][ 0 ] +
							matrix1[ 2 ][ 2 ] * matrix2[ 2 ][ 0 ];
	dest_matrix[ 2 ][ 1 ] = matrix1[ 2 ][ 0 ] * matrix2[ 0 ][ 1 ] +
							matrix1[ 2 ][ 1 ] * matrix2[ 1 ][ 1 ] +
							matrix1[ 2 ][ 2 ] * matrix2[ 2 ][ 1 ];
	dest_matrix[ 2 ][ 2 ] = matrix1[ 2 ][ 0 ] * matrix2[ 0 ][ 2 ] +
							matrix1[ 2 ][ 1 ] * matrix2[ 1 ][ 2 ] +
							matrix1[ 2 ][ 2 ] * matrix2[ 2 ][ 2 ];
}


// calculate concatenation; don't overwrite object matrix ---------------------
//
Transform2 operator *( const Transform2& trafo1, const Transform2& trafo2 )
{
	Transform2 temp;
	mat3x3_mul( temp.m_matrix, trafo1.m_matrix, trafo2.m_matrix );
	return temp;
}


// calculate concatenation as new object matrix -------------------------------
//
Transform2& Transform2::Concat( const Transform2& cattrafo )
{
	Transform2 temp;
	mat3x3_mul( temp.m_matrix, (const double(*)[3]) m_matrix, cattrafo.m_matrix );
	memcpy( m_matrix, temp.m_matrix, sizeof( m_matrix ) );
	return *this;
}


// reversed concatenation -----------------------------------------------------
//
Transform2& Transform2::ConcatR( const Transform2& cattrafo )
{
	Transform2 temp;
	mat3x3_mul( temp.m_matrix, cattrafo.m_matrix, (const double(*)[3]) m_matrix );
	memcpy( m_matrix, temp.m_matrix, sizeof( m_matrix ) );
	return *this;
}


// transform Vector2 by matrix (homogeneous component is also evaluated!) -----
//
Vector2 Transform2::TransformVector2( const Vector2& vec ) const
{
	Vector2 temp;
	temp.setX( m_matrix[ 0 ][ 0 ] * vec.getX() + m_matrix[ 1 ][ 0 ] * vec.getY() + m_matrix[ 2 ][ 0 ] * vec.getW() );
	temp.setY( m_matrix[ 0 ][ 1 ] * vec.getX() + m_matrix[ 1 ][ 1 ] * vec.getY() + m_matrix[ 2 ][ 1 ] * vec.getW() );
	temp.setW( m_matrix[ 0 ][ 2 ] * vec.getX() + m_matrix[ 1 ][ 2 ] * vec.getY() + m_matrix[ 2 ][ 2 ] * vec.getW() );
	return temp;
}


// calc determinant of matrix -------------------------------------------------
//
double Transform2::Determinant()
{
	// calc determinant using rule of sarrus
	double pos;
	pos  = m_matrix[ 0 ][ 0 ] * m_matrix[ 1 ][ 1 ] * m_matrix[ 2 ][ 2 ];
	pos += m_matrix[ 0 ][ 1 ] * m_matrix[ 1 ][ 2 ] * m_matrix[ 2 ][ 0 ];
	pos += m_matrix[ 0 ][ 2 ] * m_matrix[ 1 ][ 0 ] * m_matrix[ 2 ][ 1 ];
	double neg;
	neg  = m_matrix[ 0 ][ 0 ] * m_matrix[ 1 ][ 2 ] * m_matrix[ 2 ][ 1 ];
	neg += m_matrix[ 0 ][ 1 ] * m_matrix[ 1 ][ 0 ] * m_matrix[ 2 ][ 2 ];
	neg += m_matrix[ 0 ][ 2 ] * m_matrix[ 1 ][ 1 ] * m_matrix[ 2 ][ 0 ];

	return ( pos - neg );
}


// calc matrix inverse --------------------------------------------------------
//
int Transform2::Inverse( Transform2& inverse )
{
	// calculate determinant
	double det = Determinant();
	// check for singularity
	if ( fabs( det ) < EPS_DENOM_ZERO )
		return 0;

	// calculate inverse
	double detinv = 1 / det;
	inverse.m_matrix[ 0 ][ 0 ] = ( m_matrix[ 1 ][ 1 ] * m_matrix[ 2 ][ 2 ] - m_matrix[ 1 ][ 2 ] * m_matrix[ 2 ][ 1 ] ) * detinv;
	inverse.m_matrix[ 0 ][ 1 ] = ( m_matrix[ 0 ][ 2 ] * m_matrix[ 2 ][ 1 ] - m_matrix[ 0 ][ 1 ] * m_matrix[ 2 ][ 2 ] ) * detinv;
	inverse.m_matrix[ 0 ][ 2 ] = ( m_matrix[ 0 ][ 1 ] * m_matrix[ 1 ][ 2 ] - m_matrix[ 0 ][ 2 ] * m_matrix[ 1 ][ 1 ] ) * detinv;
	inverse.m_matrix[ 1 ][ 0 ] = ( m_matrix[ 1 ][ 2 ] * m_matrix[ 2 ][ 0 ] - m_matrix[ 1 ][ 0 ] * m_matrix[ 2 ][ 2 ] ) * detinv;
	inverse.m_matrix[ 1 ][ 1 ] = ( m_matrix[ 0 ][ 0 ] * m_matrix[ 2 ][ 2 ] - m_matrix[ 0 ][ 2 ] * m_matrix[ 2 ][ 0 ] ) * detinv;
	inverse.m_matrix[ 1 ][ 2 ] = ( m_matrix[ 0 ][ 2 ] * m_matrix[ 1 ][ 0 ] - m_matrix[ 0 ][ 0 ] * m_matrix[ 1 ][ 2 ] ) * detinv;
	inverse.m_matrix[ 2 ][ 0 ] = ( m_matrix[ 1 ][ 0 ] * m_matrix[ 2 ][ 1 ] - m_matrix[ 1 ][ 1 ] * m_matrix[ 2 ][ 0 ] ) * detinv;
	inverse.m_matrix[ 2 ][ 1 ] = ( m_matrix[ 0 ][ 1 ] * m_matrix[ 2 ][ 0 ] - m_matrix[ 0 ][ 0 ] * m_matrix[ 2 ][ 1 ] ) * detinv;
	inverse.m_matrix[ 2 ][ 2 ] = ( m_matrix[ 0 ][ 0 ] * m_matrix[ 1 ][ 1 ] - m_matrix[ 0 ][ 1 ] * m_matrix[ 1 ][ 0 ] ) * detinv;

	return 1;
}


// fetch translation part of matrix -------------------------------------------
//
Vector2 Transform2::FetchTranslation() const
{
	// return translation vector
	return Vector2( m_matrix[ 2 ][ 0 ], m_matrix[ 2 ][ 1 ] );
}


// extract translation part of matrix; set to NULL translation afterwards -----
//
Vector2 Transform2::ExtractTranslation()
{
	// create translation vector
	Vector2 temp( m_matrix[ 2 ][ 0 ], m_matrix[ 2 ][ 1 ] );
	// zero translation part of matrix
	m_matrix[ 2 ][ 0 ] = 0.0;
	m_matrix[ 2 ][ 1 ] = 0.0;
	return temp;
}


// create rotation matrix -----------------------------------------------------
//
Transform2&	Transform2::LoadRotation( double angle )
{
	// init matrix
	LoadIdentity();

	// set rotated basis
	m_matrix[ 0 ][ 0 ] = cos( angle );
	m_matrix[ 0 ][ 1 ] = sin( angle );
	m_matrix[ 1 ][ 0 ] = -m_matrix[ 0 ][ 1 ];
	m_matrix[ 1 ][ 1 ] = m_matrix[ 0 ][ 0 ];

	return *this;
}


// create scale matrix --------------------------------------------------------
//
Transform2& Transform2::LoadScale( double x, double y )
{
	LoadIdentity();
	m_matrix[ 0 ][ 0 ] = x;
	m_matrix[ 1 ][ 1 ] = y;
	return *this;
}


// create translation matrix --------------------------------------------------
//
Transform2& Transform2::LoadTranslation( double x, double y )
{
	LoadIdentity();
	m_matrix[ 2 ][ 0 ] = x;
	m_matrix[ 2 ][ 1 ] = y;
	return *this;
}


// rotate around arbitrary axis -----------------------------------------------
//
Transform2& Transform2::Rotate( double angle )
{
	Transform2 temp;
	temp.LoadRotation( angle );
	Concat( temp );
	return *this;
}


// rotate around arbitrary axis (reversed) ------------------------------------
//
Transform2& Transform2::RotateR( double angle )
{
	Transform2 temp;
	temp.LoadRotation( angle );
	ConcatR( temp );
	return *this;
}


// apply scale factors --------------------------------------------------------
//
Transform2& Transform2::Scale( double x, double y )
{
	Transform2 temp;
	temp.LoadScale( x, y );
	Concat( temp );
	return *this;
}


// apply scale factors (reversed) ---------------------------------------------
//
Transform2& Transform2::ScaleR( double x, double y )
{
	Transform2 temp;
	temp.LoadScale( x, y );
	ConcatR( temp );
	return *this;
}


// apply translation ----------------------------------------------------------
//
Transform2& Transform2::Translate( double x, double y )
{
	m_matrix[ 2 ][ 0 ] += x;
	m_matrix[ 2 ][ 1 ] += y;
	return *this;
}


// apply translation (reversed) -----------------------------------------------
//
Transform2& Transform2::TranslateR( double x, double y )
{
	Transform2 temp;
	temp.LoadTranslation( x, y );
	ConcatR( temp );
	return *this;
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
