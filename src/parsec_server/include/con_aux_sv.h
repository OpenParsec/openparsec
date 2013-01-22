/*
 * PARSEC HEADER: cons_aux.h
 */

#ifndef _CON_AUX_SV_H_
#define _CON_AUX_SV_H_


// size of SV array
// size of aux arrays
#define MAX_SV_ARRAY_SIZE			512

#define SV_PACKAGE_SCRIPTS							AuxEnabling[  0 ]
#define SV_CONSOLE_LEVEL_MESSAGES					AuxEnabling[  1 ]
#define SV_PACKAGE_SCRIPT_LISTING					AuxEnabling[  2 ]
#define SV_CLASS_MAP_TABLE_DISABLE_RESET			AuxEnabling[  3 ]
//NOTE: AUX_* flags are common with client code
#define AUX_NETCODE_FLAGS							AuxEnabling[  4 ]
#define AUX_CHEAT_DISABLE_ENERGY_CHECKS				AuxEnabling[  5 ]
#define SV_CHEAT_ENERGY_CHECKS						AuxEnabling[  5 ]
#define AUX_ENABLE_OBJECT_OVERLOADING	 			AuxEnabling[  6 ]
#define SV_OBJECTCLASSES_OVERLOADING				AuxEnabling[  6 ]
#define SV_CHEAT_DEVICE_CHECKS						AuxEnabling[  7 ]
#define AUX_CHEAT_DISABLE_DEVICE_CHECKS				AuxEnabling[  7 ]
#define SV_GAME_EXTRAS_AUTOCREATE					AuxEnabling[  8 ]
#define SV_MASTERSERVER_SENDHEARTBEAT				AuxEnabling[  9 ]
#define SV_GAME_EXTRAS_TESTPLACE					AuxEnabling[ 11 ]
#define SV_GAME_KILLLIMIT							AuxEnabling[ 12 ]
#define SV_GAME_TIMELIMIT							AuxEnabling[ 13 ]
#define SV_GAME_RESTART_TIMEOUT						AuxEnabling[ 14 ]
#define SV_SERVERID									AuxEnabling[ 15 ]
#define SV_FEDID									AuxEnabling[ 16 ]
#define SV_NETCONF_PORT								AuxEnabling[ 17 ]
#define SV_DEBUG_NETSTREAM_DUMP						AuxEnabling[ 18 ]
#define AUX_DEBUG_NETSTREAM_DUMP					AuxEnabling[ 18 ]		// for compatibility in NET_STREAM
#define AUX_ENABLE_CONSOLE_DEBUG_MESSAGES			AuxEnabling[ 19 ]
#define SV_DEBUG_CONSOLE_MESSAGES					AuxEnabling[ 19 ]
#define AUX_FORCE_WORKDIR_TO_CURRENT				AuxEnabling[ 20 ]
#define SV_GAME_WORKDIR_FORCE_CURRENT				AuxEnabling[ 20 ]
#define AUX_FORCE_FILEPATH_TO_CURRENT				AuxEnabling[ 21 ]
#define SV_GAME_FILEPATH_FORCE_CURRENT				AuxEnabling[ 21 ]
#define SV_GAME_EXTRAS_MAXNUM						AuxEnabling[ 22 ]
#define AUX_DISABLE_PACKAGE_DATA_FILES				AuxEnabling[ 23 ]
#define AUX_DISABLE_PACKAGE_SCRIPTS					AuxEnabling[ 24 ]

#define AUX_ARRAY_NUM_ENTRIES_USED					26	// UPDATE THIS!! <==

#if ( AUX_ARRAY_NUM_ENTRIES_USED > MAX_SV_ARRAY_SIZE )
	#error "MAX_AUX_ENABLING too small!"
#endif


// external functions ---------------------------------------------------------
//
void CON_AUX_SV_Register();

#endif // _CON_AUX_SV_H_


