/*
 * PARSEC - Main Server Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:46 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001
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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h> 
#include <sys/timeb.h>
#include <unistd.h>

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


// defaults -------------------------------------------------------------------
//
#define DEFAULT_SIM_FREQUENCY					100
#define DEFAULT_SERVER_TO_CLIENT_HEARTBEAT		FRAME_MEASURE_TIMEBASE * 2
#define DEFAULT_MASTERSERVER_INTERVAL			DEFAULT_REFFRAME_FREQUENCY * 10
#define DEFAULT_MASTERSERVER_NAME				"master.openparsec.com"
//#define DEFAULT_MASTERSERVER_NAME				"drax"
//#define DEFAULT_MASTERSERVER_NAME				"192.168.1.102"


// console texts --------------------------------------------------------------
//
static char error_resolving_masterserver[]      = "error resolving masterserver hostname.";


// ----------------------------------------------------------------------------
// ServerConfig methods 
// ----------------------------------------------------------------------------

// standard ctor --------------------------------------------------------------
//
ServerConfig::ServerConfig()
{
	SetServername( "Unnamed" );
	m_SimFrequency		= 0;
	m_nMaxNumClients	= 0;
	m_ServerIsMaster    = 0;
	m_ClientUpdateHeartbeat = 2 * DEFAULT_REFFRAME_FREQUENCY;
}

void ServerConfig::SetServerIsMaster(bool isMaster){
	m_ServerIsMaster = isMaster;

}
bool ServerConfig::GetServerIsMaster(){
	return m_ServerIsMaster;
}

// set the # of max players ---------------------------------------------------
// 
bool_t ServerConfig::SetMaxNumClients( int nMaxNumClients )		
{ 
	// we only accept setting the max # of clients once
	if ( m_nMaxNumClients == 0 ) {
		ASSERT( m_nMaxNumClients <= MAX_NET_ALLOC_SLOTS );
		m_nMaxNumClients = nMaxNumClients;

		// reset the simulation engine
		TheSimulator->Reset();

		return true;
	} else {
		return false;
	}
}


// return the max # of players ------------------------------------------------
//
int ServerConfig::GetMaxNumClients()
{
	if ( m_nMaxNumClients == 0 ) {
		// defaults to the upper bound 
		return MAX_NET_ALLOC_SLOTS;
	}
	return m_nMaxNumClients;
}


// set the simulation frequency -----------------------------------------------
//
bool_t ServerConfig::SetSimFrequency( int nSimFrequency ) 
{ 
	// we only accept setting the max # of clients once
	if ( m_SimFrequency == 0 ) {
		m_SimFrequency = nSimFrequency; 
		return true;
	} else {
		return false;
	}
}


// return the simulation frequency --------------------------------------------
//
int	ServerConfig::GetSimFrequency() 
{ 
	// ensure this is set to the default
	if ( m_SimFrequency == 0 ) {
		m_SimFrequency = DEFAULT_SIM_FREQUENCY;
	}
	return m_SimFrequency; 
}


// ----------------------------------------------------------------------------
// E_GameServer methods 
// ----------------------------------------------------------------------------

// default ctor ---------------------------------------------------------------
//
E_GameServer::E_GameServer() : 
m_bQuit( false )
{
}

// default dtor ---------------------------------------------------------------
//
E_GameServer::~E_GameServer()
{
}

// init data prior to running the console script ------------------------------
//
int E_GameServer::_InitPreConsoleScript()
{
	//NOTE: init all vars that are not modifiable through the console script,
	//      or set the defaults for the ones that are.

	// default values for user configurable values
	m_nInterface				= 0;
	m_MaintainFrequency			= 10;
	m_MasterServer_FrameTime	= DEFAULT_MASTERSERVER_INTERVAL;
	m_nPacketAverageSecs		= 3;
	strncpy( m_MasterServer_Hostname, DEFAULT_MASTERSERVER_NAME, MAX_MASTERSERVER_NAME );
	m_MasterServer_Hostname[ MAX_MASTERSERVER_NAME ] = 0;
	m_nNumServerLinks			= 0;

	// non modifiable
	m_nServerFrame				= 0;
	m_CurServerRefFrame			= REFFRAME_INVALID;
	m_LastServerRefFrame		= REFFRAME_INVALID;

	return TRUE;
}


// init data post running the console script ----------------------------------
//
int	E_GameServer::_InitPostConsoleScript()
{
	//NOTE: init all vars, that depend on settings set via the boot_sv.con 
	//      console script

	// idle for max. one simframe length
	m_ServerIdleTime_msec		= ( 1000 / GetSimFrequency() );

	// must be done before g_RefFrameCount is used the first time ( FRAME_MEASURE_TIMEBASE )
	SYSs_InitRefFrameCount();

	// default values for internal data
	m_Maintain_FrameTime		= FRAME_MEASURE_TIMEBASE / m_MaintainFrequency;
	m_SimTick_FrameTime			= FRAME_MEASURE_TIMEBASE / GetSimFrequency();
	
	m_MaintainFrameBase			= SYSs_GetRefFrameCount();
	m_MasterServerFrameBase		= SYSs_GetRefFrameCount();

	return TRUE;
}


// initialize the server components -------------------------------------------
//
int	E_GameServer::Init()
{
	//NOTE: some of the globals must already exist when parsing the console scripts

	// init all the modules
	TheModuleManager->InitAllModules();

	// Only init the game if I am not a master server...
	if(!this->GetServerIsMaster()){
		// init the Game globals
		TheGame				= G_Main::GetGame();
		TheGameInput		= G_Input::GetGameInput();
		TheGameExtraManager	= G_ExtraManager::GetExtraManager();
		TheGameCollDet		= G_CollDet::GetGameCollDet();

		// get the World global
		TheWorld = E_World::GetWorld();
	} 

	// init the network simulation engine, needed for master and game server
	TheSimNetInput	= E_SimNetInput::GetSimNetInput();
	TheSimNetOutput	= E_SimNetOutput::GetSimNetOutput();

	// init the simulation engine only if we are not running a master server.
	if(!this->GetServerIsMaster()){
		TheSimulator	= E_Simulator::GetSimulator();

		// register all AUX/SV vars
		CON_AUX_SV_Register();
	}
	// init CURSES
	CON_InitCurses();

	// init the input system
	INP_Init();

	// init all data not depending on the console script to be run
	_InitPreConsoleScript();

	// print the copyright
	PrintCopyRight();

	// init the console
	CON_InitConsole();

	if(this->GetServerIsMaster()) {
		MSGOUT("--- Open Parsec Master Server Running");
	}

	// init all data depending on console modifiable vars
	_InitPostConsoleScript();

	// if we are in master server mode, set some default values.
	if(this->GetServerIsMaster()){
		char tmp_srvname[MAX_SERVER_NAME + 1] = "";
		gethostname(tmp_srvname, MAX_SERVER_NAME);
		SV_SERVERID = 1; //master server is always 1
		SV_NETCONF_PORT = 6580; // master server port
		SV_MASTERSERVER_SENDHEARTBEAT = FALSE; // disable heartbeat sending.
	}

	// init the UDP driver
	TheUDPDriver = NET_UDPDriver::GetUDPDriver();
	TheUDPDriver->InitDriver( NULL, SV_NETCONF_PORT );

	// init the packet driver
	ThePacketDriver = NET_PacketDriver::GetPacketDriver();
	
	// init the packet handler
	ThePacketHandler = E_PacketHandler::GetPacketHandler();

	// init the connection manager
	TheConnManager = E_ConnManager::GetConnManager();

	// init the statistics manager
	//E_StatsManager* pStatsManager = TheStatsManager;

	// set to illegal challenge 
	m_MasterServer_Challenge = 0;

	// master server not yet resolved
	m_bMasterServer_NodeValid = false;

	// init the game, only if in game server mode.
	if(!this->GetServerIsMaster()){
		TheGame->Init();
		TheWorld->InitParticleSystem();
	}
	return TRUE;
}

// kill the server ------------------------------------------------------------
//
int	E_GameServer::Kill()
{
	// kill the console
	CON_KillConsole();

	// kill the input system
	INP_Kill();

	// kill CURSES
	CON_KillCurses();

	return TRUE;
}

// parse relevant commands from commandline -----------------------------------
//
int	E_GameServer::ParseCommandLine( int argc, char** argv )
{
	// set the programname
	sys_ProgramName = argv[ 0 ];

	// Parse the incoming commandline options and do the stuff.
	opterr = 0;
	int optchar= getopt(argc, argv, "m");

	while(optchar != -1){

		switch(optchar){
			case 'm':
				// TODO: set the option in server config to say we are a master server
				this->m_ServerIsMaster = 1;
				break;
			case '?':
				// unknown option.
				// TODO: print error(maybe) but continue...
				break;
			default:
				break;
		}
		optchar = getopt(argc, argv, "m");

	}

	return TRUE;
}


// ----------------------------------------------------------------------------
//
int	E_GameServer::PrintCopyRight()
{
	MSGOUT( "\n" );
	MSGOUT( " PARSEC SERVER V0.20 build " SERVER_BUILD_NUMBER "." );
	MSGOUT( "\n" );
	MSGOUT( "See LICENSE file for licensing information.         ");
	/*MSGOUT( " Copyright (c) 1996-2002 by Alex Mastny, Andreas Varga," );
	MSGOUT( " Clemens Beer, Markus Hadwiger, Stefan Poiss, Michael Woegerbauer." );
	MSGOUT( " All Rights Reserved." );*/
	MSGOUT( "-------------------------------------------------------" );
	MSGOUT( "               http://www.openparsec.com               " );
	MSGOUT( "-------------------------------------------------------" );
	MSGOUT( "\n" );

	return TRUE;
}


