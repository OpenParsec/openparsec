/*
 * PARSEC - SERVER Integer Variable Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:44 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001-2002
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
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

// subsystem & headers
#include "net_defs.h"
//#include "sys_defs.h"
//#include "vid_defs.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "con_int_sv.h"

// proprietary module headers
#include "con_aux_sv.h"
#include "con_main_sv.h"
//#include "e_record.h"
//#include "g_demo.h"
//#include "g_stars.h"
#include "g_extra.h"
#include "net_game_sv.h"
#include "net_util.h"
#include "e_relist.h"
#include "g_main_sv.h"
#include "e_simulator.h"


// integer input/output configuration -----------------------------------------
//
PUBLIC	const char* int_print_base = "%d";
PUBLIC	int		int_calc_base  = 10;


// external variables which need to be accessed but are not in GLOBALS.H ------
//
extern int	ConsoleEnterLength;
extern int	ConsoleTextX;
extern int	ConsoleTextY;
extern int	EchoExternalCommands;
extern int	HistoryForBatchEntry;


// intermediate storage for MyShip integer properties -------------------------
//
struct myship_int_s {

	int Weapons;
	int Specials;
	int CurDamage;
	int CurEnergy;
	int CurSpeed;
	int NumMissls;
	int NumHomMissls;
	int NumPartMissls;
	int NumMines;
	int MaxDamage;
	int MaxEnergy;
	int MaxSpeed;
	int MaxNumMissls;
	int MaxNumHomMissls;
	int MaxNumPartMissls;
	int MaxNumMines;
};

static myship_int_s myship_int;

/*

//FIXME:

#define DEF_MYSHIP_INT( f ) \
int GetMSInt##f() \
{ \
	return (int)MyShip->f; \
} \
void SetMSInt##f() \
{ \
	MyShip->f = myship_int.f; \
	Record_MyShipState##f(); \
}

*/

#define DEF_MYSHIP_INT( f ) \
int GetMSInt##f() \
{ \
	return 0; \
} \
void SetMSInt##f() \
{ \
}

DEF_MYSHIP_INT( Weapons )
DEF_MYSHIP_INT( Specials )
DEF_MYSHIP_INT( CurDamage )
DEF_MYSHIP_INT( CurEnergy )
DEF_MYSHIP_INT( CurSpeed )
DEF_MYSHIP_INT( NumMissls )
DEF_MYSHIP_INT( NumHomMissls )
DEF_MYSHIP_INT( NumPartMissls )
DEF_MYSHIP_INT( NumMines )
DEF_MYSHIP_INT( MaxDamage )
DEF_MYSHIP_INT( MaxEnergy )
DEF_MYSHIP_INT( MaxSpeed )
DEF_MYSHIP_INT( MaxNumMissls )
DEF_MYSHIP_INT( MaxNumHomMissls )
DEF_MYSHIP_INT( MaxNumPartMissls )
DEF_MYSHIP_INT( MaxNumMines )


//FIXME: get rid of these proxy stuff

// proxy structure for G_Main data members modifiable through ---------------
//
struct game_proxy_int_s {

	int EnergyExtraBoost;
	int RepairExtraBoost;
	int DumbPackNumMissls;
	int HomPackNumMissls;
	int SwarmPackNumMissls;
	int ProxPackNumMines;
    int m_NebulaID;
};

// proxy structure for G_Extra data members modifiable through the console ---
//
struct game_extra_proxy_int_s {

	int MaxExtraArea;
	int MinExtraDist;
	int ExtraProbability;
	int ProbHelixCannon;
	int ProbLightningDevice;
	int ProbPhotonCannon;
	int ProbProximityMine;
	int ProbRepairExtra;
	int ProbAfterburner;
	int ProbHoloDecoy;
	int ProbInvisibility;
	int ProbInvulnerability;
	int ProbEnergyField;
	int ProbLaserUpgrade;
	int ProbLaserUpgrade1;
	int ProbLaserUpgrade2;
	int ProbMissilePack;
	int ProbDumbMissPack;
	int ProbHomMissPack;
	int ProbSwarmMissPack;
	int ProbEmpUpgrade1;
	int ProbEmpUpgrade2;
};


static game_proxy_int_s			Game_proxy;
static game_extra_proxy_int_s	GameExtraManager_proxy;

