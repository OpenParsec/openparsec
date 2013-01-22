/*
 * PARSEC - Server Object Info Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:47 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
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
#include <ctype.h>
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
#include "od_class.h"
#include "od_props.h"

// global externals
#include "globals.h"
#include "e_world_trans.h"

// local module header
#include "con_info_sv.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux_sv.h"
#include "con_int_sv.h"
#include "con_main_sv.h"
#include "con_std_sv.h"
//#include "e_supp.h"
#include "obj_clas.h"
//#include "obj_comm.h"
//#include "obj_ctrl.h"
#include "obj_cust.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 1023
static char paste_str[ PASTE_STR_LEN + 1 ];


// string constants -----------------------------------------------------------
//
static char prop_syntax[]		= "syntax: prop <type_name>.<property> <value>";
static char propc_syntax[]		= "syntax: propc <class_name>.<property> <value>";
static char propo_syntax[]		= "syntax: propo <object_id>.<property> <value>";
static char prop_wrong_type[]	= "invalid type name.";
static char prop_wrong_class[]	= "invalid class name.";
static char prop_objid_syntax[]	= "object id must be an int.";
static char prop_no_object[]	= "no object with this id.";
static char prop_wrong_field[]	= "invalid property name.";
static char typeinfo_syntax[]	= "syntax: typeinfo <type_name>";
static char classinfo_syntax[]	= "syntax: classinfo <class_name>";
static char objectinfo_syntax[]	= "syntax: objectinfo <object_id>";
static char class_not_loaded[]	= "class is not loaded.";
static char no_objs_of_type[]	= "there are no objects of this type.";
static char not_accessible[]	= "properties of this type are not accessible.";
static char face_id_invalid[]	= "face id invalid.";
static char no_texture_found[]	= "texture invalid.";
static char no_face_selected[]	= "no face selection specified.";
static char base_object_empty[]	= "base object is empty.";


// property access type specification -----------------------------------------
//
#define ACCESSTYPE_INT			0	// access property value as int
#define ACCESSTYPE_FLOAT		1	// access property value as float
#define ACCESSTYPE_STRING		2	// access property value as char[]
#define ACCESSTYPE_CHARPTR		3	// access property value as char*


// property lists for default types -------------------------------------------
//
#define Ship1Obj_CurDamage				offsetof( ShipObject, CurDamage )
#define Ship1Obj_MaxDamage				offsetof( ShipObject, MaxDamage )
#define Ship1Obj_Energy 				offsetof( ShipObject, CurEnergy )
#define Ship1Obj_MaxEnergy				offsetof( ShipObject, MaxEnergy )
#define Ship1Obj_CurSpeed				offsetof( ShipObject, CurSpeed )
#define Ship1Obj_MaxSpeed				offsetof( ShipObject, MaxSpeed )
#define Ship1Obj_Weapons				offsetof( ShipObject, Weapons )
#define Ship1Obj_Specials				offsetof( ShipObject, Specials )
#define Ship1Obj_NumMissls				offsetof( ShipObject, NumMissls )
#define Ship1Obj_MaxNumMissls			offsetof( ShipObject, MaxNumMissls )
#define Ship1Obj_NumHomMissls			offsetof( ShipObject, NumHomMissls )
#define Ship1Obj_MaxNumHomMissls		offsetof( ShipObject, MaxNumHomMissls )
#define Ship1Obj_NumPartMissls			offsetof( ShipObject, NumPartMissls )
#define Ship1Obj_MaxNumPartMissls		offsetof( ShipObject, MaxNumPartMissls )
#define Ship1Obj_NumMines				offsetof( ShipObject, NumMines )
#define Ship1Obj_MaxNumMines			offsetof( ShipObject, MaxNumMines )
#define Ship1Obj_YawPerRefFrame			offsetof( ShipObject, YawPerRefFrame )
#define Ship1Obj_PitchPerRefFrame		offsetof( ShipObject, PitchPerRefFrame )
#define Ship1Obj_RollPerRefFrame		offsetof( ShipObject, RollPerRefFrame )
#define Ship1Obj_XSlidePerRefFrame		offsetof( ShipObject, XSlidePerRefFrame )
#define Ship1Obj_YSlidePerRefFrame		offsetof( ShipObject, YSlidePerRefFrame )
#define Ship1Obj_SpeedIncPerRefFrame	offsetof( ShipObject, SpeedIncPerRefFrame )
#define Ship1Obj_SpeedDecPerRefFrame	offsetof( ShipObject, SpeedDecPerRefFrame )
#define Ship1Obj_FireRepeatDelay		offsetof( ShipObject, FireRepeatDelay )
#define Ship1Obj_FireDisableDelay		offsetof( ShipObject, FireDisableDelay )
#define Ship1Obj_MissileDisableDelay	offsetof( ShipObject, MissileDisableDelay )
#define Ship1Obj_Laser1X1				offsetof( ShipObject, Laser1_X )
#define Ship1Obj_Laser1X2				( offsetof( ShipObject, Laser1_X ) + sizeof( geomv_t ) * 1 )
#define Ship1Obj_Laser1X3				( offsetof( ShipObject, Laser1_X ) + sizeof( geomv_t ) * 2 )
#define Ship1Obj_Laser1X4				( offsetof( ShipObject, Laser1_X ) + sizeof( geomv_t ) * 3 )
#define Ship1Obj_Laser2X1				( offsetof( ShipObject, Laser1_X ) + sizeof( geomv_t ) * 4 )
#define Ship1Obj_Laser2X2				( offsetof( ShipObject, Laser1_X ) + sizeof( geomv_t ) * 5 )
#define Ship1Obj_Laser2X3				( offsetof( ShipObject, Laser1_X ) + sizeof( geomv_t ) * 6 )
#define Ship1Obj_Laser2X4				( offsetof( ShipObject, Laser1_X ) + sizeof( geomv_t ) * 7 )
#define Ship1Obj_Laser3X1				( offsetof( ShipObject, Laser1_X ) + sizeof( geomv_t ) * 8 )
#define Ship1Obj_Laser3X2				( offsetof( ShipObject, Laser1_X ) + sizeof( geomv_t ) * 9 )
#define Ship1Obj_Laser3X3				( offsetof( ShipObject, Laser1_X ) + sizeof( geomv_t ) * 10 )
#define Ship1Obj_Laser3X4				( offsetof( ShipObject, Laser1_X ) + sizeof( geomv_t ) * 11 )
#define Ship1Obj_Laser4X1				( offsetof( ShipObject, Laser1_X ) + sizeof( geomv_t ) * 12 )
#define Ship1Obj_Laser4X2				( offsetof( ShipObject, Laser1_X ) + sizeof( geomv_t ) * 13 )
#define Ship1Obj_Laser4X3				( offsetof( ShipObject, Laser1_X ) + sizeof( geomv_t ) * 14 )
#define Ship1Obj_Laser4X4				( offsetof( ShipObject, Laser1_X ) + sizeof( geomv_t ) * 15 )
#define Ship1Obj_Laser1Y1				offsetof( ShipObject, Laser1_Y )
#define Ship1Obj_Laser1Y2				( offsetof( ShipObject, Laser1_Y ) + sizeof( geomv_t ) * 1 )
#define Ship1Obj_Laser1Y3				( offsetof( ShipObject, Laser1_Y ) + sizeof( geomv_t ) * 2 )
#define Ship1Obj_Laser1Y4				( offsetof( ShipObject, Laser1_Y ) + sizeof( geomv_t ) * 3 )
#define Ship1Obj_Laser2Y1				( offsetof( ShipObject, Laser1_Y ) + sizeof( geomv_t ) * 4 )
#define Ship1Obj_Laser2Y2				( offsetof( ShipObject, Laser1_Y ) + sizeof( geomv_t ) * 5 )
#define Ship1Obj_Laser2Y3				( offsetof( ShipObject, Laser1_Y ) + sizeof( geomv_t ) * 6 )
#define Ship1Obj_Laser2Y4				( offsetof( ShipObject, Laser1_Y ) + sizeof( geomv_t ) * 7 )
#define Ship1Obj_Laser3Y1				( offsetof( ShipObject, Laser1_Y ) + sizeof( geomv_t ) * 8 )
#define Ship1Obj_Laser3Y2				( offsetof( ShipObject, Laser1_Y ) + sizeof( geomv_t ) * 9 )
#define Ship1Obj_Laser3Y3				( offsetof( ShipObject, Laser1_Y ) + sizeof( geomv_t ) * 10 )
#define Ship1Obj_Laser3Y4				( offsetof( ShipObject, Laser1_Y ) + sizeof( geomv_t ) * 11 )
#define Ship1Obj_Laser4Y1				( offsetof( ShipObject, Laser1_Y ) + sizeof( geomv_t ) * 12 )
#define Ship1Obj_Laser4Y2				( offsetof( ShipObject, Laser1_Y ) + sizeof( geomv_t ) * 13 )
#define Ship1Obj_Laser4Y3				( offsetof( ShipObject, Laser1_Y ) + sizeof( geomv_t ) * 14 )
#define Ship1Obj_Laser4Y4				( offsetof( ShipObject, Laser1_Y ) + sizeof( geomv_t ) * 15 )
#define Ship1Obj_Laser1Z1				offsetof( ShipObject, Laser1_Z )
#define Ship1Obj_Laser1Z2				( offsetof( ShipObject, Laser1_Z ) + sizeof( geomv_t ) * 1 )
#define Ship1Obj_Laser1Z3				( offsetof( ShipObject, Laser1_Z ) + sizeof( geomv_t ) * 2 )
#define Ship1Obj_Laser1Z4				( offsetof( ShipObject, Laser1_Z ) + sizeof( geomv_t ) * 3 )
#define Ship1Obj_Laser2Z1				( offsetof( ShipObject, Laser1_Z ) + sizeof( geomv_t ) * 4 )
#define Ship1Obj_Laser2Z2				( offsetof( ShipObject, Laser1_Z ) + sizeof( geomv_t ) * 5 )
#define Ship1Obj_Laser2Z3				( offsetof( ShipObject, Laser1_Z ) + sizeof( geomv_t ) * 6 )
#define Ship1Obj_Laser2Z4				( offsetof( ShipObject, Laser1_Z ) + sizeof( geomv_t ) * 7 )
#define Ship1Obj_Laser3Z1				( offsetof( ShipObject, Laser1_Z ) + sizeof( geomv_t ) * 8 )
#define Ship1Obj_Laser3Z2				( offsetof( ShipObject, Laser1_Z ) + sizeof( geomv_t ) * 9 )
#define Ship1Obj_Laser3Z3				( offsetof( ShipObject, Laser1_Z ) + sizeof( geomv_t ) * 10 )
#define Ship1Obj_Laser3Z4				( offsetof( ShipObject, Laser1_Z ) + sizeof( geomv_t ) * 11 )
#define Ship1Obj_Laser4Z1				( offsetof( ShipObject, Laser1_Z ) + sizeof( geomv_t ) * 12 )
#define Ship1Obj_Laser4Z2				( offsetof( ShipObject, Laser1_Z ) + sizeof( geomv_t ) * 13 )
#define Ship1Obj_Laser4Z3				( offsetof( ShipObject, Laser1_Z ) + sizeof( geomv_t ) * 14 )
#define Ship1Obj_Laser4Z4				( offsetof( ShipObject, Laser1_Z ) + sizeof( geomv_t ) * 15 )
#define Ship1Obj_Missile1X1				offsetof( ShipObject, Missile1_X )
#define Ship1Obj_Missile1X2				( offsetof( ShipObject, Missile1_X ) + sizeof( geomv_t ) * 1 )
#define Ship1Obj_Missile1X3				( offsetof( ShipObject, Missile1_X ) + sizeof( geomv_t ) * 2 )
#define Ship1Obj_Missile1X4				( offsetof( ShipObject, Missile1_X ) + sizeof( geomv_t ) * 3 )
#define Ship1Obj_Missile1Y1				offsetof( ShipObject, Missile1_Y )
#define Ship1Obj_Missile1Y2				( offsetof( ShipObject, Missile1_Y ) + sizeof( geomv_t ) * 1 )
#define Ship1Obj_Missile1Y3				( offsetof( ShipObject, Missile1_Y ) + sizeof( geomv_t ) * 2 )
#define Ship1Obj_Missile1Y4				( offsetof( ShipObject, Missile1_Y ) + sizeof( geomv_t ) * 3 )
#define Ship1Obj_Missile1Z1				offsetof( ShipObject, Missile1_Z )
#define Ship1Obj_Missile1Z2				( offsetof( ShipObject, Missile1_Z ) + sizeof( geomv_t ) * 1 )
#define Ship1Obj_Missile1Z3				( offsetof( ShipObject, Missile1_Z ) + sizeof( geomv_t ) * 2 )
#define Ship1Obj_Missile1Z4				( offsetof( ShipObject, Missile1_Z ) + sizeof( geomv_t ) * 3 )
#define Ship1Obj_Missile2X1				offsetof( ShipObject, Missile2_X )
#define Ship1Obj_Missile2X2				( offsetof( ShipObject, Missile2_X ) + sizeof( geomv_t ) * 1 )
#define Ship1Obj_Missile2X3				( offsetof( ShipObject, Missile2_X ) + sizeof( geomv_t ) * 2 )
#define Ship1Obj_Missile2X4				( offsetof( ShipObject, Missile2_X ) + sizeof( geomv_t ) * 3 )
#define Ship1Obj_Missile2Y1				offsetof( ShipObject, Missile2_Y )
#define Ship1Obj_Missile2Y2				( offsetof( ShipObject, Missile2_Y ) + sizeof( geomv_t ) * 1 )
#define Ship1Obj_Missile2Y3				( offsetof( ShipObject, Missile2_Y ) + sizeof( geomv_t ) * 2 )
#define Ship1Obj_Missile2Y4				( offsetof( ShipObject, Missile2_Y ) + sizeof( geomv_t ) * 3 )
#define Ship1Obj_Missile2Z1				offsetof( ShipObject, Missile2_Z )
#define Ship1Obj_Missile2Z2				( offsetof( ShipObject, Missile2_Z ) + sizeof( geomv_t ) * 1 )
#define Ship1Obj_Missile2Z3				( offsetof( ShipObject, Missile2_Z ) + sizeof( geomv_t ) * 2 )
#define Ship1Obj_Missile2Z4				( offsetof( ShipObject, Missile2_Z ) + sizeof( geomv_t ) * 3 )
#define Ship1Obj_Mine1X					offsetof( ShipObject, Mine1_X )
#define Ship1Obj_Mine1Y					offsetof( ShipObject, Mine1_Y )
#define Ship1Obj_Mine1Z					offsetof( ShipObject, Mine1_Z )
#define Ship1Obj_BeamX1					offsetof( ShipObject, Beam_X )
#define Ship1Obj_BeamX2					( offsetof( ShipObject, Beam_X ) + sizeof( geomv_t ) * 1 )
#define Ship1Obj_BeamX3					( offsetof( ShipObject, Beam_X ) + sizeof( geomv_t ) * 2 )
#define Ship1Obj_BeamX4					( offsetof( ShipObject, Beam_X ) + sizeof( geomv_t ) * 3 )
#define Ship1Obj_BeamY					offsetof( ShipObject, Beam_Y )
#define Ship1Obj_BeamZ					offsetof( ShipObject, Beam_Z )
#define Ship1Obj_SpreadX1				offsetof( ShipObject, Spread_X )
#define Ship1Obj_SpreadX2				( offsetof( ShipObject, Spread_X ) + sizeof( geomv_t ) * 1 )
#define Ship1Obj_SpreadX3				( offsetof( ShipObject, Spread_X ) + sizeof( geomv_t ) * 2 )
#define Ship1Obj_SpreadX4				( offsetof( ShipObject, Spread_X ) + sizeof( geomv_t ) * 3 )
#define Ship1Obj_SpreadY				offsetof( ShipObject, Spread_Y )
#define Ship1Obj_SpreadZ				offsetof( ShipObject, Spread_Z )
#define Ship1Obj_SpreadSpeed			offsetof( ShipObject, SpreadSpeed )
#define Ship1Obj_SpreadLifeTime			offsetof( ShipObject, SpreadLifeTime )
#define Ship1Obj_HelixX					offsetof( ShipObject, Helix_X )
#define Ship1Obj_HelixY					offsetof( ShipObject, Helix_Y )
#define Ship1Obj_HelixZ					offsetof( ShipObject, Helix_Z )
#define Ship1Obj_HelixSpeed				offsetof( ShipObject, HelixSpeed )
#define Ship1Obj_HelixLifeTime			offsetof( ShipObject, HelixLifeTime )
#define Ship1Obj_PhotonSpeed            offsetof( ShipObject, PhotonSpeed )
#define Ship1Obj_PhotonLifeTime         offsetof( ShipObject, PhotonLifeTime )


#define Laser1_LifeTimeCount			offsetof( LaserObject, LifeTimeCount )
#define Laser1_Speed					offsetof( LaserObject, Speed )
#define Laser1_HitPoints				offsetof( LaserObject, HitPoints )
#define Laser1_EnergyNeeded 			offsetof( LaserObject, EnergyNeeded )

#define Missile1_LifeTimeCount			offsetof( MissileObject, LifeTimeCount )
#define Missile1_Speed					offsetof( MissileObject, Speed )
#define Missile1_HitPoints				offsetof( MissileObject, HitPoints )
#define Missile4_Latency				offsetof( TargetMissileObject, Latency )
#define Missile4_MaxRotation			offsetof( TargetMissileObject, MaxRotation )

#define Extra1_LifeTimeCount			offsetof( ExtraObject, LifeTimeCount )
#define Extra1_SelfRotX 				offsetof( ExtraObject, SelfRotX )
#define Extra1_SelfRotY 				offsetof( ExtraObject, SelfRotY )
#define Extra1_SelfRotZ 				offsetof( ExtraObject, SelfRotZ )

#define Mine1_HitPoints 				offsetof( MineObject, HitPoints )


//NOTE:
// bmin and bmax (value bounds) for all types other than
// PROPTYPE_INT have to be specified in fixed-point!

//NOTE:
// not all fields of struct proplist_s will be set explicitly
// in the following arrays. the remaining fields will be NULL.
// THEIR OFF-STATES HAVE TO BE NULL/ZERO.


PRIVATE
proplist_s Ship1_PropList[] = {

	{ "damage",			Ship1Obj_CurDamage,         	1,	500,		PROPTYPE_INT	},
	{ "maxdamage",      Ship1Obj_MaxDamage,         	1,	500,		PROPTYPE_INT	},
	{ "energy",         Ship1Obj_Energy,            	1,	10000,		PROPTYPE_INT	},
	{ "maxenergy",      Ship1Obj_MaxEnergy,         	1,	10000,		PROPTYPE_INT	},
	{ "speed",			Ship1Obj_CurSpeed,          	1,	100000,		PROPTYPE_INT	},
	{ "maxspeed",       Ship1Obj_MaxSpeed,          	1,	100000,		PROPTYPE_INT	},
	{ "weapons",        Ship1Obj_Weapons,           	0,	0x70007,	PROPTYPE_INT	},
	{ "specials",		Ship1Obj_Specials,				0,	0xfffff,	PROPTYPE_INT	},
	{ "dumb",           Ship1Obj_NumMissls,         	0,	99,			PROPTYPE_INT	},
	{ "maxdumb",        Ship1Obj_MaxNumMissls,      	0,	99,			PROPTYPE_INT	},
	{ "guide",          Ship1Obj_NumHomMissls,      	0,	99,			PROPTYPE_INT	},
	{ "maxguide",       Ship1Obj_MaxNumHomMissls,		0,	99,			PROPTYPE_INT	},
	{ "swarm",          Ship1Obj_NumPartMissls,      	0,	99,			PROPTYPE_INT	},
	{ "maxswarm",       Ship1Obj_MaxNumPartMissls,		0,	99,			PROPTYPE_INT	},
	{ "mines",          Ship1Obj_NumMines,          	0,	99,			PROPTYPE_INT	},
	{ "maxmines",       Ship1Obj_MaxNumMines,			0,	99,			PROPTYPE_INT	},
	{ "yawperref",		Ship1Obj_YawPerRefFrame,		0,	100,		PROPTYPE_INT	},
	{ "pitchperref",	Ship1Obj_PitchPerRefFrame,		0,	100,		PROPTYPE_INT	},
	{ "rollperref",		Ship1Obj_RollPerRefFrame,		0,	100,		PROPTYPE_INT	},
	{ "xslideperref",	Ship1Obj_XSlidePerRefFrame,		0,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "yslideperref",	Ship1Obj_YSlidePerRefFrame,		0,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "speedincperref",	Ship1Obj_SpeedIncPerRefFrame,	0,	0xffff,		PROPTYPE_INT	},
	{ "speeddecperref",	Ship1Obj_SpeedDecPerRefFrame,	0,	0xffff,		PROPTYPE_INT	},
	{ "firerepdelay",	Ship1Obj_FireRepeatDelay,		0,	500,		PROPTYPE_INT	},
	{ "firedisdelay",	Ship1Obj_FireDisableDelay,		0,	500,		PROPTYPE_INT	},
	{ "missdisdelay",	Ship1Obj_MissileDisableDelay,	0,	500,		PROPTYPE_INT	},
	{ "laser1.x1",      Ship1Obj_Laser1X1, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser1.x2",      Ship1Obj_Laser1X2, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser1.x3",      Ship1Obj_Laser1X3, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser1.x4",      Ship1Obj_Laser1X4, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser1.y1",      Ship1Obj_Laser1Y1, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser1.y2",      Ship1Obj_Laser1Y2, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser1.y3",      Ship1Obj_Laser1Y3, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser1.y4",      Ship1Obj_Laser1Y4, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser1.z1",      Ship1Obj_Laser1Z1, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser1.z2",      Ship1Obj_Laser1Z2, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser1.z3",      Ship1Obj_Laser1Z3, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser1.z4",      Ship1Obj_Laser1Z4, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser2.x1",      Ship1Obj_Laser2X1, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser2.x2",      Ship1Obj_Laser2X2, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser2.x3",      Ship1Obj_Laser2X3, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser2.x4",      Ship1Obj_Laser2X4, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser2.y1",      Ship1Obj_Laser2Y1, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser2.y2",      Ship1Obj_Laser2Y2, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser2.y3",      Ship1Obj_Laser2Y3, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser2.y4",      Ship1Obj_Laser2Y4, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser2.z1",      Ship1Obj_Laser2Z1, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser2.z2",      Ship1Obj_Laser2Z2, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser2.z3",      Ship1Obj_Laser2Z3, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser2.z4",      Ship1Obj_Laser2Z4, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.x1",      Ship1Obj_Laser3X1, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.x2",      Ship1Obj_Laser3X2, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.x3",      Ship1Obj_Laser3X3, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.x4",      Ship1Obj_Laser3X4, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.y1",      Ship1Obj_Laser3Y1, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.y2",      Ship1Obj_Laser3Y2, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.y3",      Ship1Obj_Laser3Y3, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.y4",      Ship1Obj_Laser3Y4, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.z1",      Ship1Obj_Laser3Z1, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.z2",      Ship1Obj_Laser3Z2, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.z3",      Ship1Obj_Laser3Z3, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.z4",      Ship1Obj_Laser3Z4, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.x1",      Ship1Obj_Laser3X1, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.x2",      Ship1Obj_Laser3X2, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.x3",      Ship1Obj_Laser3X3, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.x4",      Ship1Obj_Laser3X4, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.y1",      Ship1Obj_Laser3Y1, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.y2",      Ship1Obj_Laser3Y2, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.y3",      Ship1Obj_Laser3Y3, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.y4",      Ship1Obj_Laser3Y4, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.z1",      Ship1Obj_Laser3Z1, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.z2",      Ship1Obj_Laser3Z2, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.z3",      Ship1Obj_Laser3Z3, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser3.z4",      Ship1Obj_Laser3Z4, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser4.x1",      Ship1Obj_Laser4X1, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser4.x2",      Ship1Obj_Laser4X2, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser4.x3",      Ship1Obj_Laser4X3, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser4.x4",      Ship1Obj_Laser4X4, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser4.y1",      Ship1Obj_Laser4Y1, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser4.y2",      Ship1Obj_Laser4Y2, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser4.y3",      Ship1Obj_Laser4Y3, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser4.y4",      Ship1Obj_Laser4Y4, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser4.z1",      Ship1Obj_Laser4Z1, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser4.z2",      Ship1Obj_Laser4Z2, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser4.z3",      Ship1Obj_Laser4Z3, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "laser4.z4",      Ship1Obj_Laser4Z4, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "missile1.x1",    Ship1Obj_Missile1X1, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile1.x2",    Ship1Obj_Missile1X2, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile1.x3",    Ship1Obj_Missile1X3, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile1.x4",    Ship1Obj_Missile1X4, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile1.y1",    Ship1Obj_Missile1Y1, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile1.y2",    Ship1Obj_Missile1Y2, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile1.y3",    Ship1Obj_Missile1Y3, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile1.y4",    Ship1Obj_Missile1Y4, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile1.z1",    Ship1Obj_Missile1Z1, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile1.z2",    Ship1Obj_Missile1Z2, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile1.z3",    Ship1Obj_Missile1Z3, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile1.z4",    Ship1Obj_Missile1Z4, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile2.x1",    Ship1Obj_Missile2X1, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile2.x2",    Ship1Obj_Missile2X2, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile2.x3",    Ship1Obj_Missile2X3, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile2.x4",    Ship1Obj_Missile2X4, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile2.y1",    Ship1Obj_Missile2Y1, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile2.y2",    Ship1Obj_Missile2Y2, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile2.y3",    Ship1Obj_Missile2Y3, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile2.y4",    Ship1Obj_Missile2Y4, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile2.z1",    Ship1Obj_Missile2Z1, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile2.z2",    Ship1Obj_Missile2Z2, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile2.z3",    Ship1Obj_Missile2Z3, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "missile2.z4",    Ship1Obj_Missile2Z4, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "mine1.x",		Ship1Obj_Mine1X,  -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "mine1.y",		Ship1Obj_Mine1Y,  -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "mine1.z",     	Ship1Obj_Mine1Z,  -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "beam.x1",        Ship1Obj_BeamX1,  -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "beam.x2",        Ship1Obj_BeamX2,  -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "beam.x3",        Ship1Obj_BeamX3,  -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "beam.x4",        Ship1Obj_BeamX4,  -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "beam.y",         Ship1Obj_BeamY,   -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "beam.z",         Ship1Obj_BeamZ,   -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "spread.x1",      Ship1Obj_SpreadX1, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "spread.x2",      Ship1Obj_SpreadX2, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "spread.x3",      Ship1Obj_SpreadX3, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "spread.x4",      Ship1Obj_SpreadX4, -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "spread.y",       Ship1Obj_SpreadY,  -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "spread.z",       Ship1Obj_SpreadZ,  -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "spreadspeed",    Ship1Obj_SpreadSpeed,		0,	0x1000000,		PROPTYPE_INT	},
	{ "spreadlifetime", Ship1Obj_SpreadLifeTime,	0,	0x1000000,		PROPTYPE_INT	},
	{ "helix.x",		Ship1Obj_HelixX,  -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "helix.y",		Ship1Obj_HelixY,  -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "helix.z",		Ship1Obj_HelixZ,  -0x7fffffff,	0x7fffffff,		PROPTYPE_GEOMV	},
	{ "helixspeed",		Ship1Obj_HelixSpeed,		0,	0x1000000,		PROPTYPE_INT	},
	{ "helixlifetime",	Ship1Obj_HelixLifeTime,		0,	0x1000000,		PROPTYPE_INT	},
    { "photonspeed",    Ship1Obj_PhotonSpeed,       0,  0x1000000,      PROPTYPE_INT    },
    { "photonlifetime", Ship1Obj_PhotonLifeTime,    0,  0x1000000,      PROPTYPE_INT    },

	{ NULL,				0,						0,	0,			0	}
};

PRIVATE
proplist_s Laser1_PropList[] = {

	{ "lifetime",       Laser1_LifeTimeCount,	  100,	100000,		0	},
	{ "speed",          Laser1_Speed,               0,	0xa0000,	0	},
	{ "hitpoints",      Laser1_HitPoints,           0,	100,		0	},
	{ "energy",         Laser1_EnergyNeeded,        0,	100,		0	},

	{ NULL,				0,						0,	0,			0	}
};

PRIVATE
proplist_s Laser2_PropList[] = {

	{ "lifetime",       Laser1_LifeTimeCount,	  100,	100000,		0	},
	{ "speed",          Laser1_Speed,               0,	0xa0000,	0	},
	{ "hitpoints",      Laser1_HitPoints,           0,	200,		0	},
	{ "energy",         Laser1_EnergyNeeded,        0,	100,		0	},

	{ NULL,				0,						0,	0,			0	}
};

PRIVATE
proplist_s Laser3_PropList[] = {

	{ "lifetime",       Laser1_LifeTimeCount,	  100,	100000,		0	},
	{ "speed",          Laser1_Speed,               0,	0xa0000,	0	},
	{ "hitpoints",      Laser1_HitPoints,           0,	1000,		0	},
	{ "energy",         Laser1_EnergyNeeded,        0,	100,		0	},

	{ NULL,				0,						0,	0,			0	}
};

PRIVATE
proplist_s Missile1_PropList[] = {

	{ "lifetime",       Missile1_LifeTimeCount,	  100,	100000,		0	},
	{ "speed",          Missile1_Speed,				0,	0xa0000,	0	},
	{ "hitpoints",      Missile1_HitPoints,			0,	100,		0	},

	{ NULL,				0,						0,	0,			0	}
};

PRIVATE
proplist_s Missile2_PropList[] = {

	{ "lifetime",       Missile1_LifeTimeCount,	  100,	100000,		0	},
	{ "speed",          Missile1_Speed,         	0,	0xa0000,	0	},
	{ "hitpoints",      Missile1_HitPoints,     	0,	100,		0	},

	{ NULL,				0,						0,	0,			0	}
};

PRIVATE
proplist_s Missile3_PropList[] = {

	{ "lifetime",       Missile1_LifeTimeCount,	  100,	100000,		0	},
	{ "speed",          Missile1_Speed,         	0,	0xa0000,	0	},
	{ "hitpoints",      Missile1_HitPoints,     	0,	100,		0	},

	{ NULL,				0,						0,	0,			0	}
};

PRIVATE
proplist_s Missile4_PropList[] = {

	{ "lifetime",       Missile1_LifeTimeCount,	  100,	100000,		0	},
	{ "speed",          Missile1_Speed,         	0,	0xa0000,	0	},
	{ "hitpoints",      Missile1_HitPoints,     	0,	100,		0	},
	{ "latency",        Missile4_Latency,           0,	200,		0	},
	{ "maxrot",         Missile4_MaxRotation,       0,	0x10000,	0	},

	{ NULL,				0,						0,	0,			0	}
};

PRIVATE
proplist_s Missile5_PropList[] = {

	{ "lifetime",       Missile1_LifeTimeCount,	  100,	100000,		0	},
	{ "speed",          Missile1_Speed,         	0,	0xa0000,	0	},
	{ "hitpoints",      Missile1_HitPoints,     	0,	100,		0	},
	{ "latency",        Missile4_Latency,           0,	200,		0	},
	{ "maxrot",         Missile4_MaxRotation,       0,	0x10000,	0	},

	{ NULL,				0,						0,	0,			0	}
};

PRIVATE
proplist_s Extra1_PropList[] = {

	{ "lifetime",       Extra1_LifeTimeCount,		0,	100000,		0	},
	{ "rotx",           Extra1_SelfRotX,            0,	0x200,		0	},
	{ "roty",           Extra1_SelfRotY,            0,	0x200,		0	},
	{ "rotz",           Extra1_SelfRotZ,            0,	0x200,		0	},

	{ NULL,				0,						0,	0,			0	}
};

PRIVATE
proplist_s Extra2_PropList[] = {

	{ "lifetime",       Extra1_LifeTimeCount,		0,	100000,		0	},
	{ "rotx",           Extra1_SelfRotX,            0,	0x200,		0	},
	{ "roty",           Extra1_SelfRotY,            0,	0x200,		0	},
	{ "rotz",           Extra1_SelfRotZ,            0,	0x200,		0	},

	{ NULL,				0,						0,	0,			0	}
};

PRIVATE
proplist_s Extra3_PropList[] = {

	{ "lifetime",       Extra1_LifeTimeCount,		0,	100000,		0	},
	{ "rotx",           Extra1_SelfRotX,            0,	0x200,		0	},
	{ "roty",           Extra1_SelfRotY,            0,	0x200,		0	},
	{ "rotz",           Extra1_SelfRotZ,            0,	0x200,		0	},

	{ NULL,				0,						0,	0,			0	}
};

PRIVATE
proplist_s Mine1_PropList[] = {

	{ "lifetime",       Extra1_LifeTimeCount,		0,	100000,		0	},
	{ "rotx",           Extra1_SelfRotX,            0,	0x200,		0	},
	{ "roty",           Extra1_SelfRotY,            0,	0x200,		0	},
	{ "rotz",           Extra1_SelfRotZ,            0,	0x200,		0	},
	{ "hitpoints",      Mine1_HitPoints,            0,	1000,		0	},

	{ NULL,				0,						0,	0,			0	}
};


// convert default types to proplist_s pointers -------------------------------
//
PRIVATE
proplist_s *default_types_proplists[ NUM_DISTINCT_OBJTYPES ] = {

	Ship1_PropList,			// SHIP_1
	Ship1_PropList,         // SHIP_2
	Laser1_PropList,        // LASER_1
	Laser2_PropList,        // LASER_2
	Laser3_PropList,        // LASER_3
	Missile1_PropList,      // MISSL_1
	Missile2_PropList,      // MISSL_2
	Missile3_PropList,      // MISSL_3
	Missile4_PropList,      // MISSL_4
	Missile5_PropList,      // MISSL_5
	Extra1_PropList,        // EXTRA_1
	Extra2_PropList,        // EXTRA_2
	Extra3_PropList,        // EXTRA_3
	Mine1_PropList,         // MINE_1
};


// registered proplists for custom types --------------------------------------
//
PRIVATE
proplist_s *custom_types_proplists[ MAX_NUM_CUSTOM_TYPES ];


// register proplist for custom type ------------------------------------------
//
void CON_RegisterCustomType( dword objtypeid, proplist_s *proplist )
{
	ASSERT( proplist != NULL );

	//NOTE:
	// a type id of TYPE_ID_INVALID will be ignored silently,
	// in order to make the id returned by OBJ_RegisterCustomType()
	// directly useable without explicit error checking.

	if ( objtypeid == TYPE_ID_INVALID )
		return;

	int typeindx = objtypeid & TYPENUMBERMASK;

	ASSERT( typeindx >= NUM_DISTINCT_OBJTYPES );
	if ( typeindx < NUM_DISTINCT_OBJTYPES )
		return;

	typeindx -= NUM_DISTINCT_OBJTYPES;

	ASSERT( typeindx < MAX_NUM_CUSTOM_TYPES );
	if ( typeindx >= MAX_NUM_CUSTOM_TYPES )
		return;

	// register each type exactly once
	ASSERT( custom_types_proplists[ typeindx ] == NULL );
	if ( custom_types_proplists[ typeindx ] != NULL )
		return;

	custom_types_proplists[ typeindx ] = proplist;
}


// fetch pointer to property list for specified type id -----------------------
//
proplist_s *FetchTypePropertyList( dword objtypeid )
{
	ASSERT( objtypeid != TYPE_ID_INVALID );

	//NOTE:
	// may return NULL if no proplist is available
	// for the requested type (especially if custom).

	int typeindx = objtypeid & TYPENUMBERMASK;

	// default type
	if ( typeindx < NUM_DISTINCT_OBJTYPES ) {
		return default_types_proplists[ typeindx ];
	}

	// custom type
	typeindx -= NUM_DISTINCT_OBJTYPES;
	ASSERT( typeindx < MAX_NUM_CUSTOM_TYPES );
	if ( typeindx >= MAX_NUM_CUSTOM_TYPES )
		return NULL;

	// return either NULL or registered proplist
	return custom_types_proplists[ typeindx ];
}


// list of alias names for default object types -------------------------------
//
struct alias_typename_s {

	const char* objtypename;
	dword 	objtypeid;
};

PRIVATE
alias_typename_s AliasTypeNames[ NUM_DISTINCT_OBJTYPES ] = {

	{ "ship_1",		SHIP1TYPE		},
	{ "ship_2",     SHIP2TYPE		},
	{ "laser_1",    LASER1TYPE		},
	{ "laser_2",    LASER2TYPE		},
	{ "laser_3",    LASER3TYPE		},
	{ "missl_1",    MISSILE1TYPE	},
	{ "missl_2",    MISSILE2TYPE	},
	{ "missl_3",    MISSILE3TYPE	},
	{ "missl_4",    MISSILE4TYPE	},
	{ "missl_5",    MISSILE5TYPE	},
	{ "extra_1",    EXTRA1TYPE		},
	{ "extra_2",    EXTRA2TYPE		},
	{ "extra_3",    EXTRA3TYPE		},
	{ "mine_1",     MINE1TYPE		},
};


// fetch name for specified type id -------------------------------------------
//
PRIVATE
const char *FetchTypeName( dword objtypeid )
{
	// check default type ids
	for ( int tid = 0; tid < NUM_DISTINCT_OBJTYPES; tid++ )
		if ( AliasTypeNames[ tid ].objtypeid == objtypeid )
			return AliasTypeNames[ tid ].objtypename;

	// check custom type ids
	dword typeindex = ( objtypeid & TYPENUMBERMASK ) - NUM_DISTINCT_OBJTYPES;
	ASSERT( typeindex < (dword)num_custom_types );

	const char *objtypename = custom_type_info[ typeindex ].type_name;
	ASSERT( objtypename != NULL );

	return objtypename;
}


// build type info line for type list -----------------------------------------
//
char *BuildTypeInfo( int typeindex )
{
	ASSERT( typeindex >= 0 );
	ASSERT( typeindex < NUM_DISTINCT_OBJTYPES + num_custom_types );

	if ( typeindex < NUM_DISTINCT_OBJTYPES ) {
		sprintf( paste_str, "type %d: %s (id %08x)", typeindex,
				 AliasTypeNames[ typeindex ].objtypename,
				 (unsigned int)AliasTypeNames[ typeindex ].objtypeid );
	} else {
		int customindex = typeindex - NUM_DISTINCT_OBJTYPES;
		sprintf( paste_str, "type %d: %s (id %08x)", typeindex,
				 custom_type_info[ customindex ].type_name,
				 (unsigned int)custom_type_info[ customindex ].type_id );
	}

	return paste_str;
}


// list of alias names for the default object classes -------------------------
//
struct alias_classname_s {

	const char*	objclassname;
	dword 	objclassid;
};

PRIVATE
alias_classname_s AliasClassNames[ NUM_ALTERABLE_CLASSES ] = {

	{ "ship1",		SHIP_CLASS_1				},
	{ "ship2",		SHIP_CLASS_2				},
	{ "ship3",		SHIP_CLASS_3				},

	{ "laser1",     LASER0_CLASS_1				},
	{ "laser2",     LASER0_CLASS_2				},
	{ "laser3",     LASER1_CLASS_1				},
	{ "laser4",     LASER2_CLASS_1				},

	{ "missl1",     DUMB_CLASS_1				},
	{ "missl2",     GUIDE_CLASS_1				},
	{ "missl3",     SWARM_CLASS_1				},
	{ "mine1",      MINE_CLASS_1				},

	{ "extra1",     ENERGY_EXTRA_CLASS			},
	{ "extra2",     DUMB_PACK_CLASS				},
	{ "extra3",     GUIDE_PACK_CLASS			},
	{ "extra4",     HELIX_DEVICE_CLASS			},
	{ "extra5",     LIGHTNING_DEVICE_CLASS		},
	{ "extra6",     MINE_PACK_CLASS				},
	{ "extra7",     REPAIR_EXTRA_CLASS			},
	{ "extra8",		AFTERBURNER_DEVICE_CLASS	},
	{ "extra9",		SWARM_PACK_CLASS			},
	{ "extra10",	INVISIBILITY_CLASS			},
	{ "extra11",	PHOTON_DEVICE_CLASS			},
	{ "extra12",	DECOY_DEVICE_CLASS			},
	{ "extra13",	LASERUPGRADE1_CLASS			},
	{ "extra14",	LASERUPGRADE2_CLASS			},
	{ "extra15",	INVULNERABILITY_CLASS		},
};


// build class info line for class list ---------------------------------------
//
char *BuildClassInfo( int classindex )
{
	ASSERT( classindex >= 0 );
	ASSERT( classindex < NumObjClasses );
	ASSERT( NumObjClasses >= NUM_ALTERABLE_CLASSES );
	ASSERT( NumObjClasses == NumLoadedObjects );

	dword objclassid = classindex;
	if ( classindex < NUM_ALTERABLE_CLASSES ) {
		objclassid = AliasClassNames[ classindex ].objclassid;
	}

	ASSERT( objclassid < (dword)NumLoadedObjects );
	dword objtypeid   = ObjectInfo[ objclassid ].type;
	const char *objtypename = FetchTypeName( objtypeid );

	#define MAX_CLASSNAME 31
	char classname[ MAX_CLASSNAME + 1 ];
	strncpy( classname, ObjectInfo[ objclassid ].name, MAX_CLASSNAME );
	classname[ MAX_CLASSNAME ] = 0;

	if ( classindex < NUM_ALTERABLE_CLASSES ) {
		sprintf( paste_str, "class id %u [%s] (%s) type %s", (unsigned int)objclassid,
			AliasClassNames[ classindex ].objclassname, classname, objtypename );
	} else {
		sprintf( paste_str, "class id %u [%s] (%s) type %s", (unsigned int)objclassid,
			"noalias", classname, objtypename );
	}

	return paste_str;
}


// object instance to alter when no class and type is specified below ---------
//
static GenObject *alter_object_instance = NULL;


// create a pointer to a property field ---------------------------------------
//
INLINE
char *MakeFieldPointer( dword objclassid, proplist_s *plist )
{
	ASSERT( plist != NULL );

	//NOTE:
	// ( objclassid == CLASS_ID_INVALID ) means an object
	// instance should be accessed instead of the class itself.

	char *fieldpo;

	if ( objclassid == CLASS_ID_INVALID ) {
		ASSERT( alter_object_instance != NULL );
		fieldpo = plist->propoffset + (char*)alter_object_instance;
	} else {
		ASSERT( objclassid < (dword)NumObjClasses );
		fieldpo = plist->propoffset + (char*)ObjClasses[ objclassid ];
	}

	return fieldpo;
}


// get value and access-type of class/type property ---------------------------
//
PRIVATE
int GetPropertyValue( dword objclassid, proplist_s *plist, int *pint, float *pflt, char **pstr )
{
	ASSERT( plist != NULL );
	ASSERT( pint != NULL );
	ASSERT( pflt != NULL );
	ASSERT( pstr != NULL );

	//NOTE:
	// ( objclassid == CLASS_ID_INVALID ) means an object
	// instance should be queried instead of the class itself.

	char *fieldpo = MakeFieldPointer( objclassid, plist );

	int accesstype;
	switch ( plist->fieldtype ) {

		case PROPTYPE_INT:
			*pint = *(int *)fieldpo;
			accesstype = ACCESSTYPE_INT;
			break;

		case PROPTYPE_FLOAT:
			*pflt = *(float *)fieldpo;
			accesstype = ACCESSTYPE_FLOAT;
			break;

		case PROPTYPE_GEOMV:
			*pflt = GEOMV_TO_FLOAT( *(geomv_t *)fieldpo );
			accesstype = ACCESSTYPE_FLOAT;
			break;

		case PROPTYPE_FIXED:
			*pflt = FIXED_TO_FLOAT( *(fixed_t *)fieldpo );
			accesstype = ACCESSTYPE_FLOAT;
			break;

		case PROPTYPE_STRING:
			*pstr = fieldpo;
			accesstype = ACCESSTYPE_STRING;
			break;

		case PROPTYPE_CHARPTR:
			*pstr = *(char**)fieldpo;
			accesstype = ACCESSTYPE_CHARPTR;
			break;

		default:
			PANIC( 0 );
	}

	return accesstype;
}


// set value of class/type property after converting to correct type ----------
//
PRIVATE
void SetPropertyValue( dword objclassid, proplist_s *plist, int pint, float pflt, char *pstr )
{
	ASSERT( plist != NULL );

	//NOTE:
	// ( objclassid == CLASS_ID_INVALID ) means an object
	// instance should be altered instead of the class itself.

	char *fieldpo = MakeFieldPointer( objclassid, plist );

	// indicate whether the field content has actually changed
	// (i.e. was not already set to this value)
	int fieldchanged = FALSE;

	switch ( plist->fieldtype ) {

		case PROPTYPE_INT:
			fieldchanged = ( *(int *)fieldpo != pint );
			*(int *)fieldpo = pint;
			break;

		case PROPTYPE_FLOAT:
			fieldchanged = ( *(float *)fieldpo != pflt );
			*(float *)fieldpo = pflt;
			break;

		case PROPTYPE_GEOMV:
			fieldchanged = ( *(geomv_t *)fieldpo != FLOAT_TO_GEOMV( pflt ) );
			*(geomv_t *)fieldpo = FLOAT_TO_GEOMV( pflt );
			break;

		case PROPTYPE_FIXED:
			fieldchanged = ( *(fixed_t *)fieldpo != FLOAT_TO_FIXED( pflt ) );
			*(fixed_t *)fieldpo = FLOAT_TO_FIXED( pflt );
			break;

		case PROPTYPE_STRING:
			ASSERT( pstr != NULL );
			if ( strncmp( fieldpo, pstr, plist->bmax ) != 0 ) {
				strncpy( fieldpo, pstr, plist->bmax );
				fieldpo[ plist->bmax ] = 0;
				fieldchanged = TRUE;
			}
			break;

		case PROPTYPE_CHARPTR:
			ASSERT( pstr != NULL );
			ASSERT( *(char**)fieldpo != NULL );
			if ( strncmp( *(char**)fieldpo, pstr, plist->bmax ) != 0 ) {
				FREEMEM( *(char**)fieldpo );
				*(char**)fieldpo = (char *) ALLOCMEM( strlen( pstr ) + 1 );
				if ( *(char**)fieldpo == NULL )
					OUTOFMEM( 0 );
				strcpy( *(char**)fieldpo, pstr );
				fieldchanged = TRUE;
			}
			break;

		default:
			PANIC( 0 );
	}

	if ( fieldchanged ) {

		// only call the notification callback for changed fields
		// of object class instances (and type templates)
		if ( objclassid == CLASS_ID_INVALID ) {

			ASSERT( alter_object_instance != NULL );

			// call notification callback function
			if ( plist->notify_callback != NULL ) {
				(*plist->notify_callback)( alter_object_instance );
			}
		}
	}
}


// set new int-property value if supplied, otherwise just display current -----
//
INLINE
int SetObjPropertyValueInt( dword objclassid, dword objtypeid, proplist_s *plist, int *pint, const char *query )
{
	ASSERT( plist != NULL );
	ASSERT( pint != NULL );
	ASSERT( query != NULL ); // all properties must be queriable right now!

	// query/alter intermediate field
	char *scan = QueryIntArgument( query, pint );
	if ( scan != NULL ) {

		char *errpart;
		long sval = strtol( scan, &errpart, int_calc_base );

		int argvalid = ( *errpart == 0 );

		// check for increment/decrement tokens
		if ( !argvalid ) {
			if( strcmp( scan, "++" ) == 0 ) {
				sval = (*pint) + 1;	
				argvalid = TRUE;
			} else if ( strcmp( scan, "--" ) == 0 ) {
				sval = (*pint) - 1;
				argvalid = TRUE;
			} else if ( strncmp( scan, "+=", 2 ) == 0 ) {
				char* arg = scan + 2;
				long argval = strtol( arg, &errpart, int_calc_base );
				if ( *errpart == 0 ) {
					sval = (*pint) + argval;
					argvalid = TRUE;
				}
			} else if ( strncmp( scan, "-=", 2 ) == 0 ) {
				char* arg = scan + 2;
				long argval = strtol( arg, &errpart, int_calc_base );
				if ( *errpart == 0 ) {
					sval = (*pint) - argval;
					argvalid = TRUE;
				}
			}
		}

		if ( argvalid ) {
			if ( ( sval >= plist->bmin ) &&
				 ( sval <= plist->bmax ) ) {
				*pint = sval;
			} else {
				CON_AddLine( range_error );
				return FALSE;
			}
		} else {
			CON_AddLine( invalid_arg );
			return FALSE;
		}

		// transfer intermediate field into actual property
		SetPropertyValue( objclassid, plist, *pint, 0, NULL );

		// scan other classes of this type and alter also
		if ( ( objtypeid  != TYPE_ID_INVALID  ) &&
			 ( objclassid != CLASS_ID_INVALID ) ) {
			for ( int cid = objclassid + 1; cid < NumObjClasses; cid++ ) {
				if ( ObjClasses[ cid ]->ObjectType == objtypeid ) {
					SetPropertyValue( cid, plist, *pint, 0, NULL );
				}
			}
		}
	}

	return TRUE;
}


// set new float-property value if supplied, otherwise just display current ---
//
INLINE
int SetObjPropertyValueFlt( dword objclassid, dword objtypeid, proplist_s *plist, float *pflt, const char *query )
{
	ASSERT( plist != NULL );
	ASSERT( pflt != NULL );
	ASSERT( query != NULL ); // all properties must be queriable right now!
	
	// query/alter intermediate field
	char *scan = QueryFltArgument( query, pflt );
	if ( scan != NULL ) {
		
		char *errpart;
		float sval = strtod( scan, &errpart );
		
		int argvalid = ( *errpart == 0 );

		// check for increment/decrement tokens
		if ( !argvalid ) {
			if( strcmp( scan, "++" ) == 0 ) {
				sval = (*pflt) + 1.0f;	
				argvalid = TRUE;
			} else if ( strcmp( scan, "--" ) == 0 ) {
				sval = (*pflt) - 1.0f;
				argvalid = TRUE;
			} else if ( strncmp( scan, "+=", 2 ) == 0 ) {
				char* arg = scan + 2;
				float argval = strtod( arg, &errpart );
				if ( *errpart == 0 ) {
					sval = (*pflt) + argval;
					argvalid = TRUE;
				}
			} else if ( strncmp( scan, "-=", 2 ) == 0 ) {
				char* arg = scan + 2;
				float argval = strtod( arg, &errpart );
				if ( *errpart == 0 ) {
					sval = (*pflt) - argval;
					argvalid = TRUE;
				}
			}
		}

		if ( argvalid ) {
			if ( ( FLOAT_TO_FIXED( sval ) >= plist->bmin ) &&
				 ( FLOAT_TO_FIXED( sval ) <= plist->bmax ) ) {
				*pflt = sval;
			} else {
				CON_AddLine( range_error );
				return FALSE;
			}
		} else {
			CON_AddLine( invalid_arg );
			return FALSE;
		}

		// transfer intermediate field into actual property
		SetPropertyValue( objclassid, plist, 0, *pflt, NULL );

		// scan other classes of this type and alter also
		if ( ( objtypeid  != TYPE_ID_INVALID  ) &&
			 ( objclassid != CLASS_ID_INVALID ) ) {
			for ( int cid = objclassid + 1; cid < NumObjClasses; cid++ ) {
				if ( ObjClasses[ cid ]->ObjectType == objtypeid ) {
					SetPropertyValue( cid, plist, 0, *pflt, NULL );
				}
			}
		}
	}

	return TRUE;
}


// set new float-property value if supplied, otherwise just display current ---
//
INLINE
int SetObjPropertyValueStr( dword objclassid, dword objtypeid, proplist_s *plist, char *pstr )
{
	ASSERT( plist != NULL );
	ASSERT( pstr != NULL );

	char *scan = strtok( NULL, "" );
	if ( scan != NULL ) {

		// eat whitespace and leading quotations
		while ( ( *scan == ' ' ) || ( *scan == '"' ) )
			scan++;

		// argument found?
		if ( *scan != 0 ) {

			// cut off trailing whitespace and quotations
			char *start = scan;
			while ( *scan != 0 )
				scan++;
			scan--;
			while ( ( *scan == ' ' ) || ( *scan == '"' ) )
				scan--;
			*( scan + 1 ) = 0;

			// store string into field
			SetPropertyValue( objclassid, plist, 0, 0, start );

			// scan other classes of this type and alter also
			if ( ( objtypeid  != TYPE_ID_INVALID  ) &&
				 ( objclassid != CLASS_ID_INVALID ) ) {
				for ( int cid = objclassid + 1; cid < NumObjClasses; cid++ ) {
					if ( ObjClasses[ cid ]->ObjectType == objtypeid ) {
						SetPropertyValue( objclassid, plist, 0, 0, start );
					}
				}
			}

			return TRUE;
		}
	}

	// display current value
	CON_AddLine( pstr );

	return TRUE;
}


// set new property value if supplied, otherwise just display current ---------
//
PRIVATE
void SetObjPropertyValue( dword objclassid, dword objtypeid, proplist_s *plist, const char *query )
{
	ASSERT( plist != NULL );
	ASSERT( query != NULL ); // all properties must be queriable right now!

	int		pint;
	float	pflt;
	char*	pstr;
	int acct = GetPropertyValue( objclassid, plist, &pint, &pflt, &pstr );

	//NOTE:
	// ( objclassid == CLASS_ID_INVALID ) && ( objtypeid == TYPE_ID_INVALID )
	// means an object instance should be altered instead of the class itself.
	//
	// ( objtypeid == TYPE_ID_INVALID ) means a class is being altered, resulting
	// in a change of only a single class's property field.
	//
	// ( objtypeid != TYPE_ID_INVALID ) means a type is being altered, resulting
	// in a change of property fields of all classes belonging to this type.

	switch ( acct ) {

		case ACCESSTYPE_INT:
			SetObjPropertyValueInt( objclassid, objtypeid, plist, &pint, query );
			break;

		case ACCESSTYPE_FLOAT:
			SetObjPropertyValueFlt( objclassid, objtypeid, plist, &pflt, "%f" );
			break;

		case ACCESSTYPE_STRING:
		case ACCESSTYPE_CHARPTR:
			SetObjPropertyValueStr( objclassid, objtypeid, plist, pstr );
			break;
	}
}


// resolve type name to type id with aliases and custom support ---------------
//
PRIVATE
dword AliasResolveTypeName( const char *name )
{
	ASSERT( name != NULL );
    dword tid;
	// check type name aliases
	for ( tid = 0; tid < NUM_DISTINCT_OBJTYPES; tid++ )
		if ( strcmp( name, AliasTypeNames[ tid ].objtypename ) == 0 )
			break;
	if ( tid != NUM_DISTINCT_OBJTYPES ) {
		ASSERT( ( AliasTypeNames[ tid ].objtypeid & TYPENUMBERMASK ) == tid );
		return AliasTypeNames[ tid ].objtypeid;
	}

	// check custom type names
	for ( tid = 0; tid < (dword)num_custom_types; tid++ )
		if ( stricmp( name, custom_type_info[ tid ].type_name ) == 0 )
			break;
	if ( tid != (dword)num_custom_types ) {
		return custom_type_info[ tid ].type_id;
	}

	return TYPE_ID_INVALID;
}


// check argument that alters an object type property -------------------------
//
int CheckSetObjTypeProperty( const char *query, const char *scan )
{
	ASSERT( query != NULL ); // all properties must be queriable right now!
	ASSERT( scan != NULL );

	// check if prop command
	if ( strcmp( scan, CMSTR( CM_PROP ) ) != 0 )
		return FALSE;

	// isolate type name field
	char *objtypename = strtok( NULL, "." );
	if ( objtypename == NULL ) {
		CON_AddLine( prop_syntax );
		return TRUE;
	}

	// resolve name to id
	dword objtypeid = AliasResolveTypeName( objtypename );
	if ( objtypeid == TYPE_ID_INVALID ) {
		CON_AddLine( prop_wrong_type );
		return TRUE;
	}

	// isolate property name field
	char *fieldname = strtok( NULL, " " );
	if ( fieldname == NULL ) {
		CON_AddLine( prop_syntax );
		return TRUE;
	}

	// check if property name exists for specified type
	int propfound = FALSE;

	proplist_s *plist = FetchTypePropertyList( objtypeid );
	if ( plist == NULL ) {
		CON_AddLine( not_accessible );
		return TRUE;
	}

	for ( ; plist->propname; plist++ ) {
		if ( stricmp( plist->propname, fieldname ) == 0 ) {
			propfound  = TRUE;
			break;
		}
	}
	if ( !propfound ) {
		CON_AddLine( prop_wrong_field );
		return TRUE;
	}

	// default: use instance/template
	dword objclassid = CLASS_ID_INVALID;
	ASSERT( alter_object_instance == NULL );

	// try to find type template (for custom types only)
	if ( TYPEID_TYPE_CUSTOM( objtypeid ) ) {

		// (ab)use instance pointer for template
		alter_object_instance = OBJ_FetchCustomTypeTemplate( objtypeid );
	}

	// find class if no template available
	if ( alter_object_instance == NULL ) {

		//NOTE:
		// the first class of this type encountered is taken
		// at this time. still, the selected property of all
		// classes of this type will be altered, since
		// SetObjPropertyValue() continues the search in the
		// class list.
        int cid;
		// scan object class list
		for ( cid = 0; cid < NumObjClasses; cid++ )
			if ( ObjClasses[ cid ]->ObjectType == objtypeid )
				break;
		if ( cid == NumObjClasses ) {
			CON_AddLine( no_objs_of_type );
			return TRUE;
		}

		objclassid = cid;
	}

	// set new value if supplied, otherwise just display current value
	SetObjPropertyValue( objclassid, objtypeid, plist, query );

	// reset instance pointer
	alter_object_instance = NULL;

	return TRUE;
}


// resolve class name to class id with aliases --------------------------------
//
PRIVATE
dword AliasResolveClassName( const char *name )
{
	ASSERT( name != NULL );
    int cid;
	// check class name aliases
	int propindx = -1;
	for ( cid = 0; cid < NUM_ALTERABLE_CLASSES; cid++ ) {
		if ( strcmp( name, AliasClassNames[ cid ].objclassname ) == 0 ) {
			propindx = cid;
			break;
		}
	}
	if ( propindx != -1 ) {
		return AliasClassNames[ propindx ].objclassid;
	}

	// check original class names
	ASSERT( NumObjClasses >= NUM_ALTERABLE_CLASSES );
	for ( cid = 0; cid < NumObjClasses; cid++ ) {
		if ( stricmp( name, ObjectInfo[ cid ].name ) == 0 ) {
			propindx = cid;
			break;
		}
	}
	if ( propindx != -1 ) {
		return propindx;
	}

	return CLASS_ID_INVALID;
}


// check argument that alters an object class property ------------------------
//
int CheckSetObjClassProperty( const char *query, const char *scan )
{
	ASSERT( query != NULL ); // all properties must be queriable right now!
	ASSERT( scan != NULL );

	// check if propc command
	if ( strcmp( scan, CMSTR( CM_PROPC ) ) != 0 )
		return FALSE;

	// isolate class name field
	char *objclassname = strtok( NULL, "." );
	if ( objclassname == NULL ) {
		CON_AddLine( propc_syntax );
		return TRUE;
	}

	// allow class name to be parenthesized to include whitespace
	objclassname = GetParenthesizedName( objclassname );
	if ( objclassname == NULL ) {
		CON_AddLine( propc_syntax );
		return TRUE;
	}

	// resolve name to id
	dword objclassid = AliasResolveClassName( objclassname );
	if ( objclassid == CLASS_ID_INVALID ) {
		CON_AddLine( prop_wrong_class );
		return TRUE;
	}

	// isolate property name field
	char *fieldname = strtok( NULL, " " );
	if ( fieldname == NULL ) {
		CON_AddLine( propc_syntax );
		return TRUE;
	}

	ASSERT( objclassid < (dword)NumObjClasses );
	dword objtypeid	= ObjectInfo[ objclassid ].type;

	// check if property name exists for specified class
	int propfound = FALSE;

	proplist_s *plist = FetchTypePropertyList( objtypeid );
	if ( plist == NULL ) {
		CON_AddLine( not_accessible );
		return TRUE;
	}

	for ( ; plist->propname; plist++ ) {
		if ( stricmp( plist->propname, fieldname ) == 0 ) {
			propfound  = TRUE;
			break;
		}
	}
	if ( !propfound ) {
		CON_AddLine( prop_wrong_field );
		return TRUE;
	}

	// check if class is valid (i.e., actually memory resident)
	if ( objclassid >= (dword)NumObjClasses ) {
		CON_AddLine( class_not_loaded );
		return TRUE;
	}

	// set new value if supplied, otherwise just display current value
	SetObjPropertyValue( objclassid, TYPE_ID_INVALID, plist, query );

	return TRUE;
}


// check argument that alters an object instance property ---------------------
//
int CheckSetObjInstanceProperty( const char *query, const char *scan )
{
	ASSERT( query != NULL ); // all properties must be queriable right now!
	ASSERT( scan != NULL );

	// check if propo command
	if ( strcmp( scan, CMSTR( CM_PROPO ) ) != 0 )
		return FALSE;

	// isolate object id field
	char *objidstr = strtok( NULL, "." );
	if ( objidstr == NULL ) {
		CON_AddLine( propo_syntax );
		return TRUE;
	}

	// convert object id string to int
	char *errpart;
	dword objid = strtol( objidstr, &errpart, 10/*int_calc_base*/ );
	if ( *errpart != 0 ) {
		CON_AddLine( prop_objid_syntax );
		return TRUE;
	}

	//NOTE:
	// id 0 means use id of most recently
	// summoned object.

	if ( objid == 0 ) {
		objid = TheWorld->GetLastSummonedObjectID();
		if ( !SV_CONSOLE_LEVEL_MESSAGES ) {
			sprintf( paste_str, "using object id %u.", (unsigned int)objid );
			CON_AddLine( paste_str );
		}
	}

	// isolate property name field
	char *fieldname = strtok( NULL, " " );
	if ( fieldname == NULL ) {
		CON_AddLine( propo_syntax );
		return TRUE;
	}

	// find object
	GenObject *obj = FetchObject( objid );
	if ( obj == NULL ) {
		CON_AddLine( prop_no_object );
		return TRUE;
	}

	// check if property name exists
	int propfound = FALSE;

	proplist_s *plist = FetchTypePropertyList( obj->ObjectType );
	if ( plist == NULL ) {
		CON_AddLine( not_accessible );
		return TRUE;
	}

	for ( ; plist->propname; plist++ ) {
		if ( stricmp( plist->propname, fieldname ) == 0 ) {
			propfound  = TRUE;
			break;
		}
	}
	if ( !propfound ) {
		CON_AddLine( prop_wrong_field );
		return TRUE;
	}

	// set new value if supplied, otherwise just display current value
	alter_object_instance = obj;
	SetObjPropertyValue( CLASS_ID_INVALID, TYPE_ID_INVALID, plist, query );
	alter_object_instance = NULL;

	return TRUE;
}