// ----------------------------------------------------------------------------
//
int	E_GameServer::PrintUsage()
{
	PrintCopyRight();

	return TRUE;
}


//FIXME_OSS: move this to seperate module as this is not yet used

// constant tick function pointer type ----------------------------------------
//
typedef int (*ConstTickedFunction)( refframe_t now );

// class for ensuring constant calls to a tick-function -----------------------
//
class PU_ConstTicker
{
protected:
	refframe_t			m_CurRefFrame;
	refframe_t			m_LastRefFrame;
	refframe_t			m_TickRefFrames;
	ConstTickedFunction	m_pTickedFunction;

public:

	// standard ctor
	//
	PU_ConstTicker( refframe_t _TickRefFrames, ConstTickedFunction _TickedFunction )
	{
		ASSERT( _TickedFunction != NULL );
		ASSERT( _TickRefFrames  > 0 );

		m_CurRefFrame		= REFFRAME_INVALID;
		m_LastRefFrame		= REFFRAME_INVALID;
		m_TickRefFrames		= _TickRefFrames;

		m_pTickedFunction	= _TickedFunction;
	}

	// maintain the tick function to achieve as many ticks as necessary between last call and current call
	//
	void MaintainTickFunction()
	{
		// first call ?
		if ( m_LastRefFrame == REFFRAME_INVALID ) {
			m_LastRefFrame = m_CurRefFrame;
		}

		// try to catch up as many ticks as we need to get under the frametime for one tick
		// the excess is left in m_LastRefFrame and accumulated in the next tick
		while( ( m_CurRefFrame - m_LastRefFrame ) >= m_TickRefFrames ) {
			m_LastRefFrame += m_TickRefFrames;

			// call the tick function
			m_pTickedFunction( m_LastRefFrame );
		}
	}

