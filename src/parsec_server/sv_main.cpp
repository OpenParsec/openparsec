/*
 * PARSEC - Main Server Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:43 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001-2003
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

// network subsystem & server headers 
#include "net_defs.h"
#include "e_defs.h"

// UNP header
#include "net_wrap.h"

// proprietary module headers
#include "e_gameserver.h"
#include "sys_util_sv.h"

#include "MasterServer.h"

// main entrypoint function for the gameserver --------------------------------
//
int main( int argc, char** argv )
{
	// install any signal handlers
	SYSs_InstallSignalHandlers();

	// get the server global
	TheServer = E_GameServer::GetGameServer();

	// parsec the commandline and check for proper usage
	if ( TheServer->ParseCommandLine( argc, argv ) == FALSE ) {

		// print the program usage
		TheServer->PrintUsage();

	} else {
		
		// if we get here, then things went ok, so
		// let's check if we are running as a Master Server
		// and overload ourselves to a MasterServer Object
		// if we are.
		if(TheServer->GetServerIsMaster()){
			TheMaster = (MasterServer *)TheServer;  //clever...
		}

		// initialize the server components
		if ( TheServer->Init() == FALSE ) {
			return 0;
		}
		
		// run the mainloop until server is stopped
		TheServer->MainLoop();

		// kill the server components
		TheServer->Kill();
	}

	return 0;
}

