/*
 * PARSEC - Event Management
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:34 $
 *
 * Orginally written by:
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

// subsystem headers
#include "sys_defs.h"

// local module header
#include "e_events.h"

// proprietary module headers
#include "con_com.h"
#include "con_main.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// init flag ------------------------------------------------------------------
//
static int		manager_init_done = FALSE;


// list of all pending events -------------------------------------------------
//
static event_s* eventlist_head;
static event_s* eventlist_tail;

static int      num_events;
static event_s* current_event;


// initialize the event manager -----------------------------------------------
//
int	EVT_InitManager()
{
	ASSERT( !manager_init_done );
	if ( manager_init_done ) {
		return FALSE;
	}

	eventlist_head = NULL;
	eventlist_tail = NULL;
	num_events = 0;

	manager_init_done = TRUE;

	return TRUE;
}


// kill the event manager -----------------------------------------------------
//
int	EVT_KillManager()
{
	if ( !manager_init_done ) {
		return FALSE;
	}

	// an event-callback must not call this function
	ASSERT( current_event == NULL );

	// kill all autofree marked events
	for ( event_s* event = eventlist_head; event; ) {

		event_s* nextevent = event->next;

		if ( event->flags & EVENT_PARAM_AUTOFREE ) {
			if ( event->flags & EVENT_PARAM_AUTOFREE_CALLBACK ) {
				FREEMEM( event->callback_params );
			}

			FREEMEM( event );
		}

		event = nextevent;
	}

	eventlist_head = NULL;
	eventlist_tail = NULL;

	num_events = 0;

	manager_init_done = FALSE;

	return TRUE;
}


// add an event ---------------------------------------------------------------
//
size_t EVT_AddEvent( event_s *event )
{
	// check whether any other event of same type is to be deleted
	if ( event->flags & EVENT_PARAM_ONE_PER_TYPE ) {
		for ( event_s* otherevent = eventlist_head; otherevent; ) {
			event_s* nextevent = otherevent->next;

			if ( otherevent->type == event->type ) {
				EVT_RemoveEvent( (size_t)otherevent );
			}

			otherevent = nextevent;
		}
	}

	// add the event to the list
	if ( eventlist_tail != NULL ) {
		eventlist_tail->next = event;
		event->prec = eventlist_tail;
	} else {
		event->prec = NULL;
	}

	event->next		= NULL;
	eventlist_tail	= event;
	if ( eventlist_head == NULL ) {
		eventlist_head = event;
	}

	// increase the number of events
	num_events++;

	if ( event->flags & EVENT_PARAM_AUTOTRIGGER ) {
		// set the trigger time to the current refframe
		event->refframe_trigger = SYSs_GetRefFrameCount();
	} else {
		// set to not-triggered
		event->refframe_trigger = EVENT_UNTRIGGERED;
	}

	return (size_t)event;
}


// remove an event ------------------------------------------------------------
//
void EVT_RemoveEvent( size_t id )
{
	ASSERT( eventlist_head != NULL );
	ASSERT( eventlist_tail != NULL );

	event_s *event = (event_s *) id;
	ASSERT( event != NULL );

	// an event-callback must not delete its own event
	ASSERT( current_event != event );

	if ( eventlist_head == event )
		eventlist_head = event->next;

	if ( eventlist_tail == event )
		eventlist_tail = event->prec;

	if ( event->prec != NULL )
		event->prec->next = event->next;

	if ( event->next != NULL )
		event->next->prec = event->prec;

	if ( event->flags & EVENT_PARAM_AUTOFREE ) {
		if ( event->flags & EVENT_PARAM_AUTOFREE_CALLBACK ) {
			if ( event->callback_params != NULL ) {
				FREEMEM( event->callback_params );
			}
		}
		FREEMEM( event );
	}

	// decrease the number of events
	num_events--;
}


// remove all events for a specific type --------------------------------------
//
void EVT_RemoveEventType( dword type )
{
	for ( event_s* event = eventlist_head; event; ) {

		event_s* nextevent = event->next;

		if ( ( event->type & type ) == type ) {
			EVT_RemoveEvent( (size_t) event );
		}

		event = nextevent;
	}
}


// maintain the list of events to be triggered/fired --------------------------
//
void EVT_Maintain()
{
	//FIXME: introduce a frequency for issueing events (e.g. 100Hz = 6 Refframes)

	event_s* nextevent;
	for ( event_s* event = eventlist_head; event; event = nextevent ) {

		// store the next event
		nextevent = event->next;

		if ( event->refframe_trigger == EVENT_UNTRIGGERED )
			continue;

		// check whether event is triggered
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( event->refframe_trigger > refframecount )
			continue;

		// check whether the event is to be fired
		if ( ( event->refframe_trigger + event->refframe_delay ) > refframecount )
			continue;

		// check whether the event depends on another event that must be triggered
		if ( event->flags & EVENT_PARAM_DEPENDENT ) {
			event_s* parentevent = (event_s*)event->dependent_on; //Unused but I think someday it was meant to be?

			//FIXME: in order to have this work properly we must first
			//       go through all the events, resolve all dependencies
			//       and then check all the triggered events

			ASSERT( FALSE );
		}

		ASSERT( event->callback != NULL );

		// save the current event pointer
		current_event = event;

		// call the callback function
		int keepalive = event->callback( event->callback_params );

		// remove the current event pointer
		current_event = NULL;

		// might have been changed implicitly by callback function
		nextevent = event->next;

		// check whether event is to be looped
		dword loops = event->flags & EVENT_PARAM_LOOP_MASK;

		switch ( loops ) {

			case EVENT_PARAM_ONESHOT:
				event->loopcount = 0;
				break;
			case EVENT_PARAM_LOOP_COUNT:
				if ( event->loopcount > 0 )
					event->loopcount--;
				event->refframe_trigger = refframecount;
				break;
			case EVENT_PARAM_LOOP_INDEFINITE:
				event->loopcount = 1;
				event->refframe_trigger = refframecount;
				break;
			default:
				ASSERT(FALSE);
		}

		// remove event if specified by callback or loopcount exceeded
		if ( !keepalive || ( event->loopcount == 0 ) ) {
			EVT_RemoveEvent( (size_t)event );
		}
	}
}


// list all pending events ----------------------------------------------------
//
PRIVATE
int CMD_EVT_LIST( char *paramstr )
{
	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	for ( event_s* event = eventlist_head; event; ) {

		sprintf( paste_str, "type:%04xd delay:%u flags:%04xd loop:%u trigger:%u",
							(unsigned int)event->type,
							(unsigned int)event->refframe_delay,
							(unsigned int)event->flags,
							(unsigned int)event->loopcount,
							(unsigned int)event->refframe_trigger );
		CON_AddLine( paste_str );

		// advance in the list
		event = event->next;
	}

	return TRUE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( E_EVENTS )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "evt.list" command
	regcom.command	 = "evt.list";
	regcom.numparams = 0;
	regcom.execute	 = CMD_EVT_LIST;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );
}



