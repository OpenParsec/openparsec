/*
 * PARSEC - Action Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:34 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1997-2000
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
#include <math.h>
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

// subsystem headers
#include "aud_defs.h"
#include "inp_defs.h"
#include "net_defs.h"
#include "sys_defs.h"
#include "vid_defs.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "con_act.h"

// proprietary module headers
#include "con_aux.h"
#include "con_ext.h"
#include "con_int.h"
#include "con_main.h"
#include "e_demo.h"
#include "e_record.h"
#include "e_replay.h"
#include "e_supp.h"
#include "g_camera.h"
#include "g_supp.h"
#include "h_drwhud.h"
#include "h_supp.h"
#include "h_text.h"
#include "m_main.h"
#include "net_game.h"
#ifdef LINKED_PROTOCOL_GAMESERVER
	#include "net_serv.h"
#endif // LINKED_PROTOCOL_GAMESERVER
#include "obj_clas.h"
#include "obj_ctrl.h"
#include "obj_xtra.h"
#include "g_sfx.h"
#include "g_wfx.h"
#include "g_emp.h"
#include "sys_file.h"


// flags
#define REINIT_LOCAL_SHIP_ON_PREP_RESTORE
#define RECOVER_TARGET_ON_MISSILE_CREATION



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// string constants -----------------------------------------------------------
//
static char invalid_key_id[]	= "invalid key identifier.";
static char no_batch_open[] 	= "no script open.";
static char too_many_pseudos[]	= "too many pseudo stars.";
static char no_objcam_allowed[] = "object camera must not be active.";
static char invalid_list[]		= "invalid list identifier.";
static char invalid_class[] 	= "invalid object class identifier.";
static char invalid_action[]	= "invalid action.";
static char invalid_id_range[]	= "object class id out of range.";
static char no_action_found[]	= "no action specified.";
static char non_batch_use[] 	= "only valid if called from command script.";
static char stream_name_inval[]	= "stream name is invalid.";
static char no_stream_started[] = "stream could not be started.";
static char no_stream_stopped[] = "no stream has been stopped.";
static char action_preempted[]	= "action command not allowed.";
static char compile_write_err[]	= "write error during compilation.";
static char compile_arg_inval[]	= "invalid argument found during compilation.";
static char compile_non_comp[]	= "non-compilable command found during compilation.";
static char compile_pkt_inval[]	= "packet not found during compilation.";
static char only_for_compile[]	= "command is only for compilation.";
static char invalid_bin_wait[]	= "invalid binary wait argument encountered.";
static char maxplayers_diff[]	= "netplayers limit is different from recording.";


// pointer to binary parameters -----------------------------------------------
//
static char *bin_params = NULL;


// swap array of int values --------------------------------------------------
//
INLINE
void SwapParams_int( int num, int32 *source, int32 *dest )
{

#ifdef SYSTEM_BIG_ENDIAN

	for ( int indx = 0; indx < num; indx++ ) {
		dest[ indx ] = SWAP_32( source[ indx ] );
	}

#else

	memcpy( dest, source, sizeof( int32 ) * num );

#endif

}


// swap array of hprec_t values -----------------------------------------------
//
INLINE
void SwapParams_hprec( int num, hprec_t *source, hprec_t *dest )
{

#ifdef SYSTEM_BIG_ENDIAN

	for ( int indx = 0; indx < num; indx++ ) {

		dword *s = (dword *) &source[ indx ];
		dword *d = (dword *) &dest[ indx ];

		d[ 0 ] = SWAP_32( s[ 1 ] );
		d[ 1 ] = SWAP_32( s[ 0 ] );
	}

#else

	memcpy( dest, source, sizeof( hprec_t ) * num );

#endif

}


// allow no arguments at all instead of all arguments (one-shot flag!!) -------
//
static int silent_missing_args = FALSE;		// all args missing allowed
static int silent_missing_ok;				// args were missing, not invalid


// check single parameter for action command ----------------------------------
//
PRIVATE
int AcFetchIntParams( int num, int32 *params )
{
	ASSERT( num > 0 );
	ASSERT( params != NULL );

	// assume incorrectly missing arg
	silent_missing_ok = FALSE;

	// read parameters binary if possible
	if ( bin_params != NULL ) {
		SwapParams_int( num, (int32 *) bin_params, params );
		silent_missing_args = FALSE;
		return TRUE;
	}

	// read specified number of int parameters
	while ( num-- ) {

		char *nextparam = strtok( NULL, " " );
		if ( nextparam == NULL ) {
			if ( !silent_missing_args )
				CON_AddLine( arg_missing );
			silent_missing_ok	= silent_missing_args;
			silent_missing_args	= FALSE;
			return FALSE;
		}

		// disabling after first argument enforces either no
		// arguments at all, or the entire specified number
		// of arguments have to be specified
		silent_missing_args = FALSE;

		char *errpart;
		int32 ival = (int32) strtol( nextparam, &errpart, int_calc_base );

		if ( *errpart != 0 ) {
			CON_AddLine( invalid_arg );
			return FALSE;
		}

		*params++ = ival;
	}

	// make sure flag is reset
	ASSERT( !silent_missing_args );
	silent_missing_args = FALSE;

	if ( strtok( NULL, " " ) != NULL ) {
		CON_AddLine( too_many_args );
		return FALSE;
	}

	return TRUE;
}


// check float parameters for action command ----------------------------------
//
PRIVATE
int AcFetchFloatParams( int num, hprec_t *params )
{
	ASSERT( num > 0 );
	ASSERT( params != NULL );

	// assume incorrectly missing arg
	silent_missing_ok = TRUE;

	// read parameters binary if possible
	if ( bin_params != NULL ) {
		SwapParams_hprec( num, (hprec_t *) bin_params, params );
		silent_missing_args = FALSE;
		return TRUE;
	}

	// read specified number of high precision float parameters
	while ( num-- ) {

		char *nextparam = strtok( NULL, " " );
		if ( nextparam == NULL ) {
			if ( !silent_missing_args )
				CON_AddLine( arg_missing );
			silent_missing_ok	= silent_missing_args;
			silent_missing_args	= FALSE;
			return FALSE;
		}

		// disabling after first argument enforces either no
		// arguments at all, or the entire specified number
		// of arguments have to be specified
		silent_missing_args = FALSE;

		char *errpart;
		hprec_t dval = strtod( nextparam, &errpart );

		if ( *errpart != 0 ) {
			CON_AddLine( invalid_arg );
			return FALSE;
		}

		*params++ = dval;
	}

	// make sure flag is reset
	ASSERT( !silent_missing_args );
	silent_missing_args = FALSE;

	if ( strtok( NULL, " " ) != NULL ) {
		CON_AddLine( too_many_args );
		return FALSE;
	}

	return TRUE;
}


// check string parameter for action command ----------------------------------
//
PRIVATE
int AcFetchStringParam( char **param, int stripspace )
{
	ASSERT( param != NULL );

	// no invalid args
	silent_missing_ok = TRUE;

	// read parameter binary if possible
	if ( bin_params != NULL ) {
		*param = bin_params;
		silent_missing_args = FALSE;
		return TRUE;
	}

	char *nextparam = strtok( NULL, "" );
	if ( nextparam != NULL ) {

		if ( stripspace ) {

			// eat whitespace
			char *scan = nextparam;
			while ( *scan == ' ' )
				scan++;

			// argument found?
			if ( *scan != 0 ) {

				nextparam = scan;

				// cut off trailing whitespace
				while ( *scan != 0 )
					scan++;
				while ( *--scan == ' ' )
					{}
				*( scan + 1 ) = 0;

			} else {

				if ( !silent_missing_args )
					CON_AddLine( arg_missing );
				nextparam = NULL;
			}
		}

	} else {

		if ( !silent_missing_args ) {
			CON_AddLine( arg_missing );
		}
	}

	// reset flag
	silent_missing_args = FALSE;

	*param = nextparam;
	return ( nextparam != NULL );
}


// set automatic pitch (global variable) --------------------------------------
//
PRIVATE
void AcAutoPitch()
{
	bams_t angle;
	if ( AcFetchIntParams( 1, (int32 *) &angle ) ) {
		AutomaticPitch = angle;
	}
}


// set automatic yaw (global variable) ----------------------------------------
//
PRIVATE
void AcAutoYaw()
{
	bams_t angle;
	if ( AcFetchIntParams( 1, (int32 *) &angle ) ) {
		AutomaticYaw = angle;
	}
}


// set automatic roll (global variable) ---------------------------------------
//
PRIVATE
void AcAutoRoll()
{
	bams_t angle;
	if ( AcFetchIntParams( 1, (int32 *) &angle ) ) {
		AutomaticRoll = angle;
	}
}


// set automatic horizontal slide (global variable) ---------------------------
//
PRIVATE
void AcAutoSlideX()
{
	hprec_t slideval;
	if ( AcFetchFloatParams( 1, &slideval ) ) {
		AutomaticSlideHorz = FLOAT_TO_GEOMV( slideval );
	}
}


// set automatic vertical slide (global variable) -----------------------------
//
PRIVATE
void AcAutoSlideY()
{
	hprec_t slideval;
	if ( AcFetchFloatParams( 1, &slideval ) ) {
		AutomaticSlideVert = FLOAT_TO_GEOMV( slideval );
	}
}


// set automatic movement (global variable and MyShip's speed) ----------------
//
PRIVATE
void AcAutoMove()
{
	fixed_t translation;
	if ( AcFetchIntParams( 1, (int32 *) &translation ) ) {
		AutomaticMovement = translation;
		MyShip->CurSpeed  = AutomaticMovement;
	}
}


// set idle wait count --------------------------------------------------------
//
PRIVATE
void AcIdleCommand()
{
	int32 waitcount;
	if ( AcFetchIntParams( 1, &waitcount ) ) {

		// set duration to wait for
		ASSERT( waitcount >= 0 );
		CurActionWait = waitcount;

		// remember for info
		demoinfo_lastwait = (refframe_t) waitcount;

		// save level recursion is stalled on
		idle_reclevel = rec_depth;
	}
}


// flag for view camera filter reset after matrix init ------------------------
//
static int reset_viewcam_filter_on_update = FALSE;


// set single column of camera matrix -----------------------------------------
//
INLINE
void SetCameraColumn( Camera cam, int col, hprec_t *v )
{
	cam[ 0 ][ col ] = FLOAT_TO_GEOMV( v[ 0 ] );
	cam[ 1 ][ col ] = FLOAT_TO_GEOMV( v[ 1 ] );
	cam[ 2 ][ col ] = FLOAT_TO_GEOMV( v[ 2 ] );
}


// set column #1 of camera matrix ---------------------------------------------
//
PRIVATE
void AcSetCamRotCol1()
{
	hprec_t v[ 3 ];
	if ( AcFetchFloatParams( 3, v ) ) {
		SetCameraColumn( ShipViewCamera, 0, v );
	}
}


// set column #2 of camera matrix ---------------------------------------------
//
PRIVATE
void AcSetCamRotCol2()
{
	hprec_t v[ 3 ];
	if ( AcFetchFloatParams( 3, v ) ) {
		SetCameraColumn( ShipViewCamera, 1, v );
	}
}


// set column #3 of camera matrix ---------------------------------------------
//
PRIVATE
void AcSetCamRotCol3()
{
	hprec_t v[ 3 ];
	if ( AcFetchFloatParams( 3, v ) ) {
		SetCameraColumn( ShipViewCamera, 2, v );
	}
}


// set column #4 of camera matrix ---------------------------------------------
//
PRIVATE
void AcSetCamOrigin()
{
	hprec_t v[ 3 ];
	if ( AcFetchFloatParams( 3, v ) ) {
		SetCameraColumn( ShipViewCamera, 3, v );
	}

	// reset view camera smoothing filter if flag set
	// (must be used if matrix is updated discontinuously)
	if ( reset_viewcam_filter_on_update ) {
		CAMERA_ResetFilter();
		reset_viewcam_filter_on_update = FALSE;
	}
}


// set single column of object position matrix --------------------------------
//
PRIVATE
void SetObjectMatrxCol( int listid, int col, geomv_t x, geomv_t y, geomv_t z )
{
	GenObject *obj = NULL;

	switch ( listid ) {

		case PSHIP_LIST_NO:
			obj = (GenObject *) FetchFirstShip();
			break;

		case LASER_LIST_NO:
			obj = (GenObject *) FetchFirstLaser();
			break;

		case MISSL_LIST_NO:
			obj = (GenObject *) FetchFirstMissile();
			break;

		case EXTRA_LIST_NO:
			obj = (GenObject *) FetchFirstExtra();
			break;

		case CUSTM_LIST_NO:
			obj = (GenObject *) FetchFirstCustom();
			break;
	}

	if ( obj != NULL ) {
		obj->ObjPosition[ 0 ][ col ] = x;
		obj->ObjPosition[ 1 ][ col ] = y;
		obj->ObjPosition[ 2 ][ col ] = z;
	} else {
		CON_AddLine( invalid_list );
	}
}


// mapping table for class ids in demo to corresponding current ids -----------
//
static dword class_map_id_to_name[ MAX_DISTINCT_OBJCLASSES ];
static dword class_map_old_id;


// reset mapping table for class ids ------------------------------------------
//
void ResetClassMapTable()
{
	// needed to preserve table across multiple
	// demos that are played back-to-back
	if ( AUX_DISABLE_CLASS_MAP_TABLE_RESET )
		return;

	class_map_old_id = CLASS_ID_INVALID;

	for ( int cid = 0; cid < MAX_DISTINCT_OBJCLASSES; cid++ ) {
		class_map_id_to_name[ cid ] = CLASS_ID_INVALID;
	}
}


// set class id for which class name will be specified ------------------------
//
void AcClassMapId()
{
	int32 classid;
	if ( AcFetchIntParams( 1, &classid ) ) {
		if ( (dword)classid < MAX_DISTINCT_OBJCLASSES ) {
			class_map_old_id = classid;
		} else {
			class_map_old_id = CLASS_ID_INVALID;
		}
	}
}


// map class name to current id and set map to corresponding current id -------
//
void AcClassMapName()
{
	char *classname;
	if ( AcFetchStringParam( &classname, ACM_EAT_SPACE( ACM_CLASSMAPNAME ) ) ) {

		dword classid = OBJ_FetchObjectClassId( classname );
		if ( class_map_old_id != CLASS_ID_INVALID ) {
			ASSERT( class_map_old_id < MAX_DISTINCT_OBJCLASSES );
			class_map_id_to_name[ class_map_old_id ] = classid;
		}
	}
}


// execute recorded object creation -------------------------------------------
//
PRIVATE
GenObject *CreateRecordedObject( dword objclass, Xmatrx startmatrx )
{
	ASSERT( startmatrx != NULL );

	if ( objclass >= MAX_DISTINCT_OBJCLASSES ) {
		CON_AddLine( invalid_id_range );
		return NULL;
	}

	// map the recorded class id to the corresponding current class id
	if ( class_map_id_to_name[ objclass ] != CLASS_ID_INVALID ) {
		objclass = class_map_id_to_name[ objclass ];
	}

	// create object corresponding to original class
	GenObject *obj = CreateObject( objclass, startmatrx );
	if ( obj == NULL ) {

		CON_AddLine( invalid_class );

	} else {

		// play sound effects for certain types
		switch ( obj->ObjectType ) {

			case LASER1TYPE:
			case LASER2TYPE:
			case LASER3TYPE:
				AUD_Laser( obj );
				break;

			case MISSILE1TYPE:
			case MISSILE4TYPE:
				AUD_Missile( obj );
				break;

			case MINE1TYPE:
				AUD_Mine( obj );
				break;
		}
	}

	return obj;
}


// for object creation on "ac.objmatrxcol4" instead of "ac.creatobject" -------
//
static int		delayed_object_creation = FALSE;
static dword	delayed_object_class;
static dword	delayed_object_id;
static dword	delayed_object_hostid;
static Xmatrx	delayed_object_matrix;


// execute delayed creation of recorded object --------------------------------
//
INLINE
void DelayedObjectCreation()
{
	//NOTE:
	// delayed object creation ensures that the objposition matrix
	// is already valid on call of OBJ_CTRL::CreateObject().

	// create object on setting of last matrix column
	if ( delayed_object_creation ) {
		delayed_object_creation = FALSE;

		GenObject *obj =
			CreateRecordedObject( delayed_object_class, delayed_object_matrix );

		if ( obj != NULL ) {
/*
			if ( delayed_object_id != -1 ) {
				obj->ObjectNumber = delayed_object_id;
			}
*/
			if ( delayed_object_hostid != (dword)-1 ) {
				obj->HostObjNumber = delayed_object_hostid;
			}
		}
	}
}