// build description for type property of numeric type ------------------------
//
PRIVATE
void DescTypeNum( const char *name, proplist_s *plist, int acct )
{
	static char format[] = "%s.%s %s (min=%d max=%d)";
	const char typespec = ( acct == ACCESSTYPE_INT ) ? int_print_base[ 1 ] : 'f';
	format[ 15 ]  = typespec;
	format[ 22 ]  = typespec;

	if ( acct == ACCESSTYPE_INT ) {
		sprintf( paste_str, format, name, plist->propname, "int",
				 plist->bmin, plist->bmax );
	} else {
		sprintf( paste_str, format, name, plist->propname, "float",
				 FIXED_TO_FLOAT( plist->bmin ),
				 FIXED_TO_FLOAT( plist->bmax ) );
	}
}


// build description for type property of string type -------------------------
//
PRIVATE
void DescTypeStr( const char *name, proplist_s *plist )
{
	sprintf( paste_str, "%s.%s string (min=%d max=%d)",
			 name, plist->propname, plist->bmin, plist->bmax );
}


// build description for class property of numeric type -----------------------
//
PRIVATE
void DescClassNum( const char *name, proplist_s *plist, int acct, int pint, float pflt )
{
	static char format[] = "%s.%s %d (%s min=%d max=%d)";
	char typespec = ( acct == ACCESSTYPE_INT ) ? int_print_base[ 1 ] : 'f';
	format[ 7 ]  = typespec;
	format[ 18 ] = typespec;
	format[ 25 ] = typespec;

	if ( acct == ACCESSTYPE_INT ) {
		sprintf( paste_str, format, name, plist->propname, pint, "int",
				 plist->bmin, plist->bmax );
	} else {
		sprintf( paste_str, format, name, plist->propname, pflt, "float",
				 FIXED_TO_FLOAT( plist->bmin ),
				 FIXED_TO_FLOAT( plist->bmax ) );
	}
}


