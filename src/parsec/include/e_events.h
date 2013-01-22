/*
 * PARSEC HEADER: e_events.h
 */

#ifndef _E_EVENTS_H_
#define _E_EVENTS_H_



// event parameter flags ------------------------------------------------------
//
#define EVENT_PARAM_LOOP_MASK				0x000f	// mask for loop parameter
#define EVENT_PARAM_ONESHOT					0x0000	// execute event once
#define EVENT_PARAM_LOOP_COUNT				0x0001  // execute event loopinfo times
#define EVENT_PARAM_LOOP_INDEFINITE			0x0002  // execute event until removed

#define EVENT_PARAM_ONE_PER_TYPE			0x0010	// remove any other event of same type
#define EVENT_PARAM_DEPENDENT				0x0020  // trigger of event depends on other event's trigger
#define EVENT_PARAM_AUTOFREE				0x0040	// event is automatically freed after it's last fired
#define EVENT_PARAM_AUTOFREE_CALLBACK		0x0080	// callback data is automatically freed after it's last fired
#define EVENT_PARAM_AUTOTRIGGER				0x0100  // event is automatically triggered when added to the list

#define EVENT_UNTRIGGERED					-1		// event is not triggered yet


// generic event information --------------------------------------------------
//
struct event_s {

	event_s*	next;
	event_s*	prec;
	dword		type;						// type information of event
	int			(*callback)(void *);		// callback function to be called, when event is fired
	void*		callback_params;			// parameters passed to the callback function
	refframe_t	refframe_delay;				// delay in refframes for event from trigger to fire
	dword		flags;						// flags describing the event
	dword		loopcount;					// loop count for looping events
	dword		dependent_on;				// id of event this event depends on
	refframe_t	refframe_trigger;			// refframe when event is to trigger
	int			_mksiz64[6];
};


// for easy casting of event callback function pointers -----------------------
//
typedef int (*event_callback_t)(void*);


// external functions ---------------------------------------------------------
//
int		EVT_InitManager();
int		EVT_KillManager();
size_t	EVT_AddEvent( event_s *event );
void	EVT_RemoveEvent( size_t id );
void	EVT_RemoveEventType( dword type );
void	EVT_Maintain();


// include event definitions --------------------------------------------------
//
#include "e_evtdef.h"


#endif // _E_EVENTS_H_