// set single column of object position matrix --------------------------------
//
INLINE
void SetObjMCol( int col )
{
	hprec_t v[ 4 ];
	if ( AcFetchFloatParams( 4, v ) ) {

		if ( delayed_object_creation ) {
			// remember for later use
			delayed_object_matrix[ 0 ][ col ] = FLOAT_TO_GEOMV( v[ 1 ] );
			delayed_object_matrix[ 1 ][ col ] = FLOAT_TO_GEOMV( v[ 2 ] );
			delayed_object_matrix[ 2 ][ col ] = FLOAT_TO_GEOMV( v[ 3 ] );
		} else {
			// set column directly in object structure
			SetObjectMatrxCol( (int) v[ 0 ], col,
							   FLOAT_TO_GEOMV( v[ 1 ] ),
							   FLOAT_TO_GEOMV( v[ 2 ] ),
							   FLOAT_TO_GEOMV( v[ 3 ] ) );
		}
	}
}


// set single column of object position matrix --------------------------------
//
PRIVATE
void AcObjectMatrxColumn1()
{
	SetObjMCol( 0 );
}

PRIVATE
void AcObjectMatrxColumn2()
{
	SetObjMCol( 1 );
}

PRIVATE
void AcObjectMatrxColumn3()
{
	SetObjMCol( 2 );
}

PRIVATE
void AcObjectMatrxColumn4()
{
	SetObjMCol( 3 );
	DelayedObjectCreation();
}


// set column #1 of fixed star matrix -----------------------------------------
//
PRIVATE
void AcSetFixedStarRotCol1()
{
	// dummy for compatibility
}


// set column #2 of fixed star matrix -----------------------------------------
//
PRIVATE
void AcSetFixedStarRotCol2()
{
	// dummy for compatibility
}


// set column #3 of fixed star matrix -----------------------------------------
//
PRIVATE
void AcSetFixedStarRotCol3()
{
	// dummy for compatibility
}


// create object of specified class -------------------------------------------
//
PRIVATE
void AcCreateObject()
{
	ASSERT( !delayed_object_creation );

	int32 objclass;
	if ( AcFetchIntParams( 1, &objclass ) ) {

		if ( AUX_NO_DELAYED_CREATE_OBJECT_REPLAY ) {

			// create on "ac.creatobject"
			Xmatrx im;
			MakeIdMatrx( im );
			CreateRecordedObject( objclass, im );

		} else {

			// create later on "ac.objmatrxcol4"
			delayed_object_creation = TRUE;
			delayed_object_class    = objclass;
			delayed_object_id	= (dword)-1; // not yet set
			delayed_object_hostid	= (dword)-1; // not yet set
		}
	}
}


// delete all objects from specified object list ------------------------------
//
PRIVATE
void AcDestroyObjectList()
{
	if ( ObjCameraActive ) {
		CON_AddLine( no_objcam_allowed );
		return;
	}

	int32 listid;
	if ( AcFetchIntParams( 1, &listid ) ) {

		switch ( listid ) {

			case PSHIP_LIST_NO:
				FreeObjList( PShipObjects );
				break;

			case LASER_LIST_NO:
				FreeObjList( LaserObjects );
				break;

			case MISSL_LIST_NO:
				FreeObjList( MisslObjects );
				break;

			case EXTRA_LIST_NO:
				FreeObjList( ExtraObjects );
				break;

			case CUSTM_LIST_NO:
				FreeObjList( CustmObjects );
				break;

			default:
				CON_AddLine( invalid_list );
		}
	}
}


// write line 0 in small letters ----------------------------------------------
//
PRIVATE
void AcWriteSmall()
{
	char *nextparam;
	if ( AcFetchStringParam( &nextparam, ACM_EAT_SPACE( ACM_WRITESMALL ) ) ) {
//		HUD_SetDemoText( 0, nextparam );
	}
}


// write line 1 in capital letters --------------------------------------------
//
PRIVATE
void AcWriteBig()
{
	char *nextparam;
	if ( AcFetchStringParam( &nextparam, ACM_EAT_SPACE( ACM_WRITEBIG1 ) ) ) {
//		HUD_SetDemoText( 1, nextparam );
	}
}


// write line 2 in capital letters --------------------------------------------
//
PRIVATE
void AcWriteBig2()
{
	char *nextparam;
	if ( AcFetchStringParam( &nextparam, ACM_EAT_SPACE( ACM_WRITEBIG2 ) ) ) {
//		HUD_SetDemoText( 2, nextparam );
	}
}


// write line 3 in capital letters --------------------------------------------
//
PRIVATE
void AcWriteBig3()
{
	char *nextparam;
	if ( AcFetchStringParam( &nextparam, ACM_EAT_SPACE( ACM_WRITEBIG3 ) ) ) {
//		HUD_SetDemoText( 3, nextparam );
	}
}


// scroll demo text out -------------------------------------------------------
//
PRIVATE
void AcClearText()
{
//	HUD_ClearDemoText();
}


// set global maximum number of remote players (NET_GLOB::CurMaxPlayers) ------
//
PRIVATE
void AcRemoteMaxPlayers()
{
	if ( !NetConnected ) {
		CON_AddLine( "AcRemoteMaxPlayers(): no net." );
		return;
	}

	int32 maxnumplayers;
	if ( AcFetchIntParams( 1, &maxnumplayers ) ) {

		if ( NetConnected == NETWORK_GAME_SIMULATED ) {
			// simply set it
			CurMaxPlayers = maxnumplayers;
		} else {
			// can't set it, display warning if different
			if ( CurMaxPlayers != maxnumplayers ) {
				CON_AddLine( maxplayers_diff );
			}
		}
	}
}


// remember id of remote player whose state was set last ----------------------
//
static int last_remote_playerid = 0;


// create remote player with specified state ----------------------------------
//
PRIVATE
void AcRemotePlayer()
{
	if ( !NetConnected ) {
		CON_AddLine( "AcRemotePlayer(): no net." );
		return;
	}

	//NOTE:
	// if this is invoked specifying the local player, the
	// local id may change. this has to happen in order to
	// restore a consistent state with respect to the state
	// during recording. the state of the local player will
	// also be used to determine whether to activate or
	// deactivate entry-mode automatically.

	int32 params[ 4 ];
	if ( AcFetchIntParams( 4, params ) ) {

		// remember id for AcRemoteMatrix(), AcRemoteName(),
		// and AcRemoteWeaponsActive()
		last_remote_playerid = params[ 0 ];

		// init state except matrix and name
		NET_SetRemotePlayerState( params[ 0 ], params[ 1 ], params[ 2 ], params[ 3 ] );
	}
}


// set remote player matrix ---------------------------------------------------
//
PRIVATE
void AcRemoteMatrix()
{
	if ( !NetConnected ) {
		CON_AddLine( "AcRemoteMatrix(): no net." );
		return;
	}

	hprec_t v[ 12 ];
	if ( AcFetchFloatParams( 12, v ) ) {

		Xmatrx matrix;
		matrix[ 0 ][ 0 ] = FLOAT_TO_GEOMV( v[ 0 ] );
		matrix[ 0 ][ 1 ] = FLOAT_TO_GEOMV( v[ 1 ] );
		matrix[ 0 ][ 2 ] = FLOAT_TO_GEOMV( v[ 2 ] );
		matrix[ 0 ][ 3 ] = FLOAT_TO_GEOMV( v[ 3 ] );
		matrix[ 1 ][ 0 ] = FLOAT_TO_GEOMV( v[ 4 ] );
		matrix[ 1 ][ 1 ] = FLOAT_TO_GEOMV( v[ 5 ] );
		matrix[ 1 ][ 2 ] = FLOAT_TO_GEOMV( v[ 6 ] );
		matrix[ 1 ][ 3 ] = FLOAT_TO_GEOMV( v[ 7 ] );
		matrix[ 2 ][ 0 ] = FLOAT_TO_GEOMV( v[ 8 ] );
		matrix[ 2 ][ 1 ] = FLOAT_TO_GEOMV( v[ 9 ] );
		matrix[ 2 ][ 2 ] = FLOAT_TO_GEOMV( v[ 10 ] );
		matrix[ 2 ][ 3 ] = FLOAT_TO_GEOMV( v[ 11 ] );

		// set matrix of remote player (using id set by AcRemotePlayer())
		NET_SetRemotePlayerMatrix( last_remote_playerid, matrix );

		// for local player additionally set view camera
		if ( Player_ShipId[ last_remote_playerid ] == SHIPID_LOCALPLAYER ) {

			ASSERT( LocalPlayerId == last_remote_playerid );
			ASSERT( Player_Ship[ last_remote_playerid ] == MyShip );

			ObjCamOff();
			CalcOrthoInverse( MyShip->ObjPosition, DestXmatrx );
			memcpy( ShipViewCamera, DestXmatrx, sizeof( Camera ) );
			CAMERA_ResetFilter();
		}
	}
}