// build description for class property of string type ------------------------
//
PRIVATE
void DescClassStr( const char *name, proplist_s *plist, const char *pstr )
{
	sprintf( paste_str, "%s.%s \"%s\" (min=%d max=%d)",
			 name, plist->propname, pstr, plist->bmin, plist->bmax );
}


// print various info about specific object type ------------------------------
//
int CheckTypeInfo( const char *scan )
{
	ASSERT( scan != NULL );

	// check if typeinfo command
	if ( strcmp( scan, CMSTR( CM_TYPEINFO ) ) != 0 )
		return FALSE;

	// isolate type name field
	char *objtypename = strtok( NULL, " " );
	if ( objtypename == NULL ) {
		CON_AddLine( typeinfo_syntax );
		return TRUE;
	}

	//NOTE:
	// parenthesized type names are not
	// allowed.

	// check if too many fields supplied
	if ( strtok( NULL, " " ) != NULL ) {
		CON_AddLine( typeinfo_syntax );
		return TRUE;
	}

	// resolve name to id
	dword objtypeid = AliasResolveTypeName( objtypename );
	if ( objtypeid == TYPE_ID_INVALID ) {
		CON_AddLine( prop_wrong_type );
		return TRUE;
	}

	// print info header
	sprintf( paste_str, "--properties of type %s (%08x_h):",
			 objtypename, (unsigned int)objtypeid );
	CON_AddLine( paste_str );

	// default: use instance/template
	dword objclassid = CLASS_ID_INVALID;
	ASSERT( alter_object_instance == NULL );

	// try to find type template (for custom types only)
	if ( TYPEID_TYPE_CUSTOM( objtypeid ) ) {

		// (ab)use instance pointer for template
		alter_object_instance = OBJ_FetchCustomTypeTemplate( objtypeid );
	}

	// find class if no template available
	if ( alter_object_instance == NULL ) {

		// determine whether there are classes of this type
		// (scan object class list for class of this type)
		for ( int cid = 0; cid < NumObjClasses; cid++ ) {
			if ( ObjClasses[ cid ]->ObjectType == objtypeid ) {
				objclassid = cid;
				break;
			}
		}

		//NOTE:
		// to display info about a type any class of this type
		// may be used! (that is, the first class of this type
		// contained in the class-list is used.) this is the
		// behavior that should be expected, but it is only
		// guaranteed to actually obtain homogeneous type-info
		// if no class of this type has been altered separately!

		if ( objclassid == CLASS_ID_INVALID ) {
			CON_AddLine( "(there are no objects of this type)" );
		}
	}

	// fetch property descriptions
	proplist_s *plist = FetchTypePropertyList( objtypeid );
	if ( plist == NULL ) {
		CON_AddLine( not_accessible );
		return TRUE;
	}

	// print list of properties (and their current values
	// if the type has been instanced/templatized)
	for ( ; plist->propname; plist++ ) {

		if ( ( objclassid != CLASS_ID_INVALID ) ||
			 ( alter_object_instance != NULL ) ) {

			int		pint = 0;
			float	pflt = 0.0f;
			char*	pstr = NULL;
			int acct = GetPropertyValue( objclassid, plist, &pint, &pflt, &pstr );

			switch ( acct ) {

				case ACCESSTYPE_INT:
				case ACCESSTYPE_FLOAT:
					DescClassNum( objtypename, plist, acct, pint, pflt );
					break;

				case ACCESSTYPE_STRING:
				case ACCESSTYPE_CHARPTR:
					DescClassStr( objtypename, plist, pstr );
					break;
			}

		} else {

			switch ( plist->fieldtype ) {

				case PROPTYPE_INT:
					DescTypeNum( objtypename, plist, ACCESSTYPE_INT );
					break;

				case PROPTYPE_FLOAT:
				case PROPTYPE_GEOMV:
				case PROPTYPE_FIXED:
					DescTypeNum( objtypename, plist, ACCESSTYPE_FLOAT );
					break;

				case PROPTYPE_STRING:
				case PROPTYPE_CHARPTR:
					DescTypeStr( objtypename, plist );
					break;
			}
		}

		CON_AddLine( paste_str );
	}

	// reset instance pointer
	alter_object_instance = NULL;

	return TRUE;
}