	// set the current refframe
	//
	void SetNow( refframe_t now )
	{
		m_CurRefFrame = now;
	}
};


// maintain the simulation ----------------------------------------------------
//
void E_GameServer::_MaintainSimulation()
{
	if ( m_LastServerRefFrame == REFFRAME_INVALID ) {
		m_LastServerRefFrame = m_CurServerRefFrame;
	}

	// catch up m_LastServerRefFrame in m_SimTick_FrameTime steps and run the simulation
	// remainging excess will be stored in m_LastServerRefFrame
	while ( ( m_CurServerRefFrame - m_LastServerRefFrame ) >= m_SimTick_FrameTime ) {
		m_LastServerRefFrame += m_SimTick_FrameTime;
		
		//MSGOUT( "%d: TheSimulator->DoSim( %d ), cur: %d diff: %d", TheSimulator->GetSimFrame(), m_LastServerRefFrame, m_CurServerRefFrame, m_CurServerRefFrame - m_LastServerRefFrame );
		// run the simulation frame/tick
		TheSimulator->DoSim( m_LastServerRefFrame );
		//LOGOUT(( "DoSim(): SimFrame:%d m_LastServerRefFrame: %d, m_nServerFrame: %d", TheSimulator->GetSimFrame(), m_LastServerRefFrame, m_nServerFrame ));
	}
}


