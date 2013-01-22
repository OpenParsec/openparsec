//-----------------------------------------------------------------------------
//	BSPLIB MODULE: Transform3.cpp
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib headers
#include "Transform3.h"


BSPLIB_NAMESPACE_BEGIN


// multiply two 4x4 matrices --------------------------------------------------
//
PRIVATE
void mat4x4_mul( double dest_matrix[][4], const double matrix1[][4], const double matrix2[][4] )
{
	//NOTE:
	// full homogeneous multiplication is performed. (that is, all 16
	// elements are calculated.) normally this isn't necessary as
	// most matrices are affine. but since this is no real-time
	// application more flexibility seems better.
/*
	for ( int i = 0; i < 4; i++ ) {
		for ( int j = 0; j < 4; j++ ) {
			dest_matrix[ i ][ j ] = 0.0;
			for ( int k = 0; k < 4; k++ )
				dest_matrix[ i ][ j ] += matrix1[ i ][ k ] * matrix2[ k ][ j ];
		}
	}
*/
	dest_matrix[ 0 ][ 0 ] = matrix1[ 0 ][ 0 ] * matrix2[ 0 ][ 0 ] +
							matrix1[ 0 ][ 1 ] * matrix2[ 1 ][ 0 ] +
							matrix1[ 0 ][ 2 ] * matrix2[ 2 ][ 0 ] +
							matrix1[ 0 ][ 3 ] * matrix2[ 3 ][ 0 ];
	dest_matrix[ 0 ][ 1 ] = matrix1[ 0 ][ 0 ] * matrix2[ 0 ][ 1 ] +
							matrix1[ 0 ][ 1 ] * matrix2[ 1 ][ 1 ] +
							matrix1[ 0 ][ 2 ] * matrix2[ 2 ][ 1 ] +
							matrix1[ 0 ][ 3 ] * matrix2[ 3 ][ 1 ];
	dest_matrix[ 0 ][ 2 ] = matrix1[ 0 ][ 0 ] * matrix2[ 0 ][ 2 ] +
							matrix1[ 0 ][ 1 ] * matrix2[ 1 ][ 2 ] +
							matrix1[ 0 ][ 2 ] * matrix2[ 2 ][ 2 ] +
							matrix1[ 0 ][ 3 ] * matrix2[ 3 ][ 2 ];
	dest_matrix[ 0 ][ 3 ] = matrix1[ 0 ][ 0 ] * matrix2[ 0 ][ 3 ] +
							matrix1[ 0 ][ 1 ] * matrix2[ 1 ][ 3 ] +
							matrix1[ 0 ][ 2 ] * matrix2[ 2 ][ 3 ] +
							matrix1[ 0 ][ 3 ] * matrix2[ 3 ][ 3 ];

	dest_matrix[ 1 ][ 0 ] = matrix1[ 1 ][ 0 ] * matrix2[ 0 ][ 0 ] +
							matrix1[ 1 ][ 1 ] * matrix2[ 1 ][ 0 ] +
							matrix1[ 1 ][ 2 ] * matrix2[ 2 ][ 0 ] +
							matrix1[ 1 ][ 3 ] * matrix2[ 3 ][ 0 ];
	dest_matrix[ 1 ][ 1 ] = matrix1[ 1 ][ 0 ] * matrix2[ 0 ][ 1 ] +
							matrix1[ 1 ][ 1 ] * matrix2[ 1 ][ 1 ] +
							matrix1[ 1 ][ 2 ] * matrix2[ 2 ][ 1 ] +
							matrix1[ 1 ][ 3 ] * matrix2[ 3 ][ 1 ];
	dest_matrix[ 1 ][ 2 ] = matrix1[ 1 ][ 0 ] * matrix2[ 0 ][ 2 ] +
							matrix1[ 1 ][ 1 ] * matrix2[ 1 ][ 2 ] +
							matrix1[ 1 ][ 2 ] * matrix2[ 2 ][ 2 ] +
							matrix1[ 1 ][ 3 ] * matrix2[ 3 ][ 2 ];
	dest_matrix[ 1 ][ 3 ] = matrix1[ 1 ][ 0 ] * matrix2[ 0 ][ 3 ] +
							matrix1[ 1 ][ 1 ] * matrix2[ 1 ][ 3 ] +
							matrix1[ 1 ][ 2 ] * matrix2[ 2 ][ 3 ] +
							matrix1[ 1 ][ 3 ] * matrix2[ 3 ][ 3 ];

	dest_matrix[ 2 ][ 0 ] = matrix1[ 2 ][ 0 ] * matrix2[ 0 ][ 0 ] +
							matrix1[ 2 ][ 1 ] * matrix2[ 1 ][ 0 ] +
							matrix1[ 2 ][ 2 ] * matrix2[ 2 ][ 0 ] +
							matrix1[ 2 ][ 3 ] * matrix2[ 3 ][ 0 ];
	dest_matrix[ 2 ][ 1 ] = matrix1[ 2 ][ 0 ] * matrix2[ 0 ][ 1 ] +
							matrix1[ 2 ][ 1 ] * matrix2[ 1 ][ 1 ] +
							matrix1[ 2 ][ 2 ] * matrix2[ 2 ][ 1 ] +
							matrix1[ 2 ][ 3 ] * matrix2[ 3 ][ 1 ];
	dest_matrix[ 2 ][ 2 ] = matrix1[ 2 ][ 0 ] * matrix2[ 0 ][ 2 ] +
							matrix1[ 2 ][ 1 ] * matrix2[ 1 ][ 2 ] +
							matrix1[ 2 ][ 2 ] * matrix2[ 2 ][ 2 ] +
							matrix1[ 2 ][ 3 ] * matrix2[ 3 ][ 2 ];
	dest_matrix[ 2 ][ 3 ] = matrix1[ 2 ][ 0 ] * matrix2[ 0 ][ 3 ] +
							matrix1[ 2 ][ 1 ] * matrix2[ 1 ][ 3 ] +
							matrix1[ 2 ][ 2 ] * matrix2[ 2 ][ 3 ] +
							matrix1[ 2 ][ 3 ] * matrix2[ 3 ][ 3 ];

	dest_matrix[ 3 ][ 0 ] = matrix1[ 3 ][ 0 ] * matrix2[ 0 ][ 0 ] +
							matrix1[ 3 ][ 1 ] * matrix2[ 1 ][ 0 ] +
							matrix1[ 3 ][ 2 ] * matrix2[ 2 ][ 0 ] +
							matrix1[ 3 ][ 3 ] * matrix2[ 3 ][ 0 ];
	dest_matrix[ 3 ][ 1 ] = matrix1[ 3 ][ 0 ] * matrix2[ 0 ][ 1 ] +
							matrix1[ 3 ][ 1 ] * matrix2[ 1 ][ 1 ] +
							matrix1[ 3 ][ 2 ] * matrix2[ 2 ][ 1 ] +
							matrix1[ 3 ][ 3 ] * matrix2[ 3 ][ 1 ];
	dest_matrix[ 3 ][ 2 ] = matrix1[ 3 ][ 0 ] * matrix2[ 0 ][ 2 ] +
							matrix1[ 3 ][ 1 ] * matrix2[ 1 ][ 2 ] +
							matrix1[ 3 ][ 2 ] * matrix2[ 2 ][ 2 ] +
							matrix1[ 3 ][ 3 ] * matrix2[ 3 ][ 2 ];
	dest_matrix[ 3 ][ 3 ] = matrix1[ 3 ][ 0 ] * matrix2[ 0 ][ 3 ] +
							matrix1[ 3 ][ 1 ] * matrix2[ 1 ][ 3 ] +
							matrix1[ 3 ][ 2 ] * matrix2[ 2 ][ 3 ] +
							matrix1[ 3 ][ 3 ] * matrix2[ 3 ][ 3 ];
}


