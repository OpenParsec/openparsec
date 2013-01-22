/*
 * PARSEC - Global Callback Management
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:22 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   1999
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

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// local module header
#include "e_callbk.h"



// type definition for drawing callback information ---------------------------
//
struct callback_info_s {

	callback_info_s*		next;
	global_callback_fpt		callback;
	void*					param;
	int						type;
	int						uniqueid;
	int						_mksiz32[ 3 ];
};


// lists of all registered callbacks ------------------------------------------
//
PRIVATE
callback_info_s *callback_infos[ CBTYPE_NUM_TYPES ];


// module global vars ---------------------------------------------------------
//
PRIVATE int next_callback_id = 0;		// id the next callback will be assigned
PRIVATE int bInitialized     = FALSE;   // flag to indicate whether the initialization has been made


// init callback code ---------------------------------------------------------
//
INLINE
void InitCallbackCode()
{
	for ( int cbt = 0; cbt < CBTYPE_NUM_TYPES; cbt++ ) {
		callback_infos[ cbt ] = NULL;
	}

	bInitialized = TRUE;
}


// register a callback of a specific type --------------------------------------
//
int CALLBACK_RegisterCallback( int callbacktype, global_callback_fpt callback, void *param )
{
	// init if not already done
	if ( !bInitialized ) {
		InitCallbackCode();
	}

	int _type = callbacktype & CBTYPE_MASK;
	ASSERT( ( _type >= 0 ) && ( _type < CBTYPE_NUM_TYPES ) );

	//NOTE:
	// special care has to be taken when providing a pointer to
	// a dynamic structure as param. for example, when an object
	// provides a pointer to itself it has to ensure the callback
	// is deleted when the object is destroyed. otherwise, the
	// callback function will be called with a pointer to an
	// invalid (already freed) object and usually crash.
	// CALLBACK_DestroyCallback() can be used for this purpose.

	// allocate new callback info structure
	callback_info_s* callback_info = (callback_info_s *) ALLOCMEM( sizeof( callback_info_s ) );
	if ( callback_info == NULL ) {
		ASSERT( 0 );
		return CBID_FAILED;
	}

	// set the data members
	callback_info->callback = callback;
	callback_info->param    = param;
	callback_info->type		= callbacktype;
	callback_info->uniqueid	= next_callback_id++;

	// insert at top of corresponding list
	callback_info->next		= callback_infos[ _type ];
	callback_infos[ _type ] = callback_info;

	// return id to make identification possible
	return callback_info->uniqueid;
}


// delete a callback of a specific type via its unique callback id ------------
//
int CALLBACK_DeleteCallback( int callbacktype, int callbackid )
{
	ASSERT( bInitialized == TRUE );

	int _type = callbacktype & CBTYPE_MASK;
	ASSERT( ( _type >= 0 ) && ( _type < CBTYPE_NUM_TYPES ) );

	// transparently handle callbacks that
	// have never actually been registered
	if ( callbackid == CBID_FAILED )
		return TRUE;

	callback_info_s* walk = callback_infos[ _type ];
	callback_info_s* prev = NULL;
	callback_info_s* next;

	int foundid = FALSE;

	// scan for callback with specified id
	for ( ; walk; walk = next ) {

		// remember next pointer
		next = walk->next;

		// remove callback if specified id
		if ( walk->uniqueid == callbackid ) {
			if ( prev == NULL ) {
				callback_infos[ _type ] = next;
			} else {
				prev->next = next;
			}
			ASSERT( !foundid );
			foundid = TRUE;
			FREEMEM( walk );
		} else {
			prev = walk;
		}
	}

	return foundid;
}


// destroy a callback of a specific type via its param pointer ----------------
//
int CALLBACK_DestroyCallback( int callbacktype, void *param )
{
	ASSERT( bInitialized == TRUE );

	//NOTE:
	// this function is usually used on the destruction
	// of an object that has been supplied as param to a
	// callback to avoid calling the callback function
	// with an invalid (already freed) pointer.

	int _type = callbacktype & CBTYPE_MASK;
	ASSERT( ( _type >= 0 ) && ( _type < CBTYPE_NUM_TYPES ) );

	// count number of removed callbacks
	int numremoved = 0;

	// never destroy NULL callbacks
	if ( param == NULL )
		return numremoved;

	callback_info_s* walk = callback_infos[ _type ];
	callback_info_s* prev = NULL;
	callback_info_s* next;

	// scan for callbacks with specified param
	for ( ; walk; walk = next ) {

		// remember next pointer
		next = walk->next;

		// remove callback if specified param
		if ( walk->param == param ) {
			if ( prev == NULL ) {
				callback_infos[ _type ] = next;
			} else {
				prev->next = next;
			}
			numremoved++;
			FREEMEM( walk );
		} else {
			prev = walk;
		}
	}

	return numremoved;
}


// walk registered callbacks of specific type and destroy them afterwards -----
//
int CALLBACK_WalkCallbacks( int callbacktype )
{
	ASSERT( bInitialized == TRUE );

	int _type = callbacktype & CBTYPE_MASK;
	ASSERT( ( _type >= 0 ) && ( _type < CBTYPE_NUM_TYPES ) );

	callback_info_s* walk = callback_infos[ _type ];
	callback_info_s* next;
	callback_info_s* newhead = NULL;

	// walk all callbacks contained in the specified list
	for ( ; walk != NULL; ) {

		// call the callback with the supplied parameter
		( walk->callback ) ( walk->param );

		// store next pointer
		next = walk->next;

		// check whether the callback should be removed after each call
		if ( ( walk->type & CBFLAG_MASK ) == CBFLAG_REMOVE ) {

			FREEMEM( walk );
			walk = NULL;

		} else {

			// insert in reverse order
			walk->next = newhead;
			newhead = walk;
		}

		// and advance to the next
		walk = next;
	}

	// and reset the head pointer of the list
	callback_infos[ _type ] = newhead;

	return TRUE;
}