#define DEF_PROXY_INT( name, f ) \
int Get##name##Int##f() \
{ \
	return (int)The##name->f; \
} \
void Set##name##Int##f() \
{ \
	The##name->f = name##_proxy.f; \
}

DEF_PROXY_INT( GameExtraManager, MaxExtraArea )
DEF_PROXY_INT( GameExtraManager, MinExtraDist )
DEF_PROXY_INT( Game, EnergyExtraBoost )
DEF_PROXY_INT( Game, RepairExtraBoost )
DEF_PROXY_INT( Game, DumbPackNumMissls )
DEF_PROXY_INT( Game, HomPackNumMissls )
DEF_PROXY_INT( Game, SwarmPackNumMissls )
DEF_PROXY_INT( Game, ProxPackNumMines )
DEF_PROXY_INT( Game, m_NebulaID)
DEF_PROXY_INT( GameExtraManager, ExtraProbability )
DEF_PROXY_INT( GameExtraManager, ProbHelixCannon )
DEF_PROXY_INT( GameExtraManager, ProbLightningDevice )
DEF_PROXY_INT( GameExtraManager, ProbPhotonCannon )
DEF_PROXY_INT( GameExtraManager, ProbProximityMine )
DEF_PROXY_INT( GameExtraManager, ProbRepairExtra )
DEF_PROXY_INT( GameExtraManager, ProbAfterburner )
DEF_PROXY_INT( GameExtraManager, ProbHoloDecoy )
DEF_PROXY_INT( GameExtraManager, ProbInvisibility )
DEF_PROXY_INT( GameExtraManager, ProbInvulnerability )
DEF_PROXY_INT( GameExtraManager, ProbEnergyField )
DEF_PROXY_INT( GameExtraManager, ProbLaserUpgrade )
DEF_PROXY_INT( GameExtraManager, ProbLaserUpgrade1 )
DEF_PROXY_INT( GameExtraManager, ProbLaserUpgrade2 )
DEF_PROXY_INT( GameExtraManager, ProbMissilePack )
DEF_PROXY_INT( GameExtraManager, ProbDumbMissPack )
DEF_PROXY_INT( GameExtraManager, ProbHomMissPack )
DEF_PROXY_INT( GameExtraManager, ProbSwarmMissPack )
DEF_PROXY_INT( GameExtraManager, ProbEmpUpgrade1 )
DEF_PROXY_INT( GameExtraManager, ProbEmpUpgrade2 )

/*
// perform remote event syncing for certain int vars --------------------------
//
#define DEF_RMEV_STATESYNC( f, k ) \
void RESync##f() \
{ \
	if ( !NET_RmEvAllowed( RE_STATESYNC ) ) \
		return; \
	NET_RmEvStateSync( (k), (TheGame->f) ); \
}

DEF_RMEV_STATESYNC( ExtraProbability,		RMEVSTATE_PROBEXTRA );
DEF_RMEV_STATESYNC( ProbHelixCannon,		RMEVSTATE_PROBHELIX );
DEF_RMEV_STATESYNC( ProbLightningDevice,	RMEVSTATE_PROBLIGHTNING );
DEF_RMEV_STATESYNC( ProbPhotonCannon,		RMEVSTATE_PROBPHOTON );
DEF_RMEV_STATESYNC( ProbProximityMine,		RMEVSTATE_PROBMINE );
DEF_RMEV_STATESYNC( ProbRepairExtra,		RMEVSTATE_PROBREPAIR );
DEF_RMEV_STATESYNC( ProbAfterburner,		RMEVSTATE_PROBAFTERBURNER );
DEF_RMEV_STATESYNC( ProbHoloDecoy,			RMEVSTATE_PROBHOLODECOY );
DEF_RMEV_STATESYNC( ProbInvisibility,		RMEVSTATE_PROBINVISIBILITY );
DEF_RMEV_STATESYNC( ProbInvulnerability,	RMEVSTATE_PROBINVULNERABILITY );
DEF_RMEV_STATESYNC( ProbEnergyField,		RMEVSTATE_PROBENERGYFIELD );
DEF_RMEV_STATESYNC( ProbLaserUpgrade,		RMEVSTATE_PROBLASERUPGRADE );
DEF_RMEV_STATESYNC( ProbLaserUpgrade1,		RMEVSTATE_PROBLASERUPGRADE1 );
DEF_RMEV_STATESYNC( ProbLaserUpgrade2,		RMEVSTATE_PROBLASERUPGRADE2 );
DEF_RMEV_STATESYNC( ProbMissilePack,		RMEVSTATE_PROBMISSPACK );
DEF_RMEV_STATESYNC( ProbDumbMissPack,		RMEVSTATE_PROBDUMBPACK );
DEF_RMEV_STATESYNC( ProbHomMissPack,		RMEVSTATE_PROBGUIDEPACK );
DEF_RMEV_STATESYNC( ProbSwarmMissPack,		RMEVSTATE_PROBSWARMPACK );
DEF_RMEV_STATESYNC( ProbEmpUpgrade1,		RMEVSTATE_PROBEMPUPGRADE1 );
DEF_RMEV_STATESYNC( ProbEmpUpgrade2,		RMEVSTATE_PROBEMPUPGRADE2 );
*/