// calculate concatenation; don't overwrite object matrix ---------------------
//
Transform3 operator *( const Transform3& trafo1, const Transform3& trafo2 )
{
	Transform3 temp;
	mat4x4_mul( temp.m_matrix, trafo1.m_matrix, trafo2.m_matrix );
	return temp;
}


// calculate concatenation as new object matrix -------------------------------
//
Transform3& Transform3::Concat( const Transform3& cattrafo )
{
	Transform3 temp;
	mat4x4_mul( temp.m_matrix, (const double(*)[4]) m_matrix, cattrafo.m_matrix );
	memcpy( m_matrix, temp.m_matrix, sizeof( m_matrix ) );
	return *this;
}


// reversed concatenation -----------------------------------------------------
//
Transform3& Transform3::ConcatR( const Transform3& cattrafo )
{
	Transform3 temp;
	mat4x4_mul( temp.m_matrix, cattrafo.m_matrix, (const double(*)[4]) m_matrix );
	memcpy( m_matrix, temp.m_matrix, sizeof( m_matrix ) );
	return *this;
}


// transform Vector3 by matrix (homogeneous component is also evaluated!) -----
//
Vector3 Transform3::TransformVector3( const Vector3& vec ) const
{
	Vector3 temp;
	temp.setX( m_matrix[ 0 ][ 0 ] * vec.getX() + m_matrix[ 1 ][ 0 ] * vec.getY() + m_matrix[ 2 ][ 0 ] * vec.getZ() + m_matrix[ 3 ][ 0 ] * vec.getW() );
	temp.setY( m_matrix[ 0 ][ 1 ] * vec.getX() + m_matrix[ 1 ][ 1 ] * vec.getY() + m_matrix[ 2 ][ 1 ] * vec.getZ() + m_matrix[ 3 ][ 1 ] * vec.getW() );
	temp.setZ( m_matrix[ 0 ][ 2 ] * vec.getX() + m_matrix[ 1 ][ 2 ] * vec.getY() + m_matrix[ 2 ][ 2 ] * vec.getZ() + m_matrix[ 3 ][ 2 ] * vec.getW() );
	temp.setW( m_matrix[ 0 ][ 3 ] * vec.getX() + m_matrix[ 1 ][ 3 ] * vec.getY() + m_matrix[ 2 ][ 3 ] * vec.getZ() + m_matrix[ 3 ][ 3 ] * vec.getW() );
	return temp;
}