// print various info about specific object class -----------------------------
//
int CheckClassInfo( const char *scan )
{
	ASSERT( scan != NULL );

	// check if classinfo command
	if ( strcmp( scan, CMSTR( CM_CLASSINFO ) ) != 0 )
		return FALSE;

	// isolate class name field
	char *objclassname = strtok( NULL, " " );
	if ( objclassname == NULL ) {
		CON_AddLine( classinfo_syntax );
		return TRUE;
	}

	// allow class name to be parenthesized to include whitespace
	objclassname = GetParenthesizedName( objclassname );
	if ( objclassname == NULL ) {
		CON_AddLine( classinfo_syntax );
		return TRUE;
	}

	// check if too many fields supplied
	if ( strtok( NULL, " " ) != NULL ) {
		CON_AddLine( classinfo_syntax );
		return TRUE;
	}

	// resolve name to id
	dword objclassid = AliasResolveClassName( objclassname );
	if ( objclassid == CLASS_ID_INVALID ) {
		CON_AddLine( prop_wrong_class );
		return TRUE;
	}

	// check if class is valid (i.e., actually memory resident)
	if ( objclassid >= (dword)NumObjClasses ) {
		CON_AddLine( class_not_loaded );
		return TRUE;
	}

	ASSERT( objclassid < (dword)NumObjClasses );
	dword objtypeid = ObjectInfo[ objclassid ].type;

	// print info header
	sprintf( paste_str, "--properties of class %s (%02u_d) (type is %s):",
			 objclassname, (unsigned int)objclassid, FetchTypeName( objtypeid ) );
	CON_AddLine( paste_str );

	// fetch property descriptions
	proplist_s *plist = FetchTypePropertyList( objtypeid );
	if ( plist == NULL ) {
		CON_AddLine( not_accessible );
		return TRUE;
	}

	// print list of properties with current values
	for ( ; plist->propname; plist++ ) {

		int		pint = 0;
		float	pflt = 0.0f;
		char*	pstr = NULL;
		int acct = GetPropertyValue( objclassid, plist, &pint, &pflt, &pstr );

		switch ( acct ) {

			case ACCESSTYPE_INT:
			case ACCESSTYPE_FLOAT:
				DescClassNum( objclassname, plist, acct, pint, pflt );
				break;

			case ACCESSTYPE_STRING:
			case ACCESSTYPE_CHARPTR:
				DescClassStr( objclassname, plist, pstr );
				break;
		}

		CON_AddLine( paste_str );
	}

	return TRUE;
}