// add a serverlink -----------------------------------------------------------
// 
int E_GameServer::AddServerLink( int serverid, Vector3* pos_spec, Vector3* dir_spec )
{
	ASSERT( serverid > 0 );
	ASSERT( pos_spec != NULL );
	ASSERT( dir_spec != NULL );

	if ( m_nNumServerLinks >= MAX_NUM_LINKS ) {
		return FALSE;
	}

	// norm the direction
	NormVctX( dir_spec );

	m_ServerLinks[ m_nNumServerLinks ].m_serverid = serverid;

	m_ServerLinks[ m_nNumServerLinks ].m_pos.X = pos_spec->X;
	m_ServerLinks[ m_nNumServerLinks ].m_pos.Y = pos_spec->Y;
	m_ServerLinks[ m_nNumServerLinks ].m_pos.Z = pos_spec->Z;

	m_ServerLinks[ m_nNumServerLinks ].m_dir.X = dir_spec->X;
	m_ServerLinks[ m_nNumServerLinks ].m_dir.Y = dir_spec->Y;
	m_ServerLinks[ m_nNumServerLinks ].m_dir.Z = dir_spec->Z;

	// create the corresponding stargate
	TheGame->CreateStargate( serverid, pos_spec, dir_spec );

	m_nNumServerLinks++;

	return TRUE;
}



// set the new masterserver info ----------------------------------------------
//
void E_GameServer::SetMasterServerInfo( char* pHostname, refframe_t _MasterServer_FrameTime )
{
	ASSERT( pHostname != NULL );
	ASSERT( _MasterServer_FrameTime >= 0 );

	m_MasterServer_FrameTime = _MasterServer_FrameTime;
	strncpy( m_MasterServer_Hostname, pHostname, MAX_MASTERSERVER_NAME );
	m_MasterServer_Hostname[ MAX_MASTERSERVER_NAME ] = 0;
}


// check whether the node is the master server node ---------------------------
//
int	E_GameServer::IsMasterServerNode( node_t* node )
{
	return ( NODE_Compare( node, &m_MasterServer_Node ) == NODECMP_EQUAL );
}

// set the new MASV challenge and enforce a new announcement to the MASV ------
//
void E_GameServer::SetMasterServerChallenge( int nMASVChallenge ) 
{ 
	m_MasterServer_Challenge = nMASVChallenge; 
	// enforce immediate resend of server info
	m_MasterServerFrameBase = SYSs_GetRefFrameCount() - m_MasterServer_FrameTime;
}


// maintain communication to master server ------------------------------------
//
int E_GameServer::_MaintainMasterServer()
{
	// check whether to skip heartbeat sending
	if ( !SV_MASTERSERVER_SENDHEARTBEAT )
		return FALSE;

	refframe_t CurMasterServerRefFrames = SYSs_GetRefFrameCount() - m_MasterServerFrameBase;

	// check whether maintainance is necessary
	if ( CurMasterServerRefFrames >= m_MasterServer_FrameTime ) {

		// advance base
		m_MasterServerFrameBase += CurMasterServerRefFrames;

		// try to resolve master server node
		if ( !m_bMasterServer_NodeValid ) {
			if ( TheUDPDriver->ResolveHostName( m_MasterServer_Hostname, &m_MasterServer_Node ) ) {
				NODE_StorePort( &m_MasterServer_Node, DEFAULT_MASTERSERVER_UDP_PORT );
				m_bMasterServer_NodeValid = true;
			} else {
				CON_AddLine( error_resolving_masterserver );
			}
		}

		// send challenge/info packet to masterserver
		if ( m_bMasterServer_NodeValid ) {

			// build command
			char szBuffer[ MAX_RE_COMMANDINFO_COMMAND_LEN + 1 ];
			snprintf( szBuffer, MAX_RE_COMMANDINFO_COMMAND_LEN, 
						MASV_CHALLSTRING, 
						CLSV_PROTOCOL_MAJOR, CLSV_PROTOCOL_MINOR,
						m_MasterServer_Challenge,
						m_szServername,
						TheConnManager->GetNumConnected(),
						MAX_NUM_CLIENTS,
						SV_SERVERID,
						CPU_VENDOR_OS
					);

			// append a remote event containing the command
			E_REList* pUnreliable = E_REList::CreateAndAddRef( RE_LIST_MAXAVAIL );
			int rc = pUnreliable->NET_Append_RE_CommandInfo( szBuffer );
			ASSERT( rc == TRUE );

			// only send serverlinks if challenge is valid
			if ( m_MasterServer_Challenge != 0 ) {

				// append all links or until packet full
				for( int nLink = 0; nLink < m_nNumServerLinks; nLink++ ) {
					if ( !pUnreliable->NET_Append_RE_ServerLinkInfo( SV_SERVERID, m_ServerLinks[ nLink ].m_serverid, SERVERLINKINFO_1_TO_2 ) ) {
						break;
					}
				}
			}

			// send a datagram
			ThePacketHandler->Send_STREAM_Datagram( pUnreliable, &m_MasterServer_Node, PLAYERID_MASTERSERVER );

			// release the RE list from here
			pUnreliable->Release();
		}
	}

	return TRUE;
}



