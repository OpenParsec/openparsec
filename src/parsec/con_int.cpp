/*
 * PARSEC - Integer Variable Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:23 $
 *
 * Orginally written by:
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

// subsystem headers
#include "net_defs.h"
#include "sys_defs.h"
#include "vid_defs.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "con_int.h"

// proprietary module headers
#include "con_main.h"
#include "e_record.h"
#include "g_stars.h"
#include "net_rmev.h"


// integer input/output configuration -----------------------------------------
//
PUBLIC	const char*	int_print_base = "%d";
PUBLIC	int		int_calc_base  = 10;


// set fixed frame rate -------------------------------------------------------
//
PRIVATE
void SetFixedRate()
{
	FixedFrameRateVal = FRAME_MEASURE_TIMEBASE / FixedFrameRate;
}


// intermediate gamma correction value (int) ----------------------------------
//
PRIVATE
int int_gamma = 100;


// set gamma correction value -------------------------------------------------
//
PRIVATE
void SetGamma()
{
	GammaCorrection = (float)int_gamma / 100;

	if ( VIDs_SetGammaCorrection( GammaCorrection ) ) {

		char ostr[40];
		sprintf( ostr, "gamma correction set to %.2f", GammaCorrection );
		CON_AddLine( ostr );

	} else {

		CON_AddLine( "gamma correction not supported by current video subsystem." );
	}
}


// set maximum number of pseudo stars -----------------------------------------
//
PRIVATE
void SetMaxPseudoStars()
{
	NumPseudoStars = 0;
	InitPseudoStars();
}


// intermediate refframe frequency --------------------------------------------
//
PRIVATE
int int_refframefrequency;


// retrieve current refframe frequency ----------------------------------------
//
PRIVATE
int GetRefFrameFrequency()
{
	return RefFrameFrequency;
}


// set new refframe frequency -------------------------------------------------
//
PRIVATE
void SetRefFrameFrequency()
{
	if ( int_refframefrequency != RefFrameFrequency ) {

		RefFrameFrequency = int_refframefrequency;

		// reinit frame timer to apply frequency change
		SYSs_InitFrameTimer();
	}
}


// external variables which need to be accessed but are not in GLOBALS.H ------
//
extern int	ConsoleEnterLength;
extern int	ConsoleTextX;
extern int	ConsoleTextY;
extern int	EchoExternalCommands;
extern int	HistoryForBatchEntry;
extern int	timedemo_enabled;


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

DEF_MYSHIP_INT( Weapons );
DEF_MYSHIP_INT( Specials );
DEF_MYSHIP_INT( CurDamage );
DEF_MYSHIP_INT( CurEnergy );
DEF_MYSHIP_INT( CurSpeed );
DEF_MYSHIP_INT( NumMissls );
DEF_MYSHIP_INT( NumHomMissls );
DEF_MYSHIP_INT( NumPartMissls );
DEF_MYSHIP_INT( NumMines );
DEF_MYSHIP_INT( MaxDamage );
DEF_MYSHIP_INT( MaxEnergy );
DEF_MYSHIP_INT( MaxSpeed );
DEF_MYSHIP_INT( MaxNumMissls );
DEF_MYSHIP_INT( MaxNumHomMissls );
DEF_MYSHIP_INT( MaxNumPartMissls );
DEF_MYSHIP_INT( MaxNumMines );


// perform remote event syncing for certain int vars --------------------------
//
#define DEF_RMEV_STATESYNC( f, k ) \
void RESync##f() \
{ \
	if ( !NET_RmEvAllowed( RE_STATESYNC ) ) \
		return; \
	NET_RmEvStateSync( (k), (f) ); \
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


// integer variable manipulation commands -------------------------------------
//
PRIVATE
int_command_s int_commands_default[] = {

//CAVEAT:
// absolute and relative positions of the following
// 4 commands MUST NOT be altered. the CONINFOINDX_xx
// constants in CON_INT.H must be defined consistently.

	{ 0x01, "con.winx",				1,	489,	&ConsoleTextX,				CheckConExtents,    		NULL	},
	{ 0x01, "con.winy",				1,	295,	&ConsoleTextY,				CheckConExtents,    		NULL	},
	{ 0x01, "con.width",			11,	500,	&ConsoleEnterLength,		CheckConExtents,     		NULL	},
	{ 0x01, "con.height",			5,	300,	&ConsoleHeight,				CheckConExtents,			NULL	},

	{ 0x01, "con.echoextcoms",		0,	1,		&EchoExternalCommands,		NULL,             			NULL	},
	{ 0x01, "con.scripthistory",	0,	1,		&HistoryForBatchEntry,		NULL,             			NULL	},

	{ 0x01, "hud.showrate",			0,	1,      &ShowFrameRate,          	NULL,                		NULL	},

	{ 0x01, "gfx.lensflare",		0,	1,      &DoLensFlare,				NULL,                		NULL	},
	{ 0x01, "gfx.numstars",			0,	MAX_PSEUDO_STARS, &MaxPseudoStars,	SetMaxPseudoStars,			NULL	},
	{ 0x00, "gfx.framepolys",		0,	0,		&NumRenderedPolygons,		NULL,             			NULL	},

	{ 0x01, "net.interface",		0,	10,		&NetInterfaceSelect,		NULL,             			NULL	},
	{ 0x01, "net.packetrecording",	0,	1,		&RecordRemotePackets,		NULL,             			NULL	},
	{ 0x01, "net.recsessionid",		0,	999,	&RemoteRecSessionId,		NULL,             			NULL	},

	{ 0x00, "sys.refframes",		1,	6000,	&int_refframefrequency,		SetRefFrameFrequency,		GetRefFrameFrequency	},

	{ 0x01, "vid.flipsync",			0,	1,      &FlipSynched,            	NULL,                		NULL	},
	{ 0x01, "vid.gamma",			0,	500,	&int_gamma,					SetGamma,        			NULL	},
	{ 0x01, "vid.ratefix",			0,	1,      &Op_FixFrameRate,        	NULL,                		NULL	},
	{ 0x01, "vid.fixedrate",		2,	300,    &FixedFrameRate,			SetFixedRate,        		NULL	},

	{ 0x00,	"timedemo",				0,	1,		&timedemo_enabled,			NULL,						NULL	},

#ifdef ENABLE_CHEAT_COMMANDS
	{ 0x00, ".weapons",				0,	0xfff,  &myship_int.Weapons,		SetMSIntWeapons,			GetMSIntWeapons			},
	{ 0x00, ".specials",			0,	0xfffff,&myship_int.Specials,		SetMSIntSpecials,			GetMSIntSpecials		},
	{ 0x00, ".damage",				0,	10000,  &myship_int.CurDamage,		SetMSIntCurDamage,			GetMSIntCurDamage		},
	{ 0x00, ".energy",				0,	10000,  &myship_int.CurEnergy,		SetMSIntCurEnergy,			GetMSIntCurEnergy		},
	{ 0x00, ".speed",				0,	500000, &myship_int.CurSpeed,		SetMSIntCurSpeed,			GetMSIntCurSpeed		},
	{ 0x00, ".dumb",				0,	10000,  &myship_int.NumMissls,		SetMSIntNumMissls,			GetMSIntNumMissls		},
	{ 0x00, ".guide",				0,	10000,  &myship_int.NumHomMissls,	SetMSIntNumHomMissls,		GetMSIntNumHomMissls	},
	{ 0x00, ".swarm",				0,	10000,  &myship_int.NumPartMissls,	SetMSIntNumPartMissls,		GetMSIntNumPartMissls	},
	{ 0x00, ".mines",				0,	10000,  &myship_int.NumMines,		SetMSIntNumMines,			GetMSIntNumMines		},
	{ 0x00, ".damage.max",			0,	10000,  &myship_int.MaxDamage,		SetMSIntMaxDamage,			GetMSIntMaxDamage		},
	{ 0x00, ".energy.max",			0,	10000,  &myship_int.MaxEnergy,		SetMSIntMaxEnergy,			GetMSIntMaxEnergy		},
	{ 0x00, ".speed.max",			0,	500000, &myship_int.MaxSpeed,		SetMSIntMaxSpeed,			GetMSIntMaxSpeed		},
	{ 0x00, ".dumb.max",			0,	10000,  &myship_int.MaxNumMissls,	SetMSIntMaxNumMissls,		GetMSIntMaxNumMissls	},
	{ 0x00, ".guide.max",			0,	10000,  &myship_int.MaxNumHomMissls,SetMSIntMaxNumHomMissls,	GetMSIntMaxNumHomMissls	},
	{ 0x00, ".swarm.max",			0,	10000,  &myship_int.MaxNumPartMissls, SetMSIntMaxNumPartMissls,	GetMSIntMaxNumPartMissls },
	{ 0x00, ".mines.max",			0,	10000,  &myship_int.MaxNumMines,	SetMSIntMaxNumMines,		GetMSIntMaxNumMines		},
#endif // ENABLE_CHEAT_COMMANDS

	{ 0x00, "extras.max",			0,	100,    &MaxNumExtras,           	NULL,                		NULL	},
	{ 0x00, "extras.area",			100,10000,  &MaxExtraArea,           	NULL,                		NULL	},
	{ 0x00, "extras.dist",			50,	1000,   &MinExtraDist,           	NULL,                		NULL	},

	{ 0x00, "energy.boost",			0,	1000,   &EnergyExtraBoost,       	NULL,                		NULL	},
	{ 0x00, "repair.boost",			0,	1000,   &RepairExtraBoost,			NULL,             			NULL	},

	{ 0x00, "pack.dumb.size",		0,	100,    &DumbPackNumMissls,      	NULL,                		NULL	},
	{ 0x00, "pack.guide.size",		0,	100,    &HomPackNumMissls,       	NULL,                		NULL	},
	{ 0x00, "pack.swarm.size",		0,	100,    &SwarmPackNumMissls,		NULL,             			NULL	},
	{ 0x00, "pack.mine.size",		0,	100,    &ProxPackNumMines,       	NULL,                		NULL	},

	{ 0x00, "prob.extra",			0,	100,    &ExtraProbability,       	RESyncExtraProbability,		NULL	},

	{ 0x00, "prob.helix",			0,	100,    &ProbHelixCannon,        	RESyncProbHelixCannon,		NULL	},
	{ 0x00, "prob.lightning",		0,	100,    &ProbLightningDevice,      	RESyncProbLightningDevice,	NULL	},
	{ 0x00, "prob.photon",			0,	100,    &ProbPhotonCannon,         	RESyncProbPhotonCannon,		NULL	},
	{ 0x00, "prob.mine",			0,	100,    &ProbProximityMine,      	RESyncProbProximityMine,	NULL	},
	{ 0x00, "prob.repair",			0,	100,    &ProbRepairExtra,        	RESyncProbRepairExtra,		NULL	},
	{ 0x00, "prob.aburner",			0,	100,    &ProbAfterburner,        	RESyncProbAfterburner,		NULL	},
	{ 0x00, "prob.decoy",			0,	100,    &ProbHoloDecoy,	        	RESyncProbHoloDecoy,		NULL	},
	{ 0x00, "prob.invisible",		0,	100,    &ProbInvisibility,       	RESyncProbInvisibility,		NULL	},
	{ 0x00, "prob.invulnerable",	0,	100,    &ProbInvulnerability,    	RESyncProbInvulnerability,	NULL	},
	{ 0x00, "prob.energyfield",		0,	100,    &ProbEnergyField,        	RESyncProbEnergyField,		NULL	},

	{ 0x00, "prob.lupgrade",		0,	100,    &ProbLaserUpgrade,       	RESyncProbLaserUpgrade,		NULL	},
	{ 0x00, "prob.lupgrade1",		0,	100,    &ProbLaserUpgrade1,      	RESyncProbLaserUpgrade1,	NULL	},
	{ 0x00, "prob.lupgrade2",		0,	100,    &ProbLaserUpgrade2,      	RESyncProbLaserUpgrade2,	NULL	},

	{ 0x00, "prob.misspack",		0,	100,    &ProbMissilePack,        	RESyncProbMissilePack,		NULL	},
	{ 0x00, "prob.dumb",			0,	100,    &ProbDumbMissPack,       	RESyncProbDumbMissPack,		NULL	},
	{ 0x00, "prob.guide",			0,	100,    &ProbHomMissPack,        	RESyncProbHomMissPack,		NULL	},
	{ 0x00, "prob.swarm",			0,	100,    &ProbSwarmMissPack,        	RESyncProbSwarmMissPack,	NULL	},

	{ 0x00, "prob.empupgrade1",		0,	100,    &ProbEmpUpgrade1,        	RESyncProbEmpUpgrade1,		NULL	},
	{ 0x00, "prob.empupgrade2",		0,	100,    &ProbEmpUpgrade2,        	RESyncProbEmpUpgrade2,		NULL	},
/*
	{ 0x00, "intro.startwait",		0,	100000,	&intro_start_wait,			NULL,             			NULL	},
	{ 0x00, "intro.animspeed",		0,	100000,	&intro_anim_speed,			NULL,             			NULL	},
	{ 0x00, "intro.afterwait",		0,	100000,	&intro_after_wait,			NULL,             			NULL	},
	{ 0x00, "intro.goonwait", 		0,	100000,	&intro_go_on_wait,			NULL,             			NULL	},
	{ 0x00, "intro.frame1",			0,	100000,	&intro_frame_part1,			NULL,             			NULL	},
	{ 0x00, "intro.waitp1",			0,	100000,	&intro_wait_part1, 			NULL,             			NULL	},
	{ 0x00, "intro.frame2",			0,	100000,	&intro_frame_part2,			NULL,             			NULL	},
	{ 0x00, "intro.waitp2",			0,	100000,	&intro_wait_part2,			NULL,             			NULL	},
	{ 0x00, "intro.animspeed2",		0,	100000,	&intro_anim_speed2,			NULL,             			NULL	},
	{ 0x00, "intro.animspeed3",		0,	100000,	&intro_anim_speed3,			NULL,             			NULL	},
*/
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

	if ( ( regcom->bmin <= regcom->default_val ) && ( regcom->bmax >= regcom->default_val ) ) {
		*regcom->intref = regcom->default_val;

		if ( regcom->realize != NULL )
			regcom->realize();
	} else {
		regcom->default_val = 0;
	}

	// append new command
	ASSERT( num_int_commands < max_int_commands );
	int_commands[ num_int_commands++ ] = *regcom;
}