// print various info about specific object (class instance) ------------------
//
int CheckInstanceInfo( const char *scan )
{
	ASSERT( scan != NULL );

	// check if objectinfo command
	if ( strcmp( scan, CMSTR( CM_OBJECTINFO ) ) != 0 )
		return FALSE;

	// isolate object id field
	char *objidstr = strtok( NULL, " " );
	if ( objidstr == NULL ) {
		CON_AddLine( objectinfo_syntax );
		return TRUE;
	}

	// convert object id string to int
	char *errpart;
	dword objid = strtol( objidstr, &errpart, 10/*int_calc_base*/ );
	if ( *errpart != 0 ) {
		CON_AddLine( objectinfo_syntax );
		return TRUE;
	}

	// check if too many fields supplied
	if ( strtok( NULL, " " ) != NULL ) {
		CON_AddLine( objectinfo_syntax );
		return TRUE;
	}
	
	//NOTE:
	// id 0 means use id of most recently
	// summoned object.
	
	if ( objid == 0 ) {
		objid = TheWorld->GetLastSummonedObjectID();
		if ( !SV_CONSOLE_LEVEL_MESSAGES ) {
			sprintf( paste_str, "using object id %u.", (unsigned int)objid );
			CON_AddLine( paste_str );
		}
	}
	
	// find object
	GenObject *obj = FetchObject( objid );
	if ( obj == NULL ) {
		CON_AddLine( prop_no_object );
		return TRUE;
	}
	
	dword objclassid = obj->ObjectClass;
	dword objtypeid  = obj->ObjectType;
	
	ASSERT( objclassid < (dword)NumObjClasses );
	char *objclassname = ObjectInfo[ objclassid ].name;
	
	// print info header
	sprintf( paste_str, "--properties of object %u (class %s id %u, type %s):",
		(unsigned int)objid, objclassname, (unsigned int)objclassid, FetchTypeName( objtypeid ) );
	CON_AddLine( paste_str );
	
	// fetch property descriptions
	proplist_s *plist = FetchTypePropertyList( objtypeid );
	if ( plist == NULL ) {
		CON_AddLine( not_accessible );
		return TRUE;
	}
	
	// set instance pointer
	alter_object_instance = obj;
	
	// print list of properties with current values
	for ( ; plist->propname; plist++ ) {
		
		int		pint = 0;
		float	pflt = 0.0f;
		char*	pstr = NULL;
		int acct = GetPropertyValue( CLASS_ID_INVALID,
			plist, &pint, &pflt, &pstr );
		
		switch ( acct ) {
			
			case ACCESSTYPE_INT:
			case ACCESSTYPE_FLOAT:
				DescClassNum( objclassname, plist, acct, pint, pflt );
				break;
			
			case ACCESSTYPE_STRING:
			case ACCESSTYPE_CHARPTR:
				DescClassStr( objclassname, plist, pstr );
				break;
		}
		
		CON_AddLine( paste_str );
	}
	
	// reset instance pointer
	alter_object_instance = NULL;
	
	return TRUE;
}