// fetch translation part of matrix -------------------------------------------
//
Vector3 Transform3::FetchTranslation() const
{
	// return translation vector
	return Vector3( m_matrix[ 3 ][ 0 ], m_matrix[ 3 ][ 1 ], m_matrix[ 3 ][ 2 ] );
}


// extract translation part of matrix; set to NULL translation afterwards -----
//
Vector3 Transform3::ExtractTranslation()
{
	// create translation vector
	Vector3 temp( m_matrix[ 3 ][ 0 ], m_matrix[ 3 ][ 1 ], m_matrix[ 3 ][ 2 ] );
	// zero translation part of matrix
	m_matrix[ 3 ][ 0 ] = 0.0;
	m_matrix[ 3 ][ 1 ] = 0.0;
	m_matrix[ 3 ][ 2 ] = 0.0;
	return temp;
}


// create rotation matrix -----------------------------------------------------
//
Transform3&	Transform3::LoadRotation( double angle, double x, double y, double z )
{
	// init matrix
	LoadIdentity();

	// normalize axis of rotation
	Vector3 axis( x, y, z );
	if ( !axis.Normalize() ) {
		// NULL vector: return identity matrix
		return *this;
	}

	// calculate unit quaternion corresponding to rotation
	double phi2		= -angle / 2;
	double sinphi2	= sin( phi2 );
	double W		= cos( phi2 );
	double X		= sinphi2 * axis.getX();
	double Y		= sinphi2 * axis.getY();
	double Z		= sinphi2 * axis.getZ();

	// calculate intermediate terms
	double X2 = X * X;
	double Y2 = Y * Y;
	double Z2 = Z * Z;
	double XY = X * Y;
	double XZ = X * Z;
	double YZ = Y * Z;
	double WX = W * X;
	double WY = W * Y;
	double WZ = W * Z;

	// convert quaternion into rotation matrix
	m_matrix[ 0 ][ 0 ] = 1 - ( Y2 + Z2 ) * 2;
	m_matrix[ 0 ][ 1 ] =     ( XY - WZ ) * 2;
	m_matrix[ 0 ][ 2 ] =     ( XZ + WY ) * 2;
	m_matrix[ 1 ][ 0 ] =     ( XY + WZ ) * 2;
	m_matrix[ 1 ][ 1 ] = 1 - ( X2 + Z2 ) * 2;
	m_matrix[ 1 ][ 2 ] =     ( YZ - WX ) * 2;
	m_matrix[ 2 ][ 0 ] =     ( XZ - WY ) * 2;
	m_matrix[ 2 ][ 1 ] =     ( YZ + WX ) * 2;
	m_matrix[ 2 ][ 2 ] = 1 - ( X2 + Y2 ) * 2;

	return *this;
}


// create scale matrix --------------------------------------------------------
//
Transform3& Transform3::LoadScale( double x, double y, double z )
{
	LoadIdentity();
	m_matrix[ 0 ][ 0 ] = x;
	m_matrix[ 1 ][ 1 ] = y;
	m_matrix[ 2 ][ 2 ] = z;
	return *this;
}


// create translation matrix --------------------------------------------------
//
Transform3& Transform3::LoadTranslation( double x, double y, double z )
{
	LoadIdentity();
	m_matrix[ 3 ][ 0 ] = x;
	m_matrix[ 3 ][ 1 ] = y;
	m_matrix[ 3 ][ 2 ] = z;
	return *this;
}


// rotate around arbitrary axis -----------------------------------------------
//
Transform3& Transform3::Rotate( double angle, double x, double y, double z )
{
	Transform3 temp;
	temp.LoadRotation( angle, x, y, z );
	Concat( temp );
	return *this;
}


// rotate around arbitrary axis (reversed) ------------------------------------
//
Transform3& Transform3::RotateR( double angle, double x, double y, double z )
{
	Transform3 temp;
	temp.LoadRotation( angle, x, y, z );
	ConcatR( temp );
	return *this;
}


// apply scale factors --------------------------------------------------------
//
Transform3& Transform3::Scale( double x, double y, double z )
{
	Transform3 temp;
	temp.LoadScale( x, y, z );
	Concat( temp );
	return *this;
}


// apply scale factors (reversed) ---------------------------------------------
//
Transform3& Transform3::ScaleR( double x, double y, double z )
{
	Transform3 temp;
	temp.LoadScale( x, y, z );
	ConcatR( temp );
	return *this;
}


// apply translation ----------------------------------------------------------
//
Transform3& Transform3::Translate( double x, double y, double z )
{
	m_matrix[ 3 ][ 0 ] += x;
	m_matrix[ 3 ][ 1 ] += y;
	m_matrix[ 3 ][ 2 ] += z;
	return *this;
}


// apply translation (reversed) -----------------------------------------------
//
Transform3& Transform3::TranslateR( double x, double y, double z )
{
	Transform3 temp;
	temp.LoadTranslation( x, y, z );
	ConcatR( temp );
	return *this;
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