// integer variable manipulation commands -------------------------------------
//
PRIVATE
int_command_s int_commands_default[] = {

//CAVEAT:
// absolute and relative positions of the following
// 4 commands MUST NOT be altered. the CONINFOINDX_xx
// constants in CON_INT.H must be defined consistently.

	//{ 0x01, "con.winx",				1,	489,	&ConsoleTextX,						CheckConExtents,    		NULL	},
	//{ 0x01, "con.winy",				1,	295,	&ConsoleTextY,						CheckConExtents,    		NULL	},
	//{ 0x01, "con.width",			11,	500,	&ConsoleEnterLength,				CheckConExtents,     		NULL	},
	//{ 0x01, "con.height",			5,	300,	&ConsoleHeight,						CheckConExtents,			NULL	},

	{ 0x01, "con.echoextcoms",		0,	1,		&EchoExternalCommands,				NULL,             			NULL	},
	{ 0x01, "con.scripthistory",	0,	1,		&HistoryForBatchEntry,				NULL,             			NULL	},

	{ 0x01, "net.packetrecording",	0,	1,		&RecordRemotePackets,				NULL,             			NULL	},

#ifdef ENABLE_CHEAT_COMMANDS
/*	
	{ 0x00, ".weapons",				0,	0xfff,  &myship_int.Weapons,				SetMSIntWeapons,			GetMSIntWeapons			},
	{ 0x00, ".specials",			0,	0xfffff,&myship_int.Specials,				SetMSIntSpecials,			GetMSIntSpecials		},
	{ 0x00, ".damage",				0,	10000,  &myship_int.CurDamage,				SetMSIntCurDamage,			GetMSIntCurDamage		},
	{ 0x00, ".energy",				0,	10000,  &myship_int.CurEnergy,				SetMSIntCurEnergy,			GetMSIntCurEnergy		},
	{ 0x00, ".speed",				0,	500000, &myship_int.CurSpeed,				SetMSIntCurSpeed,			GetMSIntCurSpeed		},
	{ 0x00, ".dumb",				0,	10000,  &myship_int.NumMissls,				SetMSIntNumMissls,			GetMSIntNumMissls		},
	{ 0x00, ".guide",				0,	10000,  &myship_int.NumHomMissls,			SetMSIntNumHomMissls,		GetMSIntNumHomMissls	},
	{ 0x00, ".swarm",				0,	10000,  &myship_int.NumPartMissls,			SetMSIntNumPartMissls,		GetMSIntNumPartMissls	},
	{ 0x00, ".mines",				0,	10000,  &myship_int.NumMines,				SetMSIntNumMines,			GetMSIntNumMines		},
	{ 0x00, ".damage.max",			0,	10000,  &myship_int.MaxDamage,				SetMSIntMaxDamage,			GetMSIntMaxDamage		},
	{ 0x00, ".energy.max",			0,	10000,  &myship_int.MaxEnergy,				SetMSIntMaxEnergy,			GetMSIntMaxEnergy		},
	{ 0x00, ".speed.max",			0,	500000, &myship_int.MaxSpeed,				SetMSIntMaxSpeed,			GetMSIntMaxSpeed		},
	{ 0x00, ".dumb.max",			0,	10000,  &myship_int.MaxNumMissls,			SetMSIntMaxNumMissls,		GetMSIntMaxNumMissls	},
	{ 0x00, ".guide.max",			0,	10000,  &myship_int.MaxNumHomMissls,		SetMSIntMaxNumHomMissls,	GetMSIntMaxNumHomMissls	},
	{ 0x00, ".swarm.max",			0,	10000,  &myship_int.MaxNumPartMissls,		SetMSIntMaxNumPartMissls,	GetMSIntMaxNumPartMissls },
	{ 0x00, ".mines.max",			0,	10000,  &myship_int.MaxNumMines,			SetMSIntMaxNumMines,		GetMSIntMaxNumMines		},
*/
#endif // ENABLE_CHEAT_COMMANDS

{ 0x00, "extras.max",			0,	100,    &SV_GAME_EXTRAS_MAXNUM,				NULL, NULL		},
{ 0x00, "extras.area",			100,10000,  &GameExtraManager_proxy.MaxExtraArea,           SetGameExtraManagerIntMaxExtraArea,			GetGameExtraManagerIntMaxExtraArea		},	
{ 0x00, "extras.dist",			50,	1000,   &GameExtraManager_proxy.MinExtraDist,           SetGameExtraManagerIntMinExtraDist,			GetGameExtraManagerIntMinExtraDist		},	

{ 0x00, "nebula.id",            2,     5,   &Game_proxy.m_NebulaID,             SetGameIntm_NebulaID,                       GetGameIntm_NebulaID        },
{ 0x00, "energy.boost",			0,	1000,   &Game_proxy.EnergyExtraBoost,       SetGameIntEnergyExtraBoost,					GetGameIntEnergyExtraBoost	},	
{ 0x00, "repair.boost",			0,	1000,   &Game_proxy.RepairExtraBoost,		SetGameIntRepairExtraBoost,					GetGameIntRepairExtraBoost	},	
																															
{ 0x00, "pack.dumb.size",		0,	100,    &Game_proxy.DumbPackNumMissls,      SetGameIntDumbPackNumMissls,				GetGameIntDumbPackNumMissls	},	
{ 0x00, "pack.guide.size",		0,	100,    &Game_proxy.HomPackNumMissls,       SetGameIntHomPackNumMissls,					GetGameIntHomPackNumMissls	},	
{ 0x00, "pack.swarm.size",		0,	100,    &Game_proxy.SwarmPackNumMissls,		SetGameIntSwarmPackNumMissls,				GetGameIntSwarmPackNumMissls},	
{ 0x00, "pack.mine.size",		0,	100,    &Game_proxy.ProxPackNumMines,       SetGameIntProxPackNumMines,					GetGameIntProxPackNumMines	},	
																															
{ 0x00, "prob.extra",			0,	100,    &GameExtraManager_proxy.ExtraProbability,       SetGameExtraManagerIntExtraProbability,		GetGameExtraManagerIntExtraProbability	},
																															
{ 0x00, "prob.helix",			0,	100,    &GameExtraManager_proxy.ProbHelixCannon,        SetGameExtraManagerIntProbHelixCannon,		GetGameExtraManagerIntProbHelixCannon	},	
{ 0x00, "prob.lightning",		0,	100,    &GameExtraManager_proxy.ProbLightningDevice,    SetGameExtraManagerIntProbLightningDevice,	GetGameExtraManagerIntProbLightningDevice},	
{ 0x00, "prob.photon",			0,	100,    &GameExtraManager_proxy.ProbPhotonCannon,       SetGameExtraManagerIntProbPhotonCannon,		GetGameExtraManagerIntProbPhotonCannon	},	
{ 0x00, "prob.mine",			0,	100,    &GameExtraManager_proxy.ProbProximityMine,      SetGameExtraManagerIntProbProximityMine,	GetGameExtraManagerIntProbProximityMine	},	
{ 0x00, "prob.repair",			0,	100,    &GameExtraManager_proxy.ProbRepairExtra,        SetGameExtraManagerIntProbRepairExtra,		GetGameExtraManagerIntProbRepairExtra	},	
{ 0x00, "prob.aburner",			0,	100,    &GameExtraManager_proxy.ProbAfterburner,        SetGameExtraManagerIntProbAfterburner,		GetGameExtraManagerIntProbAfterburner	},	
{ 0x00, "prob.decoy",			0,	100,    &GameExtraManager_proxy.ProbHoloDecoy,	        SetGameExtraManagerIntProbHoloDecoy,		GetGameExtraManagerIntProbHoloDecoy		},	
{ 0x00, "prob.invisible",		0,	100,    &GameExtraManager_proxy.ProbInvisibility,       SetGameExtraManagerIntProbInvisibility,		GetGameExtraManagerIntProbInvisibility	},	
{ 0x00, "prob.invulnerable",	0,	100,    &GameExtraManager_proxy.ProbInvulnerability,    SetGameExtraManagerIntProbInvulnerability,	GetGameExtraManagerIntProbInvulnerability},	
{ 0x00, "prob.energyfield",		0,	100,    &GameExtraManager_proxy.ProbEnergyField,        SetGameExtraManagerIntProbEnergyField,		GetGameExtraManagerIntProbEnergyField	},	
																															
{ 0x00, "prob.lupgrade",		0,	100,    &GameExtraManager_proxy.ProbLaserUpgrade,       SetGameExtraManagerIntProbLaserUpgrade,		GetGameExtraManagerIntProbLaserUpgrade	},	
{ 0x00, "prob.lupgrade1",		0,	100,    &GameExtraManager_proxy.ProbLaserUpgrade1,      SetGameExtraManagerIntProbLaserUpgrade1,	GetGameExtraManagerIntProbLaserUpgrade1	},	
{ 0x00, "prob.lupgrade2",		0,	100,    &GameExtraManager_proxy.ProbLaserUpgrade2,      SetGameExtraManagerIntProbLaserUpgrade2,	GetGameExtraManagerIntProbLaserUpgrade2	},	
																															
{ 0x00, "prob.misspack",		0,	100,    &GameExtraManager_proxy.ProbMissilePack,        SetGameExtraManagerIntProbMissilePack,		GetGameExtraManagerIntProbMissilePack	},	
{ 0x00, "prob.dumb",			0,	100,    &GameExtraManager_proxy.ProbDumbMissPack,       SetGameExtraManagerIntProbDumbMissPack,		GetGameExtraManagerIntProbDumbMissPack	},	
{ 0x00, "prob.guide",			0,	100,    &GameExtraManager_proxy.ProbHomMissPack,        SetGameExtraManagerIntProbHomMissPack,		GetGameExtraManagerIntProbHomMissPack	},	
{ 0x00, "prob.swarm",			0,	100,    &GameExtraManager_proxy.ProbSwarmMissPack,      SetGameExtraManagerIntProbSwarmMissPack,	GetGameExtraManagerIntProbSwarmMissPack	},	
																															
{ 0x00, "prob.empupgrade1",		0,	100,    &GameExtraManager_proxy.ProbEmpUpgrade1,        SetGameExtraManagerIntProbEmpUpgrade1,		GetGameExtraManagerIntProbEmpUpgrade1	},	
{ 0x00, "prob.empupgrade2",		0,	100,    &GameExtraManager_proxy.ProbEmpUpgrade2,        SetGameExtraManagerIntProbEmpUpgrade2,		GetGameExtraManagerIntProbEmpUpgrade2	},	
};

