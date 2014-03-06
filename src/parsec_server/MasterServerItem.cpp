/*
 * MasterServerList.cpp
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


#include "MasterServerItem.h"

MasterServerItem::MasterServerItem() {
	// TODO Auto-generated constructor stub

}
MasterServerItem::MasterServerItem(int SrvID, int CurrPlayers, int MaxPlayers,
		int PMajor, int PMinor, char ServerName[MAX_SERVER_NAME + 1],
		char OS[MAX_OSNAME_LEN + 1], node_t *node) {

		_SrvID = SrvID;
		_CurrPlayers = CurrPlayers;
		_MaxPlayers = MaxPlayers;
		_PMajor = PMajor;
		_PMinor = PMinor;
		strncpy(_ServerName, ServerName, MAX_SERVER_NAME);
		_ServerName[MAX_SERVER_NAME] = '\0';
		strncpy(_OS, OS, MAX_OSNAME_LEN);
		_OS[MAX_OSNAME_LEN]  = '\0';
		NODE_Copy(&_Node, node);
		_MTime = time(NULL);
		//MSGOUT("Added %i in constructor with time %i", _SrvID, _MTime);

}
MasterServerItem::~MasterServerItem() {
	// TODO Auto-generated destructor stub
}

bool MasterServerItem::update(int SrvID, int CurrPlayers, int MaxPlayers,
		int PMajor, int PMinor, char ServerName[MAX_SERVER_NAME + 1],
		char OS[MAX_OSNAME_LEN + 1], node_t *node) {

	_SrvID = SrvID;
	_CurrPlayers = CurrPlayers;
	_MaxPlayers = MaxPlayers;
	_PMajor = PMajor;
	_PMinor = PMinor;
	SAFE_STR_DUPLICATE(_ServerName, ServerName, MAX_SERVER_NAME-1);
	_ServerName[MAX_SERVER_NAME] = '\0';
	SAFE_STR_DUPLICATE(_OS, OS, MAX_OSNAME_LEN-1);
	_OS[MAX_OSNAME_LEN]  = '\0';
	NODE_Copy(&_Node, node);
	_MTime =time(NULL);
	//MSGOUT("Added %i in update() with time %i", _SrvID, _MTime);


	return true;
}

bool MasterServerItem::remove() {

	// sets _MTime to zero which should always remove it on next 
	// pass of the array for expiry
//	_MTime=0;
	_SrvID=-1;
	return false;
}


bool MasterServerItem::operator <(const MasterServerItem& msl) const {

	return false;
}

bool MasterServerItem::operator ==(const MasterServerItem& msl) const {

	return false;
}

bool MasterServerItem::operator !=(const MasterServerItem& msl) const {

	return false;
}



bool MasterServerItem::isValid() const {
	if(_SrvID > -1) {
		return true;
	}
	return false;
}

int MasterServerItem::GetSrvID() {
	return _SrvID;
}

int MasterServerItem::GetCurrPlayers() {
	return _CurrPlayers;
}

int MasterServerItem::GetMaxPlayers() {
	return _MaxPlayers;
}

int MasterServerItem::GetPMajor() {
	return _PMajor;
}

int MasterServerItem::GetPMinor() {
	return _PMinor;
}

int MasterServerItem::GetServerName(char *buffer, int buffer_sz) {

	int str_sz = strlen(_ServerName);
	if(buffer_sz < str_sz + 1){
		return -1;
	}

	strncpy(buffer, _ServerName, str_sz);
	buffer[str_sz + 1] = '\0';

	return 0;
}

int  MasterServerItem::GetOS(char *buffer, int buffer_sz) {
	int str_sz = strlen(_OS);
	if(buffer_sz < str_sz + 1){
		return -1;
	}

	strncpy(buffer, _OS, str_sz);
	buffer[str_sz + 1] = '\0';

	return 0;
}

time_t MasterServerItem::GetMTime(){
	//MSGOUT("Reading entry time %d", (int)_MTime);
	return _MTime;
}

MasterServerItem::MasterServerItem(const MasterServerItem& msi_copy) :
			 _SrvID (msi_copy._SrvID),
			 _CurrPlayers (msi_copy._CurrPlayers),
			 _MaxPlayers (msi_copy._MaxPlayers),
			 _PMajor (msi_copy._PMajor),
			 _PMinor (msi_copy._PMinor)
{
	strncpy(_ServerName, msi_copy._ServerName, MAX_SERVER_NAME);
	_ServerName[MAX_SERVER_NAME]='\0';
	strncpy( _OS, msi_copy._OS, MAX_OSNAME_LEN);
	_OS[MAX_OSNAME_LEN]='\0';
	memcpy(&_Node, &msi_copy._Node, sizeof( node_t ));

}

int MasterServerItem::GetNode(node_t* node) {
	NODE_Copy(node, &_Node);

	return true;
}
