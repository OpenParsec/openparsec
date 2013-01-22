/*
 * PARSEC - Texture Animations
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
#include "e_texani.h"

// proprietary module headers
#include "con_main.h"



// maximum number of registered texanims --------------------------------------
//
#define MAX_NUM_REGISTERED_TEXANIMS		256


// tables for registered texanims ---------------------------------------------
//
PUBLIC int			num_registered_texanims = 0;
PUBLIC texanimreg_s	registered_texanims[ MAX_NUM_REGISTERED_TEXANIMS ];


// fetch already registered texanim via name ----------------------------------
//
texanimreg_s *FetchTexAnim( char *name, dword *texanimid )
{
	ASSERT( name != NULL );

	for ( int tid = 0; tid < num_registered_texanims; tid++ ) {

		ASSERT( registered_texanims[ tid ].name != NULL );
		if ( stricmp( registered_texanims[ tid ].name, name ) == 0 ) {

			if ( texanimid != NULL )
				*texanimid = tid;
			return &registered_texanims[ tid ];
		}
	}

	return NULL;
}


// init texanim in specified face anim state ----------------------------------
//
void InitAnimStateTexAnim( FaceAnimState *animstate, texanim_s *texanim )
{
	ASSERT( animstate != NULL );
	ASSERT( texanim != NULL );

	animstate->TexAnim = texanim;
	animstate->tex_pos = texanim->tex_start;
	animstate->xfo_pos = texanim->xfo_start;

	ASSERT( texanim->tex_table != NULL );
	texfrm_s *firsttexframe = &texanim->tex_table[ animstate->tex_pos ];
	animstate->tex_time	= firsttexframe->deltatime;

	if ( texanim->xfo_table != NULL ) {
		xfofrm_s *firstxfoframe = &texanim->xfo_table[ animstate->xfo_pos ];
		animstate->xfo_time = firstxfoframe->deltatime;
	}
}


// register new texanim -------------------------------------------------------
//
int RegisterTexAnim( texanimreg_s *texanimreg )
{
	ASSERT( texanimreg != NULL );

	if ( num_registered_texanims >= MAX_NUM_REGISTERED_TEXANIMS )
		return FALSE;

	// name is mandatory
	if ( texanimreg->name == NULL )
		return FALSE;

	int			overwrite = FALSE;
	dword		texanimid = num_registered_texanims;
//	texanim_s*	texanim   = &registered_texanims[ texanimid ].texanim;

	// check for already registered texanim of same name
	if ( FetchTexAnim( texanimreg->name, &texanimid ) != NULL ) {

//		if ( !AUX_ENABLE_COLANIM_OVERLOADING )
//			return FALSE;

		// previously registered texanim will be overwritten
//		texanim   = &registered_texanims[ texanimid ].texanim;
		overwrite = TRUE;

		// mem was reserved in one block for both tables
//		ASSERT( colanim->col_table0 != NULL );
//		FREEMEM( colanim->col_table0 );
//		memset( colanim, 0, sizeof( colanim_s ) );

	} else {

		// allocate new texanim name
		char *name = (char *) ALLOCMEM( strlen( texanimreg->name ) + 1 );
		if ( name == NULL )
			OUTOFMEM( 0 );
		strcpy( name, texanimreg->name );

		ASSERT( registered_texanims[ texanimid ].name == NULL );
		registered_texanims[ texanimid ].name = name;

		num_registered_texanims++;
	}

	return TRUE;
}