#define NUM_INT_COMMANDS	CALC_NUM_ARRAY_ENTRIES( int_commands_default )


// pointer to current integer variable commands list --------------------------
//
PUBLIC	int_command_s *int_commands		= int_commands_default;


// current number of integer variable commands --------------------------------
//
PUBLIC	int num_int_commands			= NUM_INT_COMMANDS;
PRIVATE	int max_int_commands			= NUM_INT_COMMANDS;


// register a new (user-defined) integer variable command ---------------------
//
void CON_RegisterIntCommand( int_command_s *regcom )
{
	//NOTE:
	//CAVEAT:
	// the supplied command string is not copied
	// by this function. thus, the caller MUST ENSURE
	// that this string is available indefinitely.
	// (e.g., allocated statically.)

	ASSERT( regcom != NULL );
	ASSERT( regcom->command != NULL );
	ASSERT( regcom->intref != NULL );
	ASSERT( regcom->bmin <= regcom->bmax );
	ASSERT( num_int_commands <= max_int_commands );

	// expand table memory if already used up
	if ( num_int_commands == max_int_commands ) {

		// expand exponentially
		max_int_commands *= 2;

		// alloc new table
		int_command_s *newlist = (int_command_s *) ALLOCMEM( sizeof( int_command_s ) * max_int_commands );
		ASSERT( newlist != NULL );

		// move old table
		memcpy( newlist, int_commands, sizeof( int_command_s ) * num_int_commands );
		if ( int_commands != int_commands_default )
			FREEMEM( int_commands );
		int_commands = newlist;
	}

	// append new command
	ASSERT( num_int_commands < max_int_commands );
	int_commands[ num_int_commands++ ] = *regcom;
}

