/*
 * PARSEC - Global Variables
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:25 $
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

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"
#include "od_props.h"

// local module header
#include "g_global.h"


// firing control --------------------------------------------------------
int 	FireDisable 		= 1;
int 	FireRepeat			= 1;
int 	CurGun				= 0;
int 	MissileDisable		= 1;
int 	CurLauncher 		= 0;

// set when in floating menu ---------------------------------------------
int		InFloatingMenu		= FALSE;

// set when in starmap ---------------------------------------------------
int 	InStarMap			= FALSE;

// set when in gameloop --------------------------------------------------
int		InGameLoop			= FALSE;

// if set gameloop is exited automatically -------------------------------
int		ExitGameLoop		= FALSE;

// current play position in audio stream ---------------------------------
dword	CurStreamReplayPos  = 0;

// options menu selections (without video options) -----------------------
int 	Op_DetailLevel		= DETAIL_HIGH;
int 	Op_PacketSendFreq	= DETAIL_HIGH;
int 	Op_FixFrameRate 	= 1;
int 	Op_SoundEffects 	= 0;
int		Op_Music			= 0;
int 	Op_Joystick 		= 0;
int 	Op_Mouse			= 0;

// help active? ----------------------------------------------------------
int 	HelpActive			= 0;

// targeting information -------------------------------------------------
dword	TargetObjNumber 	= TARGETID_NO_TARGET;
int 	TargetVisible		= 0;
int 	TargetLocked		= 0;
int 	TargetRemId 		= TARGETID_NO_TARGET;
dword	TargetScreenX;
dword	TargetScreenY;

// counters for concurrently displayed laser objects ---------------------
int 	MaxNumShots			= 0;
int 	NumShots			= 0;

// counters for concurrently displayed missile objects -------------------
int 	MaxNumMissiles		= 0;
int 	NumMissiles 		= 0;

// laser type currently selected by user ---------------------------------
int 	SelectedLaser		= 0;

// missile type currently selected by user -------------------------------
int 	SelectedMissile		= 0;

// current target hit (counts down from maximum to zero after event) -----
int		HitCurTarget		= 0;

// missile has locked onto local ship (counts down after event) ----------
int		IncomingMissile		= 0;


// game control data -----------------------------------------------------

//FIXME: GAMECODE !!!!

int		EnergyExtraBoost	= ENERGY_EXTRA_BOOST;
int 	RepairExtraBoost	= 0;

int 	DumbPackNumMissls	= DUMB_PACK_NUMMISSLS;
int 	HomPackNumMissls	= HOM_PACK_NUMMISSLS;
int 	SwarmPackNumMissls	= SWARM_PACK_NUMMISSLS;
int 	ProxPackNumMines	= PROX_PACK_NUMMINES;

int 	ExtraProbability	= EXTRA_PROBABILITY;
int 	MaxExtraArea		= MAX_EXTRA_AREA;
int 	MinExtraDist		= MIN_EXTRA_DIST;

int 	ProbHelixCannon		= PROB_DAZZLE_LASER;
int 	ProbLightningDevice	= PROB_THIEF_LASER;
int 	ProbPhotonCannon	= 0;

int 	ProbProximityMine	= PROB_PROXIMITY_MINE;

int 	ProbMissilePack		= PROB_MISSILE_PACK;
int 	ProbDumbMissPack	= PROB_DUMB_MISS_PACK;
int 	ProbHomMissPack		= PROB_HOM_MISS_PACK;
int 	ProbSwarmMissPack	= 0;

int 	ProbRepairExtra		= 0;
int 	ProbAfterburner		= 0;
int		ProbHoloDecoy		= 0;
int 	ProbInvisibility	= 0;
int 	ProbInvulnerability	= 0;
int 	ProbEnergyField		= 0;

int 	ProbLaserUpgrade 	= 0;
int 	ProbLaserUpgrade1	= 0;
int 	ProbLaserUpgrade2	= 0;

int 	ProbEmpUpgrade1		= 0;
int 	ProbEmpUpgrade2		= 0;

int 	MegaShieldStrength 	= MEGASHIELD_STRENGTH;


// transformation matrix for pseudo stars --------------------------------
ALLOC_DESTXMATRX( PseudoStarMovement );

// positions of pseudo and fixed stars -----------------------------------
pseudostar_s	PseudoStars[ MAX_PSEUDO_STARS ];
int 			NumPseudoStars	= 0;
int				MaxPseudoStars	= 150;
fixedstar_s		FixedStars[ MAX_FIXED_STARS ];
int 			NumFixedStars	= 0;