// write the properties of a instance to a console script ( propo 0.????? ) ---
//
int WritePropList( FILE* fp, dword objid )
{
	ASSERT( fp != NULL );

	// find object
	GenObject *obj = FetchObject( objid );
	if ( obj == NULL ) {
		CON_AddLine( prop_no_object );
		return TRUE;
	}
	
	dword objclassid = obj->ObjectClass;
	dword objtypeid  = obj->ObjectType;
	
	ASSERT( objclassid < (dword)NumObjClasses );
	char *objclassname = ObjectInfo[ objclassid ].name;
	
	// print info header
	fprintf( fp, "; properties of object %d (class %s id %d, type %s)\n",
		objid, objclassname, objclassid, FetchTypeName( objtypeid ) );
	
	// fetch property descriptions
	proplist_s *plist = FetchTypePropertyList( objtypeid );
	if ( plist == NULL ) {
		CON_AddLine( not_accessible );
		return TRUE;
	}
	
	// set instance pointer
	alter_object_instance = obj;
	
	// print list of properties with current values
	for ( ; plist->propname; plist++ ) {
		
		int		pint;
		float	pflt;
		char*	pstr;
		int acct = GetPropertyValue( CLASS_ID_INVALID,
			plist, &pint, &pflt, &pstr );
		switch ( acct ) {
			
		case ACCESSTYPE_INT:
			fprintf( fp, "propo 0.%s %d\n", plist->propname, pint ); 
			break;
		case ACCESSTYPE_FLOAT:
			fprintf( fp, "propo 0.%s %f\n", plist->propname, pflt ); 
			break;
		case ACCESSTYPE_STRING:
		case ACCESSTYPE_CHARPTR:
			fprintf( fp, "propo 0.%s \"%s\"\n", plist->propname, pstr ); 
			break;
		}
	}
	
	// reset instance pointer
	alter_object_instance = NULL;
	
	return TRUE;
}




