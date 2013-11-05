/*
 * MasterServer.cpp
 *
 *  Created on: Jan 2, 2013
 *      Author: jasonw
 */
// C library
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

// C library
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h> 
#include <sys/timeb.h>

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
//FIXME: ????
#include "sys_refframe_sv.h"

// UNP header
#include "net_wrap.h"

// server defs
#include "e_defs.h"

// net game header
#include "net_game_sv.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "e_gameserver.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux_sv.h"
#include "con_com_sv.h"
#include "con_main_sv.h"
#include "e_colldet.h"
#include "g_extra.h"
#include "inp_main_sv.h"
#include "net_csdf.h"
#include "net_limits.h"
#include "net_udpdriver.h"
#include "net_util.h"
#include "net_packetdriver.h"
#include "obj_clas.h"
//#include "e_stats.h"
#include "g_main_sv.h"
#include "e_connmanager.h"
#include "e_packethandler.h"
#include "e_simulator.h"
#include "e_simnetinput.h"
#include "e_simnetoutput.h"
#include "sys_refframe_sv.h"
#include "sys_util_sv.h"

#include "gd_help.h"

#include "MasterServer.h"

MasterServer::MasterServer() {
	// TODO Auto-generated constructor stub
	last_check=0;
}

MasterServer::MasterServer(E_GameServer* gameserver) {

	last_check=0;
}

MasterServer::~MasterServer() {
	// TODO Auto-generated destructor stub
}


int MasterServer::RemoveStaleEntries(){
	// TODO: remove stale entries from the list.

	int i = 0;
	int curr_check=(int)time(NULL);
	int entry_time=(int)time(NULL);
	if(last_check + 60 < curr_check){
		std::vector<MasterServerItem>::iterator it= ServerList.begin();
		std::vector<MasterServerItem>::iterator it_end = ServerList.end();
		for(it; it != it_end;  ++it){
		
			entry_time = it->GetMTime();

			// TODO: Hard coding a expire of 1 hour.  This should be 
			// configurable once I figure out how to get the master 
			// server to run .cons without all the bloat.
			if((curr_check > entry_time + 60) && !(entry_time < 1)){
				//DEBUG: MSGOUT("Currtime: %i, Entry_Time: %i, removing server: %i", curr_check,  entry_time, it->GetSrvID());
				char srv_name[MAX_SERVER_NAME]; 
				it->GetServerName(srv_name, MAX_SERVER_NAME-1);
				MSGOUT("Expiring %s due to lack of heartbeat.", srv_name);
				ServerList.erase(it);
			}


		}
	last_check=curr_check;
	}
	
}
