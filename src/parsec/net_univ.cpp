/*
 * PARSEC - Global Universe Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:30 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   2001
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

// subsystem headers
#include "net_defs.h"

// network code config
#include "net_conf.h"

// subsystem linkage info
#include "linkinfo.h"



// proprietary module headers
#include "net_csdf.h"
#include "obj_cust.h"
#include "obj_ctrl.h"
#include "g_stgate.h"
#include "g_telep.h"

// local module header
#include "net_univ.h"

// ----------------------------------------------------------------------------
//
#ifdef LINKED_PROTOCOL_GAMESERVER
	#include "net_serv.h"
#endif


// ----------------------------------------------------------------------------
//
PUBLIC server_s	server_list[ MAX_SERVERS ];
PUBLIC int		num_servers_joined = 0;

// ----------------------------------------------------------------------------
//
PUBLIC link_s	link_list[ MAX_LINKLIST_SIZE ];
PUBLIC int		g_nNumLinks = 0;


// ----------------------------------------------------------------------------
//
PUBLIC mapobj_s* map_objs[ MAX_MAP_OBJECTS ];
PUBLIC int		 num_map_objs = 0;


// ----------------------------------------------------------------------------
//
int NET_GetGameServerList()
{

#ifdef LINKED_PROTOCOL_GAMESERVER

	if ( NET_ProtocolGMSV() ) {
		return NET_ServerList_Get( Masters[ 0 ] );
	}

#endif // LINKED_PROTOCOL_GAMESERVER

	return TRUE;
}


// try to find the Stargate object to a specific serverid ---------------------
//
Stargate* NET_FindStargate( word serverid )
{
	// try to find the stargate for the specific serverid
	GenObject* walkobjs = FetchFirstCustom();
	for ( ; walkobjs; walkobjs = walkobjs->NextObj ) {

		// get type id of the custom stargate
		static dword stargate_typeid = TYPE_ID_INVALID;

		if ( stargate_typeid == TYPE_ID_INVALID ) {
			stargate_typeid = OBJ_FetchCustomTypeId( "stargate" );
		}

		// we only want to walk stargates
		if ( walkobjs->ObjectType != stargate_typeid ) {
			continue;
		}

		Stargate* stargate = (Stargate*) walkobjs;
		if ( stargate->serverid == serverid ) {
			return stargate;
		}
	}

	return NULL;
}

// try to find the teleporter object a location ---------------------
//
Teleporter* NET_FindTeleporter( Vertex3 *origin)
{
	// try to find the stargate for the specific serverid
	GenObject* walkobjs = FetchFirstCustom();
	for ( ; walkobjs; walkobjs = walkobjs->NextObj ) {

		// get type id of the custom stargate
		static dword teleporter_typeid = TYPE_ID_INVALID;

		if ( teleporter_typeid == TYPE_ID_INVALID ) {
			teleporter_typeid = OBJ_FetchCustomTypeId( "stargate" );
		}

		// we only want to walk stargates
		if ( walkobjs->ObjectType != teleporter_typeid ) {
			continue;
		}

		Teleporter *teleporter= (Teleporter*) walkobjs;
		if ( (teleporter->start_vtxlist->X == origin->X) &&
			 (teleporter->start_vtxlist->Y == origin->Y) &&
			 (teleporter->start_vtxlist->Z == origin->Z)) {

			return teleporter;
		}
	}

	return NULL;
}