// set remote player weapons state --------------------------------------------
//
PRIVATE
void AcRemoteWeaponsActive()
{
	//NOTE:
	// if new duration weapons are added this function
	// should be updated.

	if ( !NetConnected ) {
		CON_AddLine( "AcRemoteWeaponsActive(): no net." );
		return;
	}

	int32 weaponsactive;
	if ( AcFetchIntParams( 1, &weaponsactive ) ) {

		ASSERT( Player_Ship[ last_remote_playerid ] != NULL );
		ASSERT( Player_ShipId[ last_remote_playerid ] != SHIPID_LOCALPLAYER );

		ShipObject *shippo = (ShipObject *) Player_Ship[ last_remote_playerid ];
		ASSERT( shippo != NULL );
		ASSERT( shippo != MyShip );

		// check for lightning state change
		dword oldstate = shippo->WeaponsActive & WPMASK_CANNON_LIGHTNING;
		dword newstate = weaponsactive & WPMASK_CANNON_LIGHTNING;
		if ( ( oldstate == 0 ) && ( newstate != 0 ) ) {
			WFX_RemoteActivateLightning( last_remote_playerid );
		} else if ( ( oldstate != 0 ) && ( newstate == 0 ) ) {
			WFX_RemoteDeactivateLightning( last_remote_playerid );
		}

		// check for helix state change
		oldstate = shippo->WeaponsActive & WPMASK_CANNON_HELIX;
		newstate = weaponsactive & WPMASK_CANNON_HELIX;
		if ( ( oldstate == 0 ) && ( newstate != 0 ) ) {
			WFX_RemoteActivateHelix( last_remote_playerid );
		} else if ( ( oldstate != 0 ) && ( newstate == 0 ) ) {
			WFX_RemoteDeactivateHelix( last_remote_playerid );
		}

        // check for photon state change
        oldstate = shippo->WeaponsActive & WPMASK_CANNON_PHOTON;
        newstate = weaponsactive & WPMASK_CANNON_PHOTON;
		if ( ( oldstate == 0 ) && ( newstate != 0 ) ) {
            WFX_RemoteActivatePhoton( last_remote_playerid );
		} else if ( ( oldstate != 0 ) && ( newstate == 0 ) ) {
            WFX_RemoteDeactivatePhoton( last_remote_playerid );
		}

        // check for emp state change
        oldstate = shippo->WeaponsActive & WPMASK_DEVICE_EMP;
        newstate = weaponsactive & WPMASK_DEVICE_EMP;
		if ( ( oldstate == 0 ) && ( newstate != 0 ) ) {
            WFX_RemoteActivateEmp( last_remote_playerid );
		} else if ( ( oldstate != 0 ) && ( newstate == 0 ) ) {
            WFX_RemoteDeactivateEmp( last_remote_playerid );
		}

        // fallback for unknown duration weapons
		// (simply set/reset activation bits)
		dword unknownmask = 0xffffffff;
		unknownmask &= ~WPMASK_CANNON_LIGHTNING;
		unknownmask &= ~WPMASK_CANNON_HELIX;
		unknownmask &= ~WPMASK_CANNON_PHOTON;
		unknownmask &= ~WPMASK_DEVICE_EMP;
		shippo->WeaponsActive &= ~unknownmask;
		shippo->WeaponsActive |= weaponsactive & unknownmask;
	}
}


// should a join be performed on the next "ac.remotename" ? -------------------
//
static int join_on_setname = FALSE;


// perform recorded join ------------------------------------------------------
//
PRIVATE
void PerformRecordedJoin()
{
	if ( !NetJoined ) {
		HideFloatingMenu();
		NETs_Join();
		EntryMode		= FALSE;
		InFloatingMenu	= FALSE;
	}

	ASSERT( EntryMode == FALSE );
	ASSERT( InFloatingMenu == FALSE );
}


// set remote player name -----------------------------------------------------
//
PRIVATE
void AcRemoteName()
{
	if ( !NetConnected ) {
		CON_AddLine( "AcRemoteName(): no net." );
		return;
	}

	char *nextparam;
	if ( AcFetchStringParam( &nextparam, ACM_EAT_SPACE( ACM_REMOTENAME ) ) ) {

		// set name of remote player (using id set by AcRemotePlayer())
		NET_SetRemotePlayerName( last_remote_playerid, nextparam );
	}

	// perform delayed join
	if ( join_on_setname ) {
		join_on_setname = FALSE;
		PerformRecordedJoin();
	}
}


// perform join for local player ----------------------------------------------
//
PRIVATE
void AcRemoteJoin()
{
	ASSERT( !join_on_setname );

	if ( !NetConnected ) {
		CON_AddLine( "AcRemoteJoin(): no net." );
		return;
	}

	int32 params[ 3 ];
	if ( AcFetchIntParams( 3, params ) ) {

		ASSERT( LocalPlayerId == params[ 0 ] );

		//NOTE:
		// player position has to be set by following "ac.remotematrix".

		// remember id for AcRemoteMatrix(), AcRemoteName(),
		// and AcRemoteWeaponsActive()
		last_remote_playerid = params[ 0 ];

		// prepare local ship for join
		InitJoinReplay();

		if ( AUX_NO_DELAYED_JOIN_REPLAY ) {
			PerformRecordedJoin();
		} else {
			join_on_setname = TRUE;
		}
	}
}


// perform unjoin for local player --------------------------------------------
//
PRIVATE
void AcRemoteUnjoin()
{
	ASSERT( !join_on_setname );

	if ( !NetConnected ) {
		CON_AddLine( "AcRemoteUnjoin(): no net." );
		return;
	}

	int32 params[ 3 ];
	if ( AcFetchIntParams( 3, params ) ) {

		ASSERT( LocalPlayerId == params[ 0 ] );

		// set flags if not already unjoined
		if ( NetJoined ) {
			ExitGameLoop = 2;
			if ( params[ 1 ] == SHIP_DOWNED ) {
				ShipDowned = TRUE;
				AUD_PlayerKilled();
			} else {
				ShipDowned = FALSE;
			}
		}
	}
}


// clear remnants of demo replay ----------------------------------------------
//
PRIVATE
void AcClearDemo()
{
	//NOTE:
	// this function has to be used after a demo containing network packets
	// has been replayed (and ended) to restore/ensure a proper game state.

	//NOTE:
	// if the network mode is not active or the net simulation has already
	// been stopped (e.g., the demo has finished) the network state will
	// be left untouched (i.e., not reset!).

	ASSERT( !join_on_setname );

	// specify optional argument flag
	silent_missing_args = TRUE;

	// default is stop demo if running
	int32 stopflag = 1;
	AcFetchIntParams( 1, &stopflag );

	//NOTE:
	// ( silent_missing_ok == FALSE ) is alright here, since an error
	// message will already have been displayed and we only wanted to
	// override a default value anyway.

	// perform actual clear
	DEMO_ClearDemo( stopflag );
}


// start playback of recorded remote packet -----------------------------------
//
PRIVATE
void AcPlayRemotePacket()
{
	int32 params[ 2 ];
	if ( AcFetchIntParams( 2, params ) ) {

		REC_PlayRemotePacket( params[ 0 ], params[ 1 ] );
	}
}


// replay recorded server message ---------------------------------------------
//
PRIVATE
void AcPlayServerMessage()
{
	char *nextparam;
	if ( AcFetchStringParam( &nextparam, ACM_EAT_SPACE( ACM_SERVERMSG ) ) ) {

#ifdef LINKED_PROTOCOL_GAMESERVER
		if ( NET_ProtocolGMSV() ) {
			NET_ServerParseMessage( nextparam );
		} else {
			DBGTXT( MSGOUT( "AcPlayServerMessage(): in non GMSV mode !" ); );
		}
#endif // LINKED_PROTOCOL_GAMESERVER

	}
}


// display message in console -------------------------------------------------
//
PRIVATE
void AcWriteConsoleText()
{
	char *msgtext;
	if ( AcFetchStringParam( &msgtext, ACM_EAT_SPACE( ACM_WRITECONSOLETEXT ) ) ) {

		CON_AddLine( msgtext );
	}
}


// display message in message area --------------------------------------------
//
PRIVATE
void AcDisplayMessage()
{
	char *msgtext;
	if ( AcFetchStringParam( &msgtext, ACM_EAT_SPACE( ACM_DISPLAYMESSAGE ) ) ) {

		ShowMessage( msgtext );
	}
}


// execute a command supplied as string ---------------------------------------
//
PRIVATE
void AcExecCommandString()
{
	// make sure this is called from within a compiled demo
	if ( bin_params == NULL ) {
		CON_AddLine( "only valid during binary replay." );
		return;
	}

	//NOTE:
	// extreme care must be taken which commands are supplied to this
	// function. right now, there are no safety checks for commands
	// that must not be executed using this function.

	char *command;
	if ( AcFetchStringParam( &command, ACM_EAT_SPACE( ACM_EXECCOMMAND ) ) ) {

		// command must belong to the class of non-compilable commands
		ProcessExternalLine( command );
		ExecNonCompilableCommand( command );
	}
}


// macro to return with error message if recording inactive -------------------
//
#define CHECK_RECORDING_STATE() \
\
if ( !RecordingActive ) { \
	CON_AddLine( no_batch_open ); \
	return; \
}


// save all objects from specified object list --------------------------------
//
PRIVATE
void AcSaveObjectList()
{
	CHECK_RECORDING_STATE();

	// disallow object camera
	if ( ObjCameraActive ) {
		CON_AddLine( no_objcam_allowed );
		return;
	}

	int32 listid;
	if ( AcFetchIntParams( 1, &listid ) ) {

		switch ( listid ) {

			case PSHIP_LIST_NO:
				Save_ObjListShips( RecordingFp );
				break;

			case LASER_LIST_NO:
				Save_ObjListLasers( RecordingFp );
				break;

			case MISSL_LIST_NO:
				Save_ObjListMissls( RecordingFp );
				break;

			case EXTRA_LIST_NO:
				Save_ObjListExtras( RecordingFp );
				break;

			case CUSTM_LIST_NO:
				Save_ObjListCustom( RecordingFp );
				break;

			default:
				CON_AddLine( invalid_list );
		}
	}
}


// save pseudo star positions -------------------------------------------------
//
PRIVATE
void AcSavePseudoStars()
{
	CHECK_RECORDING_STATE();
	Save_PseudoStars( RecordingFp );
}


// save camera position and orientation ---------------------------------------
//
PRIVATE
void AcSaveCamera()
{
	CHECK_RECORDING_STATE();
	Save_Camera( RecordingFp );
}


// save fixed stars orientation -----------------------------------------------
//
PRIVATE
void AcSaveFixedStars()
{
	// dummy for compatibility
}


// save state of all remote players -------------------------------------------
//
PRIVATE
void AcSaveRemoteState()
{
	CHECK_RECORDING_STATE();
	Save_RemoteState( RecordingFp );
}


// kill all pseudo stars ------------------------------------------------------
//
PRIVATE
void AcKillPseudoStars()
{
	NumPseudoStars = 0;
}