// run housekeeping -----------------------------------------------------------
//
int E_GameServer::_MaintainHousekeeping()
{
	refframe_t CurMaintainRefFrames = SYSs_GetRefFrameCount() - m_MaintainFrameBase;
	
	// check whether maintainance is necessary
	if ( CurMaintainRefFrames >= m_Maintain_FrameTime ) {
		
		// advance base for frame measurement
		m_MaintainFrameBase += CurMaintainRefFrames;
		
		// check whether clients are alive and timeout if needed
		TheConnManager->CheckAliveStatus();

		// recalulate the average packet sizes sent to each client
		TheSimNetOutput->RecalcAveragePacketSizes();

		// cleanup all zombie distributables
		//FIXME: we could call this function after timeout of max. RTT_OF_ALL_CLIENTS
		TheSimNetOutput->CleanupZombieDistributables();
	}
	
	return TRUE;
}

// run the SERVER frame -------------------------------------------------------
//
refframe_t E_GameServer::ServerFrame()
{
	m_CurServerRefFrame = SYSs_GetRefFrameCount();
	//MSGOUT( "E_GameServer::ServerFrame(): %d", m_CurServerRefFrame );

	// run packet chain processing function ( reads network, filters out invalid events, fills input queue ) 
	ThePacketDriver->NET_ProcessPacketChain();

	// if we are not a master server, do the game server stuff...
	if(!this->GetServerIsMaster()){
		// process queue with input from all clients
		TheSimNetInput->ProcessInputREList();

		// maintain the simulation
		_MaintainSimulation();

		// maintain housekeeping
		_MaintainHousekeeping();

		// maintain info to master server
		_MaintainMasterServer();

		// update clients if necessary
		TheSimNetOutput->DoClientUpdates();
	} else {
		// TODO: Master server stuff....

	}
	// increment the serverframe counter
	//MSGOUT( "m_nServerFrame++" );
	m_nServerFrame++;

	// return the duration of the server frame
	return SYSs_GetRefFrameCount() - m_CurServerRefFrame;
}



// run the SERVER mainloop ----------------------------------------------------
//
int	E_GameServer::MainLoop()
{
	//LARGE_INTEGER pfc_freq;
	//QueryPerformanceFrequency( &pfc_freq );
	//LOGOUT(( "QPFC-Frequency:%I64d", pfc_freq.QuadPart ));

	refframe_t now = SYSs_GetRefFrameCount();

	for( ; !m_bQuit ; ) {

		//LOGOUT(( "Before select()." ));
		
		// check whether we have a network input for at most m_ServerIdleTime
		int netInput = TheUDPDriver->SleepUntilNetInput( m_ServerIdleTime_msec );
		if ( netInput < 0 ) {
			MSGOUT( "E_GameServer: error checking for network input" );
		}

		//LOGOUT(( "Before ServerFrame(). NETINPUT:%d", netInput ));

		// run one server frame
		refframe_t duration = ServerFrame();

		// just print "TICK" every second
		//if ( ( SYSs_GetRefFrameCount() - now ) > ( FRAME_MEASURE_TIMEBASE * 0.2 ) ) {

			// check input
			INP_HandleInput();

			// process console input
			CON_ConsoleMain();

		//	now = SYSs_GetRefFrameCount();
		//}

		//LOGOUT(( "After ServerFrame(). NETINPUT:%d", netInput ));

		//SYSTEMTIME st;
		//GetLocalTime( &st );
		//LOGOUT(( "The time is %02d:%02d:%02d.%03d. NETINPUT:%d", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, netInput ));
	}

	return TRUE;
}


