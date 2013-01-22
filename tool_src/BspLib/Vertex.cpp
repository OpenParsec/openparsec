//-----------------------------------------------------------------------------
//	BSPLIB MODULE: Vertex.cpp
//
//  Copyright (c) 1996-1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib headers
#include "Vertex.h"


BSPLIB_NAMESPACE_BEGIN


// set specified coordinate to specified value --------------------------------
void Vertex3::ChangeAxis( const char xchar, const hprec_t src )
{
	switch ( xchar ) {
		case 'x':
			X = src;
			break;
		case 'y':
			Y = src;
			break;
		case 'z':
			Z = src;
			break;
		default:
			fflush( stdout );
			fprintf( stderr, "\n\n**ERROR** [illegal xchange parameters]\n" );
			exit( EXIT_FAILURE );
	}
}

// change all axes according to xchange-command -------------------------------
void Vertex3::ChangeAxes( const char *xchangecmd, const Vertex3& ads )
{
	Vertex3 temp = *this;
	ChangeAxis( xchangecmd[ 0 ], temp.X * ads.X );
	ChangeAxis( xchangecmd[ 1 ], temp.Y * ads.Y );
	ChangeAxis( xchangecmd[ 2 ], temp.Z * ads.Z );
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