// key table for faceinfo command ---------------------------------------------
//
key_value_s faceinfo_key_value[] = {

	{ "class",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "classid",	NULL,	KEYVALFLAG_NONE				},
	{ "faceid",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "shader",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "texture",	NULL,	KEYVALFLAG_PARENTHESIZE		},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_FACEINFO_CLASS,
	KEY_FACEINFO_CLASSID,
	KEY_FACEINFO_FACEID,
	KEY_FACEINFO_SHADER,
	KEY_FACEINFO_TEXTURE,
};


// face search callback vars --------------------------------------------------
//
static int (*face_cmp_function)( GenObject*, int );
static TextureMap *facesearch_texture;


// return whether a face has the criterion texture ----------------------------
//
PRIVATE
int FaceHasTexture( GenObject *gobj, int faceid )
{
	ASSERT( gobj != NULL );
	ASSERT( ( faceid >= 0 ) && ( (unsigned int)faceid < gobj->NumFaces ) );

	return ( gobj->FaceList[ faceid ].TexMap == facesearch_texture );
}


// display list of face ids satisfying a specific criterion -------------------
//
PRIVATE
void DisplayFaceList( GenObject *gobj )
{
	ASSERT( gobj != NULL );
	ASSERT( face_cmp_function != NULL );

	int outpos = 0;

	for ( unsigned int fid = 0; fid < gobj->NumFaces; fid++ ) {

		if ( (*face_cmp_function)( gobj, fid ) ) {

			sprintf( paste_str + outpos, "%3d, ", fid );
			outpos += strlen( paste_str + outpos );

			if ( outpos > 60 ) {

				// strip ", " and display accumulated line
				paste_str[ outpos - 2 ] = 0;
				CON_AddLine( paste_str );
				outpos = 0;
			}
		}
	}

	if ( outpos > 0 ) {

		// strip ", " and display last line
		paste_str[ outpos - 2 ] = 0;
		CON_AddLine( paste_str );
	}
}