// add single pseudo star -----------------------------------------------------
//
PRIVATE
void AcAddPseudoStar()
{
	if ( NumPseudoStars < MAX_PSEUDO_STARS ) {
		hprec_t pos[ 3 ];
		if ( AcFetchFloatParams( 3, pos ) ) {
			int no = NumPseudoStars++;
			PseudoStars[ no ].X = FLOAT_TO_GEOMV( pos[ 0 ] );
			PseudoStars[ no ].Y = FLOAT_TO_GEOMV( pos[ 1 ] );
			PseudoStars[ no ].Z = FLOAT_TO_GEOMV( pos[ 2 ] );
		}
	} else {
		CON_AddLine( too_many_pseudos );
	}
}


// prepare restoration of data screen shot ------------------------------------
//
PRIVATE
void AcPrepareDataRestore()
{
	int shipclass = MyShip->ObjectClass;

	// allow optional specification of ship class
	int32 sclass;
	if ( AcFetchIntParams( 1, &sclass ) ) {
		shipclass = sclass;
	}

	// free objects and particles
	KillAllObjects();

	// delete pseudo stars
	AcKillPseudoStars();

#ifdef REINIT_LOCAL_SHIP_ON_PREP_RESTORE

	// reinit local player's ship
	InitLocalShipStatus( shipclass );
	InitHudDisplay();

#endif // REINIT_LOCAL_SHIP_ON_PREP_RESTORE

	// globally select shipclass
	LocalShipClass = MyShip->ObjectClass;

	// set flag that view camera filter has to be reset
	// after the new matrix has been set
	reset_viewcam_filter_on_update = TRUE;

	// reset mapping table for class ids
	ResetClassMapTable();
}


// save data screen shot ------------------------------------------------------
//
PRIVATE
void AcSaveDataInfo()
{
	//NOTE:
	// this function tries to create the logical analog to a screenshot.
	// for this, it records the current state (that is, commands that
	// are able to restore the current state on their execution).

	CHECK_RECORDING_STATE();
	Save_StateInfo( RecordingFp );
}


// fade screen out (to black) -------------------------------------------------
//
PRIVATE
void AcFadeOut()
{
//	VIDs_FadeScreenToBlack( PAL_GAME, FALSE );

	int32 speed;
	if ( AcFetchIntParams( 1, &speed ) ) {

		SetScreenFadeSpeed = speed;

		SetScreenFadeColor.R = 0;
		SetScreenFadeColor.G = 0;
		SetScreenFadeColor.B = 0;
//		SetScreenFadeColor.A = irrelevant;

		// continue fading from current value
		if ( SetScreenFade == 0 ) {
			SetScreenFade = 0xff00;
		} else if ( SetScreenFade < 0 ) {
			SetScreenFade += 0xff00;
			if ( SetScreenFade < 0 ) {
				SetScreenFade = 0;
			}
		}
	}
}


// fade screen in (from black) ------------------------------------------------
//
PRIVATE
void AcFadeIn()
{
//	VIDs_FadeScreenFromBlack( PAL_GAME );

	int32 speed;
	if ( AcFetchIntParams( 1, &speed ) ) {

		SetScreenFadeSpeed = speed;

		SetScreenFadeColor.R = 0;
		SetScreenFadeColor.G = 0;
		SetScreenFadeColor.B = 0;
//		SetScreenFadeColor.A = irrelevant;

		// continue fading from current value
		if ( ( SetScreenFade == 0 ) || ( SetScreenFade == 1 ) ) {
			SetScreenFade = -0xff00;
		} else if ( SetScreenFade > 0 ) {
			SetScreenFade += -0xff00;
			if ( SetScreenFade > 0 ) {
				SetScreenFade = 0;
			}
		}
	}
}


// fade screen out (to white) -------------------------------------------------
//
PRIVATE
void AcWhiteFadeOut()
{
//	colrgba_s scol = COLRGBA_WHITE;
//	VIDs_SetScreenToColor( scol );

	int32 speed;
	if ( AcFetchIntParams( 1, &speed ) ) {

		SetScreenFadeSpeed = speed;

		SetScreenFadeColor.R = 255;
		SetScreenFadeColor.G = 255;
		SetScreenFadeColor.B = 255;
//		SetScreenFadeColor.A = irrelevant;

		// continue fading from current value
		if ( SetScreenFade == 0 ) {
			SetScreenFade = 0xff00;
		} else if ( SetScreenFade < 0 ) {
			SetScreenFade += 0xff00;
			if ( SetScreenFade < 0 ) {
				SetScreenFade = 0;
			}
		}
	}
}


// fade screen in (from white) ------------------------------------------------
//
PRIVATE
void AcWhiteFadeIn()
{
//	VIDs_FadeScreenFromWhite( PAL_GAME );

	int32 speed;
	if ( AcFetchIntParams( 1, &speed ) ) {

		SetScreenFadeSpeed = speed;

		SetScreenFadeColor.R = 255;
		SetScreenFadeColor.G = 255;
		SetScreenFadeColor.B = 255;
//		SetScreenFadeColor.A = irrelevant;

		// continue fading from current value
		if ( ( SetScreenFade == 0 ) || ( SetScreenFade == 1 ) ) {
			SetScreenFade = -0xff00;
		} else if ( SetScreenFade > 0 ) {
			SetScreenFade += -0xff00;
			if ( SetScreenFade > 0 ) {
				SetScreenFade = 0;
			}
		}
	}
}


// set standard palette -------------------------------------------------------
//
PRIVATE
void AcStandardPalette()
{
//	AwaitVertSync();

	// turn off fading
	SetScreenFade = 0;
}


// set entire palette to black ------------------------------------------------
//
PRIVATE
void AcBlackScreen()
{
//	colrgba_s scol = COLRGBA_BLACK;
//	VIDs_SetScreenToColor( scol );

	SetScreenFadeColor.R = 0;
	SetScreenFadeColor.G = 0;
	SetScreenFadeColor.B = 0;
//	SetScreenFadeColor.A = irrelevant;

	// hold fading at opaque level
	SetScreenFade = 1;
}


// set entire palette to white ------------------------------------------------
//
PRIVATE
void AcWhiteScreen()
{
//	colrgba_s scol = COLRGBA_WHITE;
//	VIDs_SetScreenToColor( scol );

	SetScreenFadeColor.R = 255;
	SetScreenFadeColor.G = 255;
	SetScreenFadeColor.B = 255;
//	SetScreenFadeColor.A = irrelevant;

	// hold fading at opaque level
	SetScreenFade = 1;
}


// set speed property of laser object -----------------------------------------
//
PRIVATE
void AcSetLaserSpeed()
{
	fixed_t speed;
	if ( AcFetchIntParams( 1, (int32 *) &speed ) ) {
		LaserObject *scanpo = FetchFirstLaser();
		ASSERT( scanpo != NULL );
		scanpo->Speed = speed;
	}
}


// set direction vector property of laser object ------------------------------
//
PRIVATE
void AcSetLaserDirVec()
{
	hprec_t vec[ 3 ];
	if ( AcFetchFloatParams( 3, vec ) ) {
		LaserObject *scanpo = FetchFirstLaser();
		ASSERT( scanpo != NULL );
		scanpo->DirectionVec.X = FLOAT_TO_GEOMV( vec[ 0 ] );
		scanpo->DirectionVec.Y = FLOAT_TO_GEOMV( vec[ 1 ] );
		scanpo->DirectionVec.Z = FLOAT_TO_GEOMV( vec[ 2 ] );
	}
}


// set owner property of laser object -----------------------------------------
//
PRIVATE
void AcSetLaserOwner()
{
	int32 owner;
	if ( AcFetchIntParams( 1, &owner ) ) {
		LaserObject *scanpo = FetchFirstLaser();
		ASSERT( scanpo != NULL );
		scanpo->Owner = owner;
	}
}


// set speed property of missile object ---------------------------------------
//
PRIVATE
void AcSetMisslSpeed()
{
	fixed_t speed;
	if ( AcFetchIntParams( 1, (int32 *) &speed ) ) {
		MissileObject *scanpo = FetchFirstMissile();
		ASSERT( scanpo != NULL );
		scanpo->Speed = speed;
	}

}


// set direction vector property of missile object ----------------------------
//
PRIVATE
void AcSetMisslDirVec()
{
	hprec_t vec[ 3 ];
	if ( AcFetchFloatParams( 3, vec ) ) {
		MissileObject *scanpo = FetchFirstMissile();
		ASSERT( scanpo != NULL );
		scanpo->DirectionVec.X = FLOAT_TO_GEOMV( vec[ 0 ] );
		scanpo->DirectionVec.Y = FLOAT_TO_GEOMV( vec[ 1 ] );
		scanpo->DirectionVec.Z = FLOAT_TO_GEOMV( vec[ 2 ] );
	}
}


// set owner property of missile object ---------------------------------------
//
PRIVATE
void AcSetMisslOwner()
{
	int32 owner;
	if ( AcFetchIntParams( 1, &owner ) ) {
		MissileObject *scanpo = FetchFirstMissile();
		ASSERT( scanpo != NULL );
		scanpo->Owner = owner;
	}
}


// set target id property of missile object -----------------------------------
//
PRIVATE
void AcSetMisslTarget()
{
	//NOTE:
	// this command will only occur to set the target
	// of a guided missile fired by the local player.

	int32 target;
	if ( AcFetchIntParams( 1, &target ) ) {

		TargetMissileObject *scanpo = (TargetMissileObject *) FetchFirstMissile();
		ASSERT( scanpo != NULL );
		ASSERT( ( scanpo->ObjectType & TYPECONTROLMASK ) == TYPEMISSILEISHOMING );
		scanpo->TargetObjNumber = target;

#ifdef RECOVER_TARGET_ON_MISSILE_CREATION

		if ( TargetObjNumber != (dword)target ) {

			// set selected target to ensure the current
			// selection is not wrong due to a lost keypress
			TargetObjNumber = target;
			CON_AddLine( "recovering target." );
		}
#endif

	}
}


// set owner property of mine object ------------------------------------------
//
PRIVATE
void AcSetMineOwner()
{
	int32 owner;
	if ( AcFetchIntParams( 1, &owner ) ) {
		MineObject *scanpo = (MineObject *) FetchFirstExtra();
		ASSERT( scanpo != NULL );
		scanpo->Owner = owner;
	}
}


// create an extra ------------------------------------------------------------
//
PRIVATE
void AcCreateExtra()
{
	ASSERT( !delayed_object_creation );

	int32 objclass;
	if ( AcFetchIntParams( 1, &objclass ) ) {

		if ( (dword)objclass >= MAX_DISTINCT_OBJCLASSES ) {
			CON_AddLine( invalid_id_range );
			return;
		}

		// map the recorded class id to the corresponding current class id
		if ( class_map_id_to_name[ objclass ] != CLASS_ID_INVALID ) {
			objclass = class_map_id_to_name[ objclass ];
		}

		// create extra corresponding to original class
		Xmatrx im;
		MakeIdMatrx( im );
		ExtraObject *extrapo = (ExtraObject *) CreateObject( objclass, im );
		if ( extrapo == NULL ) {
			CON_AddLine( invalid_class );
		} else {
			OBJ_FillExtraMemberVars( extrapo );
		}
	}
}


// create an energy field -----------------------------------------------------
//
PRIVATE
void AcEnergyField()
{
	hprec_t vec[ 3 ];
	if ( AcFetchFloatParams( 3, vec ) ) {
		Vertex3 origin;
		origin.X = FLOAT_TO_GEOMV( vec[ 0 ] );
		origin.Y = FLOAT_TO_GEOMV( vec[ 1 ] );
		origin.Z = FLOAT_TO_GEOMV( vec[ 2 ] );
		SFX_CreateEnergyField( origin );
	}
}


// shoot spreadfire gun -------------------------------------------------------
//
PRIVATE
void AcSpreadfire()
{
	WFX_ShootParticleWeapon( MyShip, PARTICLEGUN_SPREADFIRE );
}


// activate lightning ---------------------------------------------------------
//
PRIVATE
void AcLightningOn()
{
	if ( ( MyShip->WeaponsActive & WPMASK_CANNON_LIGHTNING ) == 0 ) {
		WFX_ActivateLightning( MyShip );
	} else {
		CON_AddLine( "filtering redundant lightning activation." );
	}
}


// deactivate lightning -------------------------------------------------------
//
PRIVATE
void AcLightningOff()
{
	if ( ( MyShip->WeaponsActive & WPMASK_CANNON_LIGHTNING ) != 0 ) {
		WFX_DeactivateLightning( MyShip );
	} else {
		CON_AddLine( "filtering redundant lightning deactivation." );
	}
}


