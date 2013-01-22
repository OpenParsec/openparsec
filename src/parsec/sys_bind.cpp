/*
 * PARSEC - Dynamic Function Binding
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:33 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1997-1999
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

// subsystem linkage info
#include "linkinfo.h"

// local module header
#include "sys_bind.h"


// subsystems to bind ---------------------------------------------------------
//
int	sys_BindType_PROTOCOL	= BT_PROTOCOL_DEFAULT;



// bind PROTOCOL subsystem ----------------------------------------------------
//
void SYS_Bind_PROTOCOL()
{
#ifdef DBIND_PROTOCOL

#ifdef NET_NULL

		#undef  PFX
		#define PFX( func, arg ) NETs_ ## func arg; net_subsys_jtab.func = NETs_ ## func

		#include "def_prot.h"
#else // !NET_NULL


#ifndef LINKED_PROTOCOL_PEERTOPEER
	if ( sys_BindType_PROTOCOL == BT_PROTOCOL_PEERTOPEER ) {
		sys_BindType_PROTOCOL = BT_PROTOCOL_DEFAULT;
	}
#endif // !LINKED_PROTOCOL_PEERTOPEER

#ifndef LINKED_PROTOCOL_GAMESERVER
	if ( sys_BindType_PROTOCOL == BT_PROTOCOL_GAMESERVER ) {
		sys_BindType_PROTOCOL = BT_PROTOCOL_DEFAULT;
	}
#endif // !LINKED_PROTOCOL_GAMESERVER

	if ( 0 ) {

#ifdef LINKED_PROTOCOL_PEERTOPEER

	} else if ( sys_BindType_PROTOCOL == BT_PROTOCOL_PEERTOPEER ) {

		#undef  PFX
		#define PFX( func, arg ) NETs_PEERTOPEER_ ## func arg; net_subsys_jtab.func = NETs_PEERTOPEER_ ## func

		#include "def_prot.h"

#endif // LINKED_PROTOCOL_PEERTOPEER


#ifdef LINKED_PROTOCOL_GAMESERVER

	} else if ( sys_BindType_PROTOCOL == BT_PROTOCOL_GAMESERVER ) {

		#undef  PFX
		#define PFX( func, arg ) NETs_GAMESERVER_ ## func arg; net_subsys_jtab.func = NETs_GAMESERVER_ ## func

		#include "def_prot.h"

#endif // LINKED_PROTOCOL_GAMESERVERs

#ifdef LINKED_PROTOCOL_DEMONULL

	} else if ( sys_BindType_PROTOCOL == BT_PROTOCOL_DEMONULL ) {

		#undef  PFX
		#define PFX( func, arg ) NETs_DEMONULL_ ## func arg; net_subsys_jtab.func = NETs_DEMONULL_ ## func

		#include "def_prot.h"

#endif // LINKED_PROTOCOL_DEMONULL

	} else {

		// happens only if not even default subsystem linked.
		PANIC( 0 );
	}

#endif // !NET_NULL

#endif // DBIND_PROTOCOL
}


// bind all dynamic subsystems functions --------------------------------------
//
void SYS_BindDynamicFunctions()
{
	SYS_Bind_PROTOCOL();
}