/*
// command "faceinfo" to display various object face information --------------
//
int Cmd_FaceInfo( const char *command )
{
	//NOTE:
	//CONCOM:
	// faceinfo_command ::= 'faceinfo' <class-spec> <face-spec>
	// class_spec       ::= <class-name> | <class-id>
	// class_name       ::= 'class' <name>
	// class_id         ::= 'classid' <int>
	// face_spec        ::= <face-id-spec> | <attrib-spec>
	// face_id_spec     ::= 'faceid' <face-id>
	// attrib_spec      ::= <shader-spec> | <tex-spec>
	// shader_spec      ::= 'shader' <shader-iter> | <shader-name>
	// shader_iter      ::= '(' 'iter_xx'* 'flag_xx'* ')'
	// shader_name      ::= "valid name of shader"
	// tex_spec          ::= 'texture' <texture-name>

	ASSERT( command != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( command );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( faceinfo_key_value, command ) )
		return TRUE;

	// get object class (either name or id)
	dword objclass = ScanKeyValueObjClass( faceinfo_key_value, KEY_FACEINFO_CLASS, KEY_FACEINFO_CLASSID );
	if ( objclass == CLASS_ID_INVALID ) {
		return TRUE;
	}

	GenObject *gobj = ObjClasses[ objclass ];

	// guard against empty base objects
	if ( gobj->NumFaces == 0 ) {
		CON_AddLine( base_object_empty );
		return TRUE;
	}

	// display info about specified face
	char *faceid = faceinfo_key_value[ KEY_FACEINFO_FACEID ].value;
	if ( faceid != NULL ) {

		dword maxfaceid = gobj->NumFaces - 1;

		// read face id
		int faceid;
		if ( ScanKeyValueInt( &faceinfo_key_value[ KEY_FACEINFO_FACEID ], &faceid ) < 0 ) {
			CON_AddLine( face_id_invalid );
			return TRUE;
		}
		if ( ( faceid < 0 ) || ( faceid > maxfaceid ) ) {
			CON_AddLine( face_id_invalid );
			return TRUE;
		}

		//TODO:
		ASSERT( 0 );

		return TRUE;
	}

	// list all faces with specified texture
	char *texname = faceinfo_key_value[ KEY_FACEINFO_TEXTURE ].value;
	if ( texname != NULL ) {

		TextureMap *texmap = FetchTextureMap( texname );
		if ( texmap == NULL ) {
			CON_AddLine( no_texture_found );
			return TRUE;
		}

		face_cmp_function  = FaceHasTexture;
		facesearch_texture = texmap;
		DisplayFaceList( gobj );

		return TRUE;
	}

	// list all faces with specified shader
	char *shaderspec = faceinfo_key_value[ KEY_FACEINFO_SHADER ].value;
	if ( shaderspec != NULL ) {

		//TODO:
		ASSERT( 0 );

//		face_cmp_function  = FaceHasShader;
//		facesearch_texture = shader;
//		DisplayFaceList( gobj );

		return TRUE;
	}

	CON_AddLine( no_face_selected );
	return TRUE;
}
*/