// activate helix -------------------------------------------------------------
//
void AcHelixOn()
{
	if ( ( MyShip->WeaponsActive & WPMASK_CANNON_HELIX ) == 0 ) {
		WFX_ActivateHelix( MyShip );
	} else {
		CON_AddLine( "filtering redundant helix activation." );
	}
}


// deactivate helix -----------------------------------------------------------
//
void AcHelixOff()
{
	if ( ( MyShip->WeaponsActive & WPMASK_CANNON_HELIX ) != 0 ) {
		WFX_DeactivateHelix( MyShip );
	} else {
		CON_AddLine( "filtering redundant helix deactivation." );
	}
}


// activate photon ------------------------------------------------------------
//
void AcPhotonOn()
{
	if ( ( MyShip->WeaponsActive & WPMASK_CANNON_PHOTON ) == 0 ) {
		WFX_ActivatePhoton( MyShip );
	} else {
		CON_AddLine( "filtering redundant photon activation." );
	}
}


// deactivate photon ----------------------------------------------------------
//
void AcPhotonOff()
{
	if ( ( MyShip->WeaponsActive & WPMASK_CANNON_PHOTON ) != 0 ) {
		WFX_DeactivatePhoton( MyShip );
	} else {
		CON_AddLine( "filtering redundant photon deactivation." );
	}
}


// activate emp ---------------------------------------------------------------
//
void AcEmpOn()
{
	if ( ( MyShip->WeaponsActive & WPMASK_DEVICE_EMP ) == 0 ) {
		WFX_ActivateEmp( MyShip );
	} else {
		CON_AddLine( "filtering redundant emp activation." );
	}
}


// deactivate emp -------------------------------------------------------------
//
void AcEmpOff()
{
	if ( ( MyShip->WeaponsActive & WPMASK_DEVICE_EMP) != 0 ) {
		WFX_DeactivateEmp( MyShip );
	} else {
		CON_AddLine( "filtering redundant emp deactivation." );
	}
}


// fire emp -------------------------------------------------------------------
//
void AcEmp()
{
	WFX_EmpBlast( MyShip );
}


// set id of ship object that is first in list to specific value --------------
//
PRIVATE
void AcSetObjectId()
{
	int32 objid;
	if ( AcFetchIntParams( 1, &objid ) ) {
		if ( delayed_object_creation ) {
			delayed_object_id = objid;
		} else {
			ShipObject *scanpo = FetchFirstShip();
			// ship must have been created previously
			ASSERT( scanpo != NULL );
			if ( scanpo != NULL ) {
				scanpo->ObjectNumber = objid;
			}
		}
	}
}


// set host id of object to specific value to enable targeting for missiles ---
//
PRIVATE
void AcSetObjectHostId()
{
	int32 objid;
	if ( AcFetchIntParams( 1, &objid ) ) {
		if ( delayed_object_creation ) {
			delayed_object_hostid = objid;
		} else {
			ShipObject *scanpo = FetchFirstShip();
			// ship must have been created previously
			ASSERT( scanpo != NULL );
			if ( scanpo != NULL ) {
				scanpo->HostObjNumber = objid;
			}
		}
	}
}


// simulate depression of single key ------------------------------------------
//
PRIVATE
void AcSimulateKeyPress()
{	
	int32 keynum;
	if ( AcFetchIntParams( 1, &keynum ) ) {
		if ( (dword)keynum < NUM_GAMEFUNC_KEYS ) {

			ASSERT( sizeof( DepressedKeys->key_Escape ) == sizeof( dword ) );
			((dword*)DepressedKeys)[ keynum ]  = 1;
			((dword*)key_replay_map)[ keynum ] = 1;

			// explicitly check for object camera toggling
			if ( offsetof( keyfunc_s, key_ToggleObjCamera ) / sizeof( DepressedKeys->key_Escape ) == keynum ) {
				ReplayObjCamActive = !ReplayObjCamActive;
			}

		} else {
			CON_AddLine( invalid_key_id );
		}
	}
}


// simulate release of single key ---------------------------------------------
//
PRIVATE
void AcSimulateKeyRelease()
{
	int32 keynum;
	if ( AcFetchIntParams( 1, &keynum ) ) {
		if ( (dword)keynum < NUM_GAMEFUNC_KEYS ) {
			ASSERT( sizeof( DepressedKeys->key_Escape ) == sizeof( dword ) );
			((dword*)DepressedKeys)[ keynum ]  = 0;
			((dword*)key_replay_map)[ keynum ] = 0;
		} else {
			CON_AddLine( invalid_key_id );
		}
	}
}


// ----------------------------------------------------------------------------
// local ship configuration commands                                          -
// ----------------------------------------------------------------------------


// current damage -------------------------------------------------------------
//
PRIVATE
void AcMyShip_CurDamage()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		MyShip->CurDamage = ivalue;
	}
}


// maximum damage -------------------------------------------------------------
//
PRIVATE
void AcMyShip_MaxDamage()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		MyShip->MaxDamage = ivalue;
	}
}


// current energy -------------------------------------------------------------
//
PRIVATE
void AcMyShip_CurEnergy()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		MyShip->CurEnergy = ivalue;
	}
}


// maximum energy -------------------------------------------------------------
//
PRIVATE
void AcMyShip_MaxEnergy()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		MyShip->MaxEnergy = ivalue;
	}
}


// current speed --------------------------------------------------------------
//
PRIVATE
void AcMyShip_CurSpeed()
{
	fixed_t ivalue;
	if ( AcFetchIntParams( 1, (int32 *) &ivalue ) ) {
		MyShip->CurSpeed = ivalue;
	}
}


// maximum speed --------------------------------------------------------------
//
PRIVATE
void AcMyShip_MaxSpeed()
{
	fixed_t ivalue;
	if ( AcFetchIntParams( 1, (int32 *) &ivalue ) ) {
		MyShip->MaxSpeed = ivalue;
	}
}


// ----------------------------------------------------------------------------
//
PRIVATE
void AcMyShip_NumMissls()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		MyShip->NumMissls = ivalue;
	}
}


// ----------------------------------------------------------------------------
//
PRIVATE
void AcMyShip_MaxNumMissls()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		MyShip->MaxNumMissls = ivalue;
	}
}


// ----------------------------------------------------------------------------
//
PRIVATE
void AcMyShip_NumHomMissls()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		MyShip->NumHomMissls = ivalue;
	}
}


// ----------------------------------------------------------------------------
//
PRIVATE
void AcMyShip_MaxNumHomMissls()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		MyShip->MaxNumHomMissls = ivalue;
	}
}


// ----------------------------------------------------------------------------
//
PRIVATE
void AcMyShip_NumPartMissls()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		MyShip->NumPartMissls = ivalue;
	}
}


// ----------------------------------------------------------------------------
//
PRIVATE
void AcMyShip_MaxNumPartMissls()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		MyShip->MaxNumPartMissls = ivalue;
	}
}


// ----------------------------------------------------------------------------
//
PRIVATE
void AcMyShip_NumMines()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		MyShip->NumMines = ivalue;
	}
}


// ----------------------------------------------------------------------------
//
PRIVATE
void AcMyShip_MaxNumMines()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		MyShip->MaxNumMines = ivalue;
	}
}


// ----------------------------------------------------------------------------
//
PRIVATE
void AcMyShip_Weapons()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		MyShip->Weapons = ivalue;
	}
}


// ----------------------------------------------------------------------------
//
PRIVATE
void AcMyShip_WeaponsActive()
{
	//NOTE:
	// if new duration weapons are added this function
	// should be updated.

	int32 weaponsactive;
	if ( AcFetchIntParams( 1, &weaponsactive ) ) {

		// check for lightning state change
		dword oldstate = MyShip->WeaponsActive & WPMASK_CANNON_LIGHTNING;
		dword newstate = weaponsactive & WPMASK_CANNON_LIGHTNING;
		if ( ( oldstate == 0 ) && ( newstate != 0 ) ) {
			WFX_ActivateLightning( MyShip );
		} else if ( ( oldstate != 0 ) && ( newstate == 0 ) ) {
			WFX_DeactivateLightning( MyShip );
		}

		// check for helix state change
		oldstate = MyShip->WeaponsActive & WPMASK_CANNON_HELIX;
		newstate = weaponsactive & WPMASK_CANNON_HELIX;
		if ( ( oldstate == 0 ) && ( newstate != 0 ) ) {
			WFX_ActivateHelix( MyShip );
		} else if ( ( oldstate != 0 ) && ( newstate == 0 ) ) {
			WFX_DeactivateHelix( MyShip );
		}

		// check for photon state change
		oldstate = MyShip->WeaponsActive & WPMASK_CANNON_PHOTON;
		newstate = weaponsactive & WPMASK_CANNON_PHOTON;
		if ( ( oldstate == 0 ) && ( newstate != 0 ) ) {
			WFX_ActivatePhoton( MyShip );
		} else if ( ( oldstate != 0 ) && ( newstate == 0 ) ) {
			WFX_DeactivatePhoton( MyShip );
		}

		// check for emp state change
		oldstate = MyShip->WeaponsActive & WPMASK_DEVICE_EMP;
		newstate = weaponsactive & WPMASK_DEVICE_EMP;
		if ( ( oldstate == 0 ) && ( newstate != 0 ) ) {
			WFX_ActivateEmp( MyShip );
		} else if ( ( oldstate != 0 ) && ( newstate == 0 ) ) {
			WFX_DeactivateEmp( MyShip );
		}

		// fallback for unknown duration weapons
		// (simply set/reset activation bits)
		dword unknownmask = 0xffffffff;
		unknownmask &= ~WPMASK_CANNON_LIGHTNING;
		unknownmask &= ~WPMASK_CANNON_HELIX;
		unknownmask &= ~WPMASK_CANNON_PHOTON;
		unknownmask &= ~WPMASK_DEVICE_EMP;
		MyShip->WeaponsActive &= ~unknownmask;
		MyShip->WeaponsActive |= weaponsactive & unknownmask;
	}
}


// ----------------------------------------------------------------------------
//
PRIVATE
void AcMyShip_Specials()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		MyShip->Specials = ivalue;
	}
}


// global variables configuration commands pertaining to local ship -----------
//
PRIVATE
void AcGlobals_SelectedLaser()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		SelectedLaser = ivalue;
	}
}

PRIVATE
void AcGlobals_SelectedMissile()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		SelectedMissile = ivalue;
	}
}

PRIVATE
void AcGlobals_CurGun()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		CurGun = ivalue;
	}
}

PRIVATE
void AcGlobals_CurLauncher()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		CurLauncher = ivalue;
	}
}

PRIVATE
void AcGlobals_AUX_HUD_PANEL_3_CONTROL()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		AUX_HUD_PANEL_3_CONTROL = ivalue;
	}
}

PRIVATE
void AcGlobals_AUX_HUD_PANEL_4_CONTROL()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		AUX_HUD_PANEL_4_CONTROL = ivalue;
	}
}

PRIVATE
void AcGlobals_TargetObjNumber()
{
	int32 ivalue;
	if ( AcFetchIntParams( 1, &ivalue ) ) {
		TargetObjNumber = ivalue;
	}
}


// play specified sound effect ------------------------------------------------
//
PRIVATE
void AcPlaySound()
{
	// dummy for compatibility
}


// flag whether audio stream has been started with an action command ----------
//
PUBLIC int con_audio_stream_started = FALSE;


// start playing of specific audio stream -------------------------------------
//
PRIVATE
void AcPlayStream()
{
	char *streamname;
	if ( AcFetchStringParam( &streamname, ACM_EAT_SPACE( ACM_PLAYSTREAM ) ) ) {

		if ( strlen( streamname ) + strlen( console_audio_stream_path ) > PATH_MAX ) {
			// resulting file name too long
			CON_AddLine( stream_name_inval );
			return;
		}

		//NOTE:
		// apart from its length the filename is
		// not checked for validity in any way!

		// create full filename including path
		char fname[ PATH_MAX + 1 ];
		strcpy( fname, console_audio_stream_path );
		strcat( fname, streamname );

		// check first if file exists in pack or current directory
		FILE *fp = SYS_fopen( streamname, "rb" );
		if ( fp == NULL ) {

			// check if file exists in audio stream path
			fp = SYS_fopen( fname, "rb" );
			if ( fp == NULL ) {
				CON_AddLine( stream_name_inval );
				return;
			}

			//NOTE:
			// we use SYS_fopen() here, because the streaming code
			// also uses the SYS_f*() functions.

		} else {

			// strip path
			strcpy( fname, streamname );
		}
		SYS_fclose( fp );

		// play audio stream (automatically determines stream's format)
		int started = AUDs_PlayAudioStream( fname );

		if ( started ) {
			con_audio_stream_started = TRUE;
		} else {
			CON_AddLine( no_stream_started );
		}
	}
}