// key table for MASTERSERVER command -----------------------------------------
//
key_value_s masterserver_key_value[] = {

	{ "name",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "interval",	NULL,	KEYVALFLAG_NONE				},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_MASTERSERVER_NAME,
	KEY_MASTERSERVER_INTERVAL,
};

// min/max interval for heartbeats to the masterserve -------------------------
//
#define MASTERSERVER_INTERVAL_MIN	5			
#define MASTERSERVER_INTERVAL_MAX   86400		


// console command for the masterserver configuration -------------------------
//
PRIVATE
int Cmd_MASTERSERVER( char* masv_command )
{
	//NOTE:
	//CONCOM:
	// masterserver_command	::= 'sv.masterserver.conf' <name_spec> [<interval_spec>]
	// name_spec			::= 'name' <masterservername>
	// interval_spec		::= 'interval' <sec>

	ASSERT( masv_command != NULL );
	HANDLE_COMMAND_DOMAIN( masv_command );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( masterserver_key_value, masv_command ) )
		return TRUE;

	char* pName = masterserver_key_value[ KEY_MASTERSERVER_NAME ].value;

	refframe_t _MasterServer_FrameTime = TheServer->GetMasterServerFrameTime();

	// get the interval and adjust for refframes
	if ( masterserver_key_value[ KEY_MASTERSERVER_INTERVAL ].value != NULL ) {
	
		int interval;
		ScanKeyValueInt( &masterserver_key_value[ KEY_MASTERSERVER_INTERVAL ], &interval );

		if ( interval < MASTERSERVER_INTERVAL_MIN ) {
			interval = MASTERSERVER_INTERVAL_MIN;
			CON_AddLine( "sv.masterserver interval clamped to min" );
		} else if ( interval > MASTERSERVER_INTERVAL_MAX ) {
			interval = MASTERSERVER_INTERVAL_MAX;
			CON_AddLine( "sv.masterserver interval clamped to max" );
		}

		_MasterServer_FrameTime = interval * DEFAULT_REFFRAME_FREQUENCY;
	}

	// set the new masterserver info
	TheServer->SetMasterServerInfo( pName, _MasterServer_FrameTime );

	return TRUE;
}


// key table for "SV.CONF" command --------------------------------------------
//
key_value_s sv_conf_key_value[] = {

	{ "name",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "maxplayers",	NULL,	KEYVALFLAG_NONE				},
	{ "simfreq",	NULL,	KEYVALFLAG_NONE				},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_SERVER_NAME,
	KEY_SERVER_MAXPLAYERS,
	KEY_SERVER_SIMFREQ,
};

// min/max interval simulation frequency --------------------------------------
//
#define SIMFREQ_MIN		5			
#define SIMFREQ_MAX		DEFAULT_REFFRAME_FREQUENCY


// console command for configuring the server ---------------------------------
//
PRIVATE
int Cmd_SV_CONF( char* sv_conf_command )
{
	//NOTE:
	//CONCOM:
	// sv_conf_command	::= 'sv.conf' [<name_spec>] [<maxplayer_spec>] [<simfreq_spec>]
	// name_spec		::= 'name' <servername>
	// maxplayers_spec	::= 'maxplayers' <maxplayers>
	// simfreq_spec     ::= 'simfreq' <simfreq>

	ASSERT( sv_conf_command != NULL );
	HANDLE_COMMAND_DOMAIN( sv_conf_command );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( sv_conf_key_value, sv_conf_command ) )
		return TRUE;

	// name specified ?
	if ( sv_conf_key_value[ KEY_SERVER_NAME ].value != NULL ) {
		TheServer->SetServername( sv_conf_key_value[ KEY_MASTERSERVER_NAME ].value );
	}

	// maxplayer specified ?
	if ( sv_conf_key_value[ KEY_SERVER_MAXPLAYERS ].value != NULL ) {
		int maxplayers;
		ScanKeyValueInt( &sv_conf_key_value[ KEY_SERVER_MAXPLAYERS ], &maxplayers );

		// set the current # of max. players
		if ( !TheServer->SetMaxNumClients( maxplayers ) ) {
			CON_AddLine( "the max # of players can only be set once. please restart the server." );
		}
	}

	// simfreq specified ?
	if ( sv_conf_key_value[ KEY_SERVER_SIMFREQ ].value != NULL ) {
		int simfreq;
		ScanKeyValueInt( &sv_conf_key_value[ KEY_SERVER_SIMFREQ ], &simfreq );

		if ( simfreq < SIMFREQ_MIN ) {
			CON_AddLine( "simfrequency clamped to min" );
			simfreq = SIMFREQ_MIN;
		} else if ( simfreq > SIMFREQ_MAX ) {
			CON_AddLine( "simfrequency clamped to max" );
			simfreq = SIMFREQ_MAX;
		}

		if ( !TheServer->SetSimFrequency( simfreq ) ) {
			CON_AddLine( "the simulation frequency can only be set once. please restart the server." );
		}
	}

	return TRUE;
}

