/*
 * PARSEC - Culling Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:43 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */ 

// C library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "utl_cull.h"



// determine indexes of reject and accept points with respect to a plane ------
//
void CULL_ReAcIndexBox3( ReAcIndexBox3 *reac, Plane3 *plane )
{
	ASSERT( reac != NULL );
	ASSERT( plane != NULL );

	//NOTE:
	// reac->reject will be the point with the
	// greatest signed distance to the plane.
	// reac->accept will be the point with the
	// least signed distance to the plane.

	if ( GEOMV_NEGATIVE( plane->X ) ) {
		reac->reject[ 0 ] = 0;
		reac->accept[ 0 ] = 3;
	} else {
		reac->reject[ 0 ] = 3;
		reac->accept[ 0 ] = 0;
	}

	if ( GEOMV_NEGATIVE( plane->Y ) ) {
		reac->reject[ 1 ] = 1;
		reac->accept[ 1 ] = 4;
	} else {
		reac->reject[ 1 ] = 4;
		reac->accept[ 1 ] = 1;
	}

	if ( GEOMV_NEGATIVE( plane->Z ) ) {
		reac->reject[ 2 ] = 2;
		reac->accept[ 2 ] = 5;
	} else {
		reac->reject[ 2 ] = 5;
		reac->accept[ 2 ] = 2;
	}
}


// determine coordinates of reject and accept points with respect to a plane --
//
void CULL_ReAcPointBox3( ReAcPointBox3 *reac, CullBox3 *cullbox, Plane3 *plane )
{
	ASSERT( reac != NULL );
	ASSERT( cullbox != NULL );
	ASSERT( plane != NULL );

	//NOTE:
	// reac->reject will be the point with the
	// greatest signed distance to the plane.
	// reac->accept will be the point with the
	// least signed distance to the plane.

	if ( GEOMV_NEGATIVE( plane->X ) ) {
		reac->reject.X = cullbox->minmax[ 0 ];
		reac->accept.X = cullbox->minmax[ 3 ];
	} else {
		reac->reject.X = cullbox->minmax[ 3 ];
		reac->accept.X = cullbox->minmax[ 0 ];
	}

	if ( GEOMV_NEGATIVE( plane->Y ) ) {
		reac->reject.Y = cullbox->minmax[ 1 ];
		reac->accept.Y = cullbox->minmax[ 4 ];
	} else {
		reac->reject.Y = cullbox->minmax[ 4 ];
		reac->accept.Y = cullbox->minmax[ 1 ];
	}

	if ( GEOMV_NEGATIVE( plane->Z ) ) {
		reac->reject.Z = cullbox->minmax[ 2 ];
		reac->accept.Z = cullbox->minmax[ 5 ];
	} else {
		reac->reject.Z = cullbox->minmax[ 5 ];
		reac->accept.Z = cullbox->minmax[ 2 ];
	}
}


// augment set of planes by reject/accept points for culling ------------------
//
void CULL_MakeVolumeCullVolume( Plane3 *volume, CullPlane3 *cullvolume, dword cullmask )
{
	ASSERT( volume != NULL );
	ASSERT( cullvolume != NULL );

	for ( ; cullmask != 0x00; cullmask >>= 1, volume++, cullvolume++ ) {

		if ( cullmask & 0x01 ) {

			// copy plane
			cullvolume->plane = *volume;

			// calc reac indexes
			CULL_ReAcIndexBox3( &cullvolume->reacx, &cullvolume->plane );
		}
	}
}


// cull an axial bounding box against a cull volume (set of cull planes) ------
//
int CULL_BoxAgainstCullVolume( CullBox3 *cullbox, CullPlane3 *volume, dword *cullmask )
{
	//NOTE:
	// this function returns:
	// - TRUE if the box is trivial reject
	// - FALSE if the box is not trivial reject
	// - if ( *cullmask == 0x00 ) afterwards the box is trivial accept
	// - if ( *cullmask != 0x00 ) afterwards the box needs to be clipped

	ASSERT( cullbox != NULL );
	ASSERT( volume != NULL );
	ASSERT( cullmask != NULL );

	dword		curmask   = *cullmask;
	CullPlane3*	cullplane = volume;

	for ( int curplane = 0; curmask != 0x00; curmask >>= 1, curplane++ ) {

		if ( curmask & 0x01 ) {

			// one plane rejects: whole status is trivial reject
			geomv_t dist = PLANEDIST_REJECTPOINT( cullplane, cullbox );
			if ( GEOMV_NEGATIVE( dist ) ) {
				return TRUE;
			}

			// mask plane if trivial accept
			dist = PLANEDIST_ACCEPTPOINT( cullplane, cullbox );
			if ( GEOMV_POSITIVE( dist ) ) {
				*cullmask &= ~( 1 << curplane );
			}
		}

		// sizeof( CullPlane3 ) is no power of two
		cullplane = (CullPlane3 *)( (char *)cullplane + sizeof( cullplane[ 0 ] ) );
	}

	return FALSE;
}


// cull an axial bounding box against a volume (set of planes) ----------------
//
int CULL_BoxAgainstVolume( CullBox3 *cullbox, Plane3 *volume, dword *cullmask )
{
	//NOTE:
	// this function returns:
	// - TRUE if the box is trivial reject
	// - FALSE if the box is not trivial reject
	// - if ( *cullmask == 0x00 ) afterwards the box is trivial accept
	// - if ( *cullmask != 0x00 ) afterwards the box needs to be clipped

	ASSERT( cullbox != NULL );
	ASSERT( volume != NULL );
	ASSERT( cullmask != NULL );

	dword curmask = *cullmask;

	for ( int curplane = 0; curmask != 0x00; curmask >>= 1, curplane++ ) {

		if ( curmask & 0x01 ) {

			Plane3 *plane = &volume[ curplane ];

			// determine reject/accept points
			ReAcPointBox3 reac;
			CULL_ReAcPointBox3( &reac, cullbox, plane );

			// one plane rejects: whole status is trivial reject
			if ( PLANE_DOT( plane, &reac.reject ) <= PLANE_OFFSET( plane ) ) {
				return TRUE;
			}

			// mask plane if trivial accept
			if ( PLANE_DOT( plane, &reac.accept ) >= PLANE_OFFSET( plane ) ) {
				*cullmask &= ~( 1 << curplane );
			}
		}
	}

	return FALSE;
}


// cull a bounding sphere against a volume (set of planes) --------------------
//
int CULL_SphereAgainstVolume( Sphere3 *sphere, Plane3 *volume, dword *cullmask )
{
	//NOTE:
	// this function returns:
	// - TRUE if the sphere is trivial reject
	// - FALSE if the sphere is not trivial reject
	// - if ( *cullmask == 0x00 ) afterwards the sphere is trivial accept
	// - if ( *cullmask != 0x00 ) afterwards the sphere needs to be clipped

	ASSERT( sphere != NULL );
	ASSERT( volume != NULL );
	ASSERT( cullmask != NULL );

	dword curmask = *cullmask;

	for ( int curplane = 0; curmask != 0x00; curmask >>= 1, curplane++ ) {

		if ( curmask & 0x01 ) {

			Plane3 *plane = &volume[ curplane ];
			geomv_t dist  = PLANE_DOT( plane, sphere ) - PLANE_OFFSET( plane );

			// one plane rejects: whole status is trivial reject
			if ( dist < -sphere->R ) {
				return TRUE;
			}

			// mask plane if trivial accept
			if ( dist > sphere->R ) {
				*cullmask &= ~( 1 << curplane );
			}
		}
	}

	return FALSE;
}