// stop playing of currently active audio stream ------------------------------
//
PRIVATE
void AcStopStream()
{
	int stopped = AUDs_StopAudioStream();

	if ( stopped ) {
		con_audio_stream_started = FALSE;
	} else {
		CON_AddLine( no_stream_stopped );
	}
}


// set aux flag with ac. command ----------------------------------------------
//
PRIVATE
void AcSetAuxFlag()
{
	int32 params[ 2 ];
	if ( AcFetchIntParams( 2, params ) ) {

		if ( (dword)params[ 0 ] < MAX_AUX_ENABLING ) {
			AuxEnabling[ params[ 0 ] ] = params[ 1 ];
		}
	}
}


// set aux data with ac. command ----------------------------------------------
//
PRIVATE
void AcSetAuxData()
{
	int32 params[ 2 ];
	if ( AcFetchIntParams( 2, params ) ) {

		if ( (dword)params[ 0 ] < MAX_AUX_DATA ) {
			AuxData[ params[ 0 ] ] = params[ 1 ];
		}
	}
}


// set new refframe frequency -------------------------------------------------
//
PRIVATE
void AcSetRefFrameFrequency()
{
	int32 param;
	if ( AcFetchIntParams( 1, &param ) ) {

		if ( RefFrameFrequency != param ) {

			RefFrameFrequency = param;

			// reinit frame timer to apply frequency change
			SYSs_InitFrameTimer();
		}
	}
}


// alter entry-mode flag ------------------------------------------------------
//
PRIVATE
void AcSetEntryMode()
{
	int32 param;
	if ( AcFetchIntParams( 1, &param ) ) {

		EntryMode		= param;
		InFloatingMenu	= EntryMode ? FloatingMenu : FALSE;

		// reset the floating menu state
		if ( InFloatingMenu ) {
			ActivateFloatingMenu( FALSE );
		}
	}
}


// this gets called on execution instead of compilation, which is a nop -------
//
PRIVATE
void AcIncludeDemo()
{
	//NOTE:
	// this command is a nop in a script file and should never
	// occur in a compiled demo, since it will be substituted
	// by the specified demo on compilation.

	CON_AddLine( only_for_compile );
}


// this gets called on execution instead of compilation, which is a nop -------
//
PRIVATE
void AcScheduleCommand()
{
	//NOTE:
	// this command is a nop in a script file.
	// in a compiled demo this function will not be called at all.

	CON_AddLine( only_for_compile );
}


// set the ambient light color ------------------------------------------------
//
PRIVATE
void AcLightAmbient()
{
	int32 params[ 4 ];
	if ( AcFetchIntParams( 4, params ) ) {
		for (int i = 0; i < 4; i++)
			LightColorAmbient.index[i] = params[i];
	}
}


// set the diffuse light color ------------------------------------------------
//
PRIVATE
void AcLightDiffuse()
{
	int32 params[ 4 ];
	if ( AcFetchIntParams( 4, params ) ) {
		for (int i = 0; i < 4; i++)
			LightColorDiffuse.index[i] = params[i];
	}
}


// set the specular light color -----------------------------------------------
//
PRIVATE
void AcLightSpecular()
{
	int32 params[ 4 ];
	if ( AcFetchIntParams( 4, params ) ) {
		for (int i = 0; i < 4; i++)
			LightColorSpecular.index[i] = params[i];
	}
}


// cd audio: open drive -------------------------------------------------------
//
PRIVATE
void AcCD_Open()
{
	int32 param;
	if ( AcFetchIntParams( 1, &param ) ) {

		//AUDs_CDOpen( param );
	}
}


// cd audio: close drive ------------------------------------------------------
//
PRIVATE
void AcCD_Close()
{
	//AUDs_CDClose();
}


// cd audio: play audio track -------------------------------------------------
//
PRIVATE
void AcCD_Play()
{
	//NOTE:
	// the from and to params (param 1 and 2) cannot be omitted
	// when using this command. however, they can be set to -1
	// in order to avoid specifying specific from/to locations.

	int32 params[ 3 ];
	if ( AcFetchIntParams( 3, params ) ) {

		//AUDs_CDPlay( params[ 0 ], params[ 1 ], params[ 2 ] );
	}
}


// cd audio: pause playing ----------------------------------------------------
//
PRIVATE
void AcCD_Pause()
{
	//AUDs_CDPause();
}


// cd audio: resume playing ---------------------------------------------------
//
PRIVATE
void AcCD_Resume()
{
	//AUDs_CDResume();
}


// cd audio: stop playing -----------------------------------------------------
//
PRIVATE
void AcCD_Stop()
{
	//AUDs_CDStop();
}


// cd audio: set volume -------------------------------------------------------
//
PRIVATE
void AcCD_Volume()
{
	int32 param;
	if ( AcFetchIntParams( 1, &param ) ) {

		//AUDs_CDSetVolume( param );
	}
}