// key table for "SV.LINK" command --------------------------------------------
//
key_value_s sv_link_key_value[] = {

	{ "serverid",	NULL,	KEYVALFLAG_MANDATORY		},
	{ "pos",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "dir",		NULL,	KEYVALFLAG_PARENTHESIZE		},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_SERVERLINK_SERVERID,
	KEY_SERVERLINK_POS,
	KEY_SERVERLINK_DIR,
};


// console command for specifying server links --------------------------------
//
PRIVATE
int Cmd_SV_LINK( char* sv_link_command )
{
	//NOTE:
	//CONCOM:
	// sv_link_command	::= 'sv.link' <serverid_spec> [<pos_spec>] [<dir_spec>]
	// serverid_spec	::= 'serverid' <serverid>
	// pos_spec			::= 'pos' '(' <float> <float> <float> ')'
	// dir_spec			::= 'dir' '(' <float> <float> <float> ')'

	ASSERT( sv_link_command != NULL );
	HANDLE_COMMAND_DOMAIN( sv_link_command );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( sv_link_key_value, sv_link_command ) ) {
		return TRUE;
	}
	
	ASSERT( sv_link_key_value[ KEY_SERVERLINK_SERVERID ].value != NULL );
	int serverid;
	ScanKeyValueInt( &sv_link_key_value[ KEY_SERVERLINK_SERVERID ], &serverid );
	if ( serverid == 0 ) {
		CON_AddLine( "invalid server id specified" );
	}

	// parse position
	Vector3 pos_spec;
	if ( sv_link_key_value[ KEY_SERVERLINK_POS ].value != NULL ) {
		if ( !ScanKeyValueFloatList( &sv_link_key_value[ KEY_SERVERLINK_POS ], (float*)&pos_spec.X, 3, 3 ) ) {
			CON_AddLine( "position invalid" );
			return TRUE;
		}
	} else {
		//FIXME: constants
		pos_spec.X = ( RAND() % 1000 ) - 500;
		pos_spec.Y = ( RAND() % 1000 ) - 500;
		pos_spec.Z = ( RAND() % 1000 ) - 500;
		pos_spec.VisibleFrame = 0;
	}

	// parse direction
	Vector3 dir_spec;
	if ( sv_link_key_value[ KEY_SERVERLINK_DIR ].value != NULL ) {
		if ( !ScanKeyValueFloatList( &sv_link_key_value[ KEY_SERVERLINK_DIR ], (float*)&dir_spec.X, 3, 3 ) ) {
			CON_AddLine( "direction invalid" );
			return TRUE;
		}
	} else {
		// default to point in z direction
		dir_spec.X = 0.0f;
		dir_spec.Y = 0.0f;
		dir_spec.Z = 1.0f;
		dir_spec.VisibleFrame = 0;
	}

	// add the serverlink
	TheServer->AddServerLink( serverid, &pos_spec, &dir_spec );

	return TRUE;
}



REGISTER_MODULE( E_GAMESERVER )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "sv.masterserver.conf" command
	regcom.command	 = "sv.masterserver.conf";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_MASTERSERVER;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "sv.conf" command
	regcom.command	 = "sv.conf";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_SV_CONF;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "sv.link" command
	regcom.command	 = "sv.link";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_SV_LINK;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );
}



