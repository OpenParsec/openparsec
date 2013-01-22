/*
 * PARSEC - Color Animations
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:22 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999
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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// local module header
#include "e_colani.h"

// proprietary module headers
#include "con_aux.h"
#include "obj_ctrl.h"



// maximum number of registered colanims --------------------------------------
//
#define MAX_NUM_REGISTERED_COLANIMS		64


// tables for registered colanims ---------------------------------------------
//
PUBLIC int			num_registered_colanims = 0;
PUBLIC colanimreg_s	registered_colanims[ MAX_NUM_REGISTERED_COLANIMS ];


// fetch already registered colanim via name ----------------------------------
//
colanimreg_s *FetchColAnim( char *name, dword *colanimid )
{
	ASSERT( name != NULL );

	for ( int cid = 0; cid < num_registered_colanims; cid++ ) {

		ASSERT( registered_colanims[ cid ].name != NULL );
		if ( stricmp( registered_colanims[ cid ].name, name ) == 0 ) {

			if ( colanimid != NULL )
				*colanimid = cid;
			return &registered_colanims[ cid ];
		}
	}

	return NULL;
}


// init colanim in specified face anim state ----------------------------------
//
void InitAnimStateColAnim( FaceAnimState *animstate, colanim_s *colanim, dword colflags )
{
	ASSERT( animstate != NULL );
	ASSERT( colanim != NULL );

	animstate->ColAnim	= colanim;
	animstate->ColFlags	= colflags;
	animstate->col_pos0	= 0;
	animstate->col_pos1	= 0;

	ASSERT( colanim->col_table0 != NULL );
	colfrm_s *col0 = &colanim->col_table0[ animstate->col_pos0 ];
	animstate->col_time0 = col0->deltatime;

	if ( colanim->col_table1 != NULL ) {
		colfrm_s *col1 = &colanim->col_table1[ animstate->col_pos1 ];
		animstate->col_time1 = col1->deltatime;
	}
}


// check object for use of specified colanim, update if used ------------------
//
PRIVATE
void UpdateObjectColAnims( GenObject *obj, colanim_s *colanim )
{
	ASSERT( obj != NULL );
	ASSERT( colanim != NULL );

	if ( obj->FaceAnimStates == NULL )
		return;

	// scan all active anim states
	for ( int stindx = 0; stindx < obj->ActiveFaceAnims; stindx++ ) {

		FaceAnimState *animstate = &obj->FaceAnimStates[ stindx ];

		// update animstate with respect to changed colanim
		if ( animstate->ColAnim == colanim ) {
			InitAnimStateColAnim( animstate, colanim, animstate->ColFlags );
		}
	}
}


// update specified colanim in all objects that use it ------------------------
//
PRIVATE
void UpdateWorldColAnims( colanim_s *colanim )
{
	ASSERT( colanim != NULL );

	// scan ships
	GenObject *objlist = FetchFirstShip();
	for ( ; objlist != NULL; objlist = objlist->NextObj ) {
		UpdateObjectColAnims( objlist, colanim );
	}
	UpdateObjectColAnims( MyShip, colanim );

	// scan lasers
	objlist = FetchFirstLaser();
	for ( ; objlist != NULL; objlist = objlist->NextObj ) {
		UpdateObjectColAnims( objlist, colanim );
	}

	// scan missiles
	objlist = FetchFirstMissile();
	for ( ; objlist != NULL; objlist = objlist->NextObj ) {
		UpdateObjectColAnims( objlist, colanim );
	}

	// scan extras
	objlist = FetchFirstExtra();
	for ( ; objlist != NULL; objlist = objlist->NextObj ) {
		UpdateObjectColAnims( objlist, colanim );
	}

	// scan custom objects
	objlist = FetchFirstCustom();
	for ( ; objlist != NULL; objlist = objlist->NextObj ) {
		UpdateObjectColAnims( objlist, colanim );
	}
}


// register new colanim -------------------------------------------------------
//
int RegisterColAnim( colanimreg_s *colanimreg )
{
	ASSERT( colanimreg != NULL );

	if ( num_registered_colanims >= MAX_NUM_REGISTERED_COLANIMS )
		return FALSE;

	// name is mandatory
	if ( colanimreg->name == NULL )
		return FALSE;

	int			overwrite = FALSE;
	dword		colanimid = num_registered_colanims;
	colanim_s*	colanim   = &registered_colanims[ colanimid ].colanim;

	// check for already registered colanim of same name
	if ( FetchColAnim( colanimreg->name, &colanimid ) != NULL ) {

		if ( !AUX_ENABLE_COLANIM_OVERLOADING )
			return FALSE;

		// previously registered colanim will be overwritten
		colanim   = &registered_colanims[ colanimid ].colanim;
		overwrite = TRUE;

		// mem was reserved in one block for both tables
		ASSERT( colanim->col_table0 != NULL );
		FREEMEM( colanim->col_table0 );
		memset( colanim, 0, sizeof( colanim_s ) );

	} else {

		// allocate new colanim name
		char *name = (char *) ALLOCMEM( strlen( colanimreg->name ) + 1 );
		if ( name == NULL )
			OUTOFMEM( 0 );
		strcpy( name, colanimreg->name );

		ASSERT( registered_colanims[ colanimid ].name == NULL );
		registered_colanims[ colanimid ].name = name;

		num_registered_colanims++;
	}

	// copy new colanim info
	memcpy( colanim, &colanimreg->colanim, sizeof( colanim_s ) );

	// if old colanim has been overwritten its users must be updated
	if ( overwrite ) {
		UpdateWorldColAnims( colanim );
	}

	return TRUE;
}