// list of valid action commands and functions to be called for each ----------
//
act_command_s action_commands[] = {

	// general signature
	{ "ac.",                    NULL,                               ACF_NODIRECT | ACF_NOCOMPILE | ACF_NOPARAMS,	0	},

	{ "ac.wait",                AcIdleCommand,						ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.pitch",               AcAutoPitch,						ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.yaw",                 AcAutoYaw,							ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.roll",                AcAutoRoll,							ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.slidex",              AcAutoSlideX,						ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	1	},
	{ "ac.slidey",              AcAutoSlideY,						ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	1	},
	{ "ac.move",                AcAutoMove,							ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.camrot1",             AcSetCamRotCol1,					ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	3	},
	{ "ac.camrot2",             AcSetCamRotCol2,					ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	3	},
	{ "ac.camrot3",             AcSetCamRotCol3,					ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	3	},
	{ "ac.camorigin",           AcSetCamOrigin,						ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	3	},

	{ "ac.fixrot1",             AcSetFixedStarRotCol1,				ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	3	},
	{ "ac.fixrot2",             AcSetFixedStarRotCol2,				ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	3	},
	{ "ac.fixrot3",             AcSetFixedStarRotCol3,				ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	3	},

	{ "ac.packet",              AcPlayRemotePacket,                 ACF_NODIRECT | ACF_COMPILE   | ACF_PACKETPARAM,	1	},

	{ "ac.fadeout",             AcFadeOut,                          ACF_DIRECT   | ACF_COMPILE   | ACF_INTPARAMS,	1   },
	{ "ac.fadein",              AcFadeIn,                           ACF_DIRECT   | ACF_COMPILE   | ACF_INTPARAMS,	1   },
	{ "ac.wfadeout",            AcWhiteFadeOut,                     ACF_DIRECT   | ACF_COMPILE   | ACF_INTPARAMS,	1   },
	{ "ac.wfadein",             AcWhiteFadeIn,                      ACF_DIRECT   | ACF_COMPILE   | ACF_INTPARAMS,	1   },
	{ "ac.black",               AcBlackScreen,                      ACF_DIRECT   | ACF_COMPILE   | ACF_NOPARAMS,	0   },
	{ "ac.white",               AcWhiteScreen,                      ACF_DIRECT   | ACF_COMPILE   | ACF_NOPARAMS,	0   },
	{ "ac.stdpalette",          AcStandardPalette,                  ACF_DIRECT   | ACF_COMPILE   | ACF_NOPARAMS,	0   },

	{ "ac.savescreen",          AcSaveDataInfo,                     ACF_DIRECT   | ACF_NOCOMPILE | ACF_NOPARAMS,	0	},
	{ "ac.savelist",            AcSaveObjectList,                   ACF_DIRECT   | ACF_NOCOMPILE | ACF_NOPARAMS,	0	},
	{ "ac.savecam",             AcSaveCamera,                       ACF_DIRECT   | ACF_NOCOMPILE | ACF_NOPARAMS,	0	},
	{ "ac.savefstars",          AcSaveFixedStars,                   ACF_DIRECT   | ACF_NOCOMPILE | ACF_NOPARAMS,	0	},
	{ "ac.savepstars",          AcSavePseudoStars,                  ACF_DIRECT   | ACF_NOCOMPILE | ACF_NOPARAMS,	0	},
	{ "ac.saveremote",			AcSaveRemoteState,					ACF_DIRECT   | ACF_NOCOMPILE | ACF_NOPARAMS,	0	},

	{ "ac.killpstars",          AcKillPseudoStars,                  ACF_NODIRECT | ACF_COMPILE   | ACF_NOPARAMS,	0	},
	{ "ac.addpstar",            AcAddPseudoStar,                    ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	3	},

	{ "ac.preprestore",         AcPrepareDataRestore,               ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.creatobj",            AcCreateObject,						ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.creatextra",          AcCreateExtra,                      ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.destlist",            AcDestroyObjectList,                ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.remoteplayer",		AcRemotePlayer,						ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	4	},
	{ "ac.remotematrix",		AcRemoteMatrix,						ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	12	},
	{ "ac.remotename",			AcRemoteName,						ACF_NODIRECT | ACF_COMPILE   | ACF_STRINGPARAM,	1	},
	{ "ac.remotejoin",			AcRemoteJoin,						ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	3	},
	{ "ac.remoteunjoin",		AcRemoteUnjoin,						ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	3	},

	{ "ac.cleardemo",			AcClearDemo,						ACF_DIRECT   | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.objmatrxcol1",        AcObjectMatrxColumn1,               ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	4	},
	{ "ac.objmatrxcol2",        AcObjectMatrxColumn2,               ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	4	},
	{ "ac.objmatrxcol3",        AcObjectMatrxColumn3,               ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	4	},
	{ "ac.objmatrxcol4",        AcObjectMatrxColumn4,               ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	4	},

	{ "ac.laserspeed",          AcSetLaserSpeed,                    ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.laserdirvec",         AcSetLaserDirVec,                   ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	3	},
	{ "ac.laserowner",          AcSetLaserOwner,                    ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.misslspeed",          AcSetMisslSpeed,                    ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.missldirvec",         AcSetMisslDirVec,                   ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	3	},

	{ "ac.misslowner",          AcSetMisslOwner,                    ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.missltarget",         AcSetMisslTarget,                   ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.mineowner",           AcSetMineOwner,                     ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.energyfield",         AcEnergyField,                      ACF_NODIRECT | ACF_COMPILE   | ACF_FLOATPARAMS,	3	},
	{ "ac.spreadfire",          AcSpreadfire,                       ACF_NODIRECT | ACF_COMPILE   | ACF_NOPARAMS,	0	},
	{ "ac.lightningon",         AcLightningOn,                      ACF_NODIRECT | ACF_COMPILE   | ACF_NOPARAMS,	0	},
	{ "ac.lightningoff",        AcLightningOff,                     ACF_NODIRECT | ACF_COMPILE   | ACF_NOPARAMS,	0	},

	{ "ac.setobjid",            AcSetObjectId,                      ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.setobjhostid",        AcSetObjectHostId,                  ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.keypress",            AcSimulateKeyPress,                 ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.keyrelease",          AcSimulateKeyRelease,               ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.ms.curdamage",        AcMyShip_CurDamage,                 ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.ms.maxdamage",        AcMyShip_MaxDamage,                 ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.ms.curenergy",        AcMyShip_CurEnergy,                 ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.ms.maxenergy",        AcMyShip_MaxEnergy,                 ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.ms.curspeed",         AcMyShip_CurSpeed,                  ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.ms.maxspeed",         AcMyShip_MaxSpeed,                  ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.ms.nummissls",        AcMyShip_NumMissls,                 ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.ms.maxnummissls",     AcMyShip_MaxNumMissls,              ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.ms.numhommissls",     AcMyShip_NumHomMissls,              ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.ms.maxnumhommissls",  AcMyShip_MaxNumHomMissls,           ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.ms.nummines",         AcMyShip_NumMines,                  ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.ms.maxnummines",      AcMyShip_MaxNumMines,               ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.ms.weapons",          AcMyShip_Weapons,                   ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.ms.specials",         AcMyShip_Specials,                  ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.gl.selectedlaser",    AcGlobals_SelectedLaser,            ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.gl.selectedmissile",  AcGlobals_SelectedMissile,          ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.gl.curgun",           AcGlobals_CurGun,                   ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.gl.curlauncher",      AcGlobals_CurLauncher,              ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.gl.panel3control",    AcGlobals_AUX_HUD_PANEL_3_CONTROL,  ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.gl.panel4control",    AcGlobals_AUX_HUD_PANEL_4_CONTROL,  ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.gl.targetobjnumber",  AcGlobals_TargetObjNumber,          ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.writesmall",          AcWriteSmall,                       ACF_DIRECT   | ACF_COMPILE   | ACF_STRINGPARAM,	1	},
	{ "ac.writebig",            AcWriteBig,                         ACF_DIRECT   | ACF_COMPILE   | ACF_STRINGPARAM,	1	},
	{ "ac.writebig2",           AcWriteBig2,                        ACF_DIRECT   | ACF_COMPILE   | ACF_STRINGPARAM,	1	},
	{ "ac.writebig3",           AcWriteBig3,                        ACF_DIRECT   | ACF_COMPILE   | ACF_STRINGPARAM,	1	},
	{ "ac.cleartext",           AcClearText,                        ACF_DIRECT   | ACF_COMPILE   | ACF_NOPARAMS,	0	},

	{ "ac.playstream",          AcPlayStream,                       ACF_DIRECT   | ACF_COMPILE   | ACF_STRINGSTRIP,	1	},
	{ "ac.stopstream",          AcStopStream,                       ACF_DIRECT   | ACF_COMPILE   | ACF_NOPARAMS,	0	},
	{ "ac.playsound",			AcPlaySound,						ACF_DIRECT   | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.aux",					AcSetAuxFlag,						ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	2	},
	{ "ac.entrymode",			AcSetEntryMode,						ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.servermsg",           AcPlayServerMessage,                ACF_NODIRECT | ACF_COMPILE   | ACF_STRINGSTRIP,	1	},

	{ "ac.helixon",				AcHelixOn,							ACF_NODIRECT | ACF_COMPILE   | ACF_NOPARAMS,	0	},
	{ "ac.helixoff",			AcHelixOff,							ACF_NODIRECT | ACF_COMPILE   | ACF_NOPARAMS,	0	},

	{ "ac.remotemaxplayers",	AcRemoteMaxPlayers,					ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.include",				AcIncludeDemo,						ACF_NODIRECT | ACF_COMPILE   | ACF_DEMOPARAM,	1	},

	{ "ac.ms.weaponsactive",	AcMyShip_WeaponsActive,				ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.remoteweaponsactive",	AcRemoteWeaponsActive,				ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.write",				AcWriteConsoleText,					ACF_NODIRECT | ACF_COMPILE   | ACF_STRINGPARAM,	1	},
	{ "ac.display",				AcDisplayMessage,					ACF_NODIRECT | ACF_COMPILE   | ACF_STRINGSTRIP,	1	},

	{ "ac.cd.open",				AcCD_Open,							ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.cd.close",			AcCD_Close,							ACF_NODIRECT | ACF_COMPILE   | ACF_NOPARAMS,	0	},
	{ "ac.cd.play",				AcCD_Play,							ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	3	},
	{ "ac.cd.pause",			AcCD_Pause,							ACF_NODIRECT | ACF_COMPILE   | ACF_NOPARAMS,	0	},
	{ "ac.cd.resume",			AcCD_Resume,						ACF_NODIRECT | ACF_COMPILE   | ACF_NOPARAMS,	0	},
	{ "ac.cd.stop",				AcCD_Stop,							ACF_NODIRECT | ACF_COMPILE   | ACF_NOPARAMS,	0	},
	{ "ac.cd.volume",			AcCD_Volume,						ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.command",				AcExecCommandString,				ACF_NODIRECT | ACF_COMPILE   | ACF_STRINGPARAM,	1	},

	{ "ac.classmapid",			AcClassMapId,						ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.classmapname",		AcClassMapName,						ACF_NODIRECT | ACF_COMPILE   | ACF_STRINGPARAM,	1	},

	{ "ac.auxdata",				AcSetAuxData,						ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	2	},
	{ "ac.refframes",			AcSetRefFrameFrequency,				ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.schedule",			AcScheduleCommand,					ACF_NODIRECT | ACF_COMPILE   | ACF_SCHEDPARAM,	1	},

	{ "ac.ambient",				AcLightAmbient,						ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	4	},
	{ "ac.diffuse",				AcLightDiffuse,						ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	4	},
	{ "ac.specular",			AcLightSpecular,					ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	4	},

	{ "ac.ms.numpartmissls",	AcMyShip_NumPartMissls,				ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},
	{ "ac.ms.maxnumpartmissls",	AcMyShip_MaxNumPartMissls,			ACF_NODIRECT | ACF_COMPILE   | ACF_INTPARAMS,	1	},

	{ "ac.photonon",			AcPhotonOn,							ACF_NODIRECT | ACF_COMPILE   | ACF_NOPARAMS,	0	},
	{ "ac.photonoff",			AcPhotonOff,						ACF_NODIRECT | ACF_COMPILE   | ACF_NOPARAMS,	0	},

	{ "ac.empon",				AcEmpOn,							ACF_NODIRECT | ACF_COMPILE   | ACF_NOPARAMS,	0	},
	{ "ac.empoff",				AcEmpOff,							ACF_NODIRECT | ACF_COMPILE   | ACF_NOPARAMS,	0	},
	{ "ac.emp",					AcEmp,								ACF_NODIRECT | ACF_COMPILE   | ACF_NOPARAMS,	0	},

	{ NULL, 					NULL,								ACF_NODIRECT | ACF_NOCOMPILE | ACF_NOPARAMS,	0	},

};


// determine the number of parameter bytes of an action command ---------------
//
size_t GetActionParamSize( int actid, char *params )
{
	ASSERT( actid > ACM_SIGNATURE );
	ASSERT( params != NULL );

	size_t paramsize = 0;

	if ( action_commands[ actid ].flags & ACF_INTPARAMS ) {
		paramsize = sizeof( int32 ) * action_commands[ actid ].numparameters;
	} else if ( action_commands[ actid ].flags & ACF_FLOATPARAMS ) {
		paramsize = sizeof( hprec_t ) * action_commands[ actid ].numparameters;
	} else if ( action_commands[ actid ].flags & ACF_STRINGPARAM ) {
		paramsize = strlen( params ) + 1;
	}

	return paramsize;
}


// scheduled action commands --------------------------------------------------
//
struct scheduled_command_s {

	refframe_t	timestamp;
	char*		command;
};

#define MAX_SCHEDULED_COMMANDS	64
static int						num_scheduled_commands = 0;
static scheduled_command_s		scheduled_commands[ MAX_SCHEDULED_COMMANDS ];


// remainder of a wait interval split by a scheduled command ------------------
//
static int scheduled_wait	= 0;


// clear all scheduled action commands ----------------------------------------
//
void ResetScheduledActions()
{
	num_scheduled_commands	= 0;
	scheduled_wait			= 0;
}


// schedule an action command for automatic execution at a later time ---------
//
PRIVATE
size_t ScheduleCommand( int *params )
{
	ASSERT( params != NULL );

	// fetch delta time and command to schedule
	int timedelta = SWAP_32( params[ 0 ] );
	int actid     = SWAP_32( params[ 1 ] );

	// calc absolute activation timestamp
	refframe_t timestamp = demoinfo_curtime + timedelta;
	int cid=0;
	// insert at correct position if already commands scheduled
	for ( cid = 0; cid < num_scheduled_commands; cid++ ) {

		if ( scheduled_commands[ cid ].timestamp >= timestamp ) {

			// this also ensures the move works correctly
			if ( num_scheduled_commands < MAX_SCHEDULED_COMMANDS ) {
				num_scheduled_commands++;
			}

			// move back commands scheduled for a later time
			if ( cid < MAX_SCHEDULED_COMMANDS - 1 ) {
				size_t movesize = ( num_scheduled_commands - cid - 1 );
				movesize *= sizeof( scheduled_command_s );
				memmove( &scheduled_commands[ cid + 1 ], &scheduled_commands[ cid ], movesize );
			}

			// store this command
			scheduled_commands[ cid ].timestamp	= timestamp;
			scheduled_commands[ cid ].command	= (char *) &params[ 1 ];
			break;
		}
	}

	// insert if no commands scheduled or position should be last
	if ( cid == num_scheduled_commands ) {

		// commands that exceed the available space will be dropped silently
		if ( cid < MAX_SCHEDULED_COMMANDS ) {
			scheduled_commands[ cid ].timestamp	= timestamp;
			scheduled_commands[ cid ].command	= (char *) &params[ 1 ];
			num_scheduled_commands++;
		}
	}

	// return number of parameter bytes to skip
	size_t paramsize = sizeof( int ) * 2;
	paramsize += GetActionParamSize( actid, (char *) &params[ 2 ] );
	return paramsize;
}


// execute command in binary format as written by WriteActionCommand() --------
//
int ExecBinCommands( char* &data, int interactive )
{
	// non-interactive console mode
	con_non_interactive = !interactive;

	// to remember the current replay position when
	// it is temporarily set to a scheduled command
	char *curpos = NULL;

restartwait:

	// execute commands until wait encountered
	for (;;) {

		// restore position after scheduled command
		if ( curpos != NULL ) {
			data   = curpos;
			curpos = NULL;
		}

		// check for execution of scheduled command
		if ( num_scheduled_commands > 0 ) {

			if ( scheduled_commands[ 0 ].timestamp == demoinfo_curtime ) {

				// temporarily jump to position of scheduled command
				curpos = data;
				data   = scheduled_commands[ 0 ].command;

				// remove this command from scheduled commands
				if ( num_scheduled_commands > 1 ) {
					size_t movesize = ( num_scheduled_commands - 1 );
					movesize *= sizeof( scheduled_command_s );
					memmove( &scheduled_commands[ 0 ], &scheduled_commands[ 1 ], movesize );
				}
				num_scheduled_commands--;
			}
		}

		// for regular (non-scheduled) commands
		if ( curpos == NULL ) {

			// exit loop on implied wait command
			if ( scheduled_wait > 0 ) {
				break;
			}

			// count executed commands
			demoinfo_curline++;
		}

		// fetch command id
		int actid = SWAP_32( *(int *)data );
		ASSERT( ( actid >= ACM_SIGNATURE ) && ( actid < ACM_NUM_COMMANDS ) );

		// stop execution if end of command file reached
		if ( actid == ACM_SIGNATURE ) {
			bin_params			= NULL;
			con_non_interactive	= FALSE;
			return FALSE;
		}

		data += sizeof( actid );
		bin_params = data;

		// exit loop on wait command
		if ( actid == ACM_IDLEWAIT ) {
			break;
		}

		// check for packet replay
		if ( action_commands[ actid ].flags & ACF_PACKETPARAM ) {
	
			// replay packet and get pointer advance 
			size_t advance = REC_PlayRemotePacketBin( (NetPacketExternal *) bin_params );

			// check for invalid pointer advance ( -1 ... invalid packet )
			if ( advance == (size_t)-1 ) {
				bin_params			= NULL;
				con_non_interactive	= FALSE;
				return FALSE;
			}
			
			data += advance;
			continue;
		}

		// check for command scheduling
		if ( action_commands[ actid ].flags & ACF_SCHEDPARAM ) {
			// schedule command and advance pointer
			data += ScheduleCommand( (int *) bin_params );
			continue;
		}

		// determine how many parameter bytes need to be skipped.
		// we do this before calling the action function because
		// it might destroy its string parameter while parsing.
		size_t paramsize = GetActionParamSize( actid, bin_params );

		// call corresponding action function
		// (it knows the type of its parameters)
		ASSERT( action_commands[ actid ].func != NULL );
		(*action_commands[ actid ].func)();

		// skip command's parameters (already read by action function)
		data += paramsize;
	}

	// wait command must not be scheduled
	ASSERT( curpos == NULL );

	// determine length of next wait interval
	int waitcount;
	if ( scheduled_wait > 0 ) {

		// completion of wait interval split by scheduled command
		waitcount      = scheduled_wait;
		scheduled_wait = 0;

	} else {

		// preadvance over wait argument
		data += sizeof( int32 );

		// process wait
		int32 suppliedwait;
		if ( !AcFetchIntParams( 1, &suppliedwait ) ) {

			ASSERT( 0 );
			CON_AddLine( invalid_bin_wait );
			goto restartwait;
		}

		waitcount = suppliedwait;
	}

	// consume zero-waits transparently
	if ( waitcount == 0 ) {
		goto restartwait;
	}

	ASSERT( waitcount > 0 );
	if ( waitcount < 0 ) {
		CON_AddLine( invalid_bin_wait );
		goto restartwait;
	}

	// check for scheduled commands in the middle of the next wait interval
	if ( num_scheduled_commands > 0 ) {
		ASSERT( demoinfo_curtime < scheduled_commands[ 0 ].timestamp );
		if ( demoinfo_curtime + waitcount > scheduled_commands[ 0 ].timestamp ) {
			scheduled_wait  = waitcount;
			waitcount       = scheduled_commands[ 0 ].timestamp - demoinfo_curtime;
			scheduled_wait -= waitcount;
		}
	}

	// set duration to wait for
	CurActionWait = waitcount;

	// remember for info
	demoinfo_lastwait = (refframe_t) waitcount;

	// flag must be reset
	con_non_interactive = FALSE;

	bin_params = NULL;

	return TRUE;
}


// save game state to current recording ---------------------------------------
//
void SaveGameState()
{
	//NOTE:
	// this function is called by CON_EXT::StartRecording() to save
	// the game state before a recording is started.

	AcSaveDataInfo();
}


// include an entire demo file in the current compilation ---------------------
//
PRIVATE
int IncludeDemoFile( FILE *fp )
{
	ASSERT( fp != NULL );

	// read demo name
	char *demoname;
	if ( AcFetchStringParam( &demoname, TRUE ) ) {

		// open demo for inclusion
		FILE *cpfp = DEMO_BinaryOpenDemo( demoname );
		if ( cpfp == NULL ) {
			strcpy( paste_str, "demo file not found. (" );
			strcat( paste_str, demoname );
			strcat( paste_str, CON_FILE_COMPILED_EXTENSION );
			strcat( paste_str, ")" );
			CON_AddLine( paste_str );
			return TRUE;
		}

		// skip header if present
		DEMO_ParseDemoHeader( cpfp, -1, FALSE );

		// demo will be copied in chunks
		#define INCLUDE_BLOCK_SIZE	16384
		char *cpblock = (char *) ALLOCMEM( INCLUDE_BLOCK_SIZE );
		if ( cpblock == NULL )
			OUTOFMEM( 0 );

		// paste in entire demo in chunks
		size_t numread = 1;
		while ( numread > 0 ) {
			numread = fread( cpblock, 1, INCLUDE_BLOCK_SIZE, cpfp );
			if ( fwrite( cpblock, 1, numread, fp ) != numread ) {
				CON_AddLine( compile_write_err );
				FREEMEM( cpblock );
				return FALSE;
			}
		}

		FREEMEM( cpblock );
		fclose( cpfp );

		// strip off terminating eof marker
		fseek( fp, -sizeof( int ), SEEK_CUR );
	}

	return TRUE;
}


// write int parameters of action command in binary format to file ------------
//
PRIVATE
int WriteActionIntParams( FILE *fp, int paranum )
{
	ASSERT( fp != NULL );
	ASSERT( paranum > 0 );

	// write int parameters
	int32 *intparams			= (int32 *) ALLOCMEM( sizeof( int32 ) * paranum * 2 );
	int32 *intparams_swapped	= &intparams[ paranum ];
	if ( AcFetchIntParams( paranum, intparams ) ) {

		// must be written in little-endian format
		SwapParams_int( paranum, intparams, intparams_swapped );

		if ( fwrite( intparams_swapped, sizeof( int32 ), paranum, fp ) != (unsigned int)paranum ) {
			CON_AddLine( compile_write_err );
			FREEMEM( intparams );
			return FALSE;
		}

	} else {

		CON_AddLine( compile_arg_inval );
	}

	FREEMEM( intparams );

	return TRUE;
}


// write float parameters of action command in binary format to file ----------
//
PRIVATE
int WriteActionFloatParams( FILE *fp, int paranum )
{
	ASSERT( fp != NULL );
	ASSERT( paranum > 0 );

	// write float parameters
	hprec_t *floatparams		 = (hprec_t *) ALLOCMEM( sizeof( hprec_t ) * paranum * 2 );
	hprec_t *floatparams_swapped = &floatparams[ paranum ];
	if ( AcFetchFloatParams( paranum, floatparams ) ) {

		// must be written in little-endian format
		SwapParams_hprec( paranum, floatparams, floatparams_swapped );

		if ( fwrite( floatparams_swapped, sizeof( hprec_t ), paranum, fp ) != (unsigned int)paranum ) {
			CON_AddLine( compile_write_err );
			FREEMEM( floatparams );
			return FALSE;
		}

	} else {

		CON_AddLine( compile_arg_inval );
	}

	FREEMEM( floatparams );

	return TRUE;
}


// write string parameter of action command in binary format to file ----------
//
PRIVATE
int WriteActionStringParam( FILE *fp, dword flags )
{
	ASSERT( fp != NULL );

	// read string parameter
	char *strparam;
	if ( AcFetchStringParam( &strparam, ACF_EAT_SPACE( flags ) ) ) {

		unsigned int slen = strlen( strparam ) + 1;
		if ( fwrite( strparam, 1, slen, fp ) != slen ) {
			CON_AddLine( compile_write_err );
			return FALSE;
		}

	} else {

		CON_AddLine( compile_arg_inval );
	}

	return TRUE;
}


// write packet parameter of action command in binary format to file ----------
//
PRIVATE
int WriteActionPacketParam( FILE *fp )
{
	ASSERT( fp != NULL );

	int rc = TRUE;
	
	// read session and packet ids and fetch corresponding packet
	int32 params[ 2 ];
	if ( AcFetchIntParams( 2, params ) ) {

		// fetch packet
		NetPacketExternal* ext_gamepacket = (NetPacketExternal*) ALLOCMEM( RECORD_PACKET_SIZE );
		size_t psize = REC_FetchRemotePacket( ext_gamepacket, params[ 0 ], params[ 1 ] );

		// write packet if valid
		if ( psize > 0 ) {
			if ( fwrite( ext_gamepacket, 1, psize, fp ) != psize ) {
				CON_AddLine( compile_write_err );
				rc = FALSE;
			}
		} else {
			CON_AddLine( compile_pkt_inval );
			rc = FALSE;
		}

		FREEMEM( ext_gamepacket );

	} else {

		CON_AddLine( compile_arg_inval );
	}

	return rc;
}


// forward declaration --------------------------------------------------------
//
PRIVATE
void ExecActionCommand( char *cstr );


// write a scheduled command into the current compilation ---------------------
//
PRIVATE
int WriteScheduledCommand( FILE *fp )
{
	ASSERT( fp != NULL );

	// prevent recursive call (scheduling of schedule command)
	static int scheduled_command_writing_active = FALSE;
	if ( scheduled_command_writing_active ) {
		CON_AddLine( invalid_arg );
		return FALSE;
	}
	scheduled_command_writing_active = TRUE;

	// parse delta time parameter
	char *timeparam = strtok( NULL, " " );
	if ( timeparam == NULL ) {
		CON_AddLine( arg_missing );
		return FALSE;
	}

	char *errpart;
	int timedelta = strtol( timeparam, &errpart, int_calc_base );
	if ( *errpart != 0 ) {
		CON_AddLine( invalid_arg );
		return FALSE;
	}

	// write delta time in little-endian format
	timedelta = SWAP_32( timedelta );
	if ( fwrite( &timedelta, sizeof( timedelta ), 1, fp ) != 1 ) {
		CON_AddLine( compile_write_err );
		return FALSE;
	}

	// append scheduled command as parameter after scheduling prefix
	ExecActionCommand( NULL );

	scheduled_command_writing_active = FALSE;
	return TRUE;
}


// write action command in binary format to file ------------------------------
//
PRIVATE
int WriteActionCommand( FILE *fp, int actid )
{
	ASSERT( fp != NULL );
	ASSERT( actid > ACM_SIGNATURE );

	dword flags   = action_commands[ actid ].flags;
	int   paranum = action_commands[ actid ].numparameters;

	// exit if command may not be compiled
	if ( ( flags & ACF_COMPILE ) == 0 ) {
		CON_AddLine( compile_non_comp );
		return TRUE;
	}

	// check for demo inclusion
	// (command will be substituted)
	if ( flags & ACF_DEMOPARAM ) {
		return IncludeDemoFile( fp );
	}

	// must be written in little-endian format
	actid = SWAP_32( actid );

	// write id of command
	if ( fwrite( &actid, sizeof( actid ), 1, fp ) != 1 ) {
		CON_AddLine( compile_write_err );
		return FALSE;
	}

	if ( flags & ACF_INTPARAMS ) {

		// write int parameters to file
		if ( !WriteActionIntParams( fp, paranum ) ) {
			return FALSE;
		}

	} else if ( flags & ACF_FLOATPARAMS ) {

		// write float parameters to file
		if ( !WriteActionFloatParams( fp, paranum ) ) {
			return FALSE;
		}

	} else if ( flags & ACF_STRINGPARAM ) {

		// write string parameter to file
		ASSERT( paranum == 1 );
		if ( !WriteActionStringParam( fp, flags ) ) {
			return FALSE;
		}

	} else if ( flags & ACF_PACKETPARAM ) {

		// write packet parameter to file
		if ( !WriteActionPacketParam( fp ) ) {
			return FALSE;
		}

	} else if ( flags & ACF_SCHEDPARAM ) {

		// write scheduled command to file
		if ( !WriteScheduledCommand( fp ) ) {
			return FALSE;
		}
	}

	return TRUE;
}


// call action function corresponding to action command or compile it ---------
//
INLINE
void CallActionFunction( int actid )
{
	ASSERT( actid > ACM_SIGNATURE );

	if ( compile_script ) {

		// if fp is NULL write error occurred earlier on
		if ( compile_fp != NULL ) {

			// write command to file but do not execute it
			if ( !WriteActionCommand( compile_fp, actid ) ) {
				// write error cleanup
				fclose( compile_fp );
				compile_fp = NULL;
			}
		}

	} else {

		// call corresponding action function
		ASSERT( action_commands[ actid ].func != NULL );
		(*action_commands[ actid ].func)();
	}
}


// lookup action command and execute it ---------------------------------------
//
PRIVATE
void ExecActionCommand( char *cstr )
{
	// scan table
	char *aspec = strtok( cstr, " " );
	int aid = 0;
	for ( aid = ACM_SIGNATURE + 1; ACMSTR( aid ) != NULL; aid++ ) {

		if ( strcmp( ACMSTR( aid ), aspec ) == 0 ) {

			dword flags = action_commands[ aid ].flags;
			if ( ( action_commands[ aid ].func != NULL ) &&
				 ( processing_batch || ( flags & ACF_DIRECT ) ) ) {

				CallActionFunction( aid );

			} else {

				CON_AddLine( non_batch_use );
			}
			break;
		}
	}

	if ( ACMSTR( aid ) == NULL ) {

		// no action command with specified name found
		CON_AddLine( invalid_action );
	}
}


// exec action command (command prefixed with "ac.") --------------------------
//
void ActionCommand( char *cstr )
{
	ASSERT( cstr != NULL );

	// exit if action commands currently not allowed
	if ( preempt_action_commands ) {
		CON_AddLine( action_preempted );
		return;
	}

	char *bdot = cstr + ACMLEN( ACM_SIGNATURE );
	if ( *bdot != 0 ) {

		// look up command and execute it
		ExecActionCommand( cstr );

	} else {

		// no action specified after "ac."
		CON_AddLine( no_action_found );
	}
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( CON_ACT )
{
	// reset mapping table for class ids
	int oldflag = AUX_DISABLE_CLASS_MAP_TABLE_RESET;
	AUX_DISABLE_CLASS_MAP_TABLE_RESET = FALSE;
	ResetClassMapTable();
	AUX_DISABLE_CLASS_MAP_TABLE_RESET = oldflag;
}



